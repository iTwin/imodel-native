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
  for (; index + 8 <= size; index += 8) {
    const uint16x8_t value =
        vld1q_u16(reinterpret_cast<const uint16_t*>(input + index));
    if (vmaxvq_u16(value) > 0xffU) return false;
  }
#elif !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__SSE2__) || defined(_M_X64))
  const __m128i high_byte_mask = _mm_set1_epi16(0xff00);
  const __m128i zero = _mm_setzero_si128();
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

inline void narrowLatin1(const char16_t* input, uint8_t* output, size_t size) {
  size_t index = 0;
#if !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__aarch64__) || defined(_M_ARM64))
  for (; index + 8 <= size; index += 8) {
    const uint16x8_t value =
        vld1q_u16(reinterpret_cast<const uint16_t*>(input + index));
    vst1_u8(output + index, vmovn_u16(value));
  }
#elif !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__SSE2__) || defined(_M_X64))
  const __m128i zero = _mm_setzero_si128();
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
  for (; index + 8 <= size; index += 8) {
    const uint8x8_t value = vld1_u8(input + index);
    vst1q_u16(reinterpret_cast<uint16_t*>(output + index), vmovl_u8(value));
  }
#elif !defined(V8SERIAL_DISABLE_SIMD) && \
    (defined(__SSE2__) || defined(_M_X64))
  const __m128i zero = _mm_setzero_si128();
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
