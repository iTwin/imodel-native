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

#include <json/value.h>
#include "AddonUtils.h"
#include "TestUtils.h"
#include <ECDb/ECDb.h>
#include <ECObjects/ECSchema.h>
#include <rapidjson/rapidjson.h>
#include "ECPresentationUtils.h"

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
private:
    static Napi::FunctionReference s_constructor;
    std::unique_ptr<ECDb> m_ecdb;

public:
    NodeAddonECDb(const Napi::CallbackInfo& info) : Napi::ObjectWrap<NodeAddonECDb>(info) {}
    ~NodeAddonECDb() {}
 
    ECDbR GetECDb() 
        {
        if (m_ecdb == nullptr)
            m_ecdb = std::make_unique<ECDb>();

        return *m_ecdb;
        }

    // Check if val is really a NodeAddonECDb peer object
    static bool HasInstance(Napi::Value val) {
        if (!val.IsObject())
            return false;
        Napi::Object obj = val.As<Napi::Object>();
        return obj.InstanceOf(s_constructor.Value());
        }

    Napi::Value CreateDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        RETURN_IF_HAD_EXCEPTION
        DbResult status = AddonUtils::CreateECDb(GetECDb(), BeFileName(dbName.c_str(), true));
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value OpenDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        RETURN_IF_HAD_EXCEPTION

        DbResult status = AddonUtils::OpenECDb(GetECDb(), BeFileName(dbName.c_str(), true), (Db::OpenMode)mode);
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
        DbResult status = GetECDb().SaveChanges(changeSetName.empty() ? nullptr : changeSetName.c_str());
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
        DbResult status = AddonUtils::ImportSchema(GetECDb(), BeFileName(schemaPathName.c_str(), true));
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
            InstanceMethod("createDb", &NodeAddonECDb::CreateDb),
            InstanceMethod("openDb", &NodeAddonECDb::OpenDb),
            InstanceMethod("closeDb", &NodeAddonECDb::CloseDb),
            InstanceMethod("dispose", &NodeAddonECDb::Dispose),
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

    Napi::Value OpenDgnDb(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname);
        REQUIRE_ARGUMENT_INTEGER(1, mode);
        RETURN_IF_HAD_EXCEPTION
        auto status = AddonUtils::OpenDgnDb(m_dgndb, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        if (BE_SQLITE_OK == status)
            SetupPresentationManager();

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

    Napi::Value OpenBriefcase(const Napi::CallbackInfo& info)
        {
        REQUIRE_ARGUMENT_STRING(0, briefcaseToken);
        REQUIRE_ARGUMENT_STRING(1, changeSetTokens);
        RETURN_IF_HAD_EXCEPTION

        Json::Value jsonBriefcaseToken = Json::Value::From(briefcaseToken);
        Json::Value jsonChangeSetTokens = Json::Value::From(changeSetTokens);

        DbResult result = AddonUtils::OpenBriefcase(m_dgndb, jsonBriefcaseToken, jsonChangeSetTokens);
        if (BE_SQLITE_OK == result)
            SetupPresentationManager();

        return Napi::Number::New(Env(), (int)result);
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
        TearDownPresentationManager();
        AddonUtils::CloseDgnDb(*m_dgndb);
        m_dgndb = nullptr;
        }

    Napi::Value CreateChangeCache(const Napi::CallbackInfo& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonECDb, changeCacheECDb);
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
        REQUIRE_ARGUMENT_OBJ(0, NodeAddonECDb, changeCacheECDb);
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
            InstanceMethod("openDgnDb", &NodeAddonDgnDb::OpenDgnDb),
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
            InstanceMethod("openBriefcase", &NodeAddonDgnDb::OpenBriefcase),
            InstanceMethod("saveChanges", &NodeAddonDgnDb::SaveChanges),
            InstanceMethod("importSchema", &NodeAddonDgnDb::ImportSchema),
            InstanceMethod("getElement", &NodeAddonDgnDb::GetElement),
            InstanceMethod("getModel", &NodeAddonDgnDb::GetModel),
            InstanceMethod("insertElement", &NodeAddonDgnDb::InsertElement),
            InstanceMethod("updateElement", &NodeAddonDgnDb::UpdateElement),
            InstanceMethod("deleteElement", &NodeAddonDgnDb::DeleteElement),
            InstanceMethod("insertModel", &NodeAddonDgnDb::InsertModel),
            InstanceMethod("updateModel", &NodeAddonDgnDb::UpdateModel),
            InstanceMethod("deleteModel", &NodeAddonDgnDb::DeleteModel),
            InstanceMethod("insertCodeSpec", &NodeAddonDgnDb::InsertCodeSpec),
            InstanceMethod("getElementPropertiesForDisplay", &NodeAddonDgnDb::GetElementPropertiesForDisplay),
            InstanceMethod("insertLinkTableRelationship", &NodeAddonDgnDb::InsertLinkTableRelationship),
            InstanceMethod("updateLinkTableRelationship", &NodeAddonDgnDb::UpdateLinkTableRelationship),
            InstanceMethod("deleteLinkTableRelationship", &NodeAddonDgnDb::DeleteLinkTableRelationship),
            InstanceMethod("getECClassMetaData", &NodeAddonDgnDb::GetECClassMetaData),
            InstanceMethod("getECClassMetaData", &NodeAddonDgnDb::GetECClassMetaData),
            InstanceMethod("getCachedBriefcaseInfos", &NodeAddonDgnDb::GetCachedBriefcaseInfos),
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
            if (!var->GetECDb().IsDbOpen())
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
            return NodeUtils::CreateErrorObject0(BE_SQLITE_ERROR, AddonUtils::GetLastECDbIssue().c_str(), Env());

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
           return NodeUtils::CreateErrorObject0(BE_SQLITE_ERROR, AddonUtils::GetLastECDbIssue().c_str(), Env());

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

// TODO: This is implemented by DeskTop::FileSystem. Delete this and use that as soon as Bentley has been merged.

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

#elif defined (BENTLEYCONFIG_OS_LINUX) || defined (BENTLEYCONFIG_OS_APPLE_MACOS)

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

static BeFileName getLibraryDir()
    {
    Dl_info  dlInfo;
    if (0 == dladdr((void*)getLibraryDir, &dlInfo)) // (yes, 0 means failure)
        return BeFileName();

    return BeFileName(dlInfo.dli_fname, true).GetDirectoryName();
    }

#else

#error unsupported platform

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
