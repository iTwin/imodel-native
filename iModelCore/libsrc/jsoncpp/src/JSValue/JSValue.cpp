/*--------------------------------------------------------------------------------------+
|
|     $Source: src/JSValue/JSValue.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Bentley/Refcounted.h>
#include <Bentley/bset.h>
#include <Bentley/bmap.h>
#include <queue>
#include <JSValue/JSValue.h>

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
struct JSValueHandle__ final
    {
    struct CompareI
        {
        bool operator ()(JSValueHandle__* lhs, JSValueHandle__* rhs) const
            {
            BeAssert(lhs->m_type == JSValueType::String && rhs->m_type == JSValueType::String);
            return lhs->m_val.str->CompareToI(*rhs->m_val.str) < 0;
            }
        };
    typedef bmap<JSValueHandle__*, JSValueHandle__*, JSValueHandle__::CompareI> Map;
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
        bvector<JSValueHandle__*>* vect;
        } m_val;

    JSValueType m_type;
    uint32_t m_count;
    };
END_BENTLEY_NAMESPACE
USING_NAMESPACE_BENTLEY
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSFactory& JSValue::Factory() const { BeAssert(m_jFactory != nullptr); return *m_jFactory; }
JSValueHandle JSValue::Handle() const { return m_value; }
bool JSValue::IsEmpty() const { return m_value == nullptr; }
bool JSValue::IsNull() const { return Type() == JSValueType::Null; }
bool JSValue::IsInt32() const { return Type() == JSValueType::Int32; }
bool JSValue::IsUInt32() const { return Type() == JSValueType::UInt32; }
bool JSValue::IsInt64() const { return Type() == JSValueType::Int64; }
bool JSValue::IsUInt64() const { return Type() == JSValueType::UInt64; }
bool JSValue::IsDouble() const { return Type() == JSValueType::Double; }
bool JSValue::IsObject() const { return Type() == JSValueType::Object; }
bool JSValue::IsArray() const { return Type() == JSValueType::Array; }
bool JSValue::IsString() const { return Type() == JSValueType::String; }
bool JSValue::IsBoolean() const { return Type() == JSValueType::Boolean; }
bool JSValue::IsNumber() const 
    { 
        const JSValueType valueType = Type();
        return 
            valueType == JSValueType::Int32 ||
            valueType == JSValueType::UInt32 ||
            valueType == JSValueType::Int64 ||
            valueType == JSValueType::UInt64 ||
            valueType == JSValueType::Double;
    }
JSValue::operator JSValueHandle () { return m_value; }
bool JSValue::ReferenceEquals(JSValue rhs) const { return m_value == rhs.m_value; }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool JSBoolean::Value() const
    {
    bool d;
    if (Factory().GetBoolean(d, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return d;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSBoolean JSBoolean::New(JSFactory& provider, bool b)
    {
    JSValueHandle val;
    if (provider.CreateBoolean(val, b) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSBoolean(provider, provider.GetNull());
        }

    return JSBoolean(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue::JSValue()
    :m_value(nullptr), m_jFactory(nullptr)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue::JSValue(JSValue const& rhs)
    : m_value(rhs.m_value), m_jFactory(rhs.m_jFactory)
    {
    if (!IsEmpty())
        {
        if (m_value && m_jFactory->NotifyScopeChanges())
            m_jFactory->BeginScope(m_value);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue::JSValue(JSFactory& prv, JSValueHandle v)
    :m_value(v), m_jFactory(&prv)
    {
    if (!m_value)
        {
        BeAssert(m_value != nullptr);
        m_jFactory = nullptr;
        }
    else
        {
        if (m_jFactory->NotifyScopeChanges())
            m_jFactory->BeginScope(m_value);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue::~JSValue()
    {
    if (!IsEmpty())
        {
        if (m_jFactory->NotifyScopeChanges())
            m_jFactory->EndScope(m_value);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSNumber JSValue::AsNumber() const
    {
    BeAssert(IsNumber());
    return JSNumber(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSString JSValue::AsString() const
    {
    BeAssert(IsString());
    return JSString(Factory(), Handle());
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSBoolean JSValue::AsBoolean() const
    {
    BeAssert(IsBoolean());
    return JSBoolean(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSObject JSValue::AsObject() const
    {
    BeAssert(IsObject());
    return JSObject(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSArray JSValue::AsArray() const
    {
    BeAssert(IsArray());
    return JSArray(Factory(), Handle());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValueType JSValue::Type() const
    {
    JSValueType type;
    if (Factory().GetType(type, m_value) != JSStatus::Success)
        {
        BeAssert(false);
        }

    return type;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int32_t JSNumber::Int32Value() const
    {
    int32_t i32;
    if (Factory().GetInt32(i32, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i32;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t JSNumber::UInt32Value() const
    {
    uint32_t i32;
    if (Factory().GetUInt32(i32, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i32;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
int64_t JSNumber::Int64Value() const
    {
    int64_t i64;
    if (Factory().GetInt64(i64, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i64;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t JSNumber::UInt64Value() const
    {
    uint64_t i64;
    if (Factory().GetUInt64(i64, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return i64;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
double JSNumber::DoubleValue() const
    {
    double d;
    if (Factory().GetDouble(d, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to get value");
        }

    return d;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSNumber JSNumber::New(JSFactory& provider, int32_t i32)
    {
    JSValueHandle val;
    if (provider.CreateInt32(val, i32) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSNumber(provider, provider.GetNull());
        }

    return JSNumber(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSNumber JSNumber::New(JSFactory& provider, uint32_t i32)
    {
    JSValueHandle val;
    if (provider.CreateUInt32(val, i32) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSNumber(provider, provider.GetNull());
        }

    return JSNumber(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSNumber JSNumber::New(JSFactory& provider, int64_t i64)
    {
    JSValueHandle val;
    if (provider.CreateInt64(val, i64) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSNumber(provider, provider.GetNull());
        }

    return JSNumber(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSNumber JSNumber::New(JSFactory& provider, uint64_t i64)
    {
    JSValueHandle val;
    if (provider.CreateUInt64(val, i64) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSNumber(provider, provider.GetNull());
        }

    return JSNumber(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSNumber JSNumber::New(JSFactory& provider, double d)
    {
    JSValueHandle val;
    if (provider.CreateDouble(val, d) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSNumber(provider, provider.GetNull());
        }

    return JSNumber(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String JSString::Value() const
    {
    Utf8String str;
    if (Factory().GetString(str, Handle()) != JSStatus::Success)
        {
        BeAssert(false);
        }

    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSString JSString::New(JSFactory& provider, Utf8CP str)
    {
    JSValueHandle val;
    if (provider.CreateString(val, str) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSString(provider, provider.GetNull());
        }

    return JSString(provider, val);
    }

JSObject::LValue::operator JSValue() const { return JSObject(m_jFactory, m_obj).Get(m_key); }
JSObject::LValue::operator JSNumber() const { return JSObject(m_jFactory, m_obj).Get(m_key).AsNumber(); }
JSObject::LValue::operator JSString() const { return JSObject(m_jFactory, m_obj).Get(m_key).AsString(); }
JSObject::LValue::operator JSArray() const { return JSObject(m_jFactory, m_obj).Get(m_key).AsArray(); }
JSObject::LValue::operator JSObject() const { return JSObject(m_jFactory, m_obj).Get(m_key).AsObject(); }
JSObject::LValue::operator JSBoolean() const { return JSObject(m_jFactory, m_obj).Get(m_key).AsBoolean(); }

JSObject::LValue& JSObject::LValue::operator = (JSValue value) { JSObject(m_jFactory, m_obj).Set(m_key, value);  return *this; }
JSObject::LValue JSObject::operator [](Utf8CP key) { return LValue(Factory(), Handle(), key); }
JSObject::LValue JSObject::operator [](Utf8StringCR key) { return LValue(Factory(), Handle(), key); }
JSObject::LValue JSObject::operator [](JSString key) { return LValue(Factory(), Handle(), key.Value()); }


JSArray::LValue::operator JSValue() const { return JSArray(m_jFactory, m_obj).Get(m_key); }
JSArray::LValue::operator JSNumber() const { return JSArray(m_jFactory, m_obj).Get(m_key).AsNumber(); }
JSArray::LValue::operator JSString() const { return JSArray(m_jFactory, m_obj).Get(m_key).AsString(); }
JSArray::LValue::operator JSArray() const { return JSArray(m_jFactory, m_obj).Get(m_key).AsArray(); }
JSArray::LValue::operator JSObject() const { return JSArray(m_jFactory, m_obj).Get(m_key).AsObject(); }
JSArray::LValue::operator JSBoolean() const { return JSArray(m_jFactory, m_obj).Get(m_key).AsBoolean(); }

JSArray::LValue& JSArray::LValue::operator = (JSValue value) { JSArray(m_jFactory, m_obj).Set(m_key, value);  return *this; }
JSArray::LValue JSArray::operator [](uint32_t index) { return LValue(Factory(), Handle(), index); }
JSArray::LValue JSArray::operator [](JSNumber index) { return LValue(Factory(), Handle(), index.UInt32Value()); }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSArray JSArray::New(JSFactory& provider, uint32_t size)
    {
    JSValueHandle val;
    if (provider.CreateArray(val, size) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSArray(provider, provider.GetNull());
        }

    return JSArray(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSObject JSObject::New(JSFactory& provider)
    {
    JSValueHandle val;
    if (provider.CreateObject(val) != JSStatus::Success)
        {
        BeAssert(false && "Fail to create value");
        return JSObject(provider, provider.GetNull());
        }

    return JSObject(provider, val);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t JSArray::Length() const
    {
    uint32_t sz;
    if (Factory().GetSize(sz, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to size");
        return 0;
        }

    return sz;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t JSObject::Count() const
    {
    uint32_t sz;
    if (Factory().GetSize(sz, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to size");
        return 0;
        }

    return sz;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t JSString::Length() const
    {
    uint32_t sz;
    if (Factory().GetSize(sz, Handle()) != JSStatus::Success)
        {
        BeAssert(false && "Fail to size");
        return 0;
        }

    return sz;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue JSArray::Get(uint32_t index) const
    {
    JSValueHandle v;
    if (Factory().GetMember(v, Handle(), index) != JSStatus::Success)
        {
        BeAssert(false && "Fail to size");
        return JSValue(Factory(), Factory().GetNull());
        }

    return JSValue(Factory(), v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void JSArray::Set(uint32_t index, JSValue value)
    {
    if (Factory().SetMember(Handle(), index, value) != JSStatus::Success)
        {
        BeAssert(false && "Fail to size");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue JSObject::Get(Utf8CP key) const
    {
    JSValueHandle v;
    if (Factory().GetMember(v, Handle(), key) != JSStatus::Success)
        {
        BeAssert(false && "Fail to size");
        return JSValue(Factory(), Factory().GetNull());
        }

    return JSValue(Factory(), v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue JSObject::Get(Utf8StringCR key) const
    {
    return Get(key.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void JSObject::Set(Utf8CP key, JSValue value)
    {
    if (Factory().SetMember(Handle(), key, value) != JSStatus::Success)
        {
        BeAssert(false && "Fail to size");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
void JSObject::Set(Utf8StringCR key, JSValue value)
    {
    Set(key.c_str(), value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool JSObject::HasMember(Utf8CP key) const
    {
    bool foundIt;
    if (Factory().HasMember(foundIt, Handle(), key) != JSStatus::Success)
        {
        BeAssert(false);
        return false;
        }

    return foundIt;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
bool JSObject::HasMember(Utf8StringCR key) const
    {
    return HasMember(key.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSArray JSObject::GetMemberNames() const
    {
    JSValueHandle val;
    if (Factory().GetMemberNames(val, Handle()) != JSStatus::Success)
        {
        BeAssert(false);
        return JSArray(Factory(), Factory().GetNull());
        }

    return JSArray(Factory(), val);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
struct GCJValueFactoryImpl : public RefCounted<JSFactory>
    {
    private:
        enum class CachedObjectType
            {
            Primitive,
            String,
            Vector,
            Map,
            };

        mutable bmap<CachedObjectType, bset<JSValueHandle__*>> m_cacheUsed;
        mutable bmap<CachedObjectType, std::queue<JSValueHandle__*>> m_cacheUnused;
        mutable bmap<JSValueType, CachedObjectType> m_cacheType;
        mutable bmap<CachedObjectType, int> m_cacheMaxSize;
        JSValueHandle__* m_null;
        JSStatus TrimUnusedItemCache(CachedObjectType type, float extraTrim = 0)
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

            return JSStatus::Success;
            }

        JSStatus TrimUnusedItemCache(float extraTrim = 0)
            {
            JSStatus status = JSStatus::Success;
            for (auto itor : m_cacheMaxSize)
                {
                if (m_cacheUnused[itor.first].size() > m_cacheMaxSize[itor.first])
                    {
                    status = TrimUnusedItemCache(itor.first, extraTrim);
                    if (status != JSStatus::Success)
                        return status;
                    }
                }

            return status;
            }

        JSStatus CreateInstance(JSValueHandle& v, JSValueType type, uint32_t sz = 0)
            {
            v = nullptr;
            const CachedObjectType cacheType = m_cacheType[type];
            std::queue<JSValueHandle__*>& q = m_cacheUnused[cacheType];
            if (!q.empty())
                {
                v = q.front();
                q.pop();
                }

            if (v)
                {
                if (type == JSValueType::Object)
                    {
                    auto& map = *(v->m_val.map);
                    for (auto& pv : map)
                        {
                        EndScope(pv.first);
                        EndScope(pv.second);
                        }

                    map.clear();
                    }
                else if (type == JSValueType::Array)
                    {
                    for (auto pv : *(v->m_val.vect))
                        EndScope(pv);

                    v->m_val.vect->resize(sz, m_null);

                    }
                else if (type == JSValueType::String)
                    {
                    v->m_val.str->clear();
                    }
                else
                    {
                    memset(v, 0, sizeof(JSValueHandle__));
                    v->m_type = type;
                    }
                }
            else
                {
                v = new JSValueHandle__();
                memset(v, 0, sizeof(JSValueHandle__));
                v->m_type = type;
                if (type == JSValueType::String)
                    v->m_val.str = new Utf8String();
                else if (type == JSValueType::Object)
                    v->m_val.map = new JSValueHandle__::Map();
                else if (type == JSValueType::Array)
                    {
                    v->m_val.vect = new bvector<JSValueHandle__*>();
                    v->m_val.vect->resize(sz, m_null);
                    }
                }

            v->m_count = 0;
            m_cacheUsed[cacheType].insert(v);
            return JSStatus::Success;
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

        void Free(JSValueHandle& v)
            {
            if (v->m_type == JSValueType::String)
                delete v->m_val.str;
            else if (v->m_type == JSValueType::Object)
                delete  v->m_val.map;
            else if (v->m_type == JSValueType::Array)
                delete  v->m_val.vect;

            free(v);
            }

        bool IsPrimitve(JSValueType t) const
            {
            return !(t == JSValueType::Array || t == JSValueType::Object || t == JSValueType::String);
            }

        JSStatus EndScopeForValue(JSValueHandle v) const
            {
            BeAssert(v->m_count > 0);
            v->m_count = v->m_count - 1;

            if (v->m_count <= 0)
                {
                if (v->m_type == JSValueType::Object)
                    {
                    auto& map = *(v->m_val.map);
                    for (auto& pv : map)
                        {
                        EndScopeForValue(pv.first);
                        EndScopeForValue(pv.second);
                        }

                    map.clear();
                    }
                else if (v->m_type == JSValueType::Array)
                    {
                    auto& vect = *(v->m_val.vect);
                    for (auto pv : vect)
                        EndScopeForValue(pv);

                    vect.clear();
                    }
                else if (v->m_type == JSValueType::String)
                    {
                    v->m_val.str->clear();
                    }

                const CachedObjectType cacheType = m_cacheType[v->m_type];
                m_cacheUsed[cacheType].erase(v);
                m_cacheUnused[cacheType].push(v);
                }

            return JSStatus::Success;
            }
    public:

        GCJValueFactoryImpl(uint32_t maxStrings, uint32_t maxArray, uint32_t maxObj, uint32_t maxPri)
            {
            m_cacheType[JSValueType::Int32] = CachedObjectType::Primitive;
            m_cacheType[JSValueType::UInt32] = CachedObjectType::Primitive;
            m_cacheType[JSValueType::Int64] = CachedObjectType::Primitive;
            m_cacheType[JSValueType::UInt64] = CachedObjectType::Primitive;
            m_cacheType[JSValueType::Boolean] = CachedObjectType::Primitive;
            m_cacheType[JSValueType::Double] = CachedObjectType::Primitive;
            m_cacheType[JSValueType::Null] = CachedObjectType::Primitive;
            m_cacheType[JSValueType::Array] = CachedObjectType::Vector;
            m_cacheType[JSValueType::Object] = CachedObjectType::Map;
            m_cacheType[JSValueType::String] = CachedObjectType::String;
            m_cacheMaxSize[CachedObjectType::String] = maxStrings < 10 ? 10 : maxStrings;
            m_cacheMaxSize[CachedObjectType::Vector] = maxArray < 10 ? 10 : maxArray;
            m_cacheMaxSize[CachedObjectType::Map] = maxObj < 10 ? 10 : maxObj;
            m_cacheMaxSize[CachedObjectType::Primitive] = maxPri < 10 ? 10 : maxPri;
            CreateInstance(m_null, JSValueType::Null);
            }

        ~GCJValueFactoryImpl()
            {
            ClearCache();
            }

        /////////////////
        JSStatus CreateBoolean(JSValueHandle& val, bool b) override
            {
            JSStatus status = CreateInstance(val, JSValueType::Boolean);
            if (status != JSStatus::Success)
                return status;

            val->m_val.b = b;
            return JSStatus::Success;
            }
        JSStatus CreateString(JSValueHandle& val, Utf8CP str) override
            {
            JSStatus status = CreateInstance(val, JSValueType::String);
            if (status != JSStatus::Success)
                return status;

            val->m_val.str->AssignOrClear(str);
            return JSStatus::Success;
            }
        JSStatus CreateInt32(JSValueHandle& val, int32_t i32) override
            {
            JSStatus status = CreateInstance(val, JSValueType::Int32);
            if (status != JSStatus::Success)
                return status;

            val->m_val.i32 = i32;
            return JSStatus::Success;
            }
        JSStatus CreateUInt32(JSValueHandle& val, uint32_t i32) override
            {
            JSStatus status = CreateInstance(val, JSValueType::Int32);
            if (status != JSStatus::Success)
                return status;

            val->m_val.ui32 = i32;
            return JSStatus::Success;
            }

        JSStatus CreateUInt64(JSValueHandle& val, uint64_t i64) override
            {
            JSStatus status = CreateInstance(val, JSValueType::UInt64);
            if (status != JSStatus::Success)
                return status;

            val->m_val.ui64 = i64;
            return JSStatus::Success;
            }

        JSStatus CreateInt64(JSValueHandle& val, int64_t i64) override
            {
            JSStatus status = CreateInstance(val, JSValueType::Int64);
            if (status != JSStatus::Success)
                return status;

            val->m_val.i64 = i64;
            return JSStatus::Success;
            }

        JSStatus CreateDouble(JSValueHandle& val, double d) override
            {
            JSStatus status = CreateInstance(val, JSValueType::Double);
            if (status != JSStatus::Success)
                return status;

            val->m_val.d = d;
            return JSStatus::Success;
            }
        JSStatus CreateObject(JSValueHandle& val) override
            {
            JSStatus status = CreateInstance(val, JSValueType::Object);
            if (status != JSStatus::Success)
                return status;

            return JSStatus::Success;
            }

        JSStatus CreateArray(JSValueHandle& val, uint32_t size) override
            {
            JSStatus status = CreateInstance(val, JSValueType::Array, size);
            if (status != JSStatus::Success)
                return status;

            return JSStatus::Success;
            }

        JSStatus GetBoolean(bool& b, JSValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                b = false;
                return JSStatus::IncorrectType;
                }

            b = val->m_val.b;
            return JSStatus::Success;
            }
        JSStatus GetString(Utf8String& s, JSValueHandle val) const override
            {
            if (val->m_type != JSValueType::String)
                return JSStatus::IncorrectType;

            s = *(val->m_val.str);
            return JSStatus::Success;
            }
        JSStatus GetInt32(int32_t& i32, JSValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i32 = 0;
                return JSStatus::IncorrectType;
                }

            i32 = val->m_val.i32;
            return JSStatus::Success;
            }
        JSStatus GetUInt32(uint32_t& i32, JSValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i32 = 0;
                return JSStatus::IncorrectType;
                }

            i32 = val->m_val.ui32;
            return JSStatus::Success;
            }
        JSStatus GetInt64(int64_t& i64, JSValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i64 = 0;
                return JSStatus::IncorrectType;
                }

            i64 = val->m_val.i64;
            return JSStatus::Success;
            }
        JSStatus GetUInt64(uint64_t& i64, JSValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                i64 = 0;
                return JSStatus::IncorrectType;
                }

            i64 = val->m_val.ui64;
            return JSStatus::Success;
            }
        JSStatus GetDouble(double& d, JSValueHandle val) const override
            {
            if (!IsPrimitve(val->m_type))
                {
                d = 0;
                return JSStatus::IncorrectType;
                }

            d = val->m_val.d;
            return JSStatus::Success;
            }

        JSStatus GetMemberNames(JSValueHandle& members, JSValueHandle val) const override
            {
            if (val->m_type != JSValueType::Object)
                {
                members = m_null;
                return JSStatus::IncorrectType;
                }

            uint32_t sz;
            JSStatus status = GetSize(sz, val);
            if (status != JSStatus::Success)
                return status;

            status = const_cast<GCJValueFactoryImpl*>(this)->CreateArray(members, sz);
            if (status != JSStatus::Success)
                return status;

            uint32_t idx = 0;
            for (auto itor = val->m_val.map->begin(); itor != val->m_val.map->end(); ++itor)
                (*members->m_val.vect)[idx++] = itor->first;

            return JSStatus::Success;
            }

        JSStatus HasMember(bool& found, JSValueHandle val, Utf8CP key) const override
            {
            if (val->m_type != JSValueType::Object)
                return JSStatus::IncorrectType;

            JSValueHandle vt;
            JSStatus status = const_cast<GCJValueFactoryImpl*>(this)->CreateString(vt, key);
            if (status != JSStatus::Success)
                return status;

            BeginScope(vt);
            found = val->m_val.map->find(vt) != val->m_val.map->end();
            return EndScope(vt);
            }

        JSStatus GetMember(JSValueHandle& rt, JSValueHandle obj, Utf8CP key) const override
            {
            JSValueHandle vt;
            JSStatus status = const_cast<GCJValueFactoryImpl*>(this)->CreateString(vt, key);
            if (status != JSStatus::Success)
                return status;

            const_cast<GCJValueFactoryImpl*>(this)->BeginScope(vt);
            status = GetMember(rt, obj, vt);
            const_cast<GCJValueFactoryImpl*>(this)->EndScope(vt);
            return status;
            }

        JSStatus GetMember(JSValueHandle& rt, JSValueHandle obj, uint32_t index) const override
            {
            JSValueHandle vt;
            JSStatus status = const_cast<GCJValueFactoryImpl*>(this)->CreateUInt32(vt, index);
            if (status != JSStatus::Success)
                return status;

            const_cast<GCJValueFactoryImpl*>(this)->BeginScope(vt);
            status = GetMember(rt, obj, vt);
            const_cast<GCJValueFactoryImpl*>(this)->EndScope(vt);
            return status;
            }

        JSStatus GetMember(JSValueHandle& rt, JSValueHandle obj, JSValueHandle index) const override
            {
            rt = m_null;
            if (obj->m_type == JSValueType::Object)
                {
                auto itor = obj->m_val.map->find(index);
                if (itor != obj->m_val.map->end())
                    rt = itor->second;

                return JSStatus::Success;
                }

            if (obj->m_type == JSValueType::Array)
                {
                uint32_t idx;
                JSStatus status = GetUInt32(idx, index);
                if (status != JSStatus::Success)
                    return status;

                rt = obj->m_val.vect->at(idx);
                return JSStatus::Success;
                }

            return JSStatus::IncorrectType;
            }

        JSStatus GetSize(uint32_t& n, JSValueHandle val) const override
            {
            n = 0;
            if (val->m_type == JSValueType::Object)
                {
                n = (uint32_t) val->m_val.map->size();
                return JSStatus::Success;
                }

            if (val->m_type == JSValueType::Array)
                {
                n = (uint32_t) val->m_val.vect->size();
                return JSStatus::Success;
                }

            return JSStatus::IncorrectType;
            }

        JSStatus SetMember(JSValueHandle obj, Utf8CP key, JSValueHandle val) override
            {
            if (obj->m_type != JSValueType::Object)
                return JSStatus::IncorrectType;

            JSValueHandle str;
            JSStatus status = CreateString(str, key);
            if (status != JSStatus::Success)
                return status;

            BeginScope(str);
            status = SetMember(obj, str, val);
            EndScope(str);
            return status;
            }
        JSStatus SetMember(JSValueHandle obj, uint32_t index, JSValueHandle val) override
            {
            if (obj->m_type != JSValueType::Array)
                return JSStatus::IncorrectType;

            JSValueHandle str;
            JSStatus status = CreateUInt32(str, index);
            if (status != JSStatus::Success)
                return status;

            BeginScope(str);
            status = SetMember(obj, str, val);
            EndScope(str);
            return status;
            }
        JSStatus SetMember(JSValueHandle obj, JSValueHandle index, JSValueHandle val) override
            {
            if (obj->m_type == JSValueType::Object)
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
                return JSStatus::Success;
                }
            else if (obj->m_type == JSValueType::Array)
                {
                auto& vect = *(obj->m_val.vect);
                uint32_t idx;
                JSStatus status = GetUInt32(idx, index);
                if (status != JSStatus::Success)
                    return status;

                if (idx >= vect.size())
                    return JSStatus::IndexOutOfRange;

                auto itor = vect.begin() + idx;
                EndScope(*itor);
                BeginScope(val);
                *itor = val;
                return JSStatus::Success;
                }

            return JSStatus::IncorrectType;
            }
        JSStatus GetType(JSValueType& t, JSValueHandle val) const override
            {
            if (val == nullptr)
                return JSStatus::NullArgument;

            t = val->m_type;
            return JSStatus::Success;
            }
        JSValueHandle GetNull() const override { return m_null; }
        JSStatus BeginScope(JSValueHandle val) const  override
            {
            //This protect variable from deletion.
            //It shoudl stay above 1 to be not garbage collected
            val->m_count++;
            return JSStatus::Success;
            }

        JSStatus EndScope(JSValueHandle val) const override
            {
            if (val == m_null)
                return JSStatus::Success;

            if (EndScopeForValue(val) != JSStatus::Success)
                return JSStatus::Error;

            if (val->m_count <= 0)
                {
                const_cast<GCJValueFactoryImpl*>(this)->TrimUnusedItemCache(10.0f);
                }

            return JSStatus::Success;
            }

        bool NotifyScopeChanges() const override { return true; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
RefCountedPtr<JSFactory> GCJValueFactory::CreateInstance(Options const& opt)
    {
    return new GCJValueFactoryImpl(opt.m_maxStringValueCache,
                                   opt.m_maxArrayValueCache,
                                   opt.m_maxObjectValueCache,
                                   opt.m_maxPrimitiveValueCache);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
Json::Value JSValue::ToJson() const
    {        
    Json::Value v;
    if (_ToJson(v) != JSStatus::Success)
        {
        BeAssert(false && "Fail to convert JSValue to Json::Value");
        return Json::Value(Json::ValueType::nullValue);
        }

    return v;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
 JSStatus JSValue::_ToJson(Json::Value& v) const
    {        
    v = Json::Value(Json::ValueType::nullValue);;        
    JSValueType valueType = Type();
    if (IsEmpty() || valueType != JSValueType::Null)
        {
        BeAssert(false && "Fail to convert JSValue to Json::Value");
        return JSStatus::IncorrectType;
        }
    
    return JSStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
 JSStatus JSBoolean::_ToJson(Json::Value& v) const
    {        
    const JSValueType valueType = Type();
    if (IsEmpty() || valueType != JSValueType::Boolean)
        {
        BeAssert(false && "Fail to convert JSBoolean to Json::Value");
        v = Json::Value(Json::ValueType::nullValue);
        return JSStatus::IncorrectType;
        }
    
    if (valueType == JSValueType::Null)
        v = Json::Value(Json::ValueType::nullValue);
    else
        v = Json::Value(Value());

    return JSStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
 JSStatus JSString::_ToJson(Json::Value& v) const
    {        
    const JSValueType valueType = Type();
    if (IsEmpty() || valueType != JSValueType::String)
        {
        BeAssert(false && "Fail to convert JSString to Json::Value");
        v = Json::Value(Json::ValueType::nullValue);
        return JSStatus::IncorrectType;
        }

    if (valueType == JSValueType::Null)
        v = Json::Value(Json::ValueType::nullValue);
    else
        v = Json::Value(Value());

    return JSStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
 JSStatus JSNumber::_ToJson(Json::Value& v) const
    {    
    if (IsEmpty() || IsNumber())
        {
        BeAssert(false && "Fail to convert JSNumber to Json::Value");
        v = Json::Value(Json::ValueType::nullValue);
        return JSStatus::IncorrectType;
        }

    const JSValueType valueType = Type();
    if(valueType == JSValueType::Int32)
        v = Json::Value(Int32Value());
    else if(valueType == JSValueType::UInt32)
        v = Json::Value(UInt32Value());
    else if(valueType == JSValueType::UInt64)
        v = Json::Value(Int64Value());
    else if(valueType == JSValueType::Int64)
        v = Json::Value(UInt64Value());
    else if(valueType == JSValueType::Double)
        v = Json::Value(DoubleValue());
    else
        v = Json::Value(Json::ValueType::nullValue);

    return JSStatus::Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
 JSStatus JSArray::_ToJson(Json::Value& v) const
    {        
    const JSValueType valueType = Type();
    if (IsEmpty() || valueType != JSValueType::Array)
        {
        BeAssert(false && "Fail to convert JSArray to Json::Value");
        v = Json::Value(Json::ValueType::nullValue);
        return JSStatus::IncorrectType;
        }

    if (valueType == JSValueType::Null)
        v = Json::Value(Json::ValueType::nullValue);
    else
        {        
        v = Json::Value(Json::ValueType::arrayValue);
        const uint32_t n = Length();
        for(uint32_t i = 0; i < n; ++i)
            v[i] = Get(i).ToJson();
        }
    return JSStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
 JSStatus JSObject::_ToJson(Json::Value& v) const
    {    
    const JSValueType valueType = Type();           
    if (IsEmpty() || valueType != JSValueType::Object)
        {
        BeAssert(false && "Fail to convert JSArray to Json::Value");
        v = Json::Value(Json::ValueType::nullValue);
        return JSStatus::IncorrectType;
        }

    if (valueType == JSValueType::Null)
        v = Json::Value(Json::ValueType::nullValue);
    else
        {        
        v = Json::Value(Json::ValueType::objectValue);
        JSArray members = GetMemberNames();
        const uint32_t n = members.Length();
        for(uint32_t i = 0; i < n; ++i)
            v[i] = Get(members.Get(i).AsString().Value()).ToJson();
        }

    return JSStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------    
JSValue JSValue::Null(JSFactory& factory)
    {
    return JSValue(factory, factory.GetNull());  
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                      05/2018
//+---------------+---------------+---------------+---------------+---------------+------
JSValue JSValue::FromJson(JSFactory& factory, Json::Value const& v)
    {
    if (v.type() == Json::ValueType::nullValue)
        return JSValue::Null(factory);
    else if (v.type() == Json::ValueType::intValue)
        return JSNumber::New(factory, v.asInt());
    else if (v.type() == Json::ValueType::uintValue)
        return JSNumber::New(factory, v.asUInt());
    else if (v.type() == Json::ValueType::realValue)
        return JSNumber::New(factory, v.asDouble());
    else if (v.type() == Json::ValueType::arrayValue)
        {
        JSArray jsArray = JSArray::New(factory, (uint32_t)v.size());
        for (auto itor = v.begin(); itor != v.end(); ++itor)
            jsArray.Set((uint32_t)itor.index(), FromJson(factory, (*itor)));

        return jsArray;
        }
    else if (v.type() == Json::ValueType::objectValue)
        {
        JSObject jsObject = JSObject::New(factory);
        for (auto itor = v.begin(); itor != v.end(); ++itor)
            jsObject.Set(itor.memberName(), FromJson(factory, (*itor)));

        return jsObject;
        }

    BeAssert(false && "Unknown type");
    return JSValue::Null(factory);        
    }
