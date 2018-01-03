/*--------------------------------------------------------------------------------------+
|
|     $Source: nodejs_addon.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)
#include <windows.h>
#endif
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
#include "TestUtils.h"
#include <ECObjects/ECSchema.h>
#include <rapidjson/rapidjson.h>
#include "ECPresentationUtils.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_EC

#define PROPERTY_ATTRIBUTES static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)

#define RETURN_IF_HAD_EXCEPTION if (Env().IsExceptionPending()) return;
#define RETURN_IF_HAD_EXCEPTION_SYNC if (Env().IsExceptionPending()) return Env().Undefined();

#define REQUIRE_DB_TO_BE_OPEN      if (!m_addondb->IsOpen()) {m_status = DgnDbStatus::NotOpen; return;}
#define REQUIRE_DB_TO_BE_OPEN_SYNC if (!IsOpen()) return CreateBentleyReturnErrorObject(DgnDbStatus::NotOpen);

#define REQUIRE_ARGUMENT_ANY_OBJ(i, var)\
    if (info.Length() <= (i)) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be an object").ThrowAsJavaScriptException();\
    }\
    Napi::Object var = info[0].As<Napi::Object>();

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
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a string").ThrowAsJavaScriptException();\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_INTEGER(i, var)\
    if (info.Length() <= (i) || !info[i].IsNumber()) {\
        Napi::TypeError::New(Env(), "Argument " #i " must be an integer").ThrowAsJavaScriptException();\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

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
struct NodeAddonECDb : Napi::ObjectWrap<NodeAddonECDb> 
{
    static Napi::FunctionReference s_constructor;

    JsECDbPtr m_ecdb;
    NodeAddonECDb(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeAddonECDb>(info), m_ecdb(new JsECDb()) {}
    ~NodeAddonECDb() {}

    bool IsValid() const { return m_ecdb != nullptr; }
    ECDbR GetECDb() {return *m_ecdb;}

    // Check if val is really a NodeAddonECDb peer object
    static bool HasInstance(Napi::Value val) {
        if (!val.IsObject())
            return false;
        Napi::Object obj = val.As<Napi::Object>();
        return obj.InstanceOf(s_constructor.Value());
        }

    Napi::Value CreateDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname);
        RETURN_IF_HAD_EXCEPTION_SYNC
        if (IsOpenInternal())
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        DbResult status = AddonUtils::CreateECDb(*m_ecdb, BeFileName(dbname.c_str(), true));
        if (status != BE_SQLITE_OK)
            m_ecdb = nullptr;
            
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value OpenDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        RETURN_IF_HAD_EXCEPTION_SYNC
        if (IsOpenInternal())
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        DbResult status = AddonUtils::OpenECDb(*m_ecdb, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        if (status != BE_SQLITE_OK)
            m_ecdb = nullptr;
            
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value CloseDb(const Napi::CallbackInfo& info)
        {
        if (!IsOpenInternal())
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        m_ecdb->CloseDb();
        return Napi::Number::New(Env(), (int) BE_SQLITE_OK);
        }

    Napi::Value SaveChanges(const Napi::CallbackInfo& info)
        {            
        OPTIONAL_ARGUMENT_STRING(0, changeSetName);        
        RETURN_IF_HAD_EXCEPTION_SYNC
        if (!IsOpenInternal())
            return Napi::Number::New(Env(), (int)BE_SQLITE_ERROR);

        DbResult status = m_ecdb->SaveChanges(changeSetName.empty() ? nullptr : changeSetName.c_str());        
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value AbandonChanges(const Napi::CallbackInfo& info)
        {            
        if (!IsOpenInternal())
            return Napi::Number::New(Env(), (int)BE_SQLITE_ERROR);

        DbResult status = m_ecdb->AbandonChanges();        
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ImportSchema(const Napi::CallbackInfo& info)
        {  
        REQUIRE_ARGUMENT_STRING(0, schemaPathName);          
        RETURN_IF_HAD_EXCEPTION_SYNC
        if (!m_ecdb.IsValid() || !m_ecdb->IsDbOpen())
            return Napi::Number::New(Env(), (int)BE_SQLITE_ERROR);

        DbResult status = AddonUtils::ImportSchema(*m_ecdb, BeFileName(schemaPathName.c_str(), true));      
         return Napi::Number::New(Env(), (int)status);  
        }

    Napi::Value IsOpen(const Napi::CallbackInfo& info) { return Napi::Boolean::New(Env(), IsOpenInternal()); }

    bool IsOpenInternal() const { return m_ecdb.IsValid() && m_ecdb->IsDbOpen(); }

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
            InstanceMethod("createDb", &NodeAddonECDb::CreateDb),
            InstanceMethod("openDb", &NodeAddonECDb::OpenDb),
            InstanceMethod("closeDb", &NodeAddonECDb::CloseDb),
            InstanceMethod("saveChanges", &NodeAddonECDb::SaveChanges),
            InstanceMethod("abandonChanges", &NodeAddonECDb::AbandonChanges),
            InstanceMethod("importSchema", &NodeAddonECDb::ImportSchema),
            InstanceMethod("isOpen", &NodeAddonECDb::IsOpen)
        });

        exports.Set("NodeAddonECDb", t);

        s_constructor = Napi::Persistent(t);
        s_constructor.SuppressDestruct();             // ??? what is this?
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
struct NodeAddonBriefcaseManagerResourcesRequest : Napi::ObjectWrap<NodeAddonBriefcaseManagerResourcesRequest>
{
    IBriefcaseManager::Request m_req;
    static Napi::FunctionReference s_constructor;

    NodeAddonBriefcaseManagerResourcesRequest(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeAddonBriefcaseManagerResourcesRequest>(info)
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
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NodeAddonBriefcaseManagerResourcesRequest", {
          InstanceMethod("reset", &NodeAddonBriefcaseManagerResourcesRequest::Reset),
          InstanceMethod("isEmpty", &NodeAddonBriefcaseManagerResourcesRequest::IsEmpty),
          InstanceMethod("toJSON", &NodeAddonBriefcaseManagerResourcesRequest::ToJSON),
        });

        exports.Set("NodeAddonBriefcaseManagerResourcesRequest", t);

        s_constructor = Napi::Persistent(t);
        s_constructor.SuppressDestruct();             // ??? what is this?
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
struct NodeAddonDgnDb : Napi::ObjectWrap<NodeAddonDgnDb>
{
    static Napi::FunctionReference s_constructor;
    
    Dgn::DgnDbPtr m_dgndb;
    ConnectionManager m_connections;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;

    NodeAddonDgnDb(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeAddonDgnDb>(info)
        {
        }

    ~NodeAddonDgnDb() 
        {
        TearDownPresentationManager();
        }

    DgnDbR GetDgnDb() {return *m_dgndb;}

    //  Check if val is really a NodeAddonDgnDb peer object
    static bool HasInstance(Napi::Value val) {
        if (!val.IsObject())
            return false;
        Napi::Object obj = val.As<Napi::Object>();
        return obj.InstanceOf(s_constructor.Value());
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
    //  Get cached imodel briefcases
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
    //  Get the properties of the iModel
    //! @bsiclass
    //=======================================================================================
    Napi::Value GetIModelProps(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value props;
        AddonUtils::GetIModelProps(props, *m_dgndb);
        return Napi::String::New(Env(), props.ToString().c_str());
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
    // insert a new LinkTableRelationship -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value InsertLinkTableRelationshipSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value props = Json::Value::From(propsJsonStr);
        Json::Value idJsonObj;
        auto status = AddonUtils::InsertLinkTableRelationship(idJsonObj, GetDgnDb(), props);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), idJsonObj.asCString()));
        }

    //=======================================================================================
    // update an existing LinkTableRelationship -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value UpdateLinkTableRelationshipSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value props = Json::Value::From(propsJsonStr);
        auto status = AddonUtils::UpdateLinkTableRelationship(GetDgnDb(), props);
        return Napi::Number::New(Env(), (int)status);
        }

    //=======================================================================================
    // delete an existing LinkTableRelationship -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value DeleteLinkTableRelationshipSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value props = Json::Value::From(propsJsonStr);
        auto status = AddonUtils::DeleteLinkTableRelationship(GetDgnDb(), props);
        return Napi::Number::New(Env(), (int)status);
        }

    //=======================================================================================
    // insert a new element -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value InsertCodeSpecSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
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
        
        RETURN_IF_HAD_EXCEPTION_SYNC

        Utf8String idStr;
        DgnDbStatus status = AddonUtils::InsertCodeSpec(idStr, GetDgnDb(), name, (CodeScopeSpec::Type)specType, (CodeScopeSpec::ScopeRequirement)scopeReq);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), idStr.c_str()));
        }

    //=======================================================================================
    // insert a new Model -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value InsertModelSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        Json::Value elemIdJsonObj;
        auto status = AddonUtils::InsertModel(elemIdJsonObj, GetDgnDb(), elemProps);
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), elemIdJsonObj.ToString().c_str()));
        }

    //=======================================================================================
    // update an existing Model -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value UpdateModelSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        auto status = AddonUtils::UpdateModel(GetDgnDb(), elemProps);
        return Napi::Number::New(Env(), (int)status);
        }

    //=======================================================================================
    // delete an existing Model -- MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value DeleteModelSync(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, elemIdStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        auto status = AddonUtils::DeleteModel(GetDgnDb(), elemIdStr);
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
            ContentDescriptorCPtr descriptor = m_addondb->m_presentationManager->GetContentDescriptor(GetDgnDb(), ContentDisplayType::PropertyPane, selection, options.GetJson()).get();
            if (descriptor.IsNull())
                {
                m_status = DgnDbStatus::BadArg;
                return;
                }
            PageOptions pageOptions;
            pageOptions.SetPageStart(0);
            pageOptions.SetPageSize(0);
            ContentCPtr content = m_addondb->m_presentationManager->GetContent(GetDgnDb(), *descriptor, selection, pageOptions, options.GetJson()).get();
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
        OPTIONAL_ARGUMENT_STRING(0, description);
        REQUIRE_DB_TO_BE_OPEN_SYNC
        RETURN_IF_HAD_EXCEPTION_SYNC
        auto stat = GetDgnDb().SaveChanges(description);
        return Napi::Number::New(Env(), (int)stat);
        }

    Napi::Value ImportSchema(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, schemaPathnameStrObj);
        RETURN_IF_HAD_EXCEPTION_SYNC
        BeFileName schemaPathname(schemaPathnameStrObj.c_str(), true);
        auto stat = AddonUtils::ImportSchemaDgnDb(GetDgnDb(), schemaPathname);
        return Napi::Number::New(Env(), (int)stat);
        }

    void CloseDgnDb(const Napi::CallbackInfo& info)
        {
        TearDownPresentationManager();
        AddonUtils::CloseDgnDb(*m_dgndb);
        m_dgndb = nullptr;
        }

    //=======================================================================================
    // MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value CreateChangeCache(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changeCachePathStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        if (!changeCacheECDb->IsValid())
            return CreateBentleyReturnErrorObject(BE_SQLITE_ERROR);

        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().CreateChangeCache(changeCacheECDb->GetECDb(), changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    //=======================================================================================
    // MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD
    //! @bsimethod
    //=======================================================================================
    Napi::Value AttachChangeCache(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, changeCachePathStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        BeFileName changeCachePath(changeCachePathStr.c_str(), true);
        DbResult stat = GetDgnDb().AttachChangeCache(changeCachePath);
        return Napi::Number::New(Env(), (int) stat);
        }

    //=======================================================================================
    //! @bsimethod
    //=======================================================================================
    Napi::Value IsChangeCacheAttached(const Napi::CallbackInfo&)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        RETURN_IF_HAD_EXCEPTION_SYNC

        bool isAttached = GetDgnDb().IsChangeCacheAttached();
        return Napi::Boolean::New(Env(), isAttached);
        }

    //=======================================================================================
    // MUST ALWAYS BE SYNCHRONOUS - MUST ALWAYS BE RUN IN MAIN THREAD because it writes to disk
    //! @bsimethod
    //=======================================================================================
    Napi::Value ExtractChangeSummary(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonECDb, changeCacheECDb);
        REQUIRE_ARGUMENT_STRING(1, changesetFilePathStr);
        RETURN_IF_HAD_EXCEPTION_SYNC

        if (!changeCacheECDb->IsValid())
            return CreateBentleyReturnErrorObject(BE_SQLITE_ERROR);

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

    Napi::Value GetParentChangeSetId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        RETURN_IF_HAD_EXCEPTION_SYNC
        Utf8String parentRevId = m_dgndb->Revisions().GetParentRevisionId();
        return Napi::String::New(Env(), parentRevId.c_str());
        }

    Napi::Value GetDbGuid(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        RETURN_IF_HAD_EXCEPTION_SYNC
        BeGuid beGuid = m_dgndb->GetDbGuid();
        return Napi::String::New(Env(), beGuid.ToString().c_str());
        }

    Napi::Value SetDbGuid(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN_SYNC
        REQUIRE_ARGUMENT_STRING(0, guidStr);
        RETURN_IF_HAD_EXCEPTION_SYNC
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        m_dgndb->ChangeDbGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForElement(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_STRING(1, elemProps);
        REQUIRE_ARGUMENT_INTEGER(2, dbop);
        Json::Value elemPropsJson = Json::Value::From(elemProps);
        return Napi::Number::New(Env(), (int)AddonUtils::BuildBriefcaseManagerResourcesRequestForElement(req->m_req, GetDgnDb(), elemPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_STRING(1, props);
        REQUIRE_ARGUMENT_INTEGER(2, dbop);
        Json::Value propsJson = Json::Value::From(props);
        return Napi::Number::New(Env(), (int)AddonUtils::BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(req->m_req, GetDgnDb(), propsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForCodeSpec(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_STRING(1, props);
        REQUIRE_ARGUMENT_INTEGER(2, dbop);
        Json::Value propsJson = Json::Value::From(props);
        return Napi::Number::New(Env(), (int)AddonUtils::BuildBriefcaseManagerResourcesRequestForCodeSpec(req->m_req, GetDgnDb(), propsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForModel(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonBriefcaseManagerResourcesRequest, req);
        REQUIRE_ARGUMENT_STRING(1, modelProps);
        REQUIRE_ARGUMENT_INTEGER(2, dbop);
        Json::Value modelPropsJson = Json::Value::From(modelProps);
        return Napi::Number::New(Env(), (int)AddonUtils::BuildBriefcaseManagerResourcesRequestForModel(req->m_req, GetDgnDb(), modelPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value SetBriefcaseManagerOptimisticConcurrencyControlPolicy(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_ANY_OBJ(0, conflictRes);
        AddonUtils::BriefcaseManagerConflictResolution uu = (AddonUtils::BriefcaseManagerConflictResolution)conflictRes.Get("updateVsUpdate").ToNumber().Int32Value();
        AddonUtils::BriefcaseManagerConflictResolution ud = (AddonUtils::BriefcaseManagerConflictResolution)conflictRes.Get("updateVsDelete").ToNumber().Int32Value();
        AddonUtils::BriefcaseManagerConflictResolution du = (AddonUtils::BriefcaseManagerConflictResolution)conflictRes.Get("deleteVsUpdate").ToNumber().Int32Value();
        if (AddonUtils::BriefcaseManagerConflictResolution::Take != uu && AddonUtils::BriefcaseManagerConflictResolution::Reject != uu
          ||AddonUtils::BriefcaseManagerConflictResolution::Take != ud && AddonUtils::BriefcaseManagerConflictResolution::Reject != ud
          ||AddonUtils::BriefcaseManagerConflictResolution::Take != du && AddonUtils::BriefcaseManagerConflictResolution::Reject != du)
            {
            Napi::TypeError::New(Env(), "Invalid conflict resolution value").ThrowAsJavaScriptException();\
            return Napi::Number::New(Env(), 1);
            }

        return Napi::Number::New(Env(), (int)AddonUtils::SetBriefcaseManagerOptimisticConcurrencyControlPolicy(GetDgnDb(), uu, ud, du));
        }

    Napi::Value SetBriefcaseManagerPessimisticConcurrencyControlPolicy(const Napi::CallbackInfo& info)
        {
        return Napi::Number::New(Env(), (int)AddonUtils::SetBriefcaseManagerPessimisticConcurrencyControlPolicy(GetDgnDb()));
        }

    Napi::Value BriefcaseManagerStartBulkOperation(const Napi::CallbackInfo& info)
        {
        return Napi::Number::New(Env(), (int)AddonUtils::BriefcaseManagerStartBulkOperation(GetDgnDb()));
        }

    Napi::Value BriefcaseManagerEndBulkOperation(const Napi::CallbackInfo& info)
        {
        return Napi::Number::New(Env(), (int)AddonUtils::BriefcaseManagerEndBulkOperation(GetDgnDb()));
        }

	// ========================================================================================
	// Test method handler
	// Note: This is where the developer may specify, given an ID from JS, what function should
	//		 executed and returned.
	// ========================================================================================
	Napi::Value ExecuteTestById(const Napi::CallbackInfo& info)
		{
		REQUIRE_DB_TO_BE_OPEN_SYNC
		REQUIRE_ARGUMENT_INTEGER(0, testId);
		REQUIRE_ARGUMENT_STRING(1, params);
		RETURN_IF_HAD_EXCEPTION_SYNC

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
            InstanceMethod("openDgnDb", &NodeAddonDgnDb::StartOpenDgnDb),
            InstanceMethod("openDgnDbSync", &NodeAddonDgnDb::OpenDgnDbSync),
            InstanceMethod("closeDgnDb", &NodeAddonDgnDb::CloseDgnDb),
            InstanceMethod("createChangeCache", &NodeAddonDgnDb::CreateChangeCache),
            InstanceMethod("attachChangeCache", &NodeAddonDgnDb::AttachChangeCache),
            InstanceMethod("isChangeCacheAttached", &NodeAddonDgnDb::IsChangeCacheAttached),
            InstanceMethod("extractChangeSummary", &NodeAddonDgnDb::ExtractChangeSummary),
            InstanceMethod("setBriefcaseId", &NodeAddonDgnDb::SetBriefcaseId),
            InstanceMethod("getBriefcaseId", &NodeAddonDgnDb::GetBriefcaseId),
            InstanceMethod("getParentChangeSetId", &NodeAddonDgnDb::GetParentChangeSetId),
            InstanceMethod("getDbGuid", &NodeAddonDgnDb::GetDbGuid),
            InstanceMethod("setDbGuid", &NodeAddonDgnDb::SetDbGuid),
            InstanceMethod("openBriefcaseSync", &NodeAddonDgnDb::OpenBriefcaseSync),
            InstanceMethod("saveChanges", &NodeAddonDgnDb::SaveChanges),
            InstanceMethod("importSchema", &NodeAddonDgnDb::ImportSchema),
            InstanceMethod("getElement", &NodeAddonDgnDb::StartGetElementWorker),
            InstanceMethod("getModel", &NodeAddonDgnDb::StartGetModelWorker),
            InstanceMethod("insertElementSync", &NodeAddonDgnDb::InsertElementSync),
            InstanceMethod("updateElementSync", &NodeAddonDgnDb::UpdateElementSync),
            InstanceMethod("deleteElementSync", &NodeAddonDgnDb::DeleteElementSync),
            InstanceMethod("insertModelSync", &NodeAddonDgnDb::InsertModelSync),
            InstanceMethod("updateModelSync", &NodeAddonDgnDb::UpdateModelSync),
            InstanceMethod("deleteModelSync", &NodeAddonDgnDb::DeleteModelSync),
            InstanceMethod("insertCodeSpecSync", &NodeAddonDgnDb::InsertCodeSpecSync),
            InstanceMethod("getElementPropertiesForDisplay", &NodeAddonDgnDb::StartGetElementPropertiesForDisplayWorker),
            InstanceMethod("insertLinkTableRelationshipSync", &NodeAddonDgnDb::InsertLinkTableRelationshipSync),
            InstanceMethod("updateLinkTableRelationshipSync", &NodeAddonDgnDb::UpdateLinkTableRelationshipSync),
            InstanceMethod("deleteLinkTableRelationshipSync", &NodeAddonDgnDb::DeleteLinkTableRelationshipSync),
            InstanceMethod("getECClassMetaData", &NodeAddonDgnDb::StartGetECClassMetaData),
            InstanceMethod("getECClassMetaDataSync", &NodeAddonDgnDb::GetECClassMetaDataSync),
            InstanceMethod("executeQuery", &NodeAddonDgnDb::StartExecuteQueryWorker),
            InstanceMethod("getCachedBriefcaseInfosSync", &NodeAddonDgnDb::GetCachedBriefcaseInfosSync),
            InstanceMethod("getIModelProps", &NodeAddonDgnDb::GetIModelProps),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &NodeAddonDgnDb::BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForCodeSpec", &NodeAddonDgnDb::BuildBriefcaseManagerResourcesRequestForCodeSpec),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &NodeAddonDgnDb::BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForModel", &NodeAddonDgnDb::BuildBriefcaseManagerResourcesRequestForModel),
            InstanceMethod("setBriefcaseManagerOptimisticConcurrencyControlPolicy", &NodeAddonDgnDb::SetBriefcaseManagerOptimisticConcurrencyControlPolicy),
            InstanceMethod("setBriefcaseManagerPessimisticConcurrencyControlPolicy", &NodeAddonDgnDb::SetBriefcaseManagerPessimisticConcurrencyControlPolicy),
            InstanceMethod("briefcaseManagerStartBulkOperation", &NodeAddonDgnDb::BriefcaseManagerStartBulkOperation),
            InstanceMethod("briefcaseManagerEndBulkOperation", &NodeAddonDgnDb::BriefcaseManagerEndBulkOperation),
			
			// DEVELOPMENT-ONLY METHODS:
			InstanceMethod("executeTestById", &NodeAddonDgnDb::ExecuteTestById),

        });

        exports.Set("NodeAddonDgnDb", t);

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
    static void Init(Napi::Env& env, Napi::Object exports)
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

        exports.Set("NodeAddonECSqlStatement", t);

        s_constructor = Napi::Persistent(t);
        s_constructor.SuppressDestruct();             // ??? what is this?
        }

    Napi::Value Prepare(const Napi::CallbackInfo& info)
        {
        const int paramIdx =0;
        if (info.Length() <= (paramIdx) ) 
            {
            Napi::TypeError::New(Env(), "Argument 0 must be an object of type NodeAddonDgnDb or NodeAddonECDb").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        ECDb* ecdb;
        if (NodeAddonDgnDb::HasInstance(info[paramIdx])) {   
            NodeAddonDgnDb* var = NodeAddonDgnDb::Unwrap(info[paramIdx].As<Napi::Object>());
            if (!var->IsOpen())
                return NodeUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());            
            ecdb = &var->GetDgnDb();
        }
        else if (NodeAddonECDb::HasInstance(info[paramIdx])) {
            NodeAddonECDb* var = NodeAddonECDb::Unwrap(info[paramIdx].As<Napi::Object>());
            if (!var->IsOpenInternal())
                return NodeUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());            
            ecdb = &var->GetECDb();        
        }
        else {
            Napi::TypeError::New(Env(), "Argument 0 must be an object of type NodeAddonDgnDb or NodeAddonECDb").ThrowAsJavaScriptException();
            return Env().Undefined();
        }

        REQUIRE_ARGUMENT_STRING(1, ecsqlStr);

        BeSqliteDbMutexHolder serializeAccess(*ecdb); // hold mutex, so that we have a chance to get last ECDb error message

        auto status = m_stmt->Prepare(*ecdb, ecsqlStr.c_str());
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

//=======================================================================================
// Projects the NodeAddonECPresentationManager class into JS.
//! @bsiclass
//=======================================================================================
struct NodeAddonECPresentationManager : Napi::ObjectWrap<NodeAddonECPresentationManager>
    {
    static Napi::FunctionReference s_constructor;

    ConnectionManager m_connections;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;
    
    NodeAddonECPresentationManager(const Napi::CallbackInfo& info) 
        : Napi::ObjectWrap<NodeAddonECPresentationManager>(info)
        {
        m_presentationManager = std::unique_ptr<RulesDrivenECPresentationManager>(ECPresentationUtils::CreatePresentationManager(m_connections, T_HOST.GetIKnownLocationsAdmin()));
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
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NodeAddonECPresentationManager", {
          InstanceMethod("handleRequest", &NodeAddonECPresentationManager::HandleRequest)
        });

        exports.Set("NodeAddonECPresentationManager", t);

        s_constructor = Napi::Persistent(t);
        s_constructor.SuppressDestruct();             // ??? what is this?
        }

    Napi::Value HandleRequest(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonDgnDb, db);    // contract pre-conditions
        REQUIRE_ARGUMENT_STRING(1, serializedRequest);

        if (!db->IsOpen())
            return NodeUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());

        m_connections.NotifyConnectionOpened(db->GetDgnDb());

        Json::Value request;
        Json::Reader().parse(serializedRequest, request);
        if (request.isNull())
            return NodeUtils::CreateErrorObject0(ERROR, nullptr, Env());

        Utf8CP requestId = request["requestId"].asCString();
        if (Utf8String::IsNullOrEmpty(requestId))
            return NodeUtils::CreateErrorObject0(ERROR, nullptr, Env());

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
            return NodeUtils::CreateErrorObject0(ERROR, nullptr, Env());

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        response.Accept(writer);

        return Napi::String::New(Env(), buffer.GetString());
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

#if defined (BENTLEYCONFIG_OS_WINDOWS)

static BeFileName getLibraryDir()
    {
    void* addr = (void*)getLibraryDir;

    MEMORY_BASIC_INFORMATION mbi;
    HINSTANCE h = VirtualQuery (addr, &mbi, sizeof mbi)? (HINSTANCE)mbi.AllocationBase: (HINSTANCE)addr;

    WChar tModuleName[MAX_PATH];
    if (0 == ::GetModuleFileNameW (h, tModuleName, MAX_PATH)) // (yes, 0 means failure)
        return BeFileName();

    return BeFileName(tModuleName).GetDirectoryName();
    }

#elif defined (BENTLEYCONFIG_OS_LINUX)

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

static BeFileName getLibraryDir()
    {
    Dl_info  dlInfo;
    if (0 == dladdr((void*)getLibraryDir, &dlInfo)) // (yes, 0 means failure)
        return BeFileName();

    return BeFileName(dlInfo.dli_sname, true).GetDirectoryName();
    }

#elif defined (BENTLEYCONFIG_OS_APPLE_MACOS)
#include <Bentley/Desktop/FileSystem.h>

// *** TODO: This is probably not going to work. We need the MacOS equivalent of dladdr

static BeFileName getLibraryDir()
    {
    WString curdir;
    Desktop::FileSystem::GetCwd(curdir);
    return BeFileName(curdir.c_str());
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object registerModule(Napi::Env env, Napi::Object exports)
    {
    Napi::HandleScope scope(env);

    BeFileName addondir = getLibraryDir();

    AddonUtils::Initialize(addondir, throwJsExceptionOnAssert);
    NodeAddonDgnDb::Init(env, exports);
    NodeAddonECDb::Init(env, exports);
    NodeAddonECSqlStatement::Init(env, exports);
    NodeAddonBriefcaseManagerResourcesRequest::Init(env, exports);
    NodeAddonECPresentationManager::Init(env, exports);

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
        });

    return exports;
    }

Napi::FunctionReference NodeAddonBriefcaseManagerResourcesRequest::s_constructor;
Napi::FunctionReference NodeAddonECSqlStatement::s_constructor;
Napi::FunctionReference NodeAddonDgnDb::s_constructor;
Napi::FunctionReference NodeAddonECPresentationManager::s_constructor;
Napi::FunctionReference NodeAddonECDb::s_constructor;

NODE_API_MODULE(AddonUtils, registerModule)
