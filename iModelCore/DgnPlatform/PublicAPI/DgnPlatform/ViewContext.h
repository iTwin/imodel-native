/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ViewContext.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

/*=================================================================================**//**
 @addtogroup ViewContext
 A ViewContext holds the <i>current state</i> of an operation on a DgnViewport. A ViewContext must be first
 \e attached to a DgnViewport to be useful, and must be \e detached from the DgnViewport to free any memory associated with its internal state.
 @beginGroup 
+===============+===============+===============+===============+===============+======*/

//=======================================================================================
//! @note This is stored persistently as part of the Reference Dynamic View Settings XAttributes, and thus cannot be changed
//=======================================================================================
struct  ClipVolumeOverrides
{
    struct
    {
        unsigned    m_display:1;        //!< If true, the clip volume area displays.
        unsigned    m_disableLocate:1;  //!< If true, the elements in the clip volume area cannot be located.
        unsigned    m_disableSnap:1;    //!< If true, the elements in the clip volume area cannot be snaped.
        unsigned    m_reflected:1;      //!< If true, the clip volume area is reflected.
        unsigned    m_unused:28;
    } m_flags;

    int32_t    m_styleIndex;       //!< Display style of the clip volume area. -1 to match that view.

    int32_t GetDisplayStyleIndex() const {return m_styleIndex;}
    void SetDisplayStyleIndex(int32_t index) {m_styleIndex = index;}
    bool IsEqual(const ClipVolumeOverrides& other) const
        {
        if (m_flags.m_display != other.m_flags.m_display)
            return false;
        if (m_flags.m_disableLocate != other.m_flags.m_disableLocate)
            return false;
        if (m_flags.m_disableSnap != other.m_flags.m_disableSnap)
            return false;
        if (m_styleIndex != other.m_styleIndex)
            return false;

        return true;
        }
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct     ILineStyleComponent
{
    virtual bool _IsContinuous() const = 0;
    virtual bool _HasWidth() const = 0;
    virtual double _GetLength() const = 0;
    virtual StatusInt _StrokeLineString(ViewContextP, Render::LineStyleSymbP, DPoint3dCP, int nPts, bool isClosed) const = 0;
    virtual StatusInt _StrokeLineString2d(ViewContextP, Render::LineStyleSymbP, DPoint2dCP, int nPts, double zDepth, bool isClosed) const = 0;
    virtual StatusInt _StrokeArc(ViewContextP, Render::LineStyleSymbP, DPoint3dCP origin, RotMatrixCP rMatrix, double r0, double r1, double const* start, double const* sweep, DPoint3dCP range) const = 0;
    virtual StatusInt _StrokeBSplineCurve(ViewContextP context, Render::LineStyleSymbP lsSymb, MSBsplineCurveCP, double const* tolerance) const = 0;
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
// @bsiclass
//=======================================================================================
struct RangeNodeCheck
{
    virtual ScanCriteria::Result _CheckNodeRange(ScanCriteriaCR, DRange3dCR, bool is3d) = 0;
};


//=======================================================================================
//! Interface to supply additional topology information that describes the subsequent geometry.
//! The ViewContext's current IElemTopology will be cloned and saved as part of the HitDetail
//! when picking. Can be used to make transient geometry locatable; set context.SetElemTopology
//! before drawing the geometry (ex. IViewTransients) and implement ITransientGeometryHandler.
//! @note Always call context.SetElemTopology(nullptr) after drawing geometry.
//=======================================================================================
struct IElemTopology : IRefCounted
{
    //! Create a deep copy of this object.
    virtual IElemTopologyP _Clone() const = 0;

    //! Compare objects and return true if they should be considered the same.
    virtual bool _IsEqual (IElemTopologyCR) const = 0;

    //! Return GeometrySource to handle requests related to transient geometry (like locate) where we don't have an DgnElement.
    virtual GeometrySourceCP _ToGeometrySource() const {return nullptr;}

    //! Return IEditManipulator for interacting with transient geometry.
    //! @note Implementor is expected to check hit.GetDgnDb().IsReadonly().
    virtual IEditManipulatorPtr _GetTransientManipulator (HitDetailCR) const {return nullptr;}
};

DEFINE_REF_COUNTED_PTR(IElemTopology)

//=======================================================================================
// @bsiclass                                                     KeithBentley    04/01
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewContext : NonCopyableClass, CheckStop, RangeNodeCheck
{
    friend struct ViewController;
    friend struct SimplifyGraphic;

public:

    //=======================================================================================
    // @bsiclass                                                     KeithBentley    04/01
    //=======================================================================================
    struct ContextMark
    {
        ViewContextP m_context;
        size_t       m_transClipMark;

    public:
        DGNPLATFORM_EXPORT explicit ContextMark(ViewContextP context);
        ~ContextMark() {Pop();}
        DGNPLATFORM_EXPORT void Pop();
        DGNPLATFORM_EXPORT void SetNow();
        void Init(ViewContextP context) {m_transClipMark = 0; m_context = context;}
    };

    //=======================================================================================
    // @bsiclass                                                     Brien.Bastings  11/07
    //=======================================================================================
    struct  ClipStencil
    {
    private:
        GeometrySourceCR    m_geomSource;
        Render::GraphicPtr  m_tmpQvElem;
        CurveVectorPtr      m_curveVector;

    public:
        DGNPLATFORM_EXPORT Render::GraphicPtr GetQvElem(ViewContextR);
        DGNPLATFORM_EXPORT CurveVectorPtr GetCurveVector();
        GeometrySourceCR GetGeomSource() {return m_geomSource;}

        DGNPLATFORM_EXPORT explicit ClipStencil(GeometrySourceCR);
        DGNPLATFORM_EXPORT ~ClipStencil();
    };

protected:
    DgnDbP                  m_dgnDb = nullptr;
    bool                    m_isAttached = false;
    bool                    m_is3dView = true;
    bool                    m_wantMaterials = false;
    bool                    m_useNpcSubRange = false;
    bool                    m_ignoreViewRange = false;
    bool                    m_scanRangeValid = false;
    bool                    m_stopAfterTimeout = false;
    uint64_t                m_endTime = 0;     // abort after this time.
    Render::ViewFlags       m_viewflags;
    DrawPurpose             m_purpose;
    DRange3d                m_npcSubRange;
    DMap4d                  m_worldToNpc;
    DMap4d                  m_worldToView;
    ScanCriteria            m_scanCriteria;
    TransformClipStack      m_transformClipStack;
    DgnViewportP            m_viewport = nullptr;
    IElemTopologyCPtr       m_currElemTopo;
    GeometryStreamEntryId   m_currGeometryStreamEntryId;

    void InvalidateScanRange() {m_scanRangeValid = false;}
    DGNPLATFORM_EXPORT virtual StatusInt _Attach(DgnViewportP, DrawPurpose purpose);
    DGNPLATFORM_EXPORT virtual void _Detach();
    DGNPLATFORM_EXPORT virtual void _OutputGeometry(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual void _AddSubGraphic(Render::GraphicR, DgnGeomPartId, TransformCR, Render::GeometryParamsR);
    virtual Render::GraphicP _GetCachedPartGraphic(DgnGeomPartId, double pixelSize, ElementAlignedBox3dR) {return nullptr;}
    virtual void _SavePartGraphic(DgnGeomPartId, Render::GraphicR, ElementAlignedBox3dCR) {}
    virtual void _OutputGraphic(Render::GraphicR, GeometrySourceCP) {}
    virtual Render::GraphicP _GetCachedGraphic(GeometrySourceCR, double pixelSize) {return nullptr;}
    virtual void _SaveGraphic(GeometrySourceCR, Render::GraphicR graphic) {}
    virtual bool _AbortProgressiveDisplay() {return false;}
    DGNPLATFORM_EXPORT virtual bool _WantAreaPatterns();
    DGNPLATFORM_EXPORT virtual void _DrawAreaPattern(ClipStencil& boundary);
    DGNPLATFORM_EXPORT virtual void _DrawStyledLineString2d(int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false);
    DGNPLATFORM_EXPORT virtual void _DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed = false);
    DGNPLATFORM_EXPORT virtual void _DrawStyledArc2d(DEllipse3dCR, bool isEllipse, double zDepth, DPoint2dCP range);
    DGNPLATFORM_EXPORT virtual void _DrawStyledArc3d(DEllipse3dCR, bool isEllipse, DPoint3dCP range);
    DGNPLATFORM_EXPORT virtual void _DrawStyledBSplineCurve3d(MSBsplineCurveCR);
    DGNPLATFORM_EXPORT virtual void _DrawStyledBSplineCurve2d(MSBsplineCurveCR, double zDepth);
    DGNPLATFORM_EXPORT virtual void _AddTextString(TextStringCR);
    DGNPLATFORM_EXPORT virtual StatusInt _InitContextForView();
    DGNPLATFORM_EXPORT virtual StatusInt _VisitElement(GeometrySourceCR);
    DGNPLATFORM_EXPORT virtual void _InitScanRangeAndPolyhedron();
    DGNPLATFORM_EXPORT virtual bool _VisitAllModelElements();
    DGNPLATFORM_EXPORT virtual StatusInt _VisitDgnModel(DgnModelP);
    DGNPLATFORM_EXPORT virtual void _PushClip(ClipVectorCR clip);
    DGNPLATFORM_EXPORT virtual void _PopClip();    
    DGNPLATFORM_EXPORT virtual bool _FilterRangeIntersection(GeometrySourceCR);
    virtual IPickGeomP _GetIPickGeom() {return nullptr;}
    virtual Render::GraphicPtr _CreateGraphic(Render::Graphic::CreateParams const& params) = 0;
    DGNPLATFORM_EXPORT virtual void _SetupScanCriteria();
    virtual bool _WantUndisplayed() {return false;}
    DGNPLATFORM_EXPORT virtual void _AddViewOverrides(Render::OvrGraphicParamsR);
    DGNPLATFORM_EXPORT virtual void _AddContextOverrides(Render::OvrGraphicParamsR, GeometrySourceCP source);
    DGNPLATFORM_EXPORT virtual void _CookGeometryParams(Render::GeometryParamsR, Render::GraphicParamsR);
    DGNPLATFORM_EXPORT virtual void _SetScanReturn();
    DGNPLATFORM_EXPORT virtual void _PushFrustumClip();
    DGNPLATFORM_EXPORT virtual StatusInt _ScanDgnModel(DgnModelP model);
    DGNPLATFORM_EXPORT virtual bool _ScanRangeFromPolyhedron();
    DGNPLATFORM_EXPORT virtual void _SetDgnDb(DgnDbR);
    DGNPLATFORM_EXPORT virtual ScanCriteria::Result _CheckNodeRange(ScanCriteriaCR, DRange3dCR, bool is3d);
    DGNPLATFORM_EXPORT ViewContext();

public:
    int GetTransClipDepth() {return (int) m_transformClipStack.GetSize();}
    DMap4dCR GetWorldToView() const {return m_worldToView;}
    DMap4dCR GetWorldToNpc() const {return m_worldToNpc;}
    bool GetWantMaterials() {return m_wantMaterials;};
    bool IsAttached() {return m_isAttached;}
    void SetSubRectFromViewRect(BSIRectCP viewRect);
    DGNPLATFORM_EXPORT void SetSubRectNpc(DRange3dCR subRect);
    void SetWantMaterials(bool wantMaterials) {m_wantMaterials = wantMaterials;}
    bool IsUndisplayed(GeometrySourceCR source);
    bool ValidateScanRange() {return m_scanRangeValid ? true : _ScanRangeFromPolyhedron();}
    StatusInt Attach(DgnViewportP vp, DrawPurpose purpose) {return _Attach(vp,purpose);}
    void Detach() {_Detach();}
    bool VisitAllModelElements() {return _VisitAllModelElements();}
    DGNPLATFORM_EXPORT bool VisitAllViewElements(BSIRectCP updateRect=nullptr);
    StatusInt InitContextForView() {return _InitContextForView();}
    DGNPLATFORM_EXPORT bool IsWorldPointVisible(DPoint3dCR worldPoint, bool boresite);
    DGNPLATFORM_EXPORT bool PointInsideClip(DPoint3dCR point);
    DGNPLATFORM_EXPORT bool GetRayClipIntersection(double& distance, DPoint3dCR origin, DVec3dCR direction);
    DGNPLATFORM_EXPORT Frustum GetFrustum();
    TransformClipStackR GetTransformClipStack() {return m_transformClipStack;}
    ScanCriteriaCP GetScanCriteria() const {return &m_scanCriteria;}
    void InitScanRangeAndPolyhedron() {_InitScanRangeAndPolyhedron();}
    StatusInt VisitDgnModel(DgnModelP model){return _VisitDgnModel(model);}
    void SetScanReturn() {_SetScanReturn();}
    void EnableStopAfterTimout(uint32_t timeout) {m_endTime = BeTimeUtilities::QueryMillisecondsCounter()+timeout; m_stopAfterTimeout=true;}

    Render::GraphicPtr CreateGraphic(Render::Graphic::CreateParams const& params=Render::Graphic::CreateParams()) {return _CreateGraphic(params);}
    void AddSubGraphic(Render::GraphicR graphic, DgnGeomPartId partId, TransformCR subToGraphic, Render::GeometryParamsR geomParams) {_AddSubGraphic(graphic, partId, subToGraphic, geomParams);}
    StatusInt VisitElement(GeometrySourceCR elem) {return _VisitElement(elem);}

    /// @name Coordinate Query and Conversion
    //@{

    //! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::View.
    //! @param[out]     viewPts     An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      npcPts      Input array in DgnCoordSystem::Npc.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToView(DPoint3dP viewPts, DPoint3dCP npcPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::Npc.
    //! @param[out]     npcPts      An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToNpc(DPoint3dP npcPts, DPoint3dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::World.
    //! @param[out]     worldPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      npcPts      Input array in DgnCoordSystem::Npc.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out]     viewPts     An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      worldPts    Input array in DgnCoordSystem::World.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out]     viewPts     An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      worldPts     Input array in DgnCoordSystem::World.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out]     viewPts     An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      worldPts    Input array in DgnCoordSystem::World.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(Point2dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out]     worldPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out]     worldPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const;

    //! Calculate the size of a "pixel" at a given point in the current local coordinate system. This method can be used to
    //! approximate how large geometry in local coordinates will appear in DgnCoordSystem::View units.
    //! @param[in]      origin      The point at which the pixel size is calculated. This point is only relevant in camera views, where local coordinates
    //!                             closer to the eye are larger than those further from the eye. May be nullptr, in which case the center of the view is used.
    //! @return the length, in the current coordinate system units, of a unit bvector in the x direction in DgnCoordSystem::View, starting at \c origin.
    DGNPLATFORM_EXPORT double GetPixelSizeAtPoint(DPoint3dCP origin) const;

    //@}

    /// @name Pushing and Popping Transforms and Clips
    //@{
    //! Push a ClipVector, creating a new local clip region.
    //! @param[in]      clip       A clipping descriptor to push.
    //! @see   PopTransformClip
    void PushClip(ClipVectorCR clip) {_PushClip(clip);}

    //! Push a set of clip planes, creating a new local clip region.
    //! @param[in]      clipPlanes  Clipping planes to push - the intersections of their half planes define clip region.
    //! @see   PopTransformClip
    DGNPLATFORM_EXPORT void PushClipPlanes(ClipPlaneSetCR clipPlanes);

    void PopClip() {_PopClip();}
    //@}

    /// @name Query Methods
    //@{

    //! Get the current state of the ViewFlags for this context
    //! When a ViewContext is first attached to a DgnViewport, the ViewFlags are initialized
    //! from the DgnViewport's viewflags. However, during the course of an operation,
    //! the viewflags may be different than those on the DgnViewport.
    Render::ViewFlags GetViewFlags() const {return m_viewflags;}

    //! Sets the current state of the ViewFlags for this context
    void SetViewFlags(Render::ViewFlags flags) {m_viewflags = flags;}

    //! Get the DgnDb for this ViewContext.
    DgnDbR GetDgnDb() const {BeAssert(nullptr != m_dgnDb); return *m_dgnDb;}

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
    //@}

    /// @name Get/Set Current Display Parameters
    //@{
    //! Modify the supplied "natural" GeometryParams by resolving effective symbology as required by the context.
    //! Initializes the supplied GraphicParams from the resolved GeometryParams.
    void CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicParamsR graphicParams) {_CookGeometryParams(geomParams, graphicParams);}

    //! Modify the supplied "natural" GeometryParams by resolving effective symbology as required by the context.
    //! Initializes a GraphicParams from the resolved GeometryParams and calls ActivateGraphicParams on the supplied Render::GraphicR.
    DGNPLATFORM_EXPORT void CookGeometryParams(Render::GeometryParamsR geomParams, Render::GraphicR graphic);

    //! Get the IPickGeom interface for this ViewContext. Only contexts that are specific to picking will return a non-nullptr value.
    //! @return the IPickGeom interface for this context. May return nullptr.
    IPickGeomP GetIPickGeom() {return _GetIPickGeom();}

    //@}

    /// @name Identifying element "topology".
    //@{
    //! Query the current IElementTopology.
    //! @return An object that holds additional information about the graphics that are currently being drawn.
    IElemTopologyCP GetElemTopology() const {return (m_currElemTopo.IsValid() ? m_currElemTopo.get() : nullptr);}

    //! Set the current IElementTopology.
    //! @param topo An object holding additional information about the graphics to be drawn or nullptr to clear the current topology pointer.
    void SetElemTopology(IElemTopologyCP topo) {m_currElemTopo = topo;}

    //! Query the current GeometryStreamEntryId.
    GeometryStreamEntryId GetGeometryStreamEntryId() const {return m_currGeometryStreamEntryId;}

    //! Get a reference to the current GeometryStreamEntryId to modify.
    GeometryStreamEntryId& GetGeometryStreamEntryIdR() {return m_currGeometryStreamEntryId;}

    //@}

    DGNPLATFORM_EXPORT bool WantAreaPatterns();
    DGNPLATFORM_EXPORT void DrawAreaPattern(ClipStencil& boundary);

    /** @name Draw Geometry Using Current Linestyle */
    /** @{ */

    //! Draw a 2D linestring using the current Linestyle, if any. If there is no current Linestyle, draw a solid linestring.
    //! @param[in]      nPts        Number of vertices in \c pts.
    //! @param[in]      pts         Array of points in linestring.
    //! @param[in]      zDepth      Display priority for all vertices.
    //! @param[in]      range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                 optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    //! @param[in]      closed      Do point represent a shape or linestring.
    void DrawStyledLineString2d(int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed=false){_DrawStyledLineString2d(nPts, pts, zDepth, range, closed);}

    //! Draw a 3D linestring using the current Linestyle, if any. If there is no current Linestyle, draw a solid linestring.
    //! @param[in]      nPts        Number of vertices in \c pts.
    //! @param[in]      pts         Array of points in linestring
    //! @param[in]      range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
    //!                                 optional and is only used to speed processing. If you do not already have the range of your points, pass nullptr.
    //! @param[in]      closed      Do point represent a shape or linestring.
    void DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed=false){_DrawStyledLineString3d(nPts, pts, range, closed);}

    //! Draw a 2D elliptical arc using the current Linestyle. If there is no current Linestyle, draw a solid arc.
    //! @param[in]      ellipse     The arc data.
    //! @param[in]      isEllipse   Treat full sweep as ellipse not arc.
    //! @param[in]      zDepth      Z depth value.
    //! @param[in]      range       Array of 2 points with the range (min followed by max) of the arc. This argument is
    //!                               optional and is only used to speed processing. If you do not already have the range, pass nullptr.
    void DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range) {_DrawStyledArc2d(ellipse, isEllipse, zDepth, range);}

    //! Draw a 3D elliptical arc using the current Linestyle. If there is no current Linestyle, draw a solid arc.
    //! @param[in]      ellipse     The arc data.
    //! @param[in]      isEllipse   Treat full sweep as ellipse not arc.
    //! @param[in]      range       Array of 2 points with the range (min followed by max) of the arc. This argument is
    //!                               optional and is only used to speed processing. If you do not already have the range, pass nullptr.
    void DrawStyledArc3d(DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range) {_DrawStyledArc3d(ellipse, isEllipse, range);}

    //! Draw a 2d BSpline curve using the current Linestyle. If there is no current Linestyle, draw a solid BSpline.
    //! @param        curve       bspline curve parameters
    //! @param[in]    zDepth      Z depth value.
    void DrawStyledBSplineCurve2d(MSBsplineCurveCR curve, double zDepth) {_DrawStyledBSplineCurve2d(curve, zDepth);}

    //! Draw a BSpline curve using the current Linestyle. If there is no current Linestyle, draw a solid BSpline.
    //! @param        curve       bspline curve parameters
    void DrawStyledBSplineCurve3d(MSBsplineCurveCR curve) {_DrawStyledBSplineCurve3d(curve);}

    //! Draw a curve vector using the current Linestyle. If there is no current Linestyle, draw a solid curve vector.
    //! @param        curve       curve geometry
    DGNPLATFORM_EXPORT void DrawStyledCurveVector3d(CurveVectorCR curve);

    //! Draw a 2d curve vector using the current Linestyle. If there is no current Linestyle, draw a solid curve vector.
    //! @param        curve       curve geometry
    //! @param[in]    zDepth      Z depth value.
    DGNPLATFORM_EXPORT void DrawStyledCurveVector2d(CurveVectorCR curve, double zDepth);

    //! Draw a text string and any adornments such as background shape, underline, overline, etc. Sets up current GeometryParams for TextString symbology.
    void AddTextString(TextStringCR textString) {_AddTextString(textString);}

    bool CheckStop() {return _CheckStop();}
}; // ViewContext

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct RenderContext : ViewContext
{
    DEFINE_T_SUPER(ViewContext);

protected:
    Render::OvrGraphicParams m_ovrParams;
    Render::TargetR m_target;

public:
    Render::OvrGraphicParams& GetOvrGraphicParams() {return m_ovrParams;}

    DGNVIEW_EXPORT RenderContext(DgnViewportR vp, DrawPurpose);
    void _AddContextOverrides(Render::OvrGraphicParamsR ovrMatSymb, GeometrySourceCP source) override;
    Render::GraphicP _GetCachedGraphic(GeometrySourceCR source, double pixelSize) override {return source.Graphics().Find(*m_viewport, pixelSize);}
    DGNVIEW_EXPORT Render::GraphicP _GetCachedPartGraphic(DgnGeomPartId, double pixelSize, ElementAlignedBox3dR) override;
    void _PushFrustumClip() override {}
    Render::GraphicPtr _CreateGraphic(Render::Graphic::CreateParams const& params) override {return m_target.CreateGraphic(params);}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct DynamicsContext : RenderContext
{
    friend struct DgnPrimitiveTool;
private:
    Render::GraphicListR m_dynamics;
    void _OutputGraphic(Render::GraphicR graphic, GeometrySourceCP) override;
    DynamicsContext(DgnViewportR);
    ~DynamicsContext();
    void VisitWriteableElement(DgnElementCR element, IRedrawOperationP redrawOp);

public:
    DGNVIEW_EXPORT void DrawElements(DgnElementCPtrVec const& elements, IRedrawOperationP redrawOp=nullptr);
    DGNVIEW_EXPORT void DrawElements(DgnElementIdSet const& elemIds, IRedrawOperationP redrawOp=nullptr);
    DGNVIEW_EXPORT void DrawElement(DgnElementCR element, IRedrawOperationP redrawOp=nullptr);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/15
//=======================================================================================
struct DecorateContext : RenderContext
{
    DEFINE_T_SUPER(RenderContext);
    friend struct DgnViewport;
private:
    bool    m_isFlash = false;
    Render::Decorations& m_decorations;
    void _AddContextOverrides(Render::OvrGraphicParamsR ovrMatSymb, GeometrySourceCP source) override;
    void _OutputGraphic(Render::GraphicR graphic, GeometrySourceCP) override;
    DecorateContext(DgnViewportR vp, Render::Decorations& decorations) : RenderContext(vp, DrawPurpose::Decorate), m_decorations(decorations) {}

public:
    StatusInt VisitHit(HitDetailCR hit);

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
};  

/** @endGroup */

END_BENTLEY_DGN_NAMESPACE
