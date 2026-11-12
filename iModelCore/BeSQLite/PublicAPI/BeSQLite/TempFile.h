/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "BeSQLite.h"

typedef struct sqlite3_file* SqlFileP;

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! A scratch file of raw bytes, created through SQLite's default VFS the same way SQLite creates
//! its own temporary files: with a unique name in SQLite's temp directory (see BeSQLiteLib::Initialize),
//! deleted by the operating system even if the process crashes or is killed. On Unix the file is
//! unlinked as soon as it is opened; on Windows it is opened with delete-on-close.
//! @note Not thread-safe.
// @bsiclass
//=======================================================================================
struct TempFile final {
private:
    SqlFileP m_file = nullptr;

public:
    TempFile() {}
    ~TempFile() { Close(); }
    TempFile(TempFile const&) = delete;
    TempFile& operator=(TempFile const&) = delete;

    //! Creates the file. Returns BE_SQLITE_OK, BE_SQLITE_MISUSE if already open, or the error of the VFS.
    BE_SQLITE_EXPORT DbResult Open();
    bool IsOpen() const { return m_file != nullptr; }

    //! Writes @p size bytes at @p offset, extending the file as needed.
    BE_SQLITE_EXPORT DbResult Write(void const* data, size_t size, uint64_t offset);

    //! Reads exactly @p size bytes at @p offset. Returns BE_SQLITE_IOERR_SHORT_READ if the file ends first.
    BE_SQLITE_EXPORT DbResult Read(void* data, size_t size, uint64_t offset);

    //! Closes the file, which deletes it. Idempotent.
    BE_SQLITE_EXPORT void Close();
};

END_BENTLEY_SQLITE_NAMESPACE
