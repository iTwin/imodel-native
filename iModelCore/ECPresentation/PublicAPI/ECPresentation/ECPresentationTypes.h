/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/ECPresentationTypes.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>

ECPRESENTATION_TYPEDEFS(IConnectionCache)
ECPRESENTATION_TYPEDEFS(IConnectionManager)
ECPRESENTATION_TYPEDEFS(IECPresentationManager)
ECPRESENTATION_TYPEDEFS(IAUIDisplayItem)
ECPRESENTATION_TYPEDEFS(NavNodeKey)
ECPRESENTATION_TYPEDEFS(NavNode)
ECPRESENTATION_TYPEDEFS(INavNodeLocater)
ECPRESENTATION_TYPEDEFS(Content)
ECPRESENTATION_TYPEDEFS(ContentSetItem)
ECPRESENTATION_TYPEDEFS(ContentDescriptor)
ECPRESENTATION_TYPEDEFS(IPropertyCategorySupplier)
ECPRESENTATION_TYPEDEFS(PageOptions)
ECPRESENTATION_TYPEDEFS(RelatedClass)
ECPRESENTATION_REFCOUNTED_PTR(NavNodeKey)
ECPRESENTATION_REFCOUNTED_PTR(NavNode)
ECPRESENTATION_REFCOUNTED_PTR(Content)
ECPRESENTATION_REFCOUNTED_PTR(ISerializable)
ECPRESENTATION_REFCOUNTED_PTR(ContentSetItem)
ECPRESENTATION_REFCOUNTED_PTR(ContentDescriptor)

ECPRESENTATION_TYPEDEFS(ISelectionProvider)
ECPRESENTATION_TYPEDEFS(SelectionManager)
ECPRESENTATION_TYPEDEFS(SelectionChangedEvent)
ECPRESENTATION_TYPEDEFS(SelectionSyncHandler)
ECPRESENTATION_REFCOUNTED_PTR(SelectionSyncHandler)

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

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

public:
    //! Constructor. Creates an invalid instance.
    RelatedClass() : m_source(nullptr), m_target(nullptr), m_relationship(nullptr), m_isForwardRelationship(false), m_isPolymorphic(true) {}

    //! Constructor.
    RelatedClass(ECN::ECClassCR source, ECN::ECClassCR target, ECN::ECRelationshipClassCR relationship, bool isForward, Utf8CP targetAlias = nullptr, Utf8CP relationshipAlias = nullptr, bool isPolymorphic = true)
        : m_source(&source), m_target(&target), m_relationship(&relationship), m_isForwardRelationship(isForward), m_isPolymorphic(isPolymorphic), m_targetAlias(targetAlias), m_relationshipAlias(relationshipAlias)
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
struct IUsedUserSettingsListener
    {
    virtual ~IUsedUserSettingsListener() {}
    virtual void _OnUserSettingUsed(Utf8CP settingId) = 0;
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
