
/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"

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

    ECClassId classId;
    bool isClassIdFromChangeset = false;
    if (ChangesetValueFactory::ResolveClassId(ecdb, *dbTable, originalDbValues, classId, isClassIdFromChangeset) != SUCCESS)
        return BentleyStatus::ERROR;


    if (ChangesetValueFactory::Create(ecdb, *dbTable, originalDbValues, classId, isClassIdFromChangeset, outOriginalValues, ChangesetReader::PropertyFilter::All, nullptr) != SUCCESS)
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