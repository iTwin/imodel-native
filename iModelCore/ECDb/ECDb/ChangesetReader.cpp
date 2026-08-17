
/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"
#include "JsPropertyNaming.h"
#include <BeRapidJson/BeJsValue.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String GetJsMemberName(ECN::ECPropertyCR ecProperty) {
    Utf8String memberName = ecProperty.GetName();
    const auto prim = ecProperty.GetAsPrimitiveProperty();
    if (prim && !prim->GetExtendedTypeName().empty()) {
        const auto extendTypeId = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extendTypeId == ExtendedTypeHelper::ExtendedType::Id && memberName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
            memberName = ECN::ECJsonSystemNames::Id();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::ClassId && memberName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
            memberName = ECN::ECJsonSystemNames::ClassFullName();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::SourceId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId))
            memberName = ECN::ECJsonSystemNames::SourceId();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::SourceClassId && memberName.EqualsIAscii(ECDBSYS_PROP_SourceECClassId))
            memberName = ECN::ECJsonSystemNames::SourceClassName();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::TargetId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId))
            memberName = ECN::ECJsonSystemNames::TargetId();
        else if (extendTypeId == ExtendedTypeHelper::ExtendedType::TargetClassId && memberName.EqualsIAscii(ECDBSYS_PROP_TargetECClassId))
            memberName = ECN::ECJsonSystemNames::TargetClassName();
        else
            ECN::ECJsonUtilities::LowerFirstChar(memberName);
    } else {
        ECN::ECJsonUtilities::LowerFirstChar(memberName);
    }
    return memberName;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String GetJsAccessString(PropertyMap const& propMap) {
    Utf8String jsAccessString;
    for (PropertyMap const* segment : propMap.GetPath()) {
        if (!jsAccessString.empty())
            jsAccessString.append(".");
        jsAccessString.append(GetJsMemberName(segment->GetProperty()));
    }
    return jsAccessString;
}

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

struct ConflictRow : public IECSqlRow {
    private:
    std::vector<std::unique_ptr<IECSqlValue>> const& m_values;
    public:
    ConflictRow(std::vector<std::unique_ptr<IECSqlValue>> const& values) : m_values(values) {}
    virtual int GetColumnCount() const override { return (int)m_values.size(); }
    virtual IECSqlValue const& GetValue(int columnIndex) const override { return *m_values[columnIndex]; }
};

} // namespace

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
    bool theyDeleted = cause == ChangeSet::ConflictCause::NotFound;
    bool weInserted = opcode == DbOpcode::Insert;
    bool theyInserted = weInserted && cause == ChangeSet::ConflictCause::Conflict;

    std::unordered_map<Utf8String, DbValue> originalDbValues;
    std::unordered_map<Utf8String, DbValue> theirDbValues;
    std::unordered_map<Utf8String, DbValue> ourDbValues;
    std::unordered_set<Utf8String> dataConflictColumns;
    std::vector<Utf8String> pkColumns;
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
            pkColumns.push_back(columns[i]);
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
        bool dataConflict = originalValue.IsValid() && theirValue.IsValid() && !DbValuesAreEqual(originalValue, theirValue);
        bool ourUpdateTheirDeleteConflict = theyDeleted && originalValue.IsValid() && ourValue.IsValid() && !DbValuesAreEqual(originalValue, ourValue);
        bool bothInsertConflict = weInserted && theyInserted && theirValueAvailable && ourValueAvailable && !DbValuesAreEqual(ourValue, theirValue);
        if (dataConflict || ourUpdateTheirDeleteConflict || bothInsertConflict)
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
    adaptor.GetOptions()
        .SetIncludeNulls(true)
        .SetUseJsNames(true)
        .SetUseClassFullNameInsteadofClassName(true);

    // Helper: build a property-keyed JSON object from a DB column value map using IECSqlValues
    auto buildValuesJson = [&](BeJsValue outJson, std::unordered_map<Utf8String, DbValue> const& dbValues) -> BentleyStatus
        {
        std::vector<std::unique_ptr<IECSqlValue>> fields;
        if (ChangesetValueFactory::Create(ecdb, *dbTable, dbValues, classId, isClassIdFromChangeset, fields, ChangesetReader::PropertyFilter::All, nullptr) != SUCCESS)
            return BentleyStatus::ERROR;
        return adaptor.RenderRowAsObject(outJson, ConflictRow(fields));
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

    // Build dataConflictProperties: JS-cased property access strings of properties whose DB columns changed
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
                    conflictPropsJson.appendValue() = GetJsAccessString(*propMap).c_str();
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
        // UPDATE changesets omit columns that didn't change, even if they're part of a unique
        // index that IS violated by this change. Backfill those from the row's current value
        // (via its PK) so composite indexes can still be evaluated against the effective new row.
        std::unordered_map<Utf8String, DbValue> effectiveOurValues = ourDbValues;
        std::vector<DbDupValue> ownedBackfillValues;
        if (opcode == DbOpcode::Update && !pkColumns.empty())
            {
            std::vector<Utf8String> missingCols;
            for (Utf8StringCR col : columns)
                {
                if (ourDbValues.find(col) == ourDbValues.end())
                    missingCols.push_back(col);
                }

            if (!missingCols.empty())
                {
                Utf8String selectPart;
                for (Utf8StringCR col : missingCols)
                    {
                    if (!selectPart.empty()) selectPart.append(", ");
                    selectPart.append("[").append(col).append("]");
                    }
                Utf8String wherePart;
                for (Utf8StringCR col : pkColumns)
                    {
                    if (!wherePart.empty()) wherePart.append(" AND ");
                    wherePart.append("[").append(col).append("]=?");
                    }
                Utf8String sql = Utf8PrintfString("SELECT %s FROM [%s] WHERE %s LIMIT 1",
                    selectPart.c_str(), conflict.GetTableName().c_str(), wherePart.c_str());

                Statement backfillStmt;
                if (BE_SQLITE_OK == backfillStmt.Prepare(ecdb, sql.c_str()))
                    {
                    int bindIdx = 1;
                    for (Utf8StringCR col : pkColumns)
                        backfillStmt.BindDbValue(bindIdx++, ourDbValues.at(col));

                    if (BE_SQLITE_ROW == backfillStmt.Step())
                        {
                        ownedBackfillValues.reserve(missingCols.size());
                        for (size_t i = 0; i < missingCols.size(); ++i)
                            {
                            ownedBackfillValues.push_back(backfillStmt.GetDbValue((int)i));
                            effectiveOurValues.emplace(missingCols[i], static_cast<DbValue const&>(ownedBackfillValues.back()));
                            }
                        }
                    }
                }
            }

        Statement indexListStmt;
        if (BE_SQLITE_OK == indexListStmt.Prepare(ecdb,
                Utf8PrintfString("PRAGMA [main].[index_list]([%s])", conflict.GetTableName().c_str()).c_str()))
            {
            while (BE_SQLITE_ROW == indexListStmt.Step())
                {
                // index_list columns: seq(0), name(1), unique(2), origin(3), partial(4)
                if (indexListStmt.GetValueInt(2) == 0)
                    continue; // not a unique index

                // TODO currently ignoring partial indices. We'd have to evaluate the WHERE clause
                // to properly determine if our row is in the index.
                if (indexListStmt.GetValueInt(4) != 0)
                    continue; // partial index

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

                // Skip this index if any of its columns is absent or null in our effective new row.
                // (NULL values never violate a UNIQUE constraint.)
                bool allPresent = true;
                for (Utf8StringCR col : idxCols)
                    {
                    auto it = effectiveOurValues.find(col);
                    if (it == effectiveOurValues.end() || !it->second.IsValid() || it->second.IsNull())
                        {
                        allPresent = false;
                        break;
                        }
                    }

                if (!allPresent)
                    continue;

                // SELECT the row's ECInstanceId WHERE the indexed columns match our new values.
                // A returned row confirms the violation; the InstanceReader then fetches the full conflicting instance.
                DbColumn const* idColumn = dbTable->FindFirst(DbColumn::Kind::ECInstanceId);
                if (!idColumn)
                    continue;

                Utf8String wherePart;
                for (Utf8StringCR col : idxCols)
                    {
                    if (!wherePart.empty()) wherePart.append(" AND ");
                    wherePart.append("[").append(col).append("]=?");
                    }
                // Exclude the row itself, since its own (possibly backfilled) values otherwise match trivially.
                for (Utf8StringCR col : pkColumns)
                    wherePart.append(" AND [").append(col).append("] IS NOT ?");

                Utf8String sql = Utf8PrintfString("SELECT [%s] FROM [%s] WHERE %s LIMIT 1",
                    idColumn->GetName().c_str(), conflict.GetTableName().c_str(), wherePart.c_str());

                Statement theirStmt;
                if (BE_SQLITE_OK != theirStmt.Prepare(ecdb, sql.c_str()))
                    continue;

                int bindIdx = 1;
                for (Utf8StringCR col : idxCols)
                    theirStmt.BindDbValue(bindIdx++, effectiveOurValues.at(col));
                for (Utf8StringCR col : pkColumns)
                    theirStmt.BindDbValue(bindIdx++, effectiveOurValues.at(col));

                if (BE_SQLITE_ROW != theirStmt.Step())
                    continue;

                ECInstanceId conflictingInstanceId = theirStmt.GetValueId<ECInstanceId>(0);

                // Found a violation — build the JSON entry
                BeJsValue violationJson = violationsJson.appendValue();
                violationJson.toObject();

                // uniqueConstraintProperties: JS-cased property access strings for the violated index columns
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
                                constraintPropsJson.appendValue() = GetJsAccessString(*propMap).c_str();
                                break;
                                }
                            }
                        }
                    }
                }

                // conflictingRow: read the full existing instance that causes this unique constraint violation
                ecdb.GetInstanceReader().Seek(InstanceReader::Position(conflictingInstanceId, classId),
                    [&](InstanceReader::IRowContext const& row, PropertyReader::Finder) {
                        adaptor.RenderRowAsObject(violationJson["conflictingRow"], row);
                    });
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