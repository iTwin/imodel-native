/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/ECPresentationTypes.h>
#include <ECDb/ECDbApi.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE
USING_NAMESPACE_BENTLEY_EC

struct BoundQueryECValue;
struct BoundQueryId;
struct BoundQueryIdSet;
struct BoundECValueSet;
struct BoundRapidJsonValueSet;
struct BoundQueryValue;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IBoundQueryValueSerializer
    {
    virtual ~IBoundQueryValueSerializer() {}
    virtual rapidjson::Document _ToJson(BoundQueryECValue const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _ToJson(BoundQueryId const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _ToJson(BoundQueryIdSet const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _ToJson(BoundECValueSet const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual rapidjson::Document _ToJson(BoundRapidJsonValueSet const&, rapidjson::Document::AllocatorType*) const = 0;
    virtual std::unique_ptr<BoundQueryValue> _FromJson(BeJsConst const&) { return nullptr; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DefaultBoundQueryValueSerializer : IBoundQueryValueSerializer
    {
    ~DefaultBoundQueryValueSerializer() {}
    rapidjson::Document _ToJson(BoundQueryECValue const&, rapidjson::Document::AllocatorType*) const;
    rapidjson::Document _ToJson(BoundQueryId const&, rapidjson::Document::AllocatorType*) const;
    rapidjson::Document _ToJson(BoundQueryIdSet const&, rapidjson::Document::AllocatorType*) const;
    rapidjson::Document _ToJson(BoundECValueSet const&, rapidjson::Document::AllocatorType*) const;
    rapidjson::Document _ToJson(BoundRapidJsonValueSet const&, rapidjson::Document::AllocatorType*) const;
    std::unique_ptr<BoundQueryValue> _FromJson(BeJsConst const&);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PrimitiveECValueHasher
    {
    size_t operator()(ECValueCR value) const;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECValueVirtualSet : BeSQLite::VirtualSet
    {
    private:
        std::unordered_set<ECValue, PrimitiveECValueHasher> m_values;
    public:
        ECValueVirtualSet(bvector<ECValue> values);
        std::unordered_set<ECValue, PrimitiveECValueHasher> const& GetValues() const { return m_values; }
        bool Equals(ECValueVirtualSet const& otherSet) const;
        void Insert(ECValue value);
        bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryValue
    {
    protected:
        virtual bool _Equals(BoundQueryValue const&) const = 0;
        virtual BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const = 0;
    public:
        virtual ~BoundQueryValue() {}
        virtual rapidjson::Document ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const = 0;
        bool Equals(BoundQueryValue const& other) const { return _Equals(other); }
        bool operator==(BoundQueryValue const& other) const { return Equals(other); }
        bool operator!=(BoundQueryValue const& other) const { return !Equals(other); }
        BeSQLite::EC::ECSqlStatus Bind(BeSQLite::EC::ECSqlStatement& stmt, uint32_t index) const { return _Bind(stmt, index); }
        static std::unique_ptr<BoundQueryValue> FromJson(IBoundQueryValueSerializer &serializer, BeJsConst const& json) {
            json.Stringify(StringifyFormat::Indented);
            return serializer._FromJson(json);
            }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryECValue : BoundQueryValue
    {
    private:
        ECValue m_value;
    protected:
        ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
        ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
    public:
        BoundQueryECValue(ECValue value) : m_value(std::move(value)) {}
        rapidjson::Document ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
            {
            return serializer._ToJson(*this, alloc);
            }
        ECValue const& GetValue() const { return m_value; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryId : BoundQueryValue
    {
    private:
        BeInt64Id m_id;
    protected:
        ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
        ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
    public:
        BoundQueryId(BeInt64Id id) : m_id(id) {}
        BoundQueryId(Utf8StringCR idStr) { BeInt64Id::FromString(m_id, idStr.c_str()); }
        rapidjson::Document ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
            {
            return serializer._ToJson(*this, alloc);
            }
        BeInt64Id const& GetId() const { return m_id; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryIdSet : BoundQueryValue
    {
    private:
        BeSQLite::IdSet<BeInt64Id> m_set;
    protected:
        ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
        ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
    public:
        BoundQueryIdSet(BeSQLite::IdSet<BeInt64Id>&& set) : m_set(std::move(set)) {}
        template<typename TId> BoundQueryIdSet(bvector<TId> const& vec)
            {
            for (auto const& id : vec)
                m_set.insert(id);
            }
        BoundQueryIdSet(bvector<BeSQLite::EC::ECInstanceKey> const& vec)
            {
            for (BeSQLite::EC::ECInstanceKey const& key : vec)
                m_set.insert(key.GetInstanceId());
            }
        rapidjson::Document ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
            {
            return serializer._ToJson(*this, alloc);
            }
        BeSQLite::IdSet<BeInt64Id> const& GetSet() const { return m_set; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundECValueSet : BoundQueryValue
    {
    private:
        std::unique_ptr<BeSQLite::VirtualSet> m_set;
    protected:
        ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
        ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
    public:
        rapidjson::Document ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
            {
            return serializer._ToJson(*this, alloc);
            }
        ECPRESENTATION_EXPORT BoundECValueSet(bvector<ECValue>);
        ECPRESENTATION_EXPORT BoundECValueSet(BoundECValueSet const& other);
        std::unique_ptr<BeSQLite::VirtualSet> const& GetSet() const { return m_set; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundRapidJsonValueSet : BoundQueryValue
    {
    private:
        std::unique_ptr<BeSQLite::VirtualSet> m_set;
    protected:
        ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
        ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
    public:
        rapidjson::Document ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
            {
            return serializer._ToJson(*this, alloc);
            }
        ECPRESENTATION_EXPORT BoundRapidJsonValueSet(RapidJsonValueCR values, PrimitiveType type);
        ECPRESENTATION_EXPORT BoundRapidJsonValueSet(BoundRapidJsonValueSet const& other);
        std::unique_ptr<BeSQLite::VirtualSet> const& GetSet() const { return m_set; }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct BoundQueryValuesList : bvector<std::shared_ptr<BoundQueryValue const>>
    {
    BoundQueryValuesList() {}
    BoundQueryValuesList(std::initializer_list<std::shared_ptr<BoundQueryValue const>> values)
        : bvector<std::shared_ptr<BoundQueryValue const>>(std::move(values))
        {}
    bool operator==(BoundQueryValuesList const& other) const
        {
        if (size() != other.size())
            return false;
        for (size_t i = 0; i < size(); ++i)
            {
            if (!at(i)->Equals(*other[i]))
                return false;
            }
        return true;
        }
    ECPRESENTATION_EXPORT BentleyStatus Bind(BeSQLite::EC::ECSqlStatement&) const;
    ECPRESENTATION_EXPORT BentleyStatus FromJson(IBoundQueryValueSerializer&, BeJsConst);
    };

typedef BoundQueryValuesList const& BoundQueryValuesListCR;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PresentationQuery
    {
    private:
        Utf8String m_query;
        BoundQueryValuesList m_bindings;

    public:
        PresentationQuery() {}
        PresentationQuery(Utf8StringCR query, BoundQueryValuesList bindings = {})
            : m_query(query), m_bindings(bindings)
            {}
        std::unique_ptr<PresentationQuery> Clone() const
            {
            return std::make_unique<PresentationQuery>(GetQueryString(), GetBindings());
            }
        bool IsEqual(PresentationQuery const& other) const
            {
            return GetQueryString().Equals(other.GetQueryString())
                && GetBindings() == other.GetBindings();
            }
        Utf8StringCR GetQueryString() const { return m_query; }
        Utf8StringR GetQueryString() { return m_query; }
        BoundQueryValuesList const& GetBindings() const { return m_bindings; }
        BoundQueryValuesList& GetBindings() { return m_bindings; }

        BentleyStatus BindValues(BeSQLite::EC::ECSqlStatement& stmt) const { return GetBindings().Bind(stmt); }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RapidJsonValueComparer
    {
    bool operator() (rapidjson::Value const* left, rapidjson::Value const* right) const;
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RapidJsonValueSet : BeSQLite::VirtualSet
    {
    private:
        PrimitiveType m_type;
        rapidjson::Document m_jsonValues;
        bset<rapidjson::Value*, RapidJsonValueComparer> m_keys;

    public:
        RapidJsonValueSet(RapidJsonValueCR values, PrimitiveType type) : m_type(type)
            {
            m_jsonValues.SetArray();
            for (rapidjson::SizeType i = 0; i < values.Size(); i++)
                {
                if (PRIMITIVETYPE_Point2d == m_type || PRIMITIVETYPE_Point3d == m_type)
                    {
                    if (values[i].IsString())
                        m_jsonValues.PushBack(rapidjson::Value(values[i], m_jsonValues.GetAllocator()), m_jsonValues.GetAllocator());
                    else if (values[i].IsObject())
                        m_jsonValues.PushBack(rapidjson::Value(BeRapidJsonUtilities::ToString(values[i]).c_str(), m_jsonValues.GetAllocator()), m_jsonValues.GetAllocator());
                    else
                        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Expected Point value type to be a JSON object or JSON string. Actual: %d", (int)values[i].GetType()));
                    }
                else
                    m_jsonValues.PushBack(rapidjson::Value(values[i], m_jsonValues.GetAllocator()), m_jsonValues.GetAllocator());
                m_keys.insert(&m_jsonValues[i]);
                }
            }
        RapidJsonValueSet(RapidJsonValueSet const& other)
            : m_type(other.m_type)
            {
            m_jsonValues.CopyFrom(other.m_jsonValues, m_jsonValues.GetAllocator());
            for (rapidjson::SizeType i = 0; i < m_jsonValues.Size(); i++)
                m_keys.insert(&m_jsonValues[i]);
            }
        PrimitiveType GetValuesType() const { return m_type; }
        RapidJsonDocumentCR GetValuesJson() const { return m_jsonValues; }
        bool Equals(RapidJsonValueSet const& otherSet) const;
        bool _IsInSet(int nVals, BeSQLite::DbValue const* vals) const override;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
