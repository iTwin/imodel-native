/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/PresentationMetadataHelper.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "ECObjects.h"
#include <ECObjects/StandaloneECInstance.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//! @addtogroup ECObjectsGroup
//! @beginGroup

/*---------------------------------------------------------------------------------**//**
* Standard sorting priorities for properties.
* @bsistruct                                                    Paul.Connelly   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
enum class PropertySortPriority : int32_t
    {
    VeryHigh   = 400000,//!>Very High
    High       = 300000,//!>High
    Medium     = 200000,//!>Medium
    Low        = 100000,//!>Low
    VeryLow    = 0,     //!>Very Low
    };

/*---------------------------------------------------------------------------------**//**
* Standard sorting priorities for property categories.
* @bsistruct                                                    Paul.Connelly   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
enum class CategorySortPriority : int32_t
    {
    VeryHigh   = 400000,//!>Very High
    High       = 300000,//!>High
    Medium     = 200000,//!>Medium
    Low        = 100000,//!>Low
    VeryLow    = 0,     //!>Very Low
    };

//=======================================================================================
//! Helper object used to apply metadata to ECSchemas, ECClasses, and ECProperties used 
//! to control how they are presented in the UI.
//! Modifying a schema with this object will add a reference from that schema to the
//! standard EditorCustomAttributes schema which defines the custom attribute classes.
//! @bsiclass
//=======================================================================================
struct PresentationMetadataHelper
    {
private:
    ECSchemaPtr             m_customAttributesSchema;

/*__PUBLISH_SECTION_END__*/
    struct CustomAttributeData;

    IECInstancePtr          CreateInstance (WCharCP className) const;
    ECObjectsStatus         EnsureSchemaReference (IECCustomAttributeContainerR container) const;
    ECObjectsStatus         CreateCustomAttribute (IECCustomAttributeContainerR container, WCharCP className, CustomAttributeData const* data = NULL) const;
    ECObjectsStatus         CreateCustomAttribute (IECCustomAttributeContainerR container, WCharCP className, CustomAttributeData const& data) const;

public:
    ECOBJECTS_EXPORT PresentationMetadataHelper (ECSchemaR editorCustomAttributesSchema);
/*__PUBLISH_SECTION_START__*/
public:
    //! Creates an PresentationMetadataHelper object which can be used to apply metadata to ECSchema, ECClass, and ECProperty objects.
    //! @param schemaContext    An ECSchemaReadContext from which the standard EditorCustomAttributes schema can be obtained.
    ECOBJECTS_EXPORT PresentationMetadataHelper (ECSchemaReadContextR schemaContext);
    
    ECOBJECTS_EXPORT ~PresentationMetadataHelper();

    //! Applies a standard category to the property
    //! @param[in]      ecproperty         
    //! @param[in]      standardCategoryId 
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetStandardCategory (ECPropertyR ecproperty, int32_t standardCategoryId) const;

    //! Applies a custom category to a property
    //! @param[in]      ecproperty   
    //! @param[in]      uniqueName   
    //! @param[in]      displayLabel 
    //! @param[in]      priority     
    //! @param[in]      expand       
    //! @param[in]      description  
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetCustomCategory (ECPropertyR ecproperty, WCharCP uniqueName, WCharCP displayLabel, int32_t priority, bool expand = false, WCharCP description = NULL) const;

    // ###TODO: move standard types enum down from DgnPlatform?
    //! Sets the standard extended type for the property
    //! @param ecproperty       The property to which to apply the custom attribute
    //! @param standardTypeId   The ID of a standard extended type. Must be a valid DgnECExtendedType::StandardType.
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetExtendedType (ECPropertyR ecproperty, int32_t standardTypeId) const;

    //! Sets the extended type name for the property
    //! @param ecproperty       The property to which to apply the custom attribute
    //! @param extendTypeName   The name of the extended type.
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetExtendedType (ECPropertyR ecproperty, WCharCP extendTypeName) const;
    
    // ###TODO: move standard types enum down from DgnPlatform?
    //! Sets the standard extended type for members of the array property
    //! @param ecproperty       The property to which to apply the custom attribute
    //! @param standardTypeId   The ID of a standard extended type. Must be a valid DgnECExtendedType::StandardType.
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetMemberExtendedType (ArrayECPropertyR ecproperty, int32_t standardTypeId) const;

    //! Sets the extended type name for the members of the array property
    //! @param ecproperty       The property to which to apply the custom attribute
    //! @param extendTypeName   The name of the extended type.
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetMemberExtendedType (ArrayECPropertyR ecproperty, WCharCP extendTypeName) const;

    //! Sets the PropertyPriority for the property
    //! @param ecproperty   The property to which to apply the custom attribute
    //! @param priority     The desired priority
    //! @return ECOBJECTS_STATUS_Success if the priority was applied to the property
    ECOBJECTS_EXPORT ECObjectsStatus    SetPriority (ECPropertyR ecproperty, int32_t priority) const;

    //! Applies a custom attribute which prevents the Z coordinate of a Point3D property from being displayed in the UI
    //! @param ecproperty   The property to which to apply the custom attribute
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetIgnoreZ (ECPropertyR ecproperty) const;

    //! Applies a custom attribute to a struct property which causes the UI to display its properties as if they were part of the parent structure
    //! @param ecproperty   THe property to which to apply the custom attribute
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetMembersIndependent (ECPropertyR ecproperty) const;

    //! Applies a custom attribute to a Point3D, array, or struct property which causes the UI to to expand the property's members
    //! @param ecproperty       The property to which to apply the custom attribute. Must be a Point3D, array, or struct property.
    //! @param andArrayMembers  If true, and ecproperty is an array of structures, members of the structures are expanded as well. Ignored otherwise.
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetAlwaysExpand (ECPropertyR ecproperty, bool andArrayMembers = false) const;

    //! Applies a custom attribute indicating that when the value of the property is changed, the other properties should be refreshed in the UI
    ECOBJECTS_EXPORT ECObjectsStatus    SetRequiresReload (ECPropertyR ecproperty) const;

    //! Applies a custom attribute which prevents null property values from being displayed in the UI
    //! @param ecclass  The ECClass to which to apply the custom attribute
    //! @return ECOBJECTS_STATUS_Success if the custom attribute was applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetHideNullProperties (ECClassR ecclass) const;

    //! Applies a custom attribute to the schema which indicates properties of extended types such as Distance or Area are stored in UORs
    //! This is typically applied to schemas which define intrinsic properties of elements stored in dgn files
    //! If the attribute is not present on a schema, properties of these extended types will be assumed to be stored in meters unless a UnitSpecification attribute is applied
    ECOBJECTS_EXPORT ECObjectsStatus    SetStoresUnitsAsUORs (ECSchemaR schema) const;
    };

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

