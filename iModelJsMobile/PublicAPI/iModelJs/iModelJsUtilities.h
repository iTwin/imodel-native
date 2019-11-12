/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelJs/iModelJs.h>
#include <functional>
#include <vector>

#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
#include <JavaScriptCore/JavaScriptCore.h>
#endif

#define BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE  BEGIN_BENTLEY_IMODELJS_NAMESPACE namespace Js {
#define END_BENTLEY_IMODELJS_JS_NAMESPACE    } END_BENTLEY_IMODELJS_NAMESPACE

#define IMODELJS_JS_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_IMODELJS_JS_NAMESPACE

#define IMODELJS_JS_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_IMODELJS_JS_NAMESPACE

IMODELJS_JS_TYPEDEFS (Runtime)
IMODELJS_JS_TYPEDEFS (RuntimeInternal)

BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE

typedef std::function<Napi::Value(Napi::CallbackInfo)> CallbackHandler_T;

enum class EvaluateStatus { Success, ParseError, RuntimeError };

static constexpr uint64_t MAX_SAFE_JS_INTEGER = 9007199254740991; //2^53 - 1

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Runtime
    {
    friend struct RuntimeInternal;

public:
    DEFINE_POINTER_SUFFIX_TYPEDEFS (EvaluateResult)
    DEFINE_POINTER_SUFFIX_TYPEDEFS (Debugger)

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct EvaluateResult
        {
    public:
        EvaluateStatus status;
        Utf8String message;
        Utf8String trace;
        Napi::Value value;
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    enum class Engine
        {
        V8,
        JSC,
        Chakra
        };

    //=======================================================================================
    // @bsiclass                                                    Steve.Wilson   7/17
    //=======================================================================================
    struct Debugger
        {
        friend struct Runtime;

    private:
        RuntimeR m_runtime;
        uint16_t m_port;

    public:
        IMODELJS_EXPORT Debugger (RuntimeR runtime, uint16_t port) : m_runtime (runtime), m_port (port) { ; }
        IMODELJS_EXPORT virtual ~Debugger() { ; }

        IMODELJS_EXPORT RuntimeR GetRuntime() const { return m_runtime; }
        IMODELJS_EXPORT virtual bool IsReady() const { return false; }
        IMODELJS_EXPORT uint16_t GetPort() const { return m_port; }
        IMODELJS_EXPORT virtual void NotifyIdle() { ; }
        };

private:
    static bool s_initialized;
    static void* s_initializationData;

    Utf8CP m_name;
    RuntimeInternalP m_impl;
    DebuggerP m_debugger;
    Engine m_engine;
    mutable std::vector<CallbackHandler_T*> m_callbackHandlers;

    Runtime (RuntimeCR other) = delete;
    RuntimeR operator= (RuntimeCR other) = delete;

    static void Initialize();
    
    void RegisterCallbackHandler (CallbackHandler_T& handler) const;
    void OnCreate();
    void OnDestroy();

public:
    IMODELJS_EXPORT static void* GetInitializationData();

    IMODELJS_EXPORT Runtime (Utf8CP name = nullptr, bool startDebugger = false, uint16_t debuggerPort = 0);
    IMODELJS_EXPORT ~Runtime();

    IMODELJS_EXPORT EvaluateResult EvaluateScript (Napi::String script, Napi::String identifier);
    IMODELJS_EXPORT EvaluateResult EvaluateScript (Napi::String script);
    IMODELJS_EXPORT EvaluateResult EvaluateScript (Utf8CP script, Utf8CP identifier = "");
    IMODELJS_EXPORT Engine GetEngine() const { return m_engine; }
    IMODELJS_EXPORT Utf8CP GetName() const { return m_name; }
    IMODELJS_EXPORT DebuggerP GetDebugger() const { return m_debugger; }
    IMODELJS_EXPORT void StartDebugger (uint16_t port);
    IMODELJS_EXPORT void NotifyIdle();

    IMODELJS_EXPORT Napi::Env& Env();

    IMODELJS_EXPORT static Runtime& GetRuntime(Napi::Env const&);
        

#if defined(BENTLEYCONFIG_OS_APPLE_IOS) || defined(BENTLEYCONFIG_OS_APPLE_MACOS)
    IMODELJS_EXPORT JSContextRef GetContext() const;
#endif
    };

END_BENTLEY_IMODELJS_JS_NAMESPACE

#define JS_CALLBACK_UNDEFINED                       info.Env().Undefined()
#define JS_CALLBACK_NULL                            info.Env().Null()
#define JS_CALLBACK_BOOLEAN(_value_)                Napi::Boolean::New(info.Env(), _value_)
#define JS_CALLBACK_NUMBER(_value_)                 Napi::Number::New(info.Env(), _value_)
#define JS_CALLBACK_STRING(_value_)                 Napi::String::New(info.Env(), _value_)
#define JS_CALLBACK_OBJECT                          Napi::Object::New(info.Env())
#define JS_CALLBACK_ARRAY                           Napi::Array::New(info.Env())
#define JS_CALLBACK_ARRAY_BUFFER(_data_, _length_)  Napi::ArrayBufer::New(info.Env(), _data_, _length_)

#define JS_CALLBACK_REQUIRE(_condition_, _message_) \
    if (!(_condition_)) \
        { \
        auto undefined = JS_CALLBACK_UNDEFINED; \
        Napi::Error::New(info.Env(), _message_).ThrowAsJavaScriptException();\
        return undefined; \
        }

#define JS_CALLBACK_REQUIRE_EXTERNAL_THIS JS_CALLBACK_REQUIRE ((info.This().Type() == napi_external), "Invalid object.")
#define JS_CALLBACK_REQUIRE_N_ARGS(_argCount_) JS_CALLBACK_REQUIRE (info.Length() == _argCount_, "A required argument is missing.")
#define JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(_argCount_) JS_CALLBACK_REQUIRE (info.Length() >= _argCount_, "A required argument is missing.")

#define _JS_CALLBACK_GET_ARG(_argIndex_, _argType_) \
    info[_argIndex_].As<Napi::_argType_>(); \
    if (!info[_argIndex_].Is##_argType_()) \
        { \
        auto undefined = JS_CALLBACK_UNDEFINED; \
        Napi::Error::New(info.Env(), "Argument " #_argIndex_ " must be a " #_argType_ ).ThrowAsJavaScriptException();\
        return undefined; \
        }

#define JS_CALLBACK_GET_UNDEFINED(_argIndex_)    _JS_CALLBACK_GET_ARG (_argIndex_, Undefined)
#define JS_CALLBACK_GET_NULL(_argIndex_)         _JS_CALLBACK_GET_ARG (_argIndex_, Null)
#define JS_CALLBACK_GET_BOOLEAN(_argIndex_)      _JS_CALLBACK_GET_ARG (_argIndex_, Boolean)
#define JS_CALLBACK_GET_NUMBER(_argIndex_)       _JS_CALLBACK_GET_ARG (_argIndex_, Number)
#define JS_CALLBACK_GET_STRING(_argIndex_)       _JS_CALLBACK_GET_ARG (_argIndex_, String)
#define JS_CALLBACK_GET_OBJECT(_argIndex_)       _JS_CALLBACK_GET_ARG (_argIndex_, Object)
#define JS_CALLBACK_GET_ARRAY(_argIndex_)        _JS_CALLBACK_GET_ARG (_argIndex_, Array)
#define JS_CALLBACK_GET_ARRAY_BUFFER(_argIndex_) _JS_CALLBACK_GET_ARG (_argIndex_, ArrayBuffer)
#define JS_CALLBACK_GET_FUNCTION(_argIndex_)     _JS_CALLBACK_GET_ARG (_argIndex_, Function)
#define JS_CALLBACK_GET_CALLBACK(_argIndex_)     _JS_CALLBACK_GET_ARG (_argIndex_, Callback)
#define JS_CALLBACK_GET_EXTERNAL(_argIndex_)     _JS_CALLBACK_GET_ARG (_argIndex_, External)
#define JS_CALLBACK_GET_POINTER(_argIndex_)      _JS_CALLBACK_GET_ARG (_argIndex_, Pointer)
#define JS_CALLBACK_GET_UINT8ARRAY(_argIndex_)   info[_argIndex_].As<Napi::Uint8Array>()
//__PUBLISH_SECTION_END__
