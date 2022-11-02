/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "../../RulesEngineTypes.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryValue
{
protected:
    virtual bool _Equals(BoundQueryValue const&) const = 0;
    virtual BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const = 0;
    virtual rapidjson::Document _ToJson(rapidjson::Document::AllocatorType*) const = 0;
public:
    virtual ~BoundQueryValue() {}
    bool Equals(BoundQueryValue const& other) const {return _Equals(other);}
    bool operator==(BoundQueryValue const& other) const {return Equals(other);}
    bool operator!=(BoundQueryValue const& other) const {return !Equals(other);}
    BeSQLite::EC::ECSqlStatus Bind(BeSQLite::EC::ECSqlStatement& stmt, uint32_t index) const {return _Bind(stmt, index);}
    rapidjson::Document ToJson(rapidjson::Document::AllocatorType* alloc = nullptr) const {return _ToJson(alloc);}
    static std::unique_ptr<BoundQueryValue> FromJson(RapidJsonValueCR);
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
    ECPRESENTATION_EXPORT BentleyStatus Bind(ECSqlStatement&) const;
    ECPRESENTATION_EXPORT rapidjson::Document ToJson(rapidjson::Document::AllocatorType* alloc = nullptr) const;
    ECPRESENTATION_EXPORT BentleyStatus FromJson(RapidJsonValueCR);
    };
typedef BoundQueryValuesList const& BoundQueryValuesListCR;

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
    ECPRESENTATION_EXPORT rapidjson::Document _ToJson(rapidjson::Document::AllocatorType*) const override;
public:
    BoundQueryECValue(ECValue value) : m_value(std::move(value)) {}
    ECValue const& GetValue() const {return m_value;}
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
    ECPRESENTATION_EXPORT rapidjson::Document _ToJson(rapidjson::Document::AllocatorType*) const override;
public:
    BoundQueryId(BeInt64Id id) : m_id(id) {}
    BoundQueryId(Utf8StringCR idStr) {BeInt64Id::FromString(m_id, idStr.c_str());}
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
    ECPRESENTATION_EXPORT rapidjson::Document _ToJson(rapidjson::Document::AllocatorType*) const override;
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
    ECPRESENTATION_EXPORT rapidjson::Document _ToJson(rapidjson::Document::AllocatorType*) const override;
public:
    ECPRESENTATION_EXPORT BoundECValueSet(bvector<ECValue>);
    ECPRESENTATION_EXPORT BoundECValueSet(BoundECValueSet const& other);
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
    ECPRESENTATION_EXPORT rapidjson::Document _ToJson(rapidjson::Document::AllocatorType*) const override;
public:
    ECPRESENTATION_EXPORT BoundRapidJsonValueSet(RapidJsonValueCR values, PrimitiveType type);
    ECPRESENTATION_EXPORT BoundRapidJsonValueSet(BoundRapidJsonValueSet const& other);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryClauseAndBindings
{
private:
    Utf8String m_clause;
    BoundQueryValuesList m_bindings;
public:
    QueryClauseAndBindings() {}
    QueryClauseAndBindings(Utf8String clause, BoundQueryValuesList bindings = BoundQueryValuesList())
        : m_clause(clause), m_bindings(bindings)
        {}
    QueryClauseAndBindings(Utf8CP clause, BoundQueryValuesList bindings = BoundQueryValuesList())
        : m_clause(clause), m_bindings(bindings)
        {}
    bool operator==(QueryClauseAndBindings const& other) const
        {
        return m_clause.Equals(other.m_clause)
            && m_bindings == other.m_bindings;
        }
    bool operator!=(QueryClauseAndBindings const& other) const {return !operator==(other);}
    void Reset()
        {
        m_bindings.clear();
        m_clause.clear();
        }
    void Append(QueryClauseAndBindings const& rhs, Utf8StringCR separator)
        {
        if (!m_clause.empty())
            m_clause.append(separator);
        m_clause.append(rhs.GetClause());
        ContainerHelpers::Push(m_bindings, rhs.GetBindings());
        }
    void Append(Utf8String clause, Utf8StringCR separator, BoundQueryValuesList bindings = BoundQueryValuesList())
        {
        if (!m_clause.empty())
            m_clause.append(separator);
        m_clause.append(clause);
        ContainerHelpers::MovePush(m_bindings, bindings);
        }
    void Bind(ECSqlStatement& stmt, uint32_t* bindingIndex = nullptr)
        {
        uint32_t defaultIndex = 1;
        uint32_t& index = bindingIndex ? *bindingIndex : defaultIndex;
        for (auto const& binding : m_bindings)
            binding->Bind(stmt, index++);
        }
    Utf8StringR GetClause() {return m_clause;}
    Utf8StringCR GetClause() const {return m_clause;}
    BoundQueryValuesList const& GetBindings() const {return m_bindings;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FilteredValuesHandler
    {
    virtual ~FilteredValuesHandler() {}
    virtual void _Accept(BeInt64Id) = 0;
    virtual void _Accept(ECValue) = 0;
    virtual BoundQueryValuesList _GetBoundValues() = 0;
    virtual Utf8String _GetWhereClause(Utf8CP valueSelector, bool) const = 0;
    ECPRESENTATION_EXPORT static std::unique_ptr<FilteredValuesHandler> Create(size_t, bool onlyIds);
    };
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ValuesFilteringHelper
{
private:
    std::unique_ptr<FilteredValuesHandler> m_handler;
private:
    template<typename TIterable> void Accept(TIterable const& iterable)
        {
        for (auto const& el : iterable)
            m_handler->_Accept(el);
        }
public:
    ValuesFilteringHelper(bvector<ECValue> const& ecValues)
        : m_handler(FilteredValuesHandler::Create(ecValues.size(), false))
        {
        Accept(ecValues);
        }
    template<typename TIterable> ValuesFilteringHelper(TIterable const& iterable)
        : m_handler(FilteredValuesHandler::Create(iterable.size(), true))
        {
        Accept(iterable);
        }
    Utf8String CreateWhereClause(Utf8CP valueSelector, bool inverse = false) const {return m_handler->_GetWhereClause(valueSelector, inverse);}
    BoundQueryValuesList CreateBoundValues() {return m_handler->_GetBoundValues();}
    QueryClauseAndBindings Create(Utf8CP valueSelector, bool inverse = false) {return QueryClauseAndBindings(CreateWhereClause(valueSelector, inverse), CreateBoundValues());}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QueryHelpers
    {
    static bool IsFunction(Utf8StringCR clause);
    static bool IsLiteral(Utf8StringCR clause);
    static bool IsWrapped(Utf8StringCR clause);
    static Utf8String Wrap(Utf8StringCR clause);
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IQueryInfoProvider
{
    enum SelectionSourceFlags
        {
        SELECTION_SOURCE_From = 1 << 0,
        SELECTION_SOURCE_Join = 1 << 1,
        SELECTION_SOURCE_All  = (SELECTION_SOURCE_From | SELECTION_SOURCE_Join)
        };

protected:
    virtual ~IQueryInfoProvider() {}
    virtual bvector<Utf8CP> _GetSelectAliases(int flags) const = 0;
    virtual bvector<Utf8CP> _GetRelationshipAliases(int flags) const {return bvector<Utf8CP>();};

public:
    bvector<Utf8CP> GetSelectAliases(int flags = SELECTION_SOURCE_All) const {return _GetSelectAliases(flags);}
    bvector<Utf8CP> GetRelationshipAliases(int flags = SELECTION_SOURCE_All) const {return _GetRelationshipAliases(flags);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
