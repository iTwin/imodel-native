/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"

// cspell:ignore nblock ndirty napi ncache walfile prefetching

namespace IModelJsNative {

enum : uint64_t {
    SECONDS_PER_MINUTE = 60,
    SECONDS_PER_HOUR = SECONDS_PER_MINUTE * 60,
};

/** adapted from sqlite code */
static int64_t parseCacheSize(Utf8CP zSize) {
    int64_t ret = 0;
    int i = 0;

    for (; zSize[i]; ++i) {
        if (zSize[i] < '0' || zSize[i] > '9')
            break;
        ret = ret * 10 + (zSize[i] - '0');
    }

    switch (zSize[i]) {
    case 'K':
        ret = ret * MemorySize::K;
        i++;
        break;
    case 'M':
        ret = ret * MemorySize::MEG;
        i++;
        break;
    case 'G':
        ret = ret * MemorySize::GIG;
        i++;
        break;
    case 'T':
        ret = ret * MemorySize::T;
        i++;
        break;
    }

    return zSize[i] ? -1 : ret;
}

/**
 * JavaScript wrapper for SQLite::CloudCache
 */
struct JsCloudCache : CloudCache, Napi::ObjectWrap<JsCloudCache> {
    DEFINE_CONSTRUCTOR;
    JsCloudCache(NapiInfoCR info) : Napi::ObjectWrap<JsCloudCache>(info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, args);
        Utf8StringCR name = stringMember(args, JSON_NAME(name));
        Utf8StringCR rootDir = stringMember(args, JSON_NAME(rootDir));
        if (name.empty() || rootDir.empty())
            BeNapi::ThrowJsException(info.Env(), "invalid arguments");

        int64_t cacheSize = 50 * MemorySize::GIG; // default in sqlite is 1G, that's way too small
        auto cacheSizeStr = stringMember(args, JSON_NAME(cacheSize));
        if (!cacheSizeStr.empty()) {
            cacheSizeStr.ToUpper();
            cacheSize = parseCacheSize(cacheSizeStr.c_str());
            if (cacheSize < 0)
                BeNapi::ThrowJsException(info.Env(), "illegal cache size");
        }
        bool curlDiagnostics = boolMember(args, JSON_NAME(curlDiagnostics), false);
        auto stat = InitCache(name, rootDir, cacheSize, intMember(args, JSON_NAME(nRequests), 0), curlDiagnostics);
        if (!stat.IsSuccess()) {
            if (stat.m_status == BE_SQLITE_CANTOPEN)
                stat.m_error = "Cannot create CloudCache: invalid cache directory or directory does not exist";
            BeNapi::ThrowJsException(info.Env(), stat.m_error.c_str());
        }
    }

    Napi::Value JsIsDaemon(NapiInfoCR info) { return Napi::Boolean::New(info.Env(), IsDaemon()); }
    Napi::Value GetName(NapiInfoCR info) { return Napi::String::New(info.Env(), m_name.c_str()); }
    Napi::Value GetRootDir(NapiInfoCR info) { return Napi::String::New(info.Env(), m_rootDir.c_str()); }
    Napi::Value GetGuid(NapiInfoCR info) { return Napi::String::New(info.Env(), m_guid.c_str()); }
    void Destroy(NapiInfoCR) { CloudCache::Destroy(); }
    void SetLogMask(NapiInfoCR info) {
        REQUIRE_ARGUMENT_INTEGER(0, mask);
        CloudCache::SetLogMask(mask);
    }

    virtual void Log(NativeLogging::SEVERITY sev, Utf8CP message) override {
        auto msg = DateTime::GetCurrentTime().ToTimestampString() + " | " + message;
        NativeLogging::Logging::LogMessage("CloudSqlite", sev, msg.c_str());
    }

    static bool IsInstance(Napi::Object val) { return val.InstanceOf(Constructor().Value()); }
    static void Init(Napi::Env env, Napi::Object exports) {
        static constexpr Utf8CP className = "CloudCache";
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, className, {
            InstanceAccessor<&JsCloudCache::JsIsDaemon>("isDaemon"),
            InstanceAccessor<&JsCloudCache::GetName>(JSON_NAME(name)),
            InstanceAccessor<&JsCloudCache::GetRootDir>(JSON_NAME(rootDir)),
            InstanceAccessor<&JsCloudCache::GetGuid>(JSON_NAME(guid)),
            InstanceMethod<&JsCloudCache::Destroy>("destroy"),
            InstanceMethod<&JsCloudCache::SetLogMask>("setLogMask"),
        });

        exports.Set(className, t);
        SET_CONSTRUCTOR(t)
    }
};

using CloudSqliteFn = std::function<CloudResult()>;

/**
 * An AsyncWorker to perform a CloudSqlite operation on a worker thread. Holds promise that is fulfilled or rejected when the operation completes or fails.
 */
struct CloudSqliteWorker : Napi::AsyncWorker {
    Napi::Promise::Deferred m_promise;
    Napi::ObjectReference m_promiseRef;
    CloudSqliteFn m_fn;
    CloudResult m_result;

    CloudSqliteWorker(Napi::Env env, CloudSqliteFn fn) : Napi::AsyncWorker(env), m_promise(Napi::Promise::Deferred::New(env)), m_fn(fn) {
        m_promiseRef.Reset(m_promise.Promise(), 1); // so promise is valid until we are destroyed
    }

    ~CloudSqliteWorker() { m_promiseRef.Reset(); };
    virtual void Execute() override {
        m_result = m_fn();
        if (!m_result.IsSuccess())
            SetError(m_result.m_error);
    }
    virtual void OnOK() override { m_promise.Resolve(Env().Undefined()); }
    virtual void OnError(Napi::Error const& e) override {
        e.Value()[JSON_NAME(errorNumber)] = m_result.m_status;
        m_promise.Reject(e.Value());
    }
};

static Utf8CP writeLockName = "writeLock";

/** supplies logging for CloudUtil functions from JavaScript */
struct JsCloudUtil : CloudUtil {
    virtual void _Log(Utf8CP message) override { NativeLogging::Logging::LogMessage("CloudSqlite", NativeLogging::SEVERITY::LOG_INFO, message); }
};

/**
 * JavaScript object for accessing cloud containers via SQLite.
 */
struct JsCloudContainer : CloudContainer, Napi::ObjectWrap<JsCloudContainer> {
    int m_durationSeconds;

    DEFINE_CONSTRUCTOR;

    JsCloudContainer(NapiInfoCR info) : Napi::ObjectWrap<JsCloudContainer>(info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, obj);
        m_accessName = stringMember(obj, JSON_NAME(accessName));
        m_containerId = stringMember(obj, JSON_NAME(containerId));
        if (m_accessName.empty())
            BeNapi::ThrowJsException(obj.Env(), "accessName missing from CloudContainer constructor");
        if (m_containerId.empty())
            BeNapi::ThrowJsException(obj.Env(), "containerId missing from CloudContainer constructor");

        m_storageType = stringMember(obj, JSON_NAME(storageType), "azure");
        m_alias = stringMember(obj, JSON_NAME(alias));
        if (m_alias.empty())
            m_alias = m_containerId;
        m_accessToken = stringMember(obj, JSON_NAME(accessToken));
        m_writeable = boolMember(obj, JSON_NAME(writeable), false);
        m_durationSeconds = intMember(obj, JSON_NAME(durationSeconds), 0);
    }

    Napi::Value QueueWorker(NapiInfoCR info, CloudSqliteFn fn) {
        auto worker = new CloudSqliteWorker(info.Env(), fn);
        worker->Queue();
        return worker->m_promise.Promise();
    }

    void RequireConnected() {
        if (!IsContainerConnected())
            BeNapi::ThrowJsException(Env(), "container not connected to cache");
    }
    void RequireWriteLock() {
        RequireConnected();
        if (!m_writeLockHeld)
            BeNapi::ThrowJsException(Env(), Utf8PrintfString("container [%s] is not locked for write access", m_containerId.c_str()).c_str());
    }

    void Connect(NapiInfoCR info) {
        if (IsContainerConnected())
            return; // already connected, do nothing

        REQUIRE_ARGUMENT_ANY_OBJ(0, jsCache);
        if (!JsCloudCache::IsInstance(jsCache))
            BeNapi::ThrowJsException(Env(), "invalid cache argument");

        auto cache = Napi::ObjectWrap<JsCloudCache>::Unwrap(jsCache);
        auto stat = CloudContainer::Connect(*cache);
        if (!stat.IsSuccess())
            BeNapi::ThrowJsException(Env(), stat.m_error.c_str(), stat.m_status);

        if (cache->IsDaemon())
            m_writeable = false; // daemon cannot be used for writing

        if (m_writeable) {
            ResumeWriteLock(); // see if we are re-attaching and previously had the write lock.
            if (!m_writeLockHeld && HasLocalChanges())
                AbandonChanges(info); // we lost the write lock, we have no choice but to abandon all local changes.
        }
        Value().Set("cache", jsCache);
    }

    void Detach(NapiInfoCR info) {
        RequireConnected();
        auto stat = CloudContainer::Detach();
        if (!stat.IsSuccess())
            BeNapi::ThrowJsException(Env(), stat.m_error.c_str(), stat.m_status);
    }

    void Disconnect(NapiInfoCR info) {
        if (!IsContainerConnected())
            return;
        auto stat = CloudContainer::Disconnect(false);
        if (!stat.IsSuccess())
            BeNapi::ThrowJsException(Env(), stat.m_error.c_str(), stat.m_status);

        Value().Set("cache", Env().Undefined());
    }

    void PollManifest(NapiInfoCR info) {
        RequireConnected();
        CloudContainer::PollManifest();
    }

    Napi::Value UploadChanges(NapiInfoCR info) {
        RequireWriteLock();
        return QueueWorker(info, [=]() { return CloudContainer::UploadChanges(); });
    }

    Napi::Value GetBlockSize(NapiInfoCR) {
        RequireConnected();
        return Napi::Number::New(Env(), m_cache->GetBlockSize());
    }

    Napi::Value QueryDatabaseHash(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        if (HasLocalChanges())
            BeNapi::ThrowJsException(Env(), "cannot obtain database hash with local changes");
        return Napi::String::New(Env(), m_cache->GetDatabaseHash(*this, dbName));
    }

    Napi::Value QueryDatabases(NapiInfoCR info) {
        RequireConnected();
        OPTIONAL_ARGUMENT_STRING(0, globArg);
        Utf8String sql = "SELECT database FROM bcv_database WHERE container=?";
        if (!globArg.empty())
            sql += " AND database GLOB ?";
        sql += " ORDER BY database";

        Statement stmt;
        auto rc =  stmt.Prepare(m_containerDb, sql.c_str());
        BeAssert (rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);
        stmt.BindText(1, m_alias.c_str(), Statement::MakeCopy::No);
        if (!globArg.empty())
            stmt.BindText(2, globArg.c_str(), Statement::MakeCopy::Yes);
        auto rows = Napi::Array::New(Env());
        BeJsValue rowsValue(rows);
        while (BE_SQLITE_ROW == stmt.Step())
            rowsValue.appendValue() = stmt.GetValueText(0);
        return rows;
    }

    Napi::Value QueryDatabase(NapiInfoCR info) {
        RequireConnected();
        REQUIRE_ARGUMENT_STRING(0, dbName);
        Statement stmt;
        auto rc =  stmt.Prepare(m_containerDb, "SELECT nblock,ncache,ndirty,walfile,state FROM bcv_database WHERE container=? AND database=?");
        BeAssert (rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);
        stmt.BindText(1, m_alias.c_str(), Statement::MakeCopy::No);
        stmt.BindText(2, dbName.c_str(), Statement::MakeCopy::No);
        if (BE_SQLITE_ROW != stmt.Step())
            return Env().Undefined();
        BeJsNapiObject value(Env());
        value["totalBlocks"] = stmt.GetValueInt(0);
        value["localBlocks"] = stmt.GetValueInt(1);
        value["dirtyBlocks"] = stmt.GetValueInt(2);
        value["transactions"] = stmt.GetValueBoolean(3);
        value["state"] = stmt.GetValueText(4);

        return value;
    }

    bool HasLocalChanges() {
        RequireConnected();
        Statement stmt;
        auto rc = stmt.Prepare(m_containerDb, "SELECT 1 FROM bcv_database WHERE container=? AND (nDirty>0 OR walfile OR state!='')");
        BeAssert (rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);
        stmt.BindText(1, m_alias.c_str(), Statement::MakeCopy::No);
        return BE_SQLITE_ROW == stmt.Step();
    }

    Napi::Value HasLocalChangesJs(NapiInfoCR) {
        return Napi::Boolean::New(Env(), HasLocalChanges());
    }

    Napi::Value CopyDatabase(NapiInfoCR info) {
        RequireWriteLock();
        REQUIRE_ARGUMENT_STRING(0, fromName);
        REQUIRE_ARGUMENT_STRING(1, toName);
        return QueueWorker(info, [=]() { return CloudContainer::CopyDatabase(fromName, toName); });
    }

    Napi::Value DeleteDatabase(NapiInfoCR info) {
        RequireWriteLock();
        REQUIRE_ARGUMENT_STRING(0, dbName);
        return QueueWorker(info, [=]() { return CloudContainer::DeleteDatabase(dbName); });
    }

    void InitializeContainer(NapiInfoCR info) {
        JsCloudUtil handle;
        auto result = handle.Init(*this);
        if (result.IsSuccess()) {
            int blockSize = 0;
            bool checksumName = false;
            if (info[0].IsObject()) {
                auto opts = BeJsConst(info[0].As<Napi::Object>());
                blockSize = opts[JSON_NAME(blockSize)].asInt(0);
                checksumName = opts[JSON_NAME(checksumBlockNames)].asBool(false);
            }
            result = handle.InitializeContainer(checksumName ? 24 : 16, blockSize);
        }
        if (result.m_status != BE_SQLITE_OK)
            BeNapi::ThrowJsException(Env(), result.m_error.c_str(), result.m_status);
    }

    Napi::Value CleanDeletedBlocks(NapiInfoCR info) {
        RequireWriteLock();
        OPTIONAL_ARGUMENT_INTEGER(0, nSeconds, (60*60)); // default to one hour
        return QueueWorker(info, [=]() {
            JsCloudUtil handle;
            auto result = handle.Init(*this);
            if (result.IsSuccess())
                result = handle.CleanDeletedBlocks(nSeconds);
            return result.IsSuccess() ? CloudContainer::PollManifest() : result;
        });
    }

    void ReadWriteLock(BeJsDocument& out) {
        Statement stmt;
        auto rc = stmt.Prepare(m_containerDb, "SELECT value FROM bcv_kv WHERE name=?");
        BeAssert(rc == BE_SQLITE_OK);
        stmt.BindText(1, writeLockName, Statement::MakeCopy::No);
        rc = stmt.Step();
        if (rc != BE_SQLITE_ROW)
            return; // not locked

        Utf8String val = stmt.GetValueText(0);
        if (!val.empty())
            out.Parse(val);
        return;
    }

    void CheckLock() {
        BeJsDocument lockedBy;
        ReadWriteLock(lockedBy);
        auto lockedByGuid = lockedBy[JSON_NAME(guid)].asString();
        if (lockedByGuid.empty() || lockedByGuid.Equals(m_cache->m_guid))
            return; // not locked or already locked by same cache

        auto expiresAt = DateTime::FromString(lockedBy[JSON_NAME(expires)].asString());
        if (!expiresAt.IsValid())
            return; // the expiration time is invalid, ignore lock

        auto lockedByUser = lockedBy[JSON_NAME(user)].asString();
        if (DateTime::CompareResult::EarlierThan == DateTime::Compare(expiresAt, DateTime::GetCurrentTimeUtc())) {
            Utf8PrintfString warning("write lock on container [%s] from user [%s] was present but expired. Overwriting it.", m_containerId.c_str(), lockedByUser.c_str());
            NativeLogging::Logging::LogMessage("CloudSqlite", NativeLogging::SEVERITY::LOG_WARNING, warning.c_str());
            return; // other user's write lock has expired
        }

        m_containerDb.TryExecuteSql("ROLLBACK");

        // report that container is currently locked by another user. Set details in exception.
        auto err = Napi::Error::New(Env(), Utf8PrintfString("Container [%s] is currently locked.", m_containerId.c_str()).c_str());
        auto val = err.Value();
        val[JSON_NAME(errorNumber)] = (int) BE_SQLITE_BUSY;
        val[JSON_NAME(lockedBy)] = lockedByUser.c_str();
        val[JSON_NAME(expires)] = expiresAt.ToLocalTime().ToString().c_str();
        throw err;
    }

    void ResumeWriteLock() {
        BeJsDocument lockedBy;
        ReadWriteLock(lockedBy);

        auto lockedByGuid = lockedBy[JSON_NAME(guid)].asString();
        if (lockedByGuid.Equals(m_cache->m_guid)) {
            try {
                AcquireWriteLock(lockedBy[JSON_NAME(user)].asString());
            } catch (...) {
                // if we can't resume, don't fail
            }
        }
    }

    Utf8String GetCurrentDateString(uint64_t offsetMilliseconds) {
        uint64_t nowMs = 0;
        DateTime::GetCurrentTimeUtc().ToJulianDay(nowMs);
        return DateTime::FromJulianDay(nowMs + offsetMilliseconds, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc)).ToString();
    }

    void AcquireWriteLock(Utf8StringCR user) {
        RequireConnected();
        if (!m_writeable)
            BeNapi::ThrowJsException(Env(), "container is not writeable");

        m_containerDb.TryExecuteSql("BEGIN");
        CheckLock(); // throws if already locked by another user

        m_durationSeconds = std::min((int) (12*SECONDS_PER_HOUR), std::max((int)SECONDS_PER_HOUR, m_durationSeconds));
        BeJsDocument lockedBy;
        lockedBy[JSON_NAME(guid)] = m_cache->m_guid;
        lockedBy[JSON_NAME(user)] = user;
        lockedBy[JSON_NAME(expires)] = GetCurrentDateString(m_durationSeconds * 1000);

        Statement stmt;
        auto rc = stmt.Prepare(m_containerDb, "REPLACE INTO bcv_kv(value,name) VALUES(?,?)");
        BeAssert(rc == BE_SQLITE_OK);
        stmt.BindText(1, lockedBy.Stringify(), Statement::MakeCopy::Yes);
        stmt.BindText(2, writeLockName, Statement::MakeCopy::No);
        rc = stmt.Step();
        if (rc == BE_SQLITE_DONE)
            rc = m_containerDb.TryExecuteSql("COMMIT");
        if (rc != BE_SQLITE_OK) {
            if (rc == BE_SQLITE_IOERR_AUTH)
                BeNapi::ThrowJsException(Env(), "not authorized to obtain write lock", BE_SQLITE_AUTH);
            BeNapi::ThrowJsException(Env(), "cannot obtain write lock", rc);
        }
        m_writeLockHeld = true;
    }

    void AcquireWriteLockJs(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, user);
        AcquireWriteLock(user);
        PollManifest(info);
    }

    Napi::Value GetNumCleanupBlocks(NapiInfoCR info) {
        RequireConnected();
        return Napi::Number::New(Env(), m_cache->GetNumCleanupBlocks(*this));
    }

    CloudResult ClearWriteLock() {
        m_containerDb.TryExecuteSql("BEGIN");

        BeJsDocument currentLock;
        ReadWriteLock(currentLock);
        BeJsDocument root;
        auto lastLock = root["lastLock"];
        lastLock["time"] = GetCurrentDateString(0);
        lastLock[JSON_NAME(user)] = currentLock[JSON_NAME(user)].asString();

        Statement stmt;
        auto rc = stmt.Prepare(m_containerDb, "UPDATE bcv_kv SET value=? WHERE name=?");
        BeAssert(rc == BE_SQLITE_OK);
        stmt.BindText(1, root.Stringify(), Statement::MakeCopy::Yes);
        stmt.BindText(2, writeLockName, Statement::MakeCopy::No);
        rc = stmt.Step();
        if (rc == BE_SQLITE_DONE)
            rc = m_containerDb.TryExecuteSql("COMMIT");
        if (rc != BE_SQLITE_OK)  {
            Utf8String msg = "unable to release write lock";
            if (rc== BE_SQLITE_IOERR_AUTH)
                msg += ": authorization token expired";
            return CloudResult(rc, msg.c_str());
        }

        m_writeLockHeld = false;
        return CloudResult();
    }

    void ClearWriteLockJs(NapiInfoCR info) {
        RequireConnected();
        auto stat = ClearWriteLock();
        if (!stat.IsSuccess())
            BeNapi::ThrowJsException(Env(), stat.m_error.c_str(), stat.m_status);
    }

    void ReleaseWriteLock(NapiInfoCR info) {
        RequireConnected();
        if (!m_writeLockHeld)
            return;

        auto stat = CloudContainer::UploadChanges();
        if (stat.IsSuccess())
            stat = ClearWriteLock();

        if (!stat.IsSuccess())
            BeNapi::ThrowJsException(Env(), stat.m_error.c_str(), stat.m_status);
    }

    void AbandonChanges(NapiInfoCR info) {
        RequireConnected();

        CloudResult stat;
        if (m_writeLockHeld)
            stat = ClearWriteLock();
        if (stat.IsSuccess())
            stat = CloudContainer::RevertChanges();

        if (!stat.IsSuccess())
            BeNapi::ThrowJsException(Env(), stat.m_error.c_str(), stat.m_status);
    }

    static bool IsInstance(Napi::Object val) { return val.InstanceOf(Constructor().Value()); }
    Napi::Value IsConnected(NapiInfoCR info) { return Napi::Boolean::New(Env(), IsContainerConnected()); }
    Napi::Value IsWriteable(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_writeable); }
    Napi::Value HasWriteLock(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_writeLockHeld); }
    Napi::Value GetAccessToken(NapiInfoCR info) { return Napi::String::New(Env(), m_accessToken); }
    Napi::Value GetContainerId(NapiInfoCR info) { return Napi::String::New(Env(), m_containerId); }
    Napi::Value GetAlias(NapiInfoCR info) { return Napi::String::New(Env(), m_alias); }
    void SetAccessToken(NapiInfoCR info, Napi::Value const& value) { m_accessToken = value.As<Napi::String>().Utf8Value(); }

    static void Init(Napi::Env env, Napi::Object exports) {
        static constexpr Utf8CP className = "CloudContainer";
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, className, {
            InstanceAccessor<&JsCloudContainer::GetAccessToken, &JsCloudContainer::SetAccessToken>("accessToken"),
            InstanceAccessor<&JsCloudContainer::GetAlias>("alias"),
            InstanceAccessor<&JsCloudContainer::GetContainerId>("containerId"),
            InstanceAccessor<&JsCloudContainer::GetNumCleanupBlocks>("garbageBlocks"),
            InstanceAccessor<&JsCloudContainer::HasLocalChangesJs>("hasLocalChanges"),
            InstanceAccessor<&JsCloudContainer::HasWriteLock>("hasWriteLock"),
            InstanceAccessor<&JsCloudContainer::IsConnected>("isConnected"),
            InstanceAccessor<&JsCloudContainer::IsWriteable>("isWriteable"),
            InstanceAccessor<&JsCloudContainer::GetBlockSize>("blockSize"),
            InstanceMethod<&JsCloudContainer::AbandonChanges>("abandonChanges"),
            InstanceMethod<&JsCloudContainer::AcquireWriteLockJs>("acquireWriteLock"),
            InstanceMethod<&JsCloudContainer::CleanDeletedBlocks>("cleanDeletedBlocks"),
            InstanceMethod<&JsCloudContainer::ClearWriteLockJs>("clearWriteLock"),
            InstanceMethod<&JsCloudContainer::Connect>("connect"),
            InstanceMethod<&JsCloudContainer::CopyDatabase>("copyDatabase"),
            InstanceMethod<&JsCloudContainer::DeleteDatabase>("deleteDatabase"),
            InstanceMethod<&JsCloudContainer::Detach>("detach"),
            InstanceMethod<&JsCloudContainer::Disconnect>("disconnect"),
            InstanceMethod<&JsCloudContainer::InitializeContainer>("initializeContainer"),
            InstanceMethod<&JsCloudContainer::PollManifest>("checkForChanges"),
            InstanceMethod<&JsCloudContainer::QueryDatabase>("queryDatabase"),
            InstanceMethod<&JsCloudContainer::QueryDatabaseHash>("queryDatabaseHash"),
            InstanceMethod<&JsCloudContainer::QueryDatabases>("queryDatabases"),
            InstanceMethod<&JsCloudContainer::ReleaseWriteLock>("releaseWriteLock"),
            InstanceMethod<&JsCloudContainer::UploadChanges>("uploadChanges"),
        });

        exports.Set(className, t);
        SET_CONSTRUCTOR(t)
    }
};


/**
 * A request to upload or download a CloudDb
 */
struct CloudDbTransfer : Napi::ObjectWrap<CloudDbTransfer> {
    // Async Worker that does download on another thread.
    struct Downloader : Napi::AsyncWorker {
        enum Direction { Upload = 0, Download = 1 };
        struct JsDownloadJob : JsCloudUtil { // runs on AsyncWorker thread
            mutable BeMutex m_mutex;
            uint64_t m_nDone = 0;
            uint64_t m_nTotal = 0;
            bool m_abort = false; // set from DownloadV2Checkpoint::CancelDownload
            int _OnProgress(uint64_t nDone, uint64_t nTotal) override {
                BeMutexHolder holder(m_mutex);
                m_nDone = nDone;
                m_nTotal = nTotal;
                return m_abort ? 1 : 0;
            }
        };

        Napi::ObjectReference m_request;
        Direction m_direction;
        JsDownloadJob m_job;
        Utf8String m_dbName;
        Utf8String m_localFile;

        void Execute() override { // worker thread
            auto stat = m_direction == Direction::Upload ? m_job.UploadDatabase(m_localFile, m_dbName) : m_job.DownloadDatabase(m_dbName, m_localFile);
            if (stat.m_status == BE_SQLITE_ABORT)
                SetError("cancelled");
            else if (stat.m_status != BE_SQLITE_OK)
                SetError(stat.m_error);
        }

        void OnOK() override { // main thread
            auto* request = CloudDbTransfer::Unwrap(m_request.Value());
            request->m_promise.Resolve(Napi::Number::New(Env(), 0));
            request->m_pending = nullptr;
        }

        void OnError(Napi::Error const& error) override { // main thread
            auto* request = CloudDbTransfer::Unwrap(m_request.Value());
            request->m_promise.Reject(error.Value());
            request->m_pending = nullptr;
        }

        Downloader(Direction direction, JsCloudContainer& container, Napi::Object args, CloudDbTransfer const& request) : Napi::AsyncWorker(args.Env()) {
            if (direction == Direction::Upload)
                container.RequireWriteLock();
            m_request.Reset(request.Value(), 1);
            m_direction = direction;
            m_localFile = requireString(args, JSON_NAME(localFileName));
            m_dbName = requireString(args, JSON_NAME(dbName));
            auto stat = m_job.Init(container, 0, intMember(args, JSON_NAME(nRequests), 0));
            if (!stat.IsSuccess())
                BeNapi::ThrowJsException(Env(), stat.m_error.c_str(), stat.m_status);
        }

        ~Downloader() { m_request.Reset(); }
    };

    DEFINE_CONSTRUCTOR;
    Napi::Promise::Deferred m_promise;
    Downloader* m_pending = nullptr;

    void CheckStillPending(Napi::Env env) {
        if (m_pending == nullptr)
            BeNapi::ThrowJsException(env, "transfer already completed");
    }
    // get the current progress of the active upload/download.
    Napi::Value GetProgress(NapiInfoCR info) {
        CheckStillPending(info.Env());
        auto& job = m_pending->m_job;
        BeMutexHolder holder(job.m_mutex);
        BeJsNapiObject ret(info.Env());
        ret["loaded"] = (double)job.m_nDone;
        ret["total"] = (double)job.m_nTotal;
        return ret;
    }

    // Cancel a previous request for a upload/download
    void CancelTransfer(NapiInfoCR info) {
        CheckStillPending(info.Env());
        m_pending->m_job.m_abort = true;
    }

    CloudDbTransfer(NapiInfoCR info) : Napi::ObjectWrap<CloudDbTransfer>(info), m_promise(Napi::Promise::Deferred::New(info.Env())) {
        REQUIRE_ARGUMENT_STRING(0, direction);
        REQUIRE_ARGUMENT_ANY_OBJ(1, jsContainer);
        if (!JsCloudContainer::IsInstance(jsContainer))
            BeNapi::ThrowJsException(Env(), "invalid container argument");
        auto container = Napi::ObjectWrap<JsCloudContainer>::Unwrap(jsContainer);

        REQUIRE_ARGUMENT_ANY_OBJ(2, args);
        Value().Set(JSON_NAME(promise), m_promise.Promise()); // holds a reference to the promise to keep it alive
        m_pending = new Downloader(direction.Equals("upload") ? Downloader::Direction::Upload : Downloader::Direction::Download, *container, args, *this);
        m_pending->Queue(); // takes ownership of m_pending, deletes on completion
    }

    static void Init(Napi::Env& env, Napi::Object exports) {
        static constexpr Utf8CP className = "CloudDbTransfer";
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, className, {
            InstanceMethod<&CloudDbTransfer::CancelTransfer>("cancelTransfer"),
            InstanceMethod<&CloudDbTransfer::GetProgress>("getProgress"),
        });

        exports.Set(className, t);
        SET_CONSTRUCTOR(t);
    }
};

Napi::Value getJsCloudContainer(Napi::Value arg) {
    if (!arg.IsObject()) // did they supply a container argument?
        return arg.Env().Undefined();

    auto jsContainer = arg.As<Napi::Object>();
    if (!JsCloudContainer::IsInstance(jsContainer))
        BeNapi::ThrowJsException(arg.Env(), "invalid container argument");

    return arg;
}

CloudContainer* getCloudContainer(Napi::Value jsContainer) {
    return jsContainer.IsObject() ? Napi::ObjectWrap<JsCloudContainer>::Unwrap(jsContainer.As<Napi::Object>()) : nullptr;
}

/**
 * JavaScript object for prefetching blocks for a cloud database.
 */
struct JsCloudPrefetch : Napi::ObjectWrap<JsCloudPrefetch> {

    struct PrefetchWorker : Napi::AsyncWorker {
        CloudPrefetch m_prefetch;
        bool m_aborted = false;
        int m_maxRequests = 6;
        int m_minRequests = 3;
        int m_timeout = 100;
        BeConditionVariable m_cv;
        std::function<void()> m_removeMe;
        Napi::ObjectReference m_jsPrefetch;

        PrefetchWorker(NapiInfoCR info, JsCloudPrefetch const& jsObj) : Napi::AsyncWorker(info.Env()) {
            auto container = getCloudContainer(info[0]);
            REQUIRE_ARGUMENT_STRING(1, dbName);
            auto rc = m_prefetch.Init(*container, dbName);
            if (rc != BE_SQLITE_OK)
                BeNapi::ThrowJsException(Env(), "error initializing prefetch", rc);
            if (info[2].IsObject()) {
                BeJsValue args(info[2]);
                m_maxRequests = std::min(30, args[JSON_NAME(nRequests)].asInt(m_maxRequests));
                m_minRequests = std::min(30, args[JSON_NAME(minRequests)].asInt(m_minRequests));
                m_timeout = args[JSON_NAME(timeout)].asInt(m_timeout);
            }
            m_jsPrefetch.Reset(jsObj.Value(), 1);
            // set up notification if the container is being disconnected (e.g. on exit) to stop this prefetch
            m_removeMe = container->m_onDisconnect.AddListener([&](CloudContainerP) {
                Cancel();
            });
        }

        ~PrefetchWorker() {
            BeAssert(!m_prefetch.IsRunning());
            if (nullptr != m_removeMe) // drop this object from listeners of disconnect from container
                m_removeMe();
        }

        /**
         * Function that runs on background thread performing prefetch operation.
         *
         * This attempts to give priority to foreground (on-demand) requests, because there is finite bandwidth and
         * prefetching can slow down direct SQL operations. That is accomplished by minimizing new prefetch
         * requests whenever there is activity on other threads (nDemand>0). Of course outstanding prefetch requests must be
         * serviced as they complete until the queue empties. It is important to understand that when other
         * threads need a database block that is currently in the prefetch queue, they don't re-request it. Instead they wait for
         * its completion from prefetch. `CloudPrefetch.Run` returns after every block arrives, so it is important to call
         * it again without delay to not slow down progress on the other threads waiting for blocks from SQL.
         */
        void Execute() override { // runs on worker thread
            int nRequest = m_minRequests;
            while (true) {
                if (m_aborted)
                    break; // we were asked to stop from another thread

                // m_prefetch.Run returns after the earlier of:
                //  1. any prefetch request completes,
                //  2. timeout expired with outstanding prefetch requests,
                //  3. nRequest was 0 and there are no outstanding prefetch requests (i.e. there are more blocks to download,
                //      but the prefetch queue is empty due to foreground activity)
                auto stats = m_prefetch.Run(nRequest, m_timeout);
                if (BE_SQLITE_DONE == stats.m_status)
                    break; // prefetch finished

                if (BE_SQLITE_OK != stats.m_status) {
                    m_aborted = true;
                    break; // we hit an error, quit
                }

                // For case 3, it returns immediately. Sleep to give foreground priority for a while.
                if (stats.m_nOutstanding == 0)
                    BeDuration::FromMilliseconds(m_timeout).Sleep();

                // while there is foreground (demand) activity, use minRequests. Otherwise use maxRequests.
                nRequest = (stats.m_nDemand > 0) ? m_minRequests : m_maxRequests;
            }

            BeMutexHolder lock(m_cv.GetMutex());
            m_prefetch.Stop(); // aborted, finished, or failed. Free the SQLite prefetch object
            m_cv.notify_all(); // tell waiters
        }

        /**
         * Synchronously cancel this prefetch if it is still running. Does not return until prefetch object has been stopped and deleted.
         * NOTE: *must* only be called on the main thread.
         */
        void Cancel() { // main thread
            BeAssert(JsInterop::IsMainThread());
            BeMutexHolder lock(m_cv.GetMutex());
            if (m_prefetch.IsRunning()) {
                m_aborted = true;
                m_cv.InfiniteWait(lock);
            }
        }

        void OnOK() override { // main thread
            auto* request = JsCloudPrefetch::Unwrap(m_jsPrefetch.Value());
            request->m_promise.Resolve(Napi::Boolean::New(Env(), !m_aborted));
            request->m_pending = nullptr;
        }

        void OnError(Napi::Error const& error) override { // main thread
            auto* request = JsCloudPrefetch::Unwrap(m_jsPrefetch.Value());
            request->m_promise.Reject(error.Value());
            request->m_pending = nullptr;
        }
    };

    DEFINE_CONSTRUCTOR;
    Napi::Promise::Deferred m_promise;
    PrefetchWorker* m_pending = nullptr;

    JsCloudPrefetch(NapiInfoCR info) : Napi::ObjectWrap<JsCloudPrefetch>(info), m_promise(Napi::Promise::Deferred::New(info.Env())) {
        Value().Set(JSON_NAME(promise), m_promise.Promise()); // holds a reference to the promise to keep it alive
        m_pending = new PrefetchWorker(info, *this);
        Value().Set(JSON_NAME(cloudContainer), info[0]);
        Value().Set(JSON_NAME(dbName), info[1]);
        m_pending->Queue(); // takes ownership of m_pending, deletes on completion
    }

    // Cancel a prefetch operation
    void Cancel(NapiInfoCR info) {
        if (m_pending)
            m_pending->Cancel();
    }

    static void Init(Napi::Env& env, Napi::Object exports) {
        static constexpr Utf8CP className = "CloudPrefetch";
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, className, {
            InstanceMethod<&JsCloudPrefetch::Cancel>("cancel"),
        });

        exports.Set(className, t);
        SET_CONSTRUCTOR(t);
    }
};

/**
 * register the classes in this source file.
 */
void registerCloudSqlite(Napi::Env env, Napi::Object exports) {
    CloudDbTransfer::Init(env, exports);
    JsCloudCache::Init(env, exports);
    JsCloudContainer::Init(env, exports);
    JsCloudPrefetch::Init(env, exports);
}

} // end namespace IModelJsNative