#ifndef V8SERIAL_TYPES_HPP
#define V8SERIAL_TYPES_HPP

#include <cstddef>

namespace v8serial {

/// Selects a JavaScript view over serialized ArrayBuffer bytes.
enum class ArrayBufferViewType {
  Int8Array,
  Uint8Array,
  Uint8ClampedArray,
  Int16Array,
  Uint16Array,
  Int32Array,
  Uint32Array,
  Float32Array,
  Float64Array,
  BigInt64Array,
  BigUint64Array,
  DataView,
};

namespace detail {

constexpr size_t arrayBufferViewElementSize(ArrayBufferViewType type) {
  switch (type) {
    case ArrayBufferViewType::Int8Array:
    case ArrayBufferViewType::Uint8Array:
    case ArrayBufferViewType::Uint8ClampedArray:
    case ArrayBufferViewType::DataView:
      return 1;
    case ArrayBufferViewType::Int16Array:
    case ArrayBufferViewType::Uint16Array:
      return 2;
    case ArrayBufferViewType::Int32Array:
    case ArrayBufferViewType::Uint32Array:
    case ArrayBufferViewType::Float32Array:
      return 4;
    case ArrayBufferViewType::Float64Array:
    case ArrayBufferViewType::BigInt64Array:
    case ArrayBufferViewType::BigUint64Array:
      return 8;
  }
  return 0;
}

}  // namespace detail

}  // namespace v8serial

#endif
