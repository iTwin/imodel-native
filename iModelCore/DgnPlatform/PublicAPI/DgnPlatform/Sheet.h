/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Sheet.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnView.h>
#include <DgnPlatform/ViewController.h>

BEGIN_BENTLEY_DGN_NAMESPACE
struct SheetViewDefinition;
END_BENTLEY_DGN_NAMESPACE

#define USING_NAMESPACE_SHEET using namespace BentleyApi::Dgn::Sheet;

BEGIN_SHEET_NAMESPACE

#define BIS_CLASS_ViewAttachment "ViewAttachment"

//=======================================================================================
//! A sheet model is a GraphicalModel2d that has the following characteristics:
//!     - Has fixed extents (is not infinite), specified in meters.
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
};

//=======================================================================================
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Element : Document
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Sheet, Document)

public:
    explicit Element(CreateParams const& params) : T_Super(params) {}

    //! Create a DgnCode for a Sheet in the specified DocumentListModel
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DocumentListModelCR model, Utf8CP name);
    //! Create a unique DgnCode for a Sheet within the specified DocumentListModel
    //! @param[in] model The uniqueness scope for the DgnCode
    //! @param[in] baseName The base name for the CodeValue. A suffix will be appended (if necessary) to make it unique within the specified scope.
    DGNPLATFORM_EXPORT static DgnCode CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] height The sheet height (meters)
    //! @param[in] width The sheet width (meters)
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, double height, double width, Utf8CP name);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] sheetTemplate The sheet template. Maybe in valid if there is no template.
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, DgnElementId sheetTemplate, Utf8CP name);

    //! Get the drawing scale of the sheet
    double GetScale() const {return GetPropertyValueDouble("Scale");}

    //! Set the drawing scale of the sheet.
    //! @return DgnDbStatus::ReadOnly if the drawing scale is invalid.
    DgnDbStatus SetScale(double v) {return SetPropertyValue("Scale", v);}

    //! Get the height of the sheet
    double GetHeight() const {return GetPropertyValueDouble("Height");}

    //! Set the height of the sheet.
    //! @return DgnDbStatus::ReadOnly if the height is controlled by a template
    DgnDbStatus SetHeight(double v) {return SetPropertyValue("Height", v);}

    //! Get the width of the sheet
    double GetWidth() const {return GetPropertyValueDouble("Width");}

    //! Set the width of the sheet.
    //! @return DgnDbStatus::ReadOnly if the Width is controlled by a template
    DgnDbStatus SetWidth(double v) {return SetPropertyValue("Width", v);}

    //! Get the sheet template, if any.
    //! @return an invalid ID if the sheet has no template.
    DgnElementId GetTemplate() const {return GetPropertyValueId<DgnElementId>("Template");}

    //! Set the sheet template.
    DgnDbStatus SetTemplate(DgnElementId v) {return SetPropertyValue("Template", v);}

    //! Get the sheet border, if any.
    //! @return an invalid ID if the sheet has no border.
    DgnElementId GetBorder() const {return GetPropertyValueId<DgnElementId>("Border");}

    //! Set the sheet border.
    //! @return DgnDbStatus::ReadOnly if the Border is controlled by a template
    DgnDbStatus SetBorder(DgnElementId v) {return SetPropertyValue("Border", v);}
};

//=======================================================================================
//! A ViewAttachment is a reference to a View, placed on a sheet.
//! The attachment specifies the Id of the View and the position on the sheet.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewAttachment : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, GraphicalElement2d);

protected:
    static Utf8CP str_ViewId() {return "ViewId";}
    DGNPLATFORM_EXPORT DgnDbStatus CheckValid() const;
    virtual DgnDbStatus _OnInsert() override {auto status = CheckValid(); return DgnDbStatus::Success == status ? T_Super::_OnInsert() : status;}
    virtual DgnDbStatus _OnUpdate(DgnElementCR original) override {auto status = CheckValid(); return DgnDbStatus::Success == status ? T_Super::_OnUpdate(original) : status;}
    virtual DgnDbStatus _SetParentId(DgnElementId parentId) override {return DgnDbStatus::InvalidParent;}
    virtual DgnDbStatus _OnChildInsert(DgnElementCR) const override {return DgnDbStatus::InvalidParent;}
    virtual DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR updated) const override {return DgnDbStatus::InvalidParent;}
    
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ViewAttachment));}

public:
    explicit ViewAttachment(CreateParams const& params) : T_Super(params) {}
    ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement) : T_Super(CreateParams(db, model, QueryClassId(db), cat, placement))
        {
        SetViewId(viewId);
        SetCode(GenerateDefaultCode());
        }

    DgnViewId GetViewId() const {return GetPropertyValueId<DgnViewId>(str_ViewId());} //!< Get the Id of the view definition to be drawn by this attachment
    DgnDbStatus SetViewId(DgnViewId viewId) {return SetPropertyValue(str_ViewId(), viewId);} //!< Set the view definition to be drawn
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
    ViewControllerCP _ToSheetView() const override {return this;}
    DGNPLATFORM_EXPORT void _DrawView(ViewContextR) override;

    //! Construct a new SheetViewController.
    ViewController(SheetViewDefinitionCR def) : ViewController2d(def) {}
};


namespace Handlers
{
    //! The ElementHandler for Sheet
    struct EXPORT_VTABLE_ATTRIBUTE Element : dgn_ElementHandler::Document
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Sheet, Sheet::Element, Element, Document, DGNPLATFORM_EXPORT)
    };

    //! The handler for ViewAttachment elements
    struct Attachment : dgn_ElementHandler::Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, Sheet::ViewAttachment, Attachment, Geometric2d, DGNPLATFORM_EXPORT);
    };

    //! The ModelHandler for SheetModel
    struct EXPORT_VTABLE_ATTRIBUTE Model :  dgn_ModelHandler::Model
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_SheetModel, Sheet::Model, Model, dgn_ModelHandler::Model, DGNPLATFORM_EXPORT)
    };

};

END_SHEET_NAMESPACE
