/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "node_api-config.h"
#include "node-src/napi.h"
#include <Bentley/ByteStream.h>
#include <json/BeJsValue.h>
#include <limits>

BEGIN_BENTLEY_NAMESPACE

struct BeNapi {
public:
    [[noreturn]] static void ThrowJsException(Napi::Env env, Utf8CP str, int errorNumber) {
        auto err = Napi::Error::New(env, str);
        err.Value()["errorNumber"] = errorNumber;
        throw err;
    }
    [[noreturn]] static void ThrowJsException(Napi::Env env, Utf8CP str) {
        throw Napi::Error::New(env, str);
    }
    [[noreturn]] static void ThrowJsTypeException(Napi::Env env, Utf8CP str) {
        throw Napi::TypeError::New(env, str);
    }
};

struct NapiRootRef;
struct NapiMemberRef;
struct NapiArrayRef;

//=======================================================================================
// @bsiclass                                                    Keith.Bentley  10/20
//=======================================================================================
struct NapiValueRef : JsValueRef {
public:
    static Napi::Object GetJsonObj(Napi::Env env) { return env.Global().Get("JSON").As<Napi::Object>(); }
    static Napi::String Stringify(Napi::Value value, StringifyFormat format = StringifyFormat::Default) { return GetJsonObj(value.Env()).Get("stringify").As<Napi::Function>()({value, value.Env().Undefined(), format==StringifyFormat::Indented ? Napi::Number::New(value.Env(), 3) : value.Env().Undefined()}).ToString(); }

    static Napi::Value Parse(Napi::String value) { return GetJsonObj(value.Env()).Get("parse").As<Napi::Function>()({value}); }
    static Napi::Value Parse(Napi::Env env, Utf8CP str) { return Parse(Napi::String::New(env, str)); }

    mutable Utf8String m_strVal; // only used when we return UTf8CP values
    mutable Napi::Value m_napiVal;

protected:
    NapiValueRef(Napi::Value val) : m_napiVal(val) {}

    virtual ValueRefType GetType() const override { return ValueRefType::Napi; }
    virtual void SetEmptyNapiObject() const = 0;
    virtual void SetEmptyNapiArray() const = 0;
    static bool IsBlank(Napi::Value val) { return val.IsEmpty() || val.IsUndefined() || val.IsNull(); }
    Napi::Object GetObject() const {
        if (!m_napiVal.IsObject())
            SetEmptyNapiObject();
        return m_napiVal.As<Napi::Object>();
    }
    Napi::Array GetArray() const {
        if (!m_napiVal.IsArray())
            SetEmptyNapiArray();
        return m_napiVal.As<Napi::Array>();
    }
    NapiValueRef& GetObjectMember(Utf8CP member, bool constVal) const;
    NapiValueRef& GetArrayMember(ArrayIndex index, bool constVal) const;
    virtual NapiValueRef* AsNapiValueRef() override {return this;}
    virtual NapiValueRef& GetMember(Utf8CP name, bool staticString) override { return GetObjectMember(name, false); }
    virtual NapiValueRef const& GetConstMember(Utf8CP name) const override { return GetObjectMember(name, true); }
    virtual NapiValueRef& GetArrayMember(ArrayIndex index) override { return GetArrayMember(index, false); }
    virtual NapiValueRef const& GetConstArrayMember(ArrayIndex index) const override { return GetArrayMember(index, true); }
    virtual void removeIndex(ArrayIndex i) override {
        auto array = GetArray();
        Napi::HandleScope scope(array.Env());
        auto splice = array.Get("splice");
        if (splice.IsFunction())
            splice.As<Napi::Function>().Call(array, {Napi::Number::New(array.Env(), i), Napi::Number::New(array.Env(), 1)});
    }
    virtual void removeMember(Utf8CP name) override { GetObject().Delete(name); }
    virtual bool hasMember(Utf8CP name) const override { return GetObject().Has(name); }
    virtual bool isMember(Utf8CP name) const override {
        auto obj = GetObject();
        return obj.Has(name) && !IsBlank(obj.Get(name));
    }
    virtual bool isNull() const override { return IsBlank(m_napiVal); }
    virtual bool isBool() const override { return m_napiVal.IsBoolean(); }
    virtual bool isNumeric() const override { return m_napiVal.IsNumber(); }
    virtual bool isString() const override { return m_napiVal.IsString(); }
    virtual bool isArray() const override { return m_napiVal.IsArray(); }
    virtual bool isBinary() const override { return isArrayBuffer() || isTypedArray(); }
    virtual bool isDate() const override { return m_napiVal.IsDate(); }
    virtual bool isArrayBuffer() const override { return m_napiVal.IsArrayBuffer(); }
    virtual bool isTypedArray() const override { return m_napiVal.IsTypedArray(); }
    virtual bool isObject() const override { return m_napiVal.IsObject() && !isArray() && !isBinary() && !isDate(); } // in JavaScript, array, date, and array buffers are objects, but we want isObject to return false for them
    virtual bool isFunction() const override { return m_napiVal.IsFunction(); }
    virtual bool isPromise() const override { return m_napiVal.IsPromise(); }
    virtual bool isDataView() const override { return m_napiVal.IsDataView(); }
    virtual bool isBuffer() const override { return m_napiVal.IsBuffer(); }
    virtual bool isExternal() const override { return m_napiVal.IsExternal(); }
    virtual void toArray() override { GetArray(); }
    virtual void toObject() override { GetObject(); }
    virtual unsigned int size() const override {
        if (m_napiVal.IsArray()) // should be first, array is also an object.
            return m_napiVal.As<Napi::Array>().Length();
        if (m_napiVal.IsObject()) {
            Napi::HandleScope scope(m_napiVal.Env());
            return m_napiVal.As<Napi::Object>().GetPropertyNames().Length();
        }
        return 0;
    }
    virtual Utf8CP ToUtf8CP() const override {
        if (!isString())
            return "";
        m_strVal = m_napiVal.ToString().Utf8Value();
        return m_strVal.c_str();
    }
    virtual Utf8String ToJsonString() const override {
        if (!m_napiVal.IsObject())
            return ToUtf8CP();
        auto val = m_napiVal;
        auto toJson = val.As<Napi::Object>().Get("toJSON");
        if (toJson.IsFunction())
            val = toJson.As<Napi::Function>().Call(m_napiVal, {});
        return Utf8String(std::move(val.ToString()));
    }
    virtual double GetDouble(double defVal) const override { return isNumeric() ? m_napiVal.ToNumber() : defVal; }
    virtual bool GetBoolean(bool defVal) const override { return isNull() ? defVal : m_napiVal.ToBoolean(); }
    virtual Json::Int GetInt(Json::Int defVal) const override { return isNumeric() ? m_napiVal.ToNumber() : defVal; }
    virtual Json::UInt GetUInt(Json::UInt defVal) const override { return isNumeric() ? m_napiVal.ToNumber() : defVal; }
    virtual BentleyStatus GetBinary(std::vector<Byte>& dest) const override {
        dest.clear();
        if (m_napiVal.IsTypedArray()) {
            auto arr = m_napiVal.As<Napi::Uint8Array>();
            Byte const* data = (Byte const*)arr.Data();
            dest.insert(dest.begin(), data, data + arr.ByteLength());
            return SUCCESS;
        }
        if (m_napiVal.IsArrayBuffer()) {
            auto ab = m_napiVal.As<Napi::ArrayBuffer>();
            Byte const* data = (Byte const*)ab.Data();
            dest.insert(dest.begin(), data, data + ab.ByteLength());
            return SUCCESS;
        }
        return JsValueRef::GetBinary(dest);
    }
    virtual BentleyStatus GetBinary(ByteStream& dest) const override {
        dest.clear();
        if (m_napiVal.IsTypedArray()) {
            auto arr = m_napiVal.As<Napi::Uint8Array>();
            Byte const* data = (Byte const*)arr.Data();
            dest.Append(data, (uint32_t)arr.ByteLength());
            return SUCCESS;
        }
        if (m_napiVal.IsArrayBuffer()) {
            auto ab = m_napiVal.As<Napi::ArrayBuffer>();
            Byte const* data = (Byte const*)ab.Data();
            dest.Append(data, (uint32_t)ab.ByteLength());
            return SUCCESS;
        }
        return JsValueRef::GetBinary(dest);
    }
    virtual bool ForEachProperty(std::function<bool(Utf8CP name, BeJsConst)> fn) const override;
    virtual bool ForEachArrayMember(std::function<bool(ArrayIndex, BeJsConst)> fn) const override;
    virtual bool ForEachArrayMemberValue(std::function<bool(ArrayIndex, BeJsValue)> fn) override;
    virtual Utf8String Stringify(StringifyFormat format = StringifyFormat::Default) const override { return Stringify(m_napiVal, format).Utf8Value(); }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley  10/20
//=======================================================================================
struct NapiRootRef : NapiValueRef {
private:
    void ShowError() const { BeAssert(false && "illegal for root obj"); }
    virtual void SetEmptyNapiObject() const override { BeAssert(m_napiVal.IsObject() && size() == 0 && "root must be empty object"); }
    virtual void SetEmptyNapiArray() const override { BeAssert(m_napiVal.IsArray() && size() == 0 && "root must be empty array"); }
    virtual void SetNull() override { BeAssert(size() == 0 && "root must be an empty object"); }
    virtual void operator=(double value) override { ShowError(); }
    virtual void operator=(bool value) override { ShowError(); }
    virtual void operator=(int64_t value) override { ShowError(); }
    virtual void operator=(int32_t value) override { ShowError(); }
    virtual void operator=(uint32_t value) override { ShowError(); }
    virtual void operator=(Utf8CP value) override { ShowError(); }
    virtual void SetBinary(Byte const* data, size_t size) override { ShowError(); }
    NapiRootRef(Napi::Value val) : NapiValueRef(val) {}
    friend struct BeJsConst;
    friend struct BeJsValue;
    friend struct BeJsNapiObject;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley  11/20
//=======================================================================================
struct NapiNullRef : NapiValueRef {
private:
    virtual void SetEmptyNapiObject() const override {}
    virtual void SetEmptyNapiArray() const override {}
    virtual void SetNull() override {}
    virtual NapiValueRef& GetMember(Utf8CP name, bool staticString) override { return *this; }
    virtual NapiValueRef const& GetConstMember(Utf8CP name) const override { return *this; }
    virtual NapiValueRef& GetArrayMember(ArrayIndex index) override { return *this; }
    virtual NapiValueRef const& GetConstArrayMember(ArrayIndex index) const override { return *this; }
    virtual void removeIndex(ArrayIndex i) override {}
    virtual void removeMember(Utf8CP name) override {}
    virtual bool isMember(Utf8CP name) const override { return false; }
    virtual void operator=(double value) override {}
    virtual void operator=(bool value) override {}
    virtual void operator=(int64_t value) override {}
    virtual void operator=(int32_t value) override {}
    virtual void operator=(uint32_t value) override {}
    virtual void operator=(Utf8CP value) override {}
    virtual void SetBinary(Byte const* data, size_t size) override {}
    NapiNullRef(Napi::Env env) : NapiValueRef(env.Undefined()) {}
    friend struct NapiValueRef;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley  10/20
//=======================================================================================
struct NapiMemberRef : NapiValueRef {
private:
    std::string m_name; // usually this doesn't cost much because member names are short, so short-string-optimization works
    mutable Napi::Object m_obj;
    void SetValue(Napi::Value val) const {
        m_napiVal = val;
        m_obj.Set(m_name, val);
    }
    virtual void SetEmptyNapiObject() const override {
        BeAssert(IsBlank(m_napiVal) && "Must be blank");
        SetValue(Napi::Object::New(m_napiVal.Env()));
    }
    virtual void SetEmptyNapiArray() const override {
        BeAssert(IsBlank(m_napiVal) && "Must be blank");
        SetValue(Napi::Array::New(m_obj.Env()));
    }
    virtual void SetNull() override { SetValue(m_obj.Env().Undefined()); }
    virtual void operator=(double value) override { SetValue(Napi::Value::From(m_obj.Env(), value)); }
    virtual void operator=(bool value) override { SetValue(Napi::Value::From(m_obj.Env(), value)); }
    virtual void operator=(int64_t value) override { SetValue(Napi::Value::From(m_obj.Env(), value)); }
    virtual void operator=(int32_t value) override { SetValue(Napi::Value::From(m_obj.Env(), value)); }
    virtual void operator=(uint32_t value) override { SetValue(Napi::Value::From(m_obj.Env(), value)); }
    virtual void operator=(Utf8CP value) override { SetValue(Napi::Value::From(m_obj.Env(), value)); }
    virtual void SetBinary(Byte const* data, size_t size) override {
        auto array = Napi::Uint8Array::New(m_obj.Env(), size);
        std::copy(data, data + size, array.Data());
        SetValue(array);
    }
    NapiMemberRef(Napi::Object obj, std::string const& name, Napi::Value val) : m_obj(obj), m_name(name), NapiValueRef(val) {}
    friend struct NapiValueRef;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley  10/20
//=======================================================================================
struct NapiArrayRef : NapiValueRef {
private:
    mutable Napi::Array m_array;
    ArrayIndex m_index;
    void SetValue(Napi::Value val) const {
        m_napiVal = val;
        m_array.Set(m_index, val);
    }
    virtual void SetEmptyNapiObject() const override {
        BeAssert(IsBlank(m_napiVal) && "Must be blank");
        SetValue(Napi::Object::New(m_array.Env()));
    }
    virtual void SetEmptyNapiArray() const override {
        BeAssert(IsBlank(m_napiVal) && "Must be blank");
        SetValue(Napi::Array::New(m_array.Env()));
    }
    virtual void SetNull() override { SetValue(m_array.Env().Undefined()); }
    virtual void operator=(double value) override { SetValue(Napi::Value::From(m_array.Env(), value)); }
    virtual void operator=(bool value) override { SetValue(Napi::Value::From(m_array.Env(), value)); }
    virtual void operator=(int64_t value) override { SetValue(Napi::Value::From(m_array.Env(), value)); }
    virtual void operator=(int32_t value) override { SetValue(Napi::Value::From(m_array.Env(), value)); }
    virtual void operator=(uint32_t value) override { SetValue(Napi::Value::From(m_array.Env(), value)); }
    virtual void operator=(Utf8CP value) override { SetValue(Napi::Value::From(m_array.Env(), value)); }
    virtual void SetBinary(Byte const* data, size_t size) override {
        auto array = Napi::Uint8Array::New(m_array.Env(), size);
        std::copy(data, data + size, array.Data());
        SetValue(array);
    }
    NapiArrayRef(Napi::Array array, ArrayIndex index, Napi::Value val) : m_array(array), m_index(index), NapiValueRef(val) {}
    friend struct NapiValueRef;
};

//=======================================================================================
// A root-level Napi::Object that is a BeJsValue.
// This class can be used to create a Napi::Object on the stack.
// @bsiclass                                                    Keith.Bentley  11/20
//=======================================================================================
struct BeJsNapiObject : BeJsValue {
    // construct a BeJsNapiObject that holds a newly created Napi::Object
    BeJsNapiObject(Napi::Env env) : BeJsValue() { m_val = new NapiRootRef(Napi::Object::New(env)); }
    // construct a BeJsNapiObject initialized from stringified JSON
    BeJsNapiObject(Napi::Env env, Utf8CP jsonString) { m_val = new NapiRootRef(NapiValueRef::Parse(env, jsonString)); }
    // construct a BeJsNapiObject initialized from stringified JSON
    BeJsNapiObject(Napi::Env env, std::string const& jsonString) : BeJsNapiObject(env, jsonString.c_str()) {}
    operator napi_value() const { return ((NapiRootRef&)*m_val).m_napiVal; }
    operator Napi::Object() const { return ((NapiRootRef&)*m_val).m_napiVal.As<Napi::Object>(); }
};

inline NapiValueRef& NapiValueRef::GetObjectMember(Utf8CP member, bool constVal) const {
    auto obj = GetObject();
    auto val = obj.Get(member);
    if (constVal && (val.IsEmpty() || val.IsUndefined()))
        return *new NapiNullRef(obj.Env());
    return *new NapiMemberRef(GetObject(), member, val);
}
inline NapiValueRef& NapiValueRef::GetArrayMember(ArrayIndex index, bool constVal) const {
    auto arr = GetArray();
    auto val = arr.Get(index);
    if (constVal && (val.IsEmpty() || val.IsUndefined()))
        return *new NapiNullRef(arr.Env());
    return *new NapiArrayRef(arr, index, val);
}
inline bool NapiValueRef::ForEachProperty(std::function<bool(Utf8CP name, BeJsConst)> fn) const {
    if (!m_napiVal.IsObject())
        return false;
    auto obj = m_napiVal.As<Napi::Object>();
    Napi::HandleScope scope(m_napiVal.Env());
    auto names = obj.GetPropertyNames();
    auto len = names.Length();
    for (uint32_t i = 0; i < len; ++i) {
        auto name = names.Get(i).As<Napi::String>().Utf8Value();
        if (fn(name.c_str(), BeJsConst(*new NapiMemberRef(obj, name, obj[name]))))
            return true;
    }
    return false;
}
inline bool NapiValueRef::ForEachArrayMember(std::function<bool(ArrayIndex, BeJsConst)> fn) const {
    if (!m_napiVal.IsArray())
        return false;
    Napi::HandleScope scope(m_napiVal.Env());
    auto array = m_napiVal.As<Napi::Array>();
    for (uint32_t i = 0; i < array.Length(); ++i) {
        if (fn(i, BeJsConst(*new NapiArrayRef(array, i, array[i]))))
            return true;
    }
    return false;
}

inline bool NapiValueRef::ForEachArrayMemberValue(std::function<bool(ArrayIndex, BeJsValue)> fn) {
    if (!m_napiVal.IsArray())
        return false;
    Napi::HandleScope scope(m_napiVal.Env());
    auto array = m_napiVal.As<Napi::Array>();
    for (uint32_t i = 0; i < array.Length(); ++i) {
        if (fn(i, BeJsValue(*new NapiArrayRef(array, i, array[i]))))
            return true;
    }
    return false;
}

inline BeJsConst::BeJsConst(Napi::Value val) : m_val(new NapiRootRef(val)) {}
inline BeJsValue::BeJsValue(Napi::Value val) : BeJsConst(val) {}

END_BENTLEY_NAMESPACE
