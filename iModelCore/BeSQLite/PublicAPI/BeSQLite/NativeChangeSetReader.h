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
// @bsiclass
//=======================================================================================
struct NativeChangeSetReader
    {
public:
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
    Utf8String m_lastError;

private:
    //! Common guard + value-fetch logic shared by all column-value accessors.
    //! Returns DbValue(nullptr) when access is invalid, otherwise the old/new column value.
    DbValue GetColValue(int col, int target) const
        {
        if (!HasRow() || !(col >= 0 && col < m_columnCount) || (target != 0 && target != 1))
            return DbValue(nullptr);
        if (target == 0 && m_opcode == DbOpcode::Insert)
            return DbValue(nullptr);
        if (target != 0 && m_opcode == DbOpcode::Delete)
            return DbValue(nullptr);
        return target == 0 ? m_currentChange.GetOldValue(col) : m_currentChange.GetNewValue(col);
        }

public:
    bool HasRow() const { return IsOpen() && m_currentChange.IsValid(); }
    bool IsOpen() const { return m_changeStream != nullptr; }
    bool IsValidColumnIndex(int col) const { return HasRow() && (col >= 0 && col < m_columnCount); }
    bool IsValidPrimaryKeyColumnIndex(int col) const { return HasRow() && (col >= 0 && col < m_primaryKeyColumnCount); }

    NativeChangeSetReader() : m_invert(false), m_primaryKeyColumns(nullptr), m_currentChange(nullptr, false),
        m_opcode(DbOpcode::Insert), m_columnCount(0), m_indirect(0), m_primaryKeyColumnCount(0),
        m_primaryKeyCount(0), m_tableName(nullptr) {}

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

    Utf8CP         GetTableName()      const { return m_tableName; }
    DbOpcode       GetOpCode()         const { return m_opcode; }
    bool           IsIndirectChange()  const { return m_indirect != 0; }
    int            GetColumnCount()    const { return m_columnCount; }
    bool           GetHasRow()         const { return HasRow(); }
    Utf8StringCR   GetDdlChanges()     const { return m_ddl; }

    // --- Column value accessors ---
    // All return DbValue(nullptr) when the access is invalid (wrong opcode for target,
    // out-of-range column, or no current row). Otherwise returns the raw sqlite value.

    DbValue GetColumnValue(int col, int target)        const { return GetColValue(col, target); }
    DbValue GetColumnValueType(int col, int target)    const { return GetColValue(col, target); }
    DbValue GetColumnValueId(int col, int target)      const { return GetColValue(col, target); }
    DbValue GetColumnValueInteger(int col, int target) const { return GetColValue(col, target); }
    DbValue GetColumnValueDouble(int col, int target)  const { return GetColValue(col, target); }
    DbValue GetColumnValueText(int col, int target)    const { return GetColValue(col, target); }
    DbValue GetColumnValueBinary(int col, int target)  const { return GetColValue(col, target); }
    DbValue IsColumnValueNull(int col, int target)     const { return GetColValue(col, target); }
    };

END_BENTLEY_SQLITE_NAMESPACE
