/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
//! Describes the geometry of the sheet border in view coordinates or world coordinates.
// @bsistruct                                                   Paul.Connelly   03/18
//=======================================================================================
struct Border
{
private:
    DPoint2d                m_rect[5];
    DPoint2d                m_shadow[7];
    Render::GradientSymbPtr m_gradient;

    Border(ViewContextCP context, DPoint2dCR size);
public:
    enum class CoordSystem { View, World };

    //! Create a border of the specified size in view or world coordinates
    Border(ViewContextCR context, DPoint2dCR size, CoordSystem coords) : Border(CoordSystem::View == coords ? &context : nullptr, size) { }

    //! Create a border of the specified size in world coordinates
    explicit Border(DPoint2dCR size) : Border(nullptr, size) { }

    void AddToBuilder(Render::GraphicBuilderR) const;
    DRange2d GetRange() const;
};

//=======================================================================================
//! A Sheet::Model is a GraphicalModel2d that has the following characteristics:
//!     - Has finite  extents, specified in meters.
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

    //! Draw border graphics (static, called during update)
    DGNPLATFORM_EXPORT static Render::GraphicPtr CreateBorder(DecorateContextR viewContext, DPoint2dCR size);

    //! Get the sheet size.
    DPoint2d GetSheetSize() const;

    //! Get the sheet extents.
    AxisAlignedBox3d GetSheetExtents() const;

    //! Get the sheet attachment IDs.
    DGNPLATFORM_EXPORT bvector<DgnElementId> GetSheetAttachmentIds() const;

    //! Get the sheet attachment views.
    DGNPLATFORM_EXPORT bvector<ViewDefinitionCPtr> GetSheetAttachmentViews(DgnDbR db) const;


    //! @private
    DGNPLATFORM_EXPORT void DumpAttachments(int indent = 0) const;
};

//=======================================================================================
//! Sheet::Element
//! @ingroup GROUP_DgnElement
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Element : Document
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Sheet, Document)

public:
    BE_PROP_NAME(Scale)
    BE_PROP_NAME(Height)
    BE_PROP_NAME(Number)
    BE_PROP_NAME(Width)
    BE_PROP_NAME(Template)
    BE_PROP_NAME(Border)

    explicit Element(CreateParams const& params) : T_Super(params) {}

    //! Create a DgnCode for a Sheet in the specified DocumentListModel
    DGNPLATFORM_EXPORT static DgnCode CreateCode(DocumentListModelCR model, Utf8StringCR name);

    //! Create a unique DgnCode for a Sheet within the specified DocumentListModel
    //! @param[in] model The uniqueness scope for the DgnCode
    //! @param[in] baseName The base name for the CodeValue. A suffix will be appended (if necessary) to make it unique within the specified scope.
    //! @private
    DGNPLATFORM_EXPORT static DgnCode CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] size The sheet size (meters)
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, DPoint2dCR size, Utf8StringCR name);

    //! Creates a new Sheet in the specified InformationModel
    //! @param[in] model The model where the Sheet element will be inserted by the caller.
    //! @param[in] scale The sheet's drawing scale
    //! @param[in] sheetTemplate The sheet template. Maybe in valid if there is no template.
    //! @param[in] name This name will be used to form the Sheet element's DgnCode
    //! @return a new, non-persistent Sheet element. @note It is the caller's responsibility to call Insert on the returned element in order to make it persistent.
    DGNPLATFORM_EXPORT static ElementPtr Create(DocumentListModelCR model, double scale, DgnElementId sheetTemplate, Utf8StringCR name);

    //! Get the drawing scale of the sheet
    double GetScale() const {return GetPropertyValueDouble(prop_Scale());}

    //! Set the drawing scale of the sheet.
    //! @return DgnDbStatus::ReadOnly if the drawing scale is invalid.
    DgnDbStatus SetScale(double v) {return SetPropertyValue(prop_Scale(), v);}

    //! Get the height of the sheet
    double GetHeight() const {return GetPropertyValueDouble(prop_Height());}

    //! Set the height of the sheet.
    //! @return DgnDbStatus::ReadOnly if the height is controlled by a template
    DgnDbStatus SetHeight(double v) {return SetPropertyValue(prop_Height(), v);}

    //! Get the width of the sheet
    double GetWidth() const {return GetPropertyValueDouble(prop_Width());}

    //! Set the width of the sheet.
    //! @return DgnDbStatus::ReadOnly if the Width is controlled by a template
    DgnDbStatus SetWidth(double v) {return SetPropertyValue(prop_Width(), v);}

    //! Get the sheet template, if any.
    //! @return an invalid ID if the sheet has no template.
    DgnElementId GetTemplate() const {return GetPropertyValueId<DgnElementId>(prop_Template());}

    //! Set the sheet template.
    DgnDbStatus SetTemplate(DgnElementId v) {return SetPropertyValue(prop_Template(), v, ECN::ECClassId());}

    //! Get the sheet border, if any.
    //! @return an invalid ID if the sheet has no border.
    DgnElementId GetBorder() const {return GetPropertyValueId<DgnElementId>(prop_Border());}

        //! Set the sheet border.
    //! @return DgnDbStatus::ReadOnly if the Border is controlled by a template
    DgnDbStatus SetBorder(DgnElementId v) {return SetPropertyValue(prop_Border(), v, ECN::ECClassId());}
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
    BE_PROP_NAME(View);

    BE_JSON_NAME(details);
    BE_JSON_NAME(displayPriority);
    BE_JSON_NAME(scale);
    BE_JSON_NAME(clip);

    explicit ViewAttachment(CreateParams const& params) : T_Super(params) {}

    //! Construct an attachment in the case where you know the size of the attachment. The view scale will be computed.
    //! @param db   The DgnDb that will contain the attachment
    //! @param model The model in the DgnDb that will contain the attachment
    //! @param viewId   The view that is being attached
    //! @param cat      The attachment's category
    //! @param placement The attachment's origin and size on the sheet.
    DGNPLATFORM_EXPORT ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, Placement2dCR placement);

    //! Construct an attachment in the case where you know the view scale. Th5e placement's size will be computed.
    //! @param db   The DgnDb that will contain the attachment
    //! @param model The model in the DgnDb that will contain the attachment
    //! @param viewId   The view that is being attached
    //! @param cat      The attachment's category
    //! @param origin   The attachment's origin on the sheet
    //! @param scale    The view scale
    DGNPLATFORM_EXPORT ViewAttachment(DgnDbR db, DgnModelId model, DgnViewId viewId, DgnCategoryId cat, DPoint2dCR origin, double scale);

    DgnViewId GetAttachedViewId() const {return GetPropertyValueId<DgnViewId>(prop_View());} //!< Get the Id of the view definition to be drawn by this attachment
    DgnDbStatus SetAttachedViewId(DgnViewId viewId) {return SetPropertyValue(prop_View(), viewId, ECN::ECClassId());} //!< Set the view definition to be drawn

    Utf8CP GetDetails() const { return m_jsonProperties[json_details()].asCString(nullptr); }
    void SetDetails(Utf8CP details) { details ? m_jsonProperties[json_details()] = details : m_jsonProperties.removeMember(json_details()); }

    int32_t GetDisplayPriority() const { return m_jsonProperties[json_displayPriority()].asInt(0); }
    void SetDisplayPriority(int32_t priority) { m_jsonProperties[json_displayPriority()] = priority; }

    double GetScale() const { return m_jsonProperties[json_scale()].asDouble(0.0); }
    void SetScale(double scale) { m_jsonProperties[json_scale()] = scale; }

    //! Get the clip to be applied to this attachment, if any. 
    //! @return a clip vector or an invalid ptr if the attachment is not clipped.
    //! @see SetClip
    DGNPLATFORM_EXPORT ClipVectorPtr GetClip() const;

    //! Get the clip to be applied to this attachment, or if none is defined,
    //! create one based on the attachment's range.
    //! @see GetClip
    ClipVectorPtr GetOrCreateClip(TransformCP transform=nullptr) const;

    //! Get a rectangular clip based on the range of the ViewAttachment.
    ClipVectorPtr CreateBoundaryClip() const;

    //! Set the clip to be applied to this attachment.
    //! @see ClearClip
    DGNPLATFORM_EXPORT void SetClip(ClipVectorCR);

    //! Clear the clip for this attachment.
    //! @see SetClip
    DGNPLATFORM_EXPORT void ClearClip();
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   11/16
//=======================================================================================
namespace Attachment
{
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Root);
    DEFINE_REF_COUNTED_PTR(Root);
    DEFINE_POINTER_SUFFIX_TYPEDEFS(Root3d);

    //=======================================================================================
    // Describes the state of the scene for a given level of the tile tree.
    // All tiles on a given level use the same scene to generate their graphics.
    // @bsistruct                                                   Paul.Connelly   03/18
    //=======================================================================================
    enum class State
    {
        NotLoaded,  // We haven't tried to create the scene for this level of the tree
        Empty,      // This level of the tree has an empty scene
        Loading,    // All of the Roots for this level of the tree have been created and we are loading their tiles
        Ready,      // All of the tiles required for this level of the tree are ready for rendering
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   11/16
    //=======================================================================================
    struct Viewport : OffscreenViewport
    {
        Render::TexturePtr m_texture;
        uint32_t m_texSize;
        uint32_t m_sceneDepth = 0xffffffff;
        Render::GraphicListPtr m_scene;
        bool m_rendering = false;

        Transform m_toParent = Transform::FromIdentity(); // attachment NPC to sheet world
        double m_biasDistance = 0.0; // distance in z to position tile in parent viewport's z-buffer (should be obtained by calling DepthFromDisplayPriority)
        Render::GraphicListPtr m_terrain;
        ClipVectorCPtr m_clips;

        DGNVIEW_EXPORT virtual State _CreateScene(UpdatePlan const& updatePlan, State currentState);
        DGNVIEW_EXPORT virtual Render::Image _RenderImage();
        DGNVIEW_EXPORT virtual void _RenderTexture();
        void QueueScene(SceneContextR);
        virtual folly::Future<BentleyStatus> _CreateTile(TileTree::TileLoadStatePtr, Render::TexturePtr&, TileTree::QuadTree::Tile&, Point2dCR tileSize);
        void _AdjustAspectRatio(DPoint3dR, DVec3dR) override {}

        //! Get the transfrom from attachment view coordinates to sheet view coordinates
        DGNPLATFORM_EXPORT Transform GetTransformToSheet(DgnViewportCR sheetVp);

        //! Get the transfrom from sheet view coordinates to attachment view coordinates
        Transform GetTransformFromSheet(DgnViewportCR sheetVp) {Transform trans=GetTransformToSheet(sheetVp); trans.InverseOf(trans); return trans;}

        DGNVIEW_EXPORT Viewport();
        ClipVectorCP GetAttachClips() const {return m_clips.get();}
        void SetSceneDepth(uint32_t depth, Root3dR tree);
    };

    //=======================================================================================
    // @bsistruct                                                   Mark.Schlosser  02/2018
    //=======================================================================================
    struct Root : TileTree::Root
    {
        DEFINE_T_SUPER(TileTree::Root);

        ColorDef m_boundingBoxColor = ColorDef::DarkOrange();
        double m_biasDistance;
        ClipVectorPtr m_clip;
    protected:
        Root(DgnModelId modelId, Sheet::ViewController& sheetController, ViewAttachmentCR attach, SceneContextR context, Dgn::ViewControllerR view);

        void _OnAddToRangeIndex(DRange3dCR, DgnElementId) override { }
        void _OnRemoveFromRangeIndex(DRange3dCR, DgnElementId) override { }
        void _OnUpdateRangeIndex(DRange3dCR, DRange3dCR, DgnElementId) override { }
        void _OnProjectExtentsChanged(AxisAlignedBox3dCR) override { }

        ClipVectorCP _GetClipVector() const override { return m_clip.get(); }
    };
}

//=======================================================================================
//! A Sheet::ViewController is used to control views of Sheet::Models
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct ViewController : Dgn::ViewController2d
{
    DEFINE_T_SUPER(ViewController2d);
    friend SheetViewDefinition;
    friend struct Sheet::Attachment::Root;

    struct Attachments;
    struct Attachment : RefCountedBase
    {
    protected:
        DgnElementId m_id;

        explicit Attachment(DgnElementId id) : m_id(id) { }
    public:
        static RefCountedPtr<Attachment> Create(DgnElementId id, DgnDbR db);

        DgnElementId GetId() const { return m_id; }

        virtual bool _Load(DgnDbR db, ViewController& sheetController, SceneContextR context) = 0;
        virtual Sheet::Attachment::RootP _GetTree() const = 0;

        void CancelAllTileLoads()
            {
            auto tree = _GetTree();
            if (nullptr != tree)
                tree->CancelAllTileLoads();
            }
        void WaitForAllLoads()
            {
            auto tree = _GetTree();
            if (nullptr != tree)
                tree->WaitForAllLoads();
            }
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Attachment);
    DEFINE_REF_COUNTED_PTR(Attachment);

    struct Attachments
    {
        using List = bvector<AttachmentPtr>;
        using iterator = List::iterator;
        using const_iterator = List::const_iterator;
    private:
        List m_list;
        bool m_allAttachmentsLoaded = true; // Does every attachment have a valid RootPtr?

        void UpdateAllLoaded();
    public:
        Attachments() = default;
        Attachments(Attachments&& src) = default;
        Attachments& operator=(Attachments&& src) = default;

        bool AllLoaded() const { return m_allAttachmentsLoaded; }

        size_t size() const { return m_list.size(); }
        const_iterator begin() const { return m_list.begin(); }
        const_iterator end() const { return m_list.end(); }
        iterator begin() { return m_list.begin(); }
        iterator end() { return m_list.end(); }
        void clear() { m_list.clear(); m_allAttachmentsLoaded = true; }

        Attachment* Find(DgnElementId id)
            {
            for (auto& attach : *this)
                if (attach->GetId() == id)
                    return attach.get();

            return nullptr;
            }

        void Add(AttachmentR src);
        bool Load(size_t index, DgnDbR db, ViewController& sheetController, SceneContextR context);
        void InitBoundingBoxColors();
    };

    DEFINE_POINTER_SUFFIX_TYPEDEFS_NO_STRUCT(Attachments);
protected:
    DPoint2d m_size;
    Attachments m_attachments;
    bool m_allAttachmentTilesReady = true;

    ViewControllerCP _ToSheetView() const override {return this;}
    void _DrawView(ViewContextR) override;
    void _LoadState() override;
    BentleyStatus _CreateScene(SceneContextR) override;
    FitComplete _ComputeFitRange(FitContextR context) override;
    void _DrawDecorations(DecorateContextR context) override;
    void _OnRenderFrame() override;

    void DrawBorder(ViewContextR context) const;
    ViewController(SheetViewDefinitionCR def) : ViewController2d(def) {}  //!< Construct a new SheetViewController.

    Attachment* FindAttachment(DgnElementId id) { return m_attachments.Find(id); }
    static bool WantRenderAttachments();

    DGNPLATFORM_EXPORT void _CancelAllTileLoads(bool wait) override;
    DGNPLATFORM_EXPORT void _UnloadAllTileTrees() override;
public:
    AttachmentsCR GetAttachments() const { return m_attachments; }
    AttachmentsR GetAttachments() { return m_attachments; }
    void MarkAttachmentSceneIncomplete() { m_allAttachmentTilesReady = false; }
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
    struct EXPORT_VTABLE_ATTRIBUTE Model :  dgn_ModelHandler::Geometric2d
    {
        MODELHANDLER_DECLARE_MEMBERS(BIS_CLASS_SheetModel, Sheet::Model, Model, dgn_ModelHandler::Geometric2d, DGNPLATFORM_EXPORT)
    };
};

END_SHEET_NAMESPACE
