/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CsvImporter.h"
#include "CsvRowsReader.h"
#include <ECDb/ECSqlStatement.h>
#include <ECDb/SchemaManager.h>
#include <v8serial/reader.hpp>
#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace IModelJsNative {

USING_NAMESPACE_BENTLEY_EC

namespace {
struct CsvImportBinding {
    IECSqlBinder* m_binder;
    PrimitiveType m_primitiveType;
};

struct CsvImportPlan {
    bvector<ECPropertyCP> m_properties;
    bvector<Utf8String> m_accessStrings;
    uint32_t m_minimumColumnCount = 0;
};

ECPropertyCP resolveCsvImportProperty(ECClassCR ecClass, Utf8StringCR accessString, Utf8StringR canonicalAccessString) {
    ECClassCP currentClass = &ecClass;
    ECPropertyCP property = nullptr;
    size_t start = 0;
    while (start < accessString.size()) {
        const size_t end = accessString.find('.', start);
        Utf8String segment = accessString.substr(start, end == Utf8String::npos ? Utf8String::npos : end - start);
        if (segment.empty())
            return nullptr;

        property = currentClass->GetPropertyP(segment.c_str(), true);
        if (nullptr == property)
            return nullptr;

        if (!canonicalAccessString.empty())
            canonicalAccessString.append(".");
        canonicalAccessString.append("[").append(property->GetName()).append("]");

        if (end == Utf8String::npos)
            return property;

        const auto structProperty = property->GetAsStructProperty();
        if (nullptr == structProperty)
            return nullptr;

        currentClass = &structProperty->GetType();
        start = end + 1;
    }

    return nullptr;
}

CsvImportPlan createCsvImportPlan(ECClassCR ecClass, bvector<CsvImportMapping> const& mapping) {
    if (mapping.empty())
        throw CsvImportError("mapping must not be empty");

    CsvImportPlan plan;
    bset<Utf8String, CompareIUtf8Ascii> canonicalNames;
    plan.m_properties.reserve(mapping.size());
    plan.m_accessStrings.reserve(mapping.size());
    for (auto const& entry : mapping) {
        Utf8String canonicalName;
        const auto property = resolveCsvImportProperty(ecClass, entry.m_propertyName, canonicalName);
        if (nullptr == property)
            throw CsvImportError("propertyNames contains an invalid or unsupported property path");
        if (!canonicalNames.insert(canonicalName).second)
            throw CsvImportError("propertyNames must not contain duplicates");

        plan.m_properties.push_back(property);
        plan.m_accessStrings.push_back(std::move(canonicalName));
        plan.m_minimumColumnCount = std::max(plan.m_minimumColumnCount, entry.m_columnIndex + 1);
    }
    return plan;
}

CsvImportPlan prepareCsvImport(ECSqlStatement& statement, ECDbR db, Utf8StringCR className,
    bvector<CsvImportMapping> const& mapping, Utf8CP prepareError) {
    const auto ecClass = db.Schemas().FindClass(className.c_str());
    if (nullptr == ecClass)
        throw CsvImportError("className does not identify an ECClass");

    auto plan = createCsvImportPlan(*ecClass, mapping);
    Utf8String ecsql("INSERT INTO ");
    ecsql.append(ecClass->GetECSqlName()).append(" (");
    Utf8String valuesClause(") VALUES (");
    for (size_t i = 0; i < plan.m_accessStrings.size(); ++i) {
        if (i > 0) {
            ecsql.append(",");
            valuesClause.append(",");
        }
        ecsql.append(plan.m_accessStrings[i]);
        valuesClause.append("?");
    }
    ecsql.append(valuesClause).append(")");

    const auto prepareStatus = statement.Prepare(db, ecsql.c_str());
    if (!prepareStatus.IsSuccess())
        throw CsvImportError(prepareError, prepareStatus.IsSQLiteError() ? prepareStatus.GetSQLiteError() : BE_SQLITE_ERROR);

    return plan;
}

bvector<CsvImportBinding> createCsvImportBindings(CsvImportPlan const& plan, ECSqlStatement& statement) {
    bvector<CsvImportBinding> bindings;
    bindings.reserve(plan.m_properties.size());
    for (size_t i = 0; i < plan.m_properties.size(); ++i) {
        const auto primitiveProperty = plan.m_properties[i]->GetAsPrimitiveProperty();
        if (nullptr == primitiveProperty)
            throw CsvImportError("CSV import supports only boolean, double, integer, and string properties");

        const auto type = primitiveProperty->GetType();
        switch (type) {
            case PRIMITIVETYPE_Boolean:
            case PRIMITIVETYPE_Double:
            case PRIMITIVETYPE_Integer:
            case PRIMITIVETYPE_String:
                break;
            default:
                throw CsvImportError("CSV import supports only boolean, double, integer, and string properties");
        }
        bindings.push_back({&statement.GetBinder(static_cast<int>(i + 1)), type});
    }
    return bindings;
}

bool decodeSerializedCSVValue(Utf8StringR decoded, v8serial::DecodedValue const& value) {
    if (v8serial::DecodedType::String != value.type || value.string.find(u'\0') != std::u16string::npos)
        return false;
    return SUCCESS == BeStringUtilities::Utf16ToUtf8(decoded, reinterpret_cast<Utf16CP>(value.string.data()), value.string.size());
}

ECSqlStatus bindCsvImportValue(CsvImportBinding const& binding, Utf8StringCR value, Utf8CP nullValue) {
    if (value.find('\0') != Utf8String::npos)
        return ECSqlStatus::Error;
    if (nullptr != nullValue && value.Equals(nullValue))
        return binding.m_binder->BindNull();

    const auto begin = value.data();
    const auto end = begin + value.size();
    switch (binding.m_primitiveType) {
        case PRIMITIVETYPE_Boolean:
            if (value.Equals("true") || value.Equals("1"))
                return binding.m_binder->BindBoolean(true);
            if (value.Equals("false") || value.Equals("0"))
                return binding.m_binder->BindBoolean(false);
            return ECSqlStatus::Error;
        case PRIMITIVETYPE_Double: {
            if (value.empty())
                return ECSqlStatus::Error;
            char* parsedEnd = nullptr;
            errno = 0;
            const double parsed = std::strtod(begin, &parsedEnd);
            return 0 == errno && end == parsedEnd && std::isfinite(parsed) ? binding.m_binder->BindDouble(parsed) : ECSqlStatus::Error;
        }
        case PRIMITIVETYPE_Integer: {
            int32_t parsed = 0;
            const auto result = std::from_chars(begin, end, parsed);
            return std::errc() == result.ec && end == result.ptr ? binding.m_binder->BindInt(parsed) : ECSqlStatus::Error;
        }
        case PRIMITIVETYPE_String:
            return binding.m_binder->BindText(value.c_str(), IECSqlBinder::MakeCopy::No, static_cast<int>(value.size()));
        default:
            return ECSqlStatus::Error;
    }
}
}

uint64_t CsvImporter::ImportData(ECDbR db, Utf8StringCR className, uint8_t const* bytes, size_t byteCount,
    bvector<CsvImportMapping> const& mapping, CsvImportOptions const& options) {
    ECSqlStatement statement;
    const auto plan = prepareCsvImport(statement, db, className, mapping, "Failed to prepare CSV data import ECSQL");
    const auto bindings = createCsvImportBindings(plan, statement);
    const Utf8CP nullValue = options.m_nullValue ? options.m_nullValue->c_str() : nullptr;

    v8serial::DecodedValue serializedRowsValue;
    try {
        serializedRowsValue = v8serial::Reader(bytes, byteCount).read();
    } catch (v8serial::DecodeError const& error) {
        throw CsvImportError(error.what());
    }

    if (v8serial::DecodedType::Array != serializedRowsValue.type)
        throw CsvImportError("serialized root value must be an array");
    if (serializedRowsValue.array.size() > std::numeric_limits<uint32_t>::max())
        throw CsvImportError("serialized row count exceeds uint32");

    uint32_t expectedColumnCount = 0;
    for (auto const& row : serializedRowsValue.array) {
        if (v8serial::DecodedType::Array != row.type)
            throw CsvImportError("serialized root array must contain only row arrays");
        if (row.array.size() > std::numeric_limits<uint32_t>::max())
            throw CsvImportError("serialized column count exceeds uint32");

        const uint32_t columnCount = static_cast<uint32_t>(row.array.size());
        if (columnCount < plan.m_minimumColumnCount || (0 != expectedColumnCount && columnCount != expectedColumnCount))
            throw CsvImportError("serialized row has an unexpected column count");
        if (0 == expectedColumnCount)
            expectedColumnCount = columnCount;
    }

    bvector<Utf8String> stringBuffers(mapping.size());
    bmap<uint32_t, uint32_t> propertyIndexesByColumn;
    for (uint32_t propertyIndex = 0; propertyIndex < mapping.size(); ++propertyIndex)
        propertyIndexesByColumn[mapping[propertyIndex].m_columnIndex] = propertyIndex;

    Savepoint savepoint(db, "importCSVData");
    if (!savepoint.IsActive())
        throw CsvImportError("Failed to start CSV data import savepoint", BE_SQLITE_ERROR);

    try {
        for (uint32_t rowIndex = 0; rowIndex < serializedRowsValue.array.size(); ++rowIndex) {
            auto const& row = serializedRowsValue.array[rowIndex];
            statement.Reset();
            for (auto const& entry : propertyIndexesByColumn) {
                const auto columnIndex = entry.first;
                const auto propertyIndex = entry.second;
                auto& decoded = stringBuffers[propertyIndex];
                if (!decodeSerializedCSVValue(decoded, row.array[columnIndex]) || !bindCsvImportValue(bindings[propertyIndex], decoded, nullValue).IsSuccess())
                    throw CsvImportError(Utf8PrintfString("Failed to bind CSV data row %" PRIu32 " column %" PRIu32, rowIndex + 1, columnIndex).c_str());
            }

            const auto stepStatus = statement.Step();
            if (BE_SQLITE_DONE != stepStatus)
                throw CsvImportError(Utf8PrintfString("Failed to insert CSV data row %" PRIu32, rowIndex + 1).c_str(), stepStatus);
        }
    } catch (...) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            throw CsvImportError("Failed to roll back CSV data import", rollbackStatus);
        throw;
    }

    const auto commitStatus = savepoint.Commit();
    if (BE_SQLITE_OK != commitStatus) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            throw CsvImportError("Failed to commit or roll back CSV data import", rollbackStatus);
        throw CsvImportError("Failed to commit CSV data import", commitStatus);
    }

    return serializedRowsValue.array.size();
}

uint64_t CsvImporter::ImportFile(ECDbR db, Utf8StringCR className, Utf8StringCR filePath,
    bvector<CsvImportMapping> const& mapping, CsvImportOptions const& options) {
    ECSqlStatement statement;
    const auto plan = prepareCsvImport(statement, db, className, mapping, "Failed to prepare CSV file import ECSQL");
    const auto bindings = createCsvImportBindings(plan, statement);
    const Utf8CP nullValue = options.m_nullValue ? options.m_nullValue->c_str() : nullptr;

    Savepoint savepoint(db, "importCSVFile");
    if (!savepoint.IsActive())
        throw CsvImportError("Failed to start CSV file import savepoint", BE_SQLITE_ERROR);

    uint64_t rowCount = 0;
    try {
        CsvRowsReader reader(filePath);
        rowCount = reader.Read(plan.m_minimumColumnCount, options.m_hasHeader, [&](uint64_t recordIndex, CsvRowsReader::Row const& fields) {
            statement.Reset();
            for (uint32_t propertyIndex = 0; propertyIndex < bindings.size(); ++propertyIndex) {
                const uint32_t csvColumnIndex = mapping[propertyIndex].m_columnIndex;
                if (!bindCsvImportValue(bindings[propertyIndex], fields[csvColumnIndex], nullValue).IsSuccess())
                    throw CsvImportError(Utf8PrintfString("Failed to bind CSV record %" PRIu64 " column %" PRIu32, recordIndex + 1, csvColumnIndex).c_str());
            }

            const auto stepStatus = statement.Step();
            if (BE_SQLITE_DONE != stepStatus)
                throw CsvImportError(Utf8PrintfString("Failed to insert CSV record %" PRIu64, recordIndex + 1).c_str(), stepStatus);
            return true;
        });
    } catch (CsvRowsError const& error) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            throw CsvImportError("Failed to roll back CSV file import", rollbackStatus);
        throw CsvImportError(error.what());
    } catch (...) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            throw CsvImportError("Failed to roll back CSV file import", rollbackStatus);
        throw;
    }

    const auto commitStatus = savepoint.Commit();
    if (BE_SQLITE_OK != commitStatus) {
        const auto rollbackStatus = savepoint.Cancel();
        if (BE_SQLITE_OK != rollbackStatus)
            throw CsvImportError("Failed to commit or roll back CSV file import", rollbackStatus);
        throw CsvImportError("Failed to commit CSV file import", commitStatus);
    }

    return rowCount;
}

}
