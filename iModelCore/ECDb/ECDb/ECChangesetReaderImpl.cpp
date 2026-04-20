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
DbResult ECChangesetReader::Impl::OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert, PropertyFilter propertyFilter) {
    if (!IsPrepared())
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
    return m_prepared->OpenFile(file, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::Impl::OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter) {
    if(!IsPrepared())
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
    return m_prepared->Open(std::move(changeStream), invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::Impl::OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, bool invert, PropertyFilter propertyFilter) {
    if(!IsPrepared())
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
    return m_prepared->OpenGroup(files, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECChangesetReader::Impl::Close() {
    if (IsPrepared())
        m_prepared->Close();
    m_prepared = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::GetTableName(Utf8StringR tableName) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::GetOpcode(DbOpcode& opcode) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::Impl::Step() {
    if (!IsPrepared())
        return BE_SQLITE_ERROR;
    return m_prepared->Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ECChangesetReader::Impl::GetValue(Stage stage, int columnIndex) const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return NoopECSqlValue::GetSingleton();
    }
    return m_prepared->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ECChangesetReader::Impl::GetECDb() const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return nullptr;
    }
    return &m_prepared->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ECChangesetReader::Impl::GetColumnCount(Stage stage) const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return 0;
    }
    return m_prepared->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::IsECTable(bool& isECTable) const {
    if (!IsPrepared())
        return ERROR;
    return m_prepared->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const {
    if (!IsPrepared()) {
        return ERROR;
    }
    return m_prepared->GetChangeFetchedPropertyNames(out);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::IsIndirectChange(bool& isIndirect) const {
    if (!IsPrepared()) {
        return ERROR;
    }
    return m_prepared->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    if (!IsPrepared())
        return ERROR;
    m_prepared->SetTableFilters(tableFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    if (!IsPrepared())
        return ERROR;
    m_prepared->SetOpcodeFilters(opcodeFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    if (!IsPrepared())
        return ERROR;
    m_prepared->SetECClassNameFilters(ecclassNameFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::ClearTableFilters() {
    if (!IsPrepared())
        return ERROR;
    m_prepared->ClearTableFilters();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::ClearOpcodeFilters() {
    if (!IsPrepared())
        return ERROR;
    m_prepared->ClearOpcodeFilters();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::Impl::ClearECClassNameFilters() {
    if (!IsPrepared())
        return ERROR;
    m_prepared->ClearECClassNameFilters();
    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
