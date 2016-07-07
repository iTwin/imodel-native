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

//=======================================================================================
//! A ViewAttachment is a reference to a View of a model which can be placed on a sheet.
//! The attachment specifies the extents of the View and the scaling factor when placed
//! onto a sheet.
//! The geometry of a ViewAttachment is a snapshot of the view at the time they are generated.
//! They are not dynamic - if the view and/or viewed model changes, the ViewAttachment's
//! geometry should be explicitly refreshed.
//! The subcategories of the viewed geometry are not preserved in the attachment's geometry
//! - to control visibility of (sub)-categories or other graphic settings, edit the view
//! and/or viewed model.
//! @ingroup GROUP_DgnView
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewAttachment : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, GraphicalElement2d);
public:
    //! Holds the data which describes a ViewAttachment
    struct Data
    {
        DgnViewId   m_viewId;
        double      m_scale;

        //! Constructor
        //! @param[in]      viewId The ID of the view to be attached
        //! @param[in]      scale  Scale factor from viewed model to sheet
        explicit Data(DgnViewId viewId, double scale=1.0) { Init(viewId, scale); }

        //! Default constructor
        Data() : Data(DgnViewId()) { }

        //! Initialize from the specified values
        //! @param[in]      viewId The ID of the view to be attached
        //! @param[in]      scale  Scale factor from viewed model to sheet
        void Init(DgnViewId viewId, double scale=1.0)
            { m_viewId=viewId; m_scale=scale; }

        DGNPLATFORM_EXPORT bool IsValid() const; //!< Queries whether the data is valid (e.g., refers to a valid ViewDefinition; has meaningful scale and extents; etc)
    };

    //! Parameters used to construct a ViewAttachment element
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(ViewAttachment::T_Super::CreateParams);

        Data    m_data;

        //! Constructor
        //! @param[in]      db        The DgnDb in which the ViewAttachment will reside
        //! @param[in]      modelId   The sheet model in which the ViewAttachment will reside
        //! @param[in]      classId   Identifies the ECClass of the ViewAttachment element
        //! @param[in]      category  The element's category
        //! @param[in]      data      Describes the ViewAttachment properties
        //! @param[in]      placement The element's placement within the sheet
        //! @param[in]      code      Optional element code
        //! @param[in]      label     Optional element label
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, DgnCategoryId category, Data const& data, Placement2dCR placement=Placement2d(), DgnCode const& code=DgnCode(), Utf8CP label=nullptr)
            : T_Super(db, modelId, classId, category, placement, code, label), m_data(data) { }

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, DgnCategoryId categoryId=DgnCategoryId(), Placement2dCR placement=Placement2d(), Data const& data=Data())
            : T_Super(params, categoryId, placement), m_data(data) { }
    };
    
private:
    Data    m_data;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    Data const& GetData() const { return m_data; }
    Data& GetData() { return m_data; }

    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;

    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual void _RemapIds(DgnImportContext& importer) override;

    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnUpdate(DgnElementCR original) override;

    virtual DgnDbStatus _SetParentId(DgnElementId parentId) override { return DgnDbStatus::InvalidParent; }
    virtual DgnDbStatus _OnChildInsert(DgnElementCR) const override { return DgnDbStatus::InvalidParent; }
    virtual DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR updated) const override { return DgnDbStatus::InvalidParent; }

    virtual uint32_t _GetMemSize() const override { return T_Super::_GetMemSize() + static_cast<uint32_t>(sizeof(m_data)); }
public:
    //! Construct a ViewAttachment from the specified parameters.
    explicit ViewAttachment(CreateParams const& params) : T_Super(params), m_data(params.m_data) { }

    DgnViewId GetViewId() const { return m_data.m_viewId; } //!< The ID of the view definition to be drawn by this attachment
    double GetViewScale() const { return m_data.m_scale; } //!< Scale factor from model to sheet

    //! Look up the ViewDefinition to be drawn by this attachment
    ViewDefinitionCPtr GetViewDefinition() const { return ViewDefinition::QueryView(GetViewId(), GetDgnDb()); }

    void SetViewId(DgnViewId viewId) { m_data.m_viewId = viewId; } //!< Set the view definition to be drawn
    void SetViewScale(double scale) { m_data.m_scale = scale; } //!< Set the scale factor from model to sheet

    //! Inserts this attachment into the DgnDb and returns the new persistent copy.
    ViewAttachmentCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<ViewAttachment>(*this, status); }

    //! Updates this attachment and returns the modified persistent copy.
    ViewAttachmentCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<ViewAttachment>(*this, status); }

    //! Generates (or regenerates) this attachment's geometry to reflect the view of the viewed model.
    DGNPLATFORM_EXPORT DgnDbStatus GenerateGeomStream(DgnSubCategoryId subCategory=DgnSubCategoryId());

    //! Look up the ID of the base ViewAttachmentElement ECClass within the specified DgnDb
    static DgnClassId QueryClassId(DgnDbR db) { return DgnClassId(db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ViewAttachment)); }
};

namespace dgn_ElementHandler
{
    //! The handler for ViewAttachment elements
    struct ViewAttachmentHandler : Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, ViewAttachment, ViewAttachmentHandler, Geometric2d, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
};

END_BENTLEY_DGN_NAMESPACE

