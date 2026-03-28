/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using Stage = Changes::Change::Stage;

//=======================================================================================
// ECChangesetReader public API — delegates to Impl
//+===============+===============+===============+===============+===============+======

ECChangesetReader::ECChangesetReader() : m_pimpl(new Impl()) {}

ECChangesetReader::~ECChangesetReader() { delete m_pimpl; }

DbResult ECChangesetReader::OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert) {
    return m_pimpl->OpenFile(ecdb, changesetFile, invert);
}

DbResult ECChangesetReader::OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert) {
    return m_pimpl->OpenChangeStream(ecdb, std::move(changeStream), invert);
}

DbResult ECChangesetReader::OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, Db const& db, bool invert) {
    return m_pimpl->OpenGroup(ecdb, changesetFiles, db, invert);
}

void ECChangesetReader::Close() { m_pimpl->Close(); }
DbResult ECChangesetReader::Step() { return m_pimpl->Step(); }

DbResult ECChangesetReader::GetTableName(Utf8StringR tableName) const {
    return m_pimpl->GetTableName(tableName);
}

DbResult ECChangesetReader::GetOpcode(DbOpcode& opcode) const {
    return m_pimpl->GetOpcode(opcode);
}

IECSqlValue const& ECChangesetReader::GetValue(Stage stage, int columnIndex) const {
    return m_pimpl->GetValue(stage, columnIndex);
}

ECDb const* ECChangesetReader::GetECDb() const {
    return  m_pimpl->GetECDb();
}

int ECChangesetReader::GetColumnCount(Stage stage) const {
    return m_pimpl->GetColumnCount(stage);
}

DbResult ECChangesetReader::GetInstanceKey(Stage stage, Utf8StringR key) const {
    return m_pimpl->GetInstanceKey(stage, key);
}

END_BENTLEY_SQLITE_EC_NAMESPACE