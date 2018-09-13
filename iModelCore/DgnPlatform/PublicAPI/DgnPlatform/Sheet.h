/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Sheet.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/ViewDefinition.h>

#define USING_NAMESPACE_SHEET using namespace BentleyApi::Dgn::Sheet;
#define BIS_CLASS_ViewAttachment "ViewAttachment"

BEGIN_SHEET_NAMESPACE

//=======================================================================================
//! Describes the geometry of the sheet border in view coordinates or world coordinates.
// @bsistruct                                                   Paul.Connelly   03/18
//=======================================================================================
struct Border
{
private:
    DPoint2d                m_rect[5];
    DPoint2d                m_shadow[7];
    Render::GradientSymbPtr m_gradient;

    Border(ViewContextCP context, DPoint2dCR size);
public:
    enum class CoordSystem { View, World };

    //! Create a border of the specified size in view or world coordinates
    Border(ViewContextCR context, DPoint2dCR size, CoordSystem coords) : Border(CoordSystem::View == coords ? &context : nullptr, size) { }

    //! Create a border of the specified size in world coordinates
    explicit Border(DPoint2dCR size) : Border(nullptr, size) { }

    void AddToBuilder(Render::GraphicBuilderR) const;
    DRange2d GetRange() const;
};

//=======================================================================================
//! A Sheet::Model is a GraphicalModel2d that has the following characteristics:
//!     - Has finite  extents, specified in meters.
//!     - Can contain @b views of other models, like pictures pasted on a photo album.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Model : GraphicalModel2d
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_SheetModel, GraphicalModel2d);

protected:
    ModelCP _ToSheetModel() const override final {return this;}

    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;

public:
    //! construct a new SheetModel
    explicit Model(CreateParams const& params) : T_Super(params) {}

    //! Construct a SheetModel
    //! @param[in] params The CreateParams for the new SheetModel
    static ModelPtr Create(CreateParams const& params) {return new Model(params);}

    //! Create a SheetModel that breaks down the specified Sheet element
    DGNPLATFORM_EXPORT static ModelPtr Create(ElementCR sheet);

    //! Find the first SheetViewDefinition that displays the specified sheet model.
    DGNPLATFORM_EXPORT static DgnElementId FindFirstViewOfSheet(DgnDbR db, DgnModelId sheetModelId);

    //! Get the sheet size.
    DPoint2d GetSheetSize() const;

    //! Get the sheet extents.
    AxisAlignedBox3d GetSheetExtents() const;

    //! Get the sheet attachment IDs.
    DGNPLATFORM_EXPORT bvector<DgnElementId> GetSheetAttachmentIds() const;

    //! Get the sheet attachment views.
    DGNPLATFORM_EXPORT bvector<ViewDefinitionCPtr> GetSheetAttachmentViews(DgnDbR db) const;
};

//=======================================================================================
//! Sheet::Element
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Element : Document
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Sheet, Document)

public:
    BE_PROP_NAME(Scale)
    BE_PROP_NAME(Height)
    BE_PROP_NAME(Number)
    BE_PROP_NAME(Width)
    BE_PROP_NAME(Template)
    BE_PROP_NAME(Border)

    explicit Element(CreateParams const& params) : T_Super(params) {}

    //! Create a DgnCode for a Sheet in the specified DocumentListModel
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DocumentListModelCR model, Utf8StringCR name);

    //! Create a unique DgnCode for a Sheet within the specified DocumentListModel
    //! @param[in] model The uniqueness scope for the DgnCode
    //! @param[in] baseName The base name for the CodeValue. A suffix will be appended (if necessary) to make it unique within the specified scope.
    //! @private
    DGNPLATFORM_EXPORT static DgnCode CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] size The sheet size (meters)
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, DPoint2dCR size, Utf8StringCR name);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] sheetTemplate The sheet template. Maybe in valid if there is no template.
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, DgnElementId sheetTemplate, Utf8StringCR name);

    //! Get the drawing scale of the sheet
    double GetScale() const {return GetPropertyValueDouble(prop_Scale());}

    //! Set the drawing scale of the sheet.
    //! @return DgnDbStatus::ReadOnly if the drawing scale is invalid.
    DgnDbStatus SetScale(double v) {return SetPropertyValue(prop_Scale(), v);}

    //! Get the height of the sheet
    double GetHeight() const {return GetPropertyValueDouble(prop_Height());}

    //! Set the height of the sheet.
    //! @return DgnDbStatus::ReadOnly if the height is controlled by a template
    DgnDbStatus SetHeight(double v) {return SetPropertyValue(prop_Height(), v);}

    //! Get the width of the sheet
    double GetWidth() const {return GetPropertyValueDouble(prop_Width());}

    //! Set the width of the sheet.
    //! @return DgnDbStatus::ReadOnly if the Width is controlled by a template
    DgnDbStatus SetWidth(double v) {return SetPropertyValue(prop_Width(), v);}

    //! Get the sheet template, if any.
    //! @return an invalid ID if the sheet has no template.
    DgnElementId GetTemplate() const {return GetPropertyValueId<DgnElementId>(prop_Template());}

    //! Set the sheet template.
    DgnDbStatus SetTemplate(DgnElementId v) {return SetPropertyValue(prop_Template(), v, ECN::ECClassId());}

    //! Get the sheet border, if any.
    //! @return an invalid ID if the sheet has no border.
    DgnElementId GetBorder() const {return GetPropertyValueId<DgnElementId>(prop_Border());}

        //! Set the sheet border.
    //! @return DgnDbStatus::ReadOnly if the Border is controlled by a template
    DgnDbStatus SetBorder(DgnElementId v) {return SetPropertyValue(prop_Border(), v, ECN::ECClassId());}
};

//=======================================================================================
//! A Sheet::ViewAttachment is a reference to a View, placed on a sheet.
//! The attachment specifies the Id of the View and the position on the sheet.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewAttachment : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, GraphicalElement2d);

protected:
    DGNPLATFORM_EXPORT DgnDbStatus CheckValid() const;
    DgnDbStatus _OnInsert() override {auto status = CheckValid(); return DgnDbStatus::Success == status ? T_Super::_OnInsert() : status;}
    DgnDbStatus _OnUpdate(DgnElementCR original) override {auto status = CheckValid(); return DgnDbStatus::Success == status ? T_Super::_OnUpdate(original) : status;}
    DgnDbStatus _SetParentId(DgnElementId, DgnClassId) override {return DgnDbStatus::InvalidParent;}
    DgnDbStatus _OnChildInsert(DgnElementCR) const override {return DgnDbStatus::InvalidParent;}
    DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR updated) const override {return DgnDbStatus::InvalidParent;}

    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ViewAttachment));}
    static Placement2d ComputePlacement(DgnDbR db, DgnViewId viewId, DPoint2dCR origin, double scale);
    static double ComputeScale(DgnDbR db, DgnViewId viewId, ElementAlignedBox2dCR);

public:
    BE_PROP_NAME(View);

    BE_JSON_NAME(details);
    BE_JSON_NAME(displayPriority);
    BE_JSON_NAME(scale);
    BE_JSON_NAME(clip);

    explicit ViewAttachment(CreateParams const& params) : T_Super(params) {}

    //! Construct an attachment in the case where you know the size of the attachment. The view scale will be computed.
    //! @param db   The DgnDb that will contain the attachment
    //! @param model The model in the DgnDb that will contain the attachment
    //! @param viewId   The view that is being attached
    //! @param cat      The attachment's category
    //! @param placement The attachment's origin and size on the sheet.
    DGNPLATFORM_EXPORT ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement);

    //! Construct an attachment in the case where you know the view scale. Th5e placement's size will be computed.
    //! @param db   The DgnDb that will contain the attachment
    //! @param model The model in the DgnDb that will contain the attachment
    //! @param viewId   The view that is being attached
    //! @param cat      The attachment's category
    //! @param origin   The attachment's origin on the sheet
    //! @param scale    The view scale
    DGNPLATFORM_EXPORT ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, DPoint2dCR origin, double scale);

    DgnViewId GetAttachedViewId() const {return GetPropertyValueId<DgnViewId>(prop_View());} //!< Get the Id of the view definition to be drawn by this attachment
    DgnDbStatus SetAttachedViewId(DgnViewId viewId) {return SetPropertyValue(prop_View(), viewId, ECN::ECClassId());} //!< Set the view definition to be drawn

    Utf8CP GetDetails() const { return m_jsonProperties[json_details()].asCString(nullptr); }
    void SetDetails(Utf8CP details) { details ? m_jsonProperties[json_details()] = details : m_jsonProperties.removeMember(json_details()); }

    int32_t GetDisplayPriority() const { return m_jsonProperties[json_displayPriority()].asInt(0); }
    void SetDisplayPriority(int32_t priority) { m_jsonProperties[json_displayPriority()] = priority; }

    double GetScale() const { return m_jsonProperties[json_scale()].asDouble(0.0); }
    void SetScale(double scale) { m_jsonProperties[json_scale()] = scale; }

    //! Get the clip to be applied to this attachment, if any.
    //! @return a clip vector or an invalid ptr if the attachment is not clipped.
    //! @see SetClip
    DGNPLATFORM_EXPORT ClipVectorPtr GetClip() const;

    //! Get the clip to be applied to this attachment, or if none is defined,
    //! create one based on the attachment's range.
    //! @see GetClip
    ClipVectorPtr GetOrCreateClip(TransformCP transform=nullptr) const;

    //! Get a rectangular clip based on the range of the ViewAttachment.
    ClipVectorPtr CreateBoundaryClip() const;

    //! Set the clip to be applied to this attachment.
    //! @see ClearClip
    DGNPLATFORM_EXPORT void SetClip(ClipVectorCR);

    //! Clear the clip for this attachment.
    //! @see SetClip
    DGNPLATFORM_EXPORT void ClearClip();
};

//=======================================================================================
//! A Sheet::ViewController is used to control views of Sheet::Models
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct ViewController : Dgn::ViewController2d
{
    DEFINE_T_SUPER(ViewController2d);
    friend SheetViewDefinition;

protected:
    DPoint2d m_size;

    ViewControllerCP _ToSheetView() const override {return this;}
    void _DrawView(ViewContextR) override;
    void _LoadState() override;

    void DrawBorder(ViewContextR context) const;
    ViewController(SheetViewDefinitionCR def) : ViewController2d(def) {}  //!< Construct a new SheetViewController.
};

//=======================================================================================
// Sheet::Handlers
//=======================================================================================
namespace Handlers
{
    //! The ElementHandler for Sheet::Elements
    struct EXPORT_VTABLE_ATTRIBUTE Element : dgn_ElementHandler::Document
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Sheet, Sheet::Element, Element, Document, DGNPLATFORM_EXPORT)
    };

    //! The handler for Sheet::ViewAttachment elements
    struct AttachmentElement : dgn_ElementHandler::Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, Sheet::ViewAttachment, AttachmentElement, Geometric2d, DGNPLATFORM_EXPORT);
    };

    //! The ModelHandler for Sheet::Model
    struct EXPORT_VTABLE_ATTRIBUTE Model :  dgn_ModelHandler::Geometric2d
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_SheetModel, Sheet::Model, Model, dgn_ModelHandler::Geometric2d, DGNPLATFORM_EXPORT)
    };
};

END_SHEET_NAMESPACE
