/*--------------------------------------------------------------------------------------+
|
|     $Source: src/JS/Value.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Refcounted.h>
#include <Bentley/bset.h>
#include <Bentley/bmap.h>
#include <queue>
#include <JS/Value.h>

BEGIN_BENTLEY_NAMESPACE
namespace Js 
{
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
struct ValueHandle__ final
    {
    struct CompareI
        {
        bool operator ()(ValueHandle__* lhs, ValueHandle__* rhs) const
            {
            BeAssert(lhs->m_type == ValueType::String && rhs->m_type == ValueType::String);
            return lhs->m_val.str->CompareToI(*rhs->m_val.str) < 0;
            }
        };
    typedef bmap<ValueHandle__*, ValueHandle__*, ValueHandle__::CompareI> Map;
    union
        {
        int32_t i32;
        uint32_t ui32;
        int64_t i64;
        uint64_t ui64;
        bool b;
        double d;
        Utf8StringP str;
        Map* map;
        bvector<ValueHandle__*>* vect;
        } m_val;

    ValueType m_type;
    uint32_t m_count;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
IFactory& Value::Factory() const { BeAssert(m_factory != nullptr); return *m_factory; }
ValueHandle Value::Handle() const { return m_value; }
bool Value::IsEmpty() const { return m_value == nullptr; }
bool Value::IsNull() const { return Type() == ValueType::Null; }
bool Value::IsInt32() const { return Type() == ValueType::Int32; }
bool Value::IsUInt32() const { return Type() == ValueType::UInt32; }
bool Value::IsInt64() const { return Type() == ValueType::Int64; }
bool Value::IsUInt64() const { return Type() == ValueType::UInt64; }
bool Value::IsDouble() const { return Type() == ValueType::Double; }
bool Value::IsObject() const { return Type() == ValueType::Object; }
bool Value::IsArray() const { return Type() == ValueType::Array; }
bool Value::IsString() const { return Type() == ValueType::String; }
bool Value::IsBoolean() const { return Type() == ValueType::Boolean; }
bool Value::IsUndefined() const { return Type() == ValueType::Undefined; }
bool Value::IsNumber() const 
    { 
        const ValueType valueType = Type();
        return 
            valueType == ValueType::Int32 ||
            valueType == ValueType::UInt32 ||
            valueType == ValueType::Int64 ||
            valueType == ValueType::UInt64 ||
            valueType == ValueType::Double;
    }
Value::operator ValueHandle () { return m_value; }
bool Value::ReferenceEquals(Value rhs) const { return m_value == rhs.m_value; }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool Boolean::BoolValue() const
    {
    bool d;
    if (Factory().GetBoolean(d, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return d;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Boolean Boolean::New(IFactory& factory, bool b)
    {
    return Boolean(factory, factory.GetBoolean(b));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value::Value()
    :m_value(nullptr), m_factory(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value::Value(Value const& rhs)
    : m_value(rhs.m_value), m_factory(rhs.m_factory)
    {
    if (!IsEmpty())
        {                    
        if (m_value && m_factory->NotifyScopeChanges())
            m_factory->BeginScope(m_value);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value::Value(IFactory& prv, ValueHandle v)
    :m_value(v), m_factory(&prv)
    {
    if (!m_value)
        {
        BeAssert(m_value != nullptr);
        m_factory = nullptr;
        }
    else
        {
        if (m_factory->NotifyScopeChanges())
            m_factory->BeginScope(m_value);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value::~Value()
    {
    if (!IsEmpty())
        {
        if (m_factory->NotifyScopeChanges())
            m_factory->EndScope(m_value);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Number Value::AsNumber() const
    {
    BeAssert(IsNumber());
    if (!IsNumber())
        return Number(Factory(), Factory().GetNull());

    return Number(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
String Value::AsString() const
    {
    BeAssert(IsString());
    if (!IsString())
        return String(Factory(), Factory().GetNull());

    return String(Factory(), Handle());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Boolean Value::AsBoolean() const
    {
    BeAssert(IsBoolean());
    if (!IsBoolean())
        return Boolean(Factory(), Factory().GetNull());

    return Boolean(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Object Value::AsObject() const
    {
    BeAssert(IsObject());
    if (!IsObject())
        return Object(Factory(), Factory().GetNull());

    return Object(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Array Value::AsArray() const
    {
    BeAssert(IsArray());
    if (!IsArray())
        return Array(Factory(), Factory().GetNull());
    
    return Array(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
ValueType Value::Type() const
    {
    ValueType type;
    if (Factory().GetType(type, m_value) != Status::Success)
        {
        BeAssert(false);
        }

    return type;
    }
int32_t Value::ToInt32(int32_t defaultVal) const { return Converter::ToInt32(*this, defaultVal); }
uint32_t Value::ToUInt32(uint32_t defaultVal) const { return Converter::ToUInt32(*this, defaultVal); }
int64_t Value::ToInt64(int64_t defaultVal) const { return Converter::ToInt64(*this, defaultVal); }
uint64_t Value::ToUInt64(uint64_t defaultVal) const { return Converter::ToUInt64(*this, defaultVal); }
double Value::ToDouble(double defaultVal) const { return Converter::ToDouble(*this, defaultVal); }
bool Value::ToBoolean(bool defaultVal) const { return Converter::ToBoolean(*this, defaultVal); }
Utf8String Value::ToString(Utf8CP defaultVal) const { return Converter::ToString(*this, defaultVal); }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int32_t Number::Int32Value() const
    {
    int32_t i32;
    if (Factory().GetInt32(i32, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i32;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t Number::UInt32Value() const
    {
    uint32_t i32;
    if (Factory().GetUInt32(i32, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i32;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int64_t Number::Int64Value() const
    {
    int64_t i64;
    if (Factory().GetInt64(i64, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i64;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t Number::UInt64Value() const
    {
    uint64_t i64;
    if (Factory().GetUInt64(i64, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i64;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
double Number::DoubleValue() const
    {
    double d;
    if (Factory().GetDouble(d, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return d;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Number Number::New(IFactory& factory, int32_t i32)
    {
    ValueHandle val;
    if (factory.CreateInt32(val, i32) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return Number(factory, factory.GetNull());
        }

    return Number(factory, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Number Number::New(IFactory& factory, uint32_t i32)
    {
    ValueHandle val;
    if (factory.CreateUInt32(val, i32) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return Number(factory, factory.GetNull());
        }

    return Number(factory, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Number Number::New(IFactory& factory, int64_t i64)
    {
    ValueHandle val;
    if (factory.CreateInt64(val, i64) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return Number(factory, factory.GetNull());
        }

    return Number(factory, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Number Number::New(IFactory& factory, uint64_t i64)
    {
    ValueHandle val;
    if (factory.CreateUInt64(val, i64) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return Number(factory, factory.GetNull());
        }

    return Number(factory, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Number Number::New(IFactory& factory, double d)
    {
    ValueHandle val;
    if (factory.CreateDouble(val, d) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return Number(factory, factory.GetNull());
        }

    return Number(factory, val);
    }

uint64_t String::ParseAsUInt64(uint64_t defaultVal, IntegerOptions opts) const { return Converter::ParseUInt64(StringValue(), defaultVal, opts); }
int64_t String::ParseAsInt64(int64_t defaultVal, IntegerOptions opts) const { return Converter::ParseInt64(StringValue(), defaultVal, opts); }
uint32_t String::ParseAsUInt32(uint32_t defaultVal, IntegerOptions opts) const { return Converter::ParseUInt32(StringValue(), defaultVal, opts); }
int32_t String::ParseAsInt32(int32_t defaultVal, IntegerOptions opts) const { return Converter::ParseInt32(StringValue(), defaultVal, opts); }
double String::ParseAsDouble(double defaultVal) const { return Converter::ParseDouble(StringValue(), defaultVal); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String String::StringValue() const
    {
    Utf8String str;
    if (Factory().GetString(str, Handle()) != Status::Success)
        {
        BeAssert(false);
        }

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
String String::New(IFactory& factory, Utf8CP str)
    {
    ValueHandle val;
    if (factory.CreateString(val, str) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return String(factory, factory.GetNull());
        }

    return String(factory, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
String String::New(IFactory& factory, Utf8StringCR str)
    {
    return New(factory, str.c_str()); 
    }

Object::LValue::operator Value() const { return Object(m_factory, m_obj).Get(m_key); }
Object::LValue::operator Number() const { return Object(m_factory, m_obj).Get(m_key).AsNumber(); }
Object::LValue::operator String() const { return Object(m_factory, m_obj).Get(m_key).AsString(); }
Object::LValue::operator Array() const { return Object(m_factory, m_obj).Get(m_key).AsArray(); }
Object::LValue::operator Object() const { return Object(m_factory, m_obj).Get(m_key).AsObject(); }
Object::LValue::operator Boolean() const { return Object(m_factory, m_obj).Get(m_key).AsBoolean(); }

Object::LValue& Object::LValue::operator = (Value value) { Object(m_factory, m_obj).Set(m_key, value);  return *this; }
Object::LValue Object::operator [](Utf8CP key) { return LValue(Factory(), Handle(), key); }
Object::LValue Object::operator [](Utf8StringCR key) { return LValue(Factory(), Handle(), key); }
Object::LValue Object::operator [](String key) { return LValue(Factory(), Handle(), key.StringValue()); }


Array::LValue::operator Value() const { return Array(m_factory, m_obj).Get(m_key); }
Array::LValue::operator Number() const { return Array(m_factory, m_obj).Get(m_key).AsNumber(); }
Array::LValue::operator String() const { return Array(m_factory, m_obj).Get(m_key).AsString(); }
Array::LValue::operator Array() const { return Array(m_factory, m_obj).Get(m_key).AsArray(); }
Array::LValue::operator Object() const { return Array(m_factory, m_obj).Get(m_key).AsObject(); }
Array::LValue::operator Boolean() const { return Array(m_factory, m_obj).Get(m_key).AsBoolean(); }

Array::LValue& Array::LValue::operator = (Value value) { Array(m_factory, m_obj).Set(m_key, value);  return *this; }
Array::LValue Array::operator [](uint32_t index) { return LValue(Factory(), Handle(), index); }
Array::LValue Array::operator [](Number index) { return LValue(Factory(), Handle(), index.UInt32Value()); }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Array Array::New(IFactory& factory, uint32_t size)
    {
    ValueHandle val;
    if (factory.CreateArray(val, size) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return Array(factory, factory.GetNull());
        }

    return Array(factory, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Object Object::New(IFactory& factory)
    {
    ValueHandle val;
    if (factory.CreateObject(val) != Status::Success)
        {
        BeAssert(false && "Fail to create value");
        return Object(factory, factory.GetNull());
        }

    return Object(factory, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t Array::Length() const
    {
    uint32_t sz;
    if (Factory().GetSize(sz, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to size");
        return 0;
        }

    return sz;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t Object::Count() const
    {
    uint32_t sz;
    if (Factory().GetSize(sz, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to size");
        return 0;
        }

    return sz;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Object::RemoveMember(Utf8CP key)
    {
    if (Factory().RemoveMember(Handle(), key) != Status::Success)
        {
        BeAssert(false && "Fail to RemoveMember");
        }   
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Object::RemoveMember(String key)
    {
    if (Factory().RemoveMember(Handle(), key.Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to RemoveMember");
        }           
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t String::Length() const
    {
    uint32_t sz;
    if (Factory().GetSize(sz, Handle()) != Status::Success)
        {
        BeAssert(false && "Fail to size");
        return 0;
        }

    return sz;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value Array::Get(uint32_t index) const
    {
    ValueHandle v;
    if (Factory().GetMember(v, Handle(), index) != Status::Success)
        {
        BeAssert(false && "Fail to size");
        return Value(Factory(), Factory().GetNull());
        }

    return Value(Factory(), v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Array::Set(uint32_t index, Value value)
    {
    if (Factory().SetMember(Handle(), index, value) != Status::Success)
        {
        BeAssert(false && "Fail to size");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value Object::Get(Utf8CP key) const
    {
    ValueHandle v;
    if (Factory().GetMember(v, Handle(), key) != Status::Success)
        {
        BeAssert(false && "Fail to size");
        return Value(Factory(), Factory().GetNull());
        }

    return Value(Factory(), v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value Object::Get(Utf8StringCR key) const
    {
    return Get(key.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Object::Set(Utf8CP key, Value value)
    {
    if (Factory().SetMember(Handle(), key, value) != Status::Success)
        {
        BeAssert(false && "Fail to size");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Object::Set(Utf8StringCR key, Value value)
    {
    Set(key.c_str(), value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool Object::HasMember(Utf8CP key) const
    {
    bool foundIt;
    if (Factory().HasMember(foundIt, Handle(), key) != Status::Success)
        {
        BeAssert(false);
        return false;
        }

    return foundIt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool Object::HasMember(Utf8StringCR key) const
    {
    return HasMember(key.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Array Object::GetMemberNames() const
    {
    ValueHandle val;
    if (Factory().GetMemberNames(val, Handle()) != Status::Success)
        {
        BeAssert(false);
        return Array(Factory(), Factory().GetNull());
        }

    return Array(Factory(), val);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
struct GCValueFactoryImpl : public RefCounted<IFactory>
    {
    private:
        enum class CachedObjectType
            {
            Primitive,
            String,
            Vector,
            Map,
            };

        mutable bmap<CachedObjectType, bset<ValueHandle__*>> m_cacheUsed;
        mutable bmap<CachedObjectType, std::queue<ValueHandle__*>> m_cacheUnused;
        mutable bmap<ValueType, CachedObjectType> m_cacheType;
        mutable bmap<CachedObjectType, int> m_cacheMaxSize;
        ValueHandle__* m_null;
        ValueHandle__* m_true;
        ValueHandle__* m_false;
        ValueHandle__* m_undefined;
        Status TrimUnusedItemCache(CachedObjectType type, float extraTrim)
            {
            auto& q = m_cacheUnused.find(type)->second;
            const int maxUnusedItems = m_cacheMaxSize.find(type)->second;
            const int unusedItems = (int) q.size();
            const int delta = maxUnusedItems - unusedItems;
            const int totalItemToRemove = std::max(maxUnusedItems - unusedItems, static_cast<int>(maxUnusedItems* (100 / extraTrim)));
            while (q.size() > totalItemToRemove)
                {
                Free(q.front());
                q.pop();
                }

            return Status::Success;
            }

        Status TrimUnusedItemCache(float extraTrim)
            {
            Status status = Status::Success;
            for (auto itor : m_cacheMaxSize)
                {
                if (m_cacheUnused[itor.first].size() > m_cacheMaxSize[itor.first])
                    {
                    status = TrimUnusedItemCache(itor.first, extraTrim);
                    if (status != Status::Success)
                        return status;
                    }
                }

            return status;
            }

        Status CreateInstance(ValueHandle& v, ValueType type, uint32_t sz = 0)
            {
            v = nullptr;
            const CachedObjectType cacheType = m_cacheType[type];
            std::queue<ValueHandle__*>& q = m_cacheUnused[cacheType];
            if (!q.empty())
                {
                v = q.front();
                q.pop();
                }

            if (v)
                {
                if (type == ValueType::Object)
                    {
                    auto& map = *(v->m_val.map);
                    for (auto& pv : map)
                        {
                        EndScope(pv.first);
                        EndScope(pv.second);
                        }

                    map.clear();
                    }
                else if (type == ValueType::Array)
                    {
                    for (auto pv : *(v->m_val.vect))
                        EndScope(pv);

                    v->m_val.vect->resize(sz, m_null);

                    }
                else if (type == ValueType::String)
                    {
                    v->m_val.str->clear();
                    }
                else
                    {
                    memset(v, 0, sizeof(ValueHandle__));
                    v->m_type = type;
                    }
                }
            else
                {
                v = new ValueHandle__();
                memset(v, 0, sizeof(ValueHandle__));
                v->m_type = type;
                if (type == ValueType::String)
                    v->m_val.str = new Utf8String();
                else if (type == ValueType::Object)
                    v->m_val.map = new ValueHandle__::Map();
                else if (type == ValueType::Array)
                    {
                    v->m_val.vect = new bvector<ValueHandle__*>();
                    v->m_val.vect->resize(sz, m_null);
                    }
                }

            v->m_count;
            m_cacheUsed[cacheType].insert(v);
            return Status::Success;
            }

        void ClearCache()
            {
            for (auto itA : m_cacheUsed)
                for (auto itV : itA.second)
                    Free(itV);

            for (auto itA : m_cacheUnused)
                while (!itA.second.empty())
                    {
                    Free(itA.second.front());
                    itA.second.pop();
                    }

            m_cacheUsed.clear();
            m_cacheUnused.clear();
            }

        void Free(ValueHandle& v)
            {
            if (v->m_type == ValueType::String)
                delete v->m_val.str;
            else if (v->m_type == ValueType::Object)
                delete  v->m_val.map;
            else if (v->m_type == ValueType::Array)
                delete  v->m_val.vect;

            free(v);
            }

        bool IsPrimitve(ValueType t) const
            {
            return !(t == ValueType::Array || t == ValueType::Object || t == ValueType::String);
            }

        Status EndScopeForValue(ValueHandle v) const
            {
            BeAssert(v->m_count > 0);
            v->m_count = v->m_count - 1;

            if (v->m_count <= 0)
                {
                if (v->m_type == ValueType::Object)
                    {
                    auto& map = *(v->m_val.map);
                    for (auto& pv : map)
                        {
                        EndScopeForValue(pv.first);
                        EndScopeForValue(pv.second);
                        }

                    map.clear();
                    }
                else if (v->m_type == ValueType::Array)
                    {
                    auto& vect = *(v->m_val.vect);
                    for (auto pv : vect)
                        EndScopeForValue(pv);

                    vect.clear();
                    }
                else if (v->m_type == ValueType::String)
                    {
                    v->m_val.str->clear();
                    }

                const CachedObjectType cacheType = m_cacheType[v->m_type];
                m_cacheUsed[cacheType].erase(v);
                m_cacheUnused[cacheType].push(v);
                }

            return Status::Success;
            }
    public:

        GCValueFactoryImpl(uint32_t maxStrings, uint32_t maxArray, uint32_t maxObj, uint32_t maxPri)
            {
            m_cacheType[ValueType::Int32] = CachedObjectType::Primitive;
            m_cacheType[ValueType::UInt32] = CachedObjectType::Primitive;
            m_cacheType[ValueType::Int64] = CachedObjectType::Primitive;
            m_cacheType[ValueType::UInt64] = CachedObjectType::Primitive;
            m_cacheType[ValueType::Boolean] = CachedObjectType::Primitive;
            m_cacheType[ValueType::Double] = CachedObjectType::Primitive;
            m_cacheType[ValueType::Null] = CachedObjectType::Primitive;
            m_cacheType[ValueType::Array] = CachedObjectType::Vector;
            m_cacheType[ValueType::Object] = CachedObjectType::Map;
            m_cacheType[ValueType::String] = CachedObjectType::String;
            m_cacheMaxSize[CachedObjectType::String] = maxStrings < 10 ? 10 : maxStrings;
            m_cacheMaxSize[CachedObjectType::Vector] = maxArray < 10 ? 10 : maxArray;
            m_cacheMaxSize[CachedObjectType::Map] = maxObj < 10 ? 10 : maxObj;
            m_cacheMaxSize[CachedObjectType::Primitive] = maxPri < 10 ? 10 : maxPri;
            
            //Create global variables
            CreateInstance(m_null, ValueType::Null);
            CreateInstance(m_true, ValueType::Boolean);
            m_true->m_val.b = true;
            CreateInstance(m_false, ValueType::Boolean);
            m_true->m_val.b = false;
            CreateInstance(m_undefined, ValueType::Undefined);            
            }

        ~GCValueFactoryImpl()
            {
            ClearCache();
            }

        /////////////////
        Status CreateString(ValueHandle& val, Utf8CP str) override
            {
            Status status = CreateInstance(val, ValueType::String);
            if (status != Status::Success)
                return status;

            val->m_val.str->AssignOrClear(str);
            return Status::Success;
            }
        Status CreateInt32(ValueHandle& val, int32_t i32) override
            {
            Status status = CreateInstance(val, ValueType::Int32);
            if (status != Status::Success)
                return status;

            val->m_val.i32 = i32;
            return Status::Success;
            }
        Status CreateUInt32(ValueHandle& val, uint32_t i32) override
            {
            Status status = CreateInstance(val, ValueType::Int32);
            if (status != Status::Success)
                return status;

            val->m_val.ui32 = i32;
            return Status::Success;
            }

        Status CreateUInt64(ValueHandle& val, uint64_t i64) override
            {
            Status status = CreateInstance(val, ValueType::UInt64);
            if (status != Status::Success)
                return status;

            val->m_val.ui64 = i64;
            return Status::Success;
            }

        Status CreateInt64(ValueHandle& val, int64_t i64) override
            {
            Status status = CreateInstance(val, ValueType::Int64);
            if (status != Status::Success)
                return status;

            val->m_val.i64 = i64;
            return Status::Success;
            }

        Status CreateDouble(ValueHandle& val, double d) override
            {
            Status status = CreateInstance(val, ValueType::Double);
            if (status != Status::Success)
                return status;

            val->m_val.d = d;
            return Status::Success;
            }
        Status CreateObject(ValueHandle& val) override
            {
            Status status = CreateInstance(val, ValueType::Object);
            if (status != Status::Success)
                return status;

            return Status::Success;
            }

        Status CreateArray(ValueHandle& val, uint32_t size) override
            {
            Status status = CreateInstance(val, ValueType::Array, size);
            if (status != Status::Success)
                return status;

            return Status::Success;
            }

        Status GetBoolean(bool& b, ValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                b = false;
                return Status::IncorrectType;
                }

            b = val->m_val.b;
            return Status::Success;
            }
        Status GetString(Utf8String& s, ValueHandle val) const override
            {
            if (val->m_type != ValueType::String)
                return Status::IncorrectType;

            s = *(val->m_val.str);
            return Status::Success;
            }
        Status GetInt32(int32_t& i32, ValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i32;
                return Status::IncorrectType;
                }

            i32 = val->m_val.i32;
            return Status::Success;
            }
        Status GetUInt32(uint32_t& i32, ValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i32;
                return Status::IncorrectType;
                }

            i32 = val->m_val.ui32;
            return Status::Success;
            }
        Status GetInt64(int64_t& i64, ValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i64;
                return Status::IncorrectType;
                }

            i64 = val->m_val.i64;
            return Status::Success;
            }
        Status GetUInt64(uint64_t& i64, ValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i64;
                return Status::IncorrectType;
                }

            i64 = val->m_val.ui64;
            return Status::Success;
            }
        Status GetDouble(double& d, ValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                d;
                return Status::IncorrectType;
                }

            d = val->m_val.d;
            return Status::Success;
            }

        Status GetMemberNames(ValueHandle& members, ValueHandle val) const override
            {
            if (val->m_type != ValueType::Object)
                {
                members = m_null;
                return Status::IncorrectType;
                }

            uint32_t sz;
            Status status = GetSize(sz, val);
            if (status != Status::Success)
                return status;

            status = const_cast<GCValueFactoryImpl*>(this)->CreateArray(members, sz);
            if (status != Status::Success)
                return status;

            uint32_t idx;
            for (auto itor = val->m_val.map->begin(); itor != val->m_val.map->end(); ++itor)
                (*members->m_val.vect)[idx++] = itor->first;

            return Status::Success;
            }

        Status HasMember(bool& found, ValueHandle val, Utf8CP key) const override
            {
            if (val->m_type != ValueType::Object)
                return Status::IncorrectType;

            ValueHandle vt;
            Status status = const_cast<GCValueFactoryImpl*>(this)->CreateString(vt, key);
            if (status != Status::Success)
                return status;

            BeginScope(vt);
            found = val->m_val.map->find(vt) != val->m_val.map->end();
            return EndScope(vt);
            }

        Status GetMember(ValueHandle& rt, ValueHandle obj, Utf8CP key) const override
            {
            ValueHandle vt;
            Status status = const_cast<GCValueFactoryImpl*>(this)->CreateString(vt, key);
            if (status != Status::Success)
                return status;

            const_cast<GCValueFactoryImpl*>(this)->BeginScope(vt);
            status = GetMember(rt, obj, vt);
            const_cast<GCValueFactoryImpl*>(this)->EndScope(vt);
            return status;
            }

        Status GetMember(ValueHandle& rt, ValueHandle obj, uint32_t index) const override
            {
            ValueHandle vt;
            Status status = const_cast<GCValueFactoryImpl*>(this)->CreateUInt32(vt, index);
            if (status != Status::Success)
                return status;

            const_cast<GCValueFactoryImpl*>(this)->BeginScope(vt);
            status = GetMember(rt, obj, vt);
            const_cast<GCValueFactoryImpl*>(this)->EndScope(vt);
            return status;
            }

        Status GetMember(ValueHandle& rt, ValueHandle obj, ValueHandle index) const override
            {
            rt = m_null;
            if (obj->m_type == ValueType::Object)
                {
                auto itor = obj->m_val.map->find(index);
                if (itor != obj->m_val.map->end())
                    rt = itor->second;

                return Status::Success;
                }

            if (obj->m_type == ValueType::Array)
                {
                uint32_t idx;
                Status status = GetUInt32(idx, index);
                if (status != Status::Success)
                    return status;

                rt = obj->m_val.vect->at(idx);
                return Status::Success;
                }

            return Status::IncorrectType;
            }

        Status GetSize(uint32_t& n, ValueHandle val) const override
            {
            n;
            if (val->m_type == ValueType::Object)
                {
                n = (uint32_t) val->m_val.map->size();
                return Status::Success;
                }

            if (val->m_type == ValueType::Array)
                {
                n = (uint32_t) val->m_val.vect->size();
                return Status::Success;
                }

            return Status::IncorrectType;
            }

        Status RemoveMember(ValueHandle obj, ValueHandle key) override
            {
            if (obj->m_type != ValueType::Object)
                   return Status::IncorrectType;

            auto& map = *(obj->m_val.map);
            auto itor = map.find(key);
            if (itor == map.end())
                return Status::MemberNotFound;

            EndScope(itor->first);
            EndScope(itor->second);
            map.erase(itor);
            return Status::Success;
            }

        Status RemoveMember(ValueHandle obj, Utf8CP key) override
            {
            if (obj->m_type != ValueType::Object)
                return Status::IncorrectType;
            
            ValueHandle str;
            Status status = CreateString(str, key);
            if (status != Status::Success)
                return status;

            BeginScope(str);
            status = RemoveMember(obj, str);
            EndScope(str);
            return status;
            }

        Status SetMember(ValueHandle obj, Utf8CP key, ValueHandle val) override
            {
            if (obj->m_type != ValueType::Object)
                return Status::IncorrectType;

            ValueHandle str;
            Status status = CreateString(str, key);
            if (status != Status::Success)
                return status;

            BeginScope(str);
            status = SetMember(obj, str, val);
            EndScope(str);
            return status;
            }
        Status SetMember(ValueHandle obj, uint32_t index, ValueHandle val) override
            {
            if (obj->m_type != ValueType::Array)
                return Status::IncorrectType;

            ValueHandle str;
            Status status = CreateUInt32(str, index);
            if (status != Status::Success)
                return status;

            BeginScope(str);
            status = SetMember(obj, str, val);
            EndScope(str);
            return status;
            }
        Status SetMember(ValueHandle obj, ValueHandle index, ValueHandle val) override
            {
            if (obj->m_type == ValueType::Object)
                {
                auto& map = *(obj->m_val.map);
                auto itor = map.find(index);
                if (itor == map.end())
                    {
                    BeginScope(index);
                    BeginScope(val);
                    map.insert(make_bpair(index, val));
                    }
                else
                    {
                    BeginScope(val);
                    EndScope(itor->second);
                    itor->second = val;
                    }
                return Status::Success;
                }
            else if (obj->m_type == ValueType::Array)
                {
                auto& vect = *(obj->m_val.vect);
                uint32_t idx;
                Status status = GetUInt32(idx, index);
                if (status != Status::Success)
                    return status;

                if (idx >= vect.size())
                    return Status::IndexOutOfRange;

                auto itor = vect.begin() + idx;
                EndScope(*itor);
                BeginScope(val);
                *itor = val;
                return Status::Success;
                }

            return Status::IncorrectType;
            }
        Status GetType(ValueType& t, ValueHandle val) const override
            {
            if (val == nullptr)
                return Status::NullArgument;

            t = val->m_type;
            return Status::Success;
            }
        ValueHandle GetNull() const override { return m_null; }

        ValueHandle GetBoolean(bool b) const override { return b? m_true : m_false; }
        
        ValueHandle GetUndefined() const override { return m_undefined; }
        
        Status BeginScope(ValueHandle val) const  override
            {
            //This protect variable from deletion.
            //It shoudl stay above 1 to be not garbage collected
            val->m_count++;
            return Status::Success;
            }

        Status EndScope(ValueHandle val) const override
            {
            if (val == m_null || val == m_undefined || val == m_true  || val == m_false )
                return Status::Success;

            if (EndScopeForValue(val) != Status::Success)
                return Status::Error;

            if (val->m_count <= 0)
                {
                const_cast<GCValueFactoryImpl*>(this)->TrimUnusedItemCache(10.0f);
                }

            return Status::Success;
            }

        bool NotifyScopeChanges() const override { return true; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
RefCountedPtr<IFactory> GCValueFactory::CreateInstance(Options const& opt)
    {
    return new GCValueFactoryImpl(opt.m_maxStringValueCache,
                                   opt.m_maxArrayValueCache,
                                   opt.m_maxObjectValueCache,
                                   opt.m_maxPrimitiveValueCache);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
Json::Value Value::ToJson() const
    {
    return Converter::ToJson(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
Value Value::Null(IFactory& factory)
    {
    return Value(factory, factory.GetNull());  
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
Value Value::Undefined(IFactory& factory)
    {
    return Value(factory, factory.GetUndefined());  
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Value Value::FromJson(IFactory& factory, Json::Value const& v)
    {
    if (v.type() == Json::ValueType::nullValue)
        return Value::Null(factory);
    else if (v.type() == Json::ValueType::intValue)
        return Number::New(factory, v.asInt());
    else if (v.type() == Json::ValueType::uintValue)
        return Number::New(factory, v.asUInt());
    else if (v.type() == Json::ValueType::realValue)
        return Number::New(factory, v.asDouble());
    else if (v.type() == Json::ValueType::stringValue)
        return String::New(factory, v.asCString());
    else if (v.type() == Json::ValueType::booleanValue)
        return Boolean::New(factory, v.asBool());
    else if (v.type() == Json::ValueType::arrayValue)
        {
        Array jsArray = Array::New(factory, (uint32_t)v.size());
        for (auto itor = v.begin(); itor != v.end(); ++itor)
            jsArray.Set((uint32_t)itor.index(), FromJson(factory, (*itor)));

        return jsArray;
        }
    else if (v.type() == Json::ValueType::objectValue)
        {
        Object jsObject = Object::New(factory);
        for (auto itor = v.begin(); itor != v.end(); ++itor)
            jsObject.Set(itor.memberName(), FromJson(factory, (*itor)));

        return jsObject;
        }

    BeAssert(false && "Unknown type");
    return Value::Null(factory);        
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool Converter::IsHex(Utf8StringCR str)
  {
  return str.size() > 2 && str[0]=='0' &&  (str[1]=='X' || str[1]=='x');
  }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool Converter::HasFractionPart(double d)
    {
    return modf(d, nullptr) != 0.0f;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t Converter::ParseUInt64(Utf8StringCR str, uint64_t defaultVal, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IsHex(str) ? IntegerOptions::Hex : IntegerOptions::Decimal;

    uint64_t i = defaultVal;
    if (options == IntegerOptions::Hex)
        sscanf(str.c_str(), "%" SCNx64, &i);
    else
        sscanf(str.c_str(), "%" SCNu64, &i);

    return i;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t Converter::ParseUInt32(Utf8StringCR str, uint32_t defautlVal, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IsHex(str) ? IntegerOptions::Hex : IntegerOptions::Decimal;

    uint32_t i = defautlVal;
    if (options == IntegerOptions::Hex)
        sscanf(str.c_str(), "%" SCNx32, &i);
    else
        sscanf(str.c_str(), "%" SCNu32, &i);

    return i;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int64_t Converter::ParseInt64(Utf8StringCR str, int64_t defaultVal, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IsHex(str) ? IntegerOptions::Hex : IntegerOptions::Decimal;

    int64_t i = defaultVal;
    if (options == IntegerOptions::Hex)
        sscanf(str.c_str(), "%" SCNx64, &i);
    else
        sscanf(str.c_str(), "%" SCNd64, &i);

    return i;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int32_t Converter::ParseInt32(Utf8StringCR str, int32_t defaultVal, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IsHex(str) ? IntegerOptions::Hex : IntegerOptions::Decimal;

    int32_t i = defaultVal;
    if (options == IntegerOptions::Hex)
        sscanf(str.c_str(), "%" SCNx32, &i);
    else
        sscanf(str.c_str(), "%" SCNd32, &i);

    return i;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
double Converter::ParseDouble(Utf8StringCR str, double defaultVal)
    {
    double i = defaultVal;
    sscanf(str.c_str(), "%lf", &i);
    return i;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool Converter::ParseBoolean(Utf8StringCR str, bool defaultVal)
    {
    if (str.EqualsI("true"))
        return true;

    if (str.EqualsI("false"))
        return false;

    return defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Converter::ToString(Utf8StringR out, uint64_t i, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IntegerOptions::Decimal;

    if (options == IntegerOptions::Decimal)
        out.Sprintf("%" PRIu64, i);
    else
        out.Sprintf("%" PRIx64, i);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------

void Converter::ToString(Utf8StringR out, uint32_t i, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IntegerOptions::Decimal;

    if (options == IntegerOptions::Decimal)
        out.Sprintf("%" PRIu32, i);
    else
        out.Sprintf("%" PRIx32, i);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Converter::ToString(Utf8StringR out, int64_t i, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IntegerOptions::Decimal;

    if (options == IntegerOptions::Decimal)
        out.Sprintf("%" PRId64, i);
    else
        out.Sprintf("%" PRIx64, i);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Converter::ToString(Utf8StringR out, int32_t i, IntegerOptions options)
    {
    if (options == IntegerOptions::Auto)
        options = IntegerOptions::Decimal;

    if (options == IntegerOptions::Decimal)
        out.Sprintf("%" PRId32, i);
    else
        out.Sprintf("%" PRIx32, i);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Json::Value Converter::ToJson(Value v)
    {
    Json::Value out;
    if (v.IsEmpty())
        {
        BeAssert(false && "Fail to convert Value to Json::Value");
        return out;
        }

    const ValueType vT = v.Type();
    IFactory& factory = v.Factory();
    if (vT == ValueType::Null)
        {
        out = Json::Value(Json::ValueType::nullValue);
        }
    else if (vT == ValueType::Undefined)
        {
        BeAssert(false && "Fail to convert Value to Json::Value");
        out = Json::Value(Json::ValueType::nullValue);        
        }
    else if (vT == ValueType::String)
        {
        Utf8String str;
        if (factory.GetString(str, v.Handle()) != Status::Success)
            {
            BeAssert(false);
            }

        out = Json::Value(str.c_str());
        }
    else if (vT == ValueType::Int32)
        {
        int32_t i32;
        if (factory.GetInt32(i32, v.Handle()) != Status::Success)
            {
            BeAssert(false);
            }

        out = Json::Value(i32);
        }
    else if (vT == ValueType::UInt32)
        {
        uint32_t i32;
        if (factory.GetUInt32(i32, v.Handle()) != Status::Success)
            {
            BeAssert(false);
            }

        out = Json::Value(i32);
        }
    else if (vT == ValueType::Int64)
        {
        int64_t i64;
        if (factory.GetInt64(i64, v.Handle()) != Status::Success)
            {
            BeAssert(false);
            }

        out = Json::Value(i64);
        }
    else if (vT == ValueType::UInt64)
        {
        uint64_t i64;
        if (factory.GetUInt64(i64, v.Handle()) != Status::Success)
            {
            BeAssert(false);
            }

        out = Json::Value(i64);
        }
    else if (vT == ValueType::Double)
        {
        double d;
        if (factory.GetDouble(d, v.Handle()) != Status::Success)
            {
            BeAssert(false);
            }

        out = Json::Value(d);
        }
    else if (vT == ValueType::Array)
        {
        out = Json::Value(Json::ValueType::arrayValue);
        Array ary = v.AsArray();
        const uint32_t length = ary.Length();
        for (uint32_t i = 0; i < length; ++i)
            {
            out[i] = ToJson(ary.Get(i));
            }
        }
    else if (vT == ValueType::Object)
        {
        out = Json::Value(Json::ValueType::objectValue);
        Object obj = v.AsObject();
        Array members = obj.GetMemberNames();
        const uint32_t n = members.Length();
        for (uint32_t i = 0; i < n; ++i)
            {
            const Utf8String key = members.Get(i).AsString();
            out[key.c_str()] = obj.Get(key).ToJson();
            }
        }
    
    return out;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void Converter::ToString(Utf8StringR out, double d)
    {
    out.Sprintf("%lf", d);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int32_t Converter::ToInt32(Value val, int32_t defaultVal)
    {
    if (val.IsEmpty())
        {
        BeAssert(val.IsEmpty());
        return defaultVal;
        }

    ValueType vT = val.Type();
    if (vT == ValueType::Null || vT == ValueType::Undefined)
        return defaultVal;

    if (vT == ValueType::String)
        {
        Utf8String str = val.AsString();
        return Converter::ParseInt32(str, defaultVal);
        }
    else if (vT == ValueType::Int32)
        return static_cast<int32_t>(val.AsNumber().Int32Value());
    else if (vT == ValueType::UInt32)
        return static_cast<int32_t>(val.AsNumber().UInt32Value());
    else if (vT == ValueType::Int64)
        return static_cast<int32_t>(val.AsNumber().Int64Value());
    else if (vT == ValueType::UInt64)
        return static_cast<int32_t>(val.AsNumber().UInt64Value());
    else if (vT == ValueType::Double)
        return static_cast<int32_t>(val.AsNumber().DoubleValue());

    return defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t Converter::ToUInt32(Value val, uint32_t defaultVal)
    {
    if (val.IsEmpty())
        {
        BeAssert(val.IsEmpty());
        return defaultVal;
        }

    ValueType vT = val.Type();
    if (vT == ValueType::Null || vT == ValueType::Undefined)
        return defaultVal;

    if (vT == ValueType::String)
        {
        Utf8String str = val.AsString();
        return Converter::ParseUInt32(str, defaultVal);
        }
    else if (vT == ValueType::Int32)
        return static_cast<uint32_t>(val.AsNumber().Int32Value());
    else if (vT == ValueType::UInt32)
        return static_cast<uint32_t>(val.AsNumber().UInt32Value());
    else if (vT == ValueType::Int64)
        return static_cast<uint32_t>(val.AsNumber().Int64Value());
    else if (vT == ValueType::UInt64)
        return static_cast<uint32_t>(val.AsNumber().UInt64Value());
    else if (vT == ValueType::Double)
        return static_cast<uint32_t>(val.AsNumber().DoubleValue());

    return defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int64_t Converter::ToInt64(Value val, int64_t defaultVal)
    {
    if (val.IsEmpty())
        {
        BeAssert(val.IsEmpty());
        return defaultVal;
        }

    ValueType vT = val.Type();
    if (vT == ValueType::Null || vT == ValueType::Undefined)
        return defaultVal;

    if (vT == ValueType::String)
        {
        Utf8String str = val.AsString();
        return Converter::ParseInt64(str, defaultVal);
        }
    else if (vT == ValueType::Int32)
        return static_cast<int64_t>(val.AsNumber().Int32Value());
    else if (vT == ValueType::UInt32)
        return static_cast<int64_t>(val.AsNumber().UInt32Value());
    else if (vT == ValueType::Int64)
        return static_cast<int64_t>(val.AsNumber().Int64Value());
    else if (vT == ValueType::UInt64)
        return static_cast<int64_t>(val.AsNumber().UInt64Value());
    else if (vT == ValueType::Double)
        return static_cast<int64_t>(val.AsNumber().DoubleValue());

    return defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t Converter::ToUInt64(Value val, uint64_t defaultVal)
    {
    if (val.IsEmpty())
        {
        BeAssert(val.IsEmpty());
        return defaultVal;
        }

    ValueType vT = val.Type();
    if (vT == ValueType::Null || vT == ValueType::Undefined)
        return defaultVal;

    if (vT == ValueType::String)
        {
        Utf8String str = val.AsString();
        return Converter::ParseUInt64(str, defaultVal);
        }
    else if (vT == ValueType::Int32)
        return static_cast<uint64_t>(val.AsNumber().Int32Value());
    else if (vT == ValueType::UInt32)
        return static_cast<uint64_t>(val.AsNumber().UInt32Value());
    else if (vT == ValueType::Int64)
        return static_cast<uint64_t>(val.AsNumber().Int64Value());
    else if (vT == ValueType::UInt64)
        return static_cast<uint64_t>(val.AsNumber().UInt64Value());
    else if (vT == ValueType::Double)
        return static_cast<uint64_t>(val.AsNumber().DoubleValue());

    return defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
double Converter::ToDouble(Value val, double defaultVal)
    {
    if (val.IsEmpty())
        {
        BeAssert(val.IsEmpty());
        return defaultVal;
        }

    ValueType vT = val.Type();
    if (vT == ValueType::Null || vT == ValueType::Undefined)
        return defaultVal;

    if (vT == ValueType::String)
        {
        Utf8String str = val.AsString();
        return Converter::ParseDouble(str, defaultVal);
        }
    else if (vT == ValueType::Int32)
        return static_cast<double>(val.AsNumber().Int32Value());
    else if (vT == ValueType::UInt32)
        return static_cast<double>(val.AsNumber().UInt32Value());
    else if (vT == ValueType::Int64)
        return static_cast<double>(val.AsNumber().Int64Value());
    else if (vT == ValueType::UInt64)
        return static_cast<double>(val.AsNumber().UInt64Value());
    else if (vT == ValueType::Double)
        return static_cast<double>(val.AsNumber().DoubleValue());

    return defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool Converter::ToBoolean(Value val, bool defaultVal)
    {
    if (val.IsEmpty())
        {
        BeAssert(val.IsEmpty());
        return defaultVal;
        }

    ValueType vT = val.Type();
    if (vT == ValueType::Null || vT == ValueType::Undefined)
        return defaultVal;

    if (vT == ValueType::String)
        {
        Utf8String str = val.AsString();
        return Converter::ParseBoolean(str, defaultVal);
        }
    else if (vT == ValueType::Int32)
        return static_cast<bool>(val.AsNumber().Int32Value());
    else if (vT == ValueType::UInt32)
        return static_cast<bool>(val.AsNumber().UInt32Value());
    else if (vT == ValueType::Int64)
        return static_cast<bool>(val.AsNumber().Int64Value());
    else if (vT == ValueType::UInt64)
        return static_cast<bool>(val.AsNumber().UInt64Value());
    else if (vT == ValueType::Double)
        return static_cast<bool>(val.AsNumber().DoubleValue());

    return defaultVal;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String Converter::ToString(Value val, Utf8CP defaultVal, IntegerOptions options)
    {
    if (val.IsEmpty())
        {
        BeAssert(val.IsEmpty());
        return defaultVal;
        }

    Utf8String str;
    ValueType vT = val.Type();
    if (vT == ValueType::Null || vT == ValueType::Undefined)
        return defaultVal;

    if (vT == ValueType::String)
        str = val.AsString();
    else if (vT == ValueType::Int32)
        ToString(str, val.AsNumber().Int32Value(), options);
    else if (vT == ValueType::UInt32)
        ToString(str, val.AsNumber().UInt32Value(), options);
    else if (vT == ValueType::Int64)
        ToString(str, val.AsNumber().Int64Value(), options);
    else if (vT == ValueType::UInt64)
        ToString(str, val.AsNumber().UInt64Value(), options);
    else if (vT == ValueType::Double)
        ToString(str, val.AsNumber().DoubleValue());
    else 
        return defaultVal;

    return str;
    }

}
END_BENTLEY_NAMESPACE
