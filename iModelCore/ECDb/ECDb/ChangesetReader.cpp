
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

/*static*/ BentleyStatus ChangesetReader::GetConflictColumnValues(
    ECDbCR ecdb,
    ChangesetReader::PropertyFilter propertyFilter,
    Changes::Change const& conflict,
    std::vector<std::unique_ptr<IECSqlValue>>& originalValues,
    std::vector<std::unique_ptr<IECSqlValue>>& theirValues,
    std::vector<std::unique_ptr<IECSqlValue>>& ourValues)
    {
    originalValues.clear();
    theirValues.clear();
    ourValues.clear();

    DbTable const* dbTable = ecdb.Schemas().Main().GetDbSchema().FindTable(conflict.GetTableName());
    if (!dbTable) return BentleyStatus::ERROR;

    bvector<Utf8String> columns;
    if (!ecdb.GetColumns(columns, conflict.GetTableName().c_str()))
        return BentleyStatus::ERROR;

    std::unordered_map<Utf8String, DbValue> originalDbValues;
    std::unordered_map<Utf8String, DbValue> theirDbValues;
    std::unordered_map<Utf8String, DbValue> ourDbValues;
    for(int i = 0; i < static_cast<int>(columns.size()); ++i)
        {
            auto originalValue = conflict.GetOldValue(i);
            auto ourValue = conflict.GetNewValue(i);

            // GetConflictValue will get any column from the current database row.
            // But we only need it if is this column is in the conflicting changeset.
            auto theirValue = originalValue.IsValid() || ourValue.IsValid()
                ? conflict.GetConflictValue(i)
                : DbValue(nullptr);

            // SQLite changesets store PK column values only in the Old slot, even for UPDATE.
            if (conflict.IsPrimaryKeyColumn(i))
                {
                if (!ourValue.IsValid())
                    ourValue = originalValue;
                if (!theirValue.IsValid())
                    theirValue = originalValue;
                }

            if (originalValue.IsValid())
                originalDbValues.emplace(columns[i], originalValue);
            if (ourValue.IsValid())
                ourDbValues.emplace(columns[i], ourValue);
            if (theirValue.IsValid())
                theirDbValues.emplace(columns[i], theirValue);
        }

    // TODO: Resolving the class ID _only_ from the original row is not sufficient.
    ECClassId classId;
    bool isClassIdFromChangeset = false;
    if (ChangesetValueFactory::ResolveClassId(ecdb, *dbTable, originalDbValues, classId, isClassIdFromChangeset) != SUCCESS)
        return BentleyStatus::ERROR;


    std::vector<Utf8String> changedPropNames;
    if (ChangesetValueFactory::Create(ecdb, *dbTable, originalDbValues, classId, isClassIdFromChangeset, originalValues, ChangesetReader::PropertyFilter::All, changedPropNames) != SUCCESS)
        return BentleyStatus::ERROR;
    if (ChangesetValueFactory::Create(ecdb, *dbTable, theirDbValues, classId, isClassIdFromChangeset, theirValues, ChangesetReader::PropertyFilter::All, changedPropNames) != SUCCESS)
        return BentleyStatus::ERROR;
    if (ChangesetValueFactory::Create(ecdb, *dbTable, ourDbValues, classId, isClassIdFromChangeset, ourValues, ChangesetReader::PropertyFilter::All, changedPropNames) != SUCCESS)
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE