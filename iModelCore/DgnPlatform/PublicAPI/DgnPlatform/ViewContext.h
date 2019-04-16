/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "Render.h"
#include "ClipVector.h"
#include "TransformClipStack.h"
#include "ScanCriteria.h"
#include "IManipulator.h"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace TileTree {struct TileRequests;}
/**
* @addtogroup GROUP_ViewContext ViewContext Module
* A ViewContext holds the <i>current state</i> of an operation on a DgnViewport. A ViewContext must be first attached to a DgnViewport.
*/

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ILineStyleComponent
{
    virtual bool _IsContinuous() const = 0;
    virtual bool _HasWidth() const = 0;
    virtual double _GetMaxWidth() const = 0;
    virtual double _GetLength() const = 0;
    virtual StatusInt _StrokeLineString(LineStyleContextR, Render::LineStyleSymbR, DPoint3dCP, int nPts, bool isClosed) const = 0;
    virtual StatusInt _StrokeLineString2d(LineStyleContextR, Render::LineStyleSymbR, DPoint2dCP, int nPts, double zDepth, bool isClosed) const = 0;
    virtual StatusInt _StrokeArc(LineStyleContextR, Render::LineStyleSymbR, DEllipse3dCR, bool is3d, double zDepth, bool isClosed) const = 0;
    virtual StatusInt _StrokeBSplineCurve(LineStyleContextR, Render::LineStyleSymbR, MSBsplineCurveCR, bool is3d, double zDepth) const = 0;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ILineStyle
{
    virtual Utf8CP _GetName() const = 0;
    virtual ILineStyleComponent const* _GetComponent() const = 0;
    virtual bool _IsSnappable() const = 0;
    virtual Render::Texture* _GetTexture(double& textureWidth, ViewContextR, Render::GeometryParamsCR, bool createGeometryTexture) = 0;
    double GetMaxWidth () const { return nullptr == _GetComponent() ? 0.0 : _GetComponent()->_GetMaxWidth(); }
    double GetLength () const { return nullptr == _GetComponent() ? 0.0 : _GetComponent()->_GetLength(); }
};

//=======================================================================================
//! A description of a "hit" generated from picking logic. This is merely a RefCounted string.
// @bsiclass                                                    Keith.Bentley   01/17
//=======================================================================================
struct HitDescription : RefCountedBase
{
private:
    Utf8String m_descr;

public:
    HitDescription(Utf8StringCR descr) : m_descr(descr) {}
    Utf8String GetDescription() const {return m_descr;}
};

//=======================================================================================
//! Interface to supply additional topology information to describe subsequent geometry.
//! The current IElemTopology will be cloned and saved as part of the HitDetail
//! when picking. Can be used to make transient geometry locatable; call _SetElemTopology
//! before drawing the transient geometry during locate.
//! @note Always call _SetElemTopology(nullptr) to clear after drawing geometry.
//=======================================================================================
struct IElemTopology : IRefCounted
{
    //! Create a deep copy of this object.
    virtual IElemTopologyP _Clone() const = 0;

    //! Compare objects and return true if they should be considered the same.
    virtual bool _IsEqual(IElemTopologyCR) const = 0;

    //! Return GeometrySource to handle requests related to transient geometry (like locate) where we don't have an DgnElement.
    virtual GeometrySourceCP _ToGeometrySource() const {return nullptr;}

    //! Return IEditManipulator for interacting with transient geometry.
    //! @note Implementor is expected to check hit.GetDgnDb().IsReadonly().
    virtual IEditManipulatorPtr _GetTransientManipulator(HitDetailCR) const {return nullptr;}
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                     KeithBentley    04/01
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewContext : ScanCriteria, NonCopyableClass, CheckStop
{
    friend struct ViewController;
    friend struct SimplifyGraphic;

    struct AreaPatternTolerance
    {
    private:
        double  m_angleTolerance;
        double  m_chordTolerance;
    public:
        explicit AreaPatternTolerance(double chord=0.0, Angle angle=Angle::FromDegrees(20.0)) : m_angleTolerance(angle.Radians()), m_chordTolerance(chord) { }

        double GetAngleTolerance() const { return m_angleTolerance; }
        double GetChordTolerance() const { return m_chordTolerance; }
    };

protected:
    DgnDbP m_dgndb = nullptr;
    bool m_is3dView = true;
    bool m_wantMaterials = false;
    bool m_useNpcSubRange = false;
    bool m_ignoreViewRange = false;
    bool m_scanRangeValid = false;
    bool m_stopAfterTimeout = false;
    BeTimePoint m_endTime;     // abort after this time.
    Render::ViewFlags m_viewflags;
    ColorDef m_monochromeColor;
    DrawPurpose m_purpose;
    DRange3d m_npcSubRange;
    DMap4d m_worldToNpc;
    DMap4d m_worldToView;
    Render::FrustumPlanes m_frustumPlanes;
    DgnViewportP m_viewport = nullptr;
    ClipVectorCPtr m_volume;
    Utf8String m_auxChannel;

    void InvalidateScanRange() {m_scanRangeValid = false;}
    DGNPLATFORM_EXPORT virtual StatusInt _OutputGeometry(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual void _AddSubGraphic(Render::GraphicBuilderR, DgnGeometryPartId, TransformCR, Render::GeometryParamsR);
    virtual void _OutputGraphic(Render::GraphicR, GeometrySourceCP) {}
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _StrokeGeometry(GeometrySourceCR source, double pixelSize);
    DGNPLATFORM_EXPORT virtual bool _WantAreaPatterns();
    DGNPLATFORM_EXPORT virtual void _DrawAreaPattern(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR, bool doCook);
    virtual AreaPatternTolerance _GetAreaPatternTolerance(CurveVectorCR) {return AreaPatternTolerance();}
    DGNPLATFORM_EXPORT virtual bool _WantLineStyles();
    DGNPLATFORM_EXPORT virtual void _DrawStyledCurveVector(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR, bool doCook);
    DGNPLATFORM_EXPORT virtual StatusInt _InitContextForView();
    DGNPLATFORM_EXPORT virtual StatusInt _VisitGeometry(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual bool _AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance);
    DGNPLATFORM_EXPORT virtual void _InitScanRangeAndPolyhedron();
    DGNPLATFORM_EXPORT virtual bool _VisitAllModelElements();
    DGNPLATFORM_EXPORT virtual StatusInt _VisitDgnModel(GeometricModelR);
    virtual IPickGeomP _GetIPickGeom() {return nullptr;}
    virtual Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) = 0;
    virtual Render::GraphicPtr _CreateBranch(Render::GraphicBranch&, DgnDbR db, TransformCR tf, ClipVectorCP clips) = 0;
    DGNPLATFORM_EXPORT virtual void _SetupScanCriteria();
    virtual bool _WantUndisplayed() {return false;}
    DGNPLATFORM_EXPORT virtual void _CookGeometryParams(Render::GeometryParamsR, Render::GraphicParamsR);
    DGNPLATFORM_EXPORT virtual StatusInt _ScanDgnModel(GeometricModelR model);
    DGNPLATFORM_EXPORT virtual bool _ScanRangeFromPolyhedron();
    DGNPLATFORM_EXPORT virtual void _SetDgnDb(DgnDbR);
    DGNPLATFORM_EXPORT RangeIndex::Traverser::Accept _CheckRangeTreeNode(RangeIndex::FBoxCR, bool) const override;
    DGNPLATFORM_EXPORT ScanCriteria::Stop _OnRangeElementFound(DgnElementId) override;
    DGNPLATFORM_EXPORT virtual StatusInt _VisitElement(DgnElementId elementId, bool allowLoad);
    DGNPLATFORM_EXPORT virtual Render::MaterialPtr _GetMaterial(RenderMaterialId id) const;
    DGNPLATFORM_EXPORT virtual Render::TexturePtr _CreateTexture(Render::ImageCR image) const;
    DGNPLATFORM_EXPORT virtual Render::TexturePtr _CreateTexture(Render::ImageSourceCR source, Render::Image::BottomUp bottomUp) const;
    DGNPLATFORM_EXPORT virtual Render::SystemP _GetRenderSystem() const;
    DGNPLATFORM_EXPORT virtual Render::TargetP _GetRenderTarget() const;
    DGNPLATFORM_EXPORT virtual double _GetPixelSizeAtPoint(DPoint3dCP origin) const;
    virtual bool _WantGlyphBoxes(double sizeInPixels) const { return false; }
    DGNPLATFORM_EXPORT ViewContext();

public:
    DMap4dCR GetWorldToView() const {return m_worldToView;}
    DMap4dCR GetWorldToNpc() const {return m_worldToNpc;}
    bool GetWantMaterials() {return m_wantMaterials;};
    DGNPLATFORM_EXPORT void SetSubRectFromViewRect(BSIRectCP viewRect);
    DGNPLATFORM_EXPORT void SetSubRectNpc(DRange3dCR subRect);
    void SetWantMaterials(bool wantMaterials) {m_wantMaterials = wantMaterials;}
    bool IsUndisplayed(GeometrySourceCR source);
    bool ValidateScanRange() {return m_scanRangeValid ? true : _ScanRangeFromPolyhedron();}
    DGNPLATFORM_EXPORT StatusInt Attach(DgnViewportP vp, DrawPurpose purpose);
    bool VisitAllModelElements() {return _VisitAllModelElements();}
    DGNPLATFORM_EXPORT bool VisitAllViewElements(BSIRectCP updateRect=nullptr);
    StatusInt InitContextForView() {return _InitContextForView();}
    enum class WantBoresite : bool {Yes=true, No=false};
    DGNPLATFORM_EXPORT bool IsRangeVisible(DRange3dCR range, double tolerance=1.0e-8);
    DGNPLATFORM_EXPORT bool IsPointVisible(DPoint3dCR worldPoint, WantBoresite boresite, double tolerance=1.0e-8);
    DGNPLATFORM_EXPORT Frustum GetFrustum();
    Render::FrustumPlanes const& GetFrustumPlanes() const {return m_frustumPlanes;}
    void InitScanRangeAndPolyhedron() {_InitScanRangeAndPolyhedron();}
    StatusInt VisitDgnModel(GeometricModelR model){return _VisitDgnModel(model);}
    void OutputGraphic(Render::GraphicR graphic, GeometrySourceCP source) {_OutputGraphic(graphic, source);}
    void SetActiveVolume(ClipVectorCR volume) {m_volume=&volume;}
    ClipVectorCPtr GetActiveVolume() const {return m_volume;}
    Utf8StringCR GetActiveAuxChannel() const { return m_auxChannel; }
    void SetActiveAuxChannel(Utf8CP auxChannel) { m_auxChannel = nullptr == auxChannel ? Utf8String() : Utf8String(auxChannel); }
    void EnableStopAfterTimout(BeDuration::Milliseconds timeout) {m_endTime = BeTimePoint::FromNow(timeout); m_stopAfterTimeout=true;}

    Render::GraphicBuilderPtr CreateSceneGraphic(TransformCR tf=Transform::FromIdentity())
        { return _CreateGraphic(Render::GraphicBuilder::CreateParams::Scene(GetDgnDb(), tf, GetViewport())); }

    Render::TexturePtr CreateTexture(Render::ImageCR image) const { return _CreateTexture(image); }
    Render::TexturePtr CreateTexture(Render::ImageSourceCR source, Render::Image::BottomUp bottomUp=Render::Image::BottomUp::No) const
        { return _CreateTexture(source, bottomUp); }

    Render::GraphicPtr CreateBranch(Render::GraphicBranch& branch, DgnDbR db, TransformCR tf, ClipVectorCP clips=nullptr) {return _CreateBranch(branch, db, tf, clips);}
    void AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams) {return _AddSubGraphic(graphic, partId, subToGraphic, geomParams);}
    StatusInt VisitGeometry(GeometrySourceCR elem) {return _VisitGeometry(elem);}
    Render::MaterialPtr GetMaterial(RenderMaterialId id) const { return _GetMaterial(id); }

/** @name Coordinate Query and Conversion */
/** @{ */
    //! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] npcPts Input array in DgnCoordSystem::Npc.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToView(DPoint3dP viewPts, DPoint3dCP npcPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::Npc.
    //! @param[out] npcPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToNpc(DPoint3dP npcPts, DPoint3dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] npcPts Input array in DgnCoordSystem::Npc.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::Npc.
    //! @param[out] npcPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToNpc(DPoint3dP npcPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World.
    //! @param[in] nPts  Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out]  viewPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(Point2dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint2dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View.
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const;

    //! Calculate the size of a "pixel" at a given point in the current local coordinate system. This method can be used to
    //! approximate how large geometry in local coordinates will appear in DgnCoordSystem::View units.
    //! @param[in] origin The point at which the pixel size is calculated. This point is only relevant in camera views, where local coordinates
    //!                   closer to the eye are larger than those further from the eye. May be nullptr, in which case the center of the view is used.
    //! @return the length, in the current coordinate system units, of a unit bvector in the x direction in DgnCoordSystem::View, starting at \c origin.
    double GetPixelSizeAtPoint(DPoint3dCP origin) const { return _GetPixelSizeAtPoint(origin); }

    //! Get the current state of the ViewFlags for this context
    //! When a ViewContext is first attached to a DgnViewport, the ViewFlags are initialized
    //! from the DgnViewport's viewflags. However, during the course of an operation,
    //! the viewflags may be different than those on the DgnViewport.
    Render::ViewFlags GetViewFlags() const {return m_viewflags;}

    //! Sets the current state of the ViewFlags for this context
    void SetViewFlags(Render::ViewFlags flags) {m_viewflags = flags;}

    //! Get the DgnDb for this ViewContext.
    DgnDbR GetDgnDb() const {BeAssert(nullptr != m_dgndb); return *m_dgndb;}

    //! Set the project for this ViewContext when not attaching a viewport.
    void SetDgnDb(DgnDbR dgnDb) {return _SetDgnDb(dgnDb);}

    //! Get the DrawPurpose specified when this ViewContext was attached to the current DgnViewport.
    DrawPurpose GetDrawPurpose() const {return m_purpose;}

    //! Get the DgnViewport to which this ViewContext is attached. ViewContext's do not always have to be attached to an
    //! DgnViewport, so therefore callers must always test the result of this call for nullptr.
    //! In general, if you are using GetViewport() to access the Render::Target or Render::System associated with the ViewContext, you are doing it wrong.
    //! @return the DgnViewport. nullptr if not attached to a DgnViewport.
    DgnViewportP GetViewport() const {return m_viewport;}
    Render::SystemP GetRenderSystem() const {return _GetRenderSystem();}
    Render::TargetP GetRenderTarget() const {return _GetRenderTarget();}

    bool Is3dView() const {return m_is3dView;}
    DGNPLATFORM_EXPORT bool IsCameraOn() const;
/** @} */

/** @name Get/Set Current Display Parameters */
/** @{ */
    //! Modify the supplied "natural" GeometryParams by resolving effective symbology as required by the context.
    //! Initializes the supplied GraphicParams from the resolved GeometryParams.
    void CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicParamsR graphicParams) {_CookGeometryParams(geomParams, graphicParams);}

    //! Modify the supplied "natural" GeometryParams by resolving effective symbology as required by the context.
    //! Initializes a GraphicParams from the resolved GeometryParams and calls ActivateGraphicParams on the supplied Render::GraphicBuilderR.
    DGNPLATFORM_EXPORT void CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicBuilderR graphic);

    //! Get the IPickGeom interface for this ViewContext. Only contexts that are specific to picking will return a non-nullptr value.
    //! @return the IPickGeom interface for this context. May return nullptr.
    IPickGeomP GetIPickGeom() {return _GetIPickGeom();}
/** @} */

    bool WantAreaPatterns() {return _WantAreaPatterns();}
    void DrawAreaPattern(Render::GraphicBuilderR graphic, CurveVectorCR boundary, Render::GeometryParamsR params, bool doCook = true) {_DrawAreaPattern(graphic, boundary, params, doCook);}
    AreaPatternTolerance GetAreaPatternTolerance(CurveVectorCR boundary) {return _GetAreaPatternTolerance(boundary);}

/** @name Draw Geometry Using current Linestyle */
/** @{ */
    bool WantLineStyles() {return _WantLineStyles();}

    //! Draw a 2D or 3D curve vector using the current Linestyle. If there is no current Linestyle, draw a solid curve vector.
    //! @param[in]      graphic     Graphic to add to.
    //! @param[in]      curve       curve geometry
    //! @param[in]      params      GraphicParams for cooked style information.
    //! @param[in]      doCook      true if CookGeometryParams needs to be called for supplied GeometryParams.
    void DrawStyledCurveVector(Render::GraphicBuilderR graphic, CurveVectorCR curve, Render::GeometryParamsR params, bool doCook = true) {_DrawStyledCurveVector(graphic, curve, params, doCook);}
/** @} */

    StatusInt VisitElement(DgnElementId elementId, bool allowLoad=true) {return _VisitElement(elementId, allowLoad);}

    bool CheckStop() {return _CheckStop();}
    bool WantGlyphBoxes(double sizeInPixels) const {return _WantGlyphBoxes(sizeInPixels);}
}; // ViewContext

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct RenderContext : ViewContext
{
    DEFINE_T_SUPER(ViewContext);
    friend struct DgnViewport;

protected:
    Render::TargetR m_target;

    Render::GraphicBuilderPtr CreateGraphic(TransformCR tf, Render::GraphicType type)
        { return _CreateGraphic(Render::GraphicBuilder::CreateParams(GetViewportR(), tf, type)); }
public:
    DGNVIEW_EXPORT RenderContext(DgnViewportR vp, DrawPurpose);
    DGNVIEW_EXPORT Render::GraphicPtr _StrokeGeometry(GeometrySourceCR source, double pixelSize) override;
    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override {return m_target.CreateGraphic(params);}
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch& branch, DgnDbR db, TransformCR tf, ClipVectorCP clips) override {return m_target.GetSystem()._CreateBranch(std::move(branch), db, tf, clips);}
    Render::TargetR GetTargetR() {return m_target;}
    DgnViewportR GetViewportR()  {return *m_viewport;}   // A RenderContext always have a viewport.

    Render::GraphicBuilderPtr CreateSceneGraphic(TransformCR tf=Transform::FromIdentity()) { return CreateGraphic(tf, Render::GraphicType::Scene); }
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct RenderListContext : RenderContext
{
    DEFINE_T_SUPER(RenderContext);
    friend struct SpatialViewController;

protected:
    bool m_wantStroke = true;
    BeDuration m_checkStopInterval;
    int32_t m_checkStopElementSkip = 10;
    int32_t m_checkStopElementCount = 0;
    BeTimePoint m_nextCheckStop;
    Render::GraphicListPtr m_list;
    UpdateAbort m_abortReason = UpdateAbort::None;
    UpdatePlan const& m_plan;

    Render::GraphicPtr _StrokeGeometry(GeometrySourceCR source, double pixelSize) override {return m_wantStroke ? T_Super::_StrokeGeometry(source,pixelSize) : nullptr;}
    void _OutputGraphic(Render::GraphicR graphic, GeometrySourceCP) override;
    bool _CheckStop() override;
    int AccumulateMotion();
    bool DoCheckStop();

public:    
    void EnableCheckStop(BeDuration stopInterval, int const* motionTolerance);
    void SetNoStroking(bool val) {m_wantStroke=!val;}
    UpdatePlan const& GetUpdatePlan() const {return m_plan;}
    Render::GraphicListPtr GetList() const {return m_list;}
    RenderListContext(DgnViewportR vp, DrawPurpose purpose, Render::GraphicList* list, UpdatePlan const& plan);
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
//! Accumulates the list of Graphics which comprise the current scene, along with a set
//! of Tiles which need to be loaded to complete the scene.
//! @note The TileRequests object accumulates tile requests for multiple SceneContexts;
//! therefore the requests are not actually processed until all SceneContexts have been
//! processed. This allows previously-queued tile loads to be canceled if they are no
//! longer required for any Scene.
// @bsiclass                                                    Keith.Bentley   10/15
//=======================================================================================
struct SceneContext : RenderListContext
{
    DEFINE_T_SUPER(RenderListContext);
protected:
    SceneContext(DgnViewportR vp, Render::GraphicListR scene, UpdatePlan const& plan, TileTree::TileRequests& requests, DrawPurpose purpose);
public:
    TileTree::TileRequests& m_requests;

    SceneContext(DgnViewportR vp, Render::GraphicListR scene, UpdatePlan const& plan, TileTree::TileRequests& requests)
        : SceneContext(vp, scene, plan, requests, DrawPurpose::CreateScene) { }

    bool _CheckStop() override;
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct ThumbnailContext : SceneContext
{
    DEFINE_T_SUPER(SceneContext);

    ThumbnailContext(DgnViewportR vp, Render::GraphicListR scene, UpdatePlan const& plan, TileTree::TileRequests& requests)
        : SceneContext(vp, scene, plan, requests, DrawPurpose::CaptureThumbnail) { }
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   03/16
//=======================================================================================
struct RedrawContext : RenderListContext
{
    DEFINE_T_SUPER(RenderListContext);
public:
    RedrawContext(DgnViewportR vp, Render::GraphicListR draws, UpdatePlan const& plan) : RenderListContext(vp, DrawPurpose::Redraw, &draws, plan) {}
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct DynamicsContext : RenderContext
{
    friend struct DgnPrimitiveTool;
private:
    Render::DecorationListR m_dynamics;
    Render::OvrGraphicParams m_ovrParams;
    Render::Task::Priority m_priority;

    void _OutputGraphic(Render::GraphicR graphic, GeometrySourceCP) override;
    virtual void _AddContextOverrides(Render::OvrGraphicParamsR ovrMatSymb, GeometrySourceCP source);
    bool _WantUndisplayed() override {return true;} //!< Writeable copy of undisplayed element being drawn in dynamics should not be skipped...only want to check this for persistent elements...
    DynamicsContext(DgnViewportR, Render::Task::Priority);
    ~DynamicsContext();
    void VisitWriteableElement(DgnElementCR element, IRedrawOperationP redrawOp);

public:
    Render::OvrGraphicParams& GetOvrGraphicParams() {return m_ovrParams;}

    DGNVIEW_EXPORT void DrawElements(DgnElementCPtrVec const& elements, IRedrawOperationP redrawOp=nullptr);
    DGNVIEW_EXPORT void DrawElements(DgnElementIdSet const& elemIds, IRedrawOperationP redrawOp=nullptr);
    DGNVIEW_EXPORT void DrawElement(DgnElementCR element, IRedrawOperationP redrawOp=nullptr);
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct DecorateContext : RenderContext
{
    DEFINE_T_SUPER(RenderContext);
    friend struct DgnViewport;
private:
    Render::Decorations& m_decorations;
    Render::GraphicBranch* m_viewlet = nullptr;
    Render::OvrGraphicParams m_ovrParams;

    BentleyStatus DrawNormalHit(HitDetailCR hit);
    BentleyStatus DrawSheetHit(HitDetailCR hit);
    void _OutputGraphic(Render::GraphicR graphic, GeometrySourceCP) override;
    DecorateContext(DgnViewportR vp, Render::Decorations& decorations) : RenderContext(vp, DrawPurpose::Decorate), m_decorations(decorations) {}

public:
    //! Display world coordinate graphic with scene lighting and z testing.
    DGNPLATFORM_EXPORT void AddNormal(Render::GraphicR graphic);

    //! Display world coordinate graphic with smooth shading, default lighting, and z testing enabled.
    DGNPLATFORM_EXPORT void AddWorldDecoration(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovr=nullptr);

    //! Display world coordinate graphic with smooth shading, default lighting, and z testing disabled.
    DGNPLATFORM_EXPORT void AddWorldOverlay(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovr=nullptr);

    //! Display view coordinate graphic with smooth shading, default lighting, and z testing disabled.
    DGNPLATFORM_EXPORT void AddViewOverlay(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovr=nullptr);

    //! Display sprite as view overlay graphic.
    DGNPLATFORM_EXPORT void AddSprite(Render::ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency);

    //! @private
    DGNPLATFORM_EXPORT void DrawStandardGrid(DPoint3dR gridOrigin, RotMatrixR rMatrix, DPoint2d spacing, uint32_t gridsPerRef, bool isoGrid=false, Point2dCP fixedRepetitions=nullptr);

    //! @private
    DGNPLATFORM_EXPORT BentleyStatus DrawHit(HitDetailCR hit);

    //! Display view coordinate graphic as background with smooth shading, default lighting, and z testing disabled. e.g., a sky box.
    DGNPLATFORM_EXPORT void SetViewBackground(Render::GraphicR graphic);

    Render::OvrGraphicParams& GetOvrGraphicParams() {return m_ovrParams;}

    Render::GraphicBuilderPtr CreateViewBackground(TransformCR tf=Transform::FromIdentity()) { return CreateGraphic(tf, Render::GraphicType::ViewBackground); }
    Render::GraphicBuilderPtr CreateWorldDecoration(TransformCR tf=Transform::FromIdentity()) { return CreateGraphic(tf, Render::GraphicType::WorldDecoration); }
    Render::GraphicBuilderPtr CreateWorldOverlay(TransformCR tf=Transform::FromIdentity()) { return CreateGraphic(tf, Render::GraphicType::WorldOverlay); }
    Render::GraphicBuilderPtr CreateViewOverlay(TransformCR tf=Transform::FromIdentity()) { return CreateGraphic(tf, Render::GraphicType::ViewOverlay); }
};  

END_BENTLEY_DGN_NAMESPACE
