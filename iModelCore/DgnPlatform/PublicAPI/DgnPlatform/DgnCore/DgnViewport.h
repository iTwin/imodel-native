/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnViewport.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/WString.h>
#include <Geom/IntegerTypes/BSIRect.h>
#include "IViewOutput.h"
#include "ColorUtil.h"
#include "ViewController.h"
#include <BeSQLite/RTreeMatch.h>

#if defined (DOCUMENTATION_GENERATOR)
    // WIP: hack to get docs to come out right
    namespace BentleyApi{ namespace Dgn{
#else
    BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
#endif

/**  @addtogroup DgnViewGroup

 A View is an abstract term to describe the way that applications display contents from a \ref DgnDbGroup on a device like a screen. 
 <p>There are different types of views to show different types of DgnModels in application-specific ways.
 <p>A ViewController provides persistence and behavior to a type of view.
 <p>A DgnViewport has a reference-counted-pointer to a ViewController that controls it.
 <p>A ViewContext holds the state of an operation performed on one or more elements in a DgnViewport.
 <p>A QueryModel is used to query and display graphics from PhysicalModels. 

  <h2>%DgnViewport Coordinates</h2>
  Coordinate information can be exchanged with Viewports using the various coordinate systems defined in DgnCoordSystem.
  - \c DgnCoordSystem::Screen - coordinates are relative to the upper left corner of the screen on which the viewport resides
  - \c DgnCoordSystem::View   - coordinates are relative to the upper left corner of the viewport.
  - \c DgnCoordSystem::Npc    - (<b>N</b>ormalized <b>P</b>lane <b>C</b>oordinates) the left bottom rear of the view is (0.0, 0.0, 0.0) and
                            the right top front of the view is coordinate (1.0, 1.0, 1.0)
  - \c DgnCoordSystem::World  - For PhyscialViews, the <i>world</i> coordinate system is the DgnDb coordinate system. For DrawingViews, 
                         the world coordinate system is the drawing's coordinate system.

  \b Note: ViewContext has an additional type of coordinate system called "local" since it supports pushing and popping of transformations.
           When an ViewContext is first attached to a DgnViewport, the "local" coordinate system will be the DgnViewport's world coordinate system.

  @see DgnCoordSystem
*/

//=======================================================================================
//! Parameters for the "fit view" operation
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct FitViewParams
{
    RotMatrixCP     m_rMatrix;
    DgnModelP       m_modelIfNoViewport;
    bool            m_fitRasterRefs;
    bool            m_rasterElementsOnly;
    bool            m_includeTransients;
    bool            m_useScanRange;
    bool            m_fitMinDepth;
    bool            m_fitMaxDepth;
    bool            m_return3dRangeIn2dViews;

    FitViewParams()
        {
        m_rMatrix = NULL;
        m_modelIfNoViewport = NULL;
        m_fitRasterRefs = m_rasterElementsOnly = m_includeTransients = false;
        m_useScanRange = m_fitMinDepth = m_fitMaxDepth = m_return3dRangeIn2dViews = false;
        }
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct OcclusionScorer
{
    DMatrix4d   m_localToNpc;
    DPoint3d    m_cameraPosition;
    double      m_lodFilterNPCArea;
    uint32_t    m_orthogonalProjectionIndex;
    bool        m_cameraOn;
    void InitForViewport(DgnViewportCR viewport, double minimumSizePixels);
    bool ComputeEyeSpanningRangeOcclusionScore(double* score, DPoint3dCP rangeCorners, bool doFrustumCull);
    bool ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn);
    bool ComputeOcclusionScore(double* score, bool& overlap, bool& spansEyePlane, bool& eliminatedByLOD, DPoint3dCP localCorners, bool doFrustumCull);
};

//=======================================================================================
// @bsiclass                                                    John.Gooding    10/13
//=======================================================================================
struct OverlapScorer
{
    BeSQLite::RTree3dVal m_boundingRange;
    void Initialize(DRange3dCR boundingRange);
    bool ComputeScore(double* score, BeSQLite::RTree3dValCR range);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct IProgressiveDisplay : IRefCounted
{
    enum class Completion {Finished=0, Aborted=1, Failed=2};
    virtual Completion _Process(ViewContextR) = 0;   // if this returns Finished, it is removed from the viewport
    virtual bool _WantTimeoutSet(uint32_t& limit) = 0;  // set limit and returns true to cause caller to call EnableStopAfterTimout
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct RtreeViewFilter : BeSQLite::RTreeAcceptFunction::Tester
    {
    bool                    m_doSkewtest;
    Frustum                 m_frustum;
    double                  m_minimumSizePixels;
    BeSQLite::RTree3dVal    m_boundingRange;    // only return entries whose range intersects this cube.
    BeSQLite::RTree3dVal    m_frontFaceRange;
    OcclusionScorer         m_scorer;
    uint32_t                m_nCalls;
    uint32_t                m_nScores;
    uint32_t                m_nSkipped;
    DVec3d                  m_viewVec;  // vector from front face to back face
    ClipVectorPtr           m_clips;
    DgnElementIdSet const*  m_exclude;

    bool AllPointsClippedByOnePlane(ConvexClipPlaneSetCR cps, size_t nPoints, DPoint3dCP points) const;
    void SetClipVector(ClipVectorR clip) {m_clips = &clip;}
    bool SkewTest(BeSQLite::RTree3dValCP);
    DGNPLATFORM_EXPORT RtreeViewFilter(DgnViewportCR, BeSQLite::DbR db, double minimumSizeScreenPixels, DgnElementIdSet const* exclude);
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   04/14
//=======================================================================================
struct ProgressiveViewFilter : RefCounted<IProgressiveDisplay>, RtreeViewFilter
{
    friend struct QueryViewController;

    ViewContextP                 m_context;
    uint32_t                     m_nThisPass;
    uint32_t                     m_nLastPass;
    bool                         m_drewElementThisPass;
    bool                         m_setTimeout;
    DgnModelR                    m_existing;
    DgnDbR                       m_dgndb;
    uint64_t                     m_elementReleaseTrigger;
    uint64_t                     m_purgeTrigger;
    BeSQLite::CachedStatementPtr m_rangeStmt;
    static const double          s_purgeFactor ;
    ProgressiveViewFilter(DgnViewportCR vp, DgnDbR dgndb, DgnModelR existing, DgnElementIdSet const* exclude, uint64_t maxMemory, BeSQLite::CachedStatement* stmt)
         : RtreeViewFilter(vp, dgndb, 0.0, exclude), m_dgndb(dgndb), m_existing(existing), m_elementReleaseTrigger(maxMemory), m_purgeTrigger(maxMemory), m_rangeStmt(stmt) 
        {
        m_nThisPass = m_nLastPass = 0;
        m_drewElementThisPass = m_setTimeout = false;
        m_context=NULL;
        }  
    ~ProgressiveViewFilter();

    virtual int _TestRange(QueryInfo const&) override;
    virtual void _StepRange(BeSQLite::DbFunction::Context&, int nArgs, BeSQLite::DbValue* args) override;
    virtual bool _WantTimeoutSet(uint32_t& limit) override;
    virtual Completion _Process(ViewContextR context) override;
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnDbRTree3dViewFilter : RtreeViewFilter
    {
    typedef bmultimap<double, int64_t> T_OcclusionScoreMap;

    struct SecondaryFilter
        {
        OverlapScorer       m_scorer;
        uint32_t            m_hitLimit;
        uint32_t            m_occlusionMapCount;
        double              m_occlusionMapMinimum;
        T_OcclusionScoreMap m_occlusionScoreMap;
        double              m_lastScore;
        };

    bool                    m_passedPrimaryTest;
    bool                    m_passedSecondaryTest;
    bool                    m_useSecondary;
    bool                    m_eliminatedByLOD;
    uint32_t                m_hitLimit;
    uint32_t                m_occlusionMapCount;
    int64_t                 m_lastId;
    T_OcclusionScoreMap     m_occlusionScoreMap;
    double                  m_occlusionMapMinimum;
    double                  m_lastScore;
    SecondaryFilter         m_secondaryFilter;
    ICheckStopP             m_checkStop;
    DgnElementIdSet const*  m_alwaysDraw;

    virtual int _TestRange(QueryInfo const&) override;
    virtual void _StepRange(BeSQLite::DbFunction::Context&, int nArgs, BeSQLite::DbValue* args) override {RangeAccept(args[0].GetValueInt64());}
    void RangeAccept(int64_t elementId) ;
    double MaxOcclusionScore();

public:
    DGNPLATFORM_EXPORT DgnDbRTree3dViewFilter(DgnViewportCR, ICheckStopP, BeSQLite::DbR db, uint32_t hitLimit, double minimumSizeScreenPixels, DgnElementIdSet const* alwaysDraw, DgnElementIdSet const* neverDraw);
    void SetChceckStop(ICheckStopP checkStop) {m_checkStop = checkStop;}
    void InitializeSecondaryTest(DRange3dCR volume, uint32_t hitLimit);
    void GetStats(uint32_t& nAcceptCalls, uint32_t&nScores) { nAcceptCalls = m_nCalls; nScores = m_nScores; }
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   12/11
//=======================================================================================
struct DgnDbRTreeFitFilter : BeSQLite::RTreeAcceptFunction::Tester
    {
    DRange3d m_fitRange;
    DRange3d m_lastRange;

    DGNPLATFORM_EXPORT virtual int _TestRange(QueryInfo const&) override;
    virtual void _StepRange(BeSQLite::DbFunction::Context&, int nArgs, BeSQLite::DbValue* args) override {m_fitRange.Extend(m_lastRange);}

public:
    DgnDbRTreeFitFilter(BeSQLite::DbR db) : Tester(db) {m_fitRange = DRange3d::NullRange();}
    DRange3dCR GetRange() const {return m_fitRange;}
    };

//=======================================================================================
//! Interface to draw information into a viewport while a tool is active.
// @bsiclass                                                    Keith.Bentley   06/14
//=======================================================================================
struct ToolGraphicsHandler
{
    virtual void _DrawToolGraphics(ViewContextR context, bool preUpdate) = 0;
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class ViewportResizeMode
    {
    KeepCurrent      = 0, //!< The viewport size is unchanged (this is the default). The viewport is unchanged, and the view contents are resized to match the viewport aspect ratio.
    RelativeRect     = 1, //!< The viewport is resized to the same size, relative to the available area, that is specifed in viewPortInfo
    AspectRatio      = 2, //!< The viewport is resized to match the aspect ratio of the viewInfo.
    Size             = 3, //!< The viewport is resized to match the exact size 
    };


/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   02/04
+===============+===============+===============+===============+===============+======*/
struct StopEvents
    {
    bool    m_keystrokes;
    bool    m_wheel;
    bool    m_button;
    bool    m_buttonUp;
    bool    m_paint;
    bool    m_focus;
    bool    m_modifierKeyTransition;
    bool    m_sensor;
    bool    m_abortUpdateRequest;
    bool    m_touchMotion;          //  Ignored unless the motion exceeds range.
    bool    m_anyEvent;
    uint32_t m_touchLimit;
    uint32_t m_numTouches;
    BentleyApi::Point2d m_touches[3];

    enum StopMask
        {
        None        = 0,
        OnKeystrokes  = 1<<0,
        OnWheel       = 1<<2,
        OnButton      = 1<<3,
        OnPaint       = 1<<4,
        OnFocus       = 1<<5,
        OnModifierKey = 1<<6,
        OnTouch       = 1<<7,
        OnAbortUpdate = 1<<8,
        OnSensor      = 1<<9,   //  GPS, Gyro
        OnButtonUp    = 1<<10,
        AnyEvent      = 1<<11,   //  includes all of the other events plus unknown events

        ForFullUpdate  = OnWheel | OnAbortUpdate,             // doesn't stop on keystrokes, buttons, or touch
        ForQuickUpdate = ForFullUpdate | OnKeystrokes | OnButton | OnTouch,
        };

    void Clear()
        {
        m_keystrokes = m_wheel = m_button = m_paint = m_focus = m_modifierKeyTransition = m_abortUpdateRequest = m_touchMotion = m_anyEvent = false;
        m_touchLimit = 0;
        }

    StopEvents(int mask)
        {
        if (mask & AnyEvent)
            mask = -1;

        m_keystrokes = TO_BOOL(mask & OnKeystrokes);
        m_wheel      = TO_BOOL(mask & OnWheel);
        m_button     = TO_BOOL(mask & OnButton);
        m_buttonUp   = TO_BOOL(mask & OnButtonUp);
        m_paint      = TO_BOOL(mask & OnPaint);
        m_focus      = TO_BOOL(mask & OnFocus);
        m_sensor     = TO_BOOL(mask & OnSensor);
        m_modifierKeyTransition = TO_BOOL(mask & OnModifierKey);
        m_touchMotion = TO_BOOL(mask & OnTouch);
        m_abortUpdateRequest = TO_BOOL(mask & OnAbortUpdate);
        m_anyEvent   = TO_BOOL(mask & AnyEvent);
        m_touchLimit = 0;
        }

    void SetTouchLimit(uint32_t limit, uint32_t numTouches, Point2dCP touches);

    // Stop when the ctrl or shift key is pressed or released.
    void SetStopOnModifierKey(bool stop) {m_modifierKeyTransition = stop;}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct FullUpdateInfo
    {
private:
//__PUBLISH_SECTION_END__
    friend struct IndexedViewSet;
    friend struct UpdateContext;
//__PUBLISH_SECTION_START__
    StopEvents          m_stopEvents;
    bool                m_incremental;
    bool                m_deferShadows;
    bool                m_startEndMsg;
    bool                m_startAbortState;
    bool                m_useCachedElems;
    BSIRect const*      m_subRect;
    FenceParamsP        m_fenceParams;

public:
    FullUpdateInfo() : m_stopEvents(StopEvents::ForFullUpdate)
        {
        m_incremental       = false;
        m_deferShadows      = true;
        m_startEndMsg       = false;
        m_startAbortState   = false;
        m_useCachedElems    = true;
        m_subRect           = NULL;
        m_fenceParams       = NULL;
        }

    void SetStopEvents(StopEvents stopEvents) {m_stopEvents = stopEvents;}
    void SetIncremental(bool incremental) {m_incremental = incremental;}
    void SetDeferShadows(bool deferShadows) {m_deferShadows = deferShadows;}
    void SetStartEndMsg(bool startEndMsg) {m_startEndMsg = startEndMsg;}
    void SetStartAbortState(bool startAbortState) {m_startAbortState = startAbortState;}
    void SetUseCachedElems(bool useCachedElems) {m_useCachedElems = useCachedElems;}
    void SetSubRect(BSIRect const* subRect) {m_subRect = subRect;}
    void SetFenceParams(FenceParamsP fp) {m_fenceParams = fp;}
    bool GetIncremental() const {return m_incremental;}
    bool GetStartEndMsg() const {return m_startEndMsg;}
    void SetTouchCheckStopLimit(bool enabled, uint32_t pixels, uint32_t numberTouches, Point2dCP touches);
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/12
//=======================================================================================
struct DynamicUpdateInfo
    {
private:
//__PUBLISH_SECTION_END__
    friend struct ViewManager;
    friend struct UpdateContext;
//__PUBLISH_SECTION_START__
    StopEvents          m_stopEvents;
    bool                m_doBackingStore;
    bool                m_deferShadows;
    int                 m_maxFrameTime;
    int                 m_dynamicsStopInterval;
    int                 m_dynamicsMotionTolerance;
    int                 m_minLodDelta;
    bool                m_haveLastMotion;
    int                 m_lastTotalMotion;
    Point2d             m_lastCursorPos;

public:
    DGNVIEW_EXPORT DynamicUpdateInfo();
    Point2d& GetLastCursorPos() {return m_lastCursorPos;}
    int GetStopInterval() {return m_dynamicsStopInterval;}
    int GetMinLodDelta() {return m_minLodDelta;}
    bool GetDoBackingStore() {return m_doBackingStore;}
    StopEvents GetStopEvents() {return m_stopEvents;}
    void ClearLastMotion() {m_haveLastMotion = false; m_lastTotalMotion = 0; m_lastCursorPos.x = m_lastCursorPos.y = 0;}
    void SetStopEvents(StopEvents stopEvents) {m_stopEvents = stopEvents;}
    void SetDoBackingStore(bool doBackingStore) {m_doBackingStore = doBackingStore;}
    void SetDeferShadows(bool deferShadows) {m_deferShadows = deferShadows;}
    void SetMaxFrameTime(int maxFrameTime) {m_maxFrameTime = maxFrameTime;}
    void SetMinLODDelta(int minLodDelta) {m_minLodDelta = minLodDelta;}
    void SetDynamicsStopInterval(int dynamicsStopInterval) {m_dynamicsStopInterval = dynamicsStopInterval;}
    void SetDynamicsMotionTolerance(int dynamicsMotionTolerance) {m_dynamicsMotionTolerance = dynamicsMotionTolerance;}
    void SetTouchCheckStopLimit(bool enabled, uint32_t pixels, uint32_t numberTouches, Point2dCP touches);
    };

//=======================================================================================
/**
 A DgnViewport maps a set of DgnModels to an output device through a camera (a view frustum) and filters (e.g. categories, view flags, etc). 
 <p>
 Viewports are usually mapped to a window on a screen. But, they 
 can also be mapped to other types of output devices such as plot drivers, dialog boxes, bitmaps, etc. All output to a
 DgnViewport is through methods on its IViewDraw interface, which may be retrieved by calling GetIViewDraw.
 <p>
 When active, a DgnViewport is connected to ViewController, that determines the behavior of the view. ViewControllers supply the methods
 that control which elements are visible, and the rules for determining elements' symbology. ViewControllers may also add additional graphics
 to a view. ViewControllers store their persistent state in the properties table, and are synchronized with a DgnViewport (which may hold
 local copies of the current state). So, viewing tools that wish to change camera location or other viewing state, must call 
 DgnViewport::SynchWithViewController before the changes are visible to the user.
*/
//! @ingroup DgnViewGroup
//! @nosubgrouping
//  @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnViewport : NonCopyableClass
{
public:
    friend struct ViewManager;
    friend struct UpdateContext;
    friend struct QvOutput;
    friend struct QvOutputMT;

protected:
    bool            m_needsRefresh:1;           // screen needs to be redrawn from backing store at next opportunity
    bool            m_zClipAdjusted:1;          // were the view z clip planes adjusted due to front/back clipping off?
    bool            m_is3dView:1;               // view is of a 3d model
    bool            m_isSheetView:1;            // view is sheet
    bool            m_isCameraOn:1;             // view is 3d and the camera is turned on.
    bool            m_qvDCAssigned:1;           // whether the DC was assigned for QV
    bool            m_qvParamsSet:1;            // whether view frustum and display mode have been set
    bool            m_invertY:1;
    bool            m_frustumValid:1;
    DPoint3d        m_viewOrg;                  // view origin, potentially expanded if f/b clipping are off
    DVec3d          m_viewDelta;                // view delta, potentially expanded if f/b clipping are off
    DPoint3d        m_viewOrgUnexpanded;        // view origin (from ViewController, unexpanded for "no clip")
    DVec3d          m_viewDeltaUnexpanded;      // view delta (from ViewController, unexpanded for "no clip")
    RotMatrix       m_rotMatrix;                // rotation matrix (from ViewController)
    CameraInfo      m_camera;
    ViewFlags       m_rootViewFlags;            // view flags for root model
    ColorDef        m_backgroundColor;
    IViewOutputP    m_output;
    DMap4d          m_rootToView;
    DMap4d          m_rootToNpc;
    double          m_minLOD;                   // default level of detail filter size
    Utf8String      m_viewTitle;
    ToolGraphicsHandler* m_toolGraphicsHandler;
    ViewControllerPtr m_viewController;
    bvector<IProgressiveDisplayPtr> m_progressiveDisplay;    // progressive display of a query view and reality data.

    DGNPLATFORM_EXPORT void DestroyViewport();

    virtual void _AdjustZPlanesToModel(DPoint3dR origin, DVec3dR delta, ViewControllerCR) const = 0;
    virtual bool _IsSheetView() const {return m_isSheetView;}
    virtual bool _IsGridOn() const {return m_rootViewFlags.grid;}
    virtual DPoint3dCP _GetViewDelta() const {return &m_viewDelta;}
    virtual DPoint3dCP _GetViewOrigin() const {return &m_viewOrg;}
    virtual void _AllocateOutput() = 0;
    virtual void _CallDecorators(bool& stopFlag) {}
    virtual void _SetNeedsHeal() {m_needsRefresh = true;}
    virtual void _SetNeedsRefresh() {m_needsRefresh = true;}
    virtual ICachedDrawP _GetICachedDraw() = 0;
    virtual IViewDrawP _GetIViewDraw() {return m_output;}
    virtual IViewOutputP _GetIViewOutput() {return m_output;}
    virtual AntiAliasPref _WantAntiAliasLines() const {return AntiAliasPref::Detect;}
    virtual AntiAliasPref _WantAntiAliasText() const {return AntiAliasPref::Detect;}
    virtual void _AdjustFencePts(RotMatrixCR viewRot, DPoint3dCR oldOrg, DPoint3dCR newOrg) const {}
    virtual ColorDef _GetHiliteColor() const {return ColorDef::Magenta();}
    virtual void _SynchViewTitle() {}
    virtual void _DrawStandardGrid(DPoint3dR gridOrigin, RotMatrixR rMatrix, Point2dCP fixedRepetitions) {}
    virtual BSIRect _GetClientRect() const = 0;
    virtual Point2d _GetScreenOrigin() const  {Point2d pt={0,0}; return pt;}
    virtual DVec2d _GetDpiScale() const {return DVec2d::From(1,1);}
    virtual void _Destroy() {DestroyViewport();}
    DGNPLATFORM_EXPORT virtual void _AdjustAspectRatio(ViewControllerR, bool expandView);
    DGNPLATFORM_EXPORT virtual StatusInt _ConnectToOutput();
    DGNPLATFORM_EXPORT virtual int _GetIndexedLineWidth(int index) const;
    DGNPLATFORM_EXPORT virtual uint32_t _GetIndexedLinePattern(int index) const;
    DGNPLATFORM_EXPORT virtual void _GetViewCorners(DPoint3dR low, DPoint3dR high) const;
    DGNPLATFORM_EXPORT virtual ViewportStatus _SetupFromViewController();
    DGNPLATFORM_EXPORT virtual ViewportStatus _Activate(QvPaintOptions const&);
    DGNPLATFORM_EXPORT virtual void _SetFrustumFromRootCorners(DPoint3dCP rootBox, double compressionFraction);
    DGNPLATFORM_EXPORT virtual void _SynchWithViewController(bool saveInUndo);
    virtual DgnDisplayCoreTypes::DeviceContextP _GetDcForView() const = 0;
    virtual uintptr_t _GetBackDropTextureId() {return 0;}
    DGNPLATFORM_EXPORT virtual ColorDef _GetWindowBgColor() const;
    virtual BentleyStatus _RefreshViewport(bool always, bool synchHealingFromBs, bool& stopFlag) = 0;
    virtual void _SetICachedDraw(ICachedDrawP cachedOutput) = 0;
    virtual double _GetMinimumLOD() const {return m_minLOD;}

public:
    DGNPLATFORM_EXPORT DgnViewport();
    virtual ~DgnViewport() {DestroyViewport();}

    BSIRectCP GetShadowDirtyRect();
    void SetShadowDirtyRect(BSIRectCP rect);
    bool CheckNeedsRefresh() const {return m_needsRefresh;}
    bool ShadowCastingLightsExist() const;
    ViewFlagsP GetViewFlagsP () {return &m_rootViewFlags;}
    bool GetGridRange(DRange3d* range);
    DGNPLATFORM_EXPORT double GetGridScaleFactor();
    DGNPLATFORM_EXPORT void PointToStandardGrid(DPoint3dR point, DPoint3dR gridOrigin, RotMatrixR rMatrix);
    DGNPLATFORM_EXPORT void GetGridRoundingDistance(DPoint2dR roundingDistance);
    DGNPLATFORM_EXPORT void GridFix(DPoint3dR point, RotMatrixCR rMatrixRoot, DPoint3dCR originRoot, DPoint2dCR roundingDistanceRoot, bool isoGrid);
    DGNPLATFORM_EXPORT void DrawStandardGrid(DPoint3dR gridOrigin, RotMatrixR rMatrix, Point2dCP fixedRepetitions = NULL);
    DGNPLATFORM_EXPORT BSIRect GetClientRect() const;
    DGNPLATFORM_EXPORT Point2d GetScreenOrigin() const;
    DGNPLATFORM_EXPORT void CalcNpcToView(DMap4dR npcToView);
    void ClearNeedsRefresh() {m_needsRefresh = false;}
    void SetIViewOutput(IViewOutputP output) {m_output = output;}
    void SetBackgroundColor(ColorDef color) {m_backgroundColor = color; m_backgroundColor.SetAlpha(0);}
    void AlignWithRootZ();
    DGNPLATFORM_EXPORT ColorDef GetWindowBgColor() const;
    DGNPLATFORM_EXPORT double GetMinimumLOD() const;
    IProgressiveDisplay::Completion DoProgressiveDisplay();
    void ClearProgressiveDisplay() {m_progressiveDisplay.clear();}
    DGNPLATFORM_EXPORT void ScheduleProgressiveDisplay(IProgressiveDisplay& pd);
    void SynchShadowList();
    void UpdateShadowList(DgnDrawMode, DrawPurpose);
    DGNPLATFORM_EXPORT double GetFocusPlaneNpc();
    DGNPLATFORM_EXPORT static StatusInt RootToNpcFromViewDef(DMap4d&, double*, CameraInfo const*, DPoint3dCR, DPoint3dCR, RotMatrixCR);
    DGNPLATFORM_EXPORT static int32_t GetMaxDisplayPriority();
    DGNPLATFORM_EXPORT static int32_t GetDisplayPriorityFrontPlane();
    DGNPLATFORM_EXPORT static ViewportStatus ValidateWindowSize(DPoint3dR delta, bool displayMessage);
    DGNPLATFORM_EXPORT static void FixFrustumOrder(Frustum&);
    DGNPLATFORM_EXPORT void InitViewSettings(bool useBgTexture);
    DGNPLATFORM_EXPORT void SetDisplayFlagFill(bool fill);
    DGNPLATFORM_EXPORT void SetDisplayFlagPatterns(bool patternsOn);
    DGNPLATFORM_EXPORT void SetDisplayFlagLevelSymb(bool levelSymbOn);
    ViewportStatus SetupFromViewController() {return _SetupFromViewController();}
    DGNPLATFORM_EXPORT void SetFrustumFromRootCorners(DPoint3dCP rootBox, double compressionFraction);
    DGNPLATFORM_EXPORT ViewportStatus ChangeArea(DPoint3dCP pts);
    DGNPLATFORM_EXPORT void Destroy();
    DGNPLATFORM_EXPORT StatusInt ComputeTransientRange(DRange3dP range, RotMatrixP rMatrix, bool checkLevelClass) const;
    DGNPLATFORM_EXPORT StatusInt ComputeVisibleDepthRange (double& minDepth, double& maxDepth, bool ignoreViewExtent = false);
    DGNPLATFORM_EXPORT StatusInt ComputeViewRange(DRange3dR, FitViewParams& params) ;
    DGNPLATFORM_EXPORT void SetNeedsHeal();
    DGNPLATFORM_EXPORT bool UseClipVolume(DgnModelCP) const;
    DGNPLATFORM_EXPORT StatusInt RefreshViewport(bool always, bool synchHealingFromBs, bool& stopFlag);
    DGNPLATFORM_EXPORT static int GetDefaultIndexedLineWidth(int index);
    DGNPLATFORM_EXPORT static uint32_t GetDefaultIndexedLinePattern(int index);
    DGNPLATFORM_EXPORT static void OutputFrustumErrorMessage(ViewportStatus errorStatus);
    bool Allow3dManipulations() const {return m_viewController->Allow3dManipulations();}
    DGNVIEW_EXPORT BentleyStatus PixelsFromInches(double& pixels, double inches) const;
    void DrawToolGraphics(ViewContextR context, bool isPreUpdate);

public:
    //! @return the current Camera for this DgnViewport. Note that the DgnViewport's camera may not match its ViewController's camera
    //! due to adjustments made for front/back clipping being turned off.
    DGNPLATFORM_EXPORT CameraInfo const& GetCamera() const;

    //! @return the camera target for this DgnViewport
    DGNPLATFORM_EXPORT DPoint3d GetCameraTarget() const;

    //! sets the object to be used for drawing tool graphics
    //! @param[in] handler The new tool graphics handler. NULL to clear
    DGNPLATFORM_EXPORT void SetToolGraphicsHandler(ToolGraphicsHandler* handler);

    //! Determine the depth, in NPC units, of the elements visible within a view.
    //! @param[out] low the npc value of the furthest back element in the view
    //! @param[out] high the npc value of the element closest to the front of view
    //! @param[in] subRectNpc If non-NULL, only search within a sub rectangle of the view. In NPC coordinates.
    //! @return SUCCESS if there were visible elements within the view, ERROR otherwise.
    //! @private
    DGNPLATFORM_EXPORT StatusInt DetermineVisibleDepthNpc(double& low, double& high, DRange3dCP subRectNpc=NULL);

    //! @return the point to use as the default rotation point at the center of the visible elements in the view.
    //! @note this method calls DetermineVisibleDepthNpc, which can be time consuming.
    //! @private
    DGNPLATFORM_EXPORT DPoint3d DetermineDefaultRotatePoint();

    //! Get the number width in pixels for a line weight value for this DgnViewport. Users select, and elements store a "line weight"
    //! value in the range of 0 to 31. Users can also specify a mapping from line weight values to pixels as a user preference.
    //! This mapping can vary from view to view, and for plotting views. This method returns the number of pixels for a given line weight value
    //! from the mapping table for this DgnViewport.
    //! @param[in] index the line weight value in the range of 0 to 31.
    //! @return  the number of pixels for lineWeightValue
    //! @private
    DGNPLATFORM_EXPORT int GetIndexedLineWidth(int index) const;

    //! Get the 32 bit on-off "line pattern" for a line code value for this DgnViewport. Users select, and elements store a "line code"
    //! value in the range of 0 to 7. Output devices can change the on-off patterns based on resolution, etc. This method returns the line pattern
    //! for a given line code value from the mapping table for this DgnViewport.
    //! @param[in] index a the range of 0 to 7.
    //! @return  the line pattern for lineCodeValue
    //! @private
    DGNPLATFORM_EXPORT uint32_t GetIndexedLinePattern(int index) const;

    //! Compute the range of the element when displayed in this DgnViewport
    //! @private
    DGNPLATFORM_EXPORT StatusInt ComputeFittedElementRange(DRange3dR range, DgnElementIdSet const& elements, RotMatrixCP rMatrix=nullptr);

    //! @private
    DGNPLATFORM_EXPORT void SetMinimumLOD (double minLOD);

/** @name Color Controls */
/** @{ */
    //! Get the RGB color of the background for this DgnViewport.
    //! @return background RGB color
    DGNPLATFORM_EXPORT ColorDef GetBackgroundColor() const;

    //! @return either white or black, whichever has more contrast to the background color of this DgnViewport.
    DGNPLATFORM_EXPORT ColorDef GetContrastToBackgroundColor() const;

    //! Adjust a color such that there is visible contrast to another color. This method is useful for adjusting a
    //! color slightly so that it can be discerned by the user in the context of other objects or against the background color.
    //! If the two colors are nearly the same, such that the user wouldn't be able to tell them apart (e.g. they are both
    //! very dark or very light) this method will adjust the starting color by making it either brighter or darker, but keeping
    //! the same hue, until there is sufficient Contrast.
    //! @param[in] thisColor Starting color
    //! @param[in] againstColor Color against which the result must contrast.
    //! @return  the adjusted, contrasting color
    DGNPLATFORM_EXPORT ColorDef AdjustColorForContrast(ColorDef thisColor, ColorDef againstColor) const;

    //! Adjust the transparency value of a TRGB color, leaving the Red, Blue, and Green components unchanged.
    //! @param[in] color               Original color
    //! @param[in] transparency        New transparency (0=opaque, 255=fully transparent)
    //! @return color with transparency adjusted.
    DGNPLATFORM_EXPORT static ColorDef MakeColorTransparency(ColorDef color, int transparency);

    //! Adjust the transparency value of a TRGB color, leaving the Red, Blue, and Green components unchanged, but ONLY IF
    //! the current transparency value of the color is opaque. If the color already has a transparency value, this method
    //! returns the original value of color.
    //! @param[in] color Original color
    //! @param[in] transparency New transparency (0=opaque, 255=fully transparent)
    //! @return color with transparency adjusted.
    DGNPLATFORM_EXPORT static ColorDef MakeTransparentIfOpaque(ColorDef color, int transparency);

    //! Get the current TBGR color value of the user-selected hilite color for this DgnViewport.
    //! @return the current TBGR hilite color.
    DGNPLATFORM_EXPORT ColorDef GetHiliteColor() const;

    //! Set the current display symbology for this DgnViewport by TBGR color values, a pixel width, and 0-7 line code.
    //! @param[in]          lineColor Line color
    //! @param[in]          fillColor Fill color
    //! @param[in]          lineWidth       Line width in pixels (1 or greater)
    //! @param[in]          lineCodeIndex   Line code index (0-7)
    DGNPLATFORM_EXPORT void SetSymbologyRgb(ColorDef lineColor, ColorDef fillColor, int lineWidth, int lineCodeIndex);
/** @} */

/** @name Coordinate Query and Conversion */
/** @{ */
    //! Get the Rotation Matrix for this DgnViewport. The concept of a DgnViewport's Rotation Matrix is somewhat limiting since it does not
    //! support perspective transformations. This method is provided for compatibility with previous API only.
    //! @see the Coordinate Coordinate Query and Conversion functions and #GetWorldToViewMap
    DGNPLATFORM_EXPORT RotMatrixCR GetRotMatrix() const;

    //! Get the DgnViewport rectangle in DgnCoordSystem::View.
    BSIRect GetViewRect() const {return _GetClientRect();}

    //! Get the DgnCoordSystem::View coordinates of lower-left-back and upper-right-front corners of a viewport.
    //! @param[out] low The lower left back corner of the view
    //! @param[out] high The upper right front corner of the view
    DGNPLATFORM_EXPORT void GetViewCorners(DPoint3dR low, DPoint3dR high) const;

    //! Get the DPI scale which can be used for conversion between physical pixels and device-independent pixels (DIPs).
    DGNPLATFORM_EXPORT DVec2d GetDpiScale() const;

    //! Get an 8-point frustum corresponding to the 8 corners of the DgnViewport in the specified coordinate system.
    //! When front or back clipping is turned \em off, there are two sets of corners that may be of interest.
    //! The "expanded" box is the one that is computed by examining the extents of the content of the view and moving
    //! the front and back planes to enclose everything in the view [N.B. this is the way that views implement
    //! the concept of "no front/back clipping", since there always must be a view frustum]. The "unexpanded" box is
    //! the one that is saved in the view definition.
    //! @param[in] sys Coordinate system for \c points
    //! @param[in] adjustedBox If true, and if f/b clipping is OFF, retrieve the "adjusted" box. Otherwise retrieve the box that came from the view definition.
    //! @return the view frustum
    //! @note The "adjusted" box may, in reality, be either larger or smaller than the "unexpanded" box since the expanded box depends on the current
    //!       content of the view.
    DGNPLATFORM_EXPORT Frustum GetFrustum(DgnCoordSystem sys=DgnCoordSystem::World, bool adjustedBox=false) const;

    //! Get the size of a single pixel at a given point as a distance along the view-x axis.
    //! The size of a pixel will only differ at different points within the same DgnViewport if the camera is on for this DgnViewport (in which case,
    //! points closer to the eye return larger values than ones further from the eye.)
    //! @param[in] rootPt      The point in DgnCoordSystem::World for determining pixel size. If NULL, use the center of the DgnViewport.
    //! @param[in] coordSys    The coordinate system for the returned distance.
    //! @return the size of the pixel at point \c rootPt.
    DGNPLATFORM_EXPORT double GetPixelSizeAtPoint(DPoint3dCP rootPt, DgnCoordSystem coordSys = DgnCoordSystem::World) const;

    //! Get the DMap4d to convert between DgnCoordSystem::World and DgnCoordSystem::View coordinates for this DgnViewport.
    //! @return the current WorldToView map for this DgnViewport.
    DGNPLATFORM_EXPORT DMap4dCP GetWorldToViewMap() const;

    //! Get the DMap4d to convert between DgnCoordSystem::World and DgnCoordSystem::Npc coordinates for this DgnViewport.
    //! @return the current WorldToNpc map for this DgnViewport.
    DGNPLATFORM_EXPORT DMap4dCP GetWorldToNpcMap() const;

    //! Transfrom an array of points in DgnCoordSystem::Npc into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] npcPts Input array in DgnCoordSystem::Npc
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToView(DPoint3dP viewPts, DPoint3dCP npcPts, int nPts) const;

    //! Transforma a point from DgnCoordSystem::Npc into DgnCoordSystem::View.
    DPoint3d NpcToView(DPoint3dCR npcPt) {DPoint3d viewPt; NpcToView(&viewPt, &npcPt, 1); return viewPt;}

    //! Transfrom an array of points in DgnCoordSystem::View into DgnCoordSystem::Npc.
    //! @param[out] npcPts An array to receive the points in DgnCoordSystem::Npc. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToNpc(DPoint3dP npcPts, DPoint3dCP viewPts, int nPts) const;

    //! Transforma a point from DgnCoordSystem::View into DgnCoordSystem::Npc.
    DPoint3d ViewToNpc(DPoint3dCR viewPt) {DPoint3d npcPt; ViewToNpc(&npcPt, &viewPt, 1); return npcPt;}

    //! Transfrom an array of points in DgnCoordSystem::View into DgnCoordSystem::Screen.
    //! @param[out] screenPts An array to receive the points in DgnCoordSystem::Screen. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToScreen(DPoint3dP screenPts, DPoint3dCP viewPts, int nPts) const;

    //! Transfrom an array of points in DgnCoordSystem::Screen into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] screenPts Input array in DgnCoordSystem::Screen
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ScreenToView(DPoint3dP viewPts, DPoint3dCP screenPts, int nPts) const;

    //! Transfrom an array of points in DgnCoordSystem::Npc into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the points in DgnCoordSystem::World. Must be dimensioned to hold \c nPts points.
    //! @param[in] npcPts Input array in DgnCoordSystem::Npc
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const;

    //! Transforma a point from DgnCoordSystem::Npc into DgnCoordSystem::World.
    DPoint3d NpcToWorld(DPoint3dCR npcPt) {DPoint3d worldPt; NpcToWorld(&worldPt, &npcPt, 1); return worldPt;}

    //! Transfrom an array of points in DgnCoordSystem::World into DgnCoordSystem::Npc.
    //! @param[out] npcPts An array to receive the points in DgnCoordSystem::Npc. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts  Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToNpc(DPoint3dP npcPts, DPoint3dCP worldPts, int nPts) const;

    //! Transforma a point from DgnCoordSystem::World into DgnCoordSystem::Npc.
    DPoint3d WorldToNpc(DPoint3dCR worldPt) {DPoint3d npcPt; WorldToNpc(&npcPt, &worldPt, 1); return npcPt;}

    //! Transfrom an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out] viewPts  An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transfrom an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the points in DgnCoordSystem::World. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array of DPoint4d coordinates in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const;

    //! Transfrom an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transforma a point from DgnCoordSystem::World into DgnCoordSystem::View.
    DPoint3d WorldToView(DPoint3dCR worldPt) {DPoint3d viewPt; WorldToView(&viewPt, &worldPt, 1); return viewPt;}

    //! Transfrom an array of points in DgnCoordSystem::World into an array of 2D points in DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView2d(DPoint2dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transfrom an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the points in DgnCoordSystem::World. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const;

    //! Transform a point from DgnCoordSystem::View into DgnCoordSystem::World.
    DPoint3d ViewToWorld(DPoint3dCR viewPt) {DPoint3d worldPt; ViewToWorld(&worldPt, &viewPt, 1); return worldPt;}
/** @} */

/** @name DgnViewport Parameters */
/** @{ */
    //! Determine whether this DgnViewport is currently active. Viewports become "active" after they have
    //! been initialized and connected to an output device.
    //! @return true if the DgnViewport is active.
    DGNPLATFORM_EXPORT bool IsActive() const;

    //! Determine whether this DgnViewport currently has a camera enabled. In this context, the "camera" is on
    //! if the WorldToView transform contains a perspective transformation.
    //! @remarks Applicable only to physical views.
    //! @return true if the camera is on.
    DGNPLATFORM_EXPORT bool IsCameraOn() const;

    //! Determine whether the Grid display is currently enabled in this DgnViewport.
    //! @return true if the grid display is on.
    DGNPLATFORM_EXPORT bool IsGridOn() const;

    //! Determine whether this viewport is a 3D view.
    //! @remarks Will be true only for a physical view.
    DGNPLATFORM_EXPORT bool Is3dView() const;

    //__PUBLISH_SECTION_END__
    //! Determine whether the root model of this viewport is a Sheet model.
    DGNPLATFORM_EXPORT bool IsSheetView() const;
    //__PUBLISH_SECTION_START__
    IViewDrawP GetIViewDraw() {return _GetIViewDraw();}
    DGNPLATFORM_EXPORT IViewOutputP GetIViewOutput();

    /** @cond BENTLEY_SDK_Scope1 */
    DGNPLATFORM_EXPORT ICachedDrawP GetICachedDraw();
    /** @endcond */

    //! Get a pointer to the ViewController associated with this DgnViewport.
    DGNPLATFORM_EXPORT ViewControllerCR GetViewController() const;

    //! Get a pointer to the ViewController associated with this DgnViewport.
    DGNPLATFORM_EXPORT ViewControllerR GetViewControllerR() const;

    //! If this view is a physical view, get the physical view controller.
    PhysicalViewControllerCP GetPhysicalViewControllerCP() const {return GetViewController()._ToPhysicalView();}
    //! If this view is a physical view, get a writeable pointer to the physical view controller.
    PhysicalViewControllerP GetPhysicalViewControllerP() {return (PhysicalViewControllerP) GetPhysicalViewControllerCP();}
    //! If this view is a camera view, get the camera physical view controller.
    CameraViewControllerCP GetCameraViewControllerCP() const {return GetViewController()._ToCameraView();}
    //! If this view is a camera view, get a writeable pointer to the camera physical view controller.
    CameraViewControllerP GetCameraViewControllerP() {return (CameraViewControllerP) GetCameraViewControllerCP();}

    //! If this view is a drawing view, get the drawing view controller.
    DrawingViewControllerCP GetDrawingViewControllerCP() const {return GetViewController()._ToDrawingView();}
    //! If this view is a drawing view, get a writeable pointer to the drawing view controller.
    DrawingViewControllerP GetDrawingViewControllerP() {return (DrawingViewControllerP) GetDrawingViewControllerCP();}

    //__PUBLISH_SECTION_END__
    //! If this view is a sheet view, get the sheet view controller.
    SheetViewControllerCP GetSheetViewControllerCP() const {return GetViewController()._ToSheetView();}
    //! If this view is a sheet view, get a writeable pointer to the sheet view controller.
    SheetViewControllerP GetSheetViewControllerP() {return (SheetViewControllerP) GetSheetViewControllerCP();}
    //__PUBLISH_SECTION_START__

    //! Get View Origin for this DgnViewport.
    //! @return the root coordinates of the lower left back corner of the DgnViewport.
    DGNPLATFORM_EXPORT DPoint3dCP GetViewOrigin() const;

    //! Get the View Delta (size) of this DgnViewport in root coordinate distances.
    //! @return the view delta in root coordinate distances.
    DGNPLATFORM_EXPORT DPoint3dCP GetViewDelta() const;

    //! Get the current View Flags for this DgnViewport.
    //! @return the View flags for this DgnViewport.
    DGNPLATFORM_EXPORT ViewFlagsCP GetViewFlags() const;

    //! Synchronized this DgnViewport with the current state of its ViewController. A DgnViewport may hold local copies of the information
    //! in its ViewController. Therefore, when changes are made to the state of a ViewController, it must be synchronized with the
    //! DgnViewport to which it is connected before the changes are visible to the user. This method also allows applications to save
    //! the changes between states of a ViewController to support the "view undo" command.
    //! @param[in] saveInUndo If true, the new state of the ViewController is compared to the previous state and changes are saved in the View Undo stack.
    //! If the user issues the "view undo" command, the changes are reversed and the ViewController is reverted to the previous state.
    void SynchWithViewController(bool saveInUndo) {_SynchWithViewController(saveInUndo);}

    DGNPLATFORM_EXPORT void SetNeedsRefresh();
/** @} */

/** @name Changing DgnViewport Frustum */
/** @{ */
    //! Scroll the DgnViewport by a given number of pixels in the view's X and/or Y direction. This method will move the DgnViewport's frustum
    //! in the indicated direction, but does \em not update the screen (even if the DgnViewport happens to be a visible View.) This method does
    //! change the ViewController associated with the DgnViewport.
    //! @param[in]      viewDist    The distance to scroll, in pixels.
    //! @note To update the view, see ViewManager::UpdateView or ViewManager::UpdateViewDynamic. To save the change to the ViewController
    //!       in the view undo buffer, see SynchWithViewController.
    DGNPLATFORM_EXPORT ViewportStatus Scroll(Point2dCP viewDist);

    //! Change the size of this DgnViewport's frustum by a ratio to its current size. Also, specify a new center point
    //! for the frustum in DgnCoordSystem::World coordinates.
    //! This method will change the DgnViewport's frustum, but does \em not update the screen (even if the DgnViewport happens
    //! to be a visible View.) This method \em does change the ViewController associated with the DgnViewport.
    //! @param[in]      newCenterRoot   The position, in DgnCoordSystem::World, for the new center of the frustum. If NULL, center is unchanged.
    //! @param[in]      factor          Scale factor to apply to current frustum. Scale factors greater than 1.0 zoom out (that is, the view
    //!                                   frustum gets larger and shows more of the model), and scale factors less than 1.0 zoom in.
    //! @note To update the view, see ViewManager::UpdateView or ViewManager::UpdateViewDynamic. To save the change to the ViewController
    //!       in the view undo buffer, see SynchWithViewController.
    DGNPLATFORM_EXPORT ViewportStatus Zoom(DPoint3dCP newCenterRoot, double factor);

    //! Change the frustum for this DgnViewport. The frustum is an 8-point array of points in DgnCoordSystem::World coordinates
    //! in the order specified by NpcCorners.
    //! This method will change the DgnViewport's frustum, but does \em not update the screen (even if the DgnViewport happens
    //! to be a visible View.) This method \em does change the ViewController associated with the DgnViewport.
    //! @note To update the view, see ViewManager::UpdateView or ViewManager::UpdateViewDynamic. To save the change to the ViewController
    //!       in the view undo buffer, see SynchWithViewController.
    DGNPLATFORM_EXPORT ViewportStatus SetupFromFrustum(Frustum const& frustPts);
/** @} */

    Utf8StringCR GetTitle() {return m_viewTitle;}
    void SetTitle(Utf8CP title) {m_viewTitle = title;}

    //__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT ColorDef GetSolidFillEdgeColor(ColorDef inColor);
    //__PUBLISH_SECTION_START__

    static double GetMinViewDelta() {return DgnUnits::OneMillimeter();}
    static double GetMaxViewDelta() {return 20000 * DgnUnits::OneKilometer();}    // about twice the diameter of the earth
    static double GetCameraPlaneRatio() {return 300.0;}
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
struct NonVisibleViewport : DgnViewport
{
protected:
    virtual DgnDisplayCoreTypes::DeviceContextP _GetDcForView() const override { return NULL; }
    virtual void _AllocateOutput() override {}
    virtual ICachedDrawP _GetICachedDraw() override { return NULL; }
    virtual void _SetICachedDraw(ICachedDrawP cachedOutput) override {}
    virtual ColorDef _GetWindowBgColor() const override {return ColorDef::Black();}
    virtual StatusInt _ConnectToOutput() override { return SUCCESS; }
    virtual void _AdjustZPlanesToModel(DPoint3dR, DVec3dR, ViewControllerCR) const override {}
    virtual BSIRect _GetClientRect() const override
        {
        BSIRect rect;
        rect.origin.x = rect.origin.y = 0;
        rect.corner.x = rect.corner.y = 1;
        return rect;
        }

    virtual BentleyStatus _RefreshViewport(bool always, bool synchHealingFromBs, bool& stopFlag) override { return ERROR; }
    virtual void _SetFrustumFromRootCorners(DPoint3dCP worldBox, double compressFraction) override {}
    virtual void _AdjustAspectRatio(ViewControllerR viewController, bool expandView) override {}

public:
    NonVisibleViewport(ViewControllerR viewController) {m_viewController = &viewController; SetupFromViewController();}
};

/** @endGroup */

#if defined (DOCUMENTATION_GENERATOR)
    }}
#else
    END_BENTLEY_DGNPLATFORM_NAMESPACE
#endif
