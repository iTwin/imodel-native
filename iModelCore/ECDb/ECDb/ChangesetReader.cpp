
/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"
#include <BeRapidJson/BeJsValue.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::ChangesetReader(): m_innerReader(std::make_unique<PreparedChangesetReader>()) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::~ChangesetReader() {
    m_innerReader.reset();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangesetFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter) {
    return m_innerReader->OpenChangesetFile(ecdb, changesetFile, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    return m_innerReader->OpenChangeGroup(ecdb, changesetFiles, invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    return m_innerReader->OpenInMemoryChangeset(ecdb, std::move(changeSet), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Close() {
    return m_innerReader->Close();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::Step() {
    return m_innerReader->Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetTableName(Utf8StringR tableName) const {
    return m_innerReader->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetOpcode(DbOpcode& opcode) const {
    return m_innerReader->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetReader::GetValue(Stage stage, int columnIndex) const {
    return m_innerReader->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ChangesetReader::GetECDb() const {
    return m_innerReader->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetReader::GetColumnCount(Stage stage) const {
    return m_innerReader->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    return m_innerReader->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsECTable(bool& isECTable) const {
    return m_innerReader->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> const* ChangesetReader::GetChangeFetchedPropertyNames() const {
    return m_innerReader->GetChangeFetchedPropertyNames();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsIndirectChange(bool& isIndirect) const {
    return m_innerReader->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    return m_innerReader->SetTableFilters(tableFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    return m_innerReader->SetOpcodeFilters(opcodeFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    return m_innerReader->SetECClassNameFilters(ecclassNameFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearTableFilters() {
    return m_innerReader->ClearTableFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearOpcodeFilters() {
    return m_innerReader->ClearOpcodeFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearECClassNameFilters() {
    return m_innerReader->ClearECClassNameFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::EnableStrictMode() {
    return m_innerReader->EnableStrictMode();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::DisableStrictMode() {
    return m_innerReader->DisableStrictMode();
}

namespace {

// This function is meant to mirror the behavior of SQLite's internal sqlite3MemCompare function,
// which is used (via the IS operator) to compare expected to current column values when applying
// a changeset.
bool DbValuesAreEqual(DbValue const& a, DbValue const& b) {
    if (!a.IsValid() && !b.IsValid()) return true;
    if (!a.IsValid() || !b.IsValid()) return false;
    if (a.IsNull() && b.IsNull()) return true;
    if (a.IsNull() || b.IsNull()) return false;

    auto typeA = a.GetValueType();
    auto typeB = b.GetValueType();
    bool aIsNumeric = (typeA == DbValueType::IntegerVal || typeA == DbValueType::FloatVal);
    bool bIsNumeric = (typeB == DbValueType::IntegerVal || typeB == DbValueType::FloatVal);

    if (aIsNumeric && bIsNumeric) {
        // Cross-type numeric comparison matching SQLite IS semantics.
        // Both-integer and both-real fast paths avoid double precision issues.
        if (typeA == DbValueType::IntegerVal && typeB == DbValueType::IntegerVal)
            return a.GetValueInt64() == b.GetValueInt64();
        if (typeA == DbValueType::FloatVal && typeB == DbValueType::FloatVal)
            return a.GetValueDouble() == b.GetValueDouble();
        // Mixed int/float: use sqlite3RealSameAsInt equivalent
        int64_t i = (typeA == DbValueType::IntegerVal) ? a.GetValueInt64() : b.GetValueInt64();
        double  r = (typeA == DbValueType::FloatVal)   ? a.GetValueDouble() : b.GetValueDouble();
        return (double)i == r && (int64_t)r == i; // guards against precision loss
    }

    if (typeA != typeB) return false;  // e.g. text vs blob

    switch (typeA) {
        case DbValueType::TextVal:
            return strcmp(a.GetValueText(), b.GetValueText()) == 0;
        case DbValueType::BlobVal: {
            int na = a.GetValueBytes(), nb = b.GetValueBytes();
            return na == nb && memcmp(a.GetValueBlob(), b.GetValueBlob(), na) == 0;
        }
        default: return false;
    }
}

}

/*static*/ BentleyStatus ChangesetReader::GetConflictReportJson(
        ECDbCR ecdb,
        ChangesetReader::PropertyFilter propertyFilter,
        ChangeSet::ConflictCause cause,
        Changes::Change const& conflict,
        BeJsValue& outJsonReport)
    {
    outJsonReport.toObject();

    DbTable const* dbTable = ecdb.Schemas().Main().GetDbSchema().FindTable(conflict.GetTableName());
    if (!dbTable) return BentleyStatus::ERROR;

    bvector<Utf8String> columns;
    if (!ecdb.GetColumns(columns, conflict.GetTableName().c_str()))
        return BentleyStatus::ERROR;

    DbOpcode opcode = conflict.GetOpcode();

    bool originalValueAvailable = opcode == DbOpcode::Update || opcode == DbOpcode::Delete;
    bool ourValueAvailable = opcode == DbOpcode::Update || opcode == DbOpcode::Insert;
    bool theirValueAvailable = cause == ChangeSet::ConflictCause::Data || cause == ChangeSet::ConflictCause::Conflict;

    std::unordered_map<Utf8String, DbValue> originalDbValues;
    std::unordered_map<Utf8String, DbValue> theirDbValues;
    std::unordered_map<Utf8String, DbValue> ourDbValues;
    std::unordered_set<Utf8String> dataConflictColumns;
    for(int i = 0; i < static_cast<int>(columns.size()); ++i)
        {
        DbValue originalValue = originalValueAvailable ? conflict.GetOldValue(i) : DbValue(nullptr);
        DbValue ourValue = ourValueAvailable ? conflict.GetNewValue(i) : DbValue(nullptr);

        // GetConflictValue will get any column from the current database row.
        // But we only need it if this column is in the conflicting changeset.
        DbValue theirValue = theirValueAvailable && (originalValue.IsValid() || ourValue.IsValid())
            ? conflict.GetConflictValue(i)
            : DbValue(nullptr);

        // SQLite changesets store PK column values only in the Old slot for UPDATE and DELETE.
        // For INSERT, it will only be in the New slot.
        if (conflict.IsPrimaryKeyColumn(i))
            {
            DbValue pkValue = originalValueAvailable ? originalValue : ourValue;
            if (ourValueAvailable && !ourValue.IsValid())
                ourValue = pkValue;
            if (theirValueAvailable && !theirValue.IsValid())
                theirValue = pkValue;
            }

        if (originalValue.IsValid())
            originalDbValues.emplace(columns[i], originalValue);
        if (ourValue.IsValid())
            ourDbValues.emplace(columns[i], ourValue);
        if (theirValue.IsValid())
            theirDbValues.emplace(columns[i], theirValue);

        // Determine which columns represent genuine conflicts.
        // A column is in conflict if "their" value is different from the "original" value.
        // Note that "our" value may be the same as "their" value, but if it is different from "original", it is still a conflict.
        // Also flag conflict columns when they deleted (no THEIR) or inserted (no ORIGINAL).
        bool hasOriginalOrTheir = originalValue.IsValid() || theirValue.IsValid();
        if (hasOriginalOrTheir && (!originalValue.IsValid() || !theirValue.IsValid() || !DbValuesAreEqual(originalValue, theirValue)))
            {
            dataConflictColumns.emplace(columns[i]);
            }
        }

    // Resolve ECClassId for property-to-column mapping
    ECClassId classId;
    bool isClassIdFromChangeset = false;
    auto const& valuesForClassId = !originalDbValues.empty() ? originalDbValues : ourDbValues;
    if (ChangesetValueFactory::ResolveClassId(ecdb, *dbTable, valuesForClassId, classId, isClassIdFromChangeset) != SUCCESS)
        return BentleyStatus::ERROR;

    const ECClass* cls = ecdb.Schemas().Main().GetClass(classId);
    if (!cls) return BentleyStatus::ERROR;
    const ClassMap* classMap = ecdb.Schemas().Main().GetClassMap(*cls);

    ECSqlRowAdaptor adaptor(ecdb);
    adaptor.GetOptions().SetIncludeNulls(true);

    // Helper: build a property-keyed JSON object from a DB column value map using IECSqlValues
    auto buildValuesJson = [&](BeJsValue outJson, std::unordered_map<Utf8String, DbValue> const& dbValues) -> BentleyStatus
        {
        std::vector<std::unique_ptr<IECSqlValue>> fields;
        if (ChangesetValueFactory::Create(ecdb, *dbTable, dbValues, classId, isClassIdFromChangeset, fields, ChangesetReader::PropertyFilter::All, nullptr) != SUCCESS)
            return BentleyStatus::ERROR;
        outJson.toObject();
        for (auto const& field : fields)
            {
            if (!field) continue;
            Utf8String accessStr = field->GetColumnInfo().GetPropertyPath().ToString();
            if (accessStr.empty()) continue;
            adaptor.RenderValue(outJson[accessStr.c_str()], *field);
            }
        return BentleyStatus::SUCCESS;
        };

    if (originalValueAvailable && !originalDbValues.empty())
        {
        if (buildValuesJson(outJsonReport["original"], originalDbValues) != SUCCESS)
            return BentleyStatus::ERROR;
        }
    if (theirValueAvailable && !theirDbValues.empty())
        {
        if (buildValuesJson(outJsonReport["theirs"], theirDbValues) != SUCCESS)
            return BentleyStatus::ERROR;
        }
    if (ourValueAvailable && !ourDbValues.empty())
        {
        if (buildValuesJson(outJsonReport["ours"], ourDbValues) != SUCCESS)
            return BentleyStatus::ERROR;
        }

    // Build dataConflictProperties: property access strings of properties whose DB columns changed
    {
    BeJsValue conflictPropsJson = outJsonReport["dataConflictProperties"];
    conflictPropsJson.toArray();
    if (classMap != nullptr)
        {
        for (PropertyMap const* propMap : classMap->GetPropertyMaps())
            {
            GetColumnsPropertyMapVisitor colVisitor(PropertyMap::Type::Data);
            propMap->AcceptVisitor(colVisitor);
            for (DbColumn const* col : colVisitor.GetColumns())
                {
                if (dataConflictColumns.count(col->GetName()))
                    {
                    conflictPropsJson.appendValue() = propMap->GetAccessString().c_str();
                    break;
                    }
                }
            }
        }
    }

    // Build uniqueConstraintViolations: for each violated UNIQUE index, record the
    // violating property names and the conflicting existing row. For constraint-cause
    // conflicts sqlite3changeset_conflict() is unavailable so we probe the indexes directly.
    {
    BeJsValue violationsJson = outJsonReport["uniqueConstraintViolations"];
    violationsJson.toArray();
    if (cause == ChangeSet::ConflictCause::Constraint && !ourDbValues.empty())
        {
        Statement indexListStmt;
        if (BE_SQLITE_OK == indexListStmt.Prepare(ecdb,
                Utf8PrintfString("PRAGMA [main].[index_list]([%s])", conflict.GetTableName().c_str()).c_str()))
            {
            while (BE_SQLITE_ROW == indexListStmt.Step())
                {
                // index_list columns: seq(0), name(1), unique(2), origin(3), partial(4)
                if (indexListStmt.GetValueInt(2) == 0)
                    continue; // not a unique index

                Utf8String indexName = indexListStmt.GetValueText(1);

                // Collect the column names covered by this index.
                std::vector<Utf8String> idxCols;
                Statement indexInfoStmt;
                if (BE_SQLITE_OK == indexInfoStmt.Prepare(ecdb,
                        Utf8PrintfString("PRAGMA [main].[index_info]([%s])", indexName.c_str()).c_str()))
                    {
                    while (BE_SQLITE_ROW == indexInfoStmt.Step())
                        {
                        // index_info columns: seqno(0), cid(1), name(2)
                        Utf8CP colName = indexInfoStmt.GetValueText(2);
                        if (colName != nullptr)
                            idxCols.push_back(colName);
                        }
                    }

                if (idxCols.empty())
                    continue;

                // Skip this index if any of its columns is absent or null in our new row.
                // (NULL values never violate a UNIQUE constraint.)
                bool allPresent = true;
                for (Utf8StringCR col : idxCols)
                    {
                    auto it = ourDbValues.find(col);
                    if (it == ourDbValues.end() || !it->second.IsValid() || it->second.IsNull())
                        {
                        allPresent = false;
                        break;
                        }
                    }

                if (!allPresent)
                    continue;

                // SELECT all table columns WHERE the indexed columns match our new values.
                // A returned row confirms the violation and gives us the full conflicting row data.
                Utf8String selectPart;
                for (size_t i = 0; i < columns.size(); ++i)
                    {
                    if (!selectPart.empty()) selectPart.append(", ");
                    selectPart.append("[").append(columns[i]).append("]");
                    }
                Utf8String wherePart;
                for (Utf8StringCR col : idxCols)
                    {
                    if (!wherePart.empty()) wherePart.append(" AND ");
                    wherePart.append("[").append(col).append("]=?");
                    }
                Utf8String sql = Utf8PrintfString("SELECT %s FROM [%s] WHERE %s LIMIT 1",
                    selectPart.c_str(), conflict.GetTableName().c_str(), wherePart.c_str());

                Statement theirStmt;
                if (BE_SQLITE_OK != theirStmt.Prepare(ecdb, sql.c_str()))
                    continue;

                int bindIdx = 1;
                for (Utf8StringCR col : idxCols)
                    theirStmt.BindDbValue(bindIdx++, ourDbValues.at(col));

                if (BE_SQLITE_ROW != theirStmt.Step())
                    continue;

                // Found a violation — build the JSON entry while the statement is still active
                BeJsValue violationJson = violationsJson.appendValue();
                violationJson.toObject();

                // uniqueConstraintProperties: property access strings for the violated index columns
                {
                std::unordered_set<Utf8String> idxColSet(idxCols.begin(), idxCols.end());
                BeJsValue constraintPropsJson = violationJson["uniqueConstraintProperties"];
                constraintPropsJson.toArray();
                if (classMap != nullptr)
                    {
                    for (PropertyMap const* propMap : classMap->GetPropertyMaps())
                        {
                        GetColumnsPropertyMapVisitor colVisitor(PropertyMap::Type::Data);
                        propMap->AcceptVisitor(colVisitor);
                        for (DbColumn const* col : colVisitor.GetColumns())
                            {
                            if (idxColSet.count(col->GetName()))
                                {
                                constraintPropsJson.appendValue() = propMap->GetAccessString().c_str();
                                break;
                                }
                            }
                        }
                    }
                }

                // conflictingRow: the existing row that causes this unique constraint violation
                {
                std::unordered_map<Utf8String, DbValue> conflictingRowValues;
                std::vector<DbDupValue> ownedValues;
                ownedValues.reserve(columns.size());
                for (size_t i = 0; i < columns.size(); ++i)
                    {
                    ownedValues.push_back(theirStmt.GetDbValue((int)i));
                    conflictingRowValues.emplace(columns[i], static_cast<DbValue const&>(ownedValues.back()));
                    }
                buildValuesJson(violationJson["conflictingRow"], conflictingRowValues);
                }
                }
            }
        }
    }

    return BentleyStatus::SUCCESS;
    }

/*static*/ BentleyStatus ChangesetReader::GetConflictColumnValues(
    ECDbCR ecdb,
    ChangesetReader::PropertyFilter propertyFilter,
    ChangeSet::ConflictCause cause,
    Changes::Change const& conflict,
    std::vector<std::unique_ptr<IECSqlValue>>& outOriginalValues,
    std::vector<std::unique_ptr<IECSqlValue>>& outTheirValues,
    std::vector<std::unique_ptr<IECSqlValue>>& outOurValues,
    std::vector<Utf8String>& outConflictPropertyAccessStrings)
    {
    outOriginalValues.clear();
    outTheirValues.clear();
    outOurValues.clear();
    outConflictPropertyAccessStrings.clear();

    DbTable const* dbTable = ecdb.Schemas().Main().GetDbSchema().FindTable(conflict.GetTableName());
    if (!dbTable) return BentleyStatus::ERROR;

    bvector<Utf8String> columns;
    if (!ecdb.GetColumns(columns, conflict.GetTableName().c_str()))
        return BentleyStatus::ERROR;

    DbOpcode opcode = conflict.GetOpcode();
    bool originalValueAvailable = opcode == DbOpcode::Update || opcode == DbOpcode::Delete;
    bool ourValueAvailable = opcode == DbOpcode::Update || opcode == DbOpcode::Insert;
    bool theirValueAvailable = cause == ChangeSet::ConflictCause::Data || cause == ChangeSet::ConflictCause::Conflict;

    std::unordered_map<Utf8String, DbValue> originalDbValues;
    std::unordered_map<Utf8String, DbValue> theirDbValues;
    std::unordered_map<Utf8String, DbValue> ourDbValues;
    std::unordered_set<Utf8String> conflictColumns;
    for(int i = 0; i < static_cast<int>(columns.size()); ++i)
        {
        DbValue originalValue = originalValueAvailable ? conflict.GetOldValue(i) : DbValue(nullptr);
        DbValue ourValue = ourValueAvailable ? conflict.GetNewValue(i) : DbValue(nullptr);

        // GetConflictValue will get any column from the current database row.
        // But we only need it if this column is in the conflicting changeset.
        DbValue theirValue = theirValueAvailable && (originalValue.IsValid() || ourValue.IsValid())
            ? conflict.GetConflictValue(i)
            : DbValue(nullptr);

        // SQLite changesets store PK column values only in the Old slot for UPDATE and DELETE.
        // For INSERT, it will only be in the New slot.
        if (conflict.IsPrimaryKeyColumn(i))
            {
            DbValue pkValue = originalValueAvailable ? originalValue : ourValue;
            if (ourValueAvailable && !ourValue.IsValid())
                ourValue = pkValue;
            if (theirValueAvailable && !theirValue.IsValid())
                theirValue = pkValue;
            }

        if (originalValue.IsValid())
            originalDbValues.emplace(columns[i], originalValue);
        if (ourValue.IsValid())
            ourDbValues.emplace(columns[i], ourValue);
        if (theirValue.IsValid())
            theirDbValues.emplace(columns[i], theirValue);

        // Determine which columns represent genuine conflicts.
        // A column is in conflict if "their" value is different from the "original" value.
        // Note that "our" value may be the same as "their" value, but if it is different from "original", it is still a conflict.
        // Also flag conflict columns when they deleted (no THEIR) or inserted (no ORIGINAL).
        bool hasOriginalOrTheir = originalValue.IsValid() || theirValue.IsValid();
        if (hasOriginalOrTheir && (!originalValue.IsValid() || !theirValue.IsValid() || !DbValuesAreEqual(originalValue, theirValue)))
            {
            conflictColumns.emplace(columns[i]);
            }
        }

    // For UNIQUE constraint conflicts, sqlite3changeset_conflict() is unavailable.
    // Probe each UNIQUE index on the table to find which one already holds a row that
    // conflicts with our new values, then fetch that row as Theirs.
    std::vector<DbDupValue> ownedTheirConstraintValues;
    if (cause == ChangeSet::ConflictCause::Constraint && !ourDbValues.empty()) {
        std::vector<Utf8String> violatingCols;

        Statement indexListStmt;
        if (BE_SQLITE_OK == indexListStmt.Prepare(ecdb,
                Utf8PrintfString("PRAGMA [main].[index_list]([%s])", conflict.GetTableName().c_str()).c_str())) {
            while (BE_SQLITE_ROW == indexListStmt.Step() && violatingCols.empty()) {
                // index_list columns: seq(0), name(1), unique(2), origin(3), partial(4)
                if (indexListStmt.GetValueInt(2) == 0)
                    continue; // not a unique index

                Utf8String indexName = indexListStmt.GetValueText(1);

                // Collect the column names covered by this index.
                std::vector<Utf8String> idxCols;
                Statement indexInfoStmt;
                if (BE_SQLITE_OK == indexInfoStmt.Prepare(ecdb,
                        Utf8PrintfString("PRAGMA [main].[index_info]([%s])", indexName.c_str()).c_str())) {
                    while (BE_SQLITE_ROW == indexInfoStmt.Step()) {
                        // index_info columns: seqno(0), cid(1), name(2)
                        Utf8CP colName = indexInfoStmt.GetValueText(2);
                        if (colName != nullptr)
                            idxCols.push_back(colName);
                    }
                }
                if (idxCols.empty())
                    continue;

                // Skip this index if any of its columns is absent or null in our new row.
                // (NULL values never violate a UNIQUE constraint.)
                bool allPresent = true;
                for (Utf8StringCR col : idxCols) {
                    auto it = ourDbValues.find(col);
                    if (it == ourDbValues.end() || !it->second.IsValid() || it->second.IsNull()) {
                        allPresent = false;
                        break;
                    }
                }
                if (!allPresent)
                    continue;

                // SELECT all table columns WHERE the indexed columns match our new values.
                // A returned row confirms the violation and gives us the full Theirs data.
                Utf8String selectPart;
                for (size_t i = 0; i < columns.size(); ++i) {
                    if (!selectPart.empty()) selectPart.append(", ");
                    selectPart.append("[").append(columns[i]).append("]");
                }
                Utf8String wherePart;
                for (Utf8StringCR col : idxCols) {
                    if (!wherePart.empty()) wherePart.append(" AND ");
                    wherePart.append("[").append(col).append("]=?");
                }
                Utf8String sql = Utf8PrintfString("SELECT %s FROM [%s] WHERE %s LIMIT 1",
                    selectPart.c_str(), conflict.GetTableName().c_str(), wherePart.c_str());

                Statement theirStmt;
                if (BE_SQLITE_OK != theirStmt.Prepare(ecdb, sql.c_str()))
                    continue;

                int bindIdx = 1;
                for (Utf8StringCR col : idxCols)
                    theirStmt.BindDbValue(bindIdx++, ourDbValues.at(col));

                if (BE_SQLITE_ROW == theirStmt.Step()) {
                    ownedTheirConstraintValues.reserve(columns.size());
                    for (size_t i = 0; i < columns.size(); ++i) {
                        ownedTheirConstraintValues.push_back(theirStmt.GetDbValue((int)i));
                        theirDbValues.emplace(columns[i], static_cast<DbValue const&>(ownedTheirConstraintValues.back()));
                    }
                    theirValueAvailable = true;
                    violatingCols = std::move(idxCols);
                }
            }
        }

        // Mark only the violating index columns as "in conflict".
        if (!violatingCols.empty()) {
            conflictColumns.clear();
            for (Utf8StringCR col : violatingCols)
                conflictColumns.emplace(col);
        }
    }

    ECClassId classId;
    bool isClassIdFromChangeset = false;
    if (ChangesetValueFactory::ResolveClassId(ecdb, *dbTable, originalDbValues, classId, isClassIdFromChangeset) != SUCCESS)
        return BentleyStatus::ERROR;


    if (originalValueAvailable && ChangesetValueFactory::Create(ecdb, *dbTable, originalDbValues, classId, isClassIdFromChangeset, outOriginalValues, ChangesetReader::PropertyFilter::All, nullptr) != SUCCESS)
        return BentleyStatus::ERROR;
    if (theirValueAvailable && ChangesetValueFactory::Create(ecdb, *dbTable, theirDbValues, classId, isClassIdFromChangeset, outTheirValues, ChangesetReader::PropertyFilter::All, nullptr) != SUCCESS)
        return BentleyStatus::ERROR;
    if (ourValueAvailable && ChangesetValueFactory::Create(ecdb, *dbTable, ourDbValues, classId, isClassIdFromChangeset, outOurValues, ChangesetReader::PropertyFilter::All, nullptr) != SUCCESS)
        return BentleyStatus::ERROR;

    // Build conflict property access strings from conflict column names.
    const ECClass* cls = ecdb.Schemas().Main().GetClass(classId);
    const ClassMap* classMap = ecdb.Schemas().Main().GetClassMap(*cls);
    if (classMap) {
        for (PropertyMap const* propMap : classMap->GetPropertyMaps()) {
            // Data filter causes the visitor to recurse into Point2d/Point3d/Struct/Navigation
            // and collect all leaf DbColumn pointers. SingleColumnData alone would not recurse.
            GetColumnsPropertyMapVisitor colVisitor(PropertyMap::Type::Data);
            propMap->AcceptVisitor(colVisitor);
            for (DbColumn const* col : colVisitor.GetColumns()) {
                if (conflictColumns.count(col->GetName())) {
                    outConflictPropertyAccessStrings.push_back(propMap->GetAccessString());
                    break; // one conflicting column is enough to mark the whole property
                }
            }
        }
    }

    return BentleyStatus::SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE