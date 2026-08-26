/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeFile.h>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace IModelJsNative {

class CsvRowsError : public std::runtime_error {
public:
    CsvRowsError(uint64_t rowIndex, uint64_t byteOffset, std::string const& message)
        : std::runtime_error("CSV record " + std::to_string(rowIndex + 1) + " at byte " + std::to_string(byteOffset) + ": " + message) {}
};

class CsvRowsReader {
public:
    using Row = bvector<Utf8String>;

    explicit CsvRowsReader(Utf8StringCR filePath) {
        if (BeFileStatus::Success != m_file.Open(filePath, BeFileAccess::Read))
            throw CsvRowsError(0, 0, "failed to open file");
    }

    template <typename Consumer>
    uint64_t Read(uint32_t minimumColumnCount, bool hasHeader, Consumer&& consume) {
        if (0 == minimumColumnCount)
            throw CsvRowsError(0, 0, "column mapping must not be empty");

        Row fields(1);
        uint32_t columnIndex = 0;
        uint32_t expectedColumnCount = 0;
        uint64_t physicalRowIndex = 0;
        uint64_t rowCount = 0;
        bool inQuotes = false;
        bool afterQuote = false;
        bool fieldStarted = false;
        bool rowStarted = false;
        const auto finishRow = [&]() {
            const uint32_t columnCount = columnIndex + 1;
            if (columnCount < minimumColumnCount || (0 != expectedColumnCount && columnCount != expectedColumnCount))
                fail(physicalRowIndex, "unexpected column count");
            if (0 == expectedColumnCount)
                expectedColumnCount = columnCount;

            if (0 == physicalRowIndex && fields[0].size() >= 3
                && static_cast<uint8_t>(fields[0][0]) == 0xef
                && static_cast<uint8_t>(fields[0][1]) == 0xbb
                && static_cast<uint8_t>(fields[0][2]) == 0xbf)
                fields[0].erase(0, 3);

            if (!(hasHeader && 0 == physicalRowIndex)) {
                if (!consume(physicalRowIndex, fields))
                    return false;
                ++rowCount;
            }

            ++physicalRowIndex;
            for (auto& field : fields)
                field.clear();
            columnIndex = 0;
            fieldStarted = false;
            rowStarted = false;
            return true;
        };

        while (true) {
            if (!ensureData()) {
                if (inQuotes)
                    fail(physicalRowIndex, "unterminated quoted field");
                if (rowStarted && !finishRow())
                    return rowCount;
                return rowCount;
            }

            if (inQuotes) {
                const auto start = m_position;
                while (m_position < m_size && '"' != m_buffer[m_position])
                    ++m_position;

                const auto length = m_position - start;
                if (0 != length) {
                    fields[columnIndex].append(reinterpret_cast<Utf8CP>(m_buffer.data() + start), length);
                    m_byteOffset += length;
                    rowStarted = true;
                }

                if (m_position == m_size)
                    continue;

                consumeByte();
                rowStarted = true;
                inQuotes = false;
                afterQuote = true;
                continue;
            }

            char ch;
            if (afterQuote) {
                ch = consumeByte();
                rowStarted = true;
                if ('"' == ch) {
                    fields[columnIndex].push_back('"');
                    inQuotes = true;
                    afterQuote = false;
                    continue;
                }
                if (',' != ch && '\r' != ch && '\n' != ch)
                    fail(physicalRowIndex, "unexpected character after closing quote");
            } else {
                const auto start = m_position;
                while (m_position < m_size) {
                    const char candidate = static_cast<char>(m_buffer[m_position]);
                    if (',' == candidate || '"' == candidate || '\r' == candidate || '\n' == candidate)
                        break;
                    ++m_position;
                }

                const auto length = m_position - start;
                if (0 != length) {
                    fields[columnIndex].append(reinterpret_cast<Utf8CP>(m_buffer.data() + start), length);
                    m_byteOffset += length;
                    rowStarted = true;
                    fieldStarted = true;
                }

                if (m_position == m_size)
                    continue;

                ch = consumeByte();
                rowStarted = true;
            }

            if (',' == ch) {
                ++columnIndex;
                if (columnIndex == fields.size())
                    fields.emplace_back();
                fieldStarted = false;
                afterQuote = false;
                continue;
            }

            if ('\r' == ch || '\n' == ch) {
                if ('\r' == ch && ensureData() && '\n' == m_buffer[m_position])
                    consumeByte();
                afterQuote = false;
                if (!finishRow())
                    return rowCount;
                continue;
            }

            if ('"' == ch) {
                if (fieldStarted)
                    fail(physicalRowIndex, "quote is only valid at the start of a field");
                fieldStarted = true;
                inQuotes = true;
                afterQuote = false;
                continue;
            }
        }
    }

private:
    BeFile m_file;
    std::array<uint8_t, 64 * 1024> m_buffer{};
    uint32_t m_position = 0;
    uint32_t m_size = 0;
    uint64_t m_byteOffset = 0;

    [[noreturn]] void fail(uint64_t rowIndex, std::string const& message) const {
        throw CsvRowsError(rowIndex, m_byteOffset, message);
    }

    bool ensureData() {
        if (m_position == m_size) {
            uint32_t bytesRead = 0;
            if (BeFileStatus::Success != m_file.Read(m_buffer.data(), &bytesRead, static_cast<uint32_t>(m_buffer.size())))
                throw CsvRowsError(0, m_byteOffset, "failed to read file");
            if (0 == bytesRead)
                return false;
            m_position = 0;
            m_size = bytesRead;
        }

        return true;
    }

    char consumeByte() {
        ++m_byteOffset;
        return static_cast<char>(m_buffer[m_position++]);
    }
};

} // namespace IModelJsNative
