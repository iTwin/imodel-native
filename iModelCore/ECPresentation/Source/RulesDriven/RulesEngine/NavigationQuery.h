/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentation.h>
#include "RulesEngineTypes.h"
#include "QueryContracts.h"
#include "ExtendedData.h"

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
* @bsiclass                                     Grigas.Petraitis            06/2015
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
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryValue
{
protected:
    virtual bool _Equals(BoundQueryValue const&) const = 0;
    virtual BoundQueryValue* _Clone() const = 0;
    virtual BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const = 0;
public:
    virtual ~BoundQueryValue() {}
    bool Equals(BoundQueryValue const& other) const {return _Equals(other);}
    BoundQueryValue* Clone() const {return _Clone();}
    BeSQLite::EC::ECSqlStatus Bind(BeSQLite::EC::ECSqlStatement& stmt, uint32_t index) const {return _Bind(stmt, index);}
};
typedef bvector<BoundQueryValue const*> BoundQueryValuesList;
typedef BoundQueryValuesList const&     BoundQueryValuesListCR;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryECValue : BoundQueryValue
{
private:
    ECValue m_value;
protected:
    ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
    BoundQueryValue* _Clone() const override {return new BoundQueryECValue(m_value);}
    ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
public:
    BoundQueryECValue(ECValue value) : m_value(value) {}
    ECValue const& GetValue() const {return m_value;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryId : BoundQueryValue
{
private:
    BeInt64Id m_id;
protected:
    ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
    BoundQueryValue* _Clone() const override {return new BoundQueryId(m_id);}
    ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
public:
    BoundQueryId(BeInt64Id id) : m_id(id) {}
    BoundQueryId(Utf8StringCR idStr) {BeInt64Id::FromString(m_id, idStr.c_str());}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2017
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundQueryIdSet : BoundQueryValue
{
private:
    BeSQLite::IdSet<BeInt64Id> m_set;
protected:
    ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
    BoundQueryValue* _Clone() const override {return new BoundQueryIdSet(BeSQLite::IdSet<BeInt64Id>(m_set));}
    ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
public:
    BoundQueryIdSet(BeSQLite::IdSet<BeInt64Id>&& set) : m_set(std::move(set)) {}
    BoundQueryIdSet(bvector<BeSQLite::EC::ECInstanceId> const& vec)
        {
        for (BeSQLite::EC::ECInstanceId const& id : vec)
            m_set.insert(id);
        }
    BoundQueryIdSet(bvector<BeSQLite::EC::ECInstanceKey> const& vec)
        {
        for (BeSQLite::EC::ECInstanceKey const& key : vec)
            m_set.insert(key.GetInstanceId());
        }
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BoundRapidJsonValueSet : BoundQueryValue
{
private:
    BeSQLite::VirtualSet* m_set;
protected:
    ECPRESENTATION_EXPORT bool _Equals(BoundQueryValue const&) const override;
    BoundQueryValue* _Clone() const override { return new BoundRapidJsonValueSet(*this); }
    ECPRESENTATION_EXPORT BeSQLite::EC::ECSqlStatus _Bind(BeSQLite::EC::ECSqlStatement&, uint32_t index) const override;
public:
    BoundRapidJsonValueSet(RapidJsonValueCR values, PrimitiveType type);
    BoundRapidJsonValueSet(BoundRapidJsonValueSet const& other);
    ~BoundRapidJsonValueSet();
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
struct FilteredIdsHandler
    {
    virtual ~FilteredIdsHandler() {}
    virtual Utf8String _GetWhereClause(Utf8CP idSelector, size_t) const = 0;
    virtual void _Accept(BeInt64Id) = 0;
    virtual BoundQueryValuesList _GetBoundValues() = 0;
    ECPRESENTATION_EXPORT static FilteredIdsHandler* Create(size_t);
    };
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2018
+===============+===============+===============+===============+===============+======*/
template<typename TIterable>
struct IdsFilteringHelper
{
private:
    TIterable const* m_set;
    bool m_ownsSet;
    FilteredIdsHandler* m_handler;

public:
    IdsFilteringHelper(TIterable const& set) : m_set(&set), m_ownsSet(false), m_handler(FilteredIdsHandler::Create(m_set->size())) {}
    IdsFilteringHelper(TIterable&& set) : m_set(new TIterable(std::move(set))), m_ownsSet(true), m_handler(FilteredIdsHandler::Create(m_set->size())) {}
    ~IdsFilteringHelper()
        {
        DELETE_AND_CLEAR(m_handler);
        if (m_ownsSet)
            DELETE_AND_CLEAR(m_set);
        }
    Utf8String CreateWhereClause(Utf8CP idSelector) const {return m_handler->_GetWhereClause(idSelector, m_set->size());}
    BoundQueryValuesList CreateBoundValues()
        {
        for (auto el : *m_set)
            m_handler->_Accept(el);
        return m_handler->_GetBoundValues();
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2018
+===============+===============+===============+===============+===============+======*/
struct QueryHelpers
    {
    static bool IsFunction(Utf8StringCR clause);
    static bool IsLiteral(Utf8StringCR clause);
    static bool IsWrapped(Utf8StringCR clause);
    static Utf8String Wrap(Utf8StringCR clause);
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PresentationQueryBase : RefCountedBase
{
private:
    mutable Utf8String m_queryString;
protected:
    virtual BoundQueryValuesList _GetBoundValues() const {return BoundQueryValuesList();}
    virtual Utf8String _ToString() const = 0;
public:
    Utf8StringCR ToString() const 
        {
        if (Utf8String::IsNullOrEmpty(m_queryString.c_str()))
            m_queryString = _ToString();
        return m_queryString;
        }
    void InvalidateQueryString() const {m_queryString.clear();}
    BoundQueryValuesList GetBoundValues() const {return _GetBoundValues();}
    ECPRESENTATION_EXPORT BentleyStatus BindValues(ECSqlStatement&) const;
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
template<typename TBase, typename TContract, typename TResultParameters>
struct EXPORT_VTABLE_ATTRIBUTE PresentationQuery : PresentationQueryBase, IQueryInfoProvider, RapidJsonExtendedDataHolder<>
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
    virtual bool _IsEqual(TBase const& other) const {return m_isOuterQuery == other.m_isOuterQuery && m_resultParameters == other.m_resultParameters;}
    virtual ComplexQuery* _AsComplexQuery() {return nullptr;}
    virtual UnionQuery* _AsUnionQuery() {return nullptr;}
    virtual ExceptQuery* _AsExceptQuery() {return nullptr;}
    virtual StringQuery* _AsStringQuery() {return nullptr;}
    virtual Contract const* _GetContract(uint64_t) const {return nullptr;}
    virtual Contract const* _GetGroupingContract() const {return nullptr;}
    virtual void _OnIsOuterQueryValueChanged() {}

public:
    RefCountedPtr<TBase> Clone() const
        {
        if (nullptr != AsComplexQuery()) return new ComplexPresentationQuery<TBase>(*AsComplexQuery());
        if (nullptr != AsUnionQuery()) return new UnionPresentationQuery<TBase>(*AsUnionQuery());
        if (nullptr != AsExceptQuery()) return new ExceptPresentationQuery<TBase>(*AsExceptQuery());
        if (nullptr != AsStringQuery()) return new StringPresentationQuery<TBase>(*AsStringQuery());
        BeAssert(false);
        return nullptr;
        }

    ComplexQuery const* AsComplexQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsComplexQuery();}
    ComplexQuery* AsComplexQuery() {return _AsComplexQuery();}
    UnionQuery const* AsUnionQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsUnionQuery();}
    UnionQuery* AsUnionQuery() {return _AsUnionQuery();}
    ExceptQuery const* AsExceptQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsExceptQuery();}
    ExceptQuery* AsExceptQuery() {return _AsExceptQuery();}
    StringQuery const* AsStringQuery() const {return const_cast<PresentationQuery<TBase, TContract, TResultParameters>*>(this)->AsStringQuery();}
    StringQuery* AsStringQuery() {return _AsStringQuery();}

    Contract const* GetContract(uint64_t contractId = 0) const {return _GetContract(contractId);}
    Contract const* GetGroupingContract() const {return _GetGroupingContract();}
    bool IsEqual(TBase const& other) const {return _IsEqual(other);}
    ResultParameters const& GetResultParameters() const {return m_resultParameters;}
    ResultParameters& GetResultParametersR() {return m_resultParameters;}

    bool IsOuterQuery() const {return m_isOuterQuery;}
    void SetIsOuterQuery(bool value) {m_isOuterQuery = value; _OnIsOuterQueryValueChanged();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
template<typename TBase>
struct EXPORT_VTABLE_ATTRIBUTE ComplexPresentationQuery : TBase
{
    typedef ComplexPresentationQuery<TBase>     ThisType;
    typedef typename TBase::Contract            Contract;
    typedef typename TBase::ResultParameters    ResultParameters;

    friend struct PresentationQuery<TBase, Contract, ResultParameters>;

    /*=============================================================================**//**
    * @bsiclass                                     Grigas.Petraitis            06/2015
    +===============+===============+===============+===============+===============+==*/
    struct FromClause
        {
        ECClassCP m_class;
        Utf8String m_alias;
        bool m_isPolymorphic;

        FromClause() : m_class(nullptr), m_isPolymorphic(false) {}
        FromClause(ECClassCR fromClass, Utf8CP alias = nullptr, bool isPolymorphic = false)
            : m_class(&fromClass), m_alias(alias), m_isPolymorphic(isPolymorphic) 
            {}
        bool IsValid() const {return nullptr != m_class;}
        };

    /*=============================================================================**//**
    * @bsiclass                                     Grigas.Petraitis            06/2015
    +===============+===============+===============+===============+===============+==*/
    struct JoinUsingClause
        {
        ECClassCP m_join;
        Utf8String m_joinAlias;
        ECRelationshipClassCP m_using;
        Utf8String m_usingAlias;
        bool m_isForward;
        bool m_isPolymorphic;
        bool m_isOuterJoin;

        JoinUsingClause() : m_join(nullptr), m_using(nullptr), m_isPolymorphic(false), m_isForward(false), m_isOuterJoin(false) {}
        JoinUsingClause(ECClassCR joinClass, ECRelationshipClassCR usingRelationship, bool isForward, bool isOuter, bool isPolymorphic, Utf8CP joinAlias, Utf8CP usingAlias)
            : m_join(&joinClass), m_using(&usingRelationship), m_joinAlias(joinAlias), m_usingAlias(usingAlias), m_isPolymorphic(isPolymorphic), m_isForward(isForward), m_isOuterJoin(isOuter)
            {}
        bool IsValid() const {return nullptr != m_join && nullptr != m_using;}
        };

private:
    Utf8String m_orderByClause;
    bvector<FromClause> m_from;
    RefCountedPtr<TBase const> m_nestedQuery;
    Utf8String m_nestedQueryAlias;
    Utf8String m_whereClause;
    BoundQueryValuesList m_whereClauseBindings;
    bvector<bvector<JoinUsingClause>> m_joins;
    RefCountedPtr<Contract const> m_selectContract;
    Utf8String m_selectPrefix;
    bool m_isSelectAll;
    RefCountedPtr<Contract const> m_groupingContract;
    Utf8String m_havingClause;
    BoundQueryValuesList m_havingClauseBindings;
    BoundQueryECValue const* m_limit;
    BoundQueryECValue const* m_offset;

private:
    Utf8String CreateSelectClause() const;
    Utf8String CreateJoinClause() const;
    Utf8String CreateGroupByClause() const;
    ECPRESENTATION_EXPORT void CopyBindings(ComplexPresentationQuery<TBase> const& other);

    static void AppendToSelectClause(Utf8StringR, Utf8CP clause);
    static void Select(Utf8StringR, Utf8CP clause, bool append);
    static void SelectString(Utf8StringR, Utf8CP str, Utf8CP alias = nullptr, bool append = true);
    ECPRESENTATION_EXPORT static void SelectProperty(Utf8StringR, ECPropertyCR prop, Utf8CP alias = nullptr, bool append = true);
    ECPRESENTATION_EXPORT static void SelectField(Utf8StringR, Utf8StringCR clause, Utf8CP alias = nullptr, bool append = true);

protected:
    ComplexPresentationQuery() : m_limit(nullptr), m_offset(nullptr), m_isSelectAll(false) {}
    ComplexPresentationQuery(ComplexPresentationQuery<TBase> const& other) 
        : TBase(other), m_havingClause(other.m_havingClause), m_orderByClause(other.m_orderByClause), m_from(other.m_from), 
        m_nestedQueryAlias(other.m_nestedQueryAlias), m_whereClause(other.m_whereClause), m_joins(other.m_joins), 
        m_selectContract(other.m_selectContract), m_selectPrefix(other.m_selectPrefix), m_isSelectAll(other.m_isSelectAll), 
        m_groupingContract(other.m_groupingContract), m_limit(nullptr), m_offset(nullptr)
        {
        if (other.m_nestedQuery.IsValid())
            m_nestedQuery = other.m_nestedQuery->Clone();
        CopyBindings(other);
        }
    ~ComplexPresentationQuery()
        {
        for (BoundQueryValue const* value : m_whereClauseBindings)
            DELETE_AND_CLEAR(value);
        for (BoundQueryValue const* value : m_havingClauseBindings)
            DELETE_AND_CLEAR(value);
        DELETE_AND_CLEAR(m_limit);
        DELETE_AND_CLEAR(m_offset);
        }
    ThisType* _AsComplexQuery() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(TBase const& other) const override; 
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    ECPRESENTATION_EXPORT Contract const* _GetContract(uint64_t) const override;
    ECPRESENTATION_EXPORT Contract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT BoundQueryValuesList _GetBoundValues() const override;

public:
    static RefCountedPtr<ComplexPresentationQuery<TBase>> Create() {return new ComplexPresentationQuery();}
    ECPRESENTATION_EXPORT bool HasClause(PresentationQueryClauses clause) const;
    ECPRESENTATION_EXPORT Utf8String GetClause(PresentationQueryClauses clause) const;
    TBase const* GetNestedQuery() const {return m_nestedQuery.get();}
    Utf8CP GetSelectPrefix() const {return m_selectPrefix.c_str();}
    ECPRESENTATION_EXPORT void SetBoundValues(BoundQueryValuesListCR bindings);

    ECPRESENTATION_EXPORT ThisType& SelectAll();
    ECPRESENTATION_EXPORT ThisType& SelectContract(Contract const& contract, Utf8CP prefix = nullptr);
    ECPRESENTATION_EXPORT ThisType& From(ECClassCR fromClass, bool polymorphic, Utf8CP alias = nullptr, bool append = true);
    ECPRESENTATION_EXPORT ThisType& From(TBase& nestedQuery, Utf8CP alias = nullptr);
    ECPRESENTATION_EXPORT ThisType& Where(Utf8CP whereClause, BoundQueryValuesListCR, bool append = true);
    ECPRESENTATION_EXPORT ThisType& Join(RelatedClass const& relatedClass, bool append = true);
    ECPRESENTATION_EXPORT ThisType& Join(RelatedClassPath const& path, bool isOuter, bool append = true);
    ECPRESENTATION_EXPORT ThisType& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ThisType& GroupByContract(Contract const& contract);
    ECPRESENTATION_EXPORT ThisType& Having(Utf8CP havingClause, BoundQueryValuesListCR);
    ECPRESENTATION_EXPORT ThisType& Limit(uint64_t limit, uint64_t offset = 0);
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
template<typename TBase>
struct EXPORT_VTABLE_ATTRIBUTE UnionPresentationQuery : TBase
{
    typedef UnionPresentationQuery<TBase>       ThisType;
    typedef typename TBase::Contract            Contract;
    typedef typename TBase::ResultParameters    ResultParameters;

    friend struct PresentationQuery<TBase, Contract, ResultParameters>;

private:
    RefCountedPtr<TBase> m_first;
    RefCountedPtr<TBase> m_second;
    Utf8String m_orderByClause;
    BoundQueryECValue const* m_limit;
    BoundQueryECValue const* m_offset;

private:
    ECPRESENTATION_EXPORT void Init();

protected:
    UnionPresentationQuery(TBase& first, TBase& second) : m_first(&first), m_second(&second), m_limit(nullptr), m_offset(nullptr) {Init();}
    UnionPresentationQuery(UnionPresentationQuery<TBase> const& other)
        : TBase(other), m_orderByClause(other.m_orderByClause), m_limit(nullptr), m_offset(nullptr)
        {
        m_first = other.m_first->Clone();
        m_second = other.m_second->Clone();

        if (nullptr != other.m_limit)
            m_limit = new BoundQueryECValue(*other.m_limit);
        if (nullptr != other.m_offset)
            m_offset = new BoundQueryECValue(*other.m_offset);
        }
    ~UnionPresentationQuery()
        {
        DELETE_AND_CLEAR(m_limit);
        DELETE_AND_CLEAR(m_offset);
        }
    ThisType* _AsUnionQuery() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(TBase const& other) const override; 
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    ECPRESENTATION_EXPORT Contract const* _GetContract(uint64_t) const override;
    ECPRESENTATION_EXPORT Contract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT BoundQueryValuesList _GetBoundValues() const override;
    ECPRESENTATION_EXPORT void _OnIsOuterQueryValueChanged() override;

public:
    static RefCountedPtr<UnionPresentationQuery<TBase>> Create(TBase& first, TBase& second) {return new UnionPresentationQuery(first, second);}
    ECPRESENTATION_EXPORT ThisType& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ThisType& Limit(uint64_t limit, uint64_t offset = 0);
    RefCountedPtr<TBase> GetFirst() const {return m_first;}
    RefCountedPtr<TBase> GetSecond() const {return m_second;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
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
    BoundQueryECValue const* m_limit;
    BoundQueryECValue const* m_offset;

private:
    ECPRESENTATION_EXPORT void Init();

protected:
    ExceptPresentationQuery(TBase& base, TBase& except) : m_base(&base), m_except(&except), m_limit(nullptr), m_offset(nullptr) {Init();}
    ExceptPresentationQuery(ExceptPresentationQuery<TBase> const& other)
        : TBase(other), m_orderByClause(other.m_orderByClause), m_limit(nullptr), m_offset(nullptr)
        {
        m_base = other.m_base->Clone();
        m_except = other.m_except->Clone();

        if (nullptr != other.m_limit)
            m_limit = new BoundQueryECValue(*other.m_limit);
        if (nullptr != other.m_offset)
            m_offset = new BoundQueryECValue(*other.m_offset);
        }
    ~ExceptPresentationQuery()
        {
        DELETE_AND_CLEAR(m_limit);
        DELETE_AND_CLEAR(m_offset);
        }
    ThisType* _AsExceptQuery() override {return this;}
    ECPRESENTATION_EXPORT bvector<Utf8CP> _GetSelectAliases(int) const override;
    ECPRESENTATION_EXPORT bool _IsEqual(TBase const& other) const override; 
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    ECPRESENTATION_EXPORT Contract const* _GetContract(uint64_t) const override;
    ECPRESENTATION_EXPORT Contract const* _GetGroupingContract() const override;
    ECPRESENTATION_EXPORT BoundQueryValuesList _GetBoundValues() const override;
    ECPRESENTATION_EXPORT void _OnIsOuterQueryValueChanged() override;

public:
    static RefCountedPtr<ExceptPresentationQuery<TBase>> Create(TBase& base, TBase& except) {return new ExceptPresentationQuery(base, except);}
    ECPRESENTATION_EXPORT ThisType& OrderBy(Utf8CP orderByClause);
    ECPRESENTATION_EXPORT ThisType& Limit(uint64_t limit, uint64_t offset = 0);
    RefCountedPtr<TBase> GetBase() const {return m_base;}
    RefCountedPtr<TBase> GetExcept() const {return m_except;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
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

private:
    ECPRESENTATION_EXPORT void CopyBindings(BoundQueryValuesListCR bindings);

protected:
    StringPresentationQuery(Utf8StringCR query, BoundQueryValuesList bindings) : m_query(query) {CopyBindings(bindings);}
    StringPresentationQuery(StringPresentationQuery<TBase> const& other) : TBase(other), m_query(other.m_query) {CopyBindings(other.m_bindings);}
    ThisType* _AsStringQuery() override {return this;}
    bvector<Utf8CP> _GetSelectAliases(int) const override {return bvector<Utf8CP>();}
    ECPRESENTATION_EXPORT bool _IsEqual(TBase const& other) const override; 
    ECPRESENTATION_EXPORT Utf8String _ToString() const override;
    BoundQueryValuesList _GetBoundValues() const override {return m_bindings;}

public:
    static RefCountedPtr<StringPresentationQuery<TBase>> Create(Utf8StringCR query, BoundQueryValuesList bindings = BoundQueryValuesList())
        {
        return new StringPresentationQuery(query, bindings);
        }
    ~StringPresentationQuery()
        {
        for (BoundQueryValue const* value : m_bindings)
            DELETE_AND_CLEAR(value);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2016
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
* @bsiclass                                     Grigas.Petraitis                09/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE GenericQuery : PresentationQuery<GenericQuery, PresentationQueryContract, GenericQueryResultParameters> {};
DEFINE_QUERY_TYPEDEFS(GenericQuery)
INSTANTIATE_QUERY_SUBCLASS(GenericQuery, extern, EXPORT_VTABLE_ATTRIBUTE)

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryResultParameters
{
private:
    ChildNodeSpecificationCP m_specification;
    NavNodeExtendedData m_navNodeExtendedData;
    NavigationQueryResultType m_resultType;
    bset<ECClassId> m_matchingRelationshipIds;
    bool m_hasInstanceGroups;
    
public:
    NavNodeExtendedData& GetNavNodeExtendedDataR() {return m_navNodeExtendedData;}
    void SetResultType(NavigationQueryResultType type) {m_resultType = type;}
    void SetHasInstanceGroups(bool value) {m_hasInstanceGroups = value;}
    void SetSpecification(ChildNodeSpecificationCP spec) {m_specification = spec;}
    bset<ECClassId>& GetMatchingRelationshipIds() {return m_matchingRelationshipIds;}
    void OnContractSelected(NavigationQueryContractCR contract) {m_resultType = contract.GetResultType();}
    void MergeWith(NavigationQueryResultParameters const&);    

public:
    NavigationQueryResultParameters() 
        : m_resultType(NavigationQueryResultType::Invalid), m_specification(nullptr), m_hasInstanceGroups(false)
        {}
    NavigationQueryResultParameters(NavigationQueryResultParameters const& other) 
        : m_navNodeExtendedData(other.m_navNodeExtendedData), m_specification(other.m_specification), m_resultType(other.m_resultType), m_hasInstanceGroups(other.m_hasInstanceGroups), m_matchingRelationshipIds(other.m_matchingRelationshipIds)
        {}
    NavigationQueryResultParameters& operator=(NavigationQueryResultParameters const& other)
        {
        m_navNodeExtendedData = other.m_navNodeExtendedData;
        m_specification = other.m_specification;
        m_resultType = other.m_resultType;
        m_hasInstanceGroups = other.m_hasInstanceGroups;
        m_matchingRelationshipIds = other.m_matchingRelationshipIds;
        return *this;
        }
    ECPRESENTATION_EXPORT bool operator==(NavigationQueryResultParameters const& other) const;
    NavNodeExtendedData const& GetNavNodeExtendedData() const {return m_navNodeExtendedData;}
    NavigationQueryResultType GetResultType() const {return m_resultType;}
    bool HasInstanceGroups() const {return m_hasInstanceGroups;}
    ChildNodeSpecificationCP GetSpecification() const {return m_specification;}
    bset<ECClassId> const& GetMatchingRelationshipIds() const {return m_matchingRelationshipIds;}
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavigationQuery : PresentationQuery<NavigationQuery, NavigationQueryContract, NavigationQueryResultParameters> {};
DEFINE_QUERY_TYPEDEFS(NavigationQuery)
INSTANTIATE_QUERY_SUBCLASS(NavigationQuery, extern, EXPORT_VTABLE_ATTRIBUTE)

#define NAVIGATIONQUERY_EXTENDEDDATA_Ranges     "Ranges"
/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryExtendedData : RapidJsonAccessor
    {
    NavigationQueryExtendedData(RapidJsonValueCR data) : RapidJsonAccessor(data) {}
    NavigationQueryExtendedData(NavigationQuery& query) : RapidJsonAccessor(query) {}
    NavigationQueryExtendedData(NavigationQuery const& query) : RapidJsonAccessor(query) {}

    bool HasRangesData() const {return GetJson().HasMember(NAVIGATIONQUERY_EXTENDEDDATA_Ranges);}
    int GetRangeIndex(BeSQLite::DbValue const&) const;
    Utf8String GetRangeLabel(int rangeIndex) const;
    Utf8CP GetRangeImageId(int rangeIndex) const;
    ECPRESENTATION_EXPORT void AddRangesData(ECPropertyCR, PropertyGroupCR);
    };

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                04/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ContentQuery : PresentationQuery<ContentQuery, ContentQueryContract, GenericQueryResultParameters> {};
DEFINE_QUERY_TYPEDEFS(ContentQuery)
INSTANTIATE_QUERY_SUBCLASS(ContentQuery, extern, EXPORT_VTABLE_ATTRIBUTE)

END_BENTLEY_ECPRESENTATION_NAMESPACE
