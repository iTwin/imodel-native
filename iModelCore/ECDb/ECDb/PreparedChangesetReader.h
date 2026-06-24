/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ChangesetValueFactory.h"
#include <list>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass
//! LRU cache mapping SQLite table name to ordered column names (max 25 entries).
//! On cache miss the names are fetched from the DB via PRAGMA_TABLE_INFO and stored.
//! Not thread-safe; intended for use from a single thread.
//+===============+===============+===============+===============+===============+======
struct TableColumnCache final {
private:
    struct Entry {
        Utf8String              tableName;
        std::vector<Utf8String> columnNames;
    };
    std::list<Entry>                                           m_lruList;
    std::unordered_map<Utf8String, std::list<Entry>::iterator>  m_index;
    static constexpr size_t                                     MAX_ENTRIES = 25;

public:
    //! Returns the ordered column names for @p tableName, querying the DB on cache miss.
    //! Returns nullptr on error.
    std::vector<Utf8String> const* GetOrderedColumnNames(ECDbCR ecdb, Utf8StringCR tableName);
    //! Evicts all cached entries.
    void Clear();
};

//=======================================================================================
// @bsiclass
//! Holds all filter and mode state for PreparedChangesetReader.
//+===============+===============+===============+===============+===============+======
struct ChangesetFilterContext final {
    using PropertyFilter = ChangesetReader::PropertyFilter;

    PropertyFilter          m_propertyFilter    = PropertyFilter::All;
    bool                    m_strictMode        = false;
    std::vector<Utf8String> m_tableFilters;
    std::vector<DbOpcode>   m_opcodeFilters;
    std::vector<Utf8String> m_ecclassNameFilters;

    bool IsTableAllowed(Utf8StringCR tableName) const;
    bool IsOpcodeAllowed(DbOpcode const& opcode) const;
    bool IsECClassNameAllowed(Utf8StringCR className) const;
    //! When strict mode is active, returns ERROR if @p changeColumnCount != @p dbColumnCount.
    BentleyStatus CheckColumnCount(int changeColumnCount, int dbColumnCount, Utf8StringCR tableName) const;
    static Utf8String OpcodeToString(DbOpcode const& opcode);
    //! Resets all filter lists, property filter, and mode to defaults.
    void Reset();
};

//=======================================================================================
// @bsiclass
//! Owns the ChangeStream/Changes/Change triple and drives low-level row iteration.
//+===============+===============+===============+===============+===============+======
struct InternalChangeIterator final {
private:
    bool                          m_invert = false;
    std::unique_ptr<ChangeStream> m_changeStream;
    std::unique_ptr<Changes>      m_changes;
    Changes::Change               m_currentChange;

public:
    InternalChangeIterator() : m_currentChange(nullptr, false) {}
    InternalChangeIterator(InternalChangeIterator const&) = delete;
    InternalChangeIterator& operator=(InternalChangeIterator const&) = delete;

    bool IsOpen()           const { return m_changeStream != nullptr; }
    bool IsStepped()        const { return m_changes != nullptr && m_currentChange.IsValid(); }
    bool IsOpenAndStepped() const { return IsOpen() && IsStepped(); }

    //! Takes ownership of @p stream and records the invert flag.
    void Open(std::unique_ptr<ChangeStream> stream, bool invert);
    //! Advances to the next change. Does NOT clear per-row data fields; the caller is
    //! responsible for calling ClearFields() before Advance() when iterating.
    void Advance();
    //! Releases all stream/change resources and resets to the default state.
    void Reset();
    Changes::Change const& GetCurrentChange() const { return m_currentChange; }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PreparedChangesetReader final {
private:
    using Stage          = Changes::Change::Stage;
    using PropertyFilter = ChangesetReader::PropertyFilter;
    enum class StageProcessResult { Success, Error, Filtered };

    ECDbCR                                    m_ecdb;
    InternalChangeIterator                    m_iterator;
    ChangesetFilterContext                    m_filters;
    mutable TableColumnCache                  m_columnCache;

    // per-row output fields
    std::vector<std::unique_ptr<IECSqlValue>> m_oldFields;
    std::vector<std::unique_ptr<IECSqlValue>> m_newFields;
    std::unordered_map<Utf8String, DbValue>   m_columnValues;
    std::vector<Utf8String>                   m_changedPropNames;

    //! Path to the temporary merged changeset file created by OpenChangeGroup, empty otherwise.
    //! Deleted in Close().
    BeFileName m_tempGroupFile;

    PreparedChangesetReader(PreparedChangesetReader const&) = delete;
    PreparedChangesetReader& operator=(PreparedChangesetReader const&) = delete;

    void ClearFields();
    BentleyStatus ReFetchValues(bool& isCurrentRowFilteredOut);
    //! Builds a map from SQLite column name to DbValue for the current change at @p stage.
    //! Only columns that are present (non-absent) in the changeset are included in the map.
    //! The map can be passed directly to ChangesetValueFactory::Create.
    BentleyStatus GetColumnValues(Stage stage);
    void DumpColumnValues(std::unordered_map<Utf8String, DbValue> const& map) const;
    //! Writes @p changeGroup to a temporary LZMA-compressed changeset file, preserving @p ddlChanges
    //! and @p containsSchemaChanges in the file header. Sets @p outPath to the written path.
    //! The file is written with fast compression (level 1) since it is ephemeral.
    DbResult WriteGroupToFile(ChangeGroup& changeGroup, DdlChanges const& ddlChanges, bool containsSchemaChanges, BeFileNameR outPath);
    //! Stores @p changeStream directly in m_iterator without any size-based spill logic.
    //! Kept private so callers are steered toward the appropriate Open* methods.
    DbResult Open(std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter);
    void ClearMembers();
    //! Returns the number of columns in @p tableName, using TableColumnCache for efficiency.
    BentleyStatus GetColumnCountForCurrentChangedTable(int& columnCount, Utf8StringCR tableName) const;
    StageProcessResult ProcessStageValues(Stage stage, DbTable const& dbTable, std::vector<Utf8String>* changedPropNames);

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
    // filtering APIs
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
