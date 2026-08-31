#ifndef V8SERIAL_READER_HPP
#define V8SERIAL_READER_HPP

#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "v8serial/detail/simd.hpp"
#include "v8serial/types.hpp"

namespace v8serial {

/// Discriminator selecting the active payload in DecodedValue.
enum class DecodedType {
  Undefined,
  Null,
  Boolean,
  Int32,
  Uint32,
  Double,
  Date,
  String,
  Array,
  Object,
  ArrayBuffer,
  Uint8Array,
  ArrayBufferView,
};

/// Native value tree produced by Reader.
///
/// Read only the payload member selected by `type`. Strings and binary
/// payloads own their storage; object properties preserve wire order.
struct DecodedValue {
  /// Active value type.
  DecodedType type = DecodedType::Undefined;

  /// Payload for DecodedType::Boolean.
  bool boolean = false;

  /// Payload for DecodedType::Int32.
  int32_t int32 = 0;

  /// Payload for DecodedType::Uint32.
  uint32_t uint32 = 0;

  /// Payload for DecodedType::Double.
  double number = 0;

  /// Milliseconds since the epoch for DecodedType::Date.
  double date_milliseconds = 0;

  /// Payload for DecodedType::String.
  std::u16string string;

  /// Elements for DecodedType::Array.
  std::vector<DecodedValue> array;

  /// Ordered key/value pairs for DecodedType::Object.
  std::vector<std::pair<std::u16string, DecodedValue>> object;

  /// Concrete view type for DecodedType::ArrayBufferView.
  ArrayBufferViewType view_type = ArrayBufferViewType::Uint8Array;

  /// Bytes for ArrayBuffer and array-view decoded types.
  std::vector<uint8_t> binary;
};

/// Indicates malformed, unsupported, or incomplete serialized input.
class DecodeError : public std::runtime_error {
 public:
  /// Creates an error whose message includes the failing byte offset.
  DecodeError(size_t offset, const std::string& message)
      : std::runtime_error("V8 decode error at byte " + std::to_string(offset) +
                           ": " + message),
        offset_(offset) {}

  /// Returns the reader position associated with the failure.
  size_t offset() const noexcept { return offset_; }

 private:
  size_t offset_;
};

/// Bounds-checked reader for the supported subset of V8 wire format version 15.
///
/// Reader borrows an immutable byte span and produces an owning DecodedValue.
/// It has no V8 or N-API dependency. Independent instances may run on separate
/// threads; one instance is not safe for concurrent access.
class Reader {
 public:
  /// Only this exact wire-format version is accepted.
  static constexpr uint32_t kFormatVersion = 15;

  /// Maximum supported nested object/array depth.
  static constexpr size_t kMaxNestingDepth = 512;

  /// Borrows @p size bytes beginning at @p data.
  ///
  /// The input must remain valid until read() returns.
  /// @throws std::invalid_argument if data is null and size is nonzero.
  Reader(const uint8_t* data, size_t size)
      : begin_(normalizeInput(data, size)),
        current_(begin_),
        end_(begin_ + size) {}

  /// Borrows the contents of @p data.
  ///
  /// The vector must remain valid and unmodified until read() returns.
  explicit Reader(const std::vector<uint8_t>& data)
      : Reader(data.data(), data.size()) {}

  /// Decodes exactly one complete version-15 value.
  ///
  /// The result owns all decoded strings, containers, and binary bytes.
  ///
  /// @throws DecodeError for an invalid header, wrong version, malformed or
  /// unsupported value, exceeded limit, truncated input, or trailing bytes.
  DecodedValue read() {
    if (remaining() < 2 || readByte() != 0xff) {
      fail("version header is required");
    }
    const uint32_t version = readVarint();
    if (version != kFormatVersion) {
      fail("only V8 wire-format version 15 is supported");
    }

    DecodedValue value = readValue(0);
    skipPadding();
    if (current_ != end_) fail("trailing bytes after root value");
    return value;
  }

 private:
  inline static const uint8_t empty_input_ = 0;
  const uint8_t* begin_;
  const uint8_t* current_;
  const uint8_t* end_;

  static const uint8_t* normalizeInput(const uint8_t* data, size_t size) {
    if (size != 0 && data == nullptr) {
      throw std::invalid_argument("non-empty input has null data");
    }
    return size == 0 ? &empty_input_ : data;
  }

  size_t offset() const { return static_cast<size_t>(current_ - begin_); }
  size_t remaining() const { return static_cast<size_t>(end_ - current_); }

  [[noreturn]] void fail(const std::string& message) const {
    throw DecodeError(offset(), message);
  }

  uint8_t readByte() {
    if (current_ == end_) fail("unexpected end of input");
    return *current_++;
  }

  void skipPadding() {
    while (current_ != end_ && *current_ == 0) ++current_;
  }

  uint8_t readTag() {
    skipPadding();
    return readByte();
  }

  uint8_t peekTag() {
    skipPadding();
    if (current_ == end_) fail("unexpected end of input");
    return *current_;
  }

  uint32_t readVarint() {
    uint32_t value = 0;
    for (unsigned index = 0; index < 5; ++index) {
      const uint8_t next = readByte();
      if (index == 4 && (next & 0xf0U) != 0) {
        fail("varint exceeds uint32");
      }
      value |= static_cast<uint32_t>(next & 0x7fU) << (index * 7U);
      if ((next & 0x80U) == 0) return value;
    }
    fail("unterminated varint");
  }

  std::vector<uint8_t> readBytes(uint32_t length) {
    if (length > remaining()) fail("byte sequence exceeds input");
    std::vector<uint8_t> output(current_, current_ + length);
    current_ += length;
    return output;
  }

  std::u16string readOneByteString() {
    const uint32_t length = readVarint();
    if (length > remaining()) fail("one-byte string exceeds input");
    std::u16string output(length, u'\0');
    detail::widenLatin1(current_, output.data(), length);
    current_ += length;
    return output;
  }

  std::u16string readTwoByteString() {
    const uint32_t byte_length = readVarint();
    if ((byte_length & 1U) != 0) fail("two-byte string has an odd length");
    if (byte_length > remaining()) fail("two-byte string exceeds input");

    std::u16string output(byte_length / 2U, u'\0');
    if (byte_length != 0) {
      std::memcpy(output.data(), current_, byte_length);
      current_ += byte_length;
    }
    return output;
  }

  static void appendCodePoint(std::u16string& output, uint32_t code_point) {
    if (code_point <= 0xffffU) {
      output.push_back(static_cast<char16_t>(code_point));
      return;
    }
    code_point -= 0x10000U;
    output.push_back(static_cast<char16_t>(0xd800U + (code_point >> 10U)));
    output.push_back(static_cast<char16_t>(0xdc00U + (code_point & 0x3ffU)));
  }

  std::u16string readUtf8String() {
    const uint32_t length = readVarint();
    if (length > remaining()) fail("UTF-8 string exceeds input");
    const uint8_t* limit = current_ + length;
    std::u16string output;

    while (current_ != limit) {
      const uint8_t first = *current_++;
      uint32_t code_point;
      unsigned continuation_count;
      uint32_t minimum;

      if (first < 0x80U) {
        code_point = first;
        continuation_count = 0;
        minimum = 0;
      } else if ((first & 0xe0U) == 0xc0U) {
        code_point = first & 0x1fU;
        continuation_count = 1;
        minimum = 0x80U;
      } else if ((first & 0xf0U) == 0xe0U) {
        code_point = first & 0x0fU;
        continuation_count = 2;
        minimum = 0x800U;
      } else if ((first & 0xf8U) == 0xf0U) {
        code_point = first & 0x07U;
        continuation_count = 3;
        minimum = 0x10000U;
      } else {
        fail("invalid UTF-8 leading byte");
      }

      if (static_cast<size_t>(limit - current_) < continuation_count) {
        fail("truncated UTF-8 sequence");
      }
      for (unsigned index = 0; index < continuation_count; ++index) {
        const uint8_t next = *current_++;
        if ((next & 0xc0U) != 0x80U) fail("invalid UTF-8 continuation byte");
        code_point = (code_point << 6U) | (next & 0x3fU);
      }
      if (code_point < minimum || code_point > 0x10ffffU ||
          (code_point >= 0xd800U && code_point <= 0xdfffU)) {
        fail("invalid UTF-8 code point");
      }
      appendCodePoint(output, code_point);
    }
    return output;
  }

  double readDouble() {
    if (remaining() < sizeof(double)) fail("truncated double");
    double value;
    std::memcpy(&value, current_, sizeof(value));
    current_ += sizeof(value);
    return value;
  }

  ArrayBufferViewType readArrayBufferViewType(uint32_t tag) {
    switch (tag) {
      case 'b':
        return ArrayBufferViewType::Int8Array;
      case 'B':
        return ArrayBufferViewType::Uint8Array;
      case 'C':
        return ArrayBufferViewType::Uint8ClampedArray;
      case 'w':
        return ArrayBufferViewType::Int16Array;
      case 'W':
        return ArrayBufferViewType::Uint16Array;
      case 'd':
        return ArrayBufferViewType::Int32Array;
      case 'D':
        return ArrayBufferViewType::Uint32Array;
      case 'f':
        return ArrayBufferViewType::Float32Array;
      case 'F':
        return ArrayBufferViewType::Float64Array;
      case 'q':
        return ArrayBufferViewType::BigInt64Array;
      case 'Q':
        return ArrayBufferViewType::BigUint64Array;
      case '?':
        return ArrayBufferViewType::DataView;
      default:
        fail("unsupported ArrayBuffer view type");
    }
  }

  ArrayBufferViewType readHostObjectViewType(uint32_t node_type) {
    switch (node_type) {
      case 0:
        return ArrayBufferViewType::Int8Array;
      case 1:
      case 10:
        return ArrayBufferViewType::Uint8Array;
      case 2:
        return ArrayBufferViewType::Uint8ClampedArray;
      case 3:
        return ArrayBufferViewType::Int16Array;
      case 4:
        return ArrayBufferViewType::Uint16Array;
      case 5:
        return ArrayBufferViewType::Int32Array;
      case 6:
        return ArrayBufferViewType::Uint32Array;
      case 7:
        return ArrayBufferViewType::Float32Array;
      case 8:
        return ArrayBufferViewType::Float64Array;
      case 9:
        return ArrayBufferViewType::DataView;
      case 11:
        return ArrayBufferViewType::BigInt64Array;
      case 12:
        return ArrayBufferViewType::BigUint64Array;
      default:
        fail("unsupported Node ArrayBuffer view host object");
    }
  }

  static void setArrayBufferViewType(DecodedValue& output,
                                     ArrayBufferViewType type) {
    output.type = type == ArrayBufferViewType::Uint8Array
                      ? DecodedType::Uint8Array
                      : DecodedType::ArrayBufferView;
    output.view_type = type;
  }

  std::u16string readPropertyKey(size_t depth) {
    DecodedValue key = readValue(depth);
    if (key.type == DecodedType::String) return std::move(key.string);
    if (key.type == DecodedType::Int32) {
      const std::string text = std::to_string(key.int32);
      return std::u16string(text.begin(), text.end());
    }
    if (key.type == DecodedType::Uint32) {
      const std::string text = std::to_string(key.uint32);
      return std::u16string(text.begin(), text.end());
    }
    fail("object property key is not a string or integer");
  }

  DecodedValue readObject(size_t depth) {
    DecodedValue output;
    output.type = DecodedType::Object;
    uint32_t properties = 0;

    while (peekTag() != '{') {
      std::u16string key = readPropertyKey(depth + 1);
      output.object.emplace_back(std::move(key), readValue(depth + 1));
      if (properties == std::numeric_limits<uint32_t>::max()) {
        fail("object property count overflow");
      }
      ++properties;
    }
    readTag();
    if (readVarint() != properties) fail("object property count mismatch");
    return output;
  }

  DecodedValue readDenseArray(size_t depth) {
    const uint32_t length = readVarint();
    if (length > remaining()) fail("array length exceeds remaining input");

    DecodedValue output;
    output.type = DecodedType::Array;
    output.array.reserve(length);
    for (uint32_t index = 0; index < length; ++index) {
      if (peekTag() == '-') fail("array holes are not supported");
      output.array.push_back(readValue(depth + 1));
    }
    if (readTag() != '$') fail("named array properties are not supported");
    if (readVarint() != 0) fail("array named-property count must be zero");
    if (readVarint() != length) fail("array length mismatch");
    return output;
  }

  DecodedValue readSparseArray(size_t depth) {
    const uint32_t length = readVarint();
    if (length > remaining()) fail("array length exceeds remaining input");

    DecodedValue output;
    output.type = DecodedType::Array;
    output.array.reserve(length);
    uint32_t properties = 0;

    while (peekTag() != '@') {
      DecodedValue key = readValue(depth + 1);
      uint32_t index;
      if (key.type == DecodedType::Uint32) {
        index = key.uint32;
      } else if (key.type == DecodedType::Int32 && key.int32 >= 0) {
        index = static_cast<uint32_t>(key.int32);
      } else {
        fail("sparse array property key is not a non-negative integer");
      }
      if (index != properties) {
        fail("array holes or named properties are not supported");
      }

      output.array.push_back(readValue(depth + 1));
      if (properties == std::numeric_limits<uint32_t>::max()) {
        fail("sparse array property count overflow");
      }
      ++properties;
    }

    readTag();
    if (readVarint() != properties) {
      fail("sparse array property count mismatch");
    }
    if (readVarint() != length) fail("array length mismatch");
    if (properties != length) {
      fail("array holes or named properties are not supported");
    }
    return output;
  }

  DecodedValue readArrayBuffer() {
    DecodedValue output;
    output.type = DecodedType::ArrayBuffer;
    output.binary = readBytes(readVarint());

    skipPadding();
    if (current_ != end_ && *current_ == 'V') {
      readTag();
      const ArrayBufferViewType view_type =
          readArrayBufferViewType(readVarint());
      const uint32_t byte_offset = readVarint();
      const uint32_t byte_length = readVarint();
      const uint32_t flags = readVarint();
      if (flags != 0) fail("resizable or length-tracking views are unsupported");
      if (byte_offset > output.binary.size() ||
          byte_length > output.binary.size() - byte_offset) {
        fail("ArrayBuffer view exceeds its backing buffer");
      }
      const size_t element_size =
          detail::arrayBufferViewElementSize(view_type);
      if (byte_offset % element_size != 0 ||
          byte_length % element_size != 0) {
        fail("ArrayBuffer view is not aligned to its element size");
      }
      std::vector<uint8_t> view(output.binary.begin() + byte_offset,
                                output.binary.begin() + byte_offset +
                                    byte_length);
      setArrayBufferViewType(output, view_type);
      output.binary = std::move(view);
    }
    return output;
  }

  DecodedValue readHostObject() {
    const uint32_t node_type = readVarint();
    const uint32_t byte_length = readVarint();
    const ArrayBufferViewType view_type =
        readHostObjectViewType(node_type);
    const size_t element_size = detail::arrayBufferViewElementSize(view_type);
    if (byte_length % element_size != 0) {
      fail("Node ArrayBuffer view byte length is not a multiple of its element size");
    }

    DecodedValue output;
    setArrayBufferViewType(output, view_type);
    output.binary = readBytes(byte_length);
    return output;
  }

  DecodedValue readValue(size_t depth) {
    if (depth > kMaxNestingDepth) fail("maximum nesting depth exceeded");

    DecodedValue output;
    uint8_t tag = readTag();
    while (tag == '?') {
      readVarint();
      tag = readTag();
    }
    switch (tag) {
      case '_':
        return output;
      case '0':
        output.type = DecodedType::Null;
        return output;
      case 'T':
      case 'F':
        output.type = DecodedType::Boolean;
        output.boolean = tag == 'T';
        return output;
      case 'I': {
        const uint32_t encoded = readVarint();
        output.type = DecodedType::Int32;
        output.int32 = static_cast<int32_t>((encoded >> 1U) ^
                                            (0U - (encoded & 1U)));
        return output;
      }
      case 'U':
        output.type = DecodedType::Uint32;
        output.uint32 = readVarint();
        return output;
      case 'N':
        output.type = DecodedType::Double;
        output.number = readDouble();
        return output;
      case 'D':
        output.type = DecodedType::Date;
        output.date_milliseconds = readDouble();
        return output;
      case 'S':
        output.type = DecodedType::String;
        output.string = readUtf8String();
        return output;
      case '"':
        output.type = DecodedType::String;
        output.string = readOneByteString();
        return output;
      case 'c':
        output.type = DecodedType::String;
        output.string = readTwoByteString();
        return output;
      case 'o':
        return readObject(depth);
      case 'A':
        return readDenseArray(depth);
      case 'a':
        return readSparseArray(depth);
      case 'B':
        return readArrayBuffer();
      case '\\':
        return readHostObject();
      default:
        fail("unsupported or invalid serialization tag");
    }
  }
};

}  // namespace v8serial

#endif
