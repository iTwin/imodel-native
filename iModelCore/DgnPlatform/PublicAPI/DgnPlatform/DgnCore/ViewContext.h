/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ViewContext.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "../DgnPlatform.h"
#include "ElementHandle.h"
#include "ClipVector.h"
#include "TransformClipStack.h"
#include <stack>
#include <Bentley/bvector.h>
#include "IViewDraw.h"
#include <ECObjects/ECExpressions.h>

//__PUBLISH_SECTION_END__
#include "DisplayPath.h"
#include "ScanCriteria.h"
#include "Material.h"
#include "DisplayStyle.h"
#include "DisplayStyleHandler.h"
#include "DisplayFilterManager.h"
#include "ViewDisplayRules.h"
#include "XGraphicsCache.h"

struct XGraphicsContainerVector;

#define FOCAL_LENGTH_RATIO 0.023584905

typedef uintptr_t QvExtSymbID;

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct  IDrawRasterAttachment;

/*=================================================================================**//**
 @addtogroup ViewContext
 A ViewContext holds the <i>current state</i> of an operation on a Viewport. A ViewContext must be first
 \e attached to a Viewport to be useful, and must be \e detached from the Viewport to free any memory associated with its internal state.

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
 Application programmers typically encounter a ViewContext by having it passed to their DisplayHandler::_Draw method.

 <h2>Display Parameters</h2>
 A ViewContext always has a \e current set of Display Parameters that affect the appearance of geometry drawn to the Viewport.

 The process of drawing an element starts by setting the current the ElemDisplayParams (see ViewContext::GetCurrentDisplayParams)
 from the information contained in the element's header and display linkages. Then, all By-Cell and By-Level symbology is applied.
 Then the current ElemDisplayParams is "cooked" (see discussion at ElemMatSymb) into the current material/symbology (see ViewContext::GetCurrMatSymb).
 That forms the "natural" ElemMatSymb for the element.

 However, contexts often override the natural symbology to display the element
 in a specific manner (e.g. hiliting, transparency, viewflags, etc.) via the <i>Override Symbology</i> (see ViewContext::GetOverrideMatSymb).
 Any portion of the ElemMatSymb that is not specified in the Override Symbology comes from the natural symbology. Application programmers typically
 don't need to do anything to set up their natural symbology, since the defaults are established before IDisplayHandler::Draw is called. Also,
 applications typically don't need to worry about context specific overrides - they just modify your element's natural symbology where appropriate.

 @note At any given time, a given Viewport may have more than one attached ViewContext's (although only one may be drawing
 to the Viewport at a time). Also, a single ViewContext can be attached/detached to one Viewport and then reattached to a  different Viewport throughout its lifetime.                                                                                                                                                                               shared

+===============+===============+===============+===============+===============+======*/

//__PUBLISH_SECTION_START__

/** @beginGroup */

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   03/14
//=======================================================================================
struct Frustum
{
    DPoint3d m_pts[8];
    DPoint3dCP GetPts() const {return m_pts;}
    DPoint3dP GetPtsP() {return m_pts;}
    DPoint3dCR GetCorner(int i) const {return *(m_pts+i);}
    DPoint3dR GetCornerR(int i) {return *(m_pts+i);}
    DPoint3d GetCenter() const {DPoint3d center; center.Interpolate(m_pts[NPC_111], 0.5, m_pts[NPC_000]); return center;}
    void Multiply(TransformCR trans) {trans.Multiply(m_pts, m_pts, 8);}
    void Translate(DVec3dCR offset) {for (auto& pt : m_pts) pt.Add(offset);}
    Frustum TransformBy(TransformCR trans) {Frustum out; trans.Multiply(out.m_pts, m_pts, 8); return out;}
    DRange3d ToRange() const {DRange3d range; range.InitFrom(m_pts, 8); return range;}
    void Invalidate() {memset(this, 0, sizeof(*this));}
    bool operator==(Frustum const& rhs) const {return 0==memcmp(m_pts, rhs.m_pts, sizeof(*this));}
    bool operator!=(Frustum const& rhs) const {return !(*this == rhs);}
};

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
//__PUBLISH_SECTION_END__

//=======================================================================================
// @bsiclass                                                    John.Gooding    04/14
//=======================================================================================
struct CachedDrawHandle
    {
private:
    bool            m_is3d;
    bool            m_isGraphics;   //  why did we have to test for this with el->hdr.IsGraphic()?
    bool            m_isPersistent; //  does it make sense to save the QvElem?
    ElementHandleCP m_elementHandle;
    XGraphicsSymbolStampCP m_symbolStamp;

public:
    bool IsValid() const { return m_symbolStamp != NULL || (NULL != m_elementHandle && m_elementHandle->GetElementRef() != NULL); }
    DGNPLATFORM_EXPORT explicit CachedDrawHandle(ElementHandleCP elementHandle);
    explicit CachedDrawHandle(XGraphicsSymbolStampCR symbolStamp);
    bool IsGraphics () const { return m_isGraphics; }
    bool Is3d () const { return m_is3d; }
    bool IsPersistent () const  { return m_isPersistent; }
    DgnModelP GetDgnModelP () const { return NULL != m_elementHandle ? m_elementHandle->GetDgnModelP() : NULL; }
    ElementRefP GetElementRef () const { return NULL != m_elementHandle ? m_elementHandle->GetElementRef() : NULL; }
    ElementHandleCP GetElementHandleCP() const { return m_elementHandle; }
    XGraphicsSymbolStampCP GetSymbolStampCP() const { return m_symbolStamp; }
    DgnProjectCP GetDgnProjectCP() const;
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

    Int32           m_styleIndex;       //!< Display style of the clip volume area. -1 to match that view.

    Int32           GetDisplayStyleIndex () const                      { return m_styleIndex; }
    void            SetDisplayStyleIndex (Int32 index)                 { m_styleIndex = index; }

    bool            IsEqual (const ClipVolumeOverrides& other) const
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
    unsigned                m_reflected:1;
    unsigned                m_ignoreBoundaryClipping:1;  // if true, side crop handles are ignored and the cut plane is considered infinite 
    unsigned                m_ignoreBoundaryCutPlanes:1; // if true, section graphics are not generated along side planes
    unsigned                m_unused:29;
}; // ClipVolumeFlags

/*=================================================================================**//**
* This class has a collection of settings used by dynamic view to apply its clip volume.
*              These settings include display styles for clip regions like forward, back, inside and outside.
* @remark This Class cannot be instantiated.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  DynamicViewSettings
{
public:
    ClipVolumeFlags             m_flags;
    ClipVolumeOverrides         m_forward;
    ClipVolumeOverrides         m_backward;
    ClipVolumeOverrides         m_cut;
    ClipVolumeOverrides         m_outside;

private:
    bool                        m_fromParent;
    ElementId                   m_clipBoundElementId;       // clip boundary element.
    ElementId                   m_clipMaskElementId;        // clip mask element.
    double                      m_levelOfDetail;

public:
    bool                            ShouldDisplayForward () const { return 0 != m_forward.m_flags.m_display;  }
    bool                            ShouldDisplayBackward () const { return 0 != m_backward.m_flags.m_display; }
    bool                            ShouldDisplayCut () const { return 0 != m_cut.m_flags.m_display;      }
    bool                            ShouldDisplayOutside () const { return 0 != m_outside.m_flags.m_display;  }
    bool                            ShouldDisplayForwardBackwardCut() const;
    bool                            ShouldReflectBackward () const { return 0 != m_backward.m_flags.m_reflected; }
    bool                            ShouldIgnoreBoundaryClipping () const  { return 0 != m_flags.m_ignoreBoundaryClipping; }
    bool                            ShouldIgnoreBoundaryCutPlanes () const  { return 0 != m_flags.m_ignoreBoundaryCutPlanes; }
    
    DynamicViewSettings ();
    void                             Init ();

}; // DynamicViewSettings

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
//! MicroStation uses DisplaySymbols in linestyles and for area patterns.
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
struct          IRangeNodeCheck
{
    virtual ScanTestResult _CheckNodeRange (ScanCriteriaCR, DRange3dCR, bool is3d, bool isElement) = 0;
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

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
// @bsiclass                                                   
//=======================================================================================
struct CookedDisplayStyle
{
    struct CookedDisplayStyleFlags
        {
        unsigned    m_stylePresent                      :1;
        unsigned    m_perElementOverridesPresent        :1;
        unsigned    m_elementColor                      :1;
        unsigned    m_lineStyle                         :1;
        unsigned    m_lineWeight                        :1;
        unsigned    m_transparency                      :1;
        unsigned    m_edgeColor                         :1;
        unsigned    m_material                          :1;
        unsigned    m_applyMonochromeOverrides          :1;
        unsigned    m_legacyDrawOrder                   :1;
        unsigned    m_applyEdgeStyleToLines             :1;
        unsigned    m_generateHiddenEdgeMask            :1;
        unsigned    m_useHiddenEdgeMask                 :1;
        unsigned    m_inhibitClipEdgeDisplay            :1;
        unsigned    m_visibleEdgeLineStyle              :1;
        unsigned    m_visibleEdgeLineWeight             :1;
        unsigned    m_hiddenEdgeLineStyle               :1;
        unsigned    m_hiddenEdgeLineWeightZero          :1;
        unsigned    m_hLineTransparency                 :1;
        unsigned    m_hLineMaterialColors               :1;
        unsigned    m_smoothIgnoreLights                :1;
        unsigned    m_useDisplayHandler                 :1;
        unsigned    m_invisibleToCamera                 :1;
        unsigned    m_displayEnvironment                :1;
        unsigned    m_ignoreGeometryMaps                :1;
        unsigned    m_displayGroundPlane                :1;
        unsigned    m_unused                            :6;

        CookedDisplayStyleFlags ();
        } m_flags;

    enum
        {
        HiddenEdgeWidth_SameAsVisible = 0xffff
        };


    int             m_elementColorIndex;
    UInt32          m_elementColor;
    UInt32          m_linePattern;
    UInt32          m_lineWidth;
    UInt32          m_transparency;
    UInt32          m_edgeColorIndex;
    UInt32          m_edgeColor;
    UInt32          m_visibleEdgeWidth;
    UInt32          m_hiddenEdgeWidth;    
    MaterialCP      m_material;
    double          m_hLineTransparencyThreshold;

    ViewFlags          m_viewFlags;
    EdgeMaskState      m_edgeMaskState;
    Int32              m_lineStyleIndex;
    WString            m_environmentName;
    EnvironmentDisplay m_environmentTypeDisplayed; 
    RgbFactor          m_groundPlaneColor;
    double             m_groundPlaneHeight;
    double             m_groundPlaneTransparency;
    bool               m_groundPlaneShowGroundFromBelow;

    DisplayStyleHandlerCP                                   m_displayHandler;
    RefCountedPtr<struct DisplayStyleHandlerSettings>       m_displayHandlerSettings;

    void             Init (CookedDisplayStyle const& donor);
    ViewFlagsCP      GetViewFlags () const { return &m_viewFlags; }
    bool             StylePresent () const { return m_flags.m_stylePresent; }
    bool             PerElementOverridesPresent () const { return m_flags.m_stylePresent && m_flags.m_perElementOverridesPresent; }
    DGNPLATFORM_EXPORT   CookedDisplayStyle (DisplayStyleCP style, DgnModelP modelRef, ViewContextP context, CookedDisplayStyleCP parent, bool inheritFromParent);
    DGNPLATFORM_EXPORT   CookedDisplayStyle (CookedDisplayStyleCR source);
    DGNPLATFORM_EXPORT   CookedDisplayStyle (ViewFlagsCR flags, CookedDisplayStyleCP parent);
    WStringCR        GetEnvironmentName () const { return m_environmentName; }
    void             OnFrustumChange (DgnModelR modelRef, ViewContextR viewContext) const;
    bool             HiddenEdgeWidthSameAsVisible () { return HiddenEdgeWidth_SameAsVisible == m_hiddenEdgeWidth; }

}; // CookedDisplayStyle
#endif

/*=================================================================================**//**
* @bsiclass  
+===============+===============+===============+===============+===============+======*/
struct QvUnsizedKey
{
private:
#if defined (NEEDS_WORK_DGNITEM)
    DisplayStyleHandlerKeyPtr           m_handlerKey;
#endif
    DisplayFilterKeyPtr                 m_displayFilterKey;
    UInt32                              m_transformKey;
    Int32                               m_qvIndex;

public:
            bool        IsNull () const;
            bool        Matches (QvUnsizedKey const& other) const;
                        QvUnsizedKey (UInt32 transformKey, UInt32 qvIndex, DisplayFilterKeyP displayFilterKey) : m_transformKey (transformKey), m_qvIndex (qvIndex), m_displayFilterKey (displayFilterKey) {}
            bool        MatchesConditionalDrawState (ViewContextR viewContext, ElementHandleCP element) const;
            void        SetDisplayFilterKey(DisplayFilterKeyP key) { m_displayFilterKey = key; }
    inline  bool        OwnsQvElem() { return m_qvIndex >= 0; }
DisplayFilterKeyPtr     GetDisplayFilterKey() { return m_displayFilterKey; }
}; // QvUnsizedKey

//__PUBLISH_SECTION_END__

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

//__PUBLISH_SECTION_START__

//=======================================================================================
//! @bsiclass                                                     KeithBentley    04/01
//=======================================================================================
struct ViewContext
//__PUBLISH_SECTION_END__
    : ICheckStop, IRangeNodeCheck
//__PUBLISH_SECTION_START__
{

//__PUBLISH_SECTION_END__
    friend struct ViewController;
    friend struct SimplifyViewDrawGeom;

public:
    enum VisitPathStatus
        {
        VISIT_PATH_Filtered = 1,
        };

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
            Int8          m_contrast;                        // -128...+127, 0 -> no contrast.
            Int8          m_brightness;                      // -128...+127, 0 -> no brightness.
            bool          m_greyScale;                       // Output raster in greyscale.
            bool          m_applyBinaryWhiteOnWhiteReversal; // Apply color inversion logic on monochrome. (Prevent to display/print a white foreground on a white media)
            bool          m_enableGrid;                      // Superimpose a grid over the raster.
            RgbColorDef   m_backgroundColor;                 // Background color for binary image.
            RgbColorDef   m_foregroundColor;                 // Foreground color for binary image.
            double        m_quality;
            UInt32        m_flags;

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

            UInt32 GetFlags() const { return m_flags; }
            Int8 GetContrast() const { return m_contrast; }
            Int8 GetBrightness() const { return m_brightness; }
            bool GetGreyscale() const { return m_greyScale; }
            bool GetApplyBinaryWhiteOnWhiteReversal() const { return m_applyBinaryWhiteOnWhiteReversal; }
            bool GetEnableGrid() const { return m_enableGrid; }
            RgbColorDefCR GetBackgroundColor() const { return m_backgroundColor; }
            RgbColorDefCR GetForegroundColor() const { return m_foregroundColor; }
            double GetQualityFactor() const { return m_quality; }

            DGNPLATFORM_EXPORT void SetFlags(UInt32 flags);
            DGNPLATFORM_EXPORT void SetContrast(Int8 value);
            DGNPLATFORM_EXPORT void SetBrightness(Int8 value);
            DGNPLATFORM_EXPORT void SetGreyscale(bool value);
            DGNPLATFORM_EXPORT void SetApplyBinaryWhiteOnWhiteReversal(bool value);
            DGNPLATFORM_EXPORT void SetEnableGrid(bool value);
            DGNPLATFORM_EXPORT void SetBackgroundColor(RgbColorDefCR value);
            DGNPLATFORM_EXPORT void SetForegroundColor(RgbColorDefCR value);
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
        DGNPLATFORM_EXPORT ContextMark (ViewContextR context, ElementHandleCR testForRange);
        DGNPLATFORM_EXPORT ~ContextMark ();

        DGNPLATFORM_EXPORT void Pop ();
        DGNPLATFORM_EXPORT void SetNow ();
        void Init (ViewContextP context) { m_hdrOvrMark = m_transClipMark = 0; m_context = context; m_parentRangeResult = RangeResult::Overlap; m_pushedRange = m_ignoreScaleForDimensions = m_ignoreScaleForMultilines = m_applyRotationToDimView = false; m_reservedDouble = 0.0;}
        };

    friend struct ContextMark;

    typedef bvector<ElemHeaderOverrides> HeaderOvrArray;

/*__PUBLISH_SECTION_START__*/
    //=======================================================================================
    //! @bsiclass                                                     Brien.Bastings  11/07
    //=======================================================================================
    struct  ClipStencil
        {
/*__PUBLISH_SECTION_END__*/
    private:
        IStrokeForCache&    m_stroker;
        Int32               m_qvIndex;
        bool                m_saveQvElem;
        QvElem*             m_qvElem;
        CurveVectorPtr      m_curveVector;

    public:
        DGNPLATFORM_EXPORT QvElem*           GetQvElem (ElementHandleCR thisElm, ViewContextR context, bool& deleteQvElem);
        DGNPLATFORM_EXPORT CurveVectorPtr    GetCurveVector (ElementHandleCR thisElm);
        DGNPLATFORM_EXPORT IStrokeForCache&  GetStroker ();

/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
        DGNPLATFORM_EXPORT explicit ClipStencil (IStrokeForCache& stroker, Int32 qvIndex, bool saveQvElem = true, QvElem* qvElem = NULL);
        };

    //=======================================================================================
    //! @bsiclass                                                     Brien.Bastings  11/07
    //=======================================================================================
    struct  PatternParamSource
        {
/*__PUBLISH_SECTION_END__*/
    private:
        int                 m_patternIndex;
        DPoint3d            m_origin;
        PatternParamsP      m_paramsP;
        DwgHatchDefLineP    m_hatchLinesP;
        bool                m_freeData;

    public:
        DGNPLATFORM_EXPORT ~PatternParamSource ();
        DGNPLATFORM_EXPORT PatternParamsP GetParams (ElementHandleCR thisElm, DPoint3dP origin, DwgHatchDefLineP* hatchLines, int* index, ViewContextP viewContext);

/*__PUBLISH_CLASS_VIRTUAL__*/
/*__PUBLISH_SECTION_START__*/
        DGNPLATFORM_EXPORT explicit PatternParamSource (int patternIndex = 0);
        DGNPLATFORM_EXPORT explicit PatternParamSource (PatternParamsP params, DwgHatchDefLineP hatchLines, int patternIndex = 0);
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

/*__PUBLISH_SECTION_END__*/

   //=======================================================================================
    //! @bsiclass                                                     Ray.Bentley     10/12
    //=======================================================================================
    struct  ConditionalDrawState 
        {
        bool                m_state;
        size_t              m_drawMethodIndex;
        size_t              m_conditionalBlockIndex;

        ConditionalDrawState (bool state, size_t drawIndex, size_t blockIndex) : m_state (state), m_drawMethodIndex (drawIndex), m_conditionalBlockIndex (blockIndex) { }
        };

private:
    static IDrawRasterAttachment* s_pRasterAttInterface;

    ViewContext& operator= (ViewContext const& from) {BeAssert (false); return *this;} // NOT SUPPORTED!

protected:
    bool                    m_isAttached;
    bool                    m_blockAsyncs;
    bool                    m_blockIntermediatePaints;
    bool                    m_is3dView;
    bool                    m_isCameraOn;
    bool                    m_wantMaterials;
    bool                    m_ignoreOpenElements;       // Would be nice to remove this...just confuses people...
    bool                    m_creatingCacheElem;
    bool                    m_useNpcSubRange;
    bool                    m_ignoreScaleForDimensions;
    bool                    m_ignoreScaleForMultilines;
    bool                    m_applyRotationToDimView;
    bool                    m_useCachedGraphics;
    bool                    m_ignoreViewRange;
    byte                    m_filterLOD;
    size_t                  m_refTransClipDepth;
    size_t                  m_frustumTransClipDepth;   // stack depth when starting a frustum change
    DrawPurpose             m_purpose;
    DRange3d                m_npcSubRange;
    DMap4d                  m_frustumToNpc;
    DMap4d                  m_frustumToView;
    double                  m_cameraFraction;           // ratio of front plane to back plane.
    TransformClipStack      m_transformClipStack;
    HeaderOvrArray          m_headerOvr;
    ViewportP               m_viewport;
    IViewDrawP              m_IViewDraw;
    IDrawGeomP              m_IDrawGeom;
    ICachedDrawP            m_ICachedDraw;
    DgnProjectP             m_dgnProject;
    ElementRefPtr           m_currentElement;
    DisplayPathCP           m_sourcePath;
    ScanCriteriaP           m_scanCriteria;
    Int32                   m_displayPriorityRange[2];
    ElemDisplayParams       m_currDisplayParams;
    ElemMatSymb             m_elemMatSymb;
    OvrMatSymb              m_ovrMatSymb;
    LevelClassMask          m_levelClassMask;
    RangeResult             m_parentRangeResult;
    double                  m_minLOD;             // minimum size of default level-of-detail test.
    double                  m_arcTolerance;
    double                  m_patternScale;
    DPoint3dCP              m_startTangent;       // linestyle start tangent.
    DPoint3dCP              m_endTangent;         // linestyle end tangent.
    UInt32                  m_rasterPlane;        // Current displayed raster plane
    bool                    m_drawingClipElements;
    CookedDisplayStyleCP    m_currentDisplayStyle;
    UInt32                  m_displayStyleStackMark;
    EdgeMaskState           m_edgeMaskState;
    ElementHiliteState      m_hiliteState;
    RasterDisplayParams     m_rasterDisplayParams;
    IElemTopologyP          m_currElemTopo;

    bvector <ConditionalDrawState>      m_conditionalDrawStates;
    size_t                              m_conditionalBlockIndex;
    DisplayFilterKeyPtr                 m_displayFilterKey;
    RefCountedPtr<RefCountedBase>       m_conditionalDrawExpressionContext;
    WString                             m_presentationFormId;
    bset <WString>                      m_presentationFormFlags;

    ElemDisplayParamsIgnores            m_ignores;
    bool                                m_scanRangeValid;
    double                              m_levelOfDetail;

    ElementSymbologyExpressionContextPtr    m_displayRuleContext;
    SymbologyRulesPtr                       m_symbologyRules;

    DGNPLATFORM_EXPORT void SetLevelBitMask (BitMaskCP newBitmask) {m_levelClassMask.levelBitMaskP = newBitmask;}
    DGNPLATFORM_EXPORT void PopOneTransformClip ();
    void                    InvalidateScanRange ();
    DGNPLATFORM_EXPORT void InitDisplayPriorityRange ();
    DGNPLATFORM_EXPORT void DrawElementNormal (ElementHandleCR);
    DGNPLATFORM_EXPORT bool CheckThicknessVector ();
    virtual int _GetScanReturnType () {return MSSCANCRIT_ITERATE_ELMREF;}
    DGNPLATFORM_EXPORT virtual StatusInt       _Attach (ViewportP, DrawPurpose purpose);
    DGNPLATFORM_EXPORT virtual void            _Detach ();
    DGNPLATFORM_EXPORT virtual void            _SetupOutputs () = 0;
    virtual LevelClassMask* _GetLevelClassMask () {return &m_levelClassMask;}
    DGNPLATFORM_EXPORT virtual void            _OutputElement (ElementHandleCR element);
    DGNPLATFORM_EXPORT virtual UInt32          _GetDisplayInfo (bool isRenderable);
    DGNPLATFORM_EXPORT virtual void            _DrawWithThickness (ElementHandleCR elHandle, IStrokeForCache& stroker, Int32 cacheIndex);
    DGNPLATFORM_EXPORT virtual void            _DrawCurveVector (ElementHandleCR eh, ICurvePathQueryR query, GeomRepresentations info, bool allowCachedOutline);
    DGNPLATFORM_EXPORT virtual bool            _WantAreaPatterns ();
    DGNPLATFORM_EXPORT virtual void            _DrawAreaPattern (ElementHandleCR, ClipStencil& boundary, PatternParamSource& pattern);
    DGNPLATFORM_EXPORT virtual void            _DrawSymbol (IDisplaySymbol*, TransformCP, ClipPlaneSetP, bool ignoreColor, bool ignoreWeight);
    DGNPLATFORM_EXPORT virtual void            _DeleteSymbol (IDisplaySymbol*);
    DGNPLATFORM_EXPORT virtual ILineStyleCP    _GetCurrLineStyle (LineStyleSymbP*);
    DGNPLATFORM_EXPORT virtual void            _DrawStyledLineString2d (int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed = false);
    DGNPLATFORM_EXPORT virtual void            _DrawStyledLineString3d (int nPts, DPoint3dCP pts,  DPoint3dCP range, bool closed = false);
    DGNPLATFORM_EXPORT virtual void            _DrawStyledArc2d (DEllipse3dCR, bool isEllipse, double zDepth, DPoint2dCP range);
    DGNPLATFORM_EXPORT virtual void            _DrawStyledArc3d (DEllipse3dCR, bool isEllipse, DPoint3dCP range);
    DGNPLATFORM_EXPORT virtual void            _DrawStyledBSplineCurve3d (MSBsplineCurveCR);
    DGNPLATFORM_EXPORT virtual void            _DrawStyledBSplineCurve2d (MSBsplineCurveCR, double zDepth);
    DGNPLATFORM_EXPORT virtual QvElem*         _DrawCached (CachedDrawHandleCR, IStrokeForCache&, Int32 qvIndex);
    virtual bool _CheckFillOutline () {return true;}
    DGNPLATFORM_EXPORT virtual void            _OnFrustumChange (bool is2d);
    DGNPLATFORM_EXPORT virtual StatusInt       _InitContextForView ();
    DGNPLATFORM_EXPORT virtual StatusInt       _VisitElemHandle (ElementHandleCR inEl, bool checkRange, bool checkScanCriteria);
    DGNPLATFORM_EXPORT virtual StatusInt       _VisitTransient (ElementHandleCR, SymbologyOverridesP);
    DGNPLATFORM_EXPORT virtual void            _InitScanRangeAndPolyhedron ();
    DGNPLATFORM_EXPORT virtual bool            _VisitAllModelElements (bool includeTransients);
    DGNPLATFORM_EXPORT virtual StatusInt       _VisitDgnModel (DgnModelP);
    DGNPLATFORM_EXPORT virtual void            _PushTransform (TransformCR trans);
    DGNPLATFORM_EXPORT virtual void            _PushClip (ClipVectorCR clip);
    virtual QvExtSymbID _BuildExtSymbID (UInt32 rasterWidth, int styleIndex) const { BeAssert (false); return 0; }
    DGNPLATFORM_EXPORT virtual void            _PushViewIndependentOrigin (DPoint3dCP origin);
    DGNPLATFORM_EXPORT virtual void            _PopTransformClip ();
    DGNPLATFORM_EXPORT virtual bool            _FilterRangeIntersection (ElementHandleCR);
    DGNPLATFORM_EXPORT virtual DgnModelP       _GetViewTarget();
    virtual IPickGeomP      _GetIPickGeom () {return NULL;}
    DGNPLATFORM_EXPORT virtual void            _VisitTransientElements (bool isPreUpdate);
    DGNPLATFORM_EXPORT virtual bool            _WantShowDefaultFieldBackground ();
    DGNPLATFORM_EXPORT virtual void            _AllocateScanCriteria ();
    DGNPLATFORM_EXPORT virtual void            _SetupScanCriteria ();
    virtual bool            _WantUndisplayed () {return false;}
    virtual bool            _WantUndisplayedClips () {return false;}
    DGNPLATFORM_EXPORT virtual bool            _WantSaveQvElem (DrawExpense expense);
    virtual void                _AddSymbologyFilterOverrides () {}
    DGNPLATFORM_EXPORT virtual void            _ModifyPostCookOverrides ();
    virtual void                _AddContextOverrides () {}

    DGNPLATFORM_EXPORT virtual void            _AddPreCookParentOverrides ();
    DGNPLATFORM_EXPORT virtual void            _AddPreCookViewOverrides ();
    DGNPLATFORM_EXPORT virtual void            _ModifyPreCookOverrides ();
    DGNPLATFORM_EXPORT virtual void            _CookOverrideMatSymb ();
    DGNPLATFORM_EXPORT virtual void            _CookDisplayParamsOverrides ();

    DGNPLATFORM_EXPORT virtual void            _ModifyPreCook (ElemDisplayParamsR);    
    DGNPLATFORM_EXPORT virtual void            _CookDisplayParams (ElemDisplayParamsR, ElemMatSymbR);
    DGNPLATFORM_EXPORT virtual void            _SetScanReturn ();
    DGNPLATFORM_EXPORT virtual bool            _UseCachedDisplay (CachedDrawHandleCR);

    DGNPLATFORM_EXPORT virtual void            _PushFrustumClip ();
    DGNPLATFORM_EXPORT virtual void            _InitScanCriteria ();
    DGNPLATFORM_EXPORT virtual StatusInt       _ScanDgnModel (DgnModelP model);
    DGNPLATFORM_EXPORT virtual bool            _ScanRangeFromPolyhedron ();
    DGNPLATFORM_EXPORT virtual int             _GetViewNumber () const;

    DGNPLATFORM_EXPORT virtual void            _SetDgnProject (DgnProjectR project);
    DGNPLATFORM_EXPORT virtual void            _SetCurrentElement (ElementRefP elemRef);

    DGNPLATFORM_EXPORT virtual CookedDisplayStyleCP _GetCurrentCookedDisplayStyle () const;
    DGNPLATFORM_EXPORT virtual void            _PushDisplayStyle (DisplayStyleCP style, DgnModelP modelRef, bool allowOverrideInheritance = false);
    DGNPLATFORM_EXPORT virtual void            _PopDisplayStyle ();
    DGNPLATFORM_EXPORT virtual void            _ClearZ ();
    DGNPLATFORM_EXPORT virtual ScanTestResult  _CheckNodeRange(ScanCriteriaCR, DRange3dCR, bool is3d, bool isElement);
    DGNPLATFORM_EXPORT virtual void            _DrawQvElem (QvElem*, bool is3d);
    DGNPLATFORM_EXPORT QvElem*                 GetCachedGeometry (CachedDrawHandleCR, IStrokeForCache&, Int32 qvIndex, bool& deleteQvElem, bool saveQvElem);
    DGNPLATFORM_EXPORT QvElem*                 GetQvCacheElem (CachedDrawHandleCR, Int32 qvIndex, double pixelSize);
    DGNPLATFORM_EXPORT void                    SaveQvCacheElem (CachedDrawHandleCR, Int32 qvIndex, QvElem* qvElem, double pixelSize, double sizeDependentRatio, DisplayFilterKeyP displayFilterKey = NULL);
    DGNPLATFORM_EXPORT QvElem*                 GetQvCacheElemFromEH (ElementHandleCR, Int32 qvIndex, double pixelSize);
    DGNPLATFORM_EXPORT void                    SaveQvCacheElemFromEH (ElementHandleCR, Int32 qvIndex, QvElem* qvElem, double pixelSize, double sizeDependentRatio, DisplayFilterKeyP displayFilterKey = NULL);
    DGNPLATFORM_EXPORT void                    RefreshCurrentDisplayStyle ();
    DGNPLATFORM_EXPORT virtual bool            _IfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize);
    DGNPLATFORM_EXPORT virtual bool            _ElseIfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize);
    DGNPLATFORM_EXPORT virtual bool            _ElseConditionalDraw ();
    DGNPLATFORM_EXPORT virtual void            _EndConditionalDraw ();
    DGNPLATFORM_EXPORT virtual void            _DrawAligned (DVec3dCR axis, DPoint3dCR origin, AlignmentMode type, IStrokeAligned& stroker);
    DGNPLATFORM_EXPORT virtual void            _SetLocatePriority (int priority);
    DGNPLATFORM_EXPORT virtual void            _SetNonSnappable (bool unsnappable);

    DGNPLATFORM_EXPORT virtual void             _DrawTextString (TextStringCR);
    DGNPLATFORM_EXPORT virtual void             _DrawTextBlock (TextBlockCR);

    DisplayFilterKeyPtr CacheDisplayFilterKey (DisplayFilterKeyR key, DgnModelP dgnModel);
    QvElem* GetCachedSymbolGeometry (DisplayFilterKeyPtr& filterKey, XGraphicsSymbolR symbol, Int32 qvIndex, double pixelSize, ElementHandleCP element);
    void SaveCachedSymbolGeometry (XGraphicsSymbolR symbol, Int32 qvIndex, QvElem* qvElem, double pixelSize, double sizeDependentRatio, DisplayFilterKeyR displayFilterKey, ElementRefP elementRef);

    DGNPLATFORM_EXPORT ViewContext ();
    DGNPLATFORM_EXPORT virtual ~ViewContext ();

public:
    DGNPLATFORM_EXPORT static void SetRasterAttInterface (IDrawRasterAttachment*);
    DGNPLATFORM_EXPORT static IDrawRasterAttachment* GetRasterAttInterface ();
    DGNPLATFORM_EXPORT static UInt32 GetCountQvInitCalls();
    DGNPLATFORM_EXPORT static void IncrementCountQvInitCalls ();
                       static void FreeQvElems(XGraphicsSymbolStampCR);
                       static StampQvElemMapP CreateSymbolStampMap(DgnProjectCR dgnProject);
                       static void DeleteSymbolStampMap(StampQvElemMapP symbolStampMap);
    DGNPLATFORM_EXPORT static void MergeViewFlagsFromRef (ViewFlagsR flags, ViewFlagsCR refFlags, bool retainRenderMode, bool useRefFlags);

    bool                       GetIgnoreOpenElements() const {return m_ignoreOpenElements;}
    DMap4dCR GetFrustumToView () const {return m_frustumToView;}
    DMap4dCR GetFrustumToNpc () const {return m_frustumToNpc;}
    bool GetWantMaterials () {return m_wantMaterials;};
    void                                            SetIViewDraw (IViewDrawR output)   { m_IViewDraw = &output; m_IDrawGeom = &output;}
    void                                            SetIDrawGeom (IDrawGeomR drawGeom) { m_IDrawGeom = &drawGeom; }
    bool IsAttached () {return m_isAttached;}
    bool IsRealView () {return (-1 != _GetViewNumber());}
    void SetBlockAsynchs (bool blockAsyncs) {m_blockAsyncs = blockAsyncs;}
    bool WantAsynchs () {return !m_blockAsyncs;}
    void                       SetIntermediatePaintsBlocked (bool blockIntermediatePaints) {m_blockIntermediatePaints = blockIntermediatePaints;}
    void SetRasterPlane (UInt32 plane) {m_rasterPlane = plane;}
    void ResetRasterPlane () {m_rasterPlane = RasterPlane_Any;}
    ElementHiliteState         GetCurrHiliteState () {return m_hiliteState;}
    void SetSubRectFromViewRect(BSIRectCP viewRect);
    DGNPLATFORM_EXPORT void SetSubRectNpc(DRange3dCR subRect);
    DGNPLATFORM_EXPORT bool        SetWantMaterials (bool wantMaterials);
    DGNPLATFORM_EXPORT DMatrix4d   GetLocalToView () const;
    DGNPLATFORM_EXPORT DMatrix4d   GetViewToLocal () const;
    DGNPLATFORM_EXPORT bool        ValidateScanRange ();
    DGNPLATFORM_EXPORT StatusInt   Attach (ViewportP, DrawPurpose purpose);
    DGNPLATFORM_EXPORT void        Detach ();
    DGNPLATFORM_EXPORT bool        VisitAllModelElements (bool includeTransients); // DgnModelListP includeList, bool useUpdateSequence, bool includeRefs, bool includeTransients);
    DGNPLATFORM_EXPORT bool        VisitAllViewElements (bool includeTransients, BSIRectCP updateRect); // DgnModelListP includeList, bool useUpdateSequence, bool includeRefs, bool includeTransients);
    DGNPLATFORM_EXPORT StatusInt   VisitPath (DisplayPathCP path, void* arg);
    DGNPLATFORM_EXPORT void        VisitTransientElements (bool isPreUpdate);
    DGNPLATFORM_EXPORT BentleyStatus GetCurrLocalToWorldTrans (DMatrix4dR localToWorld) const;
    DGNPLATFORM_EXPORT void        DrawBox (DPoint3dP box, bool is3d);
    DGNPLATFORM_EXPORT StatusInt   InitContextForView ();
    DGNPLATFORM_EXPORT void        OutputElement (ElementHandleCR element);
    DGNPLATFORM_EXPORT bool        IsFrustumPointVisible (DPoint3dCR frustumPoint, bool boresite);
    DGNPLATFORM_EXPORT bool        IsLocalPointVisible (DPoint3dCR localPoint, bool boresite);
    DGNPLATFORM_EXPORT bool        PointInsideClip (DPoint3dCR point);
    DGNPLATFORM_EXPORT bool        GetRayClipIntersection (double& distance, DPoint3dCR origin, DVec3dCR direction);
    DGNPLATFORM_EXPORT Frustum     GetFrustum();

    DGNPLATFORM_EXPORT StatusInt   GetRefAnnotationScale (double* scale) const;
    DGNPLATFORM_EXPORT void        ClearZ ();
                       void        SetEdgeMaskState (EdgeMaskState state) {m_edgeMaskState = state;}
    EdgeMaskState                  GetEdgeMaskState () const {return m_edgeMaskState;}

    ElemDisplayParamsIgnores&    GetDisplayParamsIgnores ();
    DisplayPathCP                GetSourcePath () const {return m_sourcePath;}

    DGNPLATFORM_EXPORT void SetNpcSubRange(DRange3dCP pSubRange);
    DGNPLATFORM_EXPORT QvCache* GetQVCache (CachedDrawHandleCR dh);

    DGNPLATFORM_EXPORT UInt32                       GetLocalTransformKey () const;
    DGNPLATFORM_EXPORT QvUnsizedKey                 GetUnsizedKey (Int32 qvIndex, DisplayFilterKeyP displayFilterKey = NULL);
    DGNPLATFORM_EXPORT TransformClipStackR          GetTransformClipStack ()    { return m_transformClipStack; }
    DGNPLATFORM_EXPORT bool&                        GetDrawingClipElements ()   { return m_drawingClipElements; }
    DGNPLATFORM_EXPORT bool                         WantUndisplayedClips ();

    DGNPLATFORM_EXPORT size_t GetTransClipDepth ();
    DGNPLATFORM_EXPORT size_t GetRefTransClipDepth ();
    DGNPLATFORM_EXPORT size_t  GetOverridesStackDepth ();
    DGNPLATFORM_EXPORT double GetArcTolerance () const;
    DGNPLATFORM_EXPORT void SetArcTolerance (double tol);
    DGNPLATFORM_EXPORT void PushOverrides (ElemHeaderOverrides* ovr);
    DGNPLATFORM_EXPORT void PopOverrides ();
    DGNPLATFORM_EXPORT ElemHeaderOverrides const* GetHeaderOvr ();
    DGNPLATFORM_EXPORT void SetLinestyleTangents (DPoint3dCP start, DPoint3dCP end);
    DGNPLATFORM_EXPORT DgnColorMapP GetColorMapForDgnModel (DgnModelP modelRef);

    //! @return   Mask of GeomRepresentations.
    DGNPLATFORM_EXPORT UInt32 GetDisplayInfo (bool isRenderable);

    DGNPLATFORM_EXPORT StatusInt VisitElemRef (ElementRefP, void* arg, bool checkScanCriteria);

    DGNPLATFORM_EXPORT QvElem* CreateCacheElem (CachedDrawHandleCR, QvCache*, IStrokeForCache&, ViewFlagsCP, double pixelSize = 0.0, ICachedDrawP cachedDrawP = NULL);

    DGNPLATFORM_EXPORT void DrawQvElem (QvElem*, TransformCP, ClipPlaneSetP, bool ignoreColor, bool ignoreWeight, bool is3d);

    DGNPLATFORM_EXPORT void DeleteSymbol (IDisplaySymbol*);

    DGNPLATFORM_EXPORT RangeResult     GetCurrParentRangeResult ();
    DGNPLATFORM_EXPORT void            SetCurrParentRangeResult (RangeResult);
    DGNPLATFORM_EXPORT bool            FilterRangeIntersection (ElementHandleCR);
    DGNPLATFORM_EXPORT double          GetMinLOD () const;
    DGNPLATFORM_EXPORT void            SetMinLOD (double);
    DGNPLATFORM_EXPORT byte&           GetFilterLODFlag ();
    DGNPLATFORM_EXPORT void            SetFilterLODFlag (FilterLODFlags);
    DGNPLATFORM_EXPORT ScanCriteriaCP  GetScanCriteria () const;
    DGNPLATFORM_EXPORT bool            GetIgnoreScaleForDimensions ();
    DGNPLATFORM_EXPORT bool            GetIgnoreScaleForMultilines ();
    DGNPLATFORM_EXPORT bool            GetApplyRotationToDimView ();
    DGNPLATFORM_EXPORT void            SetIgnoreScaleForDimensions (bool ignore);
    DGNPLATFORM_EXPORT void            SetIgnoreScaleForMultilines (bool ignore);
    DGNPLATFORM_EXPORT void            SetApplyRotationToDimView (bool apply);
    DGNPLATFORM_EXPORT UInt32          GetRasterPlane () const;
    DGNPLATFORM_EXPORT void            InitScanRangeAndPolyhedron ();
    DGNPLATFORM_EXPORT void            AllocateScanCriteria ();
    DGNPLATFORM_EXPORT bool            CheckFillOutline ();
    DGNPLATFORM_EXPORT void            VisitDgnModel (DgnModelP modelRef);
    DGNPLATFORM_EXPORT void            SetScanReturn ();
    DGNPLATFORM_EXPORT RasterDisplayParams const& GetRasterDisplayParams() const { return m_rasterDisplayParams; }

    DGNPLATFORM_EXPORT static QvCache* GetQVCache (DgnProjectR project);

    //! !!!FOR INTERNAL USE ONLY!!!
    DGNPLATFORM_EXPORT static void DirectPushTransClipOutput (IDrawGeomR, TransformCP trans, ClipPlaneSetCP clip = NULL);
    DGNPLATFORM_EXPORT static void DirectPopTransClipOutput (IDrawGeomR);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

/** @cond BENTLEY_SDK_Scope1 */
DGNPLATFORM_EXPORT StatusInt VisitElemHandle (ElementHandleCR, bool checkRange, bool checkScanCriteria);    
DGNPLATFORM_EXPORT StatusInt VisitTransient (ElementHandleCR, SymbologyOverridesP);
/** @endcond */
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

//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT void FrustumToView (DPoint3dP viewPts, DPoint3dCP frustumPt, int nPts) const;

DGNPLATFORM_EXPORT void GetViewIndTransform (TransformP trans, DPoint3dCP originLocal);
//__PUBLISH_SECTION_START__

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
//! When a ViewContext is first attached to a Viewport, the ViewFlags are initialized
//! from the Viewport's viewflags. However, during the course of an operation,
//! the viewflags for the output may be different than those on the Viewport.
//! @return   the current state of the viewflags for this ViewContext.
DGNPLATFORM_EXPORT ViewFlagsCP GetViewFlags () const;

//! Sets the current state of the ViewFlags for this context's output.
DGNPLATFORM_EXPORT void SetViewFlags (ViewFlagsCP);

//! Get the DgnProject for this ViewContext.
DGNPLATFORM_EXPORT DgnProjectR GetDgnProject () const;

//! Get the current persistent element being visited by this ViewContext.
DGNPLATFORM_EXPORT ElementRefP GetCurrentElement () const;

/** @cond BENTLEY_SDK_Scope1 */
//! Set the project for this ViewContext when not attaching a viewport.
DGNPLATFORM_EXPORT void SetDgnProject (DgnProjectR);

//! Set or clear the current persistent element.
DGNPLATFORM_EXPORT void SetCurrentElement (ElementRefP elemRef);
/** @endcond */

//! Get the source DisplayPath for this ViewContext. The "source" DisplayPath will only be non-NULL when
//! the context is being used to "re-display" an existing path. During Updates, for example, the source
//! DisplayPath will be NULL.
//! @return the DisplayPath for this ViewContext.
//! @note see discussion of DisplayPaths in the MDL documentation.
DGNPLATFORM_EXPORT DisplayPathCP GetSourceDisplayPath () const;

//! Get the DrawPurpose specified when this ViewContext was attached to the current Viewport.
//! @return the DrawPurpose specified in the call to DrawContext#Attach (drawcontext.h)
DGNPLATFORM_EXPORT DrawPurpose GetDrawPurpose () const;

//! Get the Viewport to which this ViewContext is attached. ViewContext's do not always have to be attached to an
//! Viewport, so therefore callers must always test the result of this call for NULL.
//! @return the Viewport. NULL if not attached to a Viewport.
DGNPLATFORM_EXPORT ViewportP GetViewport () const;

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
DGNPLATFORM_EXPORT bool GetDisplayPriorityRange (Int32& low, Int32& high) const;

//! Change the supplied "natural" ElemDisplayParams. Resolves effective symbology as required by the context and initializes the supplied ElemMatSymb.
//! @note Does not call ActivateMatSymb on the output or change the current ElemDisplayParams/ElemMatSymb of the context.
DGNPLATFORM_EXPORT void CookDisplayParams (ElemDisplayParamsR, ElemMatSymbR);

//__PUBLISH_SECTION_START__

DGNPLATFORM_EXPORT UInt32 GetCurrLineColor ();
DGNPLATFORM_EXPORT UInt32 GetCurrFillColor ();
DGNPLATFORM_EXPORT UInt32 GetCurrWidth ();
//! Calculate the display priority value for a given element. Display priority is based on the priority value stored in the element header,
//! the priority value of the element's level, and the priority value of the current model.
//! @return the display priority for element. For 3D views, display priority is always 0.
DGNPLATFORM_EXPORT double GetDisplayPriority () const;

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

//! Change the current ElemDisplayParams for any context overrides. Cooks the modified ElemDisplayParams into the current OvrMatSymb.
//! @note Does not call ActivateOverrideMatSymb.
DGNPLATFORM_EXPORT void CookDisplayParamsOverrides ();

//! Perform the full operation of extracting the ElemDisplayParams from an element then calling CookDisplayParams and CookDisplayParamsOverrides.
//! @param[in]      element     The element to cook and output
//! @note Does not call ActivateOverrideMatSymb.
//! @remarks Called from VisitElemHandle, rarely necessary for applications to call directly.
DGNPLATFORM_EXPORT void CookElemDisplayParams (ElementHandleCR element);

//! Change the Override ElemMatSymb for this context.
DGNPLATFORM_EXPORT void ActivateOverrideMatSymb ();
//! Do a color lookup in the current color table.
//! @param[in]      index       Color table index value between 0 and 255.
//! @return RGBA value for index from current color table.
DGNPLATFORM_EXPORT UInt32 GetIndexedColor (int index);

//! Do a line width lookup in the current lineweight to width table.
//! @param[in]      index       Line weight between 0 and 31.
//! @return    line width in pixels
DGNPLATFORM_EXPORT UInt32 GetIndexedLineWidth (int index);

//! Get the 32 bit on-off "line pattern" for a line code value for this context.
//! Output devices can change the on-off patterns based on resolution, etc.
//! @param[in]          index       a the range of 0 to 7.
//! @return             the line pattern value.
//! @bsimethod
DGNPLATFORM_EXPORT UInt32 GetIndexedLinePattern (int index);

//! Set the line color in the given material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]      elemMatSymb ElemMatSymb in which to change the line color.
//! @param[in]      index       Color table index value between 0 and 255.
DGNPLATFORM_EXPORT void SetIndexedLineColor (ElemMatSymbR elemMatSymb, int index);

//! Set the fill color in the given material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]      elemMatSymb ElemMatSymb in which to change the fill color.
//! @param[in]      index       Color table index value between 0 and 255.
DGNPLATFORM_EXPORT void SetIndexedFillColor (ElemMatSymbR elemMatSymb, int index);

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

//! Set the line color in the given override material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]      ovrMatSymb  OvrMatSymb in which to change the line color.
//! @param[in]      index       Color table index value between 0 and 255.
DGNPLATFORM_EXPORT void SetIndexedLineColor (OvrMatSymbR ovrMatSymb, int index);

//! Set the fill color in the given override material from the given color index.
//! Since the context may need to apply special symbology rules, callers should use this
//! method instead of modifying ElemMatSymb directly.
//! @param[in]      ovrMatSymb  OvrMatSymb in which to change the fill color.
//! @param[in]      index       Color table index value between 0 and 255.
DGNPLATFORM_EXPORT void SetIndexedFillColor (OvrMatSymbR ovrMatSymb, int index);

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

//! Pushes display overrides onto the context. Note that all calls to this method should
//! be matched by a call to PopDisplayOverrides. Display overrides are cooked when
//! pushed so appropriate colors, material, et. al. are used later.
//! @param[in]      overrides                   The new overrides to apply to the context; can be NULL.
//! @param[in]      modelRef                    The model ref that the overrides are applied from (e.g. for color information).
//!                                               This can be NULL if and only if overrides is also NULL.
//! @param[in]      allowOverrideInheritance    If true, will inherit parent's overrides unless overrides being pushed explicitly override
//!                                               a particular setting (e.g. if parent has red line color specified, and overrides being pushed
//!                                               do not specify an override line color, the red line color override is propagated).
//!                                               This is most useful for references, but not for clip volumes.
void PushDisplayOverrides (ViewDisplayOverridesCP overrides, DgnModelP modelRef, bool allowOverrideInheritance);

//! Pops the most recent display overrides from the context.
void PopDisplayOverrides ();

//! Gets the current cooked display overrides on this context. Can be NULL.
//! @return       the current display overrides on this context
DGNPLATFORM_EXPORT CookedDisplayStyleCP GetCurrentCookedDisplayStyle ();

//! Gets the current level of detail.
//! @return       the current level of detail.
DGNPLATFORM_EXPORT double   GetCurrentLevelOfDetail() const;


//! Sets the current level of detail.
DGNPLATFORM_EXPORT void   SetCurrentLevelOfDetail(double levelOfDetail);

//! Check the current display style for a monochrome color override.
//! @return       whether monochrome style is currently active.
DGNPLATFORM_EXPORT bool IsMonochromeDisplayStyleActive ();
DGNPLATFORM_EXPORT bool ElementIsUndisplayed (ElementHandleCR);

DGNPLATFORM_EXPORT void PushDisplayStyle (DisplayStyleCP style, DgnModelP modelRef, bool allowOverrideInheritance = false);
DGNPLATFORM_EXPORT void PopDisplayStyle ();

bool WantShowDefaultFieldBackground ();

DGNPLATFORM_EXPORT void    CacheQvGeometryTexture (UInt32 rendMatID);

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
//! Query the current IElementTopology. @note do not delete this pointer.
//! @return An object that holds additional information about the graphics that are currently being drawn.
DGNPLATFORM_EXPORT IElemTopologyP GetElemTopology ();

//! Set the current IElementTopology. @note ViewContext stores this pointer. Do not delete this pointer while the ViewContext is holding it. Call SetElemTopology(nullptr) to clear the pointer held by ViewContext.
//! @param topo  An object that holds additional information about the graphics that are currently being drawn or nullptr to clear the current topology pointer.
DGNPLATFORM_EXPORT void           SetElemTopology (IElemTopologyP topo);
//@}

//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT void DrawWithThickness (ElementHandleCR elHandle, IStrokeForCache& stroker, UInt32 cacheIndex);
DGNPLATFORM_EXPORT void DrawCurveVector (ElementHandleCR eh, ICurvePathQueryR query, GeomRepresentations info, bool allowCachedOutline); // NOTE: May use qvIndex 0, 1, 2...
DGNPLATFORM_EXPORT bool WantAreaPatterns ();
DGNPLATFORM_EXPORT void DrawAreaPattern (ElementHandleCR, ClipStencil& boundary, PatternParamSource& pattern);
DGNPLATFORM_EXPORT void DrawAreaPattern (ElementHandleCR eh, ClipStencil const& boundary, PatternParamSource const& pattern) {DrawAreaPattern (eh, const_cast<ClipStencil&>(boundary), const_cast<PatternParamSource&>(pattern));} // WIP_NONPORT - don't pass non-const reference to temporary object
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

DGNPLATFORM_EXPORT void DrawScanRange (DRange3dCR, bool is3d, UInt32 color);

DGNPLATFORM_EXPORT void DrawElementRange (DgnElementCP);

//! Draw a text string and any adornments such as background shape, underline, overline, etc. Sets up current ElemDisplayParams for TextString symbology.
DGNPLATFORM_EXPORT void DrawTextString (TextStringCR);

//! Draw a text block with all lines and formatting.
DGNPLATFORM_EXPORT void DrawTextBlock (TextBlockCR);

//! Draw an element by either using a previously cached representation if it has already been created, or by
//! calling its stroke method if the cached representation does not yet exist.
//! <p>Any displayable element that wishes to cache any or all of its output should call this method in its IDisplayHandler::Draw implementation.
//! <p>It first checks to see whether the appropriate cached representation was previously generated, and if so it simply draws
//! that cached representation. If not, it creates an caching context and then calls the appropriate stroke methods
//! to create a cache representation using the caching context.
//! @param[in]      elHandle            Element to draw.
//! @param[in]      stroker             An object to use to create cache representation (if necessary). Must not be NULL.
//! @param[in]      cacheIndex          Index of the cached representation to search/use/save. See note below.
//! @note A single displayable element may have many saved cached representations. Draw methods can decide which cached representation is appropriate.
//! in the current context, and can even draw more than one of the cached representations by calling this method with different values for \c cacheIndex.
//! @note Only persistent elements can save cached representations.
//! @see Discussion of display caching at IDisplayHandler.
DGNPLATFORM_EXPORT QvElem* DrawCached (CachedDrawHandleCR dh, IStrokeForCache& stroker, Int32 cacheIndex);
               QvElem* DrawCached (CachedDrawHandleCR dh, IStrokeForCache const& stroker, Int32 cacheIndex) {return DrawCached(dh,const_cast<IStrokeForCache&>(stroker),cacheIndex);}
DGNPLATFORM_EXPORT QvElem* DrawCached (ElementHandleCR eh, IStrokeForCache const& stroker, Int32 cacheIndex);

DGNPLATFORM_EXPORT bool CheckStop ();

// Conditional Drawing methods.
DGNPLATFORM_EXPORT bool                     IfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize);
DGNPLATFORM_EXPORT bool                     ElseIfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize);
DGNPLATFORM_EXPORT bool                     ElseConditionalDraw ();
DGNPLATFORM_EXPORT void                     EndConditionalDraw ();
DGNPLATFORM_EXPORT void                     DrawAligned (DVec3dCR axis, DPoint3dCR origin, AlignmentMode alignmentMode, IStrokeAligned& stroker);


//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT bool                     TestConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize);
DGNPLATFORM_EXPORT CompoundDrawStatePtr     GetCompoundDrawState ();
DGNPLATFORM_EXPORT bool                     InConditionalDraw ();
DGNPLATFORM_EXPORT RefCountedBase*          GetConditionalDrawExpressionContext() const;
DGNPLATFORM_EXPORT void                     SetConditionalDrawExpressionContext (RefCountedBase& exprContext);
DGNPLATFORM_EXPORT WCharCP                  GetPresentationFormId() const;
DGNPLATFORM_EXPORT void                     SetPresentationFormId (WCharCP formId);
DGNPLATFORM_EXPORT void                     SetPresentationFormFlag (WCharCP formFlag);
DGNPLATFORM_EXPORT bool                     GetPresentationFormFlag (WCharCP formFlag);
protected:
               bool                     PushConditionalDrawState (bool state);
               bool                     PopConditionalDrawState ();

//__PUBLISH_SECTION_START__
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

    DGNPLATFORM_EXPORT void RemoveAspectRatioSkew ();

    //! Has a view handler imposed aspect ratio skewing (aka y-axis exaggeration)?
    DGNPLATFORM_EXPORT bool HasAspectRatioSkew () const;

    //! Get the aspect ratio skewing effect (aka y-axis exaggeration) imposed by the view handler expressed as non-uniform scaling in local coordinate system.
    Transform const& GetAspectRatioSkew () const {return aspectRatioSkew;}

    //! The element's current annotation scale
    double GetElementNativeScale () const {return elementScale;}

    //! The annotation scale that should be used when drawing in the current context.
    double GetDesiredScale () const {return desiredScale;}

    //! The factor by which the annotation element should be rescaled.
    double GetRescaleFactor () const {return GetDesiredScale() / GetElementNativeScale();}

}; // AnnotationDisplayParameters

/*=================================================================================**//**
* This interface allows a ViewContext to ignore callouts and annotation attachments
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  IViewContextIgnoreCallouts
    {
    };

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
#ifdef WIP_VANCOUVER_MERGE // SymbolCache
    virtual void    _EmptySymbolCache() override {}
#endif

public:
    DGNPLATFORM_EXPORT SymbolContext (ViewContextR seedContext);

    DGNPLATFORM_EXPORT virtual ~SymbolContext () { _Detach (); }
    DGNPLATFORM_EXPORT virtual QvElemP DrawSymbolForCache (IDisplaySymbol* symbol, QvCacheR symbolCache);

}; // SymbolContext

/*__PUBLISH_SECTION_START__*/

//=======================================================================================
// @bsiclass                                                    MattGooding     10/13
//=======================================================================================
struct CachedGraphicsCreator
{
public:
    virtual ICachedDrawR    _GetICachedDraw() = 0;
    virtual DgnProjectR     _GetDgnProject() = 0;
    virtual bool            _GetIs3d() = 0;
    virtual void            _DrawGraphicsForCache (IDrawGeomR output) = 0;
};

//=======================================================================================
// @bsiclass                                                    MattGooding     10/13
//=======================================================================================
struct CachedGraphics : RefCountedBase
{
private:
    QvElem*         m_cachedElem;
    bool            m_is3d;

    CachedGraphics (bool is3d);
    virtual ~CachedGraphics();

public:
    static DGNPLATFORM_EXPORT CachedGraphicsPtr Create (CachedGraphicsCreatorR creator);

    DGNPLATFORM_EXPORT void Draw (IViewDrawR output, TransformCP trans, UInt32 displayPriority = 0);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
