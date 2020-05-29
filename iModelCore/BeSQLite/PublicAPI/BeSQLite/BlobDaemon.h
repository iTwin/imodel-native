/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BeSQLite.h"

typedef struct sqlite3_bcv* BCVHandle;

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Keith.Bentley  05/20
//=======================================================================================
struct BlobStore {
private:
    BCVHandle m_handle;

public:
    enum StorageType {
        AZURE = 1,
        AZURE_SAS = 2,
        AZURE_EMU = 3,
        AZURE_EMU_SAS = 4,
        GOOGLE = 5,
    };

    struct Result {
        DbResult m_status;
        Utf8String m_error;
        Result(DbResult status = BE_SQLITE_OK, Utf8CP msg = "") {
            m_status = status;
            m_error = msg;
        }
    };

    static int ProgressCallback(void* daemon, uint64_t nDone, uint64_t nTotal) { return (int)((BlobStore*)daemon)->_OnProgress(nDone, nTotal); }
    virtual int _OnProgress(uint64_t nDone, uint64_t nTotal) { return 0; }

    BE_SQLITE_EXPORT BlobStore(StorageType type, Utf8StringCR accountName, Utf8StringCR sasKey, Utf8StringCR containerName);
    BE_SQLITE_EXPORT ~BlobStore();

    BE_SQLITE_EXPORT Result CreateContainer(int nameSize = 0, int blockSize = 0);
    BE_SQLITE_EXPORT Result DestroyContainer();

    BE_SQLITE_EXPORT Result UploadDatabase(Utf8StringCR localFileName, Utf8StringCR dbName);
    BE_SQLITE_EXPORT Result DownloadDatabase(Utf8StringCR dbName, Utf8StringCR localName);
    BE_SQLITE_EXPORT Result CopyDatabase(Utf8StringCR dbFrom, Utf8StringCR dbTo);
    BE_SQLITE_EXPORT Result DeleteDatabase(Utf8StringCR dbName);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley  05/20
//=======================================================================================
struct BlobDaemon {
    BE_SQLITE_EXPORT static BlobStore::Result AttachContainer(Utf8StringCR daemonDir, Utf8StringCR container, Utf8StringCR sasKey, bool readonly);
    BE_SQLITE_EXPORT static BlobStore::Result DetachContainer(Utf8StringCR daemonDir, Utf8StringCR container);
};

END_BENTLEY_SQLITE_NAMESPACE
