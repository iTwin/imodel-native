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
    virtual std::unique_ptr<BoundQueryValue> _FromJson(BeJsConst const) = 0;
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
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryValue
{
protected:
    virtual bool _Equals(BoundQueryValue const&) const = 0;
    virtual BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const = 0;
    virtual rapidjson::Document _ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const = 0;
public:
    virtual ~BoundQueryValue() {}
    bool Equals(BoundQueryValue const& other) const { return _Equals(other); }
    bool operator==(BoundQueryValue const& other) const { return Equals(other); }
    bool operator!=(BoundQueryValue const& other) const { return !Equals(other); }
    BeSQLite::EC::ECSqlStatus Bind(BeSQLite::EC::ECSqlStatement& stmt, uint32_t index) const { return _Bind(stmt, index); }
    rapidjson::Document ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const { return _ToJson(serializer, alloc); };
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
    rapidjson::Document _ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
        {
        return serializer._ToJson(*this, alloc);
        }
public:
    BoundQueryECValue(ECValue value) : m_value(std::move(value)) {}
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
    rapidjson::Document _ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
        {
        return serializer._ToJson(*this, alloc);
        }
public:
    BoundQueryId(BeInt64Id id) : m_id(id) {}
    BoundQueryId(Utf8StringCR idStr) { BeInt64Id::FromString(m_id, idStr.c_str()); }
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
    rapidjson::Document _ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
        {
        return serializer._ToJson(*this, alloc);
        }
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
    rapidjson::Document _ToJson(IBoundQueryValueSerializer const& serializer, rapidjson::Document::AllocatorType* alloc) const override
        {
        return serializer._ToJson(*this, alloc);
        }
public:
    ECPRESENTATION_EXPORT BoundECValueSet(bvector<ECValue>);
    ECPRESENTATION_EXPORT BoundECValueSet(BoundECValueSet const& other);
    ECPRESENTATION_EXPORT Nullable<PrimitiveType> GetValueType() const;
    ECPRESENTATION_EXPORT void ForEachValue(std::function<void(ECValue const&)> const& cb) const;
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

END_BENTLEY_ECPRESENTATION_NAMESPACE
