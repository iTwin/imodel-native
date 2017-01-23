/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ViewContext.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

/**
* @addtogroup GROUP_ViewContext ViewContext Module
* A ViewContext holds the <i>current state</i> of an operation on a DgnViewport. A ViewContext must be first attached to a DgnViewport.
*/

//=======================================================================================
// @bsiclass
//=======================================================================================
struct     ILineStyleComponent
{
    virtual bool _IsContinuous() const = 0;
    virtual bool _HasWidth() const = 0;
    virtual double _GetLength() const = 0;
    virtual StatusInt _StrokeLineString(LineStyleContextR, Render::LineStyleSymbCR, DPoint3dCP, int nPts, bool isClosed) const = 0;
    virtual StatusInt _StrokeLineString2d(LineStyleContextR, Render::LineStyleSymbCR, DPoint2dCP, int nPts, double zDepth, bool isClosed) const = 0;
    virtual StatusInt _StrokeArc(LineStyleContextR, Render::LineStyleSymbCR, DPoint3dCP origin, RotMatrixCP rMatrix, double r0, double r1, double const* start, double const* sweep) const = 0;
    virtual StatusInt _StrokeBSplineCurve(LineStyleContextR context, Render::LineStyleSymbCR lsSymb, MSBsplineCurveCP) const = 0;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct  ILineStyle
{
    virtual Utf8CP _GetName() const = 0;
    virtual ILineStyleComponent const* _GetComponent() const = 0;
    virtual bool _IsSnappable() const = 0;
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

DEFINE_REF_COUNTED_PTR(IElemTopology)

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                     KeithBentley    04/01
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewContext : ScanCriteria, NonCopyableClass, CheckStop
{
    friend struct ViewController;
    friend struct SimplifyGraphic;

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
    DrawPurpose m_purpose;
    DRange3d m_npcSubRange;
    DMap4d m_worldToNpc;
    DMap4d m_worldToView;
    Render::FrustumPlanes m_frustumPlanes;
    DgnViewportP m_viewport = nullptr;
    ClipPrimitiveCPtr m_volume;

    void InvalidateScanRange() {m_scanRangeValid = false;}
    DGNPLATFORM_EXPORT virtual StatusInt _OutputGeometry(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _AddSubGraphic(Render::GraphicBuilderR, DgnGeometryPartId, TransformCR, Render::GeometryParamsR);
    virtual void _OutputGraphic(Render::GraphicR, GeometrySourceCP) {}
    virtual Render::GraphicP _GetCachedGraphic(GeometrySourceCR, double pixelSize) {return nullptr;}
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _StrokeGeometry(GeometrySourceCR source, double pixelSize);
    DGNPLATFORM_EXPORT virtual bool _WantAreaPatterns();
    DGNPLATFORM_EXPORT virtual void _DrawAreaPattern(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR, bool doCook);
    DGNPLATFORM_EXPORT virtual bool _WantLineStyles();
    DGNPLATFORM_EXPORT virtual void _DrawStyledCurveVector(Render::GraphicBuilderR, CurveVectorCR, Render::GeometryParamsR, bool doCook);
    DGNPLATFORM_EXPORT virtual IFacetOptionsPtr _UseLineStyleStroker(Render::GraphicBuilderR, Render::LineStyleSymbCR) const;
    DGNPLATFORM_EXPORT virtual StatusInt _InitContextForView();
    DGNPLATFORM_EXPORT virtual StatusInt _VisitGeometry(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual StatusInt _VisitHit(HitDetailCR);
    DGNPLATFORM_EXPORT virtual bool _AnyPointVisible(DPoint3dCP worldPoints, int nPts, double tolerance);
    DGNPLATFORM_EXPORT virtual void _InitScanRangeAndPolyhedron();
    DGNPLATFORM_EXPORT virtual bool _VisitAllModelElements();
    DGNPLATFORM_EXPORT virtual StatusInt _VisitDgnModel(GeometricModelR);
    virtual IPickGeomP _GetIPickGeom() {return nullptr;}
    virtual Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) = 0;
    virtual Render::GraphicPtr _CreateBranch(Render::GraphicBranch&, TransformCP trans, ClipVectorCP clips) = 0;
    DGNPLATFORM_EXPORT virtual void _SetupScanCriteria();
    virtual bool _WantUndisplayed() {return false;}
    DGNPLATFORM_EXPORT virtual void _AddViewOverrides(Render::OvrGraphicParamsR);
    DGNPLATFORM_EXPORT virtual void _AddContextOverrides(Render::OvrGraphicParamsR, GeometrySourceCP source);
    DGNPLATFORM_EXPORT virtual void _CookGeometryParams(Render::GeometryParamsR, Render::GraphicParamsR);
    DGNPLATFORM_EXPORT virtual StatusInt _ScanDgnModel(GeometricModelR model);
    DGNPLATFORM_EXPORT virtual bool _ScanRangeFromPolyhedron();
    DGNPLATFORM_EXPORT virtual void _SetDgnDb(DgnDbR);
    DGNPLATFORM_EXPORT RangeIndex::Traverser::Accept _CheckRangeTreeNode(RangeIndex::FBoxCR, bool) const override;
    DGNPLATFORM_EXPORT ScanCriteria::Stop _OnRangeElementFound(DgnElementId) override;
    DGNPLATFORM_EXPORT virtual StatusInt _VisitElement(DgnElementId elementId, bool allowLoad);
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
    void SetActiveVolume(ClipPrimitiveCR volume) {m_volume=&volume;}
    ClipPrimitiveCPtr GetActiveVolume() const {return m_volume;}
    void EnableStopAfterTimout(BeDuration::MilliSeconds timeout) {m_endTime = BeTimePoint::FromNow(timeout); m_stopAfterTimeout=true;}

    Render::GraphicBuilderPtr CreateGraphic(Render::Graphic::CreateParams const& params=Render::Graphic::CreateParams()) {return _CreateGraphic(params);}
    Render::GraphicPtr CreateBranch(Render::GraphicBranch& branch, TransformCP trans=nullptr, ClipVectorCP clips=nullptr) {return _CreateBranch(branch, trans, clips);}
    Render::GraphicPtr AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams) {return _AddSubGraphic(graphic, partId, subToGraphic, geomParams);}
    StatusInt VisitGeometry(GeometrySourceCR elem) {return _VisitGeometry(elem);}
    StatusInt VisitHit(HitDetailCR hit) {return _VisitHit(hit);}

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
    DGNPLATFORM_EXPORT double GetPixelSizeAtPoint(DPoint3dCP origin) const;

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
    //! @return the DgnViewport. nullptr if not attached to a DgnViewport.
    DgnViewportP GetViewport() const {return m_viewport;}

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

    StatusInt VisitElement(DgnElementId elementId, bool allowLoad) {return _VisitElement(elementId, allowLoad);}

    bool CheckStop() {return _CheckStop();}
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
    Render::OvrGraphicParams m_ovrParams;
    Render::TargetR m_target;

public:
    Render::OvrGraphicParams& GetOvrGraphicParams() {return m_ovrParams;}

    DGNVIEW_EXPORT RenderContext(DgnViewportR vp, DrawPurpose);
    void _AddContextOverrides(Render::OvrGraphicParamsR ovrMatSymb, GeometrySourceCP source) override;
    Render::GraphicP _GetCachedGraphic(GeometrySourceCR source, double pixelSize) override {return source.Graphics().Find(*m_viewport, pixelSize);}
    DGNVIEW_EXPORT Render::GraphicPtr _StrokeGeometry(GeometrySourceCR source, double pixelSize) override;
    Render::GraphicBuilderPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override {return m_target.CreateGraphic(params);}
    Render::GraphicPtr _CreateBranch(Render::GraphicBranch& branch, TransformCP trans, ClipVectorCP clips) override {return m_target.GetSystem()._CreateBranch(branch, trans, clips);}
    Render::TargetR GetTargetR() {return m_target;}
    DgnViewportR GetViewportR()  {return *m_viewport;}   // A RenderContext always have a viewport.
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
// @bsiclass                                                    Keith.Bentley   10/15
//=======================================================================================
struct SceneContext : RenderListContext
{
    DEFINE_T_SUPER(RenderListContext);
    bool _CheckStop() override;
    SceneContext(DgnViewportR vp, Render::GraphicListR scene, UpdatePlan const& plan);
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
// @bsiclass                                                    Keith.Bentley   10/15
//=======================================================================================
struct ProgressiveContext : RenderListContext
{
    DEFINE_T_SUPER(RenderListContext);
public:
    ProgressiveContext(DgnViewportR vp, Render::GraphicListR scene, UpdatePlan const& plan) : RenderListContext(vp, DrawPurpose::Progressive, &scene, plan) {}
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   10/15
//=======================================================================================
struct HealContext : RenderListContext
{
    DEFINE_T_SUPER(RenderListContext);
    void Flush(bool restart);
public:
    virtual void _HealElement(DgnElementId);
    HealContext(DgnViewportR vp, BSIRectCR, UpdatePlan const& plan);
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   02/16
//=======================================================================================
struct TerrainContext : RenderListContext
{
    DEFINE_T_SUPER(RenderListContext);
public:
    TerrainContext(DgnViewportR vp, Render::GraphicListR terrain, UpdatePlan const& plan) : RenderListContext(vp, DrawPurpose::CreateTerrain, &terrain, plan) {}
};

//=======================================================================================
//! @ingroup GROUP_ViewContext
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct DynamicsContext : RenderContext
{
    friend struct DgnPrimitiveTool;
private:
    Render::Task::Priority m_priority;
    Render::GraphicListR m_dynamics;
    void _OutputGraphic(Render::GraphicR graphic, GeometrySourceCP) override;
    DynamicsContext(DgnViewportR, Render::Task::Priority);
    ~DynamicsContext();
    void VisitWriteableElement(DgnElementCR element, IRedrawOperationP redrawOp);

public:
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
    bool    m_isFlash = false;
    Render::Decorations& m_decorations;
    Render::GraphicBranch* m_viewlet = nullptr;

    StatusInt VisitSheetHit(HitDetailCR hit);
    void _AddContextOverrides(Render::OvrGraphicParamsR ovrMatSymb, GeometrySourceCP source) override;
    void _OutputGraphic(Render::GraphicR graphic, GeometrySourceCP) override;
    StatusInt _VisitHit(HitDetailCR hit) override;
    DecorateContext(DgnViewportR vp, Render::Decorations& decorations) : RenderContext(vp, DrawPurpose::Decorate), m_decorations(decorations) {}

public:
    //! Display world coordinate graphic with flash/hilite treatment.
    DGNPLATFORM_EXPORT void AddFlashed(Render::GraphicR graphic, Render::OvrGraphicParamsCP ovr=nullptr);

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

    //! Set context to state used to flash elements.
    void SetIsFlash(bool isFlash) {m_isFlash = isFlash;}
};  

END_BENTLEY_DGN_NAMESPACE
