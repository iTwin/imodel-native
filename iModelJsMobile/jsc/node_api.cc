
// #include <node_buffer.h>
// #include <node_object_wrap.h>
#include <limits.h>  // INT_MAX
#include <string.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <Napi/node_api.h>
#include "../node_internals.h"
#include <string>
#include <Bentley/BeAssert.h>
#undef min

#include <JavaScriptCore/JavaScriptCore.h>


#define RETURN_STATUS_IF_FALSE(env, condition, status)                  \
  do {                                                                  \
    if (!(condition)) {                                                 \
      return napi_set_last_error((env), (status));                      \
    }                                                                   \
  } while (0)

#define CHECK_ENV(env)        \
  if ((env) == nullptr) {     \
    return napi_invalid_arg;  \
  }

#define CHECK_ARG(env, arg) \
  RETURN_STATUS_IF_FALSE((env), ((arg) != nullptr), napi_invalid_arg)

#define NAPI_PREAMBLE(env)                                       \
  CHECK_ENV((env));                                              \
  RETURN_STATUS_IF_FALSE((env), !(env)->IsExceptionPending(), \
                         napi_pending_exception);                \
  napi_clear_last_error((env));     


#define GET_RETURN_STATUS(env)      \
  (!(env)->IsExceptionPending() ? napi_ok \
                         : napi_set_last_error((env), napi_pending_exception))

static
const char* error_messages[] = {nullptr,
                                "Invalid argument",
                                "An object was expected",
                                "A string was expected",
                                "A string or symbol was expected",
                                "A function was expected",
                                "A number was expected",
                                "A boolean was expected",
                                "An array was expected",
                                "Unknown failure",
                                "An exception is pending",
                                "The async work item was cancelled",
                                "napi_escape_handle already called on scope"};

//=======================================================================================
// @bsiclass                                    Satyakam.Khadilkar    03/2018
//=======================================================================================
struct JSCCallbackData {
public:
    explicit JSCCallbackData(napi_env environment, void* callbackData)
    : _env(environment), _data(callbackData) {}
    napi_env Env() {return _env;}
    void* Data() { return _data; }
protected:
    napi_env _env;
     void* _data;

};

//=======================================================================================
// @bsiclass                                    Satyakam.Khadilkar    03/2018
//=======================================================================================
struct JSCFunctionCallbackData : JSCCallbackData {
public:
    JSCFunctionCallbackData(napi_env environment, napi_callback callback, void* callbackData)
    : JSCCallbackData(environment, callbackData), _callback(callback) {}

    JSValueRef Callback(napi_callback_info info) {return _callback(_env,info);}
private:
    napi_callback _callback;
};

//=======================================================================================
// @bsiclass                                    Satyakam.Khadilkar    03/2018
//=======================================================================================
struct JSClassCallbackData : JSCFunctionCallbackData {
public:
    JSClassCallbackData(napi_env environment, napi_callback callback, void* callbackData,
                        JSObjectRef prototype) : JSCFunctionCallbackData(environment,callback,callbackData),
    _prototype(prototype){}
    
    JSObjectRef Prototype() { return _prototype;}
private:
    JSObjectRef _prototype;
};

//=======================================================================================
// @bsiclass                                    Affan.Khan                   06/2019
//=======================================================================================
struct JSCCallbackWrapper {
    JSCCallbackWrapper(JSContextRef context, JSCCallbackData* data, JSObjectRef thisObj, size_t argC, const JSValueRef argV[])
    :_context(context), _data(data), _this(thisObj), _argn(argC), _argv(argV){}
    
    void* Data() { return _data ? _data->Data() : nullptr; }
    size_t ArgsLength() { return _argn;}
    JSObjectRef This() { return _this;}
    void Args(napi_value* buffer, size_t buffer_length) {
        size_t i = 0;
        size_t min = std::min(buffer_length, _argn);
        
        for (; i < min; i += 1) {
            buffer[i] = _argv[i];
        }
        
        if (i < buffer_length) {
            napi_value undefined = JSValueMakeUndefined(_context);
            for (; i < buffer_length; i += 1) {
                buffer[i] = undefined;
            }
        }
    }
    JSContextRef GetContext() const {return _context;}
    template<class T>
    T* GetCbData() const {return reinterpret_cast<T*>(_data);}
private:
    JSContextRef _context;
    size_t _argn;
    const JSValueRef* _argv;
    JSObjectRef _this;
    JSCCallbackData* _data;
};
//=======================================================================================
// @bsiclass                                    Affan.Khan                   06/2019
//=======================================================================================
struct wrapped_object {
    napi_env env;
    void* native_object;
    napi_finalize finalize_cb;
    void* finalize_hint;
};

//=======================================================================================
// @bsiclass                                    Affan.Khan                   06/2019
//=======================================================================================
struct JSCAccessorCallbackData : JSCCallbackData {
public:
    JSCAccessorCallbackData(napi_env environment, napi_callback getCb, napi_callback setCb, void* callbackData)
    : JSCCallbackData(environment, callbackData), _getCb(getCb), _setCb(setCb) {}
    JSValueRef CallbackGet(napi_callback_info info) {return _getCb(_env,info);}
    void CallbackSet(napi_callback_info info) { _setCb(_env,info); }
private:
    napi_callback _getCb;
    napi_callback _setCb;
};

//=======================================================================================
// @bsiclass                                    Affan.Khan                   06/2019
//=======================================================================================
struct JSCAccessorCallbackWrapper : JSCCallbackWrapper {
public:
    JSCAccessorCallbackWrapper(JSContextRef context, JSCAccessorCallbackData* data, JSObjectRef thisObj, JSValueRef* value)
    :JSCCallbackWrapper(context, data, thisObj, value ? 1 : 0, value) {}
    JSValueRef InvokeCallbackGet() {
        return GetCbData<JSCAccessorCallbackData>()->CallbackGet((napi_callback_info)this);

    }
    void InvokeCallbackSet() {
        GetCbData<JSCAccessorCallbackData>()->CallbackSet((napi_callback_info)this);
    }

    static JSStringRef CreateNativeAccessorName(JSStringRef orignalProperty) {
        std::string str = "___ios_accessor_native_" + ToUtf8(orignalProperty);
        return FromUtf8(str);
    }
    static JSStringRef CreateNativeAccessorNameUtf8(Utf8CP orignalProperty) {
        std::string str = "___ios_accessor_native_" + std::string(orignalProperty);
        return FromUtf8(str);
    }
    
    static bool HasPropertyCallback(JSContextRef ctx, JSObjectRef object, JSStringRef propertyName){
        return true;
    }
    static void GetPropertyNamesCallback(JSContextRef ctx, JSObjectRef object, JSPropertyNameAccumulatorRef propertyNames) {
    }
    // (JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef *exception);
    static JSValueRef CallAsGetProperty (JSContextRef ctx, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception){

        JSStringRef sysProp = CreateNativeAccessorName(propertyName);
        JSValueRef sysPropVal = JSObjectGetProperty(ctx, thisObject, sysProp, nullptr);
        JSObjectRef sysPropObj = JSValueToObject(ctx, sysPropVal, nullptr);
        auto funcCBData = reinterpret_cast<JSCAccessorCallbackData*>(JSObjectGetPrivate(sysPropObj));
        JSStringRelease(sysProp);
        JSCAccessorCallbackWrapper cbwrapper(ctx, funcCBData, thisObject, nullptr);
        return cbwrapper.InvokeCallbackGet();
    }
    // (JSContextRef ctx, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef *exception);
    static bool CallAsSetProperty (JSContextRef ctx, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception) {
     
        JSStringRef sysProp = CreateNativeAccessorName(propertyName);
        JSValueRef sysPropVal = JSObjectGetProperty(ctx, thisObject, sysProp, nullptr);
        JSObjectRef sysPropObj = JSValueToObject(ctx, sysPropVal, nullptr);
        auto funcCBData = reinterpret_cast<JSCAccessorCallbackData*>(JSObjectGetPrivate(sysPropObj));
        JSStringRelease(sysProp);
        JSCAccessorCallbackWrapper cbwrapper(ctx, funcCBData, thisObject, &value);
        cbwrapper.InvokeCallbackSet();
        return true;
    }
private:
    static JSStringRef FromUtf8(std::string string) {
        return JSStringCreateWithUTF8CString(string.c_str());
    }
    static std::string ToUtf8(JSStringRef string) {
        const size_t sz = JSStringGetMaximumUTF8CStringSize(string);
        std::string str;
        str.resize (sz);
        JSStringGetUTF8CString(string, const_cast<char*>(str.c_str()), sz);
        return str;
    }
};

//=======================================================================================
// @bsiclass                                    Satyakam.Khadilkar    03/2018
//=======================================================================================
struct JSCFunctionCallbackWrapper : JSCCallbackWrapper{
public:
    JSCFunctionCallbackWrapper(JSContextRef context, JSCFunctionCallbackData* data, JSObjectRef thisObj, size_t argC, const JSValueRef argV[], bool isConstructor)
    : JSCCallbackWrapper(context, data, thisObj, argC, argV), _isConstructor(isConstructor) {}
    
    JSValueRef InvokeCallback() {
        return GetCbData<JSCFunctionCallbackData>()->Callback((napi_callback_info)this);
    }
    

    bool IsConstructor() { return _isConstructor; }
    
    static JSValueRef CallAsFunction (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
                              size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception){
        auto funcCBData = reinterpret_cast<JSCFunctionCallbackData*>(JSObjectGetPrivate(function));
        JSCFunctionCallbackWrapper cbwrapper(ctx, funcCBData, thisObject,
                                             argumentCount,arguments,false);
        return cbwrapper.InvokeCallback();
    }

    static JSObjectRef CallAsConstructor (JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception){
        auto classCBData = reinterpret_cast<JSClassCallbackData*>(JSObjectGetPrivate(constructor));
        JSClassDefinition classDef = kJSClassDefinitionEmpty;
        classDef.finalize = JSCFunctionCallbackWrapper::FinalizeObject;
        JSClassRef classRef = JSClassCreate(&classDef);
        JSObjectRef thisObject = JSObjectMake(ctx, classRef, nullptr);
        JSClassRelease(classRef);
        JSObjectSetPrototype(ctx, thisObject, classCBData->Prototype());
        JSCFunctionCallbackWrapper cbwrapper(ctx, classCBData, thisObject,
                                             argumentCount,arguments,true);
        return (JSObjectRef)cbwrapper.InvokeCallback();
    }
    
    static void FinalizeObject(JSObjectRef object) {
        wrapped_object* wrap =  (wrapped_object*)JSObjectGetPrivate(object);
        if (wrap) {
            if (wrap->finalize_cb) {
                wrap->finalize_cb(wrap->env, wrap->native_object, wrap->finalize_hint);
            }
            delete wrap;
        }
    }
    static bool HasInstance (JSContextRef ctx, JSObjectRef constructor, JSValueRef possibleInstance, JSValueRef* exception) {
        auto classCBData = reinterpret_cast<JSClassCallbackData*>(JSObjectGetPrivate(constructor));
        auto instanceProto = JSObjectGetPrototype(ctx, (JSObjectRef)possibleInstance);
        return JSValueIsStrictEqual(ctx, instanceProto, classCBData->Prototype());
    }
    
private:
    bool _isConstructor;
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_clear_last_error(napi_env env) {
  env->last_error.error_code = napi_ok;

  // TODO(boingoing): Should this be a callback?
  env->last_error.engine_error_code = 0;
  env->last_error.engine_reserved = nullptr;
  return napi_ok;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_set_last_error(napi_env env, napi_status error_code,
                                uint32_t engine_error_code,
                                void* engine_reserved) {
  env->last_error.error_code = error_code;
  env->last_error.engine_error_code = engine_error_code;
  env->last_error.engine_reserved = engine_reserved;
  return error_code;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_last_error_info(napi_env env,
                                     const napi_extended_error_info** result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  // you must update this assert to reference the last message
  // in the napi_status enum each time a new error message is added.
  // We don't have a napi_status_last as this would result in an ABI
  // change each time a message was added.
  static_assert(
      node::arraysize(error_messages) == napi_escape_called_twice + 1,
      "Count of error messages must match count of error values");
  CHECK_LE(env->last_error.error_code, napi_escape_called_twice);

  // Wait until someone requests the last error information to fetch the error
  // message string
  env->last_error.error_message =
      error_messages[env->last_error.error_code];

  *result = &(env->last_error);
  return napi_ok;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
NAPI_NO_RETURN void napi_fatal_error(const char* location,
                                     size_t location_len,
                                     const char* message,
                                     size_t message_len) {
  std::string location_string;
  std::string message_string;

  if (location_len != NAPI_AUTO_LENGTH) {
    location_string.assign(
        const_cast<char*>(location), location_len);
  } else {
    location_string.assign(
        const_cast<char*>(location), strlen(location));
  }

  if (message_len != NAPI_AUTO_LENGTH) {
    message_string.assign(
        const_cast<char*>(message), message_len);
  } else {
    message_string.assign(
        const_cast<char*>(message), strlen(message));
  }

  node::FatalError(location_string.c_str(), message_string.c_str());
}

void FinalizeFunctionCallback(JSObjectRef function) {
    auto funcCBData = (JSCCallbackData*)JSObjectGetPrivate(function);
    if (funcCBData) {
        delete funcCBData;
    }
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_function(napi_env env,
                                 const char* utf8name,
                                 size_t length,
                                 napi_callback cb,
                                 void* callback_data,
                                 napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    CHECK_ARG(env, cb);

    JSContextRef ctx = env->GetContext();
    
    JSClassDefinition classDef = kJSClassDefinitionEmpty;
    if (utf8name != nullptr) {
        classDef.className = utf8name;
    }
    classDef.finalize = FinalizeFunctionCallback;
    classDef.callAsFunction = JSCFunctionCallbackWrapper::CallAsFunction;
    JSClassRef classRef = JSClassCreate(&classDef);
    auto funcCBData = new JSCFunctionCallbackData(env,cb,callback_data);
    *result = JSObjectMake(ctx,classRef,funcCBData);
    return GET_RETURN_STATUS(env);
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_define_class(napi_env env,
                              const char* utf8name,
                              size_t length,
                              napi_callback constructor,
                              void* callback_data,
                              size_t property_count,
                              const napi_property_descriptor* properties,
                              napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    CHECK_ARG(env, constructor);

    JSContextRef ctx = env->GetContext();
    
    JSObjectRef prototypeObj = JSObjectMake(ctx, NULL, NULL);
    JSValueProtect(ctx, prototypeObj);
    // Separate static and instance properties;
    size_t static_property_count = 0;
    size_t instance_property_count = 0;
    for (size_t i=0; i < property_count; i++) {
        const napi_property_descriptor* p = properties + i;
        if ((p->attributes & napi_static) == 0) {
            instance_property_count++;
        } else {
            static_property_count++;
        }
    }

    // Add Instance Properties on Prototype Object Here.
    if (instance_property_count > 0) {
        auto instance_properties = new napi_property_descriptor[instance_property_count];
        size_t instance_property_index = 0;
        for (size_t i=0; i < property_count; i++) {
            const napi_property_descriptor* p = properties + i;
            if ((p->attributes & napi_static) == 0) {
                instance_properties[instance_property_index] = *p;
                instance_property_index++;
            }
        }
        napi_define_properties(env, prototypeObj, instance_property_count, instance_properties);
        delete [] instance_properties;
    }
    
    JSClassDefinition classDef = kJSClassDefinitionEmpty;
    classDef.className = utf8name;
    classDef.callAsConstructor = JSCFunctionCallbackWrapper::CallAsConstructor;
    classDef.hasInstance = JSCFunctionCallbackWrapper::HasInstance;
    classDef.finalize = FinalizeFunctionCallback;
    JSClassRef classRef = JSClassCreate(&classDef);
    auto classCBData = new JSClassCallbackData(env,constructor,callback_data,prototypeObj);
    *result = JSObjectMake(ctx,classRef,classCBData);
    JSClassRelease(classRef);
    
    // Add Static Properties to Result Object Here.
    if (static_property_count > 0) {
        auto static_properties = new napi_property_descriptor[static_property_count];
        size_t static_property_index = 0;
        for (size_t i=0; i < property_count; i++) {
            const napi_property_descriptor* p = properties + i;
            if ((p->attributes & napi_static) != 0) {
                static_properties[static_property_index] = *p;
                static_property_index++;
            }
        }
        napi_define_properties(env, *result, static_property_count, static_properties);
        delete [] static_properties;
    }

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_property_names(napi_env env,
                                    napi_value object,
                                    napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSPropertyNameArrayRef properties = JSObjectCopyPropertyNames(ctx, (JSObjectRef)object);
    size_t count = JSPropertyNameArrayGetCount(properties);
    JSObjectRef rs = JSObjectMakeArray(ctx, 0, nullptr, nullptr);
    for (size_t i = 0; i < count; i++) {
        JSStringRef propertyString = JSPropertyNameArrayGetNameAtIndex(properties, i);
        JSValueRef val = JSValueMakeString(ctx, propertyString);
        JSObjectSetPropertyAtIndex(ctx, rs, i, val, nullptr);
        JSStringRelease(propertyString);
    }
    *result = rs;
    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_set_property(napi_env env,
                              napi_value object,
                              napi_value key,
                              napi_value value) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, key);
    CHECK_ARG(env, value);

    JSContextRef ctx = env->GetContext();
    JSStringRef keyString = JSValueToStringCopy (ctx, key, NULL);
    JSObjectSetProperty (ctx, (JSObjectRef)object, keyString, value, kJSPropertyAttributeNone, NULL);
    JSStringRelease(keyString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_has_property(napi_env env,
                              napi_value object,
                              napi_value key,
                              bool* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);
    CHECK_ARG(env, key);

    JSContextRef ctx = env->GetContext();
    JSStringRef keyString = JSValueToStringCopy (ctx, key, NULL);
    *result = JSObjectHasProperty(ctx, (JSObjectRef)object, keyString);
    JSStringRelease(keyString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_property(napi_env env,
                              napi_value object,
                              napi_value key,
                              napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, key);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSStringRef keyString = JSValueToStringCopy (ctx, key, NULL);
    *result = JSObjectGetProperty(ctx, (JSObjectRef)object, keyString, NULL);
    JSStringRelease(keyString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_delete_property(napi_env env,
                                 napi_value object,
                                 napi_value key,
                                 bool* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, key);

    JSContextRef ctx = env->GetContext();
    JSStringRef keyString = JSValueToStringCopy (ctx, key, NULL);
    *result = JSObjectDeleteProperty(ctx, (JSObjectRef)object, keyString, NULL);
    JSStringRelease(keyString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_has_own_property(napi_env env,
                                  napi_value object,
                                  napi_value key,
                                  bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, key);

//  JSContextRef ctx = env->GetContext();

  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_set_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    napi_value value) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);

    JSContextRef ctx = env->GetContext();
    JSStringRef keyString = JSStringCreateWithUTF8CString (utf8name);
    JSObjectSetProperty (ctx, (JSObjectRef)object, keyString, value, kJSPropertyAttributeNone, NULL);
    JSStringRelease(keyString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_has_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    bool* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSStringRef keyString = JSStringCreateWithUTF8CString (utf8name);
    *result = JSObjectHasProperty(ctx, (JSObjectRef)object, keyString);
    JSStringRelease(keyString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSStringRef keyString = JSStringCreateWithUTF8CString (utf8name);
    *result = JSObjectGetProperty(ctx, (JSObjectRef)object, keyString, NULL);
    JSStringRelease(keyString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_set_element(napi_env env,
                             napi_value object,
                             uint32_t index,
                             napi_value value) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);

    JSContextRef ctx = env->GetContext();
    JSObjectSetPropertyAtIndex (ctx, (JSObjectRef)object, index, value, NULL);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_has_element(napi_env env,
                             napi_value object,
                             uint32_t index,
                             bool* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSPropertyNameArrayRef propertiesArray = JSObjectCopyPropertyNames(ctx, (JSObjectRef)object);
    size_t count = JSPropertyNameArrayGetCount(propertiesArray);
    JSPropertyNameArrayRelease(propertiesArray);
    *result = (index < count);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_element(napi_env env,
                             napi_value object,
                             uint32_t index,
                             napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSObjectGetPropertyAtIndex(ctx, (JSObjectRef)object, index, NULL);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_delete_element(napi_env env,
                                napi_value object,
                                uint32_t index,
                                bool* result) {
    NAPI_PREAMBLE(env);

    JSContextRef ctx = env->GetContext();
    JSPropertyNameArrayRef propertiesArray = JSObjectCopyPropertyNames(ctx, (JSObjectRef)object);
    size_t count = JSPropertyNameArrayGetCount(propertiesArray);
    if (index < count) {
        JSStringRef propertyString = JSPropertyNameArrayGetNameAtIndex(propertiesArray, index);
        *result = JSObjectDeleteProperty(ctx, (JSObjectRef)object, propertyString, NULL);
        JSStringRelease(propertyString);
    } else {
        *result = false;
    }
    JSPropertyNameArrayRelease(propertiesArray);
    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_define_properties(napi_env env,
                                   napi_value object,
                                   size_t property_count,
                                   const napi_property_descriptor* properties) {
    NAPI_PREAMBLE(env);
    if (property_count > 0) {
    CHECK_ARG(env, properties);
    }
 
    JSContextRef ctx = env->GetContext();
    for (size_t i=0; i < property_count; i++) {
        const napi_property_descriptor* p = properties + i;
        if (p->utf8name != nullptr) {
            if (p->getter != nullptr || p->setter != nullptr) {
                
                JSClassDefinition accessorDef = kJSClassDefinitionEmpty;
                accessorDef.className = p->utf8name;
                accessorDef.getProperty = JSCAccessorCallbackWrapper::CallAsGetProperty;
                accessorDef.setProperty = JSCAccessorCallbackWrapper::CallAsSetProperty;
                accessorDef.hasProperty = JSCAccessorCallbackWrapper::HasPropertyCallback;
                accessorDef.getPropertyNames = JSCAccessorCallbackWrapper::GetPropertyNamesCallback;
                accessorDef.finalize = FinalizeFunctionCallback;
                JSClassRef accessorRef = JSClassCreate(&accessorDef);
                auto funcCBData = new JSCAccessorCallbackData(env, p->getter, p->setter, p->data);
                JSObjectRef nativeObj = JSObjectMake(ctx, accessorRef, funcCBData);
                /*
                JSStringRef accessorNativeContainerName = JSCAccessorCallbackWrapper::CreateNativeAccessorNameUtf8(p->utf8name);
                JSObjectSetProperty(ctx, (JSObjectRef)object, accessorNativeContainerName, nativeObj, kJSPropertyAttributeNone, nullptr);
                JSStringRelease(accessorNativeContainerName);
                JSClassRelease(accessorRef);
                */
                JSObjectSetPrototype(ctx, (JSObjectRef)object, nativeObj);
                
                
            } else if (p->method != nullptr) {
                JSClassDefinition methodDef = kJSClassDefinitionEmpty;
                methodDef.className = p->utf8name;
                methodDef.callAsFunction = JSCFunctionCallbackWrapper::CallAsFunction;
                methodDef.finalize = FinalizeFunctionCallback;
                JSClassRef methodRef = JSClassCreate(&methodDef);
                auto funcCBData = new JSCFunctionCallbackData(env,p->method,p->data);
                JSObjectRef method = JSObjectMake(ctx,methodRef,funcCBData);
                JSStringRef methodName = JSStringCreateWithUTF8CString(p->utf8name);
                JSObjectSetProperty(ctx, (JSObjectRef)object, methodName, method, kJSPropertyAttributeNone, nullptr);
                JSStringRelease(methodName);
                JSClassRelease(methodRef);
            } else {
                JSStringRef propertyName = JSStringCreateWithUTF8CString(p->utf8name);
                JSObjectSetProperty(ctx, (JSObjectRef)object, propertyName, p->value, kJSPropertyAttributeNone, NULL);
                JSStringRelease(propertyName);
            }
        }
    }

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_array(napi_env env, napi_value value, bool* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueIsArray(ctx, value);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_array_length(napi_env env,
                                  napi_value value,
                                  uint32_t* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSStringRef lengthString = JSStringCreateWithUTF8CString ("length");
    JSValueRef lengthValue = JSObjectGetProperty(ctx, (JSObjectRef)value, lengthString, NULL);
    JSStringRelease(lengthString);
    *result = JSValueToNumber(ctx,lengthValue,NULL);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_strict_equals(napi_env env,
                               napi_value lhs,
                               napi_value rhs,
                               bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, lhs);
  CHECK_ARG(env, rhs);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();

  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_prototype(napi_env env,
                               napi_value object,
                               napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSObjectGetPrototype(ctx,(JSObjectRef)object);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_object(napi_env env, napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSClassDefinition classDef = kJSClassDefinitionEmpty;
    JSClassRef classRef = JSClassCreate(&classDef);
    *result = JSObjectMake(ctx,classRef,NULL);
    JSClassRelease(classRef);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_array(napi_env env, napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSObjectMakeArray(ctx, 0, NULL, NULL);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_array_with_length(napi_env env,
                                          size_t length,
                                          napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    
    *result = JSObjectMakeArray(ctx, 0, NULL, NULL);
    napi_set_named_property(env, *result, "length", JSValueMakeNumber(ctx, length));

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_string_latin1(napi_env env,
                                      const char* str,
                                      size_t length,
                                      napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_string_utf8(napi_env env,
                                    const char* str,
                                    size_t length,
                                    napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSStringRef strString = JSStringCreateWithUTF8CString(str);
    *result = JSValueMakeString(ctx, strString);
    JSStringRelease(strString);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_string_utf16(napi_env env,
                                     const char16_t* str,
                                     size_t length,
                                     napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_double(napi_env env,
                               double value,
                               napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueMakeNumber(ctx, value);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_int32(napi_env env,
                              int32_t value,
                              napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueMakeNumber(ctx, value);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_uint32(napi_env env,
                               uint32_t value,
                               napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueMakeNumber(ctx, value);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_int64(napi_env env,
                              int64_t value,
                              napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueMakeNumber(ctx, value);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_boolean(napi_env env, bool value, napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueMakeBoolean(ctx, value);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_symbol(napi_env env,
                               napi_value description,
                               napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);
    JSContextRef ctx = env->GetContext();
    JSStringRef valueString = JSValueToStringCopy(ctx, description, NULL);
    size_t requestedLength = JSStringGetMaximumUTF8CStringSize(valueString);
    char* buff = (char*)malloc(requestedLength);
    memset(buff, 0, requestedLength);
    JSStringGetUTF8CString(valueString, buff, requestedLength);
    free(buff);
    // create string value for now as symbol api is avaliable in ios 13 (beta)
    JSStringRelease(valueString);
    JSStringRef strString = JSStringCreateWithUTF8CString(buff);
    *result = JSValueMakeString(ctx, strString);
    JSStringRelease(strString);

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_error(napi_env env,
                              napi_value code,
                              napi_value msg,
                              napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, msg);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  *result = JSObjectMakeError(ctx, 0, NULL, NULL);

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_type_error(napi_env env,
                                   napi_value code,
                                   napi_value msg,
                                   napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, msg);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  *result = JSObjectMakeError(ctx, 0, NULL, NULL);

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_range_error(napi_env env,
                                    napi_value code,
                                    napi_value msg,
                                    napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, msg);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  *result = JSObjectMakeError(ctx, 0, NULL, NULL);

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_typeof(napi_env env,
                        napi_value value,
                        napi_valuetype* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSType type = JSValueGetType(ctx, value);

    switch (type)
      {
      case kJSTypeUndefined:
        *result = napi_undefined;
        break;
      case kJSTypeNull:
        *result = napi_null;
        break;
      case kJSTypeBoolean:
        *result = napi_boolean;
        break;
      case kJSTypeNumber:
        *result = napi_number;
        break;
      case kJSTypeString:
        *result = napi_string;
        break;
      case kJSTypeObject:
        *result = napi_object;
        if (JSObjectIsFunction(ctx, const_cast<JSObjectRef>(value)))
            *result = napi_function;
        break;
      }

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_undefined(napi_env env, napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueMakeUndefined(ctx);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_null(napi_env env, napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueMakeNull(ctx);

    return napi_clear_last_error(env);
}

// Gets all callback info in a single call. (Ugly, but faster.)
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_cb_info(
    napi_env env,               // [in] NAPI environment handle
    napi_callback_info cbinfo,  // [in] Opaque callback-info handle
    size_t* argc,      // [in-out] Specifies the size of the provided argv array
                       // and receives the actual count of args.
    napi_value* argv,  // [out] Array of values
    napi_value* this_arg,  // [out] Receives the JS 'this' arg for the call
    void** data) {         // [out] Receives the data pointer for the callback.
    CHECK_ENV(env);
    CHECK_ARG(env, cbinfo);

    JSCCallbackWrapper* info = reinterpret_cast<JSCCallbackWrapper*>(cbinfo);
    
    if (argv != nullptr) {
        CHECK_ARG(env, argc);
        info->Args(argv, *argc);
    }
    if (argc != nullptr) {
        *argc = info->ArgsLength();
    }
    if (this_arg != nullptr) {
        *this_arg = info->This();
    }
    if (data != nullptr) {
        *data = info->Data();
    }
  
    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_new_target(napi_env env,
                                napi_callback_info cbinfo,
                                napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, cbinfo);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSCFunctionCallbackWrapper* info = reinterpret_cast<JSCFunctionCallbackWrapper*>(cbinfo);
    if (info->IsConstructor()) {
        JSClassDefinition classDef = kJSClassDefinitionEmpty;
        JSClassRef classRef = JSClassCreate(&classDef);
        *result = JSObjectMake(ctx,classRef,NULL);
        JSClassRelease(classRef);
    } else {
        *result = nullptr;
    }
    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_call_function(napi_env env,
                               napi_value recv,
                               napi_value func,
                               size_t argc,
                               const napi_value* argv,
                               napi_value* result) {
    NAPI_PREAMBLE(env);
    if (argc > 0) {
    CHECK_ARG(env, argv);
    }

    JSContextRef ctx = env->GetContext();
    *result = JSObjectCallAsFunction(ctx, (JSObjectRef)func, (JSObjectRef)recv, argc, argv, NULL);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_global(napi_env env, napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSContextGetGlobalObject(ctx);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_throw(napi_env env, napi_value error) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, error);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_throw_error(napi_env env,
                             const char* code,
                             const char* msg) {
  NAPI_PREAMBLE(env);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_throw_type_error(napi_env env,
                                  const char* code,
                                  const char* msg) {
  NAPI_PREAMBLE(env);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_throw_range_error(napi_env env,
                                   const char* code,
                                   const char* msg) {
  NAPI_PREAMBLE(env);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_error(napi_env env, napi_value value, bool* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot
  // throw JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_double(napi_env env,
                                  napi_value value,
                                  double* result) {
    // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
    // JS exceptions.
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueToNumber(ctx, value, NULL);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_int32(napi_env env,
                                 napi_value value,
                                 int32_t* result) {
    // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
    // JS exceptions.
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueToNumber(ctx, value, NULL);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_uint32(napi_env env,
                                  napi_value value,
                                  uint32_t* result) {
    // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
    // JS exceptions.
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueToNumber(ctx, value, NULL);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_int64(napi_env env,
                                 napi_value value,
                                 int64_t* result) {
    // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
    // JS exceptions.
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueToNumber(ctx, value, NULL);

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result) {
    // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
    // JS exceptions.
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSValueToBoolean(ctx, value);

    return napi_clear_last_error(env);
}

// Copies a JavaScript string into a LATIN-1 string buffer. The result is the
// number of bytes (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufsize is insufficient, the string will be truncated and null terminated.
// If buf is NULL, this method returns the length of the string (in bytes)
// via the result parameter.
// The result argument is optional unless buf is NULL.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_string_latin1(napi_env env,
                                         napi_value value,
                                         char* buf,
                                         size_t bufsize,
                                         size_t* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

// Copies a JavaScript string into a UTF-8 string buffer. The result is the
// number of bytes (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufsize is insufficient, the string will be truncated and null terminated.
// If buf is NULL, this method returns the length of the string (in bytes)
// via the result parameter.
// The result argument is optional unless buf is NULL.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_string_utf8(napi_env env,
                                       napi_value value,
                                       char* buf,
                                       size_t bufsize,
                                       size_t* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, value);

    JSContextRef ctx = env->GetContext();
    JSStringRef valueString = JSValueToStringCopy(ctx, value, NULL);

    if (buf == nullptr) {
    if (result != nullptr)
      *result = JSStringGetMaximumUTF8CStringSize(valueString);
    } else {
    size_t returnSize = JSStringGetUTF8CString(valueString, buf, bufsize);
    if (result != nullptr)
      *result = returnSize;
    }

    JSStringRelease(valueString);
    return napi_clear_last_error(env);
}

// Copies a JavaScript string into a UTF-16 string buffer. The result is the
// number of 2-byte code units (excluding the null terminator) copied into buf.
// A sufficient buffer size should be greater than the length of string,
// reserving space for null terminator.
// If bufsize is insufficient, the string will be truncated and null terminated.
// If buf is NULL, this method returns the length of the string (in 2-byte
// code units) via the result parameter.
// The result argument is optional unless buf is NULL.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_string_utf16(napi_env env,
                                        napi_value value,
                                        char16_t* buf,
                                        size_t bufsize,
                                        size_t* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, value);

//    JSContextRef ctx = env->GetContext();
    // TODO

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_coerce_to_object(napi_env env,
                                  napi_value value,
                                  napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    if (JSValueIsObject(ctx, value)) {
        *result = value;
    } else {
        *result = JSValueToObject(ctx, value, NULL);
    }

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_coerce_to_bool(napi_env env,
                                napi_value value,
                                napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    if (JSValueIsBoolean(ctx, value)) {
        *result = value;
    } else {
        auto resultBool = JSValueToBoolean(ctx, value);
        *result = JSValueMakeBoolean(ctx, resultBool);
    }

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_coerce_to_number(napi_env env,
                                  napi_value value,
                                  napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    if (JSValueIsNumber(ctx, value)) {
        *result = value;
    } else {
        auto resultNumber = JSValueToNumber(ctx, value, NULL);
        *result = JSValueMakeNumber(ctx, resultNumber);
    }

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_coerce_to_string(napi_env env,
                                  napi_value value,
                                  napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    if (JSValueIsString(ctx, value)) {
        *result = value;
    } else {
        auto resultStr = JSValueToStringCopy(ctx, value, NULL);
        *result = JSValueMakeString(ctx, resultStr);
        JSStringRelease(resultStr);
    }

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_wrap(napi_env env,
                      napi_value js_object,
                      void* native_object,
                      napi_finalize finalize_cb,
                      void* finalize_hint,
                      napi_ref* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, js_object);


    wrapped_object* wrap = new wrapped_object();
    wrap->env = env;
    wrap->native_object = native_object;
    wrap->finalize_cb = finalize_cb;
    wrap->finalize_hint = finalize_hint;
    
//    JSContextRef ctx = env->GetContext();
    auto set = JSObjectSetPrivate((JSObjectRef)js_object, wrap);
    BeAssert(set);
    napi_create_reference(env, js_object, 0, result);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_unwrap(napi_env env, napi_value obj, void** result) {
    // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
    // JS exceptions.
    CHECK_ENV(env);

    //  JSContextRef ctx = env->GetContext();
    // TODO
    wrapped_object* wrap = (wrapped_object*)JSObjectGetPrivate((JSObjectRef)obj);
    *result = wrap->native_object;

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_remove_wrap(napi_env env, napi_value obj, void** result) {
  NAPI_PREAMBLE(env);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_external(napi_env env,
                                 void* data,
                                 napi_finalize finalize_cb,
                                 void* finalize_hint,
                                 napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  JSClassDefinition classDef = kJSClassDefinitionEmpty;
  JSClassRef classRef = JSClassCreate(&classDef);
  *result = JSObjectMake(ctx,classRef,data);

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_value_external(napi_env env,
                                    napi_value value,
                                    void** result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  *result = JSObjectGetPrivate((JSObjectRef)value);

  return napi_clear_last_error(env);
}

// Set initial_refcount to 0 for a weak reference, >0 for a strong reference.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_reference(napi_env env,
                                  napi_value value,
                                  uint32_t initial_refcount,
                                  napi_ref* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, value);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    napi_ref__* ref = new napi_ref__();
    ref->value = value;
    ref->refCount = initial_refcount;
    uint32_t count = initial_refcount;
    while (count != 0) {
        JSValueProtect(ctx, value);
        count--;
    };
    *result = ref;

    return napi_clear_last_error(env);
}

// Deletes a reference. The referenced value is released, and may be GC'd unless
// there are other references to it.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_delete_reference(napi_env env, napi_ref ref) {
    CHECK_ENV(env);
    CHECK_ARG(env, ref);

    JSContextRef ctx = env->GetContext();
    uint32_t count = ref->refCount;
    while (count != 0) {
        JSValueUnprotect(ctx, ref->value);
        count--;
    }
    delete ref;

    return napi_clear_last_error(env);
}

// Increments the reference count, optionally returning the resulting count.
// After this call the reference will be a strong reference because its
// refcount is >0, and the referenced object is effectively "pinned".
// Calling this when the refcount is 0 and the object is unavailable
// results in an error.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_reference_ref(napi_env env, napi_ref ref, uint32_t* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, ref);

    JSContextRef ctx = env->GetContext();
    ref->refCount++;
    JSValueProtect(ctx, ref->value);
    if (result != NULL) {
        *result = ref->refCount;
    }

    return napi_clear_last_error(env);
}

// Decrements the reference count, optionally returning the resulting count. If
// the result is 0 the reference is now weak and the object may be GC'd at any
// time if there are no other references. Calling this when the refcount is
// already 0 results in an error.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, ref);

    JSContextRef ctx = env->GetContext();
    ref->refCount--;
    JSValueUnprotect(ctx, ref->value);
    if (result != NULL) {
        *result = ref->refCount;
    }

    return napi_clear_last_error(env);
}

// Attempts to get a referenced value. If the reference is weak, the value might
// no longer be available, in that case the call is still successful but the
// result is NULL.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_reference_value(napi_env env,
                                     napi_ref ref,
                                     napi_value* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, ref);
    CHECK_ARG(env, result);

    //JSContextRef ctx = env->GetContext();
    if (result != NULL) {
        *result = ref->value;
    }

    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
    *result = new napi_handle_scope__();

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, scope);
  
//  JSContextRef ctx = env->GetContext();
    delete scope;

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_open_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
    *result = new napi_escapable_handle_scope__();

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethodhg 
//---------------------------------------------------------------------------------------
napi_status napi_close_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope scope) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, scope);
  
//  JSContextRef ctx = env->GetContext();
    delete scope;
    
  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_escape_handle(napi_env env,
                               napi_escapable_handle_scope scope,
                               napi_value escapee,
                               napi_value* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, scope);
  CHECK_ARG(env, escapee);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  *result = escapee;

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_new_instance(napi_env env,
                              napi_value constructor,
                              size_t argc,
                              const napi_value* argv,
                              napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, constructor);
    if (argc > 0) {
    CHECK_ARG(env, argv);
    }
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    *result = JSObjectCallAsConstructor(ctx, (JSObjectRef)constructor, argc, argv, NULL);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_instanceof(napi_env env,
                            napi_value object,
                            napi_value constructor,
                            bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, object);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  *result = JSValueIsInstanceOfConstructor(ctx, object, (JSObjectRef)constructor, NULL);

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_async_init(napi_env env,
                            napi_value async_resource,
                            napi_value async_resource_name,
                            napi_async_context* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, async_resource_name);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  // node::async_context* async_context = new node::async_context();

  // *async_context = node::EmitAsyncInit(isolate, v8_resource, v8_resource_name);
  // *result = reinterpret_cast<napi_async_context>(async_context);

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_async_destroy(napi_env env,
                               napi_async_context async_context) {
  CHECK_ENV(env);
  CHECK_ARG(env, async_context);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_make_callback(napi_env env,
                               napi_async_context async_context,
                               napi_value recv,
                               napi_value func,
                               size_t argc,
                               const napi_value* argv,
                               napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, recv);
  if (argc > 0) {
    CHECK_ARG(env, argv);
  }

  JSContextRef ctx = env->GetContext();
  // TODO

  napi_value ret = JSObjectCallAsFunction(ctx, const_cast<JSObjectRef>(func), const_cast<JSObjectRef>(recv), argc, argv, nullptr);
  if (result)
      *result = ret;
    
    return GET_RETURN_STATUS(env);
}

// Methods to support catching exceptions
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_exception_pending(napi_env env, bool* result) {
  // NAPI_PREAMBLE is not used here: this function must execute when there is a
  // pending exception.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_and_clear_last_exception(napi_env env,
                                              napi_value* result) {
  // NAPI_PREAMBLE is not used here: this function must execute when there is a
  // pending exception.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

// Node "Buffer" not supported => use JS ArrayBuffer
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_arraybuffer(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

struct NapiCallBackInfo {
    napi_finalize m_finalize_cb;
    void* m_finalize_hint;
    napi_env m_env;
};

static void jsTypedArrayBytesDeallocator(void* bytes, void* deallocatorContext) {
    NapiCallBackInfo* cbInfo = static_cast<NapiCallBackInfo*>(deallocatorContext);
    if (cbInfo) {
        cbInfo->m_finalize_cb(cbInfo->m_env, bytes, cbInfo->m_finalize_hint);
        delete cbInfo;
    } else {
        free(bytes);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_arraybuffer(napi_env env,
                                    size_t byte_length,
                                    void** data,
                                    napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);
  JSContextRef ctx = env->GetContext();
  *data = malloc(byte_length);
  memset(*data, 0, byte_length);
  *result = JSObjectMakeArrayBufferWithBytesNoCopy(ctx,
                                                   *data,
                                                   byte_length,
                                                   jsTypedArrayBytesDeallocator,
                                                   nullptr,
                                                   nullptr);
  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_external_arraybuffer(napi_env env,
                                             void* external_data,
                                             size_t byte_length,
                                             napi_finalize finalize_cb,
                                             void* finalize_hint,
                                             napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);
  JSContextRef ctx = env->GetContext();
    void* data;
    if (!finalize_cb) {
        data = malloc(byte_length);
        memcpy(data, external_data, byte_length);
        *result = JSObjectMakeArrayBufferWithBytesNoCopy(ctx,
                                                         data,
                                                         byte_length,
                                                         jsTypedArrayBytesDeallocator,
                                                         nullptr,
                                                         nullptr);
    } else {
        NapiCallBackInfo* cbInfo = new NapiCallBackInfo();
        cbInfo->m_finalize_cb = finalize_cb;
        cbInfo->m_finalize_hint = finalize_hint;
        cbInfo->m_env = env;
        *result = JSObjectMakeArrayBufferWithBytesNoCopy(ctx,
                                                         external_data,
                                                         byte_length,
                                                         jsTypedArrayBytesDeallocator,
                                                         cbInfo,
                                                         nullptr);
    }


    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_arraybuffer_info(napi_env env,
                                      napi_value arraybuffer,
                                      void** data,
                                      size_t* byte_length) {
  CHECK_ENV(env);
  CHECK_ARG(env, arraybuffer);

  JSContextRef ctx = env->GetContext();
  // TODO
    *data = JSObjectGetArrayBufferBytesPtr(ctx, const_cast<JSObjectRef>(arraybuffer), nullptr);
    *byte_length = JSObjectGetArrayBufferByteLength(ctx, const_cast<JSObjectRef>(arraybuffer), nullptr);
  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_typedarray(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO
  return napi_clear_last_error(env);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_typedarray(napi_env env,
                                   napi_typedarray_type type,
                                   size_t length,
                                   napi_value arraybuffer,
                                   size_t byte_offset,
                                   napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, arraybuffer);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
    JSTypedArrayType targetType;
    switch (type) {
        case napi_int8_array: targetType = kJSTypedArrayTypeInt8Array; break;
        case napi_uint8_array: targetType = kJSTypedArrayTypeUint8Array; break;
        case napi_uint8_clamped_array: targetType = kJSTypedArrayTypeUint8ClampedArray; break;
        case napi_int16_array: targetType = kJSTypedArrayTypeInt16Array; break;
        case napi_uint16_array: targetType = kJSTypedArrayTypeUint16Array; break;
        case napi_int32_array: targetType = kJSTypedArrayTypeInt32Array; break;
        case napi_uint32_array: targetType = kJSTypedArrayTypeUint32Array; break;
        case napi_float32_array: targetType = kJSTypedArrayTypeFloat32Array; break;
        case napi_float64_array: targetType = kJSTypedArrayTypeFloat64Array; break;
        default:
            return napi_set_last_error(env, napi_invalid_arg);
    }
    
 *result = JSObjectMakeTypedArrayWithArrayBufferAndOffset(ctx, targetType, const_cast<JSObjectRef>(arraybuffer), byte_offset, length, nullptr);
  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_typedarray_info(napi_env env,
                                     napi_value typedarray,
                                     napi_typedarray_type* type,
                                     size_t* length,
                                     void** data,
                                     napi_value* arraybuffer,
                                     size_t* byte_offset) {
    CHECK_ENV(env);
    CHECK_ARG(env, typedarray);
    JSContextRef ctx = env->GetContext();
    if (type) {
        JSTypedArrayType jstype = JSValueGetTypedArrayType(ctx, const_cast<JSObjectRef>(typedarray), nullptr);
        switch (jstype) {
            case kJSTypedArrayTypeInt8Array: *type = napi_int8_array ; break;
            case kJSTypedArrayTypeUint8Array: *type = napi_uint8_array; break;
            case kJSTypedArrayTypeUint8ClampedArray: *type = napi_uint8_clamped_array ; break;
            case kJSTypedArrayTypeInt16Array: *type = napi_int16_array; break;
            case kJSTypedArrayTypeUint16Array: *type = napi_uint16_array; break;
            case kJSTypedArrayTypeInt32Array: *type = napi_int32_array; break;
            case kJSTypedArrayTypeUint32Array: *type = napi_uint32_array; break;
            case kJSTypedArrayTypeFloat32Array: *type = napi_float32_array; break;
            case kJSTypedArrayTypeFloat64Array: *type = napi_float64_array; break;
            default:
                return napi_set_last_error(env, napi_invalid_arg);
        }
    }
    if (length)
        *length = JSObjectGetTypedArrayLength(ctx, const_cast<JSObjectRef>(typedarray), nullptr);
    
    if (arraybuffer)
        *arraybuffer = JSObjectGetTypedArrayBuffer(ctx, const_cast<JSObjectRef>(typedarray), nullptr);
    
    if (byte_offset)
        *byte_offset = JSObjectGetTypedArrayByteOffset(ctx, const_cast<JSObjectRef>(typedarray), nullptr);
    
    if (data)
        *data = JSObjectGetTypedArrayBytesPtr(ctx, const_cast<JSObjectRef>(typedarray), nullptr);

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_dataview(napi_env env,
                                 size_t byte_length,
                                 napi_value arraybuffer,
                                 size_t byte_offset,
                                 napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, arraybuffer);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO
  
  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_dataview(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_dataview_info(napi_env env,
                                   napi_value dataview,
                                   size_t* byte_length,
                                   void** data,
                                   napi_value* arraybuffer,
                                   size_t* byte_offset) {
  CHECK_ENV(env);
  CHECK_ARG(env, dataview);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_version(napi_env env, uint32_t* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);
  *result = NAPI_VERSION;
    // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_get_node_version(napi_env env,
                                  const napi_node_version** result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);
  static const napi_node_version version = {
    0,
    0,
    0,
    0
  };
  *result = &version;
    // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_adjust_external_memory(napi_env env,
                                        int64_t change_in_bytes,
                                        int64_t* adjusted_value) {
  CHECK_ENV(env);
  CHECK_ARG(env, adjusted_value);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}


namespace {
    namespace uvimpl {
        
        static napi_status ConvertUVErrorCode(int code) {
            switch (code) {
                case 0:
                    return napi_ok;
                case UV_EINVAL:
                    return napi_invalid_arg;
                case UV_ECANCELED:
                    return napi_cancelled;
            }
            
            return napi_generic_failure;
        }
        
        // Wrapper around uv_work_t which calls user-provided callbacks.
        class Work {
            
        private:
            explicit Work(napi_env env,
                           napi_value async_resource,
                          napi_value async_resource_name,
                          napi_async_execute_callback execute,
                          napi_async_complete_callback complete = nullptr,
                          void* data = nullptr):
            _async_resource(async_resource),
            _async_resource_name(async_resource_name),
            _env(env),
            _data(data),
            _execute(execute),
            _complete(complete) {
                memset(&_request, 0, sizeof(_request));
                _request.data = this;
                (void)_async_resource;
                (void)_async_resource_name;
                _id = ++s_id; 
            }
            
            ~Work() { }
            
        public:
            static Work* New(napi_env env,
                             napi_value async_resource,
                             napi_value async_resource_name,
                             napi_async_execute_callback execute,
                             napi_async_complete_callback complete,
                             void* data) {
                return new Work(env, async_resource, async_resource_name,
                                execute, complete, data);
            }
            
            static void Delete(Work* work) {
                delete work;
            }
            
            static uint32_t GetId(Work* work) {
                return work->_id;
            }
            static void ExecuteCallback(uv_work_t* req) {
                Work* work = static_cast<Work*>(req->data);
                work->_execute(work->_env, work->_data);
            }
            
            static void CompleteCallback(uv_work_t* req, int status) {
                Work* work = static_cast<Work*>(req->data);
                
                if (work->_complete != nullptr) {
                    napi_env env = work->_env;
                    
                    // Establish a handle scope here so that every callback doesn't have to.
                    // Also it is needed for the exception-handling below.
                    //CallbackScope callback_scope(work);
                    
                    work->_complete(env, ConvertUVErrorCode(status), work->_data);
                    
                    // Note: Don't access `work` after this point because it was
                    // likely deleted by the complete callback.
                    
                    // If there was an unhandled exception in the complete callback,
                    // report it as a fatal exception. (There is no JavaScript on the
                    // callstack that can possibly handle it.)
                    /*
                    if (!env->last_exception.IsEmpty()) {
                        v8::TryCatch try_catch(env->isolate);
                        env->isolate->ThrowException(
                                                     v8::Local<v8::Value>::New(env->isolate, env->last_exception));
                    */
                        // TODO: Call our own version of FatalException
                        // node::FatalException(env->isolate, try_catch);
                    // }
                }
            }
            
            uv_work_t* Request() {
                return &_request;
            }
            
        private:
            napi_env _env;
            void* _data;
            uv_work_t _request;
            napi_async_execute_callback _execute;
            napi_async_complete_callback _complete;
            napi_value _async_resource;
            napi_value _async_resource_name;
            uint32_t _id;
            static uint32_t s_id;
        };
    uint32_t Work::s_id = 0;
        
    }  // end of namespace uvimpl
}  // end of anonymous namespace

#define CALL_UV(env, condition)                                 \
do {                                                            \
int result = (condition);                                       \
napi_status status = uvimpl::ConvertUVErrorCode(result);        \
if (status != napi_ok) {                                        \
return napi_set_last_error(env, status, result);                \
}                                                               \
} while (0)
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_async_work(napi_env env,
                                   napi_value async_resource,
                                   napi_value async_resource_name,
                                   napi_async_execute_callback execute,
                                   napi_async_complete_callback complete,
                                   void* data,
                                   napi_async_work* result) {
    CHECK_ENV(env);
    CHECK_ARG(env, execute);
    CHECK_ARG(env, result);
    /*
    v8::Local<v8::Context> context = env->isolate->GetCurrentContext();
    
    v8::Local<v8::Object> resource;
    if (async_resource != nullptr) {
        CHECK_TO_OBJECT(env, context, resource, async_resource);
    } else {
        resource = v8::Object::New(env->isolate);
    }
    
    v8::Local<v8::String> resource_name;
    CHECK_TO_STRING(env, context, resource_name, async_resource_name);
    */
    uvimpl::Work* work =
    uvimpl::Work::New(env, async_resource, async_resource_name,
                      execute, complete, data);
    
    *result = reinterpret_cast<napi_async_work>(work);
    return napi_clear_last_error(env);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_delete_async_work(napi_env env, napi_async_work work) {
    CHECK_ENV(env);
    CHECK_ARG(env, work);
    uvimpl::Work* w = reinterpret_cast<uvimpl::Work*>(work);
    uvimpl::Work::Delete(w);
    return napi_clear_last_error(env);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_queue_async_work(napi_env env, napi_async_work work) {
    CHECK_ENV(env);
    CHECK_ARG(env, work);
    
    // Consider: Encapsulate the uv_loop_t into an opaque pointer parameter.
    // Currently the environment event loop is the same as the UV default loop.
    // Someday (if node ever supports multiple isolates), it may be better to get
    // the loop from node::Environment::GetCurrent(env->isolate)->event_loop();
    uv_loop_t* event_loop = uv_default_loop();
    
    uvimpl::Work* w = reinterpret_cast<uvimpl::Work*>(work);
    CALL_UV(env, uv_queue_work(event_loop,
                               w->Request(),
                               uvimpl::Work::ExecuteCallback,
                               uvimpl::Work::CompleteCallback));
    
    return napi_clear_last_error(env);
}
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_cancel_async_work(napi_env env, napi_async_work work) {
    CHECK_ENV(env);
    CHECK_ARG(env, work);
    
    uvimpl::Work* w = reinterpret_cast<uvimpl::Work*>(work);
    CALL_UV(env, uv_cancel(reinterpret_cast<uv_req_t*>(w->Request())));
    
    return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_promise(napi_env env,
                                napi_deferred* deferred,
                                napi_value* promise) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, deferred);
  CHECK_ARG(env, promise);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_resolve_deferred(napi_env env,
                                  napi_deferred deferred,
                                  napi_value resolution) {
  
//  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_reject_deferred(napi_env env,
                                 napi_deferred deferred,
                                 napi_value resolution) {
  
//  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_promise(napi_env env,
                            napi_value promise,
                            bool* is_promise) {
  CHECK_ENV(env);
  CHECK_ARG(env, promise);
  CHECK_ARG(env, is_promise);

//  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_run_script(napi_env env,
                            napi_value script,
                            napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, script);
    CHECK_ARG(env, result);

    JSContextRef ctx = env->GetContext();
    JSStringRef scriptString = JSValueToStringCopy (ctx, script, NULL);
    *result = JSEvaluateScript(ctx,scriptString,NULL,NULL,0,NULL);
    JSStringRelease(scriptString);

    return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_run_script_with_identifier(napi_env env,
                            napi_value script,
                            napi_value identifier,
                            napi_value* result) {
    NAPI_PREAMBLE(env);
    CHECK_ARG(env, script);
    CHECK_ARG(env, result);
    
    JSContextRef ctx = env->GetContext();
    JSStringRef scriptString = JSValueToStringCopy (ctx, script, NULL);
    JSStringRef identifierString = JSValueToStringCopy (ctx, identifier, NULL);
    *result = JSEvaluateScript(ctx,scriptString,NULL,identifierString,0,NULL);
    JSStringRelease(scriptString);
    JSStringRelease(identifierString);

    return GET_RETURN_STATUS(env);
}
