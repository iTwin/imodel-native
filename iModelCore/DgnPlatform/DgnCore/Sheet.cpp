/*-------------------------------------------------------------------------------------+
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
END_SHEET_NAMESPACE

USING_NAMESPACE_TILETREE
USING_NAMESPACE_SHEET
using namespace Attachment;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Sheet::Element::CreateCode(DocumentListModelCR model, Utf8StringCR name)
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
ElementPtr Sheet::Element::Create(DocumentListModelCR model, double scale, DPoint2dCR size, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(Handlers::Element::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || name.empty())
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
ElementPtr Sheet::Element::Create(DocumentListModelCR model, double scale, DgnElementId sheetTemplate, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();       
    DgnClassId classId = db.Domains().GetClassId(Handlers::Element::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || name.empty())
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
ViewAttachment::ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement) : T_Super(CreateParams(db, model, QueryClassId(db), cat, placement))
    {
    SetAttachedViewId(viewId);
    SetCode(GenerateDefaultCode());
    SetScale(ComputeScale(db, viewId, placement.GetElementBox()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewAttachment::ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, DPoint2dCR origin, double scale) : T_Super(CreateParams(db, model, QueryClassId(db), cat, ComputePlacement(db, viewId, origin, scale)))
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
    placement.SetOrigin(origin);

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

    return GetModel()->IsSheetModel() ? DgnDbStatus::Success : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ViewAttachment::GetClip() const
    {
    return m_jsonProperties.isMember(json_clip()) ? ClipVector::FromJson(m_jsonProperties[json_clip()]) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachment::SetClip(ClipVectorCR clipVector)
    {
    m_jsonProperties[json_clip()] = clipVector.ToJson();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                 Ramanujam.Raman   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewAttachment::ClearClip()
    {
    m_jsonProperties.removeMember(json_clip());
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
    auto system = GetRenderSystem();
    auto graphic = system->_CreateGraphic(GraphicBuilder::CreateParams::World(tree.GetDgnDb()));

    graphic->SetSymbology(tree.m_tileColor, tree.m_tileColor, 0); // this is to set transparency
    graphic->AddTile(*m_texture, tile.m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

#if defined (DEBUG_TILES)
    graphic->SetSymbology(ColorDef::DarkOrange(), ColorDef::Green(), 0);
    graphic->AddRangeBox(tile.m_range);
#endif

    tile.m_graphic = graphic->Finish();
    BeAssert(tile.m_graphic.IsValid());

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

    m_rootTile = m_viewport->GetViewControllerR().IsSpatialView() ? 
            (QuadTree::Tile*) new Tile(*this, QuadTree::TileId(0,0,0), nullptr) : new Tile2dModel(*this, QuadTree::TileId(0,0,0), nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment::Tile2dModel::_DrawGraphics(TileTree::DrawArgsR args) const 
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
        branch.SetViewFlagsOverrides(ViewFlagsOverrides(vp->GetViewFlags()));

        for (auto& graphic : *scene)
            branch.Add(*graphic);
        
        Transform toNpc;
        toNpc.InitFrom(*vp->GetWorldToNpcMap(), false);
        toNpc.form3d[2][2] = 1.0;
        toNpc.form3d[2][3] = 0;

        m_graphic = args.m_context.CreateBranch(branch, vp->GetViewControllerR().GetDgnDb(), toNpc, nullptr);
        }

    args.m_graphics.Add(*m_graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment::Tree::Draw(SceneContext& context)
    {
    Load(&context.GetTargetR().GetSystem());

    // before we can draw a ViewAttachment tree, we need to request that its scene be created.
    if (!m_sceneQueued)
        {
        // ###TODO_ELEMENT_TILE: Do we need/want the UpdatePlan?
        m_viewport->_QueueScene(context.GetUpdatePlan()); // this usually queues the scene request on the SceneThread and returns immediately
        m_sceneQueued = true; // remember that we've already queued it
        }

    // the scene is available, draw its tiles
    DrawInView(context);

#ifdef DEBUG_ATTACHMENT_RANGE
    ElementAlignedBox3d range(0,0,0, 1.0/m_scale.x,1.0/m_scale.y,1.0);
    GetLocation().Multiply(&range.low, &range.low, 2);

    Render::GraphicBuilderPtr graphicBbox = context.CreateGraphic();
    graphicBbox->SetSymbology(ColorDef::Green(), ColorDef::Green(), 2, LinePixels::Code5);
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

    if (m_clip.IsValid())
        {
        Frustum frust = context.GetFrustum();   // this frustum is the pick aperture

        for (auto& primitive : *m_clip)
            {
            if (ClipPlaneContainment_StronglyOutside == primitive->ClassifyFrustum(frust))
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
    RectanglePoints(double xlow, double ylow, double xhigh, double yhigh) 
        {
        m_pts[0].x = m_pts[3].x = m_pts[4].x = xlow;
        m_pts[0].y = m_pts[1].y = m_pts[4].y = ylow;
        m_pts[1].x = m_pts[2].x = xhigh; 
        m_pts[2].y = m_pts[3].y = yhigh;
        
        }
    operator DPoint2dP() {return m_pts;}
    operator DPoint2dCP() const {return m_pts;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
Attachment::Tree::Tree(DgnDbR db, Sheet::ViewController& sheetController, DgnElementId attachmentId, uint32_t tileSize) : 
                T_Super(db,Transform::FromIdentity(), nullptr, nullptr, 12, tileSize), m_attachmentId(attachmentId), m_pixels(tileSize)
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

    auto& def = view->GetViewDefinitionR();
    auto& style = def.GetDisplayStyle();

    // override the background color. This is to match V8, but there should probably be an option in the "Details" about whether to do this or not.
    style.SetBackgroundColor(sheetController.GetViewDefinitionR().GetDisplayStyle().GetBackgroundColor());

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

    // set a clip volume around view, so we only show the original volume
    m_clip = attach->GetClip();
    if (!m_clip.IsValid())
        m_clip = new ClipVector(ClipPrimitive::CreateFromShape(RectanglePoints(range.low.x, range.low.y, range.high.x, range.high.y), 5, false, nullptr, nullptr, nullptr).get());

    m_viewport->m_clips = m_clip;

    SetExpirationTime(BeDuration::Seconds(5)); // only save unused sheet tiles for 5 seconds

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
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> Sheet::Model::GetSheetAttachmentIds() const
    {
    bvector<DgnElementId>   attachIds;
    // Scan for viewAttachments...
    auto stmt =  GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, GetModelId());

    while (BE_SQLITE_ROW == stmt->Step())
        attachIds.push_back(stmt->GetValueId<DgnElementId>(0));

    return attachIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ViewDefinitionCPtr> Sheet::Model::GetSheetAttachmentViews(DgnDbR db) const
    {
    bvector<DgnElementId> attachmentIds = GetSheetAttachmentIds();
    bvector<ViewDefinitionCPtr> attachmentViews;

    for (auto& attachmentId : attachmentIds)
        {
        auto attachmentElement = GetDgnDb().Elements().Get<Sheet::ViewAttachment>(attachmentId);
        if (!attachmentElement.IsValid())
            {
            BeAssert(false);
            continue;
            }
        
        auto viewDefinition = GetDgnDb().Elements().Get<ViewDefinition>(attachmentElement->GetAttachedViewId());
        if (!viewDefinition.IsValid())
            {
            BeAssert(false);
            continue;
            }
        attachmentViews.push_back(viewDefinition);
        }

    return attachmentViews;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                     04/
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d Sheet::Model::GetSheetSize() const
    {
    // Get the Sheet::Element to extract the sheet size
    auto sheetElement = GetDgnDb().Elements().Get<Sheet::Element>(GetModeledElementId());
    if (!sheetElement.IsValid())
        {
        BeAssert(false); // this is fatal
        return DPoint2d::From(0.0, 0.0);
        }
    
    return DPoint2d::From(sheetElement->GetWidth(), sheetElement->GetHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Sheet::Model::GetSheetExtents() const
    {
    DPoint2d size = GetSheetSize();
    return AxisAlignedBox3d(DPoint3d::FromZero(), DPoint3d::From(size.x, size.y, 0.0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_LoadState()
    {
    auto model = GetViewedModel();  // get the sheet model for this view
    if (nullptr == model || nullptr == model->ToSheetModel())
        {
        BeAssert(false); // what happened?
        return;
        }
    m_size = model->ToSheetModel()->GetSheetSize();

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
Render::GraphicPtr Sheet::Model::CreateBorder(ViewContextR context, DPoint2dCR size)
    {
    Render::GraphicBuilderPtr border = context.CreateWorldGraphic();
    RectanglePoints rect(0, 0, size.x, size.y);
    border->SetSymbology(ColorDef::Black(), ColorDef::Black(), 2, LinePixels::Solid);
    border->AddLineString2d(5, rect, 0.0);

    double shadowWidth = .01 * size.Distance(DPoint2d::FromZero());
    double keyValues[] = {0.0, 0.5};
    ColorDef keyColors[] = {ColorDef(25,25,25), ColorDef(150,150,150)};

    DPoint2d points[7];
    points[0].y = points[1].y = points[6].y = 0.0;
    points[0].x = shadowWidth;
    points[1].x = points[2].x = size.x;
    points[3].x = points[4].x = size.x + shadowWidth;
    points[2].y = points[3].y = size.y - shadowWidth;
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

    // Make sure drop shadow displays behind border...
    int32_t priority = Render::Target::GetMinDisplayPriority();
    border->AddShape2d(7, points, true, static_cast<double>(priority));

    return border->Finish();
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Sheet::ViewController::_CreateScene(SceneContextR context)
    {
    auto status = T_Super::_CreateScene(context);
    for (auto& attach : m_attachments)
        attach->Draw(context);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_DrawDecorations(DecorateContextR context)
    {
    // On the trunk, the border is 'terrain', so gradient not affected by view's 'fill' flag.
    // Draw it as a decoration in tile world.
    auto border = Sheet::Model::CreateBorder(context, m_size);
    context.AddWorldDecoration(*border);
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
    context.ExtendFitRange(AxisAlignedBox3d(DPoint3d::FromZero(), DPoint3d::From(m_size.x,m_size.y,0.0)));
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId Sheet::Model::FindFirstViewOfSheet(DgnDbR db, DgnModelId mid)
    {
    auto findViewOfSheet = db.GetPreparedECSqlStatement("SELECT sheetView.ECInstanceId FROM bis.SheetViewDefinition sheetView WHERE (sheetView.BaseModel.Id=?)");
    findViewOfSheet->BindId(1, mid);
    return BE_SQLITE_ROW != findViewOfSheet->Step() ?  DgnElementId() : findViewOfSheet->GetValueId<DgnElementId>(0);
    }

#if defined (DEBUG_SHEETS)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void printIndent(int indent)
    {
    for (int i=0; i<indent; ++i)
        printf("  ");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpModel(DgnModel const& model, int indent)
    {
    printIndent(indent);
    printf("Mdl: [%s] (%lld)\n", model.GetName().c_str(), model.GetModelId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String fmtPoint2d(DPoint2dCR pt)
    {
    return Utf8PrintfString("(%lg,%lg)", pt.x, pt.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String fmtAngle(double a)
    {
    return Utf8PrintfString("%lg", a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpViewDefinition2d(ViewDefinition2d const& viewDef, int indent)
    {
    printIndent(indent);
    printf("VDef2d: [%s] (%lld) org=%s delta=%s rot=%s\n", viewDef.GetCode().GetValue().c_str(), viewDef.GetElementId().GetValueUnchecked(),
                fmtPoint2d(viewDef.GetOrigin2d()).c_str(),
                fmtPoint2d((DPoint2dCR)viewDef.GetDelta2d()).c_str(),
                fmtAngle(viewDef.GetRotAngle()).c_str());
    auto& db = viewDef.GetDgnDb();
    auto baseModel = db.Models().GetModel(viewDef.GetBaseModelId());
    if (!baseModel.IsValid())
        {
        printIndent(indent+1);
        printf("%lld -- broken link?!\n", viewDef.GetBaseModelId().GetValueUnchecked());
        }
    else
        {
        dumpModel(*baseModel, indent+1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpViewDefinition(ViewDefinition const& viewDef, int indent)
    {
    auto view2d = viewDef.ToView2d();
    if (nullptr != view2d)
        {
        dumpViewDefinition2d(*view2d, indent);
        return;
        }
    printIndent(indent);
    printf("VDef [%s] (%lld)\n", viewDef.GetCode().GetValue().c_str(), viewDef.GetElementId().GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpSheetAttachment(Sheet::ViewAttachment const& attachment, int indent)
    {
    printIndent(indent);
    printf("VA: %lld\n", attachment.GetElementId().GetValueUnchecked());

    auto& db = attachment.GetDgnDb();
    auto viewDef = db.Elements().Get<ViewDefinition>(attachment.GetAttachedViewId());
    if (!viewDef.IsValid())
        {
        printIndent(indent+1);
        printf("%lld -- broken link?!\n", attachment.GetAttachedViewId().GetValueUnchecked());
        }
    else
        {
        dumpViewDefinition(*viewDef, indent+1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpSheetAttachments(Sheet::Model const& sheet, int indent)
    {
    dumpModel(sheet, indent);

    auto& db = sheet.GetDgnDb();
    auto stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, sheet.GetModelId());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attElm = db.Elements().Get<Sheet::ViewAttachment>(stmt->GetValueId<DgnElementId>(0));
        if (!attElm.IsValid())
            {
            printIndent(indent+1);
            printf("%lld -- broken link?!\n", stmt->GetValueId<DgnElementId>(0).GetValueUnchecked());
            }
        else
            {
            dumpSheetAttachment(*attElm, indent+1);
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Model::DumpAttachments(int indent)
    {
#if defined (DEBUG_SHEETS)
    dumpSheetAttachments(*this, indent);
#endif
    }
