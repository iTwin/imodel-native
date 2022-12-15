/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "SQLite/bcvutil.h"
#include "SQLite/blockcachevfs.h"
#include "SQLite/sqlite3.h"
#include <BeSQLite/CloudSqlite.h>
#include <Bentley/SHA1.h>

// cspell:ignore bcvfs itwindb nrequest isdaemon ncleanup ifnot bcvconfig blockno npin

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE

// default for "nRequests" to SQLite apis (number of simultaneous HTTP connections). 6 is the maximum from Chrome.
static const int DEFAULT_MAX_HTTP_CONNECTIONS = 6;
static const int DEFAULT_HTTP_TIMEOUT = 60; // 60 seconds 

Utf8String Db::OpenParams::SetFromContainer(Utf8CP dbName, CloudContainerP container) {
    if (nullptr == container)
        return dbName;

    m_skipFileCheck = true;
    AddQueryParam(Utf8PrintfString("vfs=%s", container->m_cache->m_name.c_str()).c_str());
    return Utf8String("/") + container->m_alias + "/" + dbName;
}

/**
 * The SQLite "bcvfs" api returns error messages in sqlite memory that must be freed by the caller. This class
 * ensures that it is freed even if exceptions are thrown.
 */
struct SQLiteMsg {
    Utf8P m_msg = nullptr;
    ~SQLiteMsg() {
        if (m_msg) {
            sqlite3_free(m_msg);
        }
    }
};

DbResult CloudVdb::Open(Utf8StringCR alias, Utf8StringCR vfsName) {
    auto params = Db::OpenParams(Db::OpenMode::ReadWrite);
    params.m_rawSQLite = true;
    params.m_skipFileCheck = true;
    params.m_startDefaultTxn = DefaultTxn::No;
    Utf8String vfsParam = "vfs=" + vfsName;
    params.AddQueryParam(vfsParam.c_str());

    Utf8String name = "/" + alias;
    auto rc =  OpenBeSQLiteDb(name.c_str(), params);
    return BE_SQLITE_OK == rc ? (DbResult) sqlite3_bcvfs_register_vtab(GetDbFile()->GetSqlDb()) : rc;
}

/**
 * Read the guid of this cache from the localstore db. This is to provide a unique identifier for this cache for acquiring write locks.
 * It is persistent so a write lock may be recovered across sessions. If it does not exist, create one.
 */
void CloudCache::ReadGuid() {
    BeAssert(!IsDaemon());

    BeFileName guidFile(m_rootDir);
    guidFile.AppendToPath(L"localstore.itwindb");
    Db localStoreDb;

    Db::OpenParams openParams(Db::OpenMode::Readonly, DefaultTxn::No);
    openParams.SetImmutable();
    openParams.m_rawSQLite = true;

    auto stat = localStoreDb.OpenBeSQLiteDb(guidFile, openParams);
    if (BE_SQLITE_OK == stat) {
        Statement stmt;
        stat = stmt.Prepare(localStoreDb, "SELECT val FROM kv WHERE key='guid'");
        BeAssert(stat == BE_SQLITE_OK);
        if (BE_SQLITE_ROW == stmt.Step()) {
            m_guid = stmt.GetValueText(0);
            return; // closes localStoreDb
        }
    }
    localStoreDb.CloseDb();
    m_guid = BeGuid(true).ToString();
    Db::CreateParams params;
    params.m_failIfDbExists = false;
    params.m_rawSQLite = true;
    params.m_startDefaultTxn = DefaultTxn::No;

    if (BE_SQLITE_OK != localStoreDb.CreateNewDb(guidFile, params))
        return;

    localStoreDb.TryExecuteSql("CREATE TABLE kv(key TEXT PRIMARY KEY, val TEXT)");
    Statement stmt;
    stat = stmt.Prepare(localStoreDb, "INSERT INTO kv(key,val) VALUES('guid',?)");
    BeAssert(stat == BE_SQLITE_OK);
    stmt.BindText(1, m_guid, Statement::MakeCopy::No);
    stat = stmt.Step();
    BeAssert(stat == BE_SQLITE_DONE);
    stmt.Finalize();
    localStoreDb.TryExecuteSql("COMMIT");
    localStoreDb.CloseDb();
}

/**
 * construct a new SqliteCloud vfs, and establish the cache directory for its containers.
 * @param name the name of this vfs
 * @param rootDir the directory to use for caching cloud containers
 */
CloudResult CloudCache::InitCache(Utf8StringCR name, Utf8StringCR rootDir, int64_t cacheSize, int64_t nRequest, int64_t httpTimeout, bool curlDiagnostics) {
    m_name = name;
    m_rootDir = rootDir;
    auto stat = CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_create(m_rootDir.c_str(), m_name.c_str(), &m_vfs, msg); }, "create CloudCache");
    if (!stat.IsSuccess())
        return stat;

    sqlite3_bcvfs_auth_callback(m_vfs, this, [](void* ctx, Utf8CP storageType, Utf8CP account, Utf8CP container, Utf8P* token) {
        return ((CloudCache*)ctx)->FindToken(storageType, account, container, token);
    });

    if (!IsDaemon()) { // config and write lock only applies to daemonless mode.
        if (curlDiagnostics)
            sqlite3_bcvfs_config(m_vfs, SQLITE_BCV_CURLVERBOSE, 1);

        if (0 != cacheSize)
            sqlite3_bcvfs_config(m_vfs, SQLITE_BCV_CACHESIZE, cacheSize);
        if (nRequest <= 0)
            nRequest = DEFAULT_MAX_HTTP_CONNECTIONS;
        sqlite3_bcvfs_config(m_vfs, SQLITE_BCV_NREQUEST, nRequest);
        if (httpTimeout <= 0)
            httpTimeout = DEFAULT_HTTP_TIMEOUT;
        sqlite3_bcvfs_config(m_vfs, SQLITE_BCV_HTTPTIMEOUT, httpTimeout);

        ReadGuid(); // sets the m_guid member to either a new GUID or one from the localstore for write locks
    }

    return CloudResult(m_cloudDb.Open("", name));
}

bool CloudCache::IsDaemon() {
    return m_vfs ? sqlite3_bcvfs_isdaemon(m_vfs) : false;
}

void CloudCache::SetLogMask(int logMask) {
    if (!IsDaemon())
        sqlite3_bcvfs_log_callback(m_vfs, this, logMask, [](void* ctx, int mLog, Utf8CP message) { ((CloudCache*)ctx)->Log(NativeLogging::SEVERITY::LOG_INFO, message); });
}

void CloudCache::Destroy() {
    m_onDestroy.RaiseEvent(this);

    if (m_cloudDb.IsDbOpen())
        m_cloudDb.CloseDb();

    for (auto entry : m_containers)
        entry->Disconnect(true);

    m_containers.clear();

    if (m_vfs) {
        sqlite3_bcvfs_destroy(m_vfs);
        m_vfs = nullptr;
    }
}

/** Get the block size for this CloudCache (only works after a container has been attached) */
int CloudCache::GetBlockSize() {
    Statement stmt;
    auto rc = stmt.Prepare(m_cloudDb, "PRAGMA  bcv_blocksize");
    BeAssert(rc == BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);
    stmt.Step();
    return stmt.GetValueInt(0);
}

/** for generating a digital signature for a database */
Utf8String CloudCache::GetDatabaseHash(CloudContainer& container, Utf8StringCR dbName) {
    Statement stmt;
    auto rc = stmt.Prepare(m_cloudDb, "SELECT blockid,blockno FROM bcv_block WHERE container=? AND database=? ORDER BY blockno");
    BeAssert(rc == BE_SQLITE_OK);
    UNUSED_VARIABLE(rc);
    stmt.BindText(1, container.m_alias, Statement::MakeCopy::No);
    stmt.BindText(2, dbName, Statement::MakeCopy::No);
    SHA1 hash;
    hash.Add(dbName.c_str(), dbName.size());
    int count=0;
    for (; BE_SQLITE_ROW == stmt.Step(); ++count)
        hash.Add(stmt.GetValueBlob(0), stmt.GetColumnBytes(0));
    return count > 0 ? hash.GetHashString() : "";
}

int CloudCache::GetNumCleanupBlocks(CloudContainer& container) {
    Statement stmt;
    Utf8String error;
    auto rc = stmt.Prepare(m_cloudDb, "SELECT ncleanup FROM bcv_container WHERE container=? AND user=?");
    if (rc == BE_SQLITE_OK) {
        stmt.BindText(1, container.m_containerId, Statement::MakeCopy::No);
        stmt.BindText(2, container.m_accessName, Statement::MakeCopy::No);
        rc = stmt.Step();
    }
    return BE_SQLITE_ROW == rc ? stmt.GetValueInt(0) : -1;
}

bool CloudCache::IsAttached(CloudContainer& container) {
    Statement stmt;
    Utf8String error;
    auto rc = stmt.Prepare(m_cloudDb, "SELECT 1 FROM bcv_container WHERE container=? AND user=?");
    if (rc == BE_SQLITE_OK) {
        stmt.BindText(1, container.m_containerId, Statement::MakeCopy::No);
        stmt.BindText(2, container.m_accessName, Statement::MakeCopy::No);
        rc = stmt.Step();
    }
    return BE_SQLITE_ROW == rc;
}

/**
 * Call one of the sqlite3_bcvfs_ functions. This method provides the "msg" argument that must be freed after the call.
 */
CloudResult CloudCache::CallSqliteFn(std::function<int(Utf8P*)> fn, Utf8CP funcName) {
    Log(NativeLogging::SEVERITY::LOG_TRACE, funcName);

    SQLiteMsg msg; // frees message in dtor
    auto stat = fn(&msg.m_msg);
    return CloudResult(stat, stat == 0 ? "" : Utf8PrintfString("%s error: %s", funcName, msg.m_msg ? msg.m_msg : Db::InterpretDbResult((DbResult)stat)).c_str());
}

CloudContainer* CloudCache::FindMatching(Utf8CP accessName, Utf8CP containerName) {
    for (auto entry : m_containers) {
        if (entry->m_containerId.Equals(containerName) && entry->m_accessName.Equals(accessName))
            return entry;
    }
    return nullptr;
}

/** callback to find the authorization token for one of the containers attached to this CloudCache */
int CloudCache::FindToken(Utf8CP storage, Utf8CP accessName, Utf8CP containerName, Utf8P* token) {
    auto container = FindMatching(accessName, containerName);
    if (nullptr == container)
        return SQLITE_AUTH;

    *token = sqlite3_mprintf("%s", container->m_accessToken.c_str());
    return SQLITE_OK;
}

/**
 * Attempt to connect this container to a CloudCache and its cloud container
 */
CloudResult CloudContainer::Connect(CloudCache& cache) {
    if (nullptr != m_cache)
        return CloudResult(1, "container already attached");
    if (nullptr != cache.FindMatching(m_accessName.c_str(), m_containerId.c_str()))
        return CloudResult(1, "container with that name already attached");

    cache.m_containers.push_back(this); // needed for authorization from attach.
    if (!cache.IsAttached(*this)) {
        auto result = cache.CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_attach(cache.m_vfs, m_storageType.c_str(), m_accessName.c_str(), m_containerId.c_str(), m_alias.c_str(), SQLITE_BCV_ATTACH_IFNOT, msg); }, "attach");
        if (!result.IsSuccess()) {
            cache.m_containers.pop_back(); // failed, remove from list
            return result;
        }
    }

    auto stat = m_containerDb.Open(m_alias, cache.m_name);
    if (BE_SQLITE_OK != stat) {
        cache.m_containers.pop_back(); // failed, remove from list
        return CloudResult(stat, "can't open containerDb");
    }
    m_cache = &cache;
    return CloudResult();
}

/**
 * Disconnect this container from the CloudCache. If the containerDb is open, close it first.
 */
CloudResult CloudContainer::Disconnect(bool fromCacheDtor) {
    if (nullptr == m_cache)
        return CloudResult();

    m_onDisconnect.RaiseEvent(this);

    CloseDbIfOpen();

    auto thisCache = m_cache;
    m_cache = nullptr;

    if (!fromCacheDtor) {
        auto& containers = thisCache->m_containers;
        auto entry = std::find(containers.begin(), containers.end(), this);
        if (entry != containers.end())
            containers.erase(entry);
    }

    return CloudResult();
}

/**
 * Permanently disconnect and then detach this container from the CloudCache.
 */
CloudResult CloudContainer::Detach() {
    if (nullptr == m_cache)
        return CloudResult();
    auto thisCache = m_cache;
    auto stat = Disconnect(false);
    return !stat.IsSuccess() ? stat : thisCache->CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_detach(thisCache->m_vfs, m_alias.c_str(), msg); }, "detach");
}

static int uploadBusy(void* container, int nTries) {
    return 1;
}

/**
 * Upload all local changes from this container.
 */
CloudResult CloudContainer::UploadChanges() {
    BeAssert(m_writeLockHeld);
    return CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_upload(m_cache->m_vfs, m_alias.c_str(), uploadBusy, this, msg); }, "upload");
}

/**
 * Revert all local changes from this container.
 */
CloudResult CloudContainer::RevertChanges() {
    return CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_revert(m_cache->m_vfs, m_alias.c_str(), msg); }, "revert");
}

/**
 * Make a copy of a database in the manifest with a new name.
 * @note this does *not* actually copy any data, it merely makes a second entry in the manifest pointing to all the same blocks.
 * The two databases only differ when one or the other is modified later.
 */
CloudResult CloudContainer::CopyDatabase(Utf8StringCR dbFrom, Utf8StringCR dbTo) {
    if (dbFrom.empty() || dbTo.empty())
        return CloudResult(BE_SQLITE_ERROR, "bad argument");
    return CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_copy(m_cache->m_vfs, m_alias.c_str(), dbFrom.c_str(), dbTo.c_str(), msg); }, "copy");
}

/**
 * Remove a database from the CloudContainer.
 */
CloudResult CloudContainer::DeleteDatabase(Utf8StringCR dbName) {
    if (dbName.empty())
        return CloudResult(BE_SQLITE_ERROR, "bad argument");
    return CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_delete(m_cache->m_vfs, m_alias.c_str(), dbName.c_str(), msg); }, "delete");
}

/**
 * Poll the cloud container for updates to the manifest (made by others).
 */
CloudResult CloudContainer::PollManifest() {
    return CallSqliteFn([&](Utf8P* msg) { return sqlite3_bcvfs_poll(m_cache->m_vfs, m_alias.c_str(), msg); }, "poll");
}

/** close the bcv handle, if open */
void CloudUtil::Close() {
    if (m_handle) {
        sqlite3_bcv_close(m_handle);
        m_handle = nullptr;
    }
}

/**
 * Initialize a CloudPrefetch operation for a database. Note: no blocks are requested until Run is called.
 */
DbResult CloudPrefetch::Init(CloudContainer& container, Utf8StringCR dbName) {
    return (DbResult) sqlite3_bcvfs_prefetch_new(container.m_cache->m_vfs, container.m_containerId.c_str(), dbName.c_str(), &m_prefetch);
}

/**
 * destroy a prefetch object and abandon any outstanding requests
 */
void CloudPrefetch::Stop() {
    if (nullptr != m_prefetch) {
        sqlite3_bcvfs_prefetch_destroy(m_prefetch);
        m_prefetch = nullptr;
    }
}

/**
 * request a batch of blocks for a prefetch operation
 */
CloudPrefetch::PrefetchStatus CloudPrefetch::Run(int nRequest, int timeout) {
    PrefetchStatus status;
    if (m_prefetch == nullptr)
        return status;

    status.m_status = sqlite3_bcvfs_prefetch_run(m_prefetch, nRequest, timeout);
    if (status.m_status == BE_SQLITE_DONE)
        return status;

    if (status.m_status != BE_SQLITE_OK) {
        status.m_error = sqlite3_bcvfs_prefetch_errmsg(m_prefetch);
    } else {
        sqlite3_bcvfs_prefetch_status(m_prefetch, SQLITE_BCVFS_PFS_NOUTSTANDING, (sqlite3_int64*) &status.m_nOutstanding);
        sqlite3_bcvfs_prefetch_status(m_prefetch, SQLITE_BCVFS_PFS_NDEMAND, (sqlite3_int64*) &status.m_nDemand);
    }
    return status;
}

/**
 * Initialize a CloudUtil object for a container. Opens a "handle" from SQLite to connect to the container.
 * @param container the container for the connection
 * @param nRequest if >0, the number of simultaneous http requests to make. Default is 6.
 * @param httpTimeout if >0, the number of seconds to wait before considering an http request as timed out. Timed out requests will be tried. Default is 60 seconds.
 */
CloudResult CloudUtil::Init(CloudContainer const& container, int logLevel, int nRequest, int httpTimeout) {
    int stat = sqlite3_bcv_open(container.m_storageType.c_str(), container.m_accessName.c_str(), container.m_accessToken.c_str(), container.m_containerId.c_str(), &m_handle);
    if (SQLITE_OK != stat)
        return CloudResult(stat,  Utf8PrintfString("cannot open CloudContainer: %s", sqlite3_bcv_errmsg(m_handle)).c_str());

    sqlite3_bcv_config(m_handle, SQLITE_BCVCONFIG_PROGRESS, this, ProgressCallback);
    sqlite3_bcv_config(m_handle, SQLITE_BCVCONFIG_LOG, this, LogCallback);
    sqlite3_bcv_config(m_handle, SQLITE_BCVCONFIG_LOGLEVEL, logLevel);
    if (nRequest <= 0)
        nRequest = DEFAULT_MAX_HTTP_CONNECTIONS;
    sqlite3_bcv_config(m_handle, SQLITE_BCVCONFIG_NREQUEST, nRequest);
    if (httpTimeout <= 0) 
        httpTimeout = DEFAULT_HTTP_TIMEOUT;
    sqlite3_bcv_config(m_handle, SQLITE_BCVCONFIG_HTTPTIMEOUT, httpTimeout);
    return CloudResult();
}

/**
 * Initialize the cloud container for SQLite. Creates the "manifest" blob.
 */
CloudResult CloudUtil::InitializeContainer(int nameSize, int blockSize) {
    auto stat = (DbResult)sqlite3_bcv_create(m_handle, nameSize, blockSize);
    return CloudResult(stat, sqlite3_bcv_errmsg(m_handle));
}

/**
 * perform cleanup operation to remove deleted blocks from this CloudContainer
 * @param nSeconds delete all blocks in the container that were marked as unused before this number of seconds ago
 */
CloudResult CloudUtil::CleanDeletedBlocks(int nSeconds) {
    auto stat = sqlite3_bcv_cleanup(m_handle, nSeconds);
    return CloudResult(stat, sqlite3_bcv_errmsg(m_handle));
}

/**
 * Upload a SQLite database to the container.
 * @param localFileName name of the file holding the database
 * @param dbName name for the database within the CloudContainer.
 */
CloudResult CloudUtil::UploadDatabase(Utf8StringCR localFileName, Utf8StringCR dbName) {
    if (localFileName.empty() || dbName.empty())
        return CloudResult(BE_SQLITE_ERROR, "bad argument");
    auto stat = (DbResult)sqlite3_bcv_upload(m_handle, localFileName.c_str(), dbName.c_str());
    return CloudResult(stat, sqlite3_bcv_errmsg(m_handle));
}

/**
 * Download a full copy of a database from the CloudContainer into a local file.
 */
CloudResult CloudUtil::DownloadDatabase(Utf8StringCR dbName, Utf8StringCR localName) {
    if (localName.empty() || dbName.empty())
        return CloudResult(BE_SQLITE_ERROR, "bad argument");
    auto stat = (DbResult)sqlite3_bcv_download(m_handle, dbName.c_str(), localName.c_str());
    return CloudResult(stat, sqlite3_bcv_errmsg(m_handle));
}

