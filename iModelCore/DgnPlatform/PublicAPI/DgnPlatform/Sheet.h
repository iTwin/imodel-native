/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/Sheet.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/ViewDefinition.h>
#include <DgnPlatform/PickContext.h>
#include <DgnPlatform/TileTree.h>

#define USING_NAMESPACE_SHEET using namespace BentleyApi::Dgn::Sheet;
#define BIS_CLASS_ViewAttachment "ViewAttachment"

BEGIN_SHEET_NAMESPACE

//=======================================================================================
//! A Sheet::Model is a GraphicalModel2d that has the following characteristics:
//!     - Has fixed extents (is not infinite), specified in meters.
//!     - Can contain @b views of other models, like pictures pasted on a photo album.
//! @ingroup GROUP_DgnModel
// @bsiclass                                                    Keith.Bentley   10/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Model : GraphicalModel2d
{
    DGNMODEL_DECLARE_MEMBERS(BIS_CLASS_SheetModel, GraphicalModel2d);

protected:
    ModelCP _ToSheetModel() const override final {return this;}

    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;

public:
    //! construct a new SheetModel
    explicit Model(CreateParams const& params) : T_Super(params) {}

    //! Construct a SheetModel
    //! @param[in] params The CreateParams for the new SheetModel
    static ModelPtr Create(CreateParams const& params) {return new Model(params);}

    //! Create a SheetModel that breaks down the specified Sheet element
    DGNPLATFORM_EXPORT static ModelPtr Create(ElementCR sheet);

    //! Find the first SheetViewDefinition that displays the specified sheet model.
    DGNPLATFORM_EXPORT static DgnElementId FindFirstViewOfSheet(DgnDbR db, DgnModelId sheetModelId);
};

//=======================================================================================
//! Sheet::Element
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Element : Document
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Sheet, Document)
protected:
    static constexpr Utf8CP str_Scale() {return "Scale";}
    static constexpr Utf8CP str_Height() {return "Height";}
    static constexpr Utf8CP str_Number() {return "Number";}
    static constexpr Utf8CP str_Width() {return "Width";}
    static constexpr Utf8CP str_Template() {return "Template";}
    static constexpr Utf8CP str_Border() {return "Border";}

public:
    explicit Element(CreateParams const& params) : T_Super(params) {}

    //! Create a DgnCode for a Sheet in the specified DocumentListModel
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DocumentListModelCR model, Utf8CP name);

    //! Create a unique DgnCode for a Sheet within the specified DocumentListModel
    //! @param[in] model The uniqueness scope for the DgnCode
    //! @param[in] baseName The base name for the CodeValue. A suffix will be appended (if necessary) to make it unique within the specified scope.
    DGNPLATFORM_EXPORT static DgnCode CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] size The sheet size (meters)
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, DPoint2dCR size, Utf8CP name);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] sheetTemplate The sheet template. Maybe in valid if there is no template.
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, DgnElementId sheetTemplate, Utf8CP name);

    //! Get the drawing scale of the sheet
    double GetScale() const {return GetPropertyValueDouble(str_Scale());}

    //! Set the drawing scale of the sheet.
    //! @return DgnDbStatus::ReadOnly if the drawing scale is invalid.
    DgnDbStatus SetScale(double v) {return SetPropertyValue(str_Scale(), v);}

    //! Get the height of the sheet
    double GetHeight() const {return GetPropertyValueDouble(str_Height());}

    //! Set the height of the sheet.
    //! @return DgnDbStatus::ReadOnly if the height is controlled by a template
    DgnDbStatus SetHeight(double v) {return SetPropertyValue(str_Height(), v);}

    //! Get the width of the sheet
    double GetWidth() const {return GetPropertyValueDouble(str_Width());}

    //! Set the width of the sheet.
    //! @return DgnDbStatus::ReadOnly if the Width is controlled by a template
    DgnDbStatus SetWidth(double v) {return SetPropertyValue(str_Width(), v);}

    //! Get the sheet template, if any.
    //! @return an invalid ID if the sheet has no template.
    DgnElementId GetTemplate() const {return GetPropertyValueId<DgnElementId>(str_Template());}

    //! Set the sheet template.
    DgnDbStatus SetTemplate(DgnElementId v) {return SetPropertyValue(str_Template(), v, ECN::ECClassId());}

    //! Get the sheet border, if any.
    //! @return an invalid ID if the sheet has no border.
    DgnElementId GetBorder() const {return GetPropertyValueId<DgnElementId>(str_Border());}

        //! Set the sheet border.
    //! @return DgnDbStatus::ReadOnly if the Border is controlled by a template
    DgnDbStatus SetBorder(DgnElementId v) {return SetPropertyValue(str_Border(), v, ECN::ECClassId());}
};

//=======================================================================================
//! A Sheet::ViewAttachment is a reference to a View, placed on a sheet.
//! The attachment specifies the Id of the View and the position on the sheet.
// @bsiclass                                                      Paul.Connelly   10/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewAttachment : GraphicalElement2d
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, GraphicalElement2d);

protected:
    static constexpr Utf8CP str_View() {return "View";}
    static constexpr Utf8CP str_DisplayPriority() {return "DisplayPriority";}
    static constexpr Utf8CP str_Scale() {return "Scale";}
    DGNPLATFORM_EXPORT DgnDbStatus CheckValid() const;
    DgnDbStatus _OnInsert() override {auto status = CheckValid(); return DgnDbStatus::Success == status ? T_Super::_OnInsert() : status;}
    DgnDbStatus _OnUpdate(DgnElementCR original) override {auto status = CheckValid(); return DgnDbStatus::Success == status ? T_Super::_OnUpdate(original) : status;}
    DgnDbStatus _SetParentId(DgnElementId, DgnClassId) override {return DgnDbStatus::InvalidParent;}
    DgnDbStatus _OnChildInsert(DgnElementCR) const override {return DgnDbStatus::InvalidParent;}
    DgnDbStatus _OnChildUpdate(DgnElementCR original, DgnElementCR updated) const override {return DgnDbStatus::InvalidParent;}
    
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ViewAttachment));}
    static Placement2d ComputePlacement(DgnDbR db, DgnViewId viewId, DPoint2dCR origin, double scale);
    static double ComputeScale(DgnDbR db, DgnViewId viewId, ElementAlignedBox2dCR);

public:
    explicit ViewAttachment(CreateParams const& params) : T_Super(params) {}

    //! Construct an attachment in the case where you know the size of the attachment. The view scale will be computed.
    //! @param db   The DgnDb that will contain the attachment
    //! @param model The model in the DgnDb that will contain the attachment
    //! @param viewId   The view that is being attached
    //! @param cat      The attachment's category
    //! @param placement The attachment's origin and size on the sheet.
    DGNPLATFORM_EXPORT ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement);

    //! Construct an attachment in the case where you know the view scale. The placement's size will be computed.
    //! @param db   The DgnDb that will contain the attachment
    //! @param model The model in the DgnDb that will contain the attachment
    //! @param viewId   The view that is being attached
    //! @param cat      The attachment's category
    //! @param origin   The attachment's origin on the sheet
    //! @param scale    The view scale
    DGNPLATFORM_EXPORT ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, DPoint2dCR origin, double scale);

    DgnViewId GetAttachedViewId() const {return GetPropertyValueId<DgnViewId>(str_View());} //!< Get the Id of the view definition to be drawn by this attachment
    DgnDbStatus SetAttachedViewId(DgnViewId viewId) {return SetPropertyValue(str_View(), viewId, ECN::ECClassId());} //!< Set the view definition to be drawn
    int32_t GetDisplayPriority() const {return GetPropertyValueInt32(str_DisplayPriority());}
    DgnDbStatus SetDisplayPriority(int32_t v) {return SetPropertyValue(str_DisplayPriority(), v);}
    double GetScale() const {return GetPropertyValueDouble(str_Scale());}
    DgnDbStatus SetScale(double v) {return SetPropertyValue(str_Scale(), v);}

    //! Get the clip to be applied to this attachment, if any. 
    //! @return a clip vector or an invalid ptr if the attachment is not clipped.
    //! @see SetClip
    DGNPLATFORM_EXPORT ClipVectorPtr GetClip() const;

    //! Set the clip to be applied to this attachment.
    //! @return non-zero error status if the clip is invalid
    //! @see ClearClip
    DGNPLATFORM_EXPORT DgnDbStatus SetClip(ClipVectorCR);

    //! Set this attachment to be unclipped.
    //! @see SetClip
    DGNPLATFORM_EXPORT void ClearClip();
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/16
//=======================================================================================
namespace Attachment
{
    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   11/16
    //=======================================================================================
    struct Viewport : OffscreenViewport
    {
        Transform m_toParent = Transform::FromIdentity(); // attachment NPC to sheet world
        double m_biasDistance = 0.0; // distance in z to position tile in parent viewport's z-buffer (should be obtained by calling DepthFromDisplayPriority)
        Render::GraphicListPtr m_terrain;
        ClipVectorCPtr m_clips;

        virtual void _QueueScene(UpdatePlan const& updatePlan);
        virtual folly::Future<BentleyStatus> _CreateTile(TileTree::TileLoadStatePtr, Render::TexturePtr&, TileTree::QuadTree::Tile&, Point2dCR tileSize);
        void _AdjustAspectRatio(DPoint3dR, DVec3dR) override {}

        //! Get the transfrom from attachment view coordinates to sheet view coordinates
        DGNPLATFORM_EXPORT Transform GetTransformToSheet(DgnViewportCR sheetVp);

        //! Get the transfrom from sheet view coordinates to attachment view coordinates
        Transform GetTransformFromSheet(DgnViewportCR sheetVp) {Transform trans=GetTransformToSheet(sheetVp); trans.InverseOf(trans); return trans;}

        DGNVIEW_EXPORT Viewport();
        ClipVectorCP GetAttachClips() const {return m_clips.get();}
    };

    //=======================================================================================
    //! TileTree for displaying raster tiles generated from a sheet's view attachment
    // @bsiclass                                                    Keith.Bentley   11/16
    //=======================================================================================
    struct Tree : TileTree::QuadTree::Root
    {
        DEFINE_T_SUPER(TileTree::QuadTree::Root)
        DgnElementId m_attachmentId;
        RefCountedPtr<Viewport> m_viewport;
        DPoint2d m_scale; // scale factors to make square tiles
        uint32_t m_pixels;
        bool m_sceneQueued = false;
        bool m_sceneReady = false;

        bool Pick(PickContext&);
        void Draw(TerrainContextR);
        void Load(Render::SystemP);
        Utf8CP _GetName() const override {return "SheetTile";}
        Tree(DgnDbR db, Sheet::ViewController& sheetController, DgnElementId attachmentId, uint32_t tileSize);
        ~Tree(){ClearAllTiles();}
        DgnElementId GetAttachmentId() const {return m_attachmentId;}
    };

    struct Tile2dModel : TileTree::QuadTree::Tile
    {
        DEFINE_T_SUPER(TileTree::QuadTree::Tile)
        using T_Super::Tile;

        bool _HasChildren() const override {return false;}
        ChildTiles const* _GetChildren(bool create) const override {return nullptr;}
        TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr loads) override {return nullptr;}
        void _DrawGraphics(TileTree::DrawArgsR args, int depth) const override;
        TileTree::TilePtr _CreateChild(TileTree::QuadTree::TileId id) const override {return nullptr;}
        Tree& GetTree() const {return (Tree&) m_root;}
    };

    struct Tile : TileTree::QuadTree::Tile
    {
        DEFINE_T_SUPER(TileTree::QuadTree::Tile)

        struct Loader : TileTree::TileLoader
        {
            Render::TexturePtr m_texture;

            Loader(Utf8StringCR url, Tile& tile, TileTree::TileLoadStatePtr loads) : TileTree::TileLoader(url, tile, loads, tile._GetTileName()) {}
            BentleyStatus _LoadTile() override;
            folly::Future<BentleyStatus> _SaveToDb() override;
            folly::Future<BentleyStatus> _ReadFromDb() override;
            folly::Future<BentleyStatus> _GetFromSource() override;
        };

        Tile(Tree&, TileTree::QuadTree::TileId id, Tile const* parent);
        TileTree::TilePtr _CreateChild(TileTree::QuadTree::TileId id) const override {return new Tile(GetTree(), id, this);}
        Tree& GetTree() const {return (Tree&) m_root;}
        TileTree::TileLoaderPtr _CreateTileLoader(TileTree::TileLoadStatePtr loads) override {return new Loader(GetTree()._ConstructTileName(*this), *this, loads);}
    };

    DEFINE_REF_COUNTED_PTR(Tree)
};

//=======================================================================================
//! A Sheet::ViewController is used to control views of Sheet::Models
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct ViewController : Dgn::ViewController2d
{
    DEFINE_T_SUPER(ViewController2d);
    friend SheetViewDefinition;

protected:
    DPoint2d m_size;
    bvector<Attachment::TreePtr> m_attachments;

    ViewControllerCP _ToSheetView() const override {return this;}
    void _DrawView(ViewContextR) override;
    void _CreateTerrain(TerrainContextR context) override;
    void _LoadState() override;
    FitComplete _ComputeFitRange(FitContextR context) override;

    void DrawBorder(ViewContextR context) const;
    Attachment::TreePtr FindAttachment(DgnElementId attachId) const;
    AxisAlignedBox3d GetSheetExtents() const {return AxisAlignedBox3d(DPoint3d::FromZero(), DPoint3d::From(m_size.x,m_size.y,0));}
    ViewController(SheetViewDefinitionCR def) : ViewController2d(def) {}  //!< Construct a new SheetViewController.
};

//=======================================================================================
// Sheet::Handlers
//=======================================================================================
namespace Handlers
{
    //! The ElementHandler for Sheet::Elements
    struct EXPORT_VTABLE_ATTRIBUTE Element : dgn_ElementHandler::Document
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Sheet, Sheet::Element, Element, Document, DGNPLATFORM_EXPORT)
    };

    //! The handler for Sheet::ViewAttachment elements
    struct AttachmentElement : dgn_ElementHandler::Geometric2d
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_ViewAttachment, Sheet::ViewAttachment, AttachmentElement, Geometric2d, DGNPLATFORM_EXPORT);
    };

    //! The ModelHandler for Sheet::Model
    struct EXPORT_VTABLE_ATTRIBUTE Model :  dgn_ModelHandler::Model
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_SheetModel, Sheet::Model, Model, dgn_ModelHandler::Model, DGNPLATFORM_EXPORT)
    };

};

END_SHEET_NAMESPACE
