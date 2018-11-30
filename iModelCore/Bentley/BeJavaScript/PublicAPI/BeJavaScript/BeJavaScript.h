/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/PublicAPI/BeJavaScript/BeJavaScript.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BeAssert.h>
#include <json/json.h>
#include <BeJavaScriptTools/BeJavaScriptTools.h>

#if defined (BENTLEYCONFIG_OS_APPLE)
    #define BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE
#elif defined (BENTLEYCONFIG_OS_WINRT)
    #define BEJAVASCRIPT_JSRT_ENGINE
#elif defined (BENTLEYCONFIG_OS_ANDROID)
    #define BEJAVASCRIPT_V8_ENGINE
#elif defined (BENTLEYCONFIG_OS_WINDOWS)
    #if defined (WipPartEnable_chakracore)
        #define BEJAVASCRIPT_JSRT_ENGINE
    #else
        #if !defined (NDEBUG) && !defined(DGNCLIENTFXJS_WINDOWS_FORCE_V8_ENGINE)
        #define BEJAVASCRIPT_JSRT_ENGINE
        #else
        #define BEJAVASCRIPT_V8_ENGINE
        #endif
    #endif
#endif

#if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    #include <JavaScriptCore/JavaScriptCore.h>
#endif

#if defined (BEJAVASCRIPT_V8_ENGINE)
    #include <google_v8/v8.h>

    #if !defined (NDEBUG)
    #include <google_v8/v8-debug.h>
    #endif
#endif

#if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    #include <duktapejs/duktape.h>
#endif

#if defined (BEJAVASCRIPT_JSRT_ENGINE)
    #ifdef WipPartEnable_chakracore
        #include <chakracore/chakracore.h>
    #else
        #if defined (BENTLEYCONFIG_OS_WINRT) && _MSC_VER >= 1900
        #include <Roapi.h>
        #define USE_EDGEMODE_JSRT
        #endif

        #if defined (BEJAVASCRIPT_FORCE_JSRT_EDGE_MODE)
        #define USE_EDGEMODE_JSRT
        #endif

        #if _MSC_VER >= 1900
            #pragma warning(push)
            #pragma warning(disable:4091) // when VC14 compiles with the 8.1 SDK: 'typedef ': ignored on left of 'JsErrorCode' when no variable is declared
        #endif

        #include <jsrt.h>

        #if _MSC_VER >= 1900
            #pragma warning(pop)
        #endif
    #endif
#endif

BEGIN_BENTLEY_NAMESPACE

struct BeJsEnvironment;
typedef BeJsEnvironment& BeJsEnvironmentR;
typedef BeJsEnvironment const& BeJsEnvironmentCR;
typedef BeJsEnvironment const* BeJsEnvironmentCP;

struct BeJsContext;
typedef BeJsContext& BeJsContextR;
typedef BeJsContext const& BeJsContextCR;

struct BeJsValue;
typedef BeJsValue& BeJsValueR;
typedef BeJsValue const& BeJsValueCR;

struct BeJsObject;
typedef BeJsObject* BeJsObjectP;
typedef BeJsObject& BeJsObjectR;
typedef BeJsObject const& BeJsObjectCR;

struct BeJsNativePointer;
typedef BeJsNativePointer& BeJsNativePointerR;
typedef BeJsNativePointer const& BeJsNativePointerCR;

struct BeJsArray;
typedef BeJsArray& BeJsArrayR;
typedef BeJsArray const& BeJsArrayCR;

struct BeJsFunction;
typedef BeJsFunction& BeJsFunctionR;
typedef BeJsFunction const& BeJsFunctionCR;

struct BeJsNativeFunction;
typedef BeJsNativeFunction& BeJsNativeFunctionR;
typedef BeJsNativeFunction const& BeJsNativeFunctionCR;

struct BeJsNativeObject;
typedef BeJsNativeObject& BeJsNativeObjectR;
typedef BeJsNativeObject const& BeJsNativeObjectCR;

struct BeProjected;
struct BeJsProjection;

enum class BeJsEngine : uint8_t
    {
    V8             = 0,
    JavaScriptCore = 1,
    Jsrt           = 2,
    JsrtEdge       = 3,
    ChakraCore     = 4
    };

//=======================================================================================
// A JavaScript engine instance.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsEnvironment
    {
private:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    v8::Isolate* m_v8Isolate;
    v8::Locker* m_v8IsolateLocker;
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    JSContextGroupRef m_jscContextGroup;
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    JsRuntimeHandle m_jsrtRuntime;
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    duk_context* m_dukHeapInitialContext;
    mutable duk_uarridx_t m_lastHeapStashIndex;
    #endif

    bool m_destroying;
    bool m_ownsSystemEnvironment;

public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    typedef v8::Local<v8::Value> EngineValueHandleType;
    #elif defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    typedef JSValueRef EngineValueHandleType;
    #elif defined (BEJAVASCRIPT_JSRT_ENGINE)
    typedef JsValueRef EngineValueHandleType;
    #else
    typedef void* EngineValueHandleType;
    #endif

    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    v8::Isolate* GetV8Isolate() const { return m_v8Isolate; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    v8::Locker* GetV8Locker() const { return m_v8IsolateLocker; }
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JSContextGroupRef GetJscContextGroup() const { return m_jscContextGroup; }
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JsRuntimeHandle GetJsrtRuntime() const { return m_jsrtRuntime; }
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    duk_context* GetDukHeapInitialContext() const { return m_dukHeapInitialContext; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    duk_uarridx_t GetNextHeapStashIndex() const { return ++m_lastHeapStashIndex; }
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/17
    //---------------------------------------------------------------------------------------
    static BeJsEngine GetActiveEngine();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsEnvironment();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/17
    //---------------------------------------------------------------------------------------
    BeJsEnvironment (BeJsEngine engine, void* systemEnvironment);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    ~BeJsEnvironment();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    bool IsDestroying() const { return m_destroying; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/17
    //---------------------------------------------------------------------------------------
    void* GetSystemObject();

    //! Performs any necessary once-per-process initialization that a JS engine may require.
    static void InitializeEngine();
    };

//=======================================================================================
// A JavaScript execution context.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsContext
    {
    friend struct BeJsNativePointer;
    friend struct BeJsValue;
    friend struct BeJsNativeFunction;

public:
    enum class EvaluateStatus
        {
        Success,
        ParseError,
        RuntimeError
        };

    struct EvaluateException
        {
        Utf8String message;
        Utf8String trace;
        };

private:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    v8::Persistent<v8::Context> m_v8ContextHandle;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void ReportJavaScriptException (v8::TryCatch* tryCatch) const;
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    JSGlobalContextRef m_jscContext;
    BeJsValue* m_Function_prototype;
    void FindLanguageObjects();
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    JsContextRef m_jsrtContext;
    mutable JsSourceContext m_jsrtDebugScriptCookie;

    #if !defined (NDEBUG) && !defined (USE_EDGEMODE_JSRT) && !defined (WipPartEnable_chakracore)
    DWORD m_jsrtDebugApplicationCookie;
    IProcessDebugManager* m_jsrtDebugManager;
    IDebugApplication* m_jsrtDebugApplication;
    #endif
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    friend struct BeJsNativeObject;
    duk_context* m_dukContext;
    duk_uarridx_t m_dukHeapStashIndex;
    mutable duk_uarridx_t m_lastGlobalStashIndex;
    BeJsNativeFunction* m_proxyObjectGetter;
    BeJsNativeFunction* m_proxyObjectSetter;
    BeJsNativeFunction* m_proxyObjectDeleter;
    BeJsNativeFunction* m_proxyObjectEnumerator;
    BeJsNativeFunction* m_proxyObjectQuery;
    BeJsFunction* m_ES6_Proxy;
    #endif

    bvector<RefCountedPtr<BeJsProjection>> m_projections;
    mutable bvector<BeJsNativePointer> m_callContextValues;
    int m_callContextActive;
    BeJsEnvironment const* m_environment;
    BeJsFunction* m_JSON_parse;
    BeJsFunction* m_JSON_stringify;

    bool m_ownsSystemContext;

    typedef void(*DisposeFunctionCallback) (void* object);
    mutable bmap<void*, DisposeFunctionCallback> m_cleanupQueue;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    static Utf8CP _GetBeJavaScriptJsSource();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void _OnDestroy();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void Initialize (Utf8CP initializationScript, Utf8CP initializationScriptIdentifier);

public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    v8::Persistent<v8::Context> const& GetV8ContextHandle() const { return m_v8ContextHandle; }
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JSGlobalContextRef GetJscContext() const { return m_jscContext; }
    #endif
    
    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JsContextRef GetJsrtContext() const { return m_jsrtContext; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void SetJsrtContextActive() const;
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    duk_context* GetDukContext() const { return m_dukContext; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    duk_uarridx_t PutDukGlobalStashProperty (duk_idx_t stackIndex = -1, bool popStackValue = true) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void UpdateDukGlobalStashProperty (duk_uarridx_t propertyIndex, duk_idx_t stackIndex = -1, bool popStackValue = true) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    duk_uarridx_t DuplicateDukGlobalStashProperty (duk_uarridx_t propertyIndex) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void DeleteDukGlobalStashProperty (duk_uarridx_t propertyIndex) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void GetDukGlobalStashProperty (duk_uarridx_t propertyIndex) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    duk_uarridx_t GetNextGlobalStashIndex() const { return ++m_lastGlobalStashIndex; }
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsContext (BeJsEnvironmentCR environment, Utf8CP identifier = nullptr, Utf8CP initializationScript = nullptr);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsContext (BeJsEngine engine, BeJsEnvironmentCR environment, void* systemContext);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    virtual ~BeJsContext();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsEnvironmentCR GetEnvironment() const { return *m_environment; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsValue EvaluateScript (Utf8CP script, Utf8CP identifier = nullptr, EvaluateStatus* status = nullptr, EvaluateException* exception = nullptr) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsObject GetGlobalObject() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsValue EvaluateJson (Utf8CP json);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsValue EvaluateJson (Json::Value const& json);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/15
    //---------------------------------------------------------------------------------------
    BeJsContextR GetJsContext();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    T& RegisterProjection (Utf8CP bootstrapScript = nullptr, Utf8CP bootstrapIdentifier = nullptr, Utf8CP setupScript = nullptr, Utf8CP setupIdentifier = nullptr);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    BeJsProjection& GetProjection() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    BeJsNativePointer ObtainProjectedClassInstancePointer (T* instance, bool owner = false, BeJsObjectP customPrototype = nullptr) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    BeJsObject CopyProjectedInterfaceInstanceObject (T const* instance, BeJsObjectP objectToReuse = nullptr) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    T CreateProjectedInterfaceInstance (BeJsObject const& data) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    bool IsCallContextActive();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    void BeginCallContext();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    void EndCallContext();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    2/17
    //---------------------------------------------------------------------------------------
    void* GetSystemObject();
    };

//=======================================================================================
// A handle to a JavaScript value.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsValue
    {
    friend struct BeJsContext;
    friend struct BeJsObject;
    friend struct BeJsArray;
    friend struct BeProjected;

private:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    BeJsValue();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    9/15
    //---------------------------------------------------------------------------------------
    void Invalidate();

protected:
    BeJsContext const* m_context;

    #if defined (BEJAVASCRIPT_V8_ENGINE)
    v8::Persistent<v8::Value> m_v8ValueHandle;
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    JSValueRef m_jscValue;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void SetJscValue (JSValueRef value);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    JsValueRef m_jsrtValue;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void SetJsrtValue (JsValueRef value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JsValueType GetJsrtValueType() const;
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    duk_uarridx_t m_dukStashIndex;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void StoreDukValue (duk_idx_t stackIndex = -1, bool popStackValue = true);
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsValue (BeJsContextCR context);

public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsValue (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    v8::Persistent<v8::Value> const& GetV8ValueHandle() const { return m_v8ValueHandle; }
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsValue (BeJsContextCR context, JSValueRef jscValue);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JSValueRef GetJscValue() const { return m_jscValue; }
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsValue (BeJsContextCR context, JsValueRef jsrtValue);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JsValueRef GetJsrtValue() const { return m_jsrtValue; }
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsValue (BeJsContextCR context, duk_uarridx_t dukStashIndex);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    duk_uarridx_t GetDukStashIndex() const { return m_dukStashIndex; }
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsContextCR GetContext() const { return *m_context; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsValue (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsValueR operator= (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsValue (BeJsValue&& other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    virtual ~BeJsValue();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void Reset();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsEmpty() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    void NotifyEngineValueReleased();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsNull() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsUndefined() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsNumber() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsBoolean() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsString() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsObject() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsFunction() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsEqual (BeJsValueCR value) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool IsEqualStrict (BeJsValueCR value) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    bool IsValid() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    Utf8String Serialize();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsValue Null (BeJsContextCR context);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsValue Undefined (BeJsContextCR context);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    static BeJsValue InvalidHandle();
    };

//=======================================================================================
// A handle to a JavaScript primitive.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsPrimitive : public BeJsValue
    {
protected:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsPrimitive (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsPrimitive (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsPrimitive (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsPrimitive (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsPrimitive (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsPrimitive (BeJsContextCR context);
    };

//=======================================================================================
// A handle to a JavaScript boolean.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsBoolean : public BeJsPrimitive
    {
public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean (BeJsContextCR context, bool value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool GetValue() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetValue (bool value);
    };

//=======================================================================================
// A handle to a JavaScript number.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsNumber : public BeJsPrimitive
    {
public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsContextCR context, double value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsContextCR context, int32_t value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNumber (BeJsContextCR context, uint32_t value) : BeJsNumber (context, static_cast<double>(value)) {}

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    double GetValue() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    int32_t GetIntegerValue() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    uint32_t GetUnsignedIntegerValue() const { return static_cast<uint32_t>(GetValue()); }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetValue (double value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetValue (int32_t value);
    };

//=======================================================================================
// A handle to a JavaScript string.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsString : public BeJsPrimitive
    {
public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsString (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsString (BeJsContextCR context, JSValueRef jscValue);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    static Utf8String GetUtf8String (JSStringRef jsString);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsString (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsString (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsString (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsString (BeJsContextCR context, Utf8CP value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsString (BeJsContextCR context, Utf8StringCR value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    Utf8String GetValue() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetValue (Utf8CP value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    int64_t GetValueAsInt64() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    uint64_t GetValueAsUnsignedInt64() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsString CreateFromInt64 (BeJsContextCR context, int64_t value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsString CreateFromUnsignedInt64 (BeJsContextCR context, uint64_t value);
    };

//=======================================================================================
// A handle to a JavaScript object.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsObject : public BeJsValue
    {
    friend struct BeJsContext;

protected:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsObject (BeJsContextCR context);
public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsObject (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsObject (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsObject (BeJsContextCR context, JsValueRef jsrtValue);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    JsPropertyIdRef static GetJsrtPropertyId (const char* propertyName);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsObject (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // Creates an invalid BeJsObject handle.
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    BeJsObject();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsObject (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsObjectR GetValue();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsValue GetProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean GetBooleanProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNumber GetNumberProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsString GetStringProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsObject GetObjectProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer GetNativePointerProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsArray GetArrayProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsFunction GetFunctionProperty (Utf8CP name) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    template <typename... Arguments>
    BeJsValue CallMemberFunction (Utf8CP name, Arguments&&... arguments);
        
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetProperty (Utf8CP name, BeJsValueCR value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetBooleanProperty (Utf8CP name, bool value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetNumberProperty (Utf8CP name, double value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetNumberProperty (Utf8CP name, int32_t value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetStringProperty (Utf8CP name, Utf8CP value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void DeleteProperty (Utf8CP name);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool DoesPropertyExist (Utf8CP name);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetPrototype (BeJsValueCR prototype);

    //---------------------------------------------------------------------------------------
    // Creates a new empty object.
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsObject New (BeJsContextCR context);

    //---------------------------------------------------------------------------------------
    // Creates an empty object handle.
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsObject CreateHandle (BeJsContextCR context);
    };

//=======================================================================================
// A handle to a JavaScript object that wraps a native void*.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsNativePointer : public BeJsObject
    {
private:
    BeJsEnvironmentCP m_environment;

public:
    //---------------------------------------------------------------------------------------
    // Native callback that is invoked when the JavaScript object is disposed or garbage collected.
    //---------------------------------------------------------------------------------------
    typedef void (*DisposeCallback) (void* object, BeJsObject* jsObject);

    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // Creates an invalid BeJsNativePointer handle.
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer();

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer (BeJsContextCR context, void* object, DisposeCallback disposeCallback, BeJsObject const* prototype = nullptr, bool suppressCallContext = false);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void* GetValue() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    T& GetValueReference() const
        {
        void* value = GetValue();
        BeAssert (value != nullptr);
        return *static_cast<T*>(value);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    T* GetValueTyped() const
        {
        return static_cast<T*>(GetValue());
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetValue (void* object);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void Dispose();
    };

//=======================================================================================
// A handle to a JavaScript array.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsArray : public BeJsObject
    {
private:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsArray (BeJsContextCR context);

public:
    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsArray (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsArray (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsArray (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsArray (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsArray (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsValue GetProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsBoolean GetBooleanProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNumber GetNumberProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsString GetStringProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsObject GetObjectProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNativePointer GetNativePointerProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsArray GetArrayProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsFunction GetFunctionProperty (uint32_t index) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetProperty (uint32_t index, BeJsValueCR value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetBooleanProperty (uint32_t index, bool value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetNumberProperty (uint32_t index, double value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetNumberProperty (uint32_t index, int32_t value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void SetStringProperty (uint32_t index, Utf8CP value);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    void DeleteProperty (uint32_t index);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    bool DoesPropertyExist (uint32_t index);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    uint32_t GetLength() const;

    //---------------------------------------------------------------------------------------
    // Creates a new empty array.
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsArray New (BeJsContextCR context);
    };

//=======================================================================================
// A handle to a JavaScript function.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsFunction : public BeJsObject
    {
private:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsValue Call (BeJsValue** args,
                    BeJsEnvironment::EngineValueHandleType* engineArgs,
                    size_t argumentCount,
                    BeJsObject* callScope,
                    bool callAsConstructor) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    template <typename... Arguments>
    BeJsValue _Call (BeJsObject* callScope, bool callAsConstructor, Arguments&&... arguments) const
        {
        const size_t argumentCount = sizeof...(Arguments);
        BeJsValue* values [argumentCount == 0 ? 1 : argumentCount] = { &arguments... };
        BeJsEnvironment::EngineValueHandleType engineValues [argumentCount == 0 ? 1 : (argumentCount + 1)];
        return Call (values, engineValues, argumentCount, callScope, callAsConstructor);
        }

protected:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsFunction (BeJsContextCR context);

public:

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      8/16
    //---------------------------------------------------------------------------------------
    BeJsValue CallWithList(BeJsObject* callScope, bool callAsConstructor, BeJsValue* valuesIn, size_t argumentCount) const
        {
        BeJsEnvironment::EngineValueHandleType engineValues[32];
        BeJsValue* values[32];
        if (argumentCount >= 32)
            {
            BeAssert(false && "exceeded maximum number of arguments");
            return BeJsValue::InvalidHandle();
            }
        for (size_t i=0; i<argumentCount; ++i)
            values[i] = &valuesIn[i];
        return Call(values, engineValues, argumentCount, callScope, callAsConstructor);
        }

    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsFunction (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsFunction (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsFunction (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsFunction (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsFunction (BeJsValueCR other);

    //---------------------------------------------------------------------------------------
    // Calls the function with a scope object (can be null) and zero or more BeJsValue arguments.
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    template <typename... Arguments>
    BeJsValue Call (BeJsObject* callScope, Arguments&&... arguments) const
        {
        return _Call (callScope, false, arguments...);
        }

    //---------------------------------------------------------------------------------------
    // Calls the function as a constructor with zero or more BeJsValue arguments.
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    template <typename... Arguments>
    BeJsValue CallAsConstructor (Arguments&&... arguments) const
        {
        return _Call (nullptr, true, arguments...);
        }

    //---------------------------------------------------------------------------------------
    // Calls the function with zero or more BeJsValue arguments.
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    template <typename... Arguments>
    BeJsValue operator() (Arguments&&... arguments) const
        {
        return _Call (nullptr, false, arguments...);
        }

    //---------------------------------------------------------------------------------------
    // Creates an empty object handle.
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    static BeJsFunction CreateHandle (BeJsContextCR context);
    };

//=======================================================================================
// A handle to a JavaScript function that invokes a native function.
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsNativeFunction : public BeJsFunction
    {
private:
    std::function<void(void)> m_removeCallbackFunction = nullptr;

    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    template <typename T> static void V8Callback (const v8::FunctionCallbackInfo<v8::Value>& args);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    template <typename T> static JSValueRef JscCallback (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    template <typename T> static JSObjectRef JscCallbackConstructor (JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    template <typename T> static JsValueRef CALLBACK JsrtCallback (JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    template <typename T> static duk_ret_t DukCallback (duk_context* ctx);
    #endif

    template <typename T> void Init(BeJsContextCR context, T callback);
public:
    ~BeJsNativeFunction()
        {
        m_removeCallbackFunction = nullptr;
        }
    //=======================================================================================
    // Describes a BeJsNativeFunction function call.
    // @bsiclass                                                    Steve.Wilson    2/15
    //=======================================================================================
    struct CallbackInfo
        {
        friend struct BeJsNativeFunction;

    private:
        BeJsContext const* m_context;

        #if defined (BEJAVASCRIPT_V8_ENGINE)
        const v8::FunctionCallbackInfo<v8::Value>& m_v8CallbackInfo;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Steve.Wilson                    4/15
        //---------------------------------------------------------------------------------------
        CallbackInfo (BeJsContextCR context, v8::FunctionCallbackInfo<v8::Value> const& v8CallbackInfo);
        #endif
        
        #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
        bool m_isConstructCall;
        JSObjectRef m_function;
        JSObjectRef m_thisObject;
        size_t m_argumentCount;
        JSValueRef const* m_arguments;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Steve.Wilson                    4/15
        //---------------------------------------------------------------------------------------
        CallbackInfo (BeJsContextCR context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[]);
        #endif

        #if defined (BEJAVASCRIPT_JSRT_ENGINE)
        JsValueRef m_function;
        bool m_isConstructCall;
        unsigned short m_argumentCount;
        JsValueRef* m_arguments;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Steve.Wilson                    4/15
        //---------------------------------------------------------------------------------------
        CallbackInfo (BeJsContextCR context, JsValueRef function, bool isConstructCall, unsigned short argumentCount, JsValueRef* arguments);
        #endif

        #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
        bool m_isConstructCall;
        uint32_t m_argumentCount;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Steve.Wilson                    4/15
        //---------------------------------------------------------------------------------------
        CallbackInfo (BeJsContextCR context, bool isConstructCall, uint32_t argumentCount);
        #endif

    public:
        #if defined (BEJAVASCRIPT_V8_ENGINE)
        //---------------------------------------------------------------------------------------
        // @bsimethod                                   Steve.Wilson                    4/15
        //---------------------------------------------------------------------------------------
        v8::FunctionCallbackInfo<v8::Value> const& GetV8CallbackInfo() const { return m_v8CallbackInfo; }
        #endif

        //---------------------------------------------------------------------------------------
        // The function that is being called.
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsNativeFunction GetFunction();

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsContextCR GetContext();

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        bool IsConstructCall();

        //---------------------------------------------------------------------------------------
        // The JavaScript object that is the scope of the function call.
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsObject GetCallContext();

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        uint32_t GetArgumentCount() const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsValue GetArgument (uint32_t index) const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsBoolean GetBooleanArgument (uint32_t index) const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsNumber GetNumberArgument (uint32_t index) const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsString GetStringArgument (uint32_t index) const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsObject GetObjectArgument (uint32_t index) const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsNativePointer GetNativePointerArgument (uint32_t index) const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsArray GetArrayArgument (uint32_t index) const;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        BeJsFunction GetFunctionArgument (uint32_t index) const;
        };

    typedef CallbackInfo& CallbackInfoR;

    #if defined (BEJAVASCRIPT_V8_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativeFunction (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle);
    #endif
    
    #if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativeFunction (BeJsContextCR context, JSValueRef jscValue);
    #endif

    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativeFunction (BeJsContextCR context, JsValueRef jsrtValue);
    #endif

    #if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    4/15
    //---------------------------------------------------------------------------------------
    BeJsNativeFunction (BeJsContextCR context, duk_uarridx_t dukStashIndex);
    #endif

    //---------------------------------------------------------------------------------------
    // Native callback for a BeJsNativeFunction.
    //---------------------------------------------------------------------------------------
    typedef BeJsValue (*Callback) (CallbackInfoR info);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNativeFunction (BeJsContextCR context, Callback callback);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Mykolas.Simutis    7/17
    //---------------------------------------------------------------------------------------
    BeJsNativeFunction (BeJsContextCR context, std::function<BeJsValue(CallbackInfoR info)> callback);

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Mykolas.Simutis    7/17
    // used when BeJsNativeFunction was created with std::function<BeJsValue(CallbackInfoR info) constructor.
    // std::function is a heavyweight object,
    // that we might want to manually delete before Javascript GC kicks in and calls Dispose function.
    // m_removeCallbackFunction is created on constructor, capturing object that actually holds the function.
    // Calling this method essentially makes BeJsNativeFunction invalid,
    // thus should be used only when we're sure that underlying javascript object is not going to be used anymore.
    // This will always work, since to remove the native callback one needs to keep BeJsNativeFunction alive.
    // As long as BeJsNativeFunction is alive, Dispose function will not be called.
    //---------------------------------------------------------------------------------------
    void RemoveNativeCallback()
        {
        if (m_removeCallbackFunction != nullptr)
            {
            m_removeCallbackFunction();
            m_removeCallbackFunction = nullptr;
            }
        }
    };

//=======================================================================================
// A handle to a JavaScript object that invokes native callbacks for property access requests.
// (Not available cross-platform yet due to lack of JsRT support before Windows 10)
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsNativeObject : public BeJsObject
    {
public:
    //=======================================================================================
    // Stores callbacks for a BeJsNativeObject.
    // @bsiclass                                                    Steve.Wilson    2/15
    //=======================================================================================
    struct Callbacks
        {
    public:
        //---------------------------------------------------------------------------------------
        // Native callback for getting a BeJsNativeObject property.
        //---------------------------------------------------------------------------------------
        typedef BeJsValue (*GetProperty) (BeJsNativePointerR object, Utf8CP name);

        //---------------------------------------------------------------------------------------
        // Native callback for setting a BeJsNativeObject property.
        //---------------------------------------------------------------------------------------
        typedef void (*SetProperty) (BeJsNativePointerR object, Utf8CP name, BeJsValueCR value);

        //---------------------------------------------------------------------------------------
        // Native callback for deleting a BeJsNativeObject property.
        //---------------------------------------------------------------------------------------
        typedef void (*DeleteProperty) (BeJsNativePointerR object, Utf8CP name);

        //---------------------------------------------------------------------------------------
        // Native callback for enumerating the properties of a BeJsNativeObject.
        //---------------------------------------------------------------------------------------
        typedef BeJsArray (*EnumerateProperties) (BeJsNativePointerR object);

        //---------------------------------------------------------------------------------------
        // Native callback for checking the existence of a BeJsNativeObject property.
        //---------------------------------------------------------------------------------------
        typedef bool (*DoesPropertyExist) (BeJsNativePointerR object, Utf8CP name);

        //---------------------------------------------------------------------------------------
        // Getter callback.
        //---------------------------------------------------------------------------------------
        GetProperty getProperty;

        //---------------------------------------------------------------------------------------
        // Setter callback.
        //---------------------------------------------------------------------------------------
        SetProperty setProperty;

        //---------------------------------------------------------------------------------------
        // Deleter callback.
        //---------------------------------------------------------------------------------------
        DeleteProperty deleteProperty;

        //---------------------------------------------------------------------------------------
        // Enumerator callback.
        //---------------------------------------------------------------------------------------
        EnumerateProperties enumerateProperties;

        //---------------------------------------------------------------------------------------
        // Query callback.
        //---------------------------------------------------------------------------------------
        DoesPropertyExist doesPropertyExist;

        //---------------------------------------------------------------------------------------
        // Disposer callback.
        //---------------------------------------------------------------------------------------
        BeJsNativePointer::DisposeCallback dispose;

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Steve.Wilson    2/15
        //---------------------------------------------------------------------------------------
        Callbacks (GetProperty getter,
                   SetProperty setter = nullptr,
                   DeleteProperty deleter = nullptr,
                   EnumerateProperties enumerator = nullptr,
                   DoesPropertyExist query = nullptr,
                   BeJsNativePointer::DisposeCallback disposer = nullptr);
        };

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    2/15
    //---------------------------------------------------------------------------------------
    BeJsNativeObject (BeJsContextCR context, Callbacks const& callbacks, void* object = nullptr);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson    2/15
//=======================================================================================
struct BeJsProjection : RefCountedBase
    {
    friend struct BeJsContext;
    friend struct BeProjected;

private:
    static BeAtomic<size_t> s_lastId;
    static size_t s_idOffset;
    static size_t Id;

protected:
    BeJsContextR m_context;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    BeJsProjection (BeJsContextR context) : m_context (context) { ; }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    virtual ~BeJsProjection() { ; }

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    BeJsNativePointer ObtainInstancePointer (T* instance, bool owner = false, BeJsObjectP customPrototype = nullptr) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    BeJsObject CopyInterfaceInstanceObject (T const* instance, BeJsObjectP objectToReuse = nullptr) const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    T CreateInterfaceInstance (BeJsObject const& data) const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson    7/15
//=======================================================================================
struct BeProjected
    {
    friend struct BeJsProjection;

private:
    BeJsNativePointer m_object;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    template <typename T>
    BeJsNativePointer GetJsObject (BeJsProjection const& projection, BeJsObjectP customPrototype = nullptr);

protected:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    virtual ~BeProjected();

public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    bool IsJsObjectValid() const;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Steve.Wilson    7/15
    //---------------------------------------------------------------------------------------
    virtual void OnDispose();
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson    9/15
//=======================================================================================
struct BeProjectedRefCounted : RefCounted<BeProjected>
    {
public:
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Steve.Wilson                    9/15
    //---------------------------------------------------------------------------------------
    template <typename T, typename... Arguments>
    static RefCountedPtr<T> Create (Arguments&&... arguments)
        {
        return new T (std::forward<Arguments> (arguments)...);
        }
    };

//=======================================================================================
// Out-of-line template definitions
//=======================================================================================
template <typename... Arguments>
BeJsValue BeJsObject::CallMemberFunction (Utf8CP name, Arguments&&... arguments)
    {
    return GetFunctionProperty (name).Call (this, arguments...);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    7/15
//---------------------------------------------------------------------------------------
template <typename T>
T& BeJsContext::RegisterProjection (Utf8CP bootstrapScript, Utf8CP bootstrapIdentifier, Utf8CP setupScript, Utf8CP setupIdentifier)
    {
    if (T::Id == 0)
        {
        if (BeJsProjection::s_lastId == 0)
            BeJsProjection::s_idOffset = m_projections.size();

        T::Id = ++BeJsProjection::s_lastId;
        }

    if (m_projections.size() <= (T::Id + BeJsProjection::s_idOffset))
        m_projections.resize (T::Id + BeJsProjection::s_idOffset);

    if (setupScript != nullptr)
        EvaluateScript (setupScript, setupIdentifier);

    T* projection = new T (*this);
    m_projections [T::Id + BeJsProjection::s_idOffset - 1] = projection;

    if (bootstrapScript != nullptr)
        EvaluateScript (bootstrapScript, bootstrapIdentifier);

    GetGlobalObject().GetObjectProperty ("BentleyApi").GetObjectProperty ("BeJavaScript").GetFunctionProperty ("OnProjectionInitialized")();

    return *projection;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    7/15
//---------------------------------------------------------------------------------------
template <typename T>
BeJsNativePointer BeJsContext::ObtainProjectedClassInstancePointer (T* instance, bool owner, BeJsObjectP customPrototype) const
    {
    return GetProjection<T>().ObtainInstancePointer (instance, owner, customPrototype);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    7/15
//---------------------------------------------------------------------------------------
template <typename T>
BeJsObject BeJsContext::CopyProjectedInterfaceInstanceObject (T const* instance, BeJsObjectP objectToReuse) const
    {
    return GetProjection<T>().CopyInterfaceInstanceObject (instance, objectToReuse);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    7/15
//---------------------------------------------------------------------------------------
template <typename T>
T BeJsContext::CreateProjectedInterfaceInstance (BeJsObject const& data) const
    {
    return GetProjection<T>().template CreateInterfaceInstance<T> (data);
    }

END_BENTLEY_NAMESPACE

//=======================================================================================
// @bsimethod                                                    Steve.Wilson    4/15
//=======================================================================================
#define BEJAVASCRIPT_NUMBER(__object__) BeJsNumber (GetJsContext(), static_cast<double>(__object__))

//=======================================================================================
// @bsimethod                                                    Steve.Wilson    4/15
//=======================================================================================
#define BEJAVASCRIPT_NUMBER_INT32(__object__) BeJsNumber (GetJsContext(), static_cast<int32_t>(__object__))

//=======================================================================================
// @bsimethod                                                    Steve.Wilson    4/15
//=======================================================================================
#define BEJAVASCRIPT_NUMBER_UINT32(__object__) BeJsNumber (GetJsContext(), static_cast<uint32_t>(__object__))

//=======================================================================================
// @bsimethod                                                    Steve.Wilson    4/15
//=======================================================================================
#define BEJAVASCRIPT_NUMBER_UINT8(__object__) BeJsNumber (GetJsContext(), static_cast<uint8_t>(__object__))

//=======================================================================================
// @bsimethod                                                    Steve.Wilson    4/15
//=======================================================================================
#define BEJAVASCRIPT_BOOLEAN(__object__) BeJsBoolean (GetJsContext(), __object__)

//=======================================================================================
// @bsimethod                                                    Steve.Wilson    4/15
//=======================================================================================
#define BEJAVASCRIPT_STRING(__object__) BeJsString (GetJsContext(), __object__)

//__PUBLISH_SECTION_END__
