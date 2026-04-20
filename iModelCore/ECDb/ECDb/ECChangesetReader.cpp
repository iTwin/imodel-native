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
ECChangesetReader::ECChangesetReader() : m_pimpl(new Impl()) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECChangesetReader::~ECChangesetReader() { delete m_pimpl; }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter) {
    return m_pimpl->OpenFile(ecdb, changesetFile, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter) {
    return m_pimpl->OpenChangeStream(ecdb, std::move(changeStream), invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, PropertyFilter propertyFilter) {
    return m_pimpl->OpenGroup(ecdb, changesetFiles, invert, propertyFilter);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECChangesetReader::Close() { m_pimpl->Close(); }
DbResult ECChangesetReader::Step() { return m_pimpl->Step(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::GetTableName(Utf8StringR tableName) const {
    return m_pimpl->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::GetOpcode(DbOpcode& opcode) const {
    return m_pimpl->GetOpcode(opcode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& ECChangesetReader::GetValue(Stage stage, int columnIndex) const {
    return m_pimpl->GetValue(stage, columnIndex);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDb const* ECChangesetReader::GetECDb() const {
    return  m_pimpl->GetECDb();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ECChangesetReader::GetColumnCount(Stage stage) const {
    return m_pimpl->GetColumnCount(stage);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    return m_pimpl->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::IsECTable(bool& isECTable) const {
    return m_pimpl->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const {
    return m_pimpl->GetChangeFetchedPropertyNames(out);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::IsIndirectChange(bool& isIndirect) const {
    return m_pimpl->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    return m_pimpl->SetTableFilters(tableFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    return m_pimpl->SetOpcodeFilters(opcodeFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) {
    return m_pimpl->SetECClassNameFilters(ecclassNameFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::ClearTableFilters() {
    return m_pimpl->ClearTableFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::ClearOpcodeFilters() {
    return m_pimpl->ClearOpcodeFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECChangesetReader::ClearECClassNameFilters() {
    return m_pimpl->ClearECClassNameFilters();
}

END_BENTLEY_SQLITE_EC_NAMESPACE