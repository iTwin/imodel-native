/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
#pragma once

// The following must be the first line of every ObjectWrap's destructor:
#define OBJECT_WRAP_DTOR_DISABLE_JS_CALLS DisableJsCalls disableJsCallsInScope;

#define PROPERTY_ATTRIBUTES static_cast<napi_property_attributes>(napi_enumerable | napi_configurable)

#define OPTIONAL_ARGUMENT_STRING(i, var)                                                                         \
    Utf8String var;                                                                                              \
    if (info.Length() <= (i) || info[i].IsUndefined())                                                           \
    {                                                                                                            \
        ;                                                                                                        \
    }                                                                                                            \
    else if (info[i].IsString())                                                                                 \
    {                                                                                                            \
        var = info[i].As<Napi::String>().Utf8Value().c_str();                                                    \
    }                                                                                                            \
    else                                                                                                         \
    {                                                                                                            \
        Napi::TypeError::New(Env(), "Argument " #i " must be string or undefined").ThrowAsJavaScriptException(); \
    }

#define REQUIRE_ARGUMENT_STRING(i, var)                                                                    \
    if (info.Length() <= (i) || !info[i].IsString())                                                       \
    {                                                                                                      \
        Napi::TypeError::New(info.Env(), "Argument " #i " must be a string").ThrowAsJavaScriptException(); \
    }                                                                                                      \
    Utf8String var = info[i].As<Napi::String>().Utf8Value().c_str();

#define REQUIRE_ARGUMENT_INTEGER(i, var)                                                                \
    if (info.Length() <= (i) || !info[i].IsNumber())                                                    \
    {                                                                                                   \
        Napi::TypeError::New(Env(), "Argument " #i " must be an integer").ThrowAsJavaScriptException(); \
    }                                                                                                   \
    int32_t var = info[i].As<Napi::Number>().Int32Value();

#define REQUIRE_ARGUMENT_BOOLEAN(i, var)                                                               \
    if (info.Length() <= (i) || !info[i].IsBoolean())                                                  \
    {                                                                                                  \
        Napi::TypeError::New(Env(), "Argument " #i " must be a boolean").ThrowAsJavaScriptException(); \
    }                                                                                                  \
    bool var = info[i].As<Napi::Boolean>().Value();

#define REQUIRE_ARGUMENT_ANY_OBJ(i, var, retval)                                     \
    if (info.Length() <= (i))                                                        \
    {                                                                                \
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object", retval) \
    }                                                                                \
    Napi::Object var = info[i].As<Napi::Object>();

#define REQUIRE_ARGUMENT_OBJ(i, T, var, retval)                                                  \
    if (info.Length() <= (i) || !T::InstanceOf(info[i]))                                         \
    {                                                                                            \
        THROW_TYPE_EXCEPTION_AND_RETURN("Argument " #i " must be an object of type " #T, retval) \
    }                                                                                            \
    T *var = T::Unwrap(info[i].As<Napi::Object>());

#define THROW_JS_TYPE_ERROR(str) Napi::TypeError::New(Env(), str).ThrowAsJavaScriptException();

#define THROW_TYPE_EXCEPTION_AND_RETURN(str, retval) \
    {                                                \
        THROW_JS_TYPE_ERROR(str)                     \
        return retval;                               \
    }

#define THROW_EXCEPTION_AND_RETURN(str, retval) \
    {                                           \
        THROW_JS_ERROR(str)                     \
        return retval;                          \
    }

#define THROW_JS_ERROR(str) Napi::Error::New(Env(), str).ThrowAsJavaScriptException();

#define RETURN_IF_HAD_EXCEPTION     \
    if (Env().IsExceptionPending()) \
        return Env().Undefined();
