/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ViewController.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"

DGNPLATFORM_TYPEDEFS(PhysicalViewController)
DGNPLATFORM_TYPEDEFS(CameraViewController)
DGNPLATFORM_TYPEDEFS(HypermodelingViewController)
DGNPLATFORM_TYPEDEFS(DrawingViewController)
DGNPLATFORM_TYPEDEFS(SectionDrawingViewController)
DGNPLATFORM_TYPEDEFS(SheetViewController)
DGNPLATFORM_TYPEDEFS(FitViewParams)
DGNPLATFORM_TYPEDEFS(CameraInfo)
DGNPLATFORM_TYPEDEFS(SectioningViewController)

DGNPLATFORM_REF_COUNTED_PTR(DrawingViewController)
DGNPLATFORM_REF_COUNTED_PTR(SectionDrawingViewController)
DGNPLATFORM_REF_COUNTED_PTR(SheetViewController)
DGNPLATFORM_REF_COUNTED_PTR(PhysicalViewController)
DGNPLATFORM_REF_COUNTED_PTR(CameraViewController)
DGNPLATFORM_REF_COUNTED_PTR(SectioningViewController)
DGNPLATFORM_REF_COUNTED_PTR(HypermodelingViewController)

#include "IAuxCoordSys.h"
#include "ViewContext.h"
#include "DgnPlatformErrors.r.h"
#include "SectionClip.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct FullUpdateInfo;
struct DynamicUpdateInfo;

enum class OrientationMode
{
    CompassHeading  = 0,    //!< Use compass heading from device
    RelativeHeading = 1,    //!< Use heading relative to device's default orientation
    IgnoreHeading   = 2,    //!< Do not modify orientation from device heading
};

enum class UiOrientation
{
    Portrait            = 0,    //!< Up vector is positive device y, right vector is positive device x
    LandscapeRight      = 1,    //!< Up vector is negative device x, right vector is positive device y
    PortraitUpsideDown  = 2,    //!< Up vector is negative device y, right vector is negative device x
    LandscapeLeft       = 3,    //!< Up vector is positive device x, right vector is negative device y
};

//=======================================================================================
//! The current position, lens angle, and focus distance of a camera.
//=======================================================================================
struct CameraInfo
{
private:
    double   m_lensAngle;
    double   m_focusDistance;
    DPoint3d m_eyePoint;
    static bool IsValidLensAngle(double val) {return val>(Angle::Pi()/8.0) && val<Angle::Pi();}

public:
    void     InvalidateFocus() {m_focusDistance=-1.0;}
    bool     IsFocusValid() const {return m_focusDistance > 0.0 && m_focusDistance<1.0e14;}
    double   GetFocusDistance() const {return m_focusDistance;}
    void     SetFocusDistance(double dist) {m_focusDistance = dist;}
    bool     IsLensValid() const {return IsValidLensAngle(m_lensAngle);}
    void     ValidateLens() {if (!IsLensValid()) m_lensAngle=Angle::PiOver2();}
    double   GetLensAngle() const {return m_lensAngle;}
    void     SetLensAngle(double angle) {m_lensAngle = angle;}
    DPoint3dCR GetEyePoint() const {return m_eyePoint;}
    void     SetEyePoint(DPoint3dCR pt) {m_eyePoint = pt;}
    bool     IsValid() const {return IsLensValid() && IsFocusValid();}
};

//=======================================================================================
//! @ingroup DgnViewGroup
/**
 A ViewController provides the behavior for a type of view. It also provides the persistent information
 about how the view relates to a \ref DgnDbGroup (e.g. what models/categories are displayed, the ViewFlags that control how graphics
 are represented, etc.)
 <p>
 When a ViewController is paired with a DgnViewport, it then controls the operation of that view. Generally there will
 be a 1-1 relationship between ViewControllers and Viewports and a DgnViewport holds a reference-counted-pointer to its \
 ViewController. See discussion at #DgnViewport about synchronizing Viewports and ViewControllers.
 <p>
 By overriding virtual methods, subclasses of ViewController may:
     - load and save settings from the database
     - customize the set of elements that are displayed in the view
     - customize the way the graphics for an individual element appear
     - draw non-persistent graphics into the view
     - draw "decorations" on top of the normal graphics
     - etc.

<h3>View Controller types and sub-types</h3>
A ViewController has a DgnViewType, indicating in broad terms what kind of view it is. There are only a few different DgnViewTypes.
There are many specific types of ViewController within each view type category. ViewController defines a number of convenience methods
to dynamic_cast a controller to a subclass. See ViewController::ToPhysicalViewController, ViewController::ToCameraViewController,
ViewController::ToDrawingViewController.

@verbatim
    ViewControllerR viewController = ...

    PhysicalViewControllerP physicalView = viewController.ToPhysicalViewControllerP();
    if (physicalView != NULL)
        {
        physicalView->somefunction ();
        }

@endverbatim

<h3>Inserting a new view in the database</h3>

To create a new view, you must insert a ViewDefinition element and store type-specific settings for it. To define and store
type-specific view data, you must create and save an instance of a subclass of ViewController.

<h3>Loading data for an existing view from the database</h3>

To read the settings stored in the database for a particular view, you must create an instance of the ViewController sub-class
that corresponds to the view and then load it. The simplest way to create the correct type of controller for a view is to call
the ViewDefinition::LoadViewController method, like this:
@verbatim
    auto viewController = ViewDefinition::LoadViewController(viewId, project);
    if (!viewController.IsValid())
        return BSIERROR;

    PhysicalViewControllerP physicalView = viewController->ToPhysicalViewControllerP();
    if (physicalView != NULL)
        {
        physicalView->somefunction ();
        }

@endverbatim

<h3>Defining a subclass of ViewController</h3>

To create a subclass of ViewController, create a ViewHandler and implement _SupplyController.

*/
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewController : RefCountedBase
{
protected:
    friend struct  ViewContext;
    friend struct  DisplayHandler;
    friend struct  UpdateContext;
    friend struct  HealContext;
    friend struct  DgnViewport;
    friend struct  ViewManager;
    friend struct  IndexedViewport;
    friend struct  PhysicalRedlineViewController;
    friend struct  IACSManager;
    friend struct  ToolAdmin;
    friend struct  ViewDefinition;

    DgnDbR         m_dgndb;
    ViewFlags      m_viewFlags;
    DgnViewId      m_viewId;
    DgnClassId     m_classId;
    DgnModelId     m_baseModelId;
    DgnModelId     m_targetModelId;
    DgnModelIdSet  m_viewedModels;
    DgnCategoryIdSet  m_viewedCategories;
    ColorDef       m_backgroundColor;      // used only if bit set in flags
    RotMatrix      m_defaultDeviceOrientation;
    bool           m_defaultDeviceOrientationValid;
    mutable bmap<DgnSubCategoryId,DgnSubCategory::Appearance> m_subCategories;
    bmap<DgnSubCategoryId,DgnSubCategory::Override> m_subCategoryOverrides;

public:

protected:
    DGNPLATFORM_EXPORT ViewController(DgnDbR, DgnViewId viewId);
    void LoadCategories(JsonValueCR);
    void ReloadSubCategory(DgnSubCategoryId);

    virtual ~ViewController(){}
    virtual void _AdjustAspectRatio(double, bool expandView) = 0;
    virtual DPoint3d _GetTargetPoint() const {return GetCenter();}
    virtual DPoint3d _GetOrigin() const = 0;
    virtual DVec3d _GetDelta() const = 0;
    virtual RotMatrix _GetRotation() const = 0;
    virtual void _SetOrigin(DPoint3dCR viewOrg) = 0;
    virtual void _SetDelta(DVec3dCR viewDelta) = 0;
    virtual void _SetRotation(RotMatrixCR viewRot) = 0;
    enum class FitComplete {No=0, Yes=1};
    virtual FitComplete _ComputeFitRange(DRange3dR range, DgnViewportR, FitViewParamsR) {return FitComplete::No;}
    virtual void _OnViewOpened(DgnViewportR) {}
    virtual bool _Allow3dManipulations() const {return false;}
    virtual void _OnDynamicUpdateComplete(DgnViewportR vp, ViewContextR context, bool completedSuccessfully) {}
    virtual void _OnAttachedToViewport(DgnViewportR) {}
    virtual ColorDef _GetBackgroundColor() const {return m_backgroundColor;}
    virtual double _GetAspectRatioSkew() const {return 1.0;}

    DGNPLATFORM_EXPORT virtual void _FillModels();
    DGNPLATFORM_EXPORT virtual ViewportStatus _SetupFromFrustum(Frustum const& inFrustum);

    //! @return true to project un-snapped points to the view's ACS plane.
    //! @note Normally true for a 3d view. A 3d digitizier supplying real z values would not want this...maybe this would be a special ViewController?
    DGNPLATFORM_EXPORT virtual bool _IsPointAdjustmentRequired(DgnViewportR vp) const;

    //! @return true to project snap points to the view's ACS plane.
    //! @note Normally true for a 3d view only when ACS plane snap is enabled.
    DGNPLATFORM_EXPORT virtual bool _IsSnapAdjustmentRequired(DgnViewportR vp, bool snapLockEnabled) const;

    //! @return true to automatically orient AccuDraw to the view's ACS plane when initially made active.
    //! @note Normally true for a view only when ACS context lock is enabled.
    DGNPLATFORM_EXPORT virtual bool _IsContextRotationRequired(DgnViewportR vp, bool contextLockEnabled) const;

    //!< Store settings in the supplied Json object. These values will be persisted in the database and in the undo stack
    //!< Note that if you override _SaveToSettings, you must call T_Super::_SaveToSettings!
    DGNPLATFORM_EXPORT virtual void _SaveToSettings(JsonValueR) const;

    //!< Restore settings from the supplied Json object. These values were persisted in the database and in the undo stack
    //!< Note that if you override _RestoreFromSettings, you must call T_Super::_RestoreFromSettings!
    DGNPLATFORM_EXPORT virtual void _RestoreFromSettings(JsonValueCR);

    //! Display locate circle and information about the current AccuSnap/auto-locate HitDetail.
    DGNPLATFORM_EXPORT virtual void _DrawLocateCursor(DgnViewportR, DPoint3dCR, double aperture, bool isLocateCircleOn, HitDetailCP hit=nullptr);

    //! Decorators are not stored in the backing store and must therefore be drawn every frame. Overlay decorators are drawn with the z-buffer
    //! disabled and therefore always appear on top of elements in the view. Note that graphics drawn from this method are always drawn in a
    //! shaded render mode with a constant level of lighting, regardless of the view flags and lighting of the viewport.
    //! @param[in] viewport The DgnViewport into which the decorations should be drawn.
    virtual bool _DrawOverlayDecorations(IndexedViewportR viewport) {return false;}

    //! Decorators are not stored in the backing store and must therefore be drawn every frame. This method is called with the z-buffer enabled,
    //! so it can be used to draw decorators that are inter-mixed with elements in the view. Note that graphics drawn from this method use the
    //! active view flags and lighting for the viewport.
    //! @param[in] viewport The DgnViewport into which the decorations should be drawn.
    virtual bool _DrawZBufferedDecorations(IndexedViewportR viewport) {return false;}

    //! Background graphics are drawn whenever a view is "updated". Background graphics are drawn with the z-buffer turned off, so they will always
    //! appear "behind" any other graphics that are drawn to the view.
    //! @param[in] context the ViewContext being used to display the view. Test DrawPurpose to tell the purpose of the call.
    virtual void _DrawBackgroundGraphics(ViewContextR context) {}

    //! ZBuffered graphics are drawn whenever a view is "updated". They are drawn with the z-buffer enabled, so they intersperse with the persistent
    //! elements in the view.
    //! @param[in] context the ViewContext being used to display the view. Test DrawPurpose to tell the purpose of the call.
    //! @note ZBuffered graphics are sometimes referred to as "transient" graphics.
    virtual void _DrawZBufferedGraphics(ViewContextR context) {}

    //! Called when the display of a category is turned on or off.
    //! @param[in] singleEnable true if just turned on one category; false if
    //! turned off a category or made a group of changes.
    virtual void _OnCategoryChange(bool singleEnable) {}

    //! Turn the display of a category on or off.
    DGNPLATFORM_EXPORT virtual void _ChangeCategoryDisplay(DgnCategoryId, bool onOff);

    //! Turn the display of a model on or off.
    DGNPLATFORM_EXPORT virtual void _ChangeModelDisplay(DgnModelId, bool onOff);

    //! Draws the contents of the view.
    //! @remarks It is very rare that an applications needs to call this or to override it.
    DGNPLATFORM_EXPORT virtual void _DrawView(ViewContextR);

    //! Invokes the _VisitElement on \a context for <em>each element</em> that is in the view.
    //! For normal views, this does the same thing as _DrawView.
    DGNPLATFORM_EXPORT virtual void _VisitElements(ViewContextR& context);

    //! Draw a single element through a ViewContext.
    //! An application can override _DrawElement to change the symbology of elements.
    //! @remarks For elements that only occupy a few pixels on the screen, DgnPlatform calls \ref _DrawElementFiltered instead of _DrawElement.
    DGNPLATFORM_EXPORT virtual void _DrawElement(ViewContextR, GeometrySourceCR);

    //! DgnPlatform calls _DrawElementFiltered instead of _DrawElement when it needs to draw an element but decides that the
    //! representation in the view is small enough that it can be simplified.
    DGNPLATFORM_EXPORT virtual void _DrawElementFiltered(ViewContextR, GeometrySourceCR, DPoint3dCP pts, double size);

    //! Invoked just before the locate tooltip is displayed to retrieve the info text. Allows the ViewController to override the default description.
    //! @param[in]  hit The locate HitDetail whose info is needed.
    //! @param[out] descr The info string.
    //! @param[in] delimiter The default delimiter to use when building the info string.
    //! @return true if the info string was set or false to use the default implementation.
    virtual bool _GetInfoString(HitDetailCR hit, Utf8StringR descr, Utf8CP delimiter) const {return false;}

#if !defined (DOCUMENTATION_GENERATOR)
    DGNPLATFORM_EXPORT virtual StatusInt _VisitHit(HitDetailCR hit, ViewContextR context) const;

    //! Used to notify derived classes when an update completes.
    //! <p>See QueryViewController::_OnUpdateComplete
    virtual void _OnUpdateComplete(DrawPurpose updateType) const {}

    //! Used to notify derived classes when a heal update begins.
    //! <p>See QueryViewController::_OnHealUpdate
    virtual void _OnHealUpdate(DgnViewportR vp, ViewContextR context, bool fullHeal) {}

    //! Used to notify derived classes when a dynamic update begins.
    //! <p>See QueryViewController::_OnDynamicUpdate
    virtual void _OnDynamicUpdate(DgnViewportR vp, ViewContextR context, DynamicUpdateInfo& info) {}

    //! Used to notify derived classes when a full update begins.
    //! <p>See QueryViewController::_OnFullUpdate
    virtual void _OnFullUpdate(DgnViewportR vp, ViewContextR context, FullUpdateInfo&) {}
#endif

    //! Used to notify derived classes of an attempt to locate the viewport around the specified
    //! WGS84 location. Override to change how these points are interpreted.
    //! @param[out] status Extra information about how this event was handled; ignored if return value is false.
    //! @param[in] point the new location
    //! @return true to indicate that the view was modified.
    virtual bool _OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) {return false;}

    //! Used to notify derived classes of an attempt to orient the viewport around the specified
    //! rotation matrix from the device's orientation.
    //! @return true to indicate that the view was modified.
    virtual bool _OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui) {return false;}

    //! Returns the target model. Normally, this is the model at the m_targetIndex index of m_viewedModels.
    //! Used as the writable model in which new elements can be placed.
    //! A subclass can override this function to get the target model some other way.
    DGNPLATFORM_EXPORT virtual DgnModelP _GetTargetModel() const;

    //! Returns the project that is being viewed
    virtual DgnDbR _GetDgnDb() const {return m_dgndb;}

    //! Get the union of the range (axis-aligned bounding box) of all physical elements in project
    DGNPLATFORM_EXPORT virtual AxisAlignedBox3d _GetViewedExtents() const;

    DGNPLATFORM_EXPORT BeSQLite::DbResult QueryViewsPropertyAsJson(JsonValueR, DgnViewProperty::Spec const&) const;
    DGNPLATFORM_EXPORT BentleyStatus CheckViewSubType() const;

public:
    /*=================================================================================**//**
    * Margins for "white space" to be left around view volumes for #LookAtVolume.
    * Values mean "percent of view" and must be between 0 and .25.
    * @bsiclass                                     Grigas.Petraitis                02/13
    +===============+===============+===============+===============+===============+======*/
    struct MarginPercent
    {
    private:
        double m_left;
        double m_top;
        double m_right;
        double m_bottom;

        double LimitMargin(double val) {return (val<0.0) ? 0.0 :(val>.25) ? .25 : val;}

    public:
        MarginPercent(double left, double top, double right, double bottom) {Init(left, top, right, bottom);}
        void Init(double left, double top, double right, double bottom)
            {
            m_left   = LimitMargin(left);
            m_top    = LimitMargin(top);
            m_right  = LimitMargin(right);
            m_bottom = LimitMargin(bottom);
            }

        double Left() const   {return m_left;}
        double Top() const    {return m_top;}
        double Right() const  {return m_right;}
        double Bottom() const {return m_bottom;}
    };

    DGNPLATFORM_EXPORT StatusInt VisitHit (HitDetailCR, ViewContextR) const;
    DGNPLATFORM_EXPORT void DrawView(ViewContextR);
    DGNPLATFORM_EXPORT void VisitElements(ViewContextR);
    DGNPLATFORM_EXPORT void ChangeModelDisplay(DgnModelId, bool onOff);
    DGNPLATFORM_EXPORT StatusInt GetRangeForFit(DRange3dR range);
    DGNPLATFORM_EXPORT void OnViewOpened(DgnViewportR);
    void SetBaseModelId(DgnModelId id) {m_baseModelId = id;}
    DgnModelId GetBaseModelId() const {return m_baseModelId;}
    bool IsCategoryViewed(DgnCategoryId categoryId) const {return m_viewedCategories.Contains(categoryId);}
    bool IsModelViewed(DgnModelId modelId) const {return m_viewedModels.Contains(modelId);}
    DGNPLATFORM_EXPORT void LookAtViewAlignedVolume(DRange3dCR volume, double const* aspectRatio=nullptr, MarginPercent const* margin=nullptr, bool expandClippingPlanes=true);
    void SaveToSettings(JsonValueR val) const {_SaveToSettings(val);}
    void RestoreFromSettings(JsonValueCR val) {_RestoreFromSettings(val);}

public:
    DgnClassId GetClassId() const {return m_classId;}

    //! Get the DgnDb of this view.
    DgnDbR GetDgnDb() const {return _GetDgnDb();}

    //! Get the union of the range (axis-aligned bounding box) of all physical elements in project
    AxisAlignedBox3d GetViewedExtents() const {return _GetViewedExtents();}

    //! Load the settings of this view from persistent settings in the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Load();

    //! Save the settings of this view to persistent settings in the database.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Save();

    //! Save the current state of this ViewController to a new view name. After this call succeeds, this ViewController is
    //! directed at the new view, and the previous view's state is unchanged.
    //! @param[in] newName The name for the new view. Must be unique.
    //! @return BE_SQLITE_OK if the view was successfully saved, error code otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveAs(Utf8CP newName);

    //! Save the current state of this ViewController to a new view name. After this call succeeds, the new view will exist in the
    //! database with this ViewController's state, but this ViewController remains directed at the existing view (future calls to Save
    //! will be written to the existing not new view). However, the current state will not have been saved.
    //! @param[in] newName The name for the new view. Must be unique.
    //! @param[out] newId On success, the DgnViewId of the newly created view.
    //! @return BE_SQLITE_OK if the view was successfully saved, error code otherwise.
    DGNPLATFORM_EXPORT BeSQLite::DbResult SaveTo(Utf8CP newName, DgnViewId& newId);

    //! perform the equivalent of a dynamic_cast to a PhysicalViewController.
    //! @return a valid PhysicalViewControllerCP, or nullptr if this is not a physical view
    virtual PhysicalViewControllerCP _ToPhysicalView() const {return nullptr;}
    PhysicalViewControllerP ToPhysicalViewP() {return const_cast<PhysicalViewControllerP>(_ToPhysicalView());}

    //! perform the equivalent of a dynamic_cast to a CameraViewController.
    //! @return a valid CameraViewControllerCP, or nullptr if this is not a physical view with a camera
    virtual CameraViewControllerCP _ToCameraView() const {return nullptr;}
    CameraViewControllerP ToCameraViewP() {return const_cast<CameraViewControllerP>(_ToCameraView());}

    //! perform the equivalent of a dynamic_cast to a DrawingViewController.
    //! @return a valid DrawingViewControllerCP, or nullptr if this is not a drawing view
    virtual DrawingViewControllerCP _ToDrawingView() const {return nullptr;}
    DrawingViewControllerP  ToDrawingViewP() {return const_cast<DrawingViewControllerP>(_ToDrawingView());}

    //! perform the equivalent of a dynamic_cast to a SheetViewController.
    //! @return a valid SheetViewControllerCP, or nullptr if this is not a sheet view
    virtual SheetViewControllerCP _ToSheetView() const {return nullptr;}
    SheetViewControllerP  ToSheetViewP() {return const_cast<SheetViewControllerP>(_ToSheetView());}

    //! determine whether this is a physical view
    bool IsPhysicalView() const {return nullptr != _ToPhysicalView();}

    //! determine whether this is a camera view
    bool IsCameraView() const {return nullptr != _ToCameraView();}

    //! determine whether this is a drawing view
    bool IsDrawingView() const {return nullptr != _ToDrawingView();}

    //! determine whether this is a sheet view
    bool IsSheetView() const {return nullptr != _ToSheetView();}

    //! determine whether this view has been loaded from the database.
    DGNPLATFORM_EXPORT bool IsLoaded() const;

    //! Gets a const reference to the ViewFlags.
    ViewFlagsCR GetViewFlags() const {return m_viewFlags;}

    //! Gets a reference to the ViewFlags.
    ViewFlagsR GetViewFlagsR() {return m_viewFlags;}

    //! Gets the DgnViewId of this view.
    DgnViewId GetViewId() const {return m_viewId;}

    //! Gets the background color used in the view.
    ColorDef GetBackgroundColor() const {return _GetBackgroundColor();}

    //! Adjust the aspect ratio of this view so that it matches the aspect ratio (x/y) of the supplied rectangle.
    //! @param[in] aspect The target aspect ratio.
    //! @param[in] expandView When adjusting the view to correct the aspect ratio, the one axis (x or y) must either be lengthened or shortened.
    //! if expandView is true, the shorter axis is lengthened. Otherwise the long axis is shortened.
    void AdjustAspectRatio(double aspect, bool expandView) {_AdjustAspectRatio(aspect, expandView);}

    //! Get the origin (lower, left, front) point of of the view in coordinates of the target
    //! model (physical coordinates for PhysicalViewController and drawing coordinates for DrawingViewController).
    DPoint3d GetOrigin() const {return _GetOrigin();}

    //! Get the size of the X and Y axes of this view. The axes are in world coordinates units, aligned with the view.
    DVec3d GetDelta() const {return _GetDelta();}

    //! Get the 3x3 orthonormal rotation matrix for this view.
    RotMatrix GetRotation() const {return _GetRotation();}

    //! Change the origin (lower, left, front) point of this view.
    //! @param[in] viewOrg The new origin for this view.
    void SetOrigin(DPoint3dCR viewOrg) {_SetOrigin(viewOrg);}

    //! Change the size of this view. The axes are in world coordinates units, aligned with the view.
    //! @param[in] viewDelta the new size for the view.
    void SetDelta(DVec3dCR viewDelta) {_SetDelta(viewDelta);}

    //! Change the rotation of the view.
    //! @note viewRot must be orthonormal. For 2d views, only the rotation angle about the z axis is used.
    void SetRotation(RotMatrixCR viewRot){_SetRotation(viewRot);}

    //! Get the center point of the view.
    DGNPLATFORM_EXPORT DPoint3d GetCenter() const;

    //! Get the target point of the view. If there is no camera, Center() is returned.
    DGNPLATFORM_EXPORT DPoint3d GetTargetPoint() const;

    //! Change whether a DgnCatetory is display in this view.
    //! @param[in] categoryId the DgnCategoryId to change.
    //! @param[in] onOff if true, the category is displayed in this view.
    void ChangeCategoryDisplay(DgnCategoryId categoryId, bool onOff) {_ChangeCategoryDisplay(categoryId, onOff);}

    double GetAspectRatioSkew() const {return _GetAspectRatioSkew();}

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT ColorDef ResolveBGColor() const;
    DGNPLATFORM_EXPORT bool IsViewChanged(Utf8StringCR base) const;
    DGNPLATFORM_EXPORT bool OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point);
    DGNPLATFORM_EXPORT bool OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui);
    DGNPLATFORM_EXPORT void ResetDeviceOrientation();
    DGNPLATFORM_EXPORT void OverrideSubCategory(DgnSubCategoryId, DgnSubCategory::Override const&);
    DGNPLATFORM_EXPORT void DropSubCategoryOverride(DgnSubCategoryId);
//__PUBLISH_SECTION_START__

    //! Get the set of currently displayed DgnModels for this ViewController
    DgnModelIdSet const& GetViewedModels() const {return m_viewedModels;}
    DgnModelIdSet& GetViewedModelsR() {return m_viewedModels;}

    //! Get the set of currently displayed DgnCategories for this ViewController
    DgnCategoryIdSet const& GetViewedCategories() const {return m_viewedCategories;}
    DgnCategoryIdSet& GetViewedCategoriesR() {return m_viewedCategories;}

    //! Get the Appearance of a DgnSubCategory for this view.
    //! @param[in] id the DgnSubCategoryId of interest
    //! @return the appearance of the DgnSubCategory for this view.
    DGNPLATFORM_EXPORT DgnSubCategory::Appearance GetSubCategoryAppearance(DgnSubCategoryId id) const;

    //! Change the background color of the view.
    //! @param[in] color The new background color
    DGNPLATFORM_EXPORT void SetBackgroundColor(ColorDef color);

    //! Initialize this ViewController .
    DGNPLATFORM_EXPORT void Init();

    //! Gets the DgnModel that will be the target of tools that add new elements.
    DGNPLATFORM_EXPORT DgnModelP GetTargetModel() const;

    //! Tests whether a rotation matrix corresponds to one of the StandardView orientations.
    //! @param[in] rotation  The matrix to test.
    //! @param[in] check3D   True to check the 3D members of StandardRotation.
    //! @return The standard view index.  StandardView::NotStandard indicates that rotation does not match any standard views.
    DGNPLATFORM_EXPORT static StandardView IsStandardViewRotation(RotMatrixCR rotation, bool check3D);

    //! Gets the name of a StandardView.
    //! @param[in]  standardView The StandardView of interest
    //! @return the ViewName. 
    DGNPLATFORM_EXPORT static Utf8String GetStandardViewName(StandardView standardView);

    //! Get the RotMatrix for a standard view by name.
    //! @param[out] rotMatrix   The rotation of the standard view (optional)
    //! @param[out] standardId  The identifier of the standard view (optional)
    //! @param[in]  viewName    The name of the standard view to look up. Note that the comparison is case-insensitive.
    //! @return SUCCESS if viewName was interpreted correctly and rotMatrix and standardId are valid.
    DGNPLATFORM_EXPORT static BentleyStatus GetStandardViewByName(RotMatrixP rotMatrix, StandardView* standardId, Utf8CP viewName);

    //! Change the view orientation to one of the standard views.
    //! @param[in] standardView the rotation to which the view should be set.
    //! @return SUCCESS if the view was changed.
    DGNPLATFORM_EXPORT BentleyStatus SetStandardViewRotation(StandardView standardView);

    //! @return true if this view supports 3d viewing operations. Otherwise the z-axis of the view must remain aligned with the world z axis, even
    //! if the view is a physical view.
    DGNPLATFORM_EXPORT bool Allow3dManipulations() const;

    //! Establish the view parameters from an 8-point frustum.
    //! @param[in] frustum The 8-point frustum from which to establish the parameters of this ViewController
    //! @note The order of the points in the frustum is defined by the NpcCorners enum.
    DGNPLATFORM_EXPORT ViewportStatus SetupFromFrustum(Frustum const& frustum);

    //! Change the volume that this view displays, keeping its current rotation.
    //! @param[in] worldVolume The new volume, in world-coordinates, for the view. The resulting view will show all of worldVolume, by fitting a
    //! view-axis-aligned bounding box around it. For views that are not aligned with the world coordinate system, this will sometimes
    //! result in a much larger volume than worldVolume.
    //! @param[in] aspectRatio The X/Y aspect ratio of the view into which the result will be displayed. If the aspect ratio of the volume does not
    //! match aspectRatio, the shorter axis is lengthened and the volume is centered. If aspectRatio is nullptr, no adjustment is made.
    //! @param[in] margin The amount of "white space" to leave around the view volume (which essentially increases the volume
    //! of space shown in the view.) If nullptr, no additional white space is added.
    //! @param[in] expandClippingPlanes If false, the front and back clipping planes are not moved. This is rarely desired.
    //! @note For 3d views, the camera is centered on the new volume and moved along the view z axis using the default lens angle
    //! such that the entire volume is visible.
    //! @note, for 2d views, only the X and Y values of volume are used.
    DGNPLATFORM_EXPORT void LookAtVolume(DRange3dCR worldVolume, double const* aspectRatio=nullptr, MarginPercent const* margin=nullptr, bool expandClippingPlanes=true);

    //! Get the unit vector that points in the view X (left-to-right) direction.
    DVec3d GetXVector() const {DVec3d v; GetRotation().GetRow(v,0); return v;}

    //! Get the unit vector that points in the view Y (bottom-to-top) direction.
    DVec3d GetYVector() const {DVec3d v; GetRotation().GetRow(v,1); return v;}

    //! Get the unit vector that points in the view Z (front-to-back) direction.
    DVec3d GetZVector() const {DVec3d v; GetRotation().GetRow(v,2); return v;}
};

//=======================================================================================
//! A PhysicalViewControllerBase controls views of PhysicalModels.
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalViewController : ViewController
{
    DEFINE_T_SUPER(ViewController);

    friend struct  PhysicalRedlineViewController;

protected:
    DPoint3d        m_origin;           //!< The lower left back corner of the view frustum.
    DVec3d          m_delta;            //!< The extent of the view frustum.
    RotMatrix       m_rotation;         //!< Rotation of the view frustum.
    DgnStyleId      m_displayStyleId;   //!< The display style id of the view

    //  Non-persistent data
    IAuxCoordSysPtr     m_auxCoordSys;      //!< The auxiliary coordinate system in use.

    virtual PhysicalViewControllerCP _ToPhysicalView() const override {return this;}
    virtual ClipVectorPtr _GetClipVector() const {return nullptr;}

    DGNPLATFORM_EXPORT virtual void _AdjustAspectRatio(double, bool expandView) override;
    virtual DPoint3d _GetOrigin() const override {return m_origin;}
    virtual DVec3d _GetDelta() const override {return m_delta;}
    virtual RotMatrix _GetRotation() const override {return m_rotation;}
    virtual void _SetOrigin(DPoint3dCR origin) override {m_origin = origin;}
    virtual void _SetDelta(DVec3dCR delta) override {m_delta = delta;}
    virtual void _SetRotation(RotMatrixCR rot) override {m_rotation = rot;}
    virtual bool _Allow3dManipulations() const override {return true;}
    DGNPLATFORM_EXPORT virtual IAuxCoordSysP _GetAuxCoordinateSystem() const;
    DGNPLATFORM_EXPORT virtual bool _OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) override;
    DGNPLATFORM_EXPORT virtual bool _OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui) override;
    DGNPLATFORM_EXPORT virtual void _OnTransform(TransformCR);
    DGNPLATFORM_EXPORT virtual void _SaveToSettings(JsonValueR) const override;
    DGNPLATFORM_EXPORT virtual void _RestoreFromSettings(JsonValueCR) override;

public:
    DGNPLATFORM_EXPORT bool ViewVectorsFromOrientation(DVec3dR forward, DVec3dR up, RotMatrixCR orientation, OrientationMode mode, UiOrientation ui);

    //! Construct a new PhysicalViewController from a View in the project.
    //! @param[in] project the project for which this PhysicalViewController applies.
    //! @param[in] viewId the id of the view in the project.
    DGNPLATFORM_EXPORT PhysicalViewController(DgnDbR project, DgnViewId viewId);

    DGNPLATFORM_EXPORT ClipVectorPtr GetClipVector() const;
    DGNPLATFORM_EXPORT void TransformBy(TransformCR);

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static double CalculateMaxDepth(DVec3dCR delta, DVec3dCR zVec);

//__PUBLISH_SECTION_START__

    //! Sets the display style by-index. This display style must thus be in the file.
    //! @note You can override this display style with a display style object at the DgnViewport level if desired.
    //! @see SpecialDisplayStyleIndex for special indices (e.g. to remove a display style).
    DGNPLATFORM_EXPORT void SetDisplayStyle(DgnStyleId);

    //! Gets the Auxiliary Coordinate System for this view.
    DGNPLATFORM_EXPORT IAuxCoordSysP GetAuxCoordinateSystem() const;

    //! Sets the Auxiliary Coordinate System to use for this view.
    //! @param[in] acs The new Auxiliary Coordinate System.
    DGNPLATFORM_EXPORT void SetAuxCoordinateSystem(IAuxCoordSysP acs);

    //! Sets the Target DgnModel for this PhysicalViewController.
    //! @param[in] target The model to which new elements are added by modification tools.
    DGNPLATFORM_EXPORT BentleyStatus SetTargetModel(DgnModelP target);
};

/** @addtogroup DgnViewGroup
<h4>%PhysicalViewController Camera</h4>

 This is what the parameters to the camera methods, and the values stored by CameraViewController mean.
@verbatim
               v-- {origin}
          -----+-------------------------------------- -   [back plane]
          ^\   .                                    /  ^
          | \  .                                   /   |        p
        d |  \ .                                  /    |        o
        e |   \.         {targetPoint}           /     |        i
        l |    |---------------+----------------|      |        t    [focus plane]
        t |     \  ^delta.x    ^               /     b |        i
        a |      \             |              /      a |        v
        . |       \            |             /       c |        e
        z |        \           | f          /        k |        Z
          |         \          | o         /         D |        |
          |          \         | c        /          i |        |
          |           \        | u       /           s |        v
          v            \       | s      /            t |
          -     -       -----  | D -----               |   [front plane]
                ^              | i                     |
                |              | s                     |
    frontDist ->|              | t                     |
                |              |                       |
                v           \  v  / <- lens angle      v
                -              + {eyePoint}            -     positiveX ->

@endverbatim

   Notes:
         - Up vector (positiveY) points out of the screen towards you in this diagram. Likewise delta.y.
         - The view origin is in world coordinates. It is the point at the lower left of the rectangle at the
           focus plane, projected onto the back plane.
         - [delta.x,delta.y] are on the focus plane and delta.z is from the back plane to the front plane.
         - The three view vectors come from:
@verbatim
                {vector from eyePoint->targetPoint} : -Z (positive view Z points towards negative world Z)
                {the up vector}                     : +Y
                {Z cross Y}                         : +X
@endverbatim
           these three vectors form the rows of the view's RotMatrix
         - Objects in space in front of the front plane or behind the back plane are not displayed.
         - The focus plane is not necessarily centered between the front plane and back plane (though it often is). It should generally be
           between the front plane and the back plane.
         - targetPoint is not stored in the view parameters. Instead it may be derived from
           {origin},{eyePoint},[RotMatrix] and focusDist.
         - The view volume is completely specified by: @verbatim {origin}<delta>[RotMatrix] @endverbatim
         - Perspective is determined by {eyePoint}, which is independent of the view volume. Sometimes the eyepoint is not centered
           on the rectangle on the focus plane (that is, a vector from the eyepoint along the viewZ does not hit the view center.)
           This creates a 1-point perspective, which can be disconcerting. It is usually best to keep the camera centered.
         - Cameras hold a "lens angle" value which is defines the field-of-view for the camera in radians.
           The lens angle value is not used to compute the perspective transform for a view. Instead, the len angle value
           can be used to reposition {eyePoint} when the view volume or target changes.
         - View volumes where one dimension is very small or large relative to the other dimensions (e.g. "long skinny telescope" views,
           or "wide and shallow slices", etc.) are problematic and disallowed based on ratio limits.
*/

//=======================================================================================
//! A CameraViewController is used to control views of PhysicalModels. A CameraViewController
//! may have a camera enabled that displays world-coordinate geometry onto the image plane through a perspective projection.
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraViewController : PhysicalViewController
{
    DEFINE_T_SUPER(PhysicalViewController);

    bool            m_isCameraOn;       //!< if true, m_camera is valid.
    CameraInfo      m_camera;           //!< Information about the camera lens used for the view.
    ClipVectorPtr   m_clipVector;       //!< The clip currently applied to this view

protected:
    //! Calculate and save the lens angle formed by the current delta and focus distance
    void CalculateLensAngle();

    virtual CameraViewControllerCP _ToCameraView() const override {return this;}
    DGNPLATFORM_EXPORT virtual void _OnTransform(TransformCR) override;
    DGNPLATFORM_EXPORT virtual DPoint3d _GetTargetPoint() const override;
    DGNPLATFORM_EXPORT virtual bool _OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) override;
    DGNPLATFORM_EXPORT virtual bool _OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui) override;
    DGNPLATFORM_EXPORT virtual ClipVectorPtr _GetClipVector() const override;
    DGNPLATFORM_EXPORT virtual ViewportStatus _SetupFromFrustum(Frustum const&) override;
    DGNPLATFORM_EXPORT virtual void _SaveToSettings(JsonValueR) const override;
    DGNPLATFORM_EXPORT virtual void _RestoreFromSettings(JsonValueCR) override;

public:
    void VerifyFocusPlane();

    //! Construct a new CameraViewController
    //! @param[in] dgndb The DgnDb of this view
    //! @param[in] viewId the id of the view in the project.
    DGNPLATFORM_EXPORT CameraViewController(DgnDbR dgndb, DgnViewId viewId);

/** @name Camera */
/** @{ */
    //! Determine whether the camera is on for this view
    bool IsCameraOn() const {return m_isCameraOn;}

    //! Determine whether the camera is valid for this view
    bool IsCameraValid() const {return m_camera.IsValid();}

    //! Turn the camera on or off for this view
    //! @param[in] val whether the camera is to be on or off
    void SetCameraOn(bool val) {m_isCameraOn = val;}

    //! Get a reference to the CameraInfo for this view.
    CameraInfo& GetControllerCameraR() {return m_camera;}

    //! Get a const reference to the CameraInfo for this view.
    CameraInfo const& GetControllerCamera() const {return m_camera;}

    //! Position the camera for this view and point it at a new target point.
    //! @param[in] eyePoint The new location of the camera.
    //! @param[in] targetPoint The new location to which the camera should point. This becomes the center of the view on the focus plane.
    //! @param[in] upVector A vector that orients the camera's "up" (view y). This vector must not be parallel to the vector from eye to target.
    //! @param[in] viewDelta The new size (width and height) of the view rectangle. The view rectangle is on the focus plane centered on the targetPoint.
    //! If viewDelta is nullptr, the existing size is unchanged.
    //! @param[in] frontDistance The distance from the eyePoint to the front plane. If nullptr, the existing front distance is used.
    //! @param[in] backDistance The distance from the eyePoint to the back plane. If nullptr, the existing back distance is used.
    //! @return a status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note If the aspect ratio of viewDelta does not match the aspect ratio of a DgnViewport into which this view is displayed, it will be
    //! adjusted when the DgnViewport is synchronized from this view.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus LookAt(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVector,
                                             DVec2dCP viewDelta=nullptr, double const* frontDistance=nullptr, double const* backDistance=nullptr);

    //! Position the camera for this view and point it at a new target point, using a specified lens angle.
    //! @param[in] eyePoint The new location of the camera.
    //! @param[in] targetPoint The new location to which the camera should point. This becomes the center of the view on the focus plane.
    //! @param[in] upVector A vector that orients the camera's "up" (view y). This vector must not be parallel to the vector from eye to target.
    //! @param[in] fov The angle, in radians, that defines the field-of-view for the camera. Must be between .0001 and pi.
    //! @param[in] frontDistance The distance from the eyePoint to the front plane. If nullptr, the existing front distance is used.
    //! @param[in] backDistance The distance from the eyePoint to the back plane. If nullptr, the existing back distance is used.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note The aspect ratio of the view remains unchanged.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus LookAtUsingLensAngle(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVector,
                                             double fov, double const* frontDistance=nullptr, double const* backDistance=nullptr);

    //! Move the camera relative to its current location by a distance in camera coordinates.
    //! @param[in] distance to move camera. Length is in world units, direction relative to current camera orientation.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus MoveCameraLocal(DVec3dCR distance);

    //! Move the camera relative to its current location by a distance in world coordinates.
    //! @param[in] distance in world units.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus MoveCameraWorld(DVec3dCR distance);

    //! Rotate the camera from its current location about an axis relative to its current orientation.
    //! @param[in] angle The angle to rotate the camera, in radians.
    //! @param[in] axis The axis about which to rotate the camera. The axis is a direction relative to the current camera orientation.
    //! @param[in] aboutPt The point, in world coordinates, about which the camera is rotated. If aboutPt is nullptr, the camera rotates in place
    //! (i.e. about the current eyePoint).
    //! @note Even though the axis is relative to the current camera orientation, the aboutPt is in world coordinates, \b not relative to the camera.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus RotateCameraLocal(double angle, DVec3dCR axis, DPoint3dCP aboutPt=nullptr);

    //! Rotate the camera from its current location about an axis in world coordinates.
    //! @param[in] angle The angle to rotate the camera, in radians.
    //! @param[in] axis The world-based axis (direction) about which to rotate the camera.
    //! @param[in] aboutPt The point, in world coordinates, about which the camera is rotated. If aboutPt is nullptr, the camera rotates in place
    //! (i.e. about the current eyePoint).
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    DGNPLATFORM_EXPORT ViewportStatus RotateCameraWorld(double angle, DVec3dCR axis, DPoint3dCP aboutPt=nullptr);

    //! Get the distance from the eyePoint to the back plane for this view.
    DGNPLATFORM_EXPORT double GetBackDistance() const;

    //! Place the eyepoint of the camera so it is centered in the view. This removes any 1-point perspective skewing that may be
    //! present in the current view.
    //! @param[in] backDistance optional, If not nullptr, the new the distance from the eyepoint to the back plane. Otherwise the distance from the
    //! current eyepoint is used.
    DGNPLATFORM_EXPORT void CenterEyePoint(double const* backDistance=nullptr);

    //! Center the focus distance of the camera halfway between the front plane and the back plane, keeping the eyepont,
    //! lens angle, rotation, back distance, and front distance unchanged.
    //! @note The focus distance, origin, and delta values are modified, but the view encloses the same volume and appears visually unchanged.
    DGNPLATFORM_EXPORT void CenterFocusDistance();

    //! Get the distance from the eyePoint to the front plane for this view.
    double GetFrontDistance() const {return GetBackDistance() - GetDelta().z;}

    //! Get the lens angle for this view.
    double GetLensAngle() const {return m_camera.GetLensAngle();}

    //! Set the lens angle for this view.
    //! @param[in] angle The new lens angle in radians. Must be greater than 0 and less than pi.
    //! @note This does not change the view's current field-of-view. Instead, it changes the lens that will be used if the view
    //! is subsequently modified and the lens angle is used to position the eyepoint.
    //! @note To change the field-of-view (i.e. "zoom") of a view, pass a new viewDelta to #LookAt
    void SetLensAngle(double angle) {m_camera.SetLensAngle(angle);}

    //! Get the distance from the eyePoint to the focus plane for this view.
    double GetFocusDistance() const {return m_camera.GetFocusDistance();}

    //! Set the focus distance for this view.
    //! @note Changing the focus distance changes the plane on which the delta.x and delta.y values lie. So, changing focus distance
    //! without making corresponding changes to delta.x and delta.y essentially changes the lens angle, causing a "zoom" effect.
    void SetFocusDistance(double dist) {m_camera.SetFocusDistance(dist);}

    //! Get the current location of the eyePoint for camera in this view.
    DPoint3dCR GetEyePoint() const {return m_camera.GetEyePoint();}

    //! Change the location of the eyePoint for the camera in this view.
    //! @param[in] pt The new eyepoint.
    //! @note This method is generally for internal use only. Moving the eyePoint arbitrarily can result in skewed or illegal perspectives.
    //! The most common method for user-level camera positioning is #LookAt.
    void SetEyePoint(DPoint3dCR pt) {m_camera.SetEyePoint(pt);}
/** @} */

/** @name ClipVector */
/** @{ */
    DGNPLATFORM_EXPORT void SetClipVector(ClipVectorR);
    DGNPLATFORM_EXPORT void ClearClipVector();
/** @} */
};

//=======================================================================================
//! A SectioningViewController is a physical view with a clip that defines a section cut.
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SectioningViewController : PhysicalViewController
{
    DEFINE_T_SUPER(PhysicalViewController);

protected:
    IViewClipObjectPtr m_clip; // a SectionClipObject
    mutable bool m_hasAnalyzedCutPlanes;
    mutable uint32_t m_cutPlaneCount;
    mutable uint32_t m_foremostCutPlaneIndex;
    mutable DPlane3d m_foremostCutPlane;
    ClipVolumePass m_pass;

    DGNPLATFORM_EXPORT virtual ClipVectorPtr _GetClipVector() const override;

    DGNPLATFORM_EXPORT virtual void _DrawView(ViewContextR) override;
    DGNPLATFORM_EXPORT virtual void _DrawElement(ViewContextR, GeometrySourceCR) override;
    DGNPLATFORM_EXPORT virtual void _SaveToSettings(JsonValueR) const override;
    DGNPLATFORM_EXPORT virtual void _RestoreFromSettings(JsonValueCR) override;

    void SetOverrideMatSymb(ViewContextR) const;
    void DrawViewInternal(ViewContextR);
    ClipVectorPtr GetClipVectorInternal(ClipVolumePass) const;

    void AnalyzeCutPlanes() const;

public:
    void SetClip(IViewClipObject& clip) {m_clip = &clip;}

    //! Construct a new SectioningViewController.
    //! @remarks This constructor is normally used only as part of creating a new view in the project.
    //! @remarks Use this constructor only to create a new camera view controller. To load an existing view controller,
    //! call PhysicalViewController::Create.
    //! @param[in] project the project for which this SectioningViewController applies.
    //! @param[in] viewId the id of the view in the project.
    DGNPLATFORM_EXPORT SectioningViewController(DgnDbR project, DgnViewId viewId);

    DGNPLATFORM_EXPORT DPlane3d GetForemostCutPlane() const;

    DGNPLATFORM_EXPORT bool HasDogLeg() const;

    ClipVectorPtr GetInsideForwardClipVector() const {return GetClipVectorInternal(ClipVolumePass::InsideForward);}
};

//=======================================================================================
//! A ViewController2d is used to control views of 2d models.
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewController2d : ViewController
    {
    DEFINE_T_SUPER(ViewController);

protected:
    DPoint2d    m_origin;       //!< The lower left front corner of the view frustum.
    DVec2d      m_delta;        //!< The extent of the view frustum.
    double      m_rotAngle;     //!< Rotation of the view frustum.

    DGNPLATFORM_EXPORT virtual void _AdjustAspectRatio(double, bool expandView) override;
    DGNPLATFORM_EXPORT virtual DPoint3d _GetOrigin() const override;
    DGNPLATFORM_EXPORT virtual DVec3d _GetDelta() const override;
    DGNPLATFORM_EXPORT virtual RotMatrix _GetRotation() const override;
    DGNPLATFORM_EXPORT virtual void _SetOrigin(DPoint3dCR org) override;
    DGNPLATFORM_EXPORT virtual void _SetDelta(DVec3dCR delta) override;
    DGNPLATFORM_EXPORT virtual void _SetRotation(RotMatrixCR rot) override;
    DGNPLATFORM_EXPORT virtual void _SaveToSettings(JsonValueR) const override;
    DGNPLATFORM_EXPORT virtual void _RestoreFromSettings(JsonValueCR) override;

public:
    ViewController2d(DgnDbR project, DgnViewId viewId) : ViewController(project, viewId) {}
    double GetRotAngle() const {return m_rotAngle;}
    DPoint2d GetOrigin2d() const {return m_origin;}
    DVec2d GetDelta2d() const {return m_delta;}
    };

//=======================================================================================
//! A DrawingViewController is used to control views of DrawingModel's
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingViewController : ViewController2d
{
    DEFINE_T_SUPER(ViewController2d);

    virtual DrawingViewControllerCP _ToDrawingView() const override {return this;}
    DGNPLATFORM_EXPORT virtual bool _OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) override;

public:
    //! Construct a new DrawingViewController.
    DrawingViewController(DgnDbR project, DgnViewId viewId) : ViewController2d(project, viewId) {}
};

//=======================================================================================
//! A SectionDrawingViewController is used to control views of SectionDrawingModels
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SectionDrawingViewController : DrawingViewController
{
    DEFINE_T_SUPER(DrawingViewController);

protected:
    DGNPLATFORM_EXPORT virtual void _DrawView(ViewContextR) override;
    DGNPLATFORM_EXPORT virtual void _DrawElement(ViewContextR, GeometrySourceCR) override;
    DGNPLATFORM_EXPORT virtual StatusInt _VisitHit(HitDetailCR hit, ViewContextR context) const override;

    mutable SectioningViewControllerPtr m_sectionView;  // transient

public:
    //! Convenience method to get the drawing that is displayed in this view.
    SectionDrawingModel* GetSectionDrawing() const {return dynamic_cast<SectionDrawingModel*>(GetTargetModel());}

    //! Convenience method to query the source section `view
    DGNPLATFORM_EXPORT SectioningViewControllerPtr GetSectioningViewController() const;

    //! Convenience method to ask the section view for its "forward" clip vector. That is how to clip a physical model view in order to display this view in context.
    DGNPLATFORM_EXPORT ClipVectorPtr GetProjectClipVector() const; //!< Get the clip to apply to a physical view when drawing this view embedded in it. Never returns nullptr. May create a temporary (empty) clip if there is no clip defined.

    //! Convenience method to ask the section view has multiple section planes.
    DGNPLATFORM_EXPORT bool GetSectionHasDogLeg() const; //!< Returns true if the drawing has a dog leg
};

//=======================================================================================
//! A HypermodelingViewController is used to display zero or more drawing models in the context of one physical view.
//! A HypermodelingViewController tries to mimic what the underlying physical view controller would do. The view frustum,
//! camera, render mode, etc. are all the same. HypermodelingViewController passes on changes to the view frustum, categories on/off,
//! etc. to the underlying physical view controller, so that, if the HypermodelingViewController is deleted and the physical
//! view controller is restored, the view will not changed.
//! <p>Note that a hypermodeling view is not a persistent concept. It is created at runtime to compose other views.
//! @ingroup DgnViewGroup
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HypermodelingViewController : PhysicalViewController
{
    DEFINE_T_SUPER(PhysicalViewController);

    //! Specifies symbology for some aspects of the drawings when they are drawn in context.
    struct DrawingSymbology
        {
        ColorDef  drawingBackgroundColor; //!< The background color of the drawing
        ColorDef  hatchColor; //!< The color of the fills where section cuts occur on planes other than the section plane closest to the eye.
        };

    //! Specifies drawing elements to draw.
    enum Pass
        {
        PASS_None        = 0,   //!< No drawing graphics specified
        PASS_Cut         = 2,   //!< Draw section cut graphics, including edges and fills
        PASS_Forward     = 4,   //!< Draw section forward graphics,
        PASS_Annotation  = 8,   //!< Draw annotations other than section graphics
        PASS_Hatch       = 16,  //!< Draw fills where section cuts occur on planes other than the section plane closest to the eye.
        PASS_DrawingBackground = 32,  //!< Draw a sheet border around drawing graphics
        PASS_CutOrAnnotation = (PASS_Cut|PASS_Annotation),
        PASS_ForPicking  = (PASS_Cut|PASS_Forward|PASS_Annotation),
        PASS_All  = 0xff
        };

private:
    PhysicalViewControllerPtr m_physical;
    bvector<SectionDrawingViewControllerPtr> m_drawings;
    ViewControllerP m_currentViewController;
    bvector<DRange3d> m_unused_unused_unused_unused_unused_unused_unused;
    mutable Pass m_pass;
    int m_nearestCutPlane;
    DrawingSymbology m_symbology;
    Pass m_passesToDraw;

    virtual void _DrawView(ViewContextR) override;
    virtual void _DrawElement(ViewContextR, GeometrySourceCR) override;
    virtual StatusInt _VisitHit(HitDetailCR hit, ViewContextR context) const override;
    virtual DPoint3d _GetOrigin() const override;
    virtual DVec3d _GetDelta() const override;
    virtual RotMatrix _GetRotation() const override;
    virtual void _SetOrigin(DPoint3dCR org) override;
    virtual void _SetDelta(DVec3dCR delta) override;
    virtual void _SetRotation(RotMatrixCR rot) override;
    virtual DgnModelP _GetTargetModel() const override;
    virtual void _AdjustAspectRatio(double , bool expandView) override;
    virtual DPoint3d _GetTargetPoint() const override;
    virtual bool _Allow3dManipulations() const override;
    virtual AxisAlignedBox3d _GetViewedExtents() const override;
    virtual IAuxCoordSysP _GetAuxCoordinateSystem() const override;
    virtual ColorDef _GetBackgroundColor() const override;
    virtual ClipVectorPtr _GetClipVector() const override;

    void PushClipsForPhysicalView(ViewContextR) const;
    void PopClipsForPhysicalView(ViewContextR) const;
    void PushClipsForInContextViewPass(ViewContextR context, SectionDrawingViewControllerCR drawing) const;
    void PopClipsForInContextViewPass(ViewContextR context, SectionDrawingViewControllerCR drawing) const;

    DRange3d GetDrawingRange(DrawingViewControllerR) const;
    void DrawFakeSheetBorder(ViewContextR, DrawingViewControllerR) const;

    bool ShouldDraw(Pass p) const {return (m_passesToDraw & p) == p;}

public:
    DGNPLATFORM_EXPORT HypermodelingViewController(DgnViewId, PhysicalViewControllerR, bvector<SectionDrawingViewControllerPtr> const&);
    bool ShouldDrawProxyGraphics(ClipVolumePass proxyGraphicsType, int planeIndex) const;
    bool ShouldDrawAnnotations() const;
    DGNPLATFORM_EXPORT void SetOverrideMatSymb(ViewContextR) const;

    DGNPLATFORM_EXPORT bvector<SectionDrawingViewControllerPtr> GetSectionDrawingViews() const;
    DGNPLATFORM_EXPORT SectionDrawingViewControllerPtr FindSectionDrawingViewById(DgnViewId) const;
    DGNPLATFORM_EXPORT BentleyStatus AddDrawing(SectionDrawingViewControllerR);
    DGNPLATFORM_EXPORT BentleyStatus RemoveDrawing(DgnViewId);
    DGNPLATFORM_EXPORT PhysicalViewControllerR GetPhysicalView() const;
    DrawingSymbology GetDrawingSymbology() const {return m_symbology;} //!< Get the symbology for some aspects of the drawings when they are drawn in context.
    void SetDrawingSymbology(DrawingSymbology const& s) {m_symbology=s;} //!< Set the symbology for some aspects of the drawings when they are drawn in context.
    Pass GetPassesToDraw() const {return m_passesToDraw;} //!< Get the drawing elements to draw.
    void SetPassesToDraw(Pass p) {m_passesToDraw = p;} //!< Set the drawing elements to draw.
};

//=======================================================================================
//! A SheetViewController is used to control views of SheetModels
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct SheetViewController : ViewController2d
{
    DEFINE_T_SUPER(ViewController2d);

protected:
    virtual SheetViewControllerCP _ToSheetView() const override {return this;}

public:
    //! Construct a new SheetViewController.
    SheetViewController(DgnDbR project, DgnViewId viewId) : ViewController2d(project, viewId) {}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
