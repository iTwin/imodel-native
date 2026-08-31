#ifndef V8SERIAL_WRITER_HPP
#define V8SERIAL_WRITER_HPP

#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "v8serial/detail/simd.hpp"
#include "v8serial/types.hpp"

namespace v8serial {

/// Writes one value using the supported subset of V8 wire format version 15.
///
/// The writer owns its byte buffer, has no V8 or N-API dependency, and may be
/// used on any thread. A single instance is not safe for concurrent access.
class Writer {
 public:
  /// Wire-format version emitted in every message header.
  static constexpr uint32_t kFormatVersion = 15;

  /// Creates a writer and emits the version header.
  ///
  /// @param initial_capacity Initial output-vector capacity. This is only a
  /// performance hint and does not limit the final message size.
  explicit Writer(size_t initial_capacity = 256) {
    bytes_.reserve(initial_capacity < 2 ? 2 : initial_capacity);
    stack_.reserve(8);
    writeHeader();
  }

  /// Clears all writer state so the instance can be reused for a new root
  /// value without releasing the underlying buffer's allocated capacity.
  ///
  /// Unlike `take()`, this never throws: it may be called at any time,
  /// including mid-container or before a root value has been written. This
  /// is intended for same-thread, serial reuse (e.g. a producer thread that
  /// encodes many messages, copies each one out via `data()`/`size()`, then
  /// calls `reset()` before encoding the next message) to avoid repeated
  /// heap allocation/deallocation across messages.
  void reset() {
    bytes_.clear();
    stack_.clear();
    has_root_ = false;
    writeHeader();
  }

  /// Pointer to the current message bytes (header plus any values written so
  /// far). Valid until the next mutating call on this writer.
  const uint8_t* data() const { return bytes_.data(); }

  /// Number of bytes currently written, including the version header.
  size_t size() const { return bytes_.size(); }

  /// Writes JavaScript `undefined`.
  void undefined() { scalar('_'); }

  /// Writes JavaScript `null`.
  void null() { scalar('0'); }

  /// Writes a Boolean value.
  void boolean(bool value) { scalar(value ? 'T' : 'F'); }

  /// Writes a signed 32-bit integer using ZigZag varint encoding.
  void int32(int32_t value) {
    beginValue();
    byte('I');
    varint((static_cast<uint32_t>(value) << 1) ^
           static_cast<uint32_t>(value >> 31));
  }

  /// Writes an unsigned 32-bit integer using V8's unsigned varint tag.
  void uint32(uint32_t value) {
    beginValue();
    byte('U');
    varint(value);
  }

  /// Writes an IEEE-754 double in the version-15 native representation.
  ///
  /// Use this method for non-integral numbers, NaN, infinities, and negative
  /// zero. V8's version-15 representation uses host byte order.
  void number(double value) {
    beginValue();
    byte('N');
    rawDouble(value);
  }

  /// Writes a JavaScript Date from its milliseconds-since-epoch value.
  void date(double milliseconds) {
    beginValue();
    byte('D');
    rawDouble(milliseconds);
  }

  /// Writes a UTF-8 string.
  ///
  /// The caller must provide valid UTF-8. The bytes are copied immediately.
  void string(std::string_view utf8) {
    beginValue();
    rawUtf8(utf8);
  }

  /// Writes a UTF-16 string, selecting Latin-1 when every code unit fits.
  void string(std::u16string_view value) {
    beginValue();
    rawUtf16(value);
  }

  /// Writes an object property key.
  ///
  /// This is valid only after beginObject() or after completing the preceding
  /// property value. Exactly one value must follow each key.
  ///
  /// @throws std::logic_error if no object key is currently expected.
  void key(std::u16string_view value) {
    if (stack_.empty() || stack_.back().kind != Kind::Object ||
        !stack_.back().expecting_key) {
      throw std::logic_error("key() is only valid where an object key is expected");
    }
    rawUtf16(value);
    stack_.back().expecting_key = false;
  }

  /// Starts a plain string-keyed object.
  void beginObject() {
    beginValue();
    byte('o');
    stack_.push_back({Kind::Object, 0, 0, true});
  }

  /// Ends the current object and writes its property count.
  ///
  /// @throws std::logic_error for a mismatched container or key without value.
  void endObject() {
    requireContainer(Kind::Object, "endObject()");
    const Frame frame = stack_.back();
    if (!frame.expecting_key) {
      throw std::logic_error("object key has no value");
    }
    stack_.pop_back();
    byte('{');
    varint(frame.count);
  }

  /// Starts a dense array with a known number of elements.
  ///
  /// Exactly @p length values must be written before endArray().
  void beginArray(uint32_t length) {
    beginValue();
    byte('A');
    varint(length);
    stack_.push_back({Kind::Array, 0, length, false});
  }

  /// Ends the current dense array.
  ///
  /// @throws std::logic_error for a mismatched container or element-count
  /// mismatch.
  void endArray() {
    requireContainer(Kind::Array, "endArray()");
    const Frame frame = stack_.back();
    if (frame.count != frame.expected) {
      throw std::logic_error("array element count does not match declared length");
    }
    stack_.pop_back();
    byte('$');
    varint(0);  // No named properties.
    varint(frame.expected);
  }

  /// Writes an ordinary ArrayBuffer by copying @p size bytes from @p data.
  ///
  /// @throws std::invalid_argument if data is null and size is nonzero.
  /// @throws std::length_error if size exceeds UINT32_MAX.
  void arrayBuffer(const uint8_t* data, size_t size) {
    beginValue();
    rawArrayBuffer(data, size);
  }

  /// Writes a Uint8Array with a new backing buffer and byte offset zero.
  ///
  /// The input bytes are copied immediately. This emits V8's native
  /// ArrayBuffer-plus-view form rather than Node's host-object form.
  ///
  /// @throws std::invalid_argument if data is null and size is nonzero.
  /// @throws std::length_error if size exceeds UINT32_MAX.
  void uint8Array(const uint8_t* data, size_t size) {
    arrayBufferView(ArrayBufferViewType::Uint8Array, data, size);
  }

  /// Writes an ArrayBuffer view with a new backing buffer and offset zero.
  ///
  /// @throws std::invalid_argument if @p size is not a multiple of the view's
  /// element size, or if data is null and size is nonzero.
  /// @throws std::length_error if size exceeds UINT32_MAX.
  void arrayBufferView(ArrayBufferViewType type, const uint8_t* data,
                       size_t size) {
    const size_t element_size = detail::arrayBufferViewElementSize(type);
    if (element_size == 0) {
      throw std::invalid_argument("unknown ArrayBuffer view type");
    }
    if (size % element_size != 0) {
      throw std::invalid_argument(
          "view byte length is not a multiple of its element size");
    }
    beginValue();
    rawArrayBuffer(data, size);
    byte('V');
    byte(arrayBufferViewTag(type));
    varint(0);  // byteOffset
    checkedVarint(size);
    varint(0);  // version >= 14 view flags
  }

  /// Moves out the completed version-15 message.
  ///
  /// @throws std::logic_error if no root exists or a container remains open.
  /// Call `reset()` before writing another root value with this instance.
  std::vector<uint8_t> take() {
    if (!stack_.empty()) {
      throw std::logic_error("cannot take bytes with an open container");
    }
    if (!has_root_) {
      throw std::logic_error("cannot take bytes before writing a value");
    }
    return std::move(bytes_);
  }

 private:
  enum class Kind { Object, Array };

  struct Frame {
    Kind kind;
    uint32_t count;
    uint32_t expected;
    bool expecting_key;
  };

  std::vector<uint8_t> bytes_;
  std::vector<Frame> stack_;
  bool has_root_ = false;

  void writeHeader() {
    bytes_.push_back(0xff);
    bytes_.push_back(static_cast<uint8_t>(kFormatVersion));
  }

  void byte(uint8_t value) { bytes_.push_back(value); }

  void rawDouble(double value) {
    uint8_t bytes[sizeof(value)];
    std::memcpy(bytes, &value, sizeof(value));
    bytes_.insert(bytes_.end(), bytes, bytes + sizeof(value));
  }

  static uint8_t arrayBufferViewTag(ArrayBufferViewType type) {
    switch (type) {
      case ArrayBufferViewType::Int8Array:
        return 'b';
      case ArrayBufferViewType::Uint8Array:
        return 'B';
      case ArrayBufferViewType::Uint8ClampedArray:
        return 'C';
      case ArrayBufferViewType::Int16Array:
        return 'w';
      case ArrayBufferViewType::Uint16Array:
        return 'W';
      case ArrayBufferViewType::Int32Array:
        return 'd';
      case ArrayBufferViewType::Uint32Array:
        return 'D';
      case ArrayBufferViewType::Float32Array:
        return 'f';
      case ArrayBufferViewType::Float64Array:
        return 'F';
      case ArrayBufferViewType::BigInt64Array:
        return 'q';
      case ArrayBufferViewType::BigUint64Array:
        return 'Q';
      case ArrayBufferViewType::DataView:
        return '?';
    }
    throw std::invalid_argument("unknown ArrayBuffer view type");
  }

  void varint(uint32_t value) {
    do {
      uint8_t next = static_cast<uint8_t>(value & 0x7f);
      value >>= 7;
      if (value != 0) next |= 0x80;
      byte(next);
    } while (value != 0);
  }

  static size_t varintSize(uint32_t value) {
    size_t size = 1;
    while (value >= 0x80) {
      value >>= 7;
      ++size;
    }
    return size;
  }

  void checkedVarint(size_t value) {
    if (value > std::numeric_limits<uint32_t>::max()) {
      throw std::length_error("value exceeds V8 wire-format uint32 limit");
    }
    varint(static_cast<uint32_t>(value));
  }

  void beginValue() {
    if (stack_.empty()) {
      if (has_root_) throw std::logic_error("writer accepts exactly one root value");
      has_root_ = true;
      return;
    }

    Frame& parent = stack_.back();
    if (parent.kind == Kind::Object) {
      if (parent.expecting_key) {
        throw std::logic_error("object value must be preceded by key()");
      }
      parent.expecting_key = true;
      ++parent.count;
    } else {
      if (parent.count == parent.expected) {
        throw std::logic_error("array has more values than its declared length");
      }
      ++parent.count;
    }
  }

  void scalar(uint8_t tag) {
    beginValue();
    byte(tag);
  }

  void rawUtf8(std::string_view value) {
    byte('S');
    checkedVarint(value.size());
    bytes_.insert(bytes_.end(), value.begin(), value.end());
  }

  void rawUtf16(std::u16string_view value) {
    if (detail::allLatin1(value.data(), value.size())) {
      byte('"');
      checkedVarint(value.size());
      const size_t offset = bytes_.size();
      bytes_.resize(offset + value.size());
      detail::narrowLatin1(value.data(), bytes_.data() + offset, value.size());
      return;
    }

    if (value.size() > std::numeric_limits<uint32_t>::max() / 2U) {
      throw std::length_error("UTF-16 string exceeds V8 wire-format limit");
    }
    const uint32_t byte_length = static_cast<uint32_t>(value.size() * 2U);
    if (((bytes_.size() + 1U + varintSize(byte_length)) & 1U) != 0) byte(0);
    byte('c');
    varint(byte_length);
    static_assert(sizeof(char16_t) == 2);
    const auto* data = reinterpret_cast<const uint8_t*>(value.data());
    bytes_.insert(bytes_.end(), data, data + byte_length);
  }

  void rawArrayBuffer(const uint8_t* data, size_t size) {
    if (size != 0 && data == nullptr) {
      throw std::invalid_argument("non-empty binary value has null data");
    }
    byte('B');
    checkedVarint(size);
    if (size != 0) bytes_.insert(bytes_.end(), data, data + size);
  }

  void requireContainer(Kind expected, const char* operation) const {
    if (stack_.empty() || stack_.back().kind != expected) {
      throw std::logic_error(std::string(operation) +
                             " does not match the open container");
    }
  }
};

}  // namespace v8serial

#endif
