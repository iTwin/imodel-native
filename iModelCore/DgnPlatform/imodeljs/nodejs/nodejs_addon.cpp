/*--------------------------------------------------------------------------------------+
|
|     $Source: imodeljs/nodejs/nodejs_addon.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <functional>
#include <queue>
#include <sys/types.h>
#include <stdint.h>

#include "suppress_warnings.h"

#include <nan.h>
#undef X_OK // node\uv-win.h defines this, and then folly/portability/Unistd.h re-defines it.

#include <node.h>
#include <node_buffer.h>
#include <node_version.h>

#include <json/value.h>

#include "../imodeljs.h"

#define REQUIRE_ARGUMENT_STRING(i, var, errcode, errmsg)                        \
    if (info.Length() <= (i) || !info[i]->IsString()) {                         \
        ResolveArgumentError(info, errcode, errmsg);                            \
        return;                                                                 \
    }                                                                           \
    Nan::Utf8String var(info[i]);

#define REQUIRE_ARGUMENT_STRING_SYNC(i, var, errcode, errmsg)                   \
    if (info.Length() <= (i) || !info[i]->IsString()) {                         \
        info.GetReturnValue().Set(CreateErrorObject(errcode, errmsg));          \
        return;                                                                 \
    }                                                                           \
    Nan::Utf8String var(info[i]);

#define REQUIRE_ARGUMENT_OBJ(i, T, var, errcode, errmsg)                        \
    if (info.Length() <= (i) || !T::HasInstance(info[i])) {                     \
        ResolveArgumentError(info, errcode, errmsg);                            \
        return;                                                                 \
    }                                                                           \
    T* var = Nan::ObjectWrap::Unwrap<T>(info[i].As<Object>());

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default, errcode, errmsg)             \
    int var;                                                                    \
    if (info.Length() <= (i)) {                                                 \
        var = (default);                                                        \
    }                                                                           \
    else if (info[i]->IsInt32()) {                                              \
        var = Nan::To<int32_t>(info[i]).FromJust();                             \
    }                                                                           \
    else {                                                                      \
        ResolveArgumentError(info, errcode, errmsg);                            \
        return;                                                                 \
    }

#define OPTIONAL_ARGUMENT_BOOLEAN(i, var, default, errcode, errmsg)             \
    bool var;                                                                   \
    if (info.Length() <= (i)) {                                                 \
        var = (default);                                                        \
    }                                                                           \
    else if (info[i]->IsBoolean()) {                                            \
        var = Nan::To<bool>(info[i]).FromJust();                                \
    }                                                                           \
    else {                                                                      \
        ResolveArgumentError(info, errcode, errmsg);                            \
        return;                                                                 \
    }

#define OPTIONAL_ARGUMENT_STRING(i, var, errcode, errmsg)                       \
    v8::Local<v8::String> localString;                                          \
    if (info.Length() <= (i) || (info[i]->IsUndefined() || info[i]->IsNull()))  \
        localString = v8::String::Empty(info.GetIsolate());                     \
    else if (info[i]->IsString())                                               \
        localString = info[i]->ToString();                                      \
    else {                                                                      \
        ResolveArgumentError(info, errcode, errmsg);                            \
        return;                                                                 \
    }                                                                           \
    Nan::Utf8String var(localString);


USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
using namespace v8;
using namespace node;

//=======================================================================================
//! Base class for helper functions. All such functions have the following in common:
//  -- All return Promises and execute asynchronously
//! @bsiclass
//=======================================================================================
template<typename STATUSTYPE>
struct DgnDbPromiseAsyncWorkerBase : Nan::AsyncWorker
{
    typedef Nan::Persistent<v8::Promise::Resolver> T_ResolverPersistent;
    typedef v8::Local<v8::Promise::Resolver> T_ResolverLocal;

    T_ResolverPersistent* m_resolver;
    STATUSTYPE m_status;
    Utf8String m_errmsg;

    DgnDbPromiseAsyncWorkerBase(STATUSTYPE defaultStatus) : Nan::AsyncWorker(nullptr), m_resolver(nullptr), m_status(defaultStatus) {}
    ~DgnDbPromiseAsyncWorkerBase()
        {
        if (nullptr == m_resolver)
            return;

        v8::Isolate::GetCurrent()->RunMicrotasks(); // *** WIP_NODE - work around bug in node - https://groups.google.com/forum/#!topic/nodejs/aAKtzaWDrPo
        m_resolver->Reset();
        delete m_resolver;
        }

    STATUSTYPE GetStatus() { return m_status; }

    static void ResolveArgumentError(Nan::NAN_METHOD_ARGS_TYPE& info, STATUSTYPE errorStatus, Utf8CP errorMessage)
        {
        Nan::HandleScope scope;
        auto resolver = v8::Promise::Resolver::New(info.GetIsolate());
        info.GetReturnValue().Set(resolver->GetPromise());
        v8::Local<v8::Object> retObj = CreateErrorObject(errorStatus, errorMessage);
        resolver->Resolve(retObj);
        }

    void ScheduleAndReturnPromise(Nan::NAN_METHOD_ARGS_TYPE& info)
        {
        auto resolver = v8::Promise::Resolver::New(info.GetIsolate());
        m_resolver = new T_ResolverPersistent(resolver);
        Nan::AsyncQueueWorker(this);
        info.GetReturnValue().Set(resolver->GetPromise());
        }

    static v8::Local<v8::Object> CreateSuccessObject(v8::Local<v8::Value> const& goodVal)
        {
        v8::Local<v8::Object> retObj = Nan::New<v8::Object>();
        retObj->Set(Nan::New("result").ToLocalChecked(), goodVal);
        return retObj;
        }

        
    static v8::Local<v8::Object> CreateErrorObject(STATUSTYPE errCode, Utf8CP errMessage)
        {
        v8::Local<v8::Object> error = Nan::New<v8::Object>();
        error->Set(Nan::New("status").ToLocalChecked(), Nan::New((int) errCode));
        error->Set(Nan::New("message").ToLocalChecked(), Nan::New(errMessage).ToLocalChecked());

        v8::Local<v8::Object> retObj = Nan::New<v8::Object>();
        retObj->Set(Nan::New("error").ToLocalChecked(), error);
        return retObj;
        }

    v8::Local<v8::Object> CreateErrorObject()
        {
        BeAssert(HadError());
        return CreateErrorObject(m_status, m_errmsg.c_str());
        }

    v8::Local<v8::Object> CreateSuccessObject()
        {
        BeAssert(!HadError());
        return CreateSuccessObject(GetResultOrUndefined());
        }

    void HandleOKCallback() override
        {
        Nan::HandleScope scope;
        Nan::New(*m_resolver)->Resolve(CreateSuccessObject());
        }

    void HandleErrorCallback() override
        {
        Nan::HandleScope scope;
        Nan::New(*m_resolver)->Resolve(CreateErrorObject());
        }

    // Mark this worker as being in a failed state.
    void SetupErrorReturn()
        {
        // Calling SetErrorMessage with non-Null is the only way to tell Nan::AsyncWorker to call HandleErrorCallback instead of HandleOKCallback
        if (m_errmsg.empty())
            m_errmsg = "error";
        SetErrorMessage(m_errmsg.c_str());
        }

    bool HadError() { return nullptr != ErrorMessage(); }

    // Implement one of these methods to return true if there are results after initializing the result string
    virtual bool _GetResult(v8::Local<v8::Value>& result) { return false; }

    v8::Local<v8::Value> GetResultOrUndefined()
        {
        v8::Local<v8::Value> result;
        if (!_GetResult(result))
            result = Nan::Undefined();
        return result;
        }
};

//=======================================================================================
// Projects the ECDb class into JS
//! @bsiclass
//=======================================================================================
struct NodeAddonECDb : Nan::ObjectWrap
{
    template <typename STATUSTYPE>
    struct WorkerBase : DgnDbPromiseAsyncWorkerBase<STATUSTYPE>
        {
        NodeAddonECDb *m_addon; // input
        WorkerBase(NodeAddonECDb *addon, STATUSTYPE defaultStatus) : DgnDbPromiseAsyncWorkerBase<STATUSTYPE>(defaultStatus), m_addon(addon) { m_addon->AddRef(); }
        ~WorkerBase() { m_addon->Release(); }
        };

    struct CreateDbWorker : WorkerBase<DbResult>
        {
        BeFileName m_dbPathname; // input

        CreateDbWorker(NodeAddonECDb *addon, Utf8CP dbPathname) : WorkerBase(addon, BE_SQLITE_OK), m_dbPathname(dbPathname, true) {}

        static NAN_METHOD(NodeAddonECDb::CreateDbWorker::Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, dbname, BE_SQLITE_ERROR, "Argument 0 must be a string giving the pathname of the ecdb");
            (new CreateDbWorker(db, *dbname))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (m_addon->m_ecdb.IsValid() && m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR_AlreadyOpen;
                m_errmsg = "Close existing Db before creating a new one with the same handle";
                SetupErrorReturn();
                return;
                }

            m_addon->m_ecdb = IModelJs::CreateECDb(m_status, m_errmsg, m_dbPathname);
            if (!m_addon->m_ecdb.IsValid())
                {
                m_addon->m_ecdb = nullptr;
                SetupErrorReturn();
                }
            }
        };

    struct OpenDbWorker : WorkerBase<DbResult>
        {
        BeFileName m_dbPathname;       // input
        BeSQLite::Db::OpenMode m_mode; // input

        OpenDbWorker(NodeAddonECDb *addon, Utf8CP dbPathname, BeSQLite::Db::OpenMode mode) : WorkerBase(addon, BE_SQLITE_OK), m_dbPathname(dbPathname, true), m_mode(mode) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, dbname, BE_SQLITE_ERROR, "Argument 0 must be a string giving the pathname of the ecdb");
            OPTIONAL_ARGUMENT_INTEGER(1, mode, (int) BeSQLite::Db::OpenMode::Readonly, BE_SQLITE_ERROR, "Argument 1 must be an integer specifying the open mode (BE_SQLITE_OPEN_MODE_...)");
            (new OpenDbWorker(db, *dbname, (BeSQLite::Db::OpenMode)mode))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (m_addon->m_ecdb.IsValid() && m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR_AlreadyOpen;
                m_errmsg = "Close existing Db before opening a new one with the same handle";
                SetupErrorReturn();
                return;
                }

            m_addon->m_ecdb = IModelJs::OpenECDb(m_status, m_errmsg, m_dbPathname, m_mode);
            if (!m_addon->m_ecdb.IsValid() || m_status != BE_SQLITE_OK)
                {
                m_addon->m_ecdb = nullptr;
                SetupErrorReturn();
                }
            }
        };

    struct CloseDbWorker : WorkerBase<DbResult>
        {
        CloseDbWorker(NodeAddonECDb *addon) : WorkerBase(addon, BE_SQLITE_OK) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            (new CloseDbWorker(db))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }
            m_addon->m_ecdb->CloseDb();
            }
        };

    struct SaveChangesWorker : WorkerBase<DbResult>
        {
        Utf8String m_changeSetName; // input

        SaveChangesWorker(NodeAddonECDb *addon, Utf8CP changeSetName) : WorkerBase(addon, BE_SQLITE_OK), m_changeSetName(changeSetName) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            OPTIONAL_ARGUMENT_STRING(0, changeSetName, BE_SQLITE_ERROR, "Argument 0 must be the name of the change set to be saved");
            (new SaveChangesWorker(db, *changeSetName))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = m_addon->m_ecdb->SaveChanges(m_changeSetName.empty() ? nullptr : m_changeSetName.c_str());
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }
        };

    struct AbandonChangesWorker : WorkerBase<DbResult>
        {
        AbandonChangesWorker(NodeAddonECDb *addon) : WorkerBase(addon, BE_SQLITE_OK) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            (new AbandonChangesWorker(db))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = m_addon->m_ecdb->AbandonChanges();
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }
        };

    struct ImportSchemaWorker : WorkerBase<DbResult>
        {
        BeFileName m_schemaPathname; // input

        ImportSchemaWorker(NodeAddonECDb *addon, Utf8CP schemaPathname) : WorkerBase(addon, BE_SQLITE_OK), m_schemaPathname(schemaPathname, true) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, schemaPathname, BE_SQLITE_ERROR, "Argument 0 must be a string giving the pathname of the ECSchema");
            (new ImportSchemaWorker(db, *schemaPathname))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::ImportSchema(m_errmsg, *m_addon->m_ecdb, m_schemaPathname);
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }
        };

    struct InsertInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstance; // input
        ECInstanceId m_insertedId; //output

        InsertInstanceWorker(NodeAddonECDb *addon, Nan::Utf8String const& strInstance) : WorkerBase(addon, DbResult::BE_SQLITE_OK), m_jsonInstance(Json::Value::From(*strInstance, *strInstance + strInstance.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, strInstance, BE_SQLITE_ERROR, "Argument 0 must be a Json string of an instance to be inserted");
            (new InsertInstanceWorker(db, strInstance))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::InsertInstance(m_errmsg, m_insertedId, *m_addon->m_ecdb, m_jsonInstance);
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_insertedId.ToString().c_str()).ToLocalChecked();
            return true;
            }
        };

    struct UpdateInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstance;

        UpdateInstanceWorker(NodeAddonECDb *addon, Nan::Utf8String const& strInstance) : WorkerBase(addon, DbResult::BE_SQLITE_OK), m_jsonInstance(Json::Value::From(*strInstance, *strInstance + strInstance.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, strInstance, BE_SQLITE_ERROR, "Argument 0 must be a Json string");
            (new UpdateInstanceWorker(db, strInstance))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::UpdateInstance(m_errmsg, *m_addon->m_ecdb, m_jsonInstance);
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }
        };

    struct ReadInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstanceKey; // input   
        Json::Value m_jsonInstance; // output

        ReadInstanceWorker(NodeAddonECDb* db, Nan::Utf8String const& strInstanceKey) : WorkerBase(db, DbResult::BE_SQLITE_OK), m_jsonInstanceKey(Json::Value::From(*strInstanceKey, *strInstanceKey + strInstanceKey.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb* db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, instanceKey, BE_SQLITE_ERROR, "Argument 0 must be a JSON string representing an ECInstanceKey");
            (new ReadInstanceWorker(db, instanceKey))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::ReadInstance(m_errmsg, m_jsonInstance, *m_addon->m_ecdb, m_jsonInstanceKey);
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            Utf8String resultStr = Json::FastWriter::ToString(m_jsonInstance);
            result = Nan::New(resultStr.c_str()).ToLocalChecked();
            return true;
            }
        };


    struct DeleteInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstanceKey; // input  

        DeleteInstanceWorker(NodeAddonECDb* db, Nan::Utf8String const& strInstanceKey) : WorkerBase(db, DbResult::BE_SQLITE_OK), m_jsonInstanceKey(Json::Value::From(*strInstanceKey, *strInstanceKey + strInstanceKey.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb* db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, instanceKey, BE_SQLITE_ERROR, "Argument 0 must be a JSON string representing an ECInstanceKey");
            (new DeleteInstanceWorker(db, instanceKey))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::DeleteInstance(m_errmsg, *m_addon->m_ecdb, m_jsonInstanceKey);
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }
        };

    struct ContainsInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstanceKey; // input  
        bool m_containsInstance; // output

        ContainsInstanceWorker(NodeAddonECDb* db, Nan::Utf8String const& strInstanceKey) : WorkerBase(db, DbResult::BE_SQLITE_OK), m_jsonInstanceKey(Json::Value::From(*strInstanceKey, *strInstanceKey + strInstanceKey.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb* db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, instanceKey, BE_SQLITE_ERROR, "Argument 0 must be a JSON string representing an ECInstanceKey");
            (new ContainsInstanceWorker(db, instanceKey))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::ContainsInstance(m_errmsg, m_containsInstance, *m_addon->m_ecdb, m_jsonInstanceKey);
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_containsInstance);
            return true;
            }
        };

    struct ExecuteQueryWorker : WorkerBase<DbResult>
        {
        Utf8String m_ecsql; // input
        Json::Value m_bindings; // input
        Json::Value m_rowsJson;  // output

        ExecuteQueryWorker(NodeAddonECDb* db, Utf8CP ecsql, Utf8CP strBindings) : WorkerBase(db, BE_SQLITE_OK), m_ecsql(ecsql), m_rowsJson(Json::arrayValue),
            m_bindings(Utf8String::IsNullOrEmpty(strBindings) ? Json::nullValue : Json::Value::From(strBindings)) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb* db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, ecsql, BE_SQLITE_ERROR, "Argument 0 must be an ECSql string");
            OPTIONAL_ARGUMENT_STRING(1, strBindings, BE_SQLITE_ERROR, "Argument 1 must be a JSON string specifying the bindings");

            (new ExecuteQueryWorker(db, *ecsql, *strBindings))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            JsECDbR ecdb = *m_addon->m_ecdb;
            BeSqliteDbMutexHolder serializeAccess(ecdb); // hold mutex, so that we have a chance to get last ECDb error message

            CachedECSqlStatementPtr stmt = ecdb.GetPreparedECSqlStatement(m_ecsql.c_str());
            if (!stmt.IsValid())
                {
                m_errmsg = IModelJs::GetLastEcdbIssue();
                m_status = BE_SQLITE_ERROR;
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::ExecuteQuery(m_errmsg, m_rowsJson, *stmt, m_bindings);
            if (m_status != BE_SQLITE_DONE)
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            Utf8String resultStr = Json::FastWriter::ToString(m_rowsJson);
            result = Nan::New(resultStr.c_str()).ToLocalChecked();
            return true;
            }
        };

    struct ExecuteStatementWorker : WorkerBase<DbResult>
        {
        Utf8String m_ecsql; // input
        bool m_isInsertStmt; // input
        Json::Value m_bindings; // input
        Utf8String m_instanceId; // output

        ExecuteStatementWorker(NodeAddonECDb* db, Utf8CP ecsql, bool isInsertStatement, Utf8CP strBindings) : WorkerBase(db, BE_SQLITE_OK), m_ecsql(ecsql),
            m_bindings(Utf8String::IsNullOrEmpty(strBindings) ? Json::nullValue : Json::Value::From(strBindings)), m_isInsertStmt(isInsertStatement)
            {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb* db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, ecsql, BE_SQLITE_ERROR, "Argument 0 must be an ECSql string");
            OPTIONAL_ARGUMENT_BOOLEAN(1, isInsertStmt, false, BE_SQLITE_ERROR, "Argument 1 must be a flag that specifies it's an insert statement");
            OPTIONAL_ARGUMENT_STRING(2, strBindings, BE_SQLITE_ERROR, "Argument 2 must be a JSON string specifying the bindings");

            (new ExecuteStatementWorker(db, *ecsql, isInsertStmt, *strBindings))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "ECDb must be open to complete this operation";
                SetupErrorReturn();
                return;
                }

            JsECDbR ecdb = *m_addon->m_ecdb;
            BeSqliteDbMutexHolder serializeAccess(ecdb); // hold mutex, so that we have a chance to get last ECDb error message

            CachedECSqlStatementPtr stmt = ecdb.GetPreparedECSqlStatement(m_ecsql.c_str());
            if (!stmt.IsValid())
                {
                m_errmsg = IModelJs::GetLastEcdbIssue();
                m_status = BE_SQLITE_ERROR;
                SetupErrorReturn();
                return;
                }
            
            m_status = IModelJs::ExecuteStatement(m_errmsg, m_instanceId, *stmt, m_isInsertStmt, m_bindings);
            if (m_status != BE_SQLITE_DONE)
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_instanceId.c_str()).ToLocalChecked();
            return true;
            }
        };

private:
    JsECDbPtr m_ecdb;
    mutable Utf8String m_lastECDbIssue;
        
public:
    NodeAddonECDb() : Nan::ObjectWrap(), m_lastECDbIssue("") {}
    ~NodeAddonECDb() {}

    void AddRef() { this->Ref(); }
    void Release() { this->Unref(); }

    static Nan::Persistent<FunctionTemplate> s_constructor_template;

    static bool HasInstance(Local<Value> val)
        {
        Nan::HandleScope scope;
        if (!val->IsObject())
            return false;
        Local<Object> obj = val.As<Object>();
        return Nan::New(s_constructor_template)->HasInstance(obj);
        }

    static NAN_MODULE_INIT(Init)
        {
        printf("In NodeAddonECDb::Init method");

        Nan::HandleScope scope;

        Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(Nan::New("ECDb").ToLocalChecked());

        Nan::SetPrototypeMethod(t, "createDb", CreateDbWorker::Start);
        Nan::SetPrototypeMethod(t, "openDb", OpenDbWorker::Start);
        Nan::SetPrototypeMethod(t, "closeDb", CloseDbWorker::Start);
        Nan::SetPrototypeMethod(t, "saveChanges", SaveChangesWorker::Start);
        Nan::SetPrototypeMethod(t, "abandonChanges", AbandonChangesWorker::Start);
        Nan::SetPrototypeMethod(t, "importSchema", ImportSchemaWorker::Start);
        Nan::SetPrototypeMethod(t, "insertInstance", InsertInstanceWorker::Start);
        Nan::SetPrototypeMethod(t, "readInstance", ReadInstanceWorker::Start);
        Nan::SetPrototypeMethod(t, "updateInstance", UpdateInstanceWorker::Start);
        Nan::SetPrototypeMethod(t, "deleteInstance", DeleteInstanceWorker::Start);
        Nan::SetPrototypeMethod(t, "containsInstance", ContainsInstanceWorker::Start);
        Nan::SetPrototypeMethod(t, "executeQuery", ExecuteQueryWorker::Start);
        Nan::SetPrototypeMethod(t, "executeStatement", ExecuteStatementWorker::Start);

        Nan::SetAccessor(t->InstanceTemplate(), Nan::New("IsDbOpen").ToLocalChecked(), OpenGetter);

        s_constructor_template.Reset(t);

        Nan::Set(target, Nan::New("ECDb").ToLocalChecked(),
                    Nan::GetFunction(t).ToLocalChecked());
        }

    static NAN_METHOD(New)
        {
        if (!info.IsConstructCall())
            {
            return Nan::ThrowTypeError("Use the new operator to create new NodeAddonECDb objects");
            }

        NodeAddonECDb *db = new NodeAddonECDb();
        db->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        }

    static NAN_GETTER(NodeAddonECDb::OpenGetter)
        {
        NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());
        info.GetReturnValue().Set(db->m_ecdb.IsValid() && db->m_ecdb->IsDbOpen());
        }
};

//=======================================================================================
// Projects the DgnDb class into JS
//! @bsiclass
//=======================================================================================
struct NodeAddonDgnDb : Nan::ObjectWrap
{
    Dgn::DgnDbPtr m_dgndb;

    static Nan::Persistent<FunctionTemplate> s_constructor_template;

    //  Check if val is really a NodeAddonDgnDb peer object
    static bool HasInstance(Local<Value> val)
        {
        Nan::HandleScope scope;
        if (!val->IsObject()) return false;
        Local<Object> obj = val.As<Object>();
        return Nan::New(s_constructor_template)->HasInstance(obj);
        }

    //=======================================================================================
    // Base class for DgnDb "worker" helper classes that implement various DgnDb methods. 
    //! @bsiclass
    //=======================================================================================
    template<typename STATUSTYPE>
    struct WorkerBase : DgnDbPromiseAsyncWorkerBase<STATUSTYPE>
        {
        NodeAddonDgnDb* m_db;   // input: the DgnDb peer object

        WorkerBase(NodeAddonDgnDb* db, STATUSTYPE defaultStatus) : DgnDbPromiseAsyncWorkerBase<STATUSTYPE>(defaultStatus), m_db(db) { m_db->AddRef(); }
        ~WorkerBase() { m_db->Release(); }

        bool IsDbOpen() const { return m_db->m_dgndb.IsValid(); }
        DgnDbR GetDgnDb() { return *m_db->m_dgndb; }
        };

    //=======================================================================================
    //  Opens a DgnDb
    //! @bsiclass
    //=======================================================================================
    struct OpenDgnDbWorker : WorkerBase<DbResult>
        {
        BeFileName m_dbname;    // input
        DgnDb::OpenMode m_mode; // input

        OpenDgnDbWorker(NodeAddonDgnDb* db, Utf8CP dbname, DgnDb::OpenMode mode) : WorkerBase(db, BE_SQLITE_OK), m_dbname(dbname, true), m_mode(mode) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());
            REQUIRE_ARGUMENT_STRING(0, dbname, BE_SQLITE_ERROR, "Argument 0 must be a string giving the name of the dgndb");
            OPTIONAL_ARGUMENT_INTEGER(1, mode, (int) DgnDb::OpenMode::Readonly, BE_SQLITE_ERROR, "Argument 1 must be an integer specifying the open mode (BE_SQLITE_OPEN_MODE_...)");
            (new OpenDgnDbWorker(db, *dbname, (DgnDb::OpenMode)mode))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            IModelJs::OpenDgnDb(m_status, m_errmsg, m_db->m_dgndb, m_dbname, m_mode);
            if (m_status != BE_SQLITE_OK)
                SetupErrorReturn();
            }
        };

    //=======================================================================================
    // Returns ECClass metadata
    //! @bsiclass
    //=======================================================================================
    struct GetECClassMetaData : WorkerBase<DgnDbStatus>
        {
        Utf8String m_ecSchema;            // input
        Utf8String m_ecClass;            // input
        Json::Value m_metaDataJson;  // ouput

        GetECClassMetaData(NodeAddonDgnDb* db, Utf8CP s, Utf8CP c) : WorkerBase(db, DgnDbStatus::Success), m_ecSchema(s), m_ecClass(c), m_metaDataJson(Json::objectValue) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, s, DgnDbStatus::BadRequest, "Argument 0 must be the schema name (as a string)");
            REQUIRE_ARGUMENT_STRING(1, c, DgnDbStatus::BadRequest, "Argument 1 must be the class name (as a string)");
            (new GetECClassMetaData(db, *s, *c))->ScheduleAndReturnPromise(info);
            }

        static NAN_METHOD(ExecuteSync)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING_SYNC(0, s, DgnDbStatus::BadRequest, "Argument 0 must be the schema name (as a string)");
            REQUIRE_ARGUMENT_STRING_SYNC(1, c, DgnDbStatus::BadRequest, "Argument 1 must be the class name (as a string)");

            GetECClassMetaData worker(db, *s, *c);
            worker.Execute();
            if (worker.HadError())
                info.GetReturnValue().Set(worker.CreateErrorObject());
            else
                info.GetReturnValue().Set(worker.CreateSuccessObject());
            }

        void Execute() override
            {
            if (!m_db->m_dgndb.IsValid())
                {
                m_status = DgnDbStatus::NotOpen;
                m_errmsg = "DgnDb must be open";
                SetupErrorReturn();
                return;
                }

            if (BSISUCCESS != IModelJs::GetECClassMetaData(m_status, m_errmsg, m_metaDataJson, GetDgnDb(), m_ecSchema.c_str(), m_ecClass.c_str()))
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            Utf8String resultStr = Json::FastWriter::ToString(m_metaDataJson);
            result = Nan::New(resultStr.c_str()).ToLocalChecked();
            return true;
            }
        };

    //=======================================================================================
    // Get the properties for a DgnElement
    //! @bsiclass
    //=======================================================================================
    struct GetElementWorker : WorkerBase<DgnDbStatus>
        {
        Json::Value m_opts;         // input
        Json::Value m_elementJson;  // ouput
        
        GetElementWorker(NodeAddonDgnDb* db, Nan::Utf8String const& inOpts) : WorkerBase(db, DgnDbStatus::Success), m_opts(Json::Value::From(*inOpts, *inOpts+inOpts.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, opts, DgnDbStatus::BadRequest, "Argument 0 must be a Json string");
            (new GetElementWorker(db, opts))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_db->m_dgndb.IsValid())
                {
                m_status = DgnDbStatus::NotOpen;
                m_errmsg = "DgnDb must be open";
                SetupErrorReturn();
                return;
                }

            if (BSISUCCESS != IModelJs::GetElement(m_status, m_errmsg, m_elementJson, GetDgnDb(), m_opts))
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            Utf8String resultStr = Json::FastWriter::ToString(m_elementJson);
            result = Nan::New(resultStr.c_str()).ToLocalChecked();
            return true;
            }
        };

    //=======================================================================================
    // Get the properties for a DgnModel
    //! @bsiclass
    //=======================================================================================
    struct GetModelWorker : WorkerBase<DgnDbStatus>
        {
        Json::Value m_opts;         // input
        Json::Value m_modelJson;  // ouput
        
        GetModelWorker(NodeAddonDgnDb* db, Nan::Utf8String const& inOpts) : WorkerBase(db, DgnDbStatus::Success), m_opts(Json::Value::From(*inOpts, *inOpts+inOpts.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, opts, DgnDbStatus::BadRequest, "Argument 0 must be a Json string");
            (new GetModelWorker(db, opts))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_db->m_dgndb.IsValid())
                {
                m_status = DgnDbStatus::NotOpen;
                m_errmsg = "DgnDb must be open";
                SetupErrorReturn();
                return;
                }

            if (BSISUCCESS != IModelJs::GetModel(m_status, m_errmsg, m_modelJson, GetDgnDb(), m_opts))
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            Utf8String resultStr = Json::FastWriter::ToString(m_modelJson);
            result = Nan::New(resultStr.c_str()).ToLocalChecked();
            return true;
            }
        };

    //=======================================================================================
    // Gets a JSON description of the properties of an element, suitable for display in a property browser. 
    // The returned properties are be organized by EC display "category" as specified by CustomAttributes.
    // Properties are identified by DisplayLabel, not name.
    // The property values are formatted according to formatting CAs.
    // The returned properties also include the properties of related instances as specified by RelatedItemsDisplaySpecification custom attributes in the ECSchemas.
    //! @bsiclass
    //=======================================================================================
    struct GetElementPropertiesForDisplayWorker : WorkerBase<DgnDbStatus>
        {
        Utf8String m_id;            // input
        Json::Value m_elementJson;  // ouput

        GetElementPropertiesForDisplayWorker(NodeAddonDgnDb* db, Utf8CP id) : WorkerBase(db, DgnDbStatus::Success), m_id(id), m_elementJson(Json::objectValue) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, id, DgnDbStatus::BadRequest, "Argument 0 must be a string representing the DgnElementId");
            (new GetElementPropertiesForDisplayWorker(db, *id))->ScheduleAndReturnPromise(info);
            }

        void Execute() override 
            {
            if (!m_db->m_dgndb.IsValid())
                {
                m_status = DgnDbStatus::NotOpen;
                m_errmsg = "DgnDb must be open";
                SetupErrorReturn();
                return;
                }

            if (BSISUCCESS != IModelJs::GetElementPropertiesForDisplay(m_status, m_errmsg, m_elementJson, GetDgnDb(), m_id.c_str()))
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            Utf8String resultStr = Json::FastWriter::ToString(m_elementJson);
            result = Nan::New(resultStr.c_str()).ToLocalChecked();
            return true;
            }
        };

    //=======================================================================================
    //  Execute a query and return all rows, if any
    //! @bsiclass
    //=======================================================================================
    struct ExecuteQueryWorker : WorkerBase<DbResult>
        {
        Utf8String m_ecsql;
        Json::Value m_rowsJson;  // output

        ExecuteQueryWorker(NodeAddonDgnDb* db, Utf8CP ecsql) : WorkerBase(db, BE_SQLITE_OK), m_ecsql(ecsql), m_rowsJson(Json::arrayValue) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, ecsql, BE_SQLITE_ERROR, "Argument 0 must be an ECSql string");
            (new ExecuteQueryWorker(db, *ecsql))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_db->m_dgndb.IsValid())
                {
                m_status = BE_SQLITE_ERROR;
                m_errmsg = "DgnDb must be open";
                SetupErrorReturn();
                return;
                }

            DgnDbR dgndb = GetDgnDb();
            BeSqliteDbMutexHolder serializeAccess(dgndb); // hold mutex, so that we have a chance to get last ECDb error message

            CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(m_ecsql.c_str());
            if (!stmt.IsValid())
                {
                m_errmsg = IModelJs::GetLastEcdbIssue();
                m_status = BE_SQLITE_ERROR;
                SetupErrorReturn();
                return;
                }

            m_status = IModelJs::ExecuteQuery(m_errmsg, m_rowsJson, *stmt, Json::nullValue);
            if (m_status != BE_SQLITE_DONE)
                SetupErrorReturn();
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            Utf8String resultStr = Json::FastWriter::ToString(m_rowsJson);
            result = Nan::New(resultStr.c_str()).ToLocalChecked();
            return true;
            }
        };

    //  Add a reference to this wrapper object, keeping it and its peer JS object alive.
    void AddRef() { this->Ref(); }

    //  Remove a reference from this wrapper object and its peer JS object .
    void Release() { this->Unref(); }

    //  Construct the native wrapper object itself
    NodeAddonDgnDb() : Nan::ObjectWrap() {}

    //  Destruct the native wrapper object itself
    ~NodeAddonDgnDb() {}

    //  Create a native wrapper object that is linked to a new JS object
    static NAN_METHOD(New)
        {
        if (!info.IsConstructCall())
            {
            return Nan::ThrowTypeError("Use the new operator to create new NodeAddonDgnDb objects");
            }

        NodeAddonDgnDb* db = new NodeAddonDgnDb();
        db->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        }

    //  Implements the DgnDb.open get method
    static NAN_GETTER(OpenGetter)
        {
        NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());
        info.GetReturnValue().Set(db->m_dgndb.IsValid() && db->m_dgndb->IsDbOpen());
        }

    //  Create projections
    static void Init(v8::Local<v8::Object> target)
        {
        Nan::HandleScope scope;

        Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(Nan::New("DgnDb").ToLocalChecked());

        Nan::SetPrototypeMethod(t, "openDgnDb", OpenDgnDbWorker::Start);
        Nan::SetPrototypeMethod(t, "getElement", GetElementWorker::Start);
        Nan::SetPrototypeMethod(t, "getModel", GetModelWorker::Start);
        Nan::SetPrototypeMethod(t, "getElementPropertiesForDisplay", GetElementPropertiesForDisplayWorker::Start);
        Nan::SetPrototypeMethod(t, "getECClassMetaData", GetECClassMetaData::Start);
        Nan::SetPrototypeMethod(t, "getECClassMetaDataSync", GetECClassMetaData::ExecuteSync);
        Nan::SetPrototypeMethod(t, "executeQuery", ExecuteQueryWorker::Start);

        Nan::SetAccessor(t->InstanceTemplate(), Nan::New("IsDbOpen").ToLocalChecked(), OpenGetter);

        s_constructor_template.Reset(t);

        Nan::Set(target, Nan::New("DgnDb").ToLocalChecked(),
                 Nan::GetFunction(t).ToLocalChecked());
        }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void registerModule(v8::Handle<v8::Object> target, v8::Handle<v8::Object> module)
    {
    Nan::HandleScope scope;
    auto isolate = module->GetIsolate();

    v8::Local<v8::Value> v8filename = module->Get(v8::String::NewFromUtf8(isolate, "filename"));
    v8::String::Utf8Value v8utf8(v8filename);
    BeFileName addondir = BeFileName(*v8utf8, true).GetDirectoryName();

    IModelJs::Initialize(addondir);
    NodeAddonDgnDb::Init(target);
    NodeAddonECDb::Init(target);
    }

Nan::Persistent<FunctionTemplate> NodeAddonDgnDb::s_constructor_template;
Nan::Persistent<FunctionTemplate> NodeAddonECDb::s_constructor_template;

NODE_MODULE(IModelJs, registerModule)
