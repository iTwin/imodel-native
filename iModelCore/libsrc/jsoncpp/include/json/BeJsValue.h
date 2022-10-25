/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Bentley/BeId.h>
#include <Bentley/ByteStream.h>
#include <Bentley/Base64Utilities.h>
#include <BeRapidJson/BeRapidJson.h>
#include "json.h"

namespace Napi {
class Value;
}

BEGIN_BENTLEY_NAMESPACE

struct NapiValueRef;
struct BeJsValue;
struct BeJsConst;
enum StringifyFormat {
    Default,
    Indented,
};
//=======================================================================================
// @internal
// @bsiclass
//=======================================================================================
struct JsValueRef : RefCountedBase {
    static constexpr Utf8CP base64Header = "encoding=base64;";
    static constexpr size_t base64HeaderLen = 16;
protected:
    typedef unsigned int ArrayIndex;

    enum ValueRefType {
        JsonCpp,
        RapidJson,
        Napi
    };
    virtual ValueRefType GetType() const = 0;
    virtual NapiValueRef* AsNapiValueRef() {return nullptr;}
    virtual JsValueRef& GetMember(Utf8CP name, bool staticString) = 0;
    virtual JsValueRef const& GetConstMember(Utf8CP name) const = 0;
    virtual JsValueRef& GetArrayMember(ArrayIndex) = 0;
    virtual JsValueRef const& GetConstArrayMember(ArrayIndex) const = 0;
    virtual void SetNull() = 0;
    virtual void removeIndex(ArrayIndex i) = 0;
    virtual void removeMember(Utf8CP name) = 0;
    virtual bool isMember(Utf8CP name) const = 0;
    virtual bool hasMember(Utf8CP name) const = 0;
    virtual bool isNull() const = 0;
    virtual bool isBool() const = 0;
    virtual bool isNumeric() const = 0;
    virtual bool isString() const = 0;
    virtual bool isArray() const = 0;
    virtual bool isObject() const = 0;
    virtual bool isDate() const = 0;
    virtual bool isArrayBuffer() const = 0;
    virtual bool isTypedArray() const = 0;
    virtual bool isFunction() const = 0;
    virtual bool isPromise() const = 0;
    virtual bool isDataView() const = 0;
    virtual bool isBuffer() const = 0;
    virtual bool isExternal() const = 0;
    virtual void toArray() = 0;
    virtual void toObject() = 0;
    virtual uint32_t size() const = 0;
    virtual Utf8CP ToUtf8CP() const = 0;
    virtual Utf8String ToJsonString() const = 0;
    virtual void operator=(double value) = 0;
    virtual void operator=(bool value) = 0;
    virtual void operator=(int32_t value) = 0;
    virtual void operator=(uint32_t value) = 0;
    virtual void operator=(int64_t value) = 0;
    virtual void operator=(Utf8CP value) = 0;
    virtual double GetDouble(double defVal = 0) const = 0;
    virtual bool GetBoolean(bool defVal) const = 0;
    virtual int32_t GetInt(int32_t defVal) const = 0;
    virtual uint32_t GetUInt(uint32_t defVal) const = 0;
    Utf8CP GetBase64Data() const {
        if (!isString())
            return nullptr;
        Utf8CP str = ToUtf8CP();
        return (0 == strncmp(base64Header, str, base64HeaderLen)) ? str + base64HeaderLen : nullptr;
    }
    virtual bool isBinary() const { return nullptr != GetBase64Data(); }
    virtual BentleyStatus GetBinary(std::vector<Byte>& dest) const {
        dest.clear();
        Utf8CP data = GetBase64Data();
        if (nullptr == data)
            return ERROR;
        Base64Utilities::Decode(dest, data, strlen(data));
        return SUCCESS;
    }
    virtual BentleyStatus GetBinary(ByteStream& dest) const {
        dest.clear();
        Utf8CP data = GetBase64Data();
        if (nullptr == data)
            return ERROR;
        Base64Utilities::Decode(dest, data, strlen(data));
        return SUCCESS;
    }
    virtual void SetBinary(Byte const* data, size_t size) {
        Utf8String str;
        Base64Utilities::Encode(str, data, size, base64Header);
        *this = str.c_str();
    }
    virtual bool ForEachProperty(std::function<bool(Utf8CP name, BeJsConst)>) const = 0;
    virtual bool ForEachArrayMember(std::function<bool(ArrayIndex, BeJsConst)>) const = 0;
    virtual bool ForEachArrayMemberValue(std::function<bool(ArrayIndex, BeJsValue)>)  = 0;
    // when indented is true, writer must indent using space character and indent size is 3
    virtual Utf8String Stringify(StringifyFormat format = StringifyFormat::Default) const = 0;
    friend struct BeJsValue;
    friend struct BeJsConst;
};

//=======================================================================================
// A readonly JavaScript Value (like the JavaScript `const` keyword). This object holds a reference to either a JsonCpp, RapidJson, or a
// Napi JavaScript object if running under Node, and can only be constructed by supplying an object of one of those types.
// It is intended to be passed by value.
// @bsiclass
//=======================================================================================
struct BeJsConst {
protected:
    RefCountedPtr<JsValueRef> m_val;

    BeJsConst& operator=(BeJsConst const& rhs) = delete;
    BeJsConst() {}

public:
    typedef unsigned int ArrayIndex;

    BeJsConst(JsonValueCR);
    BeJsConst(RapidJsonDocumentCR);
    BeJsConst(RapidJsonValueCR, rapidjson::MemoryPoolAllocator<>&);
    BeJsConst(Napi::Value);
    BeJsConst(JsValueRef const& val) : m_val(&const_cast<JsValueRef&>(val)) {}

    // determine the implementation type for this value. Really for debugging only.
    JsValueRef::ValueRefType GetImplementation() const { return m_val->GetType(); }
    // Get access to the underlying NAPI object. Returns null if this is not based on NAPI object.
    NapiValueRef* AsNapiValueRef() {return m_val->AsNapiValueRef();}
    // get the value of a member of an object. If it doesn't exist, return null.
    // @note this value must be an object
    BeJsConst operator[](Utf8CP name) const { return BeJsConst(m_val->GetConstMember(name)); };
    // get the value of a member of an object. If it doesn't exist, return null.
    // @note this value must be an object
    BeJsConst operator[](std::string const& key) const { return (*this)[key.c_str()]; }
    // get the value of an entry in an array. If it doesn't exist, return null.
    // @note this value must be an array
    BeJsConst operator[](ArrayIndex index) const { return BeJsConst(m_val->GetConstArrayMember(index)); }
    // determine whether a named member of an object exists and is not null
    // @note: only valid if isObject() == true;
    bool isMember(Utf8CP name) const { return m_val->isMember(name); }
    // determine whether a named member of an object exists, even if it is undefined or null
    // @note: This method will return true if the member exists but is undefined or null, whereas `isMember` returns false
    // @note: only valid if isObject() == true;
    bool hasMember(Utf8CP name) const { return m_val->hasMember(name); }
    // determine whether a named member of an object is of type string
    // @note: only valid if isObject() == true;
    bool isStringMember(Utf8CP name) const {
        auto member = (*this)[name];
        return member.isString();
    }
    // determine whether a named member of an object is of type bool
    // @note: only valid if isObject() == true;
    bool isBoolMember(Utf8CP name) const {
        auto member = (*this)[name];
        return member.isBool();
    }
    // determine whether a named member of an object is numeric
    // @note: only valid if isObject() == true;
    bool isNumericMember(Utf8CP name) const {
        auto member = (*this)[name];
        return member.isNumeric();
    }
    // determine whether a named member of an object is an object
    // @note: only valid if isObject() == true;
    bool isObjectMember(Utf8CP name) const {
        auto member = (*this)[name];
        return member.isObject();
    }
    // determine whether a named member of an object is an array
    // @note: only valid if isObject() == true;
    bool isArrayMember(Utf8CP name) const {
        auto member = (*this)[name];
        return member.isArray();
    }
    // return true if this is of type null, undefined, or empty
    bool isNull() const { return m_val->isNull(); }
    // return true if this is a boolean
    bool isBool() const { return m_val->isBool(); }
    // return true if this is a Number
    bool isNumeric() const { return m_val->isNumeric(); }
    // return true if this is a string
    bool isString() const { return m_val->isString(); }
    // return true if this is an array
    // @note the JsonCpp api also returns true if the value is null, this differs from that intentionally
    bool isArray() const { return m_val->isArray(); }
    // return true if this is an object
    // @note the JsonCpp api also returns true if the value is null, this differs from that intentionally
    bool isObject() const { return m_val->isObject(); }
    // return true if this holds binary data
    bool isBinary() const { return m_val->isBinary(); }
    // return true if this is a JavaScript date object.
    bool isDate() const { return m_val->isDate(); }
    // return true if this is a JavaScript array buffer object.
    bool isArrayBuffer() const { return m_val->isArrayBuffer(); }
    // return true if this is a JavaScript TypedArray object.
    bool isTypedArray() const { return m_val->isTypedArray(); }
    // return true if this is a JavaScript function object.
    bool isFunction() const { return m_val->isFunction(); }
    // return true if this is a JavaScript promise.
    bool isPromise() const { return m_val->isPromise(); }
    // return true if this is a JavaScript DataView object.
    bool isDataView() const { return m_val->isDataView(); }
    // return true if this is a JavaScript Buffer object.
    bool isBuffer() const { return m_val->isBuffer(); }
    // return true if this is a JavaScript External object.
    bool isExternal() const { return m_val->isExternal(); }
    // return true if this is one of the JavaScript-only types.
    bool isJsOnlyType() const { return isDate() || isArrayBuffer() || isTypedArray() || isFunction() || isPromise() || isDataView() || isBuffer() || isExternal(); }
    // determine the size of this value. If this is an array, this returns the number of entries.
    // If this is an object, return the number of members. Otherwise, it returns 0.
    uint32_t size() const { return m_val->size(); }
    // if this is a String value, return a pointer to the value.
    Utf8CP ToUtf8CP() const { return m_val->ToUtf8CP(); }
    // if this is a JavaScript object with a "toJSON" method, return the stringified result of that method. Otherwise, just return ToUtf8CP
    Utf8String ToJsonString() const { return m_val->ToJsonString(); }
    // return true if this is an array or object with no entries, or isNull.
    bool empty() const {
        if (isArray() || isObject()) return size() == 0u;
        return isNull();
    }
    // get this value as a double, if possible. Otherwise return defVal.
    double GetDouble(double defVal = 0) const { return m_val->GetDouble(defVal); }
    // get this value as a boolean, if possible. Otherwise return defVal.
    bool GetBoolean(bool defVal = false) const { return m_val->GetBoolean(defVal); }
    // get this value as an int32_t, if possible. Otherwise return defVal.
    int32_t GetInt(int32_t defVal = 0) const { return m_val->GetInt(defVal); }
    // get this value as an int64, if possible. Otherwise return defVal.
    int64_t GetInt64(int64_t defVal = 0) const {
        if (isString()) {
            int64_t val = defVal;
            auto str = ToUtf8CP();
            auto fmt = (str[0] == '0' && (str[1] == 'X' || str[1] == 'x')) ? "%" SCNx64 : "%" SCNi64;
            Utf8String::Sscanf_safe(str, fmt, &val);
            return val;
        }
        if (isNumeric())
            return int64_t(GetDouble((double)defVal));

        return defVal;
    }
    // get this value as an uint64_t, if possible. Otherwise return defVal.
    uint64_t GetUInt64(uint64_t defVal = 0) const {
        if (isString()) {
            uint64_t val = defVal;
            auto str = ToUtf8CP();
            auto fmt = (str[0] == '0' && (str[1] == 'X' || str[1] == 'x')) ? "%" SCNx64 : "%" SCNu64;
            Utf8String::Sscanf_safe(str, fmt, &val);
            return val;
        }
        if (isNumeric()) {
            // Casting a double to a uint produces undefined results while casting a double to int is well defined.
            // Similarly, casting int to uint is well defined.
            // On ARM casting a negative double to uint results in 0.
            // On Intel, casting a negative double to uint produces the same result as below.
            return uint64_t(int64_t(GetDouble((double)defVal)));
        }

        return defVal;
    }
    // get this value as an BeInt64Id instance.
    template <class T>
    T GetId64() const {
        return T(GetUInt64());
    }
    // get this value as an unsigned int, if possible. Otherwise return defVal.
    uint32_t GetUInt(uint32_t defVal = 0) const { return m_val->GetUInt(defVal); }
    // get this value as a std::Vector<Byte>. If this is a JavaScript ArrayBuffer or TypedArray, it's value is returned. Otherwise, this must be a Base64-encoded string.
    BentleyStatus GetBinary(std::vector<Byte>& dest) const { return m_val->GetBinary(dest); }
    // get this value as a ByteStream. If this is a JavaScript ArrayBuffer or TypedArray, it's value is returned. Otherwise, this must be a Base64-encoded string.
    BentleyStatus GetBinary(ByteStream& dest) const { return m_val->GetBinary(dest); }
    // get the value of an entry in an array. If it doesn't exist, return null.
    // @note this value must be an array
    BeJsConst Get(ArrayIndex index) const { return (*this)[index]; }
    // get the value of a member of an object. If it doesn't exist, return null.
    // @note this value must be an object
    BeJsConst Get(Utf8CP name) const { return (*this)[name]; }
    // get the value of an entry in an array. If it doesn't exist, return null.
    // @note this value must be an array
    BeJsConst operator[](int index) const {
        BeAssert(index >= 0);
        return (*this)[ArrayIndex(index)];
    }
    // get the value of a named member as a boolean. If it doesn't exist, return defaultVal.
    bool getMemberBoolean(Utf8CP name, bool defaultVal = false) const {
        BeJsConst member = Get(name);
        return member.GetBoolean(defaultVal);
    }
    // alias for GetBoolean
    bool asBool(bool defaultVal = false) const { return GetBoolean(defaultVal); }
    // alias for GetDouble
    double asDouble(double defaultVal = 0.0) const { return GetDouble(defaultVal); }
    // alias for GetInt
    int32_t asInt(int32_t defaultVal = 0) const { return GetInt(defaultVal); }
    // alias for GetUInt
    uint32_t asUInt(uint32_t defaultVal = 0) const { return GetUInt(defaultVal); }
    // alias for GetInt64
    int64_t asInt64(int64_t defaultVal = 0) const { return GetInt64(defaultVal); }
    // alias for GetUInt64
    uint64_t asUInt64(uint64_t defaultVal = 0) const { return GetUInt64(defaultVal); }
    // get the value of this as a Utf8String, if possible. Otherwise return defaultVal
    Utf8String asString(Utf8CP defaultVal = "") const { return asCString(defaultVal); }
    // get the value of this as a Utf8CP, if possible. Otherwise return defaultVal
    Utf8CP asCString(Utf8CP defaultVal = "") const {
        if (isString() || isDate())
            return ToUtf8CP();
        if (isBool())
            return GetBoolean() ? "true" : "false";
        return defaultVal;
    }
    // call a function for each member of this object. If the function returns true, the iteration stops.
    // @return true if function aborted, false if all members were iterated.
    bool ForEachProperty(std::function<bool(Utf8CP name, BeJsConst)> fn) const { return m_val->ForEachProperty(fn); }
    // call a function for each entry of this array. If the function returns true, the iteration stops.
    // @return true if function aborted, false if all entries were iterated.
    bool ForEachArrayMember(std::function<bool(ArrayIndex, BeJsConst)> fn) const { return m_val->ForEachArrayMember(fn); }
    // Stringify this value and all its children.
    // @note a null value returns an empty string, not "null". This differs from rapidjson's api.
    Utf8String Stringify(StringifyFormat format = StringifyFormat::Default) const { return m_val->isNull() ? "" : m_val->Stringify(format); }
    // compare this value to another for equality, using default tolerances
    bool operator==(BeJsConst other) const { return isAlmostEqual(other); }
    // compare this value to another for inequality, using default tolerances
    bool operator!=(BeJsConst other) const { return !(*this == other); }
    // compare this value to another for equality, using 0 for tolerances
    bool isExactEqual(BeJsConst other) const { return isAlmostEqual(other, 0, 0); }
    // compare this value to another for equality, with absolute and relative tolerances on numeric values
    bool isAlmostEqual(BeJsConst other, double absTol = 1.0e-15, double relTol = 1.0e-15) const;
    // compare two doubles for equality, with absolute and relative tolerances
    static bool areAlmostEqual(double a, double b, double absTol, double relTol) {
        if (a == b)
            return true;
        double delta = fabs(b - a);
        double q = std::max(fabs(a), fabs(b));
        return delta <= absTol + relTol * q;
    }
    // copy this value into another BeJsValue, potentially changing between implementation types.
    void SaveTo(BeJsValue dest) const;
};

//=======================================================================================
// A writeable JavaScript Value (like the JavaScript `let` keyword). This object holds a reference to either a JsonCpp, RapidJson, or a
// Napi JavaScript object if running under Node, and can only be constructed by supplying an object of one of those types.
// It is intended to be passed by value.
// @bsiclass
//=======================================================================================
struct BeJsValue : BeJsConst {
protected:
    BeJsValue() : BeJsConst() {}

public:
    BeJsValue& operator=(BeJsConst const& rhs) = delete; // this usually indicates a logic error. But if you really want this, use .From
    BeJsValue& operator=(BeJsValue const& rhs) = delete; // this usually indicates a logic error. But if you really want this, use .From

    BeJsValue(RapidJsonDocumentR);
    BeJsValue(JsonValueR);
    BeJsValue(RapidJsonValueR, rapidjson::MemoryPoolAllocator<>&);
    BeJsValue(Napi::Value);
    BeJsValue(JsValueRef& val) : BeJsConst(val) {}

    // get the value of a member of an object. If it doesn't exist, return null.
    // @note this value must be an object
    BeJsConst operator[](Utf8CP name) const { return BeJsConst(m_val->GetConstMember(name)); };
    // get the value of a member of an object. If it doesn't exist, return null.
    // @note this value must be an object
    BeJsConst operator[](std::string const& key) const { return (*this)[key.c_str()]; }
    // get the value of an entry in an array. If it doesn't exist, return null.
    // @note this value must be an array
    BeJsConst operator[](ArrayIndex index) const { return BeJsConst(m_val->GetConstArrayMember(index)); }
    // get the value of a member of an object using a static string. If it doesn't exist, it is created.
    // @note this value must be an object
    // @note this can be less expensive for some implementations since a reference to the string can be stored in the returned object.
    BeJsValue operator[](Json::StaticString const& key) { return BeJsValue(m_val->GetMember(key, true)); }
    // get the value of a member of an object. If it doesn't exist, it is created.
    // @note this value must be an object
    BeJsValue operator[](Utf8CP name) { return BeJsValue(m_val->GetMember(name, false)); }
    // get the value of an entry in an array. If it doesn't exist, it is created.
    // @note this value must be an array
    BeJsValue operator[](ArrayIndex index) { return BeJsValue(m_val->GetArrayMember(index)); }
    // get the value of a member of an object. If it doesn't exist, it is created.
    // @note this value must be an object
    BeJsValue operator[](std::string const& key) { return (*this)[key.c_str()]; }
    // get the value of an entry in an array. If it doesn't exist, it is created.
    // @note this value must be an array
    BeJsValue operator[](int index) {
        BeAssert(index >= 0);
        return (*this)[ArrayIndex(index)];
    }
    // get the value of an entry in an array. If index is past the end of the array, a new entry *at the end of the array* is created.
    // @note this value must be an array
    BeJsValue Get(uint32_t index) { return (*this)[index]; }
    // get the value of a member of an object. If it doesn't exist, create it.
    // @note this value must be an object
    BeJsValue Get(Utf8CP name) { return (*this)[name]; }
    // append a new blank value onto the end of this array
    BeJsValue appendValue() {
        toArray();
        return (*this)[size()];
    }
    // append a new blank object onto the end of this array
    BeJsValue appendObject() {
        toArray();
        auto obj = (*this)[size()];
        obj.SetEmptyObject();
        return obj;
    }
    // append a new blank array onto the end of this array
    BeJsValue appendArray() {
        toArray();
        auto obj = (*this)[size()];
        obj.SetEmptyArray();
        return obj;
    }
    // set the value of a member of this object to an integer. If the value is the same as defaultVal, remove the member instead.
    void SetOrRemoveInt(Utf8CP key, int32_t val, int32_t defaultVal) {
        if (val == defaultVal)
            removeMember(key);
        else
            (*this)[key] = val;
    }
    // set the value of a member of this object to an unsigned integer. If the value is the same as defaultVal, remove the member instead.
    void SetOrRemoveUInt(Utf8CP key, uint32_t val, uint32_t defaultVal) {
        if (val == defaultVal)
            removeMember(key);
        else
            (*this)[key] = val;
    }
    // set the value of a member of this object to a double. If the value is the same as defaultVal, remove the member instead.
    void SetOrRemoveDouble(Utf8CP key, double val, double defaultVal) {
        if (val == defaultVal)
            removeMember(key);
        else
            (*this)[key] = val;
    }
    // set the value of a member of this object to a boolean. If the value is the same as defaultVal, remove the member instead.
    void SetOrRemoveBool(Utf8CP key, bool val, bool defaultVal) {
        if (val == defaultVal)
            removeMember(key);
        else
            (*this)[key] = val;
    }
    // remove a member of a JavaScript Object.
    // @note: only valid if isObject == true;
    void removeMember(Utf8CP name) { m_val->removeMember(name); }
    // remove a member of a JavaScript Object
    // @note: only valid if isObject == true;
    void removeMember(std::string const& name) { removeMember(name.c_str()); }
    // remove the entry at index i of a JavaScript array
    // @note: only valid if isArray == true;
    void removeIndex(ArrayIndex i) {
        m_val->removeIndex(i);
    }
    // Assign a double to this value.
    // @note this must be an empty value or a primitive
    BeJsValue& operator=(double value) {
        (*m_val) = value;
        return *this;
    }
    // Assign a boolean to this value.
    // @note this must be an empty value or a primitive
    BeJsValue& operator=(bool value) {
        (*m_val) = value;
        return *this;
    }
    // Assign an int to this value.
    // @note this must be an empty value or a primitive
    BeJsValue& operator=(int32_t value) {
        (*m_val) = value;
        return *this;
    }
    // Assign an unsigned int to this value.
    // @note this must be an empty value or a primitive
    BeJsValue& operator=(uint32_t value) {
        (*m_val) = value;
        return *this;
    }
    // Assign an int64 to this value.
    // @note this must be an empty value or a primitive and the value to be assigned must be less than Number.MAX_SAFE_INTEGER
    // @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/MIN_SAFE_INTEGER
    BeJsValue& operator=(int64_t value) {
        (*m_val) = value;
        return *this;
    }
    // Assign an BeInt64Id to this value. This is saved as a hexidecimal-encoded string.
    BeJsValue& operator=(BeInt64Id id) {
        (*m_val) = id.ToHexStr().c_str();
        return *this;
    }
    // Assign a string to this value.
    // @note this must be an empty value or a primitive
    BeJsValue& operator=(Utf8CP value) {
        (*m_val) = value;
        return *this;
    }
    // Assign a string to this value.
    // @note this must be an empty value or a primitive
    BeJsValue& operator=(std::string const& value) {
        (*this) = value.c_str();
        return *this;
    }
    // if this is an empty value, convert it to an object
    // @note: if this is not empty, this method does nothing
    void toObject() { m_val->toObject(); }
    // if this is an empty value, convert it to an array
    // @note: if this is not empty, this method does nothing
    void toArray() { m_val->toArray(); }
    // set this value to be an empty object
    void SetEmptyObject() {
        SetNull();
        toObject();
    }
    // set this value to be an empty array
    void SetEmptyArray() {
        SetNull();
        toArray();
    }
    // set this value to null
    void SetNull() { m_val->SetNull(); }
    // Set this value as binary. If this is a JavaScript ArrayBuffer or TypedArray, it's value is stored directly. Otherwise, it is saved as a Base64-encoded string.
    void SetBinary(Byte const* data, size_t size) { m_val->SetBinary(data, size); }
    // Set this value as binary. If this is a JavaScript ArrayBuffer or TypedArray, it's value is stored directly. Otherwise, it is saved as a Base64-encoded string.
    void SetBinary(std::vector<Byte> const& data) { SetBinary(data.data(), data.size()); }
    // make this value a *copy* of another BeJsValue, potentially changing between implementation types.
    void From(BeJsConst other) {
        SetNull();
        FromOther(other);
    }
    bool ForEachArrayMemberValue(std::function<bool(ArrayIndex, BeJsValue)> fn) { return m_val->ForEachArrayMemberValue(fn); }

private:
    void FromOther(BeJsConst other) {
        if (other.isNull()) {
            SetNull();
            return;
        }
        if (other.isBool()) {
            *this = other.GetBoolean();
            return;
        }
        if (other.isNumeric()) {
            double val = other.GetDouble();
            if (val > Json::Value::minInt64() && Json::Value::IsIntegral(val)) {
                if (val < Json::Value::minInt()) {
                    *this = (int64_t)val;
                    return;
                }
                if (val < Json::Value::maxInt()) {
                    *this = (int32_t)val;
                    return;
                }
                if (val < Json::Value::maxUInt()) {
                    *this = (uint32_t)val;
                    return;
                }
                if (val < Json::Value::maxInt64()) {
                    *this = (int64_t)val;
                    return;
                }
            }
            *this = val;
            return;
        }
        if (other.isBinary()) {
            bvector<Byte> data;
            other.GetBinary(data);
            SetBinary(data);
            return;
        }
        if (other.isString()) {
            *this = other.ToUtf8CP();
            return;
        }
        if (other.isJsOnlyType()) {
            *this = other.ToJsonString();
            return;
        }
        if (other.isArray()) { // array must be before object, because in JavaScript/Napi, they're both.
            toArray();
            other.ForEachArrayMember([&](ArrayIndex i, BeJsConst entry) {
                (*this)[i].FromOther(entry);
                return false;
            });
            return;
        }
        if (other.isObject()) {
            toObject();
            other.ForEachProperty([&](Utf8CP name, BeJsConst entry) {
                (*this)[name].FromOther(entry);
                return false;
            });
            return;
        }
    }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BeJsonCppValue : JsValueRef {
    JsonValueR m_value;

private:
    JsonValueCR ConstValue() const { return *(JsonValueCP) &m_value; }
    virtual ValueRefType GetType() const override { return ValueRefType::JsonCpp; }
    virtual BeJsonCppValue& GetMember(Utf8CP name, bool staticString) override { return *new BeJsonCppValue(m_value.resolveReference(name, staticString)); }
    virtual BeJsonCppValue const& GetConstMember(Utf8CP name) const override { return *new BeJsonCppValue(ConstValue()[name]); }
    virtual BeJsonCppValue& GetArrayMember(ArrayIndex index) override { return *new BeJsonCppValue(m_value[index]); }
    virtual BeJsonCppValue const& GetConstArrayMember(ArrayIndex index) const override { return *new BeJsonCppValue(ConstValue()[index]); }
    virtual void SetNull() override { m_value = Json::Value(); }
    virtual void removeIndex(ArrayIndex i) override { m_value.removeIndex(i); };
    virtual void removeMember(Utf8CP name) override { m_value.removeMember(name); }
    virtual bool isMember(Utf8CP name) const override { return m_value.isMember(name); }
    virtual bool hasMember(Utf8CP name) const override { return m_value.hasMember(name); }
    virtual bool isNull() const override { return m_value.isNull(); }
    virtual bool isBool() const override { return m_value.isBool(); }
    virtual bool isNumeric() const override { return m_value.isNumeric(); }
    virtual bool isString() const override { return m_value.isString(); }
    virtual bool isArray() const override { return m_value.type() == Json::ValueType::arrayValue; }
    virtual bool isObject() const override { return m_value.type() == Json::ValueType::objectValue; }
    virtual bool isDate() const override { return false; }
    virtual bool isArrayBuffer() const override { return false; }
    virtual bool isTypedArray() const override { return false; }
    virtual bool isFunction() const override { return false; }
    virtual bool isPromise() const override { return false; }
    virtual bool isDataView() const override { return false; }
    virtual bool isBuffer() const override { return false; }
    virtual bool isExternal() const override { return false; }
    virtual void toArray() override {
        if (isNull())
            m_value = Json::Value(Json::ValueType::arrayValue);
    }
    virtual void toObject() override {
        if (isNull())
            m_value = Json::Value(Json::ValueType::objectValue);
    }
    virtual uint32_t size() const override { return m_value.size(); }
    virtual Utf8CP ToUtf8CP() const override { return m_value.asCString(); }
    virtual Utf8String ToJsonString() const override { return isString() ? m_value.asCString() : ""; }
    virtual void operator=(double value) override { m_value = value; }
    virtual void operator=(bool value) override { m_value = value; }
    virtual void operator=(int64_t value) override { m_value = Json::Value(value); }
    virtual void operator=(int32_t value) override { m_value = value; }
    virtual void operator=(uint32_t value) override { m_value = value; }
    virtual void operator=(Utf8CP value) override { m_value = value; }
    virtual double GetDouble(double defVal) const override { return m_value.asDouble(defVal); }
    virtual bool GetBoolean(bool defVal) const override { return m_value.asBool(defVal); }
    virtual int32_t GetInt(int32_t defVal) const override { return m_value.asInt(defVal); }
    virtual uint32_t GetUInt(uint32_t defVal) const override { return m_value.asUInt(defVal); }
    virtual bool ForEachProperty(std::function<bool(Utf8CP name, BeJsConst)> fn) const override {
        if (m_value.isObject()) {
            auto end = ConstValue().end();
            for (Json::Value::const_iterator iter = ConstValue().begin(); iter != end; ++iter) {
                if (fn(iter.memberName(), BeJsConst(*iter)))
                    return true;
            }
        }
        return false;
    }
    virtual bool ForEachArrayMember(std::function<bool(ArrayIndex, BeJsConst)> fn) const override {
        if (m_value.isArray()) {
            auto end = ConstValue().end();
            ArrayIndex i = 0;
            for (Json::Value::const_iterator iter = ConstValue().begin(); iter != end; ++iter) {
                if (fn(i++, BeJsConst(*iter)))
                    return true;
            }
        }
        return false;
    }
    virtual bool ForEachArrayMemberValue(std::function<bool(ArrayIndex, BeJsValue)> fn) override {
        if (m_value.isArray()) {
            auto end = m_value.end();
            ArrayIndex i = 0;
            for (Json::Value::iterator iter = m_value.begin(); iter != end; ++iter) {
                if (fn(i++, BeJsValue(*iter)))
                    return true;
            }
        }
        return false;
    }

    virtual Utf8String Stringify(StringifyFormat format = StringifyFormat::Default) const override { return format == StringifyFormat::Indented? m_value.toStyledString() : m_value.ToString(); }

    explicit BeJsonCppValue(JsonValueR val) : m_value(val) {}
    explicit BeJsonCppValue(JsonValueCR val) : m_value(const_cast<JsonValueR>(val)) {}
    friend struct BeJsValue;
    friend struct BeJsConst;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct BeRapidJsonValue : JsValueRef {
    typedef rapidjson::Value* RapidJsonValueP;
    typedef rapidjson::Value const* RapidJsonValueCP;
    RapidJsonValueP m_value;
    rapidjson::MemoryPoolAllocator<>& m_allocator;

private:
    virtual ValueRefType GetType() const override { return ValueRefType::RapidJson; }
    bool IsValid() const { return m_value != nullptr; }
    virtual BeRapidJsonValue& GetMember(Utf8CP name, bool fromStatic) override {
        toObject();
        auto it = m_value->FindMember(name);
        if (it != m_value->MemberEnd())
            return *new BeRapidJsonValue(&(it->value), m_allocator);

        rapidjson::Value memberNameVal = fromStatic ? rapidjson::Value(rapidjson::StringRef(name)) : rapidjson::Value(name, m_allocator);
        return *new BeRapidJsonValue(&m_value->AddMember(memberNameVal.Move(), rapidjson::Value().Move(), m_allocator)[name], m_allocator);
    }
    virtual BeRapidJsonValue const& GetConstMember(Utf8CP name) const override {
        if (!isObject())
            return *new BeRapidJsonValue((RapidJsonValueCP) nullptr, m_allocator);

        auto it = m_value->FindMember(name);
        return *new BeRapidJsonValue((it != m_value->MemberEnd()) ? &(it->value) : nullptr, m_allocator);
    }
    virtual BeRapidJsonValue& GetArrayMember(ArrayIndex index) override {
        if (!isArray())
            toArray();
        auto arr = m_value->GetArray();
        auto size = arr.Size();
        if (size <= index) {
            arr.PushBack(rapidjson::Value().Move(), m_allocator);
            index = size;
        }
        return *new BeRapidJsonValue(&m_value->GetArray()[index], m_allocator);
    }
    virtual BeRapidJsonValue const& GetConstArrayMember(ArrayIndex index) const override {
        if (!isArray() || index >= m_value->GetArray().Size())
            return *new BeRapidJsonValue((RapidJsonValueCP) nullptr, m_allocator);

        return *new BeRapidJsonValue(&m_value->GetArray()[index], m_allocator);
    }

    virtual void SetNull() override { m_value->SetNull(); }
    virtual void removeIndex(ArrayIndex index) override {
        if (!isArray())
            return;
        ArrayIndex i = 0;
        auto arr = m_value->GetArray();
        for (auto it = arr.Begin(); it != arr.End(); ++it) {
            if (i++ == index) {
                arr.Erase(it);
                return;
            }
        }
    }
    virtual void removeMember(Utf8CP name) override {
        if (!isObject())
            return;
        m_value->GetObj().RemoveMember(name);
    }

    virtual bool isMember(Utf8CP name) const override {
      return isObject() && m_value->FindMember(name) != m_value->MemberEnd();
    }

    virtual bool hasMember(Utf8CP name) const override { return isObject() && m_value->GetObj().HasMember(name); }
    virtual bool isNull() const override { return !IsValid() || m_value->IsNull(); }
    virtual bool isBool() const override { return IsValid() && m_value->IsBool(); }
    virtual bool isNumeric() const override { return IsValid() && m_value->IsNumber(); }
    virtual bool isString() const override { return IsValid() && m_value->IsString(); }
    virtual bool isArray() const override { return IsValid() && m_value->IsArray(); }
    virtual bool isObject() const override { return IsValid() && m_value->IsObject(); }
    virtual bool isDate() const override { return false; }
    virtual bool isArrayBuffer() const override { return false; }
    virtual bool isTypedArray() const override { return false; }
    virtual bool isFunction() const override { return false; }
    virtual bool isPromise() const override { return false; }
    virtual bool isDataView() const override { return false; }
    virtual bool isBuffer() const override { return false; }
    virtual bool isExternal() const override { return false; }
    virtual void toArray() override {
        if (isNull())
            m_value->SetArray();
        BeAssert(isArray());
    }
    virtual void toObject() override {
        if (isNull())
            m_value->SetObject();
        BeAssert(isObject());
    }
    virtual uint32_t size() const override {
        if (IsValid()) {
            if (m_value->IsArray())
                return m_value->GetArray().Size();
            if (m_value->IsObject())
                return m_value->GetObj().MemberCount();
        }
        return 0;
    }
    virtual Utf8CP ToUtf8CP() const override { return isString() ? m_value->GetString() : ""; }
    virtual Utf8String ToJsonString() const override { return ToUtf8CP(); }
    virtual void operator=(double value) override { *m_value = value; }
    virtual void operator=(bool value) override { *m_value = value; }
    virtual void operator=(int64_t value) override { *m_value = value; }
    virtual void operator=(int32_t value) override { *m_value = value; }
    virtual void operator=(uint32_t value) override { *m_value = value; }
    virtual void operator=(Utf8CP value) override { m_value->SetString(value, m_allocator); }
    virtual double GetDouble(double defVal) const override { return isNumeric() ? m_value->GetDouble() : defVal; }
    virtual bool GetBoolean(bool defVal) const override {
        if (isBool())
            return m_value->GetBool();
        else if (isNumeric())
            return 0.0 != m_value->GetDouble();
        else if (isString())
            return !Utf8String::IsNullOrEmpty(ToUtf8CP());

        // Per JavaScript (and unlike JsonCpp), any non-null object or array is truthy - even an empty one.
        return isNull() ? defVal : true;
    }
    virtual int32_t GetInt(int32_t defVal) const override { return isNumeric() ? m_value->GetInt() : defVal; }
    virtual uint32_t GetUInt(uint32_t defVal) const override { return isNumeric() ? m_value->GetUint() : defVal; }
    virtual bool ForEachProperty(std::function<bool(Utf8CP name, BeJsConst)> fn) const override {
        if (isObject()) {
            auto obj = m_value->GetObj();
            for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it) {
                if (fn(it->name.GetString(), BeJsConst(it->value, m_allocator)))
                    return true;
            }
        }
        return false;
    }
    virtual bool ForEachArrayMember(std::function<bool(ArrayIndex, BeJsConst)> fn) const override {
        if (isArray()) {
            auto arr = m_value->GetArray();
            ArrayIndex i = 0;
            for (auto it = arr.Begin(); it != arr.End(); ++it) {
                if (fn(i++, BeJsConst(*it, m_allocator)))
                    return true;
            }
        }
        return false;
    }
    virtual bool ForEachArrayMemberValue(std::function<bool(ArrayIndex, BeJsValue)> fn) override {
        if (isArray()) {
            auto arr = m_value->GetArray();
            ArrayIndex i = 0;
            for (auto it = arr.Begin(); it != arr.End(); ++it) {
                if (fn(i++, BeJsValue(*it, m_allocator)))
                    return true;
            }
        }
        return false;
    }
    virtual Utf8String Stringify(StringifyFormat format = StringifyFormat::Default) const override {
        if (!IsValid())
            return "";
        rapidjson::StringBuffer buffer;
        if (format == StringifyFormat::Indented) {
            rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
            writer.SetIndent(' ', 3);
            m_value->Accept(writer);
        } else {
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            m_value->Accept(writer);
        }
        return buffer.GetString();
    }

    explicit BeRapidJsonValue(RapidJsonValueP val, rapidjson::MemoryPoolAllocator<>& alloc) : m_value(val), m_allocator(alloc) {}
    explicit BeRapidJsonValue(RapidJsonValueCP val, rapidjson::MemoryPoolAllocator<>& alloc) : m_value(const_cast<RapidJsonValueP>(val)), m_allocator(alloc) {}
    friend struct BeJsValue;
    friend struct BeJsConst;
    friend struct BeJsDocument;
};

//=======================================================================================
// A root-level "document" that is a BeJsValue. Internally it holds a rapidjson::Document object.
// This class can be used to create a JSON object on the stack. Note, this class is not copyable, and
// the lifetime its instances must outlive any references to / uses of it or its members.
// @bsiclass
//=======================================================================================
struct BeJsDocument : BeJsValue {
private:
    rapidjson::Document m_doc;
    BeJsDocument& operator=(BeJsDocument const& rhs) = delete;
public:
    // allow move but not copy.
    BeJsDocument(BeJsDocument&& other) noexcept : m_doc(std::move(other.m_doc)) { m_val = new BeRapidJsonValue(&m_doc, m_doc.GetAllocator());  }
    // construct a blank BeJsDocument
    BeJsDocument() : BeJsValue() { m_val = new BeRapidJsonValue(&m_doc, m_doc.GetAllocator()); }
    // construct a "string" BeJsDocument. The bool parameter (whose value is ignored) indicates that the string is not
    // JSON, and that the BeJsDocument should just be of type `string` with the supplied value.
    BeJsDocument(Utf8CP stringValue, bool) : BeJsDocument() { m_doc.SetString(stringValue, m_doc.GetAllocator()); }
    // construct a "string" BeJsDocument. The bool parameter (whose value is ignored) indicates that the string is not
    // JSON, and that the BeJsDocument should just be of type `string` with the supplied value.
    BeJsDocument(std::string const& stringValue, bool) : BeJsDocument(stringValue.c_str(), true) {}
    // construct a BeJsDocument initialized from stringified JSON
    BeJsDocument(Utf8CP jsonString) : BeJsDocument() { Parse(jsonString); }
    // construct a BeJsDocument initialized from stringified JSON
    BeJsDocument(std::string const& jsonString) : BeJsDocument(jsonString.c_str()) {}
    // replace the content of this document with the parsed value of stringified JSON
    void Parse(Utf8CP jsonString) { m_doc.Parse(jsonString); }
    // replace the content of this document with the parsed value of stringified JSON
    void Parse(std::string const& jsonString) { Parse(jsonString.c_str()); }

    bool hasParseError() { return m_doc.HasParseError(); }

    // Obtain a global immutable null document.
    static BeJsConst Null()
        {
        static BeJsDocument s_nullDoc;
        return s_nullDoc;
        }
};

inline bool BeJsConst::isAlmostEqual(BeJsConst other, double absTol, double relTol) const {
    if (isNull())
        return other.isNull();
    if (isBool())
        return other.isBool() && asBool() == other.asBool();
    if (isNumeric())
        return other.isNumeric() && areAlmostEqual(asDouble(), other.asDouble(), absTol, relTol);
    if (isBinary()) {
        if (!other.isBinary())
            return false;
        bvector<Byte> a, b;
        GetBinary(a);
        other.GetBinary(b);
        return b == a;
    }
    if (isString())
        return other.isString() && (0 == strcmp(asCString(), other.asCString()));
    if (isJsOnlyType())
        return other.isJsOnlyType() && ToJsonString() == other.ToJsonString();
    if (isArray()) {
        return other.isArray() &&
               (size() == other.size()) &&
               (false == ForEachArrayMember([&](ArrayIndex i, BeJsConst member) { return !member.isAlmostEqual(other[i], absTol, relTol); }));
    }
    if (isObject()) {
        return other.isObject() &&
               (size() == other.size()) &&
               (false == ForEachProperty([&](Utf8CP name, BeJsConst member) { return !member.isAlmostEqual(other[name], absTol, relTol); }));
    }
    return false;
}

inline BeJsValue::BeJsValue(RapidJsonDocumentR val) : BeJsConst(*new BeRapidJsonValue(&val, val.GetAllocator())) {}
inline BeJsValue::BeJsValue(RapidJsonValueR val, rapidjson::MemoryPoolAllocator<>& alloc) : BeJsConst(*new BeRapidJsonValue(&val, alloc)) {}
inline BeJsValue::BeJsValue(JsonValueR val) : BeJsConst(*new BeJsonCppValue(val)) {}

inline BeJsConst::BeJsConst(RapidJsonDocumentCR val) : m_val(new BeRapidJsonValue(&val, const_cast<RapidJsonDocumentR>(val).GetAllocator())) {}
inline BeJsConst::BeJsConst(RapidJsonValueCR val, rapidjson::MemoryPoolAllocator<>& alloc) : m_val(new BeRapidJsonValue(&val, alloc)) {}
inline BeJsConst::BeJsConst(JsonValueCR val) : m_val(new BeJsonCppValue(val)) {}
inline void BeJsConst::SaveTo(BeJsValue dest) const { dest.From(*this); }

END_BENTLEY_NAMESPACE
