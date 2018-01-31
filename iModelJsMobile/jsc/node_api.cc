
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

static
napi_status napi_set_last_error(napi_env env, napi_status error_code,
                                uint32_t engine_error_code = 0,
                                void* engine_reserved = nullptr);
static
napi_status napi_clear_last_error(napi_env env);

struct napi_env__ {
  explicit napi_env__(JSContextGroupRef jscContextGroup): m_jscContextGroup(jscContextGroup),
      last_error() {}
  ~napi_env__() {
  }
  JSContextGroupRef m_jscContextGroup;
  JSValueRef last_exception;
  napi_extended_error_info last_error;
  int open_handle_scopes = 0;
};

// convert from n-api property attributes to v8::PropertyAttribute
#ifdef WIP_PROPERTY_ATTRIBUTES
static inline v8::PropertyAttribute V8PropertyAttributesFromDescriptor(
    const napi_property_descriptor* descriptor) {
  unsigned int attribute_flags = v8::PropertyAttribute::None;

  if (descriptor->getter != nullptr || descriptor->setter != nullptr) {
    // The napi_writable attribute is ignored for accessor descriptors, but
    // V8 requires the ReadOnly attribute to match nonexistence of a setter.
    attribute_flags |= (descriptor->setter == nullptr ?
      v8::PropertyAttribute::ReadOnly : v8::PropertyAttribute::None);
  } else if ((descriptor->attributes & napi_writable) == 0) {
    attribute_flags |= v8::PropertyAttribute::ReadOnly;
  }

  if ((descriptor->attributes & napi_enumerable) == 0) {
    attribute_flags |= v8::PropertyAttribute::DontEnum;
  }
  if ((descriptor->attributes & napi_configurable) == 0) {
    attribute_flags |= v8::PropertyAttribute::DontDelete;
  }

  return static_cast<v8::PropertyAttribute>(attribute_flags);
}
#endif


//=== Conversion between JSC Handles and napi_value ========================

napi_status napi_create_function(napi_env env,
                                 const char* utf8name,
                                 size_t length,
                                 napi_callback cb,
                                 void* callback_data,
                                 napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);
  CHECK_ARG(env, cb);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Function> return_value;
  v8::EscapableHandleScope scope(isolate);
  v8::Local<v8::Object> cbdata =
      v8impl::CreateFunctionCallbackData(env, cb, callback_data);

  RETURN_STATUS_IF_FALSE(env, !cbdata.IsEmpty(), napi_generic_failure);

  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(
      isolate, v8impl::FunctionCallbackWrapper::Invoke, cbdata);

  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::MaybeLocal<v8::Function> maybe_function = tpl->GetFunction(context);
  CHECK_MAYBE_EMPTY(env, maybe_function, napi_generic_failure);

  return_value = scope.Escape(maybe_function.ToLocalChecked());

  if (utf8name != nullptr) {
    v8::Local<v8::String> name_string;
    CHECK_NEW_FROM_UTF8_LEN(env, name_string, utf8name, length);
    return_value->SetName(name_string);
  }

  *result = v8impl::JsValueFromV8LocalValue(return_value);

  return GET_RETURN_STATUS(env);
}

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

  v8::Isolate* isolate = env->isolate;

  v8::EscapableHandleScope scope(isolate);
  v8::Local<v8::Object> cbdata =
      v8impl::CreateFunctionCallbackData(env, constructor, callback_data);

  RETURN_STATUS_IF_FALSE(env, !cbdata.IsEmpty(), napi_generic_failure);

  v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(
      isolate, v8impl::FunctionCallbackWrapper::Invoke, cbdata);

  v8::Local<v8::String> name_string;
  CHECK_NEW_FROM_UTF8_LEN(env, name_string, utf8name, length);
  tpl->SetClassName(name_string);

  size_t static_property_count = 0;
  for (size_t i = 0; i < property_count; i++) {
    const napi_property_descriptor* p = properties + i;

    if ((p->attributes & napi_static) != 0) {
      // Static properties are handled separately below.
      static_property_count++;
      continue;
    }

    v8::Local<v8::Name> property_name;
    napi_status status =
        v8impl::V8NameFromPropertyDescriptor(env, p, &property_name);

    if (status != napi_ok) {
      return napi_set_last_error(env, status);
    }

    v8::PropertyAttribute attributes =
        v8impl::V8PropertyAttributesFromDescriptor(p);

    // This code is similar to that in napi_define_properties(); the
    // difference is it applies to a template instead of an object.
    if (p->getter != nullptr || p->setter != nullptr) {
      v8::Local<v8::Object> cbdata = v8impl::CreateAccessorCallbackData(
        env, p->getter, p->setter, p->data);

      tpl->PrototypeTemplate()->SetAccessor(
        property_name,
        p->getter ? v8impl::GetterCallbackWrapper::Invoke : nullptr,
        p->setter ? v8impl::SetterCallbackWrapper::Invoke : nullptr,
        cbdata,
        v8::AccessControl::DEFAULT,
        attributes);
    } else if (p->method != nullptr) {
      v8::Local<v8::Object> cbdata =
          v8impl::CreateFunctionCallbackData(env, p->method, p->data);

      RETURN_STATUS_IF_FALSE(env, !cbdata.IsEmpty(), napi_generic_failure);

      v8::Local<v8::FunctionTemplate> t =
        v8::FunctionTemplate::New(isolate,
          v8impl::FunctionCallbackWrapper::Invoke,
          cbdata,
          v8::Signature::New(isolate, tpl));

      tpl->PrototypeTemplate()->Set(property_name, t, attributes);
    } else {
      v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(p->value);
      tpl->PrototypeTemplate()->Set(property_name, value, attributes);
    }
  }

  *result = v8impl::JsValueFromV8LocalValue(scope.Escape(tpl->GetFunction()));

  if (static_property_count > 0) {
    std::vector<napi_property_descriptor> static_descriptors;
    static_descriptors.reserve(static_property_count);

    for (size_t i = 0; i < property_count; i++) {
      const napi_property_descriptor* p = properties + i;
      if ((p->attributes & napi_static) != 0) {
        static_descriptors.push_back(*p);
      }
    }

    napi_status status =
        napi_define_properties(env,
                               *result,
                               static_descriptors.size(),
                               static_descriptors.data());
    if (status != napi_ok) return status;
  }

  return GET_RETURN_STATUS(env);
}

napi_status napi_get_property_names(napi_env env,
                                    napi_value object,
                                    napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;
  CHECK_TO_OBJECT(env, context, obj, object);

  auto maybe_propertynames = obj->GetPropertyNames(context);

  CHECK_MAYBE_EMPTY(env, maybe_propertynames, napi_generic_failure);

  *result = v8impl::JsValueFromV8LocalValue(
      maybe_propertynames.ToLocalChecked());
  return GET_RETURN_STATUS(env);
}

napi_status napi_set_property(napi_env env,
                              napi_value object,
                              napi_value key,
                              napi_value value) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, key);
  CHECK_ARG(env, value);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  v8::Local<v8::Value> k = v8impl::V8LocalValueFromJsValue(key);
  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);

  v8::Maybe<bool> set_maybe = obj->Set(context, k, val);

  RETURN_STATUS_IF_FALSE(env, set_maybe.FromMaybe(false), napi_generic_failure);
  return GET_RETURN_STATUS(env);
}

napi_status napi_has_property(napi_env env,
                              napi_value object,
                              napi_value key,
                              bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);
  CHECK_ARG(env, key);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  v8::Local<v8::Value> k = v8impl::V8LocalValueFromJsValue(key);
  v8::Maybe<bool> has_maybe = obj->Has(context, k);

  CHECK_MAYBE_NOTHING(env, has_maybe, napi_generic_failure);

  *result = has_maybe.FromMaybe(false);
  return GET_RETURN_STATUS(env);
}

napi_status napi_get_property(napi_env env,
                              napi_value object,
                              napi_value key,
                              napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, key);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> k = v8impl::V8LocalValueFromJsValue(key);
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  auto get_maybe = obj->Get(context, k);

  CHECK_MAYBE_EMPTY(env, get_maybe, napi_generic_failure);

  v8::Local<v8::Value> val = get_maybe.ToLocalChecked();
  *result = v8impl::JsValueFromV8LocalValue(val);
  return GET_RETURN_STATUS(env);
}

napi_status napi_delete_property(napi_env env,
                                 napi_value object,
                                 napi_value key,
                                 bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, key);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Value> k = v8impl::V8LocalValueFromJsValue(key);
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);
  v8::Maybe<bool> delete_maybe = obj->Delete(context, k);
  CHECK_MAYBE_NOTHING(env, delete_maybe, napi_generic_failure);

  if (result != NULL)
    *result = delete_maybe.FromMaybe(false);

  return GET_RETURN_STATUS(env);
}

napi_status napi_has_own_property(napi_env env,
                                  napi_value object,
                                  napi_value key,
                                  bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, key);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);
  v8::Local<v8::Value> k = v8impl::V8LocalValueFromJsValue(key);
  RETURN_STATUS_IF_FALSE(env, k->IsName(), napi_name_expected);
  v8::Maybe<bool> has_maybe = obj->HasOwnProperty(context, k.As<v8::Name>());
  CHECK_MAYBE_NOTHING(env, has_maybe, napi_generic_failure);
  *result = has_maybe.FromMaybe(false);

  return GET_RETURN_STATUS(env);
}

napi_status napi_set_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    napi_value value) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  v8::Local<v8::Name> key;
  CHECK_NEW_FROM_UTF8(env, key, utf8name);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);

  v8::Maybe<bool> set_maybe = obj->Set(context, key, val);

  RETURN_STATUS_IF_FALSE(env, set_maybe.FromMaybe(false), napi_generic_failure);
  return GET_RETURN_STATUS(env);
}

napi_status napi_has_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  v8::Local<v8::Name> key;
  CHECK_NEW_FROM_UTF8(env, key, utf8name);

  v8::Maybe<bool> has_maybe = obj->Has(context, key);

  CHECK_MAYBE_NOTHING(env, has_maybe, napi_generic_failure);

  *result = has_maybe.FromMaybe(false);
  return GET_RETURN_STATUS(env);
}

napi_status napi_get_named_property(napi_env env,
                                    napi_value object,
                                    const char* utf8name,
                                    napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Name> key;
  CHECK_NEW_FROM_UTF8(env, key, utf8name);

  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  auto get_maybe = obj->Get(context, key);

  CHECK_MAYBE_EMPTY(env, get_maybe, napi_generic_failure);

  v8::Local<v8::Value> val = get_maybe.ToLocalChecked();
  *result = v8impl::JsValueFromV8LocalValue(val);
  return GET_RETURN_STATUS(env);
}

napi_status napi_set_element(napi_env env,
                             napi_value object,
                             uint32_t index,
                             napi_value value) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  auto set_maybe = obj->Set(context, index, val);

  RETURN_STATUS_IF_FALSE(env, set_maybe.FromMaybe(false), napi_generic_failure);

  return GET_RETURN_STATUS(env);
}

napi_status napi_has_element(napi_env env,
                             napi_value object,
                             uint32_t index,
                             bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  v8::Maybe<bool> has_maybe = obj->Has(context, index);

  CHECK_MAYBE_NOTHING(env, has_maybe, napi_generic_failure);

  *result = has_maybe.FromMaybe(false);
  return GET_RETURN_STATUS(env);
}

napi_status napi_get_element(napi_env env,
                             napi_value object,
                             uint32_t index,
                             napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);

  auto get_maybe = obj->Get(context, index);

  CHECK_MAYBE_EMPTY(env, get_maybe, napi_generic_failure);

  *result = v8impl::JsValueFromV8LocalValue(get_maybe.ToLocalChecked());
  return GET_RETURN_STATUS(env);
}

napi_status napi_delete_element(napi_env env,
                                napi_value object,
                                uint32_t index,
                                bool* result) {
  NAPI_PREAMBLE(env);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;

  CHECK_TO_OBJECT(env, context, obj, object);
  v8::Maybe<bool> delete_maybe = obj->Delete(context, index);
  CHECK_MAYBE_NOTHING(env, delete_maybe, napi_generic_failure);

  if (result != NULL)
    *result = delete_maybe.FromMaybe(false);

  return GET_RETURN_STATUS(env);
}

napi_status napi_define_properties(napi_env env,
                                   napi_value object,
                                   size_t property_count,
                                   const napi_property_descriptor* properties) {
  NAPI_PREAMBLE(env);
  if (property_count > 0) {
    CHECK_ARG(env, properties);
  }

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Object> obj;
  CHECK_TO_OBJECT(env, context, obj, object);

  for (size_t i = 0; i < property_count; i++) {
    const napi_property_descriptor* p = &properties[i];

    v8::Local<v8::Name> property_name;
    napi_status status =
        v8impl::V8NameFromPropertyDescriptor(env, p, &property_name);

    if (status != napi_ok) {
      return napi_set_last_error(env, status);
    }

    v8::PropertyAttribute attributes =
        v8impl::V8PropertyAttributesFromDescriptor(p);

    if (p->getter != nullptr || p->setter != nullptr) {
      v8::Local<v8::Object> cbdata = v8impl::CreateAccessorCallbackData(
        env,
        p->getter,
        p->setter,
        p->data);

      auto set_maybe = obj->SetAccessor(
        context,
        property_name,
        p->getter ? v8impl::GetterCallbackWrapper::Invoke : nullptr,
        p->setter ? v8impl::SetterCallbackWrapper::Invoke : nullptr,
        cbdata,
        v8::AccessControl::DEFAULT,
        attributes);

      if (!set_maybe.FromMaybe(false)) {
        return napi_set_last_error(env, napi_invalid_arg);
      }
    } else if (p->method != nullptr) {
      v8::Local<v8::Object> cbdata =
          v8impl::CreateFunctionCallbackData(env, p->method, p->data);

      RETURN_STATUS_IF_FALSE(env, !cbdata.IsEmpty(), napi_generic_failure);

      v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(
          isolate, v8impl::FunctionCallbackWrapper::Invoke, cbdata);

      auto define_maybe = obj->DefineOwnProperty(
        context, property_name, t->GetFunction(), attributes);

      if (!define_maybe.FromMaybe(false)) {
        return napi_set_last_error(env, napi_generic_failure);
      }
    } else {
      v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(p->value);

      auto define_maybe =
          obj->DefineOwnProperty(context, property_name, value, attributes);

      if (!define_maybe.FromMaybe(false)) {
        return napi_set_last_error(env, napi_invalid_arg);
      }
    }
  }

  return GET_RETURN_STATUS(env);
}

napi_status napi_is_array(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);

  *result = val->IsArray();
  return napi_clear_last_error(env);
}

napi_status napi_get_array_length(napi_env env,
                                  napi_value value,
                                  uint32_t* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  RETURN_STATUS_IF_FALSE(env, val->IsArray(), napi_array_expected);

  v8::Local<v8::Array> arr = val.As<v8::Array>();
  *result = arr->Length();

  return GET_RETURN_STATUS(env);
}

napi_status napi_strict_equals(napi_env env,
                               napi_value lhs,
                               napi_value rhs,
                               bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, lhs);
  CHECK_ARG(env, rhs);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> a = v8impl::V8LocalValueFromJsValue(lhs);
  v8::Local<v8::Value> b = v8impl::V8LocalValueFromJsValue(rhs);

  *result = a->StrictEquals(b);
  return GET_RETURN_STATUS(env);
}

napi_status napi_get_prototype(napi_env env,
                               napi_value object,
                               napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Object> obj;
  CHECK_TO_OBJECT(env, context, obj, object);

  v8::Local<v8::Value> val = obj->GetPrototype();
  *result = v8impl::JsValueFromV8LocalValue(val);
  return GET_RETURN_STATUS(env);
}

napi_status napi_create_object(napi_env env, napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Object::New(env->isolate));

  return napi_clear_last_error(env);
}

napi_status napi_create_array(napi_env env, napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Array::New(env->isolate));

  return napi_clear_last_error(env);
}

napi_status napi_create_array_with_length(napi_env env,
                                          size_t length,
                                          napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Array::New(env->isolate, length));

  return napi_clear_last_error(env);
}

napi_status napi_create_string_latin1(napi_env env,
                                      const char* str,
                                      size_t length,
                                      napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  auto isolate = env->isolate;
  auto str_maybe =
      v8::String::NewFromOneByte(isolate,
                                 reinterpret_cast<const uint8_t*>(str),
                                 v8::NewStringType::kInternalized,
                                 length);
  CHECK_MAYBE_EMPTY(env, str_maybe, napi_generic_failure);

  *result = v8impl::JsValueFromV8LocalValue(str_maybe.ToLocalChecked());
  return napi_clear_last_error(env);
}

napi_status napi_create_string_utf8(napi_env env,
                                    const char* str,
                                    size_t length,
                                    napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  v8::Local<v8::String> s;
  CHECK_NEW_FROM_UTF8_LEN(env, s, str, length);

  *result = v8impl::JsValueFromV8LocalValue(s);
  return napi_clear_last_error(env);
}

napi_status napi_create_string_utf16(napi_env env,
                                     const char16_t* str,
                                     size_t length,
                                     napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  auto isolate = env->isolate;
  auto str_maybe =
      v8::String::NewFromTwoByte(isolate,
                                 reinterpret_cast<const uint16_t*>(str),
                                 v8::NewStringType::kInternalized,
                                 length);
  CHECK_MAYBE_EMPTY(env, str_maybe, napi_generic_failure);

  *result = v8impl::JsValueFromV8LocalValue(str_maybe.ToLocalChecked());
  return napi_clear_last_error(env);
}

napi_status napi_create_double(napi_env env,
                               double value,
                               napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Number::New(env->isolate, value));

  return napi_clear_last_error(env);
}

napi_status napi_create_int32(napi_env env,
                              int32_t value,
                              napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Integer::New(env->isolate, value));

  return napi_clear_last_error(env);
}

napi_status napi_create_uint32(napi_env env,
                               uint32_t value,
                               napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Integer::NewFromUnsigned(env->isolate, value));

  return napi_clear_last_error(env);
}

napi_status napi_create_int64(napi_env env,
                              int64_t value,
                              napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Number::New(env->isolate, static_cast<double>(value)));

  return napi_clear_last_error(env);
}

napi_status napi_get_boolean(napi_env env, bool value, napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;

  if (value) {
    *result = v8impl::JsValueFromV8LocalValue(v8::True(isolate));
  } else {
    *result = v8impl::JsValueFromV8LocalValue(v8::False(isolate));
  }

  return napi_clear_last_error(env);
}

napi_status napi_create_symbol(napi_env env,
                               napi_value description,
                               napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;

  if (description == nullptr) {
    *result = v8impl::JsValueFromV8LocalValue(v8::Symbol::New(isolate));
  } else {
    v8::Local<v8::Value> desc = v8impl::V8LocalValueFromJsValue(description);
    RETURN_STATUS_IF_FALSE(env, desc->IsString(), napi_string_expected);

    *result = v8impl::JsValueFromV8LocalValue(
      v8::Symbol::New(isolate, desc.As<v8::String>()));
  }

  return napi_clear_last_error(env);
}

static napi_status set_error_code(napi_env env,
                                  v8::Local<v8::Value> error,
                                  napi_value code,
                                  const char* code_cstring) {
  if ((code != nullptr) || (code_cstring != nullptr)) {
    v8::Isolate* isolate = env->isolate;
    v8::Local<v8::Context> context = isolate->GetCurrentContext();
    v8::Local<v8::Object> err_object = error.As<v8::Object>();

    v8::Local<v8::Value> code_value = v8impl::V8LocalValueFromJsValue(code);
    if (code != nullptr) {
      code_value = v8impl::V8LocalValueFromJsValue(code);
      RETURN_STATUS_IF_FALSE(env, code_value->IsString(), napi_string_expected);
    } else {
      CHECK_NEW_FROM_UTF8(env, code_value, code_cstring);
    }

    v8::Local<v8::Name> code_key;
    CHECK_NEW_FROM_UTF8(env, code_key, "code");

    v8::Maybe<bool> set_maybe = err_object->Set(context, code_key, code_value);
    RETURN_STATUS_IF_FALSE(env,
                           set_maybe.FromMaybe(false),
                           napi_generic_failure);

    // now update the name to be "name [code]" where name is the
    // original name and code is the code associated with the Error
    v8::Local<v8::String> name_string;
    CHECK_NEW_FROM_UTF8(env, name_string, "");
    v8::Local<v8::Name> name_key;
    CHECK_NEW_FROM_UTF8(env, name_key, "name");

    auto maybe_name = err_object->Get(context, name_key);
    if (!maybe_name.IsEmpty()) {
      v8::Local<v8::Value> name = maybe_name.ToLocalChecked();
      if (name->IsString()) {
        name_string = v8::String::Concat(name_string, name.As<v8::String>());
      }
    }
    name_string = v8::String::Concat(name_string,
                                     FIXED_ONE_BYTE_STRING(isolate, " ["));
    name_string = v8::String::Concat(name_string, code_value.As<v8::String>());
    name_string = v8::String::Concat(name_string,
                                     FIXED_ONE_BYTE_STRING(isolate, "]"));

    set_maybe = err_object->Set(context, name_key, name_string);
    RETURN_STATUS_IF_FALSE(env,
                           set_maybe.FromMaybe(false),
                           napi_generic_failure);
  }
  return napi_ok;
}

napi_status napi_create_error(napi_env env,
                              napi_value code,
                              napi_value msg,
                              napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, msg);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> message_value = v8impl::V8LocalValueFromJsValue(msg);
  RETURN_STATUS_IF_FALSE(env, message_value->IsString(), napi_string_expected);

  v8::Local<v8::Value> error_obj =
      v8::Exception::Error(message_value.As<v8::String>());
  napi_status status = set_error_code(env, error_obj, code, nullptr);
  if (status != napi_ok) return status;

  *result = v8impl::JsValueFromV8LocalValue(error_obj);

  return napi_clear_last_error(env);
}

napi_status napi_create_type_error(napi_env env,
                                   napi_value code,
                                   napi_value msg,
                                   napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, msg);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> message_value = v8impl::V8LocalValueFromJsValue(msg);
  RETURN_STATUS_IF_FALSE(env, message_value->IsString(), napi_string_expected);

  v8::Local<v8::Value> error_obj =
      v8::Exception::TypeError(message_value.As<v8::String>());
  napi_status status = set_error_code(env, error_obj, code, nullptr);
  if (status != napi_ok) return status;

  *result = v8impl::JsValueFromV8LocalValue(error_obj);

  return napi_clear_last_error(env);
}

napi_status napi_create_range_error(napi_env env,
                                    napi_value code,
                                    napi_value msg,
                                    napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, msg);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> message_value = v8impl::V8LocalValueFromJsValue(msg);
  RETURN_STATUS_IF_FALSE(env, message_value->IsString(), napi_string_expected);

  v8::Local<v8::Value> error_obj =
      v8::Exception::RangeError(message_value.As<v8::String>());
  napi_status status = set_error_code(env, error_obj, code, nullptr);
  if (status != napi_ok) return status;

  *result = v8impl::JsValueFromV8LocalValue(error_obj);

  return napi_clear_last_error(env);
}

napi_status napi_typeof(napi_env env,
                        napi_value value,
                        napi_valuetype* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> v = v8impl::V8LocalValueFromJsValue(value);

  if (v->IsNumber()) {
    *result = napi_number;
  } else if (v->IsString()) {
    *result = napi_string;
  } else if (v->IsFunction()) {
    // This test has to come before IsObject because IsFunction
    // implies IsObject
    *result = napi_function;
  } else if (v->IsExternal()) {
    // This test has to come before IsObject because IsExternal
    // implies IsObject
    *result = napi_external;
  } else if (v->IsObject()) {
    *result = napi_object;
  } else if (v->IsBoolean()) {
    *result = napi_boolean;
  } else if (v->IsUndefined()) {
    *result = napi_undefined;
  } else if (v->IsSymbol()) {
    *result = napi_symbol;
  } else if (v->IsNull()) {
    *result = napi_null;
  } else {
    // Should not get here unless V8 has added some new kind of value.
    return napi_set_last_error(env, napi_invalid_arg);
  }

  return napi_clear_last_error(env);
}

napi_status napi_get_undefined(napi_env env, napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
      v8::Undefined(env->isolate));

  return napi_clear_last_error(env);
}

napi_status napi_get_null(napi_env env, napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsValueFromV8LocalValue(
        v8::Null(env->isolate));

  return napi_clear_last_error(env);
}

// Gets all callback info in a single call. (Ugly, but faster.)
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

  v8impl::CallbackWrapper* info =
      reinterpret_cast<v8impl::CallbackWrapper*>(cbinfo);

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

napi_status napi_get_new_target(napi_env env,
                                napi_callback_info cbinfo,
                                napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, cbinfo);
  CHECK_ARG(env, result);

  v8impl::CallbackWrapper* info =
      reinterpret_cast<v8impl::CallbackWrapper*>(cbinfo);

  *result = info->GetNewTarget();
  return napi_clear_last_error(env);
}

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

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Value> v8recv = v8impl::V8LocalValueFromJsValue(recv);

  v8::Local<v8::Function> v8func;
  CHECK_TO_FUNCTION(env, v8func, func);

  auto maybe = v8func->Call(context, v8recv, argc,
    reinterpret_cast<v8::Local<v8::Value>*>(const_cast<napi_value*>(argv)));

  if (try_catch.HasCaught()) {
    return napi_set_last_error(env, napi_pending_exception);
  } else {
    if (result != nullptr) {
      CHECK_MAYBE_EMPTY(env, maybe, napi_generic_failure);
      *result = v8impl::JsValueFromV8LocalValue(maybe.ToLocalChecked());
    }
    return napi_clear_last_error(env);
  }
}

napi_status napi_get_global(napi_env env, napi_value* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  // TODO(ianhall): what if we need the global object from a different
  // context in the same isolate?
  // Should napi_env be the current context rather than the current isolate?
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  *result = v8impl::JsValueFromV8LocalValue(context->Global());

  return napi_clear_last_error(env);
}

napi_status napi_throw(napi_env env, napi_value error) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, error);

  v8::Isolate* isolate = env->isolate;

  isolate->ThrowException(v8impl::V8LocalValueFromJsValue(error));
  // any VM calls after this point and before returning
  // to the javascript invoker will fail
  return napi_clear_last_error(env);
}

napi_status napi_throw_error(napi_env env,
                             const char* code,
                             const char* msg) {
  NAPI_PREAMBLE(env);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::String> str;
  CHECK_NEW_FROM_UTF8(env, str, msg);

  v8::Local<v8::Value> error_obj = v8::Exception::Error(str);
  napi_status status = set_error_code(env, error_obj, nullptr, code);
  if (status != napi_ok) return status;

  isolate->ThrowException(error_obj);
  // any VM calls after this point and before returning
  // to the javascript invoker will fail
  return napi_clear_last_error(env);
}

napi_status napi_throw_type_error(napi_env env,
                                  const char* code,
                                  const char* msg) {
  NAPI_PREAMBLE(env);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::String> str;
  CHECK_NEW_FROM_UTF8(env, str, msg);

  v8::Local<v8::Value> error_obj = v8::Exception::TypeError(str);
  napi_status status = set_error_code(env, error_obj, nullptr, code);
  if (status != napi_ok) return status;

  isolate->ThrowException(error_obj);
  // any VM calls after this point and before returning
  // to the javascript invoker will fail
  return napi_clear_last_error(env);
}

napi_status napi_throw_range_error(napi_env env,
                                   const char* code,
                                   const char* msg) {
  NAPI_PREAMBLE(env);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::String> str;
  CHECK_NEW_FROM_UTF8(env, str, msg);

  v8::Local<v8::Value> error_obj = v8::Exception::RangeError(str);
  napi_status status = set_error_code(env, error_obj, nullptr, code);
  if (status != napi_ok) return status;

  isolate->ThrowException(error_obj);
  // any VM calls after this point and before returning
  // to the javascript invoker will fail
  return napi_clear_last_error(env);
}

napi_status napi_is_error(napi_env env, napi_value value, bool* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot
  // throw JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  *result = val->IsNativeError();

  return napi_clear_last_error(env);
}

napi_status napi_get_value_double(napi_env env,
                                  napi_value value,
                                  double* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  RETURN_STATUS_IF_FALSE(env, val->IsNumber(), napi_number_expected);

  *result = val.As<v8::Number>()->Value();

  return napi_clear_last_error(env);
}

napi_status napi_get_value_int32(napi_env env,
                                 napi_value value,
                                 int32_t* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);

  if (val->IsInt32()) {
    *result = val.As<v8::Int32>()->Value();
  } else {
    RETURN_STATUS_IF_FALSE(env, val->IsNumber(), napi_number_expected);

    // Empty context: https://github.com/nodejs/node/issues/14379
    v8::Local<v8::Context> context;
    *result = val->Int32Value(context).FromJust();
  }

  return napi_clear_last_error(env);
}

napi_status napi_get_value_uint32(napi_env env,
                                  napi_value value,
                                  uint32_t* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);

  if (val->IsUint32()) {
    *result = val.As<v8::Uint32>()->Value();
  } else {
    RETURN_STATUS_IF_FALSE(env, val->IsNumber(), napi_number_expected);

    // Empty context: https://github.com/nodejs/node/issues/14379
    v8::Local<v8::Context> context;
    *result = val->Uint32Value(context).FromJust();
  }

  return napi_clear_last_error(env);
}

napi_status napi_get_value_int64(napi_env env,
                                 napi_value value,
                                 int64_t* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);

  // This is still a fast path very likely to be taken.
  if (val->IsInt32()) {
    *result = val.As<v8::Int32>()->Value();
    return napi_clear_last_error(env);
  }

  RETURN_STATUS_IF_FALSE(env, val->IsNumber(), napi_number_expected);

  // v8::Value::IntegerValue() converts NaN to INT64_MIN, inconsistent with
  // v8::Value::Int32Value() that converts NaN to 0. So special-case NaN here.
  double doubleValue = val.As<v8::Number>()->Value();
  if (std::isnan(doubleValue)) {
    *result = 0;
  } else {
    // Empty context: https://github.com/nodejs/node/issues/14379
    v8::Local<v8::Context> context;
    *result = val->IntegerValue(context).FromJust();
  }

  return napi_clear_last_error(env);
}

napi_status napi_get_value_bool(napi_env env, napi_value value, bool* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  RETURN_STATUS_IF_FALSE(env, val->IsBoolean(), napi_boolean_expected);

  *result = val.As<v8::Boolean>()->Value();

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
napi_status napi_get_value_string_latin1(napi_env env,
                                         napi_value value,
                                         char* buf,
                                         size_t bufsize,
                                         size_t* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  RETURN_STATUS_IF_FALSE(env, val->IsString(), napi_string_expected);

  if (!buf) {
    CHECK_ARG(env, result);
    *result = val.As<v8::String>()->Length();
  } else {
    int copied = val.As<v8::String>()->WriteOneByte(
      reinterpret_cast<uint8_t*>(buf), 0, bufsize - 1,
      v8::String::NO_NULL_TERMINATION);

    buf[copied] = '\0';
    if (result != nullptr) {
      *result = copied;
    }
  }

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
napi_status napi_get_value_string_utf8(napi_env env,
                                       napi_value value,
                                       char* buf,
                                       size_t bufsize,
                                       size_t* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  RETURN_STATUS_IF_FALSE(env, val->IsString(), napi_string_expected);

  if (!buf) {
    CHECK_ARG(env, result);
    *result = val.As<v8::String>()->Utf8Length();
  } else {
    int copied = val.As<v8::String>()->WriteUtf8(
      buf, bufsize - 1, nullptr, v8::String::REPLACE_INVALID_UTF8 |
      v8::String::NO_NULL_TERMINATION);

    buf[copied] = '\0';
    if (result != nullptr) {
      *result = copied;
    }
  }

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
napi_status napi_get_value_string_utf16(napi_env env,
                                        napi_value value,
                                        char16_t* buf,
                                        size_t bufsize,
                                        size_t* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  RETURN_STATUS_IF_FALSE(env, val->IsString(), napi_string_expected);

  if (!buf) {
    CHECK_ARG(env, result);
    // V8 assumes UTF-16 length is the same as the number of characters.
    *result = val.As<v8::String>()->Length();
  } else {
    int copied = val.As<v8::String>()->Write(
      reinterpret_cast<uint16_t*>(buf), 0, bufsize - 1,
      v8::String::NO_NULL_TERMINATION);

    buf[copied] = '\0';
    if (result != nullptr) {
      *result = copied;
    }
  }

  return napi_clear_last_error(env);
}

napi_status napi_coerce_to_object(napi_env env,
                                  napi_value value,
                                  napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Object> obj;
  CHECK_TO_OBJECT(env, context, obj, value);

  *result = v8impl::JsValueFromV8LocalValue(obj);
  return GET_RETURN_STATUS(env);
}

napi_status napi_coerce_to_bool(napi_env env,
                                napi_value value,
                                napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Boolean> b;

  CHECK_TO_BOOL(env, context, b, value);

  *result = v8impl::JsValueFromV8LocalValue(b);
  return GET_RETURN_STATUS(env);
}

napi_status napi_coerce_to_number(napi_env env,
                                  napi_value value,
                                  napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::Number> num;

  CHECK_TO_NUMBER(env, context, num, value);

  *result = v8impl::JsValueFromV8LocalValue(num);
  return GET_RETURN_STATUS(env);
}

napi_status napi_coerce_to_string(napi_env env,
                                  napi_value value,
                                  napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Local<v8::String> str;

  CHECK_TO_STRING(env, context, str, value);

  *result = v8impl::JsValueFromV8LocalValue(str);
  return GET_RETURN_STATUS(env);
}

napi_status napi_wrap(napi_env env,
                      napi_value js_object,
                      void* native_object,
                      napi_finalize finalize_cb,
                      void* finalize_hint,
                      napi_ref* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, js_object);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(js_object);
  RETURN_STATUS_IF_FALSE(env, value->IsObject(), napi_invalid_arg);
  v8::Local<v8::Object> obj = value.As<v8::Object>();

  // If we've already wrapped this object, we error out.
  RETURN_STATUS_IF_FALSE(env, !v8impl::FindWrapper(obj), napi_invalid_arg);

  // Create a wrapper object with an internal field to hold the wrapped pointer
  // and a second internal field to identify the owner as N-API.
  v8::Local<v8::ObjectTemplate> wrapper_template;
  ENV_OBJECT_TEMPLATE(env, wrap, wrapper_template, v8impl::kWrapperFields);

  auto maybe_object = wrapper_template->NewInstance(context);
  CHECK_MAYBE_EMPTY(env, maybe_object, napi_generic_failure);
  v8::Local<v8::Object> wrapper = maybe_object.ToLocalChecked();

  // Store the pointer as an external in the wrapper.
  wrapper->SetInternalField(0, v8::External::New(isolate, native_object));
  wrapper->SetInternalField(1, v8::External::New(isolate,
    reinterpret_cast<void*>(const_cast<char*>(v8impl::napi_wrap_name))));

  // Insert the wrapper into the object's prototype chain.
  v8::Local<v8::Value> proto = obj->GetPrototype();
  CHECK(wrapper->SetPrototype(context, proto).FromJust());
  CHECK(obj->SetPrototype(context, wrapper).FromJust());

  v8impl::Reference* reference = nullptr;
  if (result != nullptr) {
    // The returned reference should be deleted via napi_delete_reference()
    // ONLY in response to the finalize callback invocation. (If it is deleted
    // before then, then the finalize callback will never be invoked.)
    // Therefore a finalize callback is required when returning a reference.
    CHECK_ARG(env, finalize_cb);
    reference = v8impl::Reference::New(
        env, obj, 0, false, finalize_cb, native_object, finalize_hint);
    *result = reinterpret_cast<napi_ref>(reference);
  } else if (finalize_cb != nullptr) {
    // Create a self-deleting reference just for the finalize callback.
    reference = v8impl::Reference::New(
        env, obj, 0, true, finalize_cb, native_object, finalize_hint);
  }

  if (reference != nullptr) {
    wrapper->SetInternalField(2, v8::External::New(isolate, reference));
  }

  return GET_RETURN_STATUS(env);
}

napi_status napi_unwrap(napi_env env, napi_value obj, void** result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  v8::Local<v8::Object> wrapper;
  return napi_set_last_error(env, v8impl::Unwrap(env, obj, result, &wrapper));
}

napi_status napi_remove_wrap(napi_env env, napi_value obj, void** result) {
  NAPI_PREAMBLE(env);
  v8::Local<v8::Object> wrapper;
  v8::Local<v8::Object> parent;
  napi_status status = v8impl::Unwrap(env, obj, result, &wrapper, &parent);
  if (status != napi_ok) {
    return napi_set_last_error(env, status);
  }

  v8::Local<v8::Value> external = wrapper->GetInternalField(2);
  if (external->IsExternal()) {
    v8impl::Reference::Delete(
        static_cast<v8impl::Reference*>(external.As<v8::External>()->Value()));
  }

  if (!parent.IsEmpty()) {
    v8::Maybe<bool> maybe = parent->SetPrototype(
        env->isolate->GetCurrentContext(), wrapper->GetPrototype());
    CHECK_MAYBE_NOTHING(env, maybe, napi_generic_failure);
    if (!maybe.FromMaybe(false)) {
      return napi_set_last_error(env, napi_generic_failure);
    }
  }

  return GET_RETURN_STATUS(env);
}

napi_status napi_create_external(napi_env env,
                                 void* data,
                                 napi_finalize finalize_cb,
                                 void* finalize_hint,
                                 napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;

  v8::Local<v8::Value> external_value = v8::External::New(isolate, data);

  // The Reference object will delete itself after invoking the finalizer
  // callback.
  v8impl::Reference::New(env,
      external_value,
      0,
      true,
      finalize_cb,
      data,
      finalize_hint);

  *result = v8impl::JsValueFromV8LocalValue(external_value);

  return napi_clear_last_error(env);
}

napi_status napi_get_value_external(napi_env env,
                                    napi_value value,
                                    void** result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  RETURN_STATUS_IF_FALSE(env, val->IsExternal(), napi_invalid_arg);

  v8::Local<v8::External> external_value = val.As<v8::External>();
  *result = external_value->Value();

  return napi_clear_last_error(env);
}

// Set initial_refcount to 0 for a weak reference, >0 for a strong reference.
napi_status napi_create_reference(napi_env env,
                                  napi_value value,
                                  uint32_t initial_refcount,
                                  napi_ref* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> v8_value = v8impl::V8LocalValueFromJsValue(value);

  if (!(v8_value->IsObject() || v8_value->IsFunction())) {
    return napi_set_last_error(env, napi_object_expected);
  }

  v8impl::Reference* reference =
      v8impl::Reference::New(env, v8_value, initial_refcount, false);

  *result = reinterpret_cast<napi_ref>(reference);
  return napi_clear_last_error(env);
}

// Deletes a reference. The referenced value is released, and may be GC'd unless
// there are other references to it.
napi_status napi_delete_reference(napi_env env, napi_ref ref) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  v8impl::Reference::Delete(reinterpret_cast<v8impl::Reference*>(ref));

  return napi_clear_last_error(env);
}

// Increments the reference count, optionally returning the resulting count.
// After this call the reference will be a strong reference because its
// refcount is >0, and the referenced object is effectively "pinned".
// Calling this when the refcount is 0 and the object is unavailable
// results in an error.
napi_status napi_reference_ref(napi_env env, napi_ref ref, uint32_t* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  v8impl::Reference* reference = reinterpret_cast<v8impl::Reference*>(ref);
  uint32_t count = reference->Ref();

  if (result != nullptr) {
    *result = count;
  }

  return napi_clear_last_error(env);
}

// Decrements the reference count, optionally returning the resulting count. If
// the result is 0 the reference is now weak and the object may be GC'd at any
// time if there are no other references. Calling this when the refcount is
// already 0 results in an error.
napi_status napi_reference_unref(napi_env env, napi_ref ref, uint32_t* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);

  v8impl::Reference* reference = reinterpret_cast<v8impl::Reference*>(ref);

  if (reference->RefCount() == 0) {
    return napi_set_last_error(env, napi_generic_failure);
  }

  uint32_t count = reference->Unref();

  if (result != nullptr) {
    *result = count;
  }

  return napi_clear_last_error(env);
}

// Attempts to get a referenced value. If the reference is weak, the value might
// no longer be available, in that case the call is still successful but the
// result is NULL.
napi_status napi_get_reference_value(napi_env env,
                                     napi_ref ref,
                                     napi_value* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, ref);
  CHECK_ARG(env, result);

  v8impl::Reference* reference = reinterpret_cast<v8impl::Reference*>(ref);
  *result = v8impl::JsValueFromV8LocalValue(reference->Get());

  return napi_clear_last_error(env);
}

napi_status napi_open_handle_scope(napi_env env, napi_handle_scope* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsHandleScopeFromV8HandleScope(
      new v8impl::HandleScopeWrapper(env->isolate));
  env->open_handle_scopes++;
  return napi_clear_last_error(env);
}

napi_status napi_close_handle_scope(napi_env env, napi_handle_scope scope) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, scope);
  if (env->open_handle_scopes == 0) {
    return napi_handle_scope_mismatch;
  }

  env->open_handle_scopes--;
  delete v8impl::V8HandleScopeFromJsHandleScope(scope);
  return napi_clear_last_error(env);
}

napi_status napi_open_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope* result) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = v8impl::JsEscapableHandleScopeFromV8EscapableHandleScope(
      new v8impl::EscapableHandleScopeWrapper(env->isolate));
  env->open_handle_scopes++;
  return napi_clear_last_error(env);
}

napi_status napi_close_escapable_handle_scope(
    napi_env env,
    napi_escapable_handle_scope scope) {
  // Omit NAPI_PREAMBLE and GET_RETURN_STATUS because V8 calls here cannot throw
  // JS exceptions.
  CHECK_ENV(env);
  CHECK_ARG(env, scope);
  if (env->open_handle_scopes == 0) {
    return napi_handle_scope_mismatch;
  }

  delete v8impl::V8EscapableHandleScopeFromJsEscapableHandleScope(scope);
  env->open_handle_scopes--;
  return napi_clear_last_error(env);
}

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

  v8impl::EscapableHandleScopeWrapper* s =
      v8impl::V8EscapableHandleScopeFromJsEscapableHandleScope(scope);
  if (!s->escape_called()) {
    *result = v8impl::JsValueFromV8LocalValue(
        s->Escape(v8impl::V8LocalValueFromJsValue(escapee)));
    return napi_clear_last_error(env);
  }
  return napi_set_last_error(env, napi_escape_called_twice);
}

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

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Function> ctor;
  CHECK_TO_FUNCTION(env, ctor, constructor);

  auto maybe = ctor->NewInstance(context, argc,
    reinterpret_cast<v8::Local<v8::Value>*>(const_cast<napi_value*>(argv)));

  CHECK_MAYBE_EMPTY(env, maybe, napi_generic_failure);

  *result = v8impl::JsValueFromV8LocalValue(maybe.ToLocalChecked());
  return GET_RETURN_STATUS(env);
}

napi_status napi_instanceof(napi_env env,
                            napi_value object,
                            napi_value constructor,
                            bool* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, object);
  CHECK_ARG(env, result);

  *result = false;

  v8::Local<v8::Object> ctor;
  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  CHECK_TO_OBJECT(env, context, ctor, constructor);

  if (!ctor->IsFunction()) {
    napi_throw_type_error(env,
                          "ERR_NAPI_CONS_FUNCTION",
                          "Constructor must be a function");

    return napi_set_last_error(env, napi_function_expected);
  }

  if (env->has_instance_available) {
    napi_value value, js_result = nullptr, has_instance = nullptr;
    napi_status status = napi_generic_failure;
    napi_valuetype value_type;

    // Get "Symbol" from the global object
    if (env->has_instance.IsEmpty()) {
      status = napi_get_global(env, &value);
      if (status != napi_ok) return status;
      status = napi_get_named_property(env, value, "Symbol", &value);
      if (status != napi_ok) return status;
      status = napi_typeof(env, value, &value_type);
      if (status != napi_ok) return status;

      // Get "hasInstance" from Symbol
      if (value_type == napi_function) {
        status = napi_get_named_property(env, value, "hasInstance", &value);
        if (status != napi_ok) return status;
        status = napi_typeof(env, value, &value_type);
        if (status != napi_ok) return status;

        // Store Symbol.hasInstance in a global persistent reference
        if (value_type == napi_symbol) {
          env->has_instance.Reset(env->isolate,
              v8impl::V8LocalValueFromJsValue(value));
          has_instance = value;
        }
      }
    } else {
      has_instance = v8impl::JsValueFromV8LocalValue(
          v8::Local<v8::Value>::New(env->isolate, env->has_instance));
    }

    if (has_instance) {
      status = napi_get_property(env, constructor, has_instance, &value);
      if (status != napi_ok) return status;
      status = napi_typeof(env, value, &value_type);
      if (status != napi_ok) return status;

      // Call the function to determine whether the object is an instance of the
      // constructor
      if (value_type == napi_function) {
        status = napi_call_function(env, constructor, value, 1, &object,
          &js_result);
        if (status != napi_ok) return status;
        return napi_get_value_bool(env, js_result, result);
      }
    }

    env->has_instance_available = false;
  }

  // If running constructor[Symbol.hasInstance](object) did not work, we perform
  // a traditional instanceof (early Node.js 6.x).

  v8::Local<v8::String> prototype_string;
  CHECK_NEW_FROM_UTF8(env, prototype_string, "prototype");

  auto maybe_prototype = ctor->Get(context, prototype_string);
  CHECK_MAYBE_EMPTY(env, maybe_prototype, napi_generic_failure);

  v8::Local<v8::Value> prototype_property = maybe_prototype.ToLocalChecked();
  if (!prototype_property->IsObject()) {
    napi_throw_type_error(
        env,
        "ERR_NAPI_CONS_PROTOTYPE_OBJECT",
        "Constructor.prototype must be an object");

    return napi_set_last_error(env, napi_object_expected);
  }

  auto maybe_ctor = prototype_property->ToObject(context);
  CHECK_MAYBE_EMPTY(env, maybe_ctor, napi_generic_failure);
  ctor = maybe_ctor.ToLocalChecked();

  v8::Local<v8::Value> current_obj = v8impl::V8LocalValueFromJsValue(object);
  if (!current_obj->StrictEquals(ctor)) {
    for (v8::Local<v8::Value> original_obj = current_obj;
         !(current_obj->IsNull() || current_obj->IsUndefined());) {
      if (current_obj->StrictEquals(ctor)) {
        *result = !(original_obj->IsNumber() ||
                    original_obj->IsBoolean() ||
                    original_obj->IsString());
        break;
      }
      v8::Local<v8::Object> obj;
      CHECK_TO_OBJECT(env, context, obj, v8impl::JsValueFromV8LocalValue(
        current_obj));
      current_obj = obj->GetPrototype();
    }
  }

  return GET_RETURN_STATUS(env);
}

napi_status napi_async_init(napi_env env,
                            napi_value async_resource,
                            napi_value async_resource_name,
                            napi_async_context* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, async_resource_name);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Object> v8_resource;
  if (async_resource != nullptr) {
    CHECK_TO_OBJECT(env, context, v8_resource, async_resource);
  } else {
    v8_resource = v8::Object::New(isolate);
  }

  v8::Local<v8::String> v8_resource_name;
  CHECK_TO_STRING(env, context, v8_resource_name, async_resource_name);

  // TODO(jasongin): Consider avoiding allocation here by using
  // a tagged pointer with 2×31 bit fields instead.
  node::async_context* async_context = new node::async_context();

  *async_context = node::EmitAsyncInit(isolate, v8_resource, v8_resource_name);
  *result = reinterpret_cast<napi_async_context>(async_context);

  return napi_clear_last_error(env);
}

napi_status napi_async_destroy(napi_env env,
                               napi_async_context async_context) {
  CHECK_ENV(env);
  CHECK_ARG(env, async_context);

  v8::Isolate* isolate = env->isolate;
  node::async_context* node_async_context =
      reinterpret_cast<node::async_context*>(async_context);
  node::EmitAsyncDestroy(isolate, *node_async_context);

  return napi_clear_last_error(env);
}

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

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  v8::Local<v8::Object> v8recv;
  CHECK_TO_OBJECT(env, context, v8recv, recv);

  v8::Local<v8::Function> v8func;
  CHECK_TO_FUNCTION(env, v8func, func);

  node::async_context* node_async_context =
    reinterpret_cast<node::async_context*>(async_context);
  if (node_async_context == nullptr) {
    static node::async_context empty_context = { 0, 0 };
    node_async_context = &empty_context;
  }

  v8::MaybeLocal<v8::Value> callback_result = node::MakeCallback(
      isolate, v8recv, v8func, argc,
      reinterpret_cast<v8::Local<v8::Value>*>(const_cast<napi_value*>(argv)),
      *node_async_context);
  CHECK_MAYBE_EMPTY(env, callback_result, napi_generic_failure);

  if (result != nullptr) {
    *result = v8impl::JsValueFromV8LocalValue(
        callback_result.ToLocalChecked());
  }

  return GET_RETURN_STATUS(env);
}

// Methods to support catching exceptions
napi_status napi_is_exception_pending(napi_env env, bool* result) {
  // NAPI_PREAMBLE is not used here: this function must execute when there is a
  // pending exception.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  *result = !env->last_exception.IsEmpty();
  return napi_clear_last_error(env);
}

napi_status napi_get_and_clear_last_exception(napi_env env,
                                              napi_value* result) {
  // NAPI_PREAMBLE is not used here: this function must execute when there is a
  // pending exception.
  CHECK_ENV(env);
  CHECK_ARG(env, result);

  if (env->last_exception.IsEmpty()) {
    return napi_get_undefined(env, result);
  } else {
    *result = v8impl::JsValueFromV8LocalValue(
      v8::Local<v8::Value>::New(env->isolate, env->last_exception));
    env->last_exception.Reset();
  }

  return napi_clear_last_error(env);
}

// Node "Buffer" not supported => use JS ArrayBuffer

napi_status napi_is_arraybuffer(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  *result = val->IsArrayBuffer();

  return napi_clear_last_error(env);
}

napi_status napi_create_arraybuffer(napi_env env,
                                    size_t byte_length,
                                    void** data,
                                    napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::ArrayBuffer> buffer =
      v8::ArrayBuffer::New(isolate, byte_length);

  // Optionally return a pointer to the buffer's data, to avoid another call to
  // retrieve it.
  if (data != nullptr) {
    *data = buffer->GetContents().Data();
  }

  *result = v8impl::JsValueFromV8LocalValue(buffer);
  return GET_RETURN_STATUS(env);
}

napi_status napi_create_external_arraybuffer(napi_env env,
                                             void* external_data,
                                             size_t byte_length,
                                             napi_finalize finalize_cb,
                                             void* finalize_hint,
                                             napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, result);

  v8::Isolate* isolate = env->isolate;
  v8::Local<v8::ArrayBuffer> buffer =
      v8::ArrayBuffer::New(isolate, external_data, byte_length);

  if (finalize_cb != nullptr) {
    // Create a self-deleting weak reference that invokes the finalizer
    // callback.
    v8impl::Reference::New(env,
        buffer,
        0,
        true,
        finalize_cb,
        external_data,
        finalize_hint);
  }

  *result = v8impl::JsValueFromV8LocalValue(buffer);
  return GET_RETURN_STATUS(env);
}

napi_status napi_get_arraybuffer_info(napi_env env,
                                      napi_value arraybuffer,
                                      void** data,
                                      size_t* byte_length) {
  CHECK_ENV(env);
  CHECK_ARG(env, arraybuffer);

  v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(arraybuffer);
  RETURN_STATUS_IF_FALSE(env, value->IsArrayBuffer(), napi_invalid_arg);

  v8::ArrayBuffer::Contents contents =
      value.As<v8::ArrayBuffer>()->GetContents();

  if (data != nullptr) {
    *data = contents.Data();
  }

  if (byte_length != nullptr) {
    *byte_length = contents.ByteLength();
  }

  return napi_clear_last_error(env);
}

napi_status napi_is_typedarray(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  *result = val->IsTypedArray();

  return napi_clear_last_error(env);
}

napi_status napi_create_typedarray(napi_env env,
                                   napi_typedarray_type type,
                                   size_t length,
                                   napi_value arraybuffer,
                                   size_t byte_offset,
                                   napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, arraybuffer);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(arraybuffer);
  RETURN_STATUS_IF_FALSE(env, value->IsArrayBuffer(), napi_invalid_arg);

  v8::Local<v8::ArrayBuffer> buffer = value.As<v8::ArrayBuffer>();
  v8::Local<v8::TypedArray> typedArray;

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

  *result = v8impl::JsValueFromV8LocalValue(typedArray);
  return GET_RETURN_STATUS(env);
}

napi_status napi_get_typedarray_info(napi_env env,
                                     napi_value typedarray,
                                     napi_typedarray_type* type,
                                     size_t* length,
                                     void** data,
                                     napi_value* arraybuffer,
                                     size_t* byte_offset) {
  CHECK_ENV(env);
  CHECK_ARG(env, typedarray);

  v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(typedarray);
  RETURN_STATUS_IF_FALSE(env, value->IsTypedArray(), napi_invalid_arg);

  v8::Local<v8::TypedArray> array = value.As<v8::TypedArray>();

  if (type != nullptr) {
    if (value->IsInt8Array()) {
      *type = napi_int8_array;
    } else if (value->IsUint8Array()) {
      *type = napi_uint8_array;
    } else if (value->IsUint8ClampedArray()) {
      *type = napi_uint8_clamped_array;
    } else if (value->IsInt16Array()) {
      *type = napi_int16_array;
    } else if (value->IsUint16Array()) {
      *type = napi_uint16_array;
    } else if (value->IsInt32Array()) {
      *type = napi_int32_array;
    } else if (value->IsUint32Array()) {
      *type = napi_uint32_array;
    } else if (value->IsFloat32Array()) {
      *type = napi_float32_array;
    } else if (value->IsFloat64Array()) {
      *type = napi_float64_array;
    }
  }

  if (length != nullptr) {
    *length = array->Length();
  }

  v8::Local<v8::ArrayBuffer> buffer = array->Buffer();
  if (data != nullptr) {
    *data = static_cast<uint8_t*>(buffer->GetContents().Data()) +
            array->ByteOffset();
  }

  if (arraybuffer != nullptr) {
    *arraybuffer = v8impl::JsValueFromV8LocalValue(buffer);
  }

  if (byte_offset != nullptr) {
    *byte_offset = array->ByteOffset();
  }

  return napi_clear_last_error(env);
}

napi_status napi_create_dataview(napi_env env,
                                 size_t byte_length,
                                 napi_value arraybuffer,
                                 size_t byte_offset,
                                 napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, arraybuffer);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(arraybuffer);
  RETURN_STATUS_IF_FALSE(env, value->IsArrayBuffer(), napi_invalid_arg);

  v8::Local<v8::ArrayBuffer> buffer = value.As<v8::ArrayBuffer>();
  v8::Local<v8::DataView> DataView = v8::DataView::New(buffer, byte_offset,
                                                       byte_length);

  *result = v8impl::JsValueFromV8LocalValue(DataView);
  return GET_RETURN_STATUS(env);
}

napi_status napi_is_dataview(napi_env env, napi_value value, bool* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, value);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> val = v8impl::V8LocalValueFromJsValue(value);
  *result = val->IsDataView();

  return napi_clear_last_error(env);
}

napi_status napi_get_dataview_info(napi_env env,
                                   napi_value dataview,
                                   size_t* byte_length,
                                   void** data,
                                   napi_value* arraybuffer,
                                   size_t* byte_offset) {
  CHECK_ENV(env);
  CHECK_ARG(env, dataview);

  v8::Local<v8::Value> value = v8impl::V8LocalValueFromJsValue(dataview);
  RETURN_STATUS_IF_FALSE(env, value->IsDataView(), napi_invalid_arg);

  v8::Local<v8::DataView> array = value.As<v8::DataView>();

  if (byte_length != nullptr) {
    *byte_length = array->ByteLength();
  }

  v8::Local<v8::ArrayBuffer> buffer = array->Buffer();
  if (data != nullptr) {
    *data = static_cast<uint8_t*>(buffer->GetContents().Data()) +
            array->ByteOffset();
  }

  if (arraybuffer != nullptr) {
    *arraybuffer = v8impl::JsValueFromV8LocalValue(buffer);
  }

  if (byte_offset != nullptr) {
    *byte_offset = array->ByteOffset();
  }

  return napi_clear_last_error(env);
}

napi_status napi_get_version(napi_env env, uint32_t* result) {
  CHECK_ENV(env);
  CHECK_ARG(env, result);
  *result = NAPI_VERSION;
  return napi_clear_last_error(env);
}

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
  return napi_clear_last_error(env);
}

napi_status napi_adjust_external_memory(napi_env env,
                                        int64_t change_in_bytes,
                                        int64_t* adjusted_value) {
  CHECK_ENV(env);
  CHECK_ARG(env, adjusted_value);

  *adjusted_value = env->isolate->AdjustAmountOfExternalAllocatedMemory(
      change_in_bytes);

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
class Work : public node::AsyncResource {
 private:
  explicit Work(napi_env env,
                v8::Local<v8::Object> async_resource,
                v8::Local<v8::String> async_resource_name,
                napi_async_execute_callback execute,
                napi_async_complete_callback complete = nullptr,
                void* data = nullptr)
    : AsyncResource(env->isolate,
                    async_resource,
                    *v8::String::Utf8Value(async_resource_name)),
    _env(env),
    _data(data),
    _execute(execute),
    _complete(complete) {
    memset(&_request, 0, sizeof(_request));
    _request.data = this;
  }

  ~Work() { }

 public:
  static Work* New(napi_env env,
                   v8::Local<v8::Object> async_resource,
                   v8::Local<v8::String> async_resource_name,
                   napi_async_execute_callback execute,
                   napi_async_complete_callback complete,
                   void* data) {
    return new Work(env, async_resource, async_resource_name,
                    execute, complete, data);
  }

  static void Delete(Work* work) {
    delete work;
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
      v8::HandleScope scope(env->isolate);
      CallbackScope callback_scope(work);

      work->_complete(env, ConvertUVErrorCode(status), work->_data);

      // Note: Don't access `work` after this point because it was
      // likely deleted by the complete callback.

      // If there was an unhandled exception in the complete callback,
      // report it as a fatal exception. (There is no JavaScript on the
      // callstack that can possibly handle it.)
      if (!env->last_exception.IsEmpty()) {
        v8::TryCatch try_catch(env->isolate);
        env->isolate->ThrowException(
          v8::Local<v8::Value>::New(env->isolate, env->last_exception));

        // TODO: Call our own version of FatalException
        // node::FatalException(env->isolate, try_catch);
      }
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
};

}  // end of namespace uvimpl
}  // end of anonymous namespace

#define CALL_UV(env, condition)                                         \
  do {                                                                  \
    int result = (condition);                                           \
    napi_status status = uvimpl::ConvertUVErrorCode(result);            \
    if (status != napi_ok) {                                            \
      return napi_set_last_error(env, status, result);                  \
    }                                                                   \
  } while (0)

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

  v8::Local<v8::Context> context = env->isolate->GetCurrentContext();

  v8::Local<v8::Object> resource;
  if (async_resource != nullptr) {
    CHECK_TO_OBJECT(env, context, resource, async_resource);
  } else {
    resource = v8::Object::New(env->isolate);
  }

  v8::Local<v8::String> resource_name;
  CHECK_TO_STRING(env, context, resource_name, async_resource_name);

  uvimpl::Work* work =
      uvimpl::Work::New(env, resource, resource_name,
                        execute, complete, data);

  *result = reinterpret_cast<napi_async_work>(work);

  return napi_clear_last_error(env);
}

napi_status napi_delete_async_work(napi_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  uvimpl::Work::Delete(reinterpret_cast<uvimpl::Work*>(work));

  return napi_clear_last_error(env);
}

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

napi_status napi_cancel_async_work(napi_env env, napi_async_work work) {
  CHECK_ENV(env);
  CHECK_ARG(env, work);

  uvimpl::Work* w = reinterpret_cast<uvimpl::Work*>(work);

  CALL_UV(env, uv_cancel(reinterpret_cast<uv_req_t*>(w->Request())));

  return napi_clear_last_error(env);
}

napi_status napi_create_promise(napi_env env,
                                napi_deferred* deferred,
                                napi_value* promise) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, deferred);
  CHECK_ARG(env, promise);

  auto maybe = v8::Promise::Resolver::New(env->isolate->GetCurrentContext());
  CHECK_MAYBE_EMPTY(env, maybe, napi_generic_failure);

  auto v8_resolver = maybe.ToLocalChecked();
  auto v8_deferred = new v8::Persistent<v8::Value>();
  v8_deferred->Reset(env->isolate, v8_resolver);

  *deferred = v8impl::JsDeferredFromV8Persistent(v8_deferred);
  *promise = v8impl::JsValueFromV8LocalValue(v8_resolver->GetPromise());
  return GET_RETURN_STATUS(env);
}

napi_status napi_resolve_deferred(napi_env env,
                                  napi_deferred deferred,
                                  napi_value resolution) {
  return v8impl::ConcludeDeferred(env, deferred, resolution, true);
}

napi_status napi_reject_deferred(napi_env env,
                                 napi_deferred deferred,
                                 napi_value resolution) {
  return v8impl::ConcludeDeferred(env, deferred, resolution, false);
}

napi_status napi_is_promise(napi_env env,
                            napi_value promise,
                            bool* is_promise) {
  CHECK_ENV(env);
  CHECK_ARG(env, promise);
  CHECK_ARG(env, is_promise);

  *is_promise = v8impl::V8LocalValueFromJsValue(promise)->IsPromise();

  return napi_clear_last_error(env);
}

napi_status napi_run_script(napi_env env,
                            napi_value script,
                            napi_value* result) {
  NAPI_PREAMBLE(env);
  CHECK_ARG(env, script);
  CHECK_ARG(env, result);

  v8::Local<v8::Value> v8_script = v8impl::V8LocalValueFromJsValue(script);

  if (!v8_script->IsString()) {
    return napi_set_last_error(env, napi_string_expected);
  }

  v8::Local<v8::Context> context = env->isolate->GetCurrentContext();

  auto maybe_script = v8::Script::Compile(context,
      v8::Local<v8::String>::Cast(v8_script));
  CHECK_MAYBE_EMPTY(env, maybe_script, napi_generic_failure);

  auto script_result =
      maybe_script.ToLocalChecked()->Run(context);
  CHECK_MAYBE_EMPTY(env, script_result, napi_generic_failure);

  *result = v8impl::JsValueFromV8LocalValue(script_result.ToLocalChecked());
  return GET_RETURN_STATUS(env);
}
