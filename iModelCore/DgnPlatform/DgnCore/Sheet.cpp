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

namespace Attachment
{
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile3d);
DEFINE_REF_COUNTED_PTR(Tile3d);
DEFINE_REF_COUNTED_PTR(Root3d);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Tile2d);
DEFINE_REF_COUNTED_PTR(Tile2d);
DEFINE_POINTER_SUFFIX_TYPEDEFS(Root2d);
DEFINE_REF_COUNTED_PTR(Root2d);

//=======================================================================================
// Contains a chain of tiles containing a texture renderings of the sheet (increasing in level of detail).
// @bsistruct                                                   Mark.Schlosser  02/2018
//=======================================================================================
struct Root3d : Attachment::Root
{
    DEFINE_T_SUPER(Attachment::Root);

    ColorDef m_tileColor = ColorDef::White();
    DgnElementId m_attachmentId;
    RefCountedPtr<Attachment::Viewport> m_viewport;
    ClipVectorPtr m_graphicsClip;
    Sheet::ViewController& m_sheetController;
private:
    Root3d(Sheet::ViewController& sheetController, ViewAttachmentCR attach, SceneContextR context, Attachment::Viewport& viewport, Dgn::ViewControllerR view);

    Render::ViewFlagsOverrides _GetViewFlagsOverrides() const override;
public:
    static Attachment::Root3dPtr Create(Sheet::ViewController& sheetController, DgnElementId attachmentId, SceneContextR context);

    virtual ~Root3d() { ClearAllTiles(); }

    Utf8CP _GetName() const override {return "SheetTile3d";}
    DRange3d GetRootRange() const;
    void Draw(SceneContextR);
    Tile3dR GetRootAttachmentTile();
    Attachment::State GetState(uint32_t depth) const;
    void SetState(uint32_t depth, Attachment::State state);
};

//=======================================================================================
// @bsistruct                                                   Mark.Schlosser  02/2018
//=======================================================================================
struct Tile3d : TileTree::Tile
{
    DEFINE_T_SUPER(TileTree::Tile);

    //=======================================================================================
    // Describes the location of a tile within the range of a quad subdivided in four parts.
    // @bsistruct                                                   Mark.Schlosser  03/2018
    //=======================================================================================
    enum class Placement 
    {
        UpperLeft,
        UpperRight,
        LowerLeft,
        LowerRight,
        Root, // root placement is for root tile of a tree: a single placement representing entire image (not subdivided)
    };

    bvector<PolyfaceHeaderPtr> m_tilePolys;
    uint32_t m_maxPixelSize;
    bvector<Render::GraphicPtr> m_graphics;
    Placement m_placement;

    void _Invalidate() override { }
    bool _IsInvalidated(TileTree::DirtyRangesCR) const override { return false; }
    void _UpdateRange(DRange3dCR, DRange3dCR) override { }

    bool _HasChildren() const override;
    bool _HasGraphics() const override {return IsReady()/*###TODO: && m_graphic.IsValid()*/;}
    void _DrawGraphics(TileTree::DrawArgsR) const override;
    ChildTiles const* _GetChildren(bool) const override;
    void _ValidateChildren() const override { }
    Utf8String _GetTileCacheKey() const override { return "NotCacheable!"; }
    SelectParent Select(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args);
    SelectParent _SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const override;
    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr loads, Dgn::Render::SystemP renderSys) override {return nullptr;} // implement tileloader
    double _GetMaximumSize() const override {return m_maxPixelSize;}

    void CreatePolys(SceneContextR context);
    void CreateGraphics(SceneContextR context);
    Root3dR GetTree() const {return static_cast<Root3dR>(m_root);}
    Attachment::State GetState() const { return GetTree().GetState(GetDepth()); }
    void SetState(Attachment::State state) { GetTree().SetState(GetDepth(), state); }

    Tile3d(Root3dR root, Tile3dCP parent, Placement placement);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/18
//=======================================================================================
struct Root2d : Attachment::Root
{
    DEFINE_T_SUPER(Attachment::Root);

    RefCountedPtr<ViewController2d>         m_view;
    TileTree::RootPtr                       m_viewRoot;
    Transform                               m_drawingToAttachment;
    ClipVectorPtr                           m_graphicsClip;
    Render::FeatureSymbologyOverridesCPtr   m_symbologyOverrides;
private:
    Root2d(Sheet::ViewController& sheetController, ViewAttachmentCR attach, SceneContextR context, Dgn::ViewController2dR view, TileTree::RootR viewRoot);
public:
    static Attachment::Root2dPtr Create(Sheet::ViewController& sheetController, DgnElementId attachmentId, SceneContextR context);

    virtual ~Root2d() { ClearAllTiles(); }

    Utf8CP _GetName() const override { return "SheetTil2d"; }

    void DrawClipPolys(TileTree::DrawArgsR args) const;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/18
//=======================================================================================
struct Tile2d : TileTree::Tile
{
    DEFINE_T_SUPER(TileTree::Tile);

    bool _HasChildren() const override { return false; }
    ChildTiles const* _GetChildren(bool) const override { return nullptr; }
    TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr, Render::SystemP) override { return nullptr; }

    void _Invalidate() override { }
    bool _IsInvalidated(TileTree::DirtyRangesCR) const override { return false; }

    bool _HasGraphics() const override { return true; }
    double _GetMaximumSize() const override { return 512; } // doesn't matter - no children
    Utf8String _GetTileCacheKey() const override { return "NotCacheable!"; }

    void _DrawGraphics(TileTree::DrawArgsR) const override;

    Root2dCR GetRoot2d() const { return static_cast<Root2dCR>(GetRoot()); }

    Tile2d(Root2dR root, ElementAlignedBox2d const& range) : T_Super(root, nullptr)
        {
        m_range.low = DPoint3d::FromXYZ(0, 0, -Render::Target::Get2dFrustumDepth());
        m_range.high = DPoint3d::FromXYZ(range.high.x, range.high.y, Render::Target::Get2dFrustumDepth());

        SetIsReady();
        }
};

} // namespace Attachment

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/18
//=======================================================================================
struct Attachment3d : Sheet::ViewController::Attachment
{
    friend struct Sheet::ViewController::Attachments;
    using State = Sheet::Attachment::State;
private:
    Sheet::Attachment::Root3dPtr    m_tree = nullptr;
    bvector<State>      m_states; // per level of the tree

    bool _Load(DgnDbR db, ViewController& sheetController, SceneContextR context) override;
    Sheet::Attachment::RootP _GetTree() const override { return m_tree.get(); }
public:
    explicit Attachment3d(DgnElementId id) : Sheet::ViewController::Attachment(id) { }

    Sheet::Attachment::Root3dP GetTree() { return m_tree.get(); }
    Sheet::Attachment::Root3dCP GetTree() const { return m_tree.get(); }
    State GetState(uint32_t depth) const { return depth < m_states.size() ? m_states[depth] : State::NotLoaded; }
    void SetState(uint32_t depth, State state);
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   03/18
//=======================================================================================
struct Attachment2d : Sheet::ViewController::Attachment
{
private:
    Sheet::Attachment::Root2dPtr    m_tree;

    bool _Load(DgnDbR db, ViewController& sheetController, SceneContextR context) override;
    Sheet::Attachment::RootP _GetTree() const override { return m_tree.get(); }
public:
    explicit Attachment2d(DgnElementId id) : Sheet::ViewController::Attachment(id) { }
};

END_SHEET_NAMESPACE

USING_NAMESPACE_TILETREE
USING_NAMESPACE_SHEET
using namespace Attachment;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::ViewController::AttachmentPtr Sheet::ViewController::Attachment::Create(DgnElementId attachId, DgnDbR db)
    {
    auto attach = db.Elements().Get<ViewAttachment>(attachId);
    auto view = attach.IsValid() ? db.Elements().Get<ViewDefinition>(attach->GetAttachedViewId()) : nullptr;
    BeAssert(view.IsValid());
    if (view.IsNull())
        return nullptr;
    else if (view->IsView3d())
        return new Attachment3d(attachId);
    else
        return new Attachment2d(attachId);
    }

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

    template<typename T> RectanglePoints(T const& range) : RectanglePoints(range.low.x, range.low.y, range.high.x, range.high.y) { }

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
void Sheet::ViewController::Attachments::Add(Sheet::ViewController::AttachmentR attach)
    {
    BeAssert(nullptr == Find(attach.GetId()));
    m_allAttachmentsLoaded = m_allAttachmentsLoaded && nullptr != attach._GetTree();
    m_list.push_back(&attach);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Sheet::ViewController::Attachments::Load(size_t index, DgnDbR db, ViewController& sheetController, SceneContextR context)
    {
    BeAssert(index < size());
    if (index >= size())
        return false;

    auto& attach = m_list[index];
    if (nullptr != attach->_GetTree())
        return true;

    bool loaded = attach->_Load(db, sheetController, context);
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
        if (nullptr == attach->_GetTree())
            {
            m_allAttachmentsLoaded = false;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::ViewController::Attachments::InitBoundingBoxColors()
    {
    static ColorDef s_colors[] =
        {
        ColorDef::DarkOrange(),
        ColorDef::DarkBlue(),
        ColorDef::DarkRed(),
        ColorDef::DarkCyan(),
        ColorDef::DarkYellow(),
        ColorDef::DarkMagenta(),
        ColorDef::DarkBrown(),
        ColorDef::DarkGrey(),
        };

    for (size_t i = 0; i < size(); i++)
        {
        auto tree = m_list[i]->_GetTree();
        if (nullptr != tree)
            tree->m_boundingBoxColor = s_colors[i % _countof(s_colors)];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Attachment3d::SetState(uint32_t depth, State state)
    {
    while (m_states.size() < depth+1)
        m_states.push_back(State::NotLoaded);

    m_states[depth] = state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Attachment3d::_Load(DgnDbR db, Sheet::ViewController& sheetController, SceneContextR context)
    {
    if (!m_tree.IsValid())
        m_tree = Sheet::Attachment::Root3d::Create(sheetController, GetId(), context);

    return m_tree.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool Attachment2d::_Load(DgnDbR db, Sheet::ViewController& sheetController, SceneContextR context)
    {
    m_tree = Sheet::Attachment::Root2d::Create(sheetController, GetId(), context);
    return m_tree.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
static bool acceptAttachment(DgnElementId id, DgnDbR db, uint32_t index)
    {
//#define FILTER_ATTACHMENTS
#if defined(FILTER_ATTACHMENTS)
    // For debugging, define some criterion herein to filter out attachments not of interest...
    return 1 == index;
#else
    return true;
#endif
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

    DgnDbR db = GetDgnDb();
    Attachments attachments;
    auto stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_ViewAttachment) " WHERE Model.Id=?");
    stmt->BindId(1, model->GetModelId());

    // If we're already loaded, look in existing list so we don't reload them
    uint32_t attachmentIndex = 0;
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        if (!acceptAttachment(attachId, db, attachmentIndex++))
            continue;

        AttachmentPtr tree = FindAttachment(attachId);

        if (tree.IsNull())
            tree = Attachment::Create(attachId, GetDgnDb());

        if (tree.IsValid())
            attachments.Add(*tree);
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
bool Sheet::Attachment::Tile3d::_HasChildren() const // { return false; }
    { // this method actually means "I have children I may need to create" not "I currently have children in my m_children list".
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::ChildTiles const* Sheet::Attachment::Tile3d::_GetChildren(bool load) const
    {
    if (m_children.empty() && load)
        {
        TilePtr childTileUL = new Attachment::Tile3d(GetTree(), this, Attachment::Tile3d::Placement::UpperLeft);
        TilePtr childTileUR = new Attachment::Tile3d(GetTree(), this, Attachment::Tile3d::Placement::UpperRight);
        TilePtr childTileLL = new Attachment::Tile3d(GetTree(), this, Attachment::Tile3d::Placement::LowerLeft);
        TilePtr childTileLR = new Attachment::Tile3d(GetTree(), this, Attachment::Tile3d::Placement::LowerRight);
        m_children.push_back(childTileUL);
        m_children.push_back(childTileUR);
        m_children.push_back(childTileLL);
        m_children.push_back(childTileLR);
         }

    return m_children.empty() ? nullptr : &m_children;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static const uint32_t querySheetTilePixels() { return 512; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Viewport::SetSceneDepth(uint32_t depth, Root3d& tree)
    {
    if (m_sceneDepth != depth)
        {
        // Ensure that if we return to this depth and need to produce more tile graphics, we first recreate the scene at that depth...
        if (0xffffffff != m_sceneDepth && State::Ready == tree.GetState(m_sceneDepth))
            tree.SetState(m_sceneDepth, State::NotLoaded);

        // Discard any tiles/graphics used for previous level-of-detail - we'll generate them at the new LOD.
        InvalidateScene();
        m_viewController->_CancelAllTileLoads(false);
        m_viewController->_UnloadAllTileTrees();

        m_sceneDepth = depth;
        int dim = querySheetTilePixels();
        dim = dim * pow(2, depth); // doubling the rect dimensions for every level of depth
        SetRect(BSIRect::From(0, 0, dim, dim), /*temporary=*/true);
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
void Sheet::Attachment::Tile3d::CreateGraphics(SceneContextR context)
    {
    auto& tree = GetTree();
    auto currentState = GetState();

    // State::Ready is a valid situation.  It means another tile created the scene for this level of detail.  We will use that scene.
    // However, this means we would be using the texture for that other tile, which will be different than what we want!  We must recreate texture.

    BeAssert(State::Empty != currentState);
    if (State::Empty == currentState)
        {
        SetNotFound();
        return;
        }

    auto renderSys = context.GetRenderSystem();

    Sheet::Attachment::Viewport* viewport = tree.m_viewport.get();

    if (State::Ready != currentState)
        {
        UpdatePlan const& plan = context.GetUpdatePlan();

        viewport->SetSceneDepth(GetDepth(), tree);
        viewport->SetupFromViewController();

        // Create the scene, and if the scene is complete, mark state ready
        currentState = viewport->_CreateScene(plan, currentState);
        SetState(currentState);
        }

    switch (currentState)
        {
        case State::NotLoaded:
        case State::Loading:
            return;
        case State::Empty:
            SetNotFound();
            return;
        case State::Ready:
            {
            // Only render one tile per frame - otherwise we swamp the render thread and introduce lag
            if (!viewport->m_rendering)
                {
                viewport->m_rendering = true;

                // render the texture then create graphics from the polys and the rendered texture
                // ###TODO: must determine whether to do this at all if there were no poly results
                AutoRestoreFrustum autoRestore(*viewport);

                // Scene rect does not match this.  That rect increases with depth.  This rect is constant, because it is the rect of the final texture.
                uint32_t dim = querySheetTilePixels();
                viewport->SetRect(BSIRect::From(0, 0, dim, dim));

                // Change frustum so it looks at only the visible (after clipping) portion of the scene.
                // also only look at the relevent corner of the scene
                Frustum frust = viewport->GetFrustum(DgnCoordSystem::Npc);
                DPoint3dP frustPts = frust.GetPtsP();
                
                m_range.Get8Corners(frustPts); // use unclipped range of tile to change the frustum (this is what we're looking at)
                DMap4dCP rootToNpc = viewport->GetWorldToNpcMap();
                rootToNpc->M1.MultiplyAndRenormalize(frustPts, frustPts, NPC_CORNER_COUNT);
                viewport->SetupFromFrustum(frust);

                viewport->_RenderTexture();
                if (viewport->m_texture.IsNull())
                    {
                    SetNotFound();
                    }
                else
                    {
                    GraphicParams gfParams = GraphicParams::FromSymbology(tree.m_tileColor, tree.m_tileColor, 0);
                    m_graphics = renderSys->_CreateSheetTile(*viewport->m_texture, m_tilePolys, viewport->GetViewController().GetDgnDb(), gfParams);
                    SetIsReady();
                    }
                }

            break;
            }
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
            if (m_attachments.Load(i, GetDgnDb(), *this, context))
                ++i;
            }

        BeAssert(m_attachments.AllLoaded()); // ###TODO: remove this when we switch to loading incrementally

        if (m_attachments.AllLoaded())
            m_attachments.InitBoundingBoxColors();
        }

    for (auto& attach : m_attachments)
        {
        BeAssert(nullptr != attach->_GetTree());
        // if (!attach.GetTree()->GetRootTile()->IsReady())
        //     continue;

        attach->_GetTree()->DrawInView(context);
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
        attach->CancelAllTileLoads();

    if (!wait)
        return;

    if (m_root.IsValid())
        m_root->WaitForAllLoads();

    for (auto& attach : m_attachments)
        attach->WaitForAllLoads();
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
TileTree::Tile::SelectParent Sheet::Attachment::Tile3d::_SelectTiles(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args) const
    {
    return const_cast<Attachment::Tile3d&>(*this).Select(selected, args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TileTree::Tile::SelectParent Sheet::Attachment::Tile3d::Select(bvector<TileTree::TileCPtr>& selected, TileTree::DrawArgsR args)
    {
    if (0 == GetDepth())
        GetTree().m_viewport->m_rendering = false;

    if (IsNotFound())
        {
        // Indicates no elements in this tile's range (or some unexpected error occurred during scene creation)
        return SelectParent::No;
        }

    Visibility vis = _GetVisibility(args);
    if (Visibility::OutsideFrustum == vis)
        {
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    bool tooCoarse = Visibility::TooCoarse == vis;
    auto children = tooCoarse ? _GetChildren(true) : nullptr;

    if (nullptr != children)
        {
        size_t initialSize = selected.size();
        m_childrenLastUsed = args.m_now;
        for (auto& child : *children)
            {
            if (SelectParent::Yes == child->_SelectTiles(selected, args))
                {
                // At least one of the selected children is not ready to draw. If the parent (this) is drawable, draw in place of all the children.
                selected.resize(initialSize);
                if (IsReady())
                    {
                    selected.push_back(this);
                    return SelectParent::No;
                    }
                else
                    {
                    // This tile isn't ready to draw either. Try drawing its own parent in its place.
                    return SelectParent::Yes;
                    }
                }
            }

        return SelectParent::No;
        }

    // This tile is of appropriate resolution to draw. Enqueue it for loading if necessary.
    if (!IsReady())
        {
        if (m_tilePolys.empty())
            {
            CreatePolys(args.GetContext()); // m_graphicsClip on tree must be set before creating polys (the polys that represent the tile)
            if (m_tilePolys.empty())
                {
                SetNotFound();
                return SelectParent::No;
                }
            }

        CreateGraphics(args.GetContext());
        }

    if (IsReady())
        {
        selected.push_back(this);
        _UnloadChildren(args.m_purgeOlderThan);
        return SelectParent::No;
        }

    // Inform the sheet controller that it needs to recreate its scene next frame
    GetTree().m_sheetController.MarkAttachmentSceneIncomplete();

    // Tell parent to render in this tile's place until it becomes ready to draw.
    return SelectParent::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Tile3d::_DrawGraphics(DrawArgsR args) const
    {
    BeAssert(IsReady());
    for (auto& graphic : m_graphics)
        if (graphic.IsValid())
            args.m_graphics.Add(*graphic);

    static bool s_drawDebugRange = false;
    if (!s_drawDebugRange)
        return;

    GraphicParams params;
    params.SetWidth(0);
    ColorDef color = GetTree().m_boundingBoxColor;
    params.SetLineColor(color);
    params.SetFillColor(color);

    auto gf = args.GetContext().CreateSceneGraphic();
    gf->ActivateGraphicParams(params);
    gf->AddRangeBox(GetRange());

    args.m_graphics.Add(*gf->Finish());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Tile2d::_DrawGraphics(TileTree::DrawArgsR myArgs) const
    {
    auto const& myRoot = GetRoot2d();
    auto& viewRoot = *myRoot.m_viewRoot;

    TileTree::DrawArgs args = viewRoot.CreateDrawArgs(myArgs.GetContext());
    args.m_location = myRoot.m_drawingToAttachment;
    args.m_viewFlagsOverrides = Render::ViewFlagsOverrides(myRoot.m_view->GetViewFlags());
    args.m_clip = GetRoot2d().m_graphicsClip.get();
    args.m_graphics.m_symbologyOverrides = GetRoot2d().m_symbologyOverrides;

    myRoot.m_view->CreateScene(args);

    static bool s_drawClipPolys = false;
    if (s_drawClipPolys)
        myRoot.DrawClipPolys(myArgs);

    static bool s_drawRangeBoxes = false;
    if (!s_drawRangeBoxes)
        return;

    GraphicParams params;
    params.SetWidth(0);
    ColorDef color = myRoot.m_boundingBoxColor;
    params.SetLineColor(color);
    params.SetFillColor(color);

    auto gf = myArgs.GetContext().CreateSceneGraphic();
    gf->ActivateGraphicParams(params);
    gf->AddRangeBox(GetRange());

    // Put in a branch so it doesn't get clipped...
    GraphicBranch branch;
    branch.Add(*gf->Finish());
    myArgs.m_graphics.Add(*args.GetContext().CreateBranch(branch, GetRoot().GetDgnDb(), Transform::FromIdentity()));
    }

/*---------------------------------------------------------------------------------**//**
* For debugging purposes, create polys from ClipVector and output as graphics.
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Root2d::DrawClipPolys(TileTree::DrawArgsR args) const
    {
    DRange3d range = GetRootTile()->GetRange();

    double zDepth = Render::Target::DepthFromDisplayPriority(0.5 * Render::Target::GetMaxDisplayPriority());

    DPoint3d tmpPts[4];
    tmpPts[0] = DPoint3d::From(range.low.x, range.low.y, zDepth);
    tmpPts[1] = DPoint3d::From(range.high.x, range.low.y, zDepth);
    tmpPts[2] = DPoint3d::From(range.high.x, range.high.y, zDepth);
    tmpPts[3] = DPoint3d::From(range.low.x, range.high.y, zDepth);

    bvector<DPoint3d> pts(std::begin(tmpPts), std::end(tmpPts));

    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*facetOptions);
    builder->AddTriangulation(pts);

    auto polyface = builder->GetClientMeshPtr();
    Render::Primitives::GeometryClipper::PolyfaceClipper clipper;
    clipper.ClipPolyface(*polyface, m_clip.get(), true);
    if (!clipper.HasOutput())
        return;

    GraphicParams params;
    ColorDef fillColor = m_boundingBoxColor;
    fillColor.SetAlpha(0x7f);
    params.SetFillColor(fillColor);

    auto gf = args.GetContext().CreateSceneGraphic();
    gf->ActivateGraphicParams(params);

    for (auto& mesh : clipper.GetOutput())
        gf->AddPolyface(*mesh, true);

    args.m_graphics.Add(*gf->Finish());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Tile3d::CreatePolys(SceneContextR context)
    {
    auto renderSys = context.GetRenderSystem();

    // ###TODO: an optimization could be to make the texture non-square to save on space (make match cropped tile aspect ratio)

    // set up initial corner values (before cropping to clip)
    auto& tree = GetTree();

    // set up initial corner values (before cropping to clip); m_range must be already setup (m_range = unclipped range)
    Render::GraphicBuilder::TileCorners corners; 
    double east  = m_range.low.x;
    double west  = m_range.high.x;
    double north = m_range.low.y; 
    double south = m_range.high.y;
    corners.m_pts[0].Init(east, north, tree.m_biasDistance); 
    corners.m_pts[1].Init(west, north, tree.m_biasDistance); 
    corners.m_pts[2].Init(east, south, tree.m_biasDistance); 
    corners.m_pts[3].Init(west, south, tree.m_biasDistance);

    // first create the polys for the tile so we can get the range (create graphics from polys later)
    m_tilePolys = renderSys->_CreateSheetTilePolys(corners, tree.m_graphicsClip.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Sheet::Attachment::Root3d::GetRootRange() const
    {
    static const double s_tileSize = 1.0;
    double east  = 0.0;
    double west  = east + s_tileSize;
    double north = 0.0;
    double south = north + s_tileSize;
 
    Render::GraphicBuilder::TileCorners corners; 
    corners.m_pts[0].Init(east, north, m_biasDistance); 
    corners.m_pts[1].Init(west, north, m_biasDistance); 
    corners.m_pts[2].Init(east, south, m_biasDistance); 
    corners.m_pts[3].Init(west, south, m_biasDistance);

    return DRange3d::From(corners.m_pts, 4); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Attachment::Root3dPtr Attachment::Root3d::Create(Sheet::ViewController& sheetController, DgnElementId attachmentId, SceneContextR context)
    {
#if defined(NO_3D_ATTACHMENTS)
    return nullptr;
#else
    auto& db = sheetController.GetDgnDb();
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

    return new Sheet::Attachment::Root3d(sheetController, *attach, context, *viewport, *view);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Attachment::Root2dPtr Attachment::Root2d::Create(Sheet::ViewController& sheetController, DgnElementId attachmentId, SceneContextR context)
    {
#if defined(NO_2D_ATTACHMENTS)
    return nullptr;
#else
    auto& db = sheetController.GetDgnDb();
    auto attach = db.Elements().Get<ViewAttachment>(attachmentId);
    if (attach.IsNull())
        {
        BeAssert(false);
        return nullptr;
        }

    auto viewId = attach->GetAttachedViewId();
    auto view = ViewDefinition::LoadViewController(viewId, db);
    if (view.IsNull())
        return nullptr;

    BeAssert(nullptr != view->GetViewDefinition().ToView2d());
    BeAssert(nullptr != dynamic_cast<ViewController2dP>(view.get()));

    auto& view2d = static_cast<ViewController2dR>(*view);
    TileTree::RootP viewRoot = view2d.GetRoot(context);
    if (nullptr == viewRoot || viewRoot->GetRootTile().IsNull())
        return nullptr;

    return new Sheet::Attachment::Root2d(sheetController, *attach, context, view2d, *viewRoot);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Tile3d& Sheet::Attachment::Root3d::GetRootAttachmentTile()
    {
    BeAssert(GetRootTile().IsValid());
    return static_cast<Attachment::Tile3dR>(*GetRootTile());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::State Sheet::Attachment::Root3d::GetState(uint32_t depth) const
    {
    // ###TODO: Fix this silly lookup, called from an iterator over the Attachment list...
    auto attach = m_sheetController.GetAttachments().Find(m_attachmentId);
    BeAssert(nullptr != attach);

    // ###TODO: WIP_ATTACHMENTS
    auto attach3d = dynamic_cast<Attachment3d*>(attach);
    if (nullptr == attach3d)
        return Sheet::Attachment::State::Empty;

    return attach3d->GetState(depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
void Sheet::Attachment::Root3d::SetState(uint32_t depth, State state)
    {
    // ###TODO: Fix this silly lookup, called from an iterator over the Attachment list...
    auto attach = m_sheetController.GetAttachments().Find(m_attachmentId);
    BeAssert(nullptr != attach);

    // ###TODO: WIP_ATTACHMENTS
    auto attach3d = dynamic_cast<Attachment3d*>(attach);
    if (nullptr != attach3d)
        attach3d->SetState(depth, state);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Render::ViewFlagsOverrides Sheet::Attachment::Root3d::_GetViewFlagsOverrides() const
    {
    // TFS#863662: If sheet's ViewFlags has transparency turned off, background pixels of
    // attachments will render opaque black...
    auto ovrs = T_Super::_GetViewFlagsOverrides();
    ovrs.SetShowTransparency(true);
    return ovrs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ViewAttachment::CreateBoundaryClip() const
    {
    RectanglePoints box(GetPlacement().CalculateRange());
    return new ClipVector(ClipPrimitive::CreateFromShape(box, 5, false, nullptr, nullptr, nullptr).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr ViewAttachment::GetOrCreateClip(TransformCP tf) const
    {
    auto clip = GetClip();
    if (clip.IsNull())
        clip = CreateBoundaryClip();

    if (nullptr != tf)
        clip = clip->Clone(tf);

    return clip;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Root::Root(DgnModelId modelId, Sheet::ViewController& sheetController, ViewAttachmentCR attach, SceneContextR context, Dgn::ViewControllerR view)
  : T_Super(sheetController.GetDgnDb(), modelId, /*is3d=*/false, Transform::FromIdentity(), nullptr, nullptr)
    {
    // ###TODO: WIP_ATTACHMENTS: Common initialization (view controller etc)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Root3d::Root3d(Sheet::ViewController& sheetController, ViewAttachmentCR attach, SceneContextR context, Viewport& viewport, Dgn::ViewControllerR view)
  : T_Super(DgnModelId(), sheetController, attach, context, view),
    m_attachmentId(attach.GetElementId()), m_viewport(&viewport), m_sheetController(sheetController)
    {
    DPoint2d scale;

    // we use square tiles. If the view's aspect ratio isn't square, expand the short side in tile (NPC) space. We'll clip out the extra area below.
    double aspect = view.GetViewDefinition().GetAspectRatio();
    if (aspect < 1.0)
        scale.Init(1.0 / aspect, 1.0);
    else
        scale.Init(1.0, aspect);

    // now expand the frustum in one direction so that the view is square (so we can use square tiles)
    uint32_t dim = querySheetTilePixels();
    m_viewport->SetRect(BSIRect::From(0, 0, dim, dim));
    m_viewport->ChangeViewController(view);
    m_viewport->SetupFromViewController();

    Frustum frust = m_viewport->GetFrustum(DgnCoordSystem::Npc).TransformBy(Transform::FromScaleFactors(scale.x, scale.y, 1.0));
    m_viewport->NpcToWorld(frust.m_pts, frust.m_pts, NPC_CORNER_COUNT);
    m_viewport->SetupFromFrustum(frust);

    auto& def = view.GetViewDefinitionR();
    auto& style = def.GetDisplayStyle();

    ColorDef bgColor;
// #define DEBUG_SHEET_BACKGROUND
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

    AxisAlignedBox3d range = attach.GetPlacement().CalculateRange();

    int32_t biasDistance = Render::Target::DepthFromDisplayPriority(attach.GetDisplayPriority());
    m_biasDistance = double(biasDistance);

    LegacyMath::TMatrix::InitTransformsFromRange(&m_viewport->m_toParent, nullptr, &range.low, &range.high);
    m_viewport->m_toParent.ScaleMatrixColumns(scale.x, scale.y, 1.0);

    Transform fromParent;
    fromParent.InverseOf(m_viewport->m_toParent);
    m_graphicsClip = attach.GetOrCreateClip(&fromParent);

    Tile3d* rTile;
    m_rootTile = rTile = new Tile3d(*this, nullptr, Sheet::Attachment::Tile3d::Placement::Root);
    rTile->CreatePolys(context); // m_graphicsClip must be set before creating polys (the polys that represent the tile)

    Transform trans = m_viewport->m_toParent;
    SetLocation(trans);

    SetExpirationTime(BeDuration::Seconds(15)); // only save unused sheet tiles for 15 seconds
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Root2d::Root2d(Sheet::ViewController& sheetController, ViewAttachmentCR attach, SceneContextR context, Dgn::ViewController2dR view, TileTree::RootR viewRoot)
    : T_Super(view.GetViewedModelId(), sheetController, attach, context, view), m_view(&view), m_viewRoot(&viewRoot)
    {
    // Ensure elements inside the view attachment are not affected to changes to category display etc for the sheet view.
    m_symbologyOverrides = Render::FeatureSymbologyOverrides::Create(view);

    auto& viewDef = view.GetViewDefinitionR();
    DRange3d attachRange = attach.GetPlacement().CalculateRange();
    double attachWidth = attachRange.high.x - attachRange.low.x,
           attachHeight = attachRange.high.y - attachRange.low.y;

    DPoint3d viewExtents = viewDef.GetExtents();
    DPoint2d scale = DPoint2d::From(attachWidth / viewExtents.x, attachHeight / viewExtents.y);

    DPoint3d worldToAttachment = DPoint3d::From(attach.GetPlacement().GetOrigin());
    worldToAttachment.z = Render::Target::DepthFromDisplayPriority(attach.GetDisplayPriority());

    Transform location = Transform::From(worldToAttachment);
    SetLocation(location);
    
    double aspectRatioSkew = viewDef.GetAspectRatioSkew();
    m_drawingToAttachment = Transform::From(viewDef.GetRotation());
    m_drawingToAttachment.ScaleMatrixColumns(scale.x, aspectRatioSkew * scale.y, 1.0);
    DPoint3d viewOrg = viewDef.GetOrigin();
    DPoint3d translation = viewRoot.GetLocation().Translation();
    viewOrg.DifferenceOf(viewOrg, translation);
    m_drawingToAttachment.Multiply(viewOrg);
    viewOrg.SumOf(translation, viewOrg);
    viewOrg.z = 0.0;
    DPoint3d viewOrgToAttachment;
    viewOrgToAttachment.DifferenceOf(worldToAttachment, viewOrg);
    translation.SumOf(translation, viewOrgToAttachment);
    m_drawingToAttachment.SetTranslation(translation);

    SetExpirationTime(BeDuration::Seconds(15));

    // The renderer needs the unclipped range of the attachment in order to produce polys to be rendered as clip mask...
    // (Containment tests can also be more efficiently performed if boundary range is specified).
    Transform clipTf;
    clipTf.InverseOf(location);
    m_clip = attach.GetOrCreateClip(&clipTf);
    DRange3d clipRange;
    clipTf.Multiply(clipRange, attachRange);
    m_clip->m_boundingRange = clipRange;

    Transform sheetToDrawing;
    sheetToDrawing.InverseOf(m_drawingToAttachment);
    m_graphicsClip = attach.GetOrCreateClip(&sheetToDrawing);
    DRange3d graphicsClipRange;
    sheetToDrawing.Multiply(graphicsClipRange, attachRange);
    m_graphicsClip->m_boundingRange = graphicsClipRange;

    m_rootTile = new Tile2d(*this, attach.GetPlacement().GetElementBox());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/18
+---------------+---------------+---------------+---------------+---------------+------*/
Sheet::Attachment::Tile3d::Tile3d(Root3dR root, Tile3dCP parent, Placement placement) : T_Super(root, parent), m_placement(placement)
    {
    auto& tree = GetTree();

    uint32_t dim = querySheetTilePixels();
    m_maxPixelSize = .5 * DPoint2d::FromZero().Distance(DPoint2d::From(dim, dim));

    DRange3d fullRange;
    if (nullptr != parent)
        fullRange = parent->GetRange();
    else
        fullRange = tree.GetRootRange();

    DPoint3d mid = DPoint3d::FromInterpolate(fullRange.low, 0.5, fullRange.high);

    m_range.Init();
    switch (m_placement)
        {
        case Placement::UpperLeft:
            m_range.Extend(mid);
            m_range.Extend(DPoint3d::From(fullRange.low.x, fullRange.high.y, 0.0));
            break;

        case Placement::UpperRight:
            m_range.Extend(mid);
            m_range.Extend(fullRange.high);
            break;

        case Placement::LowerLeft:
            m_range.Extend(fullRange.low);
            m_range.Extend(mid);
            break;

        case Placement::LowerRight:
            m_range.Extend(DPoint3d::From(fullRange.high.x, fullRange.low.y, 0.0));
            m_range.Extend(mid);
            break;

        case Placement::Root:
        default:
            m_range.Extend(fullRange.low);
            m_range.Extend(fullRange.high);
            break;
        }
    m_range.low.z = 0.0;
    m_range.high.z = 1.0;
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
