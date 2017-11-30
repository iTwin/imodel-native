/*--------------------------------------------------------------------------------------+
|
|     $Source: nodejs_addon.cpp $
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
#include <imodeljs-nodeaddonapi.package.version.h>

#include <node-addon-api/napi.h>
#include <uv/uv.h>
#undef X_OK // node\uv-win.h defines this, and then folly/portability/Unistd.h re-defines it.

#include <json/value.h>
#include "AddonUtils.h"
#include <ECObjects/ECSchema.h>
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <rapidjson/rapidjson.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_EC

#define DEFINE_CONSTANT_STRING_INTEGER(name, constant)                        \
    Napi::PropertyDescriptor::Value(#name, Napi::String::New(env, constant),   \
        static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)),

#define RETURN_IF_HAD_EXCEPTION if (Env().IsExceptionPending()) return;
#define RETURN_IF_HAD_EXCEPTION_SYNC if (Env().IsExceptionPending()) return Env().Undefined();

#define REQUIRE_DB_TO_BE_OPEN      if (!m_addondb->IsOpen()) {m_status = DgnDbStatus::NotOpen; return;}
#define REQUIRE_DB_TO_BE_OPEN_SYNC if (!IsOpen()) return CreateBentleyReturnErrorObject(DgnDbStatus::NotOpen);

#define REQUIRE_ARGUMENT_OBJ(i, T, var)\
    if (info.Length() <= (i) || !T::HasInstance(info[i])) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be an object of type " #T).ThrowAsJavaScriptException();\
    }\
    T* var = T::Unwrap(info[0].As<Napi::Object>());

#define REQUIRE_ARGUMENT_FUNCTION(i, var)\
    if (info.Length() <= (i) || !info[i].IsFunction()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be a function").ThrowAsJavaScriptException();\
    }\
    Napi::Function var = info[i].As<Napi::Function>();\

#define REQUIRE_ARGUMENT_STRING(i, var)\
    if (info.Length() <= (i) || !info[i].IsString()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be a string").ThrowAsJavaScriptException();\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_INTEGER(i, var)\
    if (info.Length() <= (i) || !info[i].IsNumber()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be an integer").ThrowAsJavaScriptException();\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default)\
    int var;\
    if (info.Length() <= (i)) {\
        var = (default);\
    }\
    else if (info[i].IsNumber()) {\
        var = info[i].As<Napi::Number>().Int32Value();\
    }\
    else {\
        var = (default);\
        Napi::TypeError::New(Env(), "Argument " #i " must be an integer").ThrowAsJavaScriptException();\
    }

#define OPTIONAL_ARGUMENT_STRING(i, var)\
    Utf8String var;\
    if (info.Length() <= (i)) {\
        ;\
    }\
    else if (info[i].IsString()) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    }\
    else {\
        Napi::TypeError::New(Env(), "Argument " #i " must be string").ThrowAsJavaScriptException();\
    }

#ifdef WIP_NAPI
static Napi::Env* s_env;
#endif

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct NodeUtils
    {
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      09/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename STATUSTYPE>
    static Napi::Object CreateErrorObject0(STATUSTYPE errCode, Utf8CP msg, Napi::Env env)
        {
        Napi::Object error = Napi::Object::New(env);
        error.Set(Napi::String::New(env, "status"), Napi::Number::New(env, (int) errCode));
        if (nullptr != msg)
            error.Set(Napi::String::New(env, "message"), Napi::String::New(env, msg));
        return error;
        }
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      09/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::Object CreateBentleyReturnSuccessObject(Napi::Value goodVal, Napi::Env env)
        {
        Napi::Object retObj = Napi::Object::New(env);
        retObj.Set(Napi::String::New(env, "result"), goodVal);
        return retObj;
        }
        
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      09/17
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename STATUSTYPE>
    static Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg, Napi::Env env)
        {
        Napi::Object retObj = Napi::Object::New(env);
        retObj.Set(Napi::String::New(env, "error"), CreateErrorObject0(errCode, msg, env));
        return retObj;
        }
    };

//=======================================================================================
// Projects the ECDb class into JS
//! @bsiclass
//=======================================================================================
#ifdef WIP_NAPI
struct NodeAddonECDb : Napi::ObjectWrap
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

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            REQUIRE_ARGUMENT_STRING(0, dbname);
            (new CreateDbWorker(db, *dbname);
            }

        void Execute() override
            {
            if (m_addon->m_ecdb.IsValid() && m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR_AlreadyOpen;
                return;
                }

            m_addon->m_ecdb = AddonUtils::CreateECDb(m_status, m_dbPathname);
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

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            REQUIRE_ARGUMENT_STRING(0, dbname);
            OPTIONAL_ARGUMENT_INTEGER(1, mode, (int) BeSQLite::Db::OpenMode::Readonly);
            (new OpenDbWorker(db, *dbname, (BeSQLite::Db::OpenMode)mode);
            }

        void Execute() override
            {
            if (m_addon->m_ecdb.IsValid() && m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR_AlreadyOpen;
                return;
                }

            m_addon->m_ecdb = AddonUtils::OpenECDb(m_status, m_dbPathname, m_mode);
            if (!m_addon->m_ecdb.IsValid() || m_status != BE_SQLITE_OK)
                m_addon->m_ecdb = nullptr;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct CloseDbWorker : WorkerBase<DbResult>
        {
        CloseDbWorker(NodeAddonECDb *addon) : WorkerBase(addon, BE_SQLITE_OK) {}

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            (new CloseDbWorker(db);
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

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            OPTIONAL_ARGUMENT_STRING(0, changeSetName);
            (new SaveChangesWorker(db, *changeSetName);
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

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            (new AbandonChangesWorker(db);
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

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            REQUIRE_ARGUMENT_STRING(0, schemaPathname);
            (new ImportSchemaWorker(db, *schemaPathname);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = AddonUtils::ImportSchema(*m_addon->m_ecdb, m_schemaPathname);
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct InsertInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstance; // input
        Utf8String m_insertedId; // output
        
        InsertInstanceWorker(NodeAddonECDb *addon, std::string const& strInstance) : WorkerBase(addon, DbResult::BE_SQLITE_OK), m_jsonInstance(Json::Value::From(*strInstance, *strInstance + strInstance.Length())) {}

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            REQUIRE_ARGUMENT_STRING(0, strInstance);
            (new InsertInstanceWorker(db, strInstance);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = AddonUtils::InsertInstance(m_insertedId, *m_addon->m_ecdb, m_jsonInstance);
            }

        bool _GetResult(Napi::Value& result) override
            {
            result = Napi::New(env, m_insertedId.c_str());
            return true;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct UpdateInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstance;

        UpdateInstanceWorker(NodeAddonECDb *addon, std::string const& strInstance) : WorkerBase(addon, DbResult::BE_SQLITE_OK), m_jsonInstance(Json::Value::From(*strInstance, *strInstance + strInstance.Length())) {}

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();

            REQUIRE_ARGUMENT_STRING(0, strInstance);
            (new UpdateInstanceWorker(db, strInstance);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = AddonUtils::UpdateInstance(*m_addon->m_ecdb, m_jsonInstance);
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct ReadInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstanceKey; // input   
        Json::Value m_jsonInstance; // output

        ReadInstanceWorker(NodeAddonECDb* db, std::string const& strInstanceKey) : WorkerBase(db, DbResult::BE_SQLITE_OK), m_jsonInstanceKey(Json::Value::From(*strInstanceKey, *strInstanceKey + strInstanceKey.Length())) {}

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb* db = this;

            REQUIRE_ARGUMENT_STRING(0, instanceKey);
            (new ReadInstanceWorker(db, instanceKey);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = AddonUtils::ReadInstance(m_jsonInstance, *m_addon->m_ecdb, m_jsonInstanceKey);
            }

        bool _GetResult(Napi::Value& result) override
            {
            result = Napi::New(env, m_jsonInstance.ToString().c_str());
            return true;
            }
        bool _HadError() override {return m_status != BE_SQLITE_OK;}
        };

    struct DeleteInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstanceKey; // input  

        DeleteInstanceWorker(NodeAddonECDb* db, std::string const& strInstanceKey) : WorkerBase(db, DbResult::BE_SQLITE_OK), m_jsonInstanceKey(Json::Value::From(*strInstanceKey, *strInstanceKey + strInstanceKey.Length())) {}

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb* db = this;

            REQUIRE_ARGUMENT_STRING(0, instanceKey);
            (new DeleteInstanceWorker(db, instanceKey);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = AddonUtils::DeleteInstance(*m_addon->m_ecdb, m_jsonInstanceKey);
            }
        bool _HadError() override {return m_status != BE_SQLITE_DONE;}
        };

    struct ContainsInstanceWorker : WorkerBase<DbResult>
        {
        Json::Value m_jsonInstanceKey; // input  
        bool m_containsInstance; // output

        ContainsInstanceWorker(NodeAddonECDb* db, std::string const& strInstanceKey) : WorkerBase(db, DbResult::BE_SQLITE_OK), m_jsonInstanceKey(Json::Value::From(*strInstanceKey, *strInstanceKey + strInstanceKey.Length())) {}

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb* db = this;

            REQUIRE_ARGUMENT_STRING(0, instanceKey);
            (new ContainsInstanceWorker(db, instanceKey);
            }

        void Execute() override
            {
            if (!m_addon->m_ecdb.IsValid() || !m_addon->m_ecdb->IsDbOpen())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = AddonUtils::ContainsInstance(m_containsInstance, *m_addon->m_ecdb, m_jsonInstanceKey);
            }

        bool _GetResult(Napi::Value& result) override
            {
            result = Napi::New(env, m_containsInstance);
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

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb* db = this;

            REQUIRE_ARGUMENT_STRING(0, ecsql);
            OPTIONAL_ARGUMENT_STRING(1, strBindings);

            (new ExecuteQueryWorker(db, *ecsql, *strBindings);
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

            m_status = AddonUtils::ExecuteQuery(m_rowsJson, *stmt, m_bindings);
            }

        bool _GetResult(Napi::Value& result) override
            {
            result = Napi::New(env, m_rowsJson.ToString().c_str());
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

        static Napi::Value Start(const Napi::CallbackInfo& info)
            {
            Napi::HandleScope scope(env);
            NodeAddonECDb* db = this;

            REQUIRE_ARGUMENT_STRING(0, ecsql);
            OPTIONAL_ARGUMENT_BOOLEAN(1, isInsertStmt, false);
            OPTIONAL_ARGUMENT_STRING(2, strBindings);

            (new ExecuteStatementWorker(db, *ecsql, isInsertStmt, *strBindings);
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
            
            m_status = AddonUtils::ExecuteStatement(m_instanceId, *stmt, m_isInsertStmt, m_bindings);
            }

        bool _GetResult(Napi::Value& result) override
            {
            result = Napi::New(env, m_instanceId.c_str());
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
    NodeAddonECDb() : Napi::ObjectWrap<NodeAddonECDb>(), m_lastECDbIssue("") {}
    ~NodeAddonECDb() {}

    void AddRef() { this->Ref(); }
    void Release() { this->Unref(); }

    static Napi::FunctionReference s_constructor;

    static bool HasInstance(Napi::Value val) {
  Napi::Env env = val.Env();
        Napi::Env env = val.Env();
        Napi::HandleScope scope(env);
        if (!val.IsObject())
            return false;
        Napi::Object obj = val.As<Napi::Object>();
        return Napi::New(env, s_constructor)->HasInstance(obj);
        }

    static void Init(Napi::Env env, Napi::Object exports, Napi::Object module)
        {
        Napi::HandleScope scope(env);

        Napi::FunctionReference t = Napi::Function::New(env, New);

        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***

        t->SetClassName(Napi::String::New(env, "ECDb"));

        Napi::SetPrototypeMethod(t, "createDb", CreateDbWorker::Start);
        Napi::SetPrototypeMethod(t, "openDb", OpenDbWorker::Start);
        Napi::SetPrototypeMethod(t, "closeDb", CloseDbWorker::Start);
        Napi::SetPrototypeMethod(t, "saveChanges", SaveChangesWorker::Start);
        Napi::SetPrototypeMethod(t, "abandonChanges", AbandonChangesWorker::Start);
        Napi::SetPrototypeMethod(t, "importSchema", ImportSchemaWorker::Start);
        Napi::SetPrototypeMethod(t, "insertInstance", InsertInstanceWorker::Start);
        Napi::SetPrototypeMethod(t, "readInstance", ReadInstanceWorker::Start);
        Napi::SetPrototypeMethod(t, "updateInstance", UpdateInstanceWorker::Start);
        Napi::SetPrototypeMethod(t, "deleteInstance", DeleteInstanceWorker::Start);
        Napi::SetPrototypeMethod(t, "containsInstance", ContainsInstanceWorker::Start);
        Napi::SetPrototypeMethod(t, "executeQuery", ExecuteQueryWorker::Start);
        Napi::SetPrototypeMethod(t, "executeStatement", ExecuteStatementWorker::Start);
        Napi::SetAccessor(t->InstanceTemplate(), Napi::String::New(env, "IsDbOpen"), OpenGetter);

        s_constructor.Reset(t);

        (target).Set(Napi::String::New(env, "ECDb"),
                    Napi::GetFunction(t));
        }

    static Napi::Value New(const Napi::CallbackInfo& info)
        {
        if (!info.IsConstructCall())
            {
            Napi::Error::New(env, "Use the new operator to create new NodeAddonECDb objects").ThrowAsJavaScriptException();
  return env.Null();
            }

        NodeAddonECDb *db = new NodeAddonECDb();
        db->Wrap(info.This());
        return info.This();
        }

    Napi::Value OpenGetter(const Napi::CallbackInfo& info)
        {
        NodeAddonECDb *db = info.This().Unwrap<NodeAddonECDb>();
        return db->m_ecdb.IsValid() && db->m_ecdb->IsDbOpen();
        }
};
#endif

//=======================================================================================
// SimpleRulesetLocater
//! @bsiclass
//=======================================================================================
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
            m_ruleset->GetContentRules().back()->AddSpecification(*new SelectedNodeInstancesSpecification(1, false, "", "", true));
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
struct NodeAddonDgnDb : Napi::ObjectWrap<NodeAddonDgnDb>
{
    static Napi::FunctionReference s_constructor;
    
    Dgn::DgnDbPtr m_dgndb;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;

    NodeAddonDgnDb(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeAddonDgnDb>(info)
        {
        }

    ~NodeAddonDgnDb() {}

    DgnDbR GetDgnDb() {return *m_dgndb;}

    //  Check if val is really a NodeAddonDgnDb peer object
    static bool HasInstance(Napi::Value val) {
        Napi::Env env = val.Env();
        if (!val.IsObject())
            return false;
        Napi::Object obj = val.As<Napi::Object>();
        return obj.InstanceOf(s_constructor.Value());
        }

    void SetupPresentationManager()
        {
        BeFileName assetsDir = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        BeFileName tempDir = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
        m_presentationManager = std::unique_ptr<RulesDrivenECPresentationManager>(new RulesDrivenECPresentationManager(RulesDrivenECPresentationManager::Paths(assetsDir, tempDir)));
        m_presentationManager->GetLocaters().RegisterLocater(*SimpleRulesetLocater::Create("Ruleset_Id"));
		m_presentationManager->GetConnections().NotifyConnectionOpened(*m_dgndb);
        IECPresentationManager::RegisterImplementation(m_presentationManager.get());
        }

    Napi::Object CreateBentleyReturnSuccessObject(Napi::Value goodVal) {return NodeUtils::CreateBentleyReturnSuccessObject(goodVal, Env());}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg = nullptr) {return NodeUtils::CreateBentleyReturnErrorObject(errCode, msg, Env());}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode, Napi::Value goodValue)
        {
        if ((STATUSTYPE)0 != errCode)
            return CreateBentleyReturnErrorObject(errCode);
        return CreateBentleyReturnSuccessObject(goodValue);
        }

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode) {return CreateBentleyReturnObject(errCode, Env().Undefined());}

    bool IsOpen() const {return m_dgndb.IsValid();}

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    template<typename STATUSTYPE>
    struct DgnDbWorkerBase : Napi::AsyncWorker
        {
        NodeAddonDgnDb* m_addondb;// input
        STATUSTYPE m_status;    // output

        DgnDbWorkerBase(NodeAddonDgnDb* adb, Napi::Function cb) : Napi::AsyncWorker(adb->Value(), cb), m_addondb(adb), m_status((STATUSTYPE)0) {}

        DgnDbR GetDgnDb() { return *m_addondb->m_dgndb; }

        void OnOK() override 
            {
            if (Env().IsExceptionPending())
                {
                printf ("got here\n");
                return;
                }

            if (_HadError())
                Callback().MakeCallback(Receiver().Value(), {NodeUtils::CreateErrorObject0(m_status, _GetErrorDescription(), Env())});
            else
                Callback().MakeCallback(Receiver().Value(), {Env().Undefined(), _GetSuccessValue()});
            }

        void OnError(const Napi::Error& e) override
            {
            auto msg = e.Message();
            auto dgnErrMsg = _GetErrorDescription();
            if (dgnErrMsg)
                msg.append(dgnErrMsg);
            Callback().MakeCallback(Receiver().Value(), {NodeUtils::CreateErrorObject0(m_status, msg.c_str(), Env())});
            }

        virtual Utf8CP _GetErrorDescription() {return nullptr;}
        virtual Napi::Value _GetSuccessValue() = 0;
        virtual bool _HadError() {return (STATUSTYPE)0 != m_status;}

        };

    //=======================================================================================
    //  Opens a DgnDb
    //! @bsiclass
    //=======================================================================================
    struct OpenDgnDbWorker : DgnDbWorkerBase<DbResult>
        {
        BeFileName m_dbname;    // input
        DgnDb::OpenMode m_mode; // input

        OpenDgnDbWorker(NodeAddonDgnDb* db, Napi::Function cb, BeFileName dbname, DgnDb::OpenMode mode) : DgnDbWorkerBase(db, cb), m_dbname(dbname), m_mode(mode) {}

        void Execute() override
            {
            m_status = AddonUtils::OpenDgnDb(m_addondb->m_dgndb, m_dbname, m_mode);
            if (m_status == BE_SQLITE_OK)
                m_addondb->SetupPresentationManager();
            }

        void OnOK() override 
            {
            Callback().MakeCallback(Receiver().Value(), {Napi::Number::New(Env(), (int)m_status)}); // just return the status value, since there is other "success" value.
            }

        Napi::Value _GetSuccessValue() override {return Env().Undefined();}
        };

    void StartOpenDgnDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        REQUIRE_ARGUMENT_FUNCTION(2, callback);
        RETURN_IF_HAD_EXCEPTION
        auto work = new OpenDgnDbWorker(this, callback, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        work->Queue();
        }

    Napi::Value OpenDgnDbSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        RETURN_IF_HAD_EXCEPTION_SYNC
        auto status = AddonUtils::OpenDgnDb(m_dgndb, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        return Napi::Number::New(Env(), (int)status);
        }

    //=======================================================================================
    // Returns ECClass metadata
    //! @bsiclass
    //=======================================================================================
    struct GetECClassMetaData : DgnDbWorkerBase<DgnDbStatus>
        {
        Utf8String m_ecSchema;       // input
        Utf8String m_ecClass;        // input
        Json::Value m_metaDataJson;  // ouput

        GetECClassMetaData(NodeAddonDgnDb* db, Napi::Function cb, Utf8StringCR s, Utf8StringCR c) : DgnDbWorkerBase(db, cb), m_ecSchema(s), m_ecClass(c), m_metaDataJson(Json::objectValue) {}

        void Execute() override
            {
            REQUIRE_DB_TO_BE_OPEN
            m_status = AddonUtils::GetECClassMetaData(m_metaDataJson, GetDgnDb(), m_ecSchema.c_str(), m_ecClass.c_str());
            }

        Napi::Value _GetSuccessValue() override {return Napi::String::New(Env(), m_metaDataJson.ToString().c_str());}
        };

    void StartGetECClassMetaData(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, s);
        REQUIRE_ARGUMENT_STRING(1, c);
        REQUIRE_ARGUMENT_FUNCTION(2, callback);
        RETURN_IF_HAD_EXCEPTION
        auto work = new GetECClassMetaData(this, callback, s, c);
        work->Queue();
        }

    Napi::Value GetECClassMetaDataSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, s);
        REQUIRE_ARGUMENT_STRING(1, c);
        RETURN_IF_HAD_EXCEPTION_SYNC
        Json::Value metaDataJson;
        auto status = AddonUtils::GetECClassMetaData(metaDataJson, GetDgnDb(), s.c_str(), c.c_str());
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), metaDataJson.ToString().c_str()));
        }

    //=======================================================================================
    // Get the properties for a DgnElement
    //! @bsiclass
    //=======================================================================================
    struct GetElementWorker : DgnDbWorkerBase<DgnDbStatus>
        {
        Json::Value m_opts;         // input
        Json::Value m_elementJson;  // ouput
        
        GetElementWorker(NodeAddonDgnDb* db, Napi::Function cb, Utf8StringCR inOpts) : 
            DgnDbWorkerBase(db, cb), 
            m_opts(Json::Value::From(inOpts)) 
            {}

        void Execute() override
            {
            REQUIRE_DB_TO_BE_OPEN
            m_status = AddonUtils::GetElement(m_elementJson, GetDgnDb(), m_opts);
            }

        Napi::Value _GetSuccessValue() override {return Napi::String::New(Env(), m_elementJson.ToString().c_str());}
        };

    void StartGetElementWorker(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, opts);
        REQUIRE_ARGUMENT_FUNCTION(1, callback);
        RETURN_IF_HAD_EXCEPTION
        auto work = new GetElementWorker(this, callback, opts);
        work->Queue();
        }

    //=======================================================================================
    // Get the properties for a DgnModel
    //! @bsiclass
    //=======================================================================================
    struct GetModelWorker : DgnDbWorkerBase<DgnDbStatus>
        {
        Json::Value m_opts;       // input
        Json::Value m_modelJson;  // ouput
        
        GetModelWorker(NodeAddonDgnDb* db, Napi::Function cb, Utf8StringCR inOpts) : DgnDbWorkerBase(db, cb), m_opts(Json::Value::From(inOpts)) {}

        void Execute() override
            {
            REQUIRE_DB_TO_BE_OPEN
            m_status = AddonUtils::GetModel(m_modelJson, GetDgnDb(), m_opts);
            }
        
        Napi::Value _GetSuccessValue() override {return Napi::String::New(Env(), m_modelJson.ToString().c_str());}
        };

    void StartGetModelWorker(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, opts);
        REQUIRE_ARGUMENT_FUNCTION(1, callback);
        RETURN_IF_HAD_EXCEPTION
        auto work = new GetModelWorker(this, callback, opts);
        work->Queue();
        }

    //=======================================================================================
    //  Sets up a briefcase and opens it
    //! @bsiclass
    //=======================================================================================
    Napi::Value OpenBriefcaseSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, briefcaseToken);
        REQUIRE_ARGUMENT_STRING(1, changeSetTokens);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value jsonBriefcaseToken = Json::Value::From(briefcaseToken);
        Json::Value jsonChangeSetTokens = Json::Value::From(changeSetTokens);

        DbResult result = AddonUtils::OpenBriefcase(m_dgndb, jsonBriefcaseToken, jsonChangeSetTokens);
        Napi::Object ret;
        if (BE_SQLITE_OK == result)
            SetupPresentationManager();
            
        return Napi::Number::New(Env(), (int)result);
        }

    //=======================================================================================
    //  Get cached imodels and their briefcase versions
    //! @bsiclass
    //=======================================================================================
    Napi::Value GetCachedBriefcaseInfosSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, cachePath);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value cachedBriefcaseInfos;
        BeFileName cacheFile(cachePath.c_str(), true);
        DbResult result = AddonUtils::GetCachedBriefcaseInfos(cachedBriefcaseInfos, cacheFile);
        return CreateBentleyReturnObject(result, Napi::String::New(Env(), cachedBriefcaseInfos.ToString().c_str()));
        }

    //=======================================================================================
    // insert a new element -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value InsertElementSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        Json::Value elemIdJsonObj;
        auto status = AddonUtils::InsertElement(elemIdJsonObj, GetDgnDb(), elemProps);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), elemIdJsonObj.ToString().c_str()));
        }

    //=======================================================================================
    // update an existing element -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value UpdateElementSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        auto status = AddonUtils::UpdateElement(GetDgnDb(), elemProps);
        return Napi::Number::New(Env(), (int)status);
        }

    //=======================================================================================
    // delete an existing element -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value DeleteElementSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        auto status = AddonUtils::DeleteElement(GetDgnDb(), elemIdStr);
        return Napi::Number::New(Env(), (int)status);
        }

    //=======================================================================================
    // Gets a JSON description of the properties of an element, suitable for display in a property browser. 
    // The returned properties are be organized by EC display "category" as specified by CustomAttributes.
    // Properties are identified by DisplayLabel, not name.
    // The property values are formatted according to formatting CAs.
    // The returned properties also include the properties of related instances as specified by RelatedItemsDisplaySpecification custom attributes in the ECSchemas.
    //! @bsiclass
    //=======================================================================================
    struct GetElementPropertiesForDisplayWorker : DgnDbWorkerBase<DgnDbStatus>
        {
        Utf8String m_elementIdStr;
        Utf8String m_exportedJson;
        GetElementPropertiesForDisplayWorker(NodeAddonDgnDb* db, Napi::Function cb, Utf8StringCR id) : DgnDbWorkerBase(db, cb), m_elementIdStr(id) {}

        void Execute() override
            {
            REQUIRE_DB_TO_BE_OPEN;

            if (Env().IsExceptionPending())
                printf ("got here\n");

            ECInstanceId elemId(ECInstanceId::FromString(m_elementIdStr.c_str()).GetValueUnchecked());
            if (!elemId.IsValid())
                {
                m_status = DgnDbStatus::BadElement;
                return;
                }
                
            CachedECSqlStatementPtr stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId FROM biscore.Element WHERE ECInstanceId = ?");
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
            RulesDrivenECPresentationManager::ContentOptions options ("Items");
            if ( m_addondb->m_presentationManager == nullptr)
                {
                m_status = DgnDbStatus::BadArg;
                return;                
                }
            ContentDescriptorCPtr descriptor = m_addondb->m_presentationManager->GetContentDescriptor(GetDgnDb(), ContentDisplayType::PropertyPane, selection, options.GetJson());
            if (descriptor.IsNull())
                {
                m_status = DgnDbStatus::BadArg;
                }
            PageOptions pageOptions;
            pageOptions.SetPageStart(0);
            pageOptions.SetPageSize(0);
            ContentCPtr content = m_addondb->m_presentationManager->GetContent(GetDgnDb(), *descriptor, selection, pageOptions, options.GetJson());
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

        Napi::Value _GetSuccessValue() override {return Napi::String::New(Env(), m_exportedJson.c_str());}
        };

    void StartGetElementPropertiesForDisplayWorker(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, id);
        REQUIRE_ARGUMENT_FUNCTION(1, callback);
        RETURN_IF_HAD_EXCEPTION
        auto work = new GetElementPropertiesForDisplayWorker(this, callback, id);
        work->Queue();
        }

    //=======================================================================================
    //  Execute a query and return all rows, if any
    //! @bsiclass
    //=======================================================================================
    struct ExecuteQueryWorker : DgnDbWorkerBase<DbResult>
        {
        Utf8String m_ecsql; // input
        Json::Value m_bindings; // input
        Json::Value m_rowsJson;  // output

        ExecuteQueryWorker(NodeAddonDgnDb* db, Napi::Function cb, Utf8StringCR ecsql, Utf8StringCR strBindings) : DgnDbWorkerBase(db, cb), m_ecsql(ecsql), m_rowsJson(Json::arrayValue),
            m_bindings(strBindings.empty() ? Json::nullValue : Json::Value::From(strBindings)) {}

        void Execute() override
            {
            DgnDbR dgndb = GetDgnDb();
            BeSqliteDbMutexHolder serializeAccess(dgndb); // hold mutex, so that we have a chance to get last ECDb error message

            CachedECSqlStatementPtr stmt = dgndb.GetPreparedECSqlStatement(m_ecsql.c_str());
            if (!stmt.IsValid())
                {
                m_status = BE_SQLITE_ERROR;
                return;
                }

            m_status = AddonUtils::ExecuteQuery(m_rowsJson, *stmt, m_bindings);
            }

        Napi::Value _GetSuccessValue() override {return Napi::String::New(Env(), m_rowsJson.ToString().c_str());}

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

    void StartExecuteQueryWorker(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, ecsql);
        REQUIRE_ARGUMENT_STRING(1, strBindings);
        REQUIRE_ARGUMENT_FUNCTION(2, callback);
        RETURN_IF_HAD_EXCEPTION
        auto work = new ExecuteQueryWorker(this, callback, ecsql, strBindings);
        work->Queue();
        }

    //  Add a reference to this wrapper object, keeping it and its peer JS object alive.
    void AddRef() { this->Ref(); }

    //  Remove a reference from this wrapper object and its peer JS object .
    void Release() { this->Unref(); }

    Napi::Value SaveChanges(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        RETURN_IF_HAD_EXCEPTION_SYNC
        auto stat = GetDgnDb().SaveChanges();
        return Napi::Number::New(Env(), (int)stat);
        }

    Napi::Value ImportSchema(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, schemaPathnameStrObj);
        RETURN_IF_HAD_EXCEPTION_SYNC
        BeFileName schemaPathname(schemaPathnameStrObj.c_str(), true);
        auto stat = AddonUtils::ImportSchema(GetDgnDb(), schemaPathname);
        return Napi::Number::New(Env(), (int)stat);
        }

    void CloseDgnDb(const Napi::CallbackInfo& info)
        {
        AddonUtils::CloseDgnDb(*m_dgndb);
        m_dgndb = nullptr;
        }

    Napi::Value SetBriefcaseId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_INTEGER(0, idvalue);
        RETURN_IF_HAD_EXCEPTION_SYNC

        BeFileName name(m_dgndb->GetFileName());

        DbResult result = m_dgndb->SetAsBriefcase(BeBriefcaseId(idvalue));
        if (BE_SQLITE_OK == result)
            result = m_dgndb->SaveChanges();
        if (BE_SQLITE_OK == result)
            m_dgndb->CloseDb();
        if (BE_SQLITE_OK == result)
            m_dgndb = DgnDb::OpenDgnDb(&result, name, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value GetBriefcaseId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        RETURN_IF_HAD_EXCEPTION_SYNC
        auto bid = m_dgndb->GetBriefcaseId();
        return Napi::Number::New(Env(), bid.GetValue());
        }
    
    //  Create projections
    static void Init(Napi::Env& env, Napi::Object target, Napi::Object module)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NodeAddonDgnDb", {
            InstanceMethod("openDgnDb", &NodeAddonDgnDb::StartOpenDgnDb),
            InstanceMethod("openDgnDbSync", &NodeAddonDgnDb::OpenDgnDbSync),
            InstanceMethod("closeDgnDb", &NodeAddonDgnDb::CloseDgnDb),
            InstanceMethod("setBriefcaseId", &NodeAddonDgnDb::SetBriefcaseId),
            InstanceMethod("getBriefcaseId", &NodeAddonDgnDb::GetBriefcaseId),
            InstanceMethod("openBriefcaseSync", &NodeAddonDgnDb::OpenBriefcaseSync),
            InstanceMethod("saveChanges", &NodeAddonDgnDb::SaveChanges),
            InstanceMethod("importSchema", &NodeAddonDgnDb::ImportSchema),
            InstanceMethod("getElement", &NodeAddonDgnDb::StartGetElementWorker),
            InstanceMethod("getModel", &NodeAddonDgnDb::StartGetModelWorker),
            InstanceMethod("insertElementSync", &NodeAddonDgnDb::InsertElementSync),
            InstanceMethod("updateElementSync", &NodeAddonDgnDb::UpdateElementSync),
            InstanceMethod("deleteElementSync", &NodeAddonDgnDb::DeleteElementSync),
            InstanceMethod("getElementPropertiesForDisplay", &NodeAddonDgnDb::StartGetElementPropertiesForDisplayWorker),
            InstanceMethod("getECClassMetaData", &NodeAddonDgnDb::StartGetECClassMetaData),
            InstanceMethod("getECClassMetaDataSync", &NodeAddonDgnDb::GetECClassMetaDataSync),
            InstanceMethod("executeQuery", &NodeAddonDgnDb::StartExecuteQueryWorker),
            InstanceMethod("getCachedBriefcaseInfosSync", &NodeAddonDgnDb::GetCachedBriefcaseInfosSync),
        });

        target.Set("NodeAddonDgnDb", t);

        s_constructor = Napi::Persistent(t);
        s_constructor.SuppressDestruct();             // ??? what is this?
        }
};

//=======================================================================================
// Projects the ECSqlStatement class into JS.
//! @bsiclass
//=======================================================================================
struct NodeAddonECSqlStatement : Napi::ObjectWrap<NodeAddonECSqlStatement>
{
    #define MUST_HAVE_M_STMT if (m_stmt.get() == nullptr) Napi::TypeError::New(Env(), "Statement is not prepared").ThrowAsJavaScriptException();

    std::unique_ptr<ECSqlStatement> m_stmt;
    static Napi::FunctionReference s_constructor;

    int GetECSqlStatus(ECSqlStatus status)
        {
        return status.IsSQLiteError()? (int)status.GetSQLiteError(): (int)status.Get();
        }

    NodeAddonECSqlStatement(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeAddonECSqlStatement>(info), m_stmt(new ECSqlStatement())
        {
        }

    static bool HasInstance(Napi::Value val) {
        Napi::Env env = val.Env();
        Napi::HandleScope scope(env);
        if (!val.IsObject())
            return false;
        Napi::Object obj = val.As<Napi::Object>();
        return obj.InstanceOf(s_constructor.Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object target, Napi::Object module)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NodeAddonECSqlStatement", {
          InstanceMethod("prepare", &NodeAddonECSqlStatement::Prepare),
          InstanceMethod("reset", &NodeAddonECSqlStatement::Reset),
          InstanceMethod("dispose", &NodeAddonECSqlStatement::Dispose),
          InstanceMethod("clearBindings", &NodeAddonECSqlStatement::ClearBindings),
          InstanceMethod("bindValues", &NodeAddonECSqlStatement::BindValues),
          InstanceMethod("step", &NodeAddonECSqlStatement::Step),
          InstanceMethod("getRow", &NodeAddonECSqlStatement::GetRow),
        });

        target.Set("NodeAddonECSqlStatement", t);

        s_constructor = Napi::Persistent(t);
        s_constructor.SuppressDestruct();             // ??? what is this?
        }

    Napi::Value Prepare(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonDgnDb, db);    // contract pre-conditions
        REQUIRE_ARGUMENT_STRING(1, ecsqlStr);

        if (!db->IsOpen())
            return NodeUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());

        BeSqliteDbMutexHolder serializeAccess(db->GetDgnDb()); // hold mutex, so that we have a chance to get last ECDb error message

        auto status = m_stmt->Prepare(db->GetDgnDb(), ecsqlStr.c_str());
        if (!status.IsSuccess())
            return NodeUtils::CreateErrorObject0(BE_SQLITE_ERROR, AddonUtils::GetLastEcdbIssue().c_str(), Env());
            
        MUST_HAVE_M_STMT;                           // success post-condition
        return NodeUtils::CreateErrorObject0(BE_SQLITE_OK, nullptr, Env());
        }

    Napi::Value Reset(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        auto status = m_stmt->Reset();
        return Napi::Number::New(Env(), (int)GetECSqlStatus(status));
        }

    void Dispose(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        m_stmt = nullptr;
        }

    Napi::Value ClearBindings(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        auto status = m_stmt->ClearBindings();
        return Napi::Number::New(Env(), (int)GetECSqlStatus(status));
        }

    Napi::Value BindValues(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        REQUIRE_ARGUMENT_STRING(0, valuesStr);
        
        //BeSqliteDbMutexHolder serializeAccess(*db->m_addondb); // hold mutex, so that we have a chance to get last ECDb error message

        auto status = AddonUtils::JsonBinder::BindValues(*m_stmt, Json::Value::From(valuesStr));

        if (BSISUCCESS != status)
            return NodeUtils::CreateErrorObject0(BE_SQLITE_ERROR, AddonUtils::GetLastEcdbIssue().c_str(), Env());
         
        return NodeUtils::CreateErrorObject0(BE_SQLITE_OK, nullptr, Env());
        }

    Napi::Value Step(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        auto status = m_stmt->Step();
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value GetRow(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        Json::Value rowJson(Json::objectValue);
        AddonUtils::GetRowAsJson(rowJson, *m_stmt);
        // *** NEEDS WORK: Get the adapter to set the js object's properties directly
        return Napi::String::New(Env(), rowJson.ToString().c_str());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void throwJsExceptionOnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
    {
#ifdef WIP_NAPI
    if (s_env != nullptr)
        Napi::Error::New(*s_env, Utf8PrintfString("Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str()).ThrowAsJavaScriptException();
#endif

    //else
    //    LOG.errorv(L"ASSERTION FAILURE: %ls %ls %d\n", msg, file, line);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void registerModule(Napi::Env env, Napi::Object exports, Napi::Object module)
    {
    Napi::HandleScope scope(env);

    auto filename = module.Get("filename").As<Napi::String>();
    BeFileName addondir = BeFileName(filename.Utf8Value().c_str(), true).GetDirectoryName();

    AddonUtils::Initialize(addondir, throwJsExceptionOnAssert);
    NodeAddonDgnDb::Init(env, exports, module);
    // NodeAddonECDb::Init(env, exports, module);
    NodeAddonECSqlStatement::Init(env, exports, module);

    exports.DefineProperties(
        {
        DEFINE_CONSTANT_STRING_INTEGER(version, PACKAGE_VERSION)
        });
    }

Napi::FunctionReference NodeAddonECSqlStatement::s_constructor;
Napi::FunctionReference NodeAddonDgnDb::s_constructor;
//Napi::FunctionReference NodeAddonECDb::s_constructor;

NODE_API_MODULE(AddonUtils, registerModule)
