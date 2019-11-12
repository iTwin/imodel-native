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
    bool Equals(PageOptions const& other) {return m_pageStart == other.m_pageStart && m_pageSize == other.m_pageSize;}
};

//=======================================================================================
//! A structure that describes a related class and the properties of that relationship.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                07/2015
//=======================================================================================
struct RelatedClass
{
private:
    ECN::ECClassCP m_source;
    ECN::ECClassCP m_target;
    Utf8String m_targetAlias;
    ECN::ECRelationshipClassCP m_relationship;
    Utf8String m_relationshipAlias;
    bool m_isForwardRelationship;
    bool m_isPolymorphic;
    bool m_isOuterJoin;

public:
    //! Constructor. Creates an invalid instance.
    RelatedClass() : m_source(nullptr), m_target(nullptr), m_relationship(nullptr), m_isForwardRelationship(false), m_isPolymorphic(true), m_isOuterJoin(true) {}

    //! Constructor.
    RelatedClass(ECN::ECClassCR source, ECN::ECClassCR target, ECN::ECRelationshipClassCR relationship, bool isForward, Utf8CP targetAlias = nullptr, Utf8CP relationshipAlias = nullptr, bool isPolymorphic = true, bool isOuterJoin = true)
        : m_source(&source), m_target(&target), m_relationship(&relationship), m_isForwardRelationship(isForward), m_isPolymorphic(isPolymorphic), m_targetAlias(targetAlias), m_relationshipAlias(relationshipAlias), m_isOuterJoin(isOuterJoin)
        {}

    //! Checks whether this object is equal to the supplied one.
    ECPRESENTATION_EXPORT bool IsEqual(RelatedClass const& other) const;

    //! Checks whether this object is equal to the supplied one.
    bool operator==(RelatedClass const& other) const {return IsEqual(other);}

    //! Checks whether this object is not equal to the supplied one.
    bool operator!=(RelatedClass const& other) const {return !IsEqual(other);}

    //! Operator overload required for comparing @ref RelatedClass objects and using them in maps.
    ECPRESENTATION_EXPORT bool operator<(RelatedClass const& other) const;

    //! Is this structure valid.
    bool IsValid() const {return nullptr != m_source && nullptr != m_target && nullptr != m_relationship;}

    //! Set the source class.
    void SetSourceClass(ECN::ECClassCR sourceClass) {m_source = &sourceClass;}

    //! Get the source class.
    ECN::ECClassCP GetSourceClass() const {return m_source;}

    //! Set the related class.
    void SetTargetClass(ECN::ECClassCR targetClass) {m_target = &targetClass;}

    //! Get the related class.
    ECN::ECClassCP GetTargetClass() const {return m_target;}

    //! Set the alias for the related class.
    void SetTargetClassAlias(Utf8String alias) {m_targetAlias = alias;}

    //! Get the alias of the related class.
    Utf8CP GetTargetClassAlias() const {return m_targetAlias.c_str();}

    //! Get the relationship used to access the related class.
    ECN::ECRelationshipClassCP GetRelationship() const {return m_relationship;}

    //! Set the relationship used to access the related class.
    void SetRelationship(ECN::ECRelationshipClassCR relationship) {m_relationship = &relationship;}

    //! Set the alias for the relationship.
    void SetRelationshipAlias(Utf8String alias) {m_relationshipAlias = alias;}

    //! Get the alias of the relationship.
    Utf8CP GetRelationshipAlias() const {return m_relationshipAlias.c_str();}

    //! Should the relationship be followed in a forward direction to access the related class.
    bool IsForwardRelationship() const {return m_isForwardRelationship;}

    //! Set whether the relationship should be followed in a forward direction to access the related class.
    void SetIsForwardRelationship(bool value) {m_isForwardRelationship = value;}

    //! Is the related class is polymorphic.
    bool IsPolymorphic() const {return m_isPolymorphic;}

    //! Set whether the related class is polymorphic.
    void SetIsPolymorphic(bool value) {m_isPolymorphic = value;}

    //! Get the navigation property for this relationship.
    ECN::NavigationECPropertyCP GetNavigationProperty() const;

    //! Is related class queried using outer join
    bool IsOuterJoin() const {return m_isOuterJoin;}

    //! Set whether the related class should be queried using outer join
    void SetIsOuterJoin(bool value) {m_isOuterJoin = value;}
};

//! A stack of ECClass and ECRelationshipClass pairs representing a path of relationships.
//! @ingroup GROUP_Presentation
struct RelatedClassPath : bvector<RelatedClass>
    {
    DEFINE_T_SUPER(bvector<RelatedClass>);
    RelatedClassPath() : T_Super() {}
    RelatedClassPath(std::initializer_list<RelatedClass> list) : T_Super(list) {}
    void Reverse(Utf8CP firstTargetClassAlias, bool isFirstTargetPolymorphic);
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

END_BENTLEY_ECPRESENTATION_NAMESPACE
