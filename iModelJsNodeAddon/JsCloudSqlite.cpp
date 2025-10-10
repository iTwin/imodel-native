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
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "invalid arguments", DbResult::BE_SQLITE_MISUSE);

        int64_t cacheSize = 50 * MemorySize::GIG; // default in sqlite is 1G, that's way too small
        auto cacheSizeStr = stringMember(args, JSON_NAME(cacheSize));
        if (!cacheSizeStr.empty()) {
            cacheSizeStr.ToUpper();
            cacheSize = parseCacheSize(cacheSizeStr.c_str());
            if (cacheSize < 0)
                THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), "illegal cache size", DbResult::BE_SQLITE_MISUSE);
        }
        bool curlDiagnostics = boolMember(args, JSON_NAME(curlDiagnostics), false);
        auto stat = InitCache(name, rootDir, cacheSize, intMember(args, JSON_NAME(nRequests), 0), intMember(args, JSON_NAME(httpTimeout), 0), curlDiagnostics);
        if (!stat.IsSuccess()) {
            if (stat.m_status == BE_SQLITE_CANTOPEN)
                stat.m_error = "Cannot create CloudCache: invalid cache directory or directory does not exist";
            THROW_JS_BE_SQLITE_EXCEPTION(info.Env(), stat.m_error.c_str(), (DbResult)stat.m_status);
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
    int m_lockExpireSeconds;

    DEFINE_CONSTRUCTOR;

    JsCloudContainer(NapiInfoCR info) : Napi::ObjectWrap<JsCloudContainer>(info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, obj);

        m_baseUri = stringMember(obj, JSON_NAME(baseUri));
        m_containerId = stringMember(obj, JSON_NAME(containerId));
        if (m_baseUri.empty())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(obj.Env(), "baseUri missing from CloudContainer constructor", IModelJsNativeErrorKey::BadArg);
        if (m_containerId.empty())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(obj.Env(), "containerId missing from CloudContainer constructor", IModelJsNativeErrorKey::BadArg);

        m_storageType = stringMember(obj, JSON_NAME(storageType), "azure");
        if (m_storageType.Trim().StartsWith("azure")) {
            m_storageType = "azure";
            m_storageParams = "?customuri=1&sas=1";
        }

        m_alias = stringMember(obj, JSON_NAME(alias));
        if (m_alias.empty())
            m_alias = m_containerId;
        m_accessToken = stringMember(obj, JSON_NAME(accessToken));
        m_writeable = boolMember(obj, JSON_NAME(writeable), false);
        m_lockExpireSeconds = intMember(obj, JSON_NAME(lockExpireSeconds), 0);
        m_logId = stringMember(obj, JSON_NAME(logId), "");
        m_secure = boolMember(obj, JSON_NAME(secure), false);
        m_isPublic = boolMember(obj, JSON_NAME(isPublic), false);
    }

    Napi::Value QueueWorker(NapiInfoCR info, CloudSqliteFn fn) {
        auto worker = new CloudSqliteWorker(info.Env(), fn);
        worker->Queue();
        return worker->m_promise.Promise();
    }

    void RequireConnected() {
        if (!IsContainerConnected())
            THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), "container not connected to cache", IModelJsNativeErrorKey::NotInitialized);
    }

    void RequireWriteLock() {
        RequireConnected();
        if (!m_writeLockHeld)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), Utf8PrintfString("container [%s] is not locked for write access", m_containerId.c_str()).c_str(), IModelJsNativeErrorKey::LockNotHeld);
    }

    void CallJsMemberFunc(Utf8CP funcName, std::vector<napi_value> const& args) {
        auto func = Value().Get(funcName);
        if (func.IsFunction())
            func.As<Napi::Function>().Call(args);
    }

    void Connect(NapiInfoCR info) {
        if (IsContainerConnected())
            return; // already connected, do nothing

        REQUIRE_ARGUMENT_ANY_OBJ(0, jsCache);
        if (!JsCloudCache::IsInstance(jsCache))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), "invalid cache argument", IModelJsNativeErrorKey::BadArg);

        auto thisObj = Value();
        CallJsMemberFunc("onConnect", {thisObj, jsCache});

        auto cache = Napi::ObjectWrap<JsCloudCache>::Unwrap(jsCache);
        auto stat = CloudContainer::Connect(*cache);
        if (!stat.IsSuccess())
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), stat.m_error.c_str(), (DbResult)stat.m_status);

        if (cache->IsDaemon())
            m_writeable = false; // daemon cannot be used for writing

        if (m_writeable) {
            ResumeWriteLock(); // see if we are re-attaching and previously had the write lock.
            if (!m_writeLockHeld && HasLocalChanges())
                AbandonChanges(info); // we lost the write lock, we have no choice but to abandon all local changes.
        }

        thisObj.Set("cache", jsCache);
        CallJsMemberFunc("onConnected", {thisObj});
    }

    void OnDisconnect(bool isDetach) override {
        CallJsMemberFunc("onDisconnect", {Value(), Napi::Boolean::New(Env(), isDetach)});
    }
    void OnDisconnected(bool isDetach) override {
        CallJsMemberFunc("onDisconnected", {Value(), Napi::Boolean::New(Env(), isDetach)});
    }

    void Disconnect(NapiInfoCR info) {
        if (!IsContainerConnected())
            return;

        BeJsConst args(info[0]);
        bool isDetach = args.isObject() && args["detach"].GetBoolean(false);
        auto stat = CloudContainer::Disconnect(isDetach, false);
        if (!stat.IsSuccess())
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), stat.m_error.c_str(), (DbResult)stat.m_status);

        Value().Set("cache", Env().Undefined());
    }

    void PollManifest(NapiInfoCR info) {
        RequireConnected();
        CloudContainer::PollManifest();
    }

    Napi::Value UploadChanges(NapiInfoCR info) {
        RequireWriteLock();
        return QueueWorker(info, [=, this]() { return CloudContainer::UploadChanges(); });
    }

    Napi::Value GetBlockSize(NapiInfoCR) {
        RequireConnected();
        return Napi::Number::New(Env(), m_cache->GetBlockSize());
    }

    Napi::Value QueryDatabaseHash(NapiInfoCR info) {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        if (HasLocalChanges())
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), "cannot obtain database hash with local changes", BE_SQLITE_BUSY);
        return Napi::String::New(Env(), m_cache->GetDatabaseHash(*this, dbName));
    }

    Napi::Value QueryHttpLog(NapiInfoCR info) {
        RequireConnected();
        int startFromId = 0;
        Utf8String finishedAtOrAfterTime;
        bool showOnlyFinished = false;
        if (info[0].IsObject()) {
            auto filterOpts = BeJsConst(info[0]);
            startFromId = filterOpts[JSON_NAME(startFromId)].asInt(0);
            finishedAtOrAfterTime = filterOpts[JSON_NAME(finishedAtOrAfterTime)].asCString();
            showOnlyFinished = filterOpts[JSON_NAME(showOnlyFinished)].asBool();
        }

        Utf8String sql = "SELECT id,start_time,end_time,method,client,logmsg,uri,httpcode FROM bcv_http_log";
        bool hasWhereClause = false;
        if (startFromId != 0) {
            sql += " WHERE id >= ?";
            hasWhereClause = true;
        }
        if (showOnlyFinished) {
            if (hasWhereClause) {
                sql += " AND end_time IS NOT NULL";
            } else {
                sql += " WHERE end_time IS NOT NULL";
                hasWhereClause = true;
            }
        }
        if (!finishedAtOrAfterTime.empty()) {
            if (hasWhereClause) {
                sql += " AND julianday(end_time) >= julianday(?)";
            } else {
                sql += " WHERE julianday(end_time) >= julianday(?)";
                hasWhereClause = true;
            }
        }
        sql += " ORDER BY id ASC";

        Statement stmt;
        auto rc = stmt.Prepare(m_containerDb, sql.c_str());
        if (rc != BE_SQLITE_OK)
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), Utf8PrintfString("Got error preparing %s", sql.c_str()).c_str(), rc);

        if (startFromId != 0 && !finishedAtOrAfterTime.empty()) {
            stmt.BindInt(1, startFromId);
            stmt.BindText(2, finishedAtOrAfterTime.c_str(), Statement::MakeCopy::Yes);
        } else if (startFromId != 0) {
            stmt.BindInt(1, startFromId);
        } else if (!finishedAtOrAfterTime.empty()) {
            stmt.BindText(1, finishedAtOrAfterTime.c_str(), Statement::MakeCopy::Yes);
        }

        auto rows = Napi::Array::New(Env());
        uint32_t index = 0;
        while (BE_SQLITE_ROW == stmt.Step()) {
            BeJsNapiObject value(Env());
            value["id"] = stmt.GetValueInt(0);
            value["startTime"] = stmt.GetValueText(1);
            value["endTime"] = stmt.GetValueText(2);
            value["method"] = stmt.GetValueText(3);
            value["logId"] = stmt.GetValueText(4);
            value["logmsg"] = stmt.GetValueText(5);
            value["uri"] = stmt.GetValueText(6);
            value["httpcode"] = stmt.GetValueInt(7);
            rows.Set(index++, value);
        }
        return rows;
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
        auto rc =  stmt.Prepare(m_containerDb, "SELECT nblock,ncache,ndirty,walfile,state,nclient,nprefetch FROM bcv_database WHERE container=? AND database=?");
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
        value["nClient"] = stmt.GetValueInt(5);
        value["nPrefetch"] = stmt.GetValueInt(6);

        return value;
    }

    Napi::Value QueryBcvStats(NapiInfoCR info) {
        RequireConnected();
        bool addClientInformation = false;
        if (info[0].IsObject()) {
            auto filterOpts = BeJsConst(info[0]);
            addClientInformation = filterOpts[JSON_NAME(addClientInformation)].asBool();
        }

        Statement stmt;
        Utf8CP statNames[] = { "nlock", "ncache", "cachesize", "memory_used", "memory_highwater", "memory_manifest" };
        Utf8CP jsNames[] = { "lockedCacheslots", "populatedCacheslots", "totalCacheslots", "memoryUsed", "memoryHighwater", "memoryManifest" };
        BeAssert (sizeof(statNames) == sizeof(jsNames));
        auto rc = stmt.Prepare(m_containerDb, "SELECT value FROM bcv_stat where name = ?");
        BeAssert (rc == BE_SQLITE_OK);
        UNUSED_VARIABLE(rc);
        BeJsNapiObject value(Env());
        for (int i = 0; i < sizeof(statNames) / sizeof(statNames[0]); i++) {
            stmt.BindText(1, statNames[i], Statement::MakeCopy::No);
            auto result = stmt.Step();
            if (result != BE_SQLITE_ROW) {
                value[jsNames[i]] = Utf8String("SQLITE Error: ") + m_containerDb.GetLastError();
            } else {
                // The actual values in the database are unsigned 64 bit integers, but there is no
                // direct way to pass those to JavaScript. So we use BeStringUtilities::FormatUInt64
                // to convert the integer to a hex string, which we can then pass to JS and let JS
                // convert to a number (or directly deal with the hex string).
                // Note: the IdToHex function isn't attached to in the cloud container database.
                uint64_t intValue = stmt.GetValueUInt64(0);
                if (intValue == 0) {
                    // FormatUInt64 intentionally doesn't apply the "0x" prefix when the value is 0.
                    // So we just special case that here.
                    value[jsNames[i]] = "0x0";
                } else {
                    static const size_t stringBufferLength = 19;
                    Utf8Char stringBuffer[stringBufferLength];
                    if (BeStringUtilities::FormatUInt64(stringBuffer, stringBufferLength, intValue, HexFormatOptions::IncludePrefix) != 0) {
                        value[jsNames[i]] = stringBuffer;
                    } else {
                        value[jsNames[i]] = Utf8String("Error converting to hex string");
                    }
                }
            }
            stmt.Reset();
            stmt.ClearBindings();
        }
        stmt.Finalize();

        if (addClientInformation) {
            rc = stmt.Prepare(m_containerDb, "SELECT SUM(nclient), SUM(nprefetch), SUM(ntrans) from bcv_database");

            BeAssert(rc == BE_SQLITE_OK);
            if ((stmt.Step()) == BE_SQLITE_ROW) {
                value["totalClients"] = stmt.GetValueInt(0);
                value["ongoingPrefetches"] = stmt.GetValueInt(1);
                value["activeClients"] = stmt.GetValueInt(2);
            }
            stmt.Finalize();
            rc = stmt.Prepare(m_containerDb, "SELECT COUNT(*) from bcv_container");
            BeAssert(rc == BE_SQLITE_OK);
            if ((stmt.Step()) == BE_SQLITE_ROW) {
                value["attachedContainers"] = stmt.GetValueInt(0);
            }
        }
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
        return QueueWorker(info, [=, this]() { return CloudContainer::CopyDatabase(fromName, toName); });
    }

    Napi::Value DeleteDatabase(NapiInfoCR info) {
        RequireWriteLock();
        REQUIRE_ARGUMENT_STRING(0, dbName);
        return QueueWorker(info, [=, this]() { return CloudContainer::DeleteDatabase(dbName); });
    }

    void InitializeContainer(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(0, opts);
        JsCloudUtil handle;
        auto result = handle.Init(*this);
        if (result.IsSuccess()) {
            bool checksumName = boolMember(opts, JSON_NAME(checksumBlockNames), false);
            result = handle.InitializeContainer(checksumName ? 24 : 16, requireInt(opts, JSON_NAME(blockSize)));
        }
        if (result.m_status != BE_SQLITE_OK)
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), result.m_error.c_str(), (DbResult)result.m_status);
    }

    Napi::Value GetWriteLockExpiryTime(NapiInfoCR) {
        RequireConnected();
        BeJsDocument lockedBy;
        ReadWriteLock(lockedBy); 
        // Returns empty string if writeLock is empty.
        return Napi::String::New(Env(), lockedBy[JSON_NAME(expires)].asString());
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

    /**
     * Get the date(s) returned by the server in the http header when the bcv_kv blob was fetched.
     * This avoids any dependence on the local computer's time for detecting expired write locks.
     */
    DateTime GetServerTime(bool lastMod = false) {
        auto timeStr = lastMod ? "last-modified" : "date";
        Statement stmt;
        auto rc = stmt.Prepare(m_containerDb, "SELECT value FROM bcv_kv_meta WHERE name=?");
        BeAssert(rc == BE_SQLITE_OK);
        stmt.BindText(1, timeStr, Statement::MakeCopy::No);
        rc = stmt.Step();
        if (rc == BE_SQLITE_ROW) {
            auto date = DateTime::FromString(stmt.GetValueText(0));
            if (date.IsValid())
                return date;
        }
        // if the server didn't include a "Date" field, just fall back to local time.
        Utf8PrintfString warning("server did not return valid [%s] in http response.", timeStr);
        NativeLogging::Logging::LogMessage("CloudSqlite", NativeLogging::SEVERITY::LOG_WARNING, warning.c_str());
        return DateTime::GetCurrentTimeUtc();
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
        if (DateTime::CompareResult::EarlierThan == DateTime::Compare(expiresAt, GetServerTime())) {
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

    Utf8String GetServerDateString(uint64_t offsetMilliseconds) {
        uint64_t serverTimeMs = 0;
        GetServerTime().ToJulianDay(serverTimeMs);
        return DateTime::FromJulianDay(serverTimeMs + offsetMilliseconds, DateTime::Info::CreateForDateTime(DateTime::Kind::Utc)).ToString();
    }

    void AcquireWriteLock(Utf8StringCR user) {
        RequireConnected();
        if (!m_writeable)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), "container is not writeable", IModelJsNativeErrorKey::NotOpenForWrite);

        m_containerDb.TryExecuteSql("BEGIN");
        CheckLock(); // throws if already locked by another user

        m_lockExpireSeconds = std::min((int) (12*SECONDS_PER_HOUR), std::max((int)SECONDS_PER_HOUR, m_lockExpireSeconds));
        BeJsDocument lockedBy;
        lockedBy[JSON_NAME(guid)] = m_cache->m_guid;
        lockedBy[JSON_NAME(user)] = user;
        lockedBy[JSON_NAME(expires)] = GetServerDateString(m_lockExpireSeconds * 1000);

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
                THROW_JS_BE_SQLITE_EXCEPTION(Env(), "not authorized to obtain write lock", BE_SQLITE_AUTH);
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), "cannot obtain write lock", rc);
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
        lastLock["time"] = GetServerTime(true).ToString();
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
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), stat.m_error.c_str(), (DbResult)stat.m_status);
    }

    void ReleaseWriteLock(NapiInfoCR info) {
        RequireConnected();
        if (!m_writeLockHeld)
            return;

        auto stat = CloudContainer::UploadChanges();
        if (stat.IsSuccess())
            stat = ClearWriteLock();

        if (!stat.IsSuccess())
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), stat.m_error.c_str(), (DbResult)stat.m_status);
    }

    void AbandonChanges(NapiInfoCR info) {
        RequireConnected();

        CloudResult stat;
        if (m_writeLockHeld)
            stat = ClearWriteLock();
        if (stat.IsSuccess())
            stat = CloudContainer::RevertChanges();

        if (!stat.IsSuccess())
            THROW_JS_BE_SQLITE_EXCEPTION(Env(), stat.m_error.c_str(), (DbResult)stat.m_status);
    }

    static bool IsInstance(Napi::Object val) { return val.InstanceOf(Constructor().Value()); }
    Napi::Value IsConnected(NapiInfoCR info) { return Napi::Boolean::New(Env(), IsContainerConnected()); }
    Napi::Value IsWriteable(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_writeable); }
    Napi::Value IsPublic(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_isPublic); }
    Napi::Value HasWriteLock(NapiInfoCR info) { return Napi::Boolean::New(Env(), m_writeLockHeld); }
    Napi::Value GetAccessToken(NapiInfoCR info) { return Napi::String::New(Env(), m_accessToken); }
    Napi::Value GetContainerId(NapiInfoCR info) { return Napi::String::New(Env(), m_containerId); }
    Napi::Value GetBaseUri(NapiInfoCR info) { return Napi::String::New(Env(), m_baseUri); }
    Napi::Value GetStorageType(NapiInfoCR info) { return Napi::String::New(Env(), m_storageType); }
    Napi::Value GetLogId(NapiInfoCR info) { return Napi::String::New(Env(), m_logId); }
    Napi::Value GetAlias(NapiInfoCR info) { return Napi::String::New(Env(), m_alias); }
    void SetAccessToken(NapiInfoCR info, Napi::Value const& value) { m_accessToken = value.As<Napi::String>().Utf8Value(); }

    static void Init(Napi::Env env, Napi::Object exports) {
        static constexpr Utf8CP className = "CloudContainer";
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, className, {
            InstanceAccessor<&JsCloudContainer::GetAccessToken, &JsCloudContainer::SetAccessToken>("accessToken"),
            InstanceAccessor<&JsCloudContainer::GetBaseUri>("baseUri"),
            InstanceAccessor<&JsCloudContainer::GetAlias>("alias"),
            InstanceAccessor<&JsCloudContainer::GetContainerId>("containerId"),
            InstanceAccessor<&JsCloudContainer::GetLogId>("logId"),
            InstanceAccessor<&JsCloudContainer::GetNumCleanupBlocks>("garbageBlocks"),
            InstanceAccessor<&JsCloudContainer::GetStorageType>("storageType"),
            InstanceAccessor<&JsCloudContainer::HasLocalChangesJs>("hasLocalChanges"),
            InstanceAccessor<&JsCloudContainer::HasWriteLock>("hasWriteLock"),
            InstanceAccessor<&JsCloudContainer::IsConnected>("isConnected"),
            InstanceAccessor<&JsCloudContainer::IsWriteable>("isWriteable"),
            InstanceAccessor<&JsCloudContainer::IsPublic>("isPublic"),
            InstanceAccessor<&JsCloudContainer::GetBlockSize>("blockSize"),
            InstanceAccessor<&JsCloudContainer::GetWriteLockExpiryTime>("writeLockExpires"),
            InstanceMethod<&JsCloudContainer::AbandonChanges>("abandonChanges"),
            InstanceMethod<&JsCloudContainer::AcquireWriteLockJs>("acquireWriteLock"),
            InstanceMethod<&JsCloudContainer::ClearWriteLockJs>("clearWriteLock"),
            InstanceMethod<&JsCloudContainer::Connect>("connect"),
            InstanceMethod<&JsCloudContainer::CopyDatabase>("copyDatabase"),
            InstanceMethod<&JsCloudContainer::DeleteDatabase>("deleteDatabase"),
            InstanceMethod<&JsCloudContainer::Disconnect>("disconnect"),
            InstanceMethod<&JsCloudContainer::InitializeContainer>("initializeContainer"),
            InstanceMethod<&JsCloudContainer::PollManifest>("checkForChanges"),
            InstanceMethod<&JsCloudContainer::QueryDatabase>("queryDatabase"),
            InstanceMethod<&JsCloudContainer::QueryDatabaseHash>("queryDatabaseHash"),
            InstanceMethod<&JsCloudContainer::QueryDatabases>("queryDatabases"),
            InstanceMethod<&JsCloudContainer::QueryHttpLog>("queryHttpLog"),
            InstanceMethod<&JsCloudContainer::QueryBcvStats>("queryBcvStats"),
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
struct CancellableCloudSqliteJob : Napi::ObjectWrap<CancellableCloudSqliteJob> {
    // Async Worker that does download on another thread.
    struct Worker : Napi::AsyncWorker {
        enum JobType { Upload = 0, Download = 1, Cleanup = 2 };
        struct JsDownloadJob : JsCloudUtil { // runs on AsyncWorker thread
            mutable BeMutex m_mutex;
            uint64_t m_nDone = 0;
            uint64_t m_nTotal = 0;
            bool m_abort = false; // set from DownloadV2Checkpoint::CancelDownload
            bool m_stopAndSaveProgress = false;
            int _OnProgress(uint64_t nDone, uint64_t nTotal) override {
                BeMutexHolder holder(m_mutex);
                m_nDone = nDone;
                m_nTotal = nTotal;
                return m_abort ? BE_SQLITE_ABORT : m_stopAndSaveProgress ? BE_SQLITE_DONE : BE_SQLITE_OK;
            }
        };

        Napi::ObjectReference m_request;
        JobType m_jobType;
        JsDownloadJob m_job;
        Utf8String m_dbName;
        Utf8String m_localFile;
        int m_nSeconds;
        bool m_findOrphanedBlocks;

        void Execute() override { // worker thread
            auto stat = m_jobType == JobType::Upload ? m_job.UploadDatabase(m_localFile, m_dbName) 
            : m_jobType == JobType::Download ? m_job.DownloadDatabase(m_dbName, m_localFile)
            : m_job.CleanDeletedBlocks(m_nSeconds, m_findOrphanedBlocks);
            if (stat.m_status == BE_SQLITE_ABORT)
                SetError("cancelled");
            else if (stat.m_status != BE_SQLITE_OK)
                SetError(stat.m_error);
        }

        void OnOK() override { // main thread
            auto* request = CancellableCloudSqliteJob::Unwrap(m_request.Value());
            request->m_promise.Resolve(Napi::Number::New(Env(), 0));
            request->m_pending = nullptr;
        }

        void OnError(Napi::Error const& error) override { // main thread
            auto* request = CancellableCloudSqliteJob::Unwrap(m_request.Value());
            request->m_promise.Reject(error.Value());
            request->m_pending = nullptr;
        }

        Worker(JobType jobType, JsCloudContainer& container, Napi::Object args, CancellableCloudSqliteJob const& request) : Napi::AsyncWorker(args.Env()) {
            if (jobType == JobType::Upload || jobType == JobType::Cleanup)
                container.RequireWriteLock();
            m_request.Reset(request.Value(), 1);
            m_jobType = jobType;
            bool debugLogging = false;
            if (m_jobType == JobType::Upload || m_jobType == JobType::Download) {
                m_localFile = requireString(args, JSON_NAME(localFileName));
                m_dbName = requireString(args, JSON_NAME(dbName));
            }
            else if (m_jobType == JobType::Cleanup) {
                m_findOrphanedBlocks = requireBool(args, JSON_NAME(findOrphanedBlocks));
                m_nSeconds = requireInt(args, JSON_NAME(nSeconds));
                debugLogging = requireBool(args, JSON_NAME(debugLogging));
            }

            auto stat = m_job.Init(container, debugLogging ? 1 : 0, intMember(args, JSON_NAME(nRequests), 0), intMember(args, JSON_NAME(httpTimeout), 0));
            if (!stat.IsSuccess())
                THROW_JS_BE_SQLITE_EXCEPTION(Env(), stat.m_error.c_str(), (DbResult)stat.m_status);
        }

        ~Worker() { m_request.Reset(); }
    };

    DEFINE_CONSTRUCTOR;
    Napi::Promise::Deferred m_promise;
    Worker* m_pending = nullptr;

    void CheckStillPending(Napi::Env env) {
        if (m_pending == nullptr)
            THROW_JS_IMODEL_NATIVE_EXCEPTION(env, "transfer already completed", IModelJsNativeErrorKey::BadArg);
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

    // If this is called and some blocks have begun to be deleted, then the job will stop and upload the manifest reflecting which blocks have been deleted.
    // For uploads and downloads this will abort the transfer without saving.
    void StopAndSaveProgress(NapiInfoCR info) {
        CheckStillPending(info.Env());
        m_pending->m_job.m_stopAndSaveProgress = true;
    }

    void handleDefaultArgsForCleanupJob(NapiInfoCR info) {
        REQUIRE_ARGUMENT_ANY_OBJ(2, args);
        auto optionsForCleanup = BeJsConst(info[2]);
        if (!args.Has("debugLogging"))
            args.Set("debugLogging", Napi::Boolean::New(info.Env(), false));
        if (!args.Has("findOrphanedBlocks"))
            args.Set("findOrphanedBlocks", Napi::Boolean::New(info.Env(), true));
        if (!args.Has("nSeconds"))
            args.Set("nSeconds", Napi::Number::New(info.Env(), 3600));
    }

    CancellableCloudSqliteJob(NapiInfoCR info) : Napi::ObjectWrap<CancellableCloudSqliteJob>(info), m_promise(Napi::Promise::Deferred::New(info.Env())) {
        REQUIRE_ARGUMENT_STRING(0, direction);
        REQUIRE_ARGUMENT_ANY_OBJ(1, jsContainer);
        if (!JsCloudContainer::IsInstance(jsContainer))
            THROW_JS_IMODEL_NATIVE_EXCEPTION(Env(), "invalid container argument", IModelJsNativeErrorKey::BadArg);
        auto container = Napi::ObjectWrap<JsCloudContainer>::Unwrap(jsContainer);

        REQUIRE_ARGUMENT_ANY_OBJ(2, args);
        if (direction == "cleanup") {
            handleDefaultArgsForCleanupJob(info);
        }
        Value().Set(JSON_NAME(promise), m_promise.Promise()); // holds a reference to the promise to keep it alive
        m_pending = new Worker(direction.Equals("upload") ? Worker::JobType::Upload : direction.Equals("download") ? Worker::JobType::Download : Worker::JobType::Cleanup, *container, args, *this);
        m_pending->Queue(); // takes ownership of m_pending, deletes on completion
    }

    static void Init(Napi::Env& env, Napi::Object exports) {
        static constexpr Utf8CP className = "CancellableCloudSqliteJob";
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, className, {
            InstanceMethod<&CancellableCloudSqliteJob::CancelTransfer>("cancelTransfer"),
            InstanceMethod<&CancellableCloudSqliteJob::GetProgress>("getProgress"),
            InstanceMethod<&CancellableCloudSqliteJob::StopAndSaveProgress>("stopAndSaveProgress"),
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
        THROW_JS_IMODEL_NATIVE_EXCEPTION(arg.Env(), "invalid container argument", IModelJsNativeErrorKey::BadArg);

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
                THROW_JS_BE_SQLITE_EXCEPTION(Env(), "error initializing prefetch", rc);
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
    CancellableCloudSqliteJob::Init(env, exports);
    JsCloudCache::Init(env, exports);
    JsCloudContainer::Init(env, exports);
    JsCloudPrefetch::Init(env, exports);
    #ifndef BENTLEY_WIN32
        // Ignore SIGPIPE to prevent crashes in CloudSQLite/curl.
        signal(SIGPIPE, SIG_IGN);
    #endif
}

} // end namespace IModelJsNative