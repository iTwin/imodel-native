/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct PreparedChangesetReader;

//=======================================================================================
//! ChangesetReader provides EC-typed value access while iterating over changesets.
//! It iterates over the changeset and exposes a higher-level API that returns EC-typed values and abstracts away the underlying SQLite tables and columns.
//! It follows the same PIMPL pattern as ECSqlStatement.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ChangesetReader {
public:
    //! Controls which properties are emitted per row.
    enum class PropertyFilter {
        All = 0, //!< Emit ECInstanceId, ECClassId, and all user/domain properties.
        BisCoreElement = 1, //!< Emit ECInstanceId, ECClassId, and all BisCore Element properties if the class is derived from BisCore::Element, For classes not derived from BisCore::Element, all properties are returned.
        InstanceKey = 2,   //!< Emit ECInstanceId and ECClassId only (cheapest).
    };

private:
    using Stage = Changes::Change::Stage;
    std::unique_ptr<PreparedChangesetReader> m_pimpl;

    bool IsOpen() const;
    void CloseInfallible();

    ChangesetReader(ChangesetReader const&) = delete;
    ChangesetReader& operator=(ChangesetReader const&) = delete;
    ChangesetReader(ChangesetReader&& rhs) = delete;
    ChangesetReader& operator=(ChangesetReader&& rhs) = delete;

public:
    //! Constructs a new ChangesetReader instance. Call one of the Open methods before using it.
    ECDB_EXPORT ChangesetReader();
    //! Destructor. Closes the reader if it is still open.
    ECDB_EXPORT ~ChangesetReader();

    //! Opens a changeset file for reading.
    //! @param[in] ecdb ECDb connection used to resolve EC schema information.
    //! @param[in] changesetFile Path to the changeset file to open.
    //! @param[in] invert If true, the changeset is read as an inverted (undo) changeset.
    //! @param[in] propertyFilter Controls which properties are emitted per row. @see PropertyFilter
    //! @return BE_SQLITE_OK on success, or an error code if the file cannot be opened.
    ECDB_EXPORT DbResult OpenChangesetFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, PropertyFilter propertyFilter);

    //! Opens a group of changeset files for reading as a single logical sequence.
    //! @param[in] ecdb ECDb connection used to resolve EC schema information.
    //! @param[in] changesetFiles Ordered list of paths to the changeset files to open.
    //! @param[in] invert If true, the changesets are read as inverted (undo) changesets, iterated in reverse order.
    //! @param[in] propertyFilter Controls which properties are emitted per row. @see PropertyFilter
    //! @param[in] spillThreshold Byte size above which the merged changeset is spilled to a temporary LZMA-compressed file on disk rather than held entirely in memory.
    //! @return BE_SQLITE_OK on success, or an error code if the group could not be opened.
    ECDB_EXPORT DbResult OpenChangeGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, PropertyFilter propertyFilter, size_t spillThreshold);

    //! Opens a pre-built in-memory ChangeSet for reading.
    //! If the changeset byte size meets or exceeds @p spillThreshold, it is transparently
    //! spilled to a temporary LZMA-compressed file and read back via streaming,
    //! keeping peak RAM to roughly one copy of the data at a time.
    //! @param[in] ecdb ECDb connection used to resolve EC schema information.
    //! @param[in] changeSet The in-memory changeset to open. Ownership is transferred.
    //! @param[in] invert If true, the changeset is read as an inverted (undo) changeset.
    //! @param[in] propertyFilter Controls which properties are emitted per row. @see PropertyFilter
    //! @param[in] spillThreshold Byte size above which the changeset is spilled to disk.
    //! @return BE_SQLITE_OK on success, or an error code if the changeset could not be opened.
    ECDB_EXPORT DbResult OpenInMemoryChangeset(ECDbCR ecdb, std::unique_ptr<BeSQLite::ChangeSet> changeSet, bool invert, PropertyFilter propertyFilter, size_t spillThreshold);

    //! Closes the reader and releases all associated resources.
    //! @remarks Safe to call on a reader that was never opened or that has already been closed.
    //! @return SUCCESS on success, or ERROR if a resource could not be released (e.g. the temporary spill file could not be deleted).
    ECDB_EXPORT BentleyStatus Close();

    //! Advances the reader to the next change row.
    //! @return BE_SQLITE_ROW if a new row is available, BE_SQLITE_DONE when there are no more rows,
    //!         or an error code on failure.
    //! In case the row is filtered out by the currently set filters, the reader will automatically advance to the next unfiltered row.
    ECDB_EXPORT DbResult Step();

    //! Gets the ECDb connection associated with this reader.
    //! @return The ECDb connection passed to the Open method, or nullptr if the reader is not open.
    ECDB_EXPORT ECDb const* GetECDb() const;

    //! Gets the number of columns available for the specified change stage.
    //! @param[in] stage The change stage (Old/New) to query. @see Changes::Change::Stage
    //! @return The number of columns, or 0 if the reader is not open or has not been stepped.
    ECDB_EXPORT int GetColumnCount(Stage stage) const;

    //! Gets the name of the underlying SQLite table for the current change row.
    //! @param[out] tableName Receives the table name.
    //! @return SUCCESS on success, or ERROR if the reader is not positioned on a valid row.
    ECDB_EXPORT BentleyStatus GetTableName(Utf8StringR tableName) const;

    //! Gets the DML opcode (Insert, Update, or Delete) of the current change row.
    //! @param[out] opcode Receives the opcode for the current row.
    //! @return SUCCESS on success, or ERROR if the reader is not positioned on a valid row.
    ECDB_EXPORT BentleyStatus GetOpcode(DbOpcode& opcode) const;

    //! Gets the EC-typed value of a specific column for the given change stage.
    //! @param[in] stage The change stage (Old/New) to query. @see Changes::Change::Stage
    //! @param[in] columnIndex Zero-based index of the column to retrieve.
    //! @return The EC-typed value. Returns a no-op singleton value if the reader is closed, not stepped, or the index is out of range.
    ECDB_EXPORT IECSqlValue const& GetValue(Stage stage, int columnIndex) const;

    //! Gets a JSON string representing the ECInstanceId and ECClassId (the instance key) for the current change row.
    //! @param[in] stage The change stage (Old/New) to query. @see Changes::Change::Stage
    //! @param[out] key Receives the JSON-encoded instance key string.
    //! @return SUCCESS on success, or ERROR if the reader is not positioned on a valid row.
    ECDB_EXPORT BentleyStatus GetInstanceKey(Stage stage, Utf8StringR key) const;

    //! Determines whether the current change row belongs to an EC table.
    //! @param[out] isECTable Set to true if the current table is an EC table, false otherwise.
    //! @return SUCCESS on success, or ERROR if the reader is not positioned on a valid row.
    ECDB_EXPORT BentleyStatus IsECTable(bool& isECTable) const;

    //! Gets the names of properties that were fetched from the current row of change.
    //! @remarks For compound data properties like point2d, point3d or navigation properties,
    //! the full name of the property is returned in case all the components of the property are fetched from the change.
    //! If all of the components are not fetched from the change(meaning they did not change), 
    //! then the individual component names which changed are returned smartly by using `.` as a separator (e.g. "MyPoint.X", "MyPoint.Y" for a point3d property "MyPoint" if only X and Y changed). 
    //! For struct properties the property names are always returned in the "StructProp.MemberName" format.
    //! So if only X changed for a point2d property named "Myp2d" inide a struct "CustomStruct", the returned property name will be "CustomStruct.Myp2d.X".
    //! Similaly if both X and Y changed for the same point2d property, the returned property name will be "CustomStruct.Myp2d".
    //! @return const pointer to the list of property names, or nullptr if the reader is not positioned on a valid row.
    ECDB_EXPORT std::vector<Utf8String> const* GetChangeFetchedPropertyNames() const;

    //! Determines whether the current change row is an indirect change.
    //! @param[out] isIndirect Set to true if the current change is indirect, false otherwise.
    //! @return SUCCESS on success, or ERROR if the reader is not positioned on a valid row.
    ECDB_EXPORT BentleyStatus IsIndirectChange(bool& isIndirect) const;

    //! Sets a list of SQLite table-name filters. Only rows whose table name matches one of the entries will be returned by Step().
    //! @remarks Filters are applied on the next call to Step(). Pass an empty vector to match all tables.
    //! @param[in] tableFilters List of table names to include.
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus SetTableFilters(std::vector<Utf8String> const& tableFilters);

    //! Sets a list of opcode filters. Only rows whose opcode matches one of the entries will be returned by Step().
    //! @remarks Filters are applied on the next call to Step(). Pass an empty vector to match all opcodes.
    //! @param[in] opcodeFilters List of DbOpcode values (Insert, Update, Delete) to include.
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters);

    //! Sets a list of EC class name filters. Only rows whose EC class full name ("SchemaName:ClassName") matches one of the entries will be returned by Step().
    //! @remarks Filters are applied on the next call to Step(). Pass an empty vector to match all ECClasses.
    //! @param[in] ecclassNameFilters List of EC class full names ("SchemaName:ClassName") to include.
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus SetECClassNameFilters(std::vector<Utf8String> const& ecclassNameFilters);

    //! Clears all table-name filters previously set with SetTableFilters().
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus ClearTableFilters();

    //! Clears all opcode filters previously set with SetOpcodeFilters().
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus ClearOpcodeFilters();

    //! Clears all EC class name filters previously set with SetECClassNameFilters().
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus ClearECClassNameFilters();

    //! Enables strict mode for the changeset reader.
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus EnableStrictMode();

    //! Disables strict mode for the changeset reader.
    //! @return SUCCESS on success, or ERROR if the reader is not open.
    ECDB_EXPORT BentleyStatus DisableStrictMode();

};


//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ChangesetRow : public IECSqlRow {
    private:
        ChangesetReader const& m_reader;
        Changes::Change::Stage m_stage;       
    public:
        ChangesetRow(ChangesetReader const& reader, Changes::Change::Stage const& stage) : m_reader(reader), m_stage(stage) {}
        virtual int GetColumnCount() const override { return m_reader.GetColumnCount(m_stage); }
        virtual IECSqlValue const& GetValue(int columnIndex) const override { return m_reader.GetValue(m_stage, columnIndex); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE