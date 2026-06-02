/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/PmrObjectAllocator.h>
#include "ChangesetValueFactory.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Formatting options for changed property name output (JS name style and class name form).
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangedPropNameFormatArgs final {
private:
    JsReadOptions m_options;

public:
    bool UseJsNames() const { return m_options.UseJsNames(); }
    bool UseClassFullNameInsteadofClassName() const { return m_options.UseJsNames() && m_options.UseClassFullNameInsteadofClassName(); }

    void SetUseJsNames(bool v) { m_options.SetUseJsNames(v); }
    void SetUseClassFullNameInsteadofClassName(bool v) { m_options.SetUseClassFullNameInsteadofClassName(v); }
};

//=======================================================================================
//! Manages post-step filtering predicates for a changeset reader.
//! Holds filter lists for table names, opcodes, and EC class names, as well as strict mode.
//! No ECDb dependency — pure filter logic.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetFilter final {
private:
    using PropertyFilter = ChangesetReader::PropertyFilter;

    std::vector<Utf8String> m_tableFilters;
    std::vector<DbOpcode>   m_opcodeFilters;
    std::vector<Utf8String> m_ecclassNameFilters;
    bool                    m_strictMode = false;
    PropertyFilter           m_propertyFilter = PropertyFilter::All;


public:
    bool IsTableAllowed(Utf8StringCR tableName) const;
    bool IsOpcodeAllowed(DbOpcode opcode) const;
    bool IsECClassNameAllowed(Utf8StringCR className) const;
    bool IsStrictMode() const { return m_strictMode; }

    void SetTableFilters(std::vector<Utf8String> const& filters)       { m_tableFilters = filters; }
    void SetOpcodeFilters(std::vector<DbOpcode> const& filters)        { m_opcodeFilters = filters; }
    void SetECClassNameFilters(std::vector<Utf8String> const& filters) { m_ecclassNameFilters = filters; }
    void ClearTableFilters()       { m_tableFilters.clear(); }
    void ClearOpcodeFilters()      { m_opcodeFilters.clear(); }
    void ClearECClassNameFilters() { m_ecclassNameFilters.clear(); }
    void EnableStrictMode()        { m_strictMode = true; }
    void DisableStrictMode()       { m_strictMode = false; }
    void Reset()                   { ClearTableFilters(); ClearOpcodeFilters(); ClearECClassNameFilters(); DisableStrictMode(); m_propertyFilter = PropertyFilter::All; }
    void SetPropertyFilter(PropertyFilter filter) { m_propertyFilter = filter; }
    PropertyFilter GetPropertyFilter() const { return m_propertyFilter; }
};
//=======================================================================================
//! Manages creation and cleanup of the temporary LZMA-compressed changeset file used
//! when a merged changeset exceeds the configured spill threshold.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetTempFileManager final {
private:
    ECDbCR     m_ecdb;
    BeFileName m_tempFile;

    ChangesetTempFileManager(ChangesetTempFileManager const&) = delete;
    ChangesetTempFileManager& operator=(ChangesetTempFileManager const&) = delete;

public:
    explicit ChangesetTempFileManager(ECDbCR ecdb) : m_ecdb(ecdb) {}

    //! Writes @p changeGroup to a new temporary LZMA file and tracks the path for later cleanup.
    //! @p containsSchemaChanges and @p ddlChanges are forwarded verbatim to the file header.
    //! Uses fast compression (level 1) since the file is ephemeral.
    DbResult WriteGroupToFile(ChangeGroup& changeGroup, DdlChanges const& ddlChanges, bool containsSchemaChanges);
    //! Deletes the tracked temp file. Returns ERROR if deletion fails.
    BentleyStatus Cleanup();
    //! Deletes the tracked temp file, logging but not propagating errors.
    void CleanupInfallible();
    bool HasTempFile() const { return !m_tempFile.GetNameUtf8().empty(); }
    BeFileNameCR GetTempFilePath() const { return m_tempFile; }
    void Reset() { m_tempFile = BeFileName{}; }
};

//=======================================================================================
//! Iterates over a changeset at the raw SQLite level.
//! Owns the ChangeStream, Changes iterator, and current change handle.
//! Provides raw column values (DbValue) keyed by SQLite column name, and table-level
//! metadata (table name, opcode, EC-table membership, indirect-change flag).
//! No EC-typed output — all EC translation is the responsibility of PreparedChangesetReader.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetSqliteIterator final {
private:
    using Stage = Changes::Change::Stage;
    ECDbCR                        m_ecdb;
    bool                          m_invert = false;
    std::unique_ptr<ChangeStream> m_changeStream;
    std::unique_ptr<Changes>      m_changes;
    Changes::Change               m_currentChange;

    ChangesetSqliteIterator(ChangesetSqliteIterator const&) = delete;
    ChangesetSqliteIterator& operator=(ChangesetSqliteIterator const&) = delete;
    DbResult FirstStep();
    DbResult NextStep();

public:
    explicit ChangesetSqliteIterator(ECDbCR ecdb) : m_ecdb(ecdb), m_currentChange(nullptr, false) {}

    void Open(std::unique_ptr<ChangeStream> changeStream, bool invert);
    //! Resets to a closed state. The stream is destroyed here so that any temp file it is
    //! reading can be safely deleted by the caller afterwards.
    void Close();
    bool IsOpen()    const { return m_changeStream != nullptr; }
    bool IsStepped() const { return m_changes != nullptr && m_currentChange.IsValid(); }

    //! Advances the raw iterator. Returns BE_SQLITE_ROW when a change is available,
    //! BE_SQLITE_DONE when exhausted. Initialises the Changes object on the first call.
    DbResult StepRaw();

    Utf8StringCR GetTableName() const;
    DbOpcode GetOpcode() const;
    BentleyStatus IsECTable(bool& isECTable) const;
    bool IsIndirectChange() const;
    DbValue GetChangeValue(int colIndex, Stage stage) const;
    //! Raw column count reported by the changeset for the current change row.
    int GetChangeColumnCount() const;
    //! Column count for @p tableName via PRAGMA_TABLE_INFO.
    BentleyStatus GetColumnCount(Utf8StringCR tableName, int& columnCount) const;
    static Utf8String DbOpcodeToString(DbOpcode opcode);
};

//=======================================================================================
//! Coordinates EC-typed changeset reading by composing ChangesetFilter,
//! ChangesetTempFileManager, and ChangesetSqliteIterator.
//! Translates raw SQLite column values into IECSqlValue-typed fields via ChangesetValueFactory.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PreparedChangesetReader final {
private:
    using Stage          = Changes::Change::Stage;
    using PropertyFilter = ChangesetReader::PropertyFilter;
    enum class StageProcessResult { Success, Error, Filtered };

    ECDbCR                   m_ecdb;
    ChangesetFilter          m_filter;
    ChangesetTempFileManager m_tempFileManager;
    ChangesetSqliteIterator  m_iterator;

    ChangedPropNameFormatArgs m_args;
    std::vector<Utf8String>  m_changeFetchedPropertyNames;

    std::unordered_map<Stage, std::vector<ChangesetValue*>> m_fields;
    PmrObjectAllocator<ChangesetValue> m_fieldAllocator;

    ChangesetValueFactory m_valueFactory;

    PreparedChangesetReader(PreparedChangesetReader const&) = delete;
    PreparedChangesetReader& operator=(PreparedChangesetReader const&) = delete;

    void ClearValuesOnStep();
    BentleyStatus DoRefetchValues(bool& isCurrentRowFilteredOut);
    BentleyStatus RefetchValues(bool& isCurrentRowFilteredOut);
    bool IsOpen() const { return m_iterator.IsOpen(); }
    //! Stores @p changeStream directly via the iterator without any size-based spill logic.
    //! Kept private so callers are steered toward the appropriate Open* methods.
    DbResult Open(std::unique_ptr<ChangeStream> changeStream, bool invert, PropertyFilter propertyFilter);
    void ClearMembersBeforeClose();
    StageProcessResult ProcessStageValues(Stage stage, DbTable const& dbTable, std::vector<Utf8String>& changeFetchedPropNames);
    bool IsOpenAndStepped() const { return IsOpen() && IsStepped(); }
public:
    explicit PreparedChangesetReader(ECDbCR ecdb);
    ~PreparedChangesetReader() { CloseInfallible(); }

    DbResult OpenChangesetFile(Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter);
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
    bool IsStepped() const { return m_iterator.IsStepped(); }
    BentleyStatus IsECTable(bool& isECTable) const;
    std::vector<Utf8String> const* GetChangeFetchedPropertyNames() const;
    BentleyStatus IsIndirectChange(bool& isIndirect) const;

    // filtering APIs — delegate to m_filter
    void SetTableFilters(std::vector<Utf8String> const& tableFilters)             { m_filter.SetTableFilters(tableFilters); }
    void SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters)             { m_filter.SetOpcodeFilters(opcodeFilters); }
    void SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters) { m_filter.SetECClassNameFilters(ecclassNameFilters); }
    void ClearTableFilters()       { m_filter.ClearTableFilters(); }
    void ClearOpcodeFilters()      { m_filter.ClearOpcodeFilters(); }
    void ClearECClassNameFilters() { m_filter.ClearECClassNameFilters(); }
    void EnableStrictMode()        { m_filter.EnableStrictMode(); }
    void DisableStrictMode()       { m_filter.DisableStrictMode(); }

    // Expose Changed prop names formatting args
    void SetUseJsNamesForChangeFetchedPropertyNames(bool v) { m_args.SetUseJsNames(v); }
    void SetUseClassFullNameInsteadofClassNameForChangeFetchedPropertyNames(bool v) { m_args.SetUseClassFullNameInsteadofClassName(v); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE
