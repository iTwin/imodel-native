/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCoreDLLInlines.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeThreadLocalStorage.h>

#if defined (_MANAGED)
    #error
#endif

/*--------------------------------------------------------------------------------------+
/* This file exists to allow non-virtual-interface methods to be inlined in
/* the production builds of dgncore.dll, but non-inlined for debug builds (otherwise you can't step into them).
+--------------------------------------------------------------------------------------*/

#if !defined (__DGNPLATFORM_BUILD__)
    #error This file is only valid inside DgnPlatform.dll
#endif

#if defined (NDEBUG) && !defined (__DGNCORE_DONT_INLINE__)
    #define DG_INLINE inline
#else
    #define DG_INLINE
#endif

extern BeThreadLocalStorage g_hostForThread;

DG_INLINE double IViewClipObject::GetSize (ClipVolumeSizeProp clipVolumeCropProp) const {return _GetSize (clipVolumeCropProp);}
DG_INLINE void IViewClipObject::SetSize (ClipVolumeSizeProp clipVolumeSizeProp, double size) {return _SetSize (clipVolumeSizeProp, size);}
DG_INLINE bool IViewClipObject::GetCrop (ClipVolumeCropProp clipVolumeCropProp) const {return _GetCrop (clipVolumeCropProp);}
DG_INLINE void IViewClipObject::SetCrop (ClipVolumeCropProp clipVolumeCropProp, bool crop) {return _SetCrop (clipVolumeCropProp, crop);}
DG_INLINE RotMatrixCR IViewClipObject::GetRotationMatrix () const {return _GetRotationMatrix ();}
DG_INLINE void IViewClipObject::SetRotationMatrix (RotMatrixCR rMatrix) {return _SetRotationMatrix (rMatrix);}
DG_INLINE void IViewClipObject::SetPoints (size_t numPoints, DPoint3dCP points) {return _SetPoints (numPoints,points);}
DG_INLINE size_t IViewClipObject::GetNumPoints () const {return _GetNumPoints ();}
DG_INLINE StatusInt IViewClipObject::GetPoints (DPoint3dVector& points, size_t iFromPoint, size_t numPoints) const {return _GetPoints (points, iFromPoint, numPoints);}
DG_INLINE void IViewClipObject::CopyCrops (IViewClipObject const* from) {return _CopyCrops (from);}
DG_INLINE void IViewClipObject::SetPreserveUp (bool flag) {return _SetPreserveUp (flag);}
DG_INLINE bool IViewClipObject::GetPreserveUp () const {return _GetPreserveUp ();}
DG_INLINE double IViewClipObject::GetWidth ( ) const {return _GetWidth();}
DG_INLINE void IViewClipObject::SetWidth (double newWidth) {_SetWidth(newWidth);}
DG_INLINE StatusInt IViewClipObject::GetClipBoundary (ClipVectorPtr& clip, DRange3dR maxRange, ClipVolumePass pass, DynamicViewSettingsCP settings, bool displayCutGeometry) const {return _GetClipBoundary(clip,maxRange,pass,settings,displayCutGeometry);}
DG_INLINE bool IViewClipObject::IsClipVolumePassValid (ClipVolumePass pass) const {return _IsClipVolumePassValid(pass);}
DG_INLINE StatusInt IViewClipObject::GetCuttingPlane (DPlane3dR cutPlane, DVec3dR xDir, DVec3dR yDir, ClipMask& clipMask, DRange2dR clipRange, bool& forwardFacing, int index, ViewContextR context, DynamicViewSettingsCR dvs) const {return _GetCuttingPlane(cutPlane,xDir,yDir,clipMask,clipRange,forwardFacing,index,context,dvs);}
DG_INLINE bool IViewClipObject::GetAuxTransform (TransformR t, ClipVolumePass p, DynamicViewSettingsCR dvs) const {return _GetAuxTransform(t,p,dvs);}
DG_INLINE StatusInt IViewClipObject::GetTransform (TransformR trans) const {return _GetTransform(trans);}
DG_INLINE size_t IViewClipObject::GetPrimaryCutPlaneCount () const {return _GetPrimaryCutPlaneCount();}
DG_INLINE StatusInt IViewClipObject::ApplyTransform (TransformCR t) {return _ApplyTransform(t);}
DG_INLINE void IViewClipObject::Draw (ViewContextR c) {return _Draw(c);}

DG_INLINE BeFileNameCR DgnProject::GetFileName() const {return m_fileName;}
DG_INLINE DgnModelSelectors DgnProject::ModelSelectors() {return DgnModelSelectors(*this);}
DG_INLINE DgnViews DgnProject::Views() {return DgnViews(*this);}
DG_INLINE DgnSessions DgnProject::Sessions() {return DgnSessions(*this);}
DG_INLINE DgnKeyStrings DgnProject::KeyStrings() {return DgnKeyStrings(*this);}
DG_INLINE DgnMaterials DgnProject::Materials() {return DgnMaterials(*this);}
DG_INLINE DgnProvenances DgnProject::Provenance() {return DgnProvenances(*this);}
DG_INLINE DgnViewGeneratedDrawings DgnProject::ViewGeneratedDrawings() {return DgnViewGeneratedDrawings(*this);}
DG_INLINE DgnModels& DgnProject::Models() const {return const_cast<DgnModels&>(m_models);}
DG_INLINE DgnStamps& DgnProject::Stamps() const {return const_cast<DgnStamps&>(m_stamps);}
DG_INLINE DgnStyles& DgnProject::Styles() const {return const_cast<DgnStyles&>(m_styles);}
DG_INLINE DgnColors& DgnProject::Colors() const {return const_cast<DgnColors&>(m_colors);}
DG_INLINE DgnLevels& DgnProject::Levels() const {return const_cast<DgnLevels&>(m_levels);}
DG_INLINE DgnFonts& DgnProject::Fonts() const {return const_cast<DgnFonts&>(m_fonts);}
DG_INLINE DgnUnits&   DgnProject::Units() const {return const_cast<DgnUnits&>(m_units);}
DG_INLINE DgnDomains& DgnProject::Domains() const {return const_cast<DgnDomains&>(m_domains);}
DG_INLINE PersistentElementRefP DgnModels::FindElementById (ElementId id) const {return ElementPool().FindElementById(id);}
DG_INLINE DgnElementPool& DgnModels::ElementPool() const {return m_elements;}

DG_INLINE ElemRangeIndexP DgnModel::GetRangeIndexP(bool create) const {return m_graphicElems ? m_graphicElems->GetRangeIndexP(create) : NULL;}
DG_INLINE DgnProjectR PersistentElementRefList::GetDgnProject() const {return MyModel()->GetDgnProject();}
DG_INLINE DgnModelP PersistentElementRefList::GetDgnModelP () const { return MyModel(); }
DG_INLINE void DgnModel::Dump (int nestLevel, bool brief) const {return _Dump(nestLevel,brief);}
DG_INLINE DgnModelType DgnModel::GetModelType () const {return _GetModelType();}
DG_INLINE DgnModelId DgnModel::GetModelId () const {return m_modelId;}
DG_INLINE QvCache* DgnModel::GetQvCache() {return m_project.Models().GetQvCache();}
DG_INLINE DPoint3d DgnModel::GetGlobalOrigin() const {return _GetGlobalOrigin();}
DG_INLINE BentleyStatus DgnModel::LoadFromDb() {return _LoadFromDb();}
DG_INLINE DgnProjectR DgnModel::GetDgnProject() const {return m_project;}

DG_INLINE bool Viewport::Is3dView() const {return m_is3dView;}
DG_INLINE bool Viewport::IsActive() const {return NULL != m_output;}
DG_INLINE bool Viewport::IsCameraOn() const {return m_isCameraOn;}
DG_INLINE int Viewport::GetViewNumber() const {return m_viewNumber;}
DG_INLINE ViewFlagsCP Viewport::GetViewFlags() const {return &m_rootViewFlags;}
DG_INLINE CameraInfo const& Viewport::GetCamera() const {return m_camera;}
DG_INLINE void Viewport::SetMinimumLOD (double minLOD) {m_minLOD = minLOD;}
DG_INLINE DPoint3dCP Viewport::GetScale() const {return &m_scale;}
DG_INLINE RotMatrixCR Viewport::GetRotMatrix() const {return m_rotMatrix;}
DG_INLINE DMap4dCP Viewport::GetWorldToViewMap() const {return &m_rootToView;}
DG_INLINE DMap4dCP Viewport::GetWorldToNpcMap() const {return &m_rootToNpc;}
DG_INLINE bool Viewport::IsGridOn() const {return _IsGridOn();}
DG_INLINE bool Viewport::IsSheetView() const {return _IsSheetView();}
DG_INLINE ViewControllerCR Viewport::GetViewController () const {return *m_viewController;}
DG_INLINE ViewControllerR Viewport::GetViewControllerR () const {return *m_viewController;}
DG_INLINE int Viewport::GetScreenNumber() const {return _GetScreenNumber();}
DG_INLINE DPoint3dCP Viewport::GetViewOrigin() const {return _GetViewOrigin();}
DG_INLINE DPoint3dCP Viewport::GetViewDelta() const {return _GetViewDelta();}
DG_INLINE void Viewport::Destroy() {_Destroy();}
DG_INLINE UInt32 Viewport::GetBackgroundColor () const {return m_backgroundColor.m_int;}
DG_INLINE void Viewport::ResynchColorMap () {_ResynchColorMap ();}
DG_INLINE void Viewport::GetViewCorners (DPoint3dR low, DPoint3dR high) const {_GetViewCorners(low,high);}
DG_INLINE int Viewport::GetIndexedLineWidth (int index) const {return _GetIndexedLineWidth(index);}
DG_INLINE UInt32 Viewport::GetIndexedLinePattern (int index) const {return _GetIndexedLinePattern(index);}
DG_INLINE void Viewport::SynchWithViewController (bool saveInUndo) {_SynchWithViewController(saveInUndo);}
DG_INLINE StatusInt Viewport::SetupFromViewController () {return _SetupFromViewController();}
DG_INLINE void Viewport::SetFrustumFromRootCorners (DPoint3dCP rootBox, double compressionFraction) {_SetFrustumFromRootCorners(rootBox, compressionFraction);}
DG_INLINE IViewDrawP Viewport::GetIViewDraw() {return _GetIViewDraw();}
DG_INLINE IViewOutputP Viewport::GetIViewOutput() {return _GetIViewOutput();}
DG_INLINE ICachedDrawP Viewport::GetICachedDraw() {return _GetICachedDraw();}
DG_INLINE void Viewport::SetNeedsRefresh() {_SetNeedsRefresh();}
DG_INLINE void Viewport::SetNeedsHeal() {_SetNeedsHeal();}
DG_INLINE void Viewport::GetViewName (WStringR name) const {_GetViewName(name);}
DG_INLINE UInt32 Viewport::GetMenuColor (int index) const {return _GetMenuColor(index);}
DG_INLINE double Viewport::GetMinimumLOD() const {return _GetMinimumLOD();}
DG_INLINE BSIRect Viewport::GetClientRect() const {return _GetClientRect();}
DG_INLINE Point2d Viewport::GetScreenOrigin() const {return _GetScreenOrigin();}
DG_INLINE DVec2d Viewport::GetDpiScale() const    {return _GetDpiScale();}
DG_INLINE bool Viewport::GetWindowBgColor (RgbColorDef& rgb) const {return _GetWindowBgColor (rgb);}
DG_INLINE UInt32 Viewport::GetHiliteColor () const {return _GetHiliteColor();}
DG_INLINE StatusInt Viewport::RefreshViewport (bool always, bool synchHealingFromBs, bool& stopFlag) {return _RefreshViewport (always, synchHealingFromBs, stopFlag);}
DG_INLINE void Viewport::DrawStandardGrid (DPoint3dR origin, RotMatrixR rMatrix, Point2dCP fixedRepetitions) {_DrawStandardGrid (origin, rMatrix, fixedRepetitions);}
DG_INLINE ViewportStatus Viewport::ResizeFromViewportInfo (ViewPortInfoCR viewPortInfo, ViewportResizeMode mode, ViewControllerCP viewController) {return _ResizeFromViewportInfo (viewPortInfo, mode, viewController);}

DG_INLINE DgnProjectR     ViewController::GetDgnProject() const         {return _GetDgnProject();}
DG_INLINE DgnViewType     ViewController::GetViewType()  const          {return _GetViewType();}
DG_INLINE DgnViewId       ViewController::GetViewId()  const            {return m_viewId;}
DG_INLINE DPoint3d        ViewController::GetOrigin () const            {return _GetOrigin();}
DG_INLINE DVec3d          ViewController::GetDelta () const             {return _GetDelta();}
DG_INLINE RotMatrix       ViewController::GetRotation() const           {return _GetRotation();}
DG_INLINE void            ViewController::SetOrigin (DPoint3dCR val) {_SetOrigin(val);}
DG_INLINE void            ViewController::SetDelta (DVec3dCR val) {_SetDelta(val);}
DG_INLINE void            ViewController::SetRotation(RotMatrixCR val) {_SetRotation(val);}
DG_INLINE double          ViewController::GetAspectRatioSkew () const   {return _GetAspectRatioSkew ();}
DG_INLINE ViewFlagsCR     ViewController::GetViewFlags() const          {return const_cast<ViewController*>(this)->_GetViewFlagsR();}
DG_INLINE ViewFlagsR      ViewController::GetViewFlagsR()               {return _GetViewFlagsR();}
DG_INLINE RgbColorDef     ViewController::GetBackgroundColor() const    {return _GetBackgroundColor();}
DG_INLINE void            ViewController::SetBackgroundColor (RgbColorDef color)    {m_backgroundColor = color; m_viewFlags.overrideBackground=1;}
DG_INLINE PhysicalViewControllerCP ViewController::ToPhysicalViewController() const {return _ToPhysicalView();}
DG_INLINE CameraViewControllerCP ViewController::ToCameraViewController() const {return _ToCameraView();}
DG_INLINE DrawingViewControllerCP ViewController::ToDrawingViewController() const  {return _ToDrawingView();}
DG_INLINE SheetViewControllerCP ViewController::ToSheetViewController() const {return _ToSheetView();}
DG_INLINE void            ViewController::AdjustAspectRatio (double windowAspect, bool expandView) {_AdjustAspectRatio (windowAspect, expandView);}
DG_INLINE bool            ViewController::IsLoaded () const { return m_baseModelId.IsValid();}
DG_INLINE void            ViewController::ChangeLevelDisplay (LevelId levelId, bool onOff) {_ChangeLevelDisplay (levelId, onOff);}
DG_INLINE void            ViewController::ChangeModelDisplay (DgnModelId modelId, bool onOff) {_ChangeModelDisplay (modelId, onOff);}
DG_INLINE bool            ViewController::Allow3dManipulations() const {return _Allow3dManipulations();}
DG_INLINE DgnModelP       ViewController::GetTargetModel() const {return _GetTargetModel();}
DG_INLINE DRange3d        ViewController::GetProjectExtents() const {return _GetProjectExtents();}
DG_INLINE BitMaskCR       ViewController::GetLevelDisplayMask () const {return _GetLevelDisplayMask();}
DG_INLINE bool            ViewController::OnGeoLocationEvent (GeoLocationEventStatus& status, GeoPointCR point) {return _OnGeoLocationEvent (status, point);}
DG_INLINE void            ViewController::FillModels() {_FillModels();}
DG_INLINE void            ViewController::OnViewOpened (ViewportR vp) {_OnViewOpened (vp);}

DG_INLINE ClipVectorPtr          PhysicalViewController::GetClipVector() const {return _GetClipVector();}
DG_INLINE void                   CameraViewController::SetClipVector (ClipVectorR c) {m_clipVector = &c;}
DG_INLINE void                   CameraViewController::ClearClipVector () {m_clipVector=NULL;}
DG_INLINE ClipVectorPtr          CameraViewController::_GetClipVector() const {return m_clipVector;}
DG_INLINE CameraInfo&            CameraViewController::GetCameraR()        {return m_camera;}
DG_INLINE CameraInfo const&      CameraViewController::GetCamera() const   {return const_cast<CameraViewController*>(this)->GetCameraR();}
DG_INLINE bool                   CameraViewController::IsCameraOn () const {return m_isCameraOn;}
DG_INLINE void                   CameraViewController::SetCameraOn (bool val) {m_isCameraOn = val;}
DG_INLINE IAuxCoordSysP PhysicalViewController::GetAuxCoordinateSystem () const {return _GetAuxCoordinateSystem();}

DG_INLINE DPoint2d ViewPortInfo::GetRelativeOrigin () const {return m_globalRelativeRect.origin;}
DG_INLINE DPoint2d ViewPortInfo::GetRelativeCorner () const {return m_globalRelativeRect.corner;}
DG_INLINE Point2d  ViewPortInfo::GetPixelOrigin () const    {Point2d org; org.x = m_viewPixelRect.origin.x; org.y = m_viewPixelRect.origin.y; return org;}
DG_INLINE Point2d  ViewPortInfo::GetPixelCorner () const    {Point2d crn; crn.x = m_viewPixelRect.corner.x; crn.y = m_viewPixelRect.corner.y; return crn;}
DG_INLINE UInt32   ViewPortInfo::GetScreenNumber () const   {return m_screenNumber;}
DG_INLINE bool     ViewPortInfo::GetIsDefined () const      {return (0 != m_wasDefined);}

DG_INLINE void  DisplayPath::GetInfoString (WStringR pathDescr, WCharCP delimiter) const {_GetInfoString (pathDescr, delimiter);}
DG_INLINE void  DisplayPath::DrawInVp (ViewportP vp, DgnDrawMode drawMode, DrawPurpose drawPurpose, bool* stopFlag) const {_DrawInVp (vp, drawMode, drawPurpose, stopFlag);}

DG_INLINE HitSource       HitPath::GetLocateSource () const {return m_locateSource;}
DG_INLINE DPoint3dCR      HitPath::GetTestPoint () const {return m_testPoint;}
DG_INLINE GeomDetailCR    HitPath::GetGeomDetail () const {return m_geomDetail;}
DG_INLINE GeomDetailR     HitPath::GetGeomDetailW () {return m_geomDetail;}
DG_INLINE ViewFlagsCR     HitPath::GetViewFlags () const {return m_viewFlags; }
DG_INLINE IElemTopologyCP HitPath::GetElemTopology () const {return m_elemTopo;}
DG_INLINE SnapMode        SnapPath::GetSnapMode () const {return m_snapMode;}
DG_INLINE SnapMode        SnapPath::GetOriginalSnapMode () const {return m_originalSnapMode;}
DG_INLINE void            SnapPath::SetSnapMode (SnapMode s, bool isOriginal) {m_snapMode=s; if (isOriginal) m_originalSnapMode=s;}
DG_INLINE void            SnapPath::SetSnapDivisor (int divisor) {m_divisor = divisor ? divisor : 2;}
DG_INLINE void            SnapPath::SetAdjustedPoint (DPoint3dCR adjustedPt) {m_adjustedPt = adjustedPt;}
DG_INLINE void            SnapPath::SetHeat (SnapHeat isHot) {m_heat = isHot;}
DG_INLINE bool            SnapPath::IsHot () const {return m_heat != SNAP_HEAT_None;}
DG_INLINE bool            SnapPath::IsPointOnCurve () const {return m_heat == SNAP_HEAT_InRange;}
DG_INLINE SnapHeat        SnapPath::GetHeat () const {return m_heat;}
DG_INLINE DPoint3dCR      SnapPath::GetAdjustedPoint () const {return m_adjustedPt;}
DG_INLINE DPoint3dCR      SnapPath::GetSnapPoint () const {return m_snapPoint;}
DG_INLINE int             SnapPath::GetSnapDivisor () const {return m_divisor;}
DG_INLINE double          SnapPath::GetMinScreenDist () const {return m_minScreenDist;}
DG_INLINE Point2d const&  SnapPath::GetScreenPoint () const {return m_screenPt;}
DG_INLINE HitPath*        IntersectPath::GetSecondPath () const {return m_secondPath;}

DG_INLINE IACSManagerR  IACSManager::GetManager() {return T_HOST.GetAcsManager();}

DG_INLINE void          IAuxCoordSys::DisplayInView (ViewportP vp, ACSDisplayOptions options, bool drawName) const {return _DisplayInView (vp, options, drawName);}
DG_INLINE WString       IAuxCoordSys::GetName () const {return _GetName ();}
DG_INLINE WString       IAuxCoordSys::GetDescription () const {return _GetDescription ();}
DG_INLINE ACSType       IAuxCoordSys::GetType () const {return _GetType ();}
DG_INLINE WString       IAuxCoordSys::GetTypeName () const {return _GetTypeName ();}
DG_INLINE UInt32        IAuxCoordSys::GetExtenderId () const {return _GetExtenderId ();}
DG_INLINE UInt32        IAuxCoordSys::GetSerializedSize() const {return _GetSerializedSize ();}
DG_INLINE StatusInt     IAuxCoordSys::Serialize (void *buffer, UInt32 maxSize) const {return _Serialize (buffer, maxSize);}
DG_INLINE double        IAuxCoordSys::GetScale () const {return _GetScale ();}
DG_INLINE DPoint3dR     IAuxCoordSys::GetOrigin (DPoint3dR pOrigin) const {return _GetOrigin (pOrigin);}
DG_INLINE bool          IAuxCoordSys::GetIsReadOnly () const {return _GetIsReadOnly ();}
DG_INLINE RotMatrixR    IAuxCoordSys::GetRotation (RotMatrixR pRot) const {return _GetRotation (pRot);}
DG_INLINE RotMatrixR    IAuxCoordSys::GetRotation (RotMatrixR pRot, DPoint3dR pPosition) const {return _GetRotation (pRot, pPosition);}
DG_INLINE ACSFlags      IAuxCoordSys::GetFlags () const {return _GetFlags ();}
DG_INLINE StatusInt     IAuxCoordSys::SetName (WCharCP name) {return _SetName (name);}
DG_INLINE StatusInt     IAuxCoordSys::SetDescription (WCharCP descr) {return _SetDescription (descr);}
DG_INLINE StatusInt     IAuxCoordSys::SetType (ACSType type) {return _SetType (type);}
DG_INLINE StatusInt     IAuxCoordSys::SetScale (double scale) {return _SetScale (scale);}
DG_INLINE StatusInt     IAuxCoordSys::SetOrigin (DPoint3dCR pOrigin) {return _SetOrigin (pOrigin);}
DG_INLINE StatusInt     IAuxCoordSys::SetRotation (RotMatrixCR pRot) {return _SetRotation (pRot);}
DG_INLINE StatusInt     IAuxCoordSys::PointFromString (DPoint3dR outPoint, WStringR errorMsg, WCharCP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) {return _PointFromString (outPoint, errorMsg, inString, relative, lastPoint, modelRef);}
DG_INLINE StatusInt     IAuxCoordSys::StringFromPoint (WStringR outString, WStringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFormatter, DirectionFormatterR directionFormatter)
                                                        { return _StringFromPoint (outString, errorMsg, inPoint, delta, deltaOrigin, modelRef, distanceFormatter, directionFormatter); }
DG_INLINE StatusInt     IAuxCoordSys::SetFlags (ACSFlags flags) {return _SetFlags (flags);}
DG_INLINE void          IAuxCoordSys::DrawGrid (ViewportP viewport) const {return _DrawGrid (viewport);}
DG_INLINE void          IAuxCoordSys::PointToGrid (ViewportP viewport, DPoint3dR point) const {_PointToGrid (viewport, point);}
DG_INLINE StatusInt     IAuxCoordSys::CompleteSetupFromViewController (PhysicalViewControllerCP info) {return _CompleteSetupFromViewController (info);}
DG_INLINE bool          IAuxCoordSys::Equals (IAuxCoordSysCP other) const {return _Equals (other);}
DG_INLINE IAuxCoordSysPtr   IAuxCoordSys::Clone() const {return _Clone();}

DG_INLINE StatusInt IAuxCoordSys::GetStandardGridParams (Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, UInt32& gridPerRef) const {return _GetStandardGridParams (gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}
DG_INLINE StatusInt IAuxCoordSys::SetStandardGridParams (Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, UInt32 gridPerRef) {return _SetStandardGridParams (gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}
DG_INLINE bool ISolidKernelEntity::IsEqual (ISolidKernelEntityCR entity) const {return _IsEqual (entity);}
DG_INLINE ISolidKernelEntity::KernelEntityType ISolidKernelEntity::GetEntityType () const {return _GetEntityType ();}
DG_INLINE TransformCR ISolidKernelEntity::GetEntityTransform () const {return _GetEntityTransform ();}
DG_INLINE void ISolidKernelEntity::SetEntityTransform (TransformCR transform) {return _SetEntityTransform (transform);}
DG_INLINE void ISolidKernelEntity::PostMultiplyEntityTransformInPlace (TransformCR solidTransform)  { SetEntityTransform (Transform::FromProduct (_GetEntityTransform(), solidTransform)); }
DG_INLINE void ISolidKernelEntity::PreMultiplyEntityTransformInPlace (TransformCR uorTransform) { SetEntityTransform (Transform::FromProduct (uorTransform,_GetEntityTransform())); }


DG_INLINE BentleyStatus ISubEntity::ToString (WStringR subEntityStr) const {return _ToString (subEntityStr);}
DG_INLINE bool ISubEntity::IsEqual (ISubEntityCR subEntity) const {return _IsEqual (subEntity);}
DG_INLINE ISubEntity::SubEntityType ISubEntity::GetSubEntityType () const {return _GetSubEntityType ();}

DG_INLINE bool                  DisplayHandler::IsRenderable (ElementHandleCR el) {return _IsRenderable (el);}
DG_INLINE void                  DisplayHandler::DrawFiltered (ElementHandleCR thisElm, ViewContextR context, DPoint3dCP pts, double size){_DrawFiltered (thisElm, context, pts, size);}
DG_INLINE bool                  DisplayHandler::FilterLevelOfDetail (ElementHandleCR thisElm, ViewContextR context) {return _FilterLevelOfDetail (thisElm, context);}
DG_INLINE void                  DisplayHandler::GetElemDisplayParams (ElementHandleCR thisElm, ElemDisplayParams& params, bool wantMaterials) {_GetElemDisplayParams (thisElm, params, wantMaterials);}
DG_INLINE bool                  DisplayHandler::IsPlanar (ElementHandleCR thisElm, DVec3dP normal, DPoint3dP pointP, DVec3dCP inputDefaultNormal) {return _IsPlanar (thisElm, normal, pointP, inputDefaultNormal);}
DG_INLINE StatusInt             DisplayHandler::EvaluateCustomKeypoint (ElementHandleCR elHandle, DPoint3dP outPoint, byte* customKeypointData) {return _EvaluateCustomKeypoint (elHandle, outPoint, customKeypointData);}
DG_INLINE SnapStatus            DisplayHandler::OnSnap (SnapContextP context, int snapPathIndex) {return _OnSnap (context, snapPathIndex);}
DG_INLINE bool                  DisplayHandler::IsVisible (ElementHandleCR elHandle, ViewContextR context, bool testRange, bool testLevel, bool testClass) {return _IsVisible (elHandle, context, testRange, testLevel, testClass);}
DG_INLINE void                  DisplayHandler::GetTransformOrigin (ElementHandleCR elHandle, DPoint3dR origin)   {_GetTransformOrigin (elHandle, origin);}
DG_INLINE void                  DisplayHandler::GetSnapOrigin (ElementHandleCR elHandle, DPoint3dR origin)        {_GetSnapOrigin (elHandle, origin);}
DG_INLINE void                  DisplayHandler::GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation) {_GetOrientation (elHandle, orientation);}
DG_INLINE void                  DisplayHandler::Draw (ElementHandleCR thisElm, ViewContextR context) {_Draw (thisElm, context);}
DG_INLINE BentleyStatus         DisplayHandler::DrawPath (DisplayPathCR path, ViewContextR context) {return _DrawPath (path, context);}
DG_INLINE IAnnotationHandlerP   DisplayHandler::GetIAnnotationHandler (ElementHandleCR eh) {return _GetIAnnotationHandler (eh);}
DG_INLINE void                  DisplayHandler::GetPathDescription (ElementHandleCR eh, WStringR descr, DisplayPathCP path, WCharCP levelStr, WCharCP modelStr, WCharCP groupStr, WCharCP delimiterStr) {return _GetPathDescription (eh, descr, path, levelStr, modelStr, groupStr, delimiterStr);}
DG_INLINE StatusInt             Handler::CallOnDecorate (ElementHandleCR elemHandle, ViewContextP context) {return _OnDecorate (elemHandle, context);}
DG_INLINE UInt32                Handler::GetApiVersion () {return _GetApiVersion ();}
DG_INLINE DisplayHandlerP       Handler::GetDisplayHandler()     {return _GetDisplayHandler();}
DG_INLINE bool                  Handler::IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) {return _IsSupportedOperation (eh, stype);}
DG_INLINE bool                  Handler::IsTransformGraphics (ElementHandleCR element, TransformInfoCR transform) {return _IsTransformGraphics (element, transform);}
DG_INLINE void                  Handler::GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength) {_GetDescription (el, string, desiredLength);}
DG_INLINE void                  Handler::GetTypeName (WStringR string, UInt32 desiredLength) {_GetTypeName (string, desiredLength);}
DG_INLINE void                  Handler::GetTypeDescription (WStringR string, UInt32 desiredLength) {_GetTypeDescription (string, desiredLength);}
DG_INLINE ScanTestResult        Handler::DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP viewContext) {return _DoScannerTests (eh, levelsOn, classMask, viewContext);}
DG_INLINE StatusInt             Handler::FenceStretch (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, DgnPlatform::FenceStretchFlags options) {return _FenceStretch (element, transform, fp, options);}
DG_INLINE BentleyStatus ICurvePathEdit::SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) {return _SetCurveVector (eeh, path);}
DG_INLINE BentleyStatus ICurvePathQuery::GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) {return _GetCurveVector (eh, curves);}
DG_INLINE DgnDomainP            Handler::GetDgnDomain() const {return m_domain;}
DG_INLINE ElementHandlerId      Handler::GetHandlerId() const {return m_handlerId;}

DG_INLINE BentleyStatus IBsplineSurfaceEdit::SetBsplineSurface (EditElementHandleR eeh, MSBsplineSurfaceCR surface) {return _SetBsplineSurface (eeh, surface);}
DG_INLINE BentleyStatus IBsplineSurfaceQuery::GetBsplineSurface (ElementHandleCR source, MSBsplineSurfacePtr& surface) {return _GetBsplineSurface (source, surface);}

DG_INLINE BentleyStatus ISolidPrimitiveEdit::SetSolidPrimitive (EditElementHandleR eeh, ISolidPrimitiveCR primitive) {return _SetSolidPrimitive (eeh, primitive);}
DG_INLINE BentleyStatus ISolidPrimitiveQuery::GetSolidPrimitive (ElementHandleCR source, ISolidPrimitivePtr& primitive) {return _GetSolidPrimitive (source, primitive);}

DG_INLINE BentleyStatus IMeshEdit::SetMeshData (EditElementHandleR eeh, PolyfaceQueryR meshData) {return _SetMeshData (eeh, meshData);}
DG_INLINE BentleyStatus IMeshQuery::GetMeshData (ElementHandleCR source, PolyfaceHeaderPtr& meshData) {return _GetMeshData (source, meshData);}

DG_INLINE BentleyStatus IBRepEdit::SetBRepDataEntity (EditElementHandleR eeh, ISolidKernelEntityR entity) {return _SetBRepDataEntity (eeh, entity);}
DG_INLINE BentleyStatus IBRepQuery::GetBRepDataEntity (ElementHandleCR source, ISolidKernelEntityPtr& entity, bool useCache) {return _GetBRepDataEntity (source, entity, useCache);}

#if defined (NEEDS_WORK_DGNITEM)
DG_INLINE StatusInt             DisplayHandler::EvaluateSnap (ElementHandleCR elHandle, DPoint3dR outPoint, DisplayPathCP dp1, DisplayPathCP dp2, AssocPoint const& assoc, PersistentSnapPathCR snapPath) {return _EvaluateSnap (elHandle, outPoint, dp1, dp2, assoc, snapPath);}
DG_INLINE bool ICellQuery::IsNormalCell (ElementHandleCR eh) {return _IsNormalCell (eh);}
DG_INLINE bool ICellQuery::IsSharedCell (ElementHandleCR eh) {return _IsSharedCell (eh);}
DG_INLINE bool ICellQuery::IsSharedCellDefinition (ElementHandleCR eh) {return _IsSharedCellDefinition (eh);}

DG_INLINE bool ICellQuery::IsPointCell (ElementHandleCR eh) {return _IsPointCell (eh);}
DG_INLINE bool ICellQuery::IsAnnotation (ElementHandleCR eh) {return _IsAnnotation (eh);}
DG_INLINE bool ICellQuery::IsAnonymous (ElementHandleCR eh) {return _IsAnonymous (eh);}

DG_INLINE BentleyStatus ICellQuery::ExtractScale (DVec3dR scale, ElementHandleCR eh) {return _ExtractScale (scale, eh);}

DG_INLINE BentleyStatus ICellQuery::ExtractName (WCharP cellName, int bufferSize, ElementHandleCR eh) {return _ExtractName (cellName, bufferSize, eh);}
DG_INLINE BentleyStatus ICellQuery::ExtractDescription (WCharP descr, int bufferSize, ElementHandleCR eh) {return _ExtractDescription (descr, bufferSize, eh);}

DG_INLINE bool ISharedCellQuery::GetMlineScaleOption (ElementHandleCR eh) {return _GetMlineScaleOption (eh);}
DG_INLINE bool ISharedCellQuery::GetDimScaleOption (ElementHandleCR eh) {return _GetDimScaleOption (eh);}
DG_INLINE bool ISharedCellQuery::GetDimRotationOption (ElementHandleCR eh) {return _GetDimRotationOption (eh);}

DG_INLINE ElementRefP       ISharedCellQuery::GetDefinition (ElementHandleCR eh, DgnProjectR project) {return _GetDefinition (eh, project);}
DG_INLINE BentleyStatus     ISharedCellQuery::GetDefinitionId (ElementHandleCR eh, ElementId& elemID) {return _GetDefinitionID (eh, elemID);}
DG_INLINE SCOverride const* ISharedCellQuery::GetSharedCellOverrides (ElementHandleCR eh) {return _GetSharedCellOverrides (eh);}
#endif

DG_INLINE DgnProjectR       ViewContext::GetDgnProject () const {BeAssert (NULL != m_dgnProject); return *m_dgnProject;}
DG_INLINE void              ViewContext::SetDgnProject (DgnProjectR project) {return _SetDgnProject (project);}
DG_INLINE ElementRefP       ViewContext::GetCurrentElement () const {return m_currentElement.get ();}
DG_INLINE void              ViewContext::SetCurrentElement (ElementRefP elemRef) {return _SetCurrentElement (elemRef);}
DG_INLINE DisplayPathCP     ViewContext::GetSourceDisplayPath () const {return m_sourcePath;}

DG_INLINE DrawPurpose             ViewContext::GetDrawPurpose() const {return m_purpose;}
DG_INLINE bool                    ViewContext::IsCameraOn() const {return m_isCameraOn;}
DG_INLINE bool                    ViewContext::Is3dView() const {return m_is3dView;}

DG_INLINE ElemMatSymbP            ViewContext::GetElemMatSymb() {return &m_elemMatSymb;}
DG_INLINE OvrMatSymb*             ViewContext::GetOverrideMatSymb () {return &m_ovrMatSymb;}
DG_INLINE IViewDrawR              ViewContext::GetIViewDraw() {BeAssert (NULL != m_IViewDraw); return *m_IViewDraw;}
DG_INLINE IDrawGeomR              ViewContext::GetIDrawGeom() {BeAssert (NULL != m_IDrawGeom); return *m_IDrawGeom;}
DG_INLINE ICachedDrawP            ViewContext::GetICachedDraw() {return m_ICachedDraw;}
DG_INLINE bool                    ViewContext::CheckICachedDraw() {return m_creatingCacheElem;}
DG_INLINE byte&                   ViewContext::GetFilterLODFlag() {return m_filterLOD;}
DG_INLINE void                    ViewContext::SetFilterLODFlag(FilterLODFlags flags) { m_filterLOD = (byte) flags; }
DG_INLINE bool                    ViewContext::GetIgnoreScaleForDimensions () {return m_ignoreScaleForDimensions;}
DG_INLINE bool                    ViewContext::GetIgnoreScaleForMultilines () {return m_ignoreScaleForMultilines;}
DG_INLINE bool                    ViewContext::GetApplyRotationToDimView () {return m_applyRotationToDimView;}
DG_INLINE void                    ViewContext::SetIgnoreScaleForDimensions (bool ignore) {m_ignoreScaleForDimensions = ignore;}
DG_INLINE void                    ViewContext::SetIgnoreScaleForMultilines (bool ignore) {m_ignoreScaleForMultilines = ignore;}
DG_INLINE void                    ViewContext::SetApplyRotationToDimView (bool rotateDimView) {m_applyRotationToDimView = rotateDimView;}
DG_INLINE size_t                  ViewContext::GetOverridesStackDepth() {return m_headerOvr.size(); }
DG_INLINE size_t                  ViewContext::GetTransClipDepth()  {return  m_transformClipStack.GetSize(); }
DG_INLINE size_t                  ViewContext::GetRefTransClipDepth()  {return m_refTransClipDepth;}
DG_INLINE RangeResult             ViewContext::GetCurrParentRangeResult () {return m_parentRangeResult; }
DG_INLINE void                    ViewContext::SetCurrParentRangeResult (RangeResult val) {m_parentRangeResult = val;}
DG_INLINE bool&                   ViewContext::GetUseCachedGraphics() {return m_useCachedGraphics;}
DG_INLINE ViewportP               ViewContext::GetViewport() const             {return m_viewport;}
DG_INLINE double                  ViewContext::GetArcTolerance ()    const     {return m_arcTolerance;}
DG_INLINE double                  ViewContext::GetMinLOD() const               {return m_minLOD;}
DG_INLINE void                    ViewContext::SetMinLOD (double lod)          { m_minLOD = lod; }
DG_INLINE ScanCriteriaCP          ViewContext::GetScanCriteria () const     {return m_scanCriteria;}
DG_INLINE ViewFlagsCP             ViewContext::GetViewFlags() const                 {return m_IDrawGeom ? m_IDrawGeom->GetDrawViewFlags() : NULL;}
DG_INLINE void                    ViewContext::SetViewFlags(ViewFlagsCP flags) {if (NULL != m_IDrawGeom) m_IDrawGeom->SetDrawViewFlags (flags);}
DG_INLINE ElemDisplayParamsP      ViewContext::GetCurrentDisplayParams() {return &m_currDisplayParams;}
DG_INLINE bool                    ViewContext::GetDisplayPriorityRange (Int32& low, Int32& high) const {if (NULL == m_viewport) return false; low = m_displayPriorityRange[0]; high = m_displayPriorityRange[1]; return true;}
DG_INLINE void                    ViewContext::SetArcTolerance (double tol)    {m_arcTolerance = tol;}
#ifdef WIP_VANCOUVER_MERGE // linestyle
DG_INLINE void                    ViewContext::SetCurrLineStyle (ILineStyleCP lstyle) {m_elemMatSymb.GetLineStyleSymbP()->SetLineStyle(lstyle);}
#endif
DG_INLINE UInt32                  ViewContext::GetRasterPlane () const        {return m_rasterPlane;}
DG_INLINE StatusInt               ViewContext::Attach (ViewportP vp, DrawPurpose purpose) {return _Attach(vp,purpose);}
DG_INLINE void                    ViewContext::Detach ()                                   {_Detach();}
DG_INLINE bool                    ViewContext::CheckStop()                                 {return _CheckStop();}
DG_INLINE UInt32                  ViewContext::GetDisplayInfo (bool isRenderable)          {return _GetDisplayInfo (isRenderable);}
DG_INLINE void                    ViewContext::DrawWithThickness (ElementHandleCR thisElm, IStrokeForCache& stroker, UInt32 qvIndex) {_DrawWithThickness (thisElm, stroker, qvIndex);}
DG_INLINE void                    ViewContext::DrawCurveVector (ElementHandleCR eh, ICurvePathQueryR query, GeomRepresentations info, bool allowCachedOutline) {_DrawCurveVector (eh, query, info, allowCachedOutline);}
DG_INLINE void                    ViewContext::PopTransformClip()                               {_PopTransformClip();}
DG_INLINE void                    ViewContext::PushTransform (TransformCR trans) {_PushTransform (trans);}
DG_INLINE void                    ViewContext::PushClip (ClipVectorCR clip) {_PushClip (clip);}
DG_INLINE IPickGeomP              ViewContext::GetIPickGeom() {return _GetIPickGeom();}
DG_INLINE void                    ViewContext::DrawSymbol (IDisplaySymbol* symb, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight) {_DrawSymbol (symb, trans, clip, ignoreColor, ignoreWeight);}
DG_INLINE void                    ViewContext::InitScanRangeAndPolyhedron () {_InitScanRangeAndPolyhedron();}
DG_INLINE bool                    ViewContext::FilterRangeIntersection (ElementHandleCR elIter) {return _FilterRangeIntersection (elIter);}
DG_INLINE void                    ViewContext::DrawStyledLineString2d (int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed){_DrawStyledLineString2d (nPts, pts, zDepth, range, closed);}
DG_INLINE void                    ViewContext::DrawStyledLineString3d (int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed){_DrawStyledLineString3d (nPts, pts, range, closed);}
DG_INLINE void                    ViewContext::DrawStyledArc2d (DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range) {_DrawStyledArc2d (ellipse, isEllipse, zDepth, range);}
DG_INLINE void                    ViewContext::DrawStyledArc3d (DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range) {_DrawStyledArc3d (ellipse, isEllipse, range);}
DG_INLINE void                    ViewContext::DrawStyledBSplineCurve2d (MSBsplineCurveCR curve, double zDepth) {_DrawStyledBSplineCurve2d (curve, zDepth);}
DG_INLINE void                    ViewContext::DrawStyledBSplineCurve3d (MSBsplineCurveCR curve) {_DrawStyledBSplineCurve3d (curve);}
DG_INLINE void                    ViewContext::PushViewIndependentOrigin (DPoint3dCP origin) {_PushViewIndependentOrigin (origin);}
DG_INLINE StatusInt               ViewContext::VisitTransient (ElementHandleCR el, SymbologyOverridesP ovr) {return _VisitTransient (el, ovr);}
DG_INLINE StatusInt               ViewContext::VisitElemHandle (ElementHandleCR inEl, bool checkRange, bool checkScanCriteria) {return _VisitElemHandle (inEl, checkRange, checkScanCriteria);}
DG_INLINE void                    ViewContext::AllocateScanCriteria(){_AllocateScanCriteria();}
DG_INLINE void                    ViewContext::VisitDgnModel (DgnModelP modelRef) {_VisitDgnModel (modelRef);}
DG_INLINE void                    ViewContext::SetScanReturn() {_SetScanReturn();}
DG_INLINE QvElem*                 ViewContext::DrawCached (CachedDrawHandleCR dh, IStrokeForCache& stroker, Int32 qvIndex) {return _DrawCached (dh, stroker, qvIndex);}
DG_INLINE void                    ViewContext::VisitTransientElements (bool isPreUpdate) {_VisitTransientElements (isPreUpdate);}
DG_INLINE bool                    ViewContext::CheckFillOutline() {return _CheckFillOutline();}
DG_INLINE ILineStyleCP            ViewContext::GetCurrLineStyle (LineStyleSymbP* symb) {return _GetCurrLineStyle (symb);}
DG_INLINE void                    ViewContext::CookDisplayParams (ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) {_CookDisplayParams (elParams, elMatSymb);}
DG_INLINE void                    ViewContext::CookDisplayParamsOverrides () {_CookDisplayParamsOverrides ();}
DG_INLINE StatusInt               ViewContext::InitContextForView() {return _InitContextForView();}
DG_INLINE void                    ViewContext::OutputElement (ElementHandleCR element) {_OutputElement (element);}
DG_INLINE bool                    ViewContext::VisitAllModelElements (bool includeTransients) { return _VisitAllModelElements (includeTransients); }
DG_INLINE void                    ViewContext::PushDisplayStyle (DisplayStyleCP style, DgnModelP modelRef, bool allowOverrideInheritance) { _PushDisplayStyle(style, modelRef, allowOverrideInheritance); }
DG_INLINE void                    ViewContext::PopDisplayStyle () { _PopDisplayStyle (); }
DG_INLINE void                    ViewContext::ClearZ () { _ClearZ(); }
DG_INLINE CookedDisplayStyleCP    ViewContext::GetCurrentCookedDisplayStyle () { return _GetCurrentCookedDisplayStyle (); }
DG_INLINE bool                    ViewContext::WantShowDefaultFieldBackground () {return _WantShowDefaultFieldBackground();}
DG_INLINE void                    ViewContext::DeleteSymbol (IDisplaySymbol* symbol) {_DeleteSymbol (symbol);}
DG_INLINE bool                    ViewContext::_WantShowDefaultFieldBackground () {return T_HOST.GetGraphicsAdmin()._WantShowDefaultFieldBackground();}
DG_INLINE bool                    ViewContext::_WantSaveQvElem (DrawExpense expense) {return T_HOST.GetGraphicsAdmin()._WantSaveQvElem(static_cast<int>(expense));}
DG_INLINE bool                    ViewContext::IfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize) { return _IfConditionalDraw (filterId, element, data, dataSize); }
DG_INLINE bool                    ViewContext::ElseIfConditionalDraw (DisplayFilterHandlerId filterId, ElementHandleCP element, void const* data, size_t dataSize) { return _ElseIfConditionalDraw (filterId, element, data, dataSize); }
DG_INLINE bool                    ViewContext::ElseConditionalDraw () { return _ElseConditionalDraw (); }
DG_INLINE void                    ViewContext::EndConditionalDraw () { _EndConditionalDraw (); }
DG_INLINE void                    ViewContext::DrawAligned (DVec3dCR axis, DPoint3dCR origin, AlignmentMode type, IStrokeAligned& stroker) {  _DrawAligned (axis, origin, type, stroker); }
DG_INLINE BentleyStatus           ViewContext::GetCurrLocalToFrustumTrans (TransformR trans) const { return m_transformClipStack.GetTransform (trans); }
DG_INLINE BentleyStatus           ViewContext::GetCurrFrustumToLocalTrans (TransformR trans) const { return m_transformClipStack.GetInverseTransform (trans); }
DG_INLINE TransformCP             ViewContext::GetCurrLocalToFrustumTransformCP () const { return m_transformClipStack.GetTransformCP(); }
DG_INLINE void                    ViewContext::SetLocatePriority (int priority)                    { _SetLocatePriority (priority); }
DG_INLINE void                    ViewContext::SetNonSnappable (bool unsnappable)                  { _SetNonSnappable (unsnappable); }
DG_INLINE void                    ViewContext::DrawTextString (TextStringCR textString) {_DrawTextString (textString);}
DG_INLINE void                    ViewContext::DrawTextBlock (TextBlockCR textBlock) {_DrawTextBlock (textBlock);}
DG_INLINE BentleyStatus           ViewContext::GetLocalToFrustumTrans (TransformR trans, size_t index) const { return m_transformClipStack.GetTransformFromIndex (trans, index); }
DG_INLINE bool                    ViewContext::IsViewIndependent() { return m_transformClipStack.IsViewIndependent(); }
DG_INLINE bool                    ViewContext::WantUndisplayedClips () { return _WantUndisplayedClips (); }

DG_INLINE RefCountedBase*         ViewContext::GetConditionalDrawExpressionContext() const                          { return m_conditionalDrawExpressionContext.get(); }
DG_INLINE void                    ViewContext::SetConditionalDrawExpressionContext (RefCountedBase& exprContext)    { m_conditionalDrawExpressionContext = &exprContext; }
DG_INLINE WCharCP                 ViewContext::GetPresentationFormId() const                                        { return m_presentationFormId.c_str(); }
DG_INLINE void                    ViewContext::SetPresentationFormId (WCharCP formId)                               { m_presentationFormId = formId; }
DG_INLINE void                    ViewContext::SetPresentationFormFlag (WCharCP formFlag)                           { m_presentationFormFlags.insert (WString (formFlag)); }
DG_INLINE bool                    ViewContext::GetPresentationFormFlag (WCharCP formFlag)                           { return m_presentationFormFlags.end() != m_presentationFormFlags.find (WString (formFlag)); }
DG_INLINE double                  ViewContext::GetCurrentLevelOfDetail() const                                      { return m_levelOfDetail; }
DG_INLINE void                    ViewContext::SetCurrentLevelOfDetail (double levelOfDetail)                       { m_levelOfDetail = levelOfDetail; }
DG_INLINE ElemDisplayParamsIgnores& ViewContext::GetDisplayParamsIgnores () {return m_ignores;}
DG_INLINE ViewContext::ContextMark::~ContextMark() {Pop();}

DG_INLINE IElemTopologyP ViewContext::GetElemTopology ()  {return m_currElemTopo;} // graphite moved these out of line
DG_INLINE void           ViewContext::SetElemTopology (IElemTopologyP topo) {m_currElemTopo = topo;} // graphite moved these out of line

DG_INLINE             IDrawGeom::IDrawGeom ()  { }

DG_INLINE ViewFlagsCP IDrawGeom::GetDrawViewFlags() {return _GetDrawViewFlags();}
DG_INLINE void        IDrawGeom::SetDrawViewFlags(ViewFlagsCP flags) {_SetDrawViewFlags(flags);}
DG_INLINE void        IDrawGeom::ActivateMatSymb (ElemMatSymbCP matSymb){_ActivateMatSymb (matSymb);}
DG_INLINE void        IDrawGeom::ActivateOverrideMatSymb (OvrMatSymbCP ovrMatSymb){_ActivateOverrideMatSymb (ovrMatSymb);}

DG_INLINE void        IDrawGeom::DrawLineString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) {_DrawLineString3d (numPoints, points, range);}
DG_INLINE void        IDrawGeom::DrawLineString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) {_DrawLineString2d (numPoints, points, zDepth, range);}
DG_INLINE void        IDrawGeom::DrawPointString3d (int numPoints, DPoint3dCP points, DPoint3dCP range) {_DrawPointString3d (numPoints, points, range);}
DG_INLINE void        IDrawGeom::DrawPointString2d (int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) {_DrawPointString2d (numPoints, points, zDepth, range);}
DG_INLINE void        IDrawGeom::DrawShape3d (int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) {_DrawShape3d (numPoints, points, filled, range);}
DG_INLINE void        IDrawGeom::DrawShape2d (int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) {_DrawShape2d (numPoints, points, filled, zDepth, range);}
DG_INLINE void        IDrawGeom::DrawTriStrip3d (int numPoints, DPoint3dCP points, Int32 usageFlags, DPoint3dCP range) {_DrawTriStrip3d (numPoints, points, usageFlags, range);}
DG_INLINE void        IDrawGeom::DrawTriStrip2d (int numPoints, DPoint2dCP points, Int32 usageFlags, double zDepth, DPoint2dCP range) {_DrawTriStrip2d (numPoints, points, usageFlags, zDepth, range);}
DG_INLINE void        IDrawGeom::DrawArc3d (DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) {_DrawArc3d (ellipse, isEllipse, filled, range);}
DG_INLINE void        IDrawGeom::DrawArc2d (DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) {_DrawArc2d (ellipse, isEllipse, filled, zDepth, range);}
DG_INLINE void        IDrawGeom::DrawBSplineCurve (MSBsplineCurveCR curve, bool filled) {_DrawBSplineCurve (curve, filled);}
DG_INLINE void        IDrawGeom::DrawBSplineCurve2d (MSBsplineCurveCR curve, bool filled, double zDepth)                                { _DrawBSplineCurve2d (curve, filled, zDepth); }
DG_INLINE void        IDrawGeom::DrawCurveVector (CurveVectorCR curves, bool isFilled) {_DrawCurveVector (curves, isFilled);}
DG_INLINE void        IDrawGeom::DrawCurveVector2d (CurveVectorCR curves, bool isFilled, double zDepth) {_DrawCurveVector2d (curves, isFilled, zDepth);}
DG_INLINE void        IDrawGeom::DrawSolidPrimitive (ISolidPrimitiveCR primitive) {_DrawSolidPrimitive (primitive);}
DG_INLINE void        IDrawGeom::DrawBSplineSurface (MSBsplineSurfaceCR surface) {_DrawBSplineSurface (surface);}
DG_INLINE void        IDrawGeom::DrawPolyface (PolyfaceQueryCR meshData, bool filled)                                                   { _DrawPolyface (meshData, filled); }
DG_INLINE StatusInt   IDrawGeom::DrawBody (ISolidKernelEntityCR entity, IFaceMaterialAttachmentsCP attachments, double pixelSize)       { return _DrawBody (entity, attachments, pixelSize); }
DG_INLINE void        IDrawGeom::DrawTextString (TextStringCR text, double* zDepth) {_DrawTextString (text, zDepth);}
DG_INLINE void        IDrawGeom::DrawMosaic (int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) {_DrawMosaic(numX,numY,tileIds,verts);}

DG_INLINE RangeResult IDrawGeom::PushBoundingRange3d (DPoint3dCP range){return _PushBoundingRange3d (range);}
DG_INLINE RangeResult IDrawGeom::PushBoundingRange2d (DPoint2dCP range, double zDepth){return _PushBoundingRange2d (range, zDepth);}
DG_INLINE void        IDrawGeom::PopBoundingRange(){_PopBoundingRange();}
DG_INLINE void        IDrawGeom::DrawTorus(DPoint3dCR  center, DVec3dCR vectorX, DVec3dCR vectorY, double majorRadius, double minorRadius, double sweepAngle, bool capped) { DrawSolidPrimitive (*ISolidPrimitive::CreateDgnTorusPipe (DgnTorusPipeDetail (center, vectorX, vectorY, majorRadius, minorRadius, sweepAngle, capped))); }
DG_INLINE void        IDrawGeom::DrawBox (DVec3dCR primary, DVec3dCR secondary, DPoint3dCR basePoint, DPoint3dCR topPoint, double baseWidth, double baseLength, double topWidth, double topLength, bool capped)  { DrawSolidPrimitive (*ISolidPrimitive::CreateDgnBox (DgnBoxDetail::InitFromCenters (basePoint, topPoint, primary, secondary, baseWidth, baseLength, topWidth, topLength, capped))); }
DG_INLINE size_t      IDrawGeom::GetMethodIndex ()  { return _GetMethodIndex(); }
DG_INLINE void        IDrawGeom::PushMethodState () { _PushMethodState(); }
DG_INLINE void        IDrawGeom::PopMethodState ()  { _PopMethodState(); }

DG_INLINE bool        IViewDraw::IsOutputQuickVision () const { return _IsOutputQuickVision (); }
DG_INLINE bool        IViewDraw::DeferShadowsToHeal () const { return _DeferShadowsToHeal (); }
DG_INLINE bool        IViewDraw::ApplyMonochromeOverrides (ViewFlagsCR flags) const { return _ApplyMonochromeOverrides (flags); }

DG_INLINE void        IViewDraw::SetToViewCoords (bool yesNo){_SetToViewCoords (yesNo);}
DG_INLINE void        IViewDraw::SetSymbology (UInt32 lineColorTBGR, UInt32 fillColorTBGR, int lineWidth, UInt32 linePattern) {_SetSymbology (lineColorTBGR, fillColorTBGR, lineWidth, linePattern);}
DG_INLINE void        IViewDraw::DrawGrid (bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector,UInt32 gridsPerRef, Point2dCR repetitions){_DrawGrid (doIsoGrid, drawDots, gridOrigin, xVector, yVector, gridsPerRef, repetitions);}
DG_INLINE bool        IViewDraw::DrawSprite (ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency){return _DrawSprite (sprite, location, xVec, transparency);}
DG_INLINE void        IViewDraw::DrawTiledRaster (ITiledRaster* tiledRaster){_DrawTiledRaster (tiledRaster);}
DG_INLINE void        IViewDraw::DrawRaster2d (DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, double zDepth, DPoint2dCP range) { _DrawRaster2d (points, pitch, numTexelsX, numTexelsY, enableAlpha, format, texels, zDepth, range); }
DG_INLINE void        IViewDraw::DrawRaster (DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, byte const* texels, DPoint3dCP range) { _DrawRaster (points, pitch, numTexelsX, numTexelsY, enableAlpha, format, texels, range); }
DG_INLINE void        IViewDraw::DrawDgnOle (IDgnOleDraw* ole) {_DrawDgnOle (ole);}
DG_INLINE void        IViewDraw::DrawPointCloud (IPointCloudDrawParams* drawParams) {_DrawPointCloud (drawParams);}
DG_INLINE void        IViewDraw::DrawQvElem3d (QvElem* qvElem, int subElemIndex){_DrawQvElem3d (qvElem, subElemIndex);}
DG_INLINE void        IViewDraw::DrawQvElem2d (QvElem* qvElem, double zDepth, int subElemIndex){_DrawQvElem2d (qvElem, zDepth, subElemIndex);}
DG_INLINE void        IViewDraw::PushRenderOverrides (ViewFlags flags, CookedDisplayStyleCP displayOverrides){_PushRenderOverrides (flags, displayOverrides);}
DG_INLINE void        IViewDraw::PopRenderOverrides (){_PopRenderOverrides ();}
DG_INLINE void        IViewDraw::ClearZ() {_ClearZ(); }
DG_INLINE StatusInt   IViewDraw::TestOcclusion (int numVolumes, DPoint3dP verts, int* results){return _TestOcclusion (numVolumes, verts, results);}
DG_INLINE uintptr_t   IViewDraw::DefineQVTexture (WCharCP textureName, DgnProjectP dgnFile) {return _DefineQVTexture (textureName, dgnFile);}
DG_INLINE void        IViewDraw::DefineQVGeometryMap (MaterialCR material, ElementHandleCR eh, DPoint2dCP spacing, bool useCellColors, ViewContextR seedContext, bool forAreaPattern) {return _DefineQVGeometryMap (material, eh, spacing,  useCellColors, seedContext, forAreaPattern);}
DG_INLINE CookedDisplayStyleCP IViewDraw::GetDrawDisplayStyle () {return _GetDrawDisplayStyle ();}
DG_INLINE void        IViewDraw::PushClipStencil (QvElem* qvElem) {_PushClipStencil (qvElem);}
DG_INLINE void        IViewDraw::PopClipStencil () {_PopClipStencil (); }

DG_INLINE void        ICachedDraw::BeginCacheElement (bool is3d, QvCache* qvCache, ViewFlagsCP flags, uintptr_t elementId){_BeginCacheElement (is3d, qvCache, flags, elementId);}
DG_INLINE QvElem*     ICachedDraw::EndCacheElement () {return _EndCacheElement ();}
DG_INLINE void        ICachedDraw::AssignElementToView (QvView* qvView, QvElem* qvElem, int viewMode) {_AssignElementToView (qvView, qvElem, viewMode);}
DG_INLINE QvElem*     ICachedDraw::GetCacheElement () { return _GetCacheElement(); }
DG_INLINE void        ICachedDraw::SetCacheElement (QvElem* qvElem) { _SetCacheElement (qvElem); }
DG_INLINE void        ICachedDraw::PushTransform (TransformCR trans) {_PushTransClip (&trans);}
DG_INLINE void        ICachedDraw::PopTransform () {_PopTransClip ();}

DG_INLINE void        IViewOutput::AdjustBrightness (bool useFixedAdaptation, double brightness) {_AdjustBrightness (useFixedAdaptation, brightness);}
DG_INLINE UInt64      IViewOutput::GetLightStamp () {return _GetLightStamp ();}
DG_INLINE void        IViewOutput::SetViewAttributes (ViewFlags viewFlags, RgbColorDef const& bgColor, bool usebgTexture, AntiAliasPref aaLines, AntiAliasPref aaText, CookedDisplayStyleCP displayOverrides) {_SetViewAttributes (viewFlags, bgColor, usebgTexture, aaLines, aaText, displayOverrides);}
DG_INLINE DgnDisplayCoreTypes::DeviceContextP      IViewOutput::GetScreenDC() const {return _GetScreenDC();}
DG_INLINE StatusInt   IViewOutput::AssignDC (DgnDisplayCoreTypes::DeviceContextP dc){return _AssignDC (dc);}
DG_INLINE void        IViewOutput::AddLights (bool    threeDview, const RotMatrix* rotMatrixP, DgnModelP model){_AddLights (threeDview, rotMatrixP, model);}
DG_INLINE void        IViewOutput::DefineFrustum (DPoint3dCR frustPts, double fraction, bool is2d){_DefineFrustum (frustPts, fraction, is2d);}
DG_INLINE void        IViewOutput::SetDrawBuffer (DgnDrawBuffer drawBuffer, BSIRectCP subRect){_SetDrawBuffer (drawBuffer, subRect);}
DG_INLINE DgnDrawBuffer IViewOutput::GetDrawBuffer () const{return _GetDrawBuffer ();}
DG_INLINE void        IViewOutput::SetEraseMode (bool newMode){_SetEraseMode (newMode);}
DG_INLINE void        IViewOutput::SetFlashMode (bool newMode){_SetFlashMode (newMode);}
DG_INLINE StatusInt   IViewOutput::SynchDrawingFromBackingStore(){return _SynchDrawingFromBackingStore();}
DG_INLINE void        IViewOutput::SynchDrawingFromBackingStoreAsynch(){_SynchDrawingFromBackingStoreAsynch();}
DG_INLINE StatusInt   IViewOutput::SynchScreenFromDrawing(){return _SynchScreenFromDrawing();}
DG_INLINE void        IViewOutput::SynchScreenFromDrawingAsynch(){_SynchScreenFromDrawingAsynch();}
DG_INLINE bool        IViewOutput::IsScreenDirty (BSIRectP rect){return _IsScreenDirty (rect);}
DG_INLINE void        IViewOutput::ShowProgress(){_ShowProgress();}
DG_INLINE bool        IViewOutput::IsBackingStoreValid() const{return _IsBackingStoreValid();}
DG_INLINE bool        IViewOutput::IsAccelerated () const{return _IsAccelerated ();}
DG_INLINE void        IViewOutput::ScreenDirtied (BSIRectCP rect){_ScreenDirtied (rect);}
DG_INLINE bool        IViewOutput::EnableZTesting (bool yesNo){return _EnableZTesting (yesNo);}
DG_INLINE bool        IViewOutput::EnableZWriting (bool yesNo){return _EnableZWriting (yesNo);}
DG_INLINE void        IViewOutput::SetProjectDepth (double depth){_SetProjectDepth (depth);}
DG_INLINE StatusInt   IViewOutput::BeginDraw (bool eraseBefore){return _BeginDraw (eraseBefore);}
DG_INLINE void        IViewOutput::EndDraw (QvPaintOptions const& opts){_EndDraw (opts);}
DG_INLINE StatusInt   IViewOutput::BeginDrawCapture (){return _BeginDrawCapture ();}
DG_INLINE StatusInt   IViewOutput::EndDrawCapture (){return _EndDrawCapture ();}
DG_INLINE bool        IViewOutput::HaveCapture () const {return _HaveCapture ();}
DG_INLINE void        IViewOutput::ResetCapture (){_ResetCapture ();}
DG_INLINE StatusInt   IViewOutput::DisplayCaptured (ViewFlags flags, DPoint2dCP origin, DPoint2dCP extent, int (*stopProc)()){ return _DisplayCaptured (flags, origin, extent, stopProc); }
DG_INLINE bool        IViewOutput::IsDrawActive(){return _IsDrawActive();}
DG_INLINE void        IViewOutput::ShowTransparent(){_ShowTransparent();}
DG_INLINE void        IViewOutput::AccumulateDirtyRegion(bool val){_AccumulateDirtyRegion(val);}
DG_INLINE void        IViewOutput::ClearHealRegion(){_ClearHealRegion();}
DG_INLINE void        IViewOutput::SetNeedsHeal (BSIRectCP dirty){_SetNeedsHeal (dirty);}
DG_INLINE void        IViewOutput::HealComplete (bool aborted){_HealComplete (aborted);}
DG_INLINE bool        IViewOutput::CheckNeedsHeal (BSIRectP rect){return _CheckNeedsHeal(rect);}
DG_INLINE void        IViewOutput::BeginDecorating (BSIRectCP rect){_BeginDecorating (rect);}
DG_INLINE void        IViewOutput::BeginOverlayMode(){_BeginOverlayMode();}
DG_INLINE void        IViewOutput::OnElementStart (ElementRefP elemRef){_OnElementStart (elemRef);}
DG_INLINE void        IViewOutput::OnElementEnd (ElementRefP elemRef){_OnElementEnd (elemRef);}
DG_INLINE bool        IViewOutput::LocateQvElem (QvElem* qvElem, DPoint2dCR borePt, double radius, DPoint3dR hitPt, DVec3dP hitNormal, int (*stopProc)(CallbackArgP), CallbackArgP arg) {return _LocateQvElem (qvElem, borePt, radius, hitPt, hitNormal, stopProc, arg);}
DG_INLINE void        IViewOutput::AbortOutstandingOperations(){_AbortOutstandingOperations();}
DG_INLINE void        IViewOutput::SetIdleCallback (bool (*callback)(CallbackArgP), CallbackArgP userData){_SetIdleCallback (callback, userData);}
DG_INLINE QvView*     IViewOutput::GetQvView() const{return _GetQvView();}
DG_INLINE void        IViewOutput::DefineColorMap (DgnColorMapCP colorMap){return _DefineColorMap (colorMap);}
DG_INLINE void        IViewOutput::ModifyColorMapEntry (int index, UInt32 color){_ModifyColorMapEntry (index, color);}
DG_INLINE int         IViewOutput::GetHighestUsedColorIndex (QvElem* qvElem){return _GetHighestUsedColorIndex (qvElem);}
DG_INLINE QvCache*    IViewOutput::GetTempElementCache() {return T_HOST.GetGraphicsAdmin()._GetTempElementCache ();}
DG_INLINE void        IViewOutput::DeleteCacheElement (QvElem* qvElem) {return T_HOST.GetGraphicsAdmin()._DeleteQvElem (qvElem);}
DG_INLINE void        IViewOutput::PushTransClip (TransformCP trans, ClipPlaneSetCP clip) {_PushTransClip (trans, clip);}
DG_INLINE void        IViewOutput::PopTransClip () {_PopTransClip ();}
DG_INLINE BentleyStatus IViewOutput::FillImageCaptureBuffer (bvector<UChar>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, bool topDown) {return _FillImageCaptureBuffer (buffer, info, screenBufferRange, topDown);}
DG_INLINE int         IViewOutput::GetVisibleTiles(QvMRImageP mri, size_t bufSize, int* lrc) { return _GetVisibleTiles(mri, bufSize, lrc); }
DG_INLINE void        ITiledRaster::DrawRaster (IViewOutputR viewOutput) { return _DrawRaster(viewOutput); }
DG_INLINE void        ITiledRaster::PrintRaster (IViewOutputR viewOutput) { return _DrawRaster(viewOutput); }

DG_INLINE GradientSymb::GradientSymb () {memset (&m_mode, 0, offsetof (GradientSymb, m_values) + sizeof (m_values) - offsetof (GradientSymb, m_mode));}
DG_INLINE int           GradientSymb::GetNKeys () const {return m_nKeys;}
DG_INLINE GradientMode  GradientSymb::GetMode () const {return m_mode;}
DG_INLINE UInt16        GradientSymb::GetFlags () const {return m_flags;}
DG_INLINE double        GradientSymb::GetShift () const {return m_shift;}
DG_INLINE double        GradientSymb::GetTint () const {return m_tint;}
DG_INLINE double        GradientSymb::GetAngle () const {return m_angle;}
DG_INLINE void          GradientSymb::GetKey (RgbColorDef& color, double& value, int index) const {color = m_colors[index], value = m_values[index];}
DG_INLINE void          GradientSymb::SetMode (GradientMode mode) {m_mode = mode;}
DG_INLINE void          GradientSymb::SetFlags (UInt16 flags) {m_flags = flags;}
DG_INLINE void          GradientSymb::SetAngle (double angle) {m_angle = angle;}
DG_INLINE void          GradientSymb::SetTint (double tint) {m_tint = tint;}
DG_INLINE void          GradientSymb::SetShift (double shift) {m_shift = shift;}
DG_INLINE void          PatternParams::SetModifiers (PatternParamsModifierFlags value) {modifiers = value;}
DG_INLINE void          PatternParams::SetOrientation (RotMatrixCR value) {rMatrix = value; modifiers = modifiers |PatternParamsModifierFlags::RotMatrix;}
DG_INLINE void          PatternParams::SetOffset (DPoint3dCR value) {offset = value; modifiers = modifiers |PatternParamsModifierFlags::Offset;}
DG_INLINE void          PatternParams::SetPrimarySpacing (double value) {space1 = value; modifiers = modifiers |PatternParamsModifierFlags::Space1;}
DG_INLINE void          PatternParams::SetPrimaryAngle (double value) {angle1 = value; modifiers = modifiers |PatternParamsModifierFlags::Angle1;}
DG_INLINE void          PatternParams::SetSecondarySpacing (double value) {space2 = value; modifiers = modifiers |PatternParamsModifierFlags::Space2;}
DG_INLINE void          PatternParams::SetSecondaryAngle (double value) {angle2 = value; modifiers = modifiers |PatternParamsModifierFlags::Angle2;}
DG_INLINE void          PatternParams::SetScale (double value) {scale = value; modifiers = modifiers |PatternParamsModifierFlags::Scale;}
DG_INLINE void          PatternParams::SetTolerance (double value) {tolerance = value; modifiers = modifiers |PatternParamsModifierFlags::Tolerance;}
DG_INLINE void          PatternParams::SetCellName (WCharCP value) {BeStringUtilities::Wcsncpy (cellName, MAX_CELLNAME_LENGTH, value); modifiers = modifiers |PatternParamsModifierFlags::Cell;}
DG_INLINE void          PatternParams::SetCellId (ElementId value) {cellId = value.GetValue(); modifiers = modifiers |PatternParamsModifierFlags::Cell;}
DG_INLINE void          PatternParams::SetMinLine (Int32 value) {minLine = value; modifiers = modifiers |PatternParamsModifierFlags::Multiline;}
DG_INLINE void          PatternParams::SetMaxLine (Int32 value) {maxLine = value; modifiers = modifiers |PatternParamsModifierFlags::Multiline;}
DG_INLINE void          PatternParams::SetColor (UInt32 value) {color = value; modifiers = modifiers |PatternParamsModifierFlags::Color;}
DG_INLINE void          PatternParams::SetWeight (UInt32 value) {weight = value; modifiers = modifiers |PatternParamsModifierFlags::Weight;}
DG_INLINE void          PatternParams::SetStyle (Int32 value) {style = value; modifiers = modifiers |PatternParamsModifierFlags::Style;}
DG_INLINE void          PatternParams::SetHoleStyle (PatternParamsHoleStyleType value) {holeStyle = static_cast<Int16>(value); modifiers = modifiers |PatternParamsModifierFlags::HoleStyle;}
DG_INLINE void          PatternParams::SetDwgHatchDef (DwgHatchDefCR value) {dwgHatchDef = value; modifiers = modifiers |PatternParamsModifierFlags::DwgHatchDef;}
DG_INLINE void          PatternParams::SetOrigin (DPoint3dCR value) {origin = value; modifiers = modifiers |PatternParamsModifierFlags::Origin;}
DG_INLINE void          PatternParams::SetAnnotationScale (double scale) {annotationscale = scale; modifiers = modifiers |PatternParamsModifierFlags::AnnotationScale;}
DG_INLINE PatternParamsModifierFlags PatternParams::GetModifiers () const {return modifiers;}
DG_INLINE RotMatrixCR   PatternParams::GetOrientation () const {return rMatrix;}
DG_INLINE DPoint3dCR    PatternParams::GetOffset () const {return offset;}
DG_INLINE double        PatternParams::GetPrimarySpacing () const {return space1;}
DG_INLINE double        PatternParams::GetPrimaryAngle () const {return angle1;}
DG_INLINE double        PatternParams::GetSecondarySpacing () const {return space2;}
DG_INLINE double        PatternParams::GetSecondaryAngle () const {return angle2;}
DG_INLINE double        PatternParams::GetScale () const {return scale;}
DG_INLINE double        PatternParams::GetTolerance () const {return tolerance;}
DG_INLINE WCharCP       PatternParams::GetCellName () const {return cellName;}
DG_INLINE ElementId     PatternParams::GetCellId () const {return ElementId(cellId);}
DG_INLINE Int32         PatternParams::GetMinLine () const {return minLine;}
DG_INLINE Int32         PatternParams::GetMaxLine () const {return maxLine;}
DG_INLINE UInt32        PatternParams::GetColor () const {return color;}
DG_INLINE UInt32        PatternParams::GetWeight () const {return weight;}
DG_INLINE Int32         PatternParams::GetStyle () const {return style;}
DG_INLINE PatternParamsHoleStyleType   PatternParams::GetHoleStyle () const {return static_cast<PatternParamsHoleStyleType>(holeStyle);}
DG_INLINE DwgHatchDefCR PatternParams::GetDwgHatchDef () const {return dwgHatchDef;}
DG_INLINE DPoint3dCR    PatternParams::GetOrigin () const {return origin;}
DG_INLINE double        PatternParams::GetAnnotationScale () const {return annotationscale;}

DG_INLINE UInt32          ElemMatSymb::GetLineColorTBGR() const {return m_lineColorTBGR;}
DG_INLINE UInt32          ElemMatSymb::GetFillColorTBGR() const {return m_fillColorTBGR;}
DG_INLINE int             ElemMatSymb::GetLineColorIndex() const {return m_lineColorIndex;}
DG_INLINE int             ElemMatSymb::GetFillColorIndex() const {return m_fillColorIndex;}
DG_INLINE UInt32          ElemMatSymb::GetWidth() const {return m_rasterWidth;}
DG_INLINE bool            ElemMatSymb::HasTrueWidth() const {return m_lStyleSymb.HasTrueWidth();}
DG_INLINE UInt32          ElemMatSymb::GetRasterPattern() const {return m_rasterPat;}
DG_INLINE Int32           ElemMatSymb::GetRasterPatternIndex() const {return m_elementStyle;}
DG_INLINE bool            ElemMatSymb::IsFilled() const {return m_isFilled;}
DG_INLINE bool            ElemMatSymb::IsBlankingRegion() const {return m_isBlankingRegion;}
DG_INLINE uintptr_t       ElemMatSymb::GetExtSymbId() const {return m_extSymbID;}
DG_INLINE LineStyleSymbCR ElemMatSymb::GetLineStyleSymb() const {return m_lStyleSymb;}
DG_INLINE GradientSymbCP  ElemMatSymb::GetGradientSymb() const {return m_gradient.get();}
DG_INLINE MaterialCP      ElemMatSymb::GetMaterial () const {return m_material;}
DG_INLINE PatternParamsCP ElemMatSymb::GetPatternParams() const {return m_patternParams.get(); }
DG_INLINE LineStyleSymbR  ElemMatSymb::GetLineStyleSymbR() {return m_lStyleSymb;}

DG_INLINE void        ElemMatSymb::SetLineColorTBGR (UInt32 lineColor) {m_lineColorTBGR = lineColor; m_lineColorIndex = DgnColorMap::INDEX_Invalid;}
DG_INLINE void        ElemMatSymb::SetFillColorTBGR (UInt32 fillColor) {m_fillColorTBGR = fillColor; m_fillColorIndex = DgnColorMap::INDEX_Invalid;}
DG_INLINE void        ElemMatSymb::SetIndexedLineColorTBGR (int index, UInt32 lineColor) {m_lineColorIndex = index; m_lineColorTBGR = lineColor;}
DG_INLINE void        ElemMatSymb::SetIndexedFillColorTBGR (int index, UInt32 fillColor) {m_fillColorIndex = index; m_fillColorTBGR = fillColor;}
DG_INLINE void        ElemMatSymb::SetIsFilled (bool filled) {m_isFilled = filled;}
DG_INLINE void        ElemMatSymb::SetIsBlankingRegion (bool blanking) {m_isBlankingRegion = blanking;}
DG_INLINE void        ElemMatSymb::SetWidth (UInt32 rasterWidth){m_rasterWidth = rasterWidth;}
DG_INLINE void        ElemMatSymb::SetRasterPattern (UInt32 rasterPat) {m_rasterPat = rasterPat; m_elementStyle = 0; m_extSymbID = 0;}
DG_INLINE void        ElemMatSymb::SetIndexedRasterPattern (Int32 index, UInt32 rasterPat) {m_elementStyle = IS_LINECODE (index) ? index : 0; m_rasterPat = rasterPat; m_extSymbID = 0;}
DG_INLINE void        ElemMatSymb::SetExtSymbId (uintptr_t extSymbID) {m_extSymbID = extSymbID;}
DG_INLINE void        ElemMatSymb::SetPatternParams (PatternParamsPtr& patternParams) {m_patternParams = patternParams;}
DG_INLINE void        ElemMatSymb::SetGradient (GradientSymbP gradient) { m_gradient = gradient; }

DG_INLINE bool        OvrMatSymb::operator==(OvrMatSymbCR rhs) const {if (this == &rhs) return true; if (rhs.m_flags != m_flags) return false; return rhs.m_matSymb == m_matSymb;}
DG_INLINE UInt32      OvrMatSymb::GetFlags()                const {return m_flags;}
DG_INLINE UInt32      OvrMatSymb::GetLineColorTBGR()        const {return m_matSymb.GetLineColorTBGR();}
DG_INLINE UInt32      OvrMatSymb::GetFillColorTBGR()        const {return m_matSymb.GetFillColorTBGR();}
DG_INLINE int         OvrMatSymb::GetLineColorIndex()       const {return m_matSymb.GetLineColorIndex();}
DG_INLINE int         OvrMatSymb::GetFillColorIndex()       const {return m_matSymb.GetFillColorIndex();}
DG_INLINE UInt32      OvrMatSymb::GetWidth()                const {return m_matSymb.GetWidth();}
DG_INLINE UInt32      OvrMatSymb::GetRasterPattern()        const {return m_matSymb.GetRasterPattern();}
DG_INLINE Int32       OvrMatSymb::GetRasterPatternIndex ()  const {return m_matSymb.GetRasterPatternIndex ();}
DG_INLINE uintptr_t   OvrMatSymb::GetExtSymbId()            const {return m_matSymb.GetExtSymbId();}
DG_INLINE MaterialCP  OvrMatSymb::GetMaterial ()            const {return m_matSymb.GetMaterial();}
DG_INLINE PatternParamsCP    OvrMatSymb::GetPatternParams() const {return m_matSymb.GetPatternParams (); }
DG_INLINE void        OvrMatSymb::SetFlags(UInt32 flags) {m_flags = flags;}
DG_INLINE void        OvrMatSymb::SetLineColorTBGR (UInt32 color) {m_matSymb.SetLineColorTBGR (color); m_flags |= MATSYMB_OVERRIDE_Color;}
DG_INLINE void        OvrMatSymb::SetFillColorTBGR (UInt32 color) {m_matSymb.SetFillColorTBGR (color); m_flags |= MATSYMB_OVERRIDE_FillColor;}
DG_INLINE void        OvrMatSymb::SetIndexedLineColorTBGR (int index, UInt32 color) {m_matSymb.SetIndexedLineColorTBGR (index, color); m_flags |= MATSYMB_OVERRIDE_Color;}
DG_INLINE void        OvrMatSymb::SetIndexedFillColorTBGR (int index, UInt32 color) {m_matSymb.SetIndexedFillColorTBGR (index, color); m_flags |= MATSYMB_OVERRIDE_FillColor;}
DG_INLINE void        OvrMatSymb::SetTransparentLineColor (UInt32 trans) {m_matSymb.SetIndexedLineColorTBGR (m_matSymb.GetLineColorIndex(), (trans << 24) | (m_matSymb.GetLineColorTBGR() & 0xffffff)); m_flags |= MATSYMB_OVERRIDE_ColorTransparency;}
DG_INLINE void        OvrMatSymb::SetTransparentFillColor (UInt32 trans) {m_matSymb.SetIndexedFillColorTBGR (m_matSymb.GetFillColorIndex(), (trans << 24) | (m_matSymb.GetFillColorTBGR() & 0xffffff)); m_flags |= MATSYMB_OVERRIDE_FillColorTransparency;}
DG_INLINE void        OvrMatSymb::SetWidth     (UInt32 width)  {m_matSymb.SetWidth (width); m_flags |= MATSYMB_OVERRIDE_RastWidth;}
DG_INLINE void        OvrMatSymb::SetRasterPattern (UInt32 rasterPat) {m_matSymb.SetRasterPattern (rasterPat); m_flags |= MATSYMB_OVERRIDE_Style; m_matSymb.GetLineStyleSymbR().SetLineStyle (NULL);}
DG_INLINE void        OvrMatSymb::SetIndexedRasterPattern (Int32 index, UInt32 rasterPat) {m_matSymb.SetIndexedRasterPattern (index, rasterPat); m_flags |= MATSYMB_OVERRIDE_Style; m_matSymb.GetLineStyleSymbR().SetLineStyle (NULL);}
DG_INLINE void        OvrMatSymb::SetExtSymbId (uintptr_t extSymbID) {m_matSymb.SetExtSymbId (extSymbID); m_flags |= MATSYMB_OVERRIDE_ExtSymb;}
DG_INLINE void        OvrMatSymb::SetMaterial (MaterialCP material, ViewContextP seedContext) {m_matSymb.SetMaterial (material, seedContext); m_flags |= MATSYMB_OVERRIDE_RenderMaterial;}
DG_INLINE void        OvrMatSymb::SetProxy     (bool edge, bool hidden) {m_flags |= (MATSYMB_OVERRIDE_IsProxy | (edge ? MATSYMB_OVERRIDE_IsProxyEdge : 0) | (hidden ? MATSYMB_OVERRIDE_IsProxyHidden: 0)); }
DG_INLINE void        OvrMatSymb::SetUnderlay  () { m_flags |= MATSYMB_OVERRIDE_IsProxyUnderlay; }
DG_INLINE bool        OvrMatSymb::GetProxy     (bool& edge, bool& hidden) {edge = 0 != (m_flags & MATSYMB_OVERRIDE_IsProxyEdge); hidden = 0 != (m_flags & MATSYMB_OVERRIDE_IsProxyHidden); return 0 != (m_flags & MATSYMB_OVERRIDE_IsProxy); }
DG_INLINE void        OvrMatSymb::SetPatternParams (PatternParamsPtr& patternParams){m_matSymb.SetPatternParams(patternParams); m_flags |= MATSYMB_OVERRIDE_PatternParams;}

DG_INLINE SCOverride        ElemHeaderOverrides::GetFlags () const {return m_flags;}
DG_INLINE LevelId           ElemHeaderOverrides::GetLevel () const {return m_level;}
DG_INLINE UInt32            ElemHeaderOverrides::GetColor () const {return m_symb.color;}
DG_INLINE UInt32            ElemHeaderOverrides::GetWeight () const {return m_symb.weight;}
DG_INLINE Int32             ElemHeaderOverrides::GetLineStyle () const {return m_symb.style;}
DG_INLINE LineStyleParamsCP ElemHeaderOverrides::GetLineStyleParams () const {return &m_styleParams;}
DG_INLINE UInt32            ElemHeaderOverrides::GetDisplayPriority () const {return m_dispPriority;}
DG_INLINE DgnElementClass   ElemHeaderOverrides::GetElementClass () const {return (DgnElementClass) m_elementClass;}

DG_INLINE bool              ElemDisplayParams::IsRenderable () const {return m_isRenderable;}
DG_INLINE bool              ElemDisplayParams::IsAttachedMaterial () const {return m_materialIsAttached;}
DG_INLINE bool              ElemDisplayParams::IsLevelSymbIgnored () const {return m_ignoreLevelSymb;}
DG_INLINE bool              ElemDisplayParams::IsLineColorTBGR () const {return INVALID_COLOR == GetLineColor ();}
DG_INLINE UInt32            ElemDisplayParams::GetLineColor () const {return m_symbology.color;};
DG_INLINE UInt32            ElemDisplayParams::GetLineColorIndex () const {return m_lineColorIndex;};
DG_INLINE UInt32            ElemDisplayParams::GetLineColorTBGR () const {BeDataAssert (m_isValidLineColorTBGR && "ERROR: ElemDisplayParams::Cook has not been called!"); return m_lineColorTBGR;};
DG_INLINE bool              ElemDisplayParams::IsFillColorTBGR () const {return FillDisplay::Never != m_fillDisplay && !m_gradient.IsValid () && INVALID_COLOR == GetFillColor ();}
DG_INLINE UInt32            ElemDisplayParams::GetFillColor () const {return FillDisplay::Never != m_fillDisplay && !m_gradient.IsValid () ? m_fillColor : INVALID_COLOR;};
DG_INLINE UInt32            ElemDisplayParams::GetFillColorIndex () const {return m_fillColorIndex;};
DG_INLINE UInt32            ElemDisplayParams::GetFillColorTBGR () const {BeDataAssert (m_isValidFillColorTBGR && "ERROR: ElemDisplayParams::Cook has not been called!"); return FillDisplay::Never != m_fillDisplay && !m_gradient.IsValid () ? m_fillColorTBGR : 0;};
DG_INLINE FillDisplay       ElemDisplayParams::GetFillDisplay () const {return m_fillDisplay;}
DG_INLINE GradientSymbCP    ElemDisplayParams::GetGradient () const {return m_gradient.get(); }
DG_INLINE Int32             ElemDisplayParams::GetLineStyle () const {return m_symbology.style;}
DG_INLINE LineStyleParamsCP ElemDisplayParams::GetLineStyleParams () const {return (0 != m_styleParams.modifiers ? &m_styleParams : NULL);}
DG_INLINE UInt32            ElemDisplayParams::GetWeight () const {return m_symbology.weight;}
DG_INLINE DgnElementClass   ElemDisplayParams::GetElementClass () const {return (DgnElementClass) m_elementClass;}
DG_INLINE MaterialCP        ElemDisplayParams::GetMaterial () const {return m_material;}
DG_INLINE Int32             ElemDisplayParams::GetElementDisplayPriority () const {return m_elmPriority;}
DG_INLINE Int32             ElemDisplayParams::GetNetDisplayPriority () const {return m_netPriority;}
DG_INLINE double            ElemDisplayParams::GetTransparency () const {return m_transparency;}
DG_INLINE DVec3dCP          ElemDisplayParams::GetThickness (bool& isCapped) const {if (!m_hasThickness) return NULL; isCapped = m_isCapped; return &m_thicknessVector;}
DG_INLINE void              ElemDisplayParams::SetIsRenderable (bool isRenderable) {m_isRenderable = isRenderable;}
DG_INLINE void              ElemDisplayParams::SetLevelSymbIgnored (bool isIgnored) {m_ignoreLevelSymb = isIgnored;};
DG_INLINE void              ElemDisplayParams::SetLineColorIndex (UInt32 index) {m_lineColorIndex = index;}
DG_INLINE void              ElemDisplayParams::SetFillColorIndex (UInt32 index) {m_fillColorIndex = index;}
DG_INLINE void              ElemDisplayParams::SetSubLevelId (SubLevelId subLevel) {m_subLevel = subLevel;}
DG_INLINE void              ElemDisplayParams::SetLineStyle (Int32 style, LineStyleParamsCP params) {m_symbology.style = style; if (params) m_styleParams = *params; else m_styleParams.modifiers = 0; }
DG_INLINE void              ElemDisplayParams::SetWeight (UInt32 weight) {m_symbology.weight = weight;}
DG_INLINE void              ElemDisplayParams::SetElementClass (DgnElementClass elmClass) {m_elementClass = elmClass;}
DG_INLINE void              ElemDisplayParams::SetTransparency (double transparency) {m_transparency = transparency;}
DG_INLINE void              ElemDisplayParams::SetThickness (DVec3dCP direction, bool isCapped) {if (direction) {m_hasThickness = true; m_thicknessVector = *direction; m_isCapped = isCapped;} else {m_hasThickness = false;}};
DG_INLINE void              ElemDisplayParams::SetMaterial (MaterialCP material, bool isAttached) {m_material = material; m_materialIsAttached = (isAttached && (0 != m_material));}
DG_INLINE void              ElemDisplayParams::SetNetDisplayPriority (Int32 priority) {m_netPriority = priority;}
DG_INLINE void              ElemDisplayParams::SetFillDisplay (FillDisplay display) { m_fillDisplay = display; }
DG_INLINE void              ElemDisplayParams::SetGradient (GradientSymbP gradient) { m_gradient = gradient; }
DG_INLINE bool              ElemDisplayParams::IsScreeningSet () const {return m_hasScreening;}
DG_INLINE double            ElemDisplayParams::GetScreening () const {return m_screening;}
DG_INLINE void              ElemDisplayParams::SetScreening (double screen, bool set) {if (set) m_screening = screen; m_hasScreening = set;};
DG_INLINE bool              ElemDisplayParams::IsLineJoinSet () const {return m_hasLineJoin;}
DG_INLINE LineJoin          ElemDisplayParams::GetLineJoin () const {return m_lineJoin;}
DG_INLINE void              ElemDisplayParams::SetLineJoin (LineJoin join, bool set) {if (set) m_lineJoin = join; m_hasLineJoin = set;};
DG_INLINE bool              ElemDisplayParams::IsLineCapSet () const {return m_hasLineCap;}
DG_INLINE LineCap           ElemDisplayParams::GetLineCap () const {return m_lineCap;}
DG_INLINE void              ElemDisplayParams::SetLineCap (LineCap cap, bool set) {if (set) m_lineCap = cap; m_hasLineCap = set;};
DG_INLINE bool              ElemDisplayParams::IsLineWeightMMSet () const {return m_hasLineWeightMM;}
DG_INLINE double            ElemDisplayParams::GetLineWeightMM () const {return m_widthMM;}
DG_INLINE void              ElemDisplayParams::SetLineWeightMM (double mm, bool set) {if (set) m_widthMM = mm; m_hasLineWeightMM = set;};
DG_INLINE void              LineStyleParams::SetScale (double inScale) { modifiers |= STYLEMOD_SCALE; scale = inScale; }

DG_INLINE DPoint4dCR      IPickGeom::GetPickPointView () const {return _GetPickPointView ();}
DG_INLINE DPoint3dCR      IPickGeom::GetPickPointWorld () const {return _GetPickPointWorld ();}
DG_INLINE GeomDetail&     IPickGeom::GetGeomDetail () {return _GetGeomDetail ();}
DG_INLINE bool            IPickGeom::IsPointVisible (DPoint3dCP screenPt) {return _IsPointVisible (screenPt);}
DG_INLINE void            IPickGeom::SetHitPriorityOverride (HitPriority priority) {_SetHitPriorityOverride (priority);}
DG_INLINE void            IPickGeom::AddHit (DPoint4dCR hitPtScreen, DPoint3dCP hitPtLocal, HitPriority priority) {_AddHit (hitPtScreen, hitPtLocal, priority);}
DG_INLINE bool            IPickGeom::IsSnap () const {return _IsSnap();}
DG_INLINE DRay3d          IPickGeom::GetBoresite () const { return _GetBoresite();}

DG_INLINE StatusInt     ViewController::VisitPath (DisplayPathCP displayPath, void* arg, ViewContextR context) const{ return _VisitPath (displayPath, arg, context); }
DG_INLINE void          ViewController::DrawView (ViewContextR context) {return _DrawView(context);}
DG_INLINE ITxn&         ITxnManager::GetCurrentTxn() {return *m_currTxn;}
DG_INLINE DgnProjectR   ITxnManager::GetDgnProject() {return m_project;}

DG_INLINE bool          IAnnotationHandler::HasAnnotationScale (double* annotationScale, ElementHandleCR element) const { return _GetAnnotationScale (annotationScale, element); }
DG_INLINE bool          IAnnotationHandler::GetAnnotationScale (double* annotationScale, ElementHandleCR element) const { return _GetAnnotationScale (annotationScale, element); }
DG_INLINE StatusInt     IAnnotationHandler::AddAnnotationScale (EditElementHandleR eh, DgnModelP model) {return _AddAnnotationScale (eh, model);}
DG_INLINE StatusInt     IAnnotationHandler::RemoveAnnotationScale (EditElementHandleR eh) {return _RemoveAnnotationScale (eh);}

DG_INLINE SnapPathP     SnapContext::GetSnapPath() {return m_snapPath;}
DG_INLINE SnapMode      SnapContext::GetSnapMode() {return m_snapMode;}
DG_INLINE int           SnapContext::GetSnapDivisor() {return m_snapDivisor;}

DG_INLINE DgnPlatformLib::Host& DgnPlatformLib::GetHost() {return *static_cast<DgnPlatformLib::Host*> (g_hostForThread.GetValueAsPointer());}

DG_INLINE void          IDrawElementAgenda::DrawElementAgenda (ElementAgendaR agenda, DgnDrawMode drawMode, DrawPurpose purpose) {_DrawElementAgenda (agenda, drawMode, purpose);}
DG_INLINE Utf8StringCR                  Material::GetName () const                  { return m_name; }
DG_INLINE Utf8StringR                   Material::GetNameR ()                       { return m_name; }
DG_INLINE Utf8StringCR                  Material::GetPalette () const               { return m_palette; }
DG_INLINE Utf8StringR                   Material::GetPaletteR ()                    { return m_palette; }
DG_INLINE DgnProjectR                   Material::GetDgnProjectR () const           { return *m_dgnProject; }
DG_INLINE MaterialSettingsCR            Material::GetSettings () const              { return m_settings; }
DG_INLINE MaterialSettingsR             Material::GetSettingsR ()                   { return m_settings; }
DG_INLINE bool                          Material::GetSentToQV () const              { return m_sentToQV; }
DG_INLINE void                          Material::SetSentToQV (bool sentToQV) const { m_sentToQV = sentToQV; }

DG_INLINE WChar             AngleFormatter::GetDecimalSeparator () const                    { return m_decimalSeparator; }
DG_INLINE AngleMode         AngleFormatter::GetAngleMode        () const                    { return m_angleMode; }
DG_INLINE AnglePrecision    AngleFormatter::GetAnglePrecision   () const                    { return m_precision; }
DG_INLINE bool              AngleFormatter::GetLeadingZero      () const                    { return m_leadingZero; }
DG_INLINE bool              AngleFormatter::GetTrailingZeros    () const                    { return m_trailingZeros; }
DG_INLINE bool              AngleFormatter::GetAllowNegative    () const                    { return m_allowNegative; }
DG_INLINE bool              AngleFormatter::GetAllowUnclamped   () const                    { return m_allowUnclamped; };
DG_INLINE void              AngleFormatter::SetAngleMode        (AngleMode newVal)          { m_angleMode = newVal; }
DG_INLINE void              AngleFormatter::SetAnglePrecision   (AnglePrecision newVal)     { m_precision = newVal; }
DG_INLINE void              AngleFormatter::SetDecimalSeparator (WChar newVal)              { m_decimalSeparator = newVal; }
DG_INLINE void              AngleFormatter::SetLeadingZero      (bool newVal)               { m_leadingZero = newVal; }
DG_INLINE void              AngleFormatter::SetTrailingZeros    (bool newVal)               { m_trailingZeros = newVal; }
DG_INLINE void              AngleFormatter::SetAllowNegative    (bool newVal)               { m_allowNegative = newVal; }
DG_INLINE void              AngleFormatter::SetAllowUnclamped   (bool newVal)               { m_allowUnclamped = newVal; }

DG_INLINE AngleFormatterR   DirectionFormatter::GetAngleFormatter   ()                      { return *m_angleFormatter; }
DG_INLINE DirectionMode     DirectionFormatter::GetDirectionMode    () const                { return m_mode; }
DG_INLINE bool              DirectionFormatter::GetAddTrueNorth     () const                { return m_addTrueNorth; }
DG_INLINE double            DirectionFormatter::GetTrueNorthValue   () const                { return m_trueNorth; }
DG_INLINE double            DirectionFormatter::GetBaseDirection    () const                { return m_baseDirection; }
DG_INLINE bool              DirectionFormatter::GetClockwise        () const                { return m_clockwise; }
DG_INLINE bool              DirectionFormatter::GetBearingSpaces    () const                { return m_bearingSpaces; }
DG_INLINE void              DirectionFormatter::SetDirectionMode    (DirectionMode newVal)  { m_mode = newVal; }
DG_INLINE void              DirectionFormatter::SetAddTrueNorth     (bool newVal)           { m_addTrueNorth = newVal; }
DG_INLINE void              DirectionFormatter::SetTrueNorthValue   (double newVal)         { m_trueNorth = newVal; }
DG_INLINE void              DirectionFormatter::SetBaseDirection    (double newVal)         { m_baseDirection = newVal; }
DG_INLINE void              DirectionFormatter::SetClockwise        (bool newVal)           { m_clockwise = newVal; }
DG_INLINE void              DirectionFormatter::SetBearingSpaces    (bool newVal)           { m_bearingSpaces = newVal; }

DG_INLINE DgnUnitFormat     DistanceFormatter::GetUnitFormat () const                       { return m_unitFormat; }
DG_INLINE UnitDefinitionCR  DistanceFormatter::GetMasterUnit () const                       { return m_masterUnit; }
DG_INLINE UnitDefinitionCR  DistanceFormatter::GetSubUnit () const                          { return m_subUnit; }
DG_INLINE double            DistanceFormatter::GetScaleFactor () const                      { return m_scaleFactor; }
DG_INLINE bool              DistanceFormatter::GetUnitLabelFlag () const                    { return m_unitFlag; }
DG_INLINE bool              DistanceFormatter::GetSuppressZeroMasterUnits () const          { return m_suppressZeroMasterUnits; }
DG_INLINE bool              DistanceFormatter::GetSuppressZeroSubUnits () const             { return m_suppressZeroSubUnits; }
DG_INLINE bool              DistanceFormatter::GetIsDgnCoordReadOutCapable () const         { return m_isDgnCoordReadOutCapable; }
DG_INLINE void              DistanceFormatter::SetUnitFormat (DgnUnitFormat newVal)         { m_unitFormat = newVal; }
DG_INLINE void              DistanceFormatter::SetUnitLabelFlag (bool newVal)               { m_unitFlag = newVal; }
DG_INLINE void              DistanceFormatter::SetSuppressZeroMasterUnits (bool newVal)     { m_suppressZeroMasterUnits = newVal; }
DG_INLINE void              DistanceFormatter::SetSuppressZeroSubUnits (bool newVal)        { m_suppressZeroSubUnits = newVal; }
DG_INLINE void              DistanceFormatter::SetIsDgnCoordReadOutCapable (bool newVal)    { m_isDgnCoordReadOutCapable = newVal; }
DG_INLINE void              DistanceFormatter::SetPrecisionByte (byte newVal)               { m_precisionByte = newVal; }
DG_INLINE bool              DistanceFormatter::GetUseDWGFormattingLogic () const            { return m_useDWGFormattingLogic; }
DG_INLINE void              DistanceFormatter::SetUseDWGFormattingLogic (bool newVal)       { m_useDWGFormattingLogic = newVal; }
DG_INLINE DwgUnitFormat     DistanceFormatter::GetDWGUnitFormat () const                    { return m_dwgUnitFormat; }

DG_INLINE bool              PointFormatter::GetIs3d () const                                { return m_is3d; }
DG_INLINE void              PointFormatter::SetIs3d (bool newVal)                           { m_is3d = newVal; }

DG_INLINE UnitDefinitionCR  AreaOrVolumeFormatterBase::GetMasterUnit () const               { return m_masterUnit; }
DG_INLINE double AreaOrVolumeFormatterBase::GetScaleFactor () const { return m_scaleFactor; }
DG_INLINE bool              AreaOrVolumeFormatterBase::GetShowUnitLabel () const            { return m_showUnitLabel; }
DG_INLINE bool              AreaOrVolumeFormatterBase::GetLabelDecoratorAsSuffix () const   { return m_labelDecoratorAsSuffix; }
DG_INLINE void              AreaOrVolumeFormatterBase::SetShowUnitLabel (bool newVal)       { m_showUnitLabel = newVal; }
DG_INLINE bool              AreaOrVolumeFormatterBase::GetUseDWGFormattingLogic () const      { return m_useDWGFormattingLogic; }
DG_INLINE void              AreaOrVolumeFormatterBase::SetUseDWGFormattingLogic (bool newVal) { m_useDWGFormattingLogic = newVal; }
DG_INLINE void              AreaOrVolumeFormatterBase::SetLabelDecoratorAsSuffix (bool newVal) { m_labelDecoratorAsSuffix = newVal; }
DG_INLINE DwgUnitFormat     AreaOrVolumeFormatterBase::GetDWGUnitFormat () const            { return m_dwgUnitFormat; }

DG_INLINE void              AreaFormatter::SetPrecisionByte (byte newVal)                   { m_precisionByte = newVal; }

DG_INLINE void              AngleParser::SetAngleMode (AngleMode mode)                      { m_angleMode = mode; }
DG_INLINE AngleMode         AngleParser::GetAngleMode ()                                    { return m_angleMode; }

DG_INLINE AngleParserR      DirectionParser::GetAngleParser ()                              { return *m_angleParser; }
DG_INLINE void              DirectionParser::SetTrueNorthValue(double trueNorth)            { m_trueNorth = trueNorth; }
DG_INLINE double            DirectionParser::GetTrueNorthValue()                            { return m_trueNorth; }
DG_INLINE void              DirectionParser::SetDirectionMode (DirectionMode newVal)        { m_mode = newVal; }
DG_INLINE DirectionMode     DirectionParser::GetDirectionMode ()                            { return m_mode; }
DG_INLINE void              DirectionParser::SetClockwise     (bool isCw)                   { m_isClockwise = isCw; }
DG_INLINE bool              DirectionParser::GetClockwise     ()                            { return m_isClockwise; }
DG_INLINE void              DirectionParser::SetBaseDirection (double newVal)               { m_baseDirection = newVal; }
DG_INLINE double            DirectionParser::GetBaseDirection ()                            { return m_baseDirection; }

DG_INLINE void              DistanceParser::SetMasterUnitLabel (WCharCP label)              { m_masterUnitLabel = label; }
DG_INLINE WCharCP           DistanceParser::GetMasterUnitLabel ()                           { return m_masterUnitLabel.c_str(); }
DG_INLINE void              DistanceParser::SetSubUnitLabel    (WCharCP label)              { m_subUnitLabel = label; }
DG_INLINE WCharCP           DistanceParser::GetSubUnitLabel    ()                           { return m_subUnitLabel.c_str(); }
DG_INLINE void              DistanceParser::SetMasterUnitScale (double scale)               { m_masterUnitScale = scale; }
DG_INLINE double            DistanceParser::GetMasterUnitScale ()                           { return m_masterUnitScale; }
DG_INLINE void              DistanceParser::SetSubUnitScale    (double scale)               { m_subUnitScale = scale; }
DG_INLINE double            DistanceParser::GetSubUnitScale    ()                           { return m_subUnitScale; }
DG_INLINE void              DistanceParser::SetScale           (double scale)               { m_scale = scale; }
DG_INLINE double            DistanceParser::GetScale           ()                           { return m_scale; }

DG_INLINE DistanceParserR   PointParser::GetDistanceParser ()                               { return *m_distanceParser; }
DG_INLINE void              PointParser::SetIs3d (bool is3d)                                { m_is3d = is3d; }
DG_INLINE bool              PointParser::GetIs3d () const                                   { return m_is3d; }

DG_INLINE void              AreaOrVolumeParser::SetMasterUnitScale (double scale)                   { m_masterUnitScale = scale; }
DG_INLINE double            AreaOrVolumeParser::GetMasterUnitScale ()                               { return m_masterUnitScale; }
DG_INLINE void              AreaOrVolumeParser::SetScale           (double scale)                   { m_scale = scale; }
DG_INLINE double            AreaOrVolumeParser::GetScale           ()                               { return m_scale; }

#if defined (NEEDS_WORK_DGNITEM)
DG_INLINE void                                          DisplayStyleHandler::ApplySymbologyOverrides (ViewContextR context) const                { _ApplySymbologyOverrides (context); }
#endif

DG_INLINE bool                                          DisplayFilterHandler::DoConditionalDraw (ViewContextR viewContext, ElementHandleCP element, void const* data, size_t dataSize) const { return _DoConditionalDraw (viewContext, element, data, dataSize); }
DG_INLINE StatusInt                                     DisplayFilterHandler::OnTransform(TransformInfoCR transform, void* pData, size_t dataSize) const                                     { return _OnTransform (transform, pData, dataSize); }
DG_INLINE WString                                       DisplayFilterHandler::GetDumpString (void const* data, size_t dataSize, DgnProjectR project) const                                   { return _GetDumpString (data, dataSize, project); }
//DG_INLINE void                                          DisplayFilterHandler::DoClone (void* data, size_t dataSize, ElementCopyContextR context) const                                       { return _DoClone (data, dataSize, context); }
DG_INLINE StatusInt                                     DisplayFilterHandler::OnWriteToElement (void* data, size_t dataSize, ElementHandleCR eh) const                                       { return _OnWriteToElement (data, dataSize, eh); }
DG_INLINE bool                                          DisplayFilterHandler::IsEqual (void const* data, void const* rhsData, size_t dataSize, double distanceTolerance)                     { return _IsEqual (data, rhsData, dataSize, distanceTolerance); }
DG_INLINE BentleyStatus                                 DisplayFilterHandler::GetExpressionData (bvector<byte>& data, WCharCP expression, DgnProjectR project) const                         { return _GetExpressionData (data, expression, project); }

// Moved here in graphite from other .h files:
DG_INLINE DgnProjectP          ElementRef::GetDgnProject() const {return m_dgnModel ? &m_dgnModel->GetDgnProject() : NULL;}
DG_INLINE DgnModel*            ElementRef::GetDgnModelP () const {return m_dgnModel;}
DG_INLINE QvCache*             ElementRef::GetMyQvCache () {return _GetMyQvCache();}
DG_INLINE HeapZone&            ElementRef::GetHeapZone () {return _GetHeapZone();}
DG_INLINE ElementRefType       ElementRef::GetRefType () {return _GetRefType();}
DG_INLINE DgnElementCP          ElementRef::GetUnstableMSElementCP() const   {return _GetElemPtrC();}
DG_INLINE size_t               ElementRef::GetElement (DgnElementP out, size_t outSize) const {return _GetElement (out, outSize);}
DG_INLINE bool                 ElementRef::SetQvElem (QvElem* qvElem, UInt32 id) {return _SetQvElem (qvElem, id);}
DG_INLINE void                 ElementRef::SetDirtyFlags (DirtyFlags flags) {_SetDirtyFlags(flags);}
DG_INLINE StatusInt            ElementRef::DeleteElement () {return _DeleteElement ();}
DG_INLINE StatusInt            ElementRef::UndeleteElement() {return _UndeleteElement();}
DG_INLINE bool                 ElementRef::IsDeleted() const {return m_flags.deletedRef;}
DG_INLINE UInt32               ElementRef::AddRef() const {return _AddRef();}
DG_INLINE UInt32               ElementRef::Release() const {return _Release();}
DG_INLINE ElementId            ElementRef::GetElementId () const {return m_elementId;}
DG_INLINE DgnItemId            ElementRef::GetItemId () const {return m_itemId;}
DG_INLINE size_t               ElementRef::GetMemorySize() const {return GetElementHeaderCP()->Size();}
DG_INLINE LevelId              ElementRef::GetLevel() const {return GetElementHeaderCP()->GetLevel();}
DG_INLINE bool                 ElementRef::IsUndisplayed() const {return m_flags.undisplayed;}
DG_INLINE void                 ElementRef::SetUndisplayedFlag (bool yesNo) {m_flags.undisplayed = yesNo;}
DG_INLINE ElementHiliteState   ElementRef::IsHilited() const {return (ElementHiliteState) m_flags.hiliteState;}

DG_INLINE double               TextString::GetWidth () const { DRange2dCR extents = GetExtents (); return (extents.high.x - extents.low.x); }
DG_INLINE double               TextString::GetHeight () const { DRange2dCR extents = GetExtents (); return (extents.high.y - extents.low.y); }
DG_INLINE bool                 TextString::IsRscFont () const { return (DgnFontType::Rsc == GetProperties ().GetFont ().GetType ()); }
DG_INLINE bool                 TextString::IsShxFont () const { return (DgnFontType::Shx == GetProperties ().GetFont ().GetType ()); }
DG_INLINE bool                 TextString::IsTrueTypeFont () const { return (DgnFontType::TrueType == GetProperties ().GetFont ().GetType ()); }
DG_INLINE bool                 TextString::IsUnicodeFont () const { TextStringPropertiesCR props = GetProperties (); return (NULL == props.GetShxBigFontCP () && (LangCodePage::Unicode == props.GetFont ().GetCodePage ())); }

