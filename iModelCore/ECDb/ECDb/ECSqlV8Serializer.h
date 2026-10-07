/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../../libsrc/v8serial/vendor/include/v8serial/writer.hpp"
#include <rapidjson/document.h>
#include <cmath>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

class ECSqlV8Serializer final {
    v8serial::Writer m_writer;

    void WriteValue(rapidjson::Value const& value) {
        switch (value.GetType()) {
            case rapidjson::kNullType: m_writer.null(); break;
            case rapidjson::kFalseType: m_writer.boolean(false); break;
            case rapidjson::kTrueType: m_writer.boolean(true); break;
            case rapidjson::kStringType:
                m_writer.string(std::string_view(value.GetString(), value.GetStringLength()));
                break;
            case rapidjson::kNumberType:
                if (value.IsInt())
                    m_writer.int32(value.GetInt());
                else if (value.IsUint())
                    m_writer.uint32(value.GetUint());
                else if (std::isfinite(value.GetDouble()))
                    m_writer.number(value.GetDouble());
                else
                    m_writer.null();
                break;
            case rapidjson::kArrayType:
                m_writer.beginArray(value.Size());
                for (auto const& element : value.GetArray())
                    WriteValue(element);
                m_writer.endArray();
                break;
            case rapidjson::kObjectType:
                m_writer.beginObject();
                for (auto const& member : value.GetObject()) {
                    m_writer.key(std::string_view(member.name.GetString(), member.name.GetStringLength()));
                    WriteValue(member.value);
                }
                m_writer.endObject();
                break;
        }
    }

public:
    explicit ECSqlV8Serializer(size_t capacity) : m_writer(capacity) { m_writer.beginArray(); }

    void AppendRow(rapidjson::Value const& row) {
        if (!row.IsArray())
            throw std::invalid_argument("ECSQL V8 serialization requires array rows");
        WriteValue(row);
    }

    size_t GetSize(uint32_t rowCount) const {
        size_t countBytes = 1;
        while (rowCount >= 0x80) {
            ++countBytes;
            rowCount >>= 7;
        }
        return m_writer.size() + 2 + countBytes;
    }

    std::vector<uint8_t> Finish() {
        m_writer.endArray();
        return m_writer.take();
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
