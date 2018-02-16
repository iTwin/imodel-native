
// #include <node_buffer.h>
// #include <node_object_wrap.h>
#include <limits.h>  // INT_MAX
#include <string.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include "node_api.h"
#include "../node_internals.h"
#include <Bentley/BeAssert.h>
#undef min

#include <JavaScriptCore/JavaScriptCore.h>

#define NAPI_VERSION  1

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

  // TODO

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

  // TODO

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
  JSObjectRef objectObject = JSValueToObject(ctx, object, NULL);
  JSPropertyNameArrayRef properties = JSObjectCopyPropertyNames(ctx, objectObject);
  size_t count = JSPropertyNameArrayGetCount(properties);
  for (size_t i = 0; i < count; i++)
      {
        JSStringRef propertyString = JSPropertyNameArrayGetNameAtIndex(properties,i);
        result[i] = JSValueMakeString(ctx, propertyString);
      }

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  JSObjectSetProperty (ctx, objectObject, keyString, value, kJSPropertyAttributeNone, NULL);

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  *result = JSObjectHasProperty(ctx, objectObject, keyString);

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  *result = JSObjectGetProperty(ctx, objectObject, keyString, NULL);

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  *result = JSObjectDeleteProperty(ctx, objectObject, keyString, NULL);

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

  JSContextRef ctx = env->GetContext();

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  JSObjectSetProperty (ctx, objectObject, keyString, value, kJSPropertyAttributeNone, NULL);
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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  *result = JSObjectHasProperty(ctx, objectObject, keyString);
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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  *result = JSObjectGetProperty(ctx, objectObject, keyString, NULL);
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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  JSObjectSetPropertyAtIndex (ctx, objectObject, index, value, NULL);

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  JSPropertyNameArrayRef propertiesArray = JSObjectCopyPropertyNames(ctx, objectObject);
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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  *result = JSObjectGetPropertyAtIndex(ctx, objectObject, index, NULL);

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  JSPropertyNameArrayRef propertiesArray = JSObjectCopyPropertyNames(ctx, objectObject);
  size_t count = JSPropertyNameArrayGetCount(propertiesArray);
  if (index < count)
      {
      JSStringRef propertyString = JSPropertyNameArrayGetNameAtIndex(propertiesArray, index);
      *result = JSObjectDeleteProperty(ctx, objectObject, propertyString, NULL);
      }
  else
      {
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

  // TODO

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
  JSObjectRef valueObject = JSValueToObject (ctx, value, NULL);
  JSValueRef lengthValue = JSObjectGetProperty(ctx, valueObject, lengthString, NULL);
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

  JSContextRef ctx = env->GetContext();

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
  JSObjectRef objectObject = JSValueToObject (ctx, object, NULL);
  *result = JSObjectGetPrototype(ctx,objectObject);

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_create_object(napi_env env, napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  *result = JSObjectMake(ctx, NULL, NULL);

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
  *result = JSObjectMakeArray(ctx, length, NULL, NULL);

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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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
  // TODO

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
  // TODO

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
  // TODO

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
  // TODO

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

  JSContextRef ctx = env->GetContext();
  // TODO

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
  // TODO

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
  CHECK_ARG(env, recv);
  if (argc > 0) {
    CHECK_ARG(env, argv);
  }

  JSContextRef ctx = env->GetContext();
  JSObjectRef recvObject = JSValueToObject( ctx, recv, NULL);
  JSObjectRef funcObject = JSValueToObject( ctx, func, NULL);
  *result = JSObjectCallAsFunction(ctx, funcObject, recvObject, argc, argv, NULL);

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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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
      *result = JSStringGetLength(valueString);
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

  JSContextRef ctx = env->GetContext();
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
  // TODO

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
  // TODO

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
  // TODO

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
  // TODO

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

  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_unwrap(napi_env env, napi_value obj, void** result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);

  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_remove_wrap(napi_env env, napi_value obj, void** result) {
  NAPI_PREAMBLE(env);

  JSContextRef ctx = env->GetContext();
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
  // TODO

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

  JSContextRef ctx = env->GetContext();
  // TODO

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
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

// Deletes a reference. The referenced value is released, and may be GC'd unless
// there are other references to it.
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_delete_reference(napi_env env, napi_ref ref) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  JSContextRef ctx = env->GetContext();
  // TODO

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
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  JSContextRef ctx = env->GetContext();
  // TODO

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
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  JSContextRef ctx = env->GetContext();
  // TODO

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
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  // TODO

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

  JSContextRef ctx = env->GetContext();
  // TODO

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
  
  JSContextRef ctx = env->GetContext();
  // TODO

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

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_close_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope scope) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, scope);
  
  JSContextRef ctx = env->GetContext();
  // TODO

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

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_set_last_error(env, napi_escape_called_twice);
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
  // TODO

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
  // TODO

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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
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
  // TODO

  return GET_RETURN_STATUS(env);
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
  // TODO

  return GET_RETURN_STATUS(env);
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

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_is_typedarray(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  JSContextRef ctx = env->GetContext();
  // TODO
 
  return napi_clear_last_error(env);
}

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
  // TODO

  /*
  switch (type) {
    case napi_int8_array:
      typedArray = v8::Int8Array::New(buffer, byte_offset, length);
      break;
    case napi_uint8_array:
      typedArray = v8::Uint8Array::New(buffer, byte_offset, length);
      break;
    case napi_uint8_clamped_array:
      typedArray = v8::Uint8ClampedArray::New(buffer, byte_offset, length);
      break;
    case napi_int16_array:
      typedArray = v8::Int16Array::New(buffer, byte_offset, length);
      break;
    case napi_uint16_array:
      typedArray = v8::Uint16Array::New(buffer, byte_offset, length);
      break;
    case napi_int32_array:
      typedArray = v8::Int32Array::New(buffer, byte_offset, length);
      break;
    case napi_uint32_array:
      typedArray = v8::Uint32Array::New(buffer, byte_offset, length);
      break;
    case napi_float32_array:
      typedArray = v8::Float32Array::New(buffer, byte_offset, length);
      break;
    case napi_float64_array:
      typedArray = v8::Float64Array::New(buffer, byte_offset, length);
      break;
    default:
      return napi_set_last_error(env, napi_invalid_arg);
  }
  */


  return GET_RETURN_STATUS(env);
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
  // TODO

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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

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

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_delete_async_work(napi_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_queue_async_work(napi_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  JSContextRef ctx = env->GetContext();
  // TODO

  return napi_clear_last_error(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_cancel_async_work(napi_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  JSContextRef ctx = env->GetContext();
  // TODO

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

  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_resolve_deferred(napi_env env,
                                  napi_deferred deferred,
                                  napi_value resolution) {
  
  JSContextRef ctx = env->GetContext();
  // TODO

  return GET_RETURN_STATUS(env);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
napi_status napi_reject_deferred(napi_env env,
                                 napi_deferred deferred,
                                 napi_value resolution) {
  
  JSContextRef ctx = env->GetContext();
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

  JSContextRef ctx = env->GetContext();
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
  // TODO
  
  return GET_RETURN_STATUS(env);
}
