/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/JSValue/JSValue.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
#include <Bentley/Bentley.h>
#include <Bentley/BeAssert.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <json/json.h>
#include <stdio.h>

BEGIN_BENTLEY_NAMESPACE

typedef struct JSValueHandle__* JSValueHandle;

//=======================================================================================
//! JSValueType
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
enum class JSValueType : Byte
    {
    String,
    Boolean,
    Integer,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Double,
    Object,
    Array,
    Null
    };

//=======================================================================================
//! JSStatus
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
enum class JSStatus
    {
    Success,
    Error,
    MemberNotFound,
    IndexOutOfRange,
    NotImplemented,
    IncorrectType,
    NullArgument
    };

struct JSFactory;
struct JSNumber;
struct JSString;
struct JSArray;
struct JSObject;
struct JSBoolean;
//=======================================================================================
//! JSValue
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct JSValue
    {
    private:
        JSValueHandle m_value;
        JSFactory* m_jFactory;
        virtual JSStatus _ToJson(Json::Value& v) const;
    public:
        JSValue();
        JSValue(JSValue const& rhs);
        JSValue(JSFactory& factory, JSValueHandle v);
        ~JSValue();
        JSFactory& Factory() const;
        JSValueHandle Handle() const;
        JSValueType Type() const;
        bool IsEmpty() const;
        bool IsNull() const;
        bool IsInt32() const;
        bool IsUInt32() const;
        bool IsInt64() const;
        bool IsUInt64() const;
        bool IsDouble() const;
        bool IsObject() const;
        bool IsArray() const;
        bool IsString() const;
        bool IsNumber() const;
        bool IsBoolean() const;
        operator JSValueHandle ();
        JSNumber AsNumber() const;
        JSString AsString() const;
        JSObject AsObject() const;
        JSBoolean AsBoolean() const;
        JSArray AsArray() const;
        bool ReferenceEquals(JSValue rhs) const;
        Json::Value ToJson() const;
        static JSValue FromJson(JSFactory& factory, Json::Value const& json);
        static JSValue Null(JSFactory& factory);
    };

//=======================================================================================
//! JSNumber
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct JSNumber final: JSValue
    {
    private:
        JSStatus _ToJson(Json::Value& v) const override;
    public:
        JSNumber(JSNumber const& rhs) : JSValue(rhs.Factory(), rhs.Handle()) {}
        JSNumber(JSFactory& factory, JSValueHandle v) :JSValue(factory, v) {}
        int32_t Int32Value() const;
        uint32_t UInt32Value() const;
        int64_t Int64Value() const;
        uint64_t UInt64Value() const;
        double DoubleValue() const;
        operator int32_t () const { return Int32Value(); }
        operator uint32_t () const { return UInt32Value(); }
        operator int64_t () const { return Int64Value(); }
        operator uint64_t () const { return UInt64Value(); }
        operator double() const { return DoubleValue(); }
        static JSNumber New(JSFactory& factory, int32_t i32);
        static JSNumber New(JSFactory& factory, uint32_t i32);
        static JSNumber New(JSFactory& factory, int64_t i64);
        static JSNumber New(JSFactory& factory, uint64_t i64);
        static JSNumber New(JSFactory& factory, double d);
    };

//=======================================================================================
//! JSBoolean
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct JSBoolean final: JSValue
    {
    private:
        JSStatus _ToJson(Json::Value& v) const override;
        
    public:
        JSBoolean(JSBoolean const& rhs) : JSValue(rhs.Factory(), rhs.Handle()) {}
        JSBoolean(JSFactory& factory, JSValueHandle v) : JSValue(factory, v) {}
        bool Value() const;
        operator bool() const { return Value(); }
        static JSBoolean New(JSFactory& factory, bool b);
    };

//=======================================================================================
//! JSString
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct JSString final : JSValue
    {
    private:
        JSStatus _ToJson(Json::Value& v) const override;
        
    public:
        JSString(JSString const& rhs) : JSValue(rhs.Factory(), rhs.Handle()) {}
        JSString(JSFactory& factory, JSValueHandle v) :JSValue(factory, v) {}
        Utf8String Value() const;
        uint32_t Length() const;
        operator Utf8String  () { return Value(); }
        static JSString New(JSFactory& factory, Utf8CP str);
    };
//
//=======================================================================================
//! JSObject
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct  JSObject final: JSValue
    {
    private:
        JSStatus _ToJson(Json::Value& v) const override;
        
    public:
        struct LValue final
            {
            friend struct JSObject;
            private:
                JSFactory& m_jFactory;
                JSValueHandle m_obj;
                Utf8String m_key;
                LValue() = delete;
                LValue(JSFactory& factory, JSValueHandle object, Utf8String key)
                    :m_jFactory(factory), m_obj(object), m_key(key)
                    {}
            public:
                /// Converts an L-value to a value.
                operator JSValue() const;
                operator JSNumber() const;
                operator JSString() const;
                operator JSArray() const;
                operator JSObject() const;
                operator JSBoolean() const;
                LValue& operator = (JSValue value);
            };


        JSObject(JSObject const& rhs) : JSValue(rhs.Factory(), rhs.Handle()) {}
        JSObject(JSFactory& factory, JSValueHandle v) :JSValue(factory, v) {}
        JSValue Get(Utf8CP key) const;
        JSValue Get(Utf8StringCR key) const;
        void Set(Utf8CP key, JSValue value);
        void Set(Utf8StringCR key, JSValue value);
        JSArray GetMemberNames() const;
        bool HasMember(Utf8CP key) const;
        bool HasMember(Utf8StringCR key) const;
        LValue operator [](Utf8CP key);
        LValue operator [](Utf8StringCR key);
        LValue operator [](JSString key);
        uint32_t Count() const;
        static JSObject New(JSFactory& factory);
    };

//=======================================================================================
//! JSArray
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct JSArray final: JSValue
    {
    private:
        JSStatus _ToJson(Json::Value& v) const override;
        
    struct LValue final
        {
        friend struct JSArray;
        private:
            JSFactory& m_jFactory;
            JSValueHandle m_obj;
            uint32_t m_key;
            LValue() = delete;
            LValue(JSFactory& factory, JSValueHandle object, uint32_t key)
                :m_jFactory(factory), m_obj(object), m_key(key)
                {}
        public:
            /// Converts an L-value to a value.
            operator JSValue() const;
            operator JSNumber() const;
            operator JSString() const;
            operator JSArray() const;
            operator JSObject() const;
            operator JSBoolean() const;
            LValue& operator = (JSValue value);
        };
    public:
        JSArray(JSArray const& rhs) : JSValue(rhs.Factory(), rhs.Handle()) {}
        JSArray(JSFactory& factory, JSValueHandle v) :JSValue(factory, v) {}
        JSValue Get(uint32_t index) const;
        void Set(uint32_t index, JSValue value);
        LValue operator [](uint32_t index);
        LValue operator [](JSNumber index);
        uint32_t Length() const;
        static JSArray New(JSFactory& factory, uint32_t size);
    };

//=======================================================================================
//! JSFactory
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct JSFactory : RefCountedBase
    {
    public:
        JSFactory() {};
        virtual ~JSFactory() {};
        virtual JSStatus CreateBoolean(JSValueHandle& val, bool b) = 0;
        virtual JSStatus CreateString(JSValueHandle& val, Utf8CP s) = 0;
        virtual JSStatus CreateUInt32(JSValueHandle& val, uint32_t i32) = 0;
        virtual JSStatus CreateInt32(JSValueHandle& val, int32_t i32) = 0;
        virtual JSStatus CreateUInt64(JSValueHandle& val, uint64_t i64) = 0;
        virtual JSStatus CreateInt64(JSValueHandle& val, int64_t i64) = 0;
        virtual JSStatus CreateDouble(JSValueHandle& val, double d) = 0;
        virtual JSStatus CreateObject(JSValueHandle& val) = 0;
        virtual JSStatus CreateArray(JSValueHandle& val, uint32_t size) = 0;
        virtual JSStatus GetBoolean(bool& b, JSValueHandle val) const = 0;
        virtual JSStatus GetString(Utf8String& s, JSValueHandle val) const = 0;
        virtual JSStatus GetInt32(int32_t& i32, JSValueHandle val) const = 0;
        virtual JSStatus GetUInt32(uint32_t& i32, JSValueHandle val) const = 0;
        virtual JSStatus GetInt64(int64_t& i64, JSValueHandle val) const = 0;
        virtual JSStatus GetUInt64(uint64_t& i64, JSValueHandle val) const = 0;
        virtual JSStatus GetDouble(double& d, JSValueHandle val) const = 0;
        virtual JSStatus GetMemberNames(JSValueHandle& members, JSValueHandle val) const = 0;
        virtual JSStatus HasMember(bool& found, JSValueHandle val, Utf8CP key) const = 0;
        virtual JSStatus GetMember(JSValueHandle& rt, JSValueHandle obj, Utf8CP key) const = 0;
        virtual JSStatus GetMember(JSValueHandle& rt, JSValueHandle obj, uint32_t index) const = 0;
        virtual JSStatus GetMember(JSValueHandle& rt, JSValueHandle obj, JSValueHandle index) const = 0;
        virtual JSStatus GetSize(uint32_t& n, JSValueHandle val) const = 0;
        virtual JSStatus SetMember(JSValueHandle obj, Utf8CP key, JSValueHandle val) = 0;
        virtual JSStatus SetMember(JSValueHandle obj, uint32_t index, JSValueHandle val) = 0;
        virtual JSStatus SetMember(JSValueHandle obj, JSValueHandle index, JSValueHandle val) = 0;
        virtual JSStatus GetType(JSValueType& t, JSValueHandle val) const = 0;
        virtual JSValueHandle GetNull() const = 0;
        virtual JSStatus BeginScope(JSValueHandle val) const = 0;
        virtual JSStatus EndScope(JSValueHandle val) const = 0;
        virtual bool NotifyScopeChanges() const = 0;
    };

//=======================================================================================
//! GCJValueFactory Garbagec collected JSValue Factory.
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct GCJValueFactory final
    {
    public:
        
        struct Options final
            {
            friend struct GCJValueFactory;
            private:
                uint32_t m_maxStringValueCache;
                uint32_t m_maxArrayValueCache;
                uint32_t m_maxObjectValueCache;
                uint32_t m_maxPrimitiveValueCache;

            public:
                Options()
                    :m_maxStringValueCache(200), m_maxArrayValueCache(200), m_maxObjectValueCache(200), m_maxPrimitiveValueCache(200)
                    {}
                Options& SetMaxStringValueCache(uint32_t n) { m_maxStringValueCache = n; return *this; }
                Options& SetMaxArrayValueCache(uint32_t n) { m_maxArrayValueCache = n; return *this; }
                Options& SetMaxObjectValueCache(uint32_t n) { m_maxObjectValueCache = n; return *this; }
                Options& SetMaxPrimitiveValueCache(uint32_t n) { m_maxPrimitiveValueCache = n; return *this; }
            };


        static RefCountedPtr<JSFactory> CreateInstance(Options const& opt = Options());
    };

END_BENTLEY_NAMESPACE
