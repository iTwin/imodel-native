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

public:
    Impl() {}
    ~Impl() {}

    void OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert) {
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        m_prepared->OpenFile(file, invert);
    }

    void OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert) {
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        m_prepared->Open(std::move(changeStream), invert);
    }

    void OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, Db const& db, bool invert) {
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        m_prepared->OpenGroup(files, db, invert);
    }

    void Close() {
        if (m_prepared != nullptr)
            m_prepared->Close();
        m_prepared = nullptr;
    }

    bool IsOpen() const { return m_prepared != nullptr && m_prepared->IsOpen(); }
    bool HasRow() const { return m_prepared != nullptr && m_prepared->HasRow(); }

    DbResult Step() {
        if (m_prepared == nullptr)
            return BE_SQLITE_ERROR;
        return m_prepared->Step();
    }

    Utf8StringCR GetTableName()  const { return m_prepared->GetTableName(); }
    DbOpcode     GetOpcode()     const { return m_prepared->GetOpcode(); }
    bool         IsDirect()      const { return m_prepared->IsDirect(); }
    bool         IsIndirect()    const { return m_prepared->IsIndirect(); }
    bool         IsUpdate()      const { return m_prepared->IsUpdate(); }
    bool         IsInsert()      const { return m_prepared->IsInsert(); }
    bool         IsDelete()      const { return m_prepared->IsDelete(); }
    Utf8StringCR GetDdl()        const { return m_prepared->GetDdl(); }
    int          GetColumnCount() const { return m_prepared->GetColumnCount(); }

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const {
        return m_prepared->GetValue(stage, columnIndex);
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
