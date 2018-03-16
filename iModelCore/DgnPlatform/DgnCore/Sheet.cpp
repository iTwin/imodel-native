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
    template<typename T> static void Init(T& pts, double xlow, double ylow, double xhigh, double yhigh)
        {
        pts[0].x = pts[3].x = pts[4].x = xlow;
        pts[0].y = pts[1].y = pts[4].y = ylow;
        pts[1].x = pts[2].x = xhigh; 
        pts[2].y = pts[3].y = yhigh;
        }

    DPoint2d m_pts[5];

    // If context supplied, points will be transformed to view coords.
    RectanglePoints(double xlow, double ylow, double xhigh, double yhigh, ViewContextCP context=nullptr)
        {
        if (nullptr != context)
            {
            DPoint3d pts[5];
            Init(pts, xlow, ylow, xhigh, yhigh);
            for (auto& pt : pts)
                pt.z = 0.0;

            context->WorldToView(m_pts, pts, 5);
            }
        else
            {
            Init(m_pts, xlow, ylow, xhigh, yhigh);
            }
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
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::Attachments::Add(Sheet::ViewController::Attachment&& attach)
    {
    BeAssert(nullptr == Find(attach.GetId()));
    m_allAttachmentsLoaded = m_allAttachmentsLoaded && nullptr != attach.GetTree();
    m_list.push_back(std::move(attach));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sheet::ViewController::Attachments::LoadTree(size_t index, DgnDbR db, ViewController& sheetController, SceneContextR context)
    {
    BeAssert(index < size());
    if (index >= size())
        return false;

    auto& attach = m_list[index];
    if (nullptr != attach.GetTree())
        return true;

    bool loaded = attach.LoadTree(db, sheetController, context);
    if (!loaded)
        m_list.erase(m_list.begin() + index);

    UpdateAllLoaded();
    return loaded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::Attachments::UpdateAllLoaded()
    {
    m_allAttachmentsLoaded = true;
    for (auto const& attach : m_list)
        {
        if (nullptr == attach.GetTree())
            {
            m_allAttachmentsLoaded = false;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::Attachment::SetState(uint32_t depth, State state)
    {
    while (m_states.size() < depth+1)
        m_states.push_back(State::NotLoaded);

    m_states[depth] = state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sheet::ViewController::Attachment::LoadTree(DgnDbR db, Sheet::ViewController& sheetController, SceneContextR context)
    {
    if (!m_tree.IsValid())
        m_tree = Sheet::Attachment::Root::Create(db, sheetController, GetId(), context);

    return m_tree.IsValid();
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

    if (!WantRenderAttachments())
        return;

    Attachments attachments;
    auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, model->GetModelId());

    // If we're already loaded, look in existing list so we don't reload them
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        auto tree = FindAttachment(attachId);

        if (nullptr != tree)
            attachments.Add(std::move(*tree));
        else
            attachments.Add(Attachment(attachId));
        }

    // save new list of attachment
    m_attachments = std::move(attachments);

#ifdef DEBUG_SHEETS
    model->ToSheetModel()->DumpAttachments(0);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_OnRenderFrame()
    {
    BeAssert(nullptr != m_vp); // invoked by DgnViewport::RenderFrame()...
    if (!m_allAttachmentTilesReady || !m_attachments.AllLoaded())
        m_vp->InvalidateScene();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Border::Border(ViewContextCP context, DPoint2dCR size)
    {
    // Rect
    RectanglePoints rect(0, 0, size.x, size.y, context);
    memcpy(m_rect, rect.m_pts, 5 * sizeof(DPoint2d));

    // Shadow
    double shadowWidth = .01 * size.Distance(DPoint2d::FromZero());

    DPoint3d shadow[7];
    shadow[0].y = shadow[1].y = shadow[6].y = 0.0;
    shadow[0].x = shadowWidth;
    shadow[1].x = shadow[2].x = size.x;
    shadow[3].x = shadow[4].x = size.x + shadowWidth;
    shadow[2].y = shadow[3].y = size.y - shadowWidth;
    shadow[4].y = shadow[5].y = -shadowWidth;
    shadow[5].x = shadow[6].x = shadowWidth;

    if (nullptr != context)
        {
        for (auto& point : shadow)
            point.z = 0.0;

        context->WorldToView(m_shadow, shadow, 7);
        }
    else
        {
        for (size_t i = 0; i < 7; i++)
            {
            m_shadow[i].x = shadow[i].x;
            m_shadow[i].y = shadow[i].y;
            }
        }
            
    // Gradient
    double keyValues[] = {0.0, 0.5};
    ColorDef keyColors[] = {ColorDef(25,25,25), ColorDef(150,150,150)};

    m_gradient = GradientSymb::Create();
    m_gradient->SetMode(Render::GradientSymb::Mode::Linear);
    m_gradient->SetAngle(-45.0);
    m_gradient->SetKeys(2, keyColors, keyValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Border::AddToBuilder(Render::GraphicBuilderR builder) const
    {
    builder.SetSymbology(ColorDef::Black(), ColorDef::Black(), 2, LinePixels::Solid);
    builder.AddLineString2d(5, m_rect, 0.0);

    GraphicParams params;
    params.SetLineColor(ColorDef(25,25,25));
    params.SetGradient(m_gradient.get());

    builder.ActivateGraphicParams(params);
    builder.AddShape2d(7, m_shadow, true, Render::Target::Get2dFrustumDepth());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
DRange2d Sheet::Border::GetRange() const
    {
    DRange2d shadowRange = DRange2d::From(m_shadow, 7),
             range = DRange2d::From(m_rect, 5);
    range.Extend(shadowRange);
    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicPtr Sheet::Model::CreateBorder(DecorateContextR context, DPoint2dCR size)
    {
    Border border(context, size, Border::CoordSystem::View);
    Render::GraphicBuilderPtr builder = context.CreateViewBackground();
    border.AddToBuilder(*builder);
    return builder->Finish();
    }

#define MAX_SHEET_REFINE_DEPTH 6

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sheet::Attachment::Tile::_HasChildren() const // { return false; }
    { // this method actually means "I have children I may need to create" not "I currently have children in my m_children list".
    return GetDepth() < MAX_SHEET_REFINE_DEPTH ? true : false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::ChildTiles const* Sheet::Attachment::Tile::_GetChildren(bool load) const
    {
    if (GetDepth() + 1 < MAX_SHEET_REFINE_DEPTH && m_children.empty() && load)
        {
        TilePtr childTile = new Sheet::Attachment::Tile(GetTree(), this);
        m_children.push_back(childTile);
        }

    return m_children.empty() ? nullptr : &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void repairSheetPolys(bvector<PolyfaceHeaderPtr> polys, DRange3d polysRange)
    {
    double oldRangeX = polysRange.high.x - polysRange.low.x;
    double oldRangeY = polysRange.high.y - polysRange.low.y;
    double newRangeX = 1.0;
    double newRangeY = 1.0;
    for (auto& polyface : polys)
        {
        // repair UVs in each poly so they are placed in 0 to 1 space based on polysRange (range of entire list of polys)
        BlockedVectorDPoint2dR params = polyface->Param();
        for (auto& uv : params)
            {
            uv.x = ((uv.x - polysRange.low.x) * newRangeX) / oldRangeX;
            uv.y = ((uv.y - polysRange.low.y) * newRangeY) / oldRangeY;
            }

        // repair points in the same way
        BlockedVectorDPoint3dR pts = polyface->Point();
        for (auto& pt : pts)
            {
            pt.x = ((pt.x - polysRange.low.x) * newRangeX) / oldRangeX;
            pt.y = ((pt.y - polysRange.low.y) * newRangeY) / oldRangeY;
            pt.z = pt.z;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static uint32_t querySheetTileSize(uint32_t depth)
    {
    // ###TODO: can we base this on OpenGL capabilities so we don't rely on support for larger texture sizes if that's not reasonable?
    static const uint32_t s_texSizes[] = {32, 64, 128, 256, 512, 1024, 2048, 4096};
    return s_texSizes[depth < MAX_SHEET_REFINE_DEPTH ? depth : MAX_SHEET_REFINE_DEPTH - 1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Viewport::SetSceneDepth(uint32_t depth)
    {
    if (m_sceneDepth != depth)
        {
        // Discard any tiles/graphics used for previous level-of-detail - we'll generate them at the new LOD.
        InvalidateScene();
        m_viewController->_CancelAllTileLoads(false);
        m_viewController->_UnloadAllTileTrees();

        m_sceneDepth = depth;
        m_texSize = querySheetTileSize(depth);
        SetRect(BSIRect::From(0, 0, m_texSize, m_texSize)); // ###TODO: Make actual prev ratio of view
        }
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/18
//=======================================================================================
struct AutoRestoreFrustum
{
    Frustum m_frustum;
    DgnViewportR m_vp;

    explicit AutoRestoreFrustum(DgnViewportR vp) : m_frustum(vp.GetFrustum(DgnCoordSystem::World)), m_vp(vp) { }
    ~AutoRestoreFrustum() { m_vp.SetupFromFrustum(m_frustum); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Tile::CreateGraphics(SceneContextR context)
    {
    Root& tree = GetTree();
    auto currentState = GetState();

    // CreateGraphics() should only be called if the tile is not yet ready. An empty or ready state means tile should be considered ready.
    BeAssert(State::Empty != currentState && State::Ready != currentState);
    switch (currentState)
        {
        case State::Ready:
            SetIsReady();
            return;
        case State::Empty:
            SetNotFound();
            return;
        }

    UpdatePlan const& plan = context.GetUpdatePlan();

    auto renderSys = context.GetRenderSystem();

    Sheet::Attachment::Viewport* viewport = tree.m_viewport.get();
    viewport->SetSceneDepth(GetDepth());
    viewport->SetupFromViewController();
    viewport->m_renderSys = renderSys;
    viewport->m_db = &context.GetDgnDb();

    // Change frustum so it looks at only the visible (after clipping) portion of the scene.
    // Base this on tree.m_polysRange calculated by _CreateSheetTilePolys().
    AutoRestoreFrustum autoRestore(*viewport);

    Frustum frust = viewport->GetFrustum(DgnCoordSystem::Npc);
    DPoint3dP frustPts = frust.GetPtsP();
    tree.m_polysRange.Get8Corners(frustPts);
    DMap4dCP rootToNpc = viewport->GetWorldToNpcMap();
    rootToNpc->M1.MultiplyAndRenormalize(frustPts, frustPts, NPC_CORNER_COUNT);
    viewport->SetupFromFrustum(frust);

    // Create the scene, and if the scene is complete, render the offscreen texture
    auto newState = viewport->_CreateScene(plan, currentState);
    SetState(newState);

    switch (newState)
        {
        case State::NotLoaded:
        case State::Loading:
            return;
        case State::Ready:
            {
            // create graphics from the polys and the rendered texture
            // ###TODO: must determine whether to do this at all if there were no poly results
            GraphicParams gfParams = GraphicParams::FromSymbology(tree.m_tileColor, tree.m_tileColor, 0);
            m_graphics = renderSys->_CreateSheetTile(*viewport->m_texture, tree.m_tilePolys, *viewport->m_db, gfParams);
            SetIsReady();
            break;
            }
        case State::Empty:
            SetNotFound();
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sheet::ViewController::WantRenderAttachments()
    {
    return T_HOST._IsFeatureEnabled("Platform.RenderViewAttachments");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Sheet::ViewController::_CreateScene(SceneContextR context)
    {
    // This will be reset to false by the end of this function if any attachments are waiting on tiles...
    m_allAttachmentTilesReady = true;

    auto stat = T_Super::_CreateScene(context);
    if (SUCCESS != stat || !WantRenderAttachments())
        return stat;

    if (!m_attachments.AllLoaded())
        {
        // ###TODO: Do this incrementally (honor the timeout, if any, on the context's UpdatePlan)
        size_t i = 0;
        while (i < m_attachments.size())
            {
            // If LoadTree fails, the attachment will be dropped from m_attachments.
            if (m_attachments.LoadTree(i, GetDgnDb(), *this, context))
                ++i;
            }

        BeAssert(m_attachments.AllLoaded()); // ###TODO: remove this when we switch to loading incrementally
        }

    for (auto& attach : m_attachments)
        {
        BeAssert(nullptr != attach.GetTree());
        // if (!attach.GetTree()->GetRootTile()->IsReady())
        //     continue;

        attach.GetTree()->DrawInView(context);
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
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_UnloadAllTileTrees()
    {
    T_Super::_UnloadAllTileTrees();
    m_attachments.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::_CancelAllTileLoads(bool wait)
    {
    if (m_root.IsValid())
        m_root->CancelAllTileLoads();

    for (auto& attach : m_attachments)
        attach.CancelAllTileLoads();

    if (!wait)
        return;

    if (m_root.IsValid())
        m_root->WaitForAllLoads();

    for (auto& attach : m_attachments)
        attach.WaitForAllLoads();
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
TileTree::Tile::SelectParent Sheet::Attachment::Tile::_SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const
    {
    BeAssert(selected.empty());

    if (IsNotFound())
        {
        // Indicates no elements in this tile's range (or some unexpected error occurred during scene creation)
        return SelectParent::No;
        }

    Visibility vis = GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        return SelectParent::No;

    bool tooCoarse = Visibility::TooCoarse == vis;
    TileTree::Tile::ChildTiles const* childTiles = tooCoarse ? _GetChildren(true) : nullptr;
    Tile* child = nullptr != childTiles && !childTiles->empty() ? static_cast<TileP>((*childTiles)[0].get()) : nullptr;

    if (nullptr != child)
        {
        child->_SelectTiles(selected, args);
        return SelectParent::No;
        }

// #define DEBUG_PRINT_SHEET_TILE_SELECTION
#if defined(DEBUG_PRINT_SHEET_TILE_SELECTION)
    DEBUG_PRINTF(" ** Selecting this tile, IsReady()=%d, depth=%d", IsReady(), GetDepth());
#endif

    if (!IsReady())
        const_cast<Tile*>(this)->CreateGraphics(args.m_context);

    if (IsReady())
        {
        selected.push_back(this);
        }
    else
        {
        // Inform the sheet controller that it needs to recreate its scene next frame
        GetTree().m_sheetController.MarkAttachmentSceneIncomplete();

        // Select a tile to temporarily draw in its place. Note this logic will need to change when we start subdividing.
        TileTree::TilePtr sub;
        auto children = _GetChildren(false);
        while (nullptr != children && !children->empty())
            {
            auto child = (*children)[0];
            if (child->IsReady())
                {
                sub = child;
                break;
                }

            children = child->_GetChildren(false);
            }

        if (sub.IsNull())
            {
            auto parent = GetParent();
            while (nullptr != parent)
                {
                if (parent->IsReady())
                    {
                    sub = const_cast<TileTree::TileP>(parent);
                    break;
                    }

                parent = parent->GetParent();
                }
            }

        if (sub.IsValid())
            selected.push_back(sub);
        }

    return SelectParent::No;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Tile::_DrawGraphics(DrawArgsR args) const
    {
    BeAssert(IsReady());
    for (auto& graphic : m_graphics)
        if (graphic.IsValid())
            args.m_graphics.Add(*graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Root::CreatePolys(SceneContextR context)
    {
    auto renderSys = context.GetRenderSystem();

    // ###TODO: an optimization could be to make the texture non-square to save on space (make match cropped tile aspect ratio)

    // set up initial corner values (before cropping to clip)
    static const double s_tileSize = 1.0;
    double east  = 0.0;
    double west  = east + s_tileSize;
    double north = 0.0;
    double south = north + s_tileSize;
    m_corners.m_pts[0].Init(east, north, m_biasDistance); 
    m_corners.m_pts[1].Init(west, north, m_biasDistance); 
    m_corners.m_pts[2].Init(east, south, m_biasDistance); 
    m_corners.m_pts[3].Init(west, south, m_biasDistance);
    m_polysRangeUnclipped.InitFrom(m_corners.m_pts, 4); 

    // set max pixel size for tolerancing size on screen

    // first create the polys for the tile so we can get the range (create graphics from polys later)
    m_polysRange.Init();
    m_tilePolys = renderSys->_CreateSheetTilePolys(m_corners, m_graphicsClip.get(), m_polysRange);

    m_polysRange.low.z  = 0.0;                // make sure entire z range.
    m_polysRange.high.z = 1.0;

#if 0 // ###TODO: needed?
    // Make the range lengths match (in order to make the aspect ratio square).  ###TODO: necessary if someday view ratio becomes equal to prev ratio?
    double xLength = m_polysRange.XLength();
    double yLength = m_polysRange.YLength();
    if (xLength > yLength)
        {
        m_polysRange.high.y = m_polysRange.low.y + xLength;
        }
    else if (xLength < yLength)
        {
        m_polysRange.high.x = m_polysRange.low.x + yLength;
        }
#endif

    // scale the UVs and points into the new (clipped) space so they still are in 0 to 1 range.
    // translation on root of tree takes care of putting them in proper space based on the clipped range.  (see Root constructor)
    repairSheetPolys(m_tilePolys, m_polysRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::RootPtr Sheet::Attachment::Root::Create(DgnDbR db, Sheet::ViewController& sheetController, DgnElementId attachmentId, SceneContextR context)
    {
    auto attach = db.Elements().Get<ViewAttachment>(attachmentId);
    if (!attach.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    auto viewId = attach->GetAttachedViewId();
    auto view = ViewDefinition::LoadViewController(viewId, db);
    if (!view.IsValid())
        return nullptr;

    auto viewport = T_HOST._CreateSheetAttachViewport();
    if (!viewport.IsValid())
        return nullptr;

    return new Sheet::Attachment::Root(db, sheetController, *attach, context, *viewport, *view);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Tile& Sheet::Attachment::Root::GetRootAttachmentTile()
    {
    BeAssert(GetRootTile().IsValid());
    return static_cast<TileR>(*GetRootTile());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::State Sheet::Attachment::Root::GetState(uint32_t depth) const
    {
    // ###TODO: Fix this silly lookup, called from an iterator over the Attachment list...
    auto attach = m_sheetController.GetAttachments().Find(m_attachmentId);
    BeAssert(nullptr != attach);
    return attach->GetState(depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Root::SetState(uint32_t depth, State state)
    {
    // ###TODO: Fix this silly lookup, called from an iterator over the Attachment list...
    auto attach = m_sheetController.GetAttachments().Find(m_attachmentId);
    BeAssert(nullptr != attach);
    attach->SetState(depth, state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Render::ViewFlagsOverrides Sheet::Attachment::Root::_GetViewFlagsOverrides() const
    {
    // TFS#863662: If sheet's ViewFlags has transparency turned off, background pixels of
    // attachments will render opaque black...
    auto ovrs = T_Super::_GetViewFlagsOverrides();
    ovrs.SetShowTransparency(true);
    return ovrs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Root::Root(DgnDbR db, Sheet::ViewController& sheetController, ViewAttachmentCR attach, SceneContextR context, Viewport& viewport, Dgn::ViewControllerR view)
  : T_Super(db, DgnModelId(), /*is3d=*/false, Transform::FromIdentity(), nullptr, nullptr),
    m_attachmentId(attach.GetElementId()), m_viewport(&viewport), m_sheetController(sheetController)
    {
    m_viewport->SetupFromViewController();
    m_viewport->ChangeViewController(view);

    auto& def = view.GetViewDefinitionR();
    auto& style = def.GetDisplayStyle();

    ColorDef bgColor;
#if defined(DEBUG_SHEET_BACKGROUND)
    static int sCount = 0;
    ColorDef sCols[12] = {
        ColorDef::Brown(),
        ColorDef::LightGrey(),
        ColorDef::MediumGrey(),
        ColorDef::DarkGrey(),
        ColorDef::DarkRed(),
        ColorDef::DarkGreen(),
        ColorDef::DarkBlue(),
        ColorDef::DarkYellow(),
        ColorDef::DarkOrange(),
        ColorDef::DarkCyan(),
        ColorDef::DarkMagenta(),
        ColorDef::DarkBrown(),
    };
    bgColor = sCols[sCount++];
    if (sCount > 11) sCount = 0;
#else
    // override the background color. This is to match V8, but there should probably be an option in the "Details" about whether to do this or not.
    bgColor = sheetController.GetViewDefinitionR().GetDisplayStyle().GetBackgroundColor();

    // Set fully-transparent so that we discard background pixels (probably no point to the above line any more...)
    bgColor.SetAlpha(0xff);
#endif

    style.SetBackgroundColor(bgColor);

    SpatialViewDefinitionP spatial = def.ToSpatialViewP();
    if (spatial)
        {
        auto& env = spatial->GetDisplayStyle3d().GetEnvironmentDisplayR();
        env.m_groundPlane.m_enabled = false;
        env.m_skybox.m_enabled = false;
        }

    m_viewport->SetupFromFrustum(m_viewport->GetFrustum(DgnCoordSystem::World));

    auto& box = attach.GetPlacement().GetElementBox();
    AxisAlignedBox3d range = attach.GetPlacement().CalculateRange();

    int32_t biasDistance = Render::Target::DepthFromDisplayPriority(attach.GetDisplayPriority());
    m_biasDistance = double(biasDistance);
    DPoint3d org = range.low;

    org.z = 0.0; // ###TODO m_biasDistance;?
    Transform trans = Transform::From(org);
    trans.ScaleMatrixColumns(box.GetWidth(), box.GetHeight(), 1.0);
    SetLocation(trans);

    bsiTransform_initFromRange(&m_viewport->m_toParent, nullptr, &range.low, &range.high);

    // set a clip volume around view, so we only show the original volume
    m_clip = attach.GetClip();
    if (!m_clip.IsValid())
        m_clip = new ClipVector(ClipPrimitive::CreateFromShape(RectanglePoints(range.low.x, range.low.y, range.high.x, range.high.y), 5, false, nullptr, nullptr, nullptr).get());

    Transform fromParent;
    fromParent.InverseOf(m_viewport->m_toParent);
    m_graphicsClip = m_clip->Clone(&fromParent);

    CreatePolys(context); // m_graphicsClip must be set before creating polys (the polys that represent the tile)

    // alter location translation based on range of clipped polys
    trans = Transform::FromProduct(m_viewport->m_toParent, Transform::From(m_polysRange.low));
    trans.ScaleMatrixColumns(m_polysRange.XLength(), m_polysRange.YLength(), 1.0);
    SetLocation(trans);

    m_viewport->m_clips = m_clip; // save original clip in viewport

    SetExpirationTime(BeDuration::Seconds(5)); // only save unused sheet tiles for 5 seconds

    m_rootTile = new Tile(*this, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Tile::Tile(RootR root, TileCP parent) : T_Super(root, parent)
    {
    // Initialize range...
    uint32_t texSize = querySheetTileSize(GetDepth());
    Sheet::Attachment::Root& tree = GetTree();

    m_maxPixelSize = .5 * DPoint2d::FromZero().Distance(DPoint2d::From(texSize, texSize));
    m_range.Init();
    m_range.Extend(tree.m_polysRangeUnclipped.low);
    m_range.Extend(tree.m_polysRangeUnclipped.high);
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
