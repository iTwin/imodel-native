/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/JS/Value.h $
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

BEGIN_BENTLEY_NAMESPACE
namespace Js 
{

typedef struct ValueHandle__* ValueHandle;

//=======================================================================================
//! ValueType
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
enum class ValueType : Byte
    {
    String,
    Boolean,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Double,
    Object,
    Array,
    Null,
    Undefined
    };

//=======================================================================================
//! Status
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
enum class Status
    {
    Success = 0,
    Error,
    MemberNotFound,
    IndexOutOfRange,
    NotImplemented,
    IncorrectType,
    NullArgument
    };

//=======================================================================================
//! IntegerOptions
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
enum class IntegerOptions
    {
    Auto,
    Hex,
    Decimal
    };

struct IFactory;
struct Number;
struct String;
struct Array;
struct Object;
struct Boolean;
//=======================================================================================
//! Value
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct Value
    {
    private:
        ValueHandle m_value;
        IFactory* m_factory;

    public:
        Value();
        Value(Value const& rhs);
        Value(IFactory& factory, ValueHandle v);
        ~Value();        
        IFactory& Factory() const;
        ValueHandle Handle() const;
        ValueType Type() const;
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
        bool IsUndefined() const;
        operator ValueHandle ();
        Number AsNumber() const;
        String AsString() const;
        Object AsObject() const;
        Boolean AsBoolean() const;
        Array AsArray() const;
        bool ReferenceEquals(Value rhs) const;
        Json::Value ToJson() const;
        static Value FromJson(IFactory& factory, Json::Value const& json);
        static Value Null(IFactory& factory);
        static Value Undefined(IFactory& factory);
        
        int32_t ToInt32(int32_t defaultVal = 0) const;
        uint32_t ToUInt32(uint32_t defaultVal = 0) const;
        int64_t ToInt64(int64_t defaultVal = 0) const;
        uint64_t ToUInt64(uint64_t defaultVal = 0) const;
        double ToDouble(double defaultVal = 0) const;
        bool ToBoolean(bool defaultVal = 0) const;
        Utf8String ToString(Utf8CP defaultVal = "") const;

        
    };

//=======================================================================================
//! Number
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct Number final: Value
    {
    public:
        Number();
        Number(Number const& rhs);
        Number(IFactory& factory, ValueHandle v);
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
        static Number New(IFactory& factory, int32_t i32);
        static Number New(IFactory& factory, uint32_t i32);
        static Number New(IFactory& factory, int64_t i64);
        static Number New(IFactory& factory, uint64_t i64);
        static Number New(IFactory& factory, double d);
    };

//=======================================================================================
//! Boolean
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct Boolean final: Value
    {
    public:
        Boolean();
        Boolean(Boolean const& rhs);
        Boolean(IFactory& factory, ValueHandle v);
        bool BoolValue() const;
        operator bool() const { return BoolValue(); }
        static Boolean New(IFactory& factory, bool b);
    };

//=======================================================================================
//! String
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct String final : Value
    {
    public:
        String();
        String(String const& rhs);
        String(IFactory& factory, ValueHandle v);
        Utf8String StringValue() const;
        uint32_t Length() const;
        
        uint64_t ParseAsUInt64(uint64_t defaultVal = 0, IntegerOptions opts = IntegerOptions::Auto) const;
        int64_t ParseAsInt64(int64_t defaultVal = 0, IntegerOptions opts = IntegerOptions::Auto) const;
        uint32_t ParseAsUInt32(uint32_t defaultVal = 0, IntegerOptions opts = IntegerOptions::Auto) const;
        int32_t ParseAsInt32(int32_t defaultVal = 0, IntegerOptions opts = IntegerOptions::Auto) const;
        double ParseAsDouble(double defaultVal = 0.0f) const;

        operator Utf8String  () { return StringValue(); }
        static String New(IFactory& factory, Utf8CP str);
        static String New(IFactory& factory, Utf8StringCR str);
    };
//
//=======================================================================================
//! Object
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct  Object final: Value
    {
    public:
        struct LValue final
            {
            friend struct Object;
            private:
                IFactory& m_factory;
                ValueHandle m_obj;
                Utf8String m_key;
                LValue() = delete;
                LValue(IFactory& factory, ValueHandle object, Utf8String key)
                    :m_factory(factory), m_obj(object), m_key(key)
                    {}
            public:
                /// Converts an L-value to a value.
                operator Value() const;
                operator Number() const;
                operator String() const;
                operator Array() const;
                operator Object() const;
                operator Boolean() const;
                LValue& operator = (Value value);
            };

        Object();
        Object(Object const& rhs);
        Object(IFactory& factory, ValueHandle v);
        Value Get(Utf8CP key) const;
        Value Get(Utf8StringCR key) const;
        void Set(Utf8CP key, Value value);
        void Set(Utf8StringCR key, Value value);
        Array GetMemberNames() const;
        bool HasMember(Utf8CP key) const;
        void RemoveMember(Utf8CP key);
        void RemoveMember(String key);
        bool HasMember(Utf8StringCR key) const;
        LValue operator [](Utf8CP key);
        LValue operator [](Utf8StringCR key);
        LValue operator [](String key);
        uint32_t Count() const;
        static Object New(IFactory& factory);
    };

//=======================================================================================
//! Array
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct Array final: Value
    {
    struct LValue final
        {
        friend struct Array;
        private:
            IFactory& m_factory;
            ValueHandle m_obj;
            uint32_t m_key;
            LValue() = delete;
            LValue(IFactory& factory, ValueHandle object, uint32_t key)
                :m_factory(factory), m_obj(object), m_key(key)
                {}
        public:
            /// Converts an L-value to a value.
            operator Value() const;
            operator Number() const;
            operator String() const;
            operator Array() const;
            operator Object() const;
            operator Boolean() const;
            LValue& operator = (Value value);
        };
    public:
        Array() : Value() {}
        Array(Array const& rhs) : Value(rhs.Factory(), rhs.Handle()) {}
        Array(IFactory& factory, ValueHandle v) :Value(factory, v) {}
        Value Get(uint32_t index) const;
        void Set(uint32_t index, Value value);
        LValue operator [](uint32_t index);
        LValue operator [](Number index);
        uint32_t Length() const;
        static Array New(IFactory& factory, uint32_t size);
    };

//=======================================================================================
//! IFactory
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct IFactory : RefCountedBase
    {
    public:
        IFactory() {};
        virtual ~IFactory() {};
        /*
        virtual Status ToBoolean(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToString(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToInt32(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToUInt32(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToInt64(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToUInt64(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToDouble(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToArray(ValueHandle& toVal, ValueHandle fromVal) = 0;
        virtual Status ToObject(ValueHandle& toVal, ValueHandle fromVal) = 0;
        */
        virtual Status CreateString(ValueHandle& val, Utf8CP s) = 0;
        virtual Status CreateUInt32(ValueHandle& val, uint32_t i32) = 0;
        virtual Status CreateInt32(ValueHandle& val, int32_t i32) = 0;
        virtual Status CreateUInt64(ValueHandle& val, uint64_t i64) = 0;
        virtual Status CreateInt64(ValueHandle& val, int64_t i64) = 0;
        virtual Status CreateDouble(ValueHandle& val, double d) = 0;
        virtual Status CreateObject(ValueHandle& val) = 0;
        virtual Status CreateArray(ValueHandle& val, uint32_t size) = 0;
        virtual Status GetBoolean(bool& b, ValueHandle val) const = 0;
        virtual Status GetString(Utf8String& s, ValueHandle val) const = 0;
        virtual Status GetInt32(int32_t& i32, ValueHandle val) const = 0;
        virtual Status GetUInt32(uint32_t& i32, ValueHandle val) const = 0;
        virtual Status GetInt64(int64_t& i64, ValueHandle val) const = 0;
        virtual Status GetUInt64(uint64_t& i64, ValueHandle val) const = 0;
        virtual Status GetDouble(double& d, ValueHandle val) const = 0;
        virtual Status GetMemberNames(ValueHandle& members, ValueHandle val) const = 0;
        virtual Status HasMember(bool& found, ValueHandle val, Utf8CP key) const = 0;
        virtual Status RemoveMember(ValueHandle val, ValueHandle key) = 0;
         virtual Status RemoveMember(ValueHandle val, Utf8CP key) = 0;
        virtual Status GetMember(ValueHandle& rt, ValueHandle obj, Utf8CP key) const = 0;
        virtual Status GetMember(ValueHandle& rt, ValueHandle obj, uint32_t index) const = 0;
        virtual Status GetMember(ValueHandle& rt, ValueHandle obj, ValueHandle index) const = 0;
        virtual Status GetSize(uint32_t& n, ValueHandle val) const = 0;
        virtual Status SetMember(ValueHandle obj, Utf8CP key, ValueHandle val) = 0;
        virtual Status SetMember(ValueHandle obj, uint32_t index, ValueHandle val) = 0;
        virtual Status SetMember(ValueHandle obj, ValueHandle index, ValueHandle val) = 0;
        virtual Status GetType(ValueType& t, ValueHandle val) const = 0;
        virtual Status BeginScope(ValueHandle val) const = 0;
        virtual Status EndScope(ValueHandle val) const = 0;
        virtual ValueHandle GetNull() const = 0;        
        virtual ValueHandle GetBoolean(bool b) const = 0;
        virtual ValueHandle GetUndefined() const = 0;
        virtual bool NotifyScopeChanges() const = 0;
    };

//=======================================================================================
//! Converter 
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct Converter
    {
    static bool IsHex(Utf8StringCR str);
    static bool HasFractionPart(double d);
    static uint64_t ParseUInt64(Utf8StringCR str, uint64_t defaultVal = 0, IntegerOptions options = IntegerOptions::Auto);

    static uint32_t ParseUInt32(Utf8StringCR str, uint32_t defautlVal = 0, IntegerOptions options = IntegerOptions::Auto);
    
    static int64_t ParseInt64(Utf8StringCR str, int64_t defaultVal = 0, IntegerOptions options = IntegerOptions::Auto);
    static int32_t ParseInt32(Utf8StringCR str, int32_t defaultVal = 0, IntegerOptions options = IntegerOptions::Auto);
    static double ParseDouble(Utf8StringCR str, double defaultVal = 0);
    static bool ParseBoolean(Utf8StringCR str, bool defaultVal = true);

    static void ToString(Utf8StringR out, uint64_t i, IntegerOptions options = IntegerOptions::Decimal);
    static void ToString(Utf8StringR out, uint32_t i, IntegerOptions optiosn = IntegerOptions::Decimal);
    static void ToString(Utf8StringR out, int64_t i, IntegerOptions optiosn = IntegerOptions::Decimal);
    static void ToString(Utf8StringR out, int32_t i, IntegerOptions optiosn = IntegerOptions::Decimal);
    static void ToString(Utf8StringR out, double);
    static Json::Value ToJson(Value v);

    static int32_t ToInt32(Value val, int32_t defaultVal = 0);
    static uint32_t ToUInt32(Value val, uint32_t defaultVal = 0);
    static int64_t ToInt64(Value val, int64_t defaultVal = 0);
    static uint64_t ToUInt64(Value val, uint64_t defaultVal = 0);
    static double ToDouble(Value val, double defaultVal = 0);
    static bool ToBoolean(Value val, bool defaultVal = false);
    static Utf8String ToString(Value val, Utf8CP defaultVal = "", IntegerOptions options = IntegerOptions::Decimal);


    };

//=======================================================================================
//! GCValueFactory Garbagec collected Value IFactory.
//  @bsiclass                                           Affan.Khan                05/18
//=======================================================================================
struct GCValueFactory final
    {
    public:
        
        struct Options final
            {
            friend struct GCValueFactory;
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


        static RefCountedPtr<IFactory> CreateInstance(Options const& opt = Options());
    };
}
END_BENTLEY_NAMESPACE
