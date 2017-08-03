/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/iModelJs/iModelJsUtilities.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <iModelJs/iModelJs.h>
#include <functional>
#include <vector>

#define BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE  BEGIN_BENTLEY_IMODELJS_NAMESPACE namespace Js {
#define END_BENTLEY_IMODELJS_JS_NAMESPACE    } END_BENTLEY_IMODELJS_NAMESPACE

#define IMODELJS_JS_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_IMODELJS_JS_NAMESPACE

#define IMODELJS_JS_REF_COUNTED_PTR(_sname_) \
    BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE struct _sname_; DEFINE_REF_COUNTED_PTR(_sname_) END_BENTLEY_IMODELJS_JS_NAMESPACE

IMODELJS_JS_TYPEDEFS (Runtime)

IMODELJS_JS_TYPEDEFS (Scope)
IMODELJS_JS_TYPEDEFS (Value)
IMODELJS_JS_TYPEDEFS (Undefined)
IMODELJS_JS_TYPEDEFS (Null)
IMODELJS_JS_TYPEDEFS (Boolean)
IMODELJS_JS_TYPEDEFS (Number)
IMODELJS_JS_TYPEDEFS (String)
IMODELJS_JS_TYPEDEFS (Object)
IMODELJS_JS_TYPEDEFS (Array)
IMODELJS_JS_TYPEDEFS (ArrayBuffer)
IMODELJS_JS_TYPEDEFS (Function)
IMODELJS_JS_TYPEDEFS (Callback)
IMODELJS_JS_TYPEDEFS (Pointer)
IMODELJS_JS_TYPEDEFS (External)
IMODELJS_JS_TYPEDEFS (Reference)
IMODELJS_JS_TYPEDEFS (CallbackInfo)
IMODELJS_JS_TYPEDEFS (Promise)

IMODELJS_JS_TYPEDEFS (RuntimeInternal)
IMODELJS_JS_TYPEDEFS (ScopeInternal)

BEGIN_BENTLEY_IMODELJS_JS_NAMESPACE

typedef std::function<Value(CallbackInfoCR)> CallbackHandler_T;

enum class EvaluateStatus { Success, ParseError, RuntimeError };

static constexpr uint64_t MAX_SAFE_JS_INTEGER = 9007199254740991; //2^53 - 1

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Value
    {
    friend struct Runtime;
    friend struct ScopeInternal;
    friend struct Function;
    friend struct CallbackInfo;
    friend struct Array;
    friend struct Object;
    friend struct Reference;

public:
    enum class Type {
        None,
        Undefined,
        Null,
        Boolean,
        Number,
        String,
        Object,
        Array,
        ArrayBuffer,
        Function,
        Callback,
        External,
        Pointer
    };

private:
    void* m_data;

protected:
    Value (void* data = nullptr) : m_data (data) { ; }

public:
    IMODELJS_EXPORT bool operator== (ValueCR b) const;
    IMODELJS_EXPORT bool operator!= (ValueCR b) const { return !(*this == b); }

    IMODELJS_EXPORT bool IsEmpty() const { return m_data == nullptr; }
    
    IMODELJS_EXPORT bool IsUndefined() const;
    IMODELJS_EXPORT bool IsNull() const;
    IMODELJS_EXPORT bool IsBoolean() const;
    IMODELJS_EXPORT bool IsNumber() const;
    IMODELJS_EXPORT bool IsString() const;
    IMODELJS_EXPORT bool IsObject() const;
    IMODELJS_EXPORT bool IsArray() const;
    IMODELJS_EXPORT bool IsArrayBuffer() const;
    IMODELJS_EXPORT bool IsFunction() const;
    IMODELJS_EXPORT bool IsExternal() const;
    IMODELJS_EXPORT bool IsPointer() const;

    IMODELJS_EXPORT Undefined AsUndefined() const;
    IMODELJS_EXPORT Null AsNull() const;
    IMODELJS_EXPORT Boolean AsBoolean() const;
    IMODELJS_EXPORT Number AsNumber() const;
    IMODELJS_EXPORT String AsString() const;
    IMODELJS_EXPORT Object AsObject() const;
    IMODELJS_EXPORT Array AsArray() const;
    IMODELJS_EXPORT ArrayBuffer AsArrayBuffer() const;
    IMODELJS_EXPORT Function AsFunction() const;
    IMODELJS_EXPORT Callback AsCallback() const;
    IMODELJS_EXPORT External AsExternal() const;
    IMODELJS_EXPORT Pointer AsPointer() const;

    template <typename T>
    T AsNoTypeCheck() const
        {
        return m_data;
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Reference
    {
    friend struct ReferenceInternal;

private:
    RuntimeCP m_runtime;
    void* m_data;

    void PerformAssign (ValueCR value);

    Reference (ReferenceCR other) = delete;
    ReferenceR operator= (ReferenceCR other) = delete;

public:
    IMODELJS_EXPORT Reference() : m_runtime (nullptr), m_data (nullptr) { ; }
    IMODELJS_EXPORT Reference (RuntimeCR runtime) : m_runtime (&runtime), m_data (nullptr) { ; }
    IMODELJS_EXPORT Reference (RuntimeCR runtime, ValueCR value);

    IMODELJS_EXPORT ~Reference();

    IMODELJS_EXPORT void Assign (ValueCR value);
    IMODELJS_EXPORT void Assign (RuntimeCR runtime, ValueCR value);
    IMODELJS_EXPORT void Clear();
    IMODELJS_EXPORT bool IsEmpty() const { return m_data == nullptr; }
    IMODELJS_EXPORT Value Get() const;
    IMODELJS_EXPORT RuntimeCP GetRuntime() const { return m_runtime; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Runtime
    {
    friend struct RuntimeInternal;
    friend struct Scope;
    friend struct Promise;

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
        Value value;
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
    Reference m_promiseConstructor;
    Reference m_errorConstructor;
    Reference m_promiseHarvester;

    Runtime (RuntimeCR other) = delete;
    RuntimeR operator= (RuntimeCR other) = delete;

    static void Initialize();
    
    void RegisterCallbackHandler (CallbackHandler_T& handler) const;
    void OnCreate();
    void OnDestroy();
    Object CreatePromise (PromiseR destination) const;
    ReferenceCR GetErrorConstructor() const { return m_errorConstructor; }

public:
    IMODELJS_EXPORT static void* GetInitializationData();

    IMODELJS_EXPORT Runtime (Utf8CP name = nullptr, bool startDebugger = true, uint16_t debuggerPort = 0);
    IMODELJS_EXPORT ~Runtime();

    IMODELJS_EXPORT EvaluateResult EvaluateScript (Utf8CP script, Utf8CP identifier = nullptr);
    IMODELJS_EXPORT Object GetGlobal() const;
    IMODELJS_EXPORT void ThrowException (ValueCR value);
    IMODELJS_EXPORT Engine GetEngine() const { return m_engine; }
    IMODELJS_EXPORT Utf8CP GetName() const { return m_name; }
    IMODELJS_EXPORT DebuggerP GetDebugger() const { return m_debugger; }
    IMODELJS_EXPORT void StartDebugger (uint16_t port);
    IMODELJS_EXPORT void NotifyIdle();
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Scope
    {
    friend struct ScopeInternal;
    friend struct CallbackInfo;

private:
    RuntimeCR m_runtime;
    ScopeInternalP m_impl;

    Scope (RuntimeCR runtime, bool create);

    Scope (ScopeCR other) = delete;
    ScopeR operator= (ScopeCR other) = delete;

public:
    IMODELJS_EXPORT Scope (RuntimeCR runtime);
    IMODELJS_EXPORT ~Scope();

    IMODELJS_EXPORT RuntimeCR GetRuntime() const { return m_runtime; }

    IMODELJS_EXPORT Undefined CreateUndefined();
    IMODELJS_EXPORT Null CreateNull();
    IMODELJS_EXPORT Boolean CreateBoolean (bool value);
    IMODELJS_EXPORT Number CreateNumber (double value);
    IMODELJS_EXPORT String CreateString (Utf8CP value);
    IMODELJS_EXPORT Object CreateObject();
    IMODELJS_EXPORT Array CreateArray();
    IMODELJS_EXPORT ArrayBuffer CreateArrayBuffer (void* data, size_t length);
    IMODELJS_EXPORT ArrayBuffer CreateArrayBuffer (size_t length);
    IMODELJS_EXPORT Callback CreateCallback (CallbackHandler_T const& handler);
    IMODELJS_EXPORT External CreateExternal (void* data);
    IMODELJS_EXPORT Pointer CreatePointer (void* data);
    IMODELJS_EXPORT Object CreateError (ValueCR value);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Undefined : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    Undefined (void* data) : Value (data) { ; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Null : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    Null (void* data) : Value (data) { ; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Boolean : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    Boolean (void* data) : Value (data) { ; }

public:
    IMODELJS_EXPORT bool GetValue() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Number : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    Number (void* data) : Value (data) { ; }

public:
    IMODELJS_EXPORT double GetValue() const;

    template <typename T>
    T CastValue() const
        {
        return static_cast<T>(GetValue());
        }

    template <typename TValue, typename TOther>
    bool IsEqual (TOther const& other) const
        {
        return CastValue<TValue>() == static_cast<TValue>(other);
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct String : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    String (void* data) : Value (data) { ; }

public:
    IMODELJS_EXPORT Utf8String GetValue() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Object : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;
    friend struct Runtime;
    friend struct CallbackInfo;
    friend struct Function;

protected:
    Object (void* data) : Value (data) { ; }

public:
    IMODELJS_EXPORT Value Get (Utf8CP key, bool clear = false) const;
    IMODELJS_EXPORT bool Has (Utf8CP key) const;
    IMODELJS_EXPORT bool HasOwn (Utf8CP key) const;
    IMODELJS_EXPORT Object GetAsObject (Utf8CP key) const;
    IMODELJS_EXPORT bool Set (Utf8CP key, ValueCR value);
    IMODELJS_EXPORT bool SetPrototype (ValueCR value);
    IMODELJS_EXPORT Value GetPrototype() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Array : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    Array (void* data) : Value (data) { ; }

public:
    IMODELJS_EXPORT uint32_t GetLength() const;
    IMODELJS_EXPORT Value Get (uint32_t key) const;
    IMODELJS_EXPORT bool Set (uint32_t key, ValueCR value);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct ArrayBuffer: public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    ArrayBuffer (void* data) : Value (data) { ; }

public:
    IMODELJS_EXPORT size_t GetLength() const;
    IMODELJS_EXPORT void* GetValue() const;
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Function : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    IMODELJS_EXPORT Value CallInternal (uint32_t argc, ValueP argv, ObjectCP scope, bool construct) const;

protected:
    Function (void* data) : Value (data) { ; }

public:
    template <typename... Arguments>
    Object Construct (Arguments&&... arguments) const
        {
        const auto argc = sizeof... (Arguments);
        Value argv [argc + 1] = { arguments... };

        return CallInternal (argc, argv, nullptr, true).m_data;
        }

    template <typename... Arguments>
    Value operator() (ObjectCR scope, Arguments&&... arguments) const
        {
        const auto argc = sizeof... (Arguments);
        Value argv [argc + 1] = { arguments... };

        return CallInternal (argc, argv, &scope, false);
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Callback : public Function
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    Callback (void* data) : Function (data) { ; }

public:
    IMODELJS_EXPORT Callback() : Callback (nullptr) { ; }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct External : public Object
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    External (void* data) : Object (data) { ; }

public:
    IMODELJS_EXPORT void* GetValue() const;

    template <typename T>
    T* Cast() const
        {
        return reinterpret_cast<T*>(GetValue());
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Pointer : public Value
    {
    friend struct Scope;
    friend struct Reference;
    friend struct Value;

private:
    Pointer (void* data) : Value (data) { ; }

public:
    IMODELJS_EXPORT void* GetValue() const;

    template <typename T>
    T* Cast() const
        {
        return reinterpret_cast<T*>(GetValue());
        }
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct Promise
    {
    friend struct Runtime;

private:
    Reference m_resolveFunction;
    Reference m_rejectFunction;

public:
    IMODELJS_EXPORT bool IsInitialized() const { return !m_resolveFunction.IsEmpty() && !m_rejectFunction.IsEmpty(); }
    IMODELJS_EXPORT RuntimeCP GetRuntime() const;
    IMODELJS_EXPORT Object Initialize (ScopeR scope);
    IMODELJS_EXPORT void Resolve (ValueCR value);
    IMODELJS_EXPORT void Reject (ValueCR reason);
    };

//=======================================================================================
// @bsiclass                                                    Steve.Wilson   7/17
//=======================================================================================
struct CallbackInfo
    {
    friend struct RuntimeInternal;

private:
    RuntimeR m_runtime;
    void* m_data;
    mutable Scope m_scope;

    CallbackInfo (RuntimeR runtime, void* data);

    CallbackInfo (CallbackInfoCR other) = delete;
    CallbackInfoR operator= (CallbackInfoCR other) = delete;

public:
    IMODELJS_EXPORT uint32_t GetArgumentCount() const;
    IMODELJS_EXPORT Object GetThis() const;
    IMODELJS_EXPORT bool IsConstructCall() const;
    IMODELJS_EXPORT RuntimeR GetRuntime() const { return m_runtime; }
    IMODELJS_EXPORT ScopeR GetScope() const { return m_scope; }
    IMODELJS_EXPORT Value GetArgument (uint32_t index) const;

    
    template <typename T>
    T* CastThis() const
        {
        return GetThis().AsExternal().Cast<T>();
        }
    };

static_assert (sizeof (Undefined)   == sizeof (Value) && alignof (Undefined)   == alignof (Value), "Js::Undefined does not match Js::Value");
static_assert (sizeof (Null)        == sizeof (Value) && alignof (Null)        == alignof (Value), "Js::Null does not match Js::Value");
static_assert (sizeof (Boolean)     == sizeof (Value) && alignof (Boolean)     == alignof (Value), "Js::Boolean does not match Js::Value");
static_assert (sizeof (Number)      == sizeof (Value) && alignof (Number)      == alignof (Value), "Js::Number does not match Js::Value");
static_assert (sizeof (String)      == sizeof (Value) && alignof (String)      == alignof (Value), "Js::String does not match Js::Value");
static_assert (sizeof (Object)      == sizeof (Value) && alignof (Object)      == alignof (Value), "Js::Object does not match Js::Value");
static_assert (sizeof (Array)       == sizeof (Value) && alignof (Array)       == alignof (Value), "Js::Array does not match Js::Value");
static_assert (sizeof (ArrayBuffer) == sizeof (Value) && alignof (ArrayBuffer) == alignof (Value), "Js::ArrayBuffer does not match Js::Value");
static_assert (sizeof (Function)    == sizeof (Value) && alignof (Function)    == alignof (Value), "Js::Function does not match Js::Value");
static_assert (sizeof (Callback)    == sizeof (Value) && alignof (Callback)    == alignof (Value), "Js::Callback does not match Js::Value");
static_assert (sizeof (External)    == sizeof (Value) && alignof (External)    == alignof (Value), "Js::External does not match Js::Value");
static_assert (sizeof (Pointer)     == sizeof (Value) && alignof (Pointer)     == alignof (Value), "Js::Pointer does not match Js::Value");

END_BENTLEY_IMODELJS_JS_NAMESPACE

#define JS_CALLBACK_RUNTIME info.GetScope().GetRuntime()

#define JS_CALLBACK_UNDEFINED info.GetScope().CreateUndefined()
#define JS_CALLBACK_NULL info.GetScope().CreateNull()
#define JS_CALLBACK_BOOLEAN(_value_) info.GetScope().CreateBoolean (_value_)
#define JS_CALLBACK_NUMBER(_value_) info.GetScope().CreateNumber (_value_)
#define JS_CALLBACK_STRING(_value_) info.GetScope().CreateString (_value_)
#define JS_CALLBACK_OBJECT info.GetScope().CreateObject()
#define JS_CALLBACK_ARRAY info.GetScope().CreateArray()
#define JS_CALLBACK_ARRAY_BUFFER(_data_, _length_) info.GetScope().CreateArrayBuffer (_data_, _length_)
#define JS_CALLBACK_CALLBACK(_handler_) info.GetScope().CreateCallback (_handler_)
#define JS_CALLBACK_EXTERNAL(_data_) info.GetScope().CreateExternal (_data_)
#define JS_CALLBACK_POINTER(_data_) info.GetScope().CreatePointer (_data_)
#define JS_CALLBACK_ERROR(_value_) info.GetScope().CreateError (_value_)

#define JS_CALLBACK_REQUIRE(_condition_, _message_) \
    if (!(_condition_)) \
        { \
        auto undefined = JS_CALLBACK_UNDEFINED; \
        info.GetRuntime().ThrowException (JS_CALLBACK_STRING (_message_)); \
        return undefined; \
        }

#define JS_CALLBACK_REQUIRE_EXTERNAL_THIS JS_CALLBACK_REQUIRE (info.GetThis().IsExternal(), "Invalid object.")
#define JS_CALLBACK_REQUIRE_N_ARGS(_argCount_) JS_CALLBACK_REQUIRE (info.GetArgumentCount() == _argCount_, "A required argument is missing.")
#define JS_CALLBACK_REQUIRE_AT_LEAST_N_ARGS(_argCount_) JS_CALLBACK_REQUIRE (info.GetArgumentCount() >= _argCount_, "A required argument is missing.")

#define _JS_CALLBACK_GET_ARG(_argIndex_, _argType_) \
    info.GetArgument (_argIndex_).AsNoTypeCheck<BentleyApi::iModelJs::Js::##_argType_>(); \
    if (!info.GetArgument (_argIndex_).Is##_argType_()) \
        { \
        auto undefined = JS_CALLBACK_UNDEFINED; \
        info.GetRuntime().ThrowException (JS_CALLBACK_STRING ("Argument type is invalid.")); \
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

//__PUBLISH_SECTION_END__
