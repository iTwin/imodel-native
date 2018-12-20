/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/BeJavaScript.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BeJavaScript/BeJavaScript.h>

#if defined(BEJAVASCRIPT_V8_ENGINE)
    #include <google_v8/libplatform/libplatform.h>
#endif

#include <mutex>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    4/15
//---------------------------------------------------------------------------------------
void BeProjected::OnDispose()
    {
    if (m_object.IsValid())
        m_object.SetValue (nullptr);

    m_object.Invalidate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    4/15
//---------------------------------------------------------------------------------------
bool BeProjected::IsJsObjectValid() const
    {
    return m_object.IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    4/15
//---------------------------------------------------------------------------------------
BeProjected::~BeProjected()
    {
    if (m_object.IsValid())
        m_object.SetValue (nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    4/15
//---------------------------------------------------------------------------------------
void BeJsContext::Initialize (Utf8CP initializationScript, Utf8CP initializationScriptIdentifier)
    {
    m_callContextActive = 0;

    m_JSON_parse = new BeJsFunction (GetGlobalObject().GetObjectProperty ("JSON").GetFunctionProperty ("parse"));
    m_JSON_stringify = new BeJsFunction (GetGlobalObject().GetObjectProperty ("JSON").GetFunctionProperty ("stringify"));

    EvaluateScript (_GetBeJavaScriptJsSource(), "file:///BeJavaScript.js");

    if (initializationScript != nullptr)
        {
        Utf8String identifier ("file:///");
        identifier += initializationScriptIdentifier;
        identifier += ".js";
        EvaluateScript (initializationScript, identifier.c_str());
        }
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    4/15
//---------------------------------------------------------------------------------------
void BeJsContext::_OnDestroy()
    {
    delete m_JSON_parse;
    delete m_JSON_stringify;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    2/15
//---------------------------------------------------------------------------------------
BeJsValue BeJsContext::EvaluateJson (Utf8CP json)
    {
    return (*m_JSON_parse) (BeJsString (*this, json));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    8/15
//---------------------------------------------------------------------------------------
Utf8String BeJsValue::Serialize()
    {
    BeJsFunctionR jsonStringify = *m_context->m_JSON_stringify;

    return BeJsString (jsonStringify (*this)).GetValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    2/15
//---------------------------------------------------------------------------------------
BeJsValue BeJsContext::EvaluateJson (Json::Value const& json)
    {
    return EvaluateJson (Json::FastWriter().write (json).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    2/15
//---------------------------------------------------------------------------------------
BeJsContextR BeJsContext::GetJsContext()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    7/15
//---------------------------------------------------------------------------------------
bool BeJsContext::IsCallContextActive()
    {
    return 0 != m_callContextActive;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    7/15
//---------------------------------------------------------------------------------------
void BeJsContext::BeginCallContext()
    {
    ++m_callContextActive;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    7/15
//---------------------------------------------------------------------------------------
void BeJsContext::EndCallContext()
    {
    BeAssert (0 != m_callContextActive);
    if (1 != m_callContextActive--)
        return;

    for (auto& value : m_callContextValues)
        value.Dispose();

    m_callContextValues.clear();
    m_callContextActive = 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsObjectR BeJsObject::GetValue()
    {
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsObject BeJsObject::CreateHandle (BeJsContextCR context)
    {
    return BeJsObject (context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsString::BeJsString (BeJsContextCR context, Utf8StringCR value)
    : BeJsString (context, value.c_str())
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsString BeJsString::CreateFromInt64 (BeJsContextCR context, int64_t value)
    {
    Utf8String valueString;
    valueString.Sprintf ("%lld", value);

    return BeJsString (context, valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsString BeJsString::CreateFromUnsignedInt64 (BeJsContextCR context, uint64_t value)
    {
    Utf8String valueString;
    valueString.Sprintf ("%llu", value);

    return BeJsString (context, valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsFunction BeJsFunction::CreateHandle (BeJsContextCR context)
    {
    return BeJsFunction (context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsValue::BeJsValue()
    : m_context (nullptr)
    {
    #if defined (BEJAVASCRIPT_JSRT_ENGINE)
        m_jsrtValue = nullptr;
    #endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
bool BeJsValue::IsValid() const
    {
    return m_context != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsValue BeJsValue::InvalidHandle()
    {
    return BeJsValue();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    9/15
//---------------------------------------------------------------------------------------
void BeJsValue::Invalidate()
    {
    if (m_context != nullptr)
        Reset();

    m_context = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsObject::BeJsObject()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Steve.Wilson    2/15
//---------------------------------------------------------------------------------------
BeJsNativePointer::BeJsNativePointer()
    : m_environment (nullptr)
    {
    }

BeAtomic<size_t> BeJsProjection::s_lastId (0);
size_t BeJsProjection::s_idOffset = 0;
size_t BeJsProjection::Id = 0;

#if defined (BEJAVASCRIPT_V8_ENGINE)

namespace {

struct _BeJsNativeObjectWrapper
    {
public:
    void* m_object;
    v8::Persistent<v8::Value> m_v8ValueHandle;
    _BeJsNativeObjectWrapper (void* object = nullptr) : m_object (object) {};
    };

struct _BeJsNativePointer : public _BeJsNativeObjectWrapper
    {
public:
    BeJsNativePointer::DisposeCallback m_callback;

    _BeJsNativePointer (BeJsContextCR context, void* object, BeJsNativePointer::DisposeCallback callback)
        : _BeJsNativeObjectWrapper (object), m_callback (callback)
        {
        };

    ~_BeJsNativePointer()
        {
        m_v8ValueHandle.Reset();
        };

    static void Dispose (v8::WeakCallbackInfo<_BeJsNativePointer> const& data)
        {
        _BeJsNativePointer* external = static_cast<_BeJsNativePointer*>(data.GetParameter());

        if (external->m_callback != nullptr)
            external->m_callback (external->m_object, nullptr);

        delete external;
        };

    static _BeJsNativeObjectWrapper* GetFromHandle (BeJsContextCR context, v8::Persistent<v8::Value> const& v8Value)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");

        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
        v8::Context::Scope contextScope (contextHandle);

        v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, v8Value);
        BeAssert (handle->IsObject() && "Cannot get a property of a JavaScript value that is not an object.");
        v8::Local<v8::Object> objectHandle = v8::Local<v8::Object>::Cast (handle);
        
        BeAssert (objectHandle->InternalFieldCount() != 0 && "JavaScript object has no internal fields.");
        v8::Local<v8::External> external = v8::Local<v8::External>::Cast (objectHandle->GetInternalField (0));
        
        return static_cast<_BeJsNativeObjectWrapper*>(external->Value());
        };
    };

template <class T> struct _BeJsNativeFunction : _BeJsNativeObjectWrapper
    {
private:
    void RemoveCallback()
        {
        m_callback = nullptr;
        }

public:
    BeJsContext const* m_context;
    T m_callback;

    _BeJsNativeFunction (BeJsContextCR context, T callback)
        : m_context (&context), m_callback (callback)
        {
        };

    ~_BeJsNativeFunction()
        {
        m_callback = nullptr;
        m_v8ValueHandle.Reset();
        };

    void Callback (BeJsNativeFunction::CallbackInfoR info)
        {
        if (m_callback != nullptr)
            info.GetV8CallbackInfo().GetReturnValue().Set (m_callback (info).GetV8ValueHandle());
        };

    static void Dispose (v8::WeakCallbackInfo<_BeJsNativeFunction<T>> const& data)
        {
        _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(data.GetParameter());
        delete proxy;
        };

    static void RemoveNativeCallback(_BeJsNativeFunction<T>* proxy)
        {
        proxy->RemoveCallback();
        };
    };

struct _BeJsNativeObject : public _BeJsNativeObjectWrapper
    {
public:
    BeJsNativeObject::Callbacks m_callbacks;
    BeJsContext const* m_context;

    _BeJsNativeObject (BeJsContextCR context, BeJsNativeObject::Callbacks callbacks, void* object)
        : _BeJsNativeObjectWrapper (object), m_callbacks (callbacks), m_context (&context)
        {
        }

    ~_BeJsNativeObject()
        {
        m_v8ValueHandle.Reset();
        };

    static void Dispose (v8::WeakCallbackInfo<_BeJsNativeObject> const& data)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(data.GetParameter());

        if (proxy->m_callbacks.dispose != nullptr)
            proxy->m_callbacks.dispose (proxy->m_object, nullptr);
        
        delete proxy;
        };

    static void DoesPropertyExist (v8::Local<v8::String> propertyName, v8::PropertyCallbackInfo<v8::Integer> const& info)
        {
        void* object = v8::Local<v8::External>::Cast (info.Data())->Value();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.doesPropertyExist != nullptr)
            {
            v8::Isolate* isolate = info.GetIsolate();
            v8::HandleScope handleScope (isolate);

            v8::Local<v8::Value> objectHandle = info.This();
            BeJsNativePointer external (*proxy->m_context, objectHandle);

            v8::String::Utf8Value propertyNameS (propertyName);
        
            bool exists = proxy->m_callbacks.doesPropertyExist (external, *propertyNameS);
            if (exists)
                info.GetReturnValue().Set (v8::Integer::New (isolate, v8::PropertyAttribute::None));
            }
        }

    static void GetProperty (v8::Local<v8::String> propertyName, v8::PropertyCallbackInfo<v8::Value> const& info)
        {
        void* object = v8::Local<v8::External>::Cast (info.Data())->Value();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.getProperty != nullptr)
            {
            v8::Isolate* isolate = info.GetIsolate();
            v8::HandleScope handleScope (isolate);

            v8::Local<v8::Value> objectHandle = info.This();
            BeJsNativePointer external (*proxy->m_context, objectHandle);

            v8::String::Utf8Value propertyNameS (propertyName);

            BeJsValue response = proxy->m_callbacks.getProperty (external, *propertyNameS);
            if (!response.GetV8ValueHandle().IsEmpty())
                info.GetReturnValue().Set (response.GetV8ValueHandle());
            }
        }

    static void SetProperty (v8::Local<v8::String> propertyName, v8::Local<v8::Value> propertyValue, v8::PropertyCallbackInfo<v8::Value> const& info)
        {
        void* object = v8::Local<v8::External>::Cast (info.Data())->Value();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.setProperty != nullptr)
            {
            v8::Isolate* isolate = info.GetIsolate();
            v8::HandleScope handleScope (isolate);

            v8::Local<v8::Value> objectHandle = info.This();
            BeJsNativePointer external (*proxy->m_context, objectHandle);

            v8::String::Utf8Value propertyNameS (propertyName);

            BeJsValue propertyValueObj (*proxy->m_context, propertyValue);

            proxy->m_callbacks.setProperty (external, *propertyNameS, propertyValueObj);
            info.GetReturnValue().Set (propertyValue);
            }
        }

    static void DeleteProperty (v8::Local<v8::String> propertyName, v8::PropertyCallbackInfo<v8::Boolean> const& info)
        {
        void* object = v8::Local<v8::External>::Cast (info.Data())->Value();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.deleteProperty != nullptr)
            {
            v8::Isolate* isolate = info.GetIsolate();
            v8::HandleScope handleScope (isolate);

            v8::Local<v8::Value> objectHandle = info.This();
            BeJsNativePointer external (*proxy->m_context, objectHandle);

            v8::String::Utf8Value propertyNameS (propertyName);
        
            proxy->m_callbacks.deleteProperty (external, *propertyNameS);
            info.GetReturnValue().Set (v8::True(isolate));
            }
        }

    static void EnumerateProperties (v8::PropertyCallbackInfo<v8::Array> const& info)
        {
        void* object = v8::Local<v8::External>::Cast (info.Data())->Value();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.enumerateProperties != nullptr)
            {
            v8::Isolate* isolate = info.GetIsolate();
            v8::HandleScope handleScope (isolate);

            v8::Local<v8::Value> objectHandle = info.This();
            BeJsNativePointer external (*proxy->m_context, objectHandle);

            BeJsArray response = proxy->m_callbacks.enumerateProperties (external);
            if (!response.GetV8ValueHandle().IsEmpty())
                {
                v8::Isolate* isolate = proxy->m_context->GetEnvironment().GetV8Isolate();
                v8::HandleScope handleScope (isolate);

                v8::Local<v8::Context> context = v8::Local<v8::Context>::New (isolate, proxy->m_context->GetV8ContextHandle());
                v8::Context::Scope contextScope (context);

                v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, response.GetV8ValueHandle());
                v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast (handle);
                info.GetReturnValue().Set (arr);
                }
            }
        }
    };

struct BeJsValueHelper
    {
public:
    template <typename F>
    static bool CheckType (BeJsContextCR context, v8::Persistent<v8::Value> const& v8Value, F valueFunction)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");

        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, v8Value);
        return (*handle->*valueFunction)();
        };

    template <typename F>
    static bool CheckEqual (BeJsContextCR context, v8::Persistent<v8::Value> const& v8Value, v8::Persistent<v8::Value> const& otherV8Value, F valueFunction)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");
        BeAssert (!otherV8Value.IsEmpty() && "Cannot access an empty JavaScript value.");

        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
        v8::Context::Scope contextScope (contextHandle);

        v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, v8Value);
        v8::Local<v8::Value> otherHandle = v8::Local<v8::Value>::New (isolate, otherV8Value);
        return (*handle->*valueFunction) (otherHandle);
        };

    template <typename T>
    static T GetArgument (BeJsContextCR context, v8::FunctionCallbackInfo<v8::Value> const& v8CallbackInfo, uint32_t index)
        {
        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Value> handle = v8CallbackInfo [index];
        return T (context, handle);
        };

    template <typename T, typename F>
    static T GetCallbackValue (BeJsContextCR context, v8::FunctionCallbackInfo<v8::Value> const& v8CallbackInfo, F callbackFunction)
        {
        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Value> handle = (v8CallbackInfo.*callbackFunction)();
        return T (context, handle);
        };

    template <typename T>
    static T _GetValue (v8::Local<v8::Value>& value)
        {
        };

    template <typename T>
    static T GetValue (BeJsContextCR context, v8::Persistent<v8::Value> const& v8Value)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");
        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);
        v8::Local<v8::Value> value = v8::Local<v8::Value>::New (isolate, v8Value);
        return _GetValue<T> (value);
        };

    template <typename T>
    static v8::Local<v8::Value> CreateHandle (v8::Isolate* isolate, T value)
        {
        };

    template <typename T>
    static void SetValue (BeJsContextCR context, v8::Persistent<v8::Value>& v8Value, T value, bool enterContext=false)
        {
        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        if (enterContext)
            {
            v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
            v8::Context::Scope contextScope (contextHandle);
            v8::Local<v8::Value> valueHandle = CreateHandle<T> (isolate, value);
            v8Value.Reset (isolate, valueHandle);
            }
        else
            {
            v8::Local<v8::Value> valueHandle = CreateHandle<T> (isolate, value);
            v8Value.Reset (isolate, valueHandle);
            }
        };
    };

template <>
bool BeJsValueHelper::_GetValue<bool> (v8::Local<v8::Value>& value)
    {
    BeAssert (value->IsBoolean());
    return value->BooleanValue();
    };

template <>
double BeJsValueHelper::_GetValue<double> (v8::Local<v8::Value>& value)
    {
    BeAssert (value->IsNumber());
    return value->NumberValue();
    };

template <>
int32_t BeJsValueHelper::_GetValue<int32_t> (v8::Local<v8::Value>& value)
    {
    BeAssert (value->IsInt32());
    return value->Int32Value();
    };

template <>
Utf8String BeJsValueHelper::_GetValue<Utf8String> (v8::Local<v8::Value>& value)
    {
    BeAssert (value->IsString());
    return Utf8String (*v8::String::Utf8Value (value));
    };

template <>
v8::Local<v8::Value> BeJsValueHelper::CreateHandle<v8::Persistent<v8::Value> const&> (v8::Isolate* isolate, v8::Persistent<v8::Value> const& value)
    {
    return v8::Local<v8::Value>::New (isolate, value);
    };

template <>
v8::Local<v8::Value> BeJsValueHelper::CreateHandle<bool> (v8::Isolate* isolate, bool value)
    {
    return v8::Boolean::New (isolate, value);
    };

template <>
v8::Local<v8::Value> BeJsValueHelper::CreateHandle<double> (v8::Isolate* isolate, double value)
    {
    return v8::Number::New (isolate, value);
    };

template <>
v8::Local<v8::Value> BeJsValueHelper::CreateHandle<int32_t> (v8::Isolate* isolate, int32_t value)
    {
    return v8::Number::New (isolate, value);
    };

template <>
v8::Local<v8::Value> BeJsValueHelper::CreateHandle<Utf8CP> (v8::Isolate* isolate, Utf8CP value)
    {
    return v8::String::NewFromUtf8 (isolate, value);
    };

template <>
v8::Local<v8::Value> BeJsValueHelper::CreateHandle<BeJsObject&> (v8::Isolate* isolate, BeJsObject& value)
    {
    return v8::Object::New(isolate);
    };

template <>
v8::Local<v8::Value> BeJsValueHelper::CreateHandle<BeJsArray&> (v8::Isolate* isolate, BeJsArray& value)
    {
    return v8::Array::New(isolate);
    };

template <typename T, typename U>
struct BeJsObjectHelper
    {
public:
    static v8::Local<v8::Value> _GetProperty (v8::Isolate* isolate, v8::Local<v8::Object>& object, uint32_t index)
        {
        return object->Get (index);
        };

    static v8::Local<v8::Value> _GetProperty (v8::Isolate* isolate, v8::Local<v8::Object>& object, Utf8CP index)
        {
        return object->Get (v8::String::NewFromUtf8 (isolate, index));
        };

    static T GetProperty (BeJsContextCR context, v8::Persistent<v8::Value> const& v8Value, U index)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");

        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
        v8::Context::Scope contextScope (contextHandle);

        v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, v8Value);
        BeAssert (handle->IsObject() && "Cannot get a property of a JavaScript value that is not an object.");
        v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast (handle);

        v8::Local<v8::Value> valueObject = _GetProperty (isolate, object, index);
        return T (context, valueObject);
        };

    static void _SetProperty (v8::Isolate* isolate, v8::Local<v8::Object>& object, v8::Local<v8::Value>& value, uint32_t index)
        {
        object->Set (index, value);
        };

    static void _SetProperty (v8::Isolate* isolate, v8::Local<v8::Object>& object, v8::Local<v8::Value>& value, Utf8CP index)
        {
        object->Set (v8::String::NewFromUtf8 (isolate, index), value);
        };

    static void SetProperty (BeJsContextCR context, v8::Persistent<v8::Value>& v8Value, T value, U index)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");

        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
        v8::Context::Scope contextScope (contextHandle);

        v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, v8Value);
        BeAssert (handle->IsObject() && "Cannot set a property of a JavaScript value that is not an object.");
        v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast (handle);

        v8::Local<v8::Value> valueObject = BeJsValueHelper::CreateHandle<T> (isolate, value);
        _SetProperty (isolate, object, valueObject, index);
        };

    static void _DeleteProperty (v8::Isolate* isolate, v8::Local<v8::Object>& object, uint32_t index)
        {
        object->Delete (index);
        };

    static void _DeleteProperty (v8::Isolate* isolate, v8::Local<v8::Object>& object, Utf8CP index)
        {
        object->Delete (v8::String::NewFromUtf8 (isolate, index));
        };

    static void DeleteProperty (BeJsContextCR context, v8::Persistent<v8::Value>& v8Value, U index)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");

        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
        v8::Context::Scope contextScope (contextHandle);

        v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, v8Value);
        BeAssert (handle->IsObject() && "Cannot access a property of a JavaScript value that is not an object.");
        v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast (handle);

        _DeleteProperty (isolate, object, index);
        };

    static bool _DoesPropertyExist (v8::Isolate* isolate, v8::Local<v8::Object>& object, uint32_t index)
        {
        return object->Has (index);
        };

    static bool _DoesPropertyExist (v8::Isolate* isolate, v8::Local<v8::Object>& object, Utf8CP index)
        {
        return object->Has (v8::String::NewFromUtf8 (isolate, index));
        };

    static bool DoesPropertyExist (BeJsContextCR context, v8::Persistent<v8::Value>& v8Value, U index)
        {
        BeAssert (!v8Value.IsEmpty() && "Cannot access an empty JavaScript value.");

        v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
        v8::HandleScope handleScope (isolate);

        v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
        v8::Context::Scope contextScope (contextHandle);

        v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, v8Value);
        BeAssert (handle->IsObject() && "Cannot access a property of a JavaScript value that is not an object.");
        v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast (handle);

        return _DoesPropertyExist (isolate, object, index);
        };
    };
    
}

BeJsEngine BeJsEnvironment::GetActiveEngine()
    {
    return BeJsEngine::V8;
    }

BeJsEnvironment::BeJsEnvironment()
    : m_destroying (false),
      m_ownsSystemEnvironment (true)
    {    
    v8::Isolate::CreateParams defaultParams;
    defaultParams.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    m_v8Isolate = v8::Isolate::New(defaultParams);
    
    m_v8IsolateLocker = new v8::Locker (m_v8Isolate);
    m_v8Isolate->Enter();
    }

BeJsEnvironment::BeJsEnvironment (BeJsEngine engine, void* systemEnvironment)
    : m_destroying (false),
      m_ownsSystemEnvironment (false)
    {
    BeAssert (engine == GetActiveEngine());

    m_v8Isolate = reinterpret_cast<v8::Isolate*>(systemEnvironment);
    m_v8IsolateLocker = new v8::Locker (m_v8Isolate);
    }

void* BeJsEnvironment::GetSystemObject()
    {
    return m_v8Isolate;
    }

BeJsEnvironment::~BeJsEnvironment()
    {
    m_destroying = true;
    
    if (m_ownsSystemEnvironment)
        m_v8Isolate->Exit();

    delete m_v8IsolateLocker;

    if (m_ownsSystemEnvironment)
        m_v8Isolate->Dispose();
    }

BeJsContext::BeJsContext (BeJsEnvironmentCR environment, Utf8CP identifier, Utf8CP initializationScript)
    : m_environment (&environment),
      m_ownsSystemContext (true)
    {
    v8::Isolate* isolate = m_environment->GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::ObjectTemplate> globalTemplate = v8::ObjectTemplate::New();
    v8::Local<v8::Context> context = v8::Context::New (isolate, nullptr, globalTemplate);
    m_v8ContextHandle.Reset (isolate, context);

    Initialize (initializationScript, identifier);
    }

BeJsContext::BeJsContext (BeJsEngine engine, BeJsEnvironmentCR environment, void* systemContext)
    : m_environment (&environment),
      m_ownsSystemContext (false)
    {
    BeAssert (engine == BeJsEnvironment::GetActiveEngine());

    v8::Isolate* isolate = m_environment->GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    m_v8ContextHandle.Reset (isolate, *reinterpret_cast<v8::Persistent<v8::Context>*>(systemContext));

    Initialize (nullptr, nullptr);
    }

void* BeJsContext::GetSystemObject()
    {
    return &m_v8ContextHandle;
    }

BeJsContext::~BeJsContext()
    {
    _OnDestroy();

    m_v8ContextHandle.Reset();

    if (m_ownsSystemContext)
        {
        v8::Isolate* isolate = m_environment->GetV8Isolate();
        isolate->ContextDisposedNotification();
        while (!isolate->IdleNotification(1000)) {};
        }
    }

BeJsValue BeJsContext::EvaluateScript (Utf8CP script, Utf8CP identifier, EvaluateStatus* status, EvaluateException* exception) const
    {
    v8::Isolate* isolate = m_environment->GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Context> context = v8::Local<v8::Context>::New (isolate, m_v8ContextHandle);
    v8::Context::Scope contextScope (context);

    v8::TryCatch tryCatch;

    v8::Local<v8::Script> scriptObject;
    if (identifier != nullptr)
        scriptObject = v8::Script::Compile (v8::String::NewFromUtf8 (isolate, script), v8::String::NewFromUtf8 (isolate, identifier));
    else
        scriptObject = v8::Script::Compile (v8::String::NewFromUtf8 (isolate, script));

    if (status == nullptr)
        {
        BeAssert (!scriptObject.IsEmpty() && "Script compilation error.");
        }
    else
        {
        if (scriptObject.IsEmpty())
            {
            *status = EvaluateStatus::ParseError;
            
            return BeJsValue::InvalidHandle();
            }
        }

    v8::Local<v8::Value> scriptValue = scriptObject->Run();

    if (status == nullptr)
        {
        if (tryCatch.HasCaught() || tryCatch.HasTerminated())
            BeAssert (false && "Script runtime error.");
        }
    else
        {
        if (tryCatch.HasCaught() || tryCatch.HasTerminated())
            {
            *status = EvaluateStatus::RuntimeError;

            if (exception != nullptr)
                {
                v8::String::Utf8Value v8Exception (tryCatch.Exception());
                (*exception).message = *v8Exception;

                v8::String::Utf8Value trace (tryCatch.StackTrace());
                (*exception).trace = *trace;
                }

            return BeJsValue::InvalidHandle();
            }
        else
            {
            *status = EvaluateStatus::Success;
            }
        }

    BeJsValue response (*this, scriptValue);
    response.m_v8ValueHandle.Reset (isolate, scriptValue);

    return response;
    }

void BeJsContext::ReportJavaScriptException (v8::TryCatch* tryCatch) const
    {
    v8::Isolate* isolate = GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::String::Utf8Value exception (tryCatch->Exception());
    v8::Local<v8::Message> message = tryCatch->Message();
    
    if (!message.IsEmpty())
        {
        v8::String::Utf8Value stackTrace (tryCatch->StackTrace());
        }
    }

BeJsObject BeJsContext::GetGlobalObject() const
    {
    v8::Isolate* isolate = m_environment->GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Context> context = v8::Local<v8::Context>::New (isolate, m_v8ContextHandle);
    v8::Context::Scope contextScope (context);

    v8::Local<v8::Value> global = context->Global();
    return BeJsObject (*this, global);
    }

BeJsValue::BeJsValue (BeJsContextCR context)
    : m_context (&context)
    {
    }

BeJsValue::BeJsValue (BeJsValueCR other)
    : m_context (other.m_context), m_v8ValueHandle (other.m_context->GetEnvironment().GetV8Isolate(), other.m_v8ValueHandle)
    {
    }

BeJsValueR BeJsValue::operator= (BeJsValueCR other)
    {
    if (this == &other)
        return *this;

    m_context = other.m_context;
    m_v8ValueHandle.Reset (m_context->GetEnvironment().GetV8Isolate(), other.m_v8ValueHandle);

    return *this;
    }

BeJsValue::BeJsValue (BeJsValue&& other)
    : m_context (other.m_context), m_v8ValueHandle (other.m_context->GetEnvironment().GetV8Isolate(), other.m_v8ValueHandle)
    {
    }

BeJsValue::BeJsValue (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : m_context (&context), m_v8ValueHandle (context.GetEnvironment().GetV8Isolate(), v8Handle)
    {
    }
    
BeJsValue::~BeJsValue()
    {
    if (m_context == nullptr)
        return;

    m_v8ValueHandle.Reset();
    }

void BeJsValue::Reset()
    {
    m_v8ValueHandle.Reset();
    }

bool BeJsValue::IsEmpty() const
    {
    return m_v8ValueHandle.IsEmpty();
    }

void BeJsValue::NotifyEngineValueReleased()
    {
    m_v8ValueHandle.Reset();
    }

bool BeJsValue::IsNull() const
    {
    return BeJsValueHelper::CheckType (*m_context, m_v8ValueHandle, &v8::Value::IsNull);
    }

bool BeJsValue::IsUndefined() const
    {
    return BeJsValueHelper::CheckType (*m_context, m_v8ValueHandle, &v8::Value::IsUndefined);
    }

bool BeJsValue::IsNumber() const
    {
    return BeJsValueHelper::CheckType (*m_context, m_v8ValueHandle, &v8::Value::IsNumber);
    }

bool BeJsValue::IsBoolean() const
    {
    return BeJsValueHelper::CheckType (*m_context, m_v8ValueHandle, &v8::Value::IsBoolean);
    }

bool BeJsValue::IsString() const
    {
    return BeJsValueHelper::CheckType (*m_context, m_v8ValueHandle, &v8::Value::IsString);
    }

bool BeJsValue::IsObject() const
    {
    return BeJsValueHelper::CheckType (*m_context, m_v8ValueHandle, &v8::Value::IsObject);
    }

bool BeJsValue::IsFunction() const
    {
    return BeJsValueHelper::CheckType (*m_context, m_v8ValueHandle, &v8::Value::IsFunction);
    }

bool BeJsValue::IsEqual (BeJsValueCR value) const
    {
    return BeJsValueHelper::CheckEqual (*m_context, m_v8ValueHandle, value.GetV8ValueHandle(), static_cast<bool (v8::Value::*)(v8::Local<v8::Value>) const>(&v8::Value::Equals));
    }

bool BeJsValue::IsEqualStrict (BeJsValueCR value) const
    {
    return BeJsValueHelper::CheckEqual (*m_context, m_v8ValueHandle, value.GetV8ValueHandle(), &v8::Value::StrictEquals);
    }

BeJsValue BeJsValue::Null (BeJsContextCR context)
    {
    v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Value> handle = v8::Null (isolate);

    return BeJsValue (context, handle);
    }

BeJsValue BeJsValue::Undefined (BeJsContextCR context)
    {
    v8::Isolate* isolate = context.GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Value> handle = v8::Undefined (isolate);

    return BeJsValue (context, handle);
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsValue (context, v8Handle)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, bool value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsPrimitive (context, v8Handle)
    {
    }

bool BeJsBoolean::GetValue() const
    {
    return BeJsValueHelper::GetValue<bool> (*m_context, m_v8ValueHandle);
    }

void BeJsBoolean::SetValue (bool value)
    {
    BeJsValueHelper::SetValue (*m_context, m_v8ValueHandle, value);
    }

BeJsNumber::BeJsNumber (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, double value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, int32_t value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsPrimitive (context, v8Handle)
    {
    }

double BeJsNumber::GetValue() const
    {
    if (IsNull())
        return 0;

    return BeJsValueHelper::GetValue<double> (*m_context, m_v8ValueHandle);
    }

int32_t BeJsNumber::GetIntegerValue() const
    {
    return BeJsValueHelper::GetValue<int32_t> (*m_context, m_v8ValueHandle);
    }

void BeJsNumber::SetValue (double value)
    {
    BeJsValueHelper::SetValue (*m_context, m_v8ValueHandle, value);
    }

void BeJsNumber::SetValue (int32_t value)
    {
    BeJsValueHelper::SetValue (*m_context, m_v8ValueHandle, value);
    }

BeJsString::BeJsString (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsString::BeJsString (BeJsContextCR context, Utf8CP value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsString::BeJsString (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsPrimitive (context, v8Handle)
    {
    }

Utf8String BeJsString::GetValue() const
    {
    if (IsNull())
        return Utf8String();

    return BeJsValueHelper::GetValue<Utf8String> (*m_context, m_v8ValueHandle);
    }

void BeJsString::SetValue (Utf8CP value)
    {
    BeJsValueHelper::SetValue (*m_context, m_v8ValueHandle, value);
    }

int64_t BeJsString::GetValueAsInt64() const
    {
    Utf8String s = GetValue();
    int64_t v;
    sscanf (s.c_str(), "%" SCNd64, &v);

    return v;
    }

uint64_t BeJsString::GetValueAsUnsignedInt64() const
    {
    Utf8String s = GetValue();
    uint64_t v;
    sscanf (s.c_str(), "%" SCNu64, &v);

    return v;
    }

BeJsNativePointer::BeJsNativePointer (BeJsValueCR other)
    : BeJsObject (other),
      m_environment (&other.GetContext().GetEnvironment())
    {
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, void* object, DisposeCallback disposeCallback, BeJsObject const* prototype, bool suppressCallContext)
    : BeJsObject (context),
      m_environment (&context.GetEnvironment())
    {
    v8::Isolate* isolate = m_context->GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
    v8::Context::Scope contextScope (contextHandle);

    v8::Local<v8::ObjectTemplate> objectTemplate = v8::ObjectTemplate::New();
    objectTemplate->SetInternalFieldCount (1);
    v8::Local<v8::Object> objectHandle = objectTemplate->NewInstance();
    m_v8ValueHandle.Reset (isolate, objectHandle);
    
    if (prototype != nullptr)
        {
        v8::Local<v8::Value> prototypeValue = v8::Local<v8::Value>::New (isolate, prototype->GetV8ValueHandle());
        objectHandle->SetPrototype (prototypeValue);
        }

    _BeJsNativePointer* external = new _BeJsNativePointer (context, object, disposeCallback);
    objectHandle->SetInternalField (0, v8::External::New (isolate, external));

    external->m_v8ValueHandle.Reset (isolate, objectHandle);
    external->m_v8ValueHandle.SetWeak (external, &_BeJsNativePointer::Dispose, v8::WeakCallbackType::kParameter);

    if (context.m_callContextActive && !suppressCallContext)
        context.m_callContextValues.push_back (*this);
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsObject (context, v8Handle),
      m_environment (&context.GetEnvironment())
    {
    }

void* BeJsNativePointer::GetValue() const
    {
    if (IsNull())
        return nullptr;

    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (*m_context, m_v8ValueHandle);
    return wrapper->m_object;
    }

void BeJsNativePointer::Dispose()
    {
    if (IsNull())
        return;

    _BeJsNativePointer* wrapper = static_cast<_BeJsNativePointer*>(_BeJsNativePointer::GetFromHandle (*m_context, m_v8ValueHandle));

    if (wrapper->m_callback != nullptr)
        wrapper->m_callback (wrapper->m_object, this);

    wrapper->m_object = nullptr;
    }

void BeJsNativePointer::SetValue (void* object)
    {
    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (*m_context, m_v8ValueHandle);
    wrapper->m_object = object;
    }

BeJsObject::BeJsObject (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsValue (context, v8Handle)
    {
    }

BeJsValue BeJsObject::GetProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsValue, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

BeJsBoolean BeJsObject::GetBooleanProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsBoolean, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

BeJsNumber BeJsObject::GetNumberProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsNumber, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

BeJsString BeJsObject::GetStringProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsString, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

BeJsObject BeJsObject::GetObjectProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsObject, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

BeJsNativePointer BeJsObject::GetNativePointerProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsNativePointer, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

BeJsArray BeJsObject::GetArrayProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsArray, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

BeJsFunction BeJsObject::GetFunctionProperty (Utf8CP name) const
    {
    return BeJsObjectHelper<BeJsFunction, Utf8CP>::GetProperty (*m_context, m_v8ValueHandle, name);
    }

void BeJsObject::SetProperty (Utf8CP name, BeJsValueCR value)
    {
    BeJsObjectHelper<v8::Persistent<v8::Value> const&, Utf8CP>::SetProperty (*m_context, m_v8ValueHandle, value.m_v8ValueHandle, name);
    }

void BeJsObject::SetBooleanProperty (Utf8CP name, bool value)
    {
    BeJsObjectHelper<bool, Utf8CP>::SetProperty (*m_context, m_v8ValueHandle, value, name);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, double value)
    {
    BeJsObjectHelper<double, Utf8CP>::SetProperty (*m_context, m_v8ValueHandle, value, name);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, int32_t value)
    {
    BeJsObjectHelper<int32_t, Utf8CP>::SetProperty (*m_context, m_v8ValueHandle, value, name);
    }

void BeJsObject::SetStringProperty (Utf8CP name, Utf8CP value)
    {
    BeJsObjectHelper<Utf8CP, Utf8CP>::SetProperty (*m_context, m_v8ValueHandle, value, name);
    }

void BeJsObject::DeleteProperty (Utf8CP name)
    {
    return BeJsObjectHelper<BeJsValue, Utf8CP>::DeleteProperty (*m_context, m_v8ValueHandle, name);
    }

bool BeJsObject::DoesPropertyExist (Utf8CP name)
    {
    return BeJsObjectHelper<BeJsValue, Utf8CP>::DoesPropertyExist (*m_context, m_v8ValueHandle, name);
    }

void BeJsObject::SetPrototype (BeJsValueCR prototype)
    {
    v8::Isolate* isolate = m_context->GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, m_context->GetV8ContextHandle());
    v8::Context::Scope contextScope (contextHandle);

    v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, m_v8ValueHandle);
    BeAssert (handle->IsObject() && "Cannot set prototype of a JavaScript value that is not an object.");
    v8::Local<v8::Object> object = v8::Local<v8::Object>::Cast (handle);

    v8::Local<v8::Value> prototypeHandle = v8::Local<v8::Value>::New (isolate, prototype.GetV8ValueHandle());
    object->SetPrototype (prototypeHandle);
    }

BeJsObject BeJsObject::New (BeJsContextCR context)
    {
    BeJsObject obj (context);
    BeJsValueHelper::SetValue<BeJsObject&> (context, obj.m_v8ValueHandle, obj, true);
    return obj;
    }

BeJsNativeObject::Callbacks::Callbacks (GetProperty getter,
                                               SetProperty setter,
                                               DeleteProperty deleter,
                                               EnumerateProperties enumerator,
                                               DoesPropertyExist query,
                                               BeJsNativePointer::DisposeCallback disposer)
    : getProperty (getter), setProperty (setter), deleteProperty (deleter), enumerateProperties (enumerator), doesPropertyExist (query), dispose (disposer)
    {
    }

BeJsNativeObject::BeJsNativeObject (BeJsContextCR context, Callbacks const& callbacks, void* object)
    : BeJsObject (context)
    {
    v8::Isolate* isolate = m_context->GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, context.GetV8ContextHandle());
    v8::Context::Scope contextScope (contextHandle);

    v8::Local<v8::ObjectTemplate> objectTemplate = v8::ObjectTemplate::New();
    objectTemplate->SetInternalFieldCount (1);

    _BeJsNativeObject* proxy = new _BeJsNativeObject (context, callbacks, object);

    objectTemplate->SetNamedPropertyHandler (&_BeJsNativeObject::GetProperty,
                                             callbacks.setProperty == nullptr         ? 0 : &_BeJsNativeObject::SetProperty,
                                             callbacks.doesPropertyExist == nullptr   ? 0 : &_BeJsNativeObject::DoesPropertyExist,
                                             callbacks.deleteProperty == nullptr      ? 0 : &_BeJsNativeObject::DeleteProperty,
                                             callbacks.enumerateProperties == nullptr ? 0 : &_BeJsNativeObject::EnumerateProperties,
                                             v8::External::New (isolate, proxy));

    v8::Local<v8::Object> objectInstance = objectTemplate->NewInstance();
    objectInstance->SetInternalField (0, v8::External::New (isolate, proxy));
    m_v8ValueHandle.Reset (isolate, objectInstance);

    proxy->m_v8ValueHandle.Reset (isolate, objectInstance);
    proxy->m_v8ValueHandle.SetWeak (proxy, &_BeJsNativeObject::Dispose, v8::WeakCallbackType::kParameter);
    }

BeJsArray::BeJsArray (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsObject (context, v8Handle)
    {
    }

BeJsValue BeJsArray::GetProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsValue, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

BeJsBoolean BeJsArray::GetBooleanProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsBoolean, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

BeJsNumber BeJsArray::GetNumberProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsNumber, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

BeJsString BeJsArray::GetStringProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsString, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

BeJsObject BeJsArray::GetObjectProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsObject, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

BeJsNativePointer BeJsArray::GetNativePointerProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsNativePointer, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

BeJsArray BeJsArray::GetArrayProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsArray, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

BeJsFunction BeJsArray::GetFunctionProperty (uint32_t index) const
    {
    return BeJsObjectHelper<BeJsFunction, uint32_t>::GetProperty (*m_context, m_v8ValueHandle, index);
    }

void BeJsArray::SetProperty (uint32_t index, BeJsValueCR value)
    {
    BeJsObjectHelper<v8::Persistent<v8::Value> const&, uint32_t>::SetProperty (*m_context, m_v8ValueHandle, value.m_v8ValueHandle, index);
    }

void BeJsArray::SetBooleanProperty (uint32_t index, bool value)
    {
    BeJsObjectHelper<bool, uint32_t>::SetProperty (*m_context, m_v8ValueHandle, value, index);
    }

void BeJsArray::SetNumberProperty (uint32_t index, double value)
    {
    BeJsObjectHelper<double, uint32_t>::SetProperty (*m_context, m_v8ValueHandle, value, index);
    }

void BeJsArray::SetNumberProperty (uint32_t index, int32_t value)
    {
    BeJsObjectHelper<int32_t, uint32_t>::SetProperty (*m_context, m_v8ValueHandle, value, index);
    }

void BeJsArray::SetStringProperty (uint32_t index, Utf8CP value)
    {
    BeJsObjectHelper<Utf8CP, uint32_t>::SetProperty (*m_context, m_v8ValueHandle, value, index);
    }

void BeJsArray::DeleteProperty (uint32_t index)
    {
    return BeJsObjectHelper<BeJsValue, uint32_t>::DeleteProperty (*m_context, m_v8ValueHandle, index);
    }

bool BeJsArray::DoesPropertyExist (uint32_t index)
    {
    return BeJsObjectHelper<BeJsValue, uint32_t>::DoesPropertyExist (*m_context, m_v8ValueHandle, index);
    }

uint32_t BeJsArray::GetLength() const
    {
    BeAssert (!m_v8ValueHandle.IsEmpty() && "Cannot access an empty JavaScript value.");

    v8::Isolate* isolate = m_context->GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, m_context->GetV8ContextHandle());
    v8::Context::Scope contextScope (contextHandle);

    v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, m_v8ValueHandle);
    BeAssert (handle->IsArray() && "Cannot get the length of a JavaScript value that is not an array.");
    v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast (handle);
    return arr->Length();
    }

BeJsArray BeJsArray::New (BeJsContextCR context)
    {
    BeJsArray arr (context);
    BeJsValueHelper::SetValue<BeJsArray&> (context, arr.m_v8ValueHandle, arr, true);
    return arr;
    }

BeJsFunction::BeJsFunction (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsObject (context, v8Handle)
    {
    }

BeJsValue BeJsFunction::Call (BeJsValue** args,
                                              BeJsEnvironment::EngineValueHandleType* engineArgs,
                                              size_t argumentCount,
                                              BeJsObject* callScope,
                                              bool callAsConstructor) const
    {
    BeAssert (!m_v8ValueHandle.IsEmpty() && "Cannot access an empty JavaScript value.");

    v8::Isolate* isolate = m_context->GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope (isolate);

    v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New (isolate, m_context->GetV8ContextHandle());
    v8::Context::Scope contextScope (contextHandle);

    v8::Local<v8::Value> handle = v8::Local<v8::Value>::New (isolate, m_v8ValueHandle);
    BeAssert (handle->IsFunction() && "Cannot call a JavaScript value that is not a function.");
    v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast (handle);

    if (argumentCount != 0)
        {
        for (size_t i = 0; i != argumentCount; ++i)
            engineArgs [i] = v8::Local<v8::Value>::New (isolate, args [i]->GetV8ValueHandle());
        }

    v8::Local<v8::Value> scope;
    if (callScope == nullptr)
        scope = v8::Null(isolate);
    else
        scope = v8::Local<v8::Value>::New (isolate, callScope->GetV8ValueHandle());

    v8::Local<v8::Value> value;
    if (callAsConstructor)
        value = function->NewInstance (static_cast<int>(argumentCount), engineArgs);
    else
        value = function->Call (scope, static_cast<int>(argumentCount), engineArgs);

    return BeJsValue (*m_context, value);
    }

template <typename T> void BeJsNativeFunction::Init(BeJsContextCR context, T callback)
    {
    v8::Isolate* isolate = m_context->GetEnvironment().GetV8Isolate();
    v8::HandleScope handleScope(isolate);

    v8::Local<v8::Context> contextHandle = v8::Local<v8::Context>::New(isolate, m_context->GetV8ContextHandle());
    v8::Context::Scope contextScope(contextHandle);

    _BeJsNativeFunction<T>* proxy = new _BeJsNativeFunction<T>(context, callback);
    m_removeCallbackFunction = [proxy]() -> void {if (proxy) { _BeJsNativeFunction<T>::RemoveNativeCallback(proxy); }};

    v8::Local<v8::FunctionTemplate> functionTemplate = v8::FunctionTemplate::New(isolate, &V8Callback<T>, v8::External::New(isolate, proxy));
    v8::Local<v8::Function> function = functionTemplate->GetFunction();

    m_v8ValueHandle.Reset(isolate, function);

    proxy->m_v8ValueHandle.Reset(isolate, function);
    proxy->m_v8ValueHandle.SetWeak(proxy, &_BeJsNativeFunction<T>::Dispose, v8::WeakCallbackType::kParameter);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, Callback callback)
    : BeJsFunction (context)
    {
    Init(context, callback);
    }

BeJsNativeFunction::BeJsNativeFunction(BeJsContextCR context, std::function<BeJsValue(CallbackInfoR info)> callback)
    : BeJsFunction(context)
    {
    Init(context, callback);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, v8::Handle<v8::Value> const& v8Handle)
    : BeJsFunction (context, v8Handle)
    {
    }

template <typename T> void BeJsNativeFunction::V8Callback (v8::FunctionCallbackInfo<v8::Value> const& args)
    {
    void* object = v8::Local<v8::External>::Cast (args.Data())->Value();
    _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
    CallbackInfo info (*proxy->m_context, args);
    proxy->Callback (info);
    }

BeJsNativeFunction::CallbackInfo::CallbackInfo (BeJsContextCR context, v8::FunctionCallbackInfo<v8::Value> const& v8CallbackInfo)
    : m_context (&context), m_v8CallbackInfo (v8CallbackInfo)
    {
    }

BeJsNativeFunction BeJsNativeFunction::CallbackInfo::GetFunction()
    {
    return BeJsValueHelper::GetCallbackValue<BeJsNativeFunction> (*m_context, m_v8CallbackInfo, &v8::FunctionCallbackInfo<v8::Value>::Callee);
    }

BeJsContextCR BeJsNativeFunction::CallbackInfo::GetContext()
    {
    return *m_context;
    }

bool BeJsNativeFunction::CallbackInfo::IsConstructCall()
    {
    return m_v8CallbackInfo.IsConstructCall();
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetCallContext()
    {
    return BeJsValueHelper::GetCallbackValue<BeJsObject> (*m_context, m_v8CallbackInfo, &v8::FunctionCallbackInfo<v8::Value>::This);
    }

uint32_t BeJsNativeFunction::CallbackInfo::GetArgumentCount() const
    {
    return m_v8CallbackInfo.Length();
    }

BeJsValue BeJsNativeFunction::CallbackInfo::GetArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsValue> (*m_context, m_v8CallbackInfo, index);
    }

BeJsBoolean BeJsNativeFunction::CallbackInfo::GetBooleanArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsBoolean> (*m_context, m_v8CallbackInfo, index);
    }

BeJsNumber BeJsNativeFunction::CallbackInfo::GetNumberArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsNumber> (*m_context, m_v8CallbackInfo, index);
    }

BeJsString BeJsNativeFunction::CallbackInfo::GetStringArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsString> (*m_context, m_v8CallbackInfo, index);
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetObjectArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsObject> (*m_context, m_v8CallbackInfo, index);
    }

BeJsNativePointer BeJsNativeFunction::CallbackInfo::GetNativePointerArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsNativePointer> (*m_context, m_v8CallbackInfo, index);
    }

BeJsArray BeJsNativeFunction::CallbackInfo::GetArrayArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsArray> (*m_context, m_v8CallbackInfo, index);
    }

BeJsFunction BeJsNativeFunction::CallbackInfo::GetFunctionArgument (uint32_t index) const
    {
    return BeJsValueHelper::GetArgument<BeJsFunction> (*m_context, m_v8CallbackInfo, index);
    }

#endif

#if defined (BEJAVASCRIPT_JAVASCRIPTCORE_ENGINE)

namespace {

struct _BeJsNativeObjectWrapper
    {
public:
    void* m_object;
    _BeJsNativeObjectWrapper (void* object = nullptr) : m_object (object) {};
    };

struct _BeJsNativePointer : public _BeJsNativeObjectWrapper
    {
public:
    BeJsNativePointer::DisposeCallback m_callback;

    _BeJsNativePointer (BeJsContextCR context, void* object, BeJsNativePointer::DisposeCallback callback)
        : _BeJsNativeObjectWrapper (object), m_callback (callback)
        {
        };

    ~_BeJsNativePointer()
        {
        };

    static void Dispose (JSObjectRef object)
        {
        _BeJsNativePointer* external = static_cast<_BeJsNativePointer*>(JSObjectGetPrivate (object));

        if (external->m_callback != nullptr)
            external->m_callback (external->m_object, nullptr);

        delete external;
        };

    static _BeJsNativeObjectWrapper* GetFromHandle (BeJsContextCR context, JSValueRef jscValue)
        {
        JSObjectRef object = JSValueToObject (context.GetJscContext(), jscValue, NULL);
        void* data = JSObjectGetPrivate (object);
        
        return static_cast<_BeJsNativeObjectWrapper*>(data);
        };
    };
    
template <class T> struct _BeJsNativeFunction : _BeJsNativeObjectWrapper
    {
private:
    void RemoveCallback()
        {
        m_callback = nullptr;
        }

public:
    BeJsContext const* m_context;
    T m_callback;

    _BeJsNativeFunction (BeJsContextCR context, T callback)
        : m_context (&context), m_callback (callback)
        {
        };

    ~_BeJsNativeFunction()
        {
        m_callback = nullptr;
        };

    JSValueRef Callback (BeJsNativeFunction::CallbackInfoR info)
        {
        if (m_callback != nullptr)
            return m_callback (info).GetJscValue();
        
        return NULL;
        };

    static void Dispose (JSObjectRef object)
        {
        _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(JSObjectGetPrivate (object));
        delete proxy;
        };

    static void RemoveNativeCallback(_BeJsNativeFunction<T>* proxy)
        {
        proxy->RemoveCallback();
        };
    };
    
struct _BeJsNativeObject : public _BeJsNativeObjectWrapper
    {
public:
    BeJsNativeObject::Callbacks m_callbacks;
    BeJsContext const* m_context;

    _BeJsNativeObject (BeJsContextCR context, BeJsNativeObject::Callbacks callbacks, void* object)
        : _BeJsNativeObjectWrapper (object), m_callbacks (callbacks), m_context (&context)
        {
        }

    ~_BeJsNativeObject()
        {
        };

    static void Dispose (JSObjectRef object)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(JSObjectGetPrivate (object));

        if (proxy->m_callbacks.dispose != nullptr)
            proxy->m_callbacks.dispose (proxy->m_object, nullptr);
        
        delete proxy;
        };
        
    static bool DoesPropertyExist (JSContextRef ctx, JSObjectRef object, JSStringRef propertyName)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(JSObjectGetPrivate (object));

        if (proxy->m_callbacks.doesPropertyExist != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, object);
            Utf8String propertyNameS = BeJsString::GetUtf8String (propertyName);
            return proxy->m_callbacks.doesPropertyExist (external, propertyNameS.c_str());
            }
            
        return false;
        };

    static JSValueRef GetProperty (JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef *exception)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(JSObjectGetPrivate (object));

        if (proxy->m_callbacks.getProperty != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, object);
            Utf8String propertyNameS = BeJsString::GetUtf8String (propertyName);
            BeJsValue response = proxy->m_callbacks.getProperty (external, propertyNameS.c_str());
            return response.GetJscValue();
            }
        
        return nullptr;
        };

    static bool SetProperty (JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef *exception)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(JSObjectGetPrivate (object));

        if (proxy->m_callbacks.setProperty != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, object);
            BeJsValue propertyValueObj (*proxy->m_context, value);
            Utf8String propertyNameS = BeJsString::GetUtf8String (propertyName);
            proxy->m_callbacks.setProperty (external, propertyNameS.c_str(), propertyValueObj);
            return true;
            }
            
        return false;
        };

    static bool DeleteProperty (JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef *exception)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(JSObjectGetPrivate (object));

        if (proxy->m_callbacks.deleteProperty != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, object);
            Utf8String propertyNameS = BeJsString::GetUtf8String (propertyName);
            proxy->m_callbacks.deleteProperty (external, propertyNameS.c_str());
            return true;
            }
            
        return false;
        };

    static void EnumerateProperties (JSContextRef ctx, JSObjectRef object, JSPropertyNameAccumulatorRef propertyNames)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(JSObjectGetPrivate (object));

        if (proxy->m_callbacks.enumerateProperties != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, object);

            BeJsArray response = proxy->m_callbacks.enumerateProperties (external);
            if (response.GetJscValue() != nullptr)
                {
                auto c = response.GetLength();

                for (size_t i = 0; i != c; ++i)
                    {
                    BeJsValue property = response.GetProperty (i);
                    JSStringRef propertyName = JSValueToStringCopy (ctx, property.GetJscValue(), NULL);
                    JSPropertyNameAccumulatorAddName (propertyNames, propertyName);
                    JSStringRelease (propertyName);
                    }
                }
            }
        };
    };
    
}

BeJsEngine BeJsEnvironment::GetActiveEngine()
    {
    return BeJsEngine::JavaScriptCore;
    }

BeJsEnvironment::BeJsEnvironment()
    : m_destroying (false),
      m_ownsSystemEnvironment (true)
    {
    m_jscContextGroup = JSContextGroupCreate();
    }

BeJsEnvironment::BeJsEnvironment (BeJsEngine engine, void* systemEnvironment)
    : m_destroying (false),
      m_ownsSystemEnvironment (false)
    {
    BeAssert (engine == GetActiveEngine());

    m_ownsSystemEnvironment = false;

    m_jscContextGroup = reinterpret_cast<JSContextGroupRef>(systemEnvironment);
    JSContextGroupRetain (m_jscContextGroup);
    }

void* BeJsEnvironment::GetSystemObject()
    {
    return (void*)m_jscContextGroup;
    }

BeJsEnvironment::~BeJsEnvironment()
    {
    m_destroying = true;
    JSContextGroupRelease (m_jscContextGroup);
    }

BeJsContext::BeJsContext (BeJsEnvironmentCR environment, Utf8CP identifier, Utf8CP initializationScript)
    : m_environment (&environment)
    {
    m_jscContext = JSGlobalContextCreateInGroup (environment.GetJscContextGroup(), NULL);
    
    FindLanguageObjects();
    Initialize (initializationScript, identifier);
    }

BeJsContext::BeJsContext (BeJsEngine engine, BeJsEnvironmentCR environment, void* systemContext)
    : m_environment (&environment)
    {
    BeAssert (engine == BeJsEnvironment::GetActiveEngine());

    m_jscContext = reinterpret_cast<JSGlobalContextRef>(systemContext);
    JSGlobalContextRetain (m_jscContext);

    FindLanguageObjects();
    Initialize (nullptr, nullptr);
    }

void BeJsContext::FindLanguageObjects()
    {
    auto languageFunction = JSValueToObject (m_jscContext, GetGlobalObject().GetProperty ("Function").GetJscValue(), NULL);
    auto functionPrototype = JSObjectGetPrototype (m_jscContext, languageFunction);
    m_Function_prototype = new BeJsValue (*this, functionPrototype);
    }

void* BeJsContext::GetSystemObject()
    {
    return m_jscContext;
    }

BeJsContext::~BeJsContext()
    {
    _OnDestroy();

    JSGlobalContextRelease (m_jscContext);
    }

BeJsValue BeJsContext::EvaluateScript (Utf8CP script, Utf8CP identifier, EvaluateStatus* status, EvaluateException* exception) const
    {
    JSStringRef scriptObject = JSStringCreateWithUTF8CString (script);
    JSStringRef identifierObject = JSStringCreateWithUTF8CString (identifier);
    
    if (status != nullptr)
        {
        bool correct = JSCheckScriptSyntax (m_jscContext, scriptObject, identifierObject, 0, NULL);
        if (!correct)
            {
            *status = EvaluateStatus::ParseError;
            JSStringRelease (scriptObject);
            JSStringRelease (identifierObject);
            
            return BeJsValue::InvalidHandle();
            }
        }
    
    JSValueRef jscException;
    JSValueRef scriptValue = JSEvaluateScript (m_jscContext, scriptObject, NULL, identifierObject, 0, &jscException);
    
    if (status == nullptr)
        {
        BeAssert (scriptValue != NULL);
        }
    else
        {
        if (scriptValue != NULL)
            {
            *status = EvaluateStatus::Success;
            }
        else
            {
            *status = EvaluateStatus::RuntimeError;
            
            if (exception != nullptr)
                {
                BeJsObject exceptionObject (*this, jscException);
                BeJsString trace = exceptionObject.GetStringProperty ("stack");

                (*exception).message = BeJsString (exceptionObject.CallMemberFunction ("toString")).GetValue();

                if (trace.IsString())
                    (*exception).trace = trace.GetValue();
                }
                
            JSStringRelease (scriptObject);
            JSStringRelease (identifierObject);
            
            return BeJsValue::InvalidHandle();
            }
        }
    
    JSStringRelease (scriptObject);
    JSStringRelease (identifierObject);

    return BeJsValue (*this, scriptValue);
    }

BeJsObject BeJsContext::GetGlobalObject() const
    {
    JSObjectRef object = JSContextGetGlobalObject (m_jscContext);
    return BeJsObject (*this, object);
    }

BeJsValue::BeJsValue (BeJsContextCR context)
    : m_context (&context), m_jscValue (NULL)
    {
    }

BeJsValue::BeJsValue (BeJsValueCR other)
    : m_context (other.m_context), m_jscValue (other.m_jscValue)
    {
    JSValueProtect (m_context->GetJscContext(), m_jscValue);
    }

BeJsValueR BeJsValue::operator= (BeJsValueCR other)
    {
    if (this == &other)
        return *this;
        
    if (m_context != nullptr && m_jscValue != NULL)
        JSValueUnprotect (m_context->GetJscContext(), m_jscValue);

    m_context = other.m_context;
    m_jscValue = other.m_jscValue;
    
    JSValueProtect (m_context->GetJscContext(), m_jscValue);

    return *this;
    }

BeJsValue::BeJsValue (BeJsValue&& other)
    : m_context (other.m_context), m_jscValue (other.m_jscValue)
    {
    other.m_jscValue = NULL;
    }

BeJsValue::BeJsValue (BeJsContextCR context, JSValueRef jscValue)
    : m_context (&context), m_jscValue (jscValue)
    {
    JSValueProtect (m_context->GetJscContext(), m_jscValue);
    }
    
BeJsValue::~BeJsValue()
    {
    if (m_context == nullptr)
        return;

    if (m_jscValue != NULL)
        JSValueUnprotect (m_context->GetJscContext(), m_jscValue);
    }

void BeJsValue::Reset()
    {
    if (m_jscValue != NULL)
        {
        JSValueUnprotect (m_context->GetJscContext(), m_jscValue);
        m_jscValue = NULL;
        }
    }

bool BeJsValue::IsEmpty() const
    {
    return m_jscValue == NULL;
    }

void BeJsValue::NotifyEngineValueReleased()
    {
    m_jscValue = NULL;
    }

void BeJsValue::SetJscValue (JSValueRef value)
    {
    JSContextRef context = m_context->GetJscContext();
    
    if (m_jscValue != NULL)
        JSValueUnprotect (context, m_jscValue);
        
    m_jscValue = value;
    JSValueProtect (context, m_jscValue);
    }

bool BeJsValue::IsNull() const
    {
    return JSValueIsNull (m_context->GetJscContext(), m_jscValue);
    }

bool BeJsValue::IsUndefined() const
    {
    return JSValueIsUndefined (m_context->GetJscContext(), m_jscValue);
    }

bool BeJsValue::IsNumber() const
    {
    return JSValueIsNumber (m_context->GetJscContext(), m_jscValue);
    }

bool BeJsValue::IsBoolean() const
    {
    return JSValueIsBoolean (m_context->GetJscContext(), m_jscValue);
    }

bool BeJsValue::IsString() const
    {
    return JSValueIsString (m_context->GetJscContext(), m_jscValue);
    }

bool BeJsValue::IsObject() const
    {
    return JSValueIsObject (m_context->GetJscContext(), m_jscValue);
    }

bool BeJsValue::IsFunction() const
    {
    JSContextRef context = m_context->GetJscContext();
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    return JSObjectIsFunction (context, object);
    }

bool BeJsValue::IsEqual (BeJsValueCR value) const
    {
    return JSValueIsEqual (m_context->GetJscContext(), m_jscValue, value.GetJscValue(), NULL);
    }

bool BeJsValue::IsEqualStrict (BeJsValueCR value) const
    {
    return JSValueIsStrictEqual (m_context->GetJscContext(), m_jscValue, value.GetJscValue());
    }

BeJsValue BeJsValue::Null (BeJsContextCR context)
    {
    JSValueRef nullRef = JSValueMakeNull (context.GetJscContext());
    return BeJsValue (context, nullRef);
    }

BeJsValue BeJsValue::Undefined (BeJsContextCR context)
    {
    JSValueRef undefinedRef = JSValueMakeUndefined (context.GetJscContext());
    return BeJsValue (context, undefinedRef);
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context, JSValueRef jscValue)
    : BeJsValue (context, jscValue)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, bool value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, JSValueRef jscValue)
    : BeJsPrimitive (context, jscValue)
    {
    }

bool BeJsBoolean::GetValue() const
    {
    BeAssert (IsBoolean());
    return JSValueToBoolean (m_context->GetJscContext(), m_jscValue);
    }

void BeJsBoolean::SetValue (bool value)
    {
    JSValueRef jsValue = JSValueMakeBoolean (m_context->GetJscContext(), value);
    SetJscValue (jsValue);
    }

BeJsNumber::BeJsNumber (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, double value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, int32_t value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, JSValueRef jscValue)
    : BeJsPrimitive (context, jscValue)
    {
    }

double BeJsNumber::GetValue() const
    {
    if (IsNull())
        return 0;

    BeAssert (IsNumber());
    return JSValueToNumber (m_context->GetJscContext(), m_jscValue, NULL);
    }

int32_t BeJsNumber::GetIntegerValue() const
    {
    BeAssert (IsNumber());
    return JSValueToNumber (m_context->GetJscContext(), m_jscValue, NULL);
    }

void BeJsNumber::SetValue (double value)
    {
    JSValueRef jsValue = JSValueMakeNumber (m_context->GetJscContext(), value);
    SetJscValue (jsValue);
    }

void BeJsNumber::SetValue (int32_t value)
    {
    JSValueRef jsValue = JSValueMakeNumber (m_context->GetJscContext(), value);
    SetJscValue (jsValue);
    }

BeJsString::BeJsString (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsString::BeJsString (BeJsContextCR context, Utf8CP value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsString::BeJsString (BeJsContextCR context, JSValueRef jscValue)
    : BeJsPrimitive (context, jscValue)
    {
    }

Utf8String BeJsString::GetUtf8String (JSStringRef jsString)
    {
    size_t jsStringLength = JSStringGetMaximumUTF8CStringSize (jsString);
    char* jsStringChars = new char [jsStringLength];
    JSStringGetUTF8CString (jsString, jsStringChars, jsStringLength);
    Utf8String string (jsStringChars);
    delete[] jsStringChars;
    
    return string;
    }

Utf8String BeJsString::GetValue() const
    {
    if (IsNull())
        return Utf8String();

    BeAssert (IsString());
    JSStringRef jsString = JSValueToStringCopy (m_context->GetJscContext(), m_jscValue, NULL);
    Utf8String string = GetUtf8String (jsString);
    JSStringRelease (jsString);
    return string;
    }

void BeJsString::SetValue (Utf8CP value)
    {
    JSStringRef string = JSStringCreateWithUTF8CString (value);
    JSValueRef jsValue = JSValueMakeString (m_context->GetJscContext(), string);
    SetJscValue (jsValue);
    JSStringRelease (string);
    }

int64_t BeJsString::GetValueAsInt64() const
    {
    Utf8String s = GetValue();
    int64_t v;
    sscanf (s.c_str(), "%lld", &v);

    return v;
    }

uint64_t BeJsString::GetValueAsUnsignedInt64() const
    {
    Utf8String s = GetValue();
    uint64_t v;
    sscanf (s.c_str(), "%llu", &v);

    return v;
    }

BeJsObject::BeJsObject (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context, JSValueRef jscValue)
    : BeJsValue (context, jscValue)
    {
    }

BeJsValue BeJsObject::GetProperty (Utf8CP name) const
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef value = JSObjectGetProperty (context, object, nameString, NULL);
    JSStringRelease (nameString);

    return BeJsValue (*m_context, value);
    }

BeJsBoolean BeJsObject::GetBooleanProperty (Utf8CP name) const
    {
    return BeJsBoolean (GetProperty (name));
    }

BeJsNumber BeJsObject::GetNumberProperty (Utf8CP name) const
    {
    return BeJsNumber (GetProperty (name));
    }

BeJsString BeJsObject::GetStringProperty (Utf8CP name) const
    {
    return BeJsString (GetProperty (name));
    }

BeJsObject BeJsObject::GetObjectProperty (Utf8CP name) const
    {
    return BeJsObject (GetProperty (name));
    }

BeJsNativePointer BeJsObject::GetNativePointerProperty (Utf8CP name) const
    {
    return BeJsNativePointer (GetProperty (name));
    }

BeJsArray BeJsObject::GetArrayProperty (Utf8CP name) const
    {
    return BeJsArray (GetProperty (name));
    }

BeJsFunction BeJsObject::GetFunctionProperty (Utf8CP name) const
    {
    return BeJsFunction (GetProperty (name));
    }

void BeJsObject::SetProperty (Utf8CP name, BeJsValueCR value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSObjectSetProperty (context, object, nameString, value.m_jscValue, kJSPropertyAttributeNone, NULL);
    JSStringRelease (nameString);
    }

void BeJsObject::SetBooleanProperty (Utf8CP name, bool value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeBoolean (context, value);
    JSObjectSetProperty (context, object, nameString, jsValue, kJSPropertyAttributeNone, NULL);
    JSStringRelease (nameString);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, double value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeNumber (context, value);
    JSObjectSetProperty (context, object, nameString, jsValue, kJSPropertyAttributeNone, NULL);
    JSStringRelease (nameString);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, int32_t value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeNumber (context, value);
    JSObjectSetProperty (context, object, nameString, jsValue, kJSPropertyAttributeNone, NULL);
    JSStringRelease (nameString);
    }

void BeJsObject::SetStringProperty (Utf8CP name, Utf8CP value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSStringRef valueString = JSStringCreateWithUTF8CString (value);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeString (context, valueString);
    JSObjectSetProperty (context, object, nameString, jsValue, kJSPropertyAttributeNone, NULL);
    JSStringRelease (nameString);
    JSStringRelease (valueString);
    }

void BeJsObject::DeleteProperty (Utf8CP name)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSObjectDeleteProperty (context, object, nameString, NULL);
    JSStringRelease (nameString);
    }

bool BeJsObject::DoesPropertyExist (Utf8CP name)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString (name);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    bool hasProperty = JSObjectHasProperty (context, object, nameString);
    JSStringRelease (nameString);

    return hasProperty;
    }

void BeJsObject::SetPrototype (BeJsValueCR prototype)
    {
    BeAssert (IsObject());
    //JSObjectSetPrototype (m_context->GetJscContext(), m_jscValue, prototype.GetJscValue());
    }

BeJsObject BeJsObject::New (BeJsContextCR context)
    {
    JSObjectRef object = JSObjectMake (context.GetJscContext(), NULL, NULL);
    return BeJsObject (context, object);
    }

BeJsArray::BeJsArray (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context, JSValueRef jscValue)
    : BeJsObject (context, jscValue)
    {
    }

BeJsValue BeJsArray::GetProperty (uint32_t index) const
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef value = JSObjectGetPropertyAtIndex (context, object, index, NULL);

    return BeJsValue (*m_context, value);
    }

BeJsBoolean BeJsArray::GetBooleanProperty (uint32_t index) const
    {
    return BeJsBoolean (GetProperty (index));
    }

BeJsNumber BeJsArray::GetNumberProperty (uint32_t index) const
    {
    return BeJsNumber (GetProperty (index));
    }

BeJsString BeJsArray::GetStringProperty (uint32_t index) const
    {
    return BeJsString (GetProperty (index));
    }

BeJsObject BeJsArray::GetObjectProperty (uint32_t index) const
    {
    return BeJsObject (GetProperty (index));
    }

BeJsNativePointer BeJsArray::GetNativePointerProperty (uint32_t index) const
    {
    return BeJsNativePointer (GetProperty (index));
    }

BeJsArray BeJsArray::GetArrayProperty (uint32_t index) const
    {
    return BeJsArray (GetProperty (index));
    }

BeJsFunction BeJsArray::GetFunctionProperty (uint32_t index) const
    {
    return BeJsFunction (GetProperty (index));
    }

void BeJsArray::SetProperty (uint32_t index, BeJsValueCR value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSObjectSetPropertyAtIndex (context, object, index, value.m_jscValue, NULL);
    }

void BeJsArray::SetBooleanProperty (uint32_t index, bool value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeBoolean (context, value);
    JSObjectSetPropertyAtIndex (context, object, index, jsValue, NULL);
    }

void BeJsArray::SetNumberProperty (uint32_t index, double value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeNumber (context, value);
    JSObjectSetPropertyAtIndex (context, object, index, jsValue, NULL);
    }

void BeJsArray::SetNumberProperty (uint32_t index, int32_t value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeNumber (context, value);
    JSObjectSetPropertyAtIndex (context, object, index, jsValue, NULL);
    }

void BeJsArray::SetStringProperty (uint32_t index, Utf8CP value)
    {
    BeAssert (IsObject());
    JSContextRef context = m_context->GetJscContext();
    JSStringRef valueString = JSStringCreateWithUTF8CString (value);
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef jsValue = JSValueMakeString (context, valueString);
    JSObjectSetPropertyAtIndex (context, object, index, jsValue, NULL);
    JSStringRelease (valueString);
    }

void BeJsArray::DeleteProperty (uint32_t index)
    {
    BeAssert (IsObject());
    
    JSContextRef context = m_context->GetJscContext();
    
    Utf8String indexStr;
    indexStr.Sprintf ("%u", index);
    JSStringRef nameString = JSStringCreateWithUTF8CString (indexStr.c_str());
    
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSObjectDeleteProperty (context, object, nameString, NULL);
    
    JSStringRelease (nameString);
    }

bool BeJsArray::DoesPropertyExist (uint32_t index)
    {
    BeAssert (IsObject());
    
    JSContextRef context = m_context->GetJscContext();
    
    Utf8String indexStr;
    indexStr.Sprintf ("%u", index);
    JSStringRef nameString = JSStringCreateWithUTF8CString (indexStr.c_str());
    
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    bool hasProperty = JSObjectHasProperty (context, object, nameString);
    
    JSStringRelease (nameString);

    return hasProperty;
    }

uint32_t BeJsArray::GetLength() const
    {
    BeAssert (IsObject());
    
    JSContextRef context = m_context->GetJscContext();
    JSStringRef nameString = JSStringCreateWithUTF8CString ("length");
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    JSValueRef value = JSObjectGetProperty (context, object, nameString, NULL);
    JSStringRelease (nameString);
    return JSValueToNumber (context, value, NULL);
    }

BeJsArray BeJsArray::New (BeJsContextCR context)
    {
    JSValueRef object = context.EvaluateScript ("[]").GetJscValue();
    return BeJsArray (context, object);
    }

BeJsNativePointer::BeJsNativePointer (BeJsValueCR other)
    : BeJsObject (other),
      m_environment (&other.GetContext().GetEnvironment())
    {
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, void* object, DisposeCallback disposeCallback, BeJsObject const* prototype, bool suppressCallContext)
    : BeJsObject (context),
      m_environment (&context.GetEnvironment())
    {
    _BeJsNativePointer* external = new _BeJsNativePointer (context, object, disposeCallback);
    
    JSClassDefinition definition = kJSClassDefinitionEmpty;
    definition.finalize = &_BeJsNativePointer::Dispose;
    
    JSContextRef contextRef = m_context->GetJscContext();
    JSClassRef clazz = JSClassCreate (&definition);
    JSObjectRef jscObject = JSObjectMake (contextRef, clazz, external);
    m_jscValue = jscObject;
    JSValueProtect (contextRef, m_jscValue);
    JSClassRelease (clazz);

    if (prototype != nullptr)
        JSObjectSetPrototype (contextRef, jscObject, prototype->GetJscValue());

    if (context.m_callContextActive && !suppressCallContext)
        context.m_callContextValues.push_back (*this);
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, JSValueRef jscValue)
    : BeJsObject (context, jscValue),
      m_environment (&context.GetEnvironment())
    {
    }

void* BeJsNativePointer::GetValue() const
    {
    if (IsNull())
        return nullptr;

    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (*m_context, m_jscValue);
    return wrapper->m_object;
    }

void BeJsNativePointer::Dispose()
    {
    if (IsNull())
        return;

   _BeJsNativePointer* wrapper = static_cast<_BeJsNativePointer*>(_BeJsNativePointer::GetFromHandle (*m_context, m_jscValue));

    if (wrapper->m_callback != nullptr)
        wrapper->m_callback (wrapper->m_object, this);

    wrapper->m_object = nullptr;
    }

void BeJsNativePointer::SetValue (void* object)
    {
    if (m_environment != nullptr && m_environment->IsDestroying())
        {
        NotifyEngineValueReleased();
        return;
        }
        
    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (*m_context, m_jscValue);
    wrapper->m_object = object;
    }

BeJsFunction::BeJsFunction (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context, JSValueRef jscValue)
    : BeJsObject (context, jscValue)
    {
    }

BeJsValue BeJsFunction::Call (BeJsValue** args,
                                              BeJsEnvironment::EngineValueHandleType* engineArgs,
                                              size_t argumentCount,
                                              BeJsObject* callScope,
                                              bool callAsConstructor) const
    {
    if (argumentCount != 0)
        {
        for (size_t i = 0; i != argumentCount; ++i)
            engineArgs [i] = args [i]->GetJscValue();
        }
        
    JSContextRef context = m_context->GetJscContext();
    BeAssert (IsObject());
    JSObjectRef object = JSValueToObject (context, m_jscValue, NULL);
    BeAssert (JSObjectIsFunction (context, object) && "Cannot call a JavaScript value that is not a function.");

    JSObjectRef scope = NULL;
    if (callScope != nullptr)
        scope = JSValueToObject (context, callScope->GetJscValue(), NULL);

    JSValueRef value;
    if (callAsConstructor)
        value = JSObjectCallAsConstructor (context, object, argumentCount, engineArgs, NULL);
    else
        value = JSObjectCallAsFunction (context, object, scope, argumentCount, engineArgs, NULL);

    return BeJsValue (*m_context, value);
    }


template <typename T> void BeJsNativeFunction::Init(BeJsContextCR context, T callback)
    {
    _BeJsNativeFunction<T>* proxy = new _BeJsNativeFunction<T>(context, callback);
    m_removeCallbackFunction = [proxy]() -> void {if (proxy) { _BeJsNativeFunction<T>::RemoveNativeCallback(proxy); }};

    JSClassDefinition definition = kJSClassDefinitionEmpty;
    definition.callAsFunction = &JscCallback<T>;
    definition.callAsConstructor = &JscCallbackConstructor<T>;
    definition.finalize = &_BeJsNativeFunction<T>::Dispose;
    definition.attributes = kJSClassAttributeNoAutomaticPrototype;
    JSClassRef clazz = JSClassCreate(&definition);
    JSObjectRef object = JSObjectMake(m_context->GetJscContext(), clazz, proxy);
    JSObjectSetPrototype(m_context->GetJscContext(), object, m_context->m_Function_prototype->GetJscValue());
    m_jscValue = object;
    JSValueProtect(m_context->GetJscContext(), m_jscValue);
    JSClassRelease(clazz);

    }

BeJsNativeFunction::BeJsNativeFunction(BeJsContextCR context, std::function<BeJsValue(CallbackInfoR info)> callback)
    : BeJsFunction (context)
    {
    Init(context, callback);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, Callback callback)
    : BeJsFunction (context)
    {
    Init(context, callback);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, JSValueRef jscValue)
    : BeJsFunction (context, jscValue)
    {
    }

template <typename T> JSValueRef BeJsNativeFunction::JscCallback (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
    {
    void* object = JSObjectGetPrivate (function);
    _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
    CallbackInfo info (*proxy->m_context, function, thisObject, argumentCount, arguments);
    return proxy->Callback (info);
    }

template <typename T> JSObjectRef BeJsNativeFunction::JscCallbackConstructor (JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
    {
    void* object = JSObjectGetPrivate (constructor);
    _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
    CallbackInfo info (*proxy->m_context, constructor, NULL, argumentCount, arguments);
    
    JSValueRef obj = proxy->Callback (info);
    if (JSValueIsNull (ctx, obj) || JSValueIsUndefined (ctx, obj))
        return info.m_thisObject;
    else
        return JSValueToObject (ctx, obj, NULL);
    }

BeJsNativeFunction::CallbackInfo::CallbackInfo (BeJsContextCR context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[])
    : m_context (&context), m_function (function), m_thisObject (thisObject), m_argumentCount (argumentCount), m_arguments (arguments)
    {
    m_isConstructCall = thisObject == NULL;
    if (m_isConstructCall)
        {
        JSObjectRef object = JSObjectMake (m_context->GetJscContext(), NULL, NULL);
        m_thisObject = object;
        }
    }

BeJsNativeFunction BeJsNativeFunction::CallbackInfo::GetFunction()
    {
    return BeJsNativeFunction (*m_context, m_function);
    }

BeJsContextCR BeJsNativeFunction::CallbackInfo::GetContext()
    {
    return *m_context;
    }

bool BeJsNativeFunction::CallbackInfo::IsConstructCall()
    {
    return m_isConstructCall;
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetCallContext()
    {
    return BeJsObject (*m_context, m_thisObject);
    }

uint32_t BeJsNativeFunction::CallbackInfo::GetArgumentCount() const
    {
    return m_argumentCount;
    }

BeJsValue BeJsNativeFunction::CallbackInfo::GetArgument (uint32_t index) const
    {
    return BeJsValue (*m_context, m_arguments [index]);
    }

BeJsBoolean BeJsNativeFunction::CallbackInfo::GetBooleanArgument (uint32_t index) const
    {
    return BeJsBoolean (*m_context, m_arguments [index]);
    }

BeJsNumber BeJsNativeFunction::CallbackInfo::GetNumberArgument (uint32_t index) const
    {
    return BeJsNumber (*m_context, m_arguments [index]);
    }

BeJsString BeJsNativeFunction::CallbackInfo::GetStringArgument (uint32_t index) const
    {
    return BeJsString (*m_context, m_arguments [index]);
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetObjectArgument (uint32_t index) const
    {
    return BeJsObject (*m_context, m_arguments [index]);
    }

BeJsNativePointer BeJsNativeFunction::CallbackInfo::GetNativePointerArgument (uint32_t index) const
    {
    return BeJsNativePointer (*m_context, m_arguments [index]);
    }

BeJsArray BeJsNativeFunction::CallbackInfo::GetArrayArgument (uint32_t index) const
    {
    return BeJsArray (*m_context, m_arguments [index]);
    }

BeJsFunction BeJsNativeFunction::CallbackInfo::GetFunctionArgument (uint32_t index) const
    {
    return BeJsFunction (*m_context, m_arguments [index]);
    }

BeJsNativeObject::Callbacks::Callbacks (GetProperty getter,
                                               SetProperty setter,
                                               DeleteProperty deleter,
                                               EnumerateProperties enumerator,
                                               DoesPropertyExist query,
                                               BeJsNativePointer::DisposeCallback disposer)
    : getProperty (getter), setProperty (setter), deleteProperty (deleter), enumerateProperties (enumerator), doesPropertyExist (query), dispose (disposer)
    {
    }

BeJsNativeObject::BeJsNativeObject (BeJsContextCR context, Callbacks const& callbacks, void* object)
    : BeJsObject (context)
    {
    _BeJsNativeObject* proxy = new _BeJsNativeObject (context, callbacks, object);
    
    JSClassDefinition definition = kJSClassDefinitionEmpty;
    definition.finalize = &_BeJsNativeObject::Dispose;
    definition.getProperty = &_BeJsNativeObject::GetProperty;
    
    if (callbacks.setProperty != nullptr)
        definition.setProperty = &_BeJsNativeObject::SetProperty;
        
    if (callbacks.doesPropertyExist != nullptr)
        definition.hasProperty = &_BeJsNativeObject::DoesPropertyExist;
        
    if (callbacks.deleteProperty != nullptr)
        definition.deleteProperty = &_BeJsNativeObject::DeleteProperty;
        
    if (callbacks.enumerateProperties != nullptr)
        definition.getPropertyNames = &_BeJsNativeObject::EnumerateProperties;
        
    JSClassRef clazz = JSClassCreate (&definition);
    JSObjectRef jsObject = JSObjectMake (m_context->GetJscContext(), clazz, proxy);
    m_jscValue = jsObject;
    JSValueProtect (m_context->GetJscContext(), m_jscValue);
    JSClassRelease (clazz);
    }

#endif

#if defined (BEJAVASCRIPT_JSRT_ENGINE)

static JsSourceContext s_jsrtDebugScriptCookieBlock = 0;

namespace {

struct _BeJsNativeObjectWrapper
    {
public:
    void* m_object;
    _BeJsNativeObjectWrapper (void* object = nullptr) : m_object (object) {};
    };

struct _BeJsNativePointer : public _BeJsNativeObjectWrapper
    {
public:
    BeJsNativePointer::DisposeCallback m_callback;

    _BeJsNativePointer (BeJsContextCR context, void* object, BeJsNativePointer::DisposeCallback callback)
        : _BeJsNativeObjectWrapper (object), m_callback (callback)
        {
        };

    ~_BeJsNativePointer()
        {
        };

    static void CALLBACK Dispose (void* object)
        {
        _BeJsNativePointer* external = static_cast<_BeJsNativePointer*>(object);

        if (external->m_callback != nullptr)
            external->m_callback (external->m_object, nullptr);

        delete external;
        };

    static _BeJsNativeObjectWrapper* GetFromHandle (JsValueRef jsrtValue)
        {
        void* data;
        JsErrorCode s = JsGetExternalData (jsrtValue, &data);
        BeAssert (s == JsNoError);

        return static_cast<_BeJsNativeObjectWrapper*>(data);
        };
    };
    
template <class T> struct _BeJsNativeFunction : _BeJsNativeObjectWrapper
    {
private:
    void RemoveCallback()
        {
        m_callback = nullptr;
        }

public:
    BeJsContext const* m_context;
    T m_callback;

    _BeJsNativeFunction(BeJsContextCR context, T callback)
        : m_context (&context), m_callback (callback)
        {
        };

    ~_BeJsNativeFunction()
        {
        m_callback = nullptr;
        };

    JsValueRef Callback (BeJsNativeFunction::CallbackInfoR info)
        {
        if (m_callback != nullptr)
            return m_callback (info).GetJsrtValue();
        
        return nullptr;
        };
        

    static void CALLBACK Dispose (void* object)
        {
        _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
        delete proxy;
        };

    static void RemoveNativeCallback(_BeJsNativeFunction<T>* proxy)
        {
        proxy->RemoveCallback();
        };
    };
}

BeJsEngine BeJsEnvironment::GetActiveEngine()
    {
#if defined (WipPartEnable_chakracore)
    return BeJsEngine::ChakraCore;
#elif defined (USE_EDGEMODE_JSRT)
    return BeJsEngine::JsrtEdge;
#else
    return BeJsEngine::Jsrt;
#endif
    }

BeJsEnvironment::BeJsEnvironment()
    : m_destroying (false),
      m_ownsSystemEnvironment (true)
    {
    #if defined (USE_EDGEMODE_JSRT) || defined (WipPartEnable_chakracore)
    JsErrorCode s = JsCreateRuntime (static_cast<JsRuntimeAttributes>(JsRuntimeAttributeEnableExperimentalFeatures | JsRuntimeAttributeAllowScriptInterrupt), nullptr, &m_jsrtRuntime);
    #else
    JsErrorCode s = JsCreateRuntime (JsRuntimeAttributeAllowScriptInterrupt, JsRuntimeVersionEdge, nullptr, &m_jsrtRuntime);
    #endif

    BeAssert (s == JsNoError);
    }

BeJsEnvironment::BeJsEnvironment (BeJsEngine engine, void* systemEnvironment)
    : m_destroying (false),
      m_ownsSystemEnvironment (false)
    {
    BeAssert (engine == GetActiveEngine());

    m_jsrtRuntime = reinterpret_cast<JsRuntimeHandle>(systemEnvironment);
    }

void* BeJsEnvironment::GetSystemObject()
    {
    return m_jsrtRuntime;
    }

BeJsEnvironment::~BeJsEnvironment()
    {
    m_destroying = true;

    if (m_ownsSystemEnvironment)
        {
        JsErrorCode s = JsSetCurrentContext (JS_INVALID_REFERENCE);
        BeAssert (s == JsNoError);

        s = JsDisposeRuntime (m_jsrtRuntime);
        BeAssert (s == JsNoError);
        }
    }

BeJsContext::BeJsContext (BeJsEnvironmentCR environment, Utf8CP identifier, Utf8CP initializationScript)
    : m_environment (&environment), m_jsrtDebugScriptCookie (0), m_ownsSystemContext (true)
    {
    #if defined (USE_EDGEMODE_JSRT) || defined (WipPartEnable_chakracore)
    JsErrorCode s = JsCreateContext (environment.GetJsrtRuntime(), &m_jsrtContext);
    #else
    IDebugApplication* debugApplication = nullptr;
    
    #if !defined (NDEBUG)
    IClassFactory* classFactory = nullptr;
    CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED);
    CoGetClassObject (__uuidof (ProcessDebugManager), CLSCTX_INPROC_SERVER, NULL, __uuidof (IClassFactory), (LPVOID*)&classFactory);
    classFactory->CreateInstance (0, _uuidof (IProcessDebugManager), (LPVOID*)&m_jsrtDebugManager);

    m_jsrtDebugManager->CreateApplication (&m_jsrtDebugApplication);
    m_jsrtDebugApplication->SetName (WString (identifier, true).GetWCharCP());
    m_jsrtDebugManager->AddApplication (m_jsrtDebugApplication, &m_jsrtDebugApplicationCookie);

    debugApplication = m_jsrtDebugApplication;
    #endif
    
    JsErrorCode s = JsCreateContext (environment.GetJsrtRuntime(), debugApplication, &m_jsrtContext);
    #endif
    BeAssert (s == JsNoError);

    SetJsrtContextActive();

    #if defined (USE_EDGEMODE_JSRT)
        #if !defined (NDEBUG)
        #if defined (BENTLEYCONFIG_OS_WINRT)
        RoInitialize (RO_INIT_MULTITHREADED);
        #else
        CoInitializeEx (nullptr, COINIT_APARTMENTTHREADED);
        #endif

        s = JsStartDebugging();
        BeAssert (s == JsNoError);
        #endif
    #endif

    Initialize (initializationScript, identifier);
    }

BeJsContext::BeJsContext (BeJsEngine engine, BeJsEnvironmentCR environment, void* systemContext)
    : m_environment (&environment), m_jsrtDebugScriptCookie (s_jsrtDebugScriptCookieBlock += 10000), m_ownsSystemContext (false)
    {
    BeAssert (engine == BeJsEnvironment::GetActiveEngine());

    m_jsrtContext = reinterpret_cast<JsContextRef>(systemContext);
    SetJsrtContextActive();
    Initialize (nullptr, nullptr);
    }

void* BeJsContext::GetSystemObject()
    {
    return m_jsrtContext;
    }

void BeJsContext::SetJsrtContextActive() const
    {
    JsErrorCode s = JsSetCurrentContext (m_jsrtContext);
    BeAssert (s == JsNoError);
    }

BeJsContext::~BeJsContext()
    {
    _OnDestroy();

    for (auto p : m_cleanupQueue)
        {
        p.second(p.first);
        }

    if (m_ownsSystemContext)
        {
        #if !defined (NDEBUG) && !defined (USE_EDGEMODE_JSRT) && !defined (WipPartEnable_chakracore)
        m_jsrtDebugApplication->Close();
        m_jsrtDebugManager->RemoveApplication (m_jsrtDebugApplicationCookie);
        #endif

        JsContextRef c;
        JsErrorCode s = JsGetCurrentContext (&c);
        BeAssert (s == JsNoError);
    
        if (c == m_jsrtContext)
            {
            s = JsSetCurrentContext (JS_INVALID_REFERENCE);
            BeAssert (s == JsNoError);
            }
        }
    }

BeJsValue BeJsContext::EvaluateScript (Utf8CP script, Utf8CP identifier, EvaluateStatus* status, EvaluateException* exception) const
    {
    SetJsrtContextActive();

    WString scriptW (script, true);
    WString identifierW (identifier, true);

    JsValueRef scriptValue;
    JsErrorCode s = JsRunScript (scriptW.GetWCharCP(), identifier == nullptr ? JS_SOURCE_CONTEXT_NONE : m_jsrtDebugScriptCookie++, identifierW.GetWCharCP(), &scriptValue);

    if (status == nullptr)
        {
        BeAssert (s == JsNoError);
        }
    else
        {
        if (s == JsNoError)
            {
            *status = EvaluateStatus::Success;
            }
        else
            {
            if (s == JsErrorScriptCompile || s == JsErrorBadSerializedScript || s == JsErrorCategoryScript)
                *status = EvaluateStatus::ParseError;
            else
                *status = EvaluateStatus::RuntimeError;

            if (exception != nullptr)
                {
                JsValueRef jsrtException;
                s = JsGetAndClearException (&jsrtException);
                if (s == JsNoError)
                    {
                    BeJsObject exceptionObject (*this, jsrtException);
                    BeJsString trace = exceptionObject.GetStringProperty ("stack");

                    (*exception).message = BeJsString (exceptionObject.CallMemberFunction ("toString")).GetValue();

                    if (trace.IsString())
                        (*exception).trace = trace.GetValue();
                    }
                }

            return BeJsValue::InvalidHandle();
            }
        }

    return BeJsValue (*this, scriptValue);
    }

BeJsObject BeJsContext::GetGlobalObject() const
    {
    SetJsrtContextActive();

    JsValueRef object;
    JsErrorCode s = JsGetGlobalObject (&object);
    BeAssert (s == JsNoError);

    return BeJsObject (*this, object);
    }

BeJsValue::BeJsValue (BeJsContextCR context)
    : m_context (&context), m_jsrtValue (nullptr)
    {
    }

BeJsValue::BeJsValue (BeJsValueCR other)
    : m_context (other.m_context), m_jsrtValue (other.m_jsrtValue)
    {
    if (nullptr != m_context)
        m_context->SetJsrtContextActive();

    if (nullptr != m_jsrtValue)
        {
        JsErrorCode s = JsAddRef (m_jsrtValue, nullptr);
        BeAssert (s == JsNoError);
        }
    }

BeJsValueR BeJsValue::operator= (BeJsValueCR other)
    {
    if (this == &other)
        return *this;

    if (nullptr != other.m_context)
        other.m_context->SetJsrtContextActive();
        
    if (m_jsrtValue != nullptr)
        {
        JsErrorCode s = JsRelease (&m_jsrtValue, nullptr);
        BeAssert (s == JsNoError);
        }

    m_context = other.m_context;
    m_jsrtValue = other.m_jsrtValue;
    
    if (nullptr != m_jsrtValue)
        {
        JsErrorCode s = JsAddRef (m_jsrtValue, nullptr);
        BeAssert (s == JsNoError);
        }

    return *this;
    }

BeJsValue::BeJsValue (BeJsValue&& other)
    : m_context (other.m_context), m_jsrtValue (other.m_jsrtValue)
    {
    other.m_jsrtValue = nullptr;
    }

BeJsValue::BeJsValue (BeJsContextCR context, JsValueRef jsrtValue)
    : m_context (&context), m_jsrtValue (jsrtValue)
    {
    m_context->SetJsrtContextActive();
    JsErrorCode s = JsAddRef (m_jsrtValue, nullptr);
    BeAssert (s == JsNoError);
    }
    
BeJsValue::~BeJsValue()
    {
    if (m_context == nullptr)
        return;

    if (m_jsrtValue != nullptr)
        {
        m_context->SetJsrtContextActive();
        JsErrorCode s = JsRelease (&m_jsrtValue, nullptr);
        BeAssert (s == JsNoError);
        }
    }

void BeJsValue::Reset()
    {
    if (m_jsrtValue != nullptr)
        {
        m_context->SetJsrtContextActive();
        JsErrorCode s = JsRelease (&m_jsrtValue, nullptr);
        BeAssert (s == JsNoError);
        m_jsrtValue = nullptr;
        }
    }

bool BeJsValue::IsEmpty() const
    {
    return m_jsrtValue == nullptr;
    }

void BeJsValue::NotifyEngineValueReleased()
    {
    m_jsrtValue = nullptr;
    }

void BeJsValue::SetJsrtValue (JsValueRef value)
    {
    m_context->SetJsrtContextActive();

    if (m_jsrtValue != nullptr)
        {
        JsErrorCode s = JsRelease (&m_jsrtValue, nullptr);
        BeAssert (s == JsNoError);
        }
        
    m_jsrtValue = value;

    JsErrorCode s = JsAddRef (m_jsrtValue, nullptr);
    BeAssert (s == JsNoError);
    }

JsValueType BeJsValue::GetJsrtValueType() const
    {
    m_context->SetJsrtContextActive();

    JsValueType type;
    JsErrorCode s = JsGetValueType (m_jsrtValue, &type);
    BeAssert (s == JsNoError);
    return type;
    }

bool BeJsValue::IsNull() const
    {
    return GetJsrtValueType() == JsNull;
    }

bool BeJsValue::IsUndefined() const
    {
    return GetJsrtValueType() == JsUndefined;
    }

bool BeJsValue::IsNumber() const
    {
    return GetJsrtValueType() == JsNumber;
    }

bool BeJsValue::IsBoolean() const
    {
    return GetJsrtValueType() == JsBoolean;
    }

bool BeJsValue::IsString() const
    {
    return GetJsrtValueType() == JsString;
    }

bool BeJsValue::IsObject() const
    {
    JsValueType t = GetJsrtValueType();
    return t == JsObject || t == JsArray || t == JsFunction || t == JsError;
    }

bool BeJsValue::IsFunction() const
    {
    return GetJsrtValueType() == JsFunction;
    }

bool BeJsValue::IsEqual (BeJsValueCR value) const
    {
    m_context->SetJsrtContextActive();

    bool equal;
    JsErrorCode s = JsEquals (m_jsrtValue, value.GetJsrtValue(), &equal);
    BeAssert (s == JsNoError);

    return equal;
    }

bool BeJsValue::IsEqualStrict (BeJsValueCR value) const
    {
    m_context->SetJsrtContextActive();

    bool equal;
    JsErrorCode s = JsStrictEquals (m_jsrtValue, value.GetJsrtValue(), &equal);
    BeAssert (s == JsNoError);

    return equal;
    }

BeJsValue BeJsValue::Null (BeJsContextCR context)
    {
    context.SetJsrtContextActive();

    JsValueRef value;
    JsErrorCode s = JsGetNullValue (&value);
    BeAssert (s == JsNoError);

    return BeJsValue (context, value);
    }

BeJsValue BeJsValue::Undefined (BeJsContextCR context)
    {
    context.SetJsrtContextActive();

    JsValueRef value;
    JsErrorCode s = JsGetUndefinedValue (&value);
    BeAssert (s == JsNoError);

    return BeJsValue (context, value);
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsValue (context, jsrtValue)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, bool value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsPrimitive (context, jsrtValue)
    {
    }

bool BeJsBoolean::GetValue() const
    {
    BeAssert (IsBoolean());

    m_context->SetJsrtContextActive();

    bool value;
    JsErrorCode s = JsBooleanToBool (m_jsrtValue, &value);
    BeAssert (s == JsNoError);

    return value;
    }

void BeJsBoolean::SetValue (bool value)
    {
    m_context->SetJsrtContextActive();

    JsValueRef jsValue;
    JsErrorCode s = JsBoolToBoolean (value, &jsValue);
    BeAssert (s == JsNoError);
    SetJsrtValue (jsValue);
    }

BeJsNumber::BeJsNumber (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, double value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, int32_t value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsPrimitive (context, jsrtValue)
    {
    }

double BeJsNumber::GetValue() const
    {
    if (IsNull())
        return 0;

    BeAssert (IsNumber());

    m_context->SetJsrtContextActive();

    double value;
    JsErrorCode s = JsNumberToDouble (m_jsrtValue, &value);
    BeAssert (s == JsNoError);

    return value;
    }

int32_t BeJsNumber::GetIntegerValue() const
    {
    return static_cast<int32_t>(GetValue());
    }

void BeJsNumber::SetValue (double value)
    {
    m_context->SetJsrtContextActive();

    JsValueRef jsValue;
    JsErrorCode s = JsDoubleToNumber (value, &jsValue);
    BeAssert (s == JsNoError);
    SetJsrtValue (jsValue);
    }

void BeJsNumber::SetValue (int32_t value)
    {
    SetValue (static_cast<double>(value));
    }

BeJsString::BeJsString (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsString::BeJsString (BeJsContextCR context, Utf8CP value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsString::BeJsString (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsPrimitive (context, jsrtValue)
    {
    }

/*Utf8String BeJsString::GetUtf8String (JsValueRef value)
    {
    const wchar_t* valueW;
	size_t length;
    JsStringToPointer (value, &valueW, &length);

    return Utf8String (valueW);
    }*/

Utf8String BeJsString::GetValue() const
    {
    if (IsNull())
        return Utf8String();

    BeAssert (IsString());

    m_context->SetJsrtContextActive();

    const wchar_t* valueW;
	size_t length;
    JsErrorCode s = JsStringToPointer (m_jsrtValue, &valueW, &length);
    BeAssert (s == JsNoError);

    return Utf8String (valueW);
    }

void BeJsString::SetValue (Utf8CP value)
    {
    m_context->SetJsrtContextActive();

    JsValueRef jsValue;
    WString valueW (value, true);
    JsErrorCode s = JsPointerToString (valueW.GetWCharCP(), valueW.length(), &jsValue);
    BeAssert (s == JsNoError);
    SetJsrtValue (jsValue);
    }

int64_t BeJsString::GetValueAsInt64() const
    {
    Utf8String s = GetValue();
    int64_t v;
    sscanf (s.c_str(), "%lld", &v);

    return v;
    }

uint64_t BeJsString::GetValueAsUnsignedInt64() const
    {
    Utf8String s = GetValue();
    uint64_t v;
    sscanf (s.c_str(), "%llu", &v);

    return v;
    }

BeJsObject::BeJsObject (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsValue (context, jsrtValue)
    {
    }

JsPropertyIdRef BeJsObject::GetJsrtPropertyId (const char* propertyName)
    {
    JsPropertyIdRef propertyId;
    WString	propertyNameW (propertyName, true);
	JsErrorCode s = JsGetPropertyIdFromName (propertyNameW.GetWCharCP(), &propertyId);
    BeAssert (s == JsNoError);

    return propertyId;
    }

BeJsValue BeJsObject::GetProperty (Utf8CP name) const
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef value;
    JsErrorCode s = JsGetProperty (m_jsrtValue, GetJsrtPropertyId (name), &value);
    BeAssert (s == JsNoError);

    return BeJsValue (*m_context, value);
    }

BeJsBoolean BeJsObject::GetBooleanProperty (Utf8CP name) const
    {
    return BeJsBoolean (GetProperty (name));
    }

BeJsNumber BeJsObject::GetNumberProperty (Utf8CP name) const
    {
    return BeJsNumber (GetProperty (name));
    }

BeJsString BeJsObject::GetStringProperty (Utf8CP name) const
    {
    return BeJsString (GetProperty (name));
    }

BeJsObject BeJsObject::GetObjectProperty (Utf8CP name) const
    {
    return BeJsObject (GetProperty (name));
    }

BeJsNativePointer BeJsObject::GetNativePointerProperty (Utf8CP name) const
    {
    return BeJsNativePointer (GetProperty (name));
    }

BeJsArray BeJsObject::GetArrayProperty (Utf8CP name) const
    {
    return BeJsArray (GetProperty (name));
    }

BeJsFunction BeJsObject::GetFunctionProperty (Utf8CP name) const
    {
    return BeJsFunction (GetProperty (name));
    }

void BeJsObject::SetProperty (Utf8CP name, BeJsValueCR value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsErrorCode s = JsSetProperty (m_jsrtValue, GetJsrtPropertyId (name), value.GetJsrtValue(), true);
    BeAssert (s == JsNoError);
    }

void BeJsObject::SetBooleanProperty (Utf8CP name, bool value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsValue;
    JsErrorCode s = JsBoolToBoolean (value, &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetProperty (m_jsrtValue, GetJsrtPropertyId (name), jsValue, true);
    BeAssert (s == JsNoError);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, double value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsValue;
    JsErrorCode s = JsDoubleToNumber (value, &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetProperty (m_jsrtValue, GetJsrtPropertyId (name), jsValue, true);
    BeAssert (s == JsNoError);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, int32_t value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsValue;
    JsErrorCode s = JsDoubleToNumber (value, &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetProperty (m_jsrtValue, GetJsrtPropertyId (name), jsValue, true);
    BeAssert (s == JsNoError);
    }

void BeJsObject::SetStringProperty (Utf8CP name, Utf8CP value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsValue;
    WString valueW (value, true);
    JsErrorCode s = JsPointerToString (valueW.GetWCharCP(), valueW.length(), &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetProperty (m_jsrtValue, GetJsrtPropertyId (name), jsValue, true);
    BeAssert (s == JsNoError);
    }

void BeJsObject::DeleteProperty (Utf8CP name)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef d;
    JsErrorCode s = JsDeleteProperty (m_jsrtValue, GetJsrtPropertyId (name), true, &d);
    BeAssert (s == JsNoError);
    }

bool BeJsObject::DoesPropertyExist (Utf8CP name)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    bool hasProperty;
    JsErrorCode s = JsHasProperty (m_jsrtValue, GetJsrtPropertyId (name), &hasProperty);
    BeAssert (s == JsNoError);

    return hasProperty;
    }

void BeJsObject::SetPrototype (BeJsValueCR prototype)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsErrorCode s = JsSetPrototype (m_jsrtValue, prototype.GetJsrtValue());
    BeAssert (s == JsNoError);
    }

BeJsObject BeJsObject::New (BeJsContextCR context)
    {
    context.SetJsrtContextActive();

    JsValueRef object;
    JsErrorCode s = JsCreateObject (&object);
    BeAssert (s == JsNoError);

    return BeJsObject (context, object);
    }

BeJsArray::BeJsArray (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsObject (context, jsrtValue)
    {
    }

BeJsValue BeJsArray::GetProperty (uint32_t index) const
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    JsValueRef value;
    s = JsGetIndexedProperty (m_jsrtValue, jsIndex, &value);
    BeAssert (s == JsNoError);

    return BeJsValue (*m_context, value);
    }

BeJsBoolean BeJsArray::GetBooleanProperty (uint32_t index) const
    {
    return BeJsBoolean (GetProperty (index));
    }

BeJsNumber BeJsArray::GetNumberProperty (uint32_t index) const
    {
    return BeJsNumber (GetProperty (index));
    }

BeJsString BeJsArray::GetStringProperty (uint32_t index) const
    {
    return BeJsString (GetProperty (index));
    }

BeJsObject BeJsArray::GetObjectProperty (uint32_t index) const
    {
    return BeJsObject (GetProperty (index));
    }

BeJsNativePointer BeJsArray::GetNativePointerProperty (uint32_t index) const
    {
    return BeJsNativePointer (GetProperty (index));
    }

BeJsArray BeJsArray::GetArrayProperty (uint32_t index) const
    {
    return BeJsArray (GetProperty (index));
    }

BeJsFunction BeJsArray::GetFunctionProperty (uint32_t index) const
    {
    return BeJsFunction (GetProperty (index));
    }

void BeJsArray::SetProperty (uint32_t index, BeJsValueCR value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    s = JsSetIndexedProperty (m_jsrtValue, jsIndex, value.GetJsrtValue());
    BeAssert (s == JsNoError);
    }

void BeJsArray::SetBooleanProperty (uint32_t index, bool value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    JsValueRef jsValue;
    s = JsBoolToBoolean (value, &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetIndexedProperty (m_jsrtValue, jsIndex, jsValue);
    BeAssert (s == JsNoError);
    }

void BeJsArray::SetNumberProperty (uint32_t index, double value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    JsValueRef jsValue;
    s = JsDoubleToNumber (value, &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetIndexedProperty (m_jsrtValue, jsIndex, jsValue);
    BeAssert (s == JsNoError);
    }

void BeJsArray::SetNumberProperty (uint32_t index, int32_t value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    JsValueRef jsValue;
    s = JsDoubleToNumber (value, &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetIndexedProperty (m_jsrtValue, jsIndex, jsValue);
    BeAssert (s == JsNoError);
    }

void BeJsArray::SetStringProperty (uint32_t index, Utf8CP value)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    JsValueRef jsValue;
    WString valueW (value, true);
    s = JsPointerToString (valueW.GetWCharCP(), valueW.length(), &jsValue);
    BeAssert (s == JsNoError);

    s = JsSetIndexedProperty (m_jsrtValue, jsIndex, jsValue);
    BeAssert (s == JsNoError);
    }

void BeJsArray::DeleteProperty (uint32_t index)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    s = JsDeleteIndexedProperty (m_jsrtValue, jsIndex);
    BeAssert (s == JsNoError);
    }

bool BeJsArray::DoesPropertyExist (uint32_t index)
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef jsIndex;
    JsErrorCode s = JsDoubleToNumber (index, &jsIndex);
    BeAssert (s == JsNoError);

    bool hasProperty;
    s = JsHasIndexedProperty (m_jsrtValue, jsIndex, &hasProperty);
    BeAssert (s == JsNoError);

    return hasProperty;
    }

uint32_t BeJsArray::GetLength() const
    {
    BeAssert (IsObject());

    m_context->SetJsrtContextActive();

    JsValueRef value;
    JsErrorCode s = JsGetProperty (m_jsrtValue, GetJsrtPropertyId ("length"), &value);
    BeAssert (s == JsNoError);

    double length;
    s = JsNumberToDouble (value, &length);
    BeAssert (s == JsNoError);

    return static_cast<uint32_t>(length);
    }

BeJsArray BeJsArray::New (BeJsContextCR context)
    {
    context.SetJsrtContextActive();

    JsValueRef object;
    JsErrorCode s = JsCreateArray (0, &object);
    BeAssert (s == JsNoError);

    return BeJsArray (context, object);
    }

BeJsNativePointer::BeJsNativePointer (BeJsValueCR other)
    : BeJsObject (other),
      m_environment (&other.GetContext().GetEnvironment())
    {
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, void* object, DisposeCallback disposeCallback, BeJsObject const* prototype, bool suppressCallContext)
    : BeJsObject (context),
      m_environment (&context.GetEnvironment())
    {
    context.SetJsrtContextActive();

    _BeJsNativePointer* external = new _BeJsNativePointer (context, object, disposeCallback);

    JsValueRef jsValue;
    JsErrorCode s = JsCreateExternalObject (external, &_BeJsNativePointer::Dispose, &jsValue);
    BeAssert (s == JsNoError);

    if (prototype != nullptr)
        {
        s = JsSetPrototype (jsValue, prototype->GetJsrtValue());
        BeAssert (s == JsNoError);
        }

    SetJsrtValue (jsValue);

    if (context.m_callContextActive && !suppressCallContext)
        context.m_callContextValues.push_back (*this);
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsObject (context, jsrtValue),
      m_environment (&context.GetEnvironment())
    {
    }

void* BeJsNativePointer::GetValue() const
    {
    if (IsNull())
        return nullptr;

    m_context->SetJsrtContextActive();

    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (m_jsrtValue);
    return wrapper->m_object;
    }

void BeJsNativePointer::Dispose()
    {
    if (IsNull())
        return;

    m_context->SetJsrtContextActive();

    _BeJsNativePointer* wrapper = static_cast<_BeJsNativePointer*>(_BeJsNativePointer::GetFromHandle (m_jsrtValue));

    if (wrapper->m_callback != nullptr)
        wrapper->m_callback (wrapper->m_object, this);

    wrapper->m_object = nullptr;
    }

void BeJsNativePointer::SetValue (void* object)
    {
    if (m_environment != nullptr && m_environment->IsDestroying())
        {
        NotifyEngineValueReleased();
        return;
        }

    m_context->SetJsrtContextActive();

    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (m_jsrtValue);
    wrapper->m_object = object;
    }

BeJsFunction::BeJsFunction (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsObject (context, jsrtValue)
    {
    }

BeJsValue BeJsFunction::Call (BeJsValue** args,
                                              BeJsEnvironment::EngineValueHandleType* engineArgs,
                                              size_t argumentCount,
                                              BeJsObject* callScope,
                                              bool callAsConstructor) const
    {
    m_context->SetJsrtContextActive();

    if (argumentCount != 0)
        {
        for (size_t i = 0; i != argumentCount; ++i)
            engineArgs [i + 1] = args [i]->GetJsrtValue();
        }

    BeAssert (IsFunction());
        
    if (callScope != nullptr)
        {
        engineArgs [0] = callScope->GetJsrtValue();
        }
    else
        {
        JsErrorCode s = JsGetNullValue (&*engineArgs);
        BeAssert (s == JsNoError);
        }

    JsValueRef value;
    JsErrorCode s;
    if (callAsConstructor)
        s = JsConstructObject (m_jsrtValue, engineArgs, static_cast<unsigned short>(argumentCount + 1), &value);
    else
        s = JsCallFunction (m_jsrtValue, engineArgs, static_cast<unsigned short>(argumentCount + 1), &value);
    BeAssert (s == JsNoError);

    return BeJsValue (*m_context, value);
    }

template <typename T> void BeJsNativeFunction::Init(BeJsContextCR context, T callback)
    {
    _BeJsNativeFunction<T>* proxy = new _BeJsNativeFunction<T>(context, callback);
    m_removeCallbackFunction = [proxy]() -> void {if (proxy) { _BeJsNativeFunction<T>::RemoveNativeCallback(proxy); }};

    context.m_cleanupQueue[proxy] = _BeJsNativeFunction<T>::Dispose;

    JsValueRef jsValue;
    JsErrorCode s = JsCreateFunction(&JsrtCallback<T>, proxy, &jsValue);
    BeAssert(s == JsNoError);

    SetJsrtValue(jsValue);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, Callback callback)
    : BeJsFunction (context)
    {
    Init(context, callback);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, std::function<BeJsValue(CallbackInfoR info)> callback)
    : BeJsFunction(context)
    {
    Init(context, callback);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, JsValueRef jsrtValue)
    : BeJsFunction (context, jsrtValue)
    {
    }

template <typename T> JsValueRef CALLBACK BeJsNativeFunction::JsrtCallback (JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void* callbackState)
    {
    if (arguments [0] == JS_INVALID_RUNTIME_HANDLE)
        {
        JsErrorCode s = JsGetGlobalObject (arguments);
        BeAssert (s == JsNoError);
        }

    auto proxy = static_cast<_BeJsNativeFunction<T>*>(callbackState);
    CallbackInfo info (*proxy->m_context, callee, isConstructCall, argumentCount, arguments);
    return proxy->Callback (info);
    }

BeJsNativeFunction::CallbackInfo::CallbackInfo (BeJsContextCR context, JsValueRef function, bool isConstructCall, unsigned short argumentCount, JsValueRef* arguments)
    : m_context (&context), m_function (function), m_isConstructCall (isConstructCall), m_argumentCount (argumentCount), m_arguments (arguments)
    {
    }

BeJsNativeFunction BeJsNativeFunction::CallbackInfo::GetFunction()
    {
    return BeJsNativeFunction (*m_context, m_function);
    }

BeJsContextCR BeJsNativeFunction::CallbackInfo::GetContext()
    {
    return *m_context;
    }

bool BeJsNativeFunction::CallbackInfo::IsConstructCall()
    {
    return m_isConstructCall;
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetCallContext()
    {
    return BeJsObject (*m_context, m_arguments [0]);
    }

uint32_t BeJsNativeFunction::CallbackInfo::GetArgumentCount() const
    {
    return m_argumentCount - 1;
    }

BeJsValue BeJsNativeFunction::CallbackInfo::GetArgument (uint32_t index) const
    {
    return BeJsValue (*m_context, m_arguments [index + 1]);
    }

BeJsBoolean BeJsNativeFunction::CallbackInfo::GetBooleanArgument (uint32_t index) const
    {
    return BeJsBoolean (*m_context, m_arguments [index + 1]);
    }

BeJsNumber BeJsNativeFunction::CallbackInfo::GetNumberArgument (uint32_t index) const
    {
    return BeJsNumber (*m_context, m_arguments [index + 1]);
    }

BeJsString BeJsNativeFunction::CallbackInfo::GetStringArgument (uint32_t index) const
    {
    return BeJsString (*m_context, m_arguments [index + 1]);
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetObjectArgument (uint32_t index) const
    {
    return BeJsObject (*m_context, m_arguments [index + 1]);
    }

BeJsNativePointer BeJsNativeFunction::CallbackInfo::GetNativePointerArgument (uint32_t index) const
    {
    return BeJsNativePointer (*m_context, m_arguments [index + 1]);
    }

BeJsArray BeJsNativeFunction::CallbackInfo::GetArrayArgument (uint32_t index) const
    {
    return BeJsArray (*m_context, m_arguments [index + 1]);
    }

BeJsFunction BeJsNativeFunction::CallbackInfo::GetFunctionArgument (uint32_t index) const
    {
    return BeJsFunction (*m_context, m_arguments [index + 1]);
    }

#endif

#if defined (BEJAVASCRIPT_DUKTAPE_ENGINE)

static const char* s_dukDataPropertyName = "\xff\xff" "BeData";

namespace {

struct _BeJsNativeObjectWrapper
    {
public:
    void* m_object;
    _BeJsNativeObjectWrapper (void* object = nullptr) : m_object (object) {};
    };

struct _BeJsNativePointer : public _BeJsNativeObjectWrapper
    {
public:
    BeJsNativePointer::DisposeCallback m_callback;

    _BeJsNativePointer (BeJsContextCR context, void* object, BeJsNativePointer::DisposeCallback callback)
        : _BeJsNativeObjectWrapper (object), m_callback (callback)
        {
        };

    ~_BeJsNativePointer()
        {
        };

    static duk_ret_t Dispose (duk_context* ctx)
        {
        duk_bool_t s = duk_get_prop_string (ctx, -1, s_dukDataPropertyName);
        BeAssert (s == 1);

        void* object = duk_get_pointer (ctx, -1);
        duk_pop (ctx);

        _BeJsNativePointer* external = static_cast<_BeJsNativePointer*>(object);

        if (external->m_callback != nullptr)
            external->m_callback (external->m_object, nullptr);

        delete external;

        return 0;
        };

    static _BeJsNativeObjectWrapper* GetFromHandle (BeJsContextCR context, duk_uarridx_t stashIndex)
        {
        duk_context* ctx = context.GetDukContext();
        context.GetDukGlobalStashProperty (stashIndex);
        
        duk_bool_t s = duk_get_prop_string (ctx, -1, s_dukDataPropertyName);
        BeAssert (s == 1);
        
        void* data = duk_get_pointer (ctx, -1);
        duk_pop_2 (ctx);

        return static_cast<_BeJsNativeObjectWrapper*>(data);
        };
    };
    
template <class T> struct _BeJsNativeFunction : _BeJsNativeObjectWrapper
    {
private:
    void RemoveCallback()
        {
        m_callback = nullptr;
        }

public:
    BeJsContext const* m_context;
    T m_callback;

    _BeJsNativeFunction (BeJsContextCR context, T callback)
        : m_context (&context), m_callback (callback)
        {
        };

    ~_BeJsNativeFunction()
        {
        m_callback = nullptr;
        };

    duk_ret_t Callback (BeJsNativeFunction::CallbackInfoR info)
        {
        if (m_callback != nullptr)
            {
            BeJsValue response = m_callback (info);
            m_context->GetDukGlobalStashProperty (response.GetDukStashIndex());
            return 1;
            }
        
        return 0;
        };

    static duk_ret_t Dispose (duk_context* ctx)
        {
        duk_bool_t s = duk_get_prop_string (ctx, -1, s_dukDataPropertyName);
        BeAssert (s == 1);

        void* object = duk_get_pointer (ctx, -1);
        duk_pop (ctx);

        _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
        delete proxy;

        return 1;
        };

    static void RemoveNativeCallback(_BeJsNativeFunction<T>* proxy)
        {
        proxy->RemoveCallback();
        };
    };
    
struct _BeJsNativeObject : public _BeJsNativeObjectWrapper
    {
public:
    BeJsNativeObject::Callbacks m_callbacks;
    BeJsContext const* m_context;

    _BeJsNativeObject (BeJsContextCR context, BeJsNativeObject::Callbacks callbacks, void* object)
        : _BeJsNativeObjectWrapper (object), m_callbacks (callbacks), m_context (&context)
        {
        }

    ~_BeJsNativeObject()
        {
        };

    static void Dispose (void* object)
        {
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.dispose != nullptr)
            proxy->m_callbacks.dispose (proxy->m_object, nullptr);
        
        delete proxy;
        };
        
    static BeJsValue DoesPropertyExist (BeJsNativeFunction::CallbackInfoR info)
        {
        void* object = info.GetExternalArgument (0).GetValue();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.doesPropertyExist != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, proxy->m_object, nullptr);
            Utf8String propertyName = info.GetStringArgument (1).GetValue();
            return BeJsBoolean (info.GetContext(), proxy->m_callbacks.doesPropertyExist (external, propertyName.c_str()));
            }
            
        return BeJsBoolean (info.GetContext(), false);
        };

    static BeJsValue GetProperty (BeJsNativeFunction::CallbackInfoR info)
        {
        void* object = info.GetExternalArgument (0).GetValue();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.getProperty != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, proxy->m_object, nullptr);
            Utf8String propertyName = info.GetStringArgument (1).GetValue();
            return proxy->m_callbacks.getProperty (external, propertyName.c_str());
            }
        
        return BeJsValue::Undefined (info.GetContext());
        };

    static BeJsValue SetProperty (BeJsNativeFunction::CallbackInfoR info)
        {
        void* object = info.GetExternalArgument (0).GetValue();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.setProperty != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, proxy->m_object, nullptr);
            Utf8String propertyName = info.GetStringArgument (1).GetValue();
            proxy->m_callbacks.setProperty (external, propertyName.c_str(), info.GetArgument (2));
            }
            
        return BeJsBoolean (info.GetContext(), true);
        };

    static BeJsValue DeleteProperty (BeJsNativeFunction::CallbackInfoR info)
        {
        void* object = info.GetExternalArgument (0).GetValue();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.deleteProperty != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, proxy->m_object, nullptr);
            Utf8String propertyName = info.GetStringArgument (1).GetValue();
            proxy->m_callbacks.deleteProperty (external, propertyName.c_str());
            }
            
        return BeJsBoolean (info.GetContext(), true);
        };

    static BeJsValue EnumerateProperties (BeJsNativeFunction::CallbackInfoR info)
        {
        void* object = info.GetExternalArgument (0).GetValue();
        _BeJsNativeObject* proxy = static_cast<_BeJsNativeObject*>(object);

        if (proxy->m_callbacks.enumerateProperties != nullptr)
            {
            BeJsNativePointer external (*proxy->m_context, proxy->m_object, nullptr);
            return proxy->m_callbacks.enumerateProperties (external);
            }

        return BeJsArray::New (info.GetContext());
        };
    };
    
}

BeJsEnvironment::BeJsEnvironment()
    : m_dukHeapInitialContext (duk_create_heap (nullptr, nullptr, nullptr, nullptr, nullptr)), m_lastHeapStashIndex (0), m_destroying (false)
    {
    }

BeJsEnvironment::~BeJsEnvironment()
    {
    m_destroying = true;
    duk_destroy_heap (m_dukHeapInitialContext);
    }

BeJsContext::BeJsContext (BeJsEnvironmentCR environment, Utf8CP identifier, Utf8CP initializationScript)
    : m_environment (&environment), m_lastGlobalStashIndex (0), m_dukHeapStashIndex (environment.GetNextHeapStashIndex())
    {
    duk_context* env = environment.GetDukHeapInitialContext();
    duk_push_heap_stash (env);
    duk_push_thread_new_globalenv (env);
    m_dukContext = duk_get_context (env, -1);
    duk_bool_t s = duk_put_prop_index (env, -2, environment.GetNextHeapStashIndex());
    BeAssert (s == 1);
    duk_pop (env);

    m_proxyObjectGetter = new BeJsNativeFunction (*this, &_BeJsNativeObject::GetProperty);
    m_proxyObjectSetter = new BeJsNativeFunction (*this, &_BeJsNativeObject::SetProperty);
    m_proxyObjectDeleter = new BeJsNativeFunction (*this, &_BeJsNativeObject::DeleteProperty);
    m_proxyObjectEnumerator = new BeJsNativeFunction (*this, &_BeJsNativeObject::EnumerateProperties);
    m_proxyObjectQuery = new BeJsNativeFunction (*this, &_BeJsNativeObject::DoesPropertyExist);
    m_ES6_Proxy = new BeJsFunction (GetGlobalObject().GetFunctionProperty ("Proxy"));

    Initialize (initializationScript, identifier);
    }

BeJsContext::~BeJsContext()
    {
    _OnDestroy();

    delete m_proxyObjectGetter;
    delete m_proxyObjectSetter;
    delete m_proxyObjectDeleter;
    delete m_proxyObjectEnumerator;
    delete m_proxyObjectQuery;
    delete m_ES6_Proxy;

    duk_context* env = m_environment->GetDukHeapInitialContext();
    duk_push_heap_stash (env);
    duk_bool_t s = duk_del_prop_index (env, -1, m_dukHeapStashIndex);
    BeAssert (s == 1);
    duk_pop (env);
    }

duk_uarridx_t BeJsContext::PutDukGlobalStashProperty (duk_idx_t stackIndex, bool popStackValue) const
    {
    duk_push_global_stash (m_dukContext);
    
    duk_dup (m_dukContext, stackIndex - 1);
    
    duk_bool_t s = duk_put_prop_index (m_dukContext, -2, GetNextGlobalStashIndex());
    BeAssert (s == 1);

    if (popStackValue)
        duk_pop_2 (m_dukContext);
    else
        duk_pop (m_dukContext);

    return m_lastGlobalStashIndex;
    }

void BeJsContext::UpdateDukGlobalStashProperty (duk_uarridx_t propertyIndex, duk_idx_t stackIndex, bool popStackValue) const
    {
    duk_push_global_stash (m_dukContext);

    duk_dup (m_dukContext, stackIndex - 1);

    duk_bool_t s = duk_put_prop_index (m_dukContext, -2, propertyIndex);
    BeAssert (s == 1);

    if (popStackValue)
        duk_pop_2 (m_dukContext);
    else
        duk_pop (m_dukContext);    
    }

duk_uarridx_t BeJsContext::DuplicateDukGlobalStashProperty (duk_uarridx_t propertyIndex) const
    {
    if (propertyIndex == 0)
        return propertyIndex;

    duk_push_global_stash (m_dukContext);

    duk_bool_t s = duk_get_prop_index (m_dukContext, -1, propertyIndex);
    BeAssert (s == 1);
    
    s = duk_put_prop_index (m_dukContext, -2, GetNextGlobalStashIndex());
    BeAssert (s == 1);

    duk_pop (m_dukContext);

    return m_lastGlobalStashIndex;
    }

void BeJsContext::DeleteDukGlobalStashProperty (duk_uarridx_t propertyIndex) const
    {
    if (propertyIndex == 0)
        return;

    duk_push_global_stash (m_dukContext);
    
    duk_bool_t s = duk_del_prop_index (m_dukContext, -1, propertyIndex);
    BeAssert (s == 1);
    
    duk_pop (m_dukContext);
    }

void BeJsContext::GetDukGlobalStashProperty (duk_uarridx_t propertyIndex) const
    {
    BeAssert (propertyIndex != 0);

    duk_push_global_stash (m_dukContext);

    duk_bool_t s = duk_get_prop_index (m_dukContext, -1, propertyIndex);
    BeAssert (s == 1);
    
    duk_replace (m_dukContext, -2);
    }

BeJsValue BeJsContext::EvaluateScript (Utf8CP script, Utf8CP identifier, EvaluateStatus* status, EvaluateException* exception) const
    {
    duk_int_t s = duk_peval_string (m_dukContext, script);
    BeAssert (s == 0);
    return BeJsValue (*this, PutDukGlobalStashProperty());
    }

BeJsObject BeJsContext::GetGlobalObject() const
    {
    duk_push_global_object (m_dukContext);
    return BeJsObject (*this, PutDukGlobalStashProperty());
    }

BeJsValue::BeJsValue (BeJsContextCR context)
    : m_context (&context), m_dukStashIndex (0)
    {
    }

BeJsValue::BeJsValue (BeJsValueCR other)
    : m_context (other.m_context), m_dukStashIndex (other.m_context->DuplicateDukGlobalStashProperty (other.m_dukStashIndex))
    {
    }

BeJsValueR BeJsValue::operator= (BeJsValueCR other)
    {
    if (this == &other)
        return *this;
        
    if (m_dukStashIndex != 0)
        {
        other.m_context->DeleteDukGlobalStashProperty (m_dukStashIndex);
        }

    m_context = other.m_context;
    m_dukStashIndex = m_context->DuplicateDukGlobalStashProperty (other.m_dukStashIndex);

    return *this;
    }

BeJsValue::BeJsValue (BeJsValue&& other)
    : m_context (other.m_context), m_dukStashIndex (other.m_dukStashIndex)
    {
    other.m_dukStashIndex = 0;
    }

BeJsValue::BeJsValue (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : m_context (&context), m_dukStashIndex (context.DuplicateDukGlobalStashProperty (dukStashIndex))
    {
    }

void BeJsValue::StoreDukValue (duk_idx_t stackIndex, bool popStackValue)
    {
    if (m_dukStashIndex == 0)
        m_dukStashIndex = m_context->PutDukGlobalStashProperty (stackIndex, popStackValue);
    else
        m_context->UpdateDukGlobalStashProperty (m_dukStashIndex, stackIndex, popStackValue);
    }
    
BeJsValue::~BeJsValue()
    {
    if (m_context == nullptr)
        return;

    if (m_dukStashIndex != 0)
        m_context->DeleteDukGlobalStashProperty (m_dukStashIndex);
    }

void BeJsValue::Reset()
    {
    if (m_dukStashIndex != 0)
        {
        m_context->DeleteDukGlobalStashProperty (m_dukStashIndex);
        m_dukStashIndex = 0;
        }
    }

void BeJsValue::NotifyEngineValueReleased()
    {
    m_dukStashIndex = 0;
    }

bool BeJsValue::IsNull() const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_is_null (ctx, -1);
    duk_pop (ctx);
    return s != 0;
    }

bool BeJsValue::IsUndefined() const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_is_undefined (ctx, -1);
    duk_pop (ctx);
    return s != 0;
    }

bool BeJsValue::IsNumber() const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_is_number (ctx, -1);
    duk_pop (ctx);
    return s != 0;
    }

bool BeJsValue::IsBoolean() const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_is_boolean (ctx, -1);
    duk_pop (ctx);
    return s != 0;
    }

bool BeJsValue::IsString() const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_is_string (ctx, -1);
    duk_pop (ctx);
    return s != 0;
    }

bool BeJsValue::IsObject() const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_is_object (ctx, -1);
    duk_pop (ctx);
    return s != 0;
    }

bool BeJsValue::IsFunction() const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_is_function (ctx, -1);
    duk_pop (ctx);
    return s != 0;
    }

bool BeJsValue::IsEqual (BeJsValueCR value) const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    m_context->GetDukGlobalStashProperty (value.GetDukStashIndex());
    duk_bool_t s = duk_equals (ctx, -1, -2);
    duk_pop_2 (ctx);
    return s != 0;
    }

bool BeJsValue::IsEqualStrict (BeJsValueCR value) const
    {
    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    m_context->GetDukGlobalStashProperty (value.GetDukStashIndex());
    duk_bool_t s = duk_strict_equals (ctx, -1, -2);
    duk_pop_2 (ctx);
    return s != 0;
    }

BeJsValue BeJsValue::Null (BeJsContextCR context)
    {
    duk_push_null (context.GetDukContext());
    return BeJsValue (context, context.PutDukGlobalStashProperty());
    }

BeJsValue BeJsValue::Undefined (BeJsContextCR context)
    {
    duk_push_undefined (context.GetDukContext());
    return BeJsValue (context, context.PutDukGlobalStashProperty());
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsPrimitive::BeJsPrimitive (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsValue (context, dukStashIndex)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, bool value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsBoolean::BeJsBoolean (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsPrimitive (context, dukStashIndex)
    {
    }

bool BeJsBoolean::GetValue() const
    {
    BeAssert (IsBoolean());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t v = duk_get_boolean (ctx, -1);
    duk_pop (ctx);
    return v != 0;
    }

void BeJsBoolean::SetValue (bool value)
    {
    duk_context* ctx = m_context->GetDukContext();
    duk_push_boolean (ctx, value);
    StoreDukValue();
    }

BeJsNumber::BeJsNumber (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, double value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, int32_t value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsNumber::BeJsNumber (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsPrimitive (context, dukStashIndex)
    {
    }

double BeJsNumber::GetValue() const
    {
    if (IsNull())
        return 0;

    BeAssert (IsNumber());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_double_t v = duk_get_number (ctx, -1);
    duk_pop (ctx);
    return v;
    }

int32_t BeJsNumber::GetIntegerValue() const
    {
    return static_cast<int32_t>(GetValue());
    }

void BeJsNumber::SetValue (double value)
    {
    duk_context* ctx = m_context->GetDukContext();
    duk_push_number (ctx, value);
    StoreDukValue();
    }

void BeJsNumber::SetValue (int32_t value)
    {
    SetValue (static_cast<double>(value));
    }

BeJsString::BeJsString (BeJsValueCR other)
    : BeJsPrimitive (other)
    {
    }

BeJsString::BeJsString (BeJsContextCR context, Utf8CP value)
    : BeJsPrimitive (context)
    {
    SetValue (value);
    }

BeJsString::BeJsString (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsPrimitive (context, dukStashIndex)
    {
    }

Utf8String BeJsString::GetValue() const
    {
    if (IsNull())
        return Utf8String();

    BeAssert (IsString());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    Utf8CP v = duk_get_string (ctx, -1);
    duk_pop (ctx);

    return Utf8String (v);
    }

void BeJsString::SetValue (Utf8CP value)
    {
    duk_context* ctx = m_context->GetDukContext();
    duk_push_string (ctx, value);
    StoreDukValue();
    }

int64_t BeJsString::GetValueAsInt64() const
    {
    Utf8String s = GetValue();
    int64_t v;
    sscanf (s.c_str(), "%lld", &v);

    return v;
    }

uint64_t BeJsString::GetValueAsUnsignedInt64() const
    {
    Utf8String s = GetValue();
    uint64_t v;
    sscanf (s.c_str(), "%llu", &v);

    return v;
    }

BeJsObject::BeJsObject (BeJsValueCR other)
    : BeJsValue (other)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context)
    : BeJsValue (context)
    {
    }

BeJsObject::BeJsObject (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsValue (context, dukStashIndex)
    {
    }

BeJsValue BeJsObject::GetProperty (Utf8CP name) const
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    
    duk_bool_t s = duk_get_prop_string (ctx, -1, name);
    BeAssert (s == 1);

    duk_uarridx_t v = m_context->PutDukGlobalStashProperty();
    duk_pop (ctx);

    return BeJsValue (*m_context, v);
    }

BeJsBoolean BeJsObject::GetBooleanProperty (Utf8CP name) const
    {
    return BeJsBoolean (GetProperty (name));
    }

BeJsNumber BeJsObject::GetNumberProperty (Utf8CP name) const
    {
    return BeJsNumber (GetProperty (name));
    }

BeJsString BeJsObject::GetStringProperty (Utf8CP name) const
    {
    return BeJsString (GetProperty (name));
    }

BeJsObject BeJsObject::GetObjectProperty (Utf8CP name) const
    {
    return BeJsObject (GetProperty (name));
    }

BeJsNativePointer BeJsObject::GetNativePointerProperty (Utf8CP name) const
    {
    return BeJsNativePointer (GetProperty (name));
    }

BeJsArray BeJsObject::GetArrayProperty (Utf8CP name) const
    {
    return BeJsArray (GetProperty (name));
    }

BeJsFunction BeJsObject::GetFunctionProperty (Utf8CP name) const
    {
    return BeJsFunction (GetProperty (name));
    }

void BeJsObject::SetProperty (Utf8CP name, BeJsValueCR value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    m_context->GetDukGlobalStashProperty (value.GetDukStashIndex());
    
    duk_bool_t s = duk_put_prop_string (ctx, -2, name);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsObject::SetBooleanProperty (Utf8CP name, bool value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_boolean (ctx, value);
    
    duk_bool_t s = duk_put_prop_string (ctx, -2, name);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, double value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_number (ctx, value);
    
    duk_bool_t s = duk_put_prop_string (ctx, -2, name);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsObject::SetNumberProperty (Utf8CP name, int32_t value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_number (ctx, value);
    
    duk_bool_t s = duk_put_prop_string (ctx, -2, name);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsObject::SetStringProperty (Utf8CP name, Utf8CP value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_string (ctx, value);
    
    duk_bool_t s = duk_put_prop_string (ctx, -2, name);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsObject::DeleteProperty (Utf8CP name)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    
    duk_bool_t s = duk_del_prop_string (ctx, -1, name);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

bool BeJsObject::DoesPropertyExist (Utf8CP name)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_has_prop_string (ctx, -1, name);
    duk_pop (ctx);
    return s != 0;
    }

BeJsObject BeJsObject::New (BeJsContextCR context)
    {
    duk_push_object (context.GetDukContext());
    return BeJsObject (context, context.PutDukGlobalStashProperty());
    }

BeJsArray::BeJsArray (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsArray::BeJsArray (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsObject (context, dukStashIndex)
    {
    }

BeJsValue BeJsArray::GetProperty (uint32_t index) const
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    
    duk_bool_t s = duk_get_prop_index (ctx, -1, index);
    BeAssert (s == 1);

    duk_uarridx_t v = m_context->PutDukGlobalStashProperty();
    duk_pop (ctx);

    return BeJsValue (*m_context, v);
    }

BeJsBoolean BeJsArray::GetBooleanProperty (uint32_t index) const
    {
    return BeJsBoolean (GetProperty (index));
    }

BeJsNumber BeJsArray::GetNumberProperty (uint32_t index) const
    {
    return BeJsNumber (GetProperty (index));
    }

BeJsString BeJsArray::GetStringProperty (uint32_t index) const
    {
    return BeJsString (GetProperty (index));
    }

BeJsObject BeJsArray::GetObjectProperty (uint32_t index) const
    {
    return BeJsObject (GetProperty (index));
    }

BeJsNativePointer BeJsArray::GetNativePointerProperty (uint32_t index) const
    {
    return BeJsNativePointer (GetProperty (index));
    }

BeJsArray BeJsArray::GetArrayProperty (uint32_t index) const
    {
    return BeJsArray (GetProperty (index));
    }

BeJsFunction BeJsArray::GetFunctionProperty (uint32_t index) const
    {
    return BeJsFunction (GetProperty (index));
    }

void BeJsArray::SetProperty (uint32_t index, BeJsValueCR value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    m_context->GetDukGlobalStashProperty (value.GetDukStashIndex());
    
    duk_bool_t s = duk_put_prop_index (ctx, -2, index);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsArray::SetBooleanProperty (uint32_t index, bool value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_boolean (ctx, value);
    
    duk_bool_t s = duk_put_prop_index (ctx, -2, index);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsArray::SetNumberProperty (uint32_t index, double value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_number (ctx, value);
    
    duk_bool_t s = duk_put_prop_index (ctx, -2, index);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsArray::SetNumberProperty (uint32_t index, int32_t value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_number (ctx, value);
    
    duk_bool_t s = duk_put_prop_index (ctx, -2, index);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsArray::SetStringProperty (uint32_t index, Utf8CP value)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_push_string (ctx, value);
    
    duk_bool_t s = duk_put_prop_index (ctx, -2, index);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

void BeJsArray::DeleteProperty (uint32_t index)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    
    duk_bool_t s = duk_del_prop_index (ctx, -1, index);
    BeAssert (s == 1);

    duk_pop (ctx);
    }

bool BeJsArray::DoesPropertyExist (uint32_t index)
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_bool_t s = duk_has_prop_index (ctx, -1, index);
    duk_pop (ctx);
    return s != 0;
    }

uint32_t BeJsArray::GetLength() const
    {
    BeAssert (IsObject());

    duk_context* ctx = m_context->GetDukContext();
    m_context->GetDukGlobalStashProperty (m_dukStashIndex);
    duk_size_t length = duk_get_length (ctx, -1);
    duk_pop (ctx);
    return static_cast<uint32_t>(length);
    }

BeJsArray BeJsArray::New (BeJsContextCR context)
    {
    duk_push_array (context.GetDukContext());
    return BeJsArray (context, context.PutDukGlobalStashProperty());
    }

BeJsNativePointer::BeJsNativePointer (BeJsValueCR other)
    : BeJsObject (other),
      m_environment (&other.GetContext().GetEnvironment())
    {
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, void* object, DisposeCallback disposeCallback, BeJsObject const* prototype, bool suppressCallContext)
    : BeJsObject (context),
      m_environment (&context.GetEnvironment())
    {
    _BeJsNativePointer* external = new _BeJsNativePointer (context, object, disposeCallback);

    duk_context* ctx = context.GetDukContext();
    duk_push_object (ctx);
    duk_push_pointer (ctx, external);
    duk_bool_t s = duk_put_prop_string (ctx, -2, s_dukDataPropertyName);
    BeAssert (s == 1);
    duk_push_c_function (ctx, &_BeJsNativePointer::Dispose, 1);
    duk_set_finalizer (ctx, -2);
    StoreDukValue();

    if (context.m_callContextActive && !suppressCallContext)
        context.m_callContextValues.push_back (*this);
    }

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsObject (context, dukStashIndex),
      m_environment (&context.GetEnvironment())
    {
    }

void* BeJsNativePointer::GetValue() const
    {
    if (IsNull())
        return nullptr;

    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (*m_context, m_dukStashIndex);
    return wrapper->m_object;
    }

void BeJsNativePointer::Dispose()
    {
    if (IsNull())
        return;

    _BeJsNativePointer* wrapper = static_cast<_BeJsNativePointer*>(_BeJsNativePointer::GetFromHandle (*m_context, m_dukStashIndex));

    if (wrapper->m_callback != nullptr)
        wrapper->m_callback (wrapper->m_object, this);

    wrapper->m_object = nullptr;
    }

void BeJsNativePointer::SetValue (void* object)
    {
    _BeJsNativeObjectWrapper* wrapper = _BeJsNativePointer::GetFromHandle (*m_context, m_dukStashIndex);
    wrapper->m_object = object;
    }

BeJsFunction::BeJsFunction (BeJsValueCR other)
    : BeJsObject (other)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context)
    : BeJsObject (context)
    {
    }

BeJsFunction::BeJsFunction (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsObject (context, dukStashIndex)
    {
    }

BeJsValue BeJsFunction::Call (BeJsValue** args,
                                              BeJsEnvironment::EngineValueHandleType* engineArgs,
                                              size_t argumentCount,
                                              BeJsObject* callScope,
                                              bool callAsConstructor) const
    {
    duk_bool_t s;
    duk_idx_t o = -1;

    BeAssert (IsFunction());
    
    duk_context* ctx = m_context->GetDukContext();

    duk_push_global_stash (ctx);

    s = duk_get_prop_index (ctx, o, m_dukStashIndex);
    BeAssert (s == 1);
    --o;

    if (!callAsConstructor)
        {
        if (callScope != nullptr)
            {
            s = duk_get_prop_index (ctx, o, callScope->GetDukStashIndex());
            BeAssert (s == 1);
            }
        else
            duk_push_null (ctx);

        --o;
        }

    if (argumentCount != 0)
        {
        for (size_t i = 0; i != argumentCount; ++i)
            {
            s = duk_get_prop_index (ctx, o, args [i]->GetDukStashIndex());
            BeAssert (s == 1);
            --o;
            }
        }

    if (callAsConstructor)
        duk_new (ctx, static_cast<duk_idx_t>(argumentCount));
    else
        duk_call_method (ctx, static_cast<duk_idx_t>(argumentCount));

    duk_uarridx_t v = m_context->GetNextGlobalStashIndex();
    s = duk_put_prop_index (ctx, -2, v);
    BeAssert (s == 1);

    duk_pop (ctx);

    return BeJsValue (*m_context, v);
    }

template <typename T> void BeJsNativeFunction::Init(BeJsContextCR context, T callback)
    {
    _BeJsNativeFunction<T>* proxy = new _BeJsNativeFunction<T>(context, callback);
    m_removeCallbackFunction = [proxy]() -> void {if (proxy) { _BeJsNativeFunction<T>::RemoveNativeCallback(proxy); }};

    duk_context* ctx = m_context->GetDukContext();
    duk_push_c_function(ctx, &DukCallback<T>, DUK_VARARGS);
    duk_push_pointer(ctx, proxy);
    duk_bool_t s = duk_put_prop_string(ctx, -2, s_dukDataPropertyName);
    BeAssert(s == 1);
    duk_push_c_function(ctx, &_BeJsNativeFunction<T>::Dispose, DUK_VARARGS);
    duk_set_finalizer(ctx, -2);
    StoreDukValue();
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, Callback callback)
    : BeJsFunction (context)
    {
    Init(context, callback);
    }

BeJsNativeFunction(BeJsContextCR context, std::function<BeJsValue(CallbackInfoR info)> callback)
    {
    Init(context, callback);
    }

BeJsNativeFunction::BeJsNativeFunction (BeJsContextCR context, duk_uarridx_t dukStashIndex)
    : BeJsFunction (context, dukStashIndex)
    {
    }

template <typename T> duk_ret_t BeJsNativeFunction::DukCallback (duk_context* ctx)
    {
    duk_push_current_function (ctx);

    duk_bool_t s = duk_get_prop_string (ctx, -1, s_dukDataPropertyName);
    BeAssert (s == 1);
    void* object = duk_get_pointer (ctx, -1);
    duk_pop (ctx);

    _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
    CallbackInfo info (*proxy->m_context, duk_is_constructor_call (ctx) != 0, duk_get_top (ctx) - 1);
    
    duk_push_this (ctx);
    
    duk_ret_t response = proxy->Callback (info);

    if (response == 1)
        {
        duk_replace (ctx, -2);
        duk_replace (ctx, -2);
        }
    else
        {
        duk_pop_2 (ctx);
        }

    return response;
    }

BeJsNativeFunction::CallbackInfo::CallbackInfo (BeJsContextCR context, bool isConstructCall, uint32_t argumentCount)
    : m_context (&context), m_isConstructCall (isConstructCall), m_argumentCount (argumentCount)
    {
    }

BeJsNativeFunction BeJsNativeFunction::CallbackInfo::GetFunction()
    {
    return BeJsNativeFunction (*m_context, m_context->PutDukGlobalStashProperty (1 + m_argumentCount, false));
    }

BeJsContextCR BeJsNativeFunction::CallbackInfo::GetContext()
    {
    return *m_context;
    }

bool BeJsNativeFunction::CallbackInfo::IsConstructCall()
    {
    return m_isConstructCall;
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetCallContext()
    {
    return BeJsObject (*m_context, m_context->PutDukGlobalStashProperty (1 + m_argumentCount + 1, false));
    }

uint32_t BeJsNativeFunction::CallbackInfo::GetArgumentCount() const
    {
    return m_argumentCount;
    }

BeJsValue BeJsNativeFunction::CallbackInfo::GetArgument (uint32_t index) const
    {
    return BeJsValue (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsBoolean BeJsNativeFunction::CallbackInfo::GetBooleanArgument (uint32_t index) const
    {
    return BeJsBoolean (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsNumber BeJsNativeFunction::CallbackInfo::GetNumberArgument (uint32_t index) const
    {
    return BeJsNumber (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsString BeJsNativeFunction::CallbackInfo::GetStringArgument (uint32_t index) const
    {
    return BeJsString (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsObject BeJsNativeFunction::CallbackInfo::GetObjectArgument (uint32_t index) const
    {
    return BeJsObject (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsNativePointer BeJsNativeFunction::CallbackInfo::GetNativePointerArgument (uint32_t index) const
    {
    return BeJsNativePointer (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsArray BeJsNativeFunction::CallbackInfo::GetArrayArgument (uint32_t index) const
    {
    return BeJsArray (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsFunction BeJsNativeFunction::CallbackInfo::GetFunctionArgument (uint32_t index) const
    {
    return BeJsFunction (*m_context, m_context->PutDukGlobalStashProperty (1 + index, false));
    }

BeJsNativeObject::Callbacks::Callbacks (GetProperty getter,
                                               SetProperty setter,
                                               DeleteProperty deleter,
                                               EnumerateProperties enumerator,
                                               DoesPropertyExist query,
                                               BeJsNativePointer::DisposeCallback disposer)
    : getProperty (getter), setProperty (setter), deleteProperty (deleter), enumerateProperties (enumerator), doesPropertyExist (query), dispose (disposer)
    {
    }

BeJsNativeObject::BeJsNativeObject (BeJsContextCR context, Callbacks const& callbacks, void* object)
    : BeJsObject (context)
    {
    _BeJsNativeObject* proxy = new _BeJsNativeObject (context, callbacks, object);

    BeJsNativePointer target (context, proxy, &_BeJsNativeObject::Dispose);

    BeJsObject handler = BeJsObject::New (context);

    handler.SetProperty ("get", *context.m_proxyObjectGetter);

    if (callbacks.setProperty != nullptr)
        handler.SetProperty ("set", *context.m_proxyObjectSetter);
        
    if (callbacks.doesPropertyExist != nullptr)
        handler.SetProperty ("has", *context.m_proxyObjectQuery);
        
    if (callbacks.deleteProperty != nullptr)
        handler.SetProperty ("deleteProperty", *context.m_proxyObjectDeleter);
        
    if (callbacks.enumerateProperties != nullptr)
        handler.SetProperty ("ownKeys", *context.m_proxyObjectEnumerator);

    BeJsObject jsValue = context.m_ES6_Proxy->CallAsConstructor (target, handler);
    m_context->GetDukGlobalStashProperty (jsValue.GetDukStashIndex());
    StoreDukValue();
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     07/2017
//---------------------------------------------------------------------------------------
void BeJsEnvironment::InitializeEngine()
    {
#if defined (BEJAVASCRIPT_V8_ENGINE)
    static std::once_flag s_v8Init;
    std::call_once(s_v8Init, []()
        {
        v8::V8::InitializePlatform(v8::platform::CreateDefaultPlatform());
        v8::V8::Initialize();
        });       
#endif
    }
