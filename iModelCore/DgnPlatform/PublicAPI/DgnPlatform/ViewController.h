/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ViewController.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnPlatform.h"
#include "IAuxCoordSys.h"
#include "ViewContext.h"
#include "SectionClip.h"
#include "UpdatePlan.h"
#include "DgnView.h"
#include <Bentley/BeThread.h>
#include <BeSQLite/RTreeMatch.h>

DGNPLATFORM_TYPEDEFS(FitViewParams)
DGNPLATFORM_TYPEDEFS(HypermodelingViewController)
DGNPLATFORM_TYPEDEFS(SectionDrawingViewController)
DGNPLATFORM_TYPEDEFS(SectioningViewController)

DGNPLATFORM_REF_COUNTED_PTR(HypermodelingViewController)
DGNPLATFORM_REF_COUNTED_PTR(SectionDrawingViewController)
DGNPLATFORM_REF_COUNTED_PTR(SectioningViewController)

BEGIN_BENTLEY_DGN_NAMESPACE

enum class OrientationError
{
    None,
    Unknown,
    DeviceRequiresMovement, //! Orientation events are meaningless until the user moves the device
    TrueNorthNotAvailable,  //! True north requested but available, possibly because wifi is not enabled.
    NotAvailable,
};

enum class OrientationMode
{
    CompassHeading  = 0,    //!< Use compass heading from device
    RelativeHeading = 1,    //!< Use heading relative to device's default orientation
    IgnoreHeading   = 2,    //!< Do not modify orientation from device heading
};

enum class UiOrientation
{
    Portrait            = 0,    //!< Up vector is positive device y, right vector is positive device x
    LandscapeRight      = 1,    //!< Up vector is positive device x, right vector is negative device y
    PortraitUpsideDown  = 2,    //!< Up vector is negative device y, right vector is negative device x
    LandscapeLeft       = 3,    //!< Up vector is negative device x, right vector is positive device y
};

//=======================================================================================
//! @ingroup GROUP_DgnView
/**
 A ViewController provides the behavior for a type of view. It also provides the persistent information
 about how the view relates to a DgnDb (e.g. what models/categories are displayed, the ViewFlags that control how graphics
 are represented, etc.)
 <p>
 When a ViewController is paired with a DgnViewport, it then controls the operation of that view. Generally there will
 be a 1-1 relationship between ViewControllers and Viewports and a DgnViewport holds a reference-counted-pointer to its 
 ViewController. See discussion at #DgnViewport about synchronizing Viewports and ViewControllers.
 <p>
 By overriding virtual methods, subclasses of ViewController may:
     - load and save settings from the database
     - customize the set of elements that are displayed in the view
     - customize the way the graphics for an individual element appear
     - draw non-persistent graphics into the view
     - draw "decorations" on top of the normal graphics
     - etc.

<h3>Defining a subclass of ViewController</h3>
To create a subclass of ViewController, create a ViewDefinition and implement _SupplyController.
*/
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewController : RefCountedBase
{
    friend struct SceneQueue::Task;
    friend struct CreateSceneTask;

    struct EXPORT_VTABLE_ATTRIBUTE AppData : RefCountedBase
    {
        //! A unique identifier for this type of AppData. Use a static instance of this class to identify your AppData.
        struct Key : NonCopyableClass {};

        virtual void _Save(ViewDefinitionR view) const {}
        virtual void _Load(ViewDefinitionR view) {}
    };

    //=======================================================================================
    //! The Ids of elements that are somehow treated specially for a SpatialViewController
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct SpecialElements
    {
        DgnElementIdSet m_always;
        DgnElementIdSet m_never;
        bool IsEmpty() const {return m_always.empty() && m_never.empty();}
    };

    //! Holds the results of a query.
    struct QueryResults
    {
        typedef bmultimap<double, DgnElementId> OcclusionScores;
        bool m_incomplete = false;
        OcclusionScores m_scores;
        uint32_t GetCount() const {return (uint32_t) m_scores.size();}
    };

    //=======================================================================================
    // The set of DgnElements that are contained in a scene. This is used when performing a progressive
    // update or heal of a view to determine which elements are already visible.
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct Scene : RefCountedBase, NonCopyableClass
    {
        bset<DgnElementId> m_members;
        Render::GraphicListPtr m_graphics;
        ProgressiveTaskPtr m_progressive;
        double m_lowestScore = 0.0;
        uint32_t m_progressiveTotal = 0;
        bool m_complete = false;
        bool Contains(DgnElementId id) const {return m_members.find(id) != m_members.end();}
        ~Scene() {}
    };
    typedef RefCountedPtr<Scene> ScenePtr;

protected:
    friend struct ViewContext;
    friend struct DgnViewport;
    friend struct ViewManager;
    friend struct IACSManager;
    friend struct IAuxCoordSys;
    friend struct ToolAdmin;
    friend struct ViewDefinition;

    mutable BeMutex m_mutex;
    DgnDbR m_dgndb;
    DgnViewportP m_vp = nullptr;
    ViewDefinitionPtr m_definition;
    RotMatrix m_defaultDeviceOrientation;
    bool m_defaultDeviceOrientationValid = false;
    bool m_noQuery = false;
    SpecialElements m_special;
    ClipVectorPtr m_activeVolume; //!< the active volume. If present, elements inside this volume may be treated specially
    ScenePtr m_currentScene;
    ScenePtr m_readyScene;

    mutable bmap<AppData::Key const*, RefCountedPtr<AppData>, std::less<AppData::Key const*>, 8> m_appData;

    //! Construct a ViewController object.
    DGNPLATFORM_EXPORT ViewController(ViewDefinitionCR definition);

    virtual ~ViewController() {}
    enum class FitComplete {No=0, Yes=1};
    DGNPLATFORM_EXPORT virtual FitComplete _ComputeFitRange(FitContextR);
    virtual void _OnViewOpened(DgnViewportR) {}
    virtual bool _Allow3dManipulations() const {return false;}
    virtual void _OnAttachedToViewport(DgnViewportR vp) {m_vp = &vp;}
    virtual bool _Is3d() const {return false;}
    virtual GeometricModelP _GetTargetModel() const = 0;
    virtual QueryResults _QueryScene(DgnViewportR vp, UpdatePlan const& plan, SceneQueue::Task& task) = 0;
    virtual ProgressiveTaskPtr _CreateProgressive(DgnViewportR vp) = 0;
    DGNPLATFORM_EXPORT virtual void _LoadState();
    DGNPLATFORM_EXPORT virtual void _StoreState();

    //! @return true to project un-snapped points to the view's ACS plane.
    //! @note Normally true for a 3d view. A 3d digitizier supplying real z values would not want this...maybe this would be a special ViewController?
    DGNPLATFORM_EXPORT virtual bool _IsPointAdjustmentRequired(DgnViewportR vp) const;

    //! @return true to project snap points to the view's ACS plane.
    //! @note Normally true for a 3d view only when ACS plane snap is enabled.
    DGNPLATFORM_EXPORT virtual bool _IsSnapAdjustmentRequired(DgnViewportR vp, bool snapLockEnabled) const;

    //! @return true to automatically orient AccuDraw to the view's ACS plane when initially made active.
    //! @note Normally true for a view only when ACS context lock is enabled.
    DGNPLATFORM_EXPORT virtual bool _IsContextRotationRequired(DgnViewportR vp, bool contextLockEnabled) const;

    //! Display locate circle and information about the current AccuSnap/auto-locate HitDetail.
    DGNPLATFORM_EXPORT virtual void _DrawLocateCursor(DecorateContextR, DPoint3dCR, double aperture, bool isLocateCircleOn, HitDetailCP hit=nullptr);

    //! Grid display and point adjustment.
    virtual GridOrientationType _GetGridOrientationType() const {return GridOrientationType::View;}
    DGNPLATFORM_EXPORT virtual double _GetGridScaleFactor(DgnViewportR) const;
    DGNPLATFORM_EXPORT virtual void _GetGridSpacing(DgnViewportR, DPoint2dR, uint32_t& gridsPerRef) const;
    DGNPLATFORM_EXPORT virtual void _GetGridRoundingDistance(DgnViewportR, DPoint2dR roundingDistance) const;

    //! Display grid for this view.
    DGNPLATFORM_EXPORT virtual void _DrawGrid(DecorateContextR);

    //! Display view controller specific view decorations.
    virtual void _DrawDecorations(DecorateContextR) {}

    //! Locate/snap to view controller decorations.
    virtual void _PickDecorations(PickContextR) {}

    //! Called when the display of a category is turned on or off.
    //! @param[in] singleEnable true if just turned on one category; false if
    //! turned off a category or made a group of changes.
    virtual void _OnCategoryChange(bool singleEnable) {}

    //! Draw the contents of the view.
    virtual void _DrawView(ViewContextR) = 0;

    DGNPLATFORM_EXPORT void InvalidateScene();
    bool IsSceneReady() const;
    virtual void _DoHeal(HealContext&) {}

    virtual void _OverrideGraphicParams(Render::OvrGraphicParamsR, GeometrySourceCP) {}

    //! Invokes the _VisitGeometry on \a context for <em>each element</em> that is in the view.
    //! For normal views, this does the same thing as _DrawView.
    virtual void _VisitAllElements(ViewContextR context) {_DrawView(context);}

    //! Stroke a single GeometrySource through a ViewContext.
    //! An application can override _StrokeGeometry to change the symbology of a GeometrySource.
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _StrokeGeometry(ViewContextR, GeometrySourceCR, double pixelSize);

    //! Stroke a single HitDetail through a ViewContext.
    //! An application can override _StrokeHit to change how elements are flashed for auto-locate.
    DGNPLATFORM_EXPORT virtual Render::GraphicPtr _StrokeHit(ViewContextR, GeometrySourceCR, HitDetailCR);

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

    //! Get the extent of the model(s) viewed by this view
    virtual AxisAlignedBox3d _GetViewedExtents(DgnViewportCR) const = 0;

    enum class CloseMe {No=0, Yes=1};
    //! called when one or more models are deleted
    //! @return true to close this viewport
    virtual CloseMe _OnModelsDeleted(bset<Dgn::DgnModelId> const&, Dgn::DgnDbR db) {return CloseMe::No;}

    ViewDefinitionPtr CloneState() {StoreState(); return m_definition->MakeCopy<ViewDefinition>();}
    void ChangeState(ViewDefinitionCR newState) {m_definition=newState.MakeCopy<ViewDefinition>(); LoadState();}

public:
    ScenePtr UseReadyScene() {BeMutexHolder lock(m_mutex); if (!m_readyScene.IsValid()) return nullptr; std::swap(m_currentScene, m_readyScene); m_readyScene = nullptr; return m_currentScene;}
    BentleyStatus CreateScene(DgnViewportR vp, UpdatePlan const& plan, SceneQueue::Task& task);
    void RequestScene(DgnViewportR vp, UpdatePlan const& plan);
    ScenePtr GetScene() const {BeMutexHolder lock(m_mutex); return m_currentScene;}
    void DrawView(ViewContextR context) {return _DrawView(context);}
    void VisitAllElements(ViewContextR context) {return _VisitAllElements(context);}
    void OnViewOpened(DgnViewportR vp) {_OnViewOpened(vp);}
    virtual void _CreateTerrain(TerrainContextR context) {}
    virtual void _PickTerrain(PickContextR context) {}

    //! Get the DgnDb of this view.
    DgnDbR GetDgnDb() const {return m_dgndb;}

    DgnCategoryIdSet const& GetViewedCategories() const {return m_definition->GetCategorySelector().GetCategories();}

    //! Get the axis-aliged extent of all of the possible elements visible in this view. For physical views, this is the "project extents".
    AxisAlignedBox3d GetViewedExtents(DgnViewportCR vp) const {return _GetViewedExtents(vp);}

    //! Read the state of this controller from its definition elements.
    //! @see GetDefinitionR
    void LoadState() {_LoadState();}

    //! Store the state of this controller to its definition elements. @note It is up to the caller to write the definition elements to the database if that is the goal.
    //! @see SaveDefinition
    void StoreState() {_StoreState();}

    //! Save the current state of this ViewController to a new view name. After this call succeeds, this ViewController is
    //! directed at the new view, and the previous view's state is unchanged.
    //! @param[in] newName The name for the new view. Must be unique.
    //! @return SUCCESS if the view was successfully saved, error code otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus SaveAs(Utf8CP newName);

    //! Save the current state of this ViewController to a new view name. After this call succeeds, the new view will exist in the
    //! database with this ViewController's state, but this ViewController remains directed at the existing view (future calls to Save
    //! will be written to the existing not new view). However, the current state will not have been saved.
    //! @param[in] newName The name for the new view. Must be unique.
    //! @param[out] newId On success, the DgnViewId of the newly created view.
    //! @return SUCCESS if the view was successfully saved, error code otherwise.
    DGNPLATFORM_EXPORT DgnDbStatus SaveTo(Utf8CP newName, DgnViewId& newId);

    //! perform the equivalent of a dynamic_cast to a SpatialViewController.
    //! @return a valid SpatialViewControllerCP, or nullptr if this is not a physical view
    virtual SpatialViewControllerCP _ToSpatialView() const {return nullptr;}
    SpatialViewControllerP ToSpatialViewP() {return const_cast<SpatialViewControllerP>(_ToSpatialView());}

    //! perform the equivalent of a dynamic_cast to a OrthographicViewController.
    //! @return a valid OrthographicViewControllerCP, or nullptr if this is not a spatial view based on an orthographic project
    virtual OrthographicViewControllerCP _ToOrthographicView() const {return nullptr;}
    OrthographicViewControllerP ToOrthographicViewP() {return const_cast<OrthographicViewControllerP>(_ToOrthographicView());}

    //! perform the equivalent of a dynamic_cast to a CameraViewController.
    //! @return a valid CameraViewControllerCP, or nullptr if this is not a spatial view with a camera
    virtual CameraViewControllerCP _ToCameraView() const {return nullptr;}
    CameraViewControllerP ToCameraViewP() {return const_cast<CameraViewControllerP>(_ToCameraView());}

    //! perform the equivalent of a dynamic_cast to a DrawingViewController.
    //! @return a valid DrawingViewControllerCP, or nullptr if this is not a drawing view
    virtual DrawingViewControllerCP _ToDrawingView() const {return nullptr;}
    DrawingViewControllerP ToDrawingViewP() {return const_cast<DrawingViewControllerP>(_ToDrawingView());}

    //! perform the equivalent of a dynamic_cast to a SheetViewController.
    //! @return a valid SheetViewControllerCP, or nullptr if this is not a sheet view
    virtual Sheet::ViewControllerCP _ToSheetView() const {return nullptr;}
    Sheet::ViewControllerP ToSheetViewP() {return const_cast<Sheet::ViewControllerP>(_ToSheetView());}

    //! determine whether this view is a 3d view
    bool Is3d() const {return _Is3d();}

    //! determine whether this is a physical view
    bool IsSpatialView() const {return nullptr != _ToSpatialView();}

    //! determine whether this is a camera view
    bool IsCameraView() const {return nullptr != _ToCameraView();}

    //! determine whether this is a drawing view
    bool IsDrawingView() const {return nullptr != _ToDrawingView();}

    //! determine whether this is a sheet view
    bool IsSheetView() const {return nullptr != _ToSheetView();}

    //! Get the ViewFlags from the DisplayStyle of this view
    Render::ViewFlags GetViewFlags() const {return m_definition->GetDisplayStyle().GetViewFlags();}

    //! Gets the DgnViewId of the ViewDefinition of this view.
    DgnViewId GetViewId() const {return m_definition->GetViewId();}

    //! Gets the background color in the DisplayStyle of this view.
    ColorDef GetBackgroundColor() const {return m_definition->GetDisplayStyle().GetBackgroundColor();}

    //! Change the background color in the DisplayStyle of this view.
    //! @param[in] color The new background color
    void SetBackgroundColor(ColorDef color) {m_definition->GetDisplayStyle().SetBackgroundColor(color);}

    //! Get the origin (lower, left, back) point of of the ViewDefinition
    DPoint3d GetOrigin() const {return m_definition->GetOrigin();}

    //! Get the size of the X and Y axes of this view. The axes are in world coordinates units, aligned with the ViewDefinition.
    DVec3d GetDelta() const {return m_definition->GetExtents();}

    //! Get the 3x3 orthonormal rotation matrix for this ViewDefinition.
    RotMatrix GetRotation() const {return m_definition->GetRotation();}

    //! Change the origin (lower, left, front) in the ViewDefinition of this view
    //! @param[in] viewOrg The new origin for this view.
    void SetOrigin(DPoint3dCR viewOrg) {m_definition->SetOrigin(viewOrg);}

    //! Change the extents in the ViewDefinition of this view.
    //! @param[in] viewDelta the new size for the view.
    void SetDelta(DVec3dCR viewDelta) {m_definition->SetExtents(viewDelta);}

    //! Change the rotation in the ViewDefinition of this view.
    //! @param[in] viewRot The new rotation matrix.
    //! @note viewRot must be orthonormal. For 2d views, only the rotation angle about the z axis is used.
    void SetRotation(RotMatrixCR viewRot){m_definition->SetRotation(viewRot);}

    //! Get the center point from the ViewDefinition.
    DPoint3d GetCenter() const {return m_definition->GetCenter();}

    //! Change whether a DgnCatetory is displayed in the CategorySelector of this view.
    //! @param[in] categoryId the DgnCategoryId to change.
    //! @param[in] onOff if true, the category is displayed in this view.
    DGNPLATFORM_EXPORT void ChangeCategoryDisplay(DgnCategoryId categoryId, bool onOff);

    bool OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) {return _OnGeoLocationEvent(status, point);}
    DGNPLATFORM_EXPORT bool OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui, uint32_t nEventsSinceEnabled);
    DGNPLATFORM_EXPORT void ResetDeviceOrientation();
    DGNPLATFORM_EXPORT void PointToStandardGrid(DgnViewportR, DPoint3dR point, DPoint3dCR gridOrigin, RotMatrixCR gridOrientation) const;
    DGNPLATFORM_EXPORT void PointToGrid(DgnViewportR, DPoint3dR point) const;

    //! Get the Appearance of a DgnSubCategory for this view.
    //! @param[in] id the DgnSubCategoryId of interest
    //! @return the appearance of the DgnSubCategory for this view.
    DgnSubCategory::Appearance GetSubCategoryAppearance(DgnSubCategoryId id) const {return m_definition->GetDisplayStyle().GetSubCategoryAppearance(id);}

    //! Initialize this ViewController.
    DGNPLATFORM_EXPORT void Init();

    //! determine whether a DgnModel is displayed in this view
    bool IsModelViewed(DgnModelId modelId) const {return m_definition->ViewsModel(modelId);}

    GeometricModelP GetTargetModel() const {return _GetTargetModel();}

    //! Tests whether a rotation matrix corresponds to one of the StandardView orientations.
    //! @param[in] rotation  The matrix to test.
    //! @param[in] check3D   True to check the 3D members of StandardRotation.
    //! @return The standard view index.  StandardView::NotStandard indicates that rotation does not match any standard views.
    DGNPLATFORM_EXPORT static StandardView IsStandardViewRotation(RotMatrixCR rotation, bool check3D);

    //! Gets the name of a StandardView.
    //! @param[in]  standardView The StandardView of interest
    //! @return the ViewName.
    DGNPLATFORM_EXPORT static Utf8String GetStandardViewName(StandardView standardView);

    //! Change the view orientation to one of the standard views.
    //! @param[in] standardView the rotation to which the view should be set.
    //! @return SUCCESS if the view was changed.
    BentleyStatus SetStandardViewRotation(StandardView standardView) {return m_definition->SetStandardViewRotation(standardView);}

    //! Get the RotMatrix for a standard view by name.
    //! @param[out] rotMatrix   The rotation of the standard view (optional)
    //! @param[out] standardId  The identifier of the standard view (optional)
    //! @param[in]  viewName    The name of the standard view to look up. Note that the comparison is case-insensitive.
    //! @return SUCCESS if viewName was interpreted correctly and rotMatrix and standardId are valid.
    DGNPLATFORM_EXPORT static BentleyStatus GetStandardViewByName(RotMatrixP rotMatrix, StandardView* standardId, Utf8CP viewName);

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
    void LookAtVolume(DRange3dCR worldVolume, double const* aspectRatio=nullptr, ViewDefinition::MarginPercent const* margin=nullptr, bool expandClippingPlanes=true) 
            {m_definition->LookAtVolume(worldVolume, aspectRatio, margin, expandClippingPlanes);}

    //! @return true if this view supports 3d viewing operations. Otherwise the z-axis of the view must remain aligned with the world z axis, even
    //! if the view is a physical view.
    bool Allow3dManipulations() const {return _Allow3dManipulations();}
    
    //! Establish the view parameters from an 8-point frustum.
    //! @param[in] frustum The 8-point frustum from which to establish the parameters of this ViewController
    //! @note The order of the points in the frustum is defined by the NpcCorners enum.
    DGNPLATFORM_EXPORT ViewportStatus SetupFromFrustum(Frustum const& frustum);

    AppData* FindAppData(AppData::Key const& key) const {auto entry = m_appData.find(&key); return entry==m_appData.end() ? nullptr : entry->second.get();}
    DGNPLATFORM_EXPORT void AddAppData(AppData::Key const& key, AppData* obj) const;
    StatusInt DropAppData(AppData::Key const& key) const {return 0==m_appData.erase(&key) ? ERROR : SUCCESS;}

    ViewDefinitionR GetViewDefinition() const {return *m_definition;}

    //! @name Active Volume
    //! @{
    void AssignActiveVolume(ClipVectorPtr volume) {m_activeVolume = volume;}
    void ClearActiveVolume() {m_activeVolume = nullptr;}
    ClipVectorPtr GetActiveVolume() const {return m_activeVolume;}
    //! @}

    // Get the set of special elements for this ViewController.
    SpecialElements const& GetSpecialElements() const {return m_special;}

    //! Get the list of elements that are always drawn
    DgnElementIdSet const& GetAlwaysDrawn() {return GetSpecialElements().m_always;}

    //! Establish a set of elements that are always drawn in the view.
    //! @param[in] exclusive If true, only these elements are drawn
    DGNPLATFORM_EXPORT void SetAlwaysDrawn(DgnElementIdSet const&, bool exclusive);

    //! Empty the set of elements that are always drawn
    DGNPLATFORM_EXPORT void ClearAlwaysDrawn();

    //! Establish a set of elements that are never drawn in the view.
    DGNPLATFORM_EXPORT void SetNeverDrawn(DgnElementIdSet const&);

    //! Get the list of elements that are never drawn.
    //! @remarks An element in the never-draw list is excluded regardless of whether or not it is
    //! in the always-draw list. That is, the never-draw list gets priority over the always-draw list.
    DgnElementIdSet const& GetNeverDrawn() {return GetSpecialElements().m_never;}

    //! Empty the set of elements that are never drawn
    DGNPLATFORM_EXPORT void ClearNeverDrawn();

    //! Requests that any active or pending scene queries for this view be canceled, optionally not returning until the request is satisfied
    DGNPLATFORM_EXPORT void RequestAbort(bool waitUntilFinished);
};

//=======================================================================================
//! A ViewController3d is used to control 3d views.
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewController3d : ViewController
{
    DEFINE_T_SUPER(ViewController);

protected:
    bool _Is3d() const override {return true;}
    ViewController3d(ViewDefinition3dCR definition) : T_Super(definition) {}

public:
    ViewDefinition3dCR GetViewDefinition3d() const {return static_cast<ViewDefinition3dCR>(*m_definition);}
    ViewDefinition3dR GetViewDefinition3dR() {return static_cast<ViewDefinition3dR>(*m_definition);}
};

//=======================================================================================
//! A SpatialViewController controls views of SpatialModels.
//! It shows %DgnElements selected by an SQL query that can combine spatial criteria with business and graphic criteria.
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SpatialViewController : ViewController3d, BeSQLite::VirtualSet
{
    DEFINE_T_SUPER(ViewController3d);

    friend struct  SpatialRedlineViewController;
public:
    

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   05/16
    //=======================================================================================
    struct ElementsQuery
    {
        BeSQLite::CachedStatementPtr m_viewStmt;
        SpecialElements const* m_special;
        ClipVectorCPtr m_activeVolume;
        int m_idCol = 0;
        DGNPLATFORM_EXPORT bool TestElement(DgnElementId);
        bool IsNever(DgnElementId id) const {return m_special && m_special->m_never.Contains(id);}
        bool IsAlways(DgnElementId id) const {return m_special && m_special->m_always.Contains(id);}
        bool HasAlwaysList() const {return m_special && !m_special->m_always.empty();}
        DGNPLATFORM_EXPORT void Start(SpatialViewControllerCR); //!< when this method is called the SQL string for the "ViewStmt" is obtained from the SpatialViewController supplied.
        ElementsQuery(SpecialElements const* special, ClipVectorCP activeVolume) {m_special = (special && !special->IsEmpty()) ? special : nullptr; m_activeVolume=activeVolume;}
    };

    //=======================================================================================
    //! A query that uses both the spatial index and a DgnElementId-based filter for a SpatialView.
    //! This object holds two statements - one for the spatial query and one that filters element, by id,
    //! on the "other" criteria for a SpatialView.
    //! The Statements are retrieved from the statement cache and prepared/bound in the Start method.
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct SpatialQuery : ElementsQuery
    {
        bool m_doSkewTest = false;
        BeSQLite::CachedStatementPtr m_rangeStmt;
        BeSQLite::RTree3dVal m_boundingRange;    // only return entries whose range intersects this cube.
        BeSQLite::RTree3dVal m_backFace;
        Render::FrustumPlanes m_planes;
        Frustum m_frustum;
        DMatrix4d m_localToNpc;
        DVec3d m_viewVec;  // vector from front face to back face, for SkewScan
        DPoint3d m_cameraPosition;

        virtual int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) = 0;
        DgnElementId StepRtree();
        bool SkewTest(BeSQLite::RTree3dValCP testRange);
        BeSQLite::RTreeMatchFunction::Within TestVolume(FrustumCR box, BeSQLite::RTree3dValCP);
        void Start(SpatialViewControllerCR); //!< when this method is called the SQL string for the "ViewStmt" is obtained from the SpatialViewController supplied.
        void SetFrustum(FrustumCR);
        SpatialQuery(SpecialElements const* special, ClipVectorCP activeVolume) : ElementsQuery(special, activeVolume) {}
    };

    //=======================================================================================
    //! This object is created on the Client thread and queued to the Query thread. It populates its
    //! QueryResults with the set of n-best elements that satisfy both range and view criteria.
    // @bsiclass                                                    Keith.Bentley   02/16
    //=======================================================================================
    struct RangeQuery : SpatialQuery
    {
        DEFINE_T_SUPER(SpatialQuery)
        bool m_depthFirst = false;
        bool m_cameraOn = false;
        bool m_testLOD = false;
        uint32_t m_orthogonalProjectionIndex;
        uint32_t m_count = 0;
        uint32_t m_hitLimit = 0;     // find this many "best" elements sorted by occlusion score
        uint64_t m_lastId = 0;
        double m_lodFilterNPCArea = 0.0;
        double m_minScore = 0.0;
        double m_lastScore = 0.0;
        SpatialViewControllerCR m_view;
        UpdatePlan::Query const& m_plan;
        QueryResults* m_results;

        int _TestRTree(BeSQLite::RTreeMatchFunction::QueryInfo const&) override;
        void AddAlwaysDrawn(SpatialViewControllerCR);
        void SetDepthFirst() {m_depthFirst=true;}
        void SetTestLOD(bool onOff) {m_testLOD=onOff;}
        void SetSizeFilter(DgnViewportCR, double size);
        bool ComputeNPC(DPoint3dR npcOut, DPoint3dCR localIn);
        bool ComputeOcclusionScore(double& score, FrustumCR);

    public:
        RangeQuery(SpatialViewControllerCR, FrustumCR, DgnViewportCR, UpdatePlan::Query const& plan, QueryResults*);
        void DoQuery(SceneQueue::Task&);
    };

    //=======================================================================================
    // @bsiclass                                                    Keith.Bentley   03/16
    //=======================================================================================
    struct NonSceneQuery : RangeQuery
    {
        NonSceneQuery(SpatialViewControllerCR, FrustumCR, DgnViewportCR);
    };

    //=======================================================================================
    // A ProgressiveTask for a SpatialViewController that draws all of the elements that satisfy the query and range
    // criteria, but were too small to be in the scene.
    // @bsiclass                                                    Keith.Bentley   04/14
    //=======================================================================================
    struct ProgressiveTask : Dgn::ProgressiveTask
    {
        enum {SHOW_PROGRESS_INTERVAL = 1000}; // once per second.
        bool m_setTimeout = false;
        uint32_t m_thisBatch = 0;
        uint32_t m_batchSize = 0;
        uint64_t m_nextShow  = 0;
        DgnElementId m_abortedElement;
        NonSceneQuery m_rangeQuery;
        SpatialViewControllerR m_view;
        DgnElementId GetNextId();
        DGNPLATFORM_EXPORT ProgressiveTask(SpatialViewControllerR, DgnViewportCR);
        Completion _DoProgressive(RenderListContext& context, WantShow&) override;
    };

private:
    Utf8String m_viewSQL; // don't rely on this - it will soon be removed from the API!

protected:
    bool m_loading = false;
    Render::MaterialPtr m_skybox;
    IAuxCoordSysPtr m_auxCoordSys;     //!< The auxiliary coordinate system in use.
    double m_sceneLODSize = 6.0; 
    double m_nonSceneLODSize = 7.0; 
    mutable double m_queryElementPerSecond = 10000;
    bset<Utf8String> m_copyrightMsgs;  // from reality models. Only keep unique ones

    void QueryModelExtents(FitContextR);

    DGNPLATFORM_EXPORT void _DoHeal(HealContext&) override;
    DGNPLATFORM_EXPORT bool _IsInSet(int nVal, BeSQLite::DbValue const*) const override;
    DGNPLATFORM_EXPORT void _CreateTerrain(TerrainContextR context) override;
    DGNPLATFORM_EXPORT void _PickTerrain(PickContextR context) override;
    DGNPLATFORM_EXPORT void _VisitAllElements(ViewContextR) override;
    DGNPLATFORM_EXPORT void _DrawView(ViewContextR context) override;
    DGNPLATFORM_EXPORT void _OnCategoryChange(bool singleEnabled) override;
    DGNPLATFORM_EXPORT FitComplete _ComputeFitRange(struct FitContext&) override;
    DGNPLATFORM_EXPORT AxisAlignedBox3d _GetViewedExtents(DgnViewportCR) const override;
    DGNPLATFORM_EXPORT void _DrawDecorations(DecorateContextR) override;
    DGNPLATFORM_EXPORT virtual void _ChangeModelDisplay(DgnModelId modelId, bool onOff);
    DGNPLATFORM_EXPORT GeometricModelP _GetTargetModel() const override;
    ProgressiveTaskPtr _CreateProgressive(DgnViewportR vp) override {return new ProgressiveTask(*this, vp);}
    SpatialViewControllerCP _ToSpatialView() const override {return this;}
    bool _Allow3dManipulations() const override {return true;}
    GridOrientationType _GetGridOrientationType() const override {return GridOrientationType::ACS;}
    DGNPLATFORM_EXPORT QueryResults _QueryScene(DgnViewportR vp, UpdatePlan const& plan, SceneQueue::Task& task) override;

    //! Construct a new SpatialViewController from a View in the project.
    //! @param[in] definition the view definition
    DGNPLATFORM_EXPORT SpatialViewController(SpatialViewDefinitionCR definition);
    ~SpatialViewController() {RequestAbort(true);}

    void LoadSkyBox(Render::SystemCR system);
    Render::TexturePtr LoadTexture(Utf8CP fileName, Render::SystemCR system);
    double GetGroundElevation() const;
    AxisAlignedBox3d GetGroundExtents(DgnViewportCR) const;
    void DrawGroundPlane(DecorateContextR);
    DGNPLATFORM_EXPORT void DrawSkyBox(TerrainContextR);

public:
    SpatialViewDefinitionR GetSpatialViewDefinition() const {return static_cast<SpatialViewDefinitionR>(*m_definition);}

    //! Called when the display of a model is changed on or off
    //! @param modelId  The model to turn on or off.
    //! @param onOff    If true, elements in the model displayed
    void ChangeModelDisplay(DgnModelId modelId, bool onOff) {_ChangeModelDisplay(modelId, onOff);}

    DgnModelIdSet const& GetViewedModels() const {return GetSpatialViewDefinition().GetModelSelector().GetModels();}
    
    DGNPLATFORM_EXPORT bool ViewVectorsFromOrientation(DVec3dR forward, DVec3dR up, RotMatrixCR orientation, OrientationMode mode, UiOrientation ui);

    DGNPLATFORM_EXPORT void TransformBy(TransformCR);

    //! Gets the Auxiliary Coordinate System for this view.
    IAuxCoordSysP GetAuxCoordinateSystem() const {return m_auxCoordSys.get();}

    //! Sets the Auxiliary Coordinate System to use for this view.
    //! @param[in] acs The new Auxiliary Coordinate System.
    void SetAuxCoordinateSystem(IAuxCoordSysP acs) {m_auxCoordSys = acs;}

    //! Get the Level-of-Detail filtering size for scene creation for this SpatialViewController. This is the size, in pixels, of one side of a square. 
    //! Elements whose aabb projects onto the view an area less than this box are skippped during scene creation.
    double GetSceneLODSize() const {return m_sceneLODSize;}
    void SetSceneLODSize(double val) {m_sceneLODSize=val;} //!< see GetSceneLODSize

    //! Get the Level-of-Detail filtering size for non-scene (background) elements this SpatialViewController. This is the size, in pixels, of one side of a square. 
    //! Elements whose aabb projects onto the view an area less than this box are skippped during background-element display.
    double GetNonSceneLODSize() const {return m_nonSceneLODSize;}
    void SetNonSceneLODSize(double val) {m_nonSceneLODSize=val;} //!< see GetNonSceneLODSize
};

//=======================================================================================
//! A OrthographicViewController controls orthogrphic projections of views of SpatialModels
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE OrthographicViewController : SpatialViewController
{
    DEFINE_T_SUPER(SpatialViewController);
    friend struct OrthographicViewDefinition;

protected:
    DGNPLATFORM_EXPORT bool _OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) override;
    DGNPLATFORM_EXPORT bool _OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui) override;
    OrthographicViewControllerCP _ToOrthographicView() const override {return this;}

    //! Construct a new OrthographicViewController
    //! @param[in] definition the view definition
    OrthographicViewController(OrthographicViewDefinitionCR definition) : T_Super(definition) {}

public:
    OrthographicViewDefinitionR GetOrthographicViewDefinition() const {return static_cast<OrthographicViewDefinitionR>(*m_definition);}
};

//=======================================================================================
//! A CameraViewController is used to control perspective projections of views of SpatialModels. A CameraViewController
//! may have a camera enabled that displays world-coordinate geometry onto the image plane through a perspective projection.
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE CameraViewController : SpatialViewController
{
    DEFINE_T_SUPER(SpatialViewController);
    friend struct CameraViewDefinition;

protected:
    CameraViewControllerCP _ToCameraView() const override {return this;}
    DGNPLATFORM_EXPORT bool _OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) override;
    DGNPLATFORM_EXPORT bool _OnOrientationEvent(RotMatrixCR matrix, OrientationMode mode, UiOrientation ui) override;

    //! Construct a new CameraViewController
    //! @param[in] definition the view definition
    CameraViewController(CameraViewDefinitionCR definition) : T_Super(definition) {}

public:
    CameraViewDefinitionR GetCameraViewDefinition() const {return static_cast<CameraViewDefinitionR>(*m_definition);}

    /** @name Camera */
/** @{ */

    //! Determine whether the camera is on for this view
    bool IsCameraOn() const {return GetCameraViewDefinition().IsCameraOn();}

    //! Determine whether the camera is valid for this view
    bool IsCameraValid() const {return GetCameraViewDefinition().IsCameraValid();}

    //! Turn the camera on or off for this view
    //! @param[in] val whether the camera is to be on or off
    void SetCameraOn(bool val) {GetCameraViewDefinition().SetCameraOn(val);}

    //! Calculate the lens angle formed by the current delta and focus distance
    double CalcLensAngle() const {return GetCameraViewDefinition().CalcLensAngle();}

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
    ViewportStatus LookAt(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVector, DVec2dCP viewDelta=nullptr, double const* frontDistance=nullptr, double const* backDistance=nullptr)
                    {return GetCameraViewDefinition().LookAt(eyePoint, targetPoint, upVector, viewDelta, frontDistance, backDistance);}

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
    ViewportStatus LookAtUsingLensAngle(DPoint3dCR eyePoint, DPoint3dCR targetPoint, DVec3dCR upVector, double fov, double const* frontDistance=nullptr, double const* backDistance=nullptr)
                    {return GetCameraViewDefinition().LookAtUsingLensAngle(eyePoint, targetPoint, upVector, fov, frontDistance, backDistance);}

    //! Move the camera relative to its current location by a distance in camera coordinates.
    //! @param[in] distance to move camera. Length is in world units, direction relative to current camera orientation.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    ViewportStatus MoveCameraLocal(DVec3dCR distance) {return GetCameraViewDefinition().MoveCameraLocal(distance);}

    //! Move the camera relative to its current location by a distance in world coordinates.
    //! @param[in] distance in world units.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    ViewportStatus MoveCameraWorld(DVec3dCR distance) {return GetCameraViewDefinition().MoveCameraWorld(distance);}

    //! Rotate the camera from its current location about an axis relative to its current orientation.
    //! @param[in] angle The angle to rotate the camera, in radians.
    //! @param[in] axis The axis about which to rotate the camera. The axis is a direction relative to the current camera orientation.
    //! @param[in] aboutPt The point, in world coordinates, about which the camera is rotated. If aboutPt is nullptr, the camera rotates in place
    //! (i.e. about the current eyePoint).
    //! @note Even though the axis is relative to the current camera orientation, the aboutPt is in world coordinates, \b not relative to the camera.
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    ViewportStatus RotateCameraLocal(double angle, DVec3dCR axis, DPoint3dCP aboutPt=nullptr) {return GetCameraViewDefinition().RotateCameraLocal(angle, axis, aboutPt);}

    //! Rotate the camera from its current location about an axis in world coordinates.
    //! @param[in] angle The angle to rotate the camera, in radians.
    //! @param[in] axis The world-based axis (direction) about which to rotate the camera.
    //! @param[in] aboutPt The point, in world coordinates, about which the camera is rotated. If aboutPt is nullptr, the camera rotates in place
    //! (i.e. about the current eyePoint).
    //! @return Status indicating whether the camera was successfully positioned. See values at #ViewportStatus for possible errors.
    //! @note This method modifies this ViewController. If this ViewController is attached to DgnViewport, you must call DgnViewport::SynchWithViewController
    //! to see the new changes in the DgnViewport.
    ViewportStatus RotateCameraWorld(double angle, DVec3dCR axis, DPoint3dCP aboutPt=nullptr) {return GetCameraViewDefinition().RotateCameraWorld(angle, axis, aboutPt);}

    //! Get the distance from the eyePoint to the back plane for this view.
    double GetBackDistance() const {return GetCameraViewDefinition().GetBackDistance();}

    //! Place the eyepoint of the camera so it is centered in the view. This removes any 1-point perspective skewing that may be
    //! present in the current view.
    //! @param[in] backDistance optional, If not nullptr, the new the distance from the eyepoint to the back plane. Otherwise the distance from the
    //! current eyepoint is used.
    void CenterEyePoint(double const* backDistance=nullptr) {GetCameraViewDefinition().CenterEyePoint(backDistance);}

    //! Center the focus distance of the camera halfway between the front plane and the back plane, keeping the eyepont,
    //! lens angle, rotation, back distance, and front distance unchanged.
    //! @note The focus distance, origin, and delta values are modified, but the view encloses the same volume and appears visually unchanged.
    void CenterFocusDistance() {GetCameraViewDefinition().CenterFocusDistance();}

    //! Get the distance from the eyePoint to the front plane for this view.
    double GetFrontDistance() const {return GetCameraViewDefinition().GetFrontDistance();}

    //! Get the lens angle for this view.
    double GetLensAngle() const {return GetCameraViewDefinition().GetLensAngle();}

    //! Set the lens angle for this view.
    //! @param[in] angle The new lens angle in radians. Must be greater than 0 and less than pi.
    //! @note This does not change the view's current field-of-view. Instead, it changes the lens that will be used if the view
    //! is subsequently modified and the lens angle is used to position the eyepoint.
    //! @note To change the field-of-view (i.e. "zoom") of a view, pass a new viewDelta to #LookAt
    void SetLensAngle(double angle) {GetCameraViewDefinition().SetLensAngle(angle);}

    //! Get the distance from the eyePoint to the focus plane for this view.
    double GetFocusDistance() const {return GetCameraViewDefinition().GetFocusDistance();}

    //! Set the focus distance for this view.
    //! @note Changing the focus distance changes the plane on which the delta.x and delta.y values lie. So, changing focus distance
    //! without making corresponding changes to delta.x and delta.y essentially changes the lens angle, causing a "zoom" effect.
    void SetFocusDistance(double dist) {GetCameraViewDefinition().SetFocusDistance(dist);}

    //! Get the current location of the eyePoint for camera in this view.
    DPoint3dCR GetEyePoint() const {return GetCameraViewDefinition().GetEyePoint();}

    //! Change the location of the eyePoint for the camera in this view.
    //! @param[in] pt The new eyepoint.
    //! @note This method is generally for internal use only. Moving the eyePoint arbitrarily can result in skewed or illegal perspectives.
    //! The most common method for user-level camera positioning is #LookAt.
    void SetEyePoint(DPoint3dCR pt) {GetCameraViewDefinition().SetEyePoint(pt);}
/** @} */
};

#ifdef WIP_VIEW_DEFINITION
//=======================================================================================
//! A SectioningViewController is a spatial view with a clip that defines a section cut.
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SectioningViewController : SpatialViewController
{
    DEFINE_T_SUPER(SpatialViewController);

protected:
    IViewClipObjectPtr m_clip; // a SectionClipObject
    mutable bool m_hasAnalyzedCutPlanes;
    mutable uint32_t m_cutPlaneCount;
    mutable uint32_t m_foremostCutPlaneIndex;
    mutable DPlane3d m_foremostCutPlane;
    ClipVolumePass m_pass;

    DGNPLATFORM_EXPORT void _DrawView(ViewContextR) override;
    DGNPLATFORM_EXPORT Render::GraphicPtr _StrokeGeometry(ViewContextR, GeometrySourceCR, double) override;
    DGNPLATFORM_EXPORT void _StoreToDefinition() const override;
    DGNPLATFORM_EXPORT void _LoadFromDefinition() override;

    void SetOverrideGraphicParams(ViewContextR) const;
    void DrawViewInternal(ViewContextR);
    ClipVectorPtr GetClipVectorInternal(ClipVolumePass) const;

    void AnalyzeCutPlanes() const;

public:
    void SetClip(IViewClipObject& clip) {m_clip = &clip;}

    //! Construct a new SectioningViewController.
    //! @remarks This constructor is normally used only as part of creating a new view in the project.
    //! @remarks Use this constructor only to create a new camera view controller. To load an existing view controller,
    //! call SpatialViewController::Create.
    //! @param[in] definition the view definition
    DGNPLATFORM_EXPORT SectioningViewController(SectionViewDefinition& definition);

    DGNPLATFORM_EXPORT DPlane3d GetForemostCutPlane() const;

    DGNPLATFORM_EXPORT bool HasDogLeg() const;

    ClipVectorPtr GetInsideForwardClipVector() const {return GetClipVectorInternal(ClipVolumePass::InsideForward);}
};
#endif

//=======================================================================================
//! A ViewController2d is used to control views of 2d models.
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ViewController2d : ViewController
{
    DEFINE_T_SUPER(ViewController);

protected:
    ProgressiveTaskPtr _CreateProgressive(DgnViewportR vp) override {return nullptr;} // needs work
    DGNPLATFORM_EXPORT QueryResults _QueryScene(DgnViewportR vp, UpdatePlan const& plan, SceneQueue::Task& task) override;
    DGNPLATFORM_EXPORT void _DrawView(ViewContextR) override;
    DGNPLATFORM_EXPORT AxisAlignedBox3d _GetViewedExtents(DgnViewportCR) const override;
    DGNPLATFORM_EXPORT CloseMe _OnModelsDeleted(bset<DgnModelId> const& deletedIds, DgnDbR db) override;
    DGNPLATFORM_EXPORT GeometricModelP _GetTargetModel() const override {return GetViewedModel();}

    ViewController2d(ViewDefinition2dCR def) : T_Super(def) {}

public:
    ViewDefinition2dR GetViewDefinition2d() const {return static_cast<ViewDefinition2dR>(*m_definition);}

    DgnModelId GetViewedModelId() const {return GetViewDefinition2d().GetBaseModelId();}
    GeometricModel2dP GetViewedModel() const {return GetDgnDb().Models().Get<GeometricModel2d>(GetViewedModelId()).get();}
};

//=======================================================================================
//! A DrawingViewController is used to control views of DrawingModels
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DrawingViewController : ViewController2d
{
    DEFINE_T_SUPER(ViewController2d);
    friend struct DrawingViewDefinition;
protected:
    DrawingViewControllerCP _ToDrawingView() const override {return this;}
    DGNPLATFORM_EXPORT bool _OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) override;

    //! Construct a new DrawingViewController.
    DrawingViewController(DrawingViewDefinitionCR def) : ViewController2d(def) {}
};

//=======================================================================================
//! A SectionDrawingViewController is used to control views of SectionDrawingModels
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct SectionDrawingViewController : DrawingViewController
{
    DEFINE_T_SUPER(DrawingViewController);

protected:
    mutable SectioningViewControllerPtr m_sectionView;  // transient

public:
    //! Convenience method to get the drawing that is displayed in this view.
    SectionDrawingModel* GetSectionDrawing() const {return GetDgnDb().Models().Get<SectionDrawingModel>(GetViewedModelId()).get();}

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
//! @ingroup GROUP_DgnView
// @bsiclass                                                    Keith.Bentley   03/12
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE HypermodelingViewController : SpatialViewController
{
    DEFINE_T_SUPER(SpatialViewController);

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
    SpatialViewControllerPtr m_physical;
    bvector<SectionDrawingViewControllerPtr> m_drawings;
    ViewControllerP m_currentViewController;
    mutable Pass m_pass;
    int m_nearestCutPlane;
    DrawingSymbology m_symbology;
    Pass m_passesToDraw;

    void _DrawView(ViewContextR) override;
    Render::GraphicPtr _StrokeGeometry(ViewContextR, GeometrySourceCR, double) override;
    Render::GraphicPtr _StrokeHit(ViewContextR, GeometrySourceCR, HitDetailCR) override;
    bool _Allow3dManipulations() const override;
    AxisAlignedBox3d _GetViewedExtents(DgnViewportCR) const override;

    void PushClipsForSpatialView(ViewContextR) const;
    void PopClipsForSpatialView(ViewContextR) const;
    void PushClipsForInContextViewPass(ViewContextR context, SectionDrawingViewControllerCR drawing) const;
    void PopClipsForInContextViewPass(ViewContextR context, SectionDrawingViewControllerCR drawing) const;

    DRange3d GetDrawingRange(DrawingViewControllerR) const;
    void DrawFakeSheetBorder(ViewContextR, DrawingViewControllerR) const;

    bool ShouldDraw(Pass p) const {return (m_passesToDraw & p) == p;}

public:
    DGNPLATFORM_EXPORT HypermodelingViewController(SpatialViewDefinition const& def, SpatialViewControllerR, bvector<SectionDrawingViewControllerPtr> const&);
    bool ShouldDrawProxyGraphics(ClipVolumePass proxyGraphicsType, int planeIndex) const;
    bool ShouldDrawAnnotations() const;
    DGNPLATFORM_EXPORT void SetOverrideGraphicParams(ViewContextR) const;

    DGNPLATFORM_EXPORT bvector<SectionDrawingViewControllerPtr> GetSectionDrawingViews() const;
    DGNPLATFORM_EXPORT SectionDrawingViewControllerPtr FindSectionDrawingViewById(DgnViewId) const;
    DGNPLATFORM_EXPORT BentleyStatus AddDrawing(SectionDrawingViewControllerR);
    DGNPLATFORM_EXPORT BentleyStatus RemoveDrawing(DgnViewId);
    DGNPLATFORM_EXPORT SpatialViewControllerR GetSpatialView() const;
    DrawingSymbology GetDrawingSymbology() const {return m_symbology;} //!< Get the symbology for some aspects of the drawings when they are drawn in context.
    void SetDrawingSymbology(DrawingSymbology const& s) {m_symbology=s;} //!< Set the symbology for some aspects of the drawings when they are drawn in context.
    Pass GetPassesToDraw() const {return m_passesToDraw;} //!< Get the drawing elements to draw.
    void SetPassesToDraw(Pass p) {m_passesToDraw = p;} //!< Set the drawing elements to draw.
};

END_BENTLEY_DGN_NAMESPACE
