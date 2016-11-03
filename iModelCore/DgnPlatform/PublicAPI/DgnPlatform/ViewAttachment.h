/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ViewAttachment.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnView.h>
#include <DgnPlatform/ViewController.h>

DGNPLATFORM_TYPEDEFS(ViewAttachment);
DGNPLATFORM_REF_COUNTED_PTR(ViewAttachment);

#define BIS_CLASS_ViewAttachment "ViewAttachment"

BEGIN_BENTLEY_DGN_NAMESPACE


namespace dgn_ElementHandler {struct ViewAttachmentHandler;}

//=======================================================================================
//! A ViewAttachment is a reference to a View that can be placed on a sheet.
//! The attachment specifies the extents of the View and the scaling factor when placed
//! onto a sheet.
//! @ingroup GROUP_DgnView
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewAttachment : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, GraphicalElement2d);
    friend struct dgn_ElementHandler::ViewAttachmentHandler;

protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR original) override;

    virtual DgnDbStatus _SetParentId(DgnElementId parentId) override { return DgnDbStatus::InvalidParent; }
    virtual DgnDbStatus _OnChildInsert(DgnElementCR) const override { return DgnDbStatus::InvalidParent; }
    virtual DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR updated) const override { return DgnDbStatus::InvalidParent; }
    
    explicit ViewAttachment(CreateParams const& params) : T_Super(params) { }

    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ViewAttachment)); }

    DGNPLATFORM_EXPORT DgnDbStatus CheckValid() const;

public:
    ViewAttachment(DgnDbR db, DgnModelId mid, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement)
        : T_Super(CreateParams(db, mid, QueryClassId(db), cat, placement))
        {
        SetViewId(viewId);
        SetCode(GenerateDefaultCode());
        }

    DgnViewId GetViewId() const {return GetPropertyValueId<DgnViewId>("View");} //!< Get the ID of the view definition to be drawn by this attachment
    DgnDbStatus SetViewId(DgnViewId viewId) {return SetPropertyValue("View", viewId);} //!< Set the view definition to be drawn
    };

namespace dgn_ElementHandler
{
    //! The handler for ViewAttachment elements
    struct ViewAttachmentHandler : Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, ViewAttachment, ViewAttachmentHandler, Geometric2d, DGNPLATFORM_EXPORT);
    };
};

END_BENTLEY_DGN_NAMESPACE

