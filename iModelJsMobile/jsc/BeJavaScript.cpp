/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BeJavaScript.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    2/15
//---------------------------------------------------------------------------------------
BeJsContextR BeJsContext::GetJsContext()
    {
    return *this;
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
    BeJsNativePointer::DisposeCallback m_disposeCallback;

    _BeJsNativePointer (BeJsContextCR context, void* object, BeJsNativePointer::DisposeCallback dcallback)
        : _BeJsNativeObjectWrapper (object), m_disposeCallback (dcallback)
        {
        };

    ~_BeJsNativePointer()
        {
        };

    static void Dispose (JSObjectRef object)
        {
        _BeJsNativePointer* external = static_cast<_BeJsNativePointer*>(JSObjectGetPrivate (object));

        if (external->m_disposeCallback != nullptr)
            external->m_disposeCallback (external->m_object, nullptr);

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

    JSValueRef InvokeCallback (BeJsNativeFunction::CallbackInfoR info)
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

BeJsEnvironment::BeJsEnvironment()
    : m_destroying (false),
      m_ownsSystemEnvironment (true)
    {
    m_jscContextGroup = JSContextGroupCreate();
    }

BeJsEnvironment::BeJsEnvironment (JSContextGroupRef systemEnvironment)
    : m_destroying (false),
      m_ownsSystemEnvironment (false)
    {
    BeAssert (engine == GetActiveEngine());

    m_ownsSystemEnvironment = false;

    m_jscContextGroup = systemEnvironment;
    JSContextGroupRetain (m_jscContextGroup);
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
    }

BeJsContext::BeJsContext (BeJsEnvironmentCR environment, JSGlobalContextRef systemContext)
    : m_environment (&environment)
    {
    BeAssert (engine == BeJsEnvironment::GetActiveEngine());

    m_jscContext = systemContext;
    JSGlobalContextRetain (m_jscContext);

    FindLanguageObjects();
    }

void BeJsContext::FindLanguageObjects()
    {
    auto languageFunction = JSValueToObject (m_jscContext, GetGlobalObject().GetProperty ("Function").GetJscValue(), NULL);
    auto functionPrototype = JSObjectGetPrototype (m_jscContext, languageFunction);
    m_Function_prototype = new BeJsValue (*this, functionPrototype);
    }

BeJsContext::~BeJsContext()
    {
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
                // *** TODO BeJsString trace = exceptionObject.GetStringProperty ("stack");
BeJsString trace(*this, "TBD");
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

/*
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
    */

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

BeJsNativePointer::BeJsNativePointer (BeJsContextCR context, void* object, DisposeCallback disposeCallback, BeJsObject const* prototype)
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

    if (wrapper->m_disposeCallback != nullptr)
        wrapper->m_disposeCallback (wrapper->m_object, this);

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
                                              JSValueRef* engineArgs,
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
    definition.callAsFunction = &InvokeJscCallback<T>;
    definition.callAsConstructor = &InvokeJscCallbackConstructor<T>;
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

template <typename T> JSValueRef BeJsNativeFunction::InvokeJscCallback (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
    {
    void* object = JSObjectGetPrivate (function);
    _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
    CallbackInfo info (*proxy->m_context, function, thisObject, argumentCount, arguments);
    return proxy->InvokeCallback (info);
    }

template <typename T> JSObjectRef BeJsNativeFunction::InvokeJscCallbackConstructor (JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
    {
    void* object = JSObjectGetPrivate (constructor);
    _BeJsNativeFunction<T>* proxy = static_cast<_BeJsNativeFunction<T>*>(object);
    CallbackInfo info (*proxy->m_context, constructor, NULL, argumentCount, arguments);
    
    JSValueRef obj = proxy->InvokeCallback (info);
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

