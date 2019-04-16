/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "ViewController.h"
#include "TileTree.h"

DGNPLATFORM_TYPEDEFS(IViewportAnimator);
DGNPLATFORM_REF_COUNTED_PTR(IViewportAnimator);

BEGIN_BENTLEY_DGN_NAMESPACE

/**  @addtogroup GROUP_DgnView DgnView Module

 A View is an abstract term to describe the way that applications display contents from a DgnDb on a device like a screen. 
 <p>There are different types of views to show different types of DgnModels in application-specific ways.
 <p>A ViewController provides persistence and behavior to a type of view.
 <p>A DgnViewport has a reference-counted-pointer to a ViewController that controls it.
 <p>A ViewContext holds the state of an operation performed on one or more elements in a DgnViewport.
 <p>A SpatialViewController of some kind is used to query and display elements from SpatialModels. 

  <h2>%DgnViewport Coordinates</h2>
  Coordinate information can be exchanged with Viewports using the various coordinate systems defined in DgnCoordSystem.
  - \c DgnCoordSystem::Screen - coordinates are relative to the upper left corner of the screen on which the viewport resides
  - \c DgnCoordSystem::View   - coordinates are relative to the upper left corner of the viewport.
  - \c DgnCoordSystem::Npc    - (<b>N</b>ormalized <b>P</b>lane <b>C</b>oordinates) the left bottom rear of the view is (0.0, 0.0, 0.0) and
                            the right top front of the view is coordinate (1.0, 1.0, 1.0)
  - \c DgnCoordSystem::World  - For PhysicalViews, the <i>world</i> coordinate system is the DgnDb coordinate system. For DrawingViews, 
                         the world coordinate system is the drawing's coordinate system.

  @see DgnCoordSystem
*/

//=======================================================================================
//! Parameters for the "fit view" operation
// @bsiclass                                                    Keith.Bentley   06/15
//=======================================================================================
struct FitViewParams
{
    RotMatrixCP m_rMatrix = nullptr;
    bool m_useElementAlignedBox = false;
    bool m_fitDepthOnly = false;
    bool m_limitByVolume = false;
};

//=======================================================================================
//! Interface adopted by an object which animates a viewport.
//! Only one animator may be associated with a viewport at a given time. Registering a new
//! animator replaces any existing animator.
//! The animator's _Animate() function will be invoked just prior to the rendering of each frame.
//! The return value of _Animate() indicates whether to keep the animator active or to remove it.
//! The animator may also be removed in response to certain changes to the viewport - e.g., when
//! the viewport is closed, or its view controller changed, etc.
// @bsistruct                                                   Paul.Connelly   12/16
//=======================================================================================
struct IViewportAnimator : RefCountedBase
{
    enum class RemoveMe { Yes, No };

    //! Apply animation to the viewport. Return RemoveMe::Yes when animation is completed, causing the animator to be removed from the viewport.
    virtual RemoveMe _Animate(DgnViewportR viewport) = 0;

    //! Invoked when this IViewportAnimator is removed from the viewport, e.g. because it was replaced by a new animator, the viewport was closed -
    //! that is, for any reason other than returning RemoveMe::Yes from _Animate()
    virtual void _OnInterrupted(DgnViewportR viewport) { }
};

//=======================================================================================
//! An IViewportAnimator which enables animated decorations. While the animator is
//! active, decorations will be invalidated on each frame. The animator's
//! _AnimateDecorations() function will be invoked to update any animation state; then
//! decorations will be re-requested and rendered.
//! decorations each frame for a set duration.
// @bsistruct                                                   Paul.Connelly   12/17
//=======================================================================================
struct DecorationAnimator : IViewportAnimator
{
private:
    BeTimePoint m_start;
    BeTimePoint m_stop;

protected:
    DecorationAnimator(BeDuration duration) : m_start(BeTimePoint::Now()), m_stop(m_start + duration) { }

    //! Override to update animation state, which can then be used on the next call to produce decorations.
    //! @param[in] viewport The viewport being animated
    //! @param[in] durationPercent The ratio of duration elapsed, in [0.0,1.0]
    //! @returns RemoveMe::Yes to immediately remove this animator, RemoveMe::No to continue animating until duration elapsed or animator interrupted.
    //! If this animator is interrupted, this function will be immediately invoked with durationPercent=1.0.
    virtual RemoveMe _AnimateDecorations(DgnViewportR viewport, double durationPercent) { return RemoveMe::No; }

    DGNPLATFORM_EXPORT RemoveMe _Animate(DgnViewportR) override;
    DGNPLATFORM_EXPORT void _OnInterrupted(DgnViewportR) override;
};

//=======================================================================================
/**
 A DgnViewport maps a set of DgnModels to an output device through a camera (a view frustum) and filters (e.g. categories, view flags, etc). 
 <p>
 Viewports are usually mapped to a window on a screen. But, they 
 can also be mapped to other types of output devices such as plot drivers, dialog boxes, bitmaps, etc.
 <p>
 When active, a DgnViewport is connected to ViewController, that determines the behavior of the view. ViewControllers supply the methods
 that control which elements are visible, and the rules for determining elements' symbology. ViewControllers may also add additional graphics
 to a view. ViewControllers store their persistent state in the properties table, and are synchronized with a DgnViewport (which may hold
 local copies of the current state). So, viewing tools that wish to change camera location or other viewing state, must call 
 DgnViewport::SynchWithViewController before the changes are visible to the user.
*/
//! @ingroup GROUP_DgnView
//! @nosubgrouping
//  @bsiclass                                                     KeithBentley    10/02
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnViewport : RefCounted<NonCopyableClass>
{
    friend struct ViewManager;
    typedef std::deque<ViewDefinitionPtr> ViewUndoStack;

    struct SyncFlags
    {
        friend struct DgnViewport;
    private:
        bool m_decorations = false;
        bool m_scene = false;
        bool m_renderPlan = false;
        bool m_controller = false;
        bool m_rotatePoint = false;
        bool m_firstDrawComplete = false;
        bool m_redrawPending = false;

    public:
        void InvalidateDecorations() {m_decorations=false;}
        void InvalidateScene() {m_scene=false; InvalidateDecorations();}
        void InvalidateRenderPlan() {m_renderPlan=false; InvalidateScene();}
        void InvalidateController() {m_controller=false; InvalidateRenderPlan(); InvalidateFirstDrawComplete();}
        void InvalidateRotatePoint() {m_rotatePoint=false;}
        void InvalidateFirstDrawComplete() {m_firstDrawComplete=false;}
        void InvalidateRedrawPending() {m_redrawPending = false;}
        void SetValidDecorations() {m_decorations=true;}
        void SetFirstDrawComplete() {m_firstDrawComplete=true;}
        void SetValidScene() {m_scene=true;}
        void SetValidController() {m_controller=true;}
        void SetValidRenderPlan() {m_renderPlan=true;}
        void SetValidRotatePoint() {m_rotatePoint=true;}
        void SetRedrawPending() {m_redrawPending = true;}
        bool IsValidDecorations() const {return m_decorations;}
        bool IsValidScene() const {return m_scene;}
        bool IsValidRenderPlan() const {return m_renderPlan;}
        bool IsValidController() const {return m_controller;}
        bool IsValidRotatePoint() const {return m_rotatePoint;}
        bool IsFirstDrawComplete() const {return m_firstDrawComplete;}
        bool IsRedrawPending() const {return m_redrawPending;}
    };

    //! Object to monitor changes to a DgnViewport. See #AddTracker, #DropTracker
    struct Tracker
    {
        virtual ~Tracker() {}
        virtual void _OnViewChanged() const {} //!< Called after this DgnViewport has been modified, e.g. through a viewing tool
        virtual void _OnViewClose() const {}   //!< Called when this DgnViewport is about to be closed.
    };

protected:
    mutable SyncFlags m_sync;
    bool m_zClipAdjusted = false;    // were the view z clip planes adjusted due to front/back clipping off?
    bool m_is3dView = false;         // view is of a 3d model
    bool m_isCameraOn = false;       // view is 3d and the camera is turned on.
    bool m_undoActive = false;
    bool m_fadeOutActive = false;
    bool m_renderContinuously = false;
    Byte m_dynamicsTransparency = 64;
    Byte m_flashingTransparency = 100;
    size_t m_maxUndoSteps = 20;
    uint32_t m_minimumFrameRate = Render::Target::DefaultMinimumFrameRate();
    DPoint3d m_viewOrg;                 // view origin, potentially expanded
    DVec3d m_viewDelta;                 // view delta, potentially expanded
    DPoint3d m_viewOrgUnexpanded;       // view origin (from ViewController, unexpanded)
    DVec3d m_viewDeltaUnexpanded;       // view delta (from ViewController, unexpanded)
    RotMatrix m_rotMatrix;              // rotation matrix (from ViewController)
    ViewDefinition3d::Camera m_camera;
    Render::TargetPtr m_renderTarget;
    Render::HiliteSettings m_hilite;
    DMap4d m_rootToView;
    DMap4d m_rootToNpc;
    double m_frustFraction;
    Utf8String m_viewTitle;
    ViewControllerPtr m_viewController;
    DPoint3d m_viewCmdTargetCenter;
    ViewDefinitionPtr m_currentBaseline;
    ViewUndoStack m_forwardStack;
    ViewUndoStack m_backStack;
    EventHandlerList<Tracker> m_trackers;
    IViewportAnimatorPtr m_animator;
    BeTimePoint m_flashUpdateTime;  // time the current flash started
    double m_flashIntensity;        // current flash intensity from [0..1]
    double m_flashDuration;         // the length of time that the flash intensity will increase (in seconds)
    DgnElementId m_flashedElem;
    DgnElementId m_lastFlashedElem;

    DGNPLATFORM_EXPORT void DestroyViewport();
    DGNPLATFORM_EXPORT void SuspendViewport();
    DGNPLATFORM_EXPORT virtual void _AdjustZPlanes(DPoint3dR origin, DVec3dR delta) const;
    void ExtendRangeForBackgroundMap(DRange3dR extents) const;
    virtual double _GetCameraFrustumNearScaleLimit() const {return GetRenderTarget()->_GetCameraFrustumNearScaleLimit();}

    virtual bool _IsVisible() const {return true;}
    DGNPLATFORM_EXPORT virtual void _CallDecorators(DecorateContextR);
    virtual Render::Plan::AntiAliasPref _WantAntiAliasLines() const {return Render::Plan::AntiAliasPref::Off;}
    virtual Render::Plan::AntiAliasPref _WantAntiAliasText() const {return Render::Plan::AntiAliasPref::Detect;}
    virtual void _SynchViewTitle() {}
    virtual void _Destroy() {DestroyViewport();}
    virtual void _Suspend() {SuspendViewport();}
    DGNPLATFORM_EXPORT virtual void _AdjustAspectRatio(DPoint3dR origin, DVec3dR delta);
    DGNPLATFORM_EXPORT static void StartRenderThread();
    DMap4d CalcNpcToView();
    void ChangeScene(Render::Task::Priority);
    DGNPLATFORM_EXPORT void SaveViewUndo();
    DGNPLATFORM_EXPORT void Animate();
    DGNVIEW_EXPORT bool ProcessFlash();
    DGNVIEW_EXPORT void PrepareDecorations(UpdatePlan const&, Render::Decorations&);
public:
    DgnViewport(Render::TargetP target) {SetRenderTarget(target);}
    virtual ~DgnViewport() {DestroyViewport();}

    Byte GetFlashingTransparency() const {return m_flashingTransparency;}
    void SetFlashingTransparency(Byte val) {m_flashingTransparency = val;}
    Byte GetDynamicsTransparency() const {return m_dynamicsTransparency;}
    void SetDynamicsTransparency(Byte val) {m_dynamicsTransparency = val;}

    DGNPLATFORM_EXPORT void SetFlashed(DgnElementId id, double duration);

    DGNPLATFORM_EXPORT void SetRenderTarget(Render::TargetP target);

    double GetFrustumFraction() const {return m_frustFraction;}
    bool IsVisible() {return _IsVisible();}
    Render::Plan::AntiAliasPref WantAntiAliasLines() const {return _WantAntiAliasLines();}
    Render::Plan::AntiAliasPref WantAntiAliasText() const {return _WantAntiAliasText();}
    void AlignWithRootZ();
    DGNVIEW_EXPORT bool RenderFrame(Render::Task::Priority priority, UpdatePlan const& plan, TileTree::TileRequestsR requests); // Generally, this should not be called directly
    DGNVIEW_EXPORT bool RenderThumbnail(Render::Task::Priority priority, UpdatePlan const& plan, TileTree::TileRequestsR requests);
    DGNVIEW_EXPORT void RenderSynchronousFrame(bool wantDecorators, bool wantHilite=true);
    uint32_t GetMinimumTargetFrameRate() const {return m_minimumFrameRate;}
    DGNPLATFORM_EXPORT uint32_t SetMinimumTargetFrameRate(uint32_t frameRate);
    DGNPLATFORM_EXPORT void InvalidateScene() const;
    DGNPLATFORM_EXPORT double GetFocusPlaneNpc();
    DGNPLATFORM_EXPORT StatusInt RootToNpcFromViewDef(DMap4d&, double&, ViewDefinition3d::Camera const*, DPoint3dCR, DPoint3dCR, RotMatrixCR) const;
    DGNPLATFORM_EXPORT static void FixFrustumOrder(Frustum&);
    DGNPLATFORM_EXPORT ViewportStatus SetupFromViewController();
    void Destroy() {_Destroy();}
    DGNPLATFORM_EXPORT StatusInt ComputeViewRange(DRange3dR, FitViewParams& params);
    void InvalidateDecorations() const {m_sync.InvalidateDecorations();}
    void InvalidateController() const {m_sync.InvalidateController();}
    void InvalidateRenderPlan() const {m_sync.InvalidateRenderPlan();}
    DGNPLATFORM_EXPORT static int GetDefaultIndexedLineWidth(int index);
    DGNPLATFORM_EXPORT static void OutputFrustumErrorMessage(ViewportStatus errorStatus);
    DGNPLATFORM_EXPORT void ChangeViewController(ViewControllerR);
    bool Allow3dManipulations() const {return m_viewController->Allow3dManipulations();}
    void DrawToolGraphics(ViewContextR context, bool isPreUpdate);
    DGNVIEW_EXPORT void SetViewCmdTargetCenter(DPoint3dCP newCenter);
    DPoint3dCP GetViewCmdTargetCenter() {return m_sync.IsValidRotatePoint() ? &m_viewCmdTargetCenter : nullptr;}
    Point2d GetScreenOrigin() const {return m_renderTarget->GetScreenOrigin();}
    DGNPLATFORM_EXPORT double PixelsFromInches(double inches) const;
    DGNVIEW_EXPORT void SuspendForBackground();
    DGNVIEW_EXPORT void ResumeFromBackground(Render::Target* target);
    DGNVIEW_EXPORT void OnResized();

    void SetUndoActive(bool val, size_t numsteps=20) {m_undoActive=val; m_maxUndoSteps=numsteps;}
    bool IsUndoActive() {return m_undoActive;}
    void ClearUndo();
    void ChangeDynamics(Render::DecorationListP list, Render::Task::Priority);
    DGNVIEW_EXPORT void ChangeRenderPlan(Render::Task::Priority);
    DGNVIEW_EXPORT void ApplyViewState(ViewDefinitionCR val, bool saveInUndo=true, BeDuration animationTime=BeDuration::Milliseconds(250));
    DGNVIEW_EXPORT void ApplyNext(BeDuration animationTime); 
    DGNVIEW_EXPORT void ApplyPrevious(BeDuration animationTime);
    DGNPLATFORM_EXPORT static Render::Queue& RenderQueue();

    DGNPLATFORM_EXPORT void SetFadeOutActive(bool val);
    bool IsFadeOutActive() const { return m_fadeOutActive; }

    //! @return the current Camera for this DgnViewport. Note that the DgnViewport's camera may not match its ViewController's camera
    //! due to adjustments made for front/back clipping being turned off.
    ViewDefinition3d::Camera const& GetCamera() const {return m_camera;}

    //! Determine the depth range, in NPC units, of the elements visible within a view.
    //! @param[out] low the npc value of the furthest back element in the view
    //! @param[out] high the npc value of the element closest to the front of view
    //! @param[in] subRectNpc If non-NULL, only search within a sub rectangle of the view. In NPC coordinates.
    //! @return SUCCESS if there were visible elements within the view, ERROR otherwise.
    //! @private
    DGNPLATFORM_EXPORT StatusInt DetermineVisibleDepthNpcRange(double& low, double& high, BSIRectCP subRectNpc=nullptr);

    //! Determine the depth range, in NPC units, of the elements visible within a view.
    //! @param[out] low the npc value of the furthest back element in the view
    //! @param[out] high the npc value of the element closest to the front of view
    //! @param[in] subRectNpc If non-NULL, only search within a sub rectangle of the view. In NPC coordinates.
    //! @return SUCCESS if there were visible elements within the view, ERROR otherwise.
    //! @private
    DGNPLATFORM_EXPORT StatusInt DetermineVisibleDepthNpcAverage(double& averageNpc, BSIRectCP subRectNpc=nullptr);

    //! @return the point to use as the default rotation point at the center of the visible elements in the view.
    //! @note this method calls DetermineVisibleDepthNpc, which can be time consuming.
    //! @private
    DGNPLATFORM_EXPORT DPoint3d DetermineDefaultRotatePoint();

    //! Determien the nearest visible geometry point within radiusPixels of the supplied pick point.
    //! @param[out] outPoint point on visible geometry in world coordinates.
    //! @param[in] pickPoint pick point in world coordinates.
    //! @param[in] radiusPixels radius in pixels around pick point to search for visible geometry.
    DGNPLATFORM_EXPORT StatusInt DetermineNearestVisibleGeometryPoint(DPoint3dR outPoint, DPoint3dCR pickPoint, int radiusPixels);

    //! Compute the range of the element when displayed in this DgnViewport
    //! @private
    DGNPLATFORM_EXPORT StatusInt ComputeFittedElementRange(DRange3dR range, DgnElementIdSet const& elements, RotMatrixCP rMatrix=nullptr);

/** @name Color Controls */
/** @{ */
    //! Get the background color for this DgnViewport.
    //! @return background color
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

    //! Adjust the transparency of a color, leaving the Red, Blue, and Green components unchanged.
    //! @param[in] color Original color
    //! @param[in] transparency New transparency (0=opaque, 255=fully transparent)
    //! @return color with transparency adjusted.
    DGNPLATFORM_EXPORT static ColorDef MakeColorTransparency(ColorDef color, int transparency);

    //! Adjust the transparency of a color, leaving the Red, Blue, and Green components unchanged, but ONLY IF
    //! the current transparency value of the color is opaque. If the color already has a transparency value, this method
    //! returns the original value of color.
    //! @param[in] color Original color
    //! @param[in] transparency New transparency (0=opaque, 255=fully transparent)
    //! @return color with transparency adjusted.
    DGNPLATFORM_EXPORT static ColorDef MakeTransparentIfOpaque(ColorDef color, int transparency);

    //! Get the current hilite color for this DgnViewport.
    //! @return the current TBGR hilite color.
    ColorDef GetHiliteColor() const {return m_hilite.GetColor();}

    //! Set the current TGBR color value of the user-selected hilite color for this DgnViewport.
    //! @param color The new TBGR hilite color
    void SetHiliteColor(ColorDef color) {m_hilite.SetColor(color);}

    void SetHilite(Render::HiliteSettings const& hilite) {m_hilite=hilite;}
    Render::HiliteSettings GetHilite() const {return m_hilite;}

/** @} */

/** @name Coordinate Query and Conversion */
/** @{ */
    //! Get the Rotation Matrix for this DgnViewport. The concept of a DgnViewport's Rotation Matrix is somewhat limiting since it does not
    //! support perspective transformations. This method is provided for compatibility with previous API only.
    //! @see the Coordinate Coordinate Query and Conversion functions and #GetWorldToViewMap
    RotMatrixCR GetRotMatrix() const {return m_rotMatrix;}
    DVec3d GetXVector() const {DVec3d v; GetRotMatrix().GetRow(v,0); return v;}
    DVec3d GetYVector() const {DVec3d v; GetRotMatrix().GetRow(v,1); return v;}
    DVec3d GetZVector() const {DVec3d v; GetRotMatrix().GetRow(v,2); return v;}

    //! Get the DgnViewport rectangle in DgnCoordSystem::View.
    virtual BSIRect _GetViewRect() const {return m_renderTarget->GetViewRect();}
    BSIRect GetViewRect() const {return _GetViewRect();}

    //! Get the DgnCoordSystem::View coordinates of lower-left-back and upper-right-front corners of a viewport.
    DGNPLATFORM_EXPORT DRange3d GetViewCorners() const;

    //! Get the DPI scale which can be used for conversion between physical pixels and device-independent pixels (DIPs).
    DVec2d GetDpiScale() const {return m_renderTarget->GetDpiScale();}

    //! Get an 8-point frustum corresponding to the 8 corners of the DgnViewport in the specified coordinate system.
    //! There are two sets of corners that may be of interest.
    //! The "adjusted" box is the one that is computed by examining the "project extents" and moving
    //! the front and back planes to enclose everything in the view [N.B. this is the way that views implement
    //! the concept of "no front/back clipping", since there always must be a view frustum]. The "unadjusted" box is
    //! the one that is stored in the ViewController.
    //! @param[in] sys Coordinate system for \c points
    //! @param[in] adjustedBox If true, retrieve the adjusted box. Otherwise retrieve the box that came from the view definition.
    //! @return the view frustum
    //! @note The "adjusted" box may be either larger or smaller than the "unadjusted" box.
    DGNPLATFORM_EXPORT Frustum GetFrustum(DgnCoordSystem sys=DgnCoordSystem::World, bool adjustedBox=true) const;

    //! Get the size of a single pixel at a given point as a distance along the view-x axis.
    //! The size of a pixel will only differ at different points within the same DgnViewport if the camera is on for this DgnViewport (in which case,
    //! points closer to the eye return larger values than ones further from the eye.)
    //! @param[in] rootPt      The point in DgnCoordSystem::World for determining pixel size. If NULL, use the center of the DgnViewport.
    //! @param[in] coordSys    The coordinate system for the returned distance.
    //! @return the size of the pixel at point \c rootPt.
    DGNPLATFORM_EXPORT double GetPixelSizeAtPoint(DPoint3dCP rootPt, DgnCoordSystem coordSys = DgnCoordSystem::World) const;

    //! Get the DMap4d to convert between DgnCoordSystem::World and DgnCoordSystem::View coordinates for this DgnViewport.
    //! @return the current WorldToView map for this DgnViewport.
    DMap4dCP GetWorldToViewMap() const {return &m_rootToView;}

    //! Get the DMap4d to convert between DgnCoordSystem::World and DgnCoordSystem::Npc coordinates for this DgnViewport.
    //! @return the current WorldToNpc map for this DgnViewport.
    DMap4dCP GetWorldToNpcMap() const {return &m_rootToNpc;}

    //! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] npcPts Input array in DgnCoordSystem::Npc
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToView(DPoint3dP viewPts, DPoint3dCP npcPts, int nPts) const;

    //! Transforma a point from DgnCoordSystem::Npc into DgnCoordSystem::View.
    DPoint3d NpcToView(DPoint3dCR npcPt) const {DPoint3d viewPt; NpcToView(&viewPt, &npcPt, 1); return viewPt;}

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::Npc.
    //! @param[out] npcPts An array to receive the points in DgnCoordSystem::Npc. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToNpc(DPoint3dP npcPts, DPoint3dCP viewPts, int nPts) const;

    //! Transform a point from DgnCoordSystem::View into DgnCoordSystem::Npc.
    DPoint3d ViewToNpc(DPoint3dCR viewPt) const {DPoint3d npcPt; ViewToNpc(&npcPt, &viewPt, 1); return npcPt;}

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::Screen.
    //! @param[out] screenPts An array to receive the points in DgnCoordSystem::Screen. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToScreen(DPoint3dP screenPts, DPoint3dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::Screen into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] screenPts Input array in DgnCoordSystem::Screen
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ScreenToView(DPoint3dP viewPts, DPoint3dCP screenPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::Npc into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the points in DgnCoordSystem::World. Must be dimensioned to hold \c nPts points.
    //! @param[in] npcPts Input array in DgnCoordSystem::Npc
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void NpcToWorld(DPoint3dP worldPts, DPoint3dCP npcPts, int nPts) const;

    //! Transform a point from DgnCoordSystem::Npc into DgnCoordSystem::World.
    DPoint3d NpcToWorld(DPoint3dCR npcPt) const {DPoint3d worldPt; NpcToWorld(&worldPt, &npcPt, 1); return worldPt;}

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::Npc.
    //! @param[out] npcPts An array to receive the points in DgnCoordSystem::Npc. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts  Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToNpc(DPoint3dP npcPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform a point from DgnCoordSystem::World into DgnCoordSystem::Npc.
    DPoint3d WorldToNpc(DPoint3dCR worldPt) const {DPoint3d npcPt; WorldToNpc(&npcPt, &worldPt, 1); return npcPt;}

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out] viewPts  An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint4dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the points in DgnCoordSystem::World. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array of DPoint4d coordinates in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint4dCP viewPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::World into DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView(DPoint3dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform a point from DgnCoordSystem::World into DgnCoordSystem::View.
    DPoint3d WorldToView(DPoint3dCR worldPt) const {DPoint3d viewPt; WorldToView(&viewPt, &worldPt, 1); return viewPt;}

    //! Transform an array of points in DgnCoordSystem::World into an array of 2D points in DgnCoordSystem::View.
    //! @param[out] viewPts An array to receive the points in DgnCoordSystem::View. Must be dimensioned to hold \c nPts points.
    //! @param[in] worldPts Input array in DgnCoordSystem::World
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void WorldToView2d(DPoint2dP viewPts, DPoint3dCP worldPts, int nPts) const;

    //! Transform an array of points in DgnCoordSystem::View into DgnCoordSystem::World.
    //! @param[out] worldPts An array to receive the points in DgnCoordSystem::World. Must be dimensioned to hold \c nPts points.
    //! @param[in] viewPts Input array in DgnCoordSystem::View
    //! @param[in] nPts Number of points in both arrays.
    DGNPLATFORM_EXPORT void ViewToWorld(DPoint3dP worldPts, DPoint3dCP viewPts, int nPts) const;

    //! Transform a point from DgnCoordSystem::View into DgnCoordSystem::World.
    DPoint3d ViewToWorld(DPoint3dCR viewPt) const {DPoint3d worldPt; ViewToWorld(&worldPt, &viewPt, 1); return worldPt;}

    DGNPLATFORM_EXPORT bool IsPointAdjustmentRequired() const;
    DGNPLATFORM_EXPORT bool IsSnapAdjustmentRequired() const;
    DGNPLATFORM_EXPORT bool IsContextRotationRequired() const;

/** @} */

/** @name DgnViewport Parameters */
/** @{ */
    //! Determine whether this DgnViewport is currently active. Viewports become "active" after they have
    //! been initialized and connected to an Render::Target.
    //! @return true if the DgnViewport is active.
    bool IsActive() const {return m_viewController.IsValid() && m_renderTarget.IsValid();}

    //! Determine whether this DgnViewport currently has a camera enabled. In this context, the "camera" is on
    //! if the WorldToView transform contains a perspective transformation.
    //! @remarks Applicable only to physical views.
    //! @return true if the camera is on.
    bool IsCameraOn() const {return m_isCameraOn;}

    //! Determine whether the Grid display is currently enabled in this DgnViewport.
    //! @return true if the grid display is on.
    bool IsGridOn() const {return GetViewFlags().ShowGrid();}

    //! Determine whether this viewport is a 3d view.
    //! @remarks Will be true only for a physical views.
    bool Is3dView() const {return m_is3dView;}

    Render::TargetP GetRenderTarget() const {return m_renderTarget.get();}

    //! Get the ViewController associated with this DgnViewport.
    ViewControllerCR GetViewController() const {return *m_viewController;}

    //! Get the ViewController associated with this DgnViewport.
    ViewControllerR GetViewControllerR() const {return *m_viewController;}

    //! If this view is a physical view, get the physical view controller.
    SpatialViewControllerCP GetSpatialViewControllerCP() const {return GetViewController()._ToSpatialView();}
    //! If this view is a physical view, get a writeable pointer to the physical view controller.
    SpatialViewControllerP GetSpatialViewControllerP() {return const_cast<SpatialViewControllerP>(GetSpatialViewControllerCP());}
    //! If this view is a drawing view, get the drawing view controller.
    DrawingViewControllerCP GetDrawingViewControllerCP() const {return GetViewController()._ToDrawingView();}
    //! If this view is a drawing view, get a writeable pointer to the drawing view controller.
    DrawingViewControllerP GetDrawingViewControllerP() {return const_cast<DrawingViewControllerP>(GetDrawingViewControllerCP());}
    //! If this view is a sheet view, get the sheet view controller.
    Sheet::ViewControllerCP GetSheetViewControllerCP() const {return GetViewController()._ToSheetView();}
    //! If this view is a sheet view, get a writeable pointer to the sheet view controller.
    Sheet::ViewControllerP GetSheetViewControllerP() {return const_cast<Sheet::ViewControllerP>(GetSheetViewControllerCP());}

    //! Get View Origin for this DgnViewport.
    //! @return the root coordinates of the lower left back corner of the DgnViewport.
    DPoint3dCP GetViewOrigin() const {return &m_viewOrg;}

    //! Get the View Delta (size) of this DgnViewport in root coordinate distances.
    //! @return the view delta in root coordinate distances.
    DPoint3dCP GetViewDelta() const {return &m_viewDelta;}

    //! Get the View Flags for this DgnViewport.
    //! @return the View flags for this DgnViewport.
    Render::ViewFlags GetViewFlags() const {return m_viewController->GetViewFlags();}

    //! Synchronized this DgnViewport with the current state of its ViewController. A DgnViewport may hold local copies of the information
    //! in its ViewController. Therefore, when changes are made to the state of a ViewController, it must be synchronized with the
    //! DgnViewport to which it is connected before the changes are visible to the user. This method also allows applications to save
    //! the changes between states of a ViewController to support the "view undo" command.
    //! @param[in] saveInUndo If true, the new state of the ViewController is compared to the previous state and changes are saved in the View Undo stack.
    //! If the user issues the "view undo" command, the changes are reversed and the ViewController is reverted to the previous state.
    DGNPLATFORM_EXPORT void SynchWithViewController(bool saveInUndo);
/** @} */

/** @name Changing DgnViewport Frustum */
/** @{ */
    //! Scroll the DgnViewport by a given number of pixels in the view's X and/or Y direction. This method will move the DgnViewport's frustum
    //! in the indicated direction, but does \em not update the screen (even if the DgnViewport happens to be visible.) This method 
    //! changes the ViewController associated with the DgnViewport.
    //! @param[in] viewDist The distance to scroll, in pixels.
    DGNPLATFORM_EXPORT ViewportStatus Scroll(Point2dCP viewDist);

    //! Change the size of this DgnViewport's frustum by a ratio to its current size. Also, specify a new center point
    //! for the frustum in DgnCoordSystem::World coordinates.
    //! This method will change the DgnViewport's frustum, but does \em not update the screen (even if the DgnViewport happens
    //! to be a visible View.) This method \em does change the ViewController associated with the DgnViewport.
    //! @param[in] newCenterRoot The position, in DgnCoordSystem::World, for the new center of the frustum. If NULL, center is unchanged.
    //! @param[in] factor Scale factor to apply to current frustum. Scale factors greater than 1.0 zoom out (that is, the view
    //!                   frustum gets larger and shows more of the model), and scale factors less than 1.0 zoom in.
    DGNPLATFORM_EXPORT ViewportStatus Zoom(DPoint3dCP newCenterRoot, double factor);

    //! Change the frustum for this DgnViewport. The frustum is an 8-point array of points in DgnCoordSystem::World coordinates
    //! in the order specified by NpcCorners.
    //! This method will change the DgnViewport's frustum, but does \em not update the screen (even if the DgnViewport happens
    //! to be visible.) This method \em does change the ViewController associated with the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus SetupFromFrustum(Frustum const& frustPts);
/** @} */

    Utf8StringCR GetTitle() {return m_viewTitle;}
    void SetTitle(Utf8CP title) {m_viewTitle = title;}

    EventHandlerList<Tracker>& GetTrackers() {return m_trackers;}
    void AddTracker(Tracker* tracker) {m_trackers.AddHandler(tracker);}
    void DropTracker(Tracker* tracker) {m_trackers.DropHandler(tracker);}

    DGNPLATFORM_EXPORT void SetAnimator(IViewportAnimatorR animator);
    DGNPLATFORM_EXPORT void RemoveAnimator();

    // Continuous rendering causes the viewport to redraw on every iteration of the render loop, regardless of whether anything has changed
    // which might affect display. This can negatively impact performance. It should only be enabled while untracked animations (such as
    // those used for scientific visualization) are active.
    bool IsContinuousRenderingEnabled() const { return m_renderContinuously; }
    void EnableContinuousRendering() { m_renderContinuously = true; }
    void DisableContinuousRendering() { m_renderContinuously = false; }
    void SetContinousRendering(bool val) { m_renderContinuously = val; }

    DGNPLATFORM_EXPORT ColorDef GetSolidFillEdgeColor(ColorDef inColor);

    DGNPLATFORM_EXPORT void ChangeActiveVolume(ClipVectorP volume);

    //! Read the current image from this viewport from the Rendering system. 
    //! @param[in] viewRect The area of the view to read. The origin of \a viewRect must specify the upper left corner. It is an error to specify a view rectangle that lies outside the actual view. If not specified, the entire view is captured.
    //! @param[in] targetSize The size of the Image to be returned. The size can be larger or smaller than the original view. If not specified, the returned image is full size.
    //! @note By using a combination of \a viewRect and \a targetSize, you can tell this function to both clip and
    //! scale the image in the view. For example, use \a viewRect to specify a rectangle within the view to get a clipped image.
    //! Specify \a targetSize to be less than the size of the view rectangle to scale the image down. 
    //! @note The viewRect is adjusted as necessary to preserve the aspect ratio.
    //! The image is fitted to the smaller dimension of the viewRect and centered in the larger dimension.
    //! @return the Image containing the RGBA pixels from the specified rectangle of the viewport. On error, image.IsValid() will return false.
    DGNVIEW_EXPORT Render::Image ReadImage(BSIRectCR viewRect = BSIRect::From(0,0,-1,-1), Point2dCR targetSize=Point2d::From(0,0));

    //! Read selected data about each pixel within a rectangular portion of the viewport.
    //! @param[in] viewRect The area of the view to read. The origin specifies the upper-left corner. Must lie entirely within the viewport's dimensions.
    //! @return an IPixelDataBuffer object from which the selected data can be retrieved, or nullptr on error.
    DGNPLATFORM_EXPORT Render::IPixelDataBufferCPtr ReadPixels(BSIRectCR viewRect);

    //! Return the NPC geometry point for pixel data.
    //! @param[out] npc The npc point for visible geometry at the specified location.
    //! @param[in] pixelData Pixel data (as returned from ReadPixels).
    //! @param[in] x value in view coordinates.
    //! @param[in] y value in view coordinates.
    //! @return true if there is geometry visible at the specified location.
    bool GetPixelDataNpcPoint(DPoint3dR npc, Render::IPixelDataBufferCR pixelData, int32_t x, int32_t y);

    //! Return the world geometry point for pixel data.
    //! @param[out] world The world point for visible geometry at the specified location.
    //! @param[in] pixelData Pixel data (as returned from ReadPixels).
    //! @param[in] x value in view coordinates.
    //! @param[in] y value in view coordinates.
    //! @return true if there is geometry visible at the specified location.
    bool GetPixelDataWorldPoint(DPoint3dR world, Render::IPixelDataBufferCR pixelData, int32_t x, int32_t y);

};


//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/17
//=======================================================================================
struct OffscreenViewport : DgnViewport
{
    BSIRect m_rect;
    BSIRect _GetViewRect() const override {return m_rect;}
    void SetRect(BSIRect rect, bool temporary=false) {m_rect=rect; m_renderTarget->_SetViewRect(rect, temporary);}
    DGNVIEW_EXPORT OffscreenViewport();
    DGNVIEW_EXPORT explicit OffscreenViewport(double tileSizeModifier);
};

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   01/10
//=======================================================================================
struct NonVisibleViewport : DgnViewport
{
protected:
    void _AdjustAspectRatio(DPoint3dR origin, DVec3dR delta) override {}

public:
    NonVisibleViewport(Render::Target* target, ViewControllerR viewController) : DgnViewport(target) {m_viewController = &viewController; SetupFromViewController();}
};
END_BENTLEY_DGN_NAMESPACE

/** @endGroup */

