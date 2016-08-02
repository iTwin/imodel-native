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

DGNPLATFORM_TYPEDEFS(GenericSpatialGroup)
DGNPLATFORM_TYPEDEFS(GenericGraphicGroup2d)
DGNPLATFORM_TYPEDEFS(GenericSpatialLocation)
DGNPLATFORM_TYPEDEFS(GenericPhysicalObject)

DGNPLATFORM_REF_COUNTED_PTR(GenericSpatialGroup)
DGNPLATFORM_REF_COUNTED_PTR(GenericGraphicGroup2d)
DGNPLATFORM_REF_COUNTED_PTR(GenericSpatialLocation)
DGNPLATFORM_REF_COUNTED_PTR(GenericPhysicalObject)

#define GENERIC_DOMAIN_ECSCHEMA_PATH        L"ECSchemas/Dgn/Generic.01.00.ecschema.xml"
#define GENERIC_DOMAIN_NAME                 "Generic"
#define GENERIC_SCHEMA(className)           GENERIC_DOMAIN_NAME "." className

#define GENERIC_CLASSNAME_Graphic3d         "Graphic3d"
#define GENERIC_CLASSNAME_PhysicalObject    "PhysicalObject"
#define GENERIC_CLASSNAME_SpatialLocation   "SpatialLocation"
#define GENERIC_CLASSNAME_SpatialGroup      "SpatialGroup"
#define GENERIC_CLASSNAME_GraphicGroup2d    "GraphicGroup2d"
#define GENERIC_CLASSNAME_MultiAspect       "MultiAspect"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace generic_ElementHandler {struct GenericSpatialGroupHandler; struct GenericGraphicGroup2dHandler;};

//=======================================================================================
//! The Generic DgnDomain
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct GenericDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(GenericDomain, DGNPLATFORM_EXPORT)

public:
    GenericDomain();
    ~GenericDomain();
    
    //! Import the ECSchema for the GenericDomain into the specified DgnDb
    DGNPLATFORM_EXPORT static DgnDbStatus ImportSchema(DgnDbR);
};

//=======================================================================================
//! A generic GenericGraphic3d is used by a conversion process when:
//! - It did not have enough information to pick another domain
//! - It determined the element is 3d but does not represent a SpatialLocation
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericGraphic3d : GraphicalElement3d
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASSNAME_Graphic3d, GraphicalElement3d);

public:
    explicit GenericGraphic3d(CreateParams const& params) : T_Super(params) {} 
};

//=======================================================================================
//! A generic PhysicalObject is used by a conversion process when:
//! - It did not have enough information to pick another domain
//! - It determined the element represents a PhysicalObject
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericPhysicalObject : PhysicalElement
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASSNAME_PhysicalObject, PhysicalElement);

public:
    explicit GenericPhysicalObject(CreateParams const& params) : T_Super(params) {}

    //! Create an instance of a GenericPhysicalObject from CreateParams.
    static GenericPhysicalObjectPtr Create(CreateParams const& params) {return new GenericPhysicalObject(params);}

    //! Create an instance of a GenericPhysicalObject from a model and DgnCategoryId, using the default values for all other parameters.
    //! @param[in] model The SpatialModel for the new GenericPhysicalObject.
    //! @param[in] categoryId The category for the new GenericPhysicalObject.
    DGNPLATFORM_EXPORT static GenericPhysicalObjectPtr Create(SpatialModelR model, DgnCategoryId categoryId);
};

//=======================================================================================
//! A generic SpatialLocation is used by a conversion process when:
//! - It did not have enough information to pick another domain
//! - It determined the element represents a SpatialLocation
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialLocation : SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASSNAME_SpatialLocation, SpatialLocationElement);

public:
    explicit GenericSpatialLocation(CreateParams const& params) : T_Super(params) {} 
};

//=======================================================================================
//! A GroupInformationElement that groups SpatialElements using the ElementGroupsMembers relationship
// @bsiclass                                                    Shaun.Sewall    12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialGroup : GroupInformationElement, IElementGroupOf<GeometricElement3d>
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASSNAME_SpatialGroup, GroupInformationElement)
    friend struct generic_ElementHandler::GenericSpatialGroupHandler;

protected:
    Dgn::IElementGroupCP _ToIElementGroup() const override final {return this;}
    virtual Dgn::DgnElementCP _ToGroupElement() const override final {return this;}
    explicit GenericSpatialGroup(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static GenericSpatialGroupPtr Create(DgnDbR db, DgnCode const& code = DgnCode());
};

//=======================================================================================
//! A GroupInformationElement that groups GraphicalElement2ds using the ElementGroupsMembers relationship
// @bsiclass                                                   Carole.MacDonald            03/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericGraphicGroup2d : GroupInformationElement, IElementGroupOf<GraphicalElement2d>
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASSNAME_GraphicGroup2d, GroupInformationElement)
    friend struct generic_ElementHandler::GenericGraphicGroup2dHandler;

protected:
    Dgn::IElementGroupCP _ToIElementGroup() const override final { return this; }
    virtual Dgn::DgnElementCP _ToGroupElement() const override final { return this; }
    explicit GenericGraphicGroup2d(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static GenericGraphicGroup2dPtr Create(DgnDbR db, DgnCode const& code = DgnCode());
};

//=======================================================================================
//! The namespace that only contains ElementHandlers for the GenericDomain
//! @private
//=======================================================================================
namespace generic_ElementHandler
{
    //! The ElementHandler for GenericGraphic3d
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericGraphic3dHandler : dgn_ElementHandler::Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASSNAME_Graphic3d, GenericGraphic3d, GenericGraphic3dHandler, dgn_ElementHandler::Geometric3d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GenericPhysicalObject
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericPhysicalObjectHandler : dgn_ElementHandler::Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASSNAME_PhysicalObject, GenericPhysicalObject, GenericPhysicalObjectHandler, dgn_ElementHandler::Geometric3d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GenericSpatialLocation
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialLocationHandler : dgn_ElementHandler::Geometric3d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASSNAME_SpatialLocation, GenericSpatialLocation, GenericSpatialLocationHandler, dgn_ElementHandler::Geometric3d, DGNPLATFORM_EXPORT)
    };
    
    //! The ElementHandler for GenericSpatialGroup
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialGroupHandler : dgn_ElementHandler::Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASSNAME_SpatialGroup, GenericSpatialGroup, GenericSpatialGroupHandler, dgn_ElementHandler::Element, DGNPLATFORM_EXPORT)
    };
    
    //! The ElementHandler for GenericGraphicGroup2d
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericGraphicGroup2dHandler : dgn_ElementHandler::Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASSNAME_GraphicGroup2d, GenericGraphicGroup2d, GenericGraphicGroup2dHandler, dgn_ElementHandler::Element, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
