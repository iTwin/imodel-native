/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

namespace IModelJsNative {

class V8SerializedRowsError : public std::runtime_error {
public:
    V8SerializedRowsError(size_t offset, std::string const& message)
        : std::runtime_error("V8 row decode error at byte " + std::to_string(offset) + ": " + message) {}
};

class V8SerializedRowsReader {
public:
    enum class ValueType {
        Undefined,
        Null,
        Boolean,
        Int32,
        Uint32,
        Double,
        Latin1String,
        Utf8String,
        Utf16String,
    };

    struct Value {
        ValueType m_type = ValueType::Undefined;
        bool m_boolean = false;
        int32_t m_int32 = 0;
        uint32_t m_uint32 = 0;
        double m_double = 0;
        uint8_t const* m_bytes = nullptr;
        uint32_t m_byteCount = 0;
    };

    V8SerializedRowsReader(uint8_t const* data, size_t size)
        : m_begin(normalizeInput(data, size)), m_current(m_begin), m_end(m_begin + size) {}

    template <typename Consumer>
    uint32_t Read(uint32_t minimumColumnCount, Consumer&& consume) {
        if (remaining() < 2 || readByte() != 0xff)
            fail("version header is required");
        if (readVarint() != 15)
            fail("only V8 wire-format version 15 is supported");

        const ArrayHeader root = beginArray();
        const uint32_t rowCount = root.m_length;
        uint32_t expectedColumnCount = 0;
        for (uint32_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
            if (root.m_sparse && rowIndex != readArrayIndex())
                fail("serialized root array must not contain holes or named properties");
            const ArrayHeader row = beginArray();
            const uint32_t columnCount = row.m_length;
            if (columnCount < minimumColumnCount || (0 != expectedColumnCount && columnCount != expectedColumnCount))
                fail("serialized row has an unexpected column count");
            if (0 == expectedColumnCount)
                expectedColumnCount = columnCount;

            for (uint32_t columnIndex = 0; columnIndex < columnCount; ++columnIndex) {
                if (row.m_sparse && columnIndex != readArrayIndex())
                    fail("serialized row must not contain holes or named properties");
                if (!consume(rowIndex, columnIndex, columnCount, readScalar()))
                    return rowIndex;
            }
            if (row.m_sparse)
                endSparseArray(columnCount);
            else
                endDenseArray(columnCount);
        }
        if (root.m_sparse)
            endSparseArray(rowCount);
        else
            endDenseArray(rowCount);
        skipPadding();
        if (m_current != m_end)
            fail("trailing bytes after root value");
        return rowCount;
    }

private:
    struct ArrayHeader {
        uint32_t m_length;
        bool m_sparse;
    };

    inline static uint8_t const s_emptyInput = 0;
    uint8_t const* m_begin;
    uint8_t const* m_current;
    uint8_t const* m_end;

    static uint8_t const* normalizeInput(uint8_t const* data, size_t size) {
        if (nullptr == data && 0 != size)
            throw std::invalid_argument("non-empty input has null data");
        return 0 == size ? &s_emptyInput : data;
    }

    size_t offset() const { return static_cast<size_t>(m_current - m_begin); }
    size_t remaining() const { return static_cast<size_t>(m_end - m_current); }

    [[noreturn]] void fail(std::string const& message) const {
        throw V8SerializedRowsError(offset(), message);
    }

    uint8_t readByte() {
        if (m_current == m_end)
            fail("unexpected end of input");
        return *m_current++;
    }

    void skipPadding() {
        while (m_current != m_end && 0 == *m_current)
            ++m_current;
    }

    uint8_t readTag() {
        skipPadding();
        return readByte();
    }

    uint32_t readVarint() {
        uint32_t value = 0;
        for (uint32_t index = 0; index < 5; ++index) {
            const uint8_t next = readByte();
            if (4 == index && 0 != (next & 0xf0U))
                fail("varint exceeds uint32");
            value |= static_cast<uint32_t>(next & 0x7fU) << (index * 7U);
            if (0 == (next & 0x80U))
                return value;
        }
        fail("unterminated varint");
    }

    ArrayHeader beginArray() {
        const uint8_t tag = readTag();
        if ('A' != tag && 'a' != tag)
            fail("expected an array");
        const uint32_t length = readVarint();
        if (length > remaining())
            fail("array length exceeds remaining input");
        return {length, 'a' == tag};
    }

    void endDenseArray(uint32_t expectedLength) {
        if ('$' != readTag())
            fail("named array properties are unsupported");
        if (0 != readVarint())
            fail("array named-property count must be zero");
        if (expectedLength != readVarint())
            fail("array length mismatch");
    }

    uint32_t readArrayIndex() {
        const Value value = readScalar();
        if (ValueType::Uint32 == value.m_type)
            return value.m_uint32;
        if (ValueType::Int32 == value.m_type && value.m_int32 >= 0)
            return static_cast<uint32_t>(value.m_int32);
        fail("sparse array property key must be a non-negative integer");
    }

    void endSparseArray(uint32_t expectedLength) {
        if ('@' != readTag())
            fail("sparse array named properties are unsupported");
        if (expectedLength != readVarint())
            fail("sparse array property count mismatch");
        if (expectedLength != readVarint())
            fail("array length mismatch");
    }

    Value readString(ValueType type) {
        const uint32_t byteCount = readVarint();
        if (byteCount > remaining())
            fail("string exceeds input");
        if (ValueType::Utf16String == type && 0 != (byteCount & 1U))
            fail("UTF-16 string has an odd byte count");

        Value value;
        value.m_type = type;
        value.m_bytes = m_current;
        value.m_byteCount = byteCount;
        m_current += byteCount;
        return value;
    }

    Value readScalar() {
        Value value;
        const uint8_t tag = readTag();
        switch (tag) {
            case '_':
                return value;
            case '0':
                value.m_type = ValueType::Null;
                return value;
            case 'T':
            case 'F':
                value.m_type = ValueType::Boolean;
                value.m_boolean = 'T' == tag;
                return value;
            case 'I': {
                const uint32_t encoded = readVarint();
                value.m_type = ValueType::Int32;
                value.m_int32 = static_cast<int32_t>((encoded >> 1U) ^ (0U - (encoded & 1U)));
                return value;
            }
            case 'U':
                value.m_type = ValueType::Uint32;
                value.m_uint32 = readVarint();
                return value;
            case 'N':
                if (remaining() < sizeof(double))
                    fail("truncated double");
                value.m_type = ValueType::Double;
                std::memcpy(&value.m_double, m_current, sizeof(double));
                m_current += sizeof(double);
                return value;
            case '"':
                return readString(ValueType::Latin1String);
            case 'S':
                return readString(ValueType::Utf8String);
            case 'c':
                return readString(ValueType::Utf16String);
            default:
                fail("serialized rows may contain only primitive scalar values");
        }
    }
};

} // namespace IModelJsNative
