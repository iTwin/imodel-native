/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sheet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/TileTree.h>

BEGIN_SHEET_NAMESPACE

namespace Handlers
{
HANDLER_DEFINE_MEMBERS(Element);
HANDLER_DEFINE_MEMBERS(Attachment)
HANDLER_DEFINE_MEMBERS(Model)
}

END_SHEET_NAMESPACE

USING_NAMESPACE_SHEET

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Sheet::Element::CreateCode(DocumentListModelCR model, Utf8CP name)
    {
    return SheetAuthority::CreateSheetCode(name, model.GetModeledElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Sheet::Element::CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName)
    {
    DgnDbR db = model.GetDgnDb();
    DgnCode code = CreateCode(model, baseName);
    if (!db.Elements().QueryElementIdByCode(code).IsValid())
        return code;

    int counter=1;
    do  {
        Utf8PrintfString name("%s-%d", baseName, counter);
        code = CreateCode(model, name.c_str());
        counter++;
        } while (db.Elements().QueryElementIdByCode(code).IsValid());

    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPtr Sheet::Element::Create(DocumentListModelCR model, double scale, double height, double width, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(Handlers::Element::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || !name || !*name)
        {
        BeAssert(false);
        return nullptr;
        }

    auto sheet = new Element(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
    sheet->SetScale(scale);
    sheet->SetHeight(height);
    sheet->SetWidth(width);
    return sheet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementPtr Sheet::Element::Create(DocumentListModelCR model, double scale, DgnElementId sheetTemplate, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(Handlers::Element::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || !name || !*name)
        {
        BeAssert(false);
        return nullptr;
        }

    auto sheet = new Element(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
    sheet->SetScale(scale);
    sheet->SetTemplate(sheetTemplate);
#ifdef WIP_SHEETS
    sheet->SetHeight(sheetTemplateElem->GetHeight());
    sheet->SetWidth(sheetTemplateElem->GetWidth());
    sheet->SetBorder(sheetTemplateElem->GetBorder());
#endif
    BeAssert(false && "WIP_SHEETS - templates");
    return sheet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Sheet::ViewAttachment::CheckValid() const
    {
    if (!GetAttachedViewId().IsValid())
        return DgnDbStatus::ViewNotFound;

    if (!GetModel()->IsSheetModel())
        return DgnDbStatus::WrongModel;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Sheet::Model::_OnInsert()
    {
    if (!GetDgnDb().Elements().Get<Sheet::Element>(GetModeledElementId()).IsValid())
        {
        BeAssert(false && "A SheetModel should be modeling a Sheet element");
        return DgnDbStatus::BadElement;
        }

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelPtr Sheet::Model::Create(ElementCR sheet)
    {
    DgnDbR db = sheet.GetDgnDb();
    ModelHandlerR handler = Handlers::Model::GetHandler();
    DgnClassId classId = db.Domains().GetClassId(handler);

    if (!classId.IsValid() || !sheet.GetElementId().IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    DgnModelPtr model = handler.Create(DgnModel::CreateParams(db, classId, sheet.GetElementId()));
    if (!model.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return dynamic_cast<ModelP>(model.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ViewControllerPtr SheetViewDefinition::_SupplyController() const
    {
    return new Sheet::ViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentTree::AttachmentTree(DgnDbR db, DgnElementId attachmentId, Render::SystemP system) : m_attachmentId(attachmentId), TileTree::QuadTree::Root(db, Transform::FromIdentity(), "", system, 10, 256)
    {
    auto attach = m_db.Elements().Get<ViewAttachment>(attachmentId);
    if (!attach.IsValid())
        {
        BeAssert(false);
        return;
        }
    SetLocation(attach->GetPlacement().GetTransform());

    auto viewId = attach->GetAttachedViewId();
    m_view = m_db.Elements().Get<ViewDefinition>(DgnElementId(viewId.GetValue()));

//    m_rootTile = new Tile(*this, TileId(0,0,0), nullptr);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::ViewController::AttachmentTreePtr Sheet::ViewController::FindAttachment(DgnElementId attachId) const
    {
    for (auto& attach : m_attachments)
        {
        if (attach->GetAttachmentId() == attachId)
            return attach;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_LoadState()
    {
    auto model = GetViewedModel();
    if (nullptr == model || nullptr == m_vp)
        {
        BeAssert(false);
        return;
        }

    bvector<AttachmentTreePtr> attachments;

    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE ModelId=?");
    stmt->BindId(1, model->GetModelId());

    // If we're already loaded, look in existing AttachementTrees so we don't reload them
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        auto tree = FindAttachment(attachId);

        if (!tree.IsValid())
            tree = new AttachmentTree(GetDgnDb(), attachId, &m_vp->GetRenderTarget()->GetSystem());

        attachments.push_back(tree);
        }

    // save new list of attachment trees
    m_attachments = attachments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_DrawView(ViewContextR context)
    {
    auto model = GetViewedModel();
    if (nullptr == model)
        return;

    context.VisitDgnModel(*model);

    // Find and draw the view attachments.
    auto attachments = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId,ViewId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE ModelId=?");
    attachments->BindId(1, model->GetModelId());

    while (BE_SQLITE_ROW == attachments->Step())
        {
        auto attachmentId = attachments->GetValueId<DgnElementId>(0);
        auto viewId = attachments->GetValueId<DgnViewId>(1);
        ViewDefinitionPtr view = const_cast<ViewDefinition*>(ViewDefinition::QueryView(viewId, GetDgnDb()).get());
        if (view.IsNull())
            continue;

        // *** WIP_VIEW_ATTACHMENT - for now, show a thumbnail as a placeholder

        auto attachment = GetDgnDb().Elements().Get<ViewAttachment>(attachmentId);
        if (!attachment.IsValid())
            continue;

        auto const& placement = attachment->GetPlacement();
        auto const& box = placement.GetElementBox();

        Render::GraphicBuilderPtr graphic = context.CreateGraphic();

        IGraphicBuilder::TileCorners corners;
        //  [2]     [3]
        //  [0]     [1]
        DPoint2d tc[4];
        auto ll = box.low;
        auto ur = box.high;
        tc[0] = DPoint2d::From(ll.x , ll.y);
        tc[1] = DPoint2d::From(ur.x , ll.y);
        tc[2] = DPoint2d::From(ll.x , ur.y);
        tc[3] = DPoint2d::From(ur.x , ur.y);
        auto rot = placement.GetTransform();
        rot.Multiply(tc, tc, 4);
        for (auto i=0; i<4; ++i)
            corners.m_pts[i] = DPoint3d::From(tc[i].x, tc[i].y, 0.0);

        if (nullptr == dynamic_cast<ViewDefinition3d*>(view.get())) // don't try to generate thumbnail for 3-D views
            {
            #define WIP_SHEETS_SHOW_THUMBNAIL
            #ifdef WIP_SHEETS_SHOW_THUMBNAIL // *** generate thumbnail
                double meters_per_pixel = 0.0254 / context.GetViewport()->PixelsFromInches(1.0);

                auto imageSize = Point2d::From((int)(0.5 + box.GetWidth()/meters_per_pixel), (int)(0.5 + box.GetHeight()/meters_per_pixel));

                while (imageSize.x > 4096)
                    {
                    imageSize.x /= 10;
                    imageSize.y /= 10;
                    }

                Render::Image image;
                Render::RenderMode modeUsed;
                if (BE_SQLITE_OK != T_HOST._RenderThumbnail(image, modeUsed, *view, imageSize, nullptr, 12000))
                    continue;
            #else
                auto imageSource = GetViewDefinition().ReadThumbnail();
                if (!imageSource.IsValid())
                    continue;

                Render::Image image(imageSource);

            #endif

            auto& rsys = context.GetViewport()->GetRenderTarget()->GetSystem();
            Texture::CreateParams textureParams;
            auto texture = rsys._CreateTexture(image, textureParams);

            auto bgcolor = view->GetDisplayStyle().GetBackgroundColor();
            if (bgcolor.GetRed() == 0xff) // *** WIP_SHEETS - if I set the bg color to White (or black), the texture displays as all black??
                bgcolor.SetRed(0xfe);
            else if (bgcolor.GetRed() == 0)
                bgcolor.SetRed(1);

            graphic->SetSymbology(bgcolor, bgcolor, 0);

            graphic->AddTile(*texture, corners);
            }
        else
            {
            graphic->AddLineString(_countof(corners.m_pts), corners.m_pts);
            }

        context.OutputGraphic(*graphic, nullptr);
        }
    }
