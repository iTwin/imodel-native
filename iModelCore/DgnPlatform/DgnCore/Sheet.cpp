/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sheet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/TileTree.h>
#include <folly/BeFolly.h>

BEGIN_SHEET_NAMESPACE
namespace Handlers
{
HANDLER_DEFINE_MEMBERS(Element);
HANDLER_DEFINE_MEMBERS(AttachmentElement)
HANDLER_DEFINE_MEMBERS(Model)
}
END_SHEET_NAMESPACE

USING_NAMESPACE_TILETREE
USING_NAMESPACE_SHEET
using namespace Attachment;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Sheet::Element::CreateCode(DocumentListModelCR model, Utf8CP name)
    {
    return ModelScopeAuthority::CreateCode(BIS_AUTHORITY_Sheet, model, name);
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

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  11/2016
//----------------------------------------------------------------------------------------
folly::Future<BentleyStatus> Attachment::Tile::Loader::_SaveToDb() {return SUCCESS;}
folly::Future<BentleyStatus> Attachment::Tile::Loader::_ReadFromDb() {return ERROR;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<BentleyStatus> Attachment::Tile::Loader::_GetFromSource()
    {
    if (IsCanceledOrAbandoned())
        return ERROR;

    Tile& tile = static_cast<Tile&>(*m_tile);
    Tree& root = tile.GetTree();
    return root.m_viewport->_CreateTile(m_loads, m_image, tile, root.m_pixels);
    }

/*---------------------------------------------------------------------------------**//**
* This sheet tile just became available. Create a Render::Graphic to draw it. When finished, set the "ready" flag.
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Attachment::Tile::Loader::_LoadTile()
    {
    auto& tile = static_cast<Tile&>(*m_tile);
    Tree& tree = tile.GetTree();
    auto system = tree.GetRenderSystem();

    auto graphic = system->_CreateGraphic(Graphic::CreateParams(nullptr));

    Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = system->_CreateTexture(m_image, textureParams);

    graphic->SetSymbology(tree.m_tileColor, tree.m_tileColor, 0); // this is to set transparency
    graphic->AddTile(*texture, tile.m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

#if defined (DEBUG_TILES)
    graphic->SetSymbology(ColorDef::DarkOrange(), ColorDef::Green(), 0);
    graphic->AddRangeBox(tile.m_range);
#endif

    auto stat = graphic->Close(); // explicitly close the Graphic. This potentially blocks waiting for QV from other threads
    BeAssert(SUCCESS==stat);
    UNUSED_VARIABLE(stat);

    tile.m_graphic = graphic;

    tile.SetIsReady(); // OK, we're all done loading and the other thread may now use this data. Set the "ready" flag.
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Attachment::Tile::Tile(Tree& root, QuadTree::TileId id, Tile const* parent) : T_Super(root, id, parent)
    {
    double tileSize = 1.0 / (1 << id.m_level); // the size of a tile for this level, in NPC
    double east  = id.m_column * tileSize;
    double west  = east + tileSize;
    double north = id.m_row * tileSize;
    double south = north + tileSize;

    m_corners.m_pts[0].Init(east, north, 0.0);   //  | [0]     [1]
    m_corners.m_pts[1].Init(west, north, 0.0);   //  y
    m_corners.m_pts[2].Init(east, south, 0.0);   //  | [2]     [3]
    m_corners.m_pts[3].Init(west, south, 0.0);   //  v

    m_range.InitFrom(m_corners.m_pts, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment::Tree::Load(Render::SystemP renderSys)
    {
    if (m_rootTile.IsValid() && (nullptr==renderSys || m_renderSystem==renderSys))
        return;

    m_renderSystem = renderSys;
    m_rootTile = new Tile(*this, QuadTree::TileId(0,0,0), nullptr);
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/16
//=======================================================================================
struct SceneReadyTask : Dgn::ProgressiveTask
{
    Attachment::Tree& m_tree;
    SceneReadyTask(Attachment::Tree& tree) : m_tree(tree) {}
    ProgressiveTask::Completion _DoProgressive(ProgressiveContext& context, WantShow& showFrame) override
        {
        if (!m_tree.m_viewport->GetViewControllerR().UseReadyScene().IsValid())
            return ProgressiveTask::Completion::Aborted;

        m_tree.m_sceneReady = true;
        m_tree.DrawInView(context);
        return ProgressiveTask::Completion::Finished;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment::Tree::Draw(RenderContextR context)
    {
    Load(&context.GetTargetR().GetSystem());

    if (!m_sceneQueued)
        {
        m_viewport->m_rect.Init(0, 0, m_pixels.x, m_pixels.y);
        m_viewport->_QueueScene();
        m_sceneQueued = true;
        }

    if (!m_sceneReady)
        {
        context.GetViewport()->ScheduleProgressiveTask(*new SceneReadyTask(*this));
        return;
        }
    
    DrawInView(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Attachment::Tree::Tree(DgnDbR db, DgnElementId attachmentId, uint32_t tileSize) : T_Super(db,Transform::FromIdentity(), "", nullptr, 12, tileSize), m_attachmentId(attachmentId)
    {
    auto attach = db.Elements().Get<ViewAttachment>(attachmentId);
    if (!attach.IsValid())
        {
        BeAssert(false);
        return;
        }

    m_viewport = T_HOST._CreateTileViewport();
    if (!m_viewport.IsValid())
        return;

    auto viewId = attach->GetAttachedViewId();
    auto view = ViewDefinition::LoadViewController(viewId, db);
    if (!view.IsValid())
        return;

    double aspect = view->GetViewDefinition().GetAspectRatio();

    if (aspect<1.0)
        m_pixels.Init(tileSize*aspect, tileSize);
    else
        m_pixels.Init(tileSize, tileSize/aspect);

    BeAssert(m_pixels.x>0);
    BeAssert(m_pixels.y>0);

    auto& def=view->GetViewDefinition();

    SpatialViewDefinitionP spatial=def.ToSpatialViewP();
    if (spatial)
        {
        auto& env = spatial->GetDisplayStyle3d().GetEnvironmentDisplayR();
//        env.m_groundPlane.m_enabled = false;
        env.m_skybox.m_enabled = false;
        }

    // max pixel size is half the length of the diagonal.
    m_maxPixelSize = .5 * DPoint2d::FromZero().Distance(DPoint2d::From(m_pixels.x, m_pixels.y));

    auto range = attach->GetPlacement().CalculateRange();
    auto& box = attach->GetPlacement().GetElementBox();

    range.low.z = 0.0; // make sure we're exactly on the sheet.
    Transform location = Transform::From(range.low);
    location.ScaleMatrixColumns(box.GetWidth(), box.GetHeight(), 1.0);
    SetLocation(location);

    SetExpirationTime(std::chrono::seconds(5)); // only save unused sheet tiles for 5 seconds

    m_biasDistance = Render::Target::DepthFromDisplayPriority(attach->GetDisplayPriority());
    m_hiResBiasDistance = Render::Target::DepthFromDisplayPriority(-1);
    m_loResBiasDistance = m_hiResBiasDistance * 2.0;
    m_viewport->ChangeViewController(*view);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::TreePtr Sheet::ViewController::FindAttachment(DgnElementId attachId) const
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
    if (nullptr == model)
        {
        BeAssert(false);
        return;
        }

    bvector<TreePtr> attachments;

    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE ModelId=?");
    stmt->BindId(1, model->GetModelId());

    // If we're already loaded, look in existing list so we don't reload them
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        auto tree = FindAttachment(attachId);

        if (!tree.IsValid())
            tree = new Tree(GetDgnDb(), attachId, 512);

        attachments.push_back(tree);
        }

    // save new list of attachment
    m_attachments = attachments;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_CreateTerrain(TerrainContextR context)
    {
    DgnDb::VerifyClientThread();

    T_Super::_CreateTerrain(context);

    for (auto& attach : m_attachments)
        attach->Draw(context);
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

#ifdef DEBUG_ATTACHMENT_RANGE
    auto attachments = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId,[ViewId] FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE ModelId=?");
    attachments->BindId(1, model->GetModelId());
    while (BE_SQLITE_ROW == attachments->Step())
        {
        auto attachment = GetDgnDb().Elements().Get<ViewAttachment>(attachments->GetValueId<DgnElementId>(0));
        if (!attachment.IsValid())
            continue;

        auto const& placement = attachment->GetPlacement();
        AxisAlignedBox3d range;
        placement.GetTransform().Multiply(range, DRange3d::From(&placement.GetElementBox().low, 2, 0.0));

        Render::GraphicBuilderPtr graphicBbox = context.CreateGraphic();
        graphicBbox->SetSymbology(ColorDef::Green(), ColorDef::Green(), 2, GraphicParams::LinePixels::Code5);
        graphicBbox->AddRangeBox(range);
        context.OutputGraphic(*graphicBbox, nullptr);

        Render::GraphicBuilderPtr graphicOrigin = context.CreateGraphic();
        DPoint3d org = DPoint3d::From(placement.GetOrigin());
        graphicOrigin->SetSymbology(ColorDef::Blue(), ColorDef::Blue(), 10);
        graphicOrigin->AddPointString(1, &org);
        context.OutputGraphic(*graphicOrigin, nullptr);
        }
#endif
    }
