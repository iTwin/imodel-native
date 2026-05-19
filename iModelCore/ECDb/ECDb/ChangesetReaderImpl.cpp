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
DbResult ChangesetReader::Impl::OpenChangesetFile(ECDbCR ecdb, Utf8StringCR file, bool invert, PropertyFilter propertyFilter) {
    if (!IsPrepared()) {
        m_prepared = std::make_unique<PreparedChangesetReader>(ecdb);
    }
    return m_prepared->OpenChangesetFile(file, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::Impl::OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& files, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if(!IsPrepared()) {
        m_prepared = std::make_unique<PreparedChangesetReader>(ecdb);
    }
    return m_prepared->OpenChangeGroup(files, invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::Impl::OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (!IsPrepared()) {
        m_prepared = std::make_unique<PreparedChangesetReader>(ecdb);
    }
    return m_prepared->OpenInMemoryChangeset(std::move(changeSet), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::Close() {
    BentleyStatus status = SUCCESS;
    if (IsPrepared())
        status = m_prepared->Close();
    if(status != SUCCESS) {
        return status;
    }
    m_prepared = nullptr;
    return status;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetReader::Impl::CloseInfallible() {
    if (IsPrepared())
        m_prepared->CloseInfallible();
    m_prepared = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::GetTableName(Utf8StringR tableName) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::GetOpcode(DbOpcode& opcode) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::Impl::Step() {
    if (!IsPrepared())
        return BE_SQLITE_ERROR;
    return m_prepared->Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetReader::Impl::GetValue(Stage stage, int columnIndex) const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return NoopECSqlValue::GetSingleton();
    }
    return m_prepared->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ChangesetReader::Impl::GetECDb() const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return nullptr;
    }
    return &m_prepared->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetReader::Impl::GetColumnCount(Stage stage) const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return 0;
    }
    return m_prepared->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::IsECTable(bool& isECTable) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const {
    if (!IsPrepared()) {
        return ERROR;
    }
    return m_prepared->GetChangeFetchedPropertyNames(out);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::IsIndirectChange(bool& isIndirect) const {
    if (!IsPrepared()) {
        return ERROR;
    }
    return m_prepared->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    if (!IsPrepared())
        return ERROR;
    m_prepared->SetTableFilters(tableFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    if (!IsPrepared())
        return ERROR;
    m_prepared->SetOpcodeFilters(opcodeFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    if (!IsPrepared())
        return ERROR;
    m_prepared->SetECClassNameFilters(ecclassNameFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::ClearTableFilters() {
    if (!IsPrepared())
        return ERROR;
    m_prepared->ClearTableFilters();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::ClearOpcodeFilters() {
    if (!IsPrepared())
        return ERROR;
    m_prepared->ClearOpcodeFilters();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Impl::ClearECClassNameFilters() {
    if (!IsPrepared())
        return ERROR;
    m_prepared->ClearECClassNameFilters();
    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
