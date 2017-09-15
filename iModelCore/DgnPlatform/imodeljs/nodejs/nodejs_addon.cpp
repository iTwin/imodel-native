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
#include <memory>
#include "suppress_warnings.h"

#include <nan.h>
#undef X_OK // node\uv-win.h defines this, and then folly/portability/Unistd.h re-defines it.

#include <node.h>
#include <node_buffer.h>
#include <node_version.h>
#include <json/value.h>
#include "../imodeljs.h"
#include <ECObjects/ECSchema.h>
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <rapidjson/rapidjson.h>

#define REQUIRE_DB_TO_BE_OPEN \
    if (!m_db->m_dgndb.IsValid())\
        {\
        m_status = DgnDbStatus::NotOpen;\
        return;\
        }


#define REQUIRE_ARGUMENT_STRING(i, var, errcode)                        \
    if (info.Length() <= (i) || !info[i]->IsString()) {                         \
        ResolveArgumentError(info, errcode);                            \
        return;                                                                 \
    }                                                                           \
    Nan::Utf8String var(info[i]);

#define REQUIRE_ARGUMENT_STRING_SYNC(i, var, errcode)                   \
    if (info.Length() <= (i) || !info[i]->IsString()) {                         \
        info.GetReturnValue().Set(NodeUtils::CreateErrorObject(errcode));          \
        return;                                                                 \
    }                                                                           \
    Nan::Utf8String var(info[i]);

#define REQUIRE_ARGUMENT_OBJ(i, T, var, errcode, errmsg)                        \
    if (info.Length() <= (i) || !T::HasInstance(info[i])) {                     \
        ResolveArgumentError(info, errcode, errmsg);                            \
        return;                                                                 \
    }                                                                           \
    T* var = Nan::ObjectWrap::Unwrap<T>(info[i].As<Object>());

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default, errcode)             \
    int var;                                                                    \
    if (info.Length() <= (i)) {                                                 \
        var = (default);                                                        \
    }                                                                           \
    else if (info[i]->IsInt32()) {                                              \
        var = Nan::To<int32_t>(info[i]).FromJust();                             \
    }                                                                           \
    else {                                                                      \
        ResolveArgumentError(info, errcode);                            \
        return;                                                                 \
    }

#define OPTIONAL_ARGUMENT_BOOLEAN(i, var, default, errcode)             \
    bool var;                                                                   \
    if (info.Length() <= (i)) {                                                 \
        var = (default);                                                        \
    }                                                                           \
    else if (info[i]->IsBoolean()) {                                            \
        var = Nan::To<bool>(info[i]).FromJust();                                \
    }                                                                           \
    else {                                                                      \
        ResolveArgumentError(info, errcode);                            \
        return;                                                                 \
    }

#define OPTIONAL_ARGUMENT_STRING(i, var, errcode)                       \
    v8::Local<v8::String> localString;                                          \
    if (info.Length() <= (i) || (info[i]->IsUndefined() || info[i]->IsNull()))  \
        localString = v8::String::Empty(info.GetIsolate());                     \
    else if (info[i]->IsString())                                               \
        localString = info[i]->ToString();                                      \
    else {                                                                      \
        ResolveArgumentError(info, errcode);                            \
        return;                                                                 \
    }                                                                           \
    Nan::Utf8String var(localString);


USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_EC
using namespace v8;
using namespace node;

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct NodeUtils
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      09/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    static v8::Local<v8::Object> CreateSuccessObject(v8::Local<v8::Value> const& goodVal)
        {
        v8::Local<v8::Object> retObj = Nan::New<v8::Object>();
        retObj->Set(Nan::New("result").ToLocalChecked(), goodVal);
        return retObj;
        }
        
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      09/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename STATUSTYPE>
    static v8::Local<v8::Object> CreateErrorObject(STATUSTYPE errCode)
        {
        v8::Local<v8::Object> error = Nan::New<v8::Object>();
        error->Set(Nan::New("status").ToLocalChecked(), Nan::New((int) errCode));

        v8::Local<v8::Object> retObj = Nan::New<v8::Object>();
        retObj->Set(Nan::New("error").ToLocalChecked(), error);
        return retObj;
        }
    };

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

    static void ResolveArgumentError(Nan::NAN_METHOD_ARGS_TYPE& info, STATUSTYPE errorStatus)
        {
        Nan::HandleScope scope;
        auto resolver = v8::Promise::Resolver::New(info.GetIsolate());
        info.GetReturnValue().Set(resolver->GetPromise());
        v8::Local<v8::Object> retObj = NodeUtils::CreateErrorObject(errorStatus);
        resolver->Resolve(retObj);
        }

    void ScheduleAndReturnPromise(Nan::NAN_METHOD_ARGS_TYPE& info)
        {
        auto resolver = v8::Promise::Resolver::New(info.GetIsolate());
        m_resolver = new T_ResolverPersistent(resolver);
        Nan::AsyncQueueWorker(this);
        info.GetReturnValue().Set(resolver->GetPromise());
        }

    v8::Local<v8::Object> CreateErrorObject()
        {
        BeAssert(_HadError());
        return NodeUtils::CreateErrorObject(m_status);
        }

    v8::Local<v8::Object> CreateSuccessObject()
        {
        BeAssert(!_HadError());
        return NodeUtils::CreateSuccessObject(GetResultOrUndefined());
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

    virtual bool _HadError() = 0;
    void WorkComplete() override {if (_HadError()) {SetErrorMessage("error");} Nan::AsyncWorker::WorkComplete();}

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

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, dbname, BE_SQLITE_ERROR);
            (new CreateDbWorker(db, *dbname))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (m_addon->m_ecdb.IsValid() && m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR_AlreadyOpen;
                return;
                }

            m_addon->m_ecdb = IModelJs::CreateECDb(m_status, m_dbPathname);
            if (!m_addon->m_ecdb.IsValid())
                m_addon->m_ecdb = nullptr;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
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

            REQUIRE_ARGUMENT_STRING(0, dbname, BE_SQLITE_ERROR);
            OPTIONAL_ARGUMENT_INTEGER(1, mode, (int) BeSQLite::Db::OpenMode::Readonly, BE_SQLITE_ERROR);
            (new OpenDbWorker(db, *dbname, (BeSQLite::Db::OpenMode)mode))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (m_addon->m_ecdb.IsValid() && m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR_AlreadyOpen;
                return;
                }

            m_addon->m_ecdb = IModelJs::OpenECDb(m_status, m_dbPathname, m_mode);
            if (!m_addon->m_ecdb.IsValid() || m_status != BE_SQLITE_OK)
                m_addon->m_ecdb = nullptr;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
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
                return;
                }
            m_addon->m_ecdb->CloseDb();
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct SaveChangesWorker : WorkerBase<DbResult>
        {
        Utf8String m_changeSetName; // input

        SaveChangesWorker(NodeAddonECDb *addon, Utf8CP changeSetName) : WorkerBase(addon, BE_SQLITE_OK), m_changeSetName(changeSetName) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            OPTIONAL_ARGUMENT_STRING(0, changeSetName, BE_SQLITE_ERROR);
            (new SaveChangesWorker(db, *changeSetName))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = m_addon->m_ecdb->SaveChanges(m_changeSetName.empty() ? nullptr : m_changeSetName.c_str());
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
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
                return;
                }

            m_status = m_addon->m_ecdb->AbandonChanges();
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct ImportSchemaWorker : WorkerBase<DbResult>
        {
        BeFileName m_schemaPathname; // input

        ImportSchemaWorker(NodeAddonECDb *addon, Utf8CP schemaPathname) : WorkerBase(addon, BE_SQLITE_OK), m_schemaPathname(schemaPathname, true) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, schemaPathname, BE_SQLITE_ERROR);
            (new ImportSchemaWorker(db, *schemaPathname))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::ImportSchema(*m_addon->m_ecdb, m_schemaPathname);
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct InsertInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstance; // input
        Utf8String m_insertedId; // output
        
        InsertInstanceWorker(NodeAddonECDb *addon, Nan::Utf8String const& strInstance) : WorkerBase(addon, DbResult::BE_SQLITE_OK), m_jsonInstance(Json::Value::From(*strInstance, *strInstance + strInstance.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, strInstance, BE_SQLITE_ERROR);
            (new InsertInstanceWorker(db, strInstance))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::InsertInstance(m_insertedId, *m_addon->m_ecdb, m_jsonInstance);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_insertedId.c_str()).ToLocalChecked();
            return true;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct UpdateInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstance;

        UpdateInstanceWorker(NodeAddonECDb *addon, Nan::Utf8String const& strInstance) : WorkerBase(addon, DbResult::BE_SQLITE_OK), m_jsonInstance(Json::Value::From(*strInstance, *strInstance + strInstance.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, strInstance, BE_SQLITE_ERROR);
            (new UpdateInstanceWorker(db, strInstance))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::UpdateInstance(*m_addon->m_ecdb, m_jsonInstance);
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
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

            REQUIRE_ARGUMENT_STRING(0, instanceKey, BE_SQLITE_ERROR);
            (new ReadInstanceWorker(db, instanceKey))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::ReadInstance(m_jsonInstance, *m_addon->m_ecdb, m_jsonInstanceKey);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_jsonInstance.ToString().c_str()).ToLocalChecked();
            return true;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct DeleteInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstanceKey; // input  

        DeleteInstanceWorker(NodeAddonECDb* db, Nan::Utf8String const& strInstanceKey) : WorkerBase(db, DbResult::BE_SQLITE_OK), m_jsonInstanceKey(Json::Value::From(*strInstanceKey, *strInstanceKey + strInstanceKey.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECDb* db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, instanceKey, BE_SQLITE_ERROR);
            (new DeleteInstanceWorker(db, instanceKey))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::DeleteInstance(*m_addon->m_ecdb, m_jsonInstanceKey);
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
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

            REQUIRE_ARGUMENT_STRING(0, instanceKey, BE_SQLITE_ERROR);
            (new ContainsInstanceWorker(db, instanceKey))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::ContainsInstance(m_containsInstance, *m_addon->m_ecdb, m_jsonInstanceKey);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_containsInstance);
            return true;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
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

            REQUIRE_ARGUMENT_STRING(0, ecsql, BE_SQLITE_ERROR);
            OPTIONAL_ARGUMENT_STRING(1, strBindings, BE_SQLITE_ERROR);

            (new ExecuteQueryWorker(db, *ecsql, *strBindings))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            JsECDbR ecdb = *m_addon->m_ecdb;
            BeSqliteDbMutexHolder serializeAccess(ecdb); // hold mutex, so that we have a chance to get last ECDb error message

            CachedECSqlStatementPtr stmt = ecdb.GetPreparedECSqlStatement(m_ecsql.c_str());
            if (!stmt.IsValid())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::ExecuteQuery(m_rowsJson, *stmt, m_bindings);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_rowsJson.ToString().c_str()).ToLocalChecked();
            return true;
            }
        bool _HadError() override {return m_status != BE_SQLITE_DONE && m_status != BE_SQLITE_ROW ;}
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

            REQUIRE_ARGUMENT_STRING(0, ecsql, BE_SQLITE_ERROR);
            OPTIONAL_ARGUMENT_BOOLEAN(1, isInsertStmt, false, BE_SQLITE_ERROR);
            OPTIONAL_ARGUMENT_STRING(2, strBindings, BE_SQLITE_ERROR);

            (new ExecuteStatementWorker(db, *ecsql, isInsertStmt, *strBindings))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            JsECDbR ecdb = *m_addon->m_ecdb;
            BeSqliteDbMutexHolder serializeAccess(ecdb); // hold mutex, so that we have a chance to get last ECDb error message

            CachedECSqlStatementPtr stmt = ecdb.GetPreparedECSqlStatement(m_ecsql.c_str());
            if (!stmt.IsValid())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }
            
            m_status = IModelJs::ExecuteStatement(m_instanceId, *stmt, m_isInsertStmt, m_bindings);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_instanceId.c_str()).ToLocalChecked();
            return true;
            }
        bool _HadError() override 
            {
            switch(m_status) 
                {
                case BE_SQLITE_OK: 
                case BE_SQLITE_ROW: 
                case BE_SQLITE_DONE: 
                    return false;
                }
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

    static NAN_GETTER(OpenGetter)
        {
        NodeAddonECDb *db = Nan::ObjectWrap::Unwrap<NodeAddonECDb>(info.This());
        info.GetReturnValue().Set(db->m_ecdb.IsValid() && db->m_ecdb->IsDbOpen());
        }
};
struct SimpleRulesetLocater : RefCounted<RuleSetLocater>
{
private:
    Utf8String m_rulesetId;
    mutable PresentationRuleSetPtr m_ruleset;
 
protected:
    SimpleRulesetLocater(Utf8String rulesetId) : m_rulesetId(rulesetId) {}
    int _GetPriority() const override {return 100;}
    bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override
        {
        if (m_ruleset.IsNull())
            {
            m_ruleset = PresentationRuleSet::CreateInstance(m_rulesetId, 1, 0, false, "", "", "", false);
            m_ruleset->AddPresentationRule(*new ContentRule("", 1, false));
            m_ruleset->GetContentRules().back()->GetSpecificationsR().push_back(new SelectedNodeInstancesSpecification(1, false, "", "", true));
            }
        return bvector<PresentationRuleSetPtr>{m_ruleset};
        }
    bvector<Utf8String> _GetRuleSetIds() const override {return bvector<Utf8String>{m_rulesetId};}
    void _InvalidateCache(Utf8CP rulesetId) override
        {
        if (nullptr == rulesetId || m_rulesetId.Equals(rulesetId))
            m_ruleset = nullptr;
        }
 
public:
    static RefCountedPtr<SimpleRulesetLocater> Create(Utf8String rulesetId) {return new SimpleRulesetLocater(rulesetId);}
};

//=======================================================================================
// Projects the DgnDb class into JS
//! @bsiclass
//=======================================================================================
struct NodeAddonDgnDb : Nan::ObjectWrap
{
    Dgn::DgnDbPtr m_dgndb;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;
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
            REQUIRE_ARGUMENT_STRING(0, dbname, BE_SQLITE_ERROR);
            OPTIONAL_ARGUMENT_INTEGER(1, mode, (int) DgnDb::OpenMode::Readonly, BE_SQLITE_ERROR);
            (new OpenDgnDbWorker(db, *dbname, (DgnDb::OpenMode)mode))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            m_status = IModelJs::OpenDgnDb(m_db->m_dgndb, m_dbname, m_mode);

            BeFileName assetsDir = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
            BeFileName tempDir = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
            m_db->m_presentationManager = std::unique_ptr<RulesDrivenECPresentationManager>(new RulesDrivenECPresentationManager(RulesDrivenECPresentationManager::Paths(assetsDir,tempDir)));
            m_db->m_presentationManager->GetLocaters().RegisterLocater(*SimpleRulesetLocater::Create("Ruleset_Id"));
            IECPresentationManager::RegisterImplementation( m_db->m_presentationManager.get());
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
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

            REQUIRE_ARGUMENT_STRING(0, s, DgnDbStatus::BadRequest);
            REQUIRE_ARGUMENT_STRING(1, c, DgnDbStatus::BadRequest);
            (new GetECClassMetaData(db, *s, *c))->ScheduleAndReturnPromise(info);
            }

        static NAN_METHOD(ExecuteSync)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING_SYNC(0, s, DgnDbStatus::BadRequest);
            REQUIRE_ARGUMENT_STRING_SYNC(1, c, DgnDbStatus::BadRequest);

            GetECClassMetaData worker(db, *s, *c);
            worker.Execute();
            info.GetReturnValue().Set(worker._HadError() ? worker.CreateErrorObject() : worker.CreateSuccessObject());
            }

        void Execute() override
            {
            REQUIRE_DB_TO_BE_OPEN
            m_status = IModelJs::GetECClassMetaData(m_metaDataJson, GetDgnDb(), m_ecSchema.c_str(), m_ecClass.c_str());
            }

        bool _GetResult(v8::Local<v8::Value>& result) override {result = Nan::New(m_metaDataJson.ToString().c_str()).ToLocalChecked(); return true;}
        bool _HadError() override {return m_status != DgnDbStatus::Success;}
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

            REQUIRE_ARGUMENT_STRING(0, opts, DgnDbStatus::BadRequest);
            (new GetElementWorker(db, opts))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            REQUIRE_DB_TO_BE_OPEN
            m_status = IModelJs::GetElement(m_elementJson, GetDgnDb(), m_opts);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_elementJson.ToString().c_str()).ToLocalChecked();
            return true;
            }
        bool _HadError() override {return m_status != DgnDbStatus::Success;}
        };

    //=======================================================================================
    // Get the properties for a DgnModel
    //! @bsiclass
    //=======================================================================================
    struct GetModelWorker : WorkerBase<DgnDbStatus>
        {
        Json::Value m_opts;       // input
        Json::Value m_modelJson;  // ouput
        
        GetModelWorker(NodeAddonDgnDb* db, Nan::Utf8String const& inOpts) : WorkerBase(db, DgnDbStatus::Success), m_opts(Json::Value::From(*inOpts, *inOpts+inOpts.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

            REQUIRE_ARGUMENT_STRING(0, opts, DgnDbStatus::BadRequest);
            (new GetModelWorker(db, opts))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            REQUIRE_DB_TO_BE_OPEN
            m_status = IModelJs::GetModel(m_modelJson, GetDgnDb(), m_opts);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_modelJson.ToString().c_str()).ToLocalChecked();
            return true;
            }
        bool _HadError() override {return m_status != DgnDbStatus::Success;}
        };

    //=======================================================================================
    // insert a new element -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    static NAN_METHOD(InsertElementSync)
        {
        Nan::HandleScope scope;
        NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

        if (!db->m_dgndb.IsValid())
            {
            info.GetReturnValue().Set(NodeUtils::CreateErrorObject(DgnDbStatus::NotOpen));
            return;
            }
        REQUIRE_ARGUMENT_STRING_SYNC(0, elemPropsJsonStr, DgnDbStatus::BadRequest);

        Json::Value elemProps = Json::Value::From(*elemPropsJsonStr, *elemPropsJsonStr+elemPropsJsonStr.length());
        Json::Value elemIdJsonObj;
        auto status = IModelJs::InsertElement(elemIdJsonObj, *db->m_dgndb, elemProps);

        v8::Local<v8::Object> ret;
        if (DgnDbStatus::Success != status)
            ret = NodeUtils::CreateErrorObject(status);
        else
            {
            auto elemIdStrJsObj = Nan::New(elemIdJsonObj.ToString().c_str()).ToLocalChecked();
            ret = NodeUtils::CreateSuccessObject(elemIdStrJsObj);
            }

        info.GetReturnValue().Set(ret);
        }

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
            Utf8String m_elementIdStr;
            Utf8String m_exportedJson;
            GetElementPropertiesForDisplayWorker(NodeAddonDgnDb* db, Utf8CP id) : WorkerBase(db,  DgnDbStatus::Success),m_elementIdStr(id) {}
            static NAN_METHOD(Start)
                {
                Nan::HandleScope scope;
                NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());

                 REQUIRE_ARGUMENT_STRING(0, id, DgnDbStatus::BadRequest);
                (new GetElementPropertiesForDisplayWorker(db,*id))->ScheduleAndReturnPromise(info);
                }

            void Execute() override
                {
                if (!m_db->m_dgndb.IsValid())
                    {
                    m_status = DgnDbStatus::NotOpen;
                    return;
                    }

                ECInstanceId elemId(ECInstanceId::FromString(m_elementIdStr.c_str()).GetValueUnchecked());
                if (!elemId.IsValid())
                    {
                    m_status = DgnDbStatus::BadElement;
                    return;
                    }
                
                CachedECSqlStatementPtr stmt = m_db->m_dgndb->GetPreparedECSqlStatement("SELECT ECClassId FROM biscore.Element WHERE ECInstanceId = ?");
                if (!stmt.IsValid())
                    {
                    m_status = DgnDbStatus::SQLiteError;
                    return;
                    }
                
                stmt->BindId(1, elemId);
                if (stmt->Step() != BE_SQLITE_ROW)
                    {
                    m_status = DgnDbStatus::SQLiteError;
                    return;
                    }

                ECClassId ecclassId = stmt->GetValueId<ECClassId>(0);
                ECInstanceNodeKeyPtr nodeKey = ECInstanceNodeKey::Create(ecclassId, elemId);
                NavNodeKeyList keyList;
                keyList.push_back(nodeKey);
                INavNodeKeysContainerCPtr selectedNodeKeys = NavNodeKeyListContainer::Create(keyList);
                SelectionInfo selection ("iModelJS", false, *selectedNodeKeys);
                RulesDrivenECPresentationManager::ContentOptions options ("Items", false);
                if ( m_db->m_presentationManager == nullptr)
                    {
                    m_status = DgnDbStatus::BadArg;
                    return;                
                    }
                ContentDescriptorCPtr descriptor = m_db->m_presentationManager->GetContentDescriptor(*m_db->m_dgndb, ContentDisplayType::PropertyPane, selection, options.GetJson());
                if (descriptor.IsNull())
                    {
                    m_status = DgnDbStatus::BadArg;
                    }
                PageOptions pageOptions;
                pageOptions.SetPageStart(0);
                pageOptions.SetPageSize(0);
                ContentCPtr content = m_db->m_presentationManager->GetContent(*m_db->m_dgndb, *descriptor, selection, pageOptions, options.GetJson());
                if (content.IsNull())
                    {
                    m_status = DgnDbStatus::BadArg;
                    return;                        
                    }                

                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                content->AsJson().Accept(writer);
                m_exportedJson = buffer.GetString();
                }

            bool _GetResult(v8::Local<v8::Value>& result) override
                {
                result = Nan::New(m_exportedJson.c_str()).ToLocalChecked();
                return true;
                }
            bool _HadError() override {return m_status != DgnDbStatus::Success;}
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

            REQUIRE_ARGUMENT_STRING(0, ecsql, BE_SQLITE_ERROR);
            (new ExecuteQueryWorker(db, *ecsql))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (!m_db->m_dgndb.IsValid())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            DgnDbR dgndb = GetDgnDb();
            BeSqliteDbMutexHolder serializeAccess(dgndb); // hold mutex, so that we have a chance to get last ECDb error message

            CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(m_ecsql.c_str());
            if (!stmt.IsValid())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = IModelJs::ExecuteQuery(m_rowsJson, *stmt, Json::nullValue);
            }

        bool _GetResult(v8::Local<v8::Value>& result) override
            {
            result = Nan::New(m_rowsJson.ToString().c_str()).ToLocalChecked();
            return true;
            }
        bool _HadError() override 
            {
            switch(m_status) 
                {
                case BE_SQLITE_OK: 
                case BE_SQLITE_ROW: 
                case BE_SQLITE_DONE: 
                    return false;
                }
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

    static NAN_METHOD(CloseDgnDb)
        {
        Nan::HandleScope scope;
        NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());
        db->m_dgndb = nullptr;
        }
    
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
        Nan::SetPrototypeMethod(t, "closeDgnDb", CloseDgnDb);
        Nan::SetPrototypeMethod(t, "getElement", GetElementWorker::Start);
        Nan::SetPrototypeMethod(t, "getModel", GetModelWorker::Start);
        Nan::SetPrototypeMethod(t, "insertElementSync", InsertElementSync);
        Nan::SetPrototypeMethod(t, "getElementPropertiesForDisplay", GetElementPropertiesForDisplayWorker::Start);
        Nan::SetPrototypeMethod(t, "getECClassMetaData", GetECClassMetaData::Start);
        Nan::SetPrototypeMethod(t, "getECClassMetaDataSync", GetECClassMetaData::ExecuteSync);
        Nan::SetPrototypeMethod(t, "executeQuery", ExecuteQueryWorker::Start);

        Nan::SetAccessor(t->InstanceTemplate(), Nan::New("IsDbOpen").ToLocalChecked(), OpenGetter);

        s_constructor_template.Reset(t);

        Nan::Set(target, Nan::New("DgnDb").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());
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
