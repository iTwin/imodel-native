/*--------------------------------------------------------------------------------------+
|
|     $Source: IModelJsNative.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <functional>
#include <queue>
#include <sys/types.h>
#include <stdint.h>
#include <memory>
#include "suppress_warnings.h"
#include <imodeljs-nodeaddonapi.package.version.h>
#include <Napi/napi.h>
#include <json/value.h>
#include "IModelJsNative.h"
#include <ECDb/ECDb.h>
#include <ECObjects/ECSchema.h>
#include <rapidjson/rapidjson.h>
#include "ECPresentationUtils.h"
#include "ECSchemaXmlContextUtils.h"
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/BeThread.h>
#include <Bentley/CancellationToken.h>
#include <Bentley/BeAtomic.h>
#include <Bentley/PerformanceLogger.h>
#include <DgnPlatform/Tile.h>
#include <DgnPlatform/ChangedElementsManager.h>
#include "UlasClient.h"

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

#define REQUIRE_ARGUMENT_STRING_ID(i, strId, T, id, retval)\
    REQUIRE_ARGUMENT_STRING(i, strId, retval)\
    T id(BeInt64Id::FromString(strId.c_str()).GetValue());

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
    if (info.Length() <= (i) || (info[i].IsUndefined() || info[i].IsNull())) {\
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
    if (info.Length() <= (i) || (info[i].IsUndefined() || info[i].IsNull())) {\
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
    if (info.Length() <= (i) || (info[i].IsUndefined() || info[i].IsNull())) {\
        ;\
    }\
    else if (info[i].IsString()) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    }\
    else {\
        Utf8PrintfString msg("Argument " #i " is type %d. It must be a string or undefined", info[i].Type());\
        THROW_TYPE_EXCEPTION_AND_RETURN(msg.c_str(), retval)\
    }

#define DEFINE_CONSTRUCTOR static Napi::FunctionReference& Constructor() { static Napi::FunctionReference s_ctor; return s_ctor; }

// Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
// from running at program shutdown time, which would attempt to reset the reference when the environment is no longer valid.
#define SET_CONSTRUCTOR(t) Constructor() = Napi::Persistent(t); Constructor().SuppressDestruct();

namespace IModelJsNative {

Napi::String toJsString(Napi::Env env, Utf8CP val, size_t len) { return Napi::String::New(env, val, len); }
Napi::String toJsString(Napi::Env env, Utf8CP val) { return toJsString(env, val, std::strlen(val)); }
Napi::String toJsString(Napi::Env env, Utf8StringCR str) { return toJsString(env, str.c_str(), str.length()); }
Napi::String toJsString(Napi::Env env, BeInt64Id id) { return toJsString(env, id.ToHexStr()); }

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

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Affan.Khan                      04/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::Value Convert(Napi::Env env, Json::Value const& jsonValue)
        {
        switch (jsonValue.type())
            {
            case Json::ValueType::booleanValue:
                return Napi::Boolean::New(env, jsonValue.asBool());
            case Json::ValueType::intValue:
                {
                int64_t val = jsonValue.asInt64();
                napi_value v;
                if ((val < std::numeric_limits<std::int32_t>::max()) && napi_create_int32(env, val, &v) == napi_ok)
                    return Napi::Number(env, v);

                return Napi::Number::New (env, val);
                }
            case Json::ValueType::uintValue:
                {
                uint64_t val = jsonValue.asUInt64();
                napi_value v;
                if ((val < std::numeric_limits<std::uint32_t>::max()) && napi_create_uint32(env, val, &v) == napi_ok)
                    return Napi::Number(env, v);

                return Napi::Number::New(env, val);
                }
            case Json::ValueType::realValue:
                return Napi::Number::New(env, jsonValue.asDouble());
            case Json::ValueType::nullValue:
                return env.Null();
            case Json::ValueType::stringValue:
                return Napi::String::New(env, jsonValue.asCString());
            case Json::ValueType::arrayValue:
                {
                Napi::Array jsArray = Napi::Array::New(env, jsonValue.size());
                for (auto itor = jsonValue.begin(); itor != jsonValue.end(); ++itor)
                    jsArray.Set(itor.index(), Convert(env, (*itor)));

                return jsArray;
                }
            case Json::ValueType::objectValue:
                {
                if (jsonValue.isNull()) //also include objectValue
                    return env.Null();

                Napi::Object jsObject = Napi::Object::New(env);
                for (auto itor = jsonValue.begin(); itor != jsonValue.end(); ++itor)
                    jsObject.Set(itor.memberName(), Convert(env, (*itor)));

                return jsObject;
                }
            }

        return env.Undefined();
        }

        static Json::Value Convert(Napi::Value jsValue)
            {
            switch (jsValue.Type())
                {
                case napi_valuetype::napi_boolean:
                    return Json::Value(jsValue.ToBoolean().Value());
                case napi_valuetype::napi_undefined:
                case napi_valuetype::napi_null:
                    return Json::Value();
                case napi_valuetype::napi_number:
                    return Json::Value(jsValue.ToNumber().DoubleValue());
                case napi_valuetype::napi_object:
                    {
                    Napi::Object obj = jsValue.ToObject();
                    if (obj.IsArray())
                        {
                        Json::Value jsonArray(Json::ValueType::arrayValue);
                        for (uint32_t i = 0; i < obj.As<Napi::Array>().Length(); i++)
                            jsonArray.append(Convert(obj.Get(i)));

                        return jsonArray;
                        }

                    if (obj.IsObject())
                        {
                        Json::Value jsonObject(Json::ValueType::objectValue);
                        Napi::Array propertyNames = obj.GetPropertyNames();
                        for (uint32_t i = 0; i < propertyNames.Length(); i++)
                            {
                            Napi::Value propertyName = propertyNames.Get(i);
                            jsonObject[propertyName.ToString().Utf8Value().c_str()] = Convert(obj.Get(propertyName));
                            }

                        return jsonObject;
                        }

                    }
                case napi_valuetype::napi_string:
                    return Json::Value(jsValue.ToString().Utf8Value().c_str());
                case napi_valuetype::napi_symbol:
                case napi_valuetype::napi_external:
                case napi_valuetype::napi_function:
                    break;
                }

            BeAssert(false);
            return Json::Value();;
            }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis                05/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::Value Convert(Napi::Env env, rapidjson::Value const& jsonValue)
        {
        if (jsonValue.IsBool())
            return Napi::Boolean::New(env, jsonValue.GetBool());
        if (jsonValue.IsInt())
            {
            napi_value v;
            if (napi_create_int32(env, jsonValue.GetInt(), &v) != napi_ok)
                return env.Undefined();
            return Napi::Number(env, v);
            }
        if (jsonValue.IsUint())
            {
            napi_value v;
            if (napi_create_int32(env, jsonValue.GetUint(), &v) != napi_ok)
                return env.Undefined();
            return Napi::Number(env, v);
            }
        if (jsonValue.IsInt64())
            {
            napi_value v;
            if (napi_create_int64(env, jsonValue.GetInt64(), &v) != napi_ok)
                return env.Undefined();
            return Napi::Number(env, v);
            }
        if (jsonValue.IsUint64())
            {
            napi_value v;
            if (napi_create_int64(env, jsonValue.GetUint64(), &v) != napi_ok)
                return env.Undefined();
            return Napi::Number(env, v);
            }
        if (jsonValue.IsDouble())
            return Napi::Number::New(env, jsonValue.GetDouble());
        if (jsonValue.IsNull())
            return env.Undefined();
        if (jsonValue.IsString())
            return Napi::String::New(env, jsonValue.GetString());
        if (jsonValue.IsArray())
            {
            Napi::Array jsArray = Napi::Array::New(env, jsonValue.Size());
            for (rapidjson::SizeType i = 0; i < jsonValue.Size(); ++i)
                jsArray.Set((uint32_t)i, Convert(env, jsonValue[i]));
            return jsArray;
            }
        if (jsonValue.IsObject())
            {
            Napi::Object jsObject = Napi::Object::New(env);
            for (auto itor = jsonValue.MemberBegin(); itor != jsonValue.MemberEnd(); ++itor)
                jsObject.Set(itor->name.GetString(), Convert(env, itor->value));
            return jsObject;
            }
        return env.Undefined();
        }
};

#if 0 //WIP
Json::Value Convert(Napi::Value jsValue)
    {
    switch (jsValue.Type())
        {
        case napi_valuetype::napi_boolean:
            return Json::Value(jsValue.ToBoolean().Value());
        case napi_valuetype::napi_null:
            return Json::Value();
        case napi_valuetype::napi_number:
            return Json::Value(jsValue.ToNumber().DoubleValue());
        case napi_valuetype::napi_object:
            {
            Napi::Object obj = jsValue.ToObject();
            if (obj.IsObject())
                {
                Json::Value jsonObject(Json::ValueType::objectValue);
                Napi::Array propertyNames = obj.GetPropertyNames();
                for (uint32_t i = 0; i < propertyNames.Length(); i++)
                    {
                    Napi::Value propertyName = propertyNames.Get(i);
                    jsonObject[propertyName.ToString().Utf8Value().c_str()] = Convert(obj.Get(propertyName));
                    }

                return jsonObject;
                }
            else if (obj.IsArray())
                {
                Json::Value jsonArray(Json::ValueType::arrayValue);
                for (uint32_t i = 0; i < obj.As<Napi::Array>().Length(); i++)
                    {
                    Napi::Value arrayValue = obj.Get(i);
                    jsonArray.append(Convert(obj.Get(arrayValue)));
                    }

                return jsonArray;
                }

            throw Napi::Error::New(jsValue.Env(), "Only Array and Object is supported");
            }
        case napi_valuetype::napi_string:
            return Json::Value(jsValue.ToString().Utf8Value().c_str());
        case napi_valuetype::napi_symbol:
        case napi_valuetype::napi_undefined:
        case napi_valuetype::napi_external:
        case napi_valuetype::napi_function:
        }

    throw Napi::Error::New(jsValue.Env(), "Unsupported valuetype");
    }
#endif

//=======================================================================================
// Projects the ECDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeECDb : Napi::ObjectWrap<NativeECDb>
    {
    private:
        DEFINE_CONSTRUCTOR
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
            return val.As<Napi::Object>().InstanceOf(Constructor().Value());
            }
        ECDbR GetECDb()
            {
            if (m_ecdb == nullptr)
                m_ecdb = std::make_unique<ECDb>();

            return *m_ecdb;
            }

        Napi::Value CreateDb(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
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
            REQUIRE_ARGUMENT_STRING(0, dbName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            REQUIRE_ARGUMENT_INTEGER(1, mode, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            OPTIONAL_ARGUMENT_BOOL(2, upgrade, false, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            Db::OpenParams params((Db::OpenMode) mode);
            if (upgrade)
                params.SetProfileUpgradeOptions(Db::ProfileUpgradeOptions::Upgrade);

            DbResult status = JsInterop::OpenECDb(GetECDb(), BeFileName(dbName.c_str(), true), params);
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
            OPTIONAL_ARGUMENT_STRING(0, changeSetName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
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
            REQUIRE_ARGUMENT_STRING(0, schemaPathName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            DbResult status = JsInterop::ImportSchema(GetECDb(), BeFileName(schemaPathName.c_str(), true));
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value IsOpen(Napi::CallbackInfo const& info) { return Napi::Boolean::New(Env(), GetECDb().IsDbOpen()); }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ECDb", {
            InstanceMethod("createDb", &NativeECDb::CreateDb),
            InstanceMethod("openDb", &NativeECDb::OpenDb),
            InstanceMethod("closeDb", &NativeECDb::CloseDb),
            InstanceMethod("dispose", &NativeECDb::Dispose),
            InstanceMethod("saveChanges", &NativeECDb::SaveChanges),
            InstanceMethod("abandonChanges", &NativeECDb::AbandonChanges),
            InstanceMethod("importSchema", &NativeECDb::ImportSchema),
            InstanceMethod("isOpen", &NativeECDb::IsOpen)
            });

            exports.Set("ECDb", t);

            SET_CONSTRUCTOR(t)
            }
    };

//=======================================================================================
// Projects the NativeECSchemaXmlContext class into JS
//! @bsiclass
//=======================================================================================
struct NativeECSchemaXmlContext : Napi::ObjectWrap<NativeECSchemaXmlContext>
    {
    private:
        DEFINE_CONSTRUCTOR
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
            return obj.InstanceOf(Constructor().Value());
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

        static void Init(Napi::Env env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ECSchemaXmlContext", {
                InstanceMethod("addSchemaPath", &NativeECSchemaXmlContext::AddSchemaPath),
                InstanceMethod("readSchemaFromXmlFile", &NativeECSchemaXmlContext::ReadSchemaFromXmlFile),
                InstanceMethod("setSchemaLocater", &NativeECSchemaXmlContext::SetSchemaLocater)
            });

            exports.Set("ECSchemaXmlContext", t);

            SET_CONSTRUCTOR(t);
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
struct BriefcaseManagerResourcesRequest : Napi::ObjectWrap<BriefcaseManagerResourcesRequest>
{
    IBriefcaseManager::Request m_req;
    DEFINE_CONSTRUCTOR;

    BriefcaseManagerResourcesRequest(Napi::CallbackInfo const& info) : Napi::ObjectWrap<BriefcaseManagerResourcesRequest>(info)
        {
        }

    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "BriefcaseManagerResourcesRequest", {
          InstanceMethod("reset", &BriefcaseManagerResourcesRequest::Reset),
          InstanceMethod("isEmpty", &BriefcaseManagerResourcesRequest::IsEmpty),
          InstanceMethod("toJSON", &BriefcaseManagerResourcesRequest::ToJSON),
        });

        exports.Set("BriefcaseManagerResourcesRequest", t);

        SET_CONSTRUCTOR(t);
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
        return toJsString(Env(), json.ToString());
        }
};

static void initializeGcs()
    {
    bool s_initialized = false;
    if (s_initialized)
        return;
    s_initialized = true;
    BeFileName assetsDir = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    // we are getting back assetsDir with something like "\\?\d:\bentleyjs\imodeljs\.....". The "\\?\" part won't work when we try to open files with fopen,
    // so remove it.
    if (assetsDir.StartsWith (L"\\\\?\\"))
        assetsDir = BeFileName(assetsDir.substr (4));

    BeFileName dgnGeoCoordDir = assetsDir.AppendToPath(L"DgnGeoCoord");
    BentleyApi::GeoCoordinates::BaseGCS::Initialize(dgnGeoCoordDir.c_str());
    }

//=======================================================================================
// Projects the DgnDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeDgnDb : Napi::ObjectWrap<NativeDgnDb>
    {
public:
    static constexpr int ERROR_UsageTrackingFailed = -100;

    // Cancellation token associated with the currently-open DgnDb.
    // The 'cancelled' flag i set on the main thread when the DgnDb is being closed.
    // The token can be passed into functions which create async workers and stored on those
    // workers such that it can be checked for cancellation in the worker's Execute() function to
    // trivially exit if the DgnDb was closed in the interim.
    struct CancellationToken : ICancellationToken
    {
    private:
        BeAtomic<bool> m_canceled;
    public:
        bool IsCanceled() final { return m_canceled.load(); }
        void Register(std::weak_ptr<ICancellationListener>) final { }

        void Cancel()
            {
            // JsInterop::GetLogger().debugv("Setting cancellation token");
            m_canceled.store(true);
            }
    };

    DEFINE_CONSTRUCTOR;

    Dgn::DgnDbPtr m_dgndb;
    ConnectionManager m_connections;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;
    std::shared_ptr<CancellationToken> m_cancellationToken;
    NativeDgnDb(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeDgnDb>(info) {}
    ~NativeDgnDb() {CloseDgnDb();}

    void CloseDgnDb()
        {
        if (!m_dgndb.IsValid())
            return;

        BeAssert(nullptr != m_cancellationToken);
        m_cancellationToken->Cancel();
        m_cancellationToken = nullptr;

        TearDownPresentationManager();
        m_dgndb->RemoveFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->RemoveFunction(StrSqlFunction::GetSingleton());
        m_dgndb->m_jsIModelDb.Reset(); // disconnect the iModelDb object
        JsInterop::CloseDgnDb(*m_dgndb);
        m_dgndb = nullptr;
        }

    void OnDgnDbOpened(DgnDbR dgndb)
        {
        BeAssert(m_dgndb.IsNull());

        m_dgndb = &dgndb;

        SetupPresentationManager();
        // NativeAppData::Add(*this);
        m_dgndb->AddFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->AddFunction(StrSqlFunction::GetSingleton());

        m_cancellationToken = std::make_shared<CancellationToken>();
        }

    //  Check if val is really a NativeDgnDb peer object
    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
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
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode, Napi::Value goodValue) {return ((STATUSTYPE)0 != errCode) ? CreateBentleyReturnErrorObject(errCode) : CreateBentleyReturnSuccessObject(goodValue); }

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode) {return CreateBentleyReturnObject(errCode, Env().Undefined());}

    bool IsOpen() const {return m_dgndb.IsValid();}

    Napi::Value IsDgnDbOpen(Napi::CallbackInfo const &info) { return Napi::Boolean::New(Env(), IsOpen()); }

    void SetIModelDb(Napi::CallbackInfo const& info) {
        Napi::Value obj = info[0];
        if (m_dgndb.IsValid()) {
            if (obj.IsObject())
                m_dgndb->m_jsIModelDb.Reset(obj.As<Napi::Object>(), 1);
            else
                m_dgndb->m_jsIModelDb.Reset();
        }
    }

    Napi::Value OpenDgnDb(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, accessToken, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, appVersion, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(2, projectId, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(3, dbname, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(4, mode, Env().Undefined());

        DgnDbPtr db;
        DbResult status = JsInterop::OpenDgnDb(db, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        if (BE_SQLITE_OK == status)
            OnDgnDbOpened(*db);

        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value OpenDgnDbFile(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbname, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(1, mode, Env().Undefined());

        DgnDbPtr db;
        DbResult status = JsInterop::OpenDgnDb(db, BeFileName(dbname.c_str(), true), (Db::OpenMode)mode);
        if (BE_SQLITE_OK == status)
            OnDgnDbOpened(*db);

        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value CreateIModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, accessToken, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, appVersion, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(2, projectId, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(3, fileName, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(4, args, Env().Undefined());

        DbResult status;
        DgnDbPtr db = JsInterop::CreateIModel(status, fileName, Json::Value::From(args), Env());
        if (db.IsValid())
            OnDgnDbOpened(*db);

        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value CreateStandaloneIModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, fileName, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, args, Env().Undefined());

        DbResult status;
        DgnDbPtr db = JsInterop::CreateIModel(status, fileName, Json::Value::From(args), Env());
        if (db.IsValid())
            OnDgnDbOpened(*db);

        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value GetECClassMetaData(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, s, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, c, Env().Undefined());
        Json::Value metaDataJson;
        auto status = JsInterop::GetECClassMetaData(metaDataJson, GetDgnDb(), s.c_str(), c.c_str());
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), metaDataJson.ToString().c_str()));
        }

    Napi::Value GetSchemaItem(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, schemaName, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, itemName, Env().Undefined());
        Json::Value metaDataJson;
        auto status = JsInterop::GetSchemaItem(metaDataJson, GetDgnDb(), schemaName.c_str(), itemName.c_str());
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), metaDataJson.ToString().c_str()));
        }

    Napi::Value GetSchema(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, name, Env().Undefined());

        Json::Value metaDataJson;
        auto status = JsInterop::GetSchema(metaDataJson, GetDgnDb(), name.c_str());
        return CreateBentleyReturnObject(status, Napi::String::New(Env(), metaDataJson.ToString().c_str()));
         }

    Napi::Value GetElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, optsJsonStr, Env().Undefined());
        Json::Value opts = Json::Value::From(optsJsonStr);
        Json::Value elementJson;  // output
        auto status = JsInterop::GetElement(elementJson, GetDgnDb(), opts);

        Napi::Value jsValue = NapiUtils::Convert(Env(), elementJson);
        return CreateBentleyReturnObject(status, jsValue);
        }

    Napi::Value GetModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, optsJsonStr, Env().Undefined());
        Json::Value opts = Json::Value::From(optsJsonStr);
        Json::Value modelJson; // output
        DgnDbStatus status = JsInterop::GetModel(modelJson, GetDgnDb(), opts);
        return CreateBentleyReturnObject(status, toJsString(Env(), modelJson.ToString()));
        }

    Napi::Value QueryModelExtents(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, optionsJsonStr, Env().Undefined());
        Json::Value options = Json::Value::From(optionsJsonStr);
        Json::Value extentsJson; // output
        DgnDbStatus status = JsInterop::QueryModelExtents(extentsJson, GetDgnDb(), options);
        return CreateBentleyReturnObject(status, toJsString(Env(), extentsJson.ToString()));
        }

    Napi::Value DumpChangeSet(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, changeSetToken, Env().Undefined());
        Json::Value jsonChangeSetToken = Json::Value::From(changeSetToken);

        RevisionStatus status = JsInterop::DumpChangeSet(*m_dgndb, jsonChangeSetToken);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ApplyChangeSets(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, changeSetTokens, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(1, applyOption, Env().Undefined());

        bvector<DgnRevisionPtr> revisionPtrs;
        bool containsSchemaChanges;
        Utf8String dbGuid = m_dgndb->GetDbGuid().ToString();
        Json::Value jsonChangeSetTokens = Json::Value::From(changeSetTokens);
        RevisionStatus status = JsInterop::ReadChangeSets(revisionPtrs, containsSchemaChanges, dbGuid, jsonChangeSetTokens);
        if (RevisionStatus::Success != status)
            return Napi::Number::New(Env(), (int)status);

        bvector<DgnRevisionCP> revisions;
        for (uint32_t ii = 0; ii < revisionPtrs.size(); ii++)
            revisions.push_back(revisionPtrs[ii].get());

        /* Process change sets containing only data changes */
        if (!containsSchemaChanges)
            {
            status = JsInterop::ApplyDataChangeSets(*m_dgndb, revisions, (RevisionProcessOption) applyOption);
            return Napi::Number::New(Env(), (int)status);
            }

        /* Process change sets containing schema changes */
        BeFileName dbFileName(m_dgndb->GetDbFileName(), true);
        bool isReadonly = m_dgndb->IsReadonly();

        CloseDgnDb();

        status = JsInterop::ApplySchemaChangeSets(dbFileName, revisions, (RevisionProcessOption)applyOption);
        if (RevisionStatus::Success != status)
            return Napi::Number::New(Env(), (int)status);

        DgnDbPtr db;
        DbResult result = JsInterop::OpenDgnDb(db, dbFileName, isReadonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite);
        if (BE_SQLITE_OK == result)
            OnDgnDbOpened(*db);
        else
            status = RevisionStatus::ApplyError;

        return Napi::Number::New(Env(), (int)status);
        }

    static TxnManager::TxnId TxnIdFromString(Utf8StringCR str) {return TxnManager::TxnId(BeInt64Id::FromString(str.c_str()).GetValue());}
    static Utf8String TxnIdToString(TxnManager::TxnId txnId) {return BeInt64Id(txnId.GetValue()).ToHexStr();}

    Napi::Value GetCurrentTxnId(Napi::CallbackInfo const& info)
        {
        return toJsString(Env(), TxnIdToString(m_dgndb->Txns().GetCurrentTxnId()));
        }

    Napi::Value QueryFirstTxnId(Napi::CallbackInfo const& info)
        {
        TxnManager::TxnId startTxnId = m_dgndb->Txns().QueryNextTxnId(TxnManager::TxnId(0));
        return toJsString(Env(), TxnIdToString(startTxnId));
        }

    Napi::Value QueryNextTxnId(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        auto next = m_dgndb->Txns().QueryNextTxnId(TxnIdFromString(txnIdHexStr));
        return toJsString(Env(), TxnIdToString(next));
    }
    Napi::Value QueryPreviousTxnId(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        auto next = m_dgndb->Txns().QueryPreviousTxnId(TxnIdFromString(txnIdHexStr));
        return toJsString(Env(), TxnIdToString(next));
    }
    Napi::Value GetTxnDescription(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        return toJsString(Env(), m_dgndb->Txns().GetTxnDescription(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value IsTxnIdValid(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        return Napi::Boolean::New(Env(), TxnIdFromString(txnIdHexStr).IsValid());
    }
    Napi::Value HasFatalTxnError(Napi::CallbackInfo const& info) { return Napi::Boolean::New(Env(), m_dgndb->Txns().HasFatalError()); }
    void LogTxnError(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_BOOL(0, fatal, );
        m_dgndb->Txns().LogError(fatal);
    }
    void EnableTxnTesting(Napi::CallbackInfo const& info) {
        m_dgndb->Txns().EnableTracking(true);
        m_dgndb->Txns().InitializeTableHandlers();
    }
    Napi::Value BeginMultiTxnOperation(Napi::CallbackInfo const& info) {return Napi::Number::New(Env(),  (int) m_dgndb->Txns().BeginMultiTxnOperation());}
    Napi::Value EndMultiTxnOperation(Napi::CallbackInfo const& info) {return Napi::Number::New(Env(),  (int) m_dgndb->Txns().EndMultiTxnOperation());}
    Napi::Value GetMultiTxnOperationDepth(Napi::CallbackInfo const& info) {return Napi::Number::New(Env(),  (int) m_dgndb->Txns().GetMultiTxnOperationDepth());}
    Napi::Value GetUndoString(Napi::CallbackInfo const& info) {return toJsString(Env(), m_dgndb->Txns().GetUndoString());}
    Napi::Value GetRedoString(Napi::CallbackInfo const& info) {return toJsString(Env(), m_dgndb->Txns().GetRedoString());}
    Napi::Value HasUnsavedChanges(Napi::CallbackInfo const& info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().HasChanges());}
    Napi::Value IsRedoPossible(Napi::CallbackInfo const& info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().IsRedoPossible());}
    Napi::Value IsUndoPossible(Napi::CallbackInfo const& info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().IsUndoPossible());}
    Napi::Value ReinstateTxn(Napi::CallbackInfo const& info) {return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReinstateTxn());}
    Napi::Value ReverseAll(Napi::CallbackInfo const& info) {return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseAll(false));}
    Napi::Value ReverseTo(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseTo(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value CancelTo(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().CancelTo(TxnIdFromString(txnIdHexStr)));
    }
    Napi::Value ReverseTxns(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_NUMBER(0, numTxns , Env().Undefined());
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseTxns(numTxns));
    }

    Napi::Value StartCreateChangeSet(Napi::CallbackInfo const& info)
        {
        Json::Value changeSetInfo;
        RevisionStatus status = JsInterop::StartCreateChangeSet(changeSetInfo, *m_dgndb);
        return CreateBentleyReturnObject(status, toJsString(Env(), changeSetInfo.ToString()));
        }

    Napi::Value FinishCreateChangeSet(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        RevisionStatus status = JsInterop::FinishCreateChangeSet(*m_dgndb);
        return Napi::Number::New(Env(), (int) status);
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
        return CreateBentleyReturnObject(result, toJsString(Env(), codes.ToString()));
        }

    Napi::Value ExtractCodesFromFile(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, changeSetToken, Env().Undefined());
        Json::Value jsonChangeSetToken = Json::Value::From(changeSetToken);
        Json::Value codes;
        DbResult result = JsInterop::ExtractCodesFromFile(codes, *m_dgndb, jsonChangeSetToken);
        return CreateBentleyReturnObject(result, toJsString(Env(), codes.ToString()));
        }

    Napi::Value GetPendingChangeSets(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        Json::Value changeSets;
        DbResult result = JsInterop::GetPendingChangeSets(changeSets, *m_dgndb);
        return CreateBentleyReturnObject(result, toJsString(Env(), changeSets.ToString()));
        }

    Napi::Value AddPendingChangeSet(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, changeSetId, Env().Undefined());
        DbResult result = JsInterop::AddPendingChangeSet(*m_dgndb, changeSetId);
        return Napi::Number::New(Env(), (int) result);
        }

    Napi::Value RemovePendingChangeSet(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, changeSetId, Env().Undefined());
        DbResult result = JsInterop::RemovePendingChangeSet(*m_dgndb, changeSetId);
        return Napi::Number::New(Env(), (int) result);
        }

    Napi::Value GetIModelCoordsFromGeoCoords(Napi::CallbackInfo const& info)
        {
        // make sure GCS library is initialized.
        initializeGcs();

        // Use the macro to get the argument (which is JSON string containing an array of points) from the info argument.
        REQUIRE_ARGUMENT_STRING(0, geoCoordStr, Env().Undefined());
        // get a Json::Value from the string.
        Json::Value geoCoordProps = Json::Value::From(geoCoordStr);
        Json::Value results;
        JsInterop::GetIModelCoordsFromGeoCoords (results, GetDgnDb(), geoCoordProps);
        return toJsString(Env(), results.ToString());
        }

    Napi::Value GetGeoCoordsFromIModelCoords(Napi::CallbackInfo const& info)
        {
        // make sure GCS library is initialized.
        initializeGcs();

        // Use the macro to get the argument (which is JSON string containing an array of points) from the info argument.
        REQUIRE_ARGUMENT_STRING(0, iModelCoordStr, Env().Undefined());
        // get a Json::Value from the string.
        Json::Value iModelCoordProps = Json::Value::From(iModelCoordStr);
        Json::Value results;
        JsInterop::GetGeoCoordsFromIModelCoords(results, GetDgnDb(), iModelCoordProps);
        return toJsString(Env(), results.ToString());
        }

    Napi::Value GetIModelProps(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        Json::Value props;
        JsInterop::GetIModelProps(props, *m_dgndb);
        return toJsString(Env(), props.ToString());
        }

    Napi::Value InsertElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, elemPropsJsonStr, Env().Undefined());
        Json::Value elemProps = Json::Value::From(elemPropsJsonStr);
        Json::Value elemIdJsonObj;
        auto status = JsInterop::InsertElement(elemIdJsonObj, GetDgnDb(), elemProps);
        return CreateBentleyReturnObject(status, toJsString(Env(), elemIdJsonObj.ToString()));
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

    Napi::Value InsertElementAspect(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, aspectPropsJsonStr, Env().Undefined());
        Json::Value aspectProps = Json::Value::From(aspectPropsJsonStr);
        DgnDbStatus status = JsInterop::InsertElementAspect(GetDgnDb(), aspectProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value UpdateElementAspect(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, aspectPropsJsonStr, Env().Undefined());
        Json::Value aspectProps = Json::Value::From(aspectPropsJsonStr);
        DgnDbStatus status = JsInterop::UpdateElementAspect(GetDgnDb(), aspectProps);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value DeleteElementAspect(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, aspectIdStr, Env().Undefined());
        DgnDbStatus status = JsInterop::DeleteElementAspect(GetDgnDb(), aspectIdStr);
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value ExportGraphics(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_ANY_OBJ(0, exportProps, Env().Undefined());

        Napi::Value onGraphicsVal = exportProps.Get("onGraphics");
        if (!onGraphicsVal.IsFunction())
            THROW_TYPE_EXCEPTION_AND_RETURN("onGraphics must be a function", Env().Undefined());

        Napi::Value elementIdArrayVal = exportProps.Get("elementIdArray");
        if (!elementIdArrayVal.IsArray())
            THROW_TYPE_EXCEPTION_AND_RETURN("elementIdArray must be an array", Env().Undefined());

        DgnDbStatus status = JsInterop::ExportGraphics(GetDgnDb(), exportProps);
        return Napi::Number::New(Env(), (int)status);
        }

    void GetTileTree(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, idStr, );
        REQUIRE_ARGUMENT_FUNCTION(1, responseCallback, );

        JsInterop::GetTileTree(m_cancellationToken, GetDgnDb(), idStr, responseCallback);
        }

    void GetTileContent(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, treeIdStr, );
        REQUIRE_ARGUMENT_STRING(1, tileIdStr, );
        REQUIRE_ARGUMENT_FUNCTION(2, responseCallback, );

        JsInterop::GetTileContent(m_cancellationToken, GetDgnDb(), treeIdStr, tileIdStr, responseCallback);
        }

    Napi::Value PollTileContent(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, treeIdStr, Napi::Number::New(Env(), static_cast<int>(DgnDbStatus::BadRequest)));
        REQUIRE_ARGUMENT_STRING(1, tileIdStr, Napi::Number::New(Env(), static_cast<int>(DgnDbStatus::BadRequest)));

        auto result = JsInterop::PollTileContent(m_cancellationToken, GetDgnDb(), treeIdStr, tileIdStr);
        if (Tile::State::Completed != result.m_state)
            {
            return CreateBentleyReturnSuccessObject(Napi::Number::New(Env(), static_cast<int>(result.m_state)));
            }
        else if (DgnDbStatus::Success != result.m_status)
            {
            return CreateBentleyReturnErrorObject(result.m_status);
            }
        else
            {
            BeAssert(result.m_content.IsValid());
            ByteStreamCR geometry = result.m_content->GetBytes();
            auto blob = Napi::Uint8Array::New(Env(), geometry.size());
            memcpy(blob.Data(), geometry.data(), geometry.size());
            return CreateBentleyReturnSuccessObject(blob);
            }
        }

    Napi::Value InsertLinkTableRelationship(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, propsJsonStr, Env().Undefined());
        Json::Value props = Json::Value::From(propsJsonStr);
        Json::Value idJsonObj;
        auto status = JsInterop::InsertLinkTableRelationship(idJsonObj, GetDgnDb(), props);
        return CreateBentleyReturnObject(status, toJsString(Env(), idJsonObj.asCString()));
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
        return CreateBentleyReturnObject(status, toJsString(Env(), idStr));
        }

    Napi::Value InsertModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, modelPropsJson, Env().Undefined());
        Json::Value modelProps = Json::Value::From(modelPropsJson);
        Json::Value modelIdProps;
        auto status = JsInterop::InsertModel(modelIdProps, GetDgnDb(), modelProps);
        return CreateBentleyReturnObject(status, toJsString(Env(), modelIdProps.ToString()));
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
        ECClassCP ecclass = GetDgnDb().Schemas().GetClass(ecclassId);
        KeySetPtr input = KeySet::Create({ECClassInstanceKey(ecclass, elemId)});
        RulesDrivenECPresentationManager::ContentOptions options ("Items");
        if ( m_presentationManager == nullptr)
            return CreateBentleyReturnErrorObject(DgnDbStatus::BadArg);

        m_connections.NotifyConnectionOpened(GetDgnDb());
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

        return CreateBentleyReturnSuccessObject(toJsString(Env(), exportedJson));
        };

    Napi::Value SaveChanges(Napi::CallbackInfo const& info)
        {
        OPTIONAL_ARGUMENT_STRING(0, description, Env().Undefined());
        REQUIRE_DB_TO_BE_OPEN
        auto was = JsInterop::SetOkEndBulkMode(true);
        auto stat = GetDgnDb().SaveChanges(description);
        JsInterop::SetOkEndBulkMode(was);
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

    Napi::Value ImportFunctionalSchema(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        DbResult result = JsInterop::ImportFunctionalSchema(GetDgnDb());
        return Napi::Number::New(Env(), (int)result);
        }

    void CloseDgnDb(Napi::CallbackInfo const& info) {  CloseDgnDb(); }

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
        BeFileName changesetFilePath(changesetFilePathStr.c_str(), true);
        RevisionChangesFileReader changeStream(changesetFilePath, GetDgnDb());
        PERFLOG_START("iModelJsNative", "ExtractChangeSummary>ECDb::ExtractChangeSummary");
        ECInstanceKey changeSummaryKey;
        if (SUCCESS != ECDb::ExtractChangeSummary(changeSummaryKey, changeCacheECDb->GetECDb(), GetDgnDb(), ChangeSetArg(changeStream)))
            return CreateBentleyReturnErrorObject(BE_SQLITE_ERROR, Utf8PrintfString("Failed to extract ChangeSummary for ChangeSet file '%s'.", changesetFilePathStr.c_str()).c_str());
        PERFLOG_FINISH("iModelJsNative", "ExtractChangeSummary>ECDb::ExtractChangeSummary");

        return CreateBentleyReturnSuccessObject(toJsString(Env(), changeSummaryKey.GetInstanceId()));
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
            {
            SchemaUpgradeOptions schemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);
            DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);

            m_dgndb = DgnDb::OpenDgnDb(&result, name, openParams);
            }

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
        return toJsString(Env(), parentRevId);
        }

    Napi::Value GetReversedChangeSetId(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        if (!m_dgndb->Revisions().HasReversedRevisions())
            return Env().Undefined();
        Utf8String reversedRevId = m_dgndb->Revisions().GetReversedRevisionId();
        return toJsString(Env(), reversedRevId);
        }

    Napi::Value GetDbGuid(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        BeGuid beGuid = m_dgndb->GetDbGuid();
        return toJsString(Env(), beGuid.ToString());
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

    Napi::Value SetAsMaster(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        OPTIONAL_ARGUMENT_STRING(0, guidStr, Env().Undefined());
        if (!guidStr.empty())
            {
            BeGuid guid;
            if (guid.FromString(guidStr.c_str()) != SUCCESS)
                return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

            return Napi::Number::New(Env(), (int) m_dgndb->SetAsMaster(guid));
            }

        return Napi::Number::New(Env(), (int) m_dgndb->SetAsMaster());
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForElement(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, BriefcaseManagerResourcesRequest, req, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, elemProps, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(2, dbop, Env().Undefined());
        Json::Value elemPropsJson = Json::Value::From(elemProps);
        return Napi::Number::New(Env(), (int)JsInterop::BuildBriefcaseManagerResourcesRequestForElement(req->m_req, GetDgnDb(), elemPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, BriefcaseManagerResourcesRequest, req, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, props, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(2, dbop, Env().Undefined());
        Json::Value propsJson = Json::Value::From(props);
        return Napi::Number::New(Env(), (int)JsInterop::BuildBriefcaseManagerResourcesRequestForLinkTableRelationship(req->m_req, GetDgnDb(), propsJson, (BeSQLite::DbOpcode)dbop));
        }

    Napi::Value BuildBriefcaseManagerResourcesRequestForModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, BriefcaseManagerResourcesRequest, req, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, modelProps, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(2, dbop, Env().Undefined());
        Json::Value modelPropsJson = Json::Value::From(modelProps);
        return Napi::Number::New(Env(), (int)JsInterop::BuildBriefcaseManagerResourcesRequestForModel(req->m_req, GetDgnDb(), modelPropsJson, (BeSQLite::DbOpcode)dbop));
        }

    void ExtractBulkResourcesRequest(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, BriefcaseManagerResourcesRequest, req,);
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
        REQUIRE_ARGUMENT_OBJ(0, BriefcaseManagerResourcesRequest, req, );
        REQUIRE_ARGUMENT_OBJ(1, BriefcaseManagerResourcesRequest, reqIn, );
        req->m_req.Codes().insert(reqIn->m_req.Codes().begin(), reqIn->m_req.Codes().end());
        req->m_req.Locks().GetLockSet().insert(reqIn->m_req.Locks().GetLockSet().begin(), reqIn->m_req.Locks().GetLockSet().end());
        // TBD: merge in request options
        }

   void ExtractBriefcaseManagerResourcesRequest(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, BriefcaseManagerResourcesRequest, reqOut, );
        REQUIRE_ARGUMENT_OBJ(1, BriefcaseManagerResourcesRequest, reqIn, );
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

    static Napi::Value GetAssetDir(Napi::CallbackInfo const& info)
        {
        BeFileName asset = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
        return Napi::String::New(info.Env(), asset.GetNameUtf8().c_str());
        }

    void UpdateProjectExtents(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, newExtentsJson, )
        JsInterop::UpdateProjectExtents(GetDgnDb(), Json::Value::From(newExtentsJson));
        }
    void UpdateIModelProps(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, props, )
        JsInterop::UpdateIModelProps(GetDgnDb(), Json::Value::From(props));
        }
    Napi::Value ReadFontMap(Napi::CallbackInfo const& info)
        {
        auto const& fontMap = GetDgnDb().Fonts().FontMap();
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
        return toJsString(Env(), fonts.ToString());
        }

    Napi::Value EmbedFont(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, fontProps, Env().Undefined());
        auto propsJson = Json::Value::From(fontProps);
        if (!propsJson.isMember(DgnFonts::json_name()) || !propsJson.isMember(DgnFonts::json_type()))
            {
            Napi::TypeError::New(Env(), "Invalid FontProps").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        DgnFontType fontType = (DgnFontType) propsJson[DgnFonts::json_type()].asInt();
        if (DgnFontType::TrueType != fontType)
            {
            Napi::TypeError::New(Env(), "Unsupported font type").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        Utf8CP fontName = propsJson[DgnFonts::json_name()].asCString();
        DgnFontPtr font = DgnFontPersistence::OS::FromGlobalTrueTypeRegistry(fontName);
        if (!font.IsValid())
            {
            Napi::TypeError::New(Env(), "Unable to locate font").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        auto& fonts = GetDgnDb().Fonts();
        DgnFontId fontId = fonts.DbFontMap().QueryIdByTypeAndName(fontType, fontName);
        if (fontId.IsValid())
            {
            if (SUCCESS != fonts.DbFontMap().Update(*font, fontId)) // Update rsc and shx font metadata...
                {
                Napi::TypeError::New(Env(), "Unable to update font").ThrowAsJavaScriptException();
                return Env().Undefined();
                }
            }
        else
            {
            if (SUCCESS != fonts.DbFontMap().Insert(*font, fontId))
                {
                Napi::TypeError::New(Env(), "Unable to insert font").ThrowAsJavaScriptException();
                return Env().Undefined();
                }
            }

        if (SUCCESS != DgnFontPersistence::Db::Embed(fonts.DbFaceData(), *font))
            {
            Napi::TypeError::New(Env(), "Unable to embed font").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        fonts.Invalidate(); // Make sure font map gets updated...

        auto thisFont = Json::Value();
        thisFont[DgnFonts::json_id()] = (int)fontId.GetValue();
        thisFont[DgnFonts::json_type()] = (int)fontType;
        thisFont[DgnFonts::json_name()] = fontName;
        return toJsString(Env(), thisFont.ToString());
        }

    // query a property from the be_prop table.
    Napi::Value QueryFileProperty(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, fileProps, Env().Undefined())
        REQUIRE_ARGUMENT_BOOL(1, wantString, Env().Undefined()) // boolean indicating whether the desired property is a string or blob.
        auto propsJson = Json::Value::From(fileProps);
        if (!propsJson.isMember(JsInterop::json_namespace()) || !propsJson.isMember(JsInterop::json_name()))
            {
            Napi::TypeError::New(Env(), "Invalid FilePropertyProps").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        PropertySpec spec(propsJson[JsInterop::json_name()].asCString(), propsJson[JsInterop::json_namespace()].asCString());
        auto& db = GetDgnDb();
        uint64_t id = propsJson[JsInterop::json_id()].asUInt64();
        uint64_t subId = propsJson[JsInterop::json_subId()].asUInt64();

        if (wantString)
            {
            Utf8String strVal;
            auto stat = db.QueryProperty(strVal, spec, id, subId);
            return (stat != BE_SQLITE_ROW) ?  Env().Undefined() :  toJsString(Env(), strVal);
            }

        uint32_t size;
        auto stat = db.QueryPropertySize(size, spec, id, subId);
        if (stat != BE_SQLITE_ROW || size == 0)
            return Env().Undefined();

        auto blob = Napi::Uint8Array::New(Env(), size);
        db.QueryProperty(blob.Data(), size, spec, id, subId);
        return blob;
        }

    // save a property to the be_prop table
    Napi::Value SaveFileProperty(Napi::CallbackInfo const& info)
        {
        if (info.Length() < 3)
            THROW_TYPE_EXCEPTION_AND_RETURN("saveFileProperty requires 3 arguments", Env().Undefined());

        REQUIRE_ARGUMENT_STRING(0, fileProps, Env().Undefined())
        auto propsJson = Json::Value::From(fileProps);
        if (!propsJson.isMember(JsInterop::json_namespace()) || !propsJson.isMember(JsInterop::json_name()))
            {
            Napi::TypeError::New(Env(), "Invalid FilePropertyProps").ThrowAsJavaScriptException();
            return Env().Undefined();
            }
        PropertySpec spec(propsJson[JsInterop::json_name()].asCString(), propsJson[JsInterop::json_namespace()].asCString());
        auto& db = GetDgnDb();
        uint64_t id = propsJson[JsInterop::json_id()].asUInt64();
        uint64_t subId = propsJson[JsInterop::json_subId()].asUInt64();

        DbResult stat;
        Utf8String strbuf;
        Utf8StringCP strDataP = nullptr;
        void const* value = nullptr;
        uint32_t propsize = 0;

        if (info[1].IsString())
            {
            strbuf = info[1].ToString().Utf8Value().c_str();
            strDataP = &strbuf;
            }
        if (info[2].IsTypedArray())
            {
            auto arrayBuf = info[2].As<Napi::Uint8Array>();
            value = arrayBuf.Data();
            propsize = arrayBuf.ByteLength();
            }
        if (nullptr == strDataP && nullptr == value)
            {
            stat = db.DeleteProperty(spec, id, subId);
            if (stat == BE_SQLITE_DONE)
                stat = BE_SQLITE_OK;
            }
        else
            stat =  strDataP ? db.SaveProperty(spec, *strDataP, value, propsize, id, subId) :  db.SaveProperty(spec, value, propsize, id, subId);
        return Napi::Number::New(Env(), (int) stat);
        }

    Napi::Value QueryNextAvailableFileProperty(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, fileProps, Env().Undefined())
        auto propsJson = Json::Value::From(fileProps);
        if (!propsJson.isMember(JsInterop::json_namespace()) || !propsJson.isMember(JsInterop::json_name()))
            {
            Napi::TypeError::New(Env(), "Invalid FilePropertyProps").ThrowAsJavaScriptException();
            return Env().Undefined();
            }

        auto& db = GetDgnDb();
        Statement stmt(db, "SELECT count(Id),max(Id) FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=?");
        stmt.BindText(1, propsJson[JsInterop::json_namespace()].asCString(), Statement::MakeCopy::No);
        stmt.BindText(2, propsJson[JsInterop::json_name()].asCString(), Statement::MakeCopy::No);
        DbResult result = stmt.Step();
        uint64_t count = stmt.GetValueUInt64(0);
        uint64_t max  = stmt.GetValueUInt64(1);
        uint64_t next = (result != BE_SQLITE_ROW || 0 == count) ? 0 : max + 1;
        return Napi::Number::New(Env(), next);
        }

    // ========================================================================================
    // Test method handler
    // ========================================================================================
    Napi::Value ExecuteTest(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, testName, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, params, Env().Undefined());
        return toJsString(Env(), JsInterop::ExecuteTest(GetDgnDb(), testName, params).ToString());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "DgnDb", {
            InstanceMethod("abandonChanges", &NativeDgnDb::AbandonChanges),
            InstanceMethod("abandonCreateChangeSet", &NativeDgnDb::AbandonCreateChangeSet),
            InstanceMethod("addPendingChangeSet", &NativeDgnDb::AddPendingChangeSet),
            InstanceMethod("appendBriefcaseManagerResourcesRequest", &NativeDgnDb::AppendBriefcaseManagerResourcesRequest),
            InstanceMethod("applyChangeSets", &NativeDgnDb::ApplyChangeSets),
            InstanceMethod("attachChangeCache", &NativeDgnDb::AttachChangeCache),
            InstanceMethod("beginMultiTxnOperation", &NativeDgnDb::BeginMultiTxnOperation),
            InstanceMethod("briefcaseManagerEndBulkOperation", &NativeDgnDb::BriefcaseManagerEndBulkOperation),
            InstanceMethod("briefcaseManagerStartBulkOperation", &NativeDgnDb::BriefcaseManagerStartBulkOperation),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &NativeDgnDb::BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForModel", &NativeDgnDb::BuildBriefcaseManagerResourcesRequestForModel),
            InstanceMethod("cancelTo", &NativeDgnDb::CancelTo),
            InstanceMethod("closeIModel", &NativeDgnDb::CloseDgnDb),
            InstanceMethod("createChangeCache", &NativeDgnDb::CreateChangeCache),
            InstanceMethod("createIModel", &NativeDgnDb::CreateIModel),
            InstanceMethod("createStandaloneIModel", &NativeDgnDb::CreateStandaloneIModel),
            InstanceMethod("deleteElement", &NativeDgnDb::DeleteElement),
            InstanceMethod("deleteElementAspect", &NativeDgnDb::DeleteElementAspect),
            InstanceMethod("deleteLinkTableRelationship", &NativeDgnDb::DeleteLinkTableRelationship),
            InstanceMethod("deleteModel", &NativeDgnDb::DeleteModel),
            InstanceMethod("detachChangeCache", &NativeDgnDb::DetachChangeCache),
            InstanceMethod("dumpChangeSet", &NativeDgnDb::DumpChangeSet),
            InstanceMethod("embedFont", &NativeDgnDb::EmbedFont),
            InstanceMethod("enableTxnTesting", &NativeDgnDb::EnableTxnTesting),
            InstanceMethod("endMultiTxnOperation", &NativeDgnDb::EndMultiTxnOperation),
            InstanceMethod("executeTest", &NativeDgnDb::ExecuteTest),
            InstanceMethod("exportGraphics", &NativeDgnDb::ExportGraphics),
            InstanceMethod("extractBriefcaseManagerResourcesRequest", &NativeDgnDb::ExtractBriefcaseManagerResourcesRequest),
            InstanceMethod("extractBulkResourcesRequest", &NativeDgnDb::ExtractBulkResourcesRequest),
            InstanceMethod("extractChangeSummary", &NativeDgnDb::ExtractChangeSummary),
            InstanceMethod("extractCodes", &NativeDgnDb::ExtractCodes),
            InstanceMethod("extractCodesFromFile", &NativeDgnDb::ExtractCodesFromFile),
            InstanceMethod("finishCreateChangeSet", &NativeDgnDb::FinishCreateChangeSet),
            InstanceMethod("getBriefcaseId", &NativeDgnDb::GetBriefcaseId),
            InstanceMethod("getCurrentTxnId", &NativeDgnDb::GetCurrentTxnId),
            InstanceMethod("getDbGuid", &NativeDgnDb::GetDbGuid),
            InstanceMethod("getECClassMetaData", &NativeDgnDb::GetECClassMetaData),
            InstanceMethod("getElement", &NativeDgnDb::GetElement),
            InstanceMethod("getElementPropertiesForDisplay", &NativeDgnDb::GetElementPropertiesForDisplay),
            InstanceMethod("getGeoCoordinatesFromIModelCoordinates", &NativeDgnDb::GetGeoCoordsFromIModelCoords),
            InstanceMethod("getIModelCoordinatesFromGeoCoordinates", &NativeDgnDb::GetIModelCoordsFromGeoCoords),
            InstanceMethod("getIModelProps", &NativeDgnDb::GetIModelProps),
            InstanceMethod("getModel", &NativeDgnDb::GetModel),
            InstanceMethod("getMultiTxnOperationDepth", &NativeDgnDb::GetMultiTxnOperationDepth),
            InstanceMethod("getParentChangeSetId", &NativeDgnDb::GetParentChangeSetId),
            InstanceMethod("getPendingChangeSets", &NativeDgnDb::GetPendingChangeSets),
            InstanceMethod("getRedoString", &NativeDgnDb::GetRedoString),
            InstanceMethod("getReversedChangeSetId", &NativeDgnDb::GetReversedChangeSetId),
            InstanceMethod("getSchema", &NativeDgnDb::GetSchema),
            InstanceMethod("getSchemaItem", &NativeDgnDb::GetSchemaItem),
            InstanceMethod("getTileContent", &NativeDgnDb::GetTileContent),
            InstanceMethod("pollTileContent", &NativeDgnDb::PollTileContent),
            InstanceMethod("getTileTree", &NativeDgnDb::GetTileTree),
            InstanceMethod("getTxnDescription", &NativeDgnDb::GetTxnDescription),
            InstanceMethod("getUndoString", &NativeDgnDb::GetUndoString),
            InstanceMethod("hasFatalTxnError", &NativeDgnDb::HasFatalTxnError),
            InstanceMethod("hasUnsavedChanges", &NativeDgnDb::HasUnsavedChanges),
            InstanceMethod("importFunctionalSchema", &NativeDgnDb::ImportFunctionalSchema),
            InstanceMethod("importSchema", &NativeDgnDb::ImportSchema),
            InstanceMethod("inBulkOperation", &NativeDgnDb::InBulkOperation),
            InstanceMethod("insertCodeSpec", &NativeDgnDb::InsertCodeSpec),
            InstanceMethod("insertElement", &NativeDgnDb::InsertElement),
            InstanceMethod("insertElementAspect", &NativeDgnDb::InsertElementAspect),
            InstanceMethod("insertLinkTableRelationship", &NativeDgnDb::InsertLinkTableRelationship),
            InstanceMethod("insertModel", &NativeDgnDb::InsertModel),
            InstanceMethod("isChangeCacheAttached", &NativeDgnDb::IsChangeCacheAttached),
            InstanceMethod("isOpen", &NativeDgnDb::IsDgnDbOpen),
            InstanceMethod("isRedoPossible", &NativeDgnDb::IsRedoPossible),
            InstanceMethod("isTxnIdValid", &NativeDgnDb::IsTxnIdValid),
            InstanceMethod("isUndoPossible", &NativeDgnDb::IsUndoPossible),
            InstanceMethod("logTxnError", &NativeDgnDb::LogTxnError),
            InstanceMethod("openIModel", &NativeDgnDb::OpenDgnDb),
            InstanceMethod("openIModelFile", &NativeDgnDb::OpenDgnDbFile),
            InstanceMethod("queryFileProperty", &NativeDgnDb::QueryFileProperty),
            InstanceMethod("queryFirstTxnId", &NativeDgnDb::QueryFirstTxnId),
            InstanceMethod("queryModelExtents", &NativeDgnDb::QueryModelExtents),
            InstanceMethod("queryNextAvailableFileProperty", &NativeDgnDb::QueryNextAvailableFileProperty),
            InstanceMethod("queryNextTxnId", &NativeDgnDb::QueryNextTxnId),
            InstanceMethod("queryPreviousTxnId", &NativeDgnDb::QueryPreviousTxnId),
            InstanceMethod("readFontMap", &NativeDgnDb::ReadFontMap),
            InstanceMethod("reinstateTxn", &NativeDgnDb::ReinstateTxn),
            InstanceMethod("removePendingChangeSet", &NativeDgnDb::RemovePendingChangeSet),
            InstanceMethod("reverseAll", &NativeDgnDb::ReverseAll),
            InstanceMethod("reverseTo", &NativeDgnDb::ReverseTo),
            InstanceMethod("reverseTxns", &NativeDgnDb::ReverseTxns),
            InstanceMethod("saveChanges", &NativeDgnDb::SaveChanges),
            InstanceMethod("saveFileProperty", &NativeDgnDb::SaveFileProperty),
            InstanceMethod("setAsMaster", &NativeDgnDb::SetAsMaster),
            InstanceMethod("setBriefcaseId", &NativeDgnDb::SetBriefcaseId),
            InstanceMethod("setBriefcaseManagerOptimisticConcurrencyControlPolicy", &NativeDgnDb::SetBriefcaseManagerOptimisticConcurrencyControlPolicy),
            InstanceMethod("setBriefcaseManagerPessimisticConcurrencyControlPolicy", &NativeDgnDb::SetBriefcaseManagerPessimisticConcurrencyControlPolicy),
            InstanceMethod("setDbGuid", &NativeDgnDb::SetDbGuid),
            InstanceMethod("setIModelDb", &NativeDgnDb::SetIModelDb),
            InstanceMethod("startCreateChangeSet", &NativeDgnDb::StartCreateChangeSet),
            InstanceMethod("updateElement", &NativeDgnDb::UpdateElement),
            InstanceMethod("updateElementAspect", &NativeDgnDb::UpdateElementAspect),
            InstanceMethod("updateIModelProps", &NativeDgnDb::UpdateIModelProps),
            InstanceMethod("updateLinkTableRelationship", &NativeDgnDb::UpdateLinkTableRelationship),
            InstanceMethod("updateModel", &NativeDgnDb::UpdateModel),
            InstanceMethod("updateProjectExtents", &NativeDgnDb::UpdateProjectExtents),
            StaticMethod("getAssetsDir", &NativeDgnDb::GetAssetDir),
        });

        exports.Set("DgnDb", t);

        SET_CONSTRUCTOR(t);
        }
};

//=======================================================================================
// Projects the Changed Elements ECDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeChangedElementsECDb : Napi::ObjectWrap<NativeChangedElementsECDb>
    {
    private:
        DEFINE_CONSTRUCTOR;
        std::unique_ptr<ECDb> m_ecdb;
        std::unique_ptr<ChangedElementsManager> m_manager;

        Napi::Object CreateBentleyReturnSuccessObject(Napi::Value goodVal) { return NapiUtils::CreateBentleyReturnSuccessObject(goodVal, Env()); }

        template <typename STATUSTYPE>
        Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg = nullptr) { return NapiUtils::CreateBentleyReturnErrorObject(errCode, msg, Env()); }

        template <typename STATUSTYPE>
        Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode, Napi::Value goodValue)
        {
            if ((STATUSTYPE)0 != errCode)
                return CreateBentleyReturnErrorObject(errCode);
            return CreateBentleyReturnSuccessObject(goodValue);
        }

      public:
        NativeChangedElementsECDb(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeChangedElementsECDb>(info) {}
        ~NativeChangedElementsECDb() {}

        // Check if val is really a NativeECDb peer object
        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(Constructor().Value());
            }
        ECDbR GetECDb()
            {
            if (m_ecdb == nullptr)
                m_ecdb = std::make_unique<ECDb>();

            return *m_ecdb;
            }

        Napi::Value CreateDb(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, mainDb, Env().Undefined());
            REQUIRE_ARGUMENT_STRING(1, dbName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            m_manager = std::make_unique<ChangedElementsManager>(&mainDb->GetDgnDb());
            BeFileName file(dbName);
            BeFileName path = file.GetDirectoryName();
            if (!path.DoesPathExist())
                return Napi::Number::New(Env(), (int) BE_SQLITE_NOTFOUND);

            DbResult status = m_manager->CreateChangedElementsCache(GetECDb(), file);
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value ProcessChangesets(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, mainDb, Env().Undefined());
            REQUIRE_ARGUMENT_STRING(1, changeSetTokens, Env().Undefined());
            REQUIRE_ARGUMENT_STRING(2, rulesetId, Env().Undefined());
            REQUIRE_ARGUMENT_BOOL(3, filterSpatial, Env().Undefined());

            if (GetECDb().IsReadonly())
                return Napi::Number::New(Env(), (int) BE_SQLITE_READONLY);

            if (!m_manager)
                m_manager = std::make_unique<ChangedElementsManager>(&mainDb->GetDgnDb());

            bvector<DgnRevisionPtr> revisionPtrs;
            bool containsSchemaChanges;
            Utf8String dbGuid = mainDb->GetDgnDb().GetDbGuid().ToString();
            Json::Value jsonChangeSetTokens = Json::Value::From(changeSetTokens);
            RevisionStatus status = JsInterop::ReadChangeSets(revisionPtrs, containsSchemaChanges, dbGuid, jsonChangeSetTokens);
            if (RevisionStatus::Success != status)
                return Napi::Number::New(Env(), (int)status);

            m_manager->SetFilterSpatial(filterSpatial);
            DbResult result = m_manager->ProcessChangesets(GetECDb(), Utf8String(rulesetId), revisionPtrs);
            return Napi::Number::New(Env(), (int) result);
            }

        Napi::Value IsProcessed(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, changesetId, Env().Undefined());
            Json::Value response;
            bool isProcessed = m_manager->IsProcessed(GetECDb(), changesetId);
            return Napi::Boolean::New(Env(), isProcessed);
            }

        Napi::Value GetChangedElements(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, startChangesetId, Env().Undefined());
            REQUIRE_ARGUMENT_STRING(1, endChangesetId, Env().Undefined());

            ChangedElementsMap map;
            DbResult status = m_manager->GetChangedElements(GetECDb(), map, startChangesetId, endChangesetId);

            Json::Value data;
            ChangedElementsManager::ChangedElementsToJSON(data, map);

            Napi::Value jsValue = NapiUtils::Convert(Env(), data);
            return CreateBentleyReturnObject(status, jsValue);
            }

        Napi::Value IsOpen(Napi::CallbackInfo const& info) { return Napi::Boolean::New(Env(), GetECDb().IsDbOpen()); }

        Napi::Value OpenDb(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, dbName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            REQUIRE_ARGUMENT_INTEGER(1, mode, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            Db::OpenParams params((Db::OpenMode) mode);
            DbResult status = JsInterop::OpenECDb(GetECDb(), BeFileName(dbName.c_str(), true), params);
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value CloseDb(Napi::CallbackInfo const& info)
            {
            if (m_ecdb != nullptr)
                {
                m_ecdb->CloseDb();
                m_ecdb = nullptr;
                }

            return Napi::Number::New(Env(), (int) BE_SQLITE_OK);
            }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ChangedElementsECDb", {
            InstanceMethod("createDb", &NativeChangedElementsECDb::CreateDb),
            InstanceMethod("processChangesets", &NativeChangedElementsECDb::ProcessChangesets),
            InstanceMethod("getChangedElements", &NativeChangedElementsECDb::GetChangedElements),
            InstanceMethod("openDb", &NativeChangedElementsECDb::OpenDb),
            InstanceMethod("closeDb", &NativeChangedElementsECDb::CloseDb),
            InstanceMethod("isOpen", &NativeChangedElementsECDb::IsOpen),
            InstanceMethod("isProcessed", &NativeChangedElementsECDb::IsProcessed)
            });

            exports.Set("ChangedElementsECDb", t);

            SET_CONSTRUCTOR(t);
            }
    };

//=======================================================================================
// Projects the IECSqlBinder interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlBinder : Napi::ObjectWrap<NativeECSqlBinder>
    {
private:
    DEFINE_CONSTRUCTOR;
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

public:
    NativeECSqlBinder(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlBinder>(info)
        {
        if (info.Length() != 2)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder constructor expects two arguments.", );

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
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECSqlBinder", {
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

        exports.Set("ECSqlBinder", t);

        SET_CONSTRUCTOR(t);
        }

    static Napi::Object New(Napi::Env const& env, IECSqlBinder& binder, ECDbCR ecdb)
        {
        return Constructor().New({Napi::External<IECSqlBinder>::New(env, &binder), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
        }

    Napi::Value BindNull(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        ECSqlStatus stat = m_binder->BindNull();
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBlob(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        if (info.Length() == 0)
            THROW_TYPE_EXCEPTION_AND_RETURN("BindBlob requires an argument", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        Napi::Value blobVal = info[0];
        if (blobVal.IsTypedArray())
            {
            Napi::TypedArray typedArray = blobVal.As<Napi::TypedArray>();
            if (typedArray.TypedArrayType() == napi_uint8_array)
                {
                Napi::Uint8Array uint8Array = typedArray.As<Napi::Uint8Array>();
                ECSqlStatus stat = m_binder->BindBlob(uint8Array.Data(), (int) uint8Array.ByteLength(), IECSqlBinder::MakeCopy::Yes);
                return Napi::Number::New(Env(), (int) ToDbResult(stat));
                }
            }

        if (blobVal.IsArrayBuffer())
            {
            Napi::ArrayBuffer buf = blobVal.As<Napi::ArrayBuffer>();
            ECSqlStatus stat = m_binder->BindBlob(buf.Data(), (int) buf.ByteLength(), IECSqlBinder::MakeCopy::Yes);
            return Napi::Number::New(Env(), (int) ToDbResult(stat));
            }

        if (!blobVal.IsString())
            THROW_TYPE_EXCEPTION_AND_RETURN("BindBlob requires either a Uint8Buffer, ArrayBuffer or a base64-encoded string.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        Utf8String base64Str(blobVal.ToString().Utf8Value().c_str());
        ByteStream blob;
        Base64Utilities::Decode(blob, base64Str);

        ECSqlStatus stat = m_binder->BindBlob(blob.data(), (int) blob.size(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindBoolean(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        Napi::Value boolVal;
        if (info.Length() == 0 || !(boolVal = info[0]).IsBoolean())
            THROW_TYPE_EXCEPTION_AND_RETURN("BindBoolean expects a boolean", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        ECSqlStatus stat = m_binder->BindBoolean(boolVal.ToBoolean());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDateTime(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_STRING(0, isoString, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        DateTime dt;
        if (SUCCESS != DateTime::FromString(dt, isoString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = m_binder->BindDateTime(dt);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindDouble(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_NUMBER(0, val, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        ECSqlStatus stat = m_binder->BindDouble(val.DoubleValue());
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindGuid(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_STRING(0, guidString, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        BeGuid guid;
        if (SUCCESS != guid.FromString(guidString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = m_binder->BindGuid(guid, IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindId(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_STRING(0, hexString, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        BeInt64Id id;
        if (SUCCESS != BeInt64Id::FromString(id, hexString.c_str()))
            return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

        ECSqlStatus stat = m_binder->BindId(id);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindInteger(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        if (info.Length() == 0)
            THROW_TYPE_EXCEPTION_AND_RETURN("BindInteger expects a string or number", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        Napi::Value val = info[0];
        if (!val.IsNumber() && !val.IsString())
            THROW_TYPE_EXCEPTION_AND_RETURN("BindInteger expects a string or number", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        int64_t int64Val;
        if (val.IsNumber())
            int64Val = val.ToNumber().Int64Value();
        else
            {
            Utf8String strVal(val.ToString().Utf8Value().c_str());
            if (strVal.empty())
                THROW_TYPE_EXCEPTION_AND_RETURN("Integral string passed to BindInteger must not be empty.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            bool const isNegativeNumber = strVal[0] == '-';
            Utf8CP positiveNumberStr = isNegativeNumber ? strVal.c_str() + 1 : strVal.c_str();
            uint64_t uVal = 0;
            if (SUCCESS != BeStringUtilities::ParseUInt64(uVal, positiveNumberStr)) //also supports hex strings
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Could not parse string %s to a valid integer.", strVal.c_str());
                THROW_TYPE_EXCEPTION_AND_RETURN(error.c_str(), Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
                }

            if (isNegativeNumber && uVal > (uint64_t) std::numeric_limits<int64_t>::max())
                {
                Utf8String error;
                error.Sprintf("BindInteger failed. Number in string %s is too large to fit into a signed 64 bit integer value.", strVal.c_str());
                THROW_TYPE_EXCEPTION_AND_RETURN(error.c_str(), Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
                }

            int64Val = uVal;
            if (isNegativeNumber)
                int64Val *= -1;
            }

        ECSqlStatus stat = m_binder->BindInt64(int64Val);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint2d(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_NUMBER(0, x, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_NUMBER(1, y, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        ECSqlStatus stat = m_binder->BindPoint2d(DPoint2d::From(x.DoubleValue(),y.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindPoint3d(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_NUMBER(0, x, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_NUMBER(1, y, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_NUMBER(2, z, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        ECSqlStatus stat = m_binder->BindPoint3d(DPoint3d::From(x.DoubleValue(), y.DoubleValue(), z.DoubleValue()));
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindString(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_STRING(0, val, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        ECSqlStatus stat = m_binder->BindText(val.c_str(), IECSqlBinder::MakeCopy::Yes);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindNavigation(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_STRING(0, navIdHexStr, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        OPTIONAL_ARGUMENT_STRING(1, relClassName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        OPTIONAL_ARGUMENT_STRING(2, relClassTableSpaceName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

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

        ECSqlStatus stat = m_binder->BindNavigation(navId, relClassId);
        return Napi::Number::New(Env(), (int) ToDbResult(stat));
        }

    Napi::Value BindMember(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        REQUIRE_ARGUMENT_STRING(0, memberName, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        IECSqlBinder& memberBinder = m_binder->operator[](memberName.c_str());
        return New(info.Env(), memberBinder, *m_ecdb);
        }

    Napi::Value AddArrayElement(Napi::CallbackInfo const& info)
        {
        if (m_binder == nullptr || m_ecdb == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlBinder is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        IECSqlBinder& elementBinder = m_binder->AddArrayElement();
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

        DEFINE_CONSTRUCTOR;
        ECSqlColumnInfo const* m_colInfo = nullptr;

    public:
        NativeECSqlColumnInfo(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlColumnInfo>(info)
            {
            if (info.Length() != 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo constructor expects one argument.",);

            m_colInfo = info[0].As<Napi::External<ECSqlColumnInfo>>().Data();
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid first arg for NativeECSqlColumnInfo constructor. ECSqlColumnInfo must not be nullptr",);
            }

        ~NativeECSqlColumnInfo() {}

        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(Constructor().Value());
            }

        //  Create projections
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ECSqlColumnInfo", {
            InstanceMethod("getType", &NativeECSqlColumnInfo::GetType),
            InstanceMethod("getPropertyName", &NativeECSqlColumnInfo::GetPropertyName),
            InstanceMethod("getAccessString", &NativeECSqlColumnInfo::GetAccessString),
            InstanceMethod("isSystemProperty", &NativeECSqlColumnInfo::IsSystemProperty),
            InstanceMethod("isGeneratedProperty", &NativeECSqlColumnInfo::IsGeneratedProperty),
            InstanceMethod("isEnum", &NativeECSqlColumnInfo::IsEnum),
            InstanceMethod("getRootClassTableSpace", &NativeECSqlColumnInfo::GetRootClassTableSpace),
            InstanceMethod("getRootClassName", &NativeECSqlColumnInfo::GetRootClassName),
            InstanceMethod("getRootClassAlias", &NativeECSqlColumnInfo::GetRootClassAlias)});

            exports.Set("ECSqlColumnInfo", t);

            SET_CONSTRUCTOR(t);
            }

        static Napi::Object New(Napi::Env const& env, ECSqlColumnInfo const& colInfo)
            {
            return Constructor().New({Napi::External<ECSqlColumnInfo>::New(env, const_cast<ECSqlColumnInfo*>(&colInfo))});
            }

        Napi::Value GetType(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            ECTypeDescriptor const& dataType = m_colInfo->GetDataType();
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
                        ECPropertyCP prop = m_colInfo->GetProperty();
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
                        if (m_colInfo->IsSystemProperty())
                            {
                            type = Type::Id;
                            break;
                            }

                        ECPropertyCP prop = m_colInfo->GetProperty();
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
                            THROW_TYPE_EXCEPTION_AND_RETURN("Unsupported ECSqlValue primitive type.", Env().Undefined());
                            break;
                    }
                }

            return Napi::Number::New(Env(), (int) type);
            }

        Napi::Value GetPropertyName(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            ECPropertyCP prop = m_colInfo->GetProperty();
            if (prop == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo does not represent a property.", Env().Undefined());

            return toJsString(Env(), prop->GetName());
            }

        Napi::Value GetAccessString(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            //if property is generated, the display label contains the select clause item as is.
            //The property name in contrast would have encoded special characters of the select clause item.
            //Ex: SELECT MyProp + 4 FROM Foo -> the name must be "MyProp + 4"
            if (m_colInfo->IsGeneratedProperty())
                {
                BeAssert(m_colInfo->GetPropertyPath().Size() == 1);
                ECPropertyCP prop = m_colInfo->GetProperty();
                if (prop == nullptr)
                    THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo's Property must not be null for a generated property.", Env().Undefined());

                return toJsString(Env(), prop->GetDisplayLabel());
                }

            return toJsString(Env(), m_colInfo->GetPropertyPath().ToString());
            }

        Napi::Value IsEnum(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            return Napi::Boolean::New(Env(), m_colInfo->GetEnumType() != nullptr);
            }

        Napi::Value IsSystemProperty(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            return Napi::Boolean::New(Env(), m_colInfo->IsSystemProperty());
            }

        Napi::Value IsGeneratedProperty(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            return Napi::Boolean::New(Env(), m_colInfo->IsGeneratedProperty());
            }

        Napi::Value GetRootClassTableSpace(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            return toJsString(Env(), m_colInfo->GetRootClass().GetTableSpace());
            }

        Napi::Value GetRootClassName(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            return toJsString(Env(), ECJsonUtilities::FormatClassName(m_colInfo->GetRootClass().GetClass()));
            }

        Napi::Value GetRootClassAlias(Napi::CallbackInfo const& info)
            {
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo is not initialized.", Env().Undefined());

            return toJsString(Env(), m_colInfo->GetRootClass().GetAlias());
            }
    };

//=======================================================================================
// Projects the IECSqlValue interface into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlValue : Napi::ObjectWrap<NativeECSqlValue>
    {
private:
    DEFINE_CONSTRUCTOR;
    IECSqlValue const* m_ecsqlValue = nullptr;
    ECDb const* m_ecdb = nullptr;

public:
    NativeECSqlValue(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlValue>(info)
        {
        if (info.Length() < 2)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue constructor expects two arguments.",);

        m_ecsqlValue = info[0].As<Napi::External<IECSqlValue>>().Data();
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid first arg for NativeECSqlValue constructor. IECSqlValue must not be nullptr",);

        m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
        if (m_ecdb == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid second arg for NativeECSqlValue constructor. ECDb must not be nullptr", );
        }

    ~NativeECSqlValue() {}

    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECSqlValue", {
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
            InstanceMethod("getEnum", &NativeECSqlValue::GetEnum),
            InstanceMethod("getStructIterator", &NativeECSqlValue::GetStructIterator),
            InstanceMethod("isNull", &NativeECSqlValue::IsNull),
        });

        exports.Set("ECSqlValue", t);

        SET_CONSTRUCTOR(t);
        }

    static Napi::Object New(Napi::Env const& env, IECSqlValue const& val, ECDbCR ecdb)
        {
        return Constructor().New({Napi::External<IECSqlValue>::New(env, const_cast<IECSqlValue*>(&val)), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
        }

    Napi::Value GetColumnInfo(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        return NativeECSqlColumnInfo::New(Env(), m_ecsqlValue->GetColumnInfo());
        }

    Napi::Value IsNull(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        return Napi::Boolean::New(Env(), m_ecsqlValue->IsNull());
        }

    Napi::Value GetBlob(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        int blobSize;
        void const* data = m_ecsqlValue->GetBlob(&blobSize);
        auto blob = Napi::Uint8Array::New(Env(), blobSize);
        memcpy(blob.Data(), data, blobSize);
        return blob;
        }

    Napi::Value GetBoolean(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        return Napi::Boolean::New(Env(), m_ecsqlValue->GetBoolean());
        }

    Napi::Value GetDateTime(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        DateTime dt = m_ecsqlValue->GetDateTime();
        return toJsString(Env(), dt.ToString());
        }

    Napi::Value GetDouble(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        return Napi::Number::New(Env(), m_ecsqlValue->GetDouble());
        }

    Napi::Value GetGeometry(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        IGeometryPtr geom = m_ecsqlValue->GetGeometry();
        Json::Value json;
        if (SUCCESS != ECJsonUtilities::IGeometryToJson(json, *geom))
            THROW_TYPE_EXCEPTION_AND_RETURN("Could not convert IGeometry to JSON.", Env().Undefined());

        return toJsString(Env(), json.ToString());
        }

    Napi::Value GetGuid(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        BeGuid guid = m_ecsqlValue->GetGuid();
        if (!guid.IsValid())
            return Env().Undefined();

        return toJsString(Env(), guid.ToString());
        }

    Napi::Value GetId(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        BeInt64Id id = m_ecsqlValue->GetId<BeInt64Id>();
        if (!id.IsValid())
            return Env().Undefined();

        return toJsString(Env(), id);
        }

    Napi::Value GetClassNameForClassId(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        ECClassId classId = m_ecsqlValue->GetId<ECClassId>();
        if (!classId.IsValid())
            THROW_TYPE_EXCEPTION_AND_RETURN("Failed to get class name from ECSqlValue: The ECSqlValue does not refer to a valid class id.", Env().Undefined());

        Utf8StringCR tableSpace = m_ecsqlValue->GetColumnInfo().GetRootClass().GetTableSpace();
        ECClassCP ecClass = m_ecdb->Schemas().GetClass(classId, tableSpace.c_str());
        if (ecClass == nullptr)
            {
            Utf8String err;
            err.Sprintf("Failed to get class name from ECSqlValue: Class not found for ECClassId %s.", classId.ToHexStr().c_str());
            THROW_TYPE_EXCEPTION_AND_RETURN(err.c_str(), Env().Undefined());
            }

        return toJsString(Env(), ECJsonUtilities::FormatClassName(*ecClass));
        }

    Napi::Value GetInt(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        return Napi::Number::New(Env(), m_ecsqlValue->GetInt());
        }

    Napi::Value GetInt64(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        return Napi::Number::New(Env(), m_ecsqlValue->GetInt64());
        }

    Napi::Value GetPoint2d(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        DPoint2d pt = m_ecsqlValue->GetPoint2d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        return jsPt;
        }

    Napi::Value GetPoint3d(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        DPoint3d pt = m_ecsqlValue->GetPoint3d();
        Napi::Object jsPt = Napi::Object::New(Env());
        jsPt.Set(ECN::ECJsonSystemNames::Point::X(), Napi::Number::New(Env(), pt.x));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Y(), Napi::Number::New(Env(), pt.y));
        jsPt.Set(ECN::ECJsonSystemNames::Point::Z(), Napi::Number::New(Env(), pt.z));
        return jsPt;
        }

    Napi::Value GetString(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        return toJsString(Env(), m_ecsqlValue->GetText());
        }

    Napi::Value GetEnum(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        ECEnumerationCP enumType = m_ecsqlValue->GetColumnInfo().GetEnumType();
        if (enumType == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not an ECEnumeration.", Env().Undefined());

        bvector<ECEnumeratorCP> enumerators;
        if (SUCCESS != m_ecsqlValue->TryGetContainedEnumerators(enumerators) || enumerators.empty())
            return Env().Undefined();

        Napi::Array jsEnumList = Napi::Array::New(Env(), enumerators.size());

        uint32_t arrayIndex = 0;
        for (ECEnumeratorCP enumerator : enumerators)
            {
            Napi::Object jsEnum = Napi::Object::New(Env());
            jsEnum.Set("schema", Napi::String::New(Env(), enumType->GetSchema().GetName().c_str()));
            jsEnum.Set("name", Napi::String::New(Env(), enumType->GetName().c_str()));
            jsEnum.Set("key", Napi::String::New(Env(), enumerator->GetName().c_str()));
            jsEnum.Set("value", enumerator->IsInteger() ? Napi::Number::New(Env(), enumerator->GetInteger()) : Napi::String::New(Env(), enumerator->GetString().c_str()));
            jsEnumList.Set(arrayIndex, jsEnum);
            arrayIndex++;
            }

        return jsEnumList;
        }

    Napi::Value GetNavigation(Napi::CallbackInfo const& info)
        {
        if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

        ECClassId relClassId;
        BeInt64Id navId = m_ecsqlValue->GetNavigation(&relClassId);

        Napi::Object jsNavValue = Napi::Object::New(Env());
        jsNavValue.Set(ECN::ECJsonSystemNames::Navigation::Id(), Napi::String::New(Env(), navId.ToHexStr().c_str()));
        if (relClassId.IsValid())
            {
            Utf8StringCR relClassTableSpace = m_ecsqlValue->GetColumnInfo().GetRootClass().GetTableSpace();
            ECClassCP relClass = m_ecdb->Schemas().GetClass(relClassId, relClassTableSpace.c_str());
            if (relClass == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("Failed to find ECRelationhipClass for the Navigation Value's RelECClassId.", Env().Undefined());

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
        DEFINE_CONSTRUCTOR;
        ECDb const* m_ecdb = nullptr;
        IECSqlValueIterable const* m_iterable = nullptr;
        bool m_isBeforeFirstElement = true;
        IECSqlValueIterable::const_iterator m_it;
        IECSqlValueIterable::const_iterator m_endIt;

    public:
        NativeECSqlValueIterator(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlValueIterator>(info)
            {
            if (info.Length() < 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValueIterator constructor expects two argument.", );

            m_iterable = info[0].As<Napi::External<IECSqlValueIterable>>().Data();
            if (m_iterable == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid first arg for NativeECSqlValueIterator constructor. IECSqlValueIterable must not be nullptr",);

            m_endIt = m_iterable->end();

            m_ecdb = info[1].As<Napi::External<ECDb>>().Data();
            if (m_ecdb == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid second arg for NativeECSqlValueIterator constructor. ECDb must not be nullptr",);
            }

        ~NativeECSqlValueIterator() {}

        static bool InstanceOf(Napi::Value val)
            {
            if (!val.IsObject())
                return false;

            Napi::HandleScope scope(val.Env());
            return val.As<Napi::Object>().InstanceOf(Constructor().Value());
            }

        //  Create projections
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "ECSqlValueIterator", {
            InstanceMethod("moveNext", &NativeECSqlValueIterator::MoveNext),
            InstanceMethod("getCurrent", &NativeECSqlValueIterator::GetCurrent)});

            exports.Set("ECSqlValueIterator", t);

            SET_CONSTRUCTOR(t);
            }

        static Napi::Object New(Napi::Env const& env, IECSqlValueIterable const& iterable, ECDb const& ecdb)
            {
            return Constructor().New({Napi::External<IECSqlValueIterable>::New(env, const_cast<IECSqlValueIterable*>(&iterable)), Napi::External<ECDb>::New(env, const_cast<ECDb*>(&ecdb))});
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
    if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
        THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

    return NativeECSqlValueIterator::New(info.Env(), m_ecsqlValue->GetStructIterable(), *m_ecdb);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               01/2018
//+---------------+---------------+---------------+---------------+---------------+------
Napi::Value NativeECSqlValue::GetArrayIterator(Napi::CallbackInfo const& info)
    {
    if (m_ecsqlValue == nullptr || m_ecdb == nullptr)
        THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlValue is not initialized", Env().Undefined());

    return NativeECSqlValueIterator::New(info.Env(), m_ecsqlValue->GetArrayIterable(), *m_ecdb);
    }

//=======================================================================================
// Projects the ECSqlStatement class into JS.
//! @bsiclass
//=======================================================================================
struct NativeECSqlStatement : Napi::ObjectWrap<NativeECSqlStatement>
{
private:
    DEFINE_CONSTRUCTOR;
    std::unique_ptr<ECSqlStatement> m_stmt;

    struct IssueListener : BeSQLite::EC::ECDb::IIssueListener
        {
        mutable Utf8String m_lastIssue;
        ECDbR m_db;
        bool m_active;

        void _OnIssueReported(BentleyApi::Utf8CP message) const override { m_lastIssue = message;}

        explicit IssueListener(ECDbR db) : m_active(SUCCESS == db.AddIssueListener(*this)), m_db(db) { }
        ~IssueListener() { if (m_active) m_db.RemoveIssueListener(); }
        };

public:
    NativeECSqlStatement(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeECSqlStatement>(info), m_stmt(new ECSqlStatement()) {}

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECSqlStatement", {
          InstanceMethod("prepare", &NativeECSqlStatement::Prepare),
          InstanceMethod("reset", &NativeECSqlStatement::Reset),
          InstanceMethod("dispose", &NativeECSqlStatement::Dispose),
          InstanceMethod("clearBindings", &NativeECSqlStatement::ClearBindings),
          InstanceMethod("getBinder", &NativeECSqlStatement::GetBinder),
          InstanceMethod("step", &NativeECSqlStatement::Step),
          InstanceMethod("stepForInsert", &NativeECSqlStatement::StepForInsert),
          InstanceMethod("stepAsync", &NativeECSqlStatement::StepAsync),
          InstanceMethod("stepForInsertAsync", &NativeECSqlStatement::StepForInsertAsync),
          InstanceMethod("getColumnCount", &NativeECSqlStatement::GetColumnCount),
          InstanceMethod("getValue", &NativeECSqlStatement::GetValue)
        });

        exports.Set("ECSqlStatement", t);

        SET_CONSTRUCTOR(t);
        }

    Napi::Value Prepare(Napi::CallbackInfo const& info)
        {
        if (info.Length() < 2)
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement::Prepare requires two arguments", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

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
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement::Prepare requires first argument to be a NativeDgnDb or NativeECDb object.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            }

        REQUIRE_ARGUMENT_STRING(1, ecsql, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        IssueListener listener(*ecdb);

        ECSqlStatus status = m_stmt->Prepare(*ecdb, ecsql.c_str());
        return NapiUtils::CreateErrorObject0(ToDbResult(status), !status.IsSuccess() ? listener.m_lastIssue.c_str() : nullptr, Env());
        }

    Napi::Value Reset(Napi::CallbackInfo const& info)
        {
        if (!m_stmt->IsPrepared())
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement is not prepared.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        ECSqlStatus status = m_stmt->Reset();
        return Napi::Number::New(Env(), (int) ToDbResult(status));
        }

    void Dispose(Napi::CallbackInfo const& info)
        {
        m_stmt = nullptr;
        }

    Napi::Value ClearBindings(Napi::CallbackInfo const& info)
        {
        if (!m_stmt->IsPrepared())
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement is not prepared.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        auto status = m_stmt->ClearBindings();
        return Napi::Number::New(Env(), (int) ToDbResult(status));
        }

    Napi::Value GetBinder(Napi::CallbackInfo const& info)
        {
        if (!m_stmt->IsPrepared())
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement is not prepared.", Env().Undefined());

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
        if (!m_stmt->IsPrepared())
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement is not prepared.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        DbResult status = m_stmt->Step();
        return Napi::Number::New(Env(), (int)status);
        }

    Napi::Value StepForInsert(Napi::CallbackInfo const& info)
        {
        if (!m_stmt->IsPrepared())
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement is not prepared.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

        ECInstanceKey key;
        DbResult status = m_stmt->Step(key);

        Napi::Object ret = Napi::Object::New(Env());
        ret.Set(Napi::String::New(Env(), "status"), Napi::Number::New(Env(), (int) status));
        if (BE_SQLITE_DONE == status)
            ret.Set(toJsString(Env(), "id"), toJsString(Env(), key.GetInstanceId()));

        return ret;
        }

    void StepAsync(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_FUNCTION(0, responseCallback, );
        JsInterop::StepAsync(responseCallback, *m_stmt, false);
        }

    void StepForInsertAsync(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_FUNCTION(0, responseCallback, );
        JsInterop::StepAsync(responseCallback, *m_stmt, true);
        }

    Napi::Value GetColumnCount(Napi::CallbackInfo const& info)
        {
        if (!m_stmt->IsPrepared())
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement is not prepared.", Env().Undefined());

        int colCount = m_stmt->GetColumnCount();
        return Napi::Number::New(info.Env(), colCount);
        }

    Napi::Value GetValue(Napi::CallbackInfo const& info)
        {
        if (!m_stmt->IsPrepared())
            THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlStatement is not prepared.", Env().Undefined());

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
// Projects the BeSQLite::Statement class into JS.
//! @bsiclass
//=======================================================================================
struct NativeSqliteStatement : Napi::ObjectWrap<NativeSqliteStatement>
    {
    private:
        DEFINE_CONSTRUCTOR;
        std::unique_ptr<Statement> m_stmt;

        int GetParameterIndex(Napi::Value const& paramIndexOrNameArg) const
            {
            if (m_stmt == nullptr || (!paramIndexOrNameArg.IsNumber() && !paramIndexOrNameArg.IsString()))
                return -1;

            if (paramIndexOrNameArg.IsNumber())
                return (int) paramIndexOrNameArg.ToNumber().Int32Value();

            return m_stmt->GetParameterIndex(paramIndexOrNameArg.ToString().Utf8Value().c_str());
            }

    public:
        NativeSqliteStatement(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeSqliteStatement>(info), m_stmt(new Statement()) {}

        //  Create projections
        static void Init(Napi::Env& env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "SqliteStatement", {
            InstanceMethod("dispose", &NativeSqliteStatement::Dispose),
            InstanceMethod("prepare", &NativeSqliteStatement::Prepare),
            InstanceMethod("isReadonly", &NativeSqliteStatement::IsReadonly),
            InstanceMethod("bindNull", &NativeSqliteStatement::BindNull),
            InstanceMethod("bindBlob", &NativeSqliteStatement::BindBlob),
            InstanceMethod("bindDouble", &NativeSqliteStatement::BindDouble),
            InstanceMethod("bindInteger", &NativeSqliteStatement::BindInteger),
            InstanceMethod("bindString", &NativeSqliteStatement::BindString),
            InstanceMethod("bindId", &NativeSqliteStatement::BindId),
            InstanceMethod("bindGuid", &NativeSqliteStatement::BindGuid),
            InstanceMethod("clearBindings", &NativeSqliteStatement::ClearBindings),
            InstanceMethod("step", &NativeSqliteStatement::Step),
            InstanceMethod("stepAsync", &NativeSqliteStatement::StepAsync),
            InstanceMethod("reset", &NativeSqliteStatement::Reset),
            InstanceMethod("getColumnCount", &NativeSqliteStatement::GetColumnCount),
            InstanceMethod("getColumnType", &NativeSqliteStatement::GetColumnType),
            InstanceMethod("getColumnName", &NativeSqliteStatement::GetColumnName),
            InstanceMethod("isValueNull", &NativeSqliteStatement::IsValueNull),
            InstanceMethod("getValueBlob", &NativeSqliteStatement::GetValueBlob),
            InstanceMethod("getValueDouble", &NativeSqliteStatement::GetValueDouble),
            InstanceMethod("getValueInteger", &NativeSqliteStatement::GetValueInteger),
            InstanceMethod("getValueString", &NativeSqliteStatement::GetValueString),
            InstanceMethod("getValueId", &NativeSqliteStatement::GetValueId),
            InstanceMethod("getValueGuid", &NativeSqliteStatement::GetValueGuid),
            });

            exports.Set("SqliteStatement", t);

            SET_CONSTRUCTOR(t);
            }

        void Dispose(Napi::CallbackInfo const& info)
            {
            if (m_stmt != nullptr)
                m_stmt = nullptr;
            }

        Napi::Value Prepare(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", NapiUtils::CreateErrorObject0((int) BE_SQLITE_ERROR, nullptr, Env()));

            if (info.Length() < 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement::Prepare requires two arguments", NapiUtils::CreateErrorObject0((int) BE_SQLITE_ERROR, nullptr, Env()));

            Napi::Object dbObj = info[0].As<Napi::Object>();

            Db* db = nullptr;
            if (NativeDgnDb::InstanceOf(dbObj))
                {
                NativeDgnDb* nativeDgndb = NativeDgnDb::Unwrap(dbObj);
                if (!nativeDgndb->IsOpen())
                    return NapiUtils::CreateErrorObject0(BE_SQLITE_NOTADB, nullptr, Env());

                db = &nativeDgndb->GetDgnDb();
                }
            else if (NativeECDb::InstanceOf(dbObj))
                {
                NativeECDb* nativeECDb = NativeECDb::Unwrap(dbObj);
                db = &nativeECDb->GetECDb();
                }
            else
                {
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement::Prepare requires first argument to be a NativeDgnDb or NativeECDb object.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
                }

            REQUIRE_ARGUMENT_STRING(1, sql, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            BeSqliteDbMutexHolder serializeAccess(*db); // hold mutex, so that we have a chance to get last Db error message

            const DbResult status = m_stmt->Prepare(*db, sql.c_str());
            return NapiUtils::CreateErrorObject0(status, status != BE_SQLITE_OK ? db->GetLastError().c_str() : nullptr, Env());
            }

        Napi::Value IsReadonly(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Env().Undefined());

            if (!m_stmt->IsPrepared())
                THROW_TYPE_EXCEPTION_AND_RETURN("Cannot call IsReadonly on unprepared statement.", Env().Undefined());

            return Napi::Boolean::New(Env(), m_stmt->IsReadonly());
            }

        Napi::Value BindNull(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            if (info.Length() != 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("BindNull requires an argument", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int paramIndex = GetParameterIndex(info[0]);
            if (paramIndex < 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid parameter index or name passed to BindNull", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            return Napi::Number::New(Env(), (int) m_stmt->BindNull(paramIndex));
            }

        Napi::Value BindBlob(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            if (info.Length() != 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("BindBlob requires two arguments", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int paramIndex = GetParameterIndex(info[0]);
            if (paramIndex < 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid parameter index or name passed to BindBlob", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            Napi::Value const& blobVal = info[1];
            if (blobVal.IsTypedArray())
                {
                Napi::TypedArray typedArray = blobVal.As<Napi::TypedArray>();
                if (typedArray.TypedArrayType() == napi_uint8_array)
                    {
                    Napi::Uint8Array uint8Array = typedArray.As<Napi::Uint8Array>();
                    const DbResult stat = m_stmt->BindBlob(paramIndex, uint8Array.Data(), (int) uint8Array.ByteLength(), Statement::MakeCopy::Yes);
                    return Napi::Number::New(Env(), (int) stat);
                    }
                }

            if (blobVal.IsArrayBuffer())
                {
                Napi::ArrayBuffer buf = blobVal.As<Napi::ArrayBuffer>();
                const DbResult stat = m_stmt->BindBlob(paramIndex, buf.Data(), (int) buf.ByteLength(), Statement::MakeCopy::Yes);
                return Napi::Number::New(Env(), (int) stat);
                }

            THROW_TYPE_EXCEPTION_AND_RETURN("BindBlob requires a Uint8Array or ArrayBuffer arg", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            }

        Napi::Value BindDouble(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            if (info.Length() != 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("BindDouble requires two arguments", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int paramIndex = GetParameterIndex(info[0]);
            if (paramIndex < 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid parameter index or name passed to BindDouble", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            REQUIRE_ARGUMENT_NUMBER(1, val, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            const DbResult stat = m_stmt->BindDouble(paramIndex, val.DoubleValue());
            return Napi::Number::New(Env(), (int) stat);
            }

        Napi::Value BindInteger(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            if (info.Length() != 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("BindInteger requires two arguments", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int paramIndex = GetParameterIndex(info[0]);
            if (paramIndex < 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid parameter index or name passed to BindInteger", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            Napi::Value const& val = info[1];
            if (!val.IsNumber() && !val.IsString())
                THROW_TYPE_EXCEPTION_AND_RETURN("BindInteger expects a string or number value.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int64_t int64Val;
            if (val.IsNumber())
                int64Val = val.ToNumber().Int64Value();
            else
                {
                Utf8String strVal(val.ToString().Utf8Value().c_str());
                if (strVal.empty())
                    THROW_TYPE_EXCEPTION_AND_RETURN("Integral string passed to BindInteger must not be empty.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

                bool const isNegativeNumber = strVal[0] == '-';
                Utf8CP positiveNumberStr = isNegativeNumber ? strVal.c_str() + 1 : strVal.c_str();
                uint64_t uVal = 0;
                if (SUCCESS != BeStringUtilities::ParseUInt64(uVal, positiveNumberStr)) //also supports hex strings
                    {
                    Utf8String error;
                    error.Sprintf("BindInteger failed. Could not parse string %s to a valid integer.", strVal.c_str());
                    THROW_TYPE_EXCEPTION_AND_RETURN(error.c_str(), Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
                    }

                if (isNegativeNumber && uVal > (uint64_t) std::numeric_limits<int64_t>::max())
                    {
                    Utf8String error;
                    error.Sprintf("BindInteger failed. Number in string %s is too large to fit into a signed 64 bit integer value.", strVal.c_str());
                    THROW_TYPE_EXCEPTION_AND_RETURN(error.c_str(), Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
                    }

                int64Val = uVal;
                if (isNegativeNumber)
                    int64Val *= -1;
                }

            const DbResult stat = m_stmt->BindInt64(paramIndex, int64Val);
            return Napi::Number::New(Env(), (int) stat);
            }

        Napi::Value BindString(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            if (info.Length() != 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("BindString requires two arguments", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int paramIndex = GetParameterIndex(info[0]);
            if (paramIndex < 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid parameter index or name passed to BindString", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            REQUIRE_ARGUMENT_STRING(1, val, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            const DbResult stat = m_stmt->BindText(paramIndex, val.c_str(), Statement::MakeCopy::Yes);
            return Napi::Number::New(Env(), (int) stat);
            }

        Napi::Value BindGuid(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            if (info.Length() != 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("BindGuid requires two arguments", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int paramIndex = GetParameterIndex(info[0]);
            if (paramIndex < 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid parameter index or name passed to BindGuid", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            REQUIRE_ARGUMENT_STRING(1, guidString, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            BeGuid guid;
            if (SUCCESS != guid.FromString(guidString.c_str()))
                return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

            const DbResult stat = m_stmt->BindGuid(paramIndex, guid);
            return Napi::Number::New(Env(), (int) stat);
            }

        Napi::Value BindId(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not initialized.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            if (info.Length() != 2)
                THROW_TYPE_EXCEPTION_AND_RETURN("BindId requires two arguments", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            int paramIndex = GetParameterIndex(info[0]);
            if (paramIndex < 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid parameter index or name passed to BindId", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            REQUIRE_ARGUMENT_STRING(1, idString, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            BeInt64Id id;
            if (SUCCESS != BeInt64Id::FromString(id, idString.c_str()))
                return Napi::Number::New(Env(), (int) BE_SQLITE_ERROR);

            const DbResult  stat = m_stmt->BindId(paramIndex, id);
            return Napi::Number::New(Env(), (int) stat);
            }

        Napi::Value ClearBindings(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            const DbResult status = m_stmt->ClearBindings();
            return Napi::Number::New(Env(), (int) status);
            }

        Napi::Value Step(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            const DbResult status = m_stmt->Step();
            return Napi::Number::New(Env(), (int) status);
            }

        void StepAsync(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_FUNCTION(0, responseCallback, );
            JsInterop::StepAsync(responseCallback, *m_stmt);
            }

        Napi::Value GetColumnCount(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            const int colCount = m_stmt->GetColumnCount();
            return Napi::Number::New(info.Env(), colCount);
            }

        Napi::Value GetColumnType(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());
            return Napi::Number::New(Env(), (int) m_stmt->GetColumnType(colIndex));
            }

        Napi::Value GetColumnName(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());
            return toJsString(Env(), m_stmt->GetColumnName(colIndex));
            }

        Napi::Value IsValueNull(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());
            return Napi::Boolean::New(Env(), m_stmt->IsColumnNull(colIndex));
            }

        Napi::Value GetValueBlob(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());

            void const* data = m_stmt->GetValueBlob(colIndex);
            int blobSize = m_stmt->GetColumnBytes(colIndex);
            Napi::Uint8Array blob = Napi::Uint8Array::New(Env(), blobSize);
            memcpy(blob.Data(), data, blobSize);
            return blob;
            }

        Napi::Value GetValueDouble(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());
            return Napi::Number::New(Env(), m_stmt->GetValueDouble(colIndex));
            }

        Napi::Value GetValueInteger(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());
            return Napi::Number::New(Env(), m_stmt->GetValueInt64(colIndex));
            }

        Napi::Value GetValueString(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());
            return toJsString(Env(), m_stmt->GetValueText(colIndex));
            }

        Napi::Value GetValueId(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());

            BeInt64Id id = m_stmt->GetValueId<BeInt64Id>(colIndex);
            if (!id.IsValid())
                return Env().Undefined();

            return toJsString(Env(), id);
            }

        Napi::Value GetValueGuid(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Env().Undefined());

            REQUIRE_ARGUMENT_INTEGER(0, colIndex, Env().Undefined());

            BeGuid guid = m_stmt->GetValueGuid(colIndex);
            if (!guid.IsValid())
                return Env().Undefined();

            return toJsString(Env(), guid.ToString());
            }

        Napi::Value Reset(Napi::CallbackInfo const& info)
            {
            if (m_stmt == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("SqliteStatement is not prepared.", Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));

            const DbResult status = m_stmt->Reset();
            return Napi::Number::New(Env(), (int) status);
            }
    };

//=======================================================================================
// @bsistruct                                                   Affan.Khan   01/19
//=======================================================================================
struct ECSqlStepWorker : Napi::AsyncWorker
{
private:
    ECSqlStatement& m_stmt;
    int m_status;
    ECInstanceKey m_instanceKey;
    bool m_stepForInsert;
    void Execute() override
        {
        if (m_stmt.IsPrepared())
            {
            if (m_stepForInsert)
                m_status = m_stmt.Step(m_instanceKey);
            else
                m_status = m_stmt.Step();
            }
        else
            {
            m_status = (int) BE_SQLITE_MISUSE;
            }
        }


    void OnOK() override
        {
        if (m_stepForInsert)
            {
            Napi::Object retval = Napi::Object::New(Env());
            retval.Set(Napi::String::New(Env(), "status"), Napi::Number::New(Env(), (int)m_status));
            if (BE_SQLITE_DONE == m_status)
                retval.Set(toJsString(Env(), "id"), toJsString(Env(), m_instanceKey.GetInstanceId()));

            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        else
            {
            Napi::Number retval = Napi::Number::New(Env(), (int)m_status);
            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        }
public:
    ECSqlStepWorker( Napi::Function& callback, ECSqlStatement& stmt, bool stepForInsert)
        : Napi::AsyncWorker(callback), m_stmt(stmt), m_stepForInsert(stepForInsert)
        {
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan     01/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::StepAsync(Napi::Function& callback, ECSqlStatement& stmt, bool stepForInsert)
    {
    auto worker = new ECSqlStepWorker(callback, stmt, stepForInsert);
    worker->Queue();
    }

//=======================================================================================
// @bsistruct                                                   Affan.Khan   01/19
//=======================================================================================
struct SqliteStepWorker : Napi::AsyncWorker
{
private:
    Statement& m_stmt;
    int m_status;
    void Execute() override
        {
        if (m_stmt.IsPrepared())
            m_status = m_stmt.Step();
        else
            m_status = (int) BE_SQLITE_MISUSE;
        }

    void OnOK() override
        {
        auto retval = Napi::Number::New(Env(), (int)m_status);
        Callback().MakeCallback(Receiver().Value(), {retval});
        }
public:
    SqliteStepWorker( Napi::Function& callback, Statement& stmt)
        : Napi::AsyncWorker(callback), m_stmt(stmt)
        {
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan     01/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::StepAsync(Napi::Function& callback, Statement& stmt)
    {
    auto worker = new SqliteStepWorker(callback, stmt);
    worker->Queue();
    }

//=======================================================================================
// A request to generate a snap point given an element and additional parameters.
//! @bsiclass
//=======================================================================================
struct SnapRequest : Napi::ObjectWrap<SnapRequest>
{
    //=======================================================================================
    // Async Worker that does a snap on another thread.
    //! @bsiclass
    //=======================================================================================
    struct Snapper : Napi::AsyncWorker
    {
        DgnDbPtr m_db;
        SnapContext::Request m_input;
        SnapContext::Response m_output;
        CheckStop m_checkStop;
        Napi::ObjectReference m_snapRequest;

        void OnComplete()
            {
            auto* request = SnapRequest::Unwrap(m_snapRequest.Value());
            BeMutexHolder holder(request->m_mutex);
            request->m_pending = nullptr;
            }

        // This is invoked by node/uv in the BACKGROUND THREAD. DO NOT CALL N-API OR JS METHODS IN HERE.
        void Execute() override
            {
            m_output = SnapContext::DoSnap(m_input, *m_db, m_checkStop);
            if (m_checkStop.WasAborted())
                SetError("aborted");  // You MUST call SetError with a string. That tells N-API to invoke OnError
            }

        // This is invoked by node/uv in the main JS thread when the operation is completed successfully
        void OnOK() override
            {
            OnComplete();
            auto retval = NapiUtils::CreateBentleyReturnSuccessObject(NapiUtils::Convert(Env(), m_output), Env());
            Callback().MakeCallback(Receiver().Value(), {retval});
            }

        // This is invoked by node/uv in the main JS thread when the operation is completed with an error
        void OnError(Napi::Error const& e) override
            {
            OnComplete();
            auto retval = NapiUtils::CreateBentleyReturnErrorObject(1, e.Message().c_str(), Env());
            Callback().MakeCallback(Receiver().Value(), {retval});
            }

        Snapper(Napi::Function& callback, SnapRequest const& request, DgnDbR db, SnapContext::Request const& input) : Napi::AsyncWorker(callback), m_db(&db), m_input(input) {m_snapRequest.Reset(request.Value(), 1);}
        ~Snapper() {m_snapRequest.Reset();}
    };

    DEFINE_CONSTRUCTOR;
    mutable BeMutex m_mutex;
    Snapper* m_pending = nullptr;

    void DoSnap(Napi::CallbackInfo const& info)
        {
        BeMutexHolder holder(m_mutex);
        if (m_pending != nullptr)
            CancelSnap(info);

        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db, );
        REQUIRE_ARGUMENT_ANY_OBJ(1, snapObj, );
        REQUIRE_ARGUMENT_FUNCTION(2, callback, );

        Json::Value request(NapiUtils::Convert(snapObj));
        m_pending = new Snapper(callback, *this, db->GetDgnDb(), (SnapContext::Request const&) request); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        m_pending->Queue();  // Snap happens in another thread
        }

    // Cancel a previous request for a snap. If no snap is pending, does nothing
    void CancelSnap(Napi::CallbackInfo const& info)
        {
        BeMutexHolder holder(m_mutex);
        if (nullptr != m_pending)
            {
            m_pending->m_checkStop.SetAborted();
            m_pending = nullptr;
            }
        }

    SnapRequest(Napi::CallbackInfo const& info) : Napi::ObjectWrap<SnapRequest>(info) {}

    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "SnapRequest", {
          InstanceMethod("doSnap", &SnapRequest::DoSnap),
          InstanceMethod("cancelSnap", &SnapRequest::CancelSnap)
        });

        exports.Set("SnapRequest", t);

        SET_CONSTRUCTOR(t);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct TileWorker : Napi::AsyncWorker
{
protected:
    // Inputs
    GeometricModelPtr m_model;
    Tile::Tree::Id m_treeId;
    ICancellationTokenPtr m_cancellationToken;

    // Outputs
    DgnDbStatus m_status;

    TileWorker(ICancellationTokenPtr cancellationToken, Napi::Function& callback, GeometricModelR model, Tile::Tree::Id treeId)
        : Napi::AsyncWorker(callback), m_model(&model), m_treeId(treeId), m_cancellationToken(cancellationToken), m_status(DgnDbStatus::BadRequest)
        {
        BeAssert(nullptr != m_cancellationToken);
        }

    void OnError(Napi::Error const& e) override
        {
        auto retval = NapiUtils::CreateBentleyReturnErrorObject(1, e.Message().c_str(), Env());
        Callback().MakeCallback(Receiver().Value(), {retval});
        }

    void OnOK() override
        {
        if (DgnDbStatus::Success == m_status)
            {
            Napi::Value jsValue = GetResult();
            auto retval = NapiUtils::CreateBentleyReturnSuccessObject(jsValue, Env());
            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        else
            {
            auto retval = NapiUtils::CreateBentleyReturnErrorObject(m_status, nullptr, Env());
            Callback().MakeCallback(Receiver().Value(), {retval});
            }
        }

    virtual Napi::Value GetResult() = 0;

    Tile::TreePtr GetTileTree() { return JsInterop::GetTileTree(*m_model, m_treeId, true); }
    bool IsCanceled() const { return nullptr != m_cancellationToken && m_cancellationToken->IsCanceled(); }

    bool PreExecute()
        {
        if (!IsCanceled())
            return true;

        m_status = DgnDbStatus::NotOpen;
        return false;
        }
public:
    static DgnDbStatus ParseInputs(GeometricModelPtr& model, Tile::Tree::Id& treeId, DgnDbR db, Utf8StringCR idStr)
        {
        if (!db.IsDbOpen())
            return DgnDbStatus::NotOpen;

        treeId = Tile::Tree::Id::FromString(idStr);
        if (!treeId.IsValid())
            return DgnDbStatus::InvalidId;

        model = db.Models().Get<GeometricModel>(treeId.m_modelId);
        return model.IsValid() ? DgnDbStatus::Success : DgnDbStatus::MissingId;
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct GetTileTreeWorker : TileWorker
{
private:
    // Output
    Json::Value m_result;

    void Execute() final
        {
        if (!PreExecute())
            return;

        // JsInterop::GetLogger().debugv("GetTileTreeWorker::Execute: %s", m_treeId.ToString().c_str());
        auto tree = GetTileTree();
        if (tree.IsNull())
            {
            m_status = DgnDbStatus::NotFound;
            return;
            }

        m_status = DgnDbStatus::Success;
        m_result = tree->ToJson();
        }

    Napi::Value GetResult() final
        {
        return NapiUtils::Convert(Env(), m_result);
        }
public:
    GetTileTreeWorker(ICancellationTokenPtr cancel, Napi::Function& callback, GeometricModelR model, Tile::Tree::Id treeId) : TileWorker(cancel, callback, model, treeId)
        {
        // JsInterop::GetLogger().debugv("GetTileTreeWorker ctor: %s", m_treeId.ToString().c_str());
        }
    ~GetTileTreeWorker()
        {
        // JsInterop::GetLogger().debugv("GetTileTreeWorker dtor: %s", m_treeId.ToString().c_str());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::GetTileTree(ICancellationTokenPtr cancel, DgnDbR db, Utf8StringCR idStr, Napi::Function& callback)
    {
    GeometricModelPtr model;
    Tile::Tree::Id treeId;
    DgnDbStatus status = TileWorker::ParseInputs(model, treeId, db, idStr);
    if (DgnDbStatus::Success != status)
        {
        auto retval = NapiUtils::CreateBentleyReturnErrorObject(status, nullptr, Env());
        callback.Call({retval});
        }
    else
        {
        // If the tree already exists, return it from the main thread rather than sending to thread pool...
        auto tree = JsInterop::GetTileTree(*model, treeId, false);
        if (tree.IsValid())
            {
            Json::Value json = tree->ToJson();
            Napi::Value result = NapiUtils::Convert(Env(), json);
            auto retval = NapiUtils::CreateBentleyReturnSuccessObject(result, Env());
            callback.Call({retval});
            }
        else
            {
            // Initialize tree on thread pool.
            auto worker = new GetTileTreeWorker(cancel, callback, *model, treeId);
            worker->Queue();
            }
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/18
//=======================================================================================
struct GetTileContentWorker : TileWorker
{
private:
    // Input
    Tile::ContentId m_contentId;

    // Output
    Tile::ContentCPtr m_result;

    void Execute() final
        {
        if (!PreExecute())
            return;

        // JsInterop::GetLogger().debugv("GetTileContentWorker::Execute: %s %s", m_treeId.ToString().c_str(), m_contentId.ToString().c_str());
        auto tree = GetTileTree();
        m_result = tree.IsValid() ? tree->RequestContent(m_contentId) : nullptr;
        m_status = m_result.IsValid() ? DgnDbStatus::Success : DgnDbStatus::NotFound;
        }

    Napi::Value GetResult() final
        {
        BeAssert(m_result.IsValid());

        ByteStreamCR geometry = m_result->GetBytes();
        auto blob = Napi::Uint8Array::New(Env(), geometry.size());
        memcpy(blob.Data(), geometry.data(), geometry.size());

        return blob;
        }
public:
    GetTileContentWorker(ICancellationTokenPtr cancel, Napi::Function& callback, GeometricModelR model, Tile::Tree::Id treeId, Tile::ContentId contentId)
        : TileWorker(cancel, callback, model, treeId), m_contentId(contentId)
        {
        // JsInterop::GetLogger().debugv("GetTileContentWorker ctor: %s %s", m_treeId.ToString().c_str(), m_contentId.ToString().c_str());
        }

    ~GetTileContentWorker()
        {
        // JsInterop::GetLogger().debugv("GetTileContentWorker dtor: %s %s", m_treeId.ToString().c_str(), m_contentId.ToString().c_str());
        }

    static DgnDbStatus ParseInputs(GeometricModelPtr& model, Tile::Tree::Id& treeId, Tile::ContentIdR contentId, DgnDbR db, Utf8StringCR treeIdStr, Utf8StringCR contentIdStr)
        {
        auto status = TileWorker::ParseInputs(model, treeId, db, treeIdStr);
        if (DgnDbStatus::Success == status && !contentId.FromString(contentIdStr.c_str()))
            status = DgnDbStatus::InvalidId;

        return status;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::GetTileContent(ICancellationTokenPtr cancel, DgnDbR db, Utf8StringCR treeIdStr, Utf8StringCR tileIdStr, Napi::Function& callback)
    {
    GeometricModelPtr model;
    Tile::Tree::Id treeId;
    Tile::ContentId contentId;
    DgnDbStatus status = GetTileContentWorker::ParseInputs(model, treeId, contentId, db, treeIdStr, tileIdStr);
    if (DgnDbStatus::Success != status)
        {
        auto retval = NapiUtils::CreateBentleyReturnErrorObject(status, nullptr, Env());
        callback.Call({retval});
        }
    else
        {
        auto worker = new GetTileContentWorker(cancel, callback, *model, treeId, contentId);
        worker->Queue();
        }
    }

//=======================================================================================
// Projects the NativeECPresentationManager class into JS.
//! @bsiclass
//=======================================================================================
struct NativeECPresentationManager : Napi::ObjectWrap<NativeECPresentationManager>
    {
    //=======================================================================================
    // Async Worker which sends ECPresentationResult via callback
    //! @bsiclass
    //=======================================================================================
    struct ResponseSender : Napi::AsyncWorker
    {
        struct BoolPredicate : IConditionVariablePredicate
            {
            BeAtomic<bool>& m_flag;
            BoolPredicate(BeAtomic<bool>& flag) : m_flag(flag) {}
            bool _TestCondition(BeConditionVariable&) override {return m_flag.load();}
            };
    private:
        BeConditionVariable m_waiter;
        ECPresentationResult m_result;
        BeAtomic<bool> m_hasResult;
    protected:
        void Execute() override
            {
            BoolPredicate pred(m_hasResult);
            m_waiter.WaitOnCondition(&pred, BeConditionVariable::Infinite);
            }
        void OnOK() override
            {
            Callback().MakeCallback(Receiver().Value(), {CreateReturnValue(Env(), m_result, true)});
            }
        void OnError(Napi::Error const& e) override
            {
            Callback().MakeCallback(Receiver().Value(), {CreateReturnValue(Env(), ECPresentationResult(ECPresentationStatus::Error, "callback error"))});
            }
    public:
        ResponseSender(Napi::Function& callback) : Napi::AsyncWorker(callback), m_hasResult(false) {}
        void SetResult(ECPresentationResult&& result) {m_result = std::move(result); m_hasResult.store(true); m_waiter.notify_all();}
    };

    /*=================================================================================**//**
    * @bsiclass                                     Aidas.Kililnskas                05/2018
    +===============+===============+===============+===============+===============+======*/
    struct LocalState : IJsonLocalState
    {
    //! Saves the Utf8String value in the local state. Set to empty to delete value.
    //! @note The nameSpace and key pair must be unique.
    private:
        bmap<Utf8String, Utf8String> m_map;
    protected:
        void _SaveValue(Utf8CP nameSpace, Utf8CP key, Utf8StringCR value) override
            {
            Utf8PrintfString compositeKey("%s:%s", nameSpace, key);
            m_map[compositeKey] = value;
            }

        //! Returns a stored Utf8String from the local state. Returns empty if value does not exist.
        //! @note The nameSpace and key pair uniquely identifies the value.
        Utf8String _GetValue(Utf8CP nameSpace, Utf8CP key) const override
            {
            Utf8PrintfString compositeKey("%s:%s", nameSpace, key);
            auto iter = m_map.find(compositeKey);
            if (iter != m_map.end())
                return iter->second;
            return "";
            }
    };

    DEFINE_CONSTRUCTOR;

    ConnectionManager m_connections;
    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;
    RefCountedPtr<SimpleRuleSetLocater> m_ruleSetLocater;
    LocalState m_localState;

    NativeECPresentationManager(Napi::CallbackInfo const& info)
        : Napi::ObjectWrap<NativeECPresentationManager>(info)
        {
        m_presentationManager = std::unique_ptr<RulesDrivenECPresentationManager>(ECPresentationUtils::CreatePresentationManager(m_connections, T_HOST.GetIKnownLocationsAdmin()));
        m_ruleSetLocater = SimpleRuleSetLocater::Create();
        m_presentationManager->GetLocaters().RegisterLocater(*m_ruleSetLocater);
        m_presentationManager->SetLocalState(&m_localState);
        }
    ~NativeECPresentationManager()
        {
        // 'terminate' not called
        BeAssert(m_presentationManager == nullptr);
        }

    static bool InstanceOf(Napi::Value val) {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    //  Create projections
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ECPresentationManager", {
          InstanceMethod("setupRulesetDirectories", &NativeECPresentationManager::SetupRulesetDirectories),
          InstanceMethod("setupLocaleDirectories", &NativeECPresentationManager::SetupLocaleDirectories),
          InstanceMethod("setRulesetVariableValue", &NativeECPresentationManager::SetRulesetVariableValue),
          InstanceMethod("getRulesetVariableValue", &NativeECPresentationManager::GetRulesetVariableValue),
          InstanceMethod("getRulesets", &NativeECPresentationManager::GetRulesets),
          InstanceMethod("addRuleset", &NativeECPresentationManager::AddRuleset),
          InstanceMethod("removeRuleset", &NativeECPresentationManager::RemoveRuleset),
          InstanceMethod("clearRulesets", &NativeECPresentationManager::ClearRulesets),
          InstanceMethod("handleRequest", &NativeECPresentationManager::HandleRequest),
          InstanceMethod("dispose", &NativeECPresentationManager::Terminate)
        });

        exports.Set("ECPresentationManager", t);

        SET_CONSTRUCTOR(t);
        }

    static Napi::Value CreateReturnValue(Napi::Env const& env, ECPresentationResult const& result, bool serializeResponse = false)
        {
        // error
        if (result.IsError())
            {
            return NapiUtils::CreateBentleyReturnErrorObject(result.GetStatus(), result.GetErrorMessage().c_str(), env);
            }

        // success / jsoncpp response
        if (result.IsJsonCppResponse())
            {
            if (serializeResponse)
                return NapiUtils::CreateBentleyReturnSuccessObject(toJsString(env, result.GetJsonCppSuccessResponse().ToString().c_str()), env);
            return NapiUtils::CreateBentleyReturnSuccessObject(NapiUtils::Convert(env, result.GetJsonCppSuccessResponse()), env);
            }

        // success / rapidjson response
        if (serializeResponse)
            {
            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            result.GetSuccessResponse().Accept(writer);
            return NapiUtils::CreateBentleyReturnSuccessObject(toJsString(env, buffer.GetString()), env);
            }
        return NapiUtils::CreateBentleyReturnSuccessObject(NapiUtils::Convert(env, result.GetSuccessResponse()), env);
        }

    Napi::Value CreateReturnValue(ECPresentationResult const& result, bool serializeResponse = false)
        {
        return CreateReturnValue(Env(), result, serializeResponse);
        }

    void HandleRequest(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_FUNCTION(2, responseCallback, );

        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db, );
        if (!db->IsOpen())
            {
            responseCallback.Call({CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "iModel: not open"))});
            return;
            }

        REQUIRE_ARGUMENT_STRING(1, serializedRequest, );
        Json::Value request = Json::Value::From(serializedRequest);
        if (request.isNull())
            {
            responseCallback.Call({CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "request"))});
            return;
            }
        Utf8CP requestId = request["requestId"].asCString();
        if (Utf8String::IsNullOrEmpty(requestId))
            {
            responseCallback.Call({CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "request.requestId"))});
            return;
            }

        m_connections.NotifyConnectionOpened(db->GetDgnDb());

        JsonValueCR params = request["params"];

        ResponseSender* responseSender = new ResponseSender(responseCallback);
        responseSender->Queue();
        try
            {
            folly::Future<ECPresentationResult> result = folly::makeFutureWith([]() {return ECPresentationResult(ECPresentationStatus::InvalidArgument, "request.requestId"); });

            if (0 == strcmp("GetRootNodesCount", requestId))
                result = ECPresentationUtils::GetRootNodesCount(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetRootNodes", requestId))
                result = ECPresentationUtils::GetRootNodes(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetChildrenCount", requestId))
                result = ECPresentationUtils::GetChildrenCount(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetChildren", requestId))
                result = ECPresentationUtils::GetChildren(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetNodePaths", requestId))
                result = ECPresentationUtils::GetNodesPaths(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetFilteredNodePaths", requestId))
                result = ECPresentationUtils::GetFilteredNodesPaths(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetContentDescriptor", requestId))
                result = ECPresentationUtils::GetContentDescriptor(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetContent", requestId))
                result = ECPresentationUtils::GetContent(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetContentSetSize", requestId))
                result = ECPresentationUtils::GetContentSetSize(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetDistinctValues", requestId))
                result = ECPresentationUtils::GetDistinctValues(*m_presentationManager, db->GetDgnDb(), params);
            else if (0 == strcmp("GetDisplayLabel", requestId))
                result = ECPresentationUtils::GetDisplayLabel(*m_presentationManager, db->GetDgnDb(), params);

            result
            .then([responseSender](ECPresentationResult result)
                {
                responseSender->SetResult(std::move(result));
                })
            .onError([responseSender](folly::exception_wrapper)
                {
                responseSender->SetResult(ECPresentationResult(ECPresentationStatus::Error, "Unknown error handling request"));
                });
            }
        catch (...)
            {
            responseSender->SetResult(ECPresentationResult(ECPresentationStatus::Error, "Unknown error creating request"));
            }
        }

    Napi::Value SetupRulesetDirectories(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, rulesetDirectories, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "rulesetDirectories")));
        ECPresentationResult result = ECPresentationUtils::SetupRulesetDirectories(*m_presentationManager, rulesetDirectories);
        return CreateReturnValue(result);
        }

    Napi::Value SetupLocaleDirectories(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, localeDirectories, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "localeDirectories")));
        ECPresentationResult result = ECPresentationUtils::SetupLocaleDirectories(*m_presentationManager, localeDirectories);
        return CreateReturnValue(result);
        }

    Napi::Value GetRulesets(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "rulesetId")));
        ECPresentationResult result = ECPresentationUtils::GetRulesets(*m_ruleSetLocater, rulesetId);
        return CreateReturnValue(result, true);
        }

    Napi::Value AddRuleset(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetJsonString, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "rulesetJsonString")));
        ECPresentationResult result = ECPresentationUtils::AddRuleset(*m_ruleSetLocater, rulesetJsonString);
        return CreateReturnValue(result);
        }

    Napi::Value RemoveRuleset(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "rulesetId")));
        REQUIRE_ARGUMENT_STRING(1, hash, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "hash")));
        ECPresentationResult result = ECPresentationUtils::RemoveRuleset(*m_ruleSetLocater, rulesetId, hash);
        return CreateReturnValue(result);
        }

    Napi::Value ClearRulesets(Napi::CallbackInfo const& info)
        {
        ECPresentationResult result = ECPresentationUtils::ClearRulesets(*m_ruleSetLocater);
        return CreateReturnValue(result);
        }

    Napi::Value GetRulesetVariableValue(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, rulesetId, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "rulesetId")));
        REQUIRE_ARGUMENT_STRING(1, variableId, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "variableId")));
        REQUIRE_ARGUMENT_STRING(2, type, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "type")));
        ECPresentationResult result = ECPresentationUtils::GetRulesetVariableValue(*m_presentationManager, rulesetId, variableId, type);
        return CreateReturnValue(result);
        }

    Napi::Value SetRulesetVariableValue(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, ruleSetId, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "rulesetId")));
        REQUIRE_ARGUMENT_STRING(1, variableId, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "variableId")));
        REQUIRE_ARGUMENT_STRING(2, variableType, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "type")));
        REQUIRE_ARGUMENT_ANY_OBJ(3, value, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "value")));
        Json::Value jsonValue = NapiUtils::Convert(value);
        ECPresentationResult result = ECPresentationUtils::SetRulesetVariableValue(*m_presentationManager, ruleSetId, variableId, variableType, jsonValue);
        return CreateReturnValue(result);
        }

    void Terminate(Napi::CallbackInfo const& info)
        {
        m_presentationManager.reset();
        }
    };

static bool s_assertionsEnabled = true;
static BeMutex s_assertionMutex;

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
void handleAssertion(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
    {
    BeMutexHolder lock(s_assertionMutex);
    if (!s_assertionsEnabled || !JsInterop::IsMainThread())
        return;

    Napi::Error::New(JsInterop::Env(), Utf8PrintfString("Native Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str()).ThrowAsJavaScriptException();
    }

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct DisableNativeAssertions : Napi::ObjectWrap<DisableNativeAssertions>
    {
    private:
        DEFINE_CONSTRUCTOR;
        bool m_enableOnDispose = false;

        void DoDispose()
            {
            if (!m_enableOnDispose)
                return;
            BeMutexHolder lock(s_assertionMutex);
            s_assertionsEnabled = true;
            //only re-enable assertions once. DoDispose is be called from the explicit
            //call to Dispose and later when the destructor is called
            m_enableOnDispose = false;
            }

    public:
        DisableNativeAssertions(Napi::CallbackInfo const& info) : Napi::ObjectWrap<DisableNativeAssertions>(info)
            {
            BeMutexHolder lock(s_assertionMutex);
            m_enableOnDispose = s_assertionsEnabled;
            s_assertionsEnabled = false;
            }

        ~DisableNativeAssertions() { DoDispose(); }

        void Dispose(Napi::CallbackInfo const& info) { DoDispose(); }

        static void Init(Napi::Env env, Napi::Object exports)
            {
            Napi::HandleScope scope(env);
            Napi::Function t = DefineClass(env, "DisableNativeAssertions", {
            InstanceMethod("dispose", &DisableNativeAssertions::Dispose),
            });

            exports.Set("DisableNativeAssertions", t);

            SET_CONSTRUCTOR(t);
            }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NativeImportContext : Napi::ObjectWrap<NativeImportContext>
{
private:
    DEFINE_CONSTRUCTOR;
    DgnImportContext* m_importContext = nullptr;

public:
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ImportContext", {
            InstanceMethod("addCodeSpecId", &NativeImportContext::AddCodeSpecId),
            InstanceMethod("addElementId", &NativeImportContext::AddElementId),
            InstanceMethod("dispose", &NativeImportContext::Dispose),
            InstanceMethod("findCodeSpecId", &NativeImportContext::FindCodeSpecId),
            InstanceMethod("findElementId", &NativeImportContext::FindElementId),
            InstanceMethod("importCodeSpec", &NativeImportContext::ImportCodeSpec),
            InstanceMethod("cloneElement", &NativeImportContext::CloneElement),
            InstanceMethod("importFont", &NativeImportContext::ImportFont),
        });
        exports.Set("ImportContext", t);
        SET_CONSTRUCTOR(t);
        }

    NativeImportContext(Napi::CallbackInfo const& info) : Napi::ObjectWrap<NativeImportContext>(info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, sourceDb, );
        REQUIRE_ARGUMENT_OBJ(1, NativeDgnDb, targetDb, );
        m_importContext = new DgnImportContext(sourceDb->GetDgnDb(), targetDb->GetDgnDb());
        }

    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    void Dispose(Napi::CallbackInfo const& info)
        {
        if (nullptr != m_importContext)
            {
            delete m_importContext;
            m_importContext = nullptr;
            }
        }

    Napi::Value AddCodeSpecId(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId, Napi::Number::New(Env(), (int) BentleyStatus::ERROR));
        REQUIRE_ARGUMENT_STRING_ID(1, targetIdStr, CodeSpecId, targetId, Napi::Number::New(Env(), (int) BentleyStatus::ERROR));
        m_importContext->AddCodeSpecId(sourceId, targetId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value AddElementId(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId, Napi::Number::New(Env(), (int) BentleyStatus::ERROR));
        REQUIRE_ARGUMENT_STRING_ID(1, targetIdStr, DgnElementId, targetId, Napi::Number::New(Env(), (int) BentleyStatus::ERROR));
        m_importContext->AddElementId(sourceId, targetId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
        }

    Napi::Value FindCodeSpecId(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId, Env().Undefined());
        CodeSpecId targetId = m_importContext->FindCodeSpecId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value FindElementId(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceId, Env().Undefined());
        DgnElementId targetId = m_importContext->FindElementId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value CloneElement(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, DgnElementId, sourceElementId, Env().Undefined());
        DgnElementCPtr sourceElement = m_importContext->GetSourceDb().Elements().GetElement(sourceElementId);
        if (!sourceElement.IsValid())
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid source ElementId", Env().Undefined());

        DgnModelId targetModelId = m_importContext->FindModelId(sourceElement->GetModelId());
        DgnModelPtr targetModel = m_importContext->GetDestinationDb().Models().GetModel(targetModelId);
        if (!targetModel.IsValid())
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid target model", Env().Undefined());

        DgnElementPtr targetElement = sourceElement->CloneForImport(nullptr, *targetModel, *m_importContext);
        if (!targetElement.IsValid())
            THROW_TYPE_EXCEPTION_AND_RETURN("Unable to clone element", Env().Undefined());

        Json::Value toJsonOptions;
        toJsonOptions["wantGeometry"] = true; // want everything since insert will happen on TypeScript side
        toJsonOptions["wantBRepData"] = true;
        Json::Value targetElementJson = targetElement->ToJson(toJsonOptions);
        return NapiUtils::Convert(Env(), targetElementJson);
        }

    Napi::Value ImportCodeSpec(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING_ID(0, sourceIdStr, CodeSpecId, sourceId, Env().Undefined());
        CodeSpecId targetId = m_importContext->RemapCodeSpecId(sourceId);
        return toJsString(Env(), targetId);
        }

    Napi::Value ImportFont(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_NUMBER(0, sourceFontNumber, Env().Undefined());
        DgnFontId sourceFontId(static_cast<uint64_t>(sourceFontNumber.Uint32Value())); // BESERVER_ISSUED_ID_CLASS can be represented as a number in TypeScript
        DgnFontId targetFontId = m_importContext->RemapFont(sourceFontId);
        return Napi::Number::New(Env(), targetFontId.GetValue());
        }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                    12/18
//+---------------+---------------+---------------+---------------+---------------+------
static void initializeRegion(Napi::CallbackInfo const& info)
    {
    Region region = Region::Prod;
    if (info.Length() != 1 || !info[0].IsNumber())
        JsInterop::GetLogger().error("Invalid region. Using PROD as fallback.");
    else
        region = (Region) (int) info[0].As<Napi::Number>().Int32Value();

    UlasClient::Get().Initialize(region);
    }


static Napi::ObjectReference s_logger;
struct LogMessage {Utf8String m_category; Utf8String m_message; NativeLogging::SEVERITY m_severity;};
static bvector<LogMessage>* s_deferredLogging;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getLogger(Napi::CallbackInfo const& info) { return s_logger.Value(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void setLogger(Napi::CallbackInfo const& info)
    {
    s_logger = Napi::ObjectReference::New(info[0].ToObject());
    s_logger.SuppressDestruct();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void logMessageToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    auto env = s_logger.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = (sev == NativeLogging::LOG_TRACE)?   "logTrace":
                   (sev == NativeLogging::LOG_DEBUG)?   "logTrace": // Logger does not distinguish between trace and debug
                   (sev == NativeLogging::LOG_INFO)?    "logInfo":
                   (sev == NativeLogging::LOG_WARNING)? "logWarning":
                                                        "logError"; // Logger does not distinguish between error and fatal

    auto method = s_logger.Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
        return;
        }

    auto catJS = toJsString(env, category);
    auto msgJS = toJsString(env, msg);

    method({catJS, msgJS});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool callIsLogLevelEnabledJs(Utf8CP category, NativeLogging::SEVERITY sev)
    {
    auto env = s_logger.Env();
    Napi::HandleScope scope(env);

    auto method = s_logger.Get("isEnabled").As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
        return true;
        }

    auto catJS = toJsString(env, category);

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
void JsInterop::LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    if (s_logger.IsEmpty() || !IsMainThread())
        {
        deferLogging(category, sev, msg);
        return;
        }

    doDeferredLogging();
    logMessageToJs(category, sev, msg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsInterop::IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev)
    {
    if (s_logger.IsEmpty() || !IsMainThread())
        return true;

    doDeferredLogging();
    return callIsLogLevelEnabledJs(category, sev);
    }

static Utf8String s_mobileResourcesDir;
static Utf8String s_mobileTempDir;

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Satyakam.Khadilkar    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
extern "C"
    {
    void imodeljs_addon_setMobileResourcesDir(Utf8CP d) {s_mobileResourcesDir = d;}
    void imodeljs_addon_setMobileTempDir(Utf8CP d) {s_mobileTempDir = d;}
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object registerModule(Napi::Env env, Napi::Object exports)
    {
    Napi::HandleScope scope(env);

#if (defined(BENTLEYCONFIG_OS_WINDOWS) && !defined(BENTLEYCONFIG_OS_WINRT)) || defined(BENTLEYCONFIG_OS_LINUX) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
    BeFileName addondir = Desktop::FileSystem::GetLibraryDir();
    BeFileName tempdir;
    Desktop::FileSystem::BeGetTempPath(tempdir);
#else
    BeFileName addondir(s_mobileResourcesDir);
    BeFileName tempdir(s_mobileTempDir);
#endif

    JsInterop::Initialize(addondir, env, tempdir);
    NativeDgnDb::Init(env, exports);
    NativeECDb::Init(env, exports);
    NativeChangedElementsECDb::Init(env, exports);
    NativeECSqlStatement::Init(env, exports);
    NativeECSqlBinder::Init(env, exports);
    NativeECSqlValue::Init(env, exports);
    NativeECSqlColumnInfo::Init(env, exports);
    NativeECSqlValueIterator::Init(env, exports);
    NativeSqliteStatement::Init(env, exports);
    BriefcaseManagerResourcesRequest::Init(env, exports);
    NativeECPresentationManager::Init(env, exports);
    NativeECSchemaXmlContext::Init(env, exports);
    SnapRequest::Init(env, exports);
    DisableNativeAssertions::Init(env, exports);
    NativeImportContext::Init(env, exports);

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
        Napi::PropertyDescriptor::Function(env, exports, "initializeRegion", &initializeRegion),
        Napi::PropertyDescriptor::Accessor(env, exports, "logger", &getLogger, &setLogger)
        });

    return exports;
    }

NODE_API_MODULE(iModelJsNative, registerModule)
} // namespace IModelJsNative
