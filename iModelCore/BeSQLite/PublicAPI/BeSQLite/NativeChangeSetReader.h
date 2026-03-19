/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BeSQLite.h"
#include "ChangeSet.h"
#include <memory>

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! A reader for changeset data. Holds all changeset state and implements the core read
//! logic. Returns raw C++ types (DbValue for column data, DbResult for operations).
//! Has no dependency on Napi — all JS conversion is left to the caller.
//! @bsiclass
//=======================================================================================
struct NativeChangeSetReader
    {
private:
    bool m_invert;
    Byte* m_primaryKeyColumns;
    Changes::Change m_currentChange;
    DbOpcode m_opcode;
    int m_columnCount;
    int m_indirect;
    int m_primaryKeyColumnCount;
    int m_primaryKeyCount;
    std::unique_ptr<Changes> m_changes;
    std::unique_ptr<ChangeStream> m_changeStream;
    std::unique_ptr<ChangeGroup> m_changeGroup;
    Utf8CP m_tableName;
    Utf8String m_ddl;
    //! Populated with a human-readable description when a method returns an error DbResult.
    mutable Utf8String m_lastError;

    bool IsValidColumnIndex(int col) const { return HasRow() && (col >= 0 && col < m_columnCount); }
    bool IsOpCodeAndTargetMatch(DbOpcode opcode, int target) const;
    bool IsOpen() const { return m_changeStream != nullptr; }
public:
    NativeChangeSetReader() : m_primaryKeyColumns(nullptr), m_tableName(nullptr), m_currentChange(nullptr, false), m_invert(false) {}

    //! Open a changeset from a file on disk.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult OpenFile(Utf8StringCR changesetFile, bool invert);

    //! Open from an already-constructed ChangeStream.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult OpenChangeStream(std::unique_ptr<ChangeStream> changeStream, bool invert);

    //! Merge multiple changeset files into a single group and open it for reading.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult OpenGroup(bvector<Utf8String> const& changesetFiles, Db const& db, bool invert);

    //! Close the reader and release all resources.
    BE_SQLITE_EXPORT void Close();

    //! Rewind the iteration back to before the first row (keeps the stream open).
    BE_SQLITE_EXPORT void Reset();

    //! Write the currently open changeset to a file.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult WriteToFile(Utf8String const& fileName, bool containChanges, bool shouldOverride);

    //! Advance to the next change row.
    //! @return BE_SQLITE_ROW if a row is available, BE_SQLITE_DONE when exhausted,
    //!         or another error code; sets m_lastError on error.
    BE_SQLITE_EXPORT DbResult Step();

    // --- Accessors (only valid after a successful Step() returning BE_SQLITE_ROW) ---

    //! Get the name of the table affected by the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetTableName(Utf8StringR out) const;
    //! Get the opcode (Insert/Update/Delete) of the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetOpCode(DbOpcode& out) const;
    //! Returns true if the current change row is an indirect (cascaded) change.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult IsIndirectChange(bool& out) const;
    //! Get the number of columns in the table affected by the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetColumnCount(int& out) const;
    //! Get the accumulated DDL (schema) changes recorded in the changeset.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetDdlChanges(Utf8StringR out) const;
    //! Get a pointer to the primary-key column bitmap for the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetPrimaryKeyColumns(Byte* out) const;
    //! Returns the last error message set by any method.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT Utf8StringCR GetLastError() const { return m_lastError; }
    //! Returns true if there is a current change row available.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT bool HasRow() const { return IsOpen() && m_currentChange.IsValid(); }


    // --- Column value accessors ---
    //! Get the old (target==0) or new (target==1) value for a column in the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetColumnValue(int col, int target, DbValue& out) const;
    //! Get all old (target==0) or new (target==1) column values for the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetRow(int target, std::vector<DbValue>& out) const;
    //! Get the primary-key column values for the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetPrimaryKeys(std::vector<DbValue>& out) const;
    //! Get the 0-based column indexes that form the primary key for the current change row.
    //! @return BE_SQLITE_OK on success; sets m_lastError and returns an error code on failure.
    BE_SQLITE_EXPORT DbResult GetPrimaryKeyColumnIndexes(std::vector<uint64_t>& out) const;
    };

END_BENTLEY_SQLITE_NAMESPACE
