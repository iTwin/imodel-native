/*--------------------------------------------------------------------------------------+
|
|     $Source: IModelJsNative.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

#include <json/value.h>
#include "IModelJsNative.h"
#include <ECDb/ECDb.h>
#include <ECObjects/ECSchema.h>
#include <rapidjson/rapidjson.h>
#include "ECPresentationUtils.h"
#include "ECSchemaXmlContextUtils.h"
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/BeThread.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_EC

#define PROPERTY_ATTRIBUTES static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)

#define REQUIRE_DB_TO_BE_OPEN if (!IsOpen()) return CreateBentleyReturnErrorObject(DgnDbStatus::NotOpen);

#define THROW_JS_TYPE_ERROR(str) Napi::TypeError::New(Env(), str).ThrowAsJavaScriptException();

#define THROW_TYPE_EXCEPTION_AND_RETURN(str,retval) {THROW_JS_TYPE_ERROR(str) return retval;}

#define REQUIRE_ARGUMENT_ANY_OBJ(i, var, retval)\
    if (info.Length() <= (i)) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object", retval)\
    }\
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_OBJ(i, T, var, retval)\
    if (info.Length() <= (i) || !T::InstanceOf(info[i])) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object of type " #T, retval)\
    }\
    T* var = T::Unwrap(info[i].As<Napi::Object>());

#define REQUIRE_ARGUMENT_FUNCTION(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsFunction()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a function", retval)\
    }\
    Napi::Function var = info[i].As<Napi::Function>();\

#define REQUIRE_ARGUMENT_STRING(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsString()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a string", retval)\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_STRING_ARRAY(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsArray()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an array of strings", retval)\
    }\
    bvector<Utf8String> var;\
    Napi::Array arr = info[i].As<Napi::Array>();\
    for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {\
        Napi::Value arrValue = arr[arrIndex];\
        if (arrValue.IsString())\
            var.push_back(arrValue.As<Napi::String>().Utf8Value().c_str());\
    }

#define REQUIRE_ARGUMENT_NUMBER(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsNumber()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a number", retval)\
    }\
    Napi::Number var = info[i].As<Napi::Number>();

#define REQUIRE_ARGUMENT_INTEGER(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsNumber()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an integer", retval)\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define REQUIRE_ARGUMENT_BOOL(i, var, retval)\
    if (info.Length() <= (i) || !info[i].IsBoolean()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a boolean", retval)\
    }\
    bool var = info[i].As<Napi::Boolean>().Value();

#define OPTIONAL_ARGUMENT_BOOL(i, var, default, retval)\
    bool var;\
    if (info.Length() <= (i) || info[i].IsUndefined()) {\
        var = (default);\
    }\
    else if (info[i].IsBoolean()) {\
        var = info[i].As<Napi::Boolean>().Value();\
    }\
    else {\
        var = (default);\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an boolean", retval)\
    }

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default, retval)\
    int var;\
    if (info.Length() <= (i) || info[i].IsUndefined()) {\
        var = (default);\
    }\
    else if (info[i].IsNumber()) {\
        var = info[i].As<Napi::Number>().Int32Value();\
    }\
    else {\
        var = (default);\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an integer", retval)\
    }

#define OPTIONAL_ARGUMENT_STRING(i, var, retval)\
    Utf8String var;\
    if (info.Length() <= (i) || info[i].IsUndefined()) {\
        ;\
    }\
    else if (info[i].IsString()) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    }\
    else {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be string or undefined", retval)\
    }

namespace IModelJsNative {

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct NapiUtils
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
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      01/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::String Stringify(Napi::Env env, Napi::Value value)
        {
        return env.Global().Get("JSON").As<Napi::Object>().Get("stringify").As<Napi::Function>()({value}).ToString();
        }

};

//=======================================================================================
// Projects the ECDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeECDb : Napi::ObjectWrap<NativeECDb>
    {
    private:
        static Napi::FunctionReference s_constructor;
        std::unique_ptr<ECDb> m_ecdb;

    public:
    NativeECDb(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECDb>(info) {}
    ~NativeECDb() {}

        // Check if val is really a NativeECDb peer object
        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
            }
        ECDbR GetECDb()
            {
            if (m_ecdb == nullptr)
                m_ecdb = std::make_unique<ECDb>();

            return *m_ecdb;
            }

        Napi::Value CreateDb(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName, Env().Undefined());
            DbResult status = JsInterop::CreateECDb(GetECDb(), BeFileName(dbName.c_str(), true));
            if (BE_SQLITE_OK == status)
                {
                GetECDb().AddFunction(HexStrSqlFunction::GetSingleton());
                GetECDb().AddFunction(StrSqlFunction::GetSingleton());
                }

            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value OpenDb(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName, Env().Undefined());
            REQUIRE_ARGUMENT_INTEGER(1, mode, Env().Undefined());

            DbResult status = JsInterop::OpenECDb(GetECDb(), BeFileName(dbName.c_str(), true), (Db::OpenMode) mode);
            if (BE_SQLITE_OK == status)
                {
                GetECDb().AddFunction(HexStrSqlFunction::GetSingleton());
                GetECDb().AddFunction(StrSqlFunction::GetSingleton());
                }

            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value CloseDb(Napi::CallbackInfo const& info)
            {
            if (m_ecdb != nullptr)
                {
                m_ecdb->RemoveFunction(HexStrSqlFunction::GetSingleton());
                m_ecdb->RemoveFunction(StrSqlFunction::GetSingleton());
                m_ecdb->CloseDb();
                m_ecdb = nullptr;
                }

            return Napi::Number::New(Env(), (int) BE_SQLITE_OK);
            }

        Napi::Value Dispose(Napi::CallbackInfo const& info) { return CloseDb(info); }

        Napi::Value SaveChanges(Napi::CallbackInfo const& info)
            {
            OPTIONAL_ARGUMENT_STRING(0, changeSetName, Env().Undefined());
            DbResult status = GetECDb().SaveChanges(changeSetName.empty() ? nullptr : changeSetName.c_str());
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value AbandonChanges(Napi::CallbackInfo const& info)
            {
            DbResult status = GetECDb().AbandonChanges();
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value ImportSchema(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, schemaPathName, Env().Undefined());
            DbResult status = JsInterop::ImportSchema(GetECDb(), BeFileName(schemaPathName.c_str(), true));
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value IsOpen(Napi::CallbackInfo const& info) { return Napi::Boolean::New(Env(), GetECDb().IsDbOpen()); }

        //  Add a reference to this wrapper object, keeping it and its peer JS object alive.
        void AddRef() { this->Ref(); }

        //  Remove a reference from this wrapper object and its peer JS object .
        void Release() { this->Unref(); }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            // ***
            // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
            // ***
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "NativeECDb", {
            InstanceMethod("createDb", &NativeECDb::CreateDb),
            InstanceMethod("openDb", &NativeECDb::OpenDb),
            InstanceMethod("closeDb", &NativeECDb::CloseDb),
            InstanceMethod("dispose", &NativeECDb::Dispose),
            InstanceMethod("saveChanges", &NativeECDb::SaveChanges),
            InstanceMethod("abandonChanges", &NativeECDb::AbandonChanges),
            InstanceMethod("importSchema", &NativeECDb::ImportSchema),
            InstanceMethod("isOpen", &NativeECDb::IsOpen)
            });

            exports.Set("NativeECDb", t);

            s_constructor = Napi::Persistent(t);
            // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
            // from running at program shutdown time, which would attempt to reset the reference when
            // the environment is no longer valid.
            s_constructor.SuppressDestruct();
            }
    };

//=======================================================================================
// Projects the NativeECSchemaXmlContext class into JS
//! @bsiclass
//=======================================================================================
struct NativeECSchemaXmlContext : Napi::ObjectWrap<NativeECSchemaXmlContext>
    {
    private:
        static Napi::FunctionReference s_constructor;
        ECSchemaReadContextPtr m_context;
        ECSchemaXmlContextUtils::LocaterCallbackUPtr m_locater;

    public:
        NativeECSchemaXmlContext(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSchemaXmlContext>(info)
            {
            m_context = ECSchemaXmlContextUtils::CreateSchemaReadContext(T_HOST.GetIKnownLocationsAdmin());
            }

        // Check if val is really a NativeECSchemaXmlContext peer object
        static bool HasInstance(Napi::Value val) {
            if (!val.IsObject())
                return false;
            Napi::Object obj = val.As<Napi::Object>();
            return obj.InstanceOf(s_constructor.Value());
            }

        void SetSchemaLocater(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_FUNCTION(0, locaterCallback, );
            ECSchemaXmlContextUtils::SetSchemaLocater(*m_context, m_locater, Napi::Persistent(locaterCallback));
            }

        void AddSchemaPath(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, schemaPath, );
            ECSchemaXmlContextUtils::AddSchemaPath(*m_context, schemaPath);
            }

        Napi::Value ReadSchemaFromXmlFile(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, filePath, Env().Undefined());
            Json::Value schemaJson;
            auto status = ECSchemaXmlContextUtils::ConvertECSchemaXmlToJson(schemaJson, *m_context, filePath);

            if (ECSchemaXmlContextUtils::SchemaConversionStatus::Success == status)
                return NapiUtils::CreateBentleyReturnSuccessObject(Napi::String::New(Env(), schemaJson.ToString().c_str()), Env());

            return NapiUtils::CreateBentleyReturnErrorObject(BentleyStatus::ERROR, ECSchemaXmlContextUtils::SchemaConversionStatusToString(status), Env());
            }

        //  Add a reference to this wrapper object, keeping it and its peer JS object alive.
        void AddRef() { this->Ref(); }

        //  Remove a reference from this wrapper object and its peer JS object .
        void Release() { this->Unref(); }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            // ***
            // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
            // ***
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "NativeECSchemaXmlContext", {
                InstanceMethod("addSchemaPath", &NativeECSchemaXmlContext::AddSchemaPath),
                InstanceMethod("readSchemaFromXmlFile", &NativeECSchemaXmlContext::ReadSchemaFromXmlFile),
                InstanceMethod("setSchemaLocater", &NativeECSchemaXmlContext::SetSchemaLocater)
            });

            exports.Set("NativeECSchemaXmlContext", t);

            s_constructor = Napi::Persistent(t);
            // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
            // from running at program shutdown time, which would attempt to reset the reference when
            // the environment is no longer valid.
            s_constructor.SuppressDestruct();
            }
    };


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

// Project the IBriefcaseManager::Request class
struct NativeBriefcaseManagerResourcesRequest : Napi::ObjectWrap<NativeBriefcaseManagerResourcesRequest>
{
    IBriefcaseManager::Request m_req;
    static Napi::FunctionReference s_constructor;

    NativeBriefcaseManagerResourcesRequest(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeBriefcaseManagerResourcesRequest>(info)
        {
        }

    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeBriefcaseManagerResourcesRequest", {
          InstanceMethod("reset", &NativeBriefcaseManagerResourcesRequest::Reset),
          InstanceMethod("isEmpty", &NativeBriefcaseManagerResourcesRequest::IsEmpty),
          InstanceMethod("toJSON", &NativeBriefcaseManagerResourcesRequest::ToJSON),
        });

        exports.Set("NativeBriefcaseManagerResourcesRequest", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    void Reset(Napi::CallbackInfo const& info)
        {
        m_req.Reset();
        }

    Napi::Value IsEmpty(Napi::CallbackInfo const& info)
        {
        return Napi::Number::New(Env(), (bool)m_req.IsEmpty());
        }

    Napi::Value ToJSON(Napi::CallbackInfo const& info)
        {
        Json::Value json(Json::objectValue);
        m_req.ToJson(json);
        return Napi::String::New(Env(), json.ToString().c_str());
        }

};

//=======================================================================================
// Projects the DgnDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeDgnDb : Napi::ObjectWrap<NativeDgnDb>
    {
    struct NativeAppData : Db::AppData
        {
        NativeDgnDb& m_addonDb;

        static Key const& GetKey()
            {
            static Key s_key;
            return s_key;
            }

        NativeAppData(NativeDgnDb& adb) : m_addonDb(adb) {}

        static NativeAppData* Find(DgnDbR db)
            {
            return (NativeAppData*) db.FindAppData(GetKey());
            }

        static void Add(NativeDgnDb& adb)
            {
            BeAssert(nullptr == Find(adb.GetDgnDb()));
            adb.GetDgnDb().AddAppData(GetKey(), new NativeAppData(adb));
            }

        static void Remove(DgnDbR db)
            {
            db.DropAppData(GetKey());
            }
        };

    static Napi::FunctionReference s_constructor;

    Dgn::DgnDbPtr m_dgndb;
    ConnectionManager m_connections;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;

    NativeDgnDb(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeDgnDb>(info)
        {
        }

    ~NativeDgnDb()
        {
        CloseDgnDb();
        }

    void CloseDgnDb()
        {
        if (!m_dgndb.IsValid())
            return;

        TearDownPresentationManager();
        m_dgndb->RemoveFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->RemoveFunction(StrSqlFunction::GetSingleton());
        JsInterop::CloseDgnDb(*m_dgndb);
        NativeAppData::Remove(GetDgnDb());
        m_dgndb = nullptr;
        }

    void OnDgnDbOpened(DgnDbP dgndb)
        {
        if (nullptr == dgndb)
            CloseDgnDb();

        m_dgndb = dgndb;
        if (!m_dgndb.IsValid())
            return;

        SetupPresentationManager();
        NativeAppData::Add(*this);
        m_dgndb->AddFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->AddFunction(StrSqlFunction::GetSingleton());
        }

    static NativeDgnDb* From(DgnDbR db)
        {
        auto appdata = NativeAppData::Find(db);
        return appdata? &appdata->m_addonDb: nullptr;
        }


    //  Check if val is really a NativeDgnDb peer object
    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
        }

    DgnDbR GetDgnDb() {return *m_dgndb;}

    void SetupPresentationManager()
        {
        BeFileName assetsDir = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        BeFileName tempDir = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectoryBaseName();
        RulesDrivenECPresentationManager::Paths paths(assetsDir, tempDir);
        m_presentationManager = std::unique_ptr<RulesDrivenECPresentationManager>(new RulesDrivenECPresentationManager(m_connections, paths));
        IECPresentationManager::RegisterImplementation(m_presentationManager.get());
        m_presentationManager->GetLocaters().RegisterLocater(*SimpleRulesetLocater::Create("Ruleset_Id"));
        m_connections.NotifyConnectionOpened(*m_dgndb);
        }

    void TearDownPresentationManager()
        {
        if (m_presentationManager == nullptr)
            return;
        IECPresentationManager::RegisterImplementation(nullptr);
        m_presentationManager.reset();
        }

    Napi::Object CreateBentleyReturnSuccessObject(Napi::Value goodVal) {return NapiUtils::CreateBentleyReturnSuccessObject(goodVal, Env());}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg = nullptr) {return NapiUtils::CreateBentleyReturnErrorObject(errCode, msg, Env());}

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

    Napi::Value OpenDgnDb(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(1, mode, Env().Undefined());
        DgnDbPtr db;
        DbResult status = JsInterop::OpenDgnDb(db, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        if (BE_SQLITE_OK == status)
            OnDgnDbOpened(db.get());

        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value CreateDgnDb(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, rootSubjectName, Env().Undefined());
        OPTIONAL_ARGUMENT_STRING(2, rootSubjectDescription, Env().Undefined());

        DgnDbPtr db;
        DbResult status = JsInterop::CreateDgnDb(db, BeFileName(dbName.c_str(), true), rootSubjectName, rootSubjectDescription);
        if (BE_SQLITE_OK == status)
            OnDgnDbOpened(db.get());

        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value GetECClassMetaData(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, s, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, c, Env().Undefined());
        Json::Value metaDataJson;
        auto status = JsInterop::GetECClassMetaData(metaDataJson, GetDgnDb(), s.c_str(), c.c_str());
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), metaDataJson.ToString().c_str()));
        }

    Napi::Value GetElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, optsJsonStr, Env().Undefined());
        Json::Value opts = Json::Value::From(optsJsonStr);
        Json::Value elementJson;  // ouput
        auto status = JsInterop::GetElement(elementJson, GetDgnDb(), opts);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), elementJson.ToString().c_str()));
        }

    Napi::Value GetModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, optsJsonStr, Env().Undefined());
        Json::Value opts = Json::Value::From(optsJsonStr);
        Json::Value modelJson;;  // ouput
        auto status = JsInterop::GetModel(modelJson, GetDgnDb(), opts);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), modelJson.ToString().c_str()));
        }

    // Sets up a briefcase and opens it
    Napi::Value SetupBriefcase(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, briefcaseToken, Env().Undefined());
        Json::Value jsonBriefcaseToken = Json::Value::From(briefcaseToken);

        DbResult result = JsInterop::SetupBriefcase(m_dgndb, jsonBriefcaseToken);
        Napi::Object ret;
        if (BE_SQLITE_OK == result)
            OnDgnDbOpened(m_dgndb.get());

        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value ProcessChangeSets(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, changeSetTokens, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(1, processOptions, Env().Undefined());
        REQUIRE_ARGUMENT_BOOL(2, containsSchemaChanges, Env().Undefined());
        Json::Value jsonChangeSetTokens = Json::Value::From(changeSetTokens);

        bool isReadonly = m_dgndb->IsReadonly();
        Utf8String dbGuid = m_dgndb->GetDbGuid().ToString();;
        BeFileName dbFileName(m_dgndb->GetDbFileName(), true);
        if (containsSchemaChanges)
            CloseDgnDb();

        DbResult result = JsInterop::ProcessChangeSets(m_dgndb, jsonChangeSetTokens, (RevisionProcessOption)processOptions, dbGuid, dbFileName);
        if (BE_SQLITE_OK == result && containsSchemaChanges)
            {
            DgnDbPtr db;
            result = JsInterop::OpenDgnDb(db, dbFileName, isReadonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite);
            if (BE_SQLITE_OK == result)
                OnDgnDbOpened(db.get());
            }

        return Napi::Number::New(Env(), (int) result);
        }

    static TxnManager::TxnId TxnIdFromString(Utf8StringCR str)
        {
        BeInt64Id fakeId;
        BeInt64Id::FromString(fakeId, str.c_str());
        return TxnManager::TxnId(fakeId.GetValue());
        }

    static Utf8String TxnIdToString(TxnManager::TxnId txnId)
        {
        BeInt64Id fakeId(txnId.GetValue());
        return fakeId.ToHexStr();
        }

    Napi::Value TxnManagerGetCurrentTxnId(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        return Napi::String::New(Env(), TxnIdToString(m_dgndb->Txns().GetCurrentTxnId()).c_str());
        }

    Napi::Value TxnManagerQueryFirstTxnId(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        TxnManager::TxnId startTxnId = m_dgndb->Txns().QueryNextTxnId(TxnManager::TxnId(0));
        return Napi::String::New(Env(), TxnIdToString(startTxnId).c_str());
        }

    Napi::Value TxnManagerQueryNextTxnId(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        REQUIRE_DB_TO_BE_OPEN
        auto next = m_dgndb->Txns().QueryNextTxnId(TxnIdFromString(txnIdHexStr));
        return Napi::String::New(Env(), TxnIdToString(next).c_str());
        }

    Napi::Value TxnManagerQueryPreviousTxnId(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        REQUIRE_DB_TO_BE_OPEN
        auto next = m_dgndb->Txns().QueryPreviousTxnId(TxnIdFromString(txnIdHexStr));
        return Napi::String::New(Env(), TxnIdToString(next).c_str());
        }

    Napi::Value TxnManagerGetTxnDescription(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        REQUIRE_DB_TO_BE_OPEN

        return Napi::String::New(Env(), m_dgndb->Txns().GetTxnDescription(TxnIdFromString(txnIdHexStr)).c_str());
        }

    Napi::Value TxnManagerIsTxnIdValid(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        return Napi::Boolean::New(Env(), TxnIdFromString(txnIdHexStr).IsValid());
        }

    Napi::Value TxnManagerHasUnsavedChanges(Napi::CallbackInfo const& info)
        {
        return Napi::Boolean::New(Env(), m_dgndb->Txns().HasChanges());
        }

    Napi::Value StartCreateChangeSet(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        Json::Value changeSetInfo;
        DbResult result = JsInterop::StartCreateChangeSet(changeSetInfo, *m_dgndb);
        return CreateBentleyReturnObject(result, Napi::String::New(Env(), changeSetInfo.ToString().c_str()));
        }

    Napi::Value FinishCreateChangeSet(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        DbResult result = JsInterop::FinishCreateChangeSet(*m_dgndb);
        return Napi::Number::New(Env(), (int) result);
        }

    void AbandonCreateChangeSet(Napi::CallbackInfo const& info)
        {
        if (!m_dgndb.IsValid())
            return;
        JsInterop::AbandonCreateChangeSet(*m_dgndb);
        }

    Napi::Value ExtractCodes(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        Json::Value codes;
        DbResult result = JsInterop::ExtractCodes(codes, *m_dgndb);
        return CreateBentleyReturnObject(result, Napi::String::New(Env(), codes.ToString().c_str()));
        }

    Napi::Value GetCachedBriefcaseInfos(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, cachePath, Env().Undefined());
        Json::Value cachedBriefcaseInfos;
        BeFileName cacheFile(cachePath.c_str(), true);
        DbResult result = JsInterop::GetCachedBriefcaseInfos(cachedBriefcaseInfos, cacheFile);
        return CreateBentleyReturnObject(result, Napi::String::New(Env(), cachedBriefcaseInfos.ToString().c_str()));
        }

    Napi::Value GetIModelProps(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        Json::Value props;
        JsInterop::GetIModelProps(props, *m_dgndb);
        return Napi::String::New(Env(), props.ToString().c_str());
        }

    Napi::Value InsertElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr, Env().Undefined());
        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        Json::Value elemIdJsonObj;
        auto status = JsInterop::InsertElement(elemIdJsonObj, GetDgnDb(), elemProps);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), elemIdJsonObj.ToString().c_str()));
        }

    Napi::Value UpdateElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr, Env().Undefined());

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        auto status = JsInterop::UpdateElement(GetDgnDb(), elemProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value DeleteElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemIdStr, Env().Undefined());
        auto status = JsInterop::DeleteElement(GetDgnDb(), elemIdStr);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value InsertLinkTableRelationship(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr, Env().Undefined());
        Json::Value props = Json::Value::From(propsJsonStr);
        Json::Value idJsonObj;
        auto status = JsInterop::InsertLinkTableRelationship(idJsonObj, GetDgnDb(), props);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), idJsonObj.asCString()));
        }

    Napi::Value UpdateLinkTableRelationship(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr, Env().Undefined());
        Json::Value props = Json::Value::From(propsJsonStr);
        auto status = JsInterop::UpdateLinkTableRelationship(GetDgnDb(), props);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value DeleteLinkTableRelationship(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr, Env().Undefined());
        Json::Value props = Json::Value::From(propsJsonStr);
        auto status = JsInterop::DeleteLinkTableRelationship(GetDgnDb(), props);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value InsertCodeSpec(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, name, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(1, specType, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(2, scopeReq, Env().Undefined());

        if ((uint32_t)CodeScopeSpec::Type::Model != specType && (uint32_t)CodeScopeSpec::Type::ParentElement != specType &&
            (uint32_t)CodeScopeSpec::Type::RelatedElement != specType && (uint32_t)CodeScopeSpec::Type::Repository != specType)
            THROW_TYPE_EXCEPTION_AND_RETURN("Argument 1 must be a CodeScopeSpec.Type", Env().Undefined());
    
        if ((uint32_t)CodeScopeSpec::ScopeRequirement::ElementId != scopeReq && (uint32_t)CodeScopeSpec::ScopeRequirement::FederationGuid != scopeReq)
            THROW_TYPE_EXCEPTION_AND_RETURN("Argument 2 must be a CodeScopeSpec.ScopeRequirement", Env().Undefined());

        Utf8String idStr;
        DgnDbStatus status = JsInterop::InsertCodeSpec(idStr, GetDgnDb(), name, (CodeScopeSpec::Type)specType, (CodeScopeSpec::ScopeRequirement)scopeReq);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), idStr.c_str()));
        }

    Napi::Value InsertModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, modelPropsJson, Env().Undefined());
        Json::Value modelProps = Json::Value::From(modelPropsJson);
        Json::Value modelIdProps;
        auto status = JsInterop::InsertModel(modelIdProps, GetDgnDb(), modelProps);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), modelIdProps.ToString().c_str()));
        }

    Napi::Value UpdateModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, modelPropsJson, Env().Undefined());
        Json::Value modelProps = Json::Value::From(modelPropsJson);
        auto status = JsInterop::UpdateModel(GetDgnDb(), modelProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value DeleteModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemIdStr, Env().Undefined());
        auto status = JsInterop::DeleteModel(GetDgnDb(), elemIdStr);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value GetElementPropertiesForDisplay(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elementIdStr, Env().Undefined());
        Utf8String exportedJson;

        ECInstanceId elemId(ECInstanceId::FromString(elementIdStr.c_str()).GetValueUnchecked());
        if (!elemId.IsValid())
            return CreateBentleyReturnErrorObject(DgnDbStatus::BadElement);

        CachedECSqlStatementPtr stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId FROM biscore.Element WHERE ECInstanceId = ?");
        if (!stmt.IsValid())
            return CreateBentleyReturnErrorObject(DgnDbStatus::SQLiteError);

        stmt->BindId(1, elemId);
        if (stmt->Step() != BE_SQLITE_ROW)
            return CreateBentleyReturnErrorObject(DgnDbStatus::SQLiteError);

        ECClassId ecclassId = stmt->GetValueId<ECClassId>(0);
        ECInstanceKey instanceKey = ECInstanceKey(ecclassId, elemId);
        KeySetPtr input = KeySet::Create({instanceKey});
        RulesDrivenECPresentationManager::ContentOptions options ("Items");
        if ( m_presentationManager == nullptr)
            return CreateBentleyReturnErrorObject(DgnDbStatus::BadArg);

        ContentDescriptorCPtr descriptor = m_presentationManager->GetContentDescriptor(GetDgnDb(), ContentDisplayType::PropertyPane, *input, nullptr, options.GetJson()).get();
        if (descriptor.IsNull())
            return CreateBentleyReturnErrorObject(DgnDbStatus::BadArg);

        PageOptions pageOptions;
        pageOptions.SetPageStart(0);
        pageOptions.SetPageSize(0);
        ContentCPtr content = m_presentationManager->GetContent(*descriptor, pageOptions).get();
        if (content.IsNull())
            return CreateBentleyReturnErrorObject(DgnDbStatus::BadArg);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        content->AsJson().Accept(writer);
        exportedJson = buffer.GetString();

        return CreateBentleyReturnSuccessObject(Napi::String::New(Env(), exportedJson.c_str()));
        };

    //  Add a reference to this wrapper object, keeping it and its peer JS object alive.
    void AddRef() { this->Ref(); }

    //  Remove a reference from this wrapper object and its peer JS object .
    void Release() { this->Unref(); }

    Napi::Value SaveChanges(Napi::CallbackInfo const& info)
        {
        OPTIONAL_ARGUMENT_STRING(0, description, Env().Undefined());
        REQUIRE_DB_TO_BE_OPEN
        auto stat = GetDgnDb().SaveChanges(description);
        return Napi::Number::New(Env(), (int)stat);
        }

    Napi::Value AbandonChanges(Napi::CallbackInfo const& info)
        {
        DbResult status = GetDgnDb().AbandonChanges();
        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value ImportSchema(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, schemaPathnameStrObj, Env().Undefined());
        BeFileName schemaPathname(schemaPathnameStrObj.c_str(), true);
        auto stat = JsInterop::ImportSchemaDgnDb(GetDgnDb(), schemaPathname);
        return Napi::Number::New(Env(), (int)stat);
        }

    void CloseDgnDb(Napi::CallbackInfo const& info)
        {
        CloseDgnDb();
        }

    Napi::Value CreateChangeCache(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_OBJ(0, NativeECDb, changeCacheECDb, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, changeCachePathStr, Env().Undefined());
        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().CreateChangeCache(changeCacheECDb->GetECDb(), changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value AttachChangeCache(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, changeCachePathStr, Env().Undefined());
        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().AttachChangeCache(changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value DetachChangeCache(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        DbResult stat = GetDgnDb().DetachChangeCache();
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value IsChangeCacheAttached(Napi::CallbackInfo const&)
        {
        REQUIRE_DB_TO_BE_OPEN
        bool isAttached = GetDgnDb().IsChangeCacheAttached();
        return Napi::Boolean::New(Env(), isAttached);
        }

    Napi::Value ExtractChangeSummary(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_OBJ(0, NativeECDb, changeCacheECDb, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, changesetFilePathStr, Env().Undefined());
        struct NativeChangeSet : BeSQLite::ChangeSet
            {
            ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override { return ConflictResolution::Skip; }
            };

        NativeChangeSet changeset;
        {
        BeFileName changesetFilePath(changesetFilePathStr.c_str(), true);

        RevisionChangesFileReader fs(changesetFilePath, GetDgnDb());
        BeSQLite::ChangeGroup group;
        DbResult r = fs.ToChangeGroup(group);
        if (BE_SQLITE_OK != r)
            return CreateBentleyReturnErrorObject(r);

        r = changeset.FromChangeGroup(group);
        if (BE_SQLITE_OK != r)
            return CreateBentleyReturnErrorObject(r);
        }

        ECInstanceKey changeSummaryKey;
        if (SUCCESS != ECDb::ExtractChangeSummary(changeSummaryKey, changeCacheECDb->GetECDb(), GetDgnDb(), ChangeSetArg(changeset)))
            return CreateBentleyReturnErrorObject(BE_SQLITE_ERROR);

        return CreateBentleyReturnSuccessObject(Napi::String::New(Env(), changeSummaryKey.GetInstanceId().ToHexStr().c_str()));
        }

    Napi::Value SetBriefcaseId(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_INTEGER(0, idvalue, Env().Undefined());
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

    Napi::Value GetBriefcaseId(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        auto bid = m_dgndb->GetBriefcaseId();
        return Napi::Number::New(Env(), bid.GetValue());
        }

    Napi::Value GetParentChangeSetId(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        Utf8String parentRevId = m_dgndb->Revisions().GetParentRevisionId();
        return Napi::String::New(Env(), parentRevId.c_str());
        }

    Napi::Value GetReversedChangeSetId(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        if (!m_dgndb->Revisions().HasReversedRevisions())
            return Env().Undefined();
        Utf8String reversedRevId = m_dgndb->Revisions().GetReversedRevisionId();
        return Napi::String::New(Env(), reversedRevId.c_str());
        }

    Napi::Value GetDbGuid(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        BeGuid beGuid = m_dgndb->GetDbGuid();
        return Napi::String::New(Env(), beGuid.ToString().c_str());
        }

    Napi::Value SetDbGuid(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, guidStr, Env().Undefined());
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        m_dgndb->ChangeDbGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeBriefcaseManagerResourcesRequest, req, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, elemProps, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(2, dbop, Env().Undefined());
        Json::Value elemPropsJson = Json::Value::From(elemProps);
        return Napi::Number::New(Env(), (int)JsInterop::BuildBriefcaseManagerResourcesRequestForElement(req->m_req, GetDgnDb(), elemPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeBriefcaseManagerResourcesRequest, req, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, props, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(2, dbop, Env().Undefined());
        Json::Value propsJson = Json::Value::From(props);
        return Napi::Number::New(Env(), (int)JsInterop::BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(req->m_req, GetDgnDb(), propsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeBriefcaseManagerResourcesRequest, req, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, modelProps, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(2, dbop, Env().Undefined());
        Json::Value modelPropsJson = Json::Value::From(modelProps);
        return Napi::Number::New(Env(), (int)JsInterop::BuildBriefcaseManagerResourcesRequestForModel(req->m_req, GetDgnDb(), modelPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    void ExtractBulkResourcesRequest(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeBriefcaseManagerResourcesRequest, req,);
        REQUIRE_ARGUMENT_BOOL(1, locks, );
        REQUIRE_ARGUMENT_BOOL(2, codes, );
        GetDgnDb().BriefcaseManager().ExtractRequestFromBulkOperation(req->m_req, locks, codes);
        }

    Napi::Value InBulkOperation(Napi::CallbackInfo const& info)
        {
        return Napi::Boolean::New(Env(), GetDgnDb().BriefcaseManager().IsBulkOperation());
        }

   void AppendBriefcaseManagerResourcesRequest(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeBriefcaseManagerResourcesRequest, req, );
        REQUIRE_ARGUMENT_OBJ(1, NativeBriefcaseManagerResourcesRequest, reqIn, );
        req->m_req.Codes().insert(reqIn->m_req.Codes().begin(), reqIn->m_req.Codes().end());
        req->m_req.Locks().GetLockSet().insert(reqIn->m_req.Locks().GetLockSet().begin(), reqIn->m_req.Locks().GetLockSet().end());
        // TBD: merge in request options
        }

   void ExtractBriefcaseManagerResourcesRequest(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeBriefcaseManagerResourcesRequest, reqOut, );
        REQUIRE_ARGUMENT_OBJ(1, NativeBriefcaseManagerResourcesRequest, reqIn, );
        REQUIRE_ARGUMENT_BOOL(2, locks, );
        REQUIRE_ARGUMENT_BOOL(3, codes, );
        if (codes)
            {
            reqOut->m_req.Codes().insert(reqIn->m_req.Codes().begin(), reqIn->m_req.Codes().end());
            reqIn->m_req.Codes().clear();
            }
        if (locks)
            {
            reqOut->m_req.Locks().GetLockSet().insert(reqIn->m_req.Locks().GetLockSet().begin(), reqIn->m_req.Locks().GetLockSet().end());
            reqIn->m_req.Locks().Clear();
            }
        // TBD: merge in request options
        }

    Napi::Value SetBriefcaseManagerOptimisticConcurrencyControlPolicy(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, conflictRes, Env().Undefined());
        OptimisticConcurrencyControl::OnConflict uu = (OptimisticConcurrencyControl::OnConflict)conflictRes.Get("updateVsUpdate").ToNumber().Int32Value();
        OptimisticConcurrencyControl::OnConflict ud = (OptimisticConcurrencyControl::OnConflict)conflictRes.Get("updateVsDelete").ToNumber().Int32Value();
        OptimisticConcurrencyControl::OnConflict du = (OptimisticConcurrencyControl::OnConflict)conflictRes.Get("deleteVsUpdate").ToNumber().Int32Value();
        if (OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange != uu && OptimisticConcurrencyControl::OnConflict::RejectIncomingChange != uu
         || OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange != ud && OptimisticConcurrencyControl::OnConflict::RejectIncomingChange != ud
         || OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange != du && OptimisticConcurrencyControl::OnConflict::RejectIncomingChange != du)
            {
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid conflict resolution value", Napi::Number::New(Env(), 1));
            }
        OptimisticConcurrencyControl::Policy policy;
        policy.deleteVsUpdate = du;
        policy.updateVsUpdate = uu;
        policy.updateVsDelete = ud;

        return Napi::Number::New(Env(), (int)GetDgnDb().SetConcurrencyControl(new OptimisticConcurrencyControl(policy)));
        }

    Napi::Value SetBriefcaseManagerPessimisticConcurrencyControlPolicy(Napi::CallbackInfo const& info)
        {
        return Napi::Number::New(Env(), (int)GetDgnDb().SetConcurrencyControl(new PessimisticConcurrencyControl()));
        }

    Napi::Value BriefcaseManagerStartBulkOperation(Napi::CallbackInfo const& info)
        {
        return Napi::Number::New(Env(), (int)JsInterop::BriefcaseManagerStartBulkOperation(GetDgnDb()));
        }

    Napi::Value BriefcaseManagerEndBulkOperation(Napi::CallbackInfo const& info)
        {
        return Napi::Number::New(Env(), (int)JsInterop::BriefcaseManagerEndBulkOperation(GetDgnDb()));
        }

    Napi::Value UpdateProjectExtents(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, newExtentsJson, Env().Undefined())
        return Napi::Number::New(Env(), (int)JsInterop::UpdateProjectExtents(GetDgnDb(), Json::Value::From(newExtentsJson)));
        }
    Napi::Value ReadFontMap(Napi::CallbackInfo const& info)
        {
        auto fontMap = GetDgnDb().Fonts().FontMap();
        Json::Value fontList(Json::arrayValue);
        for (auto font : fontMap)
            {
            auto thisFont = Json::Value();
            thisFont[DgnFonts::json_id()] = (int)font.first.GetValue();
            thisFont[DgnFonts::json_type()] = (int)font.second->GetType();
            thisFont[DgnFonts::json_name()] = font.second->GetName();
            fontList.append(thisFont);
            }
        Json::Value fonts;
        fonts[DgnFonts::json_fonts()] = fontList;
        return Napi::String::New(Env(), fonts.ToString().c_str());
        }

    // ========================================================================================
    // Test method handler
    // ========================================================================================
    Napi::Value ExecuteTest(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, testName, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, params, Env().Undefined());
        return Napi::String::New(Env(), JsInterop::ExecuteTest(GetDgnDb(), testName, params).ToString().c_str());
        }


    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeDgnDb", {
            InstanceMethod("abandonChanges", &NativeDgnDb::AbandonChanges),
            InstanceMethod("abandonCreateChangeSet", &NativeDgnDb::AbandonCreateChangeSet),
            InstanceMethod("appendBriefcaseManagerResourcesRequest", &NativeDgnDb::AppendBriefcaseManagerResourcesRequest),
            InstanceMethod("attachChangeCache", &NativeDgnDb::AttachChangeCache),
            InstanceMethod("briefcaseManagerEndBulkOperation", &NativeDgnDb::BriefcaseManagerEndBulkOperation),
            InstanceMethod("briefcaseManagerStartBulkOperation", &NativeDgnDb::BriefcaseManagerStartBulkOperation),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &NativeDgnDb::BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForModel", &NativeDgnDb::BuildBriefcaseManagerResourcesRequestForModel),
            InstanceMethod("closeDgnDb", &NativeDgnDb::CloseDgnDb),
            InstanceMethod("createChangeCache", &NativeDgnDb::CreateChangeCache),
            InstanceMethod("createDgnDb", &NativeDgnDb::CreateDgnDb),
            InstanceMethod("deleteElement", &NativeDgnDb::DeleteElement),
            InstanceMethod("deleteLinkTableRelationship", &NativeDgnDb::DeleteLinkTableRelationship),
            InstanceMethod("deleteModel", &NativeDgnDb::DeleteModel),
            InstanceMethod("detachChangeCache", &NativeDgnDb::DetachChangeCache),
            InstanceMethod("executeTest", &NativeDgnDb::ExecuteTest),
            InstanceMethod("extractBriefcaseManagerResourcesRequest", &NativeDgnDb::ExtractBriefcaseManagerResourcesRequest),
            InstanceMethod("extractBulkResourcesRequest", &NativeDgnDb::ExtractBulkResourcesRequest),
            InstanceMethod("extractChangeSummary", &NativeDgnDb::ExtractChangeSummary),
            InstanceMethod("extractCodes", &NativeDgnDb::ExtractCodes),
            InstanceMethod("finishCreateChangeSet", &NativeDgnDb::FinishCreateChangeSet),
            InstanceMethod("getBriefcaseId", &NativeDgnDb::GetBriefcaseId),
            InstanceMethod("getCachedBriefcaseInfos", &NativeDgnDb::GetCachedBriefcaseInfos),
            InstanceMethod("getDbGuid", &NativeDgnDb::GetDbGuid),
            InstanceMethod("getECClassMetaData", &NativeDgnDb::GetECClassMetaData),
            InstanceMethod("getElement", &NativeDgnDb::GetElement),
            InstanceMethod("getElementPropertiesForDisplay", &NativeDgnDb::GetElementPropertiesForDisplay),
            InstanceMethod("getIModelProps", &NativeDgnDb::GetIModelProps),
            InstanceMethod("getModel", &NativeDgnDb::GetModel),
            InstanceMethod("getParentChangeSetId", &NativeDgnDb::GetParentChangeSetId),
            InstanceMethod("getReversedChangeSetId", &NativeDgnDb::GetReversedChangeSetId),
            InstanceMethod("importSchema", &NativeDgnDb::ImportSchema),
            InstanceMethod("inBulkOperation", &NativeDgnDb::InBulkOperation),
            InstanceMethod("insertCodeSpec", &NativeDgnDb::InsertCodeSpec),
            InstanceMethod("insertElement", &NativeDgnDb::InsertElement),
            InstanceMethod("insertLinkTableRelationship", &NativeDgnDb::InsertLinkTableRelationship),
            InstanceMethod("insertModel", &NativeDgnDb::InsertModel),
            InstanceMethod("isChangeCacheAttached", &NativeDgnDb::IsChangeCacheAttached),
            InstanceMethod("openDgnDb", &NativeDgnDb::OpenDgnDb),
            InstanceMethod("processChangeSets", &NativeDgnDb::ProcessChangeSets),
            InstanceMethod("readFontMap", &NativeDgnDb::ReadFontMap),
            InstanceMethod("saveChanges", &NativeDgnDb::SaveChanges),
            InstanceMethod("setBriefcaseId", &NativeDgnDb::SetBriefcaseId),
            InstanceMethod("setBriefcaseManagerOptimisticConcurrencyControlPolicy", &NativeDgnDb::SetBriefcaseManagerOptimisticConcurrencyControlPolicy),
            InstanceMethod("setBriefcaseManagerPessimisticConcurrencyControlPolicy", &NativeDgnDb::SetBriefcaseManagerPessimisticConcurrencyControlPolicy),
            InstanceMethod("setDbGuid", &NativeDgnDb::SetDbGuid),
            InstanceMethod("setupBriefcase", &NativeDgnDb::SetupBriefcase),
            InstanceMethod("startCreateChangeSet", &NativeDgnDb::StartCreateChangeSet),
            InstanceMethod("txnManagerGetCurrentTxnId", &NativeDgnDb::TxnManagerGetCurrentTxnId),
            InstanceMethod("txnManagerGetTxnDescription", &NativeDgnDb::TxnManagerGetTxnDescription),
            InstanceMethod("txnManagerHasUnsavedChanges", &NativeDgnDb::TxnManagerHasUnsavedChanges),
            InstanceMethod("txnManagerIsTxnIdValid", &NativeDgnDb::TxnManagerIsTxnIdValid),
            InstanceMethod("txnManagerQueryFirstTxnId", &NativeDgnDb::TxnManagerQueryFirstTxnId),
            InstanceMethod("txnManagerQueryNextTxnId", &NativeDgnDb::TxnManagerQueryNextTxnId),
            InstanceMethod("txnManagerQueryPreviousTxnId", &NativeDgnDb::TxnManagerQueryPreviousTxnId),
            InstanceMethod("updateElement", &NativeDgnDb::UpdateElement),
            InstanceMethod("updateLinkTableRelationship", &NativeDgnDb::UpdateLinkTableRelationship),
            InstanceMethod("updateModel", &NativeDgnDb::UpdateModel),
            InstanceMethod("updateProjectExtents", &NativeDgnDb::UpdateProjectExtents),
        });

        exports.Set("NativeDgnDb", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }
};

//=======================================================================================
// Projects the IECSqlBinder interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlBinder : Napi::ObjectWrap<NativeECSqlBinder>
    {
private:
    static Napi::FunctionReference s_constructor;
    IECSqlBinder* m_binder = nullptr;
    ECDb const* m_ecdb = nullptr;

    static DbResult ToDbResult(ECSqlStatus status)
        {
        if (status.IsSuccess())
            return BE_SQLITE_OK;

        if (status.IsSQLiteError())
            return status.GetSQLiteError();

        return BE_SQLITE_ERROR;
        }

    IECSqlBinder& GetBinder()
        {
        if (m_binder == nullptr)
            Napi::TypeError::New(Env(), "Invalid Binder").ThrowAsJavaScriptException();

        return *m_binder;
        }
public:
    NativeECSqlBinder(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlBinder>(info)
        {
        if (info.Length() != 2)
            THROW_TYPE_EXCEPTION_AND_RETURN("NativeECSqlBinder constructor expects two arguments.", );

        m_binder = info[0].As<Napi::External<IECSqlBinder>>().Data();
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid first arg for NativeECSqlBinder constructor. IECSqlBinder must not be nullptr",);

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid second arg for NativeECSqlBinder constructor. ECDb must not be nullptr",);
        }

    ~NativeECSqlBinder() {}

    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeECSqlBinder", {
            InstanceMethod("addArrayElement", &NativeECSqlBinder::AddArrayElement),
            InstanceMethod("bindBlob", &NativeECSqlBinder::BindBlob),
            InstanceMethod("bindBoolean", &NativeECSqlBinder::BindBoolean),
            InstanceMethod("bindDateTime", &NativeECSqlBinder::BindDateTime),
            InstanceMethod("bindDouble", &NativeECSqlBinder::BindDouble),
            InstanceMethod("bindGuid", &NativeECSqlBinder::BindGuid),
            InstanceMethod("bindId", &NativeECSqlBinder::BindId),
            InstanceMethod("bindInteger", &NativeECSqlBinder::BindInteger),
            InstanceMethod("bindMember", &NativeECSqlBinder::BindMember),
            InstanceMethod("bindNavigation", &NativeECSqlBinder::BindNavigation),
            InstanceMethod("bindNull", &NativeECSqlBinder::BindNull),
            InstanceMethod("bindPoint2d", &NativeECSqlBinder::BindPoint2d),
            InstanceMethod("bindPoint3d", &NativeECSqlBinder::BindPoint3d),
            InstanceMethod("bindString", &NativeECSqlBinder::BindString),
        });

        exports.Set("NativeECSqlBinder", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    static Napi::Object New(Napi::Env const& env, IECSqlBinder& binder, ECDbCR ecdb)
        {
        return s_constructor.New({Napi::External<IECSqlBinder>::New(env, &binder), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
        }

    Napi::Value BindNull(Napi::CallbackInfo const& info)
        {
        ECSqlStatus stat = GetBinder().BindNull();
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBlob(Napi::CallbackInfo const& info)
        {
        if (info.Length() == 0)
            THROW_TYPE_EXCEPTION_AND_RETURN("BindBlob requires an argument", Env().Undefined());
        
        Napi::Value blobVal = info[0];
        if (blobVal.IsArrayBuffer())
            {
            auto arrayBuf = blobVal.As<Napi::ArrayBuffer>();
            ECSqlStatus stat = GetBinder().BindBlob(arrayBuf.Data(), (int) arrayBuf.ByteLength(), IECSqlBinder::MakeCopy::Yes);
            return Napi::Number::New(Env(), (int) ToDbResult(stat));
            }
        
        if (!blobVal.IsString())
            THROW_TYPE_EXCEPTION_AND_RETURN("BindBlob requires either a base64-encoded-string or an ArrayBuffer", Env().Undefined());
        
        Utf8String base64Str(blobVal.ToString().Utf8Value().c_str());
        ByteStream blob;
        Base64Utilities::Decode(blob, base64Str);

        ECSqlStatus stat = GetBinder().BindBlob(blob.data(), (int) blob.size(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBoolean(Napi::CallbackInfo const& info)
        {
        Napi::Value boolVal;
        if (info.Length() == 0 || !(boolVal = info[0]).IsBoolean())
            THROW_TYPE_EXCEPTION_AND_RETURN("BindBoolean expects a boolean", Env().Undefined());

        ECSqlStatus stat = GetBinder().BindBoolean(boolVal.ToBoolean());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDateTime(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, isoString, Env().Undefined());

        DateTime dt;
        if (SUCCESS != DateTime::FromString(dt, isoString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = GetBinder().BindDateTime(dt);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDouble(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_NUMBER(0, val, Env().Undefined());
        ECSqlStatus stat = GetBinder().BindDouble(val.DoubleValue());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindGuid(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, guidString, Env().Undefined());
        BeGuid guid;
        if (SUCCESS != guid.FromString(guidString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = GetBinder().BindGuid(guid, IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindId(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, hexString, Env().Undefined());
        BeInt64Id id;
        if (SUCCESS != BeInt64Id::FromString(id, hexString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = GetBinder().BindId(id);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindInteger(Napi::CallbackInfo const& info)
        {
        if (info.Length() == 0)
            THROW_TYPE_EXCEPTION_AND_RETURN("BindInteger expects a string or number", Env().Undefined());

        Napi::Value val = info[0];
        if (!val.IsNumber() && !val.IsString())
            THROW_TYPE_EXCEPTION_AND_RETURN("BindInteger expects a string or number", Env().Undefined());

        int64_t int64Val;
        if (val.IsNumber())
            int64Val = val.ToNumber().Int64Value();
        else
            {
            Utf8String strVal(val.ToString().Utf8Value().c_str());
            if (strVal.empty())
                THROW_TYPE_EXCEPTION_AND_RETURN("Integral string passed to BindInteger must not be empty.", Env().Undefined());

            bool const isNegativeNumber = strVal[0] == '-';
            Utf8CP positiveNumberStr = isNegativeNumber ? strVal.c_str() + 1 : strVal.c_str();
            uint64_t uVal = 0;
            if (SUCCESS != BeStringUtilities::ParseUInt64(uVal, positiveNumberStr)) //also supports hex strings
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Could not parse string %s to a valid integer.", strVal.c_str());
                THROW_TYPE_EXCEPTION_AND_RETURN(error.c_str(), Env().Undefined());
                }

            if (isNegativeNumber && uVal > (uint64_t) std::numeric_limits<int64_t>::max())
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Number in string %s is too large to fit into a signed 64 bit integer value.", strVal.c_str());
                THROW_TYPE_EXCEPTION_AND_RETURN(error.c_str(), Env().Undefined());
                }

            int64Val = uVal;
            if (isNegativeNumber)
                int64Val *= -1;
            }

        ECSqlStatus stat = GetBinder().BindInt64(int64Val);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint2d(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_NUMBER(0, x, Env().Undefined());
        REQUIRE_ARGUMENT_NUMBER(1, y, Env().Undefined());
        ECSqlStatus stat = GetBinder().BindPoint2d(DPoint2d::From(x.DoubleValue(),y.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint3d(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_NUMBER(0, x, Env().Undefined());
        REQUIRE_ARGUMENT_NUMBER(1, y, Env().Undefined());
        REQUIRE_ARGUMENT_NUMBER(2, z, Env().Undefined());
        ECSqlStatus stat = GetBinder().BindPoint3d(DPoint3d::From(x.DoubleValue(), y.DoubleValue(), z.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindString(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, val, Env().Undefined());
        ECSqlStatus stat = GetBinder().BindText(val.c_str(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindNavigation(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, navIdHexStr, Env().Undefined());
        OPTIONAL_ARGUMENT_STRING(1, relClassName, Env().Undefined());
        OPTIONAL_ARGUMENT_STRING(2, relClassTableSpaceName, Env().Undefined());

        BeInt64Id navId;
        if (SUCCESS != BeInt64Id::FromString(navId, navIdHexStr.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECClassId relClassId;
        if (!relClassName.empty())
            {
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(relClassName.c_str(), ".:", tokens);
            if (tokens.size() != 2)
                return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

            relClassId = m_ecdb->Schemas().GetClassId(tokens[0], tokens[1], SchemaLookupMode::AutoDetect, relClassTableSpaceName.c_str());
            }

        ECSqlStatus stat = GetBinder().BindNavigation(navId, relClassId);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindMember(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, memberName, Env().Undefined());
        IECSqlBinder& memberBinder = GetBinder()[memberName.c_str()];
        return New(info.Env(), memberBinder, *m_ecdb);
        }

    Napi::Value AddArrayElement(Napi::CallbackInfo const& info)
        {
        IECSqlBinder& elementBinder = GetBinder().AddArrayElement();
        return New(info.Env(), elementBinder, *m_ecdb);
        }
    };

//=======================================================================================
// Projects the NativeECSqlColumnInfo interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlColumnInfo : Napi::ObjectWrap<NativeECSqlColumnInfo>
    {
    private:
        //Must match ECSqlValueType in imodeljs-core
        enum class Type
            {
            Blob = 1,
            Boolean = 2,
            DateTime = 3,
            Double = 4,
            Geometry = 5,
            Id = 6,
            Int = 7,
            Int64 = 8,
            Point2d = 9,
            Point3d = 10,
            String = 11,
            Navigation = 12,
            Struct = 13,
            PrimitiveArray = 14,
            StructArray = 15,
            Guid = 16
            };

        static Napi::FunctionReference s_constructor;
        ECSqlColumnInfo const* m_colInfo = nullptr;

        ECSqlColumnInfo const& GetColInfo() const
            {
            if (m_colInfo == nullptr)
                Napi::TypeError::New(Env(), "Invalid ECSqlColumnInfo").ThrowAsJavaScriptException();

            return *m_colInfo;
            }

    public:
        NativeECSqlColumnInfo(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlColumnInfo>(info)
            {
            if (info.Length() != 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("NativeECSqlColumnInfo constructor expects one argument.",);

            m_colInfo = info[0].As<Napi::External<ECSqlColumnInfo>>().Data();
            if (m_colInfo == nullptr)
                Napi::TypeError::New(Env(), "Invalid first arg for NativeECSqlColumnInfo constructor. ECSqlColumnInfo must not be nullptr").ThrowAsJavaScriptException();
            }

        ~NativeECSqlColumnInfo() {}

        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
            }

        //  Create projections
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            // ***
            // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
            // ***
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "NativeECSqlColumnInfo", {
            InstanceMethod("getType", &NativeECSqlColumnInfo::GetType),
            InstanceMethod("getPropertyName", &NativeECSqlColumnInfo::GetPropertyName),
            InstanceMethod("getAccessString", &NativeECSqlColumnInfo::GetAccessString),
            InstanceMethod("isSystemProperty", &NativeECSqlColumnInfo::IsSystemProperty),
            InstanceMethod("isGeneratedProperty", &NativeECSqlColumnInfo::IsGeneratedProperty),
            InstanceMethod("getRootClassTableSpace", &NativeECSqlColumnInfo::GetRootClassTableSpace),
            InstanceMethod("getRootClassName", &NativeECSqlColumnInfo::GetRootClassName),
            InstanceMethod("getRootClassAlias", &NativeECSqlColumnInfo::GetRootClassAlias)});

            exports.Set("NativeECSqlColumnInfo", t);

            s_constructor = Napi::Persistent(t);
            // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
            // from running at program shutdown time, which would attempt to reset the reference when
            // the environment is no longer valid.
            s_constructor.SuppressDestruct();
            }

        static Napi::Object New(Napi::Env const& env, ECSqlColumnInfo const& colInfo)
            {
            return s_constructor.New({Napi::External<ECSqlColumnInfo>::New(env, const_cast<ECSqlColumnInfo*>(&colInfo))});
            }

        Napi::Value GetType(Napi::CallbackInfo const& info)
            {
            ECTypeDescriptor const& dataType = GetColInfo().GetDataType();
            Type type = Type::Id;
            if (dataType.IsNavigation())
                type = Type::Navigation;
            else if (dataType.IsStruct())
                type = Type::Struct;
            else if (dataType.IsPrimitiveArray())
                type = Type::PrimitiveArray;
            else if (dataType.IsStructArray())
                type = Type::StructArray;
            else
                {
                BeAssert(dataType.IsPrimitive());
                switch (dataType.GetPrimitiveType())
                    {
                        case PRIMITIVETYPE_Binary:
                        {
                        ECPropertyCP prop = GetColInfo().GetProperty();
                        if (prop->HasExtendedType())
                            {
                            BeAssert(prop->GetIsPrimitive());
                            Utf8StringCR extendedTypeName = prop->GetAsPrimitiveProperty()->GetExtendedTypeName();
                            if (extendedTypeName.EqualsIAscii("Guid") || extendedTypeName.EqualsIAscii("BeGuid"))
                                {
                                type = Type::Guid;
                                break;
                                }
                            }

                        type = Type::Blob;
                        break;
                        }
                        case PRIMITIVETYPE_Boolean:
                            type = Type::Boolean;
                            break;
                        case PRIMITIVETYPE_DateTime:
                            type = Type::DateTime;
                            break;
                        case PRIMITIVETYPE_Double:
                            type = Type::Double;
                            break;
                        case PRIMITIVETYPE_IGeometry:
                            type = Type::Geometry;
                            break;
                        case PRIMITIVETYPE_Integer:
                            type = Type::Int;
                            break;
                        case PRIMITIVETYPE_Long:
                        {
                        if (GetColInfo().IsSystemProperty())
                            {
                            type = Type::Id;
                            break;
                            }

                        ECPropertyCP prop = GetColInfo().GetProperty();
                        if (prop->HasExtendedType())
                            {
                            BeAssert(prop->GetIsPrimitive());
                            if (prop->GetAsPrimitiveProperty()->GetExtendedTypeName().EqualsIAscii("Id"))
                                {
                                type = Type::Id;
                                break;
                                }
                            }

                        type = Type::Int64;
                        break;
                        }
                        case PRIMITIVETYPE_Point2d:
                            type = Type::Point2d;
                            break;
                        case PRIMITIVETYPE_Point3d:
                            type = Type::Point3d;
                            break;
                        case PRIMITIVETYPE_String:
                            type = Type::String;
                            break;
                        default:
                            Napi::TypeError::New(info.Env(), "Unsupported ECSqlValue primitive type.").ThrowAsJavaScriptException();
                            break;
                    }
                }

            return Napi::Number::New(Env(), (int) type);
            }

        Napi::Value GetPropertyName(Napi::CallbackInfo const& info)
            {
            ECPropertyCP prop = GetColInfo().GetProperty();
            if (prop == nullptr)
                Napi::TypeError::New(info.Env(), "ECSqlColumnInfo does not represent a property.").ThrowAsJavaScriptException();

            return Napi::String::New(Env(), prop->GetName().c_str());
            }

        Napi::Value GetAccessString(Napi::CallbackInfo const& info)
            {
            //if property is generated, the display label contains the select clause item as is.
            //The property name in contrast would have encoded special characters of the select clause item.
            //Ex: SELECT MyProp + 4 FROM Foo -> the name must be "MyProp + 4"
            if (GetColInfo().IsGeneratedProperty())
                {
                BeAssert(GetColInfo().GetPropertyPath().Size() == 1);
                ECPropertyCP prop = GetColInfo().GetProperty();
                if (prop == nullptr)
                    THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo's Property must not be null for a generated property.", Env().Undefined());

                return Napi::String::New(Env(), prop->GetDisplayLabel().c_str());
                }

            return Napi::String::New(Env(), GetColInfo().GetPropertyPath().ToString().c_str());
            }

        Napi::Value IsSystemProperty(Napi::CallbackInfo const& info)
            {
            return Napi::Boolean::New(Env(), GetColInfo().IsSystemProperty());
            }

        Napi::Value IsGeneratedProperty(Napi::CallbackInfo const& info)
            {
            return Napi::Boolean::New(Env(), GetColInfo().IsGeneratedProperty());
            }

        Napi::Value GetRootClassTableSpace(Napi::CallbackInfo const& info)
            {
            return Napi::String::New(Env(), GetColInfo().GetRootClass().GetTableSpace().c_str());
            }

        Napi::Value GetRootClassName(Napi::CallbackInfo const& info)
            {
            return Napi::String::New(Env(), ECJsonUtilities::FormatClassName(GetColInfo().GetRootClass().GetClass()).c_str());
            }

        Napi::Value GetRootClassAlias(Napi::CallbackInfo const& info)
            {
            return Napi::String::New(Env(), GetColInfo().GetRootClass().GetAlias().c_str());
            }
    };

//=======================================================================================
// Projects the IECSqlValue interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlValue : Napi::ObjectWrap<NativeECSqlValue>
    {
private:
    static Napi::FunctionReference s_constructor;
    IECSqlValue const* m_ecsqlValue = nullptr;
    ECDb const* m_ecdb = nullptr;


    IECSqlValue const& GetECSqlValue() const
        {
        if (m_ecsqlValue == nullptr)
            Napi::TypeError::New(Env(), "Invalid ECSqlValue").ThrowAsJavaScriptException();

        return *m_ecsqlValue;
        }
public:
    NativeECSqlValue(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlValue>(info)
        {
        if (info.Length() < 2)
            THROW_TYPE_EXCEPTION_AND_RETURN("NativeECSqlValue constructor expects two arguments.",);

        m_ecsqlValue = info[0].As<Napi::External<IECSqlValue>>().Data();
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid first arg for NativeECSqlValue constructor. IECSqlValue must not be nullptr",);

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            Napi::TypeError::New(Env(), "Invalid second arg for NativeECSqlValue constructor. ECDb must not be nullptr").ThrowAsJavaScriptException();
        }

    ~NativeECSqlValue() {}

    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeECSqlValue", {
            InstanceMethod("getArrayIterator", &NativeECSqlValue::GetArrayIterator),
            InstanceMethod("getBlob", &NativeECSqlValue::GetBlob),
            InstanceMethod("getBoolean", &NativeECSqlValue::GetBoolean),
            InstanceMethod("getClassNameForClassId", &NativeECSqlValue::GetClassNameForClassId),
            InstanceMethod("getColumnInfo", &NativeECSqlValue::GetColumnInfo),
            InstanceMethod("getDateTime", &NativeECSqlValue::GetDateTime),
            InstanceMethod("getDouble", &NativeECSqlValue::GetDouble),
            InstanceMethod("getGeometry", &NativeECSqlValue::GetGeometry),
            InstanceMethod("getGuid", &NativeECSqlValue::GetGuid),
            InstanceMethod("getId", &NativeECSqlValue::GetId),
            InstanceMethod("getInt", &NativeECSqlValue::GetInt),
            InstanceMethod("getInt64", &NativeECSqlValue::GetInt64),
            InstanceMethod("getNavigation", &NativeECSqlValue::GetNavigation),
            InstanceMethod("getPoint2d", &NativeECSqlValue::GetPoint2d),
            InstanceMethod("getPoint3d", &NativeECSqlValue::GetPoint3d),
            InstanceMethod("getString", &NativeECSqlValue::GetString),
            InstanceMethod("getStructIterator", &NativeECSqlValue::GetStructIterator),
            InstanceMethod("isNull", &NativeECSqlValue::IsNull),
        });

        exports.Set("NativeECSqlValue", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    static Napi::Object New(Napi::Env const& env, IECSqlValue const& val, ECDbCR ecdb)
        {
        return s_constructor.New({Napi::External<IECSqlValue>::New(env, const_cast<IECSqlValue*>(&val)), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
        }

    Napi::Value GetColumnInfo(Napi::CallbackInfo const& info)
        {
        return NativeECSqlColumnInfo::New(Env(), GetECSqlValue().GetColumnInfo());
        }

    Napi::Value IsNull(Napi::CallbackInfo const& info)
        {
        return Napi::Boolean::New(Env(), GetECSqlValue().IsNull());
        }

    Napi::Value GetBlob(Napi::CallbackInfo const& info)
        {
        int blobSize;
        void const* data = GetECSqlValue().GetBlob(&blobSize);
        auto blob = Napi::ArrayBuffer::New(Env(), blobSize);
        memcpy(blob.Data(), data, blobSize);
        return blob;
        }

    Napi::Value GetBoolean(Napi::CallbackInfo const& info)
        {
        return Napi::Boolean::New(Env(), GetECSqlValue().GetBoolean());
        }

    Napi::Value GetDateTime(Napi::CallbackInfo const& info)
        {
        DateTime dt = GetECSqlValue().GetDateTime();
        return Napi::String::New(Env(), dt.ToString().c_str());
        }

    Napi::Value GetDouble(Napi::CallbackInfo const& info)
        {
        return Napi::Number::New(Env(), GetECSqlValue().GetDouble());
        }

    Napi::Value GetGeometry(Napi::CallbackInfo const& info)
        {
        IGeometryPtr geom = GetECSqlValue().GetGeometry();
        Json::Value json;
        if (SUCCESS != ECJsonUtilities::IGeometryToJson(json, *geom))
            Napi::TypeError::New(info.Env(), "Could not convert IGeometry to JSON.").ThrowAsJavaScriptException();

        return Napi::String::New(Env(), json.ToString().c_str());
        }

    Napi::Value GetGuid(Napi::CallbackInfo const& info)
        {
        BeGuid guid = GetECSqlValue().GetGuid();
        return Napi::String::New(Env(), guid.ToString().c_str());
        }

    Napi::Value GetId(Napi::CallbackInfo const& info)
        {
        BeInt64Id id = GetECSqlValue().GetId<BeInt64Id>();
        return Napi::String::New(Env(), id.ToHexStr().c_str());
        }

    Napi::Value GetClassNameForClassId(Napi::CallbackInfo const& info)
        {
        ECClassId classId = GetECSqlValue().GetId<ECClassId>();
        if (!classId.IsValid())
            Napi::TypeError::New(info.Env(), "Failed to get class name from ECSqlValue: The ECSqlValue does not refer to a valid class id.").ThrowAsJavaScriptException();

        Utf8StringCR tableSpace = GetECSqlValue().GetColumnInfo().GetRootClass().GetTableSpace();
        ECClassCP ecClass = m_ecdb->Schemas().GetClass(classId, tableSpace.c_str());
        if (ecClass == nullptr)
            {
            Utf8String err;
            err.Sprintf("Failed to get class name from ECSqlValue: Class not found for ECClassId %s.", classId.ToHexStr().c_str());
            Napi::TypeError::New(info.Env(), err.c_str()).ThrowAsJavaScriptException();
            }

        return Napi::String::New(Env(), ECJsonUtilities::FormatClassName(*ecClass).c_str());
        }

    Napi::Value GetInt(Napi::CallbackInfo const& info)
        {
        return Napi::Number::New(Env(), GetECSqlValue().GetInt());
        }

    Napi::Value GetInt64(Napi::CallbackInfo const& info)
        {
        return Napi::Number::New(Env(), GetECSqlValue().GetInt64());
        }

    Napi::Value GetPoint2d(Napi::CallbackInfo const& info)
        {
        DPoint2d pt = GetECSqlValue().GetPoint2d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        return jsPt;
        }

    Napi::Value GetPoint3d(Napi::CallbackInfo const& info)
        {
        DPoint3d pt = GetECSqlValue().GetPoint3d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Z(), Napi::Number::New(Env(), pt.z));
        return jsPt;
        }

    Napi::Value GetString(Napi::CallbackInfo const& info)
        {
        return Napi::String::New(Env(), GetECSqlValue().GetText());
        }

    Napi::Value GetNavigation(Napi::CallbackInfo const& info)
        {
        ECClassId relClassId;
        BeInt64Id navId = GetECSqlValue().GetNavigation(&relClassId);

        Napi::Object jsNavValue = Napi::Object::New(Env());
        jsNavValue.Set(ECN::ECJsonSystemNames::Navigation::Id(), Napi::String::New(Env(), navId.ToHexStr().c_str()));
        if (relClassId.IsValid())
            {
            Utf8StringCR relClassTableSpace = GetECSqlValue().GetColumnInfo().GetRootClass().GetTableSpace();
            ECClassCP relClass = m_ecdb->Schemas().GetClass(relClassId, relClassTableSpace.c_str());
            if (relClass == nullptr)
                Napi::TypeError::New(info.Env(), "Failed to find ECRelationhipClass for the Navigation Value's RelECClassId.").ThrowAsJavaScriptException();

            Utf8String relClassName = ECJsonUtilities::FormatClassName(*relClass);
            jsNavValue.Set(ECN::ECJsonSystemNames::Navigation::RelClassName(), Napi::String::New(Env(), relClassName.c_str()));
            }

        return jsNavValue;
        }

    //implementations are after NativeECSqlValueIterable as it needs to call into that class
    Napi::Value GetStructIterator(Napi::CallbackInfo const&);
    Napi::Value GetArrayIterator(Napi::CallbackInfo const&);
    };

//=======================================================================================
// Projects the IECSqlValueIterable interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlValueIterator : Napi::ObjectWrap<NativeECSqlValueIterator>
    {
    private:
        static Napi::FunctionReference s_constructor;
        ECDb const* m_ecdb = nullptr;
        IECSqlValueIterable const* m_iterable = nullptr;
        bool m_isBeforeFirstElement = true;
        IECSqlValueIterable::const_iterator m_it;
        IECSqlValueIterable::const_iterator m_endIt;

    public:
        NativeECSqlValueIterator(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlValueIterator>(info)
            {
            if (info.Length() < 2)
                Napi::TypeError::New(Env(), "NativeECSqlValueIterator constructor expects two argument.").ThrowAsJavaScriptException();

            m_iterable = info[0].As<Napi::External<IECSqlValueIterable>>().Data();
            if (m_iterable == nullptr)
                Napi::TypeError::New(Env(), "Invalid first arg for NativeECSqlValueIterator constructor. IECSqlValueIterable must not be nullptr").ThrowAsJavaScriptException();

            m_endIt = m_iterable->end();

            m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
            if (m_ecdb == nullptr)
                Napi::TypeError::New(Env(), "Invalid second arg for NativeECSqlValueIterator constructor. ECDb must not be nullptr").ThrowAsJavaScriptException();
            }

        ~NativeECSqlValueIterator() {}

        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
            }

        //  Create projections
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            // ***
            // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
            // ***
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "NativeECSqlValueIterator", {
            InstanceMethod("moveNext", &NativeECSqlValueIterator::MoveNext),
            InstanceMethod("getCurrent", &NativeECSqlValueIterator::GetCurrent)});

            exports.Set("NativeECSqlValueIterator", t);

            s_constructor = Napi::Persistent(t);
            // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
            // from running at program shutdown time, which would attempt to reset the reference when
            // the environment is no longer valid.
            s_constructor.SuppressDestruct();
            }

        static Napi::Object New(Napi::Env const& env, IECSqlValueIterable const& iterable, ECDb const& ecdb)
            {
            return s_constructor.New({Napi::External<IECSqlValueIterable>::New(env, const_cast<IECSqlValueIterable*>(&iterable)), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
            }

        // A JS iterator expects the initial state of an iterator to be before the first element.
        // So on the first call to MoveNext, the C++ iterator must be created, and not incremented.
        Napi::Value MoveNext(Napi::CallbackInfo const& info)
            {
            if (m_isBeforeFirstElement)
                {
                m_it = m_iterable->begin();
                m_isBeforeFirstElement = false;
                }
            else
                {
                if (m_it != m_endIt)
                    ++m_it; //don't increment if the iterator is already at its end.
                }

            return Napi::Boolean::New(info.Env(), m_it != m_endIt);
            }

        Napi::Value GetCurrent(Napi::CallbackInfo const& info)
            {
            return NativeECSqlValue::New(Env(), *m_it, *m_ecdb);
            }
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               01/2018
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeECSqlValue::GetStructIterator(Napi::CallbackInfo const& info)
    {
    return NativeECSqlValueIterator::New(info.Env(), GetECSqlValue().GetStructIterable(), *m_ecdb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               01/2018
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeECSqlValue::GetArrayIterator(Napi::CallbackInfo const& info)
    {
    return NativeECSqlValueIterator::New(info.Env(), GetECSqlValue().GetArrayIterable(), *m_ecdb);
    }


//=======================================================================================
// Projects the ECSqlStatement class into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlStatement : Napi::ObjectWrap<NativeECSqlStatement>
{
private:
    #define MUST_HAVE_M_STMT if (m_stmt.get() == nullptr) Napi::TypeError::New(Env(), "Statement is not prepared").ThrowAsJavaScriptException();

    static Napi::FunctionReference s_constructor;
    std::unique_ptr<ECSqlStatement> m_stmt;

public:
    NativeECSqlStatement(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlStatement>(info), m_stmt(new ECSqlStatement()) {}

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeECSqlStatement", {
          InstanceMethod("prepare", &NativeECSqlStatement::Prepare),
          InstanceMethod("reset", &NativeECSqlStatement::Reset),
          InstanceMethod("dispose", &NativeECSqlStatement::Dispose),
          InstanceMethod("clearBindings", &NativeECSqlStatement::ClearBindings),
          InstanceMethod("getBinder", &NativeECSqlStatement::GetBinder),
          InstanceMethod("step", &NativeECSqlStatement::Step),
          InstanceMethod("stepForInsert", &NativeECSqlStatement::StepForInsert),
          InstanceMethod("getColumnCount", &NativeECSqlStatement::GetColumnCount),
          InstanceMethod("getValue", &NativeECSqlStatement::GetValue)
        });

        exports.Set("NativeECSqlStatement", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    Napi::Value Prepare(Napi::CallbackInfo const& info)
        {
        if (info.Length() < 2)
            THROW_TYPE_EXCEPTION_AND_RETURN("NativeECSqlStatement::Prepare requires two arguments", Env().Undefined());

        Napi::Object dbObj = info[0].As<Napi::Object>();

        ECDb* ecdb = nullptr;
        if (NativeDgnDb::InstanceOf(dbObj))
            {
            NativeDgnDb* addonDgndb = NativeDgnDb::Unwrap(dbObj);
            if (!addonDgndb->IsOpen())
                return NapiUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());

            ecdb = &addonDgndb->GetDgnDb();
            }
        else if (NativeECDb::InstanceOf(dbObj))
            {
            NativeECDb* addonECDb = NativeECDb::Unwrap(dbObj);
            ecdb = &addonECDb->GetECDb();

            if (!ecdb->IsDbOpen())
                return NapiUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());
            }
        else
            {
            Napi::TypeError::New(Env(), "Argument 0 must be a NativeDgnDb or NativeECDb object").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        REQUIRE_ARGUMENT_STRING(1, ecsql, Env().Undefined());

        BeSqliteDbMutexHolder serializeAccess(*ecdb); // hold mutex, so that we have a chance to get last ECDb error message

        ECSqlStatus status = m_stmt->Prepare(*ecdb, ecsql.c_str());
        return NapiUtils::CreateErrorObject0(ToDbResult(status), !status.IsSuccess() ? JsInterop::GetLastECDbIssue().c_str() : nullptr, Env());
        }

    Napi::Value Reset(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;
        ECSqlStatus status = m_stmt->Reset();
        return Napi::Number::New(Env(), (int) ToDbResult(status));
        }

    void Dispose(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;
        m_stmt = nullptr;
        }

    Napi::Value ClearBindings(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;
        auto status = m_stmt->ClearBindings();
        return Napi::Number::New(Env(), (int) ToDbResult(status));
        }

    Napi::Value GetBinder(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;

        if (info.Length() != 1)
            THROW_TYPE_EXCEPTION_AND_RETURN("GetBinder requires a parameter index or name as argument", Env().Undefined());

        Napi::Value paramArg = info[0];
        if (!paramArg.IsNumber() && !paramArg.IsString())
            THROW_TYPE_EXCEPTION_AND_RETURN("GetBinder requires a parameter index or name as argument", Env().Undefined());

        int paramIndex = -1;
        if (paramArg.IsNumber())
            paramIndex = (int) paramArg.ToNumber().Int32Value();
        else
            paramIndex = m_stmt->GetParameterIndex(paramArg.ToString().Utf8Value().c_str());

        IECSqlBinder& binder = m_stmt->GetBinder(paramIndex);
        return NativeECSqlBinder::New(info.Env(), binder, *m_stmt->GetECDb());
        }

    Napi::Value Step(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;
        DbResult status = m_stmt->Step();
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value StepForInsert(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;
        ECInstanceKey key;
        DbResult status = m_stmt->Step(key);

        Napi::Object ret = Napi::Object::New(Env());
        ret.Set(Napi::String::New(Env(), "status"), Napi::Number::New(Env(), (int) status));
        if (BE_SQLITE_DONE == status)
            ret.Set(Napi::String::New(Env(), "id"), Napi::String::New(Env(), key.GetInstanceId().ToHexStr().c_str()));

        return ret;
        }

    Napi::Value GetColumnCount(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;
        int colCount = m_stmt->GetColumnCount();
        return Napi::Number::New(info.Env(), colCount);
        }

    Napi::Value GetValue(Napi::CallbackInfo const& info)
        {
        MUST_HAVE_M_STMT;
        REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());

        IECSqlValue const& val = m_stmt->GetValue(colIndex);
        return NativeECSqlValue::New(info.Env(), val, *m_stmt->GetECDb());
        }

    static DbResult ToDbResult(ECSqlStatus status)
        {
        if (status.IsSuccess())
            return BE_SQLITE_OK;

        return status.IsSQLiteError() ? status.GetSQLiteError() : BE_SQLITE_ERROR;
        }
};

//=======================================================================================
// Projects the NativeECPresentationManager class into JS.
//! @bsiclass
//=======================================================================================
struct NativeECPresentationManager : Napi::ObjectWrap<NativeECPresentationManager>
    {
    static Napi::FunctionReference s_constructor;

    ConnectionManager m_connections;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;

    NativeECPresentationManager(Napi::CallbackInfo const& info)
        : Napi::ObjectWrap<NativeECPresentationManager>(info)
        {
        m_presentationManager = std::unique_ptr<RulesDrivenECPresentationManager>(ECPresentationUtils::CreatePresentationManager(m_connections, T_HOST.GetIKnownLocationsAdmin()));
        }

    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeECPresentationManager", {
          InstanceMethod("handleRequest", &NativeECPresentationManager::HandleRequest),
          InstanceMethod("setupRulesetDirectories", &NativeECPresentationManager::SetupRulesetDirectories)
        });

        exports.Set("NativeECPresentationManager", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    Napi::Value HandleRequest(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db, Env().Undefined());    // contract pre-conditions
        REQUIRE_ARGUMENT_STRING(1, serializedRequest, Env().Undefined());

        if (!db->IsOpen())
            return NapiUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());

        m_connections.NotifyConnectionOpened(db->GetDgnDb());

        Json::Value request;
        Json::Reader().parse(serializedRequest, request);
        if (request.isNull())
            return NapiUtils::CreateErrorObject0(ERROR, nullptr, Env());

        Utf8CP requestId = request["requestId"].asCString();
        if (Utf8String::IsNullOrEmpty(requestId))
            return NapiUtils::CreateErrorObject0(ERROR, nullptr, Env());

        JsonValueCR params = request["params"];
        rapidjson::Document response;
        if (0 == strcmp("GetRootNodesCount", requestId))
            ECPresentationUtils::GetRootNodesCount(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetRootNodes", requestId))
            ECPresentationUtils::GetRootNodes(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetChildrenCount", requestId))
            ECPresentationUtils::GetChildrenCount(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetChildren", requestId))
            ECPresentationUtils::GetChildren(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetNodePaths", requestId))
            ECPresentationUtils::GetNodePaths(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetFilteredNodesPaths", requestId))
            ECPresentationUtils::GetFilteredNodesPaths(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetContentDescriptor", requestId))
            ECPresentationUtils::GetContentDescriptor(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetContent", requestId))
            ECPresentationUtils::GetContent(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetContentSetSize", requestId))
            ECPresentationUtils::GetContentSetSize(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("GetDistinctValues", requestId))
            ECPresentationUtils::GetDistinctValues(*m_presentationManager, db->GetDgnDb(), params, response);
        else if (0 == strcmp("SaveValueChange", requestId))
            ECPresentationUtils::SaveValueChange(*m_presentationManager, db->GetDgnDb(), params, response);
        else
            return NapiUtils::CreateErrorObject0(ERROR, nullptr, Env());

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        response.Accept(writer);

        return Napi::String::New(Env(), buffer.GetString());
        }

    void SetupRulesetDirectories(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, rulesetDirectories,);
        ECPresentationUtils::SetupRulesetDirectories(*m_presentationManager, rulesetDirectories);
        }
    };

static Napi::Env* s_env;
static Napi::ObjectReference s_logger;
static intptr_t s_mainThreadId;
struct LogMessage {Utf8String m_category; Utf8String m_message; NativeLogging::SEVERITY m_severity;};
static bvector<LogMessage>* s_deferredLogging;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void ThrowJsExceptionOnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
    {
    if (BeThreadUtilities::GetCurrentThreadId() == s_mainThreadId)
        Napi::Error::New(*s_env, Utf8PrintfString("Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str()).ThrowAsJavaScriptException();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value GetLogger(Napi::CallbackInfo const& info)
    {
    return s_logger.Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void SetLogger(Napi::CallbackInfo const& info)
    {
    s_logger = Napi::ObjectReference::New(info[0].ToObject());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void logMessageToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    auto env = IModelJsNative::s_logger.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = (sev == NativeLogging::LOG_TRACE)?   "logTrace":
                   (sev == NativeLogging::LOG_DEBUG)?   "logTrace": // Logger does not distinguish between trace and debug
                   (sev == NativeLogging::LOG_INFO)?    "logInfo":
                   (sev == NativeLogging::LOG_WARNING)? "logWarning":
                                                        "logError"; // Logger does not distinguish between error and fatal

    auto method = IModelJsNative::s_logger.Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(*IModelJsNative::s_env, "Invalid Logger").ThrowAsJavaScriptException();
        return;
        }

    auto catJS = Napi::String::New(env, category);
    auto msgJS = Napi::String::New(env, msg);

    method({catJS, msgJS});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool callIsLogLevelEnabledJs(Utf8CP category, NativeLogging::SEVERITY sev)
    {
    auto env = IModelJsNative::s_logger.Env();
    Napi::HandleScope scope(env);

    auto method = IModelJsNative::s_logger.Get("isEnabled").As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(*IModelJsNative::s_env, "Invalid Logger").ThrowAsJavaScriptException();
        return true;
        }

    auto catJS = Napi::String::New(env, category);

    int llevel = (sev <= NativeLogging::LOG_TRACE)?   0:
                 (sev <= NativeLogging::LOG_DEBUG)?   0:        // Logger does not distinguish between trace and debug
                 (sev == NativeLogging::LOG_INFO)?    1:
                 (sev == NativeLogging::LOG_WARNING)? 2:
                                                      3;        // Logger does not distinguish between error and fatal

    auto levelJS = Napi::Number::New(env, llevel);

    return method({catJS, levelJS}).ToBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void deferLogging(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    BeSystemMutexHolder ___;

    if (!s_deferredLogging)
        s_deferredLogging = new bvector<LogMessage>();

    LogMessage lm;
    lm.m_category = category;
    lm.m_message = msg;
    lm.m_severity = sev;
    s_deferredLogging->push_back(lm);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void doDeferredLogging()
    {
    BeSystemMutexHolder ___;
    if (!s_deferredLogging)
        return;

    for (auto const& lm : *s_deferredLogging)
        {
        logMessageToJs(lm.m_category.c_str(), lm.m_severity, lm.m_message.c_str());
        }

    delete s_deferredLogging;
    s_deferredLogging = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isMainThread()
    {
    return BeThreadUtilities::GetCurrentThreadId() == IModelJsNative::s_mainThreadId;
    }

} // namespace IModelJsNative

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsNative::JsInterop::LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    if (IModelJsNative::s_logger.IsEmpty() || !IModelJsNative::isMainThread())
        {
        IModelJsNative::deferLogging(category, sev, msg);
        return;
        }

    IModelJsNative::doDeferredLogging();
    IModelJsNative::logMessageToJs(category, sev, msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool IModelJsNative::JsInterop::IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev)
    {
    if (IModelJsNative::s_logger.IsEmpty() || !IModelJsNative::isMainThread())
        return true;

    IModelJsNative::doDeferredLogging();
    return IModelJsNative::callIsLogLevelEnabledJs(category, sev);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void IModelJsNative::JsInterop::ThrowJsException(Utf8CP msg)
    {
    Napi::Error::New(*IModelJsNative::s_env, msg).ThrowAsJavaScriptException();
    }

static Utf8String s_mobileResourcesDir;
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Satyakam.Khadilkar    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void imodeljs_addon_setMobileResourcesDir(Utf8CP d) {s_mobileResourcesDir = d;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object iModelJsNativeRegisterModule(Napi::Env env, Napi::Object exports)
    {
    IModelJsNative::s_env = new Napi::Env(env);

    Napi::HandleScope scope(env);

#if (defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)) || defined(BENTLEYCONFIG_OS_LINUX) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
    BeFileName addondir = Desktop::FileSystem::GetLibraryDir();
#else
    BeFileName addondir(s_mobileResourcesDir);
#endif

    IModelJsNative::s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    IModelJsNative::JsInterop::Initialize(addondir, IModelJsNative::ThrowJsExceptionOnAssert);
    IModelJsNative::NativeDgnDb::Init(env, exports);
    IModelJsNative::NativeECDb::Init(env, exports);
    IModelJsNative::NativeECSqlStatement::Init(env, exports);
    IModelJsNative::NativeECSqlBinder::Init(env, exports);
    IModelJsNative::NativeECSqlValue::Init(env, exports);
    IModelJsNative::NativeECSqlColumnInfo::Init(env, exports);
    IModelJsNative::NativeECSqlValueIterator::Init(env, exports);
    IModelJsNative::NativeBriefcaseManagerResourcesRequest::Init(env, exports);
    IModelJsNative::NativeECPresentationManager::Init(env, exports);
    IModelJsNative::NativeECSchemaXmlContext::Init(env, exports);

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
        Napi::PropertyDescriptor::Accessor("logger", &IModelJsNative::GetLogger, &IModelJsNative::SetLogger),
        });

    return exports;
    }

Napi::FunctionReference IModelJsNative::NativeBriefcaseManagerResourcesRequest::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECSqlStatement::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECSqlBinder::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECSqlValue::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECSqlColumnInfo::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECSqlValueIterator::s_constructor;
Napi::FunctionReference IModelJsNative::NativeDgnDb::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECPresentationManager::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECDb::s_constructor;
Napi::FunctionReference IModelJsNative::NativeECSchemaXmlContext::s_constructor;

NODE_API_MODULE(at_bentley_imodeljs_nodeaddon, iModelJsNativeRegisterModule)
