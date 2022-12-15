/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "BeSQLite.h"
#include <Bentley/BeEvent.h>

// cspell:ignore bcvfs

typedef struct sqlite3_bcv* BCVHandle;
typedef struct sqlite3_bcvfs* CloudVfsP;
typedef struct sqlite3_prefetch* PrefetchP;

BEGIN_BENTLEY_SQLITE_NAMESPACE

struct CloudContainer;

/**
 * the result status and error message for a CloudSqlite operation.
 */
struct CloudResult {
    int m_status;
    Utf8String m_error;
    CloudResult(int status = BE_SQLITE_OK, Utf8CP msg = "") {
        m_status = status;
        m_error = msg;
    }
    bool IsSuccess() { return m_status == BE_SQLITE_OK; }
};

/**
 * For accessing the virtual tables in CloudSqlite without a physical database
 */
struct CloudVdb : Db {
    DbResult Open(Utf8StringCR alias, Utf8StringCR vfsName);
};

/**
 * Object to control access to a local cache for cloud-based SQLite databases.
 * This object holds the sqlite "bcvfs" object and stores a list of the currently connected CloudContainers.
 * It provides both the token lookup callback and the logging callback for SQLite.
 */
struct CloudCache {
    CloudVfsP m_vfs = nullptr;
    BeEvent<CloudCache*> m_onDestroy;
    CloudVdb m_cloudDb;
    Utf8String m_guid;
    Utf8String m_rootDir;
    Utf8String m_name;
    std::vector<CloudContainer*> m_containers;

    BE_SQLITE_EXPORT CloudResult InitCache(Utf8StringCR name, Utf8StringCR rootDir, int64_t cacheSize = 0, int64_t nRequest = 0, int64_t httpTimeout = 0, bool verboseCurlLog = false);
    BE_SQLITE_EXPORT void SetLogMask(int logMask);
    BE_SQLITE_EXPORT Utf8String GetDatabaseHash(CloudContainer& container, Utf8StringCR dbName);
    BE_SQLITE_EXPORT int GetNumCleanupBlocks(CloudContainer& container);
    BE_SQLITE_EXPORT bool IsDaemon();
    BE_SQLITE_EXPORT bool IsAttached(CloudContainer& container);
    BE_SQLITE_EXPORT void Destroy();
    BE_SQLITE_EXPORT int GetBlockSize();
    BE_SQLITE_EXPORT Utf8String GetCacheGuidString();

    virtual ~CloudCache() { Destroy(); }

    CloudResult CallSqliteFn(std::function<int(Utf8P*)>, Utf8CP fnName);
    int FindToken(Utf8CP storage, Utf8CP accountName, Utf8CP containerName, Utf8P* token);
    CloudContainer* FindMatching(Utf8CP accountName, Utf8CP containerName);
    Utf8String GetGuidFileName();
    void ReadGuid();

    /** This method may be overridden to supply logging for CloudSqlite operations */
    virtual void Log(NativeLogging::SEVERITY, Utf8CP message) {}
};

/**
 * A cloud-based container of SQLite databases.
 */
struct CloudContainer {
    CloudCache* m_cache = nullptr;
    BeEvent<CloudContainer*> m_onDisconnect;
    CloudVdb m_containerDb;
    Utf8String m_storageType;
    Utf8String m_accessName;
    Utf8String m_containerId;
    Utf8String m_alias;
    Utf8String m_accessToken;
    bool m_writeLockHeld = false;
    bool m_writeable = false;
    bool m_secure = false;

    CloudContainer() {}
    ~CloudContainer() { Disconnect(false); }
    CloudContainer(Utf8StringCR storageType, Utf8StringCR accessName, Utf8StringCR containerId, Utf8StringCR alias, Utf8StringCR accessToken) :
        m_storageType(storageType), m_accessName(accessName), m_containerId(containerId), m_alias(alias), m_accessToken(accessToken) {}

    CloudResult CallSqliteFn(std::function<int(Utf8P*)> fn, Utf8CP fnName) { return m_cache->CallSqliteFn(fn, fnName); }
    bool IsContainerConnected() const { return nullptr != m_cache; }

    void CloseDbIfOpen() {
        if (m_containerDb.IsDbOpen())
            m_containerDb.CloseDb();
    }

    BE_SQLITE_EXPORT CloudResult Connect(CloudCache&);
    BE_SQLITE_EXPORT CloudResult Disconnect(bool fromCacheDtor);
    BE_SQLITE_EXPORT CloudResult PollManifest();
    BE_SQLITE_EXPORT CloudResult UploadChanges();
    BE_SQLITE_EXPORT CloudResult RevertChanges();
    BE_SQLITE_EXPORT CloudResult CopyDatabase(Utf8StringCR dbFrom, Utf8StringCR dbTo);
    BE_SQLITE_EXPORT CloudResult DeleteDatabase(Utf8StringCR dbName);
    BE_SQLITE_EXPORT CloudResult Detach();
};

/**
 * Object to perform prefetching blocks of a cloud database
 */
struct CloudPrefetch {
    struct PrefetchStatus {
        int m_status = BE_SQLITE_DONE;
        Utf8String m_error;
        int64_t m_nOutstanding = 0;
        int64_t m_nDemand = 0;
    };
    PrefetchP m_prefetch = nullptr;

    ~CloudPrefetch() { Stop(); }
    bool IsRunning() { return m_prefetch!= nullptr; }

    BE_SQLITE_EXPORT DbResult Init(CloudContainer& container, Utf8StringCR dbName);
    BE_SQLITE_EXPORT void Stop();
    BE_SQLITE_EXPORT PrefetchStatus Run(int nRequests, int timeout);
};

/**
 * Utilities for initializing CloudContainers, as well as to upload/download/copy/delete databases within CloudContainers
 */
struct CloudUtil {
private:
    BCVHandle m_handle = nullptr;

public:
    static int ProgressCallback(void* util, uint64_t nDone, uint64_t nTotal) { return (int)((CloudUtil*)util)->_OnProgress(nDone, nTotal); }
    static void LogCallback(void* util, Utf8CP msg) { ((CloudUtil*)util)->_Log(msg); }
    virtual int _OnProgress(uint64_t nDone, uint64_t nTotal) { return 0; }
    virtual void _Log(Utf8CP msg) {}

    virtual ~CloudUtil() { Close(); }
    BE_SQLITE_EXPORT void Close();
    BE_SQLITE_EXPORT CloudResult Init(CloudContainer const& container, int logLevel = 0, int nRequests = 0, int httpTimeout = 0);
    BE_SQLITE_EXPORT CloudResult InitializeContainer(int nameSize = 0, int blockSize = 0);
    BE_SQLITE_EXPORT CloudResult CleanDeletedBlocks(int deleteTime = 0);
    BE_SQLITE_EXPORT CloudResult UploadDatabase(Utf8StringCR localFileName, Utf8StringCR dbName);
    BE_SQLITE_EXPORT CloudResult DownloadDatabase(Utf8StringCR dbName, Utf8StringCR localName);
};

END_BENTLEY_SQLITE_NAMESPACE
