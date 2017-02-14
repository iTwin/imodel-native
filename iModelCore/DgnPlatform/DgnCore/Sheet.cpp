/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sheet.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
namespace SheetStrings
{
static Utf8CP str_Clip() {return "Clip";}
};
END_SHEET_NAMESPACE

USING_NAMESPACE_TILETREE
USING_NAMESPACE_SHEET
using namespace Attachment;
using namespace SheetStrings;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Sheet::Element::CreateCode(DocumentListModelCR model, Utf8CP name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_Sheet, *model.GetModeledElement(), name);
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
ElementPtr Sheet::Element::Create(DocumentListModelCR model, double scale, DPoint2dCR size, Utf8CP name)
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
    sheet->SetWidth(size.x);
    sheet->SetHeight(size.y);
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

    return viewDef->GetExtents().x / placement.GetWidth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::CheckValid() const
    {
    if (!GetAttachedViewId().IsValid())
        return DgnDbStatus::ViewNotFound;

    if (!GetModel()->IsSheetModel())
        return DgnDbStatus::WrongModel;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ViewAttachment::GetClip() const
    {
    auto clipJsonStr = GetPropertyValueString(str_Clip());
    if (clipJsonStr.empty())
        return nullptr;

    Json::Value clipJson(Json::arrayValue);
    if (!Json::Reader::Parse(clipJsonStr, clipJson))
        return nullptr;

    return ClipVector::FromJson(clipJson);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewAttachment::SetClip(ClipVectorCR clipVector)
    {
    Json::Value clipJson = clipVector.ToJson();
    return SetPropertyValue(str_Clip(), Json::FastWriter::ToString(clipJson).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachment::ClearClip()
    {
    SetPropertyValue(str_Clip(), ECValue());
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
    double tileSize = 1.0/ (1 << id.m_level); // the size of a tile for this level, in NPC
    double east  = id.m_column * tileSize;
    double west  = east + tileSize;
    double north = id.m_row * tileSize;
    double south = north + tileSize;

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
    BeAssert(m_viewport.IsValid());

    static bool s_useTiles = false; // debugging - to be removed.
    if (s_useTiles)
        {
        m_rootTile = new Tile(*this, QuadTree::TileId(0,0,0), nullptr);
        }
    else
        {
        m_rootTile = m_viewport->GetViewControllerR().IsSpatialView() ? 
            (QuadTree::Tile*) new Tile(*this, QuadTree::TileId(0,0,0), nullptr) : new Tile2dModel(*this, QuadTree::TileId(0,0,0), nullptr);
        }
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
    ProgressiveTask::Completion _DoProgressive(RenderListContext& context, WantShow& showFrame) override
        {
        // is the scene available yet?
        if (!m_tree.m_viewport->GetViewControllerR().UseReadyScene().IsValid())
            return ProgressiveTask::Completion::Aborted; // no, keep waiting

        m_tree.m_sceneReady = true; // yes, mark it as ready and draw its tiles
        m_tree.DrawInView(context, m_tree.GetLocation(), m_tree.m_clip.get());
        return ProgressiveTask::Completion::Finished; // we're done.
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment::Tile2dModel::_DrawGraphics(TileTree::DrawArgsR args, int depth) const 
    {
    if (!m_graphic.IsValid())
        {
        auto vp = GetTree().m_viewport;
        auto scene = vp->GetViewControllerR().GetScene();
        if (!scene.IsValid())
            {
            BeAssert(false);
            return;
            }

        GraphicBranch branch;
        branch.SetViewFlags(vp->GetViewFlags());

        for (auto& graphic : scene->m_graphics->m_list)
            branch.Add(*graphic.m_ptr);
        
        Transform toNpc;
        toNpc.InitFrom(*vp->GetWorldToNpcMap(), false);
        m_graphic = args.m_context.CreateBranch(branch, &toNpc, nullptr);
        }

    args.m_graphics.Add(*m_graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment::Tree::Draw(TerrainContextR context)
    {
    Load(&context.GetTargetR().GetSystem());

    // before we can draw a ViewAttachment tree, we need to request that its scene be created.
    if (!m_sceneQueued)
        {
        m_viewport->_QueueScene(context.GetUpdatePlan()); // this queues the scene request on the SceneThread and returns immediately
        m_sceneQueued = true; // remember that we've already queued it
        m_sceneReady = m_viewport->GetViewControllerR().UseReadyScene().IsValid(); // happens if updatePlan asks to wait (_QueueScene actually created the scene).
        }

    if (!m_sceneReady) // if the scene isn't ready yet, we need to wait for it to finish.
        {
        context.GetViewport()->ScheduleProgressiveTask(*new SceneReadyTask(*this));
        return;
        }
    
    // the scene is available, draw its tiles
    DrawInView(context, GetLocation(), m_clip.get());

#ifdef DEBUG_ATTACHMENT_RANGE
    ElementAlignedBox3d range(0,0,0, 1.0/m_scale.x,1.0/m_scale.y,1.0);
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
    if (context.WasAborted())
        return true;

    if (!m_sceneReady) // we can't pick anything unless we have a valid scene.
        return false;

    Transform sheetToTile;
    sheetToTile.InverseOf(GetLocation());
    Frustum box = context.GetFrustum().TransformBy(sheetToTile);   // this frustum is the pick aperture

    if (m_clip.IsValid())
        {
        for (auto& primitive : *m_clip)
            {
            if (ClipPlaneContainment_StronglyOutside == primitive->ClassifyPointContainment(box.m_pts, NPC_CORNER_COUNT, false))
                return false;
            }
        }

    return context._ProcessSheetAttachment(*m_viewport);
    }

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/17
//=======================================================================================
struct RectanglePoints
{
    DPoint2d m_pts[5];
    RectanglePoints(double x, double y) {memset(m_pts, 0, sizeof(m_pts)); m_pts[1].x=m_pts[2].x=x; m_pts[2].y=m_pts[3].y=y;}
    operator DPoint2dP() {return m_pts;}
    operator DPoint2dCP() const {return m_pts;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Attachment::Tree::Tree(DgnDbR db, Sheet::ViewController& sheetController, DgnElementId attachmentId, uint32_t tileSize) : 
                T_Super(db,Transform::FromIdentity(), "", nullptr, 12, tileSize), m_attachmentId(attachmentId), m_pixels(tileSize)
    {
    auto attach = db.Elements().Get<ViewAttachment>(attachmentId);
    if (!attach.IsValid())
        {
        BeAssert(false);
        return;
        }

    m_viewport = T_HOST._CreateSheetAttachViewport();
    if (!m_viewport.IsValid())
        return;

    auto viewId = attach->GetAttachedViewId();
    auto view = ViewDefinition::LoadViewController(viewId, db);
    if (!view.IsValid())
        return;

    // we use square tiles. If the view's aspect ratio isn't square, expand the short side in tile (NPC) space. We'll clip out the extra area below.
    double aspect = view->GetViewDefinition().GetAspectRatio();
    if (aspect<1.0)
        m_scale.Init(1.0/aspect, 1.0);
    else
        m_scale.Init(1.0, aspect);

    // now expand the frustum in one direction so that the view is square (so we can use square tiles)
    m_viewport->SetRect(BSIRect::From(0, 0, m_pixels, m_pixels));
    m_viewport->ChangeViewController(*view);

    auto& def = view->GetViewDefinition();
    auto& style = def.GetDisplayStyle();

    // override the background color. This is to match V8, but there should probably be an option in the "Details" about whether to do this or not.
    style.SetBackgroundColor(sheetController.GetViewDefinition().GetDisplayStyle().GetBackgroundColor());

    SpatialViewDefinitionP spatial=def.ToSpatialViewP();
    if (spatial)
        {
        auto& env = spatial->GetDisplayStyle3d().GetEnvironmentDisplayR();
        env.m_groundPlane.m_enabled = false;
        env.m_skybox.m_enabled = false;
        }

    m_viewport->SetupFromViewController();
    Frustum frust = m_viewport->GetFrustum(DgnCoordSystem::Npc).TransformBy(Transform::FromScaleFactors(m_scale.x, m_scale.y, 1.0));
    m_viewport->NpcToWorld(frust.m_pts, frust.m_pts, NPC_CORNER_COUNT);
    m_viewport->SetupFromFrustum(frust);

    // max pixel size is half the length of the diagonal.
    m_maxPixelSize = .5 * DPoint2d::FromZero().Distance(DPoint2d::From(m_pixels, m_pixels));

    auto& box = attach->GetPlacement().GetElementBox();
    AxisAlignedBox3d range = attach->GetPlacement().CalculateRange();

    DPoint3d org = range.low;
    org.z = 0.0;
    Transform trans = Transform::From(org);
    trans.ScaleMatrixColumns(box.GetWidth() * m_scale.x, box.GetHeight() * m_scale.y, 1.0);
    SetLocation(trans);

    bsiTransform_initFromRange(&m_viewport->m_toParent, nullptr, &range.low, &range.high);
    m_viewport->m_toParent.ScaleMatrixColumns(m_scale.x, m_scale.y, 1.0);

    // set a clip volume around view, in tile (NPC) coorindates so we only show the original volume
    m_clip = new ClipVector(ClipPrimitive::CreateFromShape(RectanglePoints(1.0/m_scale.x, 1.0/m_scale.y), 5, false, nullptr, nullptr, nullptr).get());
    
    auto attachClip = attach->GetClip();
    if (attachClip.IsValid())
        {
        Transform sheetToNpc;
        sheetToNpc.InverseOf(m_viewport->m_toParent);
        attachClip->TransformInPlace(sheetToNpc);
        m_clip->Append(*attachClip);
        }

    m_viewport->m_attachClips = m_clip->Clone(&trans); // save so we can get it to for hiliting.

    SetExpirationTime(BeDuration::Seconds(5)); // only save unused sheet tiles for 5 seconds

    m_biasDistance = Render::Target::DepthFromDisplayPriority(attach->GetDisplayPriority());
    m_viewport->m_biasDistance = m_biasDistance; // for flashing hits

    m_hiResBiasDistance = Render::Target::DepthFromDisplayPriority(-1);
    m_loResBiasDistance = m_hiResBiasDistance * 2.0;
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
    auto model = GetViewedModel();  // get the sheet model for this view
    if (nullptr == model)
        {
        BeAssert(false); // what happened?
        return;
        }

    // Get the Sheet::Element to extract the sheet size
    auto sheetElement = GetDgnDb().Elements().Get<Sheet::Element>(model->GetModeledElementId());
    if (!sheetElement.IsValid())
        {
        BeAssert(false); // this is fatal
        return;
        }
    
    // save the sheet size in this ViewController
    m_size.Init(sheetElement->GetWidth(), sheetElement->GetHeight());

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
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::DrawBorder(ViewContextR context) const
    {
    Render::GraphicBuilderPtr border = context.CreateGraphic();
    RectanglePoints rect(m_size.x, m_size.y);
    border->SetSymbology(ColorDef::Black(), ColorDef::Black(), 2, GraphicParams::LinePixels::Solid);
    border->AddLineString2d(5, rect, 0.0);

    double shadowWidth = .01 * m_size.Distance(DPoint2d::FromZero());
    double keyValues[] = {0.0, 0.5};
    ColorDef keyColors[] = {ColorDef(25,25,25), ColorDef(150,150,150)};

    DPoint2d points[7];
    points[0].y = points[1].y = points[6].y = 0.0;
    points[0].x = shadowWidth;
    points[1].x = points[2].x = m_size.x;
    points[3].x = points[4].x = m_size.x + shadowWidth;
    points[2].y = points[3].y = m_size.y - shadowWidth;
    points[4].y = points[5].y = -shadowWidth;
    points[5].x = points[6].x = shadowWidth;

    GradientSymbPtr gradient = GradientSymb::Create();
    gradient->SetMode(Render::GradientSymb::Mode::Linear);
    gradient->SetAngle(-45.0);
    gradient->SetKeys(2, keyColors, keyValues);
 
    GraphicParams params;
    params.SetLineColor(keyColors[0]);
    params.SetGradient(gradient.get());
    border->ActivateGraphicParams(params);

    border->AddShape2d(7, points, true, 0.0);
    context.OutputGraphic(*border, nullptr);
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

    DrawBorder(context);
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
        attach->Pick((PickContext&)context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::ViewController::FitComplete Sheet::ViewController::_ComputeFitRange(FitContextR context) 
    {
    context.ExtendFitRange(GetSheetExtents());
    return FitComplete::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Transform Viewport::GetTransformToSheet(DgnViewportCR sheetVp)
    {
    Frustum frust = GetFrustum(DgnCoordSystem::Npc).TransformBy(m_toParent);
    sheetVp.WorldToView(frust.m_pts, frust.m_pts, NPC_CORNER_COUNT);
    Transform tileToSheet = Transform::From4Points(frust.m_pts[NPC_LeftTopRear], frust.m_pts[NPC_RightTopRear], frust.m_pts[NPC_LeftBottomRear], frust.m_pts[NPC_LeftTopFront]);
    tileToSheet.ScaleMatrixColumns(1.0/m_rect.corner.x, 1.0/m_rect.corner.y, 1.0);

    tileToSheet.form3d[2][2] = 1.0; // always make 1 : 1  in z
    tileToSheet.form3d[2][3] = 0.0;
    return tileToSheet;
    }

