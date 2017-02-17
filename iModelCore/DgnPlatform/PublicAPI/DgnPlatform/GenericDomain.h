/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GenericDomain.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDomain.h>

DGNPLATFORM_TYPEDEFS(GenericGroup)
DGNPLATFORM_TYPEDEFS(GenericGroupModel)
DGNPLATFORM_TYPEDEFS(GenericSpatialLocation)
DGNPLATFORM_TYPEDEFS(GenericPhysicalObject)

DGNPLATFORM_REF_COUNTED_PTR(GenericGroup)
DGNPLATFORM_REF_COUNTED_PTR(GenericGroupModel)
DGNPLATFORM_REF_COUNTED_PTR(GenericSpatialLocation)
DGNPLATFORM_REF_COUNTED_PTR(GenericPhysicalObject)

#define GENERIC_DOMAIN_ECSCHEMA_PATH        L"ECSchemas/Dgn/Generic.01.00.ecschema.xml"
#define GENERIC_DOMAIN_NAME                 "Generic"
#define GENERIC_SCHEMA(className)           GENERIC_DOMAIN_NAME "." className

#define GENERIC_CLASS_Graphic3d             "Graphic3d"
#define GENERIC_CLASS_PhysicalObject        "PhysicalObject"
#define GENERIC_CLASS_SpatialLocation       "SpatialLocation"
#define GENERIC_CLASS_Group                 "Group"
#define GENERIC_CLASS_GroupModel            "GroupModel"
#define GENERIC_CLASS_MultiAspect           "MultiAspect"
#define GENERIC_CLASS_Callout               "Callout"
#define GENERIC_CLASS_SectionCallout        "SectionCallout"
#define GENERIC_CLASS_ElevationCallout      "ElevationCallout"
#define GENERIC_CLASS_PlanCallout           "PlanCallout"
#define GENERIC_CLASS_DetailCallout         "DetailCallout"
#define GENERIC_CLASS_TitleText             "TitleText"
#define GENERIC_CLASS_DetailingSymbol       "DetailingSymbol"
#define GENERIC_CLASS_ViewAttachmentLabel   "ViewAttachmentLabel"

#define GENERIC_ViewAttachmentLabel_ViewAttachment "ViewAttachment"
#define GENERIC_Callout_DrawingModel        "DrawingModel"

#define GENERIC_REL_ViewAttachmentLabelAnnotatesViewAttachment "ViewAttachmentLabelAnnotatesViewAttachment"
#define GENERIC_REL_CalloutRefersToDrawingModel  "CalloutRefersToDrawingModel"

#define GENERIC_CODESPEC(name)                                  GENERIC_DOMAIN_NAME ":" name
#define GENERIC_CODESPEC_ViewAttachmentLabel GENERIC_CODESPEC(GENERIC_CLASS_ViewAttachmentLabel)

BEGIN_BENTLEY_DGN_NAMESPACE

namespace generic_ModelHandler {struct GenericGroupModelHandler;};
namespace generic_ElementHandler {struct GenericGroupHandler; struct GenericSpatialLocationHandler;};

//=======================================================================================
//! The Generic DgnDomain
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct GenericDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(GenericDomain, DGNPLATFORM_EXPORT)

protected:
    void _OnSchemaImported(DgnDbR) const override;

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
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASS_Graphic3d, GraphicalElement3d);

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
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASS_PhysicalObject, PhysicalElement);

public:
    explicit GenericPhysicalObject(CreateParams const& params) : T_Super(params) {}

    //! Create an instance of a GenericPhysicalObject from CreateParams.
    static GenericPhysicalObjectPtr Create(CreateParams const& params) {return new GenericPhysicalObject(params);}

    //! Create an instance of a GenericPhysicalObject
    //! @param[in] model The PhysicalModel for the new GenericPhysicalObject.
    //! @param[in] categoryId The category for the new GenericPhysicalObject.
    DGNPLATFORM_EXPORT static GenericPhysicalObjectPtr Create(PhysicalModelR model, DgnCategoryId categoryId);
};

//=======================================================================================
//! A generic SpatialLocation is used by a conversion process when:
//! - It did not have enough information to pick another domain
//! - It determined the element represents a SpatialLocation
// @bsiclass                                                    Shaun.Sewall    01/2016
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialLocation : SpatialLocationElement
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASS_SpatialLocation, SpatialLocationElement);
    friend struct generic_ElementHandler::GenericSpatialLocationHandler;

protected:
    explicit GenericSpatialLocation(CreateParams const& params) : T_Super(params) {} 

public:
    //! Create an instance of a GenericSpatialLocation
    //! @param[in] model The PhysicalModel for the new GenericPhysicalObject.
    //! @param[in] categoryId The category for the new GenericPhysicalObject.
    DGNPLATFORM_EXPORT static GenericSpatialLocationPtr Create(PhysicalModelR model, DgnCategoryId categoryId);
};

//=======================================================================================
//! A model which contains only GenericGroups.
//! @see GenericGroup
// @bsiclass                                                    Shaun.Sewall    05/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericGroupModel : GroupInformationModel
{
    DGNMODEL_DECLARE_MEMBERS(GENERIC_CLASS_GroupModel, GroupInformationModel);
    friend struct generic_ModelHandler::GenericGroupModelHandler;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsertElement(DgnElementR element) override;
    explicit GenericGroupModel(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static GenericGroupModelPtr Create(DgnElementCR modeledElement);
    DGNPLATFORM_EXPORT static GenericGroupModelPtr CreateAndInsert(DgnElementCR modeledElement);
};

//=======================================================================================
//! Groups elements using the ElementGroupsMembers relationship
//! @see GenericGroupModel
// @bsiclass                                                    Shaun.Sewall    12/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericGroup : GroupInformationElement, IElementGroupOf<DgnElement>
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASS_Group, GroupInformationElement)
    friend struct generic_ElementHandler::GenericGroupHandler;

protected:
    Dgn::IElementGroupCP _ToIElementGroup() const override final {return this;}
    Dgn::DgnElementCP _ToGroupElement() const override final {return this;}
    explicit GenericGroup(CreateParams const& params) : T_Super(params) {}

public:
    DGNPLATFORM_EXPORT static GenericGroupPtr Create(GenericGroupModelCR model, DgnCodeCR code = DgnCode());
};

//=======================================================================================
//! The namespace that only contains ModelHandlers for the GenericDomain
//! @private
//=======================================================================================
namespace generic_ModelHandler
{
    //! The ModelHandler for GroupModel
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericGroupModelHandler : dgn_ModelHandler::GroupInformation
    {
        MODELHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_GroupModel, GenericGroupModel, GenericGroupModelHandler, dgn_ModelHandler::GroupInformation, DGNPLATFORM_EXPORT)
    };
}

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
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_Graphic3d, GenericGraphic3d, GenericGraphic3dHandler, dgn_ElementHandler::Geometric3d, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GenericPhysicalObject
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericPhysicalObjectHandler : dgn_ElementHandler::Physical
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_PhysicalObject, GenericPhysicalObject, GenericPhysicalObjectHandler, dgn_ElementHandler::Physical, DGNPLATFORM_EXPORT)
    };

    //! The ElementHandler for GenericSpatialLocation
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericSpatialLocationHandler : dgn_ElementHandler::SpatialLocation
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_SpatialLocation, GenericSpatialLocation, GenericSpatialLocationHandler, dgn_ElementHandler::SpatialLocation, DGNPLATFORM_EXPORT)
    };
    
    //! The ElementHandler for GenericGroup
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericGroupHandler : dgn_ElementHandler::GroupInformation
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_Group, GenericGroup, GenericGroupHandler, dgn_ElementHandler::GroupInformation, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
