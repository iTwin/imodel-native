/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// ECChangesetReader::Impl
//+===============+===============+===============+===============+===============+======

DbResult ECChangesetReader::Impl::OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert, Mode mode) {
    if (!IsPrepared())
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
    return m_prepared->OpenFile(file, invert, mode);
}

DbResult ECChangesetReader::Impl::OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, Mode mode) {
    m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
    return m_prepared->Open(std::move(changeStream), invert, mode);
}

DbResult ECChangesetReader::Impl::OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, Db const& db, bool invert, Mode mode) {
    m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
    return m_prepared->OpenGroup(files, db, invert, mode);
}

void ECChangesetReader::Impl::Close() {
    if (IsPrepared())
        m_prepared->Close();
    m_prepared = nullptr;
}

DbResult ECChangesetReader::Impl::GetTableName(Utf8StringR tableName) const {
    if (!IsPrepared())
        return BE_SQLITE_ERROR;
    return m_prepared->GetTableName(tableName);
}

DbResult ECChangesetReader::Impl::GetOpcode(DbOpcode& opcode) const {
    if (!IsPrepared())
        return BE_SQLITE_ERROR;
    return m_prepared->GetOpcode(opcode);
}

DbResult ECChangesetReader::Impl::Step() {
    if (!IsPrepared())
        return BE_SQLITE_ERROR;
    return m_prepared->Step();
}

IECSqlValue const& ECChangesetReader::Impl::GetValue(Stage stage, int columnIndex) const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return NoopECSqlValue::GetSingleton();
    }
    return m_prepared->GetValue(stage, columnIndex);
}

ECDb const* ECChangesetReader::Impl::GetECDb() const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return nullptr;
    }
    return &m_prepared->GetECDb();
}

int ECChangesetReader::Impl::GetColumnCount(Stage stage) const {
    if (!IsPrepared()) {
        LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
        return 0;
    }
    return m_prepared->GetColumnCount(stage);
}

DbResult ECChangesetReader::Impl::GetInstanceKey(Stage stage, Utf8StringR key) const {
    if (!IsPrepared())
        return BE_SQLITE_ERROR;
    return m_prepared->GetInstanceKey(stage, key);
}

DbResult ECChangesetReader::Impl::IsECTable(bool& isECTable) const {
    if (!IsPrepared())
        return BE_SQLITE_ERROR;
    return m_prepared->IsECTable(isECTable);
}

DbResult ECChangesetReader::Impl::GetChangesetFetchedPropertyNames(std::vector<Utf8String>& out) const {
    if (!IsPrepared()) {
        return BE_SQLITE_ERROR;
    }
    return m_prepared->GetChangesetFetchedPropertyNames(out);
}

END_BENTLEY_SQLITE_EC_NAMESPACE
