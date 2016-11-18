/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Sheet.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/TileTree.h>

USING_NAMESPACE_TILETREE

BEGIN_SHEET_NAMESPACE

namespace Handlers
{
HANDLER_DEFINE_MEMBERS(Element);
HANDLER_DEFINE_MEMBERS(Attachment)
HANDLER_DEFINE_MEMBERS(Model)
}

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/16
//=======================================================================================
struct Tile : QuadTree::Tile
{
    DEFINE_T_SUPER(QuadTree::Tile)

    struct Loader : TileLoader
    {
        Render::Image m_image;

        Loader(Utf8StringCR url, Tile& tile, LoadStatePtr loads) : TileLoader(url, tile, loads, tile._GetTileName()) {}
        BentleyStatus _LoadTile() override;
        folly::Future<BentleyStatus> _GetFromSource() override;
    };
    Tile(AttachmentTree&, QuadTree::TileId id, Tile const* parent);
    TilePtr _CreateChild(QuadTree::TileId id) const override {return new Tile(GetAttachmentTree(), id, this);}
    AttachmentTree& GetAttachmentTree() const {return (AttachmentTree&) m_root;}
    TileLoaderPtr _CreateTileLoader(LoadStatePtr loads) override {return new Loader(GetRoot()._ConstructTileName(*this), *this, loads);}
};

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
folly::Future<BentleyStatus> Sheet::Tile::Loader::_GetFromSource() 
    {
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* This sheet tile just became available from some source (cache, or created). Create a 
* a Render::Graphic to draw it. Only when finished, set the "ready" flag.
* @note this method can be called on many threads, simultaneously.
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Sheet::Tile::Loader::_LoadTile() 
    {
    Tile& tile = static_cast<Tile&>(*m_tile);
    AttachmentTree& root = tile.GetAttachmentTree();

    auto graphic = root.GetRenderSystem()->_CreateGraphic(Graphic::CreateParams(nullptr));

    Texture::CreateParams textureParams;
    textureParams.SetIsTileSection();
    auto texture = root.GetRenderSystem()->_CreateTexture(m_image, textureParams);

    graphic->SetSymbology(root.m_tileColor, root.m_tileColor, 0); // this is to set transparency
    graphic->AddTile(*texture, tile.m_corners); // add the texture to the graphic, mapping to corners of tile (in BIM world coordinates)

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
Sheet::Tile::Tile(AttachmentTree& root, QuadTree::TileId id, Tile const* parent) : T_Super(root, id, parent)
    {
    double tileSize = 1.0 / (1 << id.m_level); // the size of a tile for this level, in NPC
    double east  = id.m_column * tileSize;
    double west  = east + tileSize;
    double north = id.m_row * tileSize;
    double south = north + tileSize;
    
    m_corners.m_pts[0].Init(east, north, 0.0);   //  | [0]     [1]
    m_corners.m_pts[0].Init(west, north, 0.0);   //  y
    m_corners.m_pts[0].Init(east, south, 0.0);   //  | [2]     [3]
    m_corners.m_pts[0].Init(west, south, 0.0);   //  v

    m_range.InitFrom(m_corners.m_pts, 4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
AttachmentTree::AttachmentTree(DgnDbR db, DgnElementId attachmentId, Render::SystemP system, uint32_t tileSize) : m_attachmentId(attachmentId), QuadTree::Root(db, Transform::FromIdentity(), "", system, 10, tileSize)
    {
    auto attach = m_db.Elements().Get<ViewAttachment>(attachmentId);
    if (!attach.IsValid())
        {
        BeAssert(false);
        return;
        }

    auto& placement = attach->GetPlacement();
    SetLocation(placement.GetTransform());

    auto viewId = attach->GetAttachedViewId();
    m_view = m_db.Elements().Get<ViewDefinition>(DgnElementId(viewId.GetValue()));
    if (!m_view.IsValid())
        return;

    double aspect = m_view->GetAspectRatio();

    if (aspect<1.0)
        m_pixels.Init(tileSize, tileSize*aspect);
    else
        m_pixels.Init(tileSize*aspect, tileSize);

    // max pixel size is half the length of the diagonal
    m_maxPixelSize = .5 * DPoint2d::FromZero().Distance(DPoint2d::From(m_pixels.x, m_pixels.y));

    m_rootTile = new Tile(*this, QuadTree::TileId(0,0,0), nullptr);
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

    // If we're already loaded, look in existing AttachmentTrees so we don't reload them
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto attachId = stmt->GetValueId<DgnElementId>(0);
        auto tree = FindAttachment(attachId);

        if (!tree.IsValid())
            tree = new AttachmentTree(GetDgnDb(), attachId, &m_vp->GetRenderTarget()->GetSystem(), 256);

        attachments.push_back(tree);
        }

    // save new list of attachment trees
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
        attach->DrawInView(context);
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
    }
