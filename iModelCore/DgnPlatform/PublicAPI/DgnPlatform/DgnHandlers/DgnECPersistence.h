/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnECPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDbApi.h>

DGNPLATFORM_REF_COUNTED_PTR (DgnECPropertyFormatter);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* Utility to work with "standard" ec categories. 
* @see JsonECSqlSelectAdapter
* @remarks
  A lot of this logic has been carried forward from ECPropertyPane.cs for the Graphite property 
  panes to be backward compatible with 8.11.9. However, Graphite does NOT support multiple 
  categories that 8.11.9 seems to allow. The category system will be revamped in the future - 
  (TFS 63407)
* @bsiclass                                                 Ramanujam.Raman      08/2012
+===============+===============+===============+===============+===============+======*/
struct ECStandardCategoryHelper : NonCopyableClass
{
private:
    typedef bmap<int, ECN::IECInstancePtr> CategoriesByStandard;
    CategoriesByStandard m_categoriesByStandard;
    ECN::ECClassP m_categoryClass;

public:

    enum StandardCategory
        {
        Miscellaneous       =   0,
        General             =   1,
        Extended            =   1 << 1,
        RawData             =   1 << 2,
        Geometry            =   1 << 3,
        Groups              =   1 << 4,
        Material            =   1 << 5,
        Relationships       =   1 << 6
        };

//__PUBLISH_SECTION_END__

// TODO: Move this to DgnECPersistence.cpp as soon as DgnECJsonAdapter is deprecated
// TODO: Eventually deprecate this by moving the functionality to a lower level. 
private:
    static const int CategorySortPriorityVeryHigh  = 400000;
    static const int CategorySortPriorityHigh      = 300000;
    static const int CategorySortPriorityMedium    = 200000;
    static const int CategorySortPriorityLow       = 100000;
    static const int CategorySortPriorityVeryLow   = 0;

    static WString GetDisplayLabel (StandardCategory standard);
    static int GetPriority (StandardCategory standard);
    static bool GetDefaultExpand (StandardCategory standard);
    static WCharCP GetName (StandardCategory standard);
    static void SetValue (ECN::IECInstanceR instance, WCharCP name, ECN::ECValueCR ecValue);
    static void SetStringValue (ECN::IECInstanceR instance, WCharCP name, WCharCP val);
    static void SetBooleanValue (ECN::IECInstanceR instance, WCharCP name, bool val);
    static void SetIntegerValue (ECN::IECInstanceR instance, WCharCP name, int val);

//__PUBLISH_SECTION_START__

public:
    ECStandardCategoryHelper () : m_categoryClass (NULL) {}

    void Initialize (BeSQLite::EC::ECDbR ecDb);
    ECN::IECInstancePtr GetCategory (StandardCategory standard);
    
};

/*=================================================================================**//**
* Used to format the property values returned by the ECDb API. 
* @see JsonECSqlSelectAdapter
* @bsiclass                                                 Ramanujam.Raman      10/2013
+===============+===============+===============+===============+===============+======*/
struct DgnECPropertyFormatter : BeSQLite::EC::ECPropertyFormatter
{
//__PUBLISH_SECTION_END__
private:
    DgnModelP m_dgnModel;
    ECStandardCategoryHelper m_standardCategoryHelper;
    DgnECPropertyFormatter (DgnModelP dgnModel);
    virtual ~DgnECPropertyFormatter() {}

private:
    virtual bool _FormattedStringFromECValue 
        (
        Utf8StringR strVal, 
        ECN::ECValueCR ecValue, 
        ECN::ECPropertyCR ecProperty, 
        bool isArrayMember
        ) const override;

     virtual ECN::IECInstancePtr _GetPropertyCategory (ECN::ECPropertyCR ecProperty) override;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

public:
    DGNPLATFORM_EXPORT static DgnECPropertyFormatterPtr Create (DgnModelP dgnModel);
};

/*=================================================================================**//**
* Used to perform Create, Read, Update and Delete of associations between DgnElements and 
* ECInstances. 
* @nosubgrouping
* @bsiclass                                                 Ramanujam.Raman      05/2013
+===============+===============+===============+===============+===============+======*/
struct DgnECPersistence
{
private:
    DgnDbR m_dgndb;
    BeSQLite::EC::ECInstanceFinder* m_ecInstanceFinder;

//__PUBLISH_SECTION_END__

    static bool IsGraphicalElement (const ElementId& elementId, DgnDbCR project);
    BeSQLite::EC::ECInstanceFinder* GetECInstanceFinder();
    static ECN::ECClassId GetElementClassId (DgnDbCR project);
    static bool IsElementIdUsed (ElementId elementId, DgnDbCR project);
    static DgnModelId GetModelIdFromElementId (const ElementId& elementId, DgnDbR project);

public:
    //! Creates the tables/views necessary to store/query Dgn-EC associations
    DGNPLATFORM_EXPORT static void CreateTables (DgnDbR project);

    //! Upgrades the tables/views necessary to store/query Dgn-EC associations. 
    DGNPLATFORM_EXPORT static StatusInt UpgradeTables (DgnDbR project);

//__PUBLISH_SECTION_START__

private:
    DgnECPersistence (DgnECPersistence const& other) : m_dgndb (other.m_dgndb) {}
    DgnECPersistence& operator= (DgnECPersistence const&); // Purposefully not defined to prevent calling.

public:
    /*=================================================================================**//**
    * Describes the association between a DgnElement and an ECInstance. 
    * @bsiclass                                                 Ramanujam.Raman      05/2013
    +===============+===============+===============+===============+===============+======*/
    enum AssociationType
        {
        //! Enables retrieval of <i>the</i> core instance associated with an element
        //! The Core instance contain information like level and model, typically associated with
        //! graphical data, but may have some business significance. The instance is not shared with other elements. 
        AssociationType_Core = 0x01,

        //! Enables retrieval of <i>the</i> primary instance associated with an element. 
        //! The instance contains business properties that provide some engineering identity 
        //! to the element. The instance is not shared with other elements. 
        AssociationType_Primary = 0x01 << 1,

        //! Enables retrieval of secondary instances associated with the element. 
        //! The instances typically contain business properties that are relevant for the given 
        //! element. Examples include schedule data, URL links, embedded files, etc.
        //! The instances can be shared with other elements. 
        AssociationType_Secondary = 0x01 << 2, 

        // Enables retrieval of primary and secondary instances
        AssociationType_PrimaryOrSecondary = AssociationType_Primary | AssociationType_Secondary,

        // Enables retrieval of all associated instances
        AssociationType_All = AssociationType_Core | AssociationType_Primary | AssociationType_Secondary
        };

/** @name Setup */
/** @{ */
    //! Creates a new instance of DgnECPersistence needed for some persistence operations
    //! @remarks Holds some cached state to speed up future queries, including prepared statements to traverse
    //! relationships. 
    //! @see Finalize
    DGNPLATFORM_EXPORT DgnECPersistence (DgnDbR project);

    DGNPLATFORM_EXPORT ~DgnECPersistence();

    //! Finalizes all the held prepared statements. 
    //! Note that finalization needs to be done before the Db is closed. Destroying the DgnECPersistence automatically 
    //! causes finalization. 
    DGNPLATFORM_EXPORT void Finalize();
/** @} */

/** @name Schemas */
/** @{ */
    //! Imports the dgn schema into the project, or if there is one already optionally upgrades it.
    DGNPLATFORM_EXPORT static StatusInt ImportDgnSchema (DgnDbR project, bool updateExisting);
/** @} */

/** @name Create */
/** @{ */
    //! Sets up a "primary" ECInstance on an Element
    //! @param[in] editElementHandle EditElementHandle
    //! @param[in] instanceKey Key identifying the ECInstance
    //! @param[in] project DgnDb 
    //! @return SUCCESS/ERROR 
    //! @remarks The EditElementHandle needs to be committed before the change is
    //! reflected in the DgnDb. 
    DGNPLATFORM_EXPORT static StatusInt SetPrimaryInstanceOnElement 
        (
        EditElementHandleR editElementHandle, 
        BeSQLite::EC::ECInstanceKeyCR instanceKey,
        DgnDbR project
        );

    //! Adds a "secondary" ECInstance on an Element
    //! @param[in] elementId ElementId
    //! @param[in] instanceKey Key identifying the ECInstance
    //! @param[in] project DgnDb 
    //! @return SUCCESS/ERROR 
    //! @remarks Secondary instances can only be specified on an element that's already been persisted in the DgnDb. 
    DGNPLATFORM_EXPORT static StatusInt AddSecondaryInstanceOnElement 
        (
        const ElementId& elementId, 
        BeSQLite::EC::ECInstanceKeyCR instanceKey,
        DgnDbR project
        );
/** @} */

/** @name Read (ECInstance from Element) */
/** @{ */
    //! Determines if the element has a primary ECInstance associated with it. 
    //! @param[in] elementHandle ElementHandle
    //! @param[in] instanceKey Optional key to search for a specific instance. Pass NULL to check for any instance association. 
    DGNPLATFORM_EXPORT static bool ElementHasPrimaryInstance (ElementHandleCR elementHandle, BeSQLite::EC::ECInstanceKeyCP instanceKey = NULL);

    //! Determines if the element has a primary ECInstance associated with it. 
    //! @param[in] elementId ElementId
    //! @param[in] project DgnDb
    //! @param[in] instanceKey Optional key to search for a specific instance. Pass NULL to check for any instance association. 
    DGNPLATFORM_EXPORT static bool ElementHasPrimaryInstance 
        (
        const ElementId& elementId,
        DgnDbCR project,
        BeSQLite::EC::ECInstanceKeyCP instanceKey = NULL
        );

    //! Determines if the element has a secondary ECInstance associated with it. 
    //! @param[in] elementId ElementId
    //! @param[in] project DgnDb
    //! @param[in] instanceKey Optional key to search for a specific instance. Pass NULL to check for any associated instance
    //! @remarks Secondary instances can only be associated with elements that have been persisted in the DgnDb. 
    DGNPLATFORM_EXPORT static bool ElementHasSecondaryInstance 
        (
        const ElementId& elementId, 
        DgnDbCR project,
        BeSQLite::EC::ECInstanceKeyCP instanceKey = NULL
        );
        
    //! Gets the primary ECInstance associated with an Element
    //! @param[out] instanceKey Key of the primary ECInstance associated with the specified Element
    //! @param[in] elementId ElementId
    //! @param[in] project DgnDb
    //! @return true if a primary instance was found; false otherwise
    DGNPLATFORM_EXPORT static bool GetPrimaryInstanceOnElement 
        (
        BeSQLite::EC::ECInstanceKeyR instanceKey, 
        const ElementId& elementId, 
        DgnDbCR project
        );

    //! Gets the core ECInstances associated with an Element
    //! @param[out] instanceKey Key of the graphical ECInstance associated with the specified Element
    //! @param[in] elementId ElementId
    //! @param[in] project DgnDb
    //! @return true if the element exists and therefore has a graphical instance; false otherwise
    //! @remarks To be modified to return multiple instances - level, model, etc. 
    DGNPLATFORM_EXPORT static bool GetCoreInstanceOnElement 
        (
        BeSQLite::EC::ECInstanceKeyR instanceKey, 
        const ElementId& elementId, 
        DgnDbCR project
        );

    //! Gets the primary ECInstance associated with an Element
    //! @param[out] instanceKey Key of the primary ECInstance associated with the specified Element
    //! @param[in] elementHandle ElementHandle
    //! @return true if a primary instance was found; false otherwise
    DGNPLATFORM_EXPORT static bool GetPrimaryInstanceOnElement 
        (
        BeSQLite::EC::ECInstanceKeyR instanceKey, 
        ElementHandleCR elementHandle
        );

    //! Gets the ECInstance-s associated with an Element
    //! @param[out] instanceKeys Keys of all ECInstance-s associated with the specified Element
    //! @param[in] elementId Id of Element
    //! @param[in] project DgnDb
    //! @param[in] association The association(s) between elements and instances that need to be followed.
    //! @return Number of new instances found
    //! @remarks The set of instanceKeys passed in are first cleared before adding the newly found entries.
    DGNPLATFORM_EXPORT static size_t GetInstancesOnElement 
        (
        bvector<BeSQLite::EC::ECInstanceKey>& instanceKeys, 
        const ElementId& elementId, 
        DgnDbCR project,
        DgnECPersistence::AssociationType association = DgnECPersistence::AssociationType_All
        );
/** @} */

/** @name Read (Element from ECInstance) */
/** @{ */
    //! Gets the Element associated to the specified primary ECInstance. 
    //! @param[out] elementId Id of element that's associated with the specified primary ECInstance
    //! @param[in] instanceKey Key of the ECInstance
    //! @param[in] project DgnDb
    //! @return true if an Element was found; false otherwise
    DGNPLATFORM_EXPORT static bool GetElementWithPrimaryInstance 
        (
        ElementId& elementId, 
        const BeSQLite::EC::ECInstanceKey& instanceKey, 
        DgnDbCR project
        );

    //! Gets the Element-s associated to an ECInstance
    //! @param[out] elementIds Ids of all elements associated with the specified ECInstance
    //! @param[in] instanceKey Key of the ECInstance
    //! @param[in] project DgnDb
    //! @param[in] association The association(s) between elements and instances that needs to be followed.
    //! @param[in] inludeOnlyGraphicalElements Pass true to only include elements that have graphics directly 
    //!            associated with them. Otherwise defaults to get all associated elements. 
    //! @return Number of new elements found
    //! @remarks The set of elementIds passed in are first cleared before adding the newly found entries.
    DGNPLATFORM_EXPORT static size_t GetElementsWithInstance 
        (
        bset<ElementId>& elementIds, 
        const BeSQLite::EC::ECInstanceKey& instanceKey, 
        DgnDbCR project,
        DgnECPersistence::AssociationType association = DgnECPersistence::AssociationType_All,
        bool inludeOnlyGraphicalElements = false
        );


    //! Recursively gathers elements associated with instances starting with the supplied seed instances and 
    //! navigating to other related instances.
    //! @param[out] elementIds Set of element ids
    //! @param[in] seedInstanceKeyMap Seed instances
    //! @param[in] instanceFindOptions Options to find ECInstances from the statement. 
    //! @param[in] association The association(s) between elements and instances that needs to be followed.
    //! @param[in] inludeOnlyGraphicalElements Pass true to only include elements that have graphics directly 
    //!            associated with them. Otherwise defaults to get all associated elements. 
    //! @remarks Some state (e.g., relevant relationships on a class) is cached in the containing class to avoid
    //! repeated lookups later. 
    //! The set of elementIds passed in are first cleared before adding the newly found entries.
    DGNPLATFORM_EXPORT BentleyStatus FindElements 
        (
        bset<ElementId>& elementIds, 
        const BeSQLite::EC::ECInstanceKeyMultiMap& seedInstanceKeyMap, 
        BeSQLite::EC::ECInstanceFinder::FindOptions instanceFindOptions =  BeSQLite::EC::ECInstanceFinder::FindOptions(), 
        DgnECPersistence::AssociationType association = DgnECPersistence::AssociationType_All,
        bool inludeOnlyGraphicalElements = false
        );
        
    //! Starting with the specified element, walk up the assembly hierarchy to get an element that has a 
    //! primary instance associated with it. 
    //! @param [out] outElementId Id of the element found. 
    //! @param [in] inElementId Id of the element to start the search. 
    //! @param [in] project DgnDb
    //! @return true if an element with a primary instance. false otherwise. 
    DGNPLATFORM_EXPORT static bool TryGetAssemblyElementWithPrimaryInstance 
        (
        ElementId& outElementId, 
        const ElementId& inElementId, 
        DgnDbR project
        );

    //! Helper utility to get all the EC information associated with an element
    //! @param [out] jsonInstances  Information on the element in the JSON format.
    //! @param [out] jsonDisplayInfo to properly display the element information in the JSON format.
    //! @param [in] elementId Id of Element to get information on. 
    //! @param [in] project DgnDb
    //! @remarks Consolidates all the information in the various instances associated with the element.
    //! Use @ref TryGetAssemblyElementWithPrimaryInstance() to walk up the assembly hierarchy to get an element that 
    //! has an associated primary instance. 
    DGNPLATFORM_EXPORT static StatusInt GetElementInfo 
        (
        JsonValueR jsonInstances, 
        JsonValueR jsonDisplayInfo, 
        const ElementId& elementId, 
        DgnDbR project
        );
/** @} */

/** @name Delete */
/** @{ */
    //! Removes the specified primary ECInstance from an Element. 
    //! @param[in] editElementHandle EditElementHandle
    //! @param[in] instanceKey Key identifying the ECInstance
    //! @return ERROR if instance is not found, or there are problems removing it. SUCCESS otherwise. 
    //! @remarks The EditElementHandle needs to be committed before the change is
    //! reflected in the DgnDb. 
    DGNPLATFORM_EXPORT static StatusInt RemovePrimaryInstanceOnElement 
        (
        EditElementHandleR editElementHandle, 
        const BeSQLite::EC::ECInstanceKey& instanceKey
        );

    //! Removes the specified secondary ECInstance from an Element. 
    //! @param[in] elementId ElementId
    //! @param[in] instanceKey Key identifying the ECInstance
    //! @param[in] project DgnDb
    //! @return ERROR if instance is not found, or there are problems removing it. SUCCESS otherwise. 
    DGNPLATFORM_EXPORT static StatusInt RemoveSecondaryInstanceOnElement
        (
        const ElementId& elementId, 
        BeSQLite::EC::ECInstanceKeyCR instanceKey,
        DgnDbR project
        );

    //! Clears the primary ECInstance on an Element
    //! @param[in] editElementHandle EditElementHandle
    //! @return Always SUCCESS
    //! @remarks The EditElementHandle needs to be committed before the change is
    //! reflected in the DgnDb. 
    DGNPLATFORM_EXPORT static StatusInt ClearPrimaryInstanceOnElement (EditElementHandleR editElementHandle);

    //! Clears all secondary ECInstance-s on an Element
    //! @param[in] elementId ElementId
    //! @param[in] project DgnDb
    //! @return SUCCESS/ERROR 
    DGNPLATFORM_EXPORT static StatusInt ClearSecondaryInstancesOnElement (const ElementId& elementId, DgnDbR project);
/** @} */
};
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE
