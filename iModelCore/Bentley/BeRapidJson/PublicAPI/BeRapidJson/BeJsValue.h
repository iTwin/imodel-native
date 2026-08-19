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
// The BeJsValue <-> Json::Value bridge is OPT-IN. imodel-native does not use JsonCpp, so it
// defines nothing and gets none of it. A downstream repository that still needs the bridge must
// both define USE_JSONCPP and take a SubPart dependency on iModelCore/libsrc/jsoncpp/BeJsonCpp.
//
// This header is public and its definitions are inline, so USE_JSONCPP must be set consistently
// for every part in a build: two parts that disagree get different BeJsConst/BeJsValue
// definitions and an ODR violation.
#ifdef USE_JSONCPP
#include <json/json.h>
#endif // USE_JSONCPP
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <limits>
#include <cmath>
#include <functional>
#include <optional>
// These were previously reaching this header transitively through <json/json.h>. libc++ still
// leaks them in via the headers above, but the MSVC STL does not, so declare them explicitly.
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

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
// A name with static storage duration. Implementations may store the pointer rather than
// copying the characters, so the referenced string must outlive every value that uses it.
// @bsiclass
//=======================================================================================
struct BeJsStaticString {
private:
    Utf8CP m_str;

public:
    constexpr explicit BeJsStaticString(Utf8CP str) : m_str(str) {}
    constexpr operator Utf8CP() const { return m_str; }
    constexpr Utf8CP c_str() const { return m_str; }
};

// Declares a `json_<name>()` accessor returning the member name as a static string.
// JsonCpp declares a Json::StaticString-based version of this macro. Redefine it here so a
// translation unit gets the same expansion no matter which header it included first;
// BeJsStaticString converts to Utf8CP, so JsonCpp call sites keep compiling.
#undef BE_JSON_NAME
#define BE_JSON_NAME(__val__) static constexpr BentleyApi::BeJsStaticString json_##__val__() {return BentleyApi::BeJsStaticString(#__val__);}

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
    // Not pure virtual: an out-of-tree implementation of this interface would otherwise fail to
    // compile. The default is lossy above INT64_MAX; in-tree implementations override it to be exact.
    virtual void operator=(uint64_t value) { *this = static_cast<int64_t>(value); }
    virtual void operator=(Utf8CP value) = 0;
    virtual double GetDouble(double defVal = 0) const = 0;
    virtual bool GetBoolean(bool defVal) const = 0;
    virtual int32_t GetInt(int32_t defVal) const = 0;
    virtual uint32_t GetUInt(uint32_t defVal) const = 0;
    virtual int64_t GetInt64(int64_t defVal) const = 0;
    virtual uint64_t GetUInt64(uint64_t defVal) const = 0;
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

#ifdef USE_JSONCPP
    BeJsConst(JsonValueCR);
#endif
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
        if (isNull())
            return defVal;
        if (isString()) {
            uint64_t val = defVal;
            auto str = ToUtf8CP();
            if (str[0] == '-') // negative numbers are not valid
                return defVal;
            if (strchr(str, '.') != nullptr) // decimal numbers are not valid
                return defVal;
            auto fmt = (str[0] == '0' && (str[1] == 'X' || str[1] == 'x')) ? "%" SCNx64 : "%" SCNu64;
            Utf8String::Sscanf_safe(str, fmt, &val);
            return val;
        }
        return m_val->GetUInt64(defVal);
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
    // Stringify this value and all its children so that the result is BYTE-IDENTICAL to what
    // Bentley's JsonCpp fork produced for the same data (Json::Value::ToString()).
    // Two things differ between JsonCpp and rapidjson and both are reproduced here:
    //   1. object members are emitted in alphabetical order, not insertion order;
    //   2. doubles are spelled with "%#.17g" and trailing zeros trimmed to one (so 0.3 becomes
    //      "0.29999999999999999" and 1.5 becomes "1.50"), not rapidjson's shortest round-trip form.
    // @note This exists ONLY for JSON that is PERSISTED and later hashed or compared as text
    // (e.g. ec_Format.NumericSpec/CompositeSpec, ec_Enumeration.EnumValues). Those bytes are
    // covered by PRAGMA checksum(ecdb_schema), which SchemaSync compares ACROSS briefcases, so a
    // briefcase written by old code and one written by new code must agree exactly.
    // Anything that merely round-trips through a parser should use Stringify() instead: this is
    // slower, uglier, and is not needed for correctness.
    Utf8String StringifyLegacy() const;
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
#ifdef USE_JSONCPP
    BeJsValue(JsonValueR);
#endif
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
    BeJsValue operator[](BeJsStaticString const& key) { return BeJsValue(m_val->GetMember(key, true)); }
#ifdef USE_JSONCPP
    BeJsValue operator[](Json::StaticString const& key) { return BeJsValue(m_val->GetMember(key, true)); }
#endif
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
    // Assign a uint64 to this value.
    // @note this must be an empty value or a primitive and the value to be assigned must be less than Number.MAX_SAFE_INTEGER
    // @see https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/MAX_SAFE_INTEGER
    BeJsValue& operator=(uint64_t value) {
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
    // Set this value to a JSON *string*.
    // @note On a BeJsDocument, prefer this over `doc = str`: assigning a string to a *document*
    // means "parse this JSON" (see BeJsDocument::operator=), which is almost never what you want
    // when the string is a plain value such as an id or a base64 blob.
    void SetString(Utf8CP value) { (*m_val) = value; }
    // Set this value to a JSON *string*. See the Utf8CP overload for why this exists.
    void SetString(std::string const& value) { SetString(value.c_str()); }
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
    // JavaScript's Number.MIN_SAFE_INTEGER / MAX_SAFE_INTEGER, not the int64_t limits.
    static constexpr int64_t s_minSafeInteger = -9007199254740991;
    static constexpr int64_t s_maxSafeInteger = 9007199254740991;
    static constexpr int32_t s_minInt32 = std::numeric_limits<int32_t>::min();
    static constexpr int32_t s_maxInt32 = std::numeric_limits<int32_t>::max();
    static constexpr uint32_t s_maxUInt32 = std::numeric_limits<uint32_t>::max();

    static bool IsIntegral(double d) {
        double integralPart;
        return std::modf(d, &integralPart) == 0.0;
    }

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
            if (val > s_minSafeInteger && IsIntegral(val)) {
                if (val < s_minInt32) {
                    *this = (int64_t)val;
                    return;
                }
                if (val < s_maxInt32) {
                    *this = (int32_t)val;
                    return;
                }
                if (val < s_maxUInt32) {
                    *this = (uint32_t)val;
                    return;
                }
                if (val < s_maxSafeInteger) {
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

#ifdef USE_JSONCPP
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
    virtual void operator=(uint64_t value) override { m_value = Json::Value(static_cast<Json::UInt64>(value)); }
    virtual void operator=(int32_t value) override { m_value = value; }
    virtual void operator=(uint32_t value) override { m_value = value; }
    virtual void operator=(Utf8CP value) override { m_value = value; }
    virtual double GetDouble(double defVal) const override { return m_value.asDouble(defVal); }
    virtual bool GetBoolean(bool defVal) const override { return m_value.asBool(defVal); }
    virtual int32_t GetInt(int32_t defVal) const override { return m_value.asInt(defVal); }
    virtual uint32_t GetUInt(uint32_t defVal) const override { return m_value.asUInt(defVal); }
    virtual int64_t GetInt64(int64_t defVal) const override { return m_value.asInt64(defVal); }
    virtual uint64_t GetUInt64(uint64_t defVal) const override { return m_value.asUInt64(defVal); }
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

#endif // USE_JSONCPP

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
    virtual void operator=(uint64_t value) override { *m_value = value; }
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
    // Convert a double to an integral type the way JsonCpp's asInt/asUInt/asInt64 did (truncate toward
    // zero), but clamp first: an out-of-range double->integer cast is undefined behavior in C++.
    template <typename T> static T IntegralFromDouble(double d) {
        if (std::isnan(d))
            return 0;
        if (d <= static_cast<double>(std::numeric_limits<T>::lowest()))
            return std::numeric_limits<T>::lowest();
        if (d >= static_cast<double>(std::numeric_limits<T>::max()))
            return std::numeric_limits<T>::max();
        return static_cast<T>(d);
    }
    // NOTE: rapidjson's GetInt/GetUint/GetInt64 assert that the value is stored as that exact type and
    // then read the raw union field. RAPIDJSON_ASSERT compiles away in a release build, so calling
    // GetInt() on a value stored as a double returns the low 32 bits of its IEEE-754 representation --
    // e.g. 3.1415 comes back as -1065151889. JsonCpp instead converted properly, and callers such as
    // JsonECSqlBinder (which only guards with isNumeric()) depend on that. Always check the stored type
    // before taking the fast path; otherwise convert through GetDouble(), which is safe for any number.
    virtual int32_t GetInt(int32_t defVal) const override {
        if (!isNumeric())
            return defVal;
        return m_value->IsInt() ? m_value->GetInt() : IntegralFromDouble<int32_t>(m_value->GetDouble());
    }
    virtual uint32_t GetUInt(uint32_t defVal) const override {
        if (!isNumeric())
            return defVal;
        return m_value->IsUint() ? m_value->GetUint() : IntegralFromDouble<uint32_t>(m_value->GetDouble());
    }
    virtual int64_t GetInt64(int64_t defVal) const override {
        if (!isNumeric())
            return defVal;
        return m_value->IsInt64() ? m_value->GetInt64() : IntegralFromDouble<int64_t>(m_value->GetDouble());
    }
    virtual uint64_t GetUInt64(uint64_t defVal) const override {
        if (!isNumeric())
            return defVal;
        if (m_value->IsUint64()) {
            return m_value->GetUint64();
        }
        // Unlike the accessors above, a non-integral double deliberately yields defVal rather than a
        // truncated value: this accessor is used to round-trip 64-bit ids, where silently losing the
        // fractional part would hide a real error.
        if (m_value->IsDouble() && IsLosslessUint64(m_value->GetDouble())) {
            return static_cast<uint64_t>(m_value->GetDouble());
        }
        if (m_value->IsFloat() && IsLosslessUint64(m_value->GetFloat())) {
            return static_cast<uint64_t>(m_value->GetFloat());
        }
        return defVal;
    }
    bool IsLosslessUint64(double d) const {
        if (d < 0.0 || d > static_cast<double>(std::numeric_limits<uint64_t>::max())) {
            return false;
        }
        return d == std::floor(d);
    }
    bool IsLosslessUint64(float f) const {
        if (f < 0.0 || f > static_cast<float>(std::numeric_limits<uint64_t>::max())) {
            return false;
        }
        return f == std::floor(f);
    }
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
    void PurgeNulls(rapidjson::Value& val);

public:
    // allow move but not copy.
    BeJsDocument(rapidjson::Document&& doc) noexcept : m_doc(std::move(doc)) { m_val = new BeRapidJsonValue(&m_doc, m_doc.GetAllocator()); }
    BeJsDocument(BeJsDocument&& other) noexcept : BeJsDocument(std::move(other.m_doc)) {}
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
    // allow to move BeJsDocument
    BeJsDocument& operator=(BeJsDocument&& rhs)
        {
        m_doc.Swap(rhs.m_doc);
        m_val = new BeRapidJsonValue(&m_doc, m_doc.GetAllocator());
        return *this;
        }
    // declaring operator= above otherwise hides the inherited scalar assignments, so `doc = 1.0;` would not compile.
    using BeJsValue::operator=;
    //! Assigning a string to a *document* means "parse this JSON".
    //!
    //! These two overloads are load-bearing. Before `using BeJsValue::operator=` was added above, the
    //! move-assignment operator hid every inherited assignment, so `doc = someUtf8String;` could only
    //! compile by going through the implicit BeJsDocument(Utf8CP) / BeJsDocument(std::string const&)
    //! converting constructors - which parse. The `using` declaration makes
    //! BeJsValue::operator=(Utf8CP) visible, and it would otherwise win overload resolution and
    //! silently turn the document into a JSON *string* instead of a parsed object.
    //! That regression is invisible at the assignment; it only shows up much later as members that
    //! read back as empty (it broke TxnManager's changeset health statistics map, which stores each
    //! changeset's stats as `map[id] = stats.Stringify()`).
    BeJsDocument& operator=(Utf8CP jsonString) { Parse(jsonString); return *this; }
    BeJsDocument& operator=(std::string const& jsonString) { Parse(jsonString.c_str()); return *this; }
    //! Replace the content of this document with the parsed value of stringified JSON.
    //!
    //! kParseFullPrecisionFlag is deliberate and load-bearing - DO NOT REMOVE IT.
    //! RapidJson's default number parser uses a fast path that can land 1 ULP away from strtod;
    //! JsonCpp (which this class replaced) always used strtod. Measured on 17 realistic
    //! JsonCpp-spelled doubles, the default parser disagreed with strtod on 5 of them, e.g.
    //! "9.9999999999999995e-21" and "2.2250738585072011e-308".
    //! Persisted JSON written by JsonCpp is still out there and nothing rewrites it, so a default
    //! parse silently returns a slightly different number than the one that was stored. That is
    //! invisible - no compile error, no parse error - and it has already caused real bugs
    //! (geometry round-trip comparisons, and tile content Ids, which hash the project extents).
    //! The flag costs roughly 2x on number-heavy payloads and much less on typical mixed JSON;
    //! correctness of persisted data is worth more than that.
    void Parse(Utf8CP jsonString) { m_doc.Parse<rapidjson::kParseFullPrecisionFlag>(jsonString); }
    //! Replace the content of this document with the parsed value of stringified JSON.
    void Parse(std::string const& jsonString) { Parse(jsonString.c_str()); }

    bool hasParseError() { return m_doc.HasParseError(); }
    void PurgeNulls() { PurgeNulls(m_doc); }
    // Obtain a global immutable null document.
    static BeJsConst Null()
        {
        static BeJsDocument s_nullDoc;
        return s_nullDoc;
        }
};

// Format a double exactly the way Bentley's JsonCpp fork did. Mirrors valueToString(double) in
// iModelCore/libsrc/jsoncpp/src/lib_json/json_writer.cpp, quirks included: "%#.17g" then trailing
// zeros trimmed back to a single one, which yields "1.0" for 1.0 but "1.50" for 1.5.
// See BeJsConst::StringifyLegacy.
inline Utf8String BeJsLegacyDoubleToString(double value) {
    if (std::isnan(value) || std::isinf(value))
        return "null";

    char buffer[40];
    snprintf(buffer, sizeof(buffer), "%#.17g", value);

    char* ch = buffer + strlen(buffer) - 1;
    if (*ch == '.') { // '#' guarantees a decimal point; never leave it dangling
        *(ch + 1) = '0';
        *(ch + 2) = 0;
        return buffer;
    }
    if (*ch != '0')
        return buffer; // nothing to truncate

    while (ch > buffer && *ch == '0')
        --ch;
    char* lastNonZero = ch;
    while (ch >= buffer) {
        if (*ch >= '0' && *ch <= '9') {
            --ch;
            continue;
        }
        if (*ch == '.') {
            *(lastNonZero + 2) = 0; // truncate the run of zeros, but keep one
            return buffer;
        }
        return buffer; // an exponent or sign: leave the text alone
    }
    return buffer;
}

// Append `in` to `out` as JsonCpp-compatible JSON text. See BeJsConst::StringifyLegacy.
inline void BeJsLegacyStringify(Utf8StringR out, BeJsConst in) {
    if (in.isObject()) {
        bvector<Utf8String> names;
        in.ForEachProperty([&](Utf8CP name, BeJsConst) { names.push_back(name); return false; });
        std::sort(names.begin(), names.end());
        out.append("{");
        bool first = true;
        for (auto const& name : names) {
            if (!first)
                out.append(",");
            first = false;
            BeJsDocument key; // borrow rapidjson's string escaping, which already matches JsonCpp's
            key.SetString(name);
            out.append(key.Stringify()).append(":");
            BeJsLegacyStringify(out, in[name.c_str()]);
        }
        out.append("}");
        return;
    }
    if (in.isArray()) {
        out.append("[");
        bool first = true;
        in.ForEachArrayMember([&](BeJsValue::ArrayIndex, BeJsConst member) {
            if (!first)
                out.append(",");
            first = false;
            BeJsLegacyStringify(out, member);
            return false;
            });
        out.append("]");
        return;
    }
    if (in.isNull()) {
        out.append("null");
        return;
    }
    if (in.isBool()) {
        out.append(in.asBool() ? "true" : "false");
        return;
    }
    if (in.isNumeric()) {
        // rapidjson always spells a double with a '.' or an exponent, and an integer with neither.
        Utf8String text = in.Stringify();
        if (Utf8String::npos != text.find_first_of(".eE"))
            out.append(BeJsLegacyDoubleToString(in.asDouble()));
        else
            out.append(text);
        return;
    }
    out.append(in.Stringify()); // strings, dates and binary
}

inline Utf8String BeJsConst::StringifyLegacy() const {
    if (isNull())
        return "";
    Utf8String out;
    BeJsLegacyStringify(out, *this);
    return out;
}

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

inline BeJsConst::BeJsConst(RapidJsonDocumentCR val) : m_val(new BeRapidJsonValue(&val, const_cast<RapidJsonDocumentR>(val).GetAllocator())) {}
inline BeJsConst::BeJsConst(RapidJsonValueCR val, rapidjson::MemoryPoolAllocator<>& alloc) : m_val(new BeRapidJsonValue(&val, alloc)) {}

#ifdef USE_JSONCPP
inline BeJsValue::BeJsValue(JsonValueR val) : BeJsConst(*new BeJsonCppValue(val)) {}
inline BeJsConst::BeJsConst(JsonValueCR val) : m_val(new BeJsonCppValue(val)) {}
#endif

inline void BeJsConst::SaveTo(BeJsValue dest) const { dest.From(*this); }

inline void BeJsDocument::PurgeNulls(rapidjson::Value& val) {
    if (val.IsObject()) {
        auto it = val.MemberBegin();
        while (it != val.MemberEnd()) {
            if (it->value.IsNull()) {
                it = val.EraseMember(it);
            } else {
                if (it->value.IsObject() || it->value.IsArray())
                    PurgeNulls(it->value);
                ++it;
            } 
        }
    }
    if (val.IsArray()) {
        auto it = val.Begin();
        while (it != val.End()) {
            if (it->IsNull()) {
                it = val.Erase(it);
            } else {
                if (it->IsObject() || it->IsArray())
                    PurgeNulls(*it);
                ++it;
            }
        }
    }
}

struct BeJsPath final {
private:
    struct Accessor {
        std::string token;
        int index = -1;
    };
    static std::vector<Accessor> Parse(const std::string& path) {
        std::vector<Accessor> tokens;
        if (path.empty() || path[0] != '$') {
            return tokens;
        }
        size_t start = 1;
        size_t end = 1;
        while (end < path.size()) {
            if (path[end] == '.') {
                if (end > start) {
                    tokens.push_back({path.substr(start, end - start)});
                }
                start = end + 1;
            } else if (path[end] == '[') {
                if (end > start) {
                    tokens.push_back({path.substr(start, end - start)});
                }
                start = end + 1;
                end++;
                while (end < path.size() && path[end] != ']') {
                    end++;
                }
                if (end < path.size()) {
                    tokens.push_back({path.substr(start, end - start), std::stoi(path.substr(start, end - start))});
                    start = end + 1;
                }
            }
            end++;
        }
        if (end > start) {
            tokens.push_back({path.substr(start, end - start)});
        }
        return tokens;
    }
    template <typename T>
    static std::optional<T> Get(T obj, std::vector<Accessor>& path) {
        static_assert(std::is_base_of<BeJsConst, T>::value, "T must be derived from BeJsConst");
        if (path.empty()) {
            return obj;
        }

        auto current = path.back();
        path.pop_back();

        if (current.index > 0) {
            if (!obj.isArray()) {
                return std::nullopt;
            }
            if (current.index >= (int)obj.size()) {
                return std::nullopt;
            }
            return Get(obj[current.index], path);
        } else {
            if (!obj.isObject()) {
                return std::nullopt;
            }
            if (!obj.isMember(current.token.c_str())) {
                return std::nullopt;
            }
            if (!obj[current.token.c_str()].isObject() && !obj[current.token.c_str()].isArray()) {
                return std::nullopt;
            }
            return Get(obj[current.token], path);
        }
    }

public:
    BeJsPath() = delete;
    template <typename T>
    static std::optional<T> Extract(T obj, const std::string& path) {
        auto tokens = Parse(path);
        std::reverse(tokens.begin(), tokens.end());
        return Get(obj, tokens);
    }

};
END_BENTLEY_NAMESPACE
