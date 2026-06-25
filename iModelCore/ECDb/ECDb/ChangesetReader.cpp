
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
ChangesetReader::ChangesetReader(): m_innerReader(nullptr) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::~ChangesetReader() {
    CloseInfallible();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetReader::IsOpen() const {
    return m_innerReader != nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangesetFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter) {
    if (!IsOpen())
        m_innerReader = std::make_unique<PreparedChangesetReader>(ecdb);
    auto status = m_innerReader->OpenChangesetFile(changesetFile, invert, propertyFilter);
    if(status != BE_SQLITE_OK)
        CloseInfallible();
    
    return status;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (!IsOpen())
        m_innerReader = std::make_unique<PreparedChangesetReader>(ecdb);
    auto status = m_innerReader->OpenChangeGroup(changesetFiles, invert, propertyFilter, spillThreshold);
    if(status != BE_SQLITE_OK)
        CloseInfallible();
    
    return status;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (!IsOpen())
        m_innerReader = std::make_unique<PreparedChangesetReader>(ecdb);
    auto status = m_innerReader->OpenInMemoryChangeset(std::move(changeSet), invert, propertyFilter, spillThreshold);
    if(status != BE_SQLITE_OK)
        CloseInfallible();
    
    return status;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Close() {
    BentleyStatus status = SUCCESS;
    if (IsOpen())
        status = m_innerReader->Close();
    m_innerReader = nullptr;
    return status;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetReader::CloseInfallible() {
    if (IsOpen())
        m_innerReader->CloseInfallible();
    m_innerReader = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::Step() {
    if (!IsOpen()) {
        LOG.error("A file or group of files or a txn or in memory changes or local changes must be opened before stepping.");
        return BE_SQLITE_ERROR;
    }
    return m_innerReader->Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetTableName(Utf8StringR tableName) const {
    if (!IsOpen()) {
        LOG.error("A file or group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetOpcode(DbOpcode& opcode) const {
    if (!IsOpen()) {
        LOG.error("A file or group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return NoopECSqlValue::GetSingleton();
    }
    return m_innerReader->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ChangesetReader::GetECDb() const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return nullptr;
    }
    return &m_innerReader->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetReader::GetColumnCount(Stage stage) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return 0;
    }
    return m_innerReader->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsECTable(bool& isECTable) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> const* ChangesetReader::GetChangeFetchedPropertyNames() const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return nullptr;
    }
    return m_innerReader->GetChangeFetchedPropertyNames();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsIndirectChange(bool& isIndirect) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->SetTableFilters(tableFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    if (!IsOpen())
        return ERROR;
    return m_innerReader->SetOpcodeFilters(opcodeFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->SetECClassNameFilters(ecclassNameFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearTableFilters() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->ClearTableFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearOpcodeFilters() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->ClearOpcodeFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearECClassNameFilters() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->ClearECClassNameFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::EnableStrictMode() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->EnableStrictMode();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::DisableStrictMode() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_innerReader->DisableStrictMode();
}

END_BENTLEY_SQLITE_EC_NAMESPACE