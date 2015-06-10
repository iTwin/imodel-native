/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ViewContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "ClipVector.h"
#include "TransformClipStack.h"
#include <stack>
#include <Bentley/bvector.h>
#include "IViewDraw.h"
#include "ScanCriteria.h"
#include "Material.h"

#define FOCAL_LENGTH_RATIO 0.023584905

typedef uintptr_t QvExtSymbID;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
 @addtogroup ViewContext
 A ViewContext holds the <i>current state</i> of an operation on a DgnViewport. A ViewContext must be first
 \e attached to a DgnViewport to be useful, and must be \e detached from the DgnViewport to free any memory associated with its internal state.


/** @beginGroup */

//=======================================================================================
//! @bsiclass
//=======================================================================================
enum GeomRepresentations
{
    DISPLAY_INFO_None       = 0,        //!< Don't draw anything (not renderable, i.e. has no surface, and option to render wires is off)
    DISPLAY_INFO_Edge       = (1<<0),   //!< Output wireframe geometry, i.e. outline curves and surface/solid edges/isoparametrics suitable for snapping.
    DISPLAY_INFO_Fill       = (1<<1),   //!< Output closed curves and regions as filled.
    DISPLAY_INFO_Surface    = (1<<2),   //!< Output surface/solid representation. 
    DISPLAY_INFO_Thickness  = (1<<3),   //!< Output open/closed curves as extruded surfaces/solids.
    DISPLAY_INFO_Pattern    = (1<<4),   //!< Output closed curves and regions as hatched/patterned.
};

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
        }           m_flags;

    int32_t    m_styleIndex;       //!< Display style of the clip volume area. -1 to match that view.

    int32_t GetDisplayStyleIndex () const { return m_styleIndex; }
    void SetDisplayStyleIndex (int32_t index) { m_styleIndex = index; }
    bool IsEqual (const ClipVolumeOverrides& other) const
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

/*=================================================================================**//**
* Stores ClipVolume flags which are used by DynamicViewSettings
* @remark This is stored persistently as part of the Reference Dynamic View Settings XAttributes, and thus cannot be changed
* @bsiclass                                                     
+===============+===============+===============+===============+===============+======*/
struct ClipVolumeFlags
{
    unsigned   m_reflected:1;
    unsigned   m_ignoreBoundaryClipping:1;  // if true, side crop handles are ignored and the cut plane is considered infinite 
    unsigned   m_ignoreBoundaryCutPlanes:1; // if true, section graphics are not generated along side planes
    unsigned   m_unused:29;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct     ILineStyleComponent
{
    virtual bool          _IsContinuous () const = 0;
    virtual bool          _HasWidth () const = 0;
    virtual double        _GetLength () const = 0;
    virtual StatusInt     _StrokeLineString (ViewContextP, LineStyleSymbP, DPoint3dCP, int nPts, bool isClosed) const = 0;
    virtual StatusInt     _StrokeLineString2d (ViewContextP, LineStyleSymbP, DPoint2dCP, int nPts, double zDepth, bool isClosed) const = 0;
    virtual StatusInt     _StrokeArc (ViewContextP, LineStyleSymbP, DPoint3dCP origin, RotMatrixCP rMatrix,
                                    double r0, double r1, double const* start, double const* sweep, DPoint3dCP range) const = 0;
    virtual StatusInt     _StrokeBSplineCurve (ViewContextP context, LineStyleSymbP lsSymb, MSBsplineCurveCP, double const* tolerance) const = 0;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct  ILineStyle
{
    virtual Utf8CP _GetName () const = 0;
    virtual ILineStyleComponent const* _GetComponent() const = 0;
    virtual bool _IsSnappable() const = 0;
};

//=======================================================================================
//! This interface defines methods required for a \e DisplaySymbol Definition.
//! A DisplaySymbol is a set of graphics that is cached once and can then be redrawn 
//! many times at different locations/sizes/clipping/symbology.
//! @note DisplaySymbol are drawn via IDrawGeom::DrawSymbol.
//! @bsiclass
//=======================================================================================
struct     IDisplaySymbol
{
    virtual ~IDisplaySymbol () {}
    virtual void        _Draw (ViewContextR context) = 0;
    virtual StatusInt   _GetRange (DRange3dR range) const = 0;
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct IRangeNodeCheck
{
    virtual ScanCriteria::Result _CheckNodeRange(ScanCriteriaCR, DRange3dCR, bool is3d) = 0;
};

//=======================================================================================
// @bsiclass                                                   
//=======================================================================================
enum EdgeMaskState
{
    EdgeMaskState_None,
    EdgeMaskState_GenerateMask,
    EdgeMaskState_UseMask
};

/*=================================================================================**//**
* @bsiclass  
+===============+===============+===============+===============+===============+======*/
struct QvUnsizedKey
{
private:
    uint32_t m_transformKey;
    int32_t  m_qvIndex;

public:
    bool IsNull () const;
    bool Matches (QvUnsizedKey const& other) const;
    QvUnsizedKey (uint32_t transformKey, uint32_t qvIndex) : m_transformKey (transformKey), m_qvIndex (qvIndex) {}
    bool OwnsQvElem() { return m_qvIndex >= 0; }
}; 

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct ICheckStop
{   
private:
    bool m_aborted;

public:
    bool InitAborted (bool val) {return m_aborted = val;}
    bool ClearAborted() {return m_aborted = false;}
    bool WasAborted ()  {return m_aborted;}
    bool SetAborted() {return m_aborted = true;}
    bool AddAbortTest(bool val) {return  m_aborted |= val;}

    ICheckStop() {m_aborted=false;}
    //! return true to abort the current operation.
    //! @note Overrides MUST call SetAborted or use AddAbortTest since WasAborted may be directly tested!
    virtual bool _CheckStop () {return m_aborted;}
};

//=======================================================================================
//! @bsiclass                                                     KeithBentley    04/01
//=======================================================================================
struct ViewContext : NonCopyableClass, ICheckStop, IRangeNodeCheck
{
    friend struct ViewController;
    friend struct SimplifyViewDrawGeom;

public:
    enum RasterPlane
    {
        RasterPlane_Background = (1<<0),
        RasterPlane_Design     = (1<<1),
        RasterPlane_Foreground = (1<<2),
        RasterPlane_Any        = (RasterPlane_Background | RasterPlane_Design | RasterPlane_Foreground),
    };

    //=======================================================================================
    //! @bsiclass                                                     Stephane.Poulin    11/11
    //=======================================================================================
    struct RasterDisplayParams
        {
        private:
            int8_t        m_contrast;                        // -128...+127, 0 -> no contrast.
            int8_t        m_brightness;                      // -128...+127, 0 -> no brightness.
            bool          m_greyScale;                       // Output raster in greyscale.
            bool          m_applyBinaryWhiteOnWhiteReversal; // Apply color inversion logic on monochrome. (Prevent to display/print a white foreground on a white media)
            bool          m_enableGrid;                      // Superimpose a grid over the raster.
            ColorDef      m_backgroundColor;                 // Background color for binary image.
            ColorDef      m_foregroundColor;                 // Foreground color for binary image.
            double        m_quality;
            uint32_t      m_flags;

        public:
            enum RasterDisplayParamsFlags
                {
                RASTER_PARAM_None                            = 0,
                RASTER_PARAM_Contrast                        = (1<<0),
                RASTER_PARAM_Brightness                      = (1<<1),
                RASTER_PARAM_GreyScale                       = (1<<2),
                RASTER_PARAM_BackgroundColor                 = (1<<3),
                RASTER_PARAM_ForegroundColor                 = (1<<4),
                RASTER_PARAM_Quality                         = (1<<5),
                RASTER_PARAM_ApplyBinaryWhiteOnWhiteReversal = (1<<6),
                RASTER_PARAM_EnableGrid                      = (1<<7)
                };

            DGNPLATFORM_EXPORT RasterDisplayParams();
            DGNPLATFORM_EXPORT bool operator==(RasterDisplayParams const& rhs) const;
            DGNPLATFORM_EXPORT bool operator!=(RasterDisplayParams const& rhs) const;

            uint32_t GetFlags() const { return m_flags; }
            int8_t GetContrast() const { return m_contrast; }
            int8_t GetBrightness() const { return m_brightness; }
            bool GetGreyscale() const { return m_greyScale; }
            bool GetApplyBinaryWhiteOnWhiteReversal() const { return m_applyBinaryWhiteOnWhiteReversal; }
            bool GetEnableGrid() const { return m_enableGrid; }
            ColorDefCR GetBackgroundColor() const { return m_backgroundColor; }
            ColorDefCR GetForegroundColor() const { return m_foregroundColor; }
            double GetQualityFactor() const { return m_quality; }

            DGNPLATFORM_EXPORT void SetFlags(uint32_t flags);
            DGNPLATFORM_EXPORT void SetContrast(int8_t value);
            DGNPLATFORM_EXPORT void SetBrightness(int8_t value);
            DGNPLATFORM_EXPORT void SetGreyscale(bool value);
            DGNPLATFORM_EXPORT void SetApplyBinaryWhiteOnWhiteReversal(bool value);
            DGNPLATFORM_EXPORT void SetEnableGrid(bool value);
            DGNPLATFORM_EXPORT void SetBackgroundColor(ColorDef value);
            DGNPLATFORM_EXPORT void SetForegroundColor(ColorDef value);
            DGNPLATFORM_EXPORT void SetQualityFactor(double factor);
        };

    //=======================================================================================
    //! @bsiclass                                                     KeithBentley    04/01
    //=======================================================================================
    struct  ContextMark
        {
        ViewContextP    m_context;
        size_t          m_hdrOvrMark;
        size_t          m_transClipMark;
        size_t          m_dynamicViewStateStackMark;
        size_t          m_displayStyleStackMark;
        bool            m_pushedRange;
        bool            m_ignoreScaleForDimensions;
        bool            m_ignoreScaleForMultilines;
        bool            m_applyRotationToDimView;
        RangeResult     m_parentRangeResult;
        double          m_reservedDouble;

    public:
        DGNPLATFORM_EXPORT explicit ContextMark (ViewContextP context);
        DGNPLATFORM_EXPORT ~ContextMark ();

        DGNPLATFORM_EXPORT void Pop ();
        DGNPLATFORM_EXPORT void SetNow ();
        void Init (ViewContextP context) { m_hdrOvrMark = m_transClipMark = 0; m_context = context; m_parentRangeResult = RangeResult::Overlap; m_pushedRange = m_ignoreScaleForDimensions = m_ignoreScaleForMultilines = m_applyRotationToDimView = false; m_reservedDouble = 0.0;}
        };

    friend struct ContextMark;

    //=======================================================================================
    //! @bsiclass                                                     Brien.Bastings  11/07
    //=======================================================================================
    struct  ClipStencil
        {
    private:
        IStrokeForCache&    m_stroker;
        QvElem*             m_tmpQvElem;
        CurveVectorPtr      m_curveVector;

    public:
        DGNPLATFORM_EXPORT QvElem* GetQvElem (ViewContextR);
        DGNPLATFORM_EXPORT CurveVectorPtr GetCurveVector ();
        IStrokeForCache& GetStroker () {return m_stroker;}

        DGNPLATFORM_EXPORT explicit ClipStencil (IStrokeForCache& stroker);
        DGNPLATFORM_EXPORT ~ClipStencil ();
        };

    enum AlignmentMode
        {
        AlignmentMode_None                   = 0,
        AlignmentMode_AlongLocalInDrawing    = 1,
        AlignmentMode_AlongDrawing           = 2,
        AlignmentMode_Billboard              = 3,
        };

   //=======================================================================================
    //! @bsiclass                                                     Ray.Bentley     10/12
    //=======================================================================================
    struct IStrokeAligned 
        {
        virtual void _StrokeAligned (ViewContextR viewContext) = 0;
        };

protected:
    bool                    m_isAttached;
    bool                    m_blockAsyncs;
    bool                    m_blockIntermediatePaints;
    bool                    m_is3dView;
    bool                    m_isCameraOn;
    bool                    m_wantMaterials;
    bool                    m_creatingCacheElem;
    bool                    m_useNpcSubRange;
    bool                    m_ignoreScaleForDimensions;
    bool                    m_ignoreScaleForMultilines;
    bool                    m_applyRotationToDimView;
    bool                    m_useCachedGraphics;
    bool                    m_ignoreViewRange;
    Byte                    m_filterLOD;
    size_t                  m_refTransClipDepth;
    size_t                  m_frustumTransClipDepth;   // stack depth when starting a frustum change
    DrawPurpose             m_purpose;
    DRange3d                m_npcSubRange;
    DMap4d                  m_frustumToNpc;
    DMap4d                  m_frustumToView;
    double                  m_cameraFraction;           // ratio of front plane to back plane.
    TransformClipStack      m_transformClipStack;
    DgnViewportP            m_viewport;
    IViewDrawP              m_IViewDraw;
    IDrawGeomP              m_IDrawGeom;
    ICachedDrawP            m_ICachedDraw;
    DgnDbP                  m_dgnDb;
    DgnElementPtr           m_currentElement;
    ScanCriteriaP           m_scanCriteria;
    int32_t                 m_displayPriorityRange[2];
    ElemDisplayParams       m_currDisplayParams;
    ElemMatSymb             m_elemMatSymb;
    OvrMatSymb              m_ovrMatSymb;
    RangeResult             m_parentRangeResult;
    double                  m_minLOD;             // minimum size of default level-of-detail test.
    double                  m_arcTolerance;
    double                  m_patternScale;
    DPoint3dCP              m_startTangent;       // linestyle start tangent.
    DPoint3dCP              m_endTangent;         // linestyle end tangent.
    uint32_t                m_rasterPlane;        // Current displayed raster plane
    bool                    m_drawingClipElements;
    uint32_t                m_displayStyleStackMark;
    EdgeMaskState           m_edgeMaskState;
    DgnElement::Hilited     m_hiliteState;
    RasterDisplayParams     m_rasterDisplayParams;
    IElemTopologyPtr        m_currElemTopo;
    DgnGeomPartId           m_currGeomPart;

    bool                    m_scanRangeValid;
    double                  m_levelOfDetail;

    DGNPLATFORM_EXPORT void PopOneTransformClip ();
    void InvalidateScanRange ();
    DGNPLATFORM_EXPORT void InitDisplayPriorityRange ();
    DGNPLATFORM_EXPORT bool CheckThicknessVector ();
    DGNPLATFORM_EXPORT virtual StatusInt _Attach (DgnViewportP, DrawPurpose purpose);
    DGNPLATFORM_EXPORT virtual void _Detach ();
    DGNPLATFORM_EXPORT virtual void _SetupOutputs () = 0;
    DGNPLATFORM_EXPORT virtual void _OutputElement (GeometricElementCR);
    DGNPLATFORM_EXPORT virtual uint32_t _GetDisplayInfo (bool isRenderable);
    DGNPLATFORM_EXPORT virtual bool _WantAreaPatterns ();
    DGNPLATFORM_EXPORT virtual void _DrawAreaPattern (ClipStencil& boundary);
    DGNPLATFORM_EXPORT virtual void _DrawSymbol (IDisplaySymbol*, TransformCP, ClipPlaneSetP, bool ignoreColor, bool ignoreWeight);
    DGNPLATFORM_EXPORT virtual void _DeleteSymbol (IDisplaySymbol*);
    DGNPLATFORM_EXPORT virtual ILineStyleCP _GetCurrLineStyle (LineStyleSymbP*);
    DGNPLATFORM_EXPORT virtual void _DrawStyledLineString2d (int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false);
    DGNPLATFORM_EXPORT virtual void _DrawStyledLineString3d (int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed = false);
    DGNPLATFORM_EXPORT virtual void _DrawStyledArc2d (DEllipse3dCR, bool isEllipse, double zDepth, DPoint2dCP range);
    DGNPLATFORM_EXPORT virtual void _DrawStyledArc3d (DEllipse3dCR, bool isEllipse, DPoint3dCP range);
    DGNPLATFORM_EXPORT virtual void _DrawStyledBSplineCurve3d (MSBsplineCurveCR);
    DGNPLATFORM_EXPORT virtual void _DrawStyledBSplineCurve2d (MSBsplineCurveCR, double zDepth);
    DGNPLATFORM_EXPORT virtual void _DrawTextString (TextStringCR);
    DGNPLATFORM_EXPORT virtual QvElem* _DrawCached (IStrokeForCache&);
    virtual bool _CheckFillOutline () {return true;}
    DGNPLATFORM_EXPORT virtual StatusInt _InitContextForView ();
    DGNPLATFORM_EXPORT virtual StatusInt _VisitElement (GeometricElementCR);
    DGNPLATFORM_EXPORT virtual void _InitScanRangeAndPolyhedron ();
    DGNPLATFORM_EXPORT virtual bool _VisitAllModelElements (bool includeTransients);
    DGNPLATFORM_EXPORT virtual StatusInt _VisitDgnModel (DgnModelP);
    DGNPLATFORM_EXPORT virtual void _PushTransform (TransformCR trans);
    DGNPLATFORM_EXPORT virtual void _PushClip (ClipVectorCR clip);
    virtual QvExtSymbID _BuildExtSymbID (uint32_t rasterWidth, int styleIndex) const { BeAssert (false); return 0; }
    DGNPLATFORM_EXPORT virtual void _PushViewIndependentOrigin (DPoint3dCP origin);
    DGNPLATFORM_EXPORT virtual void _PopTransformClip ();
    DGNPLATFORM_EXPORT virtual bool _FilterRangeIntersection (GeometricElementCR);
    DGNPLATFORM_EXPORT virtual DgnModelP _GetViewTarget();
    virtual IPickGeomP _GetIPickGeom () {return NULL;}
    DGNPLATFORM_EXPORT virtual void _VisitTransientGraphics (bool isPreUpdate);
    DGNPLATFORM_EXPORT virtual bool _WantShowDefaultFieldBackground ();
    DGNPLATFORM_EXPORT virtual void _AllocateScanCriteria ();
    DGNPLATFORM_EXPORT virtual void _SetupScanCriteria ();
    virtual bool _WantUndisplayed () {return false;}
    virtual bool _WantUndisplayedClips () {return false;}
    virtual bool _UseCachedDisplay () {return m_useCachedGraphics;}
    DGNPLATFORM_EXPORT virtual bool _WantSaveQvElem (DrawExpense expense);

    DGNPLATFORM_EXPORT virtual void _AddViewOverrides (OvrMatSymbR);
    DGNPLATFORM_EXPORT virtual void _AddContextOverrides (OvrMatSymbR);
    DGNPLATFORM_EXPORT virtual void _ModifyPreCook (ElemDisplayParamsR); 
    DGNPLATFORM_EXPORT virtual void _CookDisplayParams (ElemDisplayParamsR, ElemMatSymbR);
    DGNPLATFORM_EXPORT virtual void _CookDisplayParamsOverrides (ElemDisplayParamsR, OvrMatSymbR);
    DGNPLATFORM_EXPORT virtual void _SetScanReturn ();
    DGNPLATFORM_EXPORT virtual void _PushFrustumClip ();
    DGNPLATFORM_EXPORT virtual void _InitScanCriteria ();
    DGNPLATFORM_EXPORT virtual StatusInt _ScanDgnModel (DgnModelP model);
    DGNPLATFORM_EXPORT virtual bool _ScanRangeFromPolyhedron ();
    DGNPLATFORM_EXPORT virtual int _GetViewNumber () const;
    DGNPLATFORM_EXPORT virtual void _SetDgnDb (DgnDbR);
    DGNPLATFORM_EXPORT virtual void _SetCurrentElement (GeometricElementCP);
    DGNPLATFORM_EXPORT virtual void _ClearZ ();
    DGNPLATFORM_EXPORT virtual ScanCriteria::Result _CheckNodeRange(ScanCriteriaCR, DRange3dCR, bool is3d);
    DGNPLATFORM_EXPORT virtual void _DrawAligned (DVec3dCR axis, DPoint3dCR origin, AlignmentMode type, IStrokeAligned& stroker);
    DGNPLATFORM_EXPORT virtual void _SetLocatePriority (int priority);
    DGNPLATFORM_EXPORT virtual void _SetNonSnappable (bool unsnappable);

    DGNPLATFORM_EXPORT ViewContext ();
    DGNPLATFORM_EXPORT virtual ~ViewContext ();

public:
    DGNPLATFORM_EXPORT static uint32_t GetCountQvInitCalls();
    DGNPLATFORM_EXPORT static void IncrementCountQvInitCalls ();
                       static StampQvElemMapP CreateSymbolStampMap(DgnDbCR dgnProject);
                       static void DeleteSymbolStampMap(StampQvElemMapP symbolStampMap);
    DGNPLATFORM_EXPORT static void MergeViewFlagsFromRef (ViewFlagsR flags, ViewFlagsCR refFlags, bool retainRenderMode, bool useRefFlags);

    DMap4dCR GetFrustumToView () const {return m_frustumToView;}
    DMap4dCR GetFrustumToNpc () const {return m_frustumToNpc;}
    bool GetWantMaterials () {return m_wantMaterials;};
    void SetIViewDraw (IViewDrawR output) { m_IViewDraw = &output; m_IDrawGeom = &output;}
    void SetIDrawGeom (IDrawGeomR drawGeom) { m_IDrawGeom = &drawGeom; }
    bool IsAttached () {return m_isAttached;}
    bool IsRealView () {return (-1 != _GetViewNumber());}
    void SetBlockAsynchs (bool blockAsyncs) {m_blockAsyncs = blockAsyncs;}
    bool WantAsynchs () {return !m_blockAsyncs;}
    void SetIntermediatePaintsBlocked (bool blockIntermediatePaints) {m_blockIntermediatePaints = blockIntermediatePaints;}
    void SetRasterPlane (uint32_t plane) {m_rasterPlane = plane;}
    void ResetRasterPlane () {m_rasterPlane = RasterPlane_Any;}
    DgnElement::Hilited GetCurrHiliteState () {return m_hiliteState;}
    void SetSubRectFromViewRect(BSIRectCP viewRect);
    DGNPLATFORM_EXPORT void SetSubRectNpc(DRange3dCR subRect);
    DGNPLATFORM_EXPORT bool SetWantMaterials (bool wantMaterials);
    DGNPLATFORM_EXPORT DMatrix4d GetLocalToView () const;
    DGNPLATFORM_EXPORT DMatrix4d GetViewToLocal () const;
    DGNPLATFORM_EXPORT bool ValidateScanRange ();
    DGNPLATFORM_EXPORT StatusInt Attach (DgnViewportP, DrawPurpose purpose);
    DGNPLATFORM_EXPORT void Detach ();
    DGNPLATFORM_EXPORT bool VisitAllModelElements (bool includeTransients); // DgnModelListP includeList, bool useUpdateSequence, bool includeRefs, bool includeTransients);
    DGNPLATFORM_EXPORT bool VisitAllViewElements (bool includeTransients, BSIRectCP updateRect); // DgnModelListP includeList, bool useUpdateSequence, bool includeRefs, bool includeTransients);
    DGNPLATFORM_EXPORT StatusInt VisitHit (HitDetailCR hit);
    DGNPLATFORM_EXPORT void VisitTransientGraphics (bool isPreUpdate);
    DGNPLATFORM_EXPORT BentleyStatus GetCurrLocalToWorldTrans (DMatrix4dR localToWorld) const;
    DGNPLATFORM_EXPORT void DrawBox (DPoint3dP box, bool is3d);
    DGNPLATFORM_EXPORT StatusInt InitContextForView ();
    DGNPLATFORM_EXPORT bool IsFrustumPointVisible (DPoint3dCR frustumPoint, bool boresite);
    DGNPLATFORM_EXPORT bool IsLocalPointVisible (DPoint3dCR localPoint, bool boresite);
    DGNPLATFORM_EXPORT bool PointInsideClip (DPoint3dCR point);
    DGNPLATFORM_EXPORT bool GetRayClipIntersection (double& distance, DPoint3dCR origin, DVec3dCR direction);
    DGNPLATFORM_EXPORT Frustum GetFrustum();

    DGNPLATFORM_EXPORT void ClearZ ();
    void SetEdgeMaskState (EdgeMaskState state) {m_edgeMaskState = state;}
    EdgeMaskState GetEdgeMaskState () const {return m_edgeMaskState;}

    DGNPLATFORM_EXPORT void SetNpcSubRange(DRange3dCP pSubRange);

    DGNPLATFORM_EXPORT uint32_t GetLocalTransformKey () const;
    DGNPLATFORM_EXPORT TransformClipStackR GetTransformClipStack () { return m_transformClipStack; }
    DGNPLATFORM_EXPORT bool& GetDrawingClipElements () { return m_drawingClipElements; }
    DGNPLATFORM_EXPORT bool WantUndisplayedClips ();

    DGNPLATFORM_EXPORT size_t GetTransClipDepth ();
    DGNPLATFORM_EXPORT size_t GetRefTransClipDepth ();
    DGNPLATFORM_EXPORT double GetArcTolerance () const;
    DGNPLATFORM_EXPORT void SetArcTolerance (double tol);
    DGNPLATFORM_EXPORT void SetLinestyleTangents (DPoint3dCP start, DPoint3dCP end);

    //! @return   Mask of GeomRepresentations.
    DGNPLATFORM_EXPORT uint32_t GetDisplayInfo (bool isRenderable);

    DGNPLATFORM_EXPORT QvElem* CreateCacheElem (IStrokeForCache&, QvCache* qvCache = nullptr, double pixelSize = 0.0, ICachedDrawP cachedDrawP = nullptr);
    DGNPLATFORM_EXPORT QvElem* GetCachedGeometry (IStrokeForCache& stroker, bool& deleteQvElem);

    DGNPLATFORM_EXPORT void DeleteSymbol (IDisplaySymbol*);

    DGNPLATFORM_EXPORT RangeResult     GetCurrParentRangeResult ();
    DGNPLATFORM_EXPORT void            SetCurrParentRangeResult (RangeResult);
    DGNPLATFORM_EXPORT double          GetMinLOD () const;
    DGNPLATFORM_EXPORT void            SetMinLOD (double);
    DGNPLATFORM_EXPORT Byte&           GetFilterLODFlag ();
    DGNPLATFORM_EXPORT void            SetFilterLODFlag (FilterLODFlags);
    DGNPLATFORM_EXPORT ScanCriteriaCP  GetScanCriteria () const;
    DGNPLATFORM_EXPORT bool            GetIgnoreScaleForDimensions ();
    DGNPLATFORM_EXPORT bool            GetIgnoreScaleForMultilines ();
    DGNPLATFORM_EXPORT bool            GetApplyRotationToDimView ();
    DGNPLATFORM_EXPORT void            SetIgnoreScaleForDimensions (bool ignore);
    DGNPLATFORM_EXPORT void            SetIgnoreScaleForMultilines (bool ignore);
    DGNPLATFORM_EXPORT void            SetApplyRotationToDimView (bool apply);
    DGNPLATFORM_EXPORT uint32_t        GetRasterPlane () const;
    DGNPLATFORM_EXPORT void            InitScanRangeAndPolyhedron ();
    DGNPLATFORM_EXPORT void            AllocateScanCriteria ();
    DGNPLATFORM_EXPORT bool            CheckFillOutline ();
    DGNPLATFORM_EXPORT void            VisitDgnModel (DgnModelP modelRef);
    DGNPLATFORM_EXPORT void            SetScanReturn ();
    DGNPLATFORM_EXPORT RasterDisplayParams const& GetRasterDisplayParams() const { return m_rasterDisplayParams; }

    //! !!!FOR INTERNAL USE ONLY!!!
    DGNPLATFORM_EXPORT static void DirectPushTransClipOutput (IDrawGeomR, TransformCP trans, ClipPlaneSetCP clip = NULL); //<! @private
    DGNPLATFORM_EXPORT static void DirectPopTransClipOutput (IDrawGeomR); //<! @private

public:

DGNPLATFORM_EXPORT StatusInt VisitElement (GeometricElementCR);    

/// @name Coordinate Query and Conversion
//@{

//! Transform an array of points in the current local coordinate system into DgnCoordSystem::Frustum coordinates.
//! @param[out]     frustumPts  An array to receive the points in DgnCoordSystem::Frustum. Must be dimensioned to hold \c nPts points.
//! @param[in]      localPts    Input array in current local coordinates,
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void LocalToFrustum (DPoint3dP frustumPts, DPoint3dCP localPts, int nPts) const;

//! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
//! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
//! @param[in]      localPts    Input array in current local coordinates,
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void LocalToView (DPoint4dP viewPts, DPoint3dCP localPts, int nPts) const;

//! Transform an array of points in the current local coordinate system into DgnCoordSystem::View coordinates.
//! @param[out]     viewPts     An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
//! @param[in]      localPts    Input array in current local coordinates,
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void LocalToView (DPoint3dP viewPts, DPoint3dCP localPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::Frustum into the current local coordinate system.
//! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      frustumPts  Input array in DgnCoordSystem::Frustum.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void FrustumToLocal (DPoint3dP localPts, DPoint3dCP frustumPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
//! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      viewPts     Input array in DgnCoordSystem::View.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void ViewToLocal (DPoint3dP localPts, DPoint4dCP viewPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::View into the current local coordinate system.
//! @param[out]     localPts    An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      viewPts     Input array in DgnCoordSystem::View.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void ViewToLocal (DPoint3dP localPts, DPoint3dCP viewPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::View.
//! @param[out]     viewPts     An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      npcPts      Input array in DgnCoordSystem::Npc.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void NpcToView (DPoint3dP viewPts, DPoint3dCP npcPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::Npc.
//! @param[out]     npcPts      An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      viewPts     Input array in DgnCoordSystem::View.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void ViewToNpc (DPoint3dP npcPts, DPoint3dCP viewPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::Frustum.
//! @param[out]     frustumPts  An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      npcPts      Input array in DgnCoordSystem::Npc.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void NpcToFrustum (DPoint3dP frustumPts, DPoint3dCP npcPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::Frustum into DgnCoordSystem::View.
//!
//! @param[out]     viewPts     An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      frustumPts  Input array in DgnCoordSystem::Frustum.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void FrustumToView (DPoint4dP viewPts, DPoint3dCP frustumPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::Frustum into DgnCoordSystem::View.
//! @param[out]     viewPts     An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      frustumPts  Input array in DgnCoordSystem::Frustum.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void FrustumToView (Point2d* viewPts, DPoint3dCP frustumPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::Frustum.
//! @param[out]     frustumPts  An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      viewPts     Input array in DgnCoordSystem::View.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void ViewToFrustum (DPoint3dP frustumPts, DPoint4dCP viewPts, int nPts) const;

//! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::Frustum.
//! @param[out]     frustumPts  An array to receive the transformed points. Must be dimensioned to hold \c nPts points.
//! @param[in]      viewPts     Input array in DgnCoordSystem::View.
//! @param[in]      nPts        Number of points in both arrays.
DGNPLATFORM_EXPORT void ViewToFrustum (DPoint3dP frustumPts, DPoint3dCP viewPts, int nPts) const;

//! Retrieve a pointer to the the transform from the current local coordinate system into DgnCoordSystem::Frustum.
//! @return   NULL if no transform present.
DGNPLATFORM_EXPORT TransformCP GetCurrLocalToFrustumTransformCP () const;


//! Retrieve a copy of the transform from the current local coordinate system into DgnCoordSystem::Frustum.
//! @param[out]     trans       Transform from current local coordinate system to DgnCoordSystem::Frustum
//! @return   SUCCESS if there is a current local coordinate system.
DGNPLATFORM_EXPORT BentleyStatus GetCurrLocalToFrustumTrans (TransformR trans) const;

//! Retrieve a copy of the transform from the DgnCoordSystem::Frustum to current local coordinate system.
//! @param[out]     trans       Transform from DgnCoordSystem::Frustum to current local coordinate system
//! @return   SUCCESS if there is a current local coordinate system.
DGNPLATFORM_EXPORT BentleyStatus GetCurrFrustumToLocalTrans (TransformR trans) const;

//! Retrieve a copy of the transform from the local coordinate system at the specified index into DgnCoordSystem::Frustum.
//! @param[out]    trans  Transform from local coordinate system at the specified index to DgnCoordSystem::Frustum
//! @param[in]      index       Index into transform stack to return transform for.
//! @return   SUCCESS if there is a local coordinate system.
DGNPLATFORM_EXPORT BentleyStatus GetLocalToFrustumTrans (TransformR trans, size_t index) const;

//! Calculate the size of a "pixel" at a given point in the current local coordinate system. This method can be used to
//! approximate how large geometry in local coordinates will appear in DgnCoordSystem::View units.
//! @param[in]      origin      The point at which the pixel size is calculated. This point is only relevant in camera views, where local coordinates
//!                             closer to the eye are larger than those further from the eye. May be NULL, in which case the center of the view is used.
//! @return the length, in the current coordinate system units, of a unit bvector in the x direction in DgnCoordSystem::View, starting at \c origin.
DGNPLATFORM_EXPORT double GetPixelSizeAtPoint (DPoint3dCP origin) const;

DGNPLATFORM_EXPORT void FrustumToView (DPoint3dP viewPts, DPoint3dCP frustumPt, int nPts) const;

DGNPLATFORM_EXPORT void GetViewIndTransform (TransformP trans, DPoint3dCP originLocal);

//! Check whether the current transform is view independent. Several MicroStation element types can display
//! as "View independent" (e.g. text, text nodes, point cells). They do this by pushing the inverse of the current
//! view-to-local transformation via #PushViewIndependentOrigin.
//! @return   true if the current local coordinate system is a view independent transform.
DGNPLATFORM_EXPORT bool IsViewIndependent ();
//@}

/// @name Pushing and Popping Transforms and Clips
//@{
//! Push a Transform, creating a new local coordinate system.
//! @param[in]      trans       The transform to push.
//! @see   PopTransformClip
DGNPLATFORM_EXPORT void PushTransform (TransformCR trans);

/// @name Pushing and Popping Transforms and Clips
//@{
//! Push a ClipVector, creating a new local clip region.
//! @param[in]      clip       A clipping descriptor to push.
//! @see   PopTransformClip
DGNPLATFORM_EXPORT void PushClip (ClipVectorCR clip);

//! Push a set of clip planes, creating a new local clip region.
//! @param[in]      clipPlanes  Clipping planes to push - the intersections of their half planes define clip region.
//! @see   PopTransformClip
DGNPLATFORM_EXPORT void PushClipPlanes (ClipPlaneSetCR clipPlanes);

//! Push a transform such that the X,Y plane of the new local coordinate system will be aligned with the X,Y plane of the
//! view coordinate system, oriented about the given origin.
//! @param[in]      origin      Origin for rotation, in the \e current local coordinate system.
DGNPLATFORM_EXPORT void PushViewIndependentOrigin (DPoint3dCP origin);

//! Remove the most recently pushed coordinate system and clip, restoring the local coordinate system to its previous state.
DGNPLATFORM_EXPORT void PopTransformClip ();
//@}

/// @name Query Methods
//@{

//! Get the current state of the ViewFlags for this context's output, can be NULL.
//! When a ViewContext is first attached to a DgnViewport, the ViewFlags are initialized
//! from the DgnViewport's viewflags. However, during the course of an operation,
//! the viewflags for the output may be different than those on the DgnViewport.
//! @return   the current state of the viewflags for this ViewContext.
DGNPLATFORM_EXPORT ViewFlagsCP GetViewFlags () const;

//! Sets the current state of the ViewFlags for this context's output.
DGNPLATFORM_EXPORT void SetViewFlags (ViewFlagsCP);

//! Get the DgnDb for this ViewContext.
DGNPLATFORM_EXPORT DgnDbR GetDgnDb () const;

//! Get the current persistent element being visited by this ViewContext.
DGNPLATFORM_EXPORT GeometricElementCP GetCurrentElement () const;

/** @cond BENTLEY_SDK_Scope1 */
//! Set the project for this ViewContext when not attaching a viewport.
DGNPLATFORM_EXPORT void SetDgnDb (DgnDbR);

//! Set or clear the current persistent element.
DGNPLATFORM_EXPORT void SetCurrentElement (GeometricElementCP);
/** @endcond */

//! Get the DrawPurpose specified when this ViewContext was attached to the current DgnViewport.
//! @return the DrawPurpose specified in the call to DrawContext#Attach (drawcontext.h)
DGNPLATFORM_EXPORT DrawPurpose GetDrawPurpose () const;

//! Get the DgnViewport to which this ViewContext is attached. ViewContext's do not always have to be attached to an
//! DgnViewport, so therefore callers must always test the result of this call for NULL.
//! @return the DgnViewport. NULL if not attached to a DgnViewport.
DGNPLATFORM_EXPORT DgnViewportP GetViewport () const;

DGNPLATFORM_EXPORT bool Is3dView () const;
DGNPLATFORM_EXPORT bool IsCameraOn () const;
//@}

//! Get the clip planes that define the limits of the displayed volume.  This will include
//! planes for the top,bottom,left and right sides of the view and optionally the front
//! and back planes if they are enabled.
//! @return the clip planes set.
DGNPLATFORM_EXPORT ClipPlaneSetCP GetRangePlanes () const;

/// @name Get/Set Current Display Parameters
//@{

//__PUBLISH_SECTION_END__

//! Get the current LineStyleSymb.
//! @return the current LineStyleSymb.
DGNPLATFORM_EXPORT ILineStyleCP GetCurrLineStyle (LineStyleSymbP* symb);

//! Set the linestyle in the current linestyle MatSymb. This method is mainly used to temporarily clear the current
//! linestyle, for pieces of geometry that are to be drawn solid. To do that, call GetCurrLineStyle and save the
//! current value. Then call this method passing NULL and when you're done, call this method again to restore the saved linestyle.
//! @param[in]      lstyle      The new current linestyle. If NULL, no (solid) linestyle is used.
DGNPLATFORM_EXPORT void SetCurrLineStyle (ILineStyleCP lstyle);

DGNPLATFORM_EXPORT bool& GetUseCachedGraphics();
DGNPLATFORM_EXPORT bool GetDisplayPriorityRange (int32_t& low, int32_t& high) const;

//! Change the supplied "natural" ElemDisplayParams. Resolves effective symbology as required by the context and initializes the supplied ElemMatSymb.
//! @note Does NOT call ActivateMatSymb on the output or change the current ElemDisplayParams/ElemMatSymb of the context.
DGNPLATFORM_EXPORT void CookDisplayParams (ElemDisplayParamsR, ElemMatSymbR);

//! Change the supplied "natural" ElemDisplayParams. Resolves effective symbology as required by the context and initializes the supplied OvrMatSymb.
//! @note Does NOT call ActivateOverrideMatSymb on the output or change the current ElemDisplayParams/OvrMatSymb of the context.
DGNPLATFORM_EXPORT void CookDisplayParamsOverrides (ElemDisplayParamsR, OvrMatSymbR);

//__PUBLISH_SECTION_START__

DGNPLATFORM_EXPORT ColorDef GetCurrLineColor ();
DGNPLATFORM_EXPORT ColorDef GetCurrFillColor ();
DGNPLATFORM_EXPORT uint32_t GetCurrWidth ();

//! Calculate the net display priority value. The net display priority is based on the geometry (element) and sub-category priority.
//! @return the net display priority. For 3D views, display priority is always 0.
DGNPLATFORM_EXPORT int32_t ResolveNetDisplayPriority (int32_t geomPriority, DgnSubCategoryId subCategoryId, DgnCategories::SubCategory::Appearance* appearance = nullptr) const;

//! Get the current ElemMatSymb.
//! @return   the current ElemMatSymb.
DGNPLATFORM_EXPORT ElemMatSymbP GetElemMatSymb ();

//! Get the current OvrMatSymb.
//! @return the current OvrMatSymb.
DGNPLATFORM_EXPORT OvrMatSymbP GetOverrideMatSymb ();

//! Get the current ElemDisplayParams.
//! @return the current ElemDisplayParams.
DGNPLATFORM_EXPORT ElemDisplayParamsP GetCurrentDisplayParams();

//! Change the current "natural" ElemDisplayParams. Resolves effective symbology as required by the context and initializes the current ElemMatSymb.
//! @note Calls ActivateMatSymb on the output.
DGNPLATFORM_EXPORT void CookDisplayParams ();

//! Change the current ElemDisplayParams for any context overrides. Cooks the modified ElemDisplayParams into the current OvrMatSymb using the current override flags.
//! @note Calls ActivateOverrideMatSymb on the output.
DGNPLATFORM_EXPORT void CookDisplayParamsOverrides ();

//! Clears current override flags and re-applies context overrides.
//! @note Calls ActivateOverrideMatSymb on the output.
DGNPLATFORM_EXPORT void ResetContextOverrides ();

//! Do a line width lookup in the current lineweight to width table.
//! @param[in]      index       Line weight between 0 and 31.
//! @return    line width in pixels
DGNPLATFORM_EXPORT uint32_t GetIndexedLineWidth (int index);

//! Get the 32 bit on-off "line pattern" for a line code value for this context.
//! Output devices can change the on-off patterns based on resolution, etc.
//! @param[in]          index       a the range of 0 to 7.
//! @return             the line pattern value.
//! @bsimethod
DGNPLATFORM_EXPORT uint32_t GetIndexedLinePattern (int index);

//! Set the line width in the given material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]      elemMatSymb ElemMatSymb in which to change the line width.
//! @param[in]      index       Line weight between 0 and 31.
DGNPLATFORM_EXPORT void SetIndexedLineWidth (ElemMatSymbR elemMatSymb, int index);

//! Set the line pattern in the given material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]      elemMatSymb ElemMatSymb in which to change the line pattern.
//! @param[in]      index       Line style index between 0 and 7.
DGNPLATFORM_EXPORT void SetIndexedLinePattern (ElemMatSymbR elemMatSymb, int index);

//! Set the line width in the given override material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]  ovrMatSymb      OvrMatSymb in which to change the line width.
//! @param[in]  index Line      weight between 0 and 31.
DGNPLATFORM_EXPORT void SetIndexedLineWidth (OvrMatSymbR ovrMatSymb, int index);

//! Set the line pattern in the given override material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]  ovrMatSymb      OvrMatSymb in which to change the line pattern.
//! @param[in]  index Line      style index between 0 and 7.
DGNPLATFORM_EXPORT void SetIndexedLinePattern (OvrMatSymbR ovrMatSymb, int index);

//! Set the locate priority for displayed geometry. 
//! @param[in]   priority       Hit Priority
DGNPLATFORM_EXPORT void SetLocatePriority (int priority);

//! Set geometry displayed to this view to be non-snappable 
//! @param[in]   unsnappable          The non-snappable status.
DGNPLATFORM_EXPORT void SetNonSnappable (bool unsnappable);

//__PUBLISH_SECTION_END__

//! Gets the current level of detail.
//! @return       the current level of detail.
DGNPLATFORM_EXPORT double GetCurrentLevelOfDetail() const;


//! Sets the current level of detail.
DGNPLATFORM_EXPORT void SetCurrentLevelOfDetail(double levelOfDetail);

//! Check the current display style for a monochrome color override.
//! @return       whether monochrome style is currently active.
DGNPLATFORM_EXPORT bool IsMonochromeDisplayStyleActive ();
DGNPLATFORM_EXPORT bool ElementIsUndisplayed (GeometricElementCR);

bool WantShowDefaultFieldBackground ();

DGNPLATFORM_EXPORT void    CacheQvGeometryTexture (uint32_t rendMatID);

//__PUBLISH_SECTION_START__
//@}

/// @name Methods to Retrieve Related Interfaces from a ViewContext
//@{

//! Get the IViewDraw interface for this ViewContext. Usually, but not always, this will be the IViewDraw from the viewport to which this
//! context is attached.
//! @return   the IViewDraw for this context
DGNPLATFORM_EXPORT IViewDrawR GetIViewDraw ();

//! Get the IDrawGeom interface for this ViewContext. Applications should use this method to draw geometry in Draw methods.
//! @return   the IDrawGeom for this context
DGNPLATFORM_EXPORT IDrawGeomR GetIDrawGeom ();

/** @cond BENTLEY_SDK_Scope1 */
//! Get the ICachedDraw interface for this ViewContext.
//! @return   the ICachedDraw for this context.
DGNPLATFORM_EXPORT ICachedDrawP GetICachedDraw();

//! Check whether we are creating a cached presentation.
//! @return   true if we're in the process of creating a cache presentation.
DGNPLATFORM_EXPORT bool CheckICachedDraw ();
/** @endcond */

//! Get the IPickGeom interface for this ViewContext. Only contexts that are specific to picking will return a non-NULL value.
//! @return the IPickGeom interface for this context. May return NULL.
DGNPLATFORM_EXPORT IPickGeomP GetIPickGeom ();

//@}

/// @name Identifying element "topology".
//@{
//! Query the current IElementTopology.
//! @return An object that holds additional information about the graphics that are currently being drawn.
DGNPLATFORM_EXPORT IElemTopologyCP GetElemTopology () const;

//! Set the current IElementTopology.
//! @param topo An object holding additional information about the graphics to be drawn or nullptr to clear the current topology pointer.
DGNPLATFORM_EXPORT void SetElemTopology (IElemTopologyP topo);

//! Query the current DgnGeomPartId. Only valid when drawing instanced geometry.
DGNPLATFORM_EXPORT DgnGeomPartId GetDgnGeomPartId ();

//! Set the current DgnGeomPartId. Only valid when drawing instanced geometry.
DGNPLATFORM_EXPORT void SetDgnGeomPartId (DgnGeomPartId partId);

//@}

//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT bool WantAreaPatterns ();
DGNPLATFORM_EXPORT void DrawAreaPattern (ClipStencil& boundary);
//__PUBLISH_SECTION_START__

/** @name Draw Geometry Using Current Linestyle */
/** @{ */

//! Draw a 2D linestring using the current Linestyle, if any. If there is no current Linestyle, draw a solid linestring.
//! @param[in]      nPts        Number of vertices in \c pts.
//! @param[in]      pts         Array of points in linestring.
//! @param[in]      zDepth      Display priority for all vertices.
//! @param[in]      range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                 optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
//! @param[in]      closed      Do point represent a shape or linestring.
DGNPLATFORM_EXPORT void DrawStyledLineString2d (int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false);

//! Draw a 3D linestring using the current Linestyle, if any. If there is no current Linestyle, draw a solid linestring.
//! @param[in]      nPts        Number of vertices in \c pts.
//! @param[in]      pts         Array of points in linestring
//! @param[in]      range       Array of 2 points with the range (min followed by max) of the vertices in \c points. This argument is
//!                                 optional and is only used to speed processing. If you do not already have the range of your points, pass NULL.
//! @param[in]      closed      Do point represent a shape or linestring.
DGNPLATFORM_EXPORT void DrawStyledLineString3d (int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed = false);

//! Draw a 2D elliptical arc using the current Linestyle. If there is no current Linestyle, draw a solid arc.
//! @param[in]      ellipse     The arc data.
//! @param[in]      isEllipse   Treat full sweep as ellipse not arc.
//! @param[in]      zDepth      Z depth value.
//! @param[in]      range       Array of 2 points with the range (min followed by max) of the arc. This argument is
//!                               optional and is only used to speed processing. If you do not already have the range, pass NULL.
DGNPLATFORM_EXPORT void DrawStyledArc2d (DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range);

//! Draw a 3D elliptical arc using the current Linestyle. If there is no current Linestyle, draw a solid arc.
//! @param[in]      ellipse     The arc data.
//! @param[in]      isEllipse   Treat full sweep as ellipse not arc.
//! @param[in]      range       Array of 2 points with the range (min followed by max) of the arc. This argument is
//!                               optional and is only used to speed processing. If you do not already have the range, pass NULL.
DGNPLATFORM_EXPORT void DrawStyledArc3d (DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range);

//! Draw a 2d BSpline curve using the current Linestyle. If there is no current Linestyle, draw a solid BSpline.
//! @param        curve       bspline curve parameters
//! @param[in]    zDepth      Z depth value.
DGNPLATFORM_EXPORT void DrawStyledBSplineCurve2d (MSBsplineCurveCR curve, double zDepth);

//! Draw a BSpline curve using the current Linestyle. If there is no current Linestyle, draw a solid BSpline.
//! @param        curve       bspline curve parameters
DGNPLATFORM_EXPORT void DrawStyledBSplineCurve3d (MSBsplineCurveCR curve);

//! Draw a curve vector using the current Linestyle. If there is no current Linestyle, draw a solid curve vector.
//! @param        curve       curve geometry
DGNPLATFORM_EXPORT void DrawStyledCurveVector3d (CurveVectorCR curve);

//! Draw a 2d curve vector using the current Linestyle. If there is no current Linestyle, draw a solid curve vector.
//! @param        curve       curve geometry
//! @param[in]    zDepth      Z depth value.
DGNPLATFORM_EXPORT void DrawStyledCurveVector2d (CurveVectorCR curve, double zDepth);

//! Draw an instance of a DisplaySymbol given a DisplaySymbol definition (an IDisplaySymbol). DisplaySymbol definitions are generally cached globally,
//! so the first call to this method for a given symbol definition will create the cached representation, and all subsequent calls will draw
//! instances using that cached representation.
//! @param[in]      symbolDef        Symbol definition to draw from.
//! @param[in]      trans            Transform to be applied to the symbol definition to determine location, orientation, size of this instance.
//! @param[in]      clip             ClipPlaneSet to be applied to symbol. May be NULL.
//! @param[in]      ignoreColor      If true, ignore the colors in the symbol definition and use the current color from \c context.
//! @param[in]      ignoreWeight     If true, ignore line weights in the symbol definition, and use the current line weight from \c context.
DGNPLATFORM_EXPORT void DrawSymbol (IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight);

//! Draw a text string and any adornments such as background shape, underline, overline, etc. Sets up current ElemDisplayParams for TextString symbology.
DGNPLATFORM_EXPORT void DrawTextString (TextStringCR);

//! Draw geometry by either using a previously cached representation if it has already been created, or by
//! calling its stroke method if the cached representation does not yet exist.
//! <p>Any displayable that wishes to cache any or all of its output should call this method.
//! <p>It first checks to see whether the appropriate cached representation was previously generated, and if so it simply draws
//! that cached representation. If not, it creates an caching context and then calls the appropriate stroke methods
//! to create a cache representation using the caching context.
//! @param[in] stroker An object to use to create cache representation (if necessary).
//! @note A single displayable may have many saved cached representations. Draw methods can decide which cached representation is appropriate.
//! in the current context, and can even draw more than one of the cached representations by having the stroker return different cache indices.
DGNPLATFORM_EXPORT QvElem* DrawCached (IStrokeForCache& stroker);

DGNPLATFORM_EXPORT bool CheckStop ();

};

/** @endGroup */

//__PUBLISH_SECTION_END__
//=======================================================================================
//! Encapsulates all possible factors that could affect the display of an annotation element.
//! @bsiclass                                                     Sam.Wilson      03/2008
//=======================================================================================
struct          AnnotationDisplayParameters
{
private:
    double      elementScale;    //!< The element's "native" scale. This is normally applied by transforming the element when it is placed or when the annotation scale of its host model changes.
    double      desiredScale;    //!< Active Annotation Scale to be applied to annotation elements in references.
    Transform   aspectRatioSkew; //!< Non-uniform scaling that will be applied to the view.

public:
    void Init ();
    void Init (double e, double r, double d, Transform const& a);

    DGNPLATFORM_EXPORT void SwapScales ();

    //! The element's current annotation scale
    double GetElementNativeScale () const {return elementScale;}

    //! The annotation scale that should be used when drawing in the current context.
    double GetDesiredScale () const {return desiredScale;}

    //! The factor by which the annotation element should be rescaled.
    double GetRescaleFactor () const {return GetDesiredScale() / GetElementNativeScale();}

}; // AnnotationDisplayParameters

/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct SymbolContext : public ViewContext
{
    DEFINE_T_SUPER(ViewContext)

private:
    ViewContextR    m_seedContext;

protected:
    DGNPLATFORM_EXPORT ViewContextR GetSeedContext() { return m_seedContext; }

    DGNPLATFORM_EXPORT virtual void _SetupOutputs() override;
    DGNPLATFORM_EXPORT virtual void _Detach() override;

    virtual int     _GetViewNumber() const override { return -1; }
    virtual void    _AllocateScanCriteria() override {}
    virtual bool    _CheckFillOutline () override { return m_seedContext.CheckFillOutline (); }

public:
    DGNPLATFORM_EXPORT SymbolContext (ViewContextR seedContext);

    DGNPLATFORM_EXPORT virtual ~SymbolContext () { _Detach (); }
    DGNPLATFORM_EXPORT virtual QvElemP DrawSymbolForCache (IDisplaySymbol* symbol, QvCacheR symbolCache);

}; // SymbolContext

/*__PUBLISH_SECTION_START__*/

END_BENTLEY_DGNPLATFORM_NAMESPACE
