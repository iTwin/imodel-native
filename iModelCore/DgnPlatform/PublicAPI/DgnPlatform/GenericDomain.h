/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GenericDomain.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define GENERIC_DOMAIN_ECSCHEMA_PATH L"ECSchemas/Dgn/Generic.01.00.ecschema.xml"
#define GENERIC_DOMAIN_NAME "Generic"
#define GENERIC_CLASSNAME_PhysicalObject "PhysicalObject"
#define GENERIC_CLASSNAME_SpatialLocation "SpatialLocation"
#define GENERIC_SCHEMA(className) GENERIC_DOMAIN_NAME "." className

//=======================================================================================
//! The Generic DgnDomain
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct GenericDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(GenericDomain, DGNPLATFORM_EXPORT)

public:
    GenericDomain();
    ~GenericDomain();
    
    //! Import the ECSchema for the GenericDomain into the specified DgnDb
    DGNPLATFORM_EXPORT static DgnDbStatus ImportSchema(DgnDbR, ImportSchemaOptions);
};

//=======================================================================================
//! A GenericPhysicalObject is used when the conversion process did not have enough information to choose anything better
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericPhysicalObject : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASSNAME_PhysicalObject, PhysicalElement);

public:
    explicit GenericPhysicalObject(CreateParams const& params) : T_Super(params) {} 
};

//=======================================================================================
//! A GenericSpatialLocation is used when the conversion process did not have enough information to choose anything better
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialLocation : SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASSNAME_SpatialLocation, SpatialLocationElement);

public:
    explicit GenericSpatialLocation(CreateParams const& params) : T_Super(params) {} 
};

//=======================================================================================
//! The namespace that only contains ElementHandlers for the GenericDomain
//! @private
//=======================================================================================
namespace generic_ElementHandler
{
    //! The ElementHandler for GenericPhysicalObject
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericPhysicalObjectHandler : dgn_ElementHandler::Physical
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASSNAME_PhysicalObject, GenericPhysicalObject, GenericPhysicalObjectHandler, dgn_ElementHandler::Physical, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GenericSpatialLocation
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialLocationHandler : dgn_ElementHandler::Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASSNAME_SpatialLocation, GenericSpatialLocation, GenericSpatialLocationHandler, dgn_ElementHandler::Element, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
