/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
#include "ECSchemaXmlContextUtils.h"
#include <Bentley/Desktop/FileSystem.h>
#include <Bentley/BeThread.h>
#include <Bentley/CancellationToken.h>
#include <Bentley/BeAtomic.h>
#include <Bentley/PerformanceLogger.h>
#include <DgnPlatform/MeasureGeom.h>
#include <DgnPlatform/Tile.h>
#include <DgnPlatform/ChangedElementsManager.h>
#include "UlasClient.h"
#include "SignalTestUtility.h"
#include <Bentley/BeThreadLocalStorage.h>
#include "presentation/ECPresentationUtils.h"
#include "presentation/UpdateRecordsHandler.h"

#if defined(BENTLEYCONFIG_OS_WINDOWS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
#include "KeyTarInterop.h"
#endif

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_ECPRESENTATION
USING_NAMESPACE_BENTLEY_EC

/*
 *  See README-Private.md for Rules to Check in Code Reviews
 */

#define PROPERTY_ATTRIBUTES static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)

#define REQUIRE_DB_TO_BE_OPEN if (!IsOpen()) return CreateBentleyReturnErrorObject(DgnDbStatus::NotOpen);

#define THROW_JS_EXCEPTION(str) Napi::Error::New(info.Env(), str).ThrowAsJavaScriptException();
#define THROW_JS_EXCEPTION_AND_RETURN(str,retval) {THROW_JS_EXCEPTION(str) return retval;}
#define THROW_JS_TYPE_ERROR(str) Napi::TypeError::New(info.Env(), str).ThrowAsJavaScriptException();
#define THROW_TYPE_EXCEPTION_AND_RETURN(str,retval) {THROW_JS_TYPE_ERROR(str) return retval;}

#define ARGUMENT_IS_PRESENT(i) (info.Length() > (i))
#define ARGUMENT_IS_NOT_PRESENT(i) !ARGUMENT_IS_PRESENT(i)

#define ARGUMENT_IS_EMPTY(i) (ARGUMENT_IS_NOT_PRESENT(i) || info[i].IsUndefined() || info[i].IsNull())
#define ARGUMENT_IS_STRING(i) (ARGUMENT_IS_PRESENT(i) && info[i].IsString())
#define ARGUMENT_IS_NUMBER(i) (ARGUMENT_IS_PRESENT(i) && info[i].IsNumber())
#define ARGUMENT_IS_BOOL(i) (ARGUMENT_IS_PRESENT(i) && info[i].IsBoolean())

#define ARGUMENT_IS_NOT_STRING(i) !ARGUMENT_IS_STRING(i)
#define ARGUMENT_IS_NOT_BOOL(i) !ARGUMENT_IS_BOOL(i)
#define ARGUMENT_IS_NOT_NUMBER(i) !ARGUMENT_IS_NUMBER(i)

#define REJECT_DEFERRED_AND_RETURN(deferred, message) {\
    deferred.Reject(Napi::String::New(info.Env(), message));\
    return deferred.Promise();\
    }

#define REQUIRE_ARGUMENT_ANY_OBJ(i, var, retval)\
    if (ARGUMENT_IS_NOT_PRESENT(i)) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object", retval)\
    }\
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_ANY_OBJ_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_PRESENT(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be an object")\
    }\
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_OBJ(i, T, var, retval)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !T::InstanceOf(info[i])) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object of type " #T, retval)\
    }\
    T* var = T::Unwrap(info[i].As<Napi::Object>());

#define REQUIRE_ARGUMENT_FUNCTION(i, var, retval)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !info[i].IsFunction()) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a function", retval)\
    }\
    Napi::Function var = info[i].As<Napi::Function>();


#define REQUIRE_ARGUMENT_STRING(i, var, retval)\
    if (ARGUMENT_IS_NOT_STRING(i)) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a string", retval)\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_STRING_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_STRING(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a string")\
    }\
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_STRING_ID(i, strId, T, id, retval)\
    REQUIRE_ARGUMENT_STRING(i, strId, retval)\
    T id(BeInt64Id::FromString(strId.c_str()).GetValue());

#define REQUIRE_ARGUMENT_STRING_ARRAY(i, var, retval)\
    if (ARGUMENT_IS_NOT_PRESENT(i) || !info[i].IsArray()) {\
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
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a number", retval)\
    }\
    Napi::Number var = info[i].As<Napi::Number>();

#define REQUIRE_ARGUMENT_NUMBER_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a number")\
    }\
    Napi::Number var = info[i].As<Napi::Number>();

#define REQUIRE_ARGUMENT_INTEGER(i, var, retval)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an integer", retval)\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define REQUIRE_ARGUMENT_UINTEGER(i, var, retval)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an integer", retval)\
    }\
    int32_t var = info[i].As<Napi::Number>().Uint32Value();

#define REQUIRE_ARGUMENT_BOOL(i, var, retval)\
    if (ARGUMENT_IS_NOT_BOOL(i)) {\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be a boolean", retval)\
    }\
    bool var = info[i].As<Napi::Boolean>().Value();

#define REQUIRE_ARGUMENT_BOOL_ASYNC(i, var, deferred)\
    if (ARGUMENT_IS_NOT_BOOL(i)) {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a boolean")\
    }\
    bool var = info[i].As<Napi::Boolean>().Value();


#define OPTIONAL_ARGUMENT_BOOL(i, var, default, retval)\
    bool var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_BOOL(i)) {\
        var = info[i].As<Napi::Boolean>().Value();\
    } else {\
        var = (default);\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an boolean", retval)\
    }

#define OPTIONAL_ARGUMENT_BOOL_ASYNC(i, var, default, deferred)\
    bool var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_BOOL(i)) {\
        var = info[i].As<Napi::Boolean>().Value();\
    } else {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be a boolean")\
    }

#define OPTIONAL_ARGUMENT_INTEGER(i, var, default, retval)\
    int var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_NUMBER(i)) {\
        var = info[i].As<Napi::Number>().Int32Value();\
    } else {\
        var = (default);\
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an integer", retval)\
    }

#define OPTIONAL_ARGUMENT_INTEGER_ASYNC(i, var, default, deferred)\
    int var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        var = (default);\
    } else if (ARGUMENT_IS_NUMBER(i)) {\
        var = info[i].As<Napi::Number>().Int32Value();\
    } else {\
        REJECT_DEFERRED_AND_RETURN(deferred, "Argument " #i " must be an integer")\
    }

#define OPTIONAL_ARGUMENT_STRING(i, var, retval)\
    Utf8String var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        ;\
    } else if (ARGUMENT_IS_STRING(i)) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    } else {\
        Utf8PrintfString msg("Argument " #i " is type %d. It must be a string or undefined", info[i].Type());\
        THROW_TYPE_EXCEPTION_AND_RETURN(msg.c_str(), retval)\
    }

#define OPTIONAL_ARGUMENT_STRING_ASYNC(i, var, deferred)\
    Utf8String var;\
    if (ARGUMENT_IS_EMPTY(i)) {\
        ;\
    } else if (ARGUMENT_IS_STRING(i)) {\
        var = info[i].As<Napi::String>().Utf8Value().c_str();\
    } else {\
        Utf8PrintfString msg("Argument " #i " is type %d. It must be a string or undefined", info[i].Type());\
        REJECT_DEFERRED_AND_RETURN(deferred, msg.c_str())\
    }

#define DEFINE_CONSTRUCTOR static Napi::FunctionReference& Constructor() { static Napi::FunctionReference s_ctor; return s_ctor; }

// Per N-API docs: Call this on a reference that is declared as static data, to prevent its destructor
// from running at program shutdown time, which would attempt to reset the reference when the environment is no longer valid.
#define SET_CONSTRUCTOR(t) Constructor() = Napi::Persistent(t); Constructor().SuppressDestruct();

namespace IModelJsNative {

struct LogMessage {Utf8String m_message; NativeLogging::SEVERITY m_severity;};

// An ObjRefVault is place to store Napi::ObjectReferences. It holds onto the references as long as they
// are in use. ObjectReferences are stored in "slots" in the vault. Code running on any thread can hold a reference
// to a slot, so as to keep that slot and its ObjectReference alive. Only code running on the main thread can
// access the ObjectReference in a slot.
//
// Vault operations are guarded by the BeSystemMutex.
//
// Details: A slot has its own reference count. A reference to a slot is a claim to keep the slot alive.
// A reference to a slot can be incremented/decremented by code running on any thread. When the
// last reference to a slot is released, that means that the slot should release the ObjectReference itself.
// The last decrement of the slot's can occur on any thread. The actual release of the JS object can
// happen only on the main thread. Therefore, the release of the slot is handled as a request to release the JS object.
// This request is noticed and carried out later on the main thread.
struct ObjRefVault
{
  private:
    struct Slot
        {
        uint32_t m_refCount{};
        Napi::ObjectReference m_objRef;

        Slot() {}
        Slot(Slot const &) = delete;
        Slot &operator=(Slot const &) = delete;
        Slot(Slot &&);
        Slot& operator=(Slot&&);
        };

    std::map<Utf8String, Slot> m_slotMap;

  public:
    void Clear();

#ifdef COMMENT_OUT_DUMP
    void Dump();
#endif

    bool IsEmpty() const;
    void StoreObjectRef(Napi::Object obj, Utf8StringCR id);
    Utf8String FindIdFromObject(Napi::Object obj);
    // Return the object in the slot or Undefined of id is empty or invalid
    Napi::Value GetObjectById_Locked(Napi::Env env, Utf8StringCR id);
    Napi::Value GetObjectById(Napi::Env env, Utf8StringCR id);
    uint32_t GetObjectRefCountById(Utf8StringCR id);
    void ReleaseUnreferencedObjects();
    void AddRefToObject(Utf8StringCR id);
    void ReleaseRefToObject(Utf8StringCR id);
};

static Napi::ObjectReference s_logger;
static bmap<Utf8String, bmap<Utf8String, bvector<LogMessage>>>* s_deferredLogging;
static bmap<Utf8String, NativeLogging::SEVERITY>* s_categorySeverityFilter;
static BeThreadLocalStorage* s_currentClientRequestContext;
static ObjRefVault s_objRefVault;
static bset<JsInterop::ObjectReferenceClaimCheck> s_deferredLoggingClientRequestActivityClaimChecks;
static bool s_assertionsEnabled = true;
static BeMutex s_assertionMutex;
static Utf8String s_mobileResourcesDir;
static Utf8String s_mobileTempDir;

// DON'T change this flag directly. It is managed by BeObjectWrap only.
static bool s_BeObjectWrap_inDestructor;

// Every ObjectWrap subclass must derive from BeObjectWrap!
template<typename OBJ>
struct BeObjectWrap : Napi::ObjectWrap<OBJ>
    {
    protected:

    BeObjectWrap(Napi::CallbackInfo const& info) : Napi::ObjectWrap<OBJ>(info) {}

    // Every derived class must call this function on the first line of its destructor
    static void SetInDestructor()
        {
        BeAssert(!s_BeObjectWrap_inDestructor && "Call SetInDestructor once, as the first line of the subclass destructor.");
        s_BeObjectWrap_inDestructor = true;
        }

    ~BeObjectWrap()
        {
        BeAssert(s_BeObjectWrap_inDestructor && "Subclass should have called SetInDestructor at the start of its destructor");
        // We can re-enable JS callbacks, now that we know that the destructors for the subclass and its member variables have finished.
        s_BeObjectWrap_inDestructor = false;
        }
    };

static void doDeferredLogging();

static int32_t getOptionalIntProperty(Napi::Object obj, Utf8CP propName, int32_t dval)
    {
    if (!obj.Has(propName))
        return dval;
    return obj.Get(propName).ToNumber().Int32Value();
    }

static bool getOptionalBooleanProperty(Napi::Object obj, Utf8CP propName, bool dval)
    {
    if (!obj.Has(propName))
        return dval;
    return obj.Get(propName).ToBoolean().Value();
    }

static Utf8String getOptionalStringProperty(Napi::Object obj, Utf8CP propName, Utf8CP dval)
    {
    if (!obj.Has(propName))
        return dval;
    return obj.Get(propName).ToString().Utf8Value().c_str();
    }

Napi::String toJsString(Napi::Env env, Utf8CP val, size_t len) { return Napi::String::New(env, val, len); }
Napi::String toJsString(Napi::Env env, Utf8CP val) { return toJsString(env, val, std::strlen(val)); }
Napi::String toJsString(Napi::Env env, Utf8StringCR str) { return toJsString(env, str.c_str(), str.length()); }
Napi::String toJsString(Napi::Env env, BeInt64Id id) { return toJsString(env, id.ToHexStr()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
ObjRefVault::Slot::Slot(Slot &&r)
    {
    m_refCount = std::move(r.m_refCount);
    m_objRef = std::move(r.m_objRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
ObjRefVault::Slot& ObjRefVault::Slot::operator=(Slot&& r)
    {
    m_refCount = std::move(r.m_refCount);
    m_objRef = std::move(r.m_objRef);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjRefVault::Clear()
    {
    m_slotMap.clear();
    }

#ifdef COMMENT_OUT_DUMP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjRefVault::Dump()
    {
    OutputDebugStringA("----------- ObjRefVault ---------------\n");
    for (auto const& e : m_slotMap)
        {
        Utf8PrintfString s("%s: %d\n", e.first.c_str(), e.second.m_refCount);
        OutputDebugStringA(s.c_str());
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool ObjRefVault::IsEmpty() const { return m_slotMap.empty(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjRefVault::StoreObjectRef(Napi::Object obj, Utf8StringCR id)
    {
    BeAssert(JsInterop::IsMainThread());

    BeSystemMutexHolder ___;

    BeAssert(m_slotMap.find(id) == m_slotMap.end());

    auto &slot = m_slotMap[id];
    slot.m_objRef.Reset(obj, 1); // Slot holds the one and only reference to the JS object.
    slot.m_refCount = 0;      // The slot itself is initially unreferenced
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ObjRefVault::FindIdFromObject(Napi::Object obj)
    {
    Utf8PrintfString id("%" PRIx64, (intptr_t)(napi_value)obj);
    auto i = m_slotMap.find(id);
    return (i != m_slotMap.end())? i->first: "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Value ObjRefVault::GetObjectById_Locked(Napi::Env env, Utf8StringCR id)
    {
    // Caller must hold the BeSystemMutex!

    BeAssert(JsInterop::IsMainThread());

    if (id.empty())
        return env.Undefined();

    auto slotIt = m_slotMap.find(id);
    if (slotIt == m_slotMap.end())
        {
        return env.Undefined();
        }

    Slot& slot = slotIt->second;
    return slot.m_objRef.Value();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Value ObjRefVault::GetObjectById(Napi::Env env, Utf8StringCR id)
    {
    BeSystemMutexHolder ___;
    return GetObjectById_Locked(env, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ObjRefVault::GetObjectRefCountById(Utf8StringCR id)
    {
    BeSystemMutexHolder ___;
    if (id.empty())
        return 0;

    auto slotIt = m_slotMap.find(id);
    return (slotIt == m_slotMap.end())? 0: slotIt->second.m_refCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjRefVault::ReleaseUnreferencedObjects()
    {
    BeAssert(JsInterop::IsMainThread());

    bvector<Utf8String> unrefd;

    BeSystemMutexHolder ___;
    for (auto &slot : m_slotMap)
        {
        if (slot.second.m_refCount == 0)
            unrefd.push_back(slot.first);
        }

    for (auto &slotId : unrefd)
        {
        m_slotMap.erase(slotId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjRefVault::AddRefToObject(Utf8StringCR id)
    {
    if (id.empty())
        return;

    BeSystemMutexHolder ___;
    auto slotIt = m_slotMap.find(id);
    if (slotIt == m_slotMap.end())
        {
        BeAssert(false);
        return;
        }
    slotIt->second.m_refCount++;
    // DO NOT TRY TO ACCESS slot.m_objRef!!!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ObjRefVault::ReleaseRefToObject(Utf8StringCR id)
    {
    if (id.empty())
        return;

    BeSystemMutexHolder ___;
    auto slotIt = m_slotMap.find(id);
    if (slotIt == m_slotMap.end())
        {
        BeAssert(false);
        return;
        }
    if (slotIt->second.m_refCount == 0)
        {
        BeAssert(false && "ObjectReferenceClaimCheck refers to slot that is already supposed to be unref'd!!");
        return;
        }
    slotIt->second.m_refCount--;
    // DO NOT TRY TO ACCESS slot.m_objRef!!!
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck::ObjectReferenceClaimCheck()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck::ObjectReferenceClaimCheck(Utf8StringCR id) : m_id(id)
    {
    s_objRefVault.AddRefToObject(m_id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck::ObjectReferenceClaimCheck(JsInterop::ObjectReferenceClaimCheck const& rhs) : m_id(rhs.m_id)
    {
    s_objRefVault.AddRefToObject(m_id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck&  JsInterop::ObjectReferenceClaimCheck::operator=(JsInterop::ObjectReferenceClaimCheck const& rhs)
    {
    if (this == &rhs)
        return *this;
    s_objRefVault.ReleaseRefToObject(m_id);
    m_id = rhs.m_id;
    s_objRefVault.AddRefToObject(m_id);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsInterop::ObjectReferenceClaimCheck::operator< (ObjectReferenceClaimCheck const& rhs) const
    {
    return m_id < rhs.m_id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck::ObjectReferenceClaimCheck(JsInterop::ObjectReferenceClaimCheck&& rhs) : m_id(rhs.m_id)
    {
    // rhs is going away. I take over its reference to the slot. I don't add another ref.
    rhs.m_id.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck::~ObjectReferenceClaimCheck()
    {
    Dispose();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::ObjectReferenceClaimCheck::Dispose()
    {
    s_objRefVault.ReleaseRefToObject(m_id);
    m_id.clear();
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      05/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsInterop::IsJsExecutionDisabled()
    {
    return s_BeObjectWrap_inDestructor
        || !IsMainThread()
        || (!s_logger.IsEmpty() && s_logger.Env().IsExceptionPending());
    }

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
struct NativeECDb : BeObjectWrap<NativeECDb>
    {
    private:
        DEFINE_CONSTRUCTOR
        std::unique_ptr<ECDb> m_ecdb;

    public:
        NativeECDb(Napi::CallbackInfo const& info) : BeObjectWrap<NativeECDb>(info) {}
        ~NativeECDb() {SetInDestructor();}

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


        Napi::Value ConcurrentQueryInit(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_ANY_OBJ(0, cfg, Napi::Boolean::New(Env(), (int) BE_SQLITE_ERROR));
            return JsInterop::ConcurrentQueryInit(GetECDb(), Env(), cfg);
            }

        Napi::Value PostConcurrentQuery(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, ecsql, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            REQUIRE_ARGUMENT_STRING(1, bindings, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            REQUIRE_ARGUMENT_ANY_OBJ(2, limit, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            REQUIRE_ARGUMENT_ANY_OBJ(3, quota, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            REQUIRE_ARGUMENT_UINTEGER(4, priority, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            return JsInterop::PostConcurrentQuery(GetECDb(),Env(), ecsql, bindings, limit, quota, (ConcurrentQueryManager::Priority)priority);
            }

        Napi::Value PollConcurrentQuery(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_UINTEGER(0, taskId, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
            return JsInterop::PollConcurrentQuery(GetECDb(), Env(), taskId);
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

        static Napi::Value EnableSharedCache(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_BOOL(0, enabled, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
            DbResult r = BeSQLiteLib::EnableSharedCache(enabled);
            return Napi::Number::New(info.Env(), (int) r);
            }

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
                InstanceMethod("isOpen", &NativeECDb::IsOpen),
                InstanceMethod("concurrentQueryInit", &NativeECDb::ConcurrentQueryInit),
                InstanceMethod("postConcurrentQuery", &NativeECDb::PostConcurrentQuery),
                InstanceMethod("pollConcurrentQuery", &NativeECDb::PollConcurrentQuery),
                StaticMethod("enableSharedCache", &NativeECDb::EnableSharedCache),
            });

            exports.Set("ECDb", t);

            SET_CONSTRUCTOR(t)
            }
    };

//=======================================================================================
// Projects the NativeECSchemaXmlContext class into JS
//! @bsiclass
//=======================================================================================
struct NativeECSchemaXmlContext : BeObjectWrap<NativeECSchemaXmlContext>
    {
    private:
        DEFINE_CONSTRUCTOR
        ECSchemaReadContextPtr m_context;
        ECSchemaXmlContextUtils::LocaterCallbackUPtr m_locater;

    public:
        NativeECSchemaXmlContext(Napi::CallbackInfo const& info) : BeObjectWrap<NativeECSchemaXmlContext>(info)
            {
            m_context = ECSchemaXmlContextUtils::CreateSchemaReadContext(T_HOST.GetIKnownLocationsAdmin());
            }

        ~NativeECSchemaXmlContext() {SetInDestructor();}

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

		void SetFirstSchemaLocater(Napi::CallbackInfo const& info)
			{
			REQUIRE_ARGUMENT_FUNCTION(0, locaterCallback, );
			ECSchemaXmlContextUtils::SetFirstSchemaLocater(*m_context, m_locater, Napi::Persistent(locaterCallback));
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
                InstanceMethod("setSchemaLocater", &NativeECSchemaXmlContext::SetSchemaLocater),
                InstanceMethod("setFirstSchemaLocater", &NativeECSchemaXmlContext::SetFirstSchemaLocater)
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
struct BriefcaseManagerResourcesRequest : BeObjectWrap<BriefcaseManagerResourcesRequest>
{
    IBriefcaseManager::Request m_req;
    DEFINE_CONSTRUCTOR;

    BriefcaseManagerResourcesRequest(Napi::CallbackInfo const& info) : BeObjectWrap<BriefcaseManagerResourcesRequest>(info)
        {
        }

    ~BriefcaseManagerResourcesRequest() {SetInDestructor();}

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

//=======================================================================================
// Projects the DgnDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeDgnDb : BeObjectWrap<NativeDgnDb>
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
    std::shared_ptr<CancellationToken> m_cancellationToken;
    NativeDgnDb(Napi::CallbackInfo const& info) : BeObjectWrap<NativeDgnDb>(info) {}
    ~NativeDgnDb() {SetInDestructor(); CloseDgnDb();}

    //  Check if val is really a NativeDgnDb peer object
    static bool InstanceOf(Napi::Value val)
        {
        if (!val.IsObject())
            return false;

        Napi::HandleScope scope(val.Env());
        return val.As<Napi::Object>().InstanceOf(Constructor().Value());
        }

    DgnDbR GetDgnDb() {return *m_dgndb;}

    Napi::Object CreateBentleyReturnSuccessObject(Napi::Value goodVal) {return NapiUtils::CreateBentleyReturnSuccessObject(goodVal, Env());}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnErrorObject(STATUSTYPE errCode, Utf8CP msg = nullptr) {return NapiUtils::CreateBentleyReturnErrorObject(errCode, msg, Env());}

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode, Napi::Value goodValue) {return ((STATUSTYPE)0 != errCode) ? CreateBentleyReturnErrorObject(errCode) : CreateBentleyReturnSuccessObject(goodValue); }

    template<typename STATUSTYPE>
    Napi::Object CreateBentleyReturnObject(STATUSTYPE errCode) {return CreateBentleyReturnObject(errCode, Env().Undefined());}

    bool IsOpen() const {return m_dgndb.IsValid();}

    Napi::Value IsDgnDbOpen(Napi::CallbackInfo const &info) { return Napi::Boolean::New(Env(), IsOpen()); }

    Napi::Value IsReadonly(Napi::CallbackInfo const &info) { return Napi::Boolean::New(Env(), m_dgndb->IsReadonly()); }

    void SetIModelDb(Napi::CallbackInfo const &info)
        {
        Napi::Value obj = info[0];
        if (m_dgndb.IsValid())
            {
            if (obj.IsObject())
                m_dgndb->m_jsIModelDb.Reset(obj.As<Napi::Object>(), 1);
            else
                m_dgndb->m_jsIModelDb.Reset();
            }
        }

    void SetDgnDb(DgnDbR dgndb)
        {
        JsInterop::AddCrashReportDgnDb(dgndb);
        dgndb.AddFunction(HexStrSqlFunction::GetSingleton());
        dgndb.AddFunction(StrSqlFunction::GetSingleton());

        m_dgndb = &dgndb;
        m_cancellationToken = std::make_shared<CancellationToken>();
        }

    void ClearDgnDb()
        {
        BeAssert(nullptr != m_cancellationToken);
        m_cancellationToken->Cancel();
        m_cancellationToken = nullptr;

        m_dgndb->RemoveFunction(HexStrSqlFunction::GetSingleton());
        m_dgndb->RemoveFunction(StrSqlFunction::GetSingleton());
        m_dgndb->m_jsIModelDb.Reset(); // disconnect the iModelDb object
        JsInterop::RemoveCrashReportDgnDb(*m_dgndb);

        m_dgndb = nullptr;
        }

    DbResult OpenDgnDb(BeFileNameCR dbname, DgnDb::OpenParams const& openParams)
        {
        DgnDbPtr dgndb;
        DbResult result = JsInterop::OpenDgnDb(dgndb, dbname, openParams);
        if (BE_SQLITE_OK == result)
            SetDgnDb(*dgndb);
        return result;
        }

    void CloseDgnDb()
        {
        if (!m_dgndb.IsValid())
            return;

        if (m_dgndb->Txns().HasChanges())
            m_dgndb->SaveChanges();

        DgnDbPtr dgndb = m_dgndb;
        ClearDgnDb();
        dgndb->CloseDb();

        // Some code opens, merges changesets, and closes a dgndb in a background thread, so need to guard this call.
        if (!JsInterop::IsJsExecutionDisabled())
            doDeferredLogging();
        }

    DbResult CreateDgnDb(BeFileNameCR filename, JsonValueCR props, Napi::Env env)
        {
        DbResult result;
        DgnDbPtr dgndb = JsInterop::CreateDgnDb(result, filename, props, env);
        if (dgndb.IsValid())
            SetDgnDb(*dgndb);
        return result;
        }

    Napi::Value OpenIModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName, Env().Undefined());
        REQUIRE_ARGUMENT_INTEGER(1, mode, Env().Undefined());
        OPTIONAL_ARGUMENT_INTEGER(2, upgrade, (int) SchemaUpgradeOptions::DomainUpgradeOptions::CheckRequiredUpgrades, Env().Undefined());
        OPTIONAL_ARGUMENT_STRING(3, encryptionPropsString, Env().Undefined());

        BeFileName dbFileName(dbName.c_str(), true);
        SchemaUpgradeOptions schemaUpgradeOptions((SchemaUpgradeOptions::DomainUpgradeOptions) upgrade);
        DgnDb::OpenParams openParams((Db::OpenMode)mode, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);

        if (!encryptionPropsString.empty() && BeSQLite::Db::IsEncryptedDb(dbFileName))
            {
            Json::Value encryptionPropsJson = Json::Value::From(encryptionPropsString);
            if (encryptionPropsJson.isMember(JsInterop::json_password()))
                openParams.GetEncryptionParamsR().SetPassword(encryptionPropsJson[JsInterop::json_password()].asCString());
            }

        DbResult result = OpenDgnDb(dbFileName, openParams);
        return Napi::Number::New(Env(), (int) result);
        }

    Napi::Value CreateIModel(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, filename, Env().Undefined());
        REQUIRE_ARGUMENT_STRING(1, props, Env().Undefined());

        DbResult result = CreateDgnDb(BeFileName(filename.c_str(), true), Json::Value::From(props), Env());
        return Napi::Number::New(Env(), (int)result);
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

    Napi::Value ExtractChangedInstanceIdsFromChangeSet(Napi::CallbackInfo const &info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, changeSetFileName, Env().Undefined());
        BeFileName changeSetFile(changeSetFileName);
        Json::Value json;
        DgnDbStatus status = JsInterop::ExtractChangedInstanceIdsFromChangeSet(json, *m_dgndb, changeSetFile);
        return CreateBentleyReturnObject(status, toJsString(Env(), json.ToString()));
        }

    static TxnManager::TxnId TxnIdFromString(Utf8StringCR str) {
        return TxnManager::TxnId(BeInt64Id::FromString(str.c_str()).GetValueUnchecked());
    }
    static Utf8String TxnIdToString(TxnManager::TxnId txnId) {return BeInt64Id(txnId.GetValue()).ToHexStr();}

    Napi::Value GetCurrentTxnId(Napi::CallbackInfo const& info)
        {
        return toJsString(Env(), TxnIdToString(m_dgndb->Txns().GetCurrentTxnId()));
        }

    Napi::Value QueryFirstTxnId(Napi::CallbackInfo const& info) {
        OPTIONAL_ARGUMENT_BOOL(0, allowCrossSessions, true, Env().Undefined()); // default to true for backwards compatibility
        auto& txns = m_dgndb->Txns();
        TxnManager::TxnId startTxnId = allowCrossSessions ? txns.QueryNextTxnId(TxnManager::TxnId(0)) : txns.GetSessionStartId();
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
    Napi::Value GetUndoString(Napi::CallbackInfo const& info) {
        OPTIONAL_ARGUMENT_BOOL(0, allowCrossSessions, false, Env().Undefined());
        return toJsString(Env(), m_dgndb->Txns().GetUndoString((TxnManager::AllowCrossSessions) allowCrossSessions));
    }
    Napi::Value GetRedoString(Napi::CallbackInfo const& info) {return toJsString(Env(), m_dgndb->Txns().GetRedoString());}
    Napi::Value HasUnsavedChanges(Napi::CallbackInfo const& info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().HasChanges());}
    Napi::Value HasPendingTxns(Napi::CallbackInfo const& info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().HasPendingTxns());}
    Napi::Value IsRedoPossible(Napi::CallbackInfo const& info) {return Napi::Boolean::New(Env(), m_dgndb->Txns().IsRedoPossible());}
    Napi::Value IsUndoPossible(Napi::CallbackInfo const& info) {
        OPTIONAL_ARGUMENT_BOOL(0, allowCrossSessions, false, Env().Undefined());
        return Napi::Boolean::New(Env(), m_dgndb->Txns().IsUndoPossible((TxnManager::AllowCrossSessions) allowCrossSessions));
    }
    Napi::Value ReinstateTxn(Napi::CallbackInfo const& info) {return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReinstateTxn());}
    Napi::Value ReverseAll(Napi::CallbackInfo const& info) {return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseAll());}
    Napi::Value ReverseTo(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        OPTIONAL_ARGUMENT_BOOL(1, allowCrossSessions, false, Env().Undefined());
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseTo(TxnIdFromString(txnIdHexStr), (TxnManager::AllowCrossSessions) allowCrossSessions));
    }
    Napi::Value CancelTo(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_STRING(0, txnIdHexStr, Env().Undefined());
        OPTIONAL_ARGUMENT_BOOL(1, allowCrossSessions, false, Env().Undefined());
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().CancelTo(TxnIdFromString(txnIdHexStr), (TxnManager::AllowCrossSessions) allowCrossSessions));
    }
    Napi::Value ReverseTxns(Napi::CallbackInfo const& info) {
        REQUIRE_ARGUMENT_NUMBER(0, numTxns , Env().Undefined());
        OPTIONAL_ARGUMENT_BOOL(1, allowCrossSessions, false, Env().Undefined());
        return Napi::Number::New(Env(), (int) m_dgndb->Txns().ReverseTxns(numTxns, (TxnManager::AllowCrossSessions) allowCrossSessions));
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

    Napi::Value GetMassProperties(Napi::CallbackInfo const& info)
        {
        // Use the macro to get the argument (which is JSON string containing the operation and candidate elements) from the info argument.
        REQUIRE_ARGUMENT_STRING(0, measureStr, Env().Undefined());
        // get a Json::Value from the string.
        Json::Value request = Json::Value::From(measureStr);
        MeasureGeomCollector::Response response = MeasureGeomCollector::DoMeasure((MeasureGeomCollector::Request const&) request, *m_dgndb);
        return toJsString(Env(), response.ToString());
        }

    Napi::Value GetIModelCoordsFromGeoCoords(Napi::CallbackInfo const& info)
        {
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

    Napi::Value ExportPartGraphics(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_ANY_OBJ(0, exportProps, Env().Undefined());

        Napi::Value onPartGraphicsVal = exportProps.Get("onPartGraphics");
        if (!onPartGraphicsVal.IsFunction())
            THROW_TYPE_EXCEPTION_AND_RETURN("onPartsGraphics must be a function", Env().Undefined());

        Napi::Value displayPropsVal = exportProps.Get("displayProps");
        if (!displayPropsVal.IsObject())
            THROW_TYPE_EXCEPTION_AND_RETURN("displayProps must be an object", Env().Undefined());

        Napi::Value elementIdVal = exportProps.Get("elementId");
        if (!elementIdVal.IsString())
            THROW_TYPE_EXCEPTION_AND_RETURN("elementId must be a string", Env().Undefined());

        DgnDbStatus status = JsInterop::ExportPartGraphics(GetDgnDb(), exportProps);
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

    void PurgeTileTrees(Napi::CallbackInfo const& info)
        {
        bvector<DgnModelId> modelIds;
        if (info.Length() == 1 && !info[0].IsUndefined())
            {
            if (info[0].IsArray())
                {
                Napi::Array arr = info[0].As<Napi::Array>();
                for (uint32_t index = 0; index < arr.Length(); ++index)
                    {
                    Napi::Value value = arr[index];
                    if (value.IsString())
                        {
                        auto modelId = BeInt64Id::FromString(value.As<Napi::String>().Utf8Value().c_str());
                        if (modelId.IsValid())
                            modelIds.push_back(DgnModelId(modelId.GetValue()));
                        }
                    }
                }

            if (modelIds.empty())
                THROW_TYPE_EXCEPTION_AND_RETURN("Argument 0 must be a non-empty array of valid Id64Strings", );
            }

        JsInterop::PurgeTileTrees(GetDgnDb(), modelIds.empty() ? nullptr : &modelIds);
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

            Napi::Object jsTileContent = Napi::Object::New(Env());
            jsTileContent.Set(Napi::String::New(Env(), "content"), blob);
            jsTileContent.Set(Napi::String::New(Env(), "elapsedSeconds"), Napi::Number::New(Env(), result.m_elapsedSeconds));

            return CreateBentleyReturnSuccessObject(jsTileContent);
            }
        }

    void CancelTileContentRequests(Napi::CallbackInfo const& info)
        {
        if (!IsOpen())
            return;

        REQUIRE_ARGUMENT_STRING(0, treeId, );
        REQUIRE_ARGUMENT_STRING_ARRAY(1, tileIds, );

        if (!tileIds.empty())
            JsInterop::CancelContentRequests(GetDgnDb(), treeId, tileIds);
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
        REQUIRE_ARGUMENT_STRING(1, jsonPropertiesStr, Env().Undefined());
        Json::Value jsonProperties = Json::Value::From(jsonPropertiesStr);
        Utf8String idStr;
        DgnDbStatus status = JsInterop::InsertCodeSpec(idStr, GetDgnDb(), name, jsonProperties);
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

    Napi::Value ImportFunctionalSchema(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        DbResult result = JsInterop::ImportFunctionalSchema(GetDgnDb());
        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value ImportSchemas(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING_ARRAY(0, schemaFileNames, Env().Undefined());
        DbResult result = JsInterop::ImportSchemasDgnDb(GetDgnDb(), schemaFileNames);
        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value FindGeometryPartReferences(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING_ARRAY(0, partIds, Napi::Array::New(Env(), 0));
        REQUIRE_ARGUMENT_BOOL(1, is2d, Napi::Array::New(Env(), 0));

        auto elemIds = JsInterop::FindGeometryPartReferences(partIds, is2d, GetDgnDb());
        uint32_t index = 0;
        auto ret = Napi::Array::New(Env(), elemIds.size());
        for (auto elemId : elemIds)
            ret.Set(index++, Napi::String::New(Env(), elemId.ToHexStr().c_str()));

        return ret;
        }

    Napi::Value ExportSchemas(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, exportDirectory, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        bvector<ECN::ECSchemaCP> schemas = GetDgnDb().Schemas().GetSchemas();
        for (ECSchemaCP schema : schemas)
            {
            BeFileName schemaFileName(exportDirectory);
            schemaFileName.AppendSeparator();
            schemaFileName.AppendUtf8(schema->GetFullSchemaName().c_str());
            schemaFileName.AppendExtension(L"ecschema.xml");
            ECVersion xmlVersion = ECSchema::ECVersionToWrite(schema->GetOriginalECXmlVersionMajor(), schema->GetOriginalECXmlVersionMinor());

            SchemaWriteStatus status = schema->WriteToXmlFile(schemaFileName.GetName(), xmlVersion);
            if (SchemaWriteStatus::Success != status)
                return Napi::Number::New(Env(), (int) status);
            }
        return Napi::Number::New(Env(), (int) SchemaWriteStatus::Success);
        }

    void CloseIModel(Napi::CallbackInfo const& info) {  CloseDgnDb(); }

    Napi::Value CreateClassViewsInDb(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        BentleyStatus status = GetDgnDb().Schemas().CreateClassViewsInDb();
        return Napi::Number::New(Env(), (int) status);
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
        REQUIRE_ARGUMENT_INTEGER(0, idValue, Env().Undefined());
        OPTIONAL_ARGUMENT_STRING(1, encryptionPropsString, Env().Undefined());
        BeFileName name(m_dgndb->GetFileName());

        // TODO: This routine has excessive logging to diagnose performance issues with this
        // simple operation. The logs must be removed after the issue is addressed.
        PERFLOG_START("iModelJsNative", "SetAsBriefcase");
        DbResult result = m_dgndb->SetAsBriefcase(BeBriefcaseId(idValue));
        PERFLOG_FINISH("iModelJsNative", "SetAsBriefcase");

        if (BE_SQLITE_OK == result)
            {
            PERFLOG_START("iModelJsNative", "SaveChanges");
            result = m_dgndb->SaveChanges();
            PERFLOG_FINISH("iModelJsNative", "SaveChanges");
            }

         // Note: We need to close and reopen the Db to enable change tracking
        if (BE_SQLITE_OK == result)
            {
            CloseDgnDb();

            PERFLOG_START("iModelJsNative", "OpenDgnDb");
            SchemaUpgradeOptions schemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::SkipCheck);
            DgnDb::OpenParams openParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);

            if (!encryptionPropsString.empty())
                {
                Json::Value encryptionPropsJson = Json::Value::From(encryptionPropsString);
                if (encryptionPropsJson.isMember(JsInterop::json_password()))
                    openParams.GetEncryptionParamsR().SetPassword(encryptionPropsJson[JsInterop::json_password()].asCString());
                }

            result = OpenDgnDb(name, openParams);
            PERFLOG_FINISH("iModelJsNative", "OpenDgnDb");
            }

        return Napi::Number::New(Env(), (int)result);
        }

    Napi::Value GetFilePath(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        return toJsString(Env(), m_dgndb->GetDbFileName());
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

    Napi::Value SaveProjectGuid(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        REQUIRE_ARGUMENT_STRING(0, guidStr, Env().Undefined());
        BeGuid guid;
        guid.FromString(guidStr.c_str());
        m_dgndb->SaveProjectGuid(guid);
        return Napi::Number::New(Env(), (int)BE_SQLITE_OK);
        }

    Napi::Value QueryProjectGuid(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN
        BeGuid beGuid = m_dgndb->QueryProjectGuid();
        return toJsString(Env(), beGuid.ToString());
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

    static Napi::Value EnableSharedCache(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_BOOL(0, enabled, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
        DbResult r = BeSQLiteLib::EnableSharedCache(enabled);
        return Napi::Number::New(info.Env(), (int) r);
        }

    static Napi::Value EncryptDb(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_STRING(1, encryptionPropsString, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));

        BeFileName dbFileName(dbName.c_str(), true);
        Json::Value encryptionPropsJson = Json::Value::From(encryptionPropsString);

        DbResult status = BE_SQLITE_ERROR;
        BeSQLite::Db::EncryptionParams encryptionParams;
        if (encryptionPropsJson.isMember(JsInterop::json_password()))
            {
            encryptionParams.SetPassword(encryptionPropsJson[JsInterop::json_password()].asCString());
            status = BeSQLite::Db::EncryptDb(dbFileName, encryptionParams);
            }

        return Napi::Number::New(info.Env(), (int) status);
        }

    Napi::Value IsEncrypted(Napi::CallbackInfo const&)
        {
        REQUIRE_DB_TO_BE_OPEN
        bool isEncrypted = BeSQLite::Db::IsEncryptedDb(GetDgnDb().GetFileName());
        return Napi::Boolean::New(Env(), isEncrypted);
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


    Napi::Value ConcurrentQueryInit(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN;
        REQUIRE_ARGUMENT_ANY_OBJ(0, cfg, Napi::Boolean::New(Env(), (int) BE_SQLITE_ERROR));
        return JsInterop::ConcurrentQueryInit(GetDgnDb(), Env(), cfg);
        }

    Napi::Value PostConcurrentQuery(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN;
        REQUIRE_ARGUMENT_STRING(0, ecsql, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_STRING(1, bindings, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_ANY_OBJ(2, limit, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_ANY_OBJ(3, quota, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_UINTEGER(4, priority, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        return JsInterop::PostConcurrentQuery(GetDgnDb(), Env(), ecsql, bindings, limit, quota, (ConcurrentQueryManager::Priority)priority);
        }

    Napi::Value PollConcurrentQuery(Napi::CallbackInfo const& info)
        {
        REQUIRE_DB_TO_BE_OPEN;
        REQUIRE_ARGUMENT_UINTEGER(0, taskId, Napi::Number::New(Env(), (int) BE_SQLITE_ERROR));
        return JsInterop::PollConcurrentQuery(GetDgnDb(), Env(), taskId);
        }

    static Napi::Value Vacuum(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
        OPTIONAL_ARGUMENT_INTEGER(1, pageSize, 0, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
        DbResult status = Db::Vacuum(dbName.c_str(), pageSize);
        return Napi::Number::New(info.Env(), (int)status);
        }

    static Napi::Value UnsafeSetBriefcaseId(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING(0, dbName, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
        REQUIRE_ARGUMENT_INTEGER(1, briefcaseId, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
        OPTIONAL_ARGUMENT_STRING(2, dbGuid, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));
        OPTIONAL_ARGUMENT_STRING(3, projectGuid, Napi::Number::New(info.Env(), (int) BE_SQLITE_ERROR));

        DbResult status = JsInterop::UnsafeSetBriefcaseId(BeFileName(dbName), BeBriefcaseId(briefcaseId), dbGuid, projectGuid);
        return Napi::Number::New(info.Env(), (int)status);
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
            InstanceMethod("attachChangeCache", &NativeDgnDb::AttachChangeCache),
            InstanceMethod("beginMultiTxnOperation", &NativeDgnDb::BeginMultiTxnOperation),
            InstanceMethod("briefcaseManagerEndBulkOperation", &NativeDgnDb::BriefcaseManagerEndBulkOperation),
            InstanceMethod("briefcaseManagerStartBulkOperation", &NativeDgnDb::BriefcaseManagerStartBulkOperation),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForElement", &NativeDgnDb::BuildBriefcaseManagerResourcesRequestForElement),
            InstanceMethod("buildBriefcaseManagerResourcesRequestForModel", &NativeDgnDb::BuildBriefcaseManagerResourcesRequestForModel),
            InstanceMethod("cancelTileContentRequests", &NativeDgnDb::CancelTileContentRequests),
            InstanceMethod("cancelTo", &NativeDgnDb::CancelTo),
            InstanceMethod("closeIModel", &NativeDgnDb::CloseIModel),
            InstanceMethod("concurrentQueryInit", &NativeDgnDb::ConcurrentQueryInit),
            InstanceMethod("createChangeCache", &NativeDgnDb::CreateChangeCache),
            InstanceMethod("createClassViewsInDb", &NativeDgnDb::CreateClassViewsInDb),
            InstanceMethod("createIModel", &NativeDgnDb::CreateIModel),
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
            InstanceMethod("exportPartGraphics", &NativeDgnDb::ExportPartGraphics),
            InstanceMethod("exportSchemas", &NativeDgnDb::ExportSchemas),
            InstanceMethod("extractBriefcaseManagerResourcesRequest", &NativeDgnDb::ExtractBriefcaseManagerResourcesRequest),
            InstanceMethod("extractBulkResourcesRequest", &NativeDgnDb::ExtractBulkResourcesRequest),
            InstanceMethod("extractChangedInstanceIdsFromChangeSet", &NativeDgnDb::ExtractChangedInstanceIdsFromChangeSet),
            InstanceMethod("extractChangeSummary", &NativeDgnDb::ExtractChangeSummary),
            InstanceMethod("extractCodes", &NativeDgnDb::ExtractCodes),
            InstanceMethod("extractCodesFromFile", &NativeDgnDb::ExtractCodesFromFile),
            InstanceMethod("findGeometryPartReferences", &NativeDgnDb::FindGeometryPartReferences),
            InstanceMethod("finishCreateChangeSet", &NativeDgnDb::FinishCreateChangeSet),
            InstanceMethod("getBriefcaseId", &NativeDgnDb::GetBriefcaseId),
            InstanceMethod("getCurrentTxnId", &NativeDgnDb::GetCurrentTxnId),
            InstanceMethod("getDbGuid", &NativeDgnDb::GetDbGuid),
            InstanceMethod("getECClassMetaData", &NativeDgnDb::GetECClassMetaData),
            InstanceMethod("getElement", &NativeDgnDb::GetElement),
            InstanceMethod("getFilePath", &NativeDgnDb::GetFilePath),
            InstanceMethod("getGeoCoordinatesFromIModelCoordinates", &NativeDgnDb::GetGeoCoordsFromIModelCoords),
            InstanceMethod("getIModelCoordinatesFromGeoCoordinates", &NativeDgnDb::GetIModelCoordsFromGeoCoords),
            InstanceMethod("getIModelProps", &NativeDgnDb::GetIModelProps),
            InstanceMethod("getMassProperties", &NativeDgnDb::GetMassProperties),
            InstanceMethod("getModel", &NativeDgnDb::GetModel),
            InstanceMethod("getMultiTxnOperationDepth", &NativeDgnDb::GetMultiTxnOperationDepth),
            InstanceMethod("getParentChangeSetId", &NativeDgnDb::GetParentChangeSetId),
            InstanceMethod("getPendingChangeSets", &NativeDgnDb::GetPendingChangeSets),
            InstanceMethod("getRedoString", &NativeDgnDb::GetRedoString),
            InstanceMethod("getReversedChangeSetId", &NativeDgnDb::GetReversedChangeSetId),
            InstanceMethod("getSchema", &NativeDgnDb::GetSchema),
            InstanceMethod("getSchemaItem", &NativeDgnDb::GetSchemaItem),
            InstanceMethod("getTileContent", &NativeDgnDb::GetTileContent),
            InstanceMethod("getTileTree", &NativeDgnDb::GetTileTree),
            InstanceMethod("getTxnDescription", &NativeDgnDb::GetTxnDescription),
            InstanceMethod("getUndoString", &NativeDgnDb::GetUndoString),
            InstanceMethod("hasFatalTxnError", &NativeDgnDb::HasFatalTxnError),
            InstanceMethod("hasSavedChanges", &NativeDgnDb::HasPendingTxns), // deprecated, use hasPendingChanges
            InstanceMethod("hasPendingTxns", &NativeDgnDb::HasPendingTxns),
            InstanceMethod("hasUnsavedChanges", &NativeDgnDb::HasUnsavedChanges),
            InstanceMethod("importFunctionalSchema", &NativeDgnDb::ImportFunctionalSchema),
            InstanceMethod("importSchemas", &NativeDgnDb::ImportSchemas),
            InstanceMethod("inBulkOperation", &NativeDgnDb::InBulkOperation),
            InstanceMethod("insertCodeSpec", &NativeDgnDb::InsertCodeSpec),
            InstanceMethod("insertElement", &NativeDgnDb::InsertElement),
            InstanceMethod("insertElementAspect", &NativeDgnDb::InsertElementAspect),
            InstanceMethod("insertLinkTableRelationship", &NativeDgnDb::InsertLinkTableRelationship),
            InstanceMethod("insertModel", &NativeDgnDb::InsertModel),
            InstanceMethod("isChangeCacheAttached", &NativeDgnDb::IsChangeCacheAttached),
            InstanceMethod("isEncrypted", &NativeDgnDb::IsEncrypted),
            InstanceMethod("isOpen", &NativeDgnDb::IsDgnDbOpen),
            InstanceMethod("isReadonly", &NativeDgnDb::IsReadonly),
            InstanceMethod("isRedoPossible", &NativeDgnDb::IsRedoPossible),
            InstanceMethod("isTxnIdValid", &NativeDgnDb::IsTxnIdValid),
            InstanceMethod("isUndoPossible", &NativeDgnDb::IsUndoPossible),
            InstanceMethod("logTxnError", &NativeDgnDb::LogTxnError),
            InstanceMethod("openIModel", &NativeDgnDb::OpenIModel),
            InstanceMethod("pollConcurrentQuery", &NativeDgnDb::PollConcurrentQuery),
            InstanceMethod("pollTileContent", &NativeDgnDb::PollTileContent),
            InstanceMethod("postConcurrentQuery", &NativeDgnDb::PostConcurrentQuery),
            InstanceMethod("purgeTileTrees", &NativeDgnDb::PurgeTileTrees),
            InstanceMethod("queryFileProperty", &NativeDgnDb::QueryFileProperty),
            InstanceMethod("queryFirstTxnId", &NativeDgnDb::QueryFirstTxnId),
            InstanceMethod("queryModelExtents", &NativeDgnDb::QueryModelExtents),
            InstanceMethod("queryNextAvailableFileProperty", &NativeDgnDb::QueryNextAvailableFileProperty),
            InstanceMethod("queryNextTxnId", &NativeDgnDb::QueryNextTxnId),
            InstanceMethod("queryPreviousTxnId", &NativeDgnDb::QueryPreviousTxnId),
            InstanceMethod("queryProjectGuid", &NativeDgnDb::QueryProjectGuid),
            InstanceMethod("readFontMap", &NativeDgnDb::ReadFontMap),
            InstanceMethod("reinstateTxn", &NativeDgnDb::ReinstateTxn),
            InstanceMethod("removePendingChangeSet", &NativeDgnDb::RemovePendingChangeSet),
            InstanceMethod("reverseAll", &NativeDgnDb::ReverseAll),
            InstanceMethod("reverseTo", &NativeDgnDb::ReverseTo),
            InstanceMethod("reverseTxns", &NativeDgnDb::ReverseTxns),
            InstanceMethod("saveChanges", &NativeDgnDb::SaveChanges),
            InstanceMethod("saveFileProperty", &NativeDgnDb::SaveFileProperty),
            InstanceMethod("saveProjectGuid", &NativeDgnDb::SaveProjectGuid),
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
            StaticMethod("enableSharedCache", &NativeDgnDb::EnableSharedCache),
            StaticMethod("encryptDb", &NativeDgnDb::EncryptDb),
            StaticMethod("vacuum", &NativeDgnDb::Vacuum),
            StaticMethod("unsafeSetBriefcaseId", &NativeDgnDb::UnsafeSetBriefcaseId),
        });

        exports.Set("DgnDb", t);

        SET_CONSTRUCTOR(t);
        }
};

//=======================================================================================
// Projects the Changed Elements ECDb class into JS
//! @bsiclass
//=======================================================================================
struct NativeChangedElementsECDb : BeObjectWrap<NativeChangedElementsECDb>
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
        NativeChangedElementsECDb(Napi::CallbackInfo const& info) : BeObjectWrap<NativeChangedElementsECDb>(info) {}
        ~NativeChangedElementsECDb() {SetInDestructor();}

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
            OPTIONAL_ARGUMENT_STRING(4, rulesetDir, Env().Undefined());
            OPTIONAL_ARGUMENT_STRING(5, tempDir, Env().Undefined());

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
            if (!rulesetDir.empty())
                m_manager->SetPresentationRulesetDirectory(rulesetDir);

            if (!tempDir.empty())
                m_manager->SetTempLocation(tempDir.c_str());

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
struct NativeECSqlBinder : BeObjectWrap<NativeECSqlBinder>
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
    NativeECSqlBinder(Napi::CallbackInfo const& info) : BeObjectWrap<NativeECSqlBinder>(info)
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

    ~NativeECSqlBinder() {SetInDestructor();}

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
struct NativeECSqlColumnInfo : BeObjectWrap<NativeECSqlColumnInfo>
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
        NativeECSqlColumnInfo(Napi::CallbackInfo const& info) : BeObjectWrap<NativeECSqlColumnInfo>(info)
            {
            if (info.Length() != 1)
                THROW_TYPE_EXCEPTION_AND_RETURN("ECSqlColumnInfo constructor expects one argument.",);

            m_colInfo = info[0].As<Napi::External<ECSqlColumnInfo>>().Data();
            if (m_colInfo == nullptr)
                THROW_TYPE_EXCEPTION_AND_RETURN("Invalid first arg for NativeECSqlColumnInfo constructor. ECSqlColumnInfo must not be nullptr",);
            }

        ~NativeECSqlColumnInfo() {SetInDestructor();}

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
                        if (prop && prop->HasExtendedType())
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
                        if (prop && prop->HasExtendedType())
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
struct NativeECSqlValue : BeObjectWrap<NativeECSqlValue>
    {
private:
    DEFINE_CONSTRUCTOR;
    IECSqlValue const* m_ecsqlValue = nullptr;
    ECDb const* m_ecdb = nullptr;

public:
    NativeECSqlValue(Napi::CallbackInfo const& info) : BeObjectWrap<NativeECSqlValue>(info)
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

    ~NativeECSqlValue() {SetInDestructor();}

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
        if (SUCCESS != ECJsonUtilities::IGeometryToIModelJson(json, *geom))
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
struct NativeECSqlValueIterator : BeObjectWrap<NativeECSqlValueIterator>
    {
    private:
        DEFINE_CONSTRUCTOR;
        ECDb const* m_ecdb = nullptr;
        IECSqlValueIterable const* m_iterable = nullptr;
        bool m_isBeforeFirstElement = true;
        IECSqlValueIterable::const_iterator m_it;
        IECSqlValueIterable::const_iterator m_endIt;

    public:
        NativeECSqlValueIterator(Napi::CallbackInfo const& info) : BeObjectWrap<NativeECSqlValueIterator>(info)
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

        ~NativeECSqlValueIterator() {SetInDestructor();}

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
struct NativeECSqlStatement : BeObjectWrap<NativeECSqlStatement>
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
    NativeECSqlStatement(Napi::CallbackInfo const& info) : BeObjectWrap<NativeECSqlStatement>(info), m_stmt(new ECSqlStatement()) {}
    ~NativeECSqlStatement() {SetInDestructor();}

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
        if (!m_stmt || !m_stmt->IsPrepared())
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
struct NativeSqliteStatement : BeObjectWrap<NativeSqliteStatement>
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
        NativeSqliteStatement(Napi::CallbackInfo const& info) : BeObjectWrap<NativeSqliteStatement>(info), m_stmt(new Statement()) {}
        ~NativeSqliteStatement() {SetInDestructor();}

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
//  Utility to apply change sets (synchronously or asynchronously)
//! @bsiclass
//=======================================================================================
struct ApplyChangeSetsRequest : BeObjectWrap<ApplyChangeSetsRequest>
    {
    struct ApplyChangeSetsAsyncWorker : Napi::AsyncWorker
    {
    private:
        NativeDgnDb& m_nativeDgnDb;
        BeFileNameCR m_dbname;
        bvector<DgnRevisionCP>& m_revisions;
        RevisionProcessOption m_applyOption;
        IConcurrencyControl* m_concurrencyControl;
        RevisionStatus m_status;
        JsInterop::ObjectReferenceClaimCheck m_requestContext;
        Napi::ObjectReference m_applyChangeSetsRequest; // Note: This holds a reference to ApplyChangeSetsRequest instance!

        void OnComplete()
            {
            ApplyChangeSetsRequest* request = ApplyChangeSetsRequest::Unwrap(m_applyChangeSetsRequest.Value());
            request->m_pending = nullptr;
            }

        void Execute() override
            {
            JsInterop::SetCurrentClientRequestContextForWorkerThread(m_requestContext);
            NativeLogging::LoggingManager::GetLogger("DgnCore")->info("ApplyChangeSetsAsyncWorker: In worker thread");

            // Apply change sets
            if (m_nativeDgnDb.IsOpen())
                m_status = JsInterop::ApplyDataChangeSets(m_nativeDgnDb.GetDgnDb(), m_revisions, m_applyOption);
            else
                m_status = JsInterop::ApplySchemaChangeSets(m_dbname, m_revisions, m_applyOption, m_concurrencyControl);
            }

        void OnOK() override
            {
            OnComplete();

            // Warning! If you want to log in a specific ClientRequestContext, then you can't use NativeLogging directly. You must call this helper function:
            JsInterop::LogMessageInContext("DgnCore", NativeLogging::LOG_INFO, "ApplyChangeSetsAsyncWorker: Back on main thread", m_requestContext);
            m_requestContext.Dispose();

            auto retval = Napi::Number::New(Env(), (int)m_status);
            Callback().MakeCallback(Receiver().Value(), {retval});
            }

    public:
        ApplyChangeSetsAsyncWorker(Napi::Function &callback, ApplyChangeSetsRequest const& request, NativeDgnDb& nativeDgnDb, BeFileNameCR dbname, bvector<DgnRevisionCP>& revisions, RevisionProcessOption applyOption, IConcurrencyControl* concurrencyControl)
            : Napi::AsyncWorker(callback), m_nativeDgnDb(nativeDgnDb), m_dbname(dbname), m_revisions(revisions), m_applyOption(applyOption), m_concurrencyControl(concurrencyControl)
            {
            m_applyChangeSetsRequest.Reset(request.Value(), 1);

            m_requestContext = JsInterop::GetCurrentClientRequestContextForMainThread();
            NativeLogging::LoggingManager::GetLogger("DgnCore")->info("ApplyChangeSetsAsyncWorker: Start on main thread");
            }

        ~ApplyChangeSetsAsyncWorker() {m_applyChangeSetsRequest.Reset();}
    };

private:
    DEFINE_CONSTRUCTOR;

    NativeDgnDb* m_nativeDgnDb;
    ApplyChangeSetsAsyncWorker *m_pending = nullptr;
    bvector<DgnRevisionPtr> m_revisionPtrs;
    bvector<DgnRevisionCP> m_revisions;
    bool m_containsSchemaChanges;
    BeFileName m_dbname;
    Napi::ObjectReference m_jsIModelDb;
    RefCountedPtr<IConcurrencyControl> m_concurrencyControl;

    /** Reads the change sets to be applied asynchronously */
    Napi::Value ReadChangeSets(Napi::CallbackInfo const &info)
        {
        REQUIRE_ARGUMENT_STRING(0, changeSetTokensStr, Napi::Number::New(Env(), (int) RevisionStatus::ApplyError));
        Json::Value changeSetTokens = Json::Value::From(changeSetTokensStr);
        if (!m_nativeDgnDb || !m_nativeDgnDb->IsOpen())
            THROW_JS_TYPE_ERROR("Briefcase must be open to ReadChangeSets");

        Utf8String dbGuid = m_nativeDgnDb->GetDgnDb().GetDbGuid().ToString();
        RevisionStatus status = JsInterop::ReadChangeSets(m_revisionPtrs, m_containsSchemaChanges, dbGuid, changeSetTokens);
        if (RevisionStatus::Success == status)
            {
            m_revisions.clear();
            for (uint32_t ii = 0; ii < m_revisionPtrs.size(); ii++)
                m_revisions.push_back(m_revisionPtrs[ii].get());
            }

        return Napi::Number::New(Env(), (int) status);
        }

    /** Returns true if any of the change sets that were read contain schema changes */
    Napi::Value ContainsSchemaChanges(Napi::CallbackInfo const &info)
        {
        if (!m_nativeDgnDb)
            THROW_JS_TYPE_ERROR("Briefcase must be setup when applying change sets");
        return Napi::Number::New(Env(), m_containsSchemaChanges);
        }

    static void CloseBriefcaseLow(BeFileNameR dbname,  Napi::ObjectReference& jsIModelDb, RefCountedPtr<IConcurrencyControl> &concurrencyControl, NativeDgnDb& nativeDgnDb)
        {
        DgnDbR dgndb = nativeDgnDb.GetDgnDb();
        dbname = BeFileName(dgndb.GetDbFileName(), true);
        jsIModelDb = std::move(dgndb.m_jsIModelDb); // Moves the reference
        concurrencyControl = dgndb.GetConcurrencyControl();
        nativeDgnDb.CloseDgnDb();
        }

    /**
     * Close the briefcase, backing up any state that are required at/after open
     * - needs to be done before applying change sets asynchronously
     */
    void CloseBriefcase(Napi::CallbackInfo const &info)
        {
        if (!m_nativeDgnDb || !m_nativeDgnDb->IsOpen())
            THROW_JS_TYPE_ERROR("Briefcase not open");
        CloseBriefcaseLow(m_dbname, m_jsIModelDb, m_concurrencyControl, *m_nativeDgnDb);
        }

    /**
     * Apply change sets asynchronously
     * Notes:
     * - the Db must be closed prior to this call (using the CloseBriefcase call in this utility)
     * - ReadChangeSets must have been called prior to this call
     * - the Db must be reopened after this call (using the OpenBriefcase call in this utility)
     */
    void DoApplyAsync(Napi::CallbackInfo const &info)
        {
        REQUIRE_ARGUMENT_FUNCTION(0, responseCallback, );
        REQUIRE_ARGUMENT_INTEGER(1, applyOption, );

        if (!m_nativeDgnDb)
            THROW_JS_TYPE_ERROR("Briefcase must be setup when applying change sets");
        if (m_nativeDgnDb->IsOpen())
            THROW_JS_TYPE_ERROR("Briefcase must be closed when applying change sets asynchrnously");

        m_pending = new ApplyChangeSetsAsyncWorker(responseCallback, *this, *m_nativeDgnDb, m_dbname, m_revisions, (RevisionProcessOption) applyOption, m_concurrencyControl.get()); // freed in caller of OnOK and OnError see AsyncWorker::OnWorkComplete
        m_pending->Queue();  // ApplyChanges happens in another thread
        }

    static DbResult ReopenBriefcaseLow(NativeDgnDb& nativeDgnDb, Db::OpenMode openMode, BeFileNameCR dbname, Napi::ObjectReference& jsIModelDb, IConcurrencyControl* concurrencyControl)
        {
        SchemaUpgradeOptions schemaUpgradeOptions(SchemaUpgradeOptions::DomainUpgradeOptions::CheckRequiredUpgrades);
        DgnDb::OpenParams openParams(openMode, BeSQLite::DefaultTxn::Yes, schemaUpgradeOptions);
        DbResult result = nativeDgnDb.OpenDgnDb(dbname, openParams);
        if (BE_SQLITE_OK == result)
            {
            DgnDbR dgndb = nativeDgnDb.GetDgnDb();
            dgndb.m_jsIModelDb = std::move(jsIModelDb); // Moves the references
            dgndb.SetConcurrencyControl(concurrencyControl);
            }
        return result;
        }

    /**
     * Reopen the briefcase, restoring any backed up state
     * - needs to be done after applying change sets asynchronously.
     */
    Napi::Value ReopenBriefcase(Napi::CallbackInfo const &info)
        {
        if (!m_nativeDgnDb)
            THROW_JS_TYPE_ERROR("Briefcase must be setup when applying change sets");
        if (m_nativeDgnDb->IsOpen())
            THROW_JS_TYPE_ERROR("Briefcase is already open when ReopenBriefcase is called");

        REQUIRE_ARGUMENT_INTEGER(0, openMode, Env().Undefined());

        DbResult result = ReopenBriefcaseLow(*m_nativeDgnDb, (Db::OpenMode) openMode, m_dbname, m_jsIModelDb, m_concurrencyControl.get());
        return Napi::Number::New(Env(), (int) result);
        }

    /**
     * Apply change sets synchronously
     *  Notes:
     *  - the briefcase must be open
     *  - causes the briefcase to be closed and reopened *if* the change set contains schema changes
     *  - the change sets should not be to large to cause a potential timeout since the operation blocks the main thread
     */
    static Napi::Value DoApplySync(Napi::CallbackInfo const &info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, nativeDgnDb, Napi::Number::New(info.Env(), (int) RevisionStatus::ApplyError));
        REQUIRE_ARGUMENT_STRING(1, changeSetTokensStr, Napi::Number::New(info.Env(), (int) RevisionStatus::ApplyError));
        REQUIRE_ARGUMENT_INTEGER(2, applyOption, Napi::Number::New(info.Env(), (int) RevisionStatus::ApplyError));

        if (!nativeDgnDb)
            THROW_JS_TYPE_ERROR("A valid briefcase must be passed in when applying change sets");
        if (!nativeDgnDb->IsOpen())
            THROW_JS_TYPE_ERROR("Briefcase must be open when applying a change set synchronously");
        DgnDbR dgndb = nativeDgnDb->GetDgnDb();

        bvector<DgnRevisionPtr> revisionPtrs;
        bool containsSchemaChanges;
        Utf8String dbGuid = dgndb.GetDbGuid().ToString();
        Json::Value changeSetTokens = Json::Value::From(changeSetTokensStr);
        RevisionStatus status = JsInterop::ReadChangeSets(revisionPtrs, containsSchemaChanges, dbGuid, changeSetTokens);
        if (RevisionStatus::Success != status)
            return Napi::Number::New(info.Env(), (int)status);

        bvector<DgnRevisionCP> revisions;
        for (uint32_t ii = 0; ii < revisionPtrs.size(); ii++)
            revisions.push_back(revisionPtrs[ii].get());

        /* Process change sets containing only data changes */
        if (!containsSchemaChanges)
            {
            status = JsInterop::ApplyDataChangeSets(dgndb, revisions, (RevisionProcessOption) applyOption);
            return Napi::Number::New(info.Env(), (int)status);
            }

        /* Process change sets containing schema changes */
        BeFileName dbname;
        Napi::ObjectReference jsIModelDb;
        RefCountedPtr<IConcurrencyControl> concurrencyControl;
        bool isReadonly = nativeDgnDb->GetDgnDb().IsReadonly();

        CloseBriefcaseLow(dbname, jsIModelDb, concurrencyControl, *nativeDgnDb);

        status = JsInterop::ApplySchemaChangeSets(dbname, revisions, (RevisionProcessOption)applyOption, concurrencyControl.get());
        if (RevisionStatus::Success != status)
            return Napi::Number::New(info.Env(), (int)status);

        Db::OpenMode openMode = isReadonly ? Db::OpenMode::Readonly : Db::OpenMode::ReadWrite;
        DbResult result = ReopenBriefcaseLow(*nativeDgnDb, openMode, dbname, jsIModelDb, concurrencyControl.get());
        if (BE_SQLITE_OK != result)
            status = RevisionStatus::ApplyError;

        return Napi::Number::New(info.Env(), (int)status);
        }

public:
    ApplyChangeSetsRequest(Napi::CallbackInfo const& info) : BeObjectWrap<ApplyChangeSetsRequest>(info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, nativeDgnDb, );
        m_nativeDgnDb = nativeDgnDb;
        }

    ~ApplyChangeSetsRequest() { SetInDestructor(); }

    static void Init(Napi::Env env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ApplyChangeSetsRequest", {
            InstanceMethod("readChangeSets", &ApplyChangeSetsRequest::ReadChangeSets),
            InstanceMethod("containsSchemaChanges", &ApplyChangeSetsRequest::ContainsSchemaChanges),
            InstanceMethod("closeBriefcase", &ApplyChangeSetsRequest::CloseBriefcase),
            InstanceMethod("reopenBriefcase", &ApplyChangeSetsRequest::ReopenBriefcase),
            InstanceMethod("doApplyAsync", &ApplyChangeSetsRequest::DoApplyAsync),
            StaticMethod("doApplySync", &ApplyChangeSetsRequest::DoApplySync),
        });
        exports.Set("ApplyChangeSetsRequest", t);
        SET_CONSTRUCTOR(t)
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
    JsInterop::ObjectReferenceClaimCheck m_requestContext;

    void Execute() override
        {
        JsInterop::SetCurrentClientRequestContextForWorkerThread(m_requestContext);

        NativeLogging::LoggingManager::GetLogger("ECSqlStepWorkerTestCategory")->error("ECSqlStepWorker: In worker thread");

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
        // Warning! If you want to log in a specific ClientRequestContext, then you can't use NativeLogging directly. You must call this helper function:
        JsInterop::LogMessageInContext("ECSqlStepWorkerTestCategory", NativeLogging::LOG_ERROR, "ECSqlStepWorker: Back on main thread", m_requestContext);
        m_requestContext.Dispose();

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
        m_requestContext = JsInterop::GetCurrentClientRequestContextForMainThread();
        NativeLogging::LoggingManager::GetLogger("ECSqlStepWorkerTestCategory")->error("ECSqlStepWorker: Start on main thread");
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
struct SnapRequest : BeObjectWrap<SnapRequest>
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

    SnapRequest(Napi::CallbackInfo const& info) : BeObjectWrap<SnapRequest>(info) {}
    ~SnapRequest() {SetInDestructor();}

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
// @bsistruct                                     Evan.Preslar                    05/2019
//=======================================================================================
struct NativeUlasClient : BeObjectWrap<NativeUlasClient>
    {
    private:
        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Krischan.Eberle                    12/18
        //+---------------+---------------+---------------+---------------+---------------+------
        static void InitializeRegion(Napi::CallbackInfo const& info)
            {
            Region region = Region::Prod;
            if (info.Length() != 1 || !info[0].IsNumber())
                JsInterop::GetLogger().error("Invalid region. Using PROD as fallback.");
            else
                region = (Region) (int) info[0].As<Napi::Number>().Int32Value();

            UlasClient::Get().Initialize(region);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Zain.Ulabidin                    11/2019
        //+---------------+---------------+---------------+---------------+---------------+------
        static Utf8String GetUsageType(Licensing::UsageType usageType)
            {
            switch (usageType)
                {
                case Licensing::UsageType::Production:
                    return "Production";

                case Licensing::UsageType::Trial:
                    return "Trial";

                case Licensing::UsageType::HomeUse:
                    return "HomeUse";

                case Licensing::UsageType::PreActivation:
                    return "PreActivation";

                case Licensing::UsageType::Evaluation:
                    return "Evaluation";

                case Licensing::UsageType::Academic:
                    return "Academic";
                }
                return "Not Defined";
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Zain.Ulabidin                    11/2019
        //+---------------+---------------+---------------+---------------+---------------+------
        static Napi::Value CheckEntitlement(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, accessToken, info.Env().Undefined());
            REQUIRE_ARGUMENT_STRING(1, appVersionStr, info.Env().Undefined());
            BeVersion appVersion = BeVersion(appVersionStr.c_str());
            REQUIRE_ARGUMENT_STRING(2, projectId, info.Env().Undefined());
            OPTIONAL_ARGUMENT_INTEGER(3, authType, (int)Licensing::AuthType::OIDC, info.Env().Undefined());
            OPTIONAL_ARGUMENT_INTEGER(4, productId, -1, info.Env().Undefined());
            OPTIONAL_ARGUMENT_STRING(5, deviceId, info.Env().Undefined());
            OPTIONAL_ARGUMENT_STRING(6, correlationId, info.Env().Undefined());

            Licensing::EntitlementResult entitlementResult;
            BentleyStatus status = UlasClient::Get().CheckEntitlement(accessToken, appVersion, projectId, (Licensing::AuthType) authType, productId, deviceId, correlationId, entitlementResult);
            Napi::Object jsNavValue = Napi::Object::New(info.Env());

            if (status == BentleyStatus::ERROR)
                THROW_TYPE_EXCEPTION_AND_RETURN("Could not validate entitlements", info.Env().Undefined());

            Utf8StringCR usageType = GetUsageType(entitlementResult.usageType);
            jsNavValue.Set("allowed", Napi::Boolean::New(info.Env(), entitlementResult.allowed));
            jsNavValue.Set("principalId", Napi::String::New(info.Env(), entitlementResult.principalId.c_str()));
            jsNavValue.Set("usageType", Napi::String::New(info.Env(), usageType.c_str()));

            return jsNavValue;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Evan.Preslar                    03/2020
        // @deprecated use PostUserUsage instead
        //+---------------+---------------+---------------+---------------+---------------+------
        static Napi::Value TrackUsage(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, accessToken, info.Env().Undefined());
            REQUIRE_ARGUMENT_STRING(1, appVersionStr, info.Env().Undefined());
            BeVersion appVersion = BeVersion(appVersionStr.c_str());
            REQUIRE_ARGUMENT_STRING(2, projectId, info.Env().Undefined());
            OPTIONAL_ARGUMENT_INTEGER(3, authType, (int) Licensing::AuthType::OIDC, info.Env().Undefined());
            OPTIONAL_ARGUMENT_INTEGER(4, productId, -1, info.Env().Undefined());
            OPTIONAL_ARGUMENT_STRING(5, deviceId, info.Env().Undefined());
            OPTIONAL_ARGUMENT_INTEGER(6, usageType, (int) Licensing::UsageType::Production, info.Env().Undefined());
            OPTIONAL_ARGUMENT_STRING(7, correlationId, info.Env().Undefined());

            BentleyStatus status = UlasClient::Get().TrackUsage(accessToken, appVersion, projectId, (Licensing::AuthType) authType, productId, deviceId, (Licensing::UsageType) usageType, correlationId).get();
            if (status == BentleyStatus::ERROR)
                THROW_TYPE_EXCEPTION_AND_RETURN("Could not log usage information", info.Env().Undefined());

            return Napi::Number::New(info.Env(), (int)BentleyStatus::SUCCESS);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Evan.Preslar                    03/2020
        //+---------------+---------------+---------------+---------------+---------------+------
        static Napi::Value PostUserUsage(Napi::CallbackInfo const& info)
            {
            Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());

            REQUIRE_ARGUMENT_STRING_ASYNC(0, accessToken, deferred);
            REQUIRE_ARGUMENT_STRING_ASYNC(1, appVersionStr, deferred);
            BeVersion appVersion = BeVersion(appVersionStr.c_str());
            REQUIRE_ARGUMENT_STRING_ASYNC(2, projectId, deferred);
            OPTIONAL_ARGUMENT_INTEGER_ASYNC(3, authType, (int) Licensing::AuthType::OIDC, deferred);
            OPTIONAL_ARGUMENT_INTEGER_ASYNC(4, productId, -1, deferred);
            OPTIONAL_ARGUMENT_STRING_ASYNC(5, deviceId, deferred);
            OPTIONAL_ARGUMENT_INTEGER_ASYNC(6, usageType, (int) Licensing::UsageType::Production, deferred);
            OPTIONAL_ARGUMENT_STRING_ASYNC(7, correlationId, deferred);
            OPTIONAL_ARGUMENT_STRING_ASYNC(8, principalId, deferred);

            Utf8String errorMessage = "";
            try
                {
                // TODO: find a way to avoid calling get() (which blocks the thread) and instead incorporate a .then() for the future, return the Napi promise immediately, and resolve/reject it later.
                UlasClient::Get().PostUserUsage(accessToken, appVersion, projectId, (Licensing::AuthType) authType, productId, deviceId, (Licensing::UsageType) usageType, correlationId, principalId).get();
                deferred.Resolve(info.Env().Undefined());
                }
            catch (const Http::HttpError& err)
                {
                errorMessage.Sprintf("Could not send user usage: request was rejected: %s", err.GetMessage().c_str());
                }
            catch (const std::exception err)
                {
                errorMessage.Sprintf("Could not send user usage: %s", err.what());
                }
            catch (...)
                {
                errorMessage = "Could not send user usage: internal error";
                }

            if (errorMessage != "")
                {
                REJECT_DEFERRED_AND_RETURN(deferred, errorMessage.c_str())
                }

            return deferred.Promise();
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Evan.Preslar                    08/2019
        // @deprecated use PostFeatureUsage instead
        //+---------------+---------------+---------------+---------------+---------------+------
        static Napi::Value MarkFeature(Napi::CallbackInfo const& info)
            {
            REQUIRE_ARGUMENT_STRING(0, accessToken, info.Env().Undefined());

            REQUIRE_ARGUMENT_ANY_OBJ(1, featureEventObj, info.Env().Undefined());
            if (!featureEventObj.Has("featureId"))
                THROW_TYPE_EXCEPTION_AND_RETURN("Could not track feature information: featureId must be specified", info.Env().Undefined());

            Utf8String featureId = featureEventObj.Get("featureId").ToString().Utf8Value().c_str();

            if (!featureEventObj.Has("versionStr"))
                THROW_TYPE_EXCEPTION_AND_RETURN("Could not track feature information: version must be specified", info.Env().Undefined());

            BeVersion appVersion = BeVersion(featureEventObj.Get("versionStr").ToString().Utf8Value().c_str());

            Utf8String projectId;
            if (featureEventObj.Has("projectId"))
                {
                Utf8String projectIdString(featureEventObj.Get("projectId").ToString().Utf8Value().c_str());
                if(BentleyStatus::SUCCESS != BeSQLite::BeGuid(false).FromString(projectIdString.c_str()))
                    projectId = Utf8String("00000000-0000-0000-0000-000000000000");
                else
                    projectId = projectIdString;
                }

            Licensing::FeatureUserDataMapPtr userFeatureData = std::make_shared<Licensing::FeatureUserDataMap>();
            if (featureEventObj.Has("featureUserData"))
              {
              Napi::Array arr = featureEventObj.Get("featureUserData").As<Napi::Array>();
              for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex)
                {
                Napi::Value arrValue = arr[arrIndex];
                if (arrValue.IsObject())
                  {
                  Napi::Object item = arrValue.As<Napi::Object>();
                  if (!item.Has("value"))
                    {
                    JsInterop::GetLogger().error("Invalid feature tracking data: userFeatureData entry is missing the field: value. Omitting this field from the feature tracking request.");
                    continue;
                    }

                  Utf8String key = getOptionalStringProperty(item, "key", "");
                  Utf8String value = getOptionalStringProperty(item, "value", "");

                  if (key.empty())
                    {
                    JsInterop::GetLogger().error("Invalid feature tracking data: userFeatureData entry has an empty or invalid field: key. Omitting this field from the feature tracking request.");
                    continue;
                    }
                  userFeatureData->AddAttribute(Utf8String(key.c_str()), Utf8String(value.c_str()));
                  }
                else
                  {
                  JsInterop::GetLogger().error("Invalid feature tracking data: userFeatureData entry is not a key-value pair. Omitting this field from the feature tracking request.");
                  continue;
                  }
                }
              }

            Licensing::FeatureEvent featureEvent = Licensing::FeatureEvent(featureId, appVersion, projectId, userFeatureData);

            OPTIONAL_ARGUMENT_INTEGER(2, authType, (int) Licensing::AuthType::OIDC, info.Env().Undefined());
            OPTIONAL_ARGUMENT_INTEGER(3, productId, -1, info.Env().Undefined());
            OPTIONAL_ARGUMENT_STRING(4, deviceId, info.Env().Undefined());
            OPTIONAL_ARGUMENT_INTEGER(5, usageType, (int) Licensing::UsageType::Production, info.Env().Undefined());
            OPTIONAL_ARGUMENT_STRING(6, correlationId, info.Env().Undefined());

            BentleyStatus status = UlasClient::Get().MarkFeature(accessToken, featureEvent, (Licensing::AuthType) authType, productId, deviceId, (Licensing::UsageType) usageType, correlationId).get();
            if (status == BentleyStatus::ERROR)
                THROW_TYPE_EXCEPTION_AND_RETURN("Could not track feature information", info.Env().Undefined());

            return Napi::Number::New(info.Env(), (int)BentleyStatus::SUCCESS);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Evan.Preslar                    03/2020
        //+---------------+---------------+---------------+---------------+---------------+------
        static Napi::Value PostFeatureUsage(Napi::CallbackInfo const& info)
            {
            Napi::Promise::Deferred deferred = Napi::Promise::Deferred::New(info.Env());

            REQUIRE_ARGUMENT_STRING_ASYNC(0, accessToken, deferred);

            REQUIRE_ARGUMENT_ANY_OBJ_ASYNC(1, featureEventObj, deferred);
            if (!featureEventObj.Has("featureId"))
                REJECT_DEFERRED_AND_RETURN(deferred, "Could not track feature information: featureId must be specified");

            Utf8String featureId = featureEventObj.Get("featureId").ToString().Utf8Value().c_str();

            if (!featureEventObj.Has("versionStr"))
                REJECT_DEFERRED_AND_RETURN(deferred, "Could not track feature information: version must be specified");

            BeVersion appVersion = BeVersion(featureEventObj.Get("versionStr").ToString().Utf8Value().c_str());

            Utf8String projectId = "99999999-9999-9999-9999-999999999999"; // All iTwin applications are obligated by ULAS to send the 9-Guid (instead of the 0-Guid or undefined) to denote a global project scope.
            if (featureEventObj.Has("projectId"))
                {
                Napi::Value projectIdValue = featureEventObj.Get("projectId");
                if (!projectIdValue.IsUndefined() && !projectIdValue.IsNull())
                    {
                    Utf8String projectIdString(projectIdValue.ToString().Utf8Value().c_str());
                    if(BentleyStatus::SUCCESS != BeSQLite::BeGuid(false).FromString(projectIdString.c_str()))
                        JsInterop::GetLogger().warningv("Invalid feature tracking data: projectId: %s is not resolvable to a valid Guid. Feature request will continue as if no projectId was specified.",  projectIdString.c_str());
                    else
                        projectId = projectIdString;
                    }
                }

            DateTime startDateZ;
            if (featureEventObj.Has("startDateZ"))
                {
                Napi::Value startDateZValue = featureEventObj.Get("startDateZ");
                if (!startDateZValue.IsUndefined() && !startDateZValue.IsNull())
                    {
                    Utf8String startDateZString(startDateZValue.ToString().Utf8Value().c_str());
                    if (BentleyStatus::SUCCESS != DateTime::FromString(startDateZ, startDateZString.c_str()))
                        JsInterop::GetLogger().warningv("Invalid feature tracking data: startDateZ: %s is not resolvable to a valid DateTime. Feature request will continue as if no startDateZ was specified.", startDateZString.c_str());
                    }
                }

            DateTime endDateZ;
            if (featureEventObj.Has("endDateZ"))
                {
                Napi::Value endDateZValue = featureEventObj.Get("endDateZ");
                if (!endDateZValue.IsUndefined() && !endDateZValue.IsNull())
                    {
                    Utf8String endDateZString(endDateZValue.ToString().Utf8Value().c_str());
                    if (BentleyStatus::SUCCESS != DateTime::FromString(endDateZ, endDateZString.c_str()))
                        JsInterop::GetLogger().warningv("Invalid feature tracking data: endDateZ: %s is not resolvable to a valid DateTime. Feature request will continue as if no endDateZ was specified.", endDateZString.c_str());
                    }
                }

            Licensing::FeatureUserDataMapPtr userFeatureData = std::make_shared<Licensing::FeatureUserDataMap>();
            if (featureEventObj.Has("featureUserData"))
              {
              Napi::Array arr = featureEventObj.Get("featureUserData").As<Napi::Array>();
              for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex)
                {
                Napi::Value arrValue = arr[arrIndex];
                if (arrValue.IsObject())
                  {
                  Napi::Object item = arrValue.As<Napi::Object>();
                  if (!item.Has("value"))
                    {
                    JsInterop::GetLogger().warning("Invalid feature tracking data: userFeatureData entry is missing the field: value. Omitting this field from the feature tracking request.");
                    continue;
                    }

                  Utf8String key = getOptionalStringProperty(item, "key", "");
                  Utf8String value = getOptionalStringProperty(item, "value", "");

                  if (key.empty())
                    {
                    JsInterop::GetLogger().warning("Invalid feature tracking data: userFeatureData entry has an empty or invalid field: key. Omitting this field from the feature tracking request.");
                    continue;
                    }
                  userFeatureData->AddAttribute(Utf8String(key.c_str()), Utf8String(value.c_str()));
                  }
                else
                  {
                  JsInterop::GetLogger().warning("Invalid feature tracking data: userFeatureData entry is not a key-value pair. Omitting this field from the feature tracking request.");
                  continue;
                  }
                }
              }

            Licensing::FeatureEvent featureEvent = startDateZ.IsValid() && endDateZ.IsValid()
                ? Licensing::FeatureEvent(featureId, appVersion, projectId, startDateZ, endDateZ, userFeatureData)
                : Licensing::FeatureEvent(featureId, appVersion, projectId, userFeatureData);

            OPTIONAL_ARGUMENT_INTEGER_ASYNC(2, authType, (int) Licensing::AuthType::OIDC, deferred);
            OPTIONAL_ARGUMENT_INTEGER_ASYNC(3, productId, -1, deferred);
            OPTIONAL_ARGUMENT_STRING_ASYNC(4, deviceId, deferred);
            OPTIONAL_ARGUMENT_INTEGER_ASYNC(5, usageType, (int) Licensing::UsageType::Production, deferred);
            OPTIONAL_ARGUMENT_STRING_ASYNC(6, correlationId, deferred);
            OPTIONAL_ARGUMENT_STRING_ASYNC(7, principalId, deferred);

            Utf8String errorMessage = "";
            try
                {
                // TODO: find a way to avoid calling get() (which blocks the thread) and instead incorporate a .then() for the future, return the Napi promise immediately, and resolve/reject it later.
                UlasClient::Get().PostFeatureUsage(accessToken, featureEvent, (Licensing::AuthType) authType, productId, deviceId, (Licensing::UsageType) usageType, correlationId, principalId).get();
                deferred.Resolve(info.Env().Undefined());
                }
            catch (const Http::HttpError& err)
                {
                errorMessage.Sprintf("Could not send feature usage: request was rejected: %s", err.GetMessage().c_str());
                }
            catch (const std::exception err)
                {
                errorMessage.Sprintf("Could not send feature usage: %s", err.what());
                }
            catch (...)
                {
                errorMessage = "Could not send feature usage: internal error";
                }

            if (errorMessage != "")
                {
                REJECT_DEFERRED_AND_RETURN(deferred, errorMessage.c_str())
                }

            return deferred.Promise();
            }
    public:
        NativeUlasClient(Napi::CallbackInfo const &info) : BeObjectWrap<NativeUlasClient>(info) {}
        ~NativeUlasClient() {SetInDestructor();}

        //---------------------------------------------------------------------------------------
        // @bsimethod                                     Evan.Preslar                    05/2019
        //+---------------+---------------+---------------+---------------+---------------+------
        static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "NativeUlasClient", {
            StaticMethod("initializeRegion", &NativeUlasClient::InitializeRegion),
            StaticMethod("trackUsage", &NativeUlasClient::TrackUsage),
            StaticMethod("postUserUsage", &NativeUlasClient::PostUserUsage),
            StaticMethod("markFeature", &NativeUlasClient::MarkFeature),
            StaticMethod("postFeatureUsage", &NativeUlasClient::PostFeatureUsage),
            StaticMethod("checkEntitlement", &NativeUlasClient::CheckEntitlement),
        });
        exports.Set("NativeUlasClient", t);
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

        treeId = Tile::Tree::Id::FromString(idStr, &db);
        if (!treeId.IsValid())
            return DgnDbStatus::InvalidId;

        model = db.Models().Get<GeometricModel>(treeId.GetModelId());
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
        m_result = tree.IsValid() ? tree->RequestContent(m_contentId, JsInterop::GetUseTileCache()) : nullptr;
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
        if (DgnDbStatus::Success != status)
            return status;

        if (!contentId.FromString(contentIdStr.c_str(), treeId.GetMajorVersion()))
            return DgnDbStatus::InvalidId;

        if (!treeId.IsValidContentId(contentId))
            return DgnDbStatus::InvalidId;

        return DgnDbStatus::Success;
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
struct NativeECPresentationManager : BeObjectWrap<NativeECPresentationManager>
    {
    //=======================================================================================
    // Async Worker which sends ECPresentationResult via callback
    //! @bsiclass
    //=======================================================================================
    struct ResponseSender : Napi::AsyncWorker
    {
        struct BoolPredicate : IConditionVariablePredicate
            {
            bool& m_flag;
            BoolPredicate(bool& flag) : m_flag(flag) {}
            bool _TestCondition(BeConditionVariable&) override {return m_flag;}
            };
    private:
        JsInterop::ObjectReferenceClaimCheck m_requestContext;
        BeConditionVariable m_waiter;
        ECPresentationResult m_result;
        bool m_hasResult;
        uint64_t m_startTime;
    protected:
        void Execute() override
            {
            BoolPredicate pred(m_hasResult);
            m_waiter.WaitOnCondition(&pred, BeConditionVariable::Infinite);
            }
        void OnOK() override
            {
            JsInterop::LogMessageInContext("ECPresentation.Node", NativeLogging::LOG_DEBUG, Utf8PrintfString("Sending success response (took %" PRIu64 " ms)",
                (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_startTime)), m_requestContext);
            Callback().MakeCallback(Receiver().Value(), {CreateReturnValue(Env(), m_result, true)});
            }
        void OnError(Napi::Error const& e) override
            {
            JsInterop::LogMessageInContext("ECPresentation.Node", NativeLogging::LOG_DEBUG, "Sending error response", m_requestContext);
            Callback().MakeCallback(Receiver().Value(), {CreateReturnValue(Env(), ECPresentationResult(ECPresentationStatus::Error, "callback error"))});
            }
    public:
        ResponseSender(Napi::Function& callback)
            : Napi::AsyncWorker(callback), m_requestContext(JsInterop::GetCurrentClientRequestContextForMainThread()), m_hasResult(false), m_startTime(BeTimeUtilities::GetCurrentTimeAsUnixMillis())
            {}
        void SetResult(ECPresentationResult&& result)
            {
            BeMutexHolder lock(m_waiter.GetMutex());
            m_result = std::move(result);
            m_hasResult = true;
            m_waiter.notify_all();
            }
    };

    //=======================================================================================
    //! @bsiclass
    //=======================================================================================
    struct SchemasLoader : Napi::AsyncWorker
        {
        private:
            JsInterop::ObjectReferenceClaimCheck m_requestContext;
            DgnDbPtr m_dgndb;
            uint64_t m_startTime;
        protected:
            void Execute() override
                {
                m_startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
                JsInterop::SetCurrentClientRequestContextForWorkerThread(m_requestContext);
                if (!m_dgndb->IsDbOpen())
                    {
                    SetError("iModel not open");
                    return;
                    }
                NativeLogging::LoggingManager::GetLogger("ECPresentation.Node")->info("Preloading ECSchemas");
                m_dgndb->Schemas().GetSchemas(true);
                }
            void OnOK() override
                {
                JsInterop::LogMessageInContext("ECPresentation.Node", NativeLogging::LOG_DEBUG, Utf8PrintfString("Preloading ECSchemas took %" PRIu64 " ms",
                    (BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_startTime)), m_requestContext);
                Callback().MakeCallback(Receiver().Value(), { Napi::Number::New(Env(), (int)ECPresentationStatus::Success) });
                }
            void OnError(Napi::Error const& e) override
                {
                JsInterop::LogMessageInContext("ECPresentation.Node", NativeLogging::LOG_ERROR, "Error preloading ECSchemas", m_requestContext);
                Callback().MakeCallback(Receiver().Value(), { Napi::Number::New(Env(), (int)ECPresentationStatus::Error) });
                }
        public:
            SchemasLoader(Napi::Function& callback, DgnDb& db)
                : Napi::AsyncWorker(callback), m_dgndb(&db), m_requestContext(JsInterop::GetCurrentClientRequestContextForMainThread()), m_startTime(0)
                {}
        };

    DEFINE_CONSTRUCTOR;

    std::unique_ptr<RulesDrivenECPresentationManager> m_presentationManager;
    RefCountedPtr<SimpleRuleSetLocater> m_ruleSetLocater;
    RuntimeJsonLocalState m_localState;
    std::shared_ptr<IModelJsECPresentationUpdateRecordsHandler> m_updateRecords;

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
            InstanceMethod("forceLoadSchemas", &NativeECPresentationManager::ForceLoadSchemas),
            InstanceMethod("setupRulesetDirectories", &NativeECPresentationManager::SetupRulesetDirectories),
            InstanceMethod("setupSupplementalRulesetDirectories", &NativeECPresentationManager::SetupSupplementalRulesetDirectories),
            InstanceMethod("setRulesetVariableValue", &NativeECPresentationManager::SetRulesetVariableValue),
            InstanceMethod("getRulesetVariableValue", &NativeECPresentationManager::GetRulesetVariableValue),
            InstanceMethod("getRulesets", &NativeECPresentationManager::GetRulesets),
            InstanceMethod("addRuleset", &NativeECPresentationManager::AddRuleset),
            InstanceMethod("removeRuleset", &NativeECPresentationManager::RemoveRuleset),
            InstanceMethod("clearRulesets", &NativeECPresentationManager::ClearRulesets),
            InstanceMethod("handleRequest", &NativeECPresentationManager::HandleRequest),
            InstanceMethod("getUpdateInfo", &NativeECPresentationManager::GetUpdateInfo),
            InstanceMethod("dispose", &NativeECPresentationManager::Terminate)
        });

        exports.Set("ECPresentationManager", t);

        SET_CONSTRUCTOR(t);
        }

    static bool ParseInt(int& value, Napi::Value const& jsValue)
        {
        if (jsValue.IsNumber())
            {
            value = jsValue.ToNumber().Int32Value();
            return true;
            }
        if (jsValue.IsString())
            {
            value = atoi(jsValue.ToString().Utf8Value().c_str());
            return true;
            }
        return false;
        }

    static bmap<int, unsigned> CreateTaskAllocationSlotsMap(Napi::Object& jsMap)
        {
        bmap<int, unsigned> slots;
        Napi::Array jsMemberNames = jsMap.GetPropertyNames();
        for (uint32_t i = 0; i < jsMemberNames.Length(); ++i) {
            Napi::Value const& jsMemberName = jsMemberNames[i];
            Napi::Value const& jsMemberValue = jsMap.Get(jsMemberName);
            int priority, slotsCount;
            if (ParseInt(priority, jsMemberName) && ParseInt(slotsCount, jsMemberValue))
                slots.Insert(priority, slotsCount);
            }
        if (slots.empty())
            slots.Insert(INT_MAX, 1);
        return slots;
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

    NativeECPresentationManager(Napi::CallbackInfo const& info)
        : BeObjectWrap<NativeECPresentationManager>(info)
        {
        REQUIRE_ARGUMENT_STRING(0, id, );
        REQUIRE_ARGUMENT_STRING_ARRAY(1, localeDirectories, );
        REQUIRE_ARGUMENT_ANY_OBJ(2, taskAllocationSlots, );
        REQUIRE_ARGUMENT_STRING(3, mode, );
        REQUIRE_ARGUMENT_BOOL(4, isChangeTrackingEnabled, );
        REQUIRE_ARGUMENT_STRING(5, cacheDirectory, );
        m_updateRecords = std::make_shared<IModelJsECPresentationUpdateRecordsHandler>();
        m_presentationManager = std::unique_ptr<RulesDrivenECPresentationManager>(ECPresentationUtils::CreatePresentationManager(T_HOST.GetIKnownLocationsAdmin(),
            m_localState, id, localeDirectories, CreateTaskAllocationSlotsMap(taskAllocationSlots), mode, isChangeTrackingEnabled, m_updateRecords, cacheDirectory));
        m_ruleSetLocater = SimpleRuleSetLocater::Create();
        m_presentationManager->GetLocaters().RegisterLocater(*m_ruleSetLocater);
        }

    ~NativeECPresentationManager()
        {
        SetInDestructor();  // Must be the first line of every ObjectWrap destructor
        // 'terminate' not called
        BeAssert(m_presentationManager == nullptr);
        }

    Napi::Value CreateReturnValue(ECPresentationResult const& result, bool serializeResponse = false)
        {
        return CreateReturnValue(Env(), result, serializeResponse);
        }

    static PresentationRequestContext CreateRequestContext()
        {
        JsInterop::ObjectReferenceClaimCheck jsRequestContext = JsInterop::GetCurrentClientRequestContextForMainThread();
        PresentationRequestContext context([jsRequestContext]()
            {
            if (!JsInterop::IsMainThread())
                {
                // WIP: Seems like on MacOS _only_ we somehow get here on the main thread. Needs investigation.
                JsInterop::SetCurrentClientRequestContextForWorkerThread(jsRequestContext);
                }
            });
        return context;
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

        m_presentationManager->GetConnections().CreateConnection(db->GetDgnDb());

        JsonValueCR params = request["params"];

        JsInterop::LogMessage("ECPresentation.Node", NativeLogging::LOG_DEBUG, Utf8PrintfString("Received request: %s", requestId).c_str());
        ResponseSender* responseSender = new ResponseSender(responseCallback);
        responseSender->Queue();
        try
            {
            folly::Future<ECPresentationResult> result = folly::makeFutureWith([]() {return ECPresentationResult(ECPresentationStatus::InvalidArgument, "request.requestId"); });

            if (0 == strcmp("GetRootNodesCount", requestId))
                result = ECPresentationUtils::GetRootNodesCount(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetRootNodes", requestId))
                result = ECPresentationUtils::GetRootNodes(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetChildrenCount", requestId))
                result = ECPresentationUtils::GetChildrenCount(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetChildren", requestId))
                result = ECPresentationUtils::GetChildren(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetNodePaths", requestId))
                result = ECPresentationUtils::GetNodesPaths(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetFilteredNodePaths", requestId))
                result = ECPresentationUtils::GetFilteredNodesPaths(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("LoadHierarchy", requestId))
                result = ECPresentationUtils::LoadHierarchy(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetContentDescriptor", requestId))
                result = ECPresentationUtils::GetContentDescriptor(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetContent", requestId))
                result = ECPresentationUtils::GetContent(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetContentSetSize", requestId))
                result = ECPresentationUtils::GetContentSetSize(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetDistinctValues", requestId))
                result = ECPresentationUtils::GetDistinctValues(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());
            else if (0 == strcmp("GetDisplayLabel", requestId))
                result = ECPresentationUtils::GetDisplayLabel(*m_presentationManager, db->GetDgnDb(), params, CreateRequestContext());

            result
            .then([responseSender](ECPresentationResult result)
                {
                responseSender->SetResult(std::move(result));
                })
            .onError([responseSender](folly::exception_wrapper e)
                {
                if (e.is_compatible_with<folly::FutureCancellation>())
                    responseSender->SetResult(ECPresentationResult(ECPresentationStatus::Canceled, "Request was canceled"));
                else
                    responseSender->SetResult(ECPresentationResult(ECPresentationStatus::Error, Utf8PrintfString("Error handling request: %s", e.what().c_str())));
                });
            }
        catch (...)
            {
            responseSender->SetResult(ECPresentationResult(ECPresentationStatus::Error, "Unknown error creating request"));
            }
        }

    void ForceLoadSchemas(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, db, );
        REQUIRE_ARGUMENT_FUNCTION(1, responseCallback, );
        if (!db->IsOpen())
            {
            THROW_TYPE_EXCEPTION_AND_RETURN("NativeECPresentationManager::ForceLoadSchemas: iModel not open", );
            return;
            }
        (new SchemasLoader(responseCallback, db->GetDgnDb()))->Queue();
        }

    Napi::Value SetupRulesetDirectories(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, directories, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "directories")));
        ECPresentationResult result = ECPresentationUtils::SetupRulesetDirectories(*m_presentationManager, directories);
        return CreateReturnValue(result);
        }

    Napi::Value SetupSupplementalRulesetDirectories(Napi::CallbackInfo const& info)
        {
        REQUIRE_ARGUMENT_STRING_ARRAY(0, directories, CreateReturnValue(ECPresentationResult(ECPresentationStatus::InvalidArgument, "directories")));
        ECPresentationResult result = ECPresentationUtils::SetupSupplementalRulesetDirectories(*m_presentationManager, directories);
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

    Napi::Value GetUpdateInfo(Napi::CallbackInfo const& info)
        {
        return CreateReturnValue(ECPresentationResult(m_updateRecords->GetReport()));
        }

    void Terminate(Napi::CallbackInfo const& info)
        {
        m_presentationManager.reset();
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      02/18
//+---------------+---------------+---------------+---------------+---------------+------
void JsInterop::HandleAssertion(WCharCP msg, WCharCP file, unsigned line, BeAssertFunctions::AssertType type)
    {
    BeMutexHolder lock(s_assertionMutex);
    if (!s_assertionsEnabled)
        return;

    if (IsJsExecutionDisabled())
        {
        fwprintf(stderr, L"%ls %ls %d", msg, file, (int)line);
        return;
        }

    Napi::Error::New(JsInterop::Env(), Utf8PrintfString("Native Assertion Failure: %ls (%ls:%d)\n", msg, file, line).c_str()).ThrowAsJavaScriptException();
    }

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct DisableNativeAssertions : BeObjectWrap<DisableNativeAssertions>
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
        DisableNativeAssertions(Napi::CallbackInfo const& info) : BeObjectWrap<DisableNativeAssertions>(info)
            {
            BeMutexHolder lock(s_assertionMutex);
            m_enableOnDispose = s_assertionsEnabled;
            s_assertionsEnabled = false;
            }

        ~DisableNativeAssertions() { SetInDestructor(); DoDispose(); }

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
struct NativeImportContext : BeObjectWrap<NativeImportContext>
{
private:
    DEFINE_CONSTRUCTOR;
    DgnImportContext* m_importContext = nullptr;

public:
    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "ImportContext", {
            InstanceMethod("addClass", &NativeImportContext::AddClass),
            InstanceMethod("addCodeSpecId", &NativeImportContext::AddCodeSpecId),
            InstanceMethod("addElementId", &NativeImportContext::AddElementId),
            InstanceMethod("dispose", &NativeImportContext::Dispose),
            InstanceMethod("findCodeSpecId", &NativeImportContext::FindCodeSpecId),
            InstanceMethod("findElementId", &NativeImportContext::FindElementId),
            InstanceMethod("importCodeSpec", &NativeImportContext::ImportCodeSpec),
            InstanceMethod("cloneElement", &NativeImportContext::CloneElement),
            InstanceMethod("importFont", &NativeImportContext::ImportFont),
            InstanceMethod("dump", &NativeImportContext::Dump),
        });
        exports.Set("ImportContext", t);
        SET_CONSTRUCTOR(t);
        }

    NativeImportContext(Napi::CallbackInfo const& info) : BeObjectWrap<NativeImportContext>(info)
        {
        REQUIRE_ARGUMENT_OBJ(0, NativeDgnDb, sourceDb, );
        REQUIRE_ARGUMENT_OBJ(1, NativeDgnDb, targetDb, );
        m_importContext = new DgnImportContext(sourceDb->GetDgnDb(), targetDb->GetDgnDb());
        }

    ~NativeImportContext() {SetInDestructor();}

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

    Napi::Value Dump(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING(0, outputFileName, Napi::Number::New(Env(), (int) BentleyStatus::ERROR));
        BentleyStatus status = m_importContext->Dump(outputFileName);
        return Napi::Number::New(Env(), (int) status);
        }

    Napi::Value AddClass(Napi::CallbackInfo const& info)
        {
        if (nullptr == m_importContext)
            THROW_TYPE_EXCEPTION_AND_RETURN("Invalid NativeImportContext", Env().Undefined());

        REQUIRE_ARGUMENT_STRING(0, sourceClassFullName, Napi::Number::New(Env(), (int) BentleyStatus::ERROR));
        REQUIRE_ARGUMENT_STRING(1, targetClassFullName, Napi::Number::New(Env(), (int) BentleyStatus::ERROR));
        bvector<Utf8String> sourceTokens;
        bvector<Utf8String> targetTokens;
        BeStringUtilities::Split(sourceClassFullName.c_str(), ".:", sourceTokens);
        BeStringUtilities::Split(targetClassFullName.c_str(), ".:", targetTokens);
        if ((2 != sourceTokens.size()) || (2 != targetTokens.size()))
            return Napi::Number::New(Env(), (int) DgnDbStatus::InvalidName);

        DgnClassId sourceClassId = m_importContext->GetSourceDb().Schemas().GetClassId(sourceTokens[0].c_str(), sourceTokens[1].c_str());
        DgnClassId targetClassId = m_importContext->GetDestinationDb().Schemas().GetClassId(targetTokens[0].c_str(), targetTokens[1].c_str());
        if ((!sourceClassId.IsValid()) || (!targetClassId.IsValid()))
            return Napi::Number::New(Env(), (int) DgnDbStatus::InvalidName);

        m_importContext->AddClassId(sourceClassId, targetClassId);
        return Napi::Number::New(Env(), (int) BentleyStatus::SUCCESS);
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

        // NOTE: Elements and Models share the same mapping table. However, the root Subject can be remapped but not the RepositoryModel
        DgnModelId targetModelId;
        if (sourceElement->GetModelId() == DgnModel::RepositoryModelId())
            targetModelId = DgnModel::RepositoryModelId();
        else
            targetModelId = m_importContext->FindModelId(sourceElement->GetModelId());

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

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct NativeDevTools : BeObjectWrap<NativeDevTools>
{
private:
static Napi::Value Signal(Napi::CallbackInfo const &info)
    {
    if (info.Length() == 0 || !info[0].IsNumber())
        Napi::TypeError::New(info.Env(), "Must supply SignalType").ThrowAsJavaScriptException();
    SignalType signalType = (SignalType) info[0].As<Napi::Number>().Int32Value();
    bool status = SignalTestUtility::Signal(signalType);
    return Napi::Number::New(info.Env(), status);
    }

public:
NativeDevTools(Napi::CallbackInfo const &info) : BeObjectWrap<NativeDevTools>(info) {}
~NativeDevTools() {SetInDestructor();}

static void Init(Napi::Env env, Napi::Object exports)
    {
    Napi::HandleScope scope(env);
    Napi::Function t = DefineClass(env, "NativeDevTools", {
        StaticMethod("signal", &NativeDevTools::Signal),
    });
    exports.Set("NativeDevTools", t);
    }
};


#if defined(BENTLEYCONFIG_OS_WINDOWS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
//=======================================================================================
//! @bsiclass
//=======================================================================================
struct KeyTar : BeObjectWrap<KeyTar>
{
public:
    KeyTar(Napi::CallbackInfo const &info) : BeObjectWrap<KeyTar>(info) {}
    ~KeyTar() {SetInDestructor();}

    static void Init(Napi::Env& env, Napi::Object exports)
        {
        Napi::HandleScope scope(env);
        Napi::Function t = DefineClass(env, "KeyTar", {
            StaticMethod("getPassword", &GetPasswordWorker::GetPasswordAsync),
            StaticMethod("setPassword", &SetPasswordWorker::SetPasswordAsync),
            StaticMethod("deletePassword", &DeletePasswordWorker::DeletePasswordAsync),
            StaticMethod("findPassword", &FindPasswordWorker::FindPasswordAsync),
            StaticMethod("findCredentials", &FindCredentialsWorker::FindCredentialsAsync),
        });
        exports.Set("KeyTar", t);
        }
};
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void setUseTileCache(Napi::CallbackInfo const& info)
    {
    if (info.Length() != 1 || !info[0].IsBoolean())
        JsInterop::GetLogger().error("Invalid argument for setUseTileCache.");
    else
        JsInterop::SetUseTileCache(info[0].As<Napi::Boolean>().Value());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void setCrashReporting(Napi::CallbackInfo const& info)
    {
    static bool s_crashReportingInitialized;

    if (s_crashReportingInitialized) {
        Napi::TypeError::New(info.Env(), "Crash reporting is already initialized.").ThrowAsJavaScriptException();
        return;
    }

    if ((info.Length() != 1) || !info[0].IsObject()) {
        Napi::TypeError::New(info.Env(), "Argument must be CrashReportingConfig object").ThrowAsJavaScriptException();
        return;
    }
    Napi::Object obj = info[0].As<Napi::Object>();

    JsInterop::CrashReportingConfig ccfg;
    ccfg.m_crashDir.SetNameA(getOptionalStringProperty(obj, "crashDir", "").c_str());
    ccfg.m_dumpProcessorScriptFileName.SetNameA(getOptionalStringProperty(obj, "dumpProcessorScriptFileName", "").c_str());
    ccfg.m_enableCrashDumps         = getOptionalBooleanProperty(obj, "enableCrashDumps", false);
    ccfg.m_maxDumpsInDir            = getOptionalIntProperty(obj, "maxDumpsInDir", 50);
    ccfg.m_wantFullMemory           = getOptionalBooleanProperty(obj, "wantFullMemory", false);
    if (obj.Has("params"))
        {
        Napi::Array arr = obj.Get("params").As<Napi::Array>();
        for (uint32_t arrIndex = 0; arrIndex < arr.Length(); ++arrIndex) {
            Napi::Value arrValue = arr[arrIndex];
            if (arrValue.IsObject())
                {
                auto item = arrValue.As<Napi::Object>();
                auto name = getOptionalStringProperty(item, "name", "");
                auto value = getOptionalStringProperty(item, "value", "");
                if (!name.empty())
                    ccfg.m_params[name] = value;
                }
            }
        }
#ifdef WIP_DUMP_UPLOAD
    ccfg.m_maxUploadRetries         = getOptionalIntProperty(obj, "maxUploadRetries", 5);
    ccfg.m_uploadRetryWaitInterval  = getOptionalIntProperty(obj, "uploadRetryWaitInterval", 10000);
    ccfg.m_maxReportsPerDay         = getOptionalIntProperty(obj, "maxReportsPerDay", 1000);
    ccfg.m_uploadUrl                = getOptionalStringProperty(obj, "uploadUrl", "");
#endif
    ccfg.m_needsVectorExceptionHandler = true;
    JsInterop::InitializeCrashReporting(ccfg);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void storeObjectInVault(Napi::CallbackInfo const& info)
    {
    if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsString())
        {
        Napi::TypeError::New(info.Env(), "Argument 0 must be an object, and argument 1 must be a string (the ID to be assigned to it)").ThrowAsJavaScriptException();
        return;
        }
    Napi::Object obj = info[0].As<Napi::Object>();
    Utf8String id = info[1].ToString().Utf8Value().c_str();

    if (s_objRefVault.GetObjectById(info.Env(), id) == info.Env().Undefined())
        s_objRefVault.StoreObjectRef(obj, id);

    s_objRefVault.AddRefToObject(id);   // The caller will hold the ref. See dropObjectFromVault
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void addReferenceToObjectInVault(Napi::CallbackInfo const& info)
    {
    if (info.Length() != 1 || !info[0].IsString())
        {
        Napi::TypeError::New(info.Env(), "Argument must be a string - the ID of an object in the vault").ThrowAsJavaScriptException();
        return;
        }
    Utf8String id = info[0].ToString().Utf8Value().c_str();

    if (s_objRefVault.GetObjectById(info.Env(), id) == info.Env().Undefined())
        {
        Napi::TypeError::New(info.Env(), "ID not found in object vault").ThrowAsJavaScriptException();
        return;
        }

    s_objRefVault.AddRefToObject(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void dropObjectFromVault(Napi::CallbackInfo const& info)
    {
    if (info.Length() != 1 || !info[0].IsString())
        {
        Napi::TypeError::New(info.Env(), "Argument must be a string - the ID of an object in the vault").ThrowAsJavaScriptException();
        return;
        }
    Utf8String id = info[0].ToString().Utf8Value().c_str();

    if (s_objRefVault.GetObjectById(info.Env(), id) == info.Env().Undefined())
        {
        Napi::TypeError::New(info.Env(), "ID not found in object vault").ThrowAsJavaScriptException();
        return;
        }

    s_objRefVault.ReleaseRefToObject(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getObjectFromVault(Napi::CallbackInfo const& info)
    {
    if (info.Length() != 1 || !info[0].IsString())
        {
        Napi::TypeError::New(info.Env(), "Argument must be a string - the ID of an object in the vault").ThrowAsJavaScriptException();
        return info.Env().Undefined();
        }
    Utf8String id = info[0].ToString().Utf8Value().c_str();
    return s_objRefVault.GetObjectById(info.Env(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getObjectRefCountFromVault(Napi::CallbackInfo const& info)
    {
    if (info.Length() != 1 || !info[0].IsString())
        {
        Napi::TypeError::New(info.Env(), "Argument must be a string - the ID of an object in the vault").ThrowAsJavaScriptException();
        return info.Env().Undefined();
        }
    Utf8String id = info[0].ToString().Utf8Value().c_str();

    if (s_objRefVault.GetObjectById(info.Env(), id) == info.Env().Undefined())
        {
        Napi::TypeError::New(info.Env(), "ID not found in object vault").ThrowAsJavaScriptException();
        return info.Env().Undefined();
        }
    return Napi::Number::New(info.Env(), (int) s_objRefVault.GetObjectRefCountById(id));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value getLogger(Napi::CallbackInfo const& info) { return s_logger.Value(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void setLogger(Napi::CallbackInfo const& info)
    {
    s_logger = Napi::ObjectReference::New(info[0].ToObject(), 1);
    s_logger.SuppressDestruct();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearLogLevelCache(Napi::CallbackInfo const&)
    {
    BeSystemMutexHolder ___;

    if (nullptr == s_categorySeverityFilter)
        return;
    delete s_categorySeverityFilter;
    s_categorySeverityFilter = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void setNopBriefcaseManager(Napi::CallbackInfo const&)
    {
    JsInterop::SetNopBriefcaseManager();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void logMessageToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        return;
        }

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
        Napi::Error::New(JsInterop::Env(), Utf8PrintfString("Invalid Logger -- missing %s function", fname).c_str()).ThrowAsJavaScriptException();
        return;
        }

    auto catJS = toJsString(env, category);
    auto msgJS = toJsString(env, msg);

    method({catJS, msgJS});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsInterop::SetCurrentClientRequestContextForWorkerThread(ObjectReferenceClaimCheck const& ctx)
    {
    if (IsMainThread())
        {
        BeAssert(false && "SetCurrentClientRequestContextForThread should be called only on non-main threads");
        return BSIERROR;
        }

    if (s_currentClientRequestContext == nullptr)
        s_currentClientRequestContext = new BeThreadLocalStorage();

    ObjectReferenceClaimCheck* current = (ObjectReferenceClaimCheck*) s_currentClientRequestContext->GetValueAsPointer();
    if (current != nullptr)
        delete current;

    s_currentClientRequestContext->SetValueAsPointer(new ObjectReferenceClaimCheck(ctx));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck const&     JsInterop::GetCurrentClientRequestContextForWorkerThread()
    {
    if (nullptr != s_currentClientRequestContext)
        {
        ObjectReferenceClaimCheck* current = s_currentClientRequestContext? (ObjectReferenceClaimCheck*) s_currentClientRequestContext->GetValueAsPointer(): nullptr;
        if (nullptr != current)
            return *current;
        }

    // Caller did not set a context for this thread. Assume he knows what he's doing. Use an empty context.
    static ObjectReferenceClaimCheck* s_missingCtx;
    if (nullptr == s_missingCtx)
        s_missingCtx = new ObjectReferenceClaimCheck("");

    return *s_missingCtx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void callSetCurrentClientRequestContext(Napi::Env env, Napi::Value ctxObj)
    {
    // WARNING! Caller must manage the HandleScope in which this runs!

    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        return;
        }

    auto method = s_logger.Get("setCurrentClientRequestContext").As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(JsInterop::Env(), "Invalid Logger -- missing 'setCurrentClientRequestContext' function").ThrowAsJavaScriptException();
        return;
        }

    method({ctxObj});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object callGetCurrentClientRequestContext(Napi::Env env)
    {
    // WARNING! Caller must manage the HandleScope in which this runs!

    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        }

    auto method = s_logger.Get("getCurrentClientRequestContext").As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(JsInterop::Env(), "Invalid Logger -- missing getCurrentClientRequestContext function").ThrowAsJavaScriptException();
        return Napi::Object::New(env);
        }

    return method({}).ToObject();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
JsInterop::ObjectReferenceClaimCheck JsInterop::GetCurrentClientRequestContextForMainThread()
    {
    if (s_logger.IsEmpty() || IsJsExecutionDisabled())
        {
        BeAssert(false && "GetCurrentClientRequestContextForMainThread must be called on the main thread only.");
        return ObjectReferenceClaimCheck();
        }

    auto env = s_logger.Env();
    Napi::HandleScope scope(env);

    auto obj = callGetCurrentClientRequestContext(env);

    if (obj == env.Undefined())
        {
        return ObjectReferenceClaimCheck("");
        }

    Utf8String slotId = obj.Get("activityId").ToString().Utf8Value().c_str();

    if (s_objRefVault.GetObjectById(env, slotId) == env.Undefined())
        s_objRefVault.StoreObjectRef(obj, slotId);

    return ObjectReferenceClaimCheck(slotId);   // Calls s_objRefVault.AddRefToObject
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool callIsLogLevelEnabledJs(Utf8CP category, NativeLogging::SEVERITY sev)
    {
    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        return false;
        }

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
* @bsimethod                                    Sam.Wilson                      04/19
+---------------+---------------+---------------+---------------+---------------+------*/
NativeLogging::SEVERITY callGetLogLevelJs(Utf8StringCR category)
    {
    if (JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false);
        return NativeLogging::LOG_TRACE;
        }

    auto env = s_logger.Env();
    Napi::HandleScope scope(env);

    auto method = s_logger.Get("getLevel").As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
        return NativeLogging::LOG_TRACE;
        }

    auto catJS = toJsString(env, category);

    auto jsLevelValue = method({catJS});

    int logLevel = (jsLevelValue == env.Undefined())? 4: jsLevelValue.ToNumber().Int32Value();

    switch (logLevel)
        {
        case 0: return NativeLogging::LOG_TRACE;
        case 1: return NativeLogging::LOG_INFO;
        case 2: return NativeLogging::LOG_WARNING;
        case 3: return NativeLogging::LOG_ERROR;
        case 4: return (NativeLogging::SEVERITY)(NativeLogging::LOG_FATAL + 1); // LogLevel.None => Fake importance that is higher than the highest that could really be set.
        }
    BeAssert(false);
    return NativeLogging::LOG_TRACE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void pushDeferredLoggingMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg, JsInterop::ObjectReferenceClaimCheck const& ctx)
    {
    BeSystemMutexHolder ___;

    if (nullptr == s_deferredLogging)
        s_deferredLogging = new bmap<Utf8String, bmap<Utf8String, bvector<LogMessage>>>();

    if (nullptr != s_categorySeverityFilter)
        {
        auto i = s_categorySeverityFilter->find(category);                  // if we know the level set for the category
        if ((i != s_categorySeverityFilter->end()) && (sev < i->second))    // and we know that the current message is of lower severity than that
            return;                                                         // then don't both to save the message, since we know that Logger will just filter it out.
        }

    LogMessage lm;
    lm.m_message = msg;
    lm.m_severity = sev;
    auto& ctxEntry = (*s_deferredLogging)[ctx.GetId()];
    auto& catEntry = ctxEntry[category];
    catEntry.push_back(lm);

    s_deferredLoggingClientRequestActivityClaimChecks.insert(ctx); // make sure this claim check (and the vault slot) remain alive until doDeferredLogging is run.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void deferLogging(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    pushDeferredLoggingMessage(category, sev, msg, JsInterop::GetCurrentClientRequestContextForWorkerThread());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
static void doDeferredLogging()
    {
    if (s_logger.IsEmpty() || JsInterop::IsJsExecutionDisabled())
        {
        BeAssert(false && "deferred logging can be processed only on the main thread and while JS execution is allowed");
        return;
        }

    s_objRefVault.ReleaseUnreferencedObjects();

    BeSystemMutexHolder ___;

    if (nullptr == s_deferredLogging)
        return;

    if (nullptr == s_categorySeverityFilter)
        s_categorySeverityFilter = new bmap<Utf8String, NativeLogging::SEVERITY>();

    for (auto const& contextWithCategories : *s_deferredLogging)
        {
        auto const& ctx = contextWithCategories.first;

        auto env = s_logger.Env();
        Napi::HandleScope scope(env);

        auto wasCtx = callGetCurrentClientRequestContext(env);

        auto ctxObj = s_objRefVault.GetObjectById_Locked(env, ctx);
        callSetCurrentClientRequestContext(env, ctxObj);

        for (auto const& categoryAndMessages : contextWithCategories.second)
            {
            auto const& category = categoryAndMessages.first;

            auto iSev = s_categorySeverityFilter->find(category);
            if (iSev == s_categorySeverityFilter->end()) // build up knowledge of severity settings
                (*s_categorySeverityFilter)[category] = callGetLogLevelJs(category);

            for (auto const& msg : categoryAndMessages.second)
                {
                logMessageToJs(category.c_str(), msg.m_severity, msg.m_message.c_str());
                }
            }

        callSetCurrentClientRequestContext(env, wasCtx);
        }

    delete s_deferredLogging;
    s_deferredLogging = nullptr;

    s_deferredLoggingClientRequestActivityClaimChecks.clear();
    s_objRefVault.ReleaseUnreferencedObjects();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::DoDeferredLogging()
    {
    doDeferredLogging();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/19
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::LogMessageInContext(Utf8StringCR category, NativeLogging::SEVERITY sev, Utf8StringCR msg, ObjectReferenceClaimCheck const& ctx)
    {
    if (IsJsExecutionDisabled())
        {
        pushDeferredLoggingMessage(category.c_str(), sev, msg.c_str(), ctx);
        return;
        }

    doDeferredLogging();

    auto env = s_logger.Env();
    if (!env)
        {
        // know issue in iOS with JSC
        return;
        }

    Napi::HandleScope scope(env);

    auto wasCtx = callGetCurrentClientRequestContext(env);

    auto ctxObj = s_objRefVault.GetObjectById_Locked(env, ctx.GetId());
    callSetCurrentClientRequestContext(env, ctxObj);

    logMessageToJs(category.c_str(), sev, msg.c_str());

    callSetCurrentClientRequestContext(env, wasCtx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void JsInterop::LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    if (s_logger.IsEmpty() || IsJsExecutionDisabled())
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
    if (s_logger.IsEmpty() || IsJsExecutionDisabled())
        return true;

    doDeferredLogging();
    return callIsLogLevelEnabledJs(category, sev);
    }

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
#ifdef NOT_UNTIL_ALL_BOXES_RUN_NODE_10_2
static void onNodeExiting(void*)
    {
    if (nullptr != s_deferredLogging)
        {
        delete s_deferredLogging; // Too late for logging
        s_deferredLogging = nullptr;
        }

    if (!s_logger.IsEmpty())
        s_logger.Reset();

    s_objRefVault.Clear();  // Force the vault to release all ObjectReferences, regardless of the refcounts of the slots.

    if (nullptr != s_currentClientRequestContext)
        {
        delete s_currentClientRequestContext;
        s_currentClientRequestContext = nullptr;        // This kills off all TLS keys. Threads should not be accessing them at this point.
        }

    BeAssert(nullptr == s_deferredLogging);
    BeAssert(nullptr == s_currentClientRequestContext);
    BeAssert(s_logger.IsEmpty());
    BeAssert(s_objRefVault.IsEmpty());
    }
#endif

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
    NativeUlasClient::Init(env, exports);
    DisableNativeAssertions::Init(env, exports);
    NativeImportContext::Init(env, exports);
    NativeDevTools::Init(env, exports);
    ApplyChangeSetsRequest::Init(env, exports);

#if defined(BENTLEYCONFIG_OS_WINDOWS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
    KeyTar::Init(env, exports);
#endif

#ifdef NOT_UNTIL_ALL_BOXES_RUN_NODE_10_2
    napi_add_env_cleanup_hook(env, onNodeExiting, nullptr);
#endif

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Value("version", Napi::String::New(env, PACKAGE_VERSION), PROPERTY_ATTRIBUTES),
        Napi::PropertyDescriptor::Accessor(env, exports, "logger", &getLogger, &setLogger),
        Napi::PropertyDescriptor::Function(env, exports, "setUseTileCache", &setUseTileCache),
        Napi::PropertyDescriptor::Function(env, exports, "setCrashReporting", &setCrashReporting),
        Napi::PropertyDescriptor::Function(env, exports, "storeObjectInVault", &storeObjectInVault),
        Napi::PropertyDescriptor::Function(env, exports, "getObjectFromVault", &getObjectFromVault),
        Napi::PropertyDescriptor::Function(env, exports, "dropObjectFromVault", &dropObjectFromVault),
        Napi::PropertyDescriptor::Function(env, exports, "addReferenceToObjectInVault", &addReferenceToObjectInVault),
        Napi::PropertyDescriptor::Function(env, exports, "getObjectRefCountFromVault", &getObjectRefCountFromVault),
        Napi::PropertyDescriptor::Function(env, exports, "clearLogLevelCache", &clearLogLevelCache),
        Napi::PropertyDescriptor::Function(env, exports, "setNopBriefcaseManager", &setNopBriefcaseManager),
        });

    return exports;
    }

NODE_API_MODULE(iModelJsNative, registerModule)
} // namespace IModelJsNative
