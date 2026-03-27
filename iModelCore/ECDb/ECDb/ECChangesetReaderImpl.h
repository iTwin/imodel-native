/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECChangesetReader.h"
#include "PreparedECChangesetReader.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// ECChangesetReader::Impl — thin wrapper owning a PreparedECChangesetReader
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECChangesetReader::Impl final {
    using Stage = Changes::Change::Stage;
private:
    std::unique_ptr<PreparedECChangesetReader> m_prepared;

    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    bool IsPrepared() const { return m_prepared != nullptr }
public:
    Impl() {}
    ~Impl() {}

    DbResult OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert) {
        if(!IsPrepared()) {
            m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        }
        return m_prepared->OpenFile(file, invert);
    }

    DbResult OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert) {
        if(!IsPrepared()) {
            m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        }
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        return m_prepared->Open(std::move(changeStream), invert);
    }

    DbResult OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, Db const& db, bool invert) {
        if(!IsPrepared()) {
            m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        }
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        return m_prepared->OpenGroup(files, db, invert);
    }

    void Close() {
        if (IsPrepared())
            m_prepared->Close();
        m_prepared = nullptr;
    }

    DbResult GetTableName(Utf8StringR tableName) const {
        if (!IsPrepared())
            return BE_SQLITE_ERROR;
        Utf8String tableName;
        return m_prepared->GetTableName(tableName);
    }

    DbResult GetOpcode(DbOpcode& opcode) const {
        if (!IsPrepared())
            return BE_SQLITE_ERROR;
        DbOpcode opcode = DbOpcode::Insert;
        return m_prepared->GetOpcode(opcode);
    }
    
    DbResult Step() {
        if (!IsPrepared())
            return BE_SQLITE_ERROR;
        return m_prepared->Step();
    }

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const {
        if(!IsPrepared()) {
            LOG.error("A file or a txn or in memory changes must be opened before accessing values.");
            return NoopECSqlValue::GetSingleton();
        }
        return m_prepared->GetValue(stage, columnIndex);
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
