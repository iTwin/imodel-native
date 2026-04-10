/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECChangesetReader provides EC-typed value access for iterating over changesets.
//! It follows the same PIMPL pattern as ECSqlStatement.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECChangesetReader {
public:
    struct Impl;

    //! Controls which properties are emitted per row.
    enum class Mode {
        All_Properties = 0, //!< Emit ECInstanceId, ECClassId, and all user/domain properties.
        Bis_Element_Properties = 1, //!< Emit ECInstanceId, ECClassId, and BIS-schema properties only.
        Instance_Key = 2,   //!< Emit ECInstanceId and ECClassId only (cheapest).
    };

private:
    using Stage = Changes::Change::Stage;
    Impl* m_pimpl = nullptr;

    ECChangesetReader(ECChangesetReader const&) = delete;
    ECChangesetReader& operator=(ECChangesetReader const&) = delete;
    ECChangesetReader(ECChangesetReader&& rhs) = delete;
    ECChangesetReader& operator=(ECChangesetReader&& rhs) = delete;

public:
    //! Constructs a new ECChangesetReader instance. Call one of the Open methods before using it.
    ECDB_EXPORT ECChangesetReader();
    //! Destructor. Closes the reader if it is still open.
    ECDB_EXPORT ~ECChangesetReader();

    //! Opens a changeset file for reading.
    //! @param[in] ecdb ECDb connection used to resolve EC schema information.
    //! @param[in] changesetFile Path to the changeset file to open.
    //! @param[in] invert If true, the changeset is read as an inverted (undo) changeset.
    //! @param[in] mode Controls which properties are emitted per row. @see Mode
    //! @return BE_SQLITE_OK on success, or an error code if the file cannot be opened.
    ECDB_EXPORT DbResult OpenFile(ECDbCR ecdb, Utf8StringCR changesetFile, bool invert, Mode mode);

    //! Opens a ChangeStream for reading.
    //! @param[in] ecdb ECDb connection used to resolve EC schema information.
    //! @param[in] changeStream The change stream to read from. Ownership is transferred.
    //! @param[in] invert If true, the changeset is read as an inverted (undo) changeset.
    //! @param[in] mode Controls which properties are emitted per row. @see Mode
    //! @return BE_SQLITE_OK on success, or an error code if the stream could not be opened.
    ECDB_EXPORT DbResult OpenChangeStream(ECDbCR ecdb, std::unique_ptr<ChangeStream> changeStream, bool invert, Mode mode);

    //! Opens a group of changeset files for reading as a single logical sequence.
    //! @param[in] ecdb ECDb connection used to resolve EC schema information.
    //! @param[in] changesetFiles Ordered list of paths to the changeset files to open.
    //! @param[in] invert If true, the changesets are read as inverted (undo) changesets, iterated in reverse order.
    //! @param[in] mode Controls which properties are emitted per row. @see Mode
    //! @return BE_SQLITE_OK on success, or an error code if the group could not be opened.
    ECDB_EXPORT DbResult OpenGroup(ECDbCR ecdb, T_Utf8StringVector const& changesetFiles, bool invert, Mode mode);

    //! Closes the reader and releases all associated resources.
    //! @remarks Safe to call on a reader that was never opened or that has already been closed.
    ECDB_EXPORT void Close();

    //! Advances the reader to the next change row.
    //! @return BE_SQLITE_ROW if a new row is available, BE_SQLITE_DONE when there are no more rows,
    //!         or an error code on failure.
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
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not positioned on a valid row.
    ECDB_EXPORT DbResult GetTableName(Utf8StringR tableName) const;

    //! Gets the DML opcode (Insert, Update, or Delete) of the current change row.
    //! @param[out] opcode Receives the opcode for the current row.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not positioned on a valid row.
    ECDB_EXPORT DbResult GetOpcode(DbOpcode& opcode) const;

    //! Gets the EC-typed value of a specific column for the given change stage.
    //! @param[in] stage The change stage (Old/New) to query. @see Changes::Change::Stage
    //! @param[in] columnIndex Zero-based index of the column to retrieve.
    //! @return The EC-typed value. Returns a no-op singleton value if the reader is closed, not stepped, or the index is out of range.
    ECDB_EXPORT IECSqlValue const& GetValue(Stage stage, int columnIndex) const;

    //! Gets a JSON string representing the ECInstanceId and ECClassId (the instance key) for the current change row.
    //! @param[in] stage The change stage (Old/New) to query. @see Changes::Change::Stage
    //! @param[out] key Receives the JSON-encoded instance key string.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not positioned on a valid row.
    ECDB_EXPORT DbResult GetInstanceKey(Stage stage, Utf8StringR key) const;

    //! Determines whether the current change row belongs to an EC table.
    //! @param[out] isECTable Set to true if the current table is an EC table, false otherwise.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not positioned on a valid row.
    ECDB_EXPORT DbResult IsECTable(bool& isECTable) const;

    //! Gets the names of properties that were fetched from the current row of change.
    //! @remarks For compound data properties like point2d, point3d or navigation properties,
    //! the full name of the property is returned in case all the components of the property are fetched from the change.
    //! If all of the components are not fetched from the change(meaning they did not change), 
    //! then the individual component names which changed are returned smartly by using `.` as a separator (e.g. "MyPoint.X", "MyPoint.Y" for a point3d property "MyPoint" if only X and Y changed). 
    //! For struct properties the property names are always returned in the "StructProp.MemberName" format.
    //! So if only X changed for a point2d property named "Myp2d" inide a struct "CustomStruct", the returned property name will be "CustomStruct.Myp2d.X".
    //! Similaly if both X and Y changed for the same point2d property, the returned property name will be "CustomStruct.Myp2d".
    //! @param[out] out Receives the list of property names.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not positioned on a valid row.
    ECDB_EXPORT DbResult GetChangeFetchedPropertyNames(std::vector<Utf8String>& out) const;

    //! Determines whether the current change row is an indirect change.
    //! @param[out] isIndirect Set to true if the current change is indirect, false otherwise.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not positioned on a valid row.
    ECDB_EXPORT DbResult IsIndirectChange(bool& isIndirect) const;

    //! Sets a list of SQLite table-name filters. Only rows whose table name matches one of the entries will be returned by Step().
    //! @remarks Filters are applied on the next call to Step(). Pass an empty vector to match all tables.
    //! @param[in] tableFilters List of table names to include.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not open.
    ECDB_EXPORT DbResult SetTableFilters(std::vector<Utf8String> const& tableFilters);

    //! Sets a list of opcode filters. Only rows whose opcode matches one of the entries will be returned by Step().
    //! @remarks Filters are applied on the next call to Step(). Pass an empty vector to match all opcodes.
    //! @param[in] opcodeFilters List of DbOpcode values (Insert, Update, Delete) to include.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not open.
    ECDB_EXPORT DbResult SetOpcodeFilters(std::vector<DbOpcode> const& opcodeFilters);

    //! Sets a list of ECClassId filters. Only rows whose ECClassId matches one of the entries will be returned by Step().
    //! @remarks Filters are applied on the next call to Step(). Pass an empty vector to match all ECClasses.
    //! @param[in] ecclassIdFilters List of ECClassIds to include.
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not open.
    ECDB_EXPORT DbResult SetECClassIdFilters(std::vector<ECN::ECClassId> const& ecclassIdFilters);

    //! Clears all table-name filters previously set with SetTableFilters().
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not open.
    ECDB_EXPORT DbResult ClearTableFilters();

    //! Clears all opcode filters previously set with SetOpcodeFilters().
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not open.
    ECDB_EXPORT DbResult ClearOpcodeFilters();

    //! Clears all ECClassId filters previously set with SetECClassIdFilters().
    //! @return BE_SQLITE_OK on success, or an error code if the reader is not open.
    ECDB_EXPORT DbResult ClearECClassIdFilters();

};


//=======================================================================================
//! @bsiclass
//=======================================================================================
struct ECChangesetRow : public IECSqlRow {
    private:
        ECChangesetReader const& m_reader;
        Changes::Change::Stage m_stage;       
    public:
        ECChangesetRow(ECChangesetReader const& reader, Changes::Change::Stage const& stage) : m_reader(reader), m_stage(stage) {}
        virtual int GetColumnCount() const override { return m_reader.GetColumnCount(m_stage); }
        virtual IECSqlValue const& GetValue(int columnIndex) const override { return m_reader.GetValue(m_stage, columnIndex); }
};

END_BENTLEY_SQLITE_EC_NAMESPACE