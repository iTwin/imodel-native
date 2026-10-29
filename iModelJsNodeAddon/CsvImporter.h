/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECDb.h>
#include <optional>
#include <stdexcept>

namespace IModelJsNative {

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

struct CsvImportMapping {
    uint32_t m_columnIndex;
    Utf8String m_propertyName;
};

struct CsvImportOptions {
    std::optional<Utf8String> m_nullValue;
    bool m_hasHeader = false;
};

class CsvImportError : public std::runtime_error {
    DbResult m_sqliteError;
public:
    explicit CsvImportError(Utf8CP message, DbResult sqliteError = BE_SQLITE_OK)
        : std::runtime_error(message), m_sqliteError(sqliteError) {}

    DbResult GetSQLiteError() const { return m_sqliteError; }
};

struct CsvImporter {
    static uint64_t ImportData(ECDbR db, Utf8StringCR className, uint8_t const* bytes, size_t byteCount,
        bvector<CsvImportMapping> const& mapping, CsvImportOptions const& options);
    static uint64_t ImportFile(ECDbR db, Utf8StringCR className, Utf8StringCR filePath,
        bvector<CsvImportMapping> const& mapping, CsvImportOptions const& options);
};

}
