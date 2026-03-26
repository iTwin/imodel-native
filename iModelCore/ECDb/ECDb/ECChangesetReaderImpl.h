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
    bool IsOpen() const { return m_prepared != nullptr && m_prepared->IsOpen(); }
public:
    Impl() {}
    ~Impl() {}

    void OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert) {
        if(IsOpen())
            Close();
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        m_prepared->OpenFile(file, invert);
    }

    void OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert) {
        if(IsOpen())
            Close();
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        m_prepared->Open(std::move(changeStream), invert);
    }

    void OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, Db const& db, bool invert) {
        if(IsOpen())
            Close();
        m_prepared = std::make_unique<PreparedECChangesetReader>(ecdb);
        m_prepared->OpenGroup(files, db, invert);
    }

    void Close() {
        if (m_prepared != nullptr)
            m_prepared->Close();
        m_prepared = nullptr;
    }


    DbResult Step() {
        if (m_prepared == nullptr)
            return BE_SQLITE_ERROR;
        return m_prepared->Step();
    }

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const {
        if(!IsOpen()) {
            LOG.error("ECChangesetReader is not open.");
            return NoopECSqlValue::GetSingleton();
        }
        return m_prepared->GetValue(stage, columnIndex);
    }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
