/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "../../RulesEngineTypes.h"
#include "QueryContracts.h"
#include "QueryBuilding.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=============================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+==*/
enum PresentationQueryClauses
    {
    CLAUSE_Select    = 1 << 0,
    CLAUSE_From      = 1 << 1,
    CLAUSE_Where     = 1 << 2,
    CLAUSE_OrderBy   = 1 << 3,
    CLAUSE_Limit     = 1 << 4,
    CLAUSE_Union     = 1 << 5,
    CLAUSE_Except    = 1 << 6,
    CLAUSE_GroupBy   = 1 << 7,
    CLAUSE_Having    = 1 << 8,
    CLAUSE_JoinUsing = 1 << 9,
    CLAUSE_All       = CLAUSE_Select | CLAUSE_From | CLAUSE_Where | CLAUSE_OrderBy | CLAUSE_Limit | CLAUSE_Union | CLAUSE_Except | CLAUSE_GroupBy | CLAUSE_Having | CLAUSE_JoinUsing,
    };

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
    Utf8StringCR GetQueryString() const {return m_query;}
    Utf8StringR GetQueryString() {return m_query;}
    BoundQueryValuesList const& GetBindings() const {return m_bindings;}
    BoundQueryValuesList& GetBindings() {return m_bindings;}

    BentleyStatus BindValues(ECSqlStatement& stmt) const {return GetBindings().Bind(stmt);}

    ECPRESENTATION_EXPORT rapidjson::Document ToJson(rapidjson::Document::AllocatorType* = nullptr) const;
    ECPRESENTATION_EXPORT static std::unique_ptr<PresentationQuery> FromJson(RapidJsonValueCR);
};

struct NavigationQueryResultParameters;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryBuilder : RefCountedBase, IQueryInfoProvider, IContractProvider<PresentationQueryContract>, RapidJsonExtendedDataHolder<>
{
    typedef PresentationQueryContract PresentationQueryContract;

private:
    bool m_isOuterQuery;
    mutable std::unique_ptr<PresentationQuery> m_query;
    std::unique_ptr<NavigationQueryResultParameters> m_navigationResultParams;

protected:
    ECPRESENTATION_EXPORT PresentationQueryBuilder();
    ECPRESENTATION_EXPORT PresentationQueryBuilder(PresentationQueryBuilder const&);
    ECPRESENTATION_EXPORT ~PresentationQueryBuilder();
    virtual bool _IsEqual(PresentationQueryBuilder const& other) const {return m_isOuterQuery == other.m_isOuterQuery;}
    virtual ComplexQueryBuilder* _AsComplexQueryBuilder() {return nullptr;}
    virtual UnionQueryBuilder* _AsUnionQueryBuilder() {return nullptr;}
    virtual ExceptQueryBuilder* _AsExceptQueryBuilder() {return nullptr;}
    virtual StringQueryBuilder* _AsStringQueryBuilder() {return nullptr;}
    virtual std::unique_ptr<PresentationQuery> _CreateQuery() const = 0;
    virtual PresentationQueryContract const* _GetContract(size_t) const override {return nullptr;}
    virtual PresentationQueryContract const* _GetGroupingContract() const {return nullptr;}
    virtual void _OnIsOuterQueryValueChanged() {}
    void InvalidateQuery() {m_query = nullptr;}

public:
    ECPRESENTATION_EXPORT RefCountedPtr<PresentationQueryBuilder> Clone() const;

    ComplexQueryBuilder const* AsComplexQueryBuilder() const {return const_cast<PresentationQueryBuilder*>(this)->AsComplexQueryBuilder();}
    ComplexQueryBuilder* AsComplexQueryBuilder() {return _AsComplexQueryBuilder();}
    UnionQueryBuilder const* AsUnionQueryBuilder() const {return const_cast<PresentationQueryBuilder*>(this)->AsUnionQueryBuilder();}
    UnionQueryBuilder* AsUnionQueryBuilder() {return _AsUnionQueryBuilder();}
    ExceptQueryBuilder const* AsExceptQueryBuilder() const {return const_cast<PresentationQueryBuilder*>(this)->AsExceptQueryBuilder();}
    ExceptQueryBuilder* AsExceptQueryBuilder() {return _AsExceptQueryBuilder();}
    StringQueryBuilder const* AsStringQueryBuilder() const {return const_cast<PresentationQueryBuilder*>(this)->AsStringQueryBuilder();}
    StringQueryBuilder* AsStringQueryBuilder() {return _AsStringQueryBuilder();}

    bool IsEqual(PresentationQueryBuilder const& other) const {return _IsEqual(other);}

    std::unique_ptr<PresentationQuery> CreateQuery() const {return _CreateQuery();}
    PresentationQuery const* GetQuery() const
        {
        if (!m_query)
            m_query = CreateQuery();
        return m_query.get();
        }

    PresentationQueryContract const* GetGroupingContract() const {return _GetGroupingContract();}

    bool IsOuterQuery() const {return m_isOuterQuery;}
    void SetIsOuterQuery(bool value) {m_isOuterQuery = value; _OnIsOuterQueryValueChanged();}

    ECPRESENTATION_EXPORT NavigationQueryResultParameters const& GetNavigationResultParameters() const;
    ECPRESENTATION_EXPORT NavigationQueryResultParameters& GetNavigationResultParameters();
};

/*=============================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+==*/
enum class JoinType
    {
    Inner,
    Outer,
    };

struct JoinClassClause;
struct JoinClassWithRelationshipClause;
struct JoinQueryClause;
/*=============================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+==*/
struct JoinClause
{
private:
    QueryClauseAndBindings m_joinFilter;
    JoinType m_joinType;
protected:
    JoinClause(QueryClauseAndBindings joinFilter, JoinType joinType)
        : m_joinFilter(joinFilter), m_joinType(joinType)
        {}
public:
    virtual ~JoinClause() {}
    virtual JoinClassClause const* _AsClassJoin() const { return nullptr; }
    virtual JoinClassWithRelationshipClause const* _AsClassWithRelationshipJoin() const { return nullptr; }
    virtual JoinQueryClause const* _AsQueryJoin() const { return nullptr; }
    QueryClauseAndBindings const& GetJoinFilter() const {return m_joinFilter;}
    JoinType GetJoinType() const {return m_joinType;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ComplexQueryBuilder : PresentationQueryBuilder
{
    friend struct PresentationQueryBuilder;

private:
    RefCountedPtr<PresentationQueryContract const> m_selectContract;
    Utf8String m_selectPrefix;
    bool m_isSelectAll;
    mutable QueryClauseAndBindings m_selectClause;
    bvector<std::shared_ptr<SelectClassWithExcludes<ECClass>>> m_from;
    mutable QueryClauseAndBindings m_fromClause;
    RefCountedPtr<PresentationQueryBuilder> m_nestedQuery;
    Utf8String m_nestedQueryAlias;
    QueryClauseAndBindings m_whereClause;
    bvector<bvector<std::shared_ptr<JoinClause>>> m_joins;
    mutable QueryClauseAndBindings m_joinClause;
    RefCountedPtr<PresentationQueryContract const> m_groupingContract;
    QueryClauseAndBindings m_havingClause;
    Utf8String m_orderByClause;
    std::shared_ptr<BoundQueryECValue const> m_limit;
    std::shared_ptr<BoundQueryECValue const> m_offset;

private:
    void InitSelectClause() const;
    void InitFromClause() const;
    void InitJoinClause() const;
    Utf8String CreateGroupByClause() const;
    bvector<Utf8CP> GetAliases(int, bool) const;

protected:
    ComplexQueryBuilder() : m_isSelectAll(false) {}
    ComplexQueryBuilder(ComplexQueryBuilder const& other)
        : PresentationQueryBuilder(other), m_havingClause(other.m_havingClause), m_orderByClause(other.m_orderByClause), m_from(other.m_from),
        m_nestedQueryAlias(other.m_nestedQueryAlias), m_whereClause(other.m_whereClause), m_joins(other.m_joins),
        m_selectContract(other.m_selectContract), m_selectPrefix(other.m_selectPrefix), m_isSelectAll(other.m_isSelectAll),
        m_groupingContract(other.m_groupingContract), m_limit(other.m_limit), m_offset(other.m_offset)
        {
        if (other.m_nestedQuery.IsValid())
            m_nestedQuery = other.m_nestedQuery->Clone();
        }
    ComplexQueryBuilder* _AsComplexQueryBuilder() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetRelationshipAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBuilder const& other) const override;
    ECPRESENTATION_EXPORT std::unique_ptr<PresentationQuery> _CreateQuery() const override;
    ECPRESENTATION_EXPORT PresentationQueryContract const* _GetContract(size_t) const override;
    ECPRESENTATION_EXPORT PresentationQueryContract const* _GetGroupingContract() const override;

public:
    static RefCountedPtr<ComplexQueryBuilder> Create() {return new ComplexQueryBuilder();}
    RefCountedPtr<ComplexQueryBuilder> Clone() const {return new ComplexQueryBuilder(*this);}
    ECPRESENTATION_EXPORT bool HasClause(PresentationQueryClauses clause) const;
    ECPRESENTATION_EXPORT Utf8String GetClause(PresentationQueryClauses clause) const;
    PresentationQueryBuilder const* GetNestedQuery() const {return m_nestedQuery.get();}
    PresentationQueryBuilder* GetNestedQuery() {return m_nestedQuery.get();}
    Utf8CP GetSelectPrefix() const {return m_selectPrefix.c_str();}

    ECPRESENTATION_EXPORT ComplexQueryBuilder& SelectAll();
    ECPRESENTATION_EXPORT ComplexQueryBuilder& SelectContract(PresentationQueryContract const& contract, Utf8CP prefix = nullptr);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& From(ECClassCR fromClass, bool polymorphic, Utf8CP alias);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& From(SelectClassWithExcludes<ECClass> const& fromClass);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& From(PresentationQueryBuilder& nestedQuery, Utf8CP alias = nullptr);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& From(PresentationQuery const& nestedQuery, Utf8CP alias = nullptr);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Where(Utf8CP whereClause, BoundQueryValuesListCR);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Where(QueryClauseAndBindings whereClause);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Join(SelectClass<ECClass> const& join, QueryClauseAndBindings joinClause, bool isOuter);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Join(PresentationQueryBuilder const& nestedQuery, Utf8CP alias, QueryClauseAndBindings joinClause, bool isOuter);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Join(PresentationQuery const& nestedQuery, Utf8CP alias, QueryClauseAndBindings joinClause, bool isOuter);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Join(RelatedClass const& relatedClass);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Join(RelatedClassPath const& path, bool shouldIncludeTargetClass = true);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& GroupByContract(PresentationQueryContract const& contract);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Having(Utf8CP havingClause, BoundQueryValuesListCR);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Having(QueryClauseAndBindings);
    ECPRESENTATION_EXPORT ComplexQueryBuilder& Limit(uint64_t limit, uint64_t offset = 0);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE UnionQueryBuilder : PresentationQueryBuilder
{
    friend struct PresentationQueryBuilder;

private:
    bvector<RefCountedPtr<PresentationQueryBuilder>> m_queries;
    Utf8String m_orderByClause;
    std::shared_ptr<BoundQueryECValue const> m_limit;
    std::shared_ptr<BoundQueryECValue const> m_offset;

private:
    ECPRESENTATION_EXPORT void Init(PresentationQueryBuilder*);

protected:
    UnionQueryBuilder(bvector<RefCountedPtr<PresentationQueryBuilder>> queries) : m_queries(queries) {Init(nullptr);}
    UnionQueryBuilder(UnionQueryBuilder const& other)
        : PresentationQueryBuilder(other), m_orderByClause(other.m_orderByClause), m_limit(other.m_limit), m_offset(other.m_offset)
        {
        for (RefCountedPtr<PresentationQueryBuilder> const& query : other.m_queries)
            m_queries.push_back(query->Clone());
        }
    UnionQueryBuilder* _AsUnionQueryBuilder() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBuilder const& other) const override;
    ECPRESENTATION_EXPORT std::unique_ptr<PresentationQuery> _CreateQuery() const override;
    ECPRESENTATION_EXPORT PresentationQueryContract const* _GetContract(size_t) const override;
    ECPRESENTATION_EXPORT PresentationQueryContract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT void _OnIsOuterQueryValueChanged() override;

public:
    static RefCountedPtr<UnionQueryBuilder> Create(bvector<RefCountedPtr<PresentationQueryBuilder>> queries) {return new UnionQueryBuilder(queries);}
    Utf8StringCR GetOrderByClause() const {return m_orderByClause;}
    ECPRESENTATION_EXPORT UnionQueryBuilder& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT UnionQueryBuilder& Limit(uint64_t limit, uint64_t offset = 0);
    bvector<RefCountedPtr<PresentationQueryBuilder>> const& GetQueries() const {return m_queries;}
    void SetQueries(bvector<RefCountedPtr<PresentationQueryBuilder>> queries) {m_queries = queries;}
    void AddQuery(PresentationQueryBuilder& query) {m_queries.push_back(&query); Init(&query);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ExceptQueryBuilder : PresentationQueryBuilder
{
    friend struct PresentationQueryBuilder;

private:
    RefCountedPtr<PresentationQueryBuilder> m_base;
    RefCountedPtr<PresentationQueryBuilder> m_except;
    Utf8String m_orderByClause;
    std::shared_ptr<BoundQueryECValue const> m_limit;
    std::shared_ptr<BoundQueryECValue const> m_offset;

private:
    ECPRESENTATION_EXPORT void Init();

protected:
    ExceptQueryBuilder(PresentationQueryBuilder& base, PresentationQueryBuilder& except) : m_base(&base), m_except(&except) {Init();}
    ExceptQueryBuilder(ExceptQueryBuilder const& other)
        : PresentationQueryBuilder(other), m_orderByClause(other.m_orderByClause), m_limit(other.m_limit), m_offset(other.m_offset)
        {
        m_base = other.m_base->Clone();
        m_except = other.m_except->Clone();
        }
    ExceptQueryBuilder* _AsExceptQueryBuilder() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBuilder const& other) const override;
    ECPRESENTATION_EXPORT std::unique_ptr<PresentationQuery> _CreateQuery() const override;
    ECPRESENTATION_EXPORT PresentationQueryContract const* _GetContract(size_t) const override;
    ECPRESENTATION_EXPORT PresentationQueryContract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT void _OnIsOuterQueryValueChanged() override;

public:
    static RefCountedPtr<ExceptQueryBuilder> Create(PresentationQueryBuilder& base, PresentationQueryBuilder& except) {return new ExceptQueryBuilder(base, except);}
    Utf8StringCR GetOrderByClause() const {return m_orderByClause;}
    ECPRESENTATION_EXPORT ExceptQueryBuilder& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ExceptQueryBuilder& Limit(uint64_t limit, uint64_t offset = 0);
    RefCountedPtr<PresentationQueryBuilder> GetBase() const {return m_base;}
    RefCountedPtr<PresentationQueryBuilder> GetExcept() const {return m_except;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE StringQueryBuilder : PresentationQueryBuilder
{
    friend struct PresentationQueryBuilder;

private:
    std::unique_ptr<PresentationQuery> m_query;

protected:
    StringQueryBuilder(Utf8StringCR query, BoundQueryValuesList bindings)
        : m_query(std::make_unique<PresentationQuery>(query, bindings))
        {}
    StringQueryBuilder* _AsStringQueryBuilder() override {return this;}
    bvector<Utf8CP> _GetSelectAliases(int) const override {return bvector<Utf8CP>();}
    ECPRESENTATION_EXPORT std::unique_ptr<PresentationQuery> _CreateQuery() const override;
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBuilder const& other) const override;

public:
    static RefCountedPtr<StringQueryBuilder> Create(Utf8StringCR query, BoundQueryValuesList bindings = BoundQueryValuesList())
        {
        return new StringQueryBuilder(query, bindings);
        }
    static RefCountedPtr<StringQueryBuilder> Create(PresentationQueryCR query)
        {
        return new StringQueryBuilder(query.GetQueryString(), query.GetBindings());
        }
    static RefCountedPtr<StringQueryBuilder> Create(PresentationQueryBuilder const& source)
        {
        auto const& sourceQuery = source.GetQuery();
        if (!sourceQuery)
            return new StringQueryBuilder("", {});
        return new StringQueryBuilder(sourceQuery->GetQueryString(), sourceQuery->GetBindings());
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct QuerySet : IContractProvider<PresentationQueryContract>
{
private:
    bvector<RefCountedPtr<PresentationQueryBuilder>> m_queries;
protected:
    PresentationQueryContract const* _GetContract(size_t contractId) const override
        {
        for (auto const& query : m_queries)
            {
            auto contract = query->GetContract(contractId);
            if (nullptr != contract)
                return contract;
            }
        return nullptr;
        }
public:
    QuerySet() {}
    QuerySet(bvector<RefCountedPtr<PresentationQueryBuilder>> queries) : m_queries(queries) {}
    bvector<RefCountedPtr<PresentationQueryBuilder>>& GetQueries() {return m_queries;}
    bvector<RefCountedPtr<PresentationQueryBuilder>> const& GetQueries() const {return m_queries;}
    bool Equals(QuerySet const& other) const
        {
        if (GetQueries().size() != other.GetQueries().size())
            return false;
        for (size_t i = 0; i < GetQueries().size(); ++i)
            {
            if (!GetQueries().at(i)->IsEqual(*other.GetQueries().at(i)))
                return false;
            }
        return true;
        }
    Utf8String ToString() const
        {
        Json::Value json;
        for (auto const& query : m_queries)
            json.append(query->GetQuery()->GetQueryString());
        return json.toStyledString();
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
