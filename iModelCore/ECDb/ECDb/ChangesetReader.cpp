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
ChangesetReader::ChangesetReader() : m_pimpl(new Impl()) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::~ChangesetReader() { delete m_pimpl; }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangesetFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter) {
    return m_pimpl->OpenChangesetFile(ecdb, changesetFile, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    return m_pimpl->OpenChangeGroup(ecdb, changesetFiles, invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    return m_pimpl->OpenInMemoryChangeset(ecdb, std::move(changeSet), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Close() { return m_pimpl->Close(); }
DbResult ChangesetReader::Step() { return m_pimpl->Step(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetTableName(Utf8StringR tableName) const {
    return m_pimpl->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetOpcode(DbOpcode& opcode) const {
    return m_pimpl->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetReader::GetValue(Stage stage, int columnIndex) const {
    return m_pimpl->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ChangesetReader::GetECDb() const {
    return  m_pimpl->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetReader::GetColumnCount(Stage stage) const {
    return m_pimpl->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    return m_pimpl->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsECTable(bool& isECTable) const {
    return m_pimpl->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const {
    return m_pimpl->GetChangeFetchedPropertyNames(out);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsIndirectChange(bool& isIndirect) const {
    return m_pimpl->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    return m_pimpl->SetTableFilters(tableFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    return m_pimpl->SetOpcodeFilters(opcodeFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    return m_pimpl->SetECClassNameFilters(ecclassNameFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearTableFilters() {
    return m_pimpl->ClearTableFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearOpcodeFilters() {
    return m_pimpl->ClearOpcodeFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearECClassNameFilters() {
    return m_pimpl->ClearECClassNameFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::EnableStrictMode() {
    return m_pimpl->EnableStrictMode();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::DisableStrictMode() {
    return m_pimpl->DisableStrictMode();
}

/*static*/ BentleyStatus ChangesetReader::GetConflictColumnValues(
    ECDbCR ecdb,
    ChangesetReader::PropertyFilter propertyFilter,
    Changes::Change const& conflict,
    std::vector<std::unique_ptr<IECSqlValue>>& originalValues,
    std::vector<std::unique_ptr<IECSqlValue>>& theirValues,
    std::vector<std::unique_ptr<IECSqlValue>>& myValues)
    {
    originalValues.clear();
    theirValues.clear();
    myValues.clear();

    DbTable const* dbTable = ecdb.Schemas().Main().GetDbSchema().FindTable(conflict.GetTableName());
    if (!dbTable) return BentleyStatus::ERROR;

    bvector<Utf8String> columns;
    if (!ecdb.GetColumns(columns, conflict.GetTableName().c_str()))
        return BentleyStatus::ERROR;

    std::unordered_map<Utf8String, DbValue> originalDbValues;
    std::unordered_map<Utf8String, DbValue> theirDbValues;
    std::unordered_map<Utf8String, DbValue> myDbValues;
    for(int i = 0; i < static_cast<int>(columns.size()); ++i)
        {
            auto originalValue = conflict.GetOldValue(i);
            auto myValue = conflict.GetNewValue(i);
            auto theirValue = conflict.GetConflictValue(i);

            // SQLite changesets store PK column values only in the Old slot, even for UPDATE.
            if (conflict.IsPrimaryKeyColumn(i))
                {
                if (!myValue.IsValid())
                    myValue = originalValue;
                if (!theirValue.IsValid())
                    theirValue = originalValue;
                }

            if (originalValue.IsValid())
                originalDbValues.emplace(columns[i], originalValue);
            if (myValue.IsValid())
                myDbValues.emplace(columns[i], myValue);
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
    if (ChangesetValueFactory::Create(ecdb, *dbTable, myDbValues, classId, isClassIdFromChangeset, myValues, ChangesetReader::PropertyFilter::All, changedPropNames) != SUCCESS)
        return BentleyStatus::ERROR;

    return BentleyStatus::SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE