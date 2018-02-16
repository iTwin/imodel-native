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
            if (BE_SQLITE_OK == status)
                {
                GetECDb().AddFunction(HexStrSqlFunction::GetSingleton());
                GetECDb().AddFunction(StrSqlFunction::GetSingleton());
                }

            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value OpenDb(const Napi::CallbackInfo& info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName);
            REQUIRE_ARGUMENT_INTEGER(1, mode);
            RETURN_IF_HAD_EXCEPTION

            const DbResult status = AddonUtils::OpenECDb(GetECDb(), BeFileName(dbName.c_str(), true), (Db::OpenMode) mode);
            if (BE_SQLITE_OK == status)
                {
                GetECDb().AddFunction(HexStrSqlFunction::GetSingleton());
                GetECDb().AddFunction(StrSqlFunction::GetSingleton());
                }

            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value CloseDb(const Napi::CallbackInfo& info)
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
            Napi::Function t = DefineClass(env, "AddonECDb", {
            InstanceMethod("createDb", &AddonECDb::CreateDb),
            InstanceMethod("openDb", &AddonECDb::OpenDb),
            InstanceMethod("closeDb", &AddonECDb::CloseDb),
            InstanceMethod("dispose", &AddonECDb::Dispose),
            InstanceMethod("saveChanges", &AddonECDb::SaveChanges),
            InstanceMethod("abandonChanges", &AddonECDb::AbandonChanges),
            InstanceMethod("importSchema", &AddonECDb::ImportSchema),
            InstanceMethod("isOpen", &AddonECDb::IsOpen)
            });

            exports.Set("AddonECDb", t);

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
        Napi::Function t = DefineClass(env, "AddonBriefcaseManagerResourcesRequest", {
          InstanceMethod("reset", &AddonBriefcaseManagerResourcesRequest::Reset),
          InstanceMethod("isEmpty", &AddonBriefcaseManagerResourcesRequest::IsEmpty),
          InstanceMethod("toJSON", &AddonBriefcaseManagerResourcesRequest::ToJSON),
        });

        exports.Set("AddonBriefcaseManagerResourcesRequest", t);

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
        m_dgndb->RemoveFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->RemoveFunction(StrSqlFunction::GetSingleton());
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
        m_dgndb->AddFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->AddFunction(StrSqlFunction::GetSingleton());
        }

    static AddonDgnDb* From(DgnDbR db)
        {
        auto appdata = AddonAppData::Find(db);
        return appdata? &appdata->m_addonDb: nullptr;
        }


    //  Check if val is really a AddonDgnDb peer object
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

    Napi::Value OpenDgnDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        RETURN_IF_HAD_EXCEPTION
        DgnDbPtr db;
        DbResult status = AddonUtils::OpenDgnDb(db, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        if (BE_SQLITE_OK == status)
            OnDgnDbOpened(db.get());

        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value CreateDgnDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_STRING(1, rootSubjectName);
        OPTIONAL_ARGUMENT_STRING(2, rootSubjectDescription);
        RETURN_IF_HAD_EXCEPTION

        DgnDbPtr db;
        DbResult status = AddonUtils::CreateDgnDb(db, BeFileName(dbName.c_str(), true), rootSubjectName, rootSubjectDescription);
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

    Napi::Value TxnManagerGetCurrentTxnId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN

        return Napi::String::New(Env(), TxnIdToString(m_dgndb->Txns().GetCurrentTxnId()).c_str());
        }

    Napi::Value TxnManagerQueryFirstTxnId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN

        TxnManager::TxnId startTxnId = m_dgndb->Txns().QueryNextTxnId(TxnManager::TxnId(0));

        return Napi::String::New(Env(), TxnIdToString(startTxnId).c_str());
        }

    Napi::Value TxnManagerQueryNextTxnId(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION

        auto next = m_dgndb->Txns().QueryNextTxnId(TxnIdFromString(txnIdHexStr));

        return Napi::String::New(Env(), TxnIdToString(next).c_str());
        }

    Napi::Value TxnManagerQueryPreviousTxnId(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION

        auto next = m_dgndb->Txns().QueryPreviousTxnId(TxnIdFromString(txnIdHexStr));

        return Napi::String::New(Env(), TxnIdToString(next).c_str());
        }

    Napi::Value TxnManagerGetTxnDescription(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION

        return Napi::String::New(Env(), m_dgndb->Txns().GetTxnDescription(TxnIdFromString(txnIdHexStr)).c_str());
        }

    Napi::Value TxnManagerIsTxnIdValid(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr);
        return Napi::Boolean::New(Env(), TxnIdFromString(txnIdHexStr).IsValid());
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

    Napi::Value GetReversedChangeSetId(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        RETURN_IF_HAD_EXCEPTION
        if (!m_dgndb->Revisions().HasReversedRevisions())
            return Env().Undefined();
        Utf8String reversedRevId = m_dgndb->Revisions().GetReversedRevisionId();
        return Napi::String::New(Env(), reversedRevId.c_str());
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
	// ========================================================================================
	Napi::Value ExecuteTest(const Napi::CallbackInfo& info)
		{
		REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, testName);
        REQUIRE_ARGUMENT_STRING(1, params);
		RETURN_IF_HAD_EXCEPTION

        return Napi::String::New(Env(), AddonUtils::ExecuteTest(GetDgnDb(), testName, params).ToString().c_str());
		}


    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        // ***
        // *** WARNING: If you modify this API or fix a bug, increment the appropriate digit in package_version.txt
        // ***
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "AddonDgnDb", {
            InstanceMethod("createDgnDb", &AddonDgnDb::CreateDgnDb),
            InstanceMethod("openDgnDb", &AddonDgnDb::OpenDgnDb),
            InstanceMethod("closeDgnDb", &AddonDgnDb::CloseDgnDb),
            InstanceMethod("processChangeSets", &AddonDgnDb::ProcessChangeSets),
            InstanceMethod("startCreateChangeSet", &AddonDgnDb::StartCreateChangeSet),
            InstanceMethod("finishCreateChangeSet", &AddonDgnDb::FinishCreateChangeSet),
            InstanceMethod("createChangeCache", &AddonDgnDb::CreateChangeCache),
            InstanceMethod("attachChangeCache", &AddonDgnDb::AttachChangeCache),
            InstanceMethod("isChangeCacheAttached", &AddonDgnDb::IsChangeCacheAttached),
            InstanceMethod("extractChangeSummary", &AddonDgnDb::ExtractChangeSummary),
            InstanceMethod("setBriefcaseId", &AddonDgnDb::SetBriefcaseId),
            InstanceMethod("getBriefcaseId", &AddonDgnDb::GetBriefcaseId),
            InstanceMethod("getReversedChangeSetId", &AddonDgnDb::GetReversedChangeSetId),
            InstanceMethod("getParentChangeSetId", &AddonDgnDb::GetParentChangeSetId),
            InstanceMethod("getDbGuid", &AddonDgnDb::GetDbGuid),
            InstanceMethod("setDbGuid", &AddonDgnDb::SetDbGuid),
            InstanceMethod("setupBriefcase", &AddonDgnDb::SetupBriefcase),
            InstanceMethod("saveChanges", &AddonDgnDb::SaveChanges),
            InstanceMethod("abandonChanges", &AddonDgnDb::AbandonChanges),
            InstanceMethod("importSchema", &AddonDgnDb::ImportSchema),
            InstanceMethod("getElement", &AddonDgnDb::GetElement),
            InstanceMethod("getModel", &AddonDgnDb::GetModel),
            InstanceMethod("insertElement", &AddonDgnDb::InsertElement),
            InstanceMethod("updateElement", &AddonDgnDb::UpdateElement),
            InstanceMethod("deleteElement", &AddonDgnDb::DeleteElement),
            InstanceMethod("insertModel", &AddonDgnDb::InsertModel),
            InstanceMethod("updateModel", &AddonDgnDb::UpdateModel),
            InstanceMethod("deleteModel", &AddonDgnDb::DeleteModel),
            InstanceMethod("insertCodeSpec", &AddonDgnDb::InsertCodeSpec),
            InstanceMethod("getElementPropertiesForDisplay", &AddonDgnDb::GetElementPropertiesForDisplay),
            InstanceMethod("insertLinkTableRelationship", &AddonDgnDb::InsertLinkTableRelationship),
            InstanceMethod("updateLinkTableRelationship", &AddonDgnDb::UpdateLinkTableRelationship),
            InstanceMethod("deleteLinkTableRelationship", &AddonDgnDb::DeleteLinkTableRelationship),
            InstanceMethod("getECClassMetaData", &AddonDgnDb::GetECClassMetaData),
            InstanceMethod("getECClassMetaData", &AddonDgnDb::GetECClassMetaData),
            InstanceMethod("getCachedBriefcaseInfos", &AddonDgnDb::GetCachedBriefcaseInfos),
            InstanceMethod("getIModelProps", &AddonDgnDb::GetIModelProps),
			InstanceMethod("updateProjectExtents", &AddonDgnDb::UpdateProjectExtents),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &AddonDgnDb::BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &AddonDgnDb::BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForModel", &AddonDgnDb::BuildBriefcaseManagerResourcesRequestForModel),
            InstanceMethod("setBriefcaseManagerOptimisticConcurrencyControlPolicy", &AddonDgnDb::SetBriefcaseManagerOptimisticConcurrencyControlPolicy),
            InstanceMethod("setBriefcaseManagerPessimisticConcurrencyControlPolicy", &AddonDgnDb::SetBriefcaseManagerPessimisticConcurrencyControlPolicy),
            InstanceMethod("briefcaseManagerStartBulkOperation", &AddonDgnDb::BriefcaseManagerStartBulkOperation),
            InstanceMethod("briefcaseManagerEndBulkOperation", &AddonDgnDb::BriefcaseManagerEndBulkOperation),
            InstanceMethod("extractBulkResourcesRequest", &AddonDgnDb::ExtractBulkResourcesRequest),
            InstanceMethod("appendBriefcaseManagerResourcesRequest", &AddonDgnDb::AppendBriefcaseManagerResourcesRequest),
            InstanceMethod("extractBriefcaseManagerResourcesRequest", &AddonDgnDb::ExtractBriefcaseManagerResourcesRequest),
            InstanceMethod("inBulkOperation", &AddonDgnDb::InBulkOperation),
            InstanceMethod("txnManagerQueryFirstTxnId", &AddonDgnDb::TxnManagerQueryFirstTxnId),
            InstanceMethod("txnManagerQueryNextTxnId", &AddonDgnDb::TxnManagerQueryNextTxnId),
            InstanceMethod("txnManagerQueryPreviousTxnId", &AddonDgnDb::TxnManagerQueryPreviousTxnId),
            InstanceMethod("txnManagerGetCurrentTxnId", &AddonDgnDb::TxnManagerGetCurrentTxnId),
            InstanceMethod("txnManagerGetTxnDescription", &AddonDgnDb::TxnManagerGetTxnDescription),
            InstanceMethod("txnManagerIsTxnIdValid", &AddonDgnDb::TxnManagerIsTxnIdValid),
						
			// DEVELOPMENT-ONLY METHODS:
			InstanceMethod("executeTest", &AddonDgnDb::ExecuteTest),

        });

        exports.Set("AddonDgnDb", t);

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
        Napi::Function t = DefineClass(env, "AddonECSqlBinder", {
        InstanceMethod("dispose", &AddonECSqlBinder::Dispose),
        InstanceMethod("bindNull", &AddonECSqlBinder::BindNull),
        InstanceMethod("bindBlob", &AddonECSqlBinder::BindBlob),
        InstanceMethod("bindBoolean", &AddonECSqlBinder::BindBoolean),
        InstanceMethod("bindDateTime", &AddonECSqlBinder::BindDateTime),
        InstanceMethod("bindDouble", &AddonECSqlBinder::BindDouble),
        InstanceMethod("bindId", &AddonECSqlBinder::BindId),
        InstanceMethod("bindInteger", &AddonECSqlBinder::BindInteger),
        InstanceMethod("bindPoint2d", &AddonECSqlBinder::BindPoint2d),
        InstanceMethod("bindPoint3d", &AddonECSqlBinder::BindPoint3d),
        InstanceMethod("bindString", &AddonECSqlBinder::BindString),
        InstanceMethod("bindNavigation", &AddonECSqlBinder::BindNavigation),
        InstanceMethod("bindMember", &AddonECSqlBinder::BindMember),
        InstanceMethod("addArrayElement", &AddonECSqlBinder::AddArrayElement)
        });

        exports.Set("AddonECSqlBinder", t);

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

    Napi::Value BindInteger(const Napi::CallbackInfo& info)
        {
        if (info.Length() == 0)
            Napi::TypeError::New(info.Env(), "BindInteger expects a string or number").ThrowAsJavaScriptException();
        
        Napi::Value val = info[0];
        if (!val.IsNumber() && !val.IsString())
            Napi::TypeError::New(info.Env(), "BindInteger expects a string or number").ThrowAsJavaScriptException();

        int64_t int64Val;
        if (val.IsNumber())
            int64Val = val.ToNumber().Int64Value();
        else
            {
            Utf8String strVal(val.ToString().Utf8Value().c_str());
            if (strVal.empty())
                Napi::TypeError::New(info.Env(), "Integral string passed to BindInteger must not be empty.").ThrowAsJavaScriptException();

            const bool isNegativeNumber = strVal[0] == '-';
            Utf8CP positiveNumberStr = isNegativeNumber ? strVal.c_str() + 1 : strVal.c_str();
            uint64_t uVal = 0;
            if (SUCCESS != BeStringUtilities::ParseUInt64(uVal, positiveNumberStr)) //also supports hex strings
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Could not parse string %s to a valid integer.", strVal.c_str());
                Napi::TypeError::New(info.Env(), error.c_str()).ThrowAsJavaScriptException();
                }

            if (isNegativeNumber && uVal > (uint64_t) std::numeric_limits<int64_t>::max())
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Number in string %s is too large to fit into a signed 64 bit integer value.", strVal.c_str());
                Napi::TypeError::New(info.Env(), error.c_str()).ThrowAsJavaScriptException();
                }

            int64Val = uVal;
            if (isNegativeNumber)
                int64Val *= -1;
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
// Projects the AddonECSqlColumnInfo interface into JS.
//! @bsiclass
//=======================================================================================
struct AddonECSqlColumnInfo : Napi::ObjectWrap<AddonECSqlColumnInfo>
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
            StructArray = 15
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
        AddonECSqlColumnInfo(Napi::CallbackInfo const& info) : Napi::ObjectWrap<AddonECSqlColumnInfo>(info)
            {
            if (info.Length() != 1)
                Napi::TypeError::New(Env(), "AddonECSqlColumnInfo constructor expects two argument.").ThrowAsJavaScriptException();

            m_colInfo = info[0].As<Napi::External<ECSqlColumnInfo>>().Data();
            if (m_colInfo == nullptr)
                Napi::TypeError::New(Env(), "Invalid first arg for AddonECSqlColumnInfo constructor. ECSqlColumnInfo must not be nullptr").ThrowAsJavaScriptException();
            }

        ~AddonECSqlColumnInfo() {}

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
            Napi::Function t = DefineClass(env, "AddonECSqlColumnInfo", {
            InstanceMethod("getType", &AddonECSqlColumnInfo::GetType),
            InstanceMethod("getPropertyName", &AddonECSqlColumnInfo::GetPropertyName),
            InstanceMethod("getAccessString", &AddonECSqlColumnInfo::GetAccessString),
            InstanceMethod("isSystemProperty", &AddonECSqlColumnInfo::IsSystemProperty),
            InstanceMethod("isGeneratedProperty", &AddonECSqlColumnInfo::IsGeneratedProperty),
            InstanceMethod("getRootClassTableSpace", &AddonECSqlColumnInfo::GetRootClassTableSpace),
            InstanceMethod("getRootClassName", &AddonECSqlColumnInfo::GetRootClassName),
            InstanceMethod("getRootClassAlias", &AddonECSqlColumnInfo::GetRootClassAlias)});

            exports.Set("AddonECSqlColumnInfo", t);

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

        Napi::Value GetType(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "GetType must not have arguments").ThrowAsJavaScriptException();

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
                            type = Type::Blob;
                            break;
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

        Napi::Value GetPropertyName(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "GetPropertyName must not have arguments").ThrowAsJavaScriptException();

            ECPropertyCP prop = GetColInfo().GetProperty();
            if (prop == nullptr)
                Napi::TypeError::New(info.Env(), "ECSqlColumnInfo does not represent a property.").ThrowAsJavaScriptException();

            return Napi::String::New(Env(), prop->GetName().c_str());
            }

        Napi::Value GetAccessString(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "GetAccessString must not have arguments").ThrowAsJavaScriptException();

            //if property is generated, the display label contains the select clause item as is.
            //The property name in contrast would have encoded special characters of the select clause item.
            //Ex: SELECT MyProp + 4 FROM Foo -> the name must be "MyProp + 4"
            if (GetColInfo().IsGeneratedProperty())
                {
                BeAssert(GetColInfo().GetPropertyPath().Size() == 1);
                ECPropertyCP prop = GetColInfo().GetProperty();
                if (prop == nullptr)
                    Napi::TypeError::New(info.Env(), "ECSqlColumnInfo's Property must not be null for a generated property.").ThrowAsJavaScriptException();

                return Napi::String::New(Env(), prop->GetDisplayLabel().c_str());
                }

            return Napi::String::New(Env(), GetColInfo().GetPropertyPath().ToString().c_str());
            }

        Napi::Value IsSystemProperty(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "IsSystemProperty must not have arguments").ThrowAsJavaScriptException();

            return Napi::Boolean::New(Env(), GetColInfo().IsSystemProperty());
            }

        Napi::Value IsGeneratedProperty(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "IsGeneratedProperty must not have arguments").ThrowAsJavaScriptException();

            return Napi::Boolean::New(Env(), GetColInfo().IsGeneratedProperty());
            }

        Napi::Value GetRootClassTableSpace(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "GetRootClassTableSpace must not have arguments").ThrowAsJavaScriptException();

            return Napi::String::New(Env(), GetColInfo().GetRootClass().GetTableSpace().c_str());
            }

        Napi::Value GetRootClassName(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "GetRootClassName must not have arguments").ThrowAsJavaScriptException();

            return Napi::String::New(Env(), ECJsonUtilities::FormatClassName(GetColInfo().GetRootClass().GetClass()).c_str());
            }

        Napi::Value GetRootClassAlias(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "GetRootClassAlias must not have arguments").ThrowAsJavaScriptException();

            return Napi::String::New(Env(), GetColInfo().GetRootClass().GetAlias().c_str());
            }
    };

//=======================================================================================
// Projects the IECSqlValue interface into JS.
//! @bsiclass
//=======================================================================================
struct AddonECSqlValue : Napi::ObjectWrap<AddonECSqlValue>
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
    AddonECSqlValue(Napi::CallbackInfo const& info) : Napi::ObjectWrap<AddonECSqlValue>(info)
        {
        if (info.Length() != 2)
            Napi::TypeError::New(Env(), "AddonECSqlValue constructor expects two arguments.").ThrowAsJavaScriptException();

        m_ecsqlValue = info[0].As<Napi::External<IECSqlValue>>().Data();
        if (m_ecsqlValue == nullptr)
            Napi::TypeError::New(Env(), "Invalid first arg for AddonECSqlValue constructor. IECSqlValue must not be nullptr").ThrowAsJavaScriptException();

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            Napi::TypeError::New(Env(), "Invalid second arg for AddonECSqlValue constructor. ECDb must not be nullptr").ThrowAsJavaScriptException();
        }

    ~AddonECSqlValue() {}

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
        Napi::Function t = DefineClass(env, "AddonECSqlValue", {
        InstanceMethod("dispose", &AddonECSqlValue::Dispose),
        InstanceMethod("isNull", &AddonECSqlValue::IsNull),
        InstanceMethod("getColumnInfo", &AddonECSqlValue::GetColumnInfo),
        InstanceMethod("getBlob", &AddonECSqlValue::GetBlob),
        InstanceMethod("getBoolean", &AddonECSqlValue::GetBoolean),
        InstanceMethod("getDateTime", &AddonECSqlValue::GetDateTime),
        InstanceMethod("getDouble", &AddonECSqlValue::GetDouble),
        InstanceMethod("getGeometry", &AddonECSqlValue::GetGeometry),
        InstanceMethod("getId", &AddonECSqlValue::GetId),
        InstanceMethod("getClassNameForClassId", &AddonECSqlValue::GetClassNameForClassId),
        InstanceMethod("getInt", &AddonECSqlValue::GetInt),
        InstanceMethod("getInt64", &AddonECSqlValue::GetInt64),
        InstanceMethod("getPoint2d", &AddonECSqlValue::GetPoint2d),
        InstanceMethod("getPoint3d", &AddonECSqlValue::GetPoint3d),
        InstanceMethod("getString", &AddonECSqlValue::GetString),
        InstanceMethod("getNavigation", &AddonECSqlValue::GetNavigation),
        InstanceMethod("getStructIterator", &AddonECSqlValue::GetStructIterator),
        InstanceMethod("getArrayIterator", &AddonECSqlValue::GetArrayIterator)
        });

        exports.Set("AddonECSqlValue", t);

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

    void Dispose(const Napi::CallbackInfo& info)
        {
        if (m_ecsqlValue != nullptr)
            m_ecsqlValue = nullptr;

        if (m_ecdb != nullptr)
            m_ecdb = nullptr;
        }

    Napi::Value GetColumnInfo(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetColumnInfo must not have arguments").ThrowAsJavaScriptException();

        return AddonECSqlColumnInfo::New(Env(), GetECSqlValue().GetColumnInfo());
        }

    Napi::Value IsNull(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "IsNull must not have arguments").ThrowAsJavaScriptException();
        
        return Napi::Boolean::New(Env(), GetECSqlValue().IsNull());
        }

    Napi::Value GetBlob(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetBlob must not have arguments").ThrowAsJavaScriptException();

        int blobSize = -1;
        void const* blob = GetECSqlValue().GetBlob(&blobSize);

        Utf8String base64Str;
        Base64Utilities::Encode(base64Str, (Byte const*) blob, (size_t) blobSize);

        return Napi::String::New(Env(), base64Str.c_str());
        }

    Napi::Value GetBoolean(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetBoolean must not have arguments").ThrowAsJavaScriptException();

        return Napi::Boolean::New(Env(), GetECSqlValue().GetBoolean());
        }

    Napi::Value GetDateTime(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetDateTime must not have arguments").ThrowAsJavaScriptException();

        DateTime dt = GetECSqlValue().GetDateTime();
        return Napi::String::New(Env(), dt.ToString().c_str());
        }

    Napi::Value GetDouble(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetBlob must not have arguments").ThrowAsJavaScriptException();

        return Napi::Number::New(Env(), GetECSqlValue().GetDouble());
        }

    Napi::Value GetGeometry(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetGeometry must not have arguments").ThrowAsJavaScriptException();

        IGeometryPtr geom = GetECSqlValue().GetGeometry();
        Json::Value json;
        if (SUCCESS != ECJsonUtilities::IGeometryToJson(json, *geom))
            Napi::TypeError::New(info.Env(), "Could not convert IGeometry to JSON.").ThrowAsJavaScriptException();

        return Napi::String::New(Env(), json.ToString().c_str());
        }

    Napi::Value GetId(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetId must not have arguments").ThrowAsJavaScriptException();

        const BeInt64Id id = GetECSqlValue().GetId<BeInt64Id>();
        return Napi::String::New(Env(), id.ToHexStr().c_str());
        }

    Napi::Value GetClassNameForClassId(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetClassNameForClassId must not have arguments").ThrowAsJavaScriptException();

        const ECClassId classId = GetECSqlValue().GetId<ECClassId>();
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

    Napi::Value GetInt(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetInt must not have arguments").ThrowAsJavaScriptException();

        return Napi::Number::New(Env(), GetECSqlValue().GetInt());
        }

    Napi::Value GetInt64(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetInt64 must not have arguments").ThrowAsJavaScriptException();

        return Napi::Number::New(Env(), GetECSqlValue().GetInt64());
        }

    Napi::Value GetPoint2d(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetPoint2d must not have arguments").ThrowAsJavaScriptException();

        DPoint2d pt = GetECSqlValue().GetPoint2d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        return jsPt;
        }

    Napi::Value GetPoint3d(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetPoint3d must not have arguments").ThrowAsJavaScriptException();

        DPoint3d pt = GetECSqlValue().GetPoint3d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Z(), Napi::Number::New(Env(), pt.z));
        return jsPt;
        }

    Napi::Value GetString(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetString must not have arguments").ThrowAsJavaScriptException();

        return Napi::String::New(Env(), GetECSqlValue().GetText());
        }

    Napi::Value GetNavigation(const Napi::CallbackInfo& info)
        {
        if (info.Length() != 0)
            Napi::TypeError::New(info.Env(), "GetNavigation must not have arguments").ThrowAsJavaScriptException();

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

    //implementations are after AddonECSqlValueIterable as it needs to call into that class
    Napi::Value GetStructIterator(const Napi::CallbackInfo&);
    Napi::Value GetArrayIterator(const Napi::CallbackInfo&);
    };

//=======================================================================================
// Projects the IECSqlValueIterable interface into JS.
//! @bsiclass
//=======================================================================================
struct AddonECSqlValueIterator : Napi::ObjectWrap<AddonECSqlValueIterator>
    {
    private:
        static Napi::FunctionReference s_constructor;
        ECDb const* m_ecdb = nullptr;
        IECSqlValueIterable const* m_iterable = nullptr;
        bool m_isBeforeFirstElement = true;
        IECSqlValueIterable::const_iterator m_it;
        IECSqlValueIterable::const_iterator m_endIt;

    public:
        AddonECSqlValueIterator(Napi::CallbackInfo const& info) : Napi::ObjectWrap<AddonECSqlValueIterator>(info)
            {
            if (info.Length() != 2)
                Napi::TypeError::New(Env(), "AddonECSqlValueIterator constructor expects two argument.").ThrowAsJavaScriptException();

            m_iterable = info[0].As<Napi::External<IECSqlValueIterable>>().Data();
            if (m_iterable == nullptr)
                Napi::TypeError::New(Env(), "Invalid first arg for AddonECSqlValueIterator constructor. IECSqlValueIterable must not be nullptr").ThrowAsJavaScriptException();

            m_endIt = m_iterable->end();

            m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
            if (m_ecdb == nullptr)
                Napi::TypeError::New(Env(), "Invalid second arg for AddonECSqlValueIterator constructor. ECDb must not be nullptr").ThrowAsJavaScriptException();
            }

        ~AddonECSqlValueIterator() {}

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
            Napi::Function t = DefineClass(env, "AddonECSqlValueIterator", {
            InstanceMethod("dispose", &AddonECSqlValueIterator::Dispose),
            InstanceMethod("moveNext", &AddonECSqlValueIterator::MoveNext),
            InstanceMethod("getCurrent", &AddonECSqlValueIterator::GetCurrent)});

            exports.Set("AddonECSqlValueIterator", t);

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

        void Dispose(const Napi::CallbackInfo& info) 
            {
            if (m_ecdb != nullptr)
                m_ecdb = nullptr;

            if (m_iterable != nullptr)
                m_iterable = nullptr;
            }

        // A JS iterator expects the initial state of an iterator to be before the first element.
        // So on the first call to MoveNext, the C++ iterator must be created, and not incremented.
        Napi::Value MoveNext(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "AddonECSqlValueIterator::MoveNext must not have arguments").ThrowAsJavaScriptException();

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

        Napi::Value GetCurrent(const Napi::CallbackInfo& info)
            {
            if (info.Length() != 0)
                Napi::TypeError::New(info.Env(), "AddonECSqlValueIterator::GetCurrent must not have arguments").ThrowAsJavaScriptException();

            return AddonECSqlValue::New(Env(), *m_it, *m_ecdb);
            }
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               01/2018
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value AddonECSqlValue::GetStructIterator(const Napi::CallbackInfo& info)
    {
    if (info.Length() != 0)
        Napi::TypeError::New(info.Env(), "GetStructIterator must not have arguments").ThrowAsJavaScriptException();

    return AddonECSqlValueIterator::New(info.Env(), GetECSqlValue().GetStructIterable(), *m_ecdb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               01/2018
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value AddonECSqlValue::GetArrayIterator(const Napi::CallbackInfo& info)
    {
    if (info.Length() != 0)
        Napi::TypeError::New(info.Env(), "GetArrayIterator must not have arguments").ThrowAsJavaScriptException();

    return AddonECSqlValueIterator::New(info.Env(), GetECSqlValue().GetArrayIterable(), *m_ecdb);
    }


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
        Napi::Function t = DefineClass(env, "AddonECSqlStatement", {
          InstanceMethod("prepare", &AddonECSqlStatement::Prepare),
          InstanceMethod("reset", &AddonECSqlStatement::Reset),
          InstanceMethod("dispose", &AddonECSqlStatement::Dispose),
          InstanceMethod("clearBindings", &AddonECSqlStatement::ClearBindings),
          InstanceMethod("getBinder", &AddonECSqlStatement::GetBinder),
          InstanceMethod("step", &AddonECSqlStatement::Step),
          InstanceMethod("stepForInsert", &AddonECSqlStatement::StepForInsert),
          InstanceMethod("getColumnCount", &AddonECSqlStatement::GetColumnCount),
          InstanceMethod("getValue", &AddonECSqlStatement::GetValue)
        });

        exports.Set("AddonECSqlStatement", t);

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
        return AddonECSqlBinder::New(info.Env(), binder, *m_stmt->GetECDb());
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

    Napi::Value GetColumnCount(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;
        if (info.Length() != 0)
            {
            Napi::TypeError::New(Env(), "GetColumnCount requires no arguments").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        int colCount = m_stmt->GetColumnCount();
        return Napi::Number::New(info.Env(), colCount);
        }

    Napi::Value GetValue(const Napi::CallbackInfo& info)
        {
        MUST_HAVE_M_STMT;

        if (info.Length() != 1)
            {
            Napi::TypeError::New(Env(), "GetValue requires column index").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        REQUIRE_ARGUMENT_INTEGER(0, colIndex);

        IECSqlValue const& val = m_stmt->GetValue(colIndex);
        return AddonECSqlValue::New(info.Env(), val, *m_stmt->GetECDb());
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
        Napi::Function t = DefineClass(env, "AddonECPresentationManager", {
          InstanceMethod("handleRequest", &AddonECPresentationManager::HandleRequest),
          InstanceMethod("setupRulesetDirectories", &AddonECPresentationManager::SetupRulesetDirectories)
        });

        exports.Set("AddonECPresentationManager", t);

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
    IModelJsAddon::AddonECSqlBinder::Init(env, exports);
    IModelJsAddon::AddonECSqlValue::Init(env, exports);
    IModelJsAddon::AddonECSqlColumnInfo::Init(env, exports);
    IModelJsAddon::AddonECSqlValueIterator::Init(env, exports);
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
Napi::FunctionReference IModelJsAddon::AddonECSqlBinder::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECSqlValue::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECSqlColumnInfo::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECSqlValueIterator::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonDgnDb::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECPresentationManager::s_constructor;
Napi::FunctionReference IModelJsAddon::AddonECDb::s_constructor;

NODE_API_MODULE(at_bentley_imodeljs_nodeaddon, iModelJsAddonRegisterModule)
