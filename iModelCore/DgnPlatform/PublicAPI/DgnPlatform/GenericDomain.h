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
DGNPLATFORM_TYPEDEFS(GenericCallout)
DGNPLATFORM_TYPEDEFS(GenericDetailingSymbol)
DGNPLATFORM_TYPEDEFS(GenericViewAttachmentLabel)

DGNPLATFORM_REF_COUNTED_PTR(GenericGroup)
DGNPLATFORM_REF_COUNTED_PTR(GenericGroupModel)
DGNPLATFORM_REF_COUNTED_PTR(GenericSpatialLocation)
DGNPLATFORM_REF_COUNTED_PTR(GenericPhysicalObject)
DGNPLATFORM_REF_COUNTED_PTR(GenericCallout)
DGNPLATFORM_REF_COUNTED_PTR(GenericDetailingSymbol)
DGNPLATFORM_REF_COUNTED_PTR(GenericViewAttachmentLabel)

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
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericDetailingSymbol : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASS_DetailingSymbol, GraphicalElement2d);
public:
    explicit GenericDetailingSymbol(CreateParams const& params) : T_Super(params) {} 
};

//=======================================================================================
//! Specifies that more information about the graphics on a sheet may be or should be 
//! found on another sheet.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericCallout : GenericDetailingSymbol
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASS_Callout, GenericDetailingSymbol);
public:
    explicit GenericCallout(CreateParams const& params) : T_Super(params) {} 

    //! Get the drawing model that fulfills this callout
    DgnModelId GetDrawingModel() const {return GetPropertyValueId<DgnModelId>(GENERIC_Callout_DrawingModel);}
    //! Set the drawing model that fulfills this callout
    DgnDbStatus SetDrawingModel(DgnModelId v) {return SetPropertyValue(GENERIC_Callout_DrawingModel, v);}

    //! Look up the Sheet::ViewAttachment on the sheet that shows the drawing that is referenced by this callout. This is the reverse of GenericViewAttachmentLabel::FindCallout.
    DGNPLATFORM_EXPORT DgnElementId FindViewAttachment() const;
};

//=======================================================================================
//! Identifies a ViewAttachment on a sheet as a labelled callout drawing.
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericViewAttachmentLabel : GenericDetailingSymbol
{
    DGNELEMENT_DECLARE_MEMBERS(GENERIC_CLASS_ViewAttachmentLabel, GenericDetailingSymbol);
public:
    explicit GenericViewAttachmentLabel(CreateParams const& params) : T_Super(params) {} 

    //! Get the ViewAttachment that is annotated by this label
    DgnElementId GetViewAttachment() const {return GetPropertyValueId<DgnElementId>(GENERIC_ViewAttachmentLabel_ViewAttachment);}

    //! Convenience method to look up the ViewAttachment element
    Sheet::ViewAttachmentCPtr GetViewAttachmentElement() const {return GetDgnDb().Elements().Get<Sheet::ViewAttachment>(GetViewAttachment());}

    //! Set the ViewAttachment that is annotated by this label
    DgnDbStatus SetViewAttachment(DgnElementId v) {return SetPropertyValue(GENERIC_ViewAttachmentLabel_ViewAttachment, v);}

    //! Look up the ViewAttachmentLabel that labels the specified ViewAttachment. This is the reverse of GetViewAttachment.
    DGNPLATFORM_EXPORT static DgnElementId FindFromViewAttachment(DgnDbR, DgnElementId viewAttachmentId);

    //! Look up the ViewAttachmentLabel that labels the specified ViewAttachment. This is the reverse of GetViewAttachment.
    static DgnElementId FindFromViewAttachment(Sheet::ViewAttachmentCR va) {return FindFromViewAttachment(va.GetDgnDb(), va.GetElementId());}

    //! Look up the Callout that is fulfilled by the DrawingModel that is the target of this ViewAttachment. This is the reverse of GenericCallout::FindViewAttachment.
    DgnElementId FindCallout() const {auto va = GetViewAttachmentElement(); return va.IsValid()? FindCalloutFor(*va): DgnElementId();}

    //! Look up the Callout that is fulfilled by the DrawingModel that is the target of the specified ViewAttachment. This is the reverse of GenericCallout::FindViewAttachment.
    DGNPLATFORM_EXPORT static DgnElementId FindCalloutFor(Sheet::ViewAttachmentCR viewAttachment);
};

//=======================================================================================
//! Information about where the called out drawing is displayed. 
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericCalloutDestination
    {
    private:
    GenericViewAttachmentLabelCPtr m_viewAttachmentLabel;
    DrawingModelCPtr m_drawingModel;
    Sheet::ViewAttachmentCPtr m_viewAttachment;
    SheetViewDefinitionCPtr m_sheetView;

    public:
    //! Find the drawing that is referenced by the callout, and locate the exact area of the sheet that it displays the drawing and the view of the sheet.
    //! This is the reverse of GenericCalloutLocation::FindCalloutFor.
    DGNPLATFORM_EXPORT static GenericCalloutDestination FindDestinationOf(GenericCalloutCR callout);

    //! The drawing that is referenced by the callout
    DrawingModelCPtr GetDrawingModel() const {return m_drawingModel;}
    //! The label for the drawing
    GenericViewAttachmentLabelCP GetViewAttachmentLabel() const {return m_viewAttachmentLabel.get();}
    //! The area of the sheet that dislays the drawing. Note that the model that contains the returned ViewAttachment 
    //! is the Sheet::Model to which the view of the drawing it attached.
    Sheet::ViewAttachmentCP GetViewAttachment() const {return m_viewAttachment.get();} 
    //! The view of the sheet that displays the drawing
    SheetViewDefinitionCP GetSheetView() const {return m_sheetView.get();} 
    };

//=======================================================================================
//! Information about where a callout is located and displayed. 
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GenericCalloutLocation
    {
    private:
    GenericCalloutCPtr m_callout;
    SheetViewDefinitionCPtr m_sheetView;

    public:
    //! Find the callout that is related to the specified viewAttachment, and locate the first view of that sheet that contains the callout.
    //! This is the reverse of GenericCalloutDestination::FindDestinationOf.
    DGNPLATFORM_EXPORT static GenericCalloutLocation FindCalloutFor(Sheet::ViewAttachmentCR viewAttachment);

    //! Find the callout that is related to the specified viewAttachment, and locate the first view of that sheet that contains the callout.
    //! This is the reverse of GenericCalloutDestination::FindDestinationOf.
    static GenericCalloutLocation FindCalloutFor(GenericViewAttachmentLabelCR label) {auto va = label.GetViewAttachmentElement(); return va.IsValid()? FindCalloutFor(*va): GenericCalloutLocation();}

    //! The callout. The sheet that contains the callout is the model of the returned element.
    GenericCalloutCP GetCallout() const {return m_callout.get();}
    //! The view of the sheet that contains the callout
    SheetViewDefinitionCP GetSheetView() const {return m_sheetView.get();} 
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
    DGNPLATFORM_EXPORT static GenericGroupPtr Create(GenericGroupModelR model, DgnCodeCR code = DgnCode());
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
    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericDetailingSymbolHandler : dgn_ElementHandler::Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_DetailingSymbol, GenericDetailingSymbol, GenericDetailingSymbolHandler, dgn_ElementHandler::Geometric2d, DGNPLATFORM_EXPORT)
    };

    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericCalloutHandler : GenericDetailingSymbolHandler
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_Callout, GenericCallout, GenericCalloutHandler, GenericDetailingSymbolHandler, DGNPLATFORM_EXPORT)
    };

    //! @private
    struct EXPORT_VTABLE_ATTRIBUTE GenericViewAttachmentLabelHandler : GenericDetailingSymbolHandler
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(GENERIC_CLASS_ViewAttachmentLabel, GenericViewAttachmentLabel, GenericViewAttachmentLabelHandler, GenericDetailingSymbolHandler, DGNPLATFORM_EXPORT)
    };

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
