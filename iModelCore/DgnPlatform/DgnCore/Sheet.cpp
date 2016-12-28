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
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewAttachment::ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement)
    : T_Super(CreateParams(db, model, QueryClassId(db), cat, placement))
    {
    SetAttachedViewId(viewId);
    SetCode(GenerateDefaultCode());
    SetScale(ComputeScale(db, viewId, placement.GetElementBox()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewAttachment::ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, DPoint2dCR origin, double scale)
    : T_Super(CreateParams(db, model, QueryClassId(db), cat, ComputePlacement(db, viewId, origin, scale)))
    {
    SetAttachedViewId(viewId);
    SetCode(GenerateDefaultCode());
    SetScale(scale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
Placement2d ViewAttachment::ComputePlacement(DgnDbR db, DgnViewId viewId, DPoint2dCR origin, double scale)
    {
    auto viewDef = db.Elements().Get<ViewDefinition>(viewId);
    if (!viewDef.IsValid())
        {
        BeAssert(false);
        return Placement2d{};
        }
    auto viewExtents = viewDef->GetExtents();

    Placement2d placement;
    placement.GetOriginR() = origin;

    ElementAlignedBox2d box;
    box.low.Zero();
    box.high.x = viewExtents.x / scale;
    box.high.y = viewExtents.y / scale;
    placement.SetElementBox(box);
    
    return placement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
double ViewAttachment::ComputeScale(DgnDbR db, DgnViewId viewId, ElementAlignedBox2dCR placement)
    {
    auto viewDef = db.Elements().Get<ViewDefinition>(viewId);
    if (!viewDef.IsValid())
        {
        BeAssert(false);
        return 1.0;
        }
    auto viewExtents = viewDef->GetExtents();

    return viewExtents.x / placement.GetWidth();
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
    return root.m_viewport->_CreateTile(m_loads, m_texture, tile, Point2d::From(root.m_pixels, root.m_pixels));
    }

/*---------------------------------------------------------------------------------**//**
* This sheet tile just became available. Create a Render::Graphic to draw it. When finished, set the "ready" flag.
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Attachment::Tile::Loader::_LoadTile()
    {
    if (!m_texture.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    auto& tile = static_cast<Tile&>(*m_tile);
    Tree& tree = tile.GetTree();
    auto system = tree.GetRenderSystem();
    auto graphic = system->_CreateGraphic(Graphic::CreateParams(nullptr));

    graphic->SetSymbology(tree.m_tileColor, tree.m_tileColor, 0); // this is to set transparency
    graphic->AddTile(*m_texture, tile.m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

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
    double tileSizeX = root.m_sizeNPC.x / (1 << id.m_level); // the x size of a tile for this level, in NPC
    double tileSizeY = root.m_sizeNPC.y / (1 << id.m_level); // the y size of a tile for this level, in NPC

    double east  = id.m_column * tileSizeX;
    double west  = east + tileSizeX;
    double north = id.m_row * tileSizeY;
    double south = north + tileSizeY;

    m_corners.m_pts[0].Init(east, south, 0.0); 
    m_corners.m_pts[1].Init(west, south, 0.0); 
    m_corners.m_pts[2].Init(east, north, 0.0); 
    m_corners.m_pts[3].Init(west, north, 0.0); 

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
// When we draw ViweAttachments on sheets, we first create the scene asynchronusly. While that's 
// in process we create an instance of this class to trigger the creation of the tiles when the scene becomes
// available. Note that there is one instance of this class per attachment, so there can be many of them
// for the same sheet (and of course many sheets) at any given time.
// @bsiclass                                                    Keith.Bentley   12/16
//=======================================================================================
struct SceneReadyTask : ProgressiveTask
{
    Attachment::Tree& m_tree;
    SceneReadyTask(Attachment::Tree& tree) : m_tree(tree) {}
    ProgressiveTask::Completion _DoProgressive(ProgressiveContext& context, WantShow& showFrame) override
        {
        // is the scene available yet?
        if (!m_tree.m_viewport->GetViewControllerR().UseReadyScene().IsValid())
            return ProgressiveTask::Completion::Aborted; // no, keep waiting

        m_tree.m_sceneReady = true; // yes, mark it as ready and draw its tiles
        m_tree.DrawInView(context);
        return ProgressiveTask::Completion::Finished; // we're done.
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment::Tree::Draw(RenderContextR context)
    {
    Load(&context.GetTargetR().GetSystem());

    // before we can draw a ViewAttachment tree, we need to request that its scene be created.
    if (!m_sceneQueued)
        {
        m_viewport->m_rect.Init(0, 0, m_pixels, m_pixels);
        m_viewport->_QueueScene(); // this queues the scene request on the SceneThread and returns immediately
        m_sceneQueued = true; // remember that we've already queued it
        }

    if (!m_sceneReady) // if the scene isn't ready yet, we need to wait for it to finish.
        {
        context.GetViewport()->ScheduleProgressiveTask(*new SceneReadyTask(*this));
        return;
        }
    
    // the scene is available, draw its tiles
    DrawInView(context);

#ifdef DEBUG_ATTACHMENT_RANGE
    ElementAlignedBox3d range(0,0,0, 1.0,1.0,1.0);
    GetLocation().Multiply(&range.low, &range.low, 2);

    Render::GraphicBuilderPtr graphicBbox = context.CreateGraphic();
    graphicBbox->SetSymbology(ColorDef::Green(), ColorDef::Green(), 2, GraphicParams::LinePixels::Code5);
    graphicBbox->AddRangeBox(range);
    context.OutputGraphic(*graphicBbox, nullptr);

    Render::GraphicBuilderPtr graphicOrigin = context.CreateGraphic();
    graphicOrigin->SetSymbology(ColorDef::Blue(), ColorDef::Blue(), 10);
    graphicOrigin->AddPointString(1, &range.low);
    context.OutputGraphic(*graphicOrigin, nullptr);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool Attachment::Tree::Pick(PickContext& context)
    {
    if (!m_sceneReady) // we can't pick anything unless we have a valid scene.
        return false;

    Transform sheetToTile;
    sheetToTile.InverseOf(GetLocation());
    Frustum box = context.GetFrustum().TransformBy(sheetToTile);

    if (m_clip.IsValid() && (ClipPlaneContainment::ClipPlaneContainment_StronglyOutside == m_clip->ClassifyPointContainment(box.m_pts, 8)))
        return false;

    Frustum frust = m_viewport->GetFrustum(DgnCoordSystem::Npc).TransformBy(GetLocation());
    context.WorldToView(frust.m_pts, frust.m_pts, 8);
    Transform attachViewToSheetView = Transform::FromPlaneOf3Points(frust.m_pts[NPC_LeftTopRear], frust.m_pts[NPC_RightTopRear], frust.m_pts[NPC_LeftBottomRear]);
    attachViewToSheetView.ScaleMatrixColumns(m_sizeNPC.x/m_pixels, m_sizeNPC.y/m_pixels, 1.0);

    return context._ProcessSheetAttachment(*m_viewport, attachViewToSheetView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Attachment::Tree::Tree(DgnDbR db, Sheet::ViewController& sheetController, DgnElementId attachmentId, uint32_t tileSize) : T_Super(db,Transform::FromIdentity(), "", nullptr, 12, tileSize), m_attachmentId(attachmentId), m_pixels(tileSize)
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

    // we use square tiles. If the view's aspect ratio isn't square, expand the short side in tile (NPC) space. We'll clip out the extra area below.
    double aspect = view->GetViewDefinition().GetAspectRatio();
    if (aspect<1.0)
        m_sizeNPC.Init(1.0/aspect, 1.0);
    else
        m_sizeNPC.Init(1.0, aspect);

    auto& def=view->GetViewDefinition();

    // override the background color. This is to match V8, but there should probably be an option in the "Details" about whether to do this or not.
    def.GetDisplayStyle().SetBackgroundColor(sheetController.GetViewDefinition().GetDisplayStyle().GetBackgroundColor());

    SpatialViewDefinitionP spatial=def.ToSpatialViewP();
    if (spatial)
        {
        auto& env = spatial->GetDisplayStyle3d().GetEnvironmentDisplayR();
        env.m_groundPlane.m_enabled = false;
        env.m_skybox.m_enabled = false;
        }

    // max pixel size is half the length of the diagonal.
    m_maxPixelSize = .5 * DPoint2d::FromZero().Distance(DPoint2d::From(m_pixels, m_pixels));

    auto& box = attach->GetPlacement().GetElementBox();
    auto range = attach->GetPlacement().CalculateRange();
    range.low.z = 0.0; // make sure we're exactly on the sheet.
    Transform location = Transform::From(range.low);
    location.ScaleMatrixColumns(box.GetWidth(), box.GetHeight(), 1.0);
    SetLocation(location);

    // set a clip volume around view, in tile (NPC) coorindates. 0.0 -> 1.0 on both axes
    DPoint2d clipPts[5];
    memset(clipPts, 0, sizeof(clipPts)); 
    clipPts[1].x = clipPts[2].x = clipPts[2].y = clipPts[3].y = 1.0;
    m_clip = new ClipVector(ClipPrimitive::CreateFromShape(clipPts, 5, false, nullptr, nullptr, nullptr).get());

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

    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, model->GetModelId());

    // If we're already loaded, look in existing list so we don't reload them
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        auto tree = FindAttachment(attachId);

        if (!tree.IsValid())
            tree = new Tree(GetDgnDb(), *this, attachId, 512);

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
* @bsimethod                                    Keith.Bentley                   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_DrawView(ViewContextR context)
    {
    auto model = GetViewedModel();
    if (nullptr == model)
        return;

    context.VisitDgnModel(*model);

    if (DrawPurpose::Pick != context.GetDrawPurpose())
        return;

    for (auto& attach : m_attachments)
        {
        if (attach->Pick((PickContext&)context))
            return;
        }
    }
