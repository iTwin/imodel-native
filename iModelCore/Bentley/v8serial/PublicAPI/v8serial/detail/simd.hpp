/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#ifndef V8SERIAL_DETAIL_SIMD_HPP
#define V8SERIAL_DETAIL_SIMD_HPP

#include <cstddef>
#include <cstdint>

#if !defined(V8SERIAL_DISABLE_SIMD) && defined(_M_ARM64)
#include <arm64_neon.h>
#elif !defined(V8SERIAL_DISABLE_SIMD) && defined(__aarch64__)
#include <arm_neon.h>
#elif !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__SSE2__) || defined(_M_X64))
#include <emmintrin.h>
#endif

namespace v8serial::detail {

#if defined(V8SERIAL_DISABLE_SIMD) && defined(__clang__)
#define V8SERIAL_DETAIL_SCALAR_LOOP \
  _Pragma("clang loop vectorize(disable) interleave(disable)")
#elif defined(V8SERIAL_DISABLE_SIMD) && defined(__GNUC__)
#define V8SERIAL_DETAIL_SCALAR_LOOP _Pragma("GCC novector")
#elif defined(V8SERIAL_DISABLE_SIMD) && defined(_MSC_VER)
#define V8SERIAL_DETAIL_SCALAR_LOOP __pragma(loop(no_vector))
#else
#define V8SERIAL_DETAIL_SCALAR_LOOP
#endif

inline bool allLatin1(const char16_t* input, size_t size) {
  size_t index = 0;
#if !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__aarch64__) || defined(_M_ARM64))
  const auto* raw = reinterpret_cast<const uint16_t*>(input);
  // Accumulate four vectors before the horizontal reduction: vmaxvq_u16 is a
  // cross-lane operation, so folding with vorrq_u16 first amortizes it over
  // 32 code units instead of paying it every 8.
  for (; index + 32 <= size; index += 32) {
    const uint16x8_t folded =
        vorrq_u16(vorrq_u16(vld1q_u16(raw + index), vld1q_u16(raw + index + 8)),
                  vorrq_u16(vld1q_u16(raw + index + 16),
                            vld1q_u16(raw + index + 24)));
    if (vmaxvq_u16(folded) > 0xffU) return false;
  }
  for (; index + 8 <= size; index += 8) {
    if (vmaxvq_u16(vld1q_u16(raw + index)) > 0xffU) return false;
  }
#elif !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__SSE2__) || defined(_M_X64))
  const __m128i high_byte_mask = _mm_set1_epi16(static_cast<short>(0xff00));
  const __m128i zero = _mm_setzero_si128();
  for (; index + 32 <= size; index += 32) {
    const auto* block =
        reinterpret_cast<const __m128i*>(input + index);
    const __m128i folded =
        _mm_or_si128(_mm_or_si128(_mm_loadu_si128(block),
                                  _mm_loadu_si128(block + 1)),
                     _mm_or_si128(_mm_loadu_si128(block + 2),
                                  _mm_loadu_si128(block + 3)));
    if (_mm_movemask_epi8(
            _mm_cmpeq_epi8(_mm_and_si128(folded, high_byte_mask), zero)) !=
        0xffff) {
      return false;
    }
  }
  for (; index + 8 <= size; index += 8) {
    const __m128i value = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(input + index));
    const __m128i high_bytes = _mm_and_si128(value, high_byte_mask);
    if (_mm_movemask_epi8(_mm_cmpeq_epi8(high_bytes, zero)) != 0xffff) {
      return false;
    }
  }
#endif
  V8SERIAL_DETAIL_SCALAR_LOOP
  for (; index < size; ++index) {
    if (input[index] > 0xffU) return false;
  }
  return true;
}

/// Narrows Latin-1 code units to bytes.
///
/// Every input code unit must be <= 0xff; callers establish that with
/// allLatin1(). Out-of-range input produces unspecified output bytes.
inline void narrowLatin1(const char16_t* input, uint8_t* output, size_t size) {
  size_t index = 0;
#if !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__aarch64__) || defined(_M_ARM64))
  const auto* raw = reinterpret_cast<const uint16_t*>(input);
  // Pair the narrowed halves into full 128-bit stores so each iteration
  // retires 32 code units instead of 8.
  for (; index + 32 <= size; index += 32) {
    vst1q_u8(output + index,
             vcombine_u8(vmovn_u16(vld1q_u16(raw + index)),
                         vmovn_u16(vld1q_u16(raw + index + 8))));
    vst1q_u8(output + index + 16,
             vcombine_u8(vmovn_u16(vld1q_u16(raw + index + 16)),
                         vmovn_u16(vld1q_u16(raw + index + 24))));
  }
  for (; index + 8 <= size; index += 8) {
    vst1_u8(output + index, vmovn_u16(vld1q_u16(raw + index)));
  }
#elif !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__SSE2__) || defined(_M_X64))
  const __m128i zero = _mm_setzero_si128();
  // Pack two source vectors per iteration so the store writes all 16 bytes.
  for (; index + 16 <= size; index += 16) {
    const __m128i low = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(input + index));
    const __m128i high = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(input + index + 8));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output + index),
                     _mm_packus_epi16(low, high));
  }
  for (; index + 8 <= size; index += 8) {
    const __m128i value = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(input + index));
    const __m128i narrowed = _mm_packus_epi16(value, zero);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(output + index), narrowed);
  }
#endif
  V8SERIAL_DETAIL_SCALAR_LOOP
  for (; index < size; ++index) {
    output[index] = static_cast<uint8_t>(input[index]);
  }
}

inline void widenLatin1(const uint8_t* input, char16_t* output, size_t size) {
  size_t index = 0;
#if !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__aarch64__) || defined(_M_ARM64))
  auto* raw = reinterpret_cast<uint16_t*>(output);
  // Each 128-bit load feeds two widening stores, retiring 32 bytes per pass.
  for (; index + 32 <= size; index += 32) {
    const uint8x16_t low = vld1q_u8(input + index);
    const uint8x16_t high = vld1q_u8(input + index + 16);
    vst1q_u16(raw + index, vmovl_u8(vget_low_u8(low)));
    vst1q_u16(raw + index + 8, vmovl_u8(vget_high_u8(low)));
    vst1q_u16(raw + index + 16, vmovl_u8(vget_low_u8(high)));
    vst1q_u16(raw + index + 24, vmovl_u8(vget_high_u8(high)));
  }
  for (; index + 8 <= size; index += 8) {
    vst1q_u16(raw + index, vmovl_u8(vld1_u8(input + index)));
  }
#elif !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__SSE2__) || defined(_M_X64))
  const __m128i zero = _mm_setzero_si128();
  // One 128-bit load unpacks into two 128-bit stores, doubling the old width.
  for (; index + 16 <= size; index += 16) {
    const __m128i value = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(input + index));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output + index),
                     _mm_unpacklo_epi8(value, zero));
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output + index + 8),
                     _mm_unpackhi_epi8(value, zero));
  }
  for (; index + 8 <= size; index += 8) {
    const __m128i value =
        _mm_loadl_epi64(reinterpret_cast<const __m128i*>(input + index));
    const __m128i widened = _mm_unpacklo_epi8(value, zero);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(output + index), widened);
  }
#endif
  V8SERIAL_DETAIL_SCALAR_LOOP
  for (; index < size; ++index) {
    output[index] = static_cast<char16_t>(input[index]);
  }
}

#undef V8SERIAL_DETAIL_SCALAR_LOOP

}  // namespace v8serial::detail

#endif
