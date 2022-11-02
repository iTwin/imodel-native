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

// defines P, CP, R, CR, Ptr, CPtr suffix typedefs for the given typename;
#define DEFINE_QUERY_SUFFIX_TYPEDEFS(_name_) \
    typedef _name_* _name_##P, &_name_##R; \
    typedef _name_ const* _name_##CP; \
    typedef _name_ const& _name_##CR; \
    typedef RefCountedPtr<_name_> _name_##Ptr; \
    typedef RefCountedPtr<_name_ const> _name_##CPtr;

// defines typedefs for PresentationQuery subclasses;
#define DEFINE_QUERY_SUBCLASS_TYPEDEFS(_prefix_, _name_) \
    typedef _prefix_##PresentationQuery<_name_> _prefix_##_name_; \
    DEFINE_QUERY_SUFFIX_TYPEDEFS(_prefix_##_name_);

// defines typedefs for PresentationQuery and all its subclasses;
#define DEFINE_QUERY_TYPEDEFS(_name_)               \
    DEFINE_QUERY_SUFFIX_TYPEDEFS(_name_)            \
    DEFINE_QUERY_SUBCLASS_TYPEDEFS(Complex, _name_) \
    DEFINE_QUERY_SUBCLASS_TYPEDEFS(Union, _name_)   \
    DEFINE_QUERY_SUBCLASS_TYPEDEFS(Except, _name_)  \
    DEFINE_QUERY_SUBCLASS_TYPEDEFS(String, _name_)

// excplitly instantiates PresentationQuery subclass template for the given template argument;
#define INSTANTIATE_QUERY_SUBCLASS(_name_, _ext_, _export_)          \
    _ext_ template struct _export_ ComplexPresentationQuery<_name_>;\
    _ext_ template struct _export_ UnionPresentationQuery<_name_>;  \
    _ext_ template struct _export_ ExceptPresentationQuery<_name_>; \
    _ext_ template struct _export_ StringPresentationQuery<_name_>;

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

template<typename TBase> struct ComplexPresentationQuery;
template<typename TBase> struct UnionPresentationQuery;
template<typename TBase> struct ExceptPresentationQuery;
template<typename TBase> struct StringPresentationQuery;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryBase : RefCountedBase
{
private:
    mutable Utf8String m_queryString;
protected:
    virtual BoundQueryValuesList _GetBoundValues() const {return BoundQueryValuesList();}
    virtual Utf8String _ToString() const = 0;
    virtual bool _IsEqual(PresentationQueryBase const& other) const = 0;
public:
    Utf8StringCR ToString() const
        {
        if (Utf8String::IsNullOrEmpty(m_queryString.c_str()))
            m_queryString = _ToString();
        return m_queryString;
        }
    void InvalidateQueryString() const {m_queryString.clear();}
    bool IsEqual(PresentationQueryBase const& other) const {return _IsEqual(other);}
    BoundQueryValuesList GetBoundValues() const {return _GetBoundValues();}
    ECPRESENTATION_EXPORT BentleyStatus BindValues(ECSqlStatement&) const;
    ECPRESENTATION_EXPORT rapidjson::Document ToJson(rapidjson::Document::AllocatorType* = nullptr) const;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TBase, typename TContract, typename TResultParameters>
struct EXPORT_VTABLE_ATTRIBUTE PresentationQuery : PresentationQueryBase, IQueryInfoProvider, RapidJsonExtendedDataHolder<>, IContractProvider<TContract>
{
    typedef ComplexPresentationQuery<TBase> ComplexQuery;
    typedef UnionPresentationQuery<TBase>   UnionQuery;
    typedef ExceptPresentationQuery<TBase>  ExceptQuery;
    typedef StringPresentationQuery<TBase>  StringQuery;
    typedef TContract                       Contract;
    typedef TResultParameters               ResultParameters;

private:
    ResultParameters m_resultParameters;
    bool m_isOuterQuery;

protected:
    PresentationQuery() : m_isOuterQuery(true) {}
    PresentationQuery(PresentationQuery const& other) : m_isOuterQuery(other.m_isOuterQuery), m_resultParameters(other.m_resultParameters) {}
    virtual bool _IsEqual(PresentationQueryBase const& otherBase) const override
        {
        auto other = dynamic_cast<PresentationQuery const*>(&otherBase);
        return other
            && m_isOuterQuery == other->m_isOuterQuery
            && m_resultParameters == other->m_resultParameters;
        }
    virtual ComplexQuery* _AsComplexQuery() {return nullptr;}
    virtual UnionQuery* _AsUnionQuery() {return nullptr;}
    virtual ExceptQuery* _AsExceptQuery() {return nullptr;}
    virtual StringQuery* _AsStringQuery() {return nullptr;}
    virtual Contract const* _GetContract(size_t) const override {return nullptr;}
    virtual Contract const* _GetGroupingContract() const {return nullptr;}
    virtual void _OnIsOuterQueryValueChanged() {}

public:
    RefCountedPtr<TBase> Clone() const
        {
        RefCountedPtr<TBase> clone;
        if (nullptr != AsComplexQuery())
            clone = new ComplexPresentationQuery<TBase>(*AsComplexQuery());
        else if (nullptr != AsUnionQuery())
            clone = new UnionPresentationQuery<TBase>(*AsUnionQuery());
        else if (nullptr != AsExceptQuery())
            clone = new ExceptPresentationQuery<TBase>(*AsExceptQuery());
        else if (nullptr != AsStringQuery())
            clone = new StringPresentationQuery<TBase>(*AsStringQuery());
        else
            DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, Utf8PrintfString("Unhandled NavigationQuery type"));

        clone->GetResultParametersR().MergeWith(GetResultParameters());
        clone->GetExtendedDataR().CopyFrom(GetExtendedData(), clone->GetExtendedDataAllocator());
        return clone;
        }

    ComplexQuery const* AsComplexQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsComplexQuery();}
    ComplexQuery* AsComplexQuery() {return _AsComplexQuery();}
    UnionQuery const* AsUnionQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsUnionQuery();}
    UnionQuery* AsUnionQuery() {return _AsUnionQuery();}
    ExceptQuery const* AsExceptQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsExceptQuery();}
    ExceptQuery* AsExceptQuery() {return _AsExceptQuery();}
    StringQuery const* AsStringQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsStringQuery();}
    StringQuery* AsStringQuery() {return _AsStringQuery();}

    Contract const* GetGroupingContract() const {return _GetGroupingContract();}
    ResultParameters const& GetResultParameters() const {return m_resultParameters;}
    ResultParameters& GetResultParametersR() {return m_resultParameters;}

    bool IsOuterQuery() const {return m_isOuterQuery;}
    void SetIsOuterQuery(bool value) {m_isOuterQuery = value; _OnIsOuterQueryValueChanged();}
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
template<typename TBase>
struct EXPORT_VTABLE_ATTRIBUTE ComplexPresentationQuery : TBase
{
    typedef ComplexPresentationQuery<TBase>     ThisType;
    typedef typename TBase::Contract            Contract;
    typedef typename TBase::ResultParameters    ResultParameters;
    friend struct PresentationQuery<TBase, Contract, ResultParameters>;

private:
    RefCountedPtr<Contract const> m_selectContract;
    Utf8String m_selectPrefix;
    bool m_isSelectAll;
    mutable QueryClauseAndBindings m_selectClause;
    bvector<std::shared_ptr<SelectClassWithExcludes<ECClass>>> m_from;
    mutable QueryClauseAndBindings m_fromClause;
    RefCountedPtr<TBase> m_nestedQuery;
    Utf8String m_nestedQueryAlias;
    QueryClauseAndBindings m_whereClause;
    bvector<bvector<std::shared_ptr<JoinClause>>> m_joins;
    mutable QueryClauseAndBindings m_joinClause;
    RefCountedPtr<Contract const> m_groupingContract;
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
    ComplexPresentationQuery() : m_isSelectAll(false) {}
    ComplexPresentationQuery(ComplexPresentationQuery<TBase> const& other)
        : TBase(other), m_havingClause(other.m_havingClause), m_orderByClause(other.m_orderByClause), m_from(other.m_from),
        m_nestedQueryAlias(other.m_nestedQueryAlias), m_whereClause(other.m_whereClause), m_joins(other.m_joins),
        m_selectContract(other.m_selectContract), m_selectPrefix(other.m_selectPrefix), m_isSelectAll(other.m_isSelectAll),
        m_groupingContract(other.m_groupingContract), m_limit(other.m_limit), m_offset(other.m_offset)
        {
        if (other.m_nestedQuery.IsValid())
            m_nestedQuery = other.m_nestedQuery->Clone();
        }
    ThisType* _AsComplexQuery() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetRelationshipAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBase const& other) const override;
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    ECPRESENTATION_EXPORT Contract const* _GetContract(size_t) const override;
    ECPRESENTATION_EXPORT Contract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT BoundQueryValuesList _GetBoundValues() const override;

public:
    static RefCountedPtr<ComplexPresentationQuery<TBase>> Create() {return new ComplexPresentationQuery();}
    RefCountedPtr<ComplexPresentationQuery<TBase>> Clone() const {return TBase::Clone()->AsComplexQuery();}
    ECPRESENTATION_EXPORT bool HasClause(PresentationQueryClauses clause) const;
    ECPRESENTATION_EXPORT Utf8String GetClause(PresentationQueryClauses clause) const;
    TBase const* GetNestedQuery() const {return m_nestedQuery.get();}
    TBase* GetNestedQuery() {return m_nestedQuery.get();}
    Utf8CP GetSelectPrefix() const {return m_selectPrefix.c_str();}

    ECPRESENTATION_EXPORT ThisType& SelectAll();
    ECPRESENTATION_EXPORT ThisType& SelectContract(Contract const& contract, Utf8CP prefix = nullptr);
    ECPRESENTATION_EXPORT ThisType& From(ECClassCR fromClass, bool polymorphic, Utf8CP alias);
    ECPRESENTATION_EXPORT ThisType& From(SelectClassWithExcludes<ECClass> const& fromClass);
    ECPRESENTATION_EXPORT ThisType& From(TBase& nestedQuery, Utf8CP alias = nullptr);
    ECPRESENTATION_EXPORT ThisType& Where(Utf8CP whereClause, BoundQueryValuesListCR);
    ECPRESENTATION_EXPORT ThisType& Where(QueryClauseAndBindings whereClause);
    ECPRESENTATION_EXPORT ThisType& Join(SelectClass<ECClass> const& join, QueryClauseAndBindings joinClause, bool isOuter);
    ECPRESENTATION_EXPORT ThisType& Join(TBase& nestedQuery, Utf8CP alias, QueryClauseAndBindings joinClause, bool isOuter);
    ECPRESENTATION_EXPORT ThisType& Join(RelatedClass const& relatedClass);
    ECPRESENTATION_EXPORT ThisType& Join(RelatedClassPath const& path, bool shouldIncludeTargetClass = true);
    ECPRESENTATION_EXPORT ThisType& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ThisType& GroupByContract(Contract const& contract);
    ECPRESENTATION_EXPORT ThisType& Having(Utf8CP havingClause, BoundQueryValuesListCR);
    ECPRESENTATION_EXPORT ThisType& Having(QueryClauseAndBindings);
    ECPRESENTATION_EXPORT ThisType& Limit(uint64_t limit, uint64_t offset = 0);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TBase>
struct EXPORT_VTABLE_ATTRIBUTE UnionPresentationQuery : TBase
{
    typedef UnionPresentationQuery<TBase>       ThisType;
    typedef typename TBase::Contract            Contract;
    typedef typename TBase::ResultParameters    ResultParameters;

    friend struct PresentationQuery<TBase, Contract, ResultParameters>;

private:
    bvector<RefCountedPtr<TBase>> m_queries;
    Utf8String m_orderByClause;
    std::shared_ptr<BoundQueryECValue const> m_limit;
    std::shared_ptr<BoundQueryECValue const> m_offset;

private:
    ECPRESENTATION_EXPORT void Init(TBase*);

protected:
    UnionPresentationQuery(bvector<RefCountedPtr<TBase>> queries) : m_queries(queries) {Init(nullptr);}
    UnionPresentationQuery(UnionPresentationQuery<TBase> const& other)
        : TBase(other), m_orderByClause(other.m_orderByClause), m_limit(other.m_limit), m_offset(other.m_offset)
        {
        for (RefCountedPtr<TBase> const& query : other.m_queries)
            m_queries.push_back(query->Clone());
        }
    ThisType* _AsUnionQuery() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBase const& other) const override;
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    ECPRESENTATION_EXPORT Contract const* _GetContract(size_t) const override;
    ECPRESENTATION_EXPORT Contract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT BoundQueryValuesList _GetBoundValues() const override;
    ECPRESENTATION_EXPORT void _OnIsOuterQueryValueChanged() override;

public:
    static RefCountedPtr<UnionPresentationQuery<TBase>> Create(bvector<RefCountedPtr<TBase>> queries) {return new UnionPresentationQuery(queries);}
    Utf8StringCR GetOrderByClause() const {return m_orderByClause;}
    ECPRESENTATION_EXPORT ThisType& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ThisType& Limit(uint64_t limit, uint64_t offset = 0);
    bvector<RefCountedPtr<TBase>> const& GetQueries() const {return m_queries;}
    void SetQueries(bvector<RefCountedPtr<TBase>> queries) {m_queries = queries;}
    void AddQuery(TBase& query) {m_queries.push_back(&query); Init(&query);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TBase>
struct EXPORT_VTABLE_ATTRIBUTE ExceptPresentationQuery : TBase
{
    typedef ExceptPresentationQuery<TBase>      ThisType;
    typedef typename TBase::Contract            Contract;
    typedef typename TBase::ResultParameters    ResultParameters;

    friend struct PresentationQuery<TBase, Contract, ResultParameters>;

private:
    RefCountedPtr<TBase> m_base;
    RefCountedPtr<TBase> m_except;
    Utf8String m_orderByClause;
    std::shared_ptr<BoundQueryECValue const> m_limit;
    std::shared_ptr<BoundQueryECValue const> m_offset;

private:
    ECPRESENTATION_EXPORT void Init();

protected:
    ExceptPresentationQuery(TBase& base, TBase& except) : m_base(&base), m_except(&except) {Init();}
    ExceptPresentationQuery(ExceptPresentationQuery<TBase> const& other)
        : TBase(other), m_orderByClause(other.m_orderByClause), m_limit(other.m_limit), m_offset(other.m_offset)
        {
        m_base = other.m_base->Clone();
        m_except = other.m_except->Clone();
        }
    ThisType* _AsExceptQuery() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBase const& other) const override;
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    ECPRESENTATION_EXPORT Contract const* _GetContract(size_t) const override;
    ECPRESENTATION_EXPORT Contract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT BoundQueryValuesList _GetBoundValues() const override;
    ECPRESENTATION_EXPORT void _OnIsOuterQueryValueChanged() override;

public:
    static RefCountedPtr<ExceptPresentationQuery<TBase>> Create(TBase& base, TBase& except) {return new ExceptPresentationQuery(base, except);}
    Utf8StringCR GetOrderByClause() const {return m_orderByClause;}
    ECPRESENTATION_EXPORT ThisType& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ThisType& Limit(uint64_t limit, uint64_t offset = 0);
    RefCountedPtr<TBase> GetBase() const {return m_base;}
    RefCountedPtr<TBase> GetExcept() const {return m_except;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TBase>
struct EXPORT_VTABLE_ATTRIBUTE StringPresentationQuery : TBase
{
    typedef StringPresentationQuery<TBase>      ThisType;
    typedef typename TBase::Contract            Contract;
    typedef typename TBase::ResultParameters    ResultParameters;

    friend struct PresentationQuery<TBase, Contract, ResultParameters>;

private:
    Utf8String m_query;
    BoundQueryValuesList m_bindings;

protected:
    StringPresentationQuery(Utf8StringCR query, BoundQueryValuesList bindings) : m_query(query), m_bindings(bindings) {}
    ThisType* _AsStringQuery() override {return this;}
    bvector<Utf8CP> _GetSelectAliases(int) const override {return bvector<Utf8CP>();}
    ECPRESENTATION_EXPORT bool _IsEqual(PresentationQueryBase const& other) const override;
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    BoundQueryValuesList _GetBoundValues() const override {return m_bindings;}

public:
    static RefCountedPtr<StringPresentationQuery<TBase>> Create(Utf8StringCR query, BoundQueryValuesList bindings = BoundQueryValuesList())
        {
        return new StringPresentationQuery(query, bindings);
        }
    static RefCountedPtr<StringPresentationQuery<TBase>> Create(PresentationQueryBase const& source)
        {
        return new StringPresentationQuery(source.ToString(), source.GetBoundValues());
        }
    ECPRESENTATION_EXPORT static RefCountedPtr<StringPresentationQuery<TBase>> FromJson(RapidJsonValueCR);
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TQuery>
struct QuerySet : IContractProvider<typename TQuery::Contract>
{
private:
    bvector<RefCountedPtr<TQuery>> m_queries;
protected:
    typename TQuery::Contract const* _GetContract(size_t contractId) const override
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
    QuerySet(bvector<RefCountedPtr<TQuery>> queries) : m_queries(queries) {}
    bvector<RefCountedPtr<TQuery>>& GetQueries() {return m_queries;}
    bvector<RefCountedPtr<TQuery>> const& GetQueries() const {return m_queries;}
    template<typename TOtherQuery> bool Equals(QuerySet<TOtherQuery> const& other) const
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
            json.append(query->ToString());
        return json.toStyledString();
        }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct GenericQueryResultParameters
{
public:
    GenericQueryResultParameters() {}
    void MergeWith(GenericQueryResultParameters const&) {}
    void OnContractSelected(PresentationQueryContractCR) {}
    bool operator==(GenericQueryResultParameters const& other) const {return true;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE GenericQuery : PresentationQuery<GenericQuery, PresentationQueryContract, GenericQueryResultParameters> {};
DEFINE_QUERY_TYPEDEFS(GenericQuery)
INSTANTIATE_QUERY_SUBCLASS(GenericQuery, extern, EXPORT_VTABLE_ATTRIBUTE)
typedef QuerySet<GenericQuery> GenericQuerySet;

END_BENTLEY_ECPRESENTATION_NAMESPACE
