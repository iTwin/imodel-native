/*-------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sheet.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   02/17
//=======================================================================================
struct RectanglePoints
{
    DPoint2d m_pts[5]; // view coords

    RectanglePoints(double xlow, double ylow, double xhigh, double yhigh) 
        {
        m_pts[0].x = m_pts[3].x = m_pts[4].x = xlow;
        m_pts[0].y = m_pts[1].y = m_pts[4].y = ylow;
        m_pts[1].x = m_pts[2].x = xhigh; 
        m_pts[2].y = m_pts[3].y = yhigh;
        }

    // Inputs in world coords
    RectanglePoints(double xlow, double ylow, double xhigh, double yhigh, ViewContextCR context)
        {
        DPoint3d pts[5];
        pts[0].x = pts[3].x = pts[4].x = xlow;
        pts[0].y = pts[1].y = pts[4].y = ylow;
        pts[1].x = pts[2].x = xhigh; 
        pts[2].y = pts[3].y = yhigh;
        
        context.WorldToView(m_pts, pts, 5);
        }

    operator DPoint2dP() {return m_pts;}
    operator DPoint2dCP() const {return m_pts;}
};

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
    m_attachments.clear();

    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, model->GetModelId());

    // If we're already loaded, look in existing list so we don't reload them
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        m_attachments.push_back(Attachment(attachId));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr Sheet::Model::CreateBorder(DecorateContextR context, DPoint2dCR size)
    {
    Render::GraphicBuilderPtr border = context.CreateViewBackground();
    RectanglePoints rect(0, 0, size.x, size.y, context);
    border->SetSymbology(ColorDef::Black(), ColorDef::Black(), 2, LinePixels::Solid);
    border->AddLineString2d(5, rect, 0.0);

    double shadowWidth = .01 * size.Distance(DPoint2d::FromZero());

    DPoint3d points[7];
    points[0].y = points[1].y = points[6].y = 0.0;
    points[0].x = shadowWidth;
    points[1].x = points[2].x = size.x;
    points[3].x = points[4].x = size.x + shadowWidth;
    points[2].y = points[3].y = size.y - shadowWidth;
    points[4].y = points[5].y = -shadowWidth;
    points[5].x = points[6].x = shadowWidth;
    for (auto& point : points)
        point.z =0.0;

    DPoint2d shadowPoints[7];
    context.WorldToView(shadowPoints, points, 7);

    double keyValues[] = {0.0, 0.5};
    ColorDef keyColors[] = {ColorDef(25,25,25), ColorDef(150,150,150)};

    GradientSymbPtr gradient = GradientSymb::Create();
    gradient->SetMode(Render::GradientSymb::Mode::Linear);
    gradient->SetAngle(-45.0);
    gradient->SetKeys(2, keyColors, keyValues);
 
    GraphicParams params;
    params.SetLineColor(keyColors[0]);
    params.SetGradient(gradient.get());
    border->ActivateGraphicParams(params);

    // Make sure drop shadow displays behind border...
    border->AddShape2d(7, shadowPoints, true, Render::Target::Get2dFrustumDepth());

    return border->Finish();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser   02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::TTile::SetupRange()
    {
    static const double s_tileSize = 1.0;

    double east  = 0.0;
    double west  = east + s_tileSize;
    double north = 0.0;
    double south = north + s_tileSize;

    m_corners.m_pts[0].Init(east, north, GetTree().m_biasDistance); 
    m_corners.m_pts[1].Init(west, north, GetTree().m_biasDistance); 
    m_corners.m_pts[2].Init(east, south, GetTree().m_biasDistance); 
    m_corners.m_pts[3].Init(west, south, GetTree().m_biasDistance); 
    m_range.InitFrom(m_corners.m_pts, 4);
    }

#define MAX_SHEET_REFINE_DEPTH 7

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sheet::Attachment::TTile::_HasChildren() const // { return false; }
    { // this method actually means "I have children I may need to create" not "I currently have children in my m_children list".
    return GetDepth() < MAX_SHEET_REFINE_DEPTH ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::ChildTiles const* Sheet::Attachment::TTile::_GetChildren(bool load) const
    {
    if (GetDepth() + 1 < MAX_SHEET_REFINE_DEPTH && m_children.empty() && load)
        {
        TilePtr childTile = new TTile(GetTree(), (Sheet::Attachment::TTile* const)this);
        m_children.push_back(childTile);
        }
    return m_children.empty() ? nullptr : &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void populateSheetTile(TTile* tile, uint32_t depth, SceneContextR context)
    {
    static const uint32_t s_texSizes[] = {32, 64, 128, 256, 512, 1024, 2048};
    uint32_t texSize = s_texSizes[depth < MAX_SHEET_REFINE_DEPTH ? depth : MAX_SHEET_REFINE_DEPTH - 1];

    auto renderSys = context.GetRenderSystem();
    TRoot& tree = tile->GetTree();
    Sheet::Attachment::Viewport* viewport = tree.m_viewport.get();
    UpdatePlan const& plan = context.GetUpdatePlan();

    // max pixel size is half the length of the diagonal.
    tile->m_maxPixelSize = .5 * DPoint2d::FromZero().Distance(DPoint2d::From(texSize, texSize));

    viewport->InvalidateRenderPlan();
    viewport->SetRect(BSIRect::From(0, 0, texSize, texSize));
    viewport->SetupFromViewController();
    viewport->m_renderSys = renderSys;
    viewport->m_db = &context.GetDgnDb();
    viewport->m_texSize = texSize;
    viewport->_CreateScene(plan); // view controller for a drawing or spatial view, make a context in dgnview (renderthumbnail)
    // this will sit and wait; tile will be ready when here (RenderThumbnail path)

    // add texture in the tree's viewport to a graphic
    GraphicParams gfParams = GraphicParams::FromSymbology(tree.m_tileColor, tree.m_tileColor, 0);
    tile->SetupRange(); // set up tile corners and range; uses biasDistance of tree
    auto graphic = renderSys->_CreateTile(*viewport->m_texture, tile->m_corners, *viewport->m_db, gfParams);
    tile->m_graphic = graphic;
    tile->SetIsReady();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Sheet::ViewController::_CreateScene(SceneContextR context)
    {
    if (SUCCESS != T_Super::_CreateScene(context))
        {
        return ERROR;
        }

    if (!m_allAttachmentsLoaded)
        {
        size_t i = 0;
        while (i < m_attachments.size())
            {
            Attachment& attach = m_attachments[i];
            if (!attach.m_tree.IsValid())
                {
                const uint32_t texSize = 32; // based on lowest level in populateSheetTile; ###TODO: clean this up (make more common)

                attach.m_tree = new TRoot(GetDgnDb(), *this, attach.m_id, texSize);
                attach.m_tree->Populate();

                if (!attach.m_tree.IsValid())
                    m_attachments.erase(m_attachments.begin() + i);
                else
                    i++;

                TTile& tile = static_cast<TTile&>(*attach.m_tree->GetRootTile());
                populateSheetTile(&tile, tile.GetDepth(), context);
                }
            }

        m_allAttachmentsLoaded = true;
        }

    for (auto& attach : m_attachments)
        {
        if (!attach.m_tree.IsValid())// || !attach.m_tree->GetRootTile()->IsReady())
            continue;

        attach.m_tree->DrawInView(context);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_DrawDecorations(DecorateContextR context)
    {
    auto border = Sheet::Model::CreateBorder(context, m_size);
    context.SetViewBackground(*border);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Tile::SelectParent TTile::_SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const
    {
    BeAssert(selected.empty());

    Visibility vis = GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        return SelectParent::No;
        }

    bool tooCoarse = Visibility::TooCoarse == vis;
    TileTree::Tile::ChildTiles const* childTiles = _GetChildren(true);

    TTile* child = nullptr;

    if (nullptr != childTiles && !childTiles->empty())
        {
        child = (TTile*) ((*childTiles)[0].get()); // there should only be a single child
        if (!child->IsReady()) // ###TODO: only populate child if about to be selected; need way to specify pixel size without making entire graphic
            {
            populateSheetTile(child, child->GetDepth(), args.m_context);
            }
        }

    if (tooCoarse && nullptr != child)
        {
        child->_SelectTiles(selected, args);
        return SelectParent::No;
        }

// #define DEBUG_PRINT_SHEET_TILE_SELECTION
#if defined(DEBUG_PRINT_SHEET_TILE_SELECTION)
    DEBUG_PRINTF(" ** Selecting this tile, IsReady()=%d, depth=%d", IsReady(), GetDepth());
#endif

    selected.push_back(this);
    return SelectParent::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TTile::_DrawGraphics(DrawArgsR args) const
    {
    BeAssert(IsReady());
    if (m_graphic.IsValid())
        args.m_graphics.Add(*m_graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TRoot::TRoot(DgnDbR db, Sheet::ViewController& sheetController, DgnElementId attachmentId, uint32_t tileSize) : 
                T_Super(db,Transform::FromIdentity(), nullptr, nullptr), m_attachmentId(attachmentId), m_pixels(tileSize)
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

#if 0 // ###TODO - needed?
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
#endif

    m_viewport->SetupFromViewController();
    Frustum frust = m_viewport->GetFrustum(DgnCoordSystem::Npc).TransformBy(Transform::FromScaleFactors(m_scale.x, m_scale.y, 1.0));
    m_viewport->NpcToWorld(frust.m_pts, frust.m_pts, NPC_CORNER_COUNT);
    m_viewport->SetupFromFrustum(frust);

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

	int32_t biasDistance = Render::Target::DepthFromDisplayPriority(attach->GetDisplayPriority());
	m_biasDistance = double(biasDistance);
    m_viewport->m_clips = m_clip;

    SetExpirationTime(BeDuration::Seconds(5)); // only save unused sheet tiles for 5 seconds
    }

/*---------------------------------------------------------------------------------**//**
! Populates TRoot with a single child tile.
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void TRoot::Populate()
    {
    m_rootTile = new TTile(*this, nullptr);
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
    printf("VDef2d: [%s] (%lld) org=%s delta=%s rot=%s\n", viewDef.GetCode().GetValue().GetUtf8CP(), viewDef.GetElementId().GetValueUnchecked(),
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
    printf("VDef [%s] (%lld)\n", viewDef.GetCode().GetValue().GetUtf8CP(), viewDef.GetElementId().GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void dumpSheetAttachment(Sheet::ViewAttachment const& attachment, int indent)
    {
    printIndent(indent);
    printf("VA: %lld scale:%lf displayPriority:%d clip?:%d details:%s\n", attachment.GetElementId().GetValueUnchecked(),
           attachment.GetScale(), attachment.GetDisplayPriority(), attachment.GetClip().IsValid(), attachment.GetDetails());

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
void Sheet::Model::DumpAttachments(int indent) const
    {
#if defined (DEBUG_SHEETS)
    dumpSheetAttachments(*this, indent);
#endif
    }
