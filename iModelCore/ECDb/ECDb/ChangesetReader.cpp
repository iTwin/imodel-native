
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::ChangesetReader() {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ChangesetReader::~ChangesetReader() {
    CloseInfallible();
}

bool ChangesetReader::IsOpen() const {
    return m_pimpl != nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangesetFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter) {
    if (!IsOpen())
        m_pimpl = std::make_unique<PreparedChangesetReader>(ecdb);
    return m_pimpl->OpenChangesetFile(changesetFile, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (!IsOpen())
        m_pimpl = std::make_unique<PreparedChangesetReader>(ecdb);
    return m_pimpl->OpenChangeGroup(changesetFiles, invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold) {
    if (!IsOpen())
        m_pimpl = std::make_unique<PreparedChangesetReader>(ecdb);
    return m_pimpl->OpenInMemoryChangeset(std::move(changeSet), invert, propertyFilter, spillThreshold);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::Close() {
    BentleyStatus status = SUCCESS;
    if (IsOpen())
        status = m_pimpl->Close();
    m_pimpl = nullptr;
    return status;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetReader::CloseInfallible() {
    if (IsOpen())
        m_pimpl->CloseInfallible();
    m_pimpl = nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ChangesetReader::Step() {
    if (!IsOpen()) {
        LOG.error("A file or group of files or a txn or in memory changes or local changes must be opened before stepping.");
        return BE_SQLITE_ERROR;
    }
    return m_pimpl->Step();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetTableName(Utf8StringR tableName) const {
    if (!IsOpen()) {
        LOG.error("A file or group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_pimpl->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetOpcode(DbOpcode& opcode) const {
    if (!IsOpen()) {
        LOG.error("A file or group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_pimpl->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ChangesetReader::GetValue(Stage stage, int columnIndex) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return NoopECSqlValue::GetSingleton();
    }
    return m_pimpl->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ChangesetReader::GetECDb() const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return nullptr;
    }
    return &m_pimpl->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ChangesetReader::GetColumnCount(Stage stage) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return 0;
    }
    return m_pimpl->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_pimpl->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsECTable(bool& isECTable) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_pimpl->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Utf8String> const* ChangesetReader::GetChangeFetchedPropertyNames() const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return nullptr;
    }
    return m_pimpl->GetChangeFetchedPropertyNames();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::IsIndirectChange(bool& isIndirect) const {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    return m_pimpl->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    m_pimpl->SetTableFilters(tableFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    if (!IsOpen())
        return ERROR;
    m_pimpl->SetOpcodeFilters(opcodeFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    m_pimpl->SetECClassNameFilters(ecclassNameFilters);
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearTableFilters() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    m_pimpl->ClearTableFilters();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearOpcodeFilters() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    m_pimpl->ClearOpcodeFilters();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::ClearECClassNameFilters() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    m_pimpl->ClearECClassNameFilters();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::EnableStrictMode() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    m_pimpl->EnableStrictMode();
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetReader::DisableStrictMode() {
    if (!IsOpen()) {
        LOG.error("A file or a group of files or a txn or in memory changes or local changes must be opened before accessing values.");
        return ERROR;
    }
    m_pimpl->DisableStrictMode();
    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE