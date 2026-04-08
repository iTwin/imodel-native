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
DbResult ECChangesetReader::OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, Mode mode) {
    return m_pimpl->OpenFile(ecdb, changesetFile, invert, mode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, Mode mode) {
    return m_pimpl->OpenChangeStream(ecdb, std::move(changeStream), invert, mode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, Mode mode) {
    return m_pimpl->OpenGroup(ecdb, changesetFiles, invert, mode);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECChangesetReader::Close() { m_pimpl->Close(); }
DbResult ECChangesetReader::Step() { return m_pimpl->Step(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::GetTableName(Utf8StringR tableName) const {
    return m_pimpl->GetTableName(tableName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::GetOpcode(DbOpcode& opcode) const {
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
DbResult ECChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    return m_pimpl->GetInstanceKey(stage, key);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::IsECTable(bool& isECTable) const {
    return m_pimpl->IsECTable(isECTable);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::GetChangesetFetchedPropertyNames(std::vector<Utf8String>& out) const {
    return m_pimpl->GetChangesetFetchedPropertyNames(out);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::IsIndirectChange(bool& isIndirect) const {
    return m_pimpl->IsIndirectChange(isIndirect);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::SetTableFilters(std::vector<Utf8String> const& tableFilters) {
    return m_pimpl->SetTableFilters(tableFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters) {
    return m_pimpl->SetOpcodeFilters(opcodeFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::SetECClassIdFilters(std::vector<ECN::ECClassId> const& ecclassIdFilters) {
    return m_pimpl->SetECClassIdFilters(ecclassIdFilters);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::ClearTableFilters() {
    return m_pimpl->ClearTableFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::ClearOpcodeFilters() {
    return m_pimpl->ClearOpcodeFilters();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult ECChangesetReader::ClearECClassIdFilters() {
    return m_pimpl->ClearECClassIdFilters();
}

END_BENTLEY_SQLITE_EC_NAMESPACE