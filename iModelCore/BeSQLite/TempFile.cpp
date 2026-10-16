/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/TempFile.h>
#include "SQLite/sqlite3.h"
#include <algorithm>
#include <cstring>

BEGIN_BENTLEY_SQLITE_NAMESPACE

// SQLite only asks a VFS for page-sized transfers (64 KiB at most), and the Unix VFS handles at most 128 KiB per call,
// so larger requests are split.
static constexpr size_t s_maxChunkBytes = 64 * 1024;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult TempFile::Open() {
    if (m_file != nullptr)
        return BE_SQLITE_MISUSE;
    sqlite3_vfs* vfs = sqlite3_vfs_find(nullptr);
    if (vfs == nullptr)
        return BE_SQLITE_ERROR;
    auto file = (sqlite3_file*) sqlite3_malloc(vfs->szOsFile);
    if (file == nullptr)
        return BE_SQLITE_NOMEM;
    memset(file, 0, vfs->szOsFile);

    // Same flags SQLite uses for its own temp files: a null name makes the VFS choose a unique name in its temp directory.
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_EXCLUSIVE | SQLITE_OPEN_DELETEONCLOSE | SQLITE_OPEN_TEMP_JOURNAL;
    const int rc = vfs->xOpen(vfs, nullptr, file, flags, nullptr);
    if (rc != SQLITE_OK) {
        if (file->pMethods != nullptr)
            file->pMethods->xClose(file);
        sqlite3_free(file);
        return (DbResult) rc;
    }
    m_file = file;
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult TempFile::Write(void const* data, size_t size, uint64_t offset) {
    if (m_file == nullptr)
        return BE_SQLITE_MISUSE;
    auto bytes = static_cast<Byte const*>(data);
    for (size_t done = 0; done < size;) {
        const size_t chunk = std::min(size - done, s_maxChunkBytes);
        const int rc = m_file->pMethods->xWrite(m_file, bytes + done, (int) chunk, (sqlite3_int64) (offset + done));
        if (rc != SQLITE_OK)
            return (DbResult) rc;
        done += chunk;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DbResult TempFile::Read(void* data, size_t size, uint64_t offset) {
    if (m_file == nullptr)
        return BE_SQLITE_MISUSE;
    auto bytes = static_cast<Byte*>(data);
    for (size_t done = 0; done < size;) {
        const size_t chunk = std::min(size - done, s_maxChunkBytes);
        const int rc = m_file->pMethods->xRead(m_file, bytes + done, (int) chunk, (sqlite3_int64) (offset + done));
        if (rc != SQLITE_OK)
            return (DbResult) rc;
        done += chunk;
    }
    return BE_SQLITE_OK;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void TempFile::Close() {
    if (m_file == nullptr)
        return;
    if (m_file->pMethods != nullptr)
        m_file->pMethods->xClose(m_file);
    sqlite3_free(m_file);
    m_file = nullptr;
}

END_BENTLEY_SQLITE_NAMESPACE
