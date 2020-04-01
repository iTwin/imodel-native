/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>

ECPRESENTATION_TYPEDEFS(IConnectionCache)
ECPRESENTATION_TYPEDEFS(IConnectionManager)
ECPRESENTATION_TYPEDEFS(IConnection)
ECPRESENTATION_REFCOUNTED_PTR(IConnection)

ECPRESENTATION_TYPEDEFS(IECPresentationManager)
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

ECPRESENTATION_TYPEDEFS(ICancelationToken)
ECPRESENTATION_REFCOUNTED_PTR(ICancelationToken)

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*=================================================================================**//**
* ECClass comparer which guarantees that classes in a sorted set always are in the same
* order (as opposed to the default comparer which compares by pointers).
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECSchemaNameComparer
    {
    bool operator() (ECSchemaCP const& lhs, ECSchemaCP const& rhs) const {return lhs->GetSchemaKey() < rhs->GetSchemaKey();}
    };

/*=================================================================================**//**
* ECClass comparer which guarantees that classes in a sorted set always are in the same
* order (as opposed to the default comparer which compares by pointers).
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECClassNameComparer
    {
    bool operator() (ECClassCP const& lhs, ECClassCP const& rhs) const {return strcmp(lhs->GetFullName(), rhs->GetFullName()) < 0;}
    };

//=======================================================================================
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                04/2018
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
    bool operator==(ECClassInstanceKey const& other) const {return m_class == other.m_class && m_id == other.m_id;}
    bool operator==(ECInstanceKeyCR other) const {return m_class->GetId() == other.GetClassId() && m_id == other.GetInstanceId();}
    ECClassCP GetClass() const {return m_class;}
    ECInstanceId GetId() const {return m_id;}
};
DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(ECClassInstanceKey);

//=======================================================================================
//! Sort direction.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
enum class SortDirection
    {
    Ascending,  //!< Sort in ascending order.
    Descending, //!< Sort in descending order.
    };

//=======================================================================================
//! Unit system
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                04/2020
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
// @bsiclass                                    Grigas.Petraitis                05/2016
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
};

//=======================================================================================
//! Data structure that describes an ECClass used to selected ECInstances using ECSQL.
// @bsiclass                                    Grigas.Petraitis                12/2019
//=======================================================================================
struct SelectClass
{
private:
    ECClassCP m_class;
    bool m_isPolymorphic;
public:
    //! Constructor. Creates an invalid object.
    SelectClass() : m_class(nullptr) {}
    //! Constructor. Creates an information instance with the specified ECClass.
    SelectClass(ECClassCR selectClass) : m_class(&selectClass), m_isPolymorphic(true) {}
    //! Constructor. Creates an information instance with the specified ECClass.
    SelectClass(ECClassCR selectClass, bool isPolymorphic) : m_class(&selectClass), m_isPolymorphic(isPolymorphic) {}
    bool IsValid() const {return m_class != nullptr;}
    //! Returns whether this info is equal to the supplied one.
    bool Equals(SelectClass const& other) const {return m_class == other.m_class && m_isPolymorphic == other.m_isPolymorphic;}
    bool operator==(SelectClass const& other) const {return Equals(other);}
    bool operator!=(SelectClass const& other) const {return !Equals(other);}
    bool operator<(SelectClass const& other) const
        {
        if (!m_class && other.m_class)
            return true;
        if (!other.m_class)
            return false;
        int nameCmp = strcmp(m_class->GetFullName(), other.m_class->GetFullName());
        return nameCmp < 0;
        }
    //! Get the select ECClass.
    ECClassCR GetClass() const {return *m_class;}
    void SetClass(ECClassCR ecClass) {m_class = &ecClass;}
    //! Is the select polymorphic.
    bool IsSelectPolymorphic() const {return m_isPolymorphic;}
    void SetIsSelectPolymorphic(bool value) {m_isPolymorphic = value;}
};

//=======================================================================================
//! Data structure that describes an ECClass used to selected ECInstances using ECSQL. In
//! addition, it specifies whether any derived classes should be excluded.
// @bsiclass                                    Grigas.Petraitis                12/2019
//=======================================================================================
struct SelectClassWithExcludes : SelectClass
{
private:
    bvector<SelectClass> m_derivedExcludes;
public:
    SelectClassWithExcludes() : SelectClass() {}
    SelectClassWithExcludes(SelectClass const& selectClass) : SelectClass(selectClass) {}
    SelectClassWithExcludes(ECClassCR selectClass) : SelectClass(selectClass) {}
    SelectClassWithExcludes(ECClassCR selectClass, bool isPolymorphic) : SelectClass(selectClass, isPolymorphic) {}
    //! Returns whether this info is equal to the supplied one.
    bool Equals(SelectClassWithExcludes const& other) const {return SelectClass::Equals(other) && m_derivedExcludes == other.m_derivedExcludes;}
    bool operator==(SelectClassWithExcludes const& other) const {return Equals(other);}
    bool operator!=(SelectClassWithExcludes const& other) const {return !Equals(other);}
    //! Get derived class excludes
    bvector<SelectClass> const& GetDerivedExcludedClasses() const {return m_derivedExcludes;}
    bvector<SelectClass>& GetDerivedExcludedClasses() {return m_derivedExcludes;}
};

//=======================================================================================
//! A structure that describes a related class and the properties of that relationship.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                07/2015
//=======================================================================================
struct RelatedClass
{
private:
    ECClassCP m_source;
    ECRelationshipClassCP m_relationship;
    Utf8String m_relationshipAlias;
    bool m_isForwardRelationship;
    SelectClassWithExcludes m_target;
    Utf8String m_targetAlias;
    bset<ECInstanceId> m_targetIds;
    bool m_isTargetOptional;

public:
    RelatedClass() : m_source(nullptr), m_relationship(nullptr), m_isForwardRelationship(false), m_isTargetOptional(true) {}
    //! deprecated
    RelatedClass(ECN::ECClassCR source, SelectClassWithExcludes target, ECN::ECRelationshipClassCR relationship, bool isForward, Utf8CP targetAlias = nullptr, Utf8CP relationshipAlias = nullptr, bool isTargetOptional = true)
        : m_source(&source), m_relationship(&relationship), m_isForwardRelationship(isForward), m_target(target), m_targetAlias(targetAlias), m_relationshipAlias(relationshipAlias), m_isTargetOptional(isTargetOptional)
        {}
    RelatedClass(ECClassCR source, ECRelationshipClassCR relationship, bool isForward, Utf8String relationshipAlias, SelectClassWithExcludes target, Utf8String targetAlias, bool isTargetOptional = true)
        : m_source(&source), m_relationship(&relationship), m_isForwardRelationship(isForward), m_relationshipAlias(relationshipAlias), m_target(target), m_targetAlias(targetAlias), m_isTargetOptional(isTargetOptional)
        {}
    RelatedClass(ECClassCR source, ECRelationshipClassCR relationship, bool isForward, Utf8String relationshipAlias, SelectClassWithExcludes target, bset<ECInstanceId> targetIds, Utf8String targetAlias, bool isTargetOptional = true)
        : m_source(&source), m_relationship(&relationship), m_isForwardRelationship(isForward), m_relationshipAlias(relationshipAlias), m_target(target), m_targetIds(targetIds), m_targetAlias(targetAlias), m_isTargetOptional(isTargetOptional)
        {}
    RelatedClass(ECClassCR source, SelectClassWithExcludes target, bset<ECInstanceId> targetIds, Utf8String targetAlias, bool isTargetOptional = true)
        : m_source(&source), m_relationship(nullptr), m_isForwardRelationship(false), m_target(target), m_targetIds(targetIds), m_targetAlias(targetAlias), m_isTargetOptional(isTargetOptional)
        {}

    //! Checks whether this object is equal to the supplied one.
    ECPRESENTATION_EXPORT bool IsEqual(RelatedClass const& other) const;
    bool operator==(RelatedClass const& other) const {return IsEqual(other);}
    bool operator!=(RelatedClass const& other) const {return !IsEqual(other);}
    ECPRESENTATION_EXPORT bool operator<(RelatedClass const& other) const;

    //! Is this structure valid.
    bool IsValid() const {return nullptr != m_source && m_target.IsValid();}

    //! The source class.
    void SetSourceClass(ECN::ECClassCR sourceClass) {m_source = &sourceClass;}
    ECN::ECClassCP GetSourceClass() const {return m_source;}
    
    //! The related class.
    void SetTargetClass(SelectClassWithExcludes targetClass) {m_target = targetClass;}
    SelectClassWithExcludes const& GetTargetClass() const {return m_target;}
    SelectClassWithExcludes& GetTargetClass() {return m_target;}

    //! Target ECInstance IDs. Used to join the target to source if relationship is not specified
    bset<ECInstanceId> const& GetTargetIds() const {return m_targetIds;}
    bset<ECInstanceId>& GetTargetIds() {return m_targetIds;}
    void SetTargetIds(bset<ECInstanceId> ids) {m_targetIds = ids;}

    //! Alias for the related class.
    void SetTargetClassAlias(Utf8String alias) {m_targetAlias = alias;}
    Utf8CP GetTargetClassAlias() const {return m_targetAlias.c_str();}

    //! Relationship used to access the related class.
    ECN::ECRelationshipClassCP GetRelationship() const {return m_relationship;}
    void SetRelationship(ECN::ECRelationshipClassCR relationship) {m_relationship = &relationship;}

    //! Alias for the relationship.
    void SetRelationshipAlias(Utf8String alias) {m_relationshipAlias = alias;}
    Utf8CP GetRelationshipAlias() const {return m_relationshipAlias.c_str();}

    //! Should the relationship be followed in a forward direction to access the related class.
    bool IsForwardRelationship() const {return m_isForwardRelationship;}
    void SetIsForwardRelationship(bool value) {m_isForwardRelationship = value;}

    //! Get the navigation property for this relationship.
    ECN::NavigationECPropertyCP GetNavigationProperty() const;

    //! Is related class queried using outer join
    bool IsTargetOptional() const {return m_isTargetOptional;}
    void SetIsTargetOptional(bool value) { m_isTargetOptional = value;}

    ECPRESENTATION_EXPORT static RelatedClass Unify(RelatedClass const&, RelatedClass const&);
};

//! A stack of ECClass and ECRelationshipClass pairs representing a path of relationships.
//! @ingroup GROUP_Presentation
struct RelatedClassPath : bvector<RelatedClass>
    {
    DEFINE_T_SUPER(bvector<RelatedClass>);
    RelatedClassPath() : T_Super() {}
    RelatedClassPath(std::initializer_list<RelatedClass> list) : T_Super(list) {}
    ECPRESENTATION_EXPORT RelatedClassPath& Reverse(Utf8CP resultTargetClassAlias, bool isResultTargetPolymorphic);
    ECPRESENTATION_EXPORT static RelatedClassPath Unify(RelatedClassPath const&, RelatedClassPath const&);
    };
typedef RelatedClassPath& RelatedClassPathR;
typedef RelatedClassPath const& RelatedClassPathCR;

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2016
+===============+===============+===============+===============+===============+======*/
struct IUsedUserSettingsListener : RefCountedBase
{
protected:
    virtual void _OnUserSettingUsed(Utf8CP settingId) = 0;
public:
    void OnUserSettingUsed(Utf8CP settingId) {_OnUserSettingUsed(settingId);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
+===============+===============+===============+===============+===============+======*/
struct ICancelationToken : RefCountedBase
{
protected:
    virtual bool _IsCanceled() const = 0;
public:
    bool IsCanceled() const {return _IsCanceled();}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                11/2017
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
// @bsiclass                                    Ainoras.Zukauskas                04/2020
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

END_BENTLEY_ECPRESENTATION_NAMESPACE
