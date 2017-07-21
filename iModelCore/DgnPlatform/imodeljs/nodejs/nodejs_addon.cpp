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

#define DGNDB_MUST_BE_OPEN(db) if (!(db)->m_dgndb.IsValid()) {return Nan::ThrowTypeError("DgnDb must be open");}

#define REQUIRE_ARGUMENT_STRING(i, var, msg)                                    \
    if (info.Length() <= (i) || !info[i]->IsString()) {                         \
        return Nan::ThrowTypeError(msg);                                        \
    }                                                                           \
    Nan::Utf8String var(info[i]);

#define REQUIRE_ARGUMENT_OBJ(i, T, var, msg)                                    \
    if (info.Length() <= (i) || !T ::HasInstance(info[i])) {                    \
        return Nan::ThrowTypeError(msg);                                        \
    }                                                                           \
    T* var = Nan::ObjectWrap::Unwrap<T>(info[i].As<Object>());


#define OPTIONAL_ARGUMENT_INTEGER(i, var, default, msg)                         \
    int var;                                                                    \
    if (info.Length() <= (i)) {                                                 \
        var = (default);                                                        \
    }                                                                           \
    else if (info[i]->IsInt32()) {                                              \
        var = Nan::To<int32_t>(info[i]).FromJust();                             \
    }                                                                           \
    else {                                                                      \
        return Nan::ThrowTypeError(msg);                                        \
    }

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
using namespace v8;
using namespace node;

//=======================================================================================
//! Base class for helper functions. All such functions have the following in common:
//  -- All return Promises and execute asynchronously, which they must resolve on success.
//  -- All return an error message if the function fails and the Promise is to be rejected.
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

    DgnDbPromiseAsyncWorkerBase(STATUSTYPE defaultStatus)
        : Nan::AsyncWorker(nullptr), m_resolver(nullptr), m_status(defaultStatus)
        {
        }

    ~DgnDbPromiseAsyncWorkerBase()
        {
        if (nullptr == m_resolver)
            return;

        v8::Isolate::GetCurrent()->RunMicrotasks(); // *** WIP_NODE - work around bug in node - https://groups.google.com/forum/#!topic/nodejs/aAKtzaWDrPo
        m_resolver->Reset();
        delete m_resolver;
        }

    STATUSTYPE GetStatus() {return m_status;}

    void ScheduleAndReturnPromise(Nan::NAN_METHOD_ARGS_TYPE& info)
        {
        auto resolver = v8::Promise::Resolver::New(info.GetIsolate());
        m_resolver = new T_ResolverPersistent(resolver);
        Nan::AsyncQueueWorker(this);
        info.GetReturnValue().Set(resolver->GetPromise());
        }

    void HandleOKCallback() override
        {
        Nan::HandleScope scope;
        auto resolver = Nan::New(*m_resolver);
        _ResolvePromise(resolver);
        }

    void HandleErrorCallback() override
        {
        Nan::HandleScope scope;

        v8::Local<v8::Object> obj = Nan::New<v8::Object>();
        obj->Set(Nan::New("error").ToLocalChecked(), Nan::New((int)m_status));
        obj->Set(Nan::New("message").ToLocalChecked(), Nan::New(m_errmsg.c_str()).ToLocalChecked());

        auto resolver = Nan::New(*m_resolver);
        resolver->Reject(obj);
        }


    // Mark this worker as being in a failed state.
    void Reject() 
        {
        // Calling SetErrorMessage with non-Null is the only way to tell Nan::AsyncWorker to call HandleErrorCallback instead of HandleOKCallback

        if (m_errmsg.empty())
            m_errmsg = "error";

        SetErrorMessage(m_errmsg.c_str());
        }

    bool HadError() {return nullptr != ErrorMessage();}

    virtual void _ResolvePromise(T_ResolverLocal&) = 0;
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
        
        WorkerBase(NodeAddonDgnDb* db, STATUSTYPE defaultStatus) : DgnDbPromiseAsyncWorkerBase<STATUSTYPE>(defaultStatus), m_db(db) {m_db->AddRef();}
        ~WorkerBase() {m_db->Release();}

        bool IsDbOpen() const {return m_db->m_dgndb.IsValid();}
        DgnDbR GetDgnDb() {return *m_db->m_dgndb;}
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
            REQUIRE_ARGUMENT_STRING(0, dbname, "Argument 0 must be a string giving the name of the dgndb");
            OPTIONAL_ARGUMENT_INTEGER(1, mode, (int)DgnDb::OpenMode::Readonly, "Argument 1 must be an integer specifying the open mode (BE_SQLITE_OPEN_MODE_...)");
            (new OpenDgnDbWorker(db, *dbname, (DgnDb::OpenMode)mode))->ScheduleAndReturnPromise(info);
            }
        
        void Execute() override
            {
            if (BSISUCCESS != imodeljs::OpenDgnDb(m_status, m_errmsg, m_db->m_dgndb, m_dbname, m_mode))
                Reject();
            }

        void _ResolvePromise(T_ResolverLocal& r) override {r->Resolve(v8::Integer::New(v8::Isolate::GetCurrent(), m_status));}
        };

    //=======================================================================================
    // Returns ECClass metadata
    //! @bsiclass
    //=======================================================================================
    // Return the results of calling an element's ToJson method
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
            DGNDB_MUST_BE_OPEN(db);
            REQUIRE_ARGUMENT_STRING(0, s, "Argument 0 must be the schema name (as a string)");
            REQUIRE_ARGUMENT_STRING(1, c, "Argument 1 must be the class name (as a string)");
            (new GetECClassMetaData(db, *s, *c))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (BSISUCCESS != imodeljs::GetECClassMetaData(m_status, m_errmsg, m_metaDataJson, GetDgnDb(), m_ecSchema.c_str(), m_ecClass.c_str()))
                Reject();
            }

        void _ResolvePromise(T_ResolverLocal& r) override
            {
            auto json = Json::FastWriter::ToString(m_metaDataJson);
            r->Resolve(Nan::New(json.c_str()).ToLocalChecked());
            }
        };

    //=======================================================================================
    // Returns the results of calling an element's ToJson method
    //! @bsiclass
    //=======================================================================================
    struct GetElementPropertiesWorker : WorkerBase<DgnDbStatus>
        {
        Json::Value m_opts;         // input
        Json::Value m_elementJson;  // ouput
        
        GetElementPropertiesWorker(NodeAddonDgnDb* db, Nan::Utf8String const& inOpts) : WorkerBase(db, DgnDbStatus::Success), m_opts(Json::Value::From(*inOpts, *inOpts+inOpts.length())) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonDgnDb* db = Nan::ObjectWrap::Unwrap<NodeAddonDgnDb>(info.This());
            DGNDB_MUST_BE_OPEN(db);
            REQUIRE_ARGUMENT_STRING(0, opts, "Argument 0 must be a Json string");
            (new GetElementPropertiesWorker(db, opts))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (BSISUCCESS != imodeljs::GetElementProperties(m_status, m_errmsg, m_elementJson, GetDgnDb(), m_opts))
                Reject();
            }

        void _ResolvePromise(T_ResolverLocal& r) override {r->Resolve(Nan::New(m_elementJson.ToString().c_str()).ToLocalChecked());}
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
            DGNDB_MUST_BE_OPEN(db);
            REQUIRE_ARGUMENT_STRING(0, id, "Argument 0 must be a string representing the DgnElementId");
            (new GetElementPropertiesForDisplayWorker(db, *id))->ScheduleAndReturnPromise(info);
            }

        void Execute() override 
            {
            if (BSISUCCESS != imodeljs::GetElementPropertiesForDisplay(m_status, m_errmsg, m_elementJson, GetDgnDb(), m_id.c_str()))
                Reject();
            }
        
        void _ResolvePromise(T_ResolverLocal& r) override
            {
            auto json = Json::FastWriter::ToString(m_elementJson);
            r->Resolve(Nan::New(json.c_str()).ToLocalChecked());
            }
        };

    //  Add a reference to this wrapper object, keeping it and its peer JS object alive.
    void AddRef() {this->Ref();}
    
    //  Remove a reference from this wrapper object and its peer JS object .
    void Release() {this->Unref();}

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
        Nan::SetPrototypeMethod(t, "getElement", GetElementPropertiesWorker::Start);
        Nan::SetPrototypeMethod(t, "getElementPropertiesForDisplay", GetElementPropertiesForDisplayWorker::Start);
        Nan::SetPrototypeMethod(t, "getECClassMetaData", GetECClassMetaData::Start);

        Nan::SetAccessor(t->InstanceTemplate(), Nan::New("IsDbOpen").ToLocalChecked(), OpenGetter);
    
        s_constructor_template.Reset(t);

        Nan::Set(target, Nan::New("DgnDb").ToLocalChecked(),
        Nan::GetFunction(t).ToLocalChecked());
        }

};

//=======================================================================================
// Projects ECSqlStatement into JS using V8 API and defines a number of methods
// that are implemented as libuv async tasks.
// @bsiclass
//=======================================================================================
struct NodeAddonECSqlStatement : Nan::ObjectWrap 
{
    NodeAddonDgnDb* m_db;
    CachedECSqlStatementPtr m_statement;

    DgnDbR GetDgnDb() {return *m_db->m_dgndb;}

    static Nan::Persistent<FunctionTemplate> s_constructor_template;

    //=======================================================================================
    // Base class for helper classes that implement ECSqlStatement methods.
    //! @bsiclass
    //=======================================================================================
    struct WorkerBase : DgnDbPromiseAsyncWorkerBase<DbResult>
        {
        NodeAddonECSqlStatement* m_stmt;    // input
        
        WorkerBase(NodeAddonECSqlStatement* s) : DgnDbPromiseAsyncWorkerBase<DbResult>(BE_SQLITE_OK), m_stmt(s) {m_stmt->AddRef();}
        ~WorkerBase() {m_stmt->Release();}

        DgnDbR GetDgnDb() {return m_stmt->GetDgnDb();}
        };

    //=======================================================================================
    //  Get a prepared ECSqlStatement
    //! @bsiclass
    //=======================================================================================
    struct PrepareWorker : WorkerBase 
        {
        Utf8String m_ecsql;                 // input

        PrepareWorker(NodeAddonECSqlStatement* stmt_, Utf8CP ecsql) : WorkerBase(stmt_), m_ecsql(ecsql) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECSqlStatement* stmt = Nan::ObjectWrap::Unwrap<NodeAddonECSqlStatement>(info.This());
            REQUIRE_ARGUMENT_STRING(0, ecsql, "Argument 0 must be a string containing the ECSql statement to prepare");
            (new PrepareWorker(stmt, *ecsql))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (BSISUCCESS != imodeljs::GetCachedECSqlStatement(m_status, m_errmsg, m_stmt->m_statement, GetDgnDb(), m_ecsql.c_str()))
                {
                m_status = DbResult::BE_SQLITE_ERROR;
                Reject();
                }
            }

        void _ResolvePromise(T_ResolverLocal& r) override {r->Resolve(m_stmt->handle());}
        };

    //=======================================================================================
    //  Step a prepared statement and return the first row, if any
    //! @bsiclass
    //=======================================================================================
    struct StepOnceWorker : WorkerBase
        {
        Json::Value m_rowJson;              // ouput

        StepOnceWorker(NodeAddonECSqlStatement* s) : WorkerBase(s), m_rowJson(Json::objectValue) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECSqlStatement* stmt = Nan::ObjectWrap::Unwrap<NodeAddonECSqlStatement>(info.This());
            if (info.Length() != 0)
                return Nan::ThrowTypeError("no arguments expected");  
            if (!stmt->m_statement.IsValid())
                return Nan::ThrowError("Statement is not valid");  
            (new StepOnceWorker(stmt))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (BSISUCCESS != imodeljs::StepStatementOnce(m_status, m_errmsg, m_rowJson, GetDgnDb(), *m_stmt->m_statement))
                Reject();
            }
        
        void _ResolvePromise(T_ResolverLocal& r) override
            {
            auto json = Json::FastWriter::ToString(m_rowJson);
            r->Resolve(Nan::New(json.c_str()).ToLocalChecked());
            }
        };

    //=======================================================================================
    //  Step a prepared statement and return all rows, if any
    //! @bsiclass
    //=======================================================================================
    struct StepAllWorker : WorkerBase
        {
        Json::Value m_rowsJson;             // ouput

        StepAllWorker(NodeAddonECSqlStatement* s) : WorkerBase(s), m_rowsJson(Json::arrayValue) {}

        static NAN_METHOD(Start)
            {
            Nan::HandleScope scope;
            NodeAddonECSqlStatement* stmt = Nan::ObjectWrap::Unwrap<NodeAddonECSqlStatement>(info.This());
            if (info.Length() != 0)
                return Nan::ThrowTypeError("no arguments expected");  
            if (!stmt->m_statement.IsValid())
                return Nan::ThrowError("Statement is not valid");  
            (new StepAllWorker(stmt))->ScheduleAndReturnPromise(info);
            }

        void Execute() override
            {
            if (BSISUCCESS != imodeljs::StepStatementAll(m_status, m_errmsg, m_rowsJson, GetDgnDb(), *m_stmt->m_statement))
                Reject();
            }

        void _ResolvePromise(T_ResolverLocal& r) override
            {
            auto json = Json::FastWriter::ToString(m_rowsJson);
            r->Resolve(Nan::New(json.c_str()).ToLocalChecked());
            }
        };


    //  Add a reference to this wrapper object, keeping it and its peer JS object alive.
    void AddRef() {this->Ref();}

    //  Relase reference to this wrapper object and its peer JS object
    void Release() {this->Unref();}

    // Construct the native wrapper object
    NodeAddonECSqlStatement(NodeAddonDgnDb* db_) : Nan::ObjectWrap(), m_db(db_) {m_db->AddRef();}

    // Destruct the wrapper object, releasing its reference to the DgnDb wrapper object (and its JS obj)
    ~NodeAddonECSqlStatement() {m_db->Release();}

    //  Create a native wrapper object that is linked to a new JS object
    static NAN_METHOD(New)
        {
        if (!info.IsConstructCall()) 
            return Nan::ThrowTypeError("Use the new operator to create new ECSqlStatement objects");

        REQUIRE_ARGUMENT_OBJ(0, NodeAddonDgnDb, db, "Argument 0 must be DgnDb object");
        NodeAddonECSqlStatement* stmt = new NodeAddonECSqlStatement(db);
        stmt->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
        }

    //  Project the ECSqlStatement class into JS
    static void Init(v8::Local<v8::Object> target)
        {
        Nan::HandleScope scope;

        Local<FunctionTemplate> t = Nan::New<FunctionTemplate>(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);
        t->SetClassName(Nan::New("ECSqlStatement").ToLocalChecked());

        Nan::SetPrototypeMethod(t, "prepare",   PrepareWorker::Start);
        Nan::SetPrototypeMethod(t, "step_once", StepOnceWorker::Start);
        Nan::SetPrototypeMethod(t, "step_all",  StepAllWorker::Start);

        s_constructor_template.Reset(t);
        Nan::Set(target, Nan::New("ECSqlStatement").ToLocalChecked(), Nan::GetFunction(t).ToLocalChecked());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void registerModule(v8::Handle<v8::Object> target, v8::Handle<v8::Object> module) {
    Nan::HandleScope scope;
    auto isolate = module->GetIsolate();

    v8::Local<v8::Value> v8filename = module->Get(v8::String::NewFromUtf8 (isolate, "filename"));
    v8::String::Utf8Value v8utf8(v8filename);
    BeFileName addondir = BeFileName(*v8utf8, true).GetDirectoryName();

    imodeljs::Initialize(addondir);
    NodeAddonDgnDb::Init(target);
    NodeAddonECSqlStatement::Init(target);
    }

Nan::Persistent<FunctionTemplate> NodeAddonDgnDb::s_constructor_template;
Nan::Persistent<FunctionTemplate> NodeAddonECSqlStatement::s_constructor_template;

NODE_MODULE(imodeljs, registerModule)
