/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ChangesetValueFactory.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PreparedChangesetReader final {
private:
    using Stage = Changes::Change::Stage;
    using PropertyFilter  = ChangesetReader::PropertyFilter;
    enum class StageProcessResult { Success, Error, Filtered };
    ECDbCR                         m_ecdb;

    // change mappings
    bool                            m_invert = false;
    PropertyFilter                  m_propertyFilter = PropertyFilter::All;
    std::unique_ptr<ChangeStream>   m_changeStream;
    std::unique_ptr<Changes>        m_changes;
    Changes::Change                 m_currentChange;

    // data fields
    std::vector<std::unique_ptr<IECSqlValue>> m_oldFields;
    std::vector<std::unique_ptr<IECSqlValue>> m_newFields;
    std::unordered_map<Utf8String, DbValue> m_columnValues;
    std::vector<Utf8String> m_changedPropNames;

    //! Path to the temporary merged changeset file created by OpenChangeGroup, empty otherwise.
    //! Deleted in Close().
    BeFileName m_tempGroupFile;

    //filters
    bool m_strictMode = false;
    std::vector<Utf8String> m_tableFilters;
    std::vector<DbOpcode> m_opcodeFilters;
    std::vector<Utf8String> m_ecclassNameFilters;

    PreparedChangesetReader(PreparedChangesetReader const&) = delete;
    PreparedChangesetReader& operator=(PreparedChangesetReader const&) = delete;
    void ClearFields();
    BentleyStatus ReFetchValues(bool& isCurrentRowFilteredOut);
    bool IsOpen() const { return m_changeStream != nullptr; }
    bool IsStepped() const { return m_changes != nullptr && m_currentChange.IsValid(); }
    //! Builds a map from SQLite column name to DbValue for the current change at @p stage.
    //! Only columns that are present (non-absent) in the changeset are included in the map.
    //! The map can be passed directly to ChangesetValueFactory::Create.
    BentleyStatus GetColumnValues(Stage stage);
    void DumpColumnValues(std::unordered_map<Utf8String, DbValue> const& map) const;
    bool IsTableAllowedPostFilter(Utf8StringCR tableName) const;
    bool IsOpcodeAllowedPostFilter(DbOpcode const& opcode) const;
    bool IsECClassNameAllowedPostFilter(Utf8StringCR className) const;
    Utf8String DbOpcodeToString(DbOpcode const& opcode) const;
    //! Writes @p changeGroup to a temporary LZMA-compressed changeset file, preserving @p ddlChanges
    //! and @p containsSchemaChanges in the file header. Sets @p outPath to the written path.
    //! The file is written with fast compression (level 1) since it is ephemeral.
    DbResult WriteGroupToFile(ChangeGroup& changeGroup, DdlChanges const& ddlChanges, bool containsSchemaChanges, BeFileNameR outPath);
    //! Stores @p changeStream directly in m_changeStream without any size-based spill logic.
    //! Kept private so callers are steered toward the appropriate Open* methods.
    //! Only ChangesetReader::Impl may call this directly (for the generic ChangeStream path).
    DbResult Open(std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter);
    void ClearMembers();
    BentleyStatus GetColumnCountForCurrentChangedTable(int& columnCount, Utf8StringCR tableName) const;
    StageProcessResult ProcessStageValues(Stage stage, DbTable const& dbTable, std::vector<Utf8String>* changedPropNames);
    void DoStep();
    bool IsOpenAndStepped() const { return IsOpen() && IsStepped(); }
    BentleyStatus CheckColumnCountForStrictMode() const;
public:
    explicit PreparedChangesetReader(ECDbCR ecdb);
    ~PreparedChangesetReader() { CloseInfallible(); }

    DbResult OpenChangesetFile(Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter);
    //! Opens a pre-built in-memory ChangeSet for reading.
    //! If the changeset size exceeds the spill threshold it is transparently written to a
    //! temporary LZMA-compressed file and read back via streaming, keeping peak RAM to a
    //! single copy at a time.
    DbResult OpenInMemoryChangeset(std::unique_ptr<ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold);
    DbResult OpenChangeGroup(T_Utf8StringVector const& files, bool invert, PropertyFilter propertyFilter, size_t spillThreshold);
    BentleyStatus Close();
    void CloseInfallible();
    DbResult Step();
    ECDbCR GetECDb() const { return m_ecdb; }

    BentleyStatus GetTableName(Utf8StringR tableName) const;
    BentleyStatus GetOpcode(DbOpcode& opcode) const;
    int GetColumnCount(Stage stage) const;

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;
    BentleyStatus GetInstanceKey(Stage stage, Utf8StringR key) const;
    BentleyStatus IsECTable(bool& isECTable) const;
    std::vector<Utf8String> const* GetChangeFetchedPropertyNames() const;
    BentleyStatus IsIndirectChange(bool& isIndirect) const;
    //filtering apis
    BentleyStatus SetTableFilters(std::vector<Utf8String> const& tableFilters);
    BentleyStatus SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters);
    BentleyStatus SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters);
    BentleyStatus ClearTableFilters();
    BentleyStatus ClearOpcodeFilters();
    BentleyStatus ClearECClassNameFilters();
    BentleyStatus EnableStrictMode();
    BentleyStatus DisableStrictMode();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
