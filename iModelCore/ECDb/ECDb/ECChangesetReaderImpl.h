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
    using Mode  = ECChangesetReader::Mode;
    std::unique_ptr<PreparedECChangesetReader> m_prepared;

    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    bool IsPrepared() const { return m_prepared != nullptr; }
public:
    Impl() {}
    ~Impl() {Close();}

    DbResult OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert, Mode mode);
    DbResult OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, Mode mode);
    DbResult OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, bool invert, Mode mode);
    void Close();
    DbResult Step();

    BentleyStatus GetTableName(Utf8StringR tableName) const;
    BentleyStatus GetOpcode(DbOpcode& opcode) const;
    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
    ECDb const* GetECDb() const;
    int GetColumnCount(Stage stage) const;
    BentleyStatus GetInstanceKey(Stage stage, Utf8StringR key) const;
    BentleyStatus IsECTable(bool& isECTable) const;
    BentleyStatus GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const;
    BentleyStatus IsIndirectChange(bool& isIndirect) const;

    // Filtering
    BentleyStatus SetTableFilters(std::vector<Utf8String> const& tableFilters);
    BentleyStatus SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters);
    BentleyStatus SetECClassIdFilters(std::vector<ECN::ECClassId> const& ecclassIdFilters);
    BentleyStatus ClearTableFilters();
    BentleyStatus ClearOpcodeFilters();
    BentleyStatus ClearECClassIdFilters();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
