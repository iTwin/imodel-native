/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include "PreparedECChangesetReader.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// ECChangesetReader::Impl — thin wrapper owning a PreparedECChangesetReader
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECChangesetReader::Impl final {
private:
    using Stage = Changes::Change::Stage;
    using Mode  = ECChangesetReader::Mode;
    std::unique_ptr<PreparedECChangesetReader> m_prepared;

    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    bool IsPrepared() const { return m_prepared != nullptr; }
public:
    Impl() {}
    ~Impl() {}

    DbResult OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert, Mode mode);
    DbResult OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, Mode mode);
    DbResult OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, Db const& db, bool invert, Mode mode);
    void Close();
    DbResult Step();

    DbResult GetTableName(Utf8StringR tableName) const;
    DbResult GetOpcode(DbOpcode& opcode) const;
    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
    ECDb const* GetECDb() const;
    int GetColumnCount(Stage stage) const;
    DbResult GetInstanceKey(Stage stage, Utf8StringR key) const;
    DbResult IsECTable(bool& isECTable) const;
    DbResult GetChangedPropertyNames(std::unordered_set<Utf8String>& out) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
