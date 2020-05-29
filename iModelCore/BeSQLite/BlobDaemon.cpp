/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <BeSQLite/BlobDaemon.h>
#include "SQLite/sqlite3.h"
#include "SQLite/bcvutil.h"
#include "SQLite/blockcachevfs.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::BlobStore(StorageType type, Utf8StringCR accountName, Utf8StringCR sasKey, Utf8StringCR containerName) {
    sqlite3_bcv_register(0);
    m_handle = nullptr;
    sqlite3_bcv_open(type, accountName.c_str(), sasKey.c_str(), containerName.c_str(), &m_handle);
    sqlite3_bcv_config(m_handle, SQLITE_BCVCONFIG_PROGRESS, this, ProgressCallback);
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::~BlobStore() {
    if (m_handle) {
        sqlite3_bcv_close(m_handle);
        m_handle = nullptr;
    }
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobStore::CreateContainer(int nameSize, int blockSize) {
    auto stat = (DbResult)sqlite3_bcv_create(m_handle, nameSize, blockSize);
    return Result(stat, sqlite3_bcv_errmsg(m_handle));
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobStore::DestroyContainer() {
    auto stat = (DbResult)sqlite3_bcv_destroy(m_handle);
    return Result(stat, sqlite3_bcv_errmsg(m_handle));
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobStore::UploadDatabase(Utf8StringCR localFileName, Utf8StringCR dbName) {
    if (localFileName.empty() || dbName.empty())
        return Result(BE_SQLITE_ERROR, "bad argument");
    auto stat = (DbResult)sqlite3_bcv_upload(m_handle, localFileName.c_str(), dbName.c_str());
    return Result(stat, sqlite3_bcv_errmsg(m_handle));
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobStore::DownloadDatabase(Utf8StringCR dbName, Utf8StringCR localName) {
    if (localName.empty() || dbName.empty())
        return Result(BE_SQLITE_ERROR, "bad argument");
    auto stat = (DbResult)sqlite3_bcv_download(m_handle, dbName.c_str(), localName.c_str());
    return Result(stat, sqlite3_bcv_errmsg(m_handle));
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobStore::CopyDatabase(Utf8StringCR dbFrom, Utf8StringCR dbTo) {
    if (dbFrom.empty() || dbTo.empty())
        return Result(BE_SQLITE_ERROR, "bad argument");
    auto stat = (DbResult)sqlite3_bcv_copy(m_handle, dbFrom.c_str(), dbTo.c_str());
    return Result(stat, sqlite3_bcv_errmsg(m_handle));
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobStore::DeleteDatabase(Utf8StringCR dbName) {
    if (dbName.empty())
        return Result(BE_SQLITE_ERROR, "bad argument");
    auto stat = (DbResult)sqlite3_bcv_delete(m_handle, dbName.c_str());
    return Result(stat, sqlite3_bcv_errmsg(m_handle));
}

struct SQLiteMsg {
    Utf8P m_msg = nullptr;
    ~SQLiteMsg() {
        if (m_msg) {
            sqlite3_free(m_msg);
            m_msg = nullptr;
        }
    }
};

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobDaemon::AttachContainer(Utf8StringCR daemonDir, Utf8StringCR container, Utf8StringCR sasKey, bool readonly) {
    sqlite3_bcv_register(0);
    SQLiteMsg errMsg;
    auto stat = (DbResult)sqlite3_bcv_attach(daemonDir.c_str(), container.c_str(), sasKey.c_str(), SQLITE_BCVATTACH_POLL | (readonly ? SQLITE_BCVATTACH_READONLY : 0), &errMsg.m_msg);
    return BlobStore::Result(stat, errMsg.m_msg);
}

/*---------------------------------------------------------------------------------**/ /**
 @bsimethod                                    Keith.Bentley                    05/20
+---------------+---------------+---------------+---------------+---------------+------*/
BlobStore::Result BlobDaemon::DetachContainer(Utf8StringCR daemonDir, Utf8StringCR container) {
    sqlite3_bcv_register(0);
    SQLiteMsg errMsg;
    auto stat = (DbResult)sqlite3_bcv_detach(daemonDir.c_str(), container.c_str(), &errMsg.m_msg);
    return BlobStore::Result(stat, errMsg.m_msg);
}
