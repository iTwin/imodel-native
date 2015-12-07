/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ViewContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "ClipVector.h"
#include "TransformClipStack.h"
#include "Render.h"
#include "ScanCriteria.h"
#include "IPickGeom.h"

#define FOCAL_LENGTH_RATIO 0.023584905


BEGIN_BENTLEY_DGN_NAMESPACE

/*=================================================================================**//**
 @addtogroup ViewContext
 A ViewContext holds the <i>current state</i> of an operation on a DgnViewport. A ViewContext must be first
 \e attached to a DgnViewport to be useful, and must be \e detached from the DgnViewport to free any memory associated with its internal state.
 @beginGroup 
+===============+===============+===============+===============+===============+======*/

enum FilterLODFlags
{
    FILTER_LOD_Off          = 0,        //!< don't do Level-of-detail filtering at all
    FILTER_LOD_ShowRange    = 1,        //!< when too small, just show range
    FILTER_LOD_ShowNothing  = 2,        //!< when too small, show nothing
};

enum
{
    DEFAULT_MINUMUM_LOD     = 50,       // extent squared
    DEFAULT_MINUMUM_CUT_LOD = 20,       // extent squared
    LOD_DISPLAY_AS_POINT    = 6,        // extent squared
    LOD_DELTA_INCREASE      = 9,        // extent squared
    LOD_DELTA_DECREASE      = 100,      // extent squared
    MAX_LOD_DELTA           = 40000,    // extent squared
};

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
struct IRangeNodeCheck
{
    virtual ScanCriteria::Result _CheckNodeRange(ScanCriteriaCR, DRange3dCR, bool is3d) = 0;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct ICheckStop
{  
private:
    bool m_aborted;

public:
    bool InitAborted(bool val) {return m_aborted = val;}
    bool ClearAborted() {return m_aborted = false;}
    bool WasAborted()  {return m_aborted;}
    bool SetAborted() {return m_aborted = true;}
    bool AddAbortTest(bool val) {return  m_aborted |= val;}

    ICheckStop() {m_aborted=false;}

    //! return true to abort the current operation.
    //! @note Overrides MUST call SetAborted or use AddAbortTest since WasAborted may be directly tested!
    virtual bool _CheckStop() {return m_aborted;}
};

struct IElemTopology;
struct IEditManipulator;
DEFINE_REF_COUNTED_PTR(IElemTopology)
DEFINE_REF_COUNTED_PTR(IEditManipulator)

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

//=======================================================================================
// @bsiclass                                                     KeithBentley    04/01
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewContext : NonCopyableClass, ICheckStop, IRangeNodeCheck
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
    DgnDbP                  m_dgnDb;
    bool                    m_isAttached;
    bool                    m_is3dView;
    bool                    m_wantMaterials;
    bool                    m_useNpcSubRange;
    bool                    m_ignoreViewRange;
    bool                    m_scanRangeValid;
    Byte                    m_filterLOD;
    ViewFlags               m_viewflags;
    DrawPurpose             m_purpose;
    DRange3d                m_npcSubRange;
    DMap4d                  m_worldToNpc;
    DMap4d                  m_worldToView;
    ScanCriteriaP           m_scanCriteria;
    int32_t                 m_displayPriorityRange[2];
    TransformClipStack      m_transformClipStack;
    DgnViewportP            m_viewport;
    GeometrySourceCP        m_currentGeomSource;
    Render::GeometryParams  m_currGeometryParams;
    Render::GraphicParams   m_graphicParams;
    Render::OvrGraphicParams      m_ovrGraphicParams;
    DPoint3dCP              m_startTangent;       // linestyle start tangent.
    DPoint3dCP              m_endTangent;         // linestyle end tangent.
    DgnElement::Hilited     m_hiliteState;
    IElemTopologyCPtr       m_currElemTopo;
    GeometryStreamEntryId       m_currGeometryStreamEntryId;
    double                  m_levelOfDetail;
    double                  m_minLOD;             // minimum size of default level-of-detail test.
    double                  m_arcTolerance;

    void InvalidateScanRange() {m_scanRangeValid = false;}
    DGNPLATFORM_EXPORT void InitDisplayPriorityRange();
    DGNPLATFORM_EXPORT virtual StatusInt _Attach(DgnViewportP, DrawPurpose purpose);
    DGNPLATFORM_EXPORT virtual void _Detach();
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _OutputElement(GeometrySourceCR);
    virtual Render::GraphicP _GetCachedGraphic(double pixelSize) {return nullptr;}
    virtual void _SaveGraphic(Render::GraphicR graphic) {}
    DGNPLATFORM_EXPORT virtual bool _WantAreaPatterns();
    DGNPLATFORM_EXPORT virtual void _DrawAreaPattern(ClipStencil& boundary);
    DGNPLATFORM_EXPORT virtual ILineStyleCP _GetCurrLineStyle(Render::LineStyleSymbP*);
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
    DGNPLATFORM_EXPORT virtual bool _VisitAllModelElements(bool includeTransients);
    DGNPLATFORM_EXPORT virtual StatusInt _VisitDgnModel(DgnModelP);
//    DGNPLATFORM_EXPORT virtual void _PushTransform(TransformCR trans);
    DGNPLATFORM_EXPORT virtual void _PushClip(ClipVectorCR clip);
//    DGNPLATFORM_EXPORT virtual void _PushViewIndependentOrigin(DPoint3dCP origin);
//    DGNPLATFORM_EXPORT virtual void _PopTransformClip();
    DGNPLATFORM_EXPORT virtual void _PopClip();    
    DGNPLATFORM_EXPORT virtual bool _FilterRangeIntersection(GeometrySourceCR);
    virtual IPickGeomP _GetIPickGeom() {return nullptr;}
    virtual void _OnPreDrawTransient() {}
    virtual Render::GraphicPtr _BeginGraphic(Render::Graphic::CreateParams const& params) = 0;
    DGNPLATFORM_EXPORT virtual void _VisitTransientGraphics(bool isPreUpdate);
    DGNPLATFORM_EXPORT virtual void _AllocateScanCriteria();
    DGNPLATFORM_EXPORT virtual void _SetupScanCriteria();
    virtual bool _WantUndisplayed() {return false;}
    DGNPLATFORM_EXPORT virtual void _AddViewOverrides(Render::OvrGraphicParamsR);
    DGNPLATFORM_EXPORT virtual void _AddContextOverrides(Render::OvrGraphicParamsR);
    DGNPLATFORM_EXPORT virtual void _ModifyPreCook(Render::GeometryParamsR); 
    DGNPLATFORM_EXPORT virtual void _CookGeometryParams(Render::GeometryParamsR, Render::GraphicParamsR);
    DGNPLATFORM_EXPORT virtual void _SetScanReturn();
    DGNPLATFORM_EXPORT virtual void _PushFrustumClip();
    DGNPLATFORM_EXPORT virtual StatusInt _ScanDgnModel(DgnModelP model);
    DGNPLATFORM_EXPORT virtual bool _ScanRangeFromPolyhedron();
    DGNPLATFORM_EXPORT virtual void _SetDgnDb(DgnDbR);
    DGNPLATFORM_EXPORT virtual ScanCriteria::Result _CheckNodeRange(ScanCriteriaCR, DRange3dCR, bool is3d);
    DGNPLATFORM_EXPORT ViewContext();
    DGNPLATFORM_EXPORT virtual ~ViewContext();

public:
    int ViewContext::GetTransClipDepth() {return (int) m_transformClipStack.GetSize();}
    DMap4dCR GetWorldToView() const {return m_worldToView;}
    DMap4dCR GetWorldToNpc() const {return m_worldToNpc;}
    bool GetWantMaterials() {return m_wantMaterials;};
    bool IsAttached() {return m_isAttached;}
    DgnElement::Hilited GetCurrHiliteState() {return m_hiliteState;}
    void SetSubRectFromViewRect(BSIRectCP viewRect);
    void OnPreDrawTransient() {_OnPreDrawTransient();} // Initialize per-transient state since _OutputElement may not be called...
    DGNPLATFORM_EXPORT void SetSubRectNpc(DRange3dCR subRect);
    void SetWantMaterials(bool wantMaterials) {m_wantMaterials = wantMaterials;}
//    DGNPLATFORM_EXPORT DMatrix4d GetLocalToView() const;
//    DGNPLATFORM_EXPORT DMatrix4d GetViewToLocal() const;
    bool IsUndisplayed(GeometrySourceCR source);
    bool ValidateScanRange() {return m_scanRangeValid ? true : _ScanRangeFromPolyhedron();}
    StatusInt Attach(DgnViewportP vp, DrawPurpose purpose) {return _Attach(vp,purpose);}
    void Detach() {_Detach();}
    bool VisitAllModelElements(bool includeTransients) {return _VisitAllModelElements(includeTransients);}
    DGNPLATFORM_EXPORT bool VisitAllViewElements(bool includeTransients, BSIRectCP updateRect);
    DGNPLATFORM_EXPORT StatusInt VisitHit(HitDetailCR hit);
    void VisitTransientGraphics(bool isPreUpdate) {_VisitTransientGraphics(isPreUpdate);}
    DGNPLATFORM_EXPORT void DrawBox(DPoint3dP box, bool is3d);
    StatusInt InitContextForView() {return _InitContextForView();}
    DGNPLATFORM_EXPORT bool IsWorldPointVisible(DPoint3dCR worldPoint, bool boresite);
//    DGNPLATFORM_EXPORT bool IsLocalPointVisible(DPoint3dCR localPoint, bool boresite);
    DGNPLATFORM_EXPORT bool PointInsideClip(DPoint3dCR point);
    DGNPLATFORM_EXPORT bool GetRayClipIntersection(double& distance, DPoint3dCR origin, DVec3dCR direction);
    DGNPLATFORM_EXPORT Frustum GetFrustum();
    TransformClipStackR GetTransformClipStack() {return m_transformClipStack;}
    double GetArcTolerance() const {return m_arcTolerance;}
    void SetArcTolerance(double tol) {m_arcTolerance = tol;}
    DGNPLATFORM_EXPORT void SetLinestyleTangents(DPoint3dCP start, DPoint3dCP end);
    double GetMinLOD() const {return m_minLOD;}
    void SetMinLOD(double lod) {m_minLOD = lod;}
    Byte& GetFilterLODFlag() {return m_filterLOD;}
    void SetFilterLODFlag(FilterLODFlags flags) {m_filterLOD =(Byte) flags;}
    ScanCriteriaCP GetScanCriteria() const {return m_scanCriteria;}
    void InitScanRangeAndPolyhedron() {_InitScanRangeAndPolyhedron();}
    void AllocateScanCriteria(){_AllocateScanCriteria();}
    void VisitDgnModel(DgnModelP model){_VisitDgnModel(model);}
    void SetScanReturn() {_SetScanReturn();}

public:
    Render::GraphicPtr BeginGraphic(Render::Graphic::CreateParams const& params=Render::Graphic::CreateParams()) {return _BeginGraphic(params);}
    StatusInt VisitElement(GeometrySourceCR elem) {return _VisitElement(elem);}

    /// @name Coordinate Query and Conversion
    //@{

#if defined (NOT_NOW)
    //! Transform an array of points in the current local coordinate system into DgnCoordSystem::World coordinates.
    //! @param[out]     worldPts    An array to receive the points in DgnCoordSystem::World. Must be dimensioned to hold \c nPts points.
    //! @param[in]      localPts    Input array in current local coordinates,
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void LocalToWorld(DPoint3dP worldPts, DPoint3dCP localPts, int nPts) const;

    //! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
    //! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in]      localPts    Input array in current local coordinates,
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void LocalToView(DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const;

    //! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
    //! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in]      localPts    Input array in current local coordinates,
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void LocalToView(DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into the current local coordinate system.
    //! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      worldPts    Input array in DgnCoordSystem::World.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToLocal(DPoint3dP localPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
    //! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToLocal(DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
    //! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
    //! @param[in]      viewPts     Input array in DgnCoordSystem::View.
    //! @param[in]      nPts        Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToLocal(DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const;
#endif

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

#if defined (NOT_NOW)
    //! Retrieve a pointer to the the transform from the current local coordinate system into DgnCoordSystem::World.
    //! @return   nullptr if no transform present.
    TransformCP GetCurrLocalToWorldTransformCP() const {return m_transformClipStack.GetTransformCP();}

    //! Retrieve a copy of the transform from the current local coordinate system into DgnCoordSystem::World.
    //! @param[out]     trans       Transform from current local coordinate system to DgnCoordSystem::World
    //! @return   SUCCESS if there is a current local coordinate system.
    BentleyStatus GetCurrLocalToWorldTrans(TransformR trans) const {return m_transformClipStack.GetTransform(trans);}

    //! Retrieve a copy of the transform from the DgnCoordSystem::World to current local coordinate system.
    //! @param[out]     trans       Transform from DgnCoordSystem::World to current local coordinate system
    //! @return   SUCCESS if there is a current local coordinate system.
    BentleyStatus GetCurrWorldToLocalTrans(TransformR trans) const {return m_transformClipStack.GetInverseTransform(trans);}

    //! Retrieve a copy of the transform from the local coordinate system at the specified index into DgnCoordSystem::World.
    //! @param[out]     trans  Transform from local coordinate system at the specified index to DgnCoordSystem::World
    //! @param[in]      index  Index into transform stack to return transform for.
    //! @return   SUCCESS if there is a local coordinate system.
    BentleyStatus GetLocalToWorldTrans(TransformR trans, size_t index) const {return m_transformClipStack.GetTransformFromIndex(trans, index);}
#endif

    //! Calculate the size of a "pixel" at a given point in the current local coordinate system. This method can be used to
    //! approximate how large geometry in local coordinates will appear in DgnCoordSystem::View units.
    //! @param[in]      origin      The point at which the pixel size is calculated. This point is only relevant in camera views, where local coordinates
    //!                             closer to the eye are larger than those further from the eye. May be nullptr, in which case the center of the view is used.
    //! @return the length, in the current coordinate system units, of a unit bvector in the x direction in DgnCoordSystem::View, starting at \c origin.
    DGNPLATFORM_EXPORT double GetPixelSizeAtPoint(DPoint3dCP origin) const;

    //! Get transform aligned with current view rotation.
//    DGNPLATFORM_EXPORT void GetViewIndependentTransform(TransformP trans, DPoint3dCP originLocal);

    //! Check whether the current transform is view independent. Several MicroStation element types can display
    //! as "View independent" (e.g. text, text nodes, point cells). They do this by pushing the inverse of the current
    //! view-to-local transformation via #PushViewIndependentOrigin.
    //! @return   true if the current local coordinate system is a view independent transform.
//    bool IsViewIndependent() {return m_transformClipStack.IsViewIndependent();}
    //@}

    /// @name Pushing and Popping Transforms and Clips
    //@{
    //! Push a Transform, creating a new local coordinate system.
    //! @param[in]      trans       The transform to push.
    //! @see   PopTransformClip
//    void PushTransform(TransformCR trans) {_PushTransform(trans);}

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

#if defined (NOT_NOW)
    //! Push a transform such that the X,Y plane of the new local coordinate system will be aligned with the X,Y plane of the
    //! view coordinate system, oriented about the given origin.
    //! @param[in]      origin      Origin for rotation, in the \e current local coordinate system.
    void PushViewIndependentOrigin(DPoint3dCP origin) {_PushViewIndependentOrigin(origin);}

    //! Remove the most recently pushed coordinate system and clip, restoring the local coordinate system to its previous state.
    void PopTransformClip() {_PopTransformClip();}
#else
    void PopClip() {_PopClip();}
#endif
    //@}

    /// @name Query Methods
    //@{

    //! Get the current state of the ViewFlags for this context
    //! When a ViewContext is first attached to a DgnViewport, the ViewFlags are initialized
    //! from the DgnViewport's viewflags. However, during the course of an operation,
    //! the viewflags may be different than those on the DgnViewport.
    ViewFlags GetViewFlags() const {return m_viewflags;}

    //! Sets the current state of the ViewFlags for this context
    void SetViewFlags(ViewFlags flags) {m_viewflags = flags;}

    //! Get the DgnDb for this ViewContext.
    DgnDbR GetDgnDb() const {BeAssert(nullptr != m_dgnDb); return *m_dgnDb;}

    //! Set the project for this ViewContext when not attaching a viewport.
    void SetDgnDb(DgnDbR dgnDb) {return _SetDgnDb(dgnDb);}

    void SetCurrentGeomSource(GeometrySourceCP source) {m_currentGeomSource = source;}
    
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
    bool GetDisplayPriorityRange(int32_t& low, int32_t& high) const {if (NULL == m_viewport) return false; low = m_displayPriorityRange[0]; high = m_displayPriorityRange[1]; return true;}

    //! Change the supplied "natural" GeometryParams. Resolves effective symbology as required by the context and initializes the supplied GraphicParams.
    //! @note Does NOT call ActivateMatSymb on the output or change the current GeometryParams/GraphicParams of the context.
    void CookGeometryParams(Render::GeometryParamsR elParams, Render::GraphicParamsR elMatSymb) {_CookGeometryParams(elParams, elMatSymb);}
    DGNPLATFORM_EXPORT void CookGeometryParams();

    //! Calculate the net display priority value. The net display priority is based on the geometry (element) and sub-category priority.
    //! @return the net display priority. For 3D views, display priority is always 0.
    DGNPLATFORM_EXPORT int32_t ResolveNetDisplayPriority(int32_t geomPriority, DgnSubCategoryId subCategoryId, DgnSubCategory::Appearance* appearance = nullptr) const;

    //! Get the current GraphicParams.
    //! @return   the current GraphicParams.
    Render::GraphicParamsP GetGraphicParams() {return &m_graphicParams;}

    //! Get the current OvrGraphicParams.
    //! @return the current OvrGraphicParams.
    Render::OvrGraphicParamsP GetOverrideGraphicParams() {return &m_ovrGraphicParams;}

    //! Get the current GeometryParams.
    //! @return the current GeometryParams.
    Render::GeometryParams& GetCurrentGeometryParams() {return m_currGeometryParams;}

    //! Clears current override flags and re-applies context overrides.
    //! @note Calls ActivateOverrideGraphicParams on the output.
    DGNPLATFORM_EXPORT void ResetContextOverrides();

    //! Gets the current level of detail.
    //! @return       the current level of detail.
    double GetCurrentLevelOfDetail() const {return m_levelOfDetail;}

    //! Sets the current level of detail.
    void SetCurrentLevelOfDetail(double levelOfDetail) {m_levelOfDetail = levelOfDetail;}

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

    //! Set the current GeometryStreamEntryId.
    void SetGeometryStreamEntryId(GeometryStreamEntryId geomId) {m_currGeometryStreamEntryId = geomId;}

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

/** @endGroup */

END_BENTLEY_DGN_NAMESPACE
