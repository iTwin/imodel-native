/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Diagnostics.h>

ECPRESENTATION_TYPEDEFS(ECPresentationManager)

ECPRESENTATION_TYPEDEFS(IConnectionCache)
ECPRESENTATION_TYPEDEFS(IConnectionManager)
ECPRESENTATION_TYPEDEFS(IConnection)
ECPRESENTATION_REFCOUNTED_PTR(IConnection)

ECPRESENTATION_TYPEDEFS(IRulesPreprocessor)
ECPRESENTATION_REFCOUNTED_PTR(IRulesPreprocessor)
ECPRESENTATION_TYPEDEFS(PageOptions)
ECPRESENTATION_TYPEDEFS(RelatedClass)
ECPRESENTATION_TYPEDEFS(KeySet)
ECPRESENTATION_REFCOUNTED_PTR(KeySet)

ECPRESENTATION_TYPEDEFS(NavNodeKey)
ECPRESENTATION_REFCOUNTED_PTR(NavNodeKey)
ECPRESENTATION_TYPEDEFS(NavNode)
ECPRESENTATION_REFCOUNTED_PTR(NavNode)
ECPRESENTATION_TYPEDEFS(INavNodeLocater)
ECPRESENTATION_TYPEDEFS(INavNodesFactory)
ECPRESENTATION_REFCOUNTED_PTR(INavNodesFactory)
ECPRESENTATION_TYPEDEFS(LabelDefinition)
ECPRESENTATION_REFCOUNTED_PTR(LabelDefinition)

ECPRESENTATION_TYPEDEFS(Content)
ECPRESENTATION_REFCOUNTED_PTR(Content)
ECPRESENTATION_TYPEDEFS(ContentSetItem)
ECPRESENTATION_REFCOUNTED_PTR(ContentSetItem)
ECPRESENTATION_TYPEDEFS(ContentDescriptor)
ECPRESENTATION_REFCOUNTED_PTR(ContentDescriptor)
ECPRESENTATION_TYPEDEFS(IPropertyCategorySupplier)
ECPRESENTATION_TYPEDEFS(SelectionInfo)
ECPRESENTATION_REFCOUNTED_PTR(SelectionInfo)
ECPRESENTATION_TYPEDEFS(DisplayValueGroup)
ECPRESENTATION_REFCOUNTED_PTR(DisplayValueGroup)
ECPRESENTATION_TYPEDEFS(IContentFieldMatcher)
ECPRESENTATION_TYPEDEFS(IECPropertyFormatter)

ECPRESENTATION_TYPEDEFS(ICancelationToken)
ECPRESENTATION_REFCOUNTED_PTR(ICancelationToken)

ECPRESENTATION_TYPEDEFS(PresentationQueryBase)
ECPRESENTATION_REFCOUNTED_PTR(PresentationQueryBase)

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

#define NOT_NULL_PRECONDITION(value, location) \
    if (value == nullptr) \
        DIAGNOSTICS_HANDLE_FAILURE(DiagnosticsCategory::Default, "[" location "] Encountered NULL object.")

#define NUMERIC_LESS_COMPARE(lhs, rhs) \
    { \
    if (lhs < rhs) \
        return true; \
    if (rhs < lhs) \
        return false; \
    }

#define STR_LESS_COMPARE(lhs, rhs) \
    { \
    int cmp = strcmp(lhs, rhs); \
    if (cmp < 0) \
        return true; \
    if (cmp > 0) \
        return false; \
    }

#define PTR_VALUE_LESS_COMPARE(lhs, rhs) \
    { \
    if (nullptr == lhs && nullptr != rhs) \
        return true; \
    if (nullptr != lhs && nullptr == rhs) \
        return false; \
    return nullptr != lhs && nullptr != rhs && *lhs < *rhs; \
    }

/*=================================================================================**//**
* ECClass comparer which guarantees that classes in a sorted set always are in the same
* order (as opposed to the default comparer which compares by pointers).
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECSchemaNameComparer
    {
    bool operator() (ECSchemaCP const& lhs, ECSchemaCP const& rhs) const {return lhs->GetSchemaKey() < rhs->GetSchemaKey();}
    };

/*=================================================================================**//**
* ECClass comparer which guarantees that classes in a sorted set always are in the same
* order (as opposed to the default comparer which compares by pointers).
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECClassNameComparer
    {
    bool operator() (ECClassCP const& lhs, ECClassCP const& rhs) const {return strcmp(lhs->GetFullName(), rhs->GetFullName()) < 0;}
    };

//=======================================================================================
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct ECClassInstanceKey
{
private:
    ECClassCP m_class;
    ECInstanceId m_id;
public:
    ECClassInstanceKey() : m_class(nullptr) {}
    ECClassInstanceKey(ECClassCP cls, ECInstanceId id) : m_class(cls), m_id(id) {}
    ECClassInstanceKey(ECClassCR cls, ECInstanceId id) : m_class(&cls), m_id(id) {}
    bool IsValid() const {return (nullptr != m_class) && m_id.IsValid();}
    bool operator<(ECClassInstanceKey const& other) const { return m_class < other.m_class || m_class == other.m_class && m_id < other.m_id; }
    bool operator==(ECClassInstanceKey const& other) const {return m_class == other.m_class && m_id == other.m_id;}
    bool operator==(ECInstanceKeyCR other) const {return m_class->GetId() == other.GetClassId() && m_id == other.GetInstanceId();}
    ECClassCP GetClass() const {return m_class;}
    ECInstanceId GetId() const {return m_id;}
};
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ECClassInstanceKey);

//=======================================================================================
//! Sort direction.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
enum class SortDirection
    {
    Ascending,  //!< Sort in ascending order.
    Descending, //!< Sort in descending order.
    };

//=======================================================================================
//! Unit system
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
enum class UnitSystem
    {
    Undefined,
    Metric,
    BritishImperial,
    UsCustomary,
    UsSurvey,
    };

//=======================================================================================
//! Paging options.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct PageOptions
{
private:
    size_t m_pageStart;
    size_t m_pageSize;

public:
    //! Constructor.
    //! @param[in] pageStart Start of the page. Defaults to 0.
    //! @param[in] pageSize The size of the page. Defaults to 0 which means infinite.
    PageOptions(size_t pageStart = 0, size_t pageSize = 0) : m_pageStart(pageStart), m_pageSize(pageSize) {}

    //! Set the page start index.
    void SetPageStart(size_t pageStart) {m_pageStart = pageStart;}

    //! Set the page size.
    //! @note 0 means infinite.
    void SetPageSize(size_t size) {m_pageSize = size;}

    //! Get page start index.
    size_t GetPageStart() const {return m_pageStart;}

    //! Get page size.
    //! @note 0 means infinite.
    size_t GetPageSize() const {return m_pageSize;}

    //! Check whether this @ref PageOptions object is equal to the supplied one.
    bool Equals(PageOptions const& other) const {return m_pageStart == other.m_pageStart && m_pageSize == other.m_pageSize;}

    //! Are options empty
    bool Empty() const {return m_pageStart == 0 && m_pageSize == 0;}
};

//! Pair of node hash paths representing position during hierarchies comparison.
typedef bpair<Utf8String, Utf8String> HierarchyComparePosition;
typedef std::shared_ptr<HierarchyComparePosition> HierarchyComparePositionPtr;

//=======================================================================================
//! Data structure that describes an ECClass used to selected ECInstances using ECSQL.
// @bsiclass
//=======================================================================================
template<typename TClass = ECClass>
struct SelectClass
{
private:
    TClass const* m_class;
    Utf8String m_alias;
    bool m_isPolymorphic;
    bool m_disqualify;
public:
    //! Constructor. Creates an invalid object.
    SelectClass() : m_class(nullptr), m_isPolymorphic(false), m_disqualify(false) {}
    //! Constructor. Creates an information instance with the specified ECClass.
    SelectClass(TClass const& selectClass, Utf8String alias) : m_class(&selectClass), m_alias(alias), m_isPolymorphic(true), m_disqualify(false) {}
    //! Constructor. Creates an information instance with the specified ECClass.
    SelectClass(TClass const& selectClass, Utf8String alias, bool isPolymorphic, bool disqualify = false) : m_class(&selectClass), m_alias(alias), m_isPolymorphic(isPolymorphic), m_disqualify(disqualify) {}
    bool IsValid() const {return m_class != nullptr;}
    //! Returns whether this info is equal to the supplied one.
    bool Equals(SelectClass const& other) const {return m_class == other.m_class && m_alias == other.m_alias && m_isPolymorphic == other.m_isPolymorphic && m_disqualify == other.m_disqualify;}
    bool operator==(SelectClass const& other) const {return Equals(other);}
    bool operator!=(SelectClass const& other) const {return !Equals(other);}
    bool operator<(SelectClass const& other) const
        {
        NUMERIC_LESS_COMPARE(m_class, other.m_class);
        NUMERIC_LESS_COMPARE(m_isPolymorphic, other.m_isPolymorphic);
        NUMERIC_LESS_COMPARE(m_disqualify, other.m_disqualify);
        STR_LESS_COMPARE(m_alias.c_str(), other.m_alias.c_str());
        return false;
        }
    //! Get select ECClass.
    TClass const& GetClass() const {return *m_class;}
    void SetClass(TClass const& ecClass) {m_class = &ecClass;}
    //! Get select alias.
    Utf8StringCR GetAlias() const {return m_alias;}
    void SetAlias(Utf8String alias) {m_alias = alias;}
    //! Is select polymorphic.
    bool IsSelectPolymorphic() const {return m_isPolymorphic;}
    void SetIsSelectPolymorphic(bool value) {m_isPolymorphic = value;}
    //! Should the class be disqualified.
    //! ECClass disqualification is an ECSQL thing which means that ECSQL should internally
    //! disqualify join indexes when creating an SQLite view for this ECClass.
    bool ShouldDisqualify() const {return m_disqualify;}
    void SetShouldDisqualify(bool value) {m_disqualify = value;}
};

//=======================================================================================
//! Data structure that describes an ECClass used to selected ECInstances using ECSQL. In
//! addition, it specifies whether any derived classes should be excluded.
// @bsiclass
//=======================================================================================
template<typename TClass = ECClass>
struct SelectClassWithExcludes : SelectClass<TClass>
{
private:
    bvector<SelectClass<TClass>> m_derivedExcludes;
public:
    SelectClassWithExcludes() : SelectClass<TClass>() {}
    SelectClassWithExcludes(SelectClass<TClass> const& selectClass) : SelectClass<TClass>(selectClass) {}
    SelectClassWithExcludes(TClass const& selectClass, Utf8String alias) : SelectClass<TClass>(selectClass, alias) {}
    SelectClassWithExcludes(TClass const& selectClass, Utf8String alias, bool isPolymorphic) : SelectClass<TClass>(selectClass, alias, isPolymorphic) {}
    //! Returns whether this info is equal to the supplied one.
    bool Equals(SelectClassWithExcludes const& other) const {return SelectClass<TClass>::Equals(other) && m_derivedExcludes == other.m_derivedExcludes;}
    bool operator==(SelectClassWithExcludes const& other) const {return Equals(other);}
    bool operator!=(SelectClassWithExcludes const& other) const {return !Equals(other);}
    //! Get derived class excludes
    bvector<SelectClass<TClass>> const& GetDerivedExcludedClasses() const {return m_derivedExcludes;}
    bvector<SelectClass<TClass>>& GetDerivedExcludedClasses() {return m_derivedExcludes;}
};

//=======================================================================================
//! A structure that describes a related class and the properties of that relationship.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct RelatedClass
{
    /* Compares class names */
    struct ClassNamesComparer { ECPRESENTATION_EXPORT bool operator()(RelatedClass const& lhs, RelatedClass const& rhs) const; };
    /* Compares class pointers */
    struct ClassPointersComparer { ECPRESENTATION_EXPORT bool operator()(RelatedClass const& lhs, RelatedClass const& rhs) const; };
    /* Compares class pointers and aliases */
    struct FullComparer { ECPRESENTATION_EXPORT bool operator()(RelatedClass const& lhs, RelatedClass const& rhs) const; };

private:
    ECClassCP m_source;
    SelectClass<ECRelationshipClass> m_relationship;
    bool m_isForwardRelationship;
    SelectClassWithExcludes<ECClass> m_target;
    bvector<ECInstanceId> m_targetIds;
    bool m_isTargetOptional;
    Utf8String m_targetInstanceFilter;

public:
    RelatedClass() : m_source(nullptr), m_isForwardRelationship(false), m_isTargetOptional(true) {}
    RelatedClass(ECClassCR source, SelectClass<ECRelationshipClass> relationship, bool isForward, SelectClassWithExcludes<ECClass> target, bool isTargetOptional = true)
        : m_source(&source), m_relationship(relationship), m_isForwardRelationship(isForward), m_target(target), m_isTargetOptional(isTargetOptional)
        {}
    RelatedClass(ECClassCR source, SelectClass<ECRelationshipClass> relationship, bool isForward, SelectClassWithExcludes<ECClass> target, bvector<ECInstanceId> targetIds, bool isTargetOptional = true)
        : m_source(&source), m_relationship(relationship), m_isForwardRelationship(isForward), m_target(target), m_targetIds(targetIds), m_isTargetOptional(isTargetOptional)
        {}
    RelatedClass(ECClassCR source, SelectClassWithExcludes<ECClass> target, bvector<ECInstanceId> targetIds, bool isTargetOptional = true)
        : m_source(&source), m_target(target), m_targetIds(targetIds), m_isTargetOptional(isTargetOptional), m_isForwardRelationship(false)
        {}

    //! Checks whether this object is equal to the supplied one.
    ECPRESENTATION_EXPORT bool IsEqual(RelatedClass const& other) const;
    bool operator==(RelatedClass const& other) const {return IsEqual(other);}
    bool operator!=(RelatedClass const& other) const {return !IsEqual(other);}
    bool operator<(RelatedClass const& other) const {static const FullComparer cmp; return cmp(*this, other);}

    //! Is this structure valid.
    bool IsValid() const {return nullptr != m_source && m_target.IsValid();}

    //! The source class.
    void SetSourceClass(ECN::ECClassCR sourceClass) {m_source = &sourceClass;}
    ECN::ECClassCP GetSourceClass() const {return m_source;}

    //! The related class.
    void SetTargetClass(SelectClassWithExcludes<ECClass> targetClass) {m_target = targetClass;}
    SelectClassWithExcludes<ECClass> const& GetTargetClass() const {return m_target;}
    SelectClassWithExcludes<ECClass>& GetTargetClass() {return m_target;}

    //! Target instance filter
    Utf8StringCR GetTargetInstanceFilter() const {return m_targetInstanceFilter;}
    void SetTargetInstanceFilter(Utf8String targetInstanceFilter) {m_targetInstanceFilter = targetInstanceFilter;}

    //! Target ECInstance IDs. Used to join the target to source if relationship is not specified
    bvector<ECInstanceId> const& GetTargetIds() const {return m_targetIds;}
    bvector<ECInstanceId>& GetTargetIds() {return m_targetIds;}
    void SetTargetIds(bvector<ECInstanceId> ids) {m_targetIds = ids;}

    //! Relationship used to access the related class.
    void SetRelationship(SelectClass<ECRelationshipClass> const& relationship) {m_relationship = relationship;}
    SelectClass<ECRelationshipClass> const& GetRelationship() const {return m_relationship;}
    SelectClass<ECRelationshipClass>& GetRelationship() {return m_relationship;}

    //! Should the relationship be followed in a forward direction to access the related class.
    bool IsForwardRelationship() const {return m_isForwardRelationship;}
    void SetIsForwardRelationship(bool value) {m_isForwardRelationship = value;}

    //! Get the navigation property for this relationship.
    ECN::NavigationECPropertyCP GetNavigationProperty() const;

    //! Is related class queried using outer join
    bool IsTargetOptional() const {return m_isTargetOptional;}
    void SetIsTargetOptional(bool value) {m_isTargetOptional = value;}

    ECPRESENTATION_EXPORT bool ClassesEqual(RelatedClassCR other) const;
    ECPRESENTATION_EXPORT bool Is(RelatedClassCR other) const;
    ECPRESENTATION_EXPORT bool SourceAndRelationshipEqual(RelatedClassCR) const;

    ECPRESENTATION_EXPORT static BentleyStatus Unify(RelatedClass& result, RelatedClass const&, RelatedClass const&);
};

//! A stack of ECClass and ECRelationshipClass pairs representing a path of relationships.
//! @ingroup GROUP_Presentation
struct RelatedClassPath : bvector<RelatedClass>
{
    DEFINE_T_SUPER(bvector<RelatedClass>);

private:
    ECInstanceKey m_inputKey;
    Nullable<uint64_t> m_targetsCount;

public:
    RelatedClassPath() : T_Super() {}
    RelatedClassPath(std::initializer_list<RelatedClass> list) : T_Super(list) {}
    RelatedClassPath(RelatedClassPath::const_iterator begin, RelatedClassPath::const_iterator end) : T_Super(begin, end) {}

    ECInstanceKeyCR GetInputKey() const {return m_inputKey;}
    void SetInputKey(ECInstanceKey value) {m_inputKey = std::move(value);}

    Nullable<uint64_t> const& GetTargetsCount() const {return m_targetsCount;}
    void SetTargetsCount(Nullable<uint64_t> value) {m_targetsCount = value;}

    ECPRESENTATION_EXPORT BentleyStatus Reverse(Utf8CP resultTargetClassAlias, bool isResultTargetPolymorphic);
    ECPRESENTATION_EXPORT bool ClassesEqual(RelatedClassPath const&) const;
    ECPRESENTATION_EXPORT bool StartsWith(RelatedClassPath const&) const;

    ECPRESENTATION_EXPORT static BentleyStatus Unify(RelatedClassPath& result, RelatedClassPath const&, RelatedClassPath const&);
    ECPRESENTATION_EXPORT static RelatedClassPath Combine(RelatedClassPath const&, RelatedClassPath const&);
    ECPRESENTATION_EXPORT static RelatedClassPath Combine(RelatedClassPath, RelatedClass);
};
typedef RelatedClassPath& RelatedClassPathR;
typedef RelatedClassPath const& RelatedClassPathCR;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct IUsedRulesetVariablesListener : RefCountedBase
{
protected:
    virtual void _OnVariableUsed(Utf8CP settingId) = 0;
public:
    void OnVariableUsed(Utf8CP settingId) {_OnVariableUsed(settingId);}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ICancelationToken : RefCountedBase
{
protected:
    virtual bool _IsCanceled() const = 0;
public:
    bool IsCanceled() const {return _IsCanceled();}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SimpleCancelationToken : ICancelationToken
{
private:
    BeAtomic<bool> m_isCanceled;
protected:
    SimpleCancelationToken() : m_isCanceled(false) {}
    bool _IsCanceled() const override {return m_isCanceled.load();}
public:
    static RefCountedPtr<SimpleCancelationToken> Create() {return new SimpleCancelationToken();}
    void SetCanceled(bool value) {m_isCanceled.store(value);}
};
typedef RefCountedPtr<SimpleCancelationToken> SimpleCancelationTokenPtr;

//=======================================================================================
//! Wrapper of bvector that ensures pushed items are unique like in a set.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
template <class T>
struct VectorSet
{
private:
    bvector<T> m_vector;
    bset<T>    m_set;
public:
    using iterator = typename bvector<T>::iterator;
    using const_iterator = typename bvector<T>::const_iterator;
    //! Iterator to start of vector
    iterator begin() { return m_vector.begin(); }
    //! Iterator to end of vector
    iterator end() { return m_vector.end(); }
    //! Const iterator to start of vector
    const_iterator begin() const { return m_vector.begin(); }
    //! Const iterator to end of vector
    const_iterator end() const { return m_vector.end(); }
    //! Item at front of vector
    const T& front() const { return m_vector.front(); }
    //! Item at back of vector
    const T& back() const { return m_vector.back(); }
    //! Push back with uniqueness check
    void push_back(const T& item) { if (m_set.insert(item).second) m_vector.push_back(item); }
    //! Is vector empty
    bool empty() const { return m_vector.empty(); }
    //! Vector size
    size_t size() const { return m_vector.size(); }
    //! Finds item in vector and returns iterator to it.
    iterator find(const T& key) { return std::find(begin(), end(), key); }
};

//=======================================================================================
//! An interface for a ECClass formatter.
// @bsiclass
//=======================================================================================
struct IECClassSerializer
{
protected:
    virtual rapidjson::Document _SerializeECClass(ECClassCR ecClass, rapidjson::Document::AllocatorType& allocator) = 0;
public:
	virtual ~IECClassSerializer() {}

	rapidjson::Document SerializeECClass(ECClassCR ecClass, rapidjson::Document::AllocatorType& allocator)
	    {
		return _SerializeECClass(ecClass, allocator);
	    }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ECPresentationSerializerContext
{
private:
    ECPresentation::UnitSystem m_unitSystem;
    IECPropertyFormatter const* m_propertyFormatter;
	IECClassSerializer* m_classSerializer;
public:
    ECPresentationSerializerContext(): m_unitSystem(UnitSystem::Undefined), m_propertyFormatter(nullptr), m_classSerializer(nullptr) {}
    ECPresentationSerializerContext(UnitSystem unitSystem, IECPropertyFormatter const* formatter, IECClassSerializer* serializer = nullptr) : m_unitSystem(unitSystem), m_propertyFormatter(formatter), m_classSerializer(serializer) {}

    void SetUnitSystem(UnitSystem unitSystem) {m_unitSystem = unitSystem;}
    UnitSystem GetUnitSystem() const {return m_unitSystem;}
    void SetPropertyFormatter(IECPropertyFormatter const* formatter) {m_propertyFormatter = formatter;}
    ECPresentation::IECPropertyFormatter const* GetPropertyFormatter() {return m_propertyFormatter;}
	void SetClassSerializer(IECClassSerializer* serializer) { m_classSerializer = serializer; }
	ECPresentation::IECClassSerializer* GetClassSerializer() { return m_classSerializer; }
};
typedef ECPresentationSerializerContext& ECPresentationSerializerContextR;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
template<typename TValue>
struct PossiblyApproximate
{
private:
    TValue m_value;
    bool m_isApproximate;
public:
    PossiblyApproximate(TValue value, bool isApproximate)
        : m_value(value), m_isApproximate(isApproximate)
        {}
    TValue const& GetValue() const {return m_value;}
    bool IsApproximate() const {return m_isApproximate;}
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct InstanceFilterDefinition
{
private:
    ECClassCP m_selectClass;
    Utf8String m_expression;
    bvector<RelatedClassPath> m_relatedInstances;

public:
    InstanceFilterDefinition() : m_selectClass(nullptr) {}
    InstanceFilterDefinition(Utf8String expression) : m_expression(expression), m_selectClass(nullptr) {}
    InstanceFilterDefinition(Utf8String expression, ECClassCP selectClass, bvector<RelatedClassPath> const& relatedInstances)
        : m_expression(expression), m_selectClass(selectClass), m_relatedInstances(relatedInstances)
        {}

    bool Equals(InstanceFilterDefinition const& other) const {return m_selectClass == other.m_selectClass && m_expression.Equals(other.m_expression) && m_relatedInstances == other.m_relatedInstances; }
    bool operator==(InstanceFilterDefinition const& other) const {return Equals(other);}
    bool operator!=(InstanceFilterDefinition const& other) const {return !Equals(other);}

    ECClassCP GetSelectClass() const {return m_selectClass;}
    bvector<RelatedClassPath> const& GetRelatedInstances() const {return m_relatedInstances;}
    Utf8StringCR GetExpression() const {return m_expression;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
