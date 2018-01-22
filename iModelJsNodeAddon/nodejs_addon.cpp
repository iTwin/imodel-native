/*--------------------------------------------------------------------------------------+
|
|     $Source: nodejs_addon.cpp $
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
#include "AddonUtils.h"
#include "TestUtils.h"
#include <ECDb/ECDb.h>
#include <ECObjects/ECSchema.h>
#include <rapidjson/rapidjson.h>
#include "ECPresentationUtils.h"
#include <Bentley/Desktop/FileSystem.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_EC

#define PROPERTY_ATTRIBUTES static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)

#define RETURN_IF_HAD_EXCEPTION if (Env().IsExceptionPending()) return Env().Undefined();

#define REQUIRE_DB_TO_BE_OPEN if (!IsOpen()) return CreateBentleyReturnErrorObject(DgnDbStatus::NotOpen);

#define REQUIRE_ARGUMENT_ANY_OBJ(i, var)\
    if (info.Length() <= (i)) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be an object").ThrowAsJavaScriptException();\
    }\
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_OBJ(i, T, var)\
    if (info.Length() <= (i) || !T::InstanceOf(info[i])) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be an object of type " #T).ThrowAsJavaScriptException();\
    }\
    T* var = T::Unwrap(info[i].As<Napi::Object>());

#define REQUIRE_ARGUMENT_FUNCTION(i, var)\
    if (info.Length() <= (i) || !info[i].IsFunction()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be a function").ThrowAsJavaScriptException();\
    }\
    Napi::Function var = info[i].As<Napi::Function>();\

#define REQUIRE_ARGUMENT_STRING(i, var)\
    if (info.Length() <= (i) || !info[i].IsString()) {\
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a string").ThrowAsJavaScriptException();\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_STRING_ARRAY(i, var)\
    if (info.Length() <= (i) || !info[i].IsArray()) {\
        Napi::TypeError::New(info.Env(), "Argument " #i " must be an array of strings").ThrowAsJavaScriptException();\
    }\
    bvector<Utf8String> var;\
    Napi::Array arr = info[i].As<Napi::Array>();\
    for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {\
        Napi::Value arrValue = arr[arrIndex];\
        if (arrValue.IsString())\
            var.push_back(arrValue.As<Napi::String>().Utf8Value().c_str());\
    }

#define REQUIRE_ARGUMENT_NUMBER(i, var)\
    if (info.Length() <= (i) || !info[i].IsNumber()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be a number").ThrowAsJavaScriptException();\
    }\
    Napi::Number var = info[i].As<Napi::Number>();

#define REQUIRE_ARGUMENT_INTEGER(i, var)\
    if (info.Length() <= (i) || !info[i].IsNumber()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be an integer").ThrowAsJavaScriptException();\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define REQUIRE_ARGUMENT_BOOL(i, var)\
    if (info.Length() <= (i) || !info[i].IsBoolean()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be a boolean").ThrowAsJavaScriptException();\
    }\
    bool var = info[i].As<Napi::Boolean>().Value();

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default)\
    int var;\
    if (info.Length() <= (i) || info[i].IsUndefined()) {\
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
    if (info.Length() <= (i) || info[i].IsUndefined()) {\
        ;\
    }\
    else if (info[i].IsString()) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    }\
    else {\
        Napi::TypeError::New(Env(), "Argument " #i " must be string or undefined").ThrowAsJavaScriptException();\
    }

namespace IModelJsAddon {

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
struct AddonECDb : Napi::ObjectWrap<AddonECDb> 
    {
    private:
        static Napi::FunctionReference s_constructor;
        std::unique_ptr<ECDb> m_ecdb;

    public:
    AddonECDb(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AddonECDb>(info) {}
    ~AddonECDb() {}

        // Check if val is really a AddonECDb peer object
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

        Napi::Value CreateDb(const Napi::CallbackInfo& info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName);
            RETURN_IF_HAD_EXCEPTION
            const DbResult status = AddonUtils::CreateECDb(GetECDb(), BeFileName(dbName.c_str(), true));
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value OpenDb(const Napi::CallbackInfo& info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName);
            REQUIRE_ARGUMENT_INTEGER(1, mode);
            RETURN_IF_HAD_EXCEPTION

            const DbResult status = AddonUtils::OpenECDb(GetECDb(), BeFileName(dbName.c_str(), true), (Db::OpenMode) mode);
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value CloseDb(const Napi::CallbackInfo& info)
            {
            if (m_ecdb != nullptr)
                {
                m_ecdb->CloseDb();
                m_ecdb = nullptr;
                }

            return Napi::Number::New(Env(), (int) BE_SQLITE_OK);
            }

        Napi::Value Dispose(const Napi::CallbackInfo& info) { return CloseDb(info); }

        Napi::Value SaveChanges(const Napi::CallbackInfo& info)
            {
            OPTIONAL_ARGUMENT_STRING(0, changeSetName);
            RETURN_IF_HAD_EXCEPTION
            const DbResult status = GetECDb().SaveChanges(changeSetName.empty() ? nullptr : changeSetName.c_str());
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value AbandonChanges(const Napi::CallbackInfo& info)
            {
            DbResult status = GetECDb().AbandonChanges();
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value ImportSchema(const Napi::CallbackInfo& info)
            {
            REQUIRE_ARGUMENT_STRING(0, schemaPathName);
            RETURN_IF_HAD_EXCEPTION
            const DbResult status = AddonUtils::ImportSchema(GetECDb(), BeFileName(schemaPathName.c_str(), true));
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value IsOpen(const Napi::CallbackInfo& info) { return Napi::Boolean::New(Env(), GetECDb().IsDbOpen()); }

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
            Napi::Function t = DefineClass(env, "NodeAddonECDb", {
            InstanceMethod("createDb", &CreateDb),
            InstanceMethod("openDb", &OpenDb),
            InstanceMethod("closeDb", &CloseDb),
            InstanceMethod("dispose", &Dispose),
            InstanceMethod("saveChanges", &SaveChanges),
            InstanceMethod("abandonChanges", &AbandonChanges),
            InstanceMethod("importSchema", &ImportSchema),
            InstanceMethod("isOpen", &IsOpen)
            });

            exports.Set("NodeAddonECDb", t);

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
struct AddonBriefcaseManagerResourcesRequest : Napi::ObjectWrap<AddonBriefcaseManagerResourcesRequest>
{
    IBriefcaseManager::Request m_req;
    static Napi::FunctionReference s_constructor;

    AddonBriefcaseManagerResourcesRequest(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AddonBriefcaseManagerResourcesRequest>(info)
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
        Napi::Function t = DefineClass(env, "NodeAddonBriefcaseManagerResourcesRequest", {
          InstanceMethod("reset", &AddonBriefcaseManagerResourcesRequest::Reset),
          InstanceMethod("isEmpty", &AddonBriefcaseManagerResourcesRequest::IsEmpty),
          InstanceMethod("toJSON", &AddonBriefcaseManagerResourcesRequest::ToJSON),
        });

        exports.Set("NodeAddonBriefcaseManagerResourcesRequest", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    void Reset(const Napi::CallbackInfo& info)
        {
        m_req.Reset();
        }

    Napi::Value IsEmpty(const Napi::CallbackInfo& info)
        {
        return Napi::Number::New(Env(), (bool)m_req.IsEmpty());
        }

    Napi::Value ToJSON(const Napi::CallbackInfo& info)
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
struct AddonDgnDb : Napi::ObjectWrap<AddonDgnDb>
    {
    struct AddonAppData : Db::AppData 
        {
        AddonDgnDb& m_addonDb;
private:
        static Key const& GetKey()
            {
            static Key s_key;
            return s_key;
            }

        AddonAppData(AddonDgnDb& adb) : m_addonDb(adb) {}

        static AddonAppData* Find(DgnDbR db)
            {
            return (AddonAppData*) db.FindAppData(GetKey());
            }

        static void Add(AddonDgnDb& adb)
            {
            BeAssert(nullptr == Find(adb.GetDgnDb()));
            adb.GetDgnDb().AddAppData(GetKey(), new AddonAppData(adb));
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

    AddonDgnDb(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AddonDgnDb>(info)
        {
        }

    ~AddonDgnDb() 
        {
        CloseDgnDb();
        }

    void CloseDgnDb()
        {
        if (!m_dgndb.IsValid())
            return;
        TearDownPresentationManager();
        AddonUtils::CloseDgnDb(*m_dgndb);
        AddonAppData::Remove(GetDgnDb());
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
        AddonAppData::Add(*this);
        }

    static AddonDgnDb* From(DgnDbR db)
        {
        auto appdata = AddonAppData::Find(db);
        return appdata? &appdata->m_addonDb: nullptr;
        }

    DgnDbR GetDgnDb() {return *m_dgndb;}

    //  Check if val is really a AddonDgnDb peer object
    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(s_constructor.Value());
        }

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

    Napi::Value OpenDgnDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        RETURN_IF_HAD_EXCEPTION
        DgnDbPtr db;
        auto status = AddonUtils::OpenDgnDb(db, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        if (BE_SQLITE_OK == status)
            OnDgnDbOpened(db.get());

        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value GetECClassMetaData(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, s);
        REQUIRE_ARGUMENT_STRING(1, c);
        RETURN_IF_HAD_EXCEPTION
        Json::Value metaDataJson;
        auto status = AddonUtils::GetECClassMetaData(metaDataJson, GetDgnDb(), s.c_str(), c.c_str());
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), metaDataJson.ToString().c_str()));
        }

    Napi::Value GetElement(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, optsJsonStr);
        RETURN_IF_HAD_EXCEPTION
        Json::Value opts = Json::Value::From(optsJsonStr);
        Json::Value elementJson;  // ouput
        auto status = AddonUtils::GetElement(elementJson, GetDgnDb(), opts);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), elementJson.ToString().c_str()));
        }

    Napi::Value GetModel(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, optsJsonStr);
        RETURN_IF_HAD_EXCEPTION
        Json::Value opts = Json::Value::From(optsJsonStr);
        Json::Value modelJson;;  // ouput
        auto status = AddonUtils::GetModel(modelJson, GetDgnDb(), opts);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), modelJson.ToString().c_str()));
        }

    // Sets up a briefcase and opens it
    Napi::Value SetupBriefcase(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, briefcaseToken);
        RETURN_IF_HAD_EXCEPTION

        Json::Value jsonBriefcaseToken = Json::Value::From(briefcaseToken);
        
        DbResult result = AddonUtils::SetupBriefcase(m_dgndb, jsonBriefcaseToken);
        Napi::Object ret;
        if (BE_SQLITE_OK == result)
            OnDgnDbOpened(m_dgndb.get());

        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value ProcessChangeSets(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, changeSetTokens);
        REQUIRE_ARGUMENT_INTEGER(1, processOptions);
        RETURN_IF_HAD_EXCEPTION

        Json::Value jsonChangeSetTokens = Json::Value::From(changeSetTokens);

        DbResult result = AddonUtils::ProcessChangeSets(m_dgndb, jsonChangeSetTokens, (RevisionProcessOption) processOptions);
        return Napi::Number::New(Env(), (int) result);
        }

    Napi::Value StartCreateChangeSet(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION

        Json::Value changeSetInfo;
        DbResult result = AddonUtils::StartCreateChangeSet(changeSetInfo, *m_dgndb);
        return CreateBentleyReturnObject(result, Napi::String::New(Env(), changeSetInfo.ToString().c_str()));
        }

    Napi::Value FinishCreateChangeSet(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION

        DbResult result = AddonUtils::FinishCreateChangeSet(*m_dgndb);
        return Napi::Number::New(Env(), (int) result);
        }

    Napi::Value GetCachedBriefcaseInfos(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, cachePath);
        RETURN_IF_HAD_EXCEPTION

        Json::Value cachedBriefcaseInfos;
        BeFileName cacheFile(cachePath.c_str(), true);
        DbResult result = AddonUtils::GetCachedBriefcaseInfos(cachedBriefcaseInfos, cacheFile);
        return CreateBentleyReturnObject(result, Napi::String::New(Env(), cachedBriefcaseInfos.ToString().c_str()));
        }

    Napi::Value GetIModelProps(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION

        Json::Value props;
        AddonUtils::GetIModelProps(props, *m_dgndb);
        return Napi::String::New(Env(), props.ToString().c_str());
        }

    Napi::Value InsertElement(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        Json::Value elemIdJsonObj;
        auto status = AddonUtils::InsertElement(elemIdJsonObj, GetDgnDb(), elemProps);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), elemIdJsonObj.ToString().c_str()));
        }

    Napi::Value UpdateElement(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        auto status = AddonUtils::UpdateElement(GetDgnDb(), elemProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value DeleteElement(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        RETURN_IF_HAD_EXCEPTION

        auto status = AddonUtils::DeleteElement(GetDgnDb(), elemIdStr);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value InsertLinkTableRelationship(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr);
        RETURN_IF_HAD_EXCEPTION

        Json::Value props = Json::Value::From(propsJsonStr);
        Json::Value idJsonObj;
        auto status = AddonUtils::InsertLinkTableRelationship(idJsonObj, GetDgnDb(), props);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), idJsonObj.asCString()));
        }

    Napi::Value UpdateLinkTableRelationship(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr);
        RETURN_IF_HAD_EXCEPTION

        Json::Value props = Json::Value::From(propsJsonStr);
        auto status = AddonUtils::UpdateLinkTableRelationship(GetDgnDb(), props);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value DeleteLinkTableRelationship(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr);
        RETURN_IF_HAD_EXCEPTION

        Json::Value props = Json::Value::From(propsJsonStr);
        auto status = AddonUtils::DeleteLinkTableRelationship(GetDgnDb(), props);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value InsertCodeSpec(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, name);
        REQUIRE_ARGUMENT_INTEGER(1, specType);
        REQUIRE_ARGUMENT_INTEGER(2, scopeReq);

        if ((uint32_t)CodeScopeSpec::Type::Model != specType && (uint32_t)CodeScopeSpec::Type::ParentElement != specType &&
            (uint32_t)CodeScopeSpec::Type::RelatedElement != specType && (uint32_t)CodeScopeSpec::Type::Repository != specType)
            {
            Napi::TypeError::New(Env(), "Argument 1 must be a CodeScopeSpec.Type").ThrowAsJavaScriptException();
            }
        else if ((uint32_t)CodeScopeSpec::ScopeRequirement::ElementId != scopeReq && (uint32_t)CodeScopeSpec::ScopeRequirement::FederationGuid != scopeReq)
            {
            Napi::TypeError::New(Env(), "Argument 2 must be a CodeScopeSpec.ScopeRequirement").ThrowAsJavaScriptException();
            }
        
        RETURN_IF_HAD_EXCEPTION

        Utf8String idStr;
        DgnDbStatus status = AddonUtils::InsertCodeSpec(idStr, GetDgnDb(), name, (CodeScopeSpec::Type)specType, (CodeScopeSpec::ScopeRequirement)scopeReq);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), idStr.c_str()));
        }

    Napi::Value InsertModel(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        Json::Value elemIdJsonObj;
        auto status = AddonUtils::InsertModel(elemIdJsonObj, GetDgnDb(), elemProps);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), elemIdJsonObj.ToString().c_str()));
        }

    Napi::Value UpdateModel(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        auto status = AddonUtils::UpdateModel(GetDgnDb(), elemProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value DeleteModel(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        RETURN_IF_HAD_EXCEPTION

        auto status = AddonUtils::DeleteModel(GetDgnDb(), elemIdStr);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value GetElementPropertiesForDisplay(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elementIdStr);
        RETURN_IF_HAD_EXCEPTION

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
        ECInstanceNodeKeyPtr nodeKey = ECInstanceNodeKey::Create(ecclassId, elemId);
        NavNodeKeyList keyList;
        keyList.push_back(nodeKey);
        INavNodeKeysContainerCPtr selectedNodeKeys = NavNodeKeyListContainer::Create(keyList);
        SelectionInfo selection ("iModelJS", false, *selectedNodeKeys);
        RulesDrivenECPresentationManager::ContentOptions options ("Items");
        if ( m_presentationManager == nullptr)
            return CreateBentleyReturnErrorObject(DgnDbStatus::BadArg);
        
        ContentDescriptorCPtr descriptor = m_presentationManager->GetContentDescriptor(GetDgnDb(), ContentDisplayType::PropertyPane, selection, options.GetJson()).get();
        if (descriptor.IsNull())
            return CreateBentleyReturnErrorObject(DgnDbStatus::BadArg);

        PageOptions pageOptions;
        pageOptions.SetPageStart(0);
        pageOptions.SetPageSize(0);
        ContentCPtr content = m_presentationManager->GetContent(GetDgnDb(), *descriptor, selection, pageOptions, options.GetJson()).get();
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

    Napi::Value SaveChanges(const Napi::CallbackInfo& info)
        {
        OPTIONAL_ARGUMENT_STRING(0, description);
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION
        auto stat = GetDgnDb().SaveChanges(description);
        return Napi::Number::New(Env(), (int)stat);
        }

    Napi::Value AbandonChanges(const Napi::CallbackInfo& info)
        {            
        DbResult status = GetDgnDb().AbandonChanges();
        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value ImportSchema(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, schemaPathnameStrObj);
        RETURN_IF_HAD_EXCEPTION
        BeFileName schemaPathname(schemaPathnameStrObj.c_str(), true);
        auto stat = AddonUtils::ImportSchemaDgnDb(GetDgnDb(), schemaPathname);
        return Napi::Number::New(Env(), (int)stat);
        }

    void CloseDgnDb(const Napi::CallbackInfo& info)
        {
        CloseDgnDb();
        }

    Napi::Value CreateChangeCache(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_OBJ(0, AddonECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changeCachePathStr);
        RETURN_IF_HAD_EXCEPTION

        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().CreateChangeCache(changeCacheECDb->GetECDb(), changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value AttachChangeCache(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, changeCachePathStr);
        RETURN_IF_HAD_EXCEPTION

        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().AttachChangeCache(changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value IsChangeCacheAttached(const Napi::CallbackInfo&)
        {
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION

        bool isAttached = GetDgnDb().IsChangeCacheAttached();
        return Napi::Boolean::New(Env(), isAttached);
        }

    Napi::Value ExtractChangeSummary(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_OBJ(0, AddonECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changesetFilePathStr);
        RETURN_IF_HAD_EXCEPTION

        struct AddonChangeSet : BeSQLite::ChangeSet
            {
            ConflictResolution _OnConflict(ConflictCause cause, BeSQLite::Changes::Change iter) override { return ConflictResolution::Skip; }
            };

        AddonChangeSet changeset;
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

    Napi::Value SetBriefcaseId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_INTEGER(0, idvalue);
        RETURN_IF_HAD_EXCEPTION

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
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION
        auto bid = m_dgndb->GetBriefcaseId();
        return Napi::Number::New(Env(), bid.GetValue());
        }

    Napi::Value GetParentChangeSetId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION
        Utf8String parentRevId = m_dgndb->Revisions().GetParentRevisionId();
        return Napi::String::New(Env(), parentRevId.c_str());
        }

    Napi::Value GetDbGuid(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION
        BeGuid beGuid = m_dgndb->GetDbGuid();
        return Napi::String::New(Env(), beGuid.ToString().c_str());
        }

    Napi::Value SetDbGuid(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, guidStr);
        RETURN_IF_HAD_EXCEPTION
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        m_dgndb->ChangeDbGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForElement(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, AddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_STRING(1, elemProps);
        REQUIRE_ARGUMENT_INTEGER(2, dbop);
        Json::Value elemPropsJson = Json::Value::From(elemProps);
        return Napi::Number::New(Env(), (int)AddonUtils::BuildBriefcaseManagerResourcesRequestForElement(req->m_req, GetDgnDb(), elemPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, AddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_STRING(1, props);
        REQUIRE_ARGUMENT_INTEGER(2, dbop);
        Json::Value propsJson = Json::Value::From(props);
        return Napi::Number::New(Env(), (int)AddonUtils::BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(req->m_req, GetDgnDb(), propsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForModel(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, AddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_STRING(1, modelProps);
        REQUIRE_ARGUMENT_INTEGER(2, dbop);
        Json::Value modelPropsJson = Json::Value::From(modelProps);
        return Napi::Number::New(Env(), (int)AddonUtils::BuildBriefcaseManagerResourcesRequestForModel(req->m_req, GetDgnDb(), modelPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    void ExtractBulkResourcesRequest(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, AddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_BOOL(1, locks);
        REQUIRE_ARGUMENT_BOOL(2, codes);
        GetDgnDb().BriefcaseManager().ExtractRequestFromBulkOperation(req->m_req, locks, codes);
        }

    Napi::Value InBulkOperation(const Napi::CallbackInfo& info)
        {
        return Napi::Boolean::New(Env(), GetDgnDb().BriefcaseManager().IsBulkOperation());
        }
		
   void AppendBriefcaseManagerResourcesRequest(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, AddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_OBJ(1, AddonBriefcaseManagerResourcesRequest, reqIn);
        req->m_req.Codes().insert(reqIn->m_req.Codes().begin(), reqIn->m_req.Codes().end());
        req->m_req.Locks().GetLockSet().insert(reqIn->m_req.Locks().GetLockSet().begin(), reqIn->m_req.Locks().GetLockSet().end());
        // TBD: merge in request options 
        }

   void ExtractBriefcaseManagerResourcesRequest(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, AddonBriefcaseManagerResourcesRequest, reqOut);
        REQUIRE_ARGUMENT_OBJ(1, AddonBriefcaseManagerResourcesRequest, reqIn);
        REQUIRE_ARGUMENT_BOOL(2, locks);
        REQUIRE_ARGUMENT_BOOL(3, codes);
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

    Napi::Value SetBriefcaseManagerOptimisticConcurrencyControlPolicy(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, conflictRes);
        OptimisticConcurrencyControl::OnConflict uu = (OptimisticConcurrencyControl::OnConflict)conflictRes.Get("updateVsUpdate").ToNumber().Int32Value();
        OptimisticConcurrencyControl::OnConflict ud = (OptimisticConcurrencyControl::OnConflict)conflictRes.Get("updateVsDelete").ToNumber().Int32Value();
        OptimisticConcurrencyControl::OnConflict du = (OptimisticConcurrencyControl::OnConflict)conflictRes.Get("deleteVsUpdate").ToNumber().Int32Value();
        if (OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange != uu && OptimisticConcurrencyControl::OnConflict::RejectIncomingChange != uu
         || OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange != ud && OptimisticConcurrencyControl::OnConflict::RejectIncomingChange != ud
         || OptimisticConcurrencyControl::OnConflict::AcceptIncomingChange != du && OptimisticConcurrencyControl::OnConflict::RejectIncomingChange != du)
            {
            Napi::TypeError::New(Env(), "Invalid conflict resolution value").ThrowAsJavaScriptException();
            return Napi::Number::New(Env(), 1);
            }
        OptimisticConcurrencyControl::Policy policy;
        policy.deleteVsUpdate = du;
        policy.updateVsDelete = uu;
        policy.updateVsDelete = ud;

        return Napi::Number::New(Env(), (int)GetDgnDb().SetConcurrencyControl(new OptimisticConcurrencyControl(policy)));
        }

    Napi::Value SetBriefcaseManagerPessimisticConcurrencyControlPolicy(const Napi::CallbackInfo& info)
        {
        return Napi::Number::New(Env(), (int)GetDgnDb().SetConcurrencyControl(new PessimisticConcurrencyControl()));
        }

    Napi::Value BriefcaseManagerStartBulkOperation(const Napi::CallbackInfo& info)
        {
        return Napi::Number::New(Env(), (int)AddonUtils::BriefcaseManagerStartBulkOperation(GetDgnDb()));
        }

    Napi::Value BriefcaseManagerEndBulkOperation(const Napi::CallbackInfo& info)
        {
        return Napi::Number::New(Env(), (int)AddonUtils::BriefcaseManagerEndBulkOperation(GetDgnDb()));
        }

	Napi::Value UpdateProjectExtents(const Napi::CallbackInfo& info)
	{
		REQUIRE_ARGUMENT_STRING(0, newExtentsJson)
		return Napi::Number::New(Env(), (int)AddonUtils::UpdateProjectExtents(GetDgnDb(), Json::Value::From(newExtentsJson)));
	}

	// ========================================================================================
	// Test method handler
	// Note: This is where the developer may specify, given an ID from JS, what function should
	//		 executed and returned.
	// ========================================================================================
	Napi::Value ExecuteTestById(const Napi::CallbackInfo& info)
		{
		REQUIRE_DB_TO_BE_OPEN
		REQUIRE_ARGUMENT_INTEGER(0, testId);
		REQUIRE_ARGUMENT_STRING(1, params);
		RETURN_IF_HAD_EXCEPTION

		switch (testId)
			{
			case 1:
				return Napi::String::New(Env(), TestUtils::ViewStateCreate(params).ToString().c_str());
			case 2:
				return Napi::String::New(Env(), TestUtils::ViewStateVolumeAdjustments(params).ToString().c_str());
			case 3:
				return Napi::String::New(Env(), TestUtils::ViewStateLookAt(params).ToString().c_str());
			case 4:
				return Napi::String::New(Env(), TestUtils::DeserializeGeometryStream(params).ToString().c_str());
			case 5:
				return Napi::String::New(Env(), TestUtils::BuildKnownGeometryStream(params).ToString().c_str());
			default:
				return Napi::String::New(Env(), "{}");
			}
		}


    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NodeAddonDgnDb", {
            InstanceMethod("openDgnDb", &OpenDgnDb),
            InstanceMethod("closeDgnDb", &CloseDgnDb),
            InstanceMethod("processChangeSets", &ProcessChangeSets),
            InstanceMethod("startCreateChangeSet", &StartCreateChangeSet),
            InstanceMethod("finishCreateChangeSet", &FinishCreateChangeSet),
            InstanceMethod("createChangeCache", &CreateChangeCache),
            InstanceMethod("attachChangeCache", &AttachChangeCache),
            InstanceMethod("isChangeCacheAttached", &IsChangeCacheAttached),
            InstanceMethod("extractChangeSummary", &ExtractChangeSummary),
            InstanceMethod("setBriefcaseId", &SetBriefcaseId),
            InstanceMethod("getBriefcaseId", &GetBriefcaseId),
            InstanceMethod("getParentChangeSetId", &GetParentChangeSetId),
            InstanceMethod("getDbGuid", &GetDbGuid),
            InstanceMethod("setDbGuid", &SetDbGuid),
            InstanceMethod("setupBriefcase", &SetupBriefcase),
            InstanceMethod("saveChanges", &SaveChanges),
            InstanceMethod("abandonChanges", &AbandonChanges),
            InstanceMethod("importSchema", &ImportSchema),
            InstanceMethod("getElement", &GetElement),
            InstanceMethod("getModel", &GetModel),
            InstanceMethod("insertElement", &InsertElement),
            InstanceMethod("updateElement", &UpdateElement),
            InstanceMethod("deleteElement", &DeleteElement),
            InstanceMethod("insertModel", &InsertModel),
            InstanceMethod("updateModel", &UpdateModel),
            InstanceMethod("deleteModel", &DeleteModel),
            InstanceMethod("insertCodeSpec", &InsertCodeSpec),
            InstanceMethod("getElementPropertiesForDisplay", &GetElementPropertiesForDisplay),
            InstanceMethod("insertLinkTableRelationship", &InsertLinkTableRelationship),
            InstanceMethod("updateLinkTableRelationship", &UpdateLinkTableRelationship),
            InstanceMethod("deleteLinkTableRelationship", &DeleteLinkTableRelationship),
            InstanceMethod("getECClassMetaData", &GetECClassMetaData),
            InstanceMethod("getECClassMetaData", &GetECClassMetaData),
            InstanceMethod("getCachedBriefcaseInfos", &GetCachedBriefcaseInfos),
            InstanceMethod("getIModelProps", &GetIModelProps),
			InstanceMethod("updateProjectExtents", &UpdateProjectExtents),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForModel", &BuildBriefcaseManagerResourcesRequestForModel),
            InstanceMethod("setBriefcaseManagerOptimisticConcurrencyControlPolicy", &SetBriefcaseManagerOptimisticConcurrencyControlPolicy),
            InstanceMethod("setBriefcaseManagerPessimisticConcurrencyControlPolicy", &SetBriefcaseManagerPessimisticConcurrencyControlPolicy),
            InstanceMethod("briefcaseManagerStartBulkOperation", &BriefcaseManagerStartBulkOperation),
            InstanceMethod("briefcaseManagerEndBulkOperation", &BriefcaseManagerEndBulkOperation),
            InstanceMethod("extractBulkResourcesRequest", &ExtractBulkResourcesRequest),
            InstanceMethod("appendBriefcaseManagerResourcesRequest", &AppendBriefcaseManagerResourcesRequest),
            InstanceMethod("extractBriefcaseManagerResourcesRequest", &ExtractBriefcaseManagerResourcesRequest),
            InstanceMethod("inBulkOperation", &InBulkOperation),
						
			// DEVELOPMENT-ONLY METHODS:
			InstanceMethod("executeTestById", &ExecuteTestById),

        });

        exports.Set("NodeAddonDgnDb", t);

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
struct AddonECSqlBinder : Napi::ObjectWrap<AddonECSqlBinder>
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
    AddonECSqlBinder(Napi::CallbackInfo const& info) : Napi::ObjectWrap<AddonECSqlBinder>(info) 
        {
        if (info.Length() != 2)
            Napi::TypeError::New(Env(), "AddonECSqlBinder constructor expects two arguments.").ThrowAsJavaScriptException();

        m_binder = info[0].As<Napi::External<IECSqlBinder>>().Data();
        if (m_binder == nullptr)
            Napi::TypeError::New(Env(), "Invalid first arg for AddonECSqlBinder constructor. IECSqlBinder must not be nullptr").ThrowAsJavaScriptException();

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            Napi::TypeError::New(Env(), "Invalid second arg for AddonECSqlBinder constructor. ECDb must not be nullptr").ThrowAsJavaScriptException();
        }

    ~AddonECSqlBinder() {}

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
        Napi::Function t = DefineClass(env, "NodeAddonECSqlBinder", {
        InstanceMethod("dispose", &Dispose),
        InstanceMethod("bindNull", &BindNull),
        InstanceMethod("bindBlob", &BindBlob),
        InstanceMethod("bindBoolean", &BindBoolean),
        InstanceMethod("bindDateTime", &BindDateTime),
        InstanceMethod("bindDouble", &BindDouble),
        InstanceMethod("bindId", &BindId),
        InstanceMethod("bindInt", &BindInt),
        InstanceMethod("bindInt64", &BindInt64),
        InstanceMethod("bindPoint2d", &BindPoint2d),
        InstanceMethod("bindPoint3d", &BindPoint3d),
        InstanceMethod("bindString", &BindString),
        InstanceMethod("bindNavigation", &BindNavigation),
        InstanceMethod("bindMember", &BindMember),
        InstanceMethod("addArrayElement", &AddArrayElement)
        });

        exports.Set("NodeAddonECSqlBinder", t);

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

    void Dispose(const Napi::CallbackInfo& info)
        {
        if (m_binder != nullptr)
            m_binder = nullptr;

        if (m_ecdb != nullptr)
            m_ecdb = nullptr;
        }

    Napi::Value BindNull(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "BindNull must not have arguments").ThrowAsJavaScriptException();

        const ECSqlStatus stat = GetBinder().BindNull();
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBlob(const Napi::CallbackInfo& info)
        {
        Napi::Value blobVal;
        if (info.Length() == 0 || !(blobVal = info[0]).IsString())
            Napi::TypeError::New(info.Env(), "BindBlob expects a base64 string").ThrowAsJavaScriptException();

        Utf8String base64Str(blobVal.ToString().Utf8Value().c_str());
        ByteStream blob;
        Base64Utilities::Decode(blob, base64Str);

        const ECSqlStatus stat = GetBinder().BindBlob(blob.data(), (int) blob.size(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBoolean(const Napi::CallbackInfo& info)
        {
        Napi::Value boolVal;
        if (info.Length() == 0 || !(boolVal = info[0]).IsBoolean())
            Napi::TypeError::New(info.Env(), "BindBoolean expects a boolean").ThrowAsJavaScriptException();

        const ECSqlStatus stat = GetBinder().BindBoolean(boolVal.ToBoolean());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDateTime(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, isoString);

        DateTime dt;
        if (SUCCESS != DateTime::FromString(dt, isoString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        const ECSqlStatus stat = GetBinder().BindDateTime(dt);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDouble(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_NUMBER(0, val);
        const ECSqlStatus stat = GetBinder().BindDouble(val.DoubleValue());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindId(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, hexString);

        BeInt64Id id;
        if (SUCCESS != BeInt64Id::FromString(id, hexString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        const ECSqlStatus stat = GetBinder().BindId(id);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindInt(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_INTEGER(0, val);

        const ECSqlStatus stat = GetBinder().BindInt((int) val);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindInt64(const Napi::CallbackInfo& info)
        {
        if (info.Length() == 0)
            Napi::TypeError::New(info.Env(), "BindInt64 expects a string or number").ThrowAsJavaScriptException();
        
        Napi::Value val = info[0];
        if (!val.IsNumber() && !val.IsString())
            Napi::TypeError::New(info.Env(), "BindInt64 expects a string or number").ThrowAsJavaScriptException();

        int64_t int64Val;
        if (val.IsNumber())
            int64Val = val.ToNumber().Int64Value();
        else
            {
            Utf8String strVal(val.ToString().Utf8Value().c_str());
            if (BeStringUtilities::HasHexPrefix(strVal.c_str()))
                {
                BentleyStatus hexParseStat = SUCCESS;
                int64Val = (int64_t) BeStringUtilities::ParseHex(strVal.c_str(), &hexParseStat);
                if (SUCCESS != hexParseStat)
                    return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);
                }
            else
                sscanf(strVal.c_str(), "%" SCNi64, &int64Val);
            }

        const ECSqlStatus stat = GetBinder().BindInt64(int64Val);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint2d(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_NUMBER(0, x);
        REQUIRE_ARGUMENT_NUMBER(1, y);

        const ECSqlStatus stat = GetBinder().BindPoint2d(DPoint2d::From(x.DoubleValue(),y.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint3d(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_NUMBER(0, x);
        REQUIRE_ARGUMENT_NUMBER(1, y);
        REQUIRE_ARGUMENT_NUMBER(2, z);

        const ECSqlStatus stat = GetBinder().BindPoint3d(DPoint3d::From(x.DoubleValue(), y.DoubleValue(), z.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindString(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, val);

        const ECSqlStatus stat = GetBinder().BindText(val.c_str(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindNavigation(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, navIdHexStr);
        OPTIONAL_ARGUMENT_STRING(1, relClassName);
        OPTIONAL_ARGUMENT_STRING(2, relClassTableSpaceName);

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

        const ECSqlStatus stat = GetBinder().BindNavigation(navId, relClassId);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindMember(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, memberName);
        IECSqlBinder& memberBinder = GetBinder()[memberName.c_str()];
        return New(info.Env(), memberBinder, *m_ecdb);
        }

    Napi::Value AddArrayElement(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "AddArrayElement must not have arguments").ThrowAsJavaScriptException();

        IECSqlBinder& elementBinder = GetBinder().AddArrayElement();
        return New(info.Env(), elementBinder, *m_ecdb);
        }
    };

//=======================================================================================
// Projects the ECSqlStatement class into JS.
//! @bsiclass
//=======================================================================================
struct AddonECSqlStatement : Napi::ObjectWrap<AddonECSqlStatement>
{
private:
    #define MUST_HAVE_M_STMT if (m_stmt.get() == nullptr) Napi::TypeError::New(Env(), "Statement is not prepared").ThrowAsJavaScriptException();

    static Napi::FunctionReference s_constructor;
    std::unique_ptr<ECSqlStatement> m_stmt;

public:
    AddonECSqlStatement(const Napi::CallbackInfo& info) : Napi::ObjectWrap<AddonECSqlStatement>(info), m_stmt(new ECSqlStatement()) {}

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NodeAddonECSqlStatement", {
          InstanceMethod("prepare", &Prepare),
          InstanceMethod("reset", &Reset),
          InstanceMethod("dispose", &Dispose),
          InstanceMethod("clearBindings", &ClearBindings),
          InstanceMethod("getBinder", &GetBinder),
          InstanceMethod("bindValues", &BindValues),
          InstanceMethod("step", &Step),
          InstanceMethod("stepForInsert", &StepForInsert),
          InstanceMethod("getRow", &GetRow),
        });

        exports.Set("NodeAddonECSqlStatement", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    Napi::Value Prepare(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 2) 
            {
            Napi::TypeError::New(Env(), "AddonECSqlStatement::Prepare requires two arguments").ThrowAsJavaScriptException();
            return NapiUtils::CreateErrorObject0(BE_SQLITE_ERROR, nullptr, Env());
            }

        Napi::Object dbObj = info[0].As<Napi::Object>();

        ECDb* ecdb = nullptr;
        if (AddonDgnDb::InstanceOf(dbObj))
            {
            AddonDgnDb* addonDgndb = AddonDgnDb::Unwrap(dbObj);
            if (!addonDgndb->IsOpen())
                return NapiUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());

            ecdb = &addonDgndb->GetDgnDb();
            }
        else if (AddonECDb::InstanceOf(dbObj))
            {
            AddonECDb* addonECDb = AddonECDb::Unwrap(dbObj);
            ecdb = &addonECDb->GetECDb();

            if (!ecdb->IsDbOpen())
                return NapiUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());
            }
        else 
            {
            Napi::TypeError::New(Env(), "Argument 0 must be a AddonDgnDb or AddonECDb object").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        REQUIRE_ARGUMENT_STRING(1, ecsql);

        BeSqliteDbMutexHolder serializeAccess(*ecdb); // hold mutex, so that we have a chance to get last ECDb error message

        const ECSqlStatus status = m_stmt->Prepare(*ecdb, ecsql.c_str());
        return NapiUtils::CreateErrorObject0(ToDbResult(status), !status.IsSuccess() ? AddonUtils::GetLastECDbIssue().c_str() : nullptr, Env());
        }

    Napi::Value Reset(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        const ECSqlStatus status = m_stmt->Reset();
        return Napi::Number::New(Env(), (int) ToDbResult(status));
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
        return Napi::Number::New(Env(), (int) ToDbResult(status));
        }

    Napi::Value GetBinder(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;

        if (info.Length() != 1)
            {
            Napi::TypeError::New(Env(), "GetBinder requires a parameter index or name as argument").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        Napi::Value paramArg = info[0];
        if (!paramArg.IsNumber() && !paramArg.IsString())
            {
            Napi::TypeError::New(Env(), "GetBinder requires a parameter index or name as argument").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        int paramIndex = -1;
        if (paramArg.IsNumber())
            paramIndex = (int) paramArg.ToNumber().Int32Value();
        else
            paramIndex = m_stmt->GetParameterIndex(paramArg.ToString().Utf8Value().c_str());

        IECSqlBinder& binder = m_stmt->GetBinder(paramIndex);
        return New(info.Env(), binder, *m_stmt->GetECDb());
        }

    //! @deprecated Use AddonECSqlStatement::GetBinder instead
    Napi::Value BindValues(const Napi::CallbackInfo& info)
       {
       MUST_HAVE_M_STMT;
       REQUIRE_ARGUMENT_STRING(0, valuesStr);

       //BeSqliteDbMutexHolder serializeAccess(*db->m_addondb); // hold mutex, so that we have a chance to get last ECDb error message

       auto status = AddonUtils::JsonBinder::BindValues(*m_stmt, Json::Value::From(valuesStr));

       if (BSISUCCESS != status)
           return NapiUtils::CreateErrorObject0(BE_SQLITE_ERROR, AddonUtils::GetLastECDbIssue().c_str(), Env());

       return NapiUtils::CreateErrorObject0(BE_SQLITE_OK, nullptr, Env());
       }


    Napi::Value Step(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        const DbResult status = m_stmt->Step();
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value StepForInsert(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        ECInstanceKey key;
        const DbResult status = m_stmt->Step(key);

        Napi::Object ret = Napi::Object::New(Env());
        ret.Set(Napi::String::New(Env(), "status"), Napi::Number::New(Env(), (int) status));
        if (BE_SQLITE_DONE == status)
            ret.Set(Napi::String::New(Env(), "id"), Napi::String::New(Env(), key.GetInstanceId().ToHexStr().c_str()));

        return ret;
        }

    Napi::Value GetRow(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        Json::Value rowJson(Json::objectValue);
        AddonUtils::GetRowAsJson(rowJson, *m_stmt);
        // *** NEEDS WORK: Get the adapter to set the js object's properties directly
        return Napi::String::New(Env(), rowJson.ToString().c_str());
        }

    static DbResult ToDbResult(ECSqlStatus status) 
        {
        if (status.IsSuccess())
            return BE_SQLITE_OK;

        return status.IsSQLiteError() ? status.GetSQLiteError() : BE_SQLITE_ERROR; 
        }

};

//=======================================================================================
// Projects the AddonECPresentationManager class into JS.
//! @bsiclass
//=======================================================================================
struct AddonECPresentationManager : Napi::ObjectWrap<AddonECPresentationManager>
    {
    static Napi::FunctionReference s_constructor;

    ConnectionManager m_connections;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;
    
    AddonECPresentationManager(const Napi::CallbackInfo& info) 
        : Napi::ObjectWrap<AddonECPresentationManager>(info)
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
        Napi::Function t = DefineClass(env, "NodeAddonECPresentationManager", {
          InstanceMethod("handleRequest", &HandleRequest),
          InstanceMethod("setupRulesetDirectories", &SetupRulesetDirectories)
        });

        exports.Set("NodeAddonECPresentationManager", t);

        s_constructor = Napi::Persistent(t);
        // Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
        // from running at program shutdown time, which would attempt to reset the reference when
        // the environment is no longer valid.
        s_constructor.SuppressDestruct();
        }

    Napi::Value HandleRequest(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, AddonDgnDb, db);    // contract pre-conditions
        REQUIRE_ARGUMENT_STRING(1, serializedRequest);

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

    void SetupRulesetDirectories(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, rulesetDirectories);
        ECPresentationUtils::SetupRulesetDirectories(*m_presentationManager, rulesetDirectories);
        }
    };

static Napi::Env* s_env;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void ThrowJsExceptionOnAssert(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
    {
    Napi::Error::New(*s_env, Utf8PrintfString("Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str()).ThrowAsJavaScriptException();
    }

} // namespace IModelJsAddon


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void BentleyApi::Dgn::AddonUtils::ThrowJsException(Utf8CP msg)
    {
    Napi::Error::New(*IModelJsAddon::s_env, msg).ThrowAsJavaScriptException();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object iModelJsAddonRegisterModule(Napi::Env env, Napi::Object exports)
    {
    IModelJsAddon::s_env = new Napi::Env(env);

    Napi::HandleScope scope(env);

    BeFileName addondir = Desktop::FileSystem::GetLibraryDir();

    AddonUtils::Initialize(addondir, IModelJsAddon::ThrowJsExceptionOnAssert);
    IModelJsAddon::AddonDgnDb::Init(env, exports);
    IModelJsAddon::AddonECDb::Init(env, exports);
    IModelJsAddon::AddonECSqlStatement::Init(env, exports);
    Init(env, exports);
    IModelJsAddon::AddonBriefcaseManagerResourcesRequest::Init(env, exports);
    IModelJsAddon::AddonECPresentationManager::Init(env, exports);

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
        });

    return exports;
    }

Napi::FunctionReference IModelJsAddon::AddonBriefcaseManagerResourcesRequest::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECSqlStatement::s_constructor;
Napi::FunctionReference s_constructor;
Napi::FunctionReference IModelJsAddon::AddonDgnDb::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECPresentationManager::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECDb::s_constructor;

NODE_API_MODULE(at_bentley_imodeljs_nodeaddon, iModelJsAddonRegisterModule)
