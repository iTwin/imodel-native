/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include "PreparedChangesetReader.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// ChangesetReader::Impl — thin wrapper owning a PreparedChangesetReader
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetReader::Impl final {
private:
    using PropertyFilter = ChangesetReader::PropertyFilter;
    std::unique_ptr<PreparedChangesetReader> m_prepared;

    Impl(Impl const&) = delete;
    Impl& operator=(Impl const&) = delete;
    bool IsPrepared() const { return m_prepared != nullptr; }
public:
    Impl() {}
    ~Impl() {Close();}

    DbResult OpenFile(ECDbCR ecdb, Utf8StringCR file, bool invert, PropertyFilter propertyFilter);
    DbResult OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter);
    DbResult OpenChangeSet(ECDbCR ecdb, std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold);
    DbResult OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& files, bool invert, PropertyFilter propertyFilter, size_t spillThreshold);
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
    BentleyStatus SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters);
    BentleyStatus ClearTableFilters();
    BentleyStatus ClearOpcodeFilters();
    BentleyStatus ClearECClassNameFilters();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
