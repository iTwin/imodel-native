/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCoreDLLInlines.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

DG_INLINE double IViewClipObject::GetSize(ClipVolumeSizeProp clipVolumeCropProp) const {return _GetSize(clipVolumeCropProp);}
DG_INLINE void IViewClipObject::SetSize(ClipVolumeSizeProp clipVolumeSizeProp, double size) {return _SetSize(clipVolumeSizeProp, size);}
DG_INLINE bool IViewClipObject::GetCrop(ClipVolumeCropProp clipVolumeCropProp) const {return _GetCrop(clipVolumeCropProp);}
DG_INLINE void IViewClipObject::SetCrop(ClipVolumeCropProp clipVolumeCropProp, bool crop) {return _SetCrop(clipVolumeCropProp, crop);}
DG_INLINE RotMatrixCR IViewClipObject::GetRotationMatrix() const {return _GetRotationMatrix();}
DG_INLINE void IViewClipObject::SetRotationMatrix(RotMatrixCR rMatrix) {return _SetRotationMatrix(rMatrix);}
DG_INLINE void IViewClipObject::SetPoints(size_t numPoints, DPoint3dCP points) {return _SetPoints(numPoints,points);}
DG_INLINE size_t IViewClipObject::GetNumPoints() const {return _GetNumPoints();}
DG_INLINE StatusInt IViewClipObject::GetPoints(DPoint3dVector& points, size_t iFromPoint, size_t numPoints) const {return _GetPoints(points, iFromPoint, numPoints);}
DG_INLINE void IViewClipObject::CopyCrops(IViewClipObject const* from) {return _CopyCrops(from);}
DG_INLINE void IViewClipObject::SetPreserveUp(bool flag) {return _SetPreserveUp(flag);}
DG_INLINE bool IViewClipObject::GetPreserveUp() const {return _GetPreserveUp();}
DG_INLINE double IViewClipObject::GetWidth() const {return _GetWidth();}
DG_INLINE void IViewClipObject::SetWidth(double newWidth) {_SetWidth(newWidth);}
DG_INLINE StatusInt IViewClipObject::GetClipBoundary(ClipVectorPtr& clip, DRange3dR maxRange, ClipVolumePass pass, bool displayCutGeometry) const {return _GetClipBoundary(clip,maxRange,pass,displayCutGeometry);}
DG_INLINE bool IViewClipObject::IsClipVolumePassValid(ClipVolumePass pass) const {return _IsClipVolumePassValid(pass);}
DG_INLINE StatusInt IViewClipObject::GetCuttingPlane(DPlane3dR cutPlane, DVec3dR xDir, DVec3dR yDir, ClipMask& clipMask, DRange2dR clipRange, bool& forwardFacing, int index, ViewContextR context) const {return _GetCuttingPlane(cutPlane,xDir,yDir,clipMask,clipRange,forwardFacing,index,context);}
DG_INLINE bool IViewClipObject::GetAuxTransform(TransformR t, ClipVolumePass p) const {return _GetAuxTransform(t,p);}
DG_INLINE StatusInt IViewClipObject::GetTransform(TransformR trans) const {return _GetTransform(trans);}
DG_INLINE size_t IViewClipObject::GetPrimaryCutPlaneCount() const {return _GetPrimaryCutPlaneCount();}
DG_INLINE StatusInt IViewClipObject::ApplyTransform(TransformCR t) {return _ApplyTransform(t);}
DG_INLINE void IViewClipObject::Draw(ViewContextR c) {return _Draw(c);}

DG_INLINE bool DgnViewport::Is3dView() const {return m_is3dView;}
DG_INLINE bool DgnViewport::IsActive() const {return NULL != m_output;}
DG_INLINE bool DgnViewport::IsCameraOn() const {return m_isCameraOn;}
DG_INLINE ViewFlagsCP DgnViewport::GetViewFlags() const {return &m_rootViewFlags;}
DG_INLINE CameraInfo const& DgnViewport::GetCamera() const {return m_camera;}
DG_INLINE void DgnViewport::SetMinimumLOD(double minLOD) {m_minLOD = minLOD;}
DG_INLINE RotMatrixCR DgnViewport::GetRotMatrix() const {return m_rotMatrix;}
DG_INLINE DMap4dCP DgnViewport::GetWorldToViewMap() const {return &m_rootToView;}
DG_INLINE DMap4dCP DgnViewport::GetWorldToNpcMap() const {return &m_rootToNpc;}
DG_INLINE bool DgnViewport::IsGridOn() const {return _IsGridOn();}
DG_INLINE bool DgnViewport::IsSheetView() const {return _IsSheetView();}
DG_INLINE ViewControllerCR DgnViewport::GetViewController() const {return *m_viewController;}
DG_INLINE ViewControllerR DgnViewport::GetViewControllerR() const {return *m_viewController;}
DG_INLINE DPoint3dCP DgnViewport::GetViewOrigin() const {return _GetViewOrigin();}
DG_INLINE DPoint3dCP DgnViewport::GetViewDelta() const {return _GetViewDelta();}
DG_INLINE void DgnViewport::Destroy() {_Destroy();}
DG_INLINE ColorDef DgnViewport::GetBackgroundColor() const {return m_backgroundColor;}
DG_INLINE void DgnViewport::GetViewCorners(DPoint3dR low, DPoint3dR high) const {_GetViewCorners(low,high);}
DG_INLINE int DgnViewport::GetIndexedLineWidth(int index) const {return _GetIndexedLineWidth(index);}
DG_INLINE uint32_t        DgnViewport::GetIndexedLinePattern(int index) const {return _GetIndexedLinePattern(index);}
DG_INLINE void DgnViewport::SetFrustumFromRootCorners(DPoint3dCP rootBox, double compressionFraction) {_SetFrustumFromRootCorners(rootBox, compressionFraction);}
DG_INLINE IViewOutputP DgnViewport::GetIViewOutput() {return _GetIViewOutput();}
DG_INLINE ICachedDrawP DgnViewport::GetICachedDraw() {return _GetICachedDraw();}
DG_INLINE void DgnViewport::SetNeedsRefresh() {_SetNeedsRefresh();}
DG_INLINE void DgnViewport::SetNeedsHeal() {_SetNeedsHeal();}
DG_INLINE double DgnViewport::GetMinimumLOD() const {return _GetMinimumLOD();}
DG_INLINE BSIRect DgnViewport::GetClientRect() const {return _GetClientRect();}
DG_INLINE Point2d DgnViewport::GetScreenOrigin() const {return _GetScreenOrigin();}
DG_INLINE DVec2d DgnViewport::GetDpiScale() const    {return _GetDpiScale();}
DG_INLINE ColorDef DgnViewport::GetWindowBgColor() const {return _GetWindowBgColor();}
DG_INLINE ColorDef DgnViewport::GetHiliteColor() const {return _GetHiliteColor();}
DG_INLINE StatusInt DgnViewport::RefreshViewport(bool always, bool synchHealingFromBs, bool& stopFlag) {return _RefreshViewport(always, synchHealingFromBs, stopFlag);}
DG_INLINE void DgnViewport::DrawStandardGrid(DPoint3dR origin, RotMatrixR rMatrix, Point2dCP fixedRepetitions) {_DrawStandardGrid(origin, rMatrix, fixedRepetitions);}

DG_INLINE void ViewController::SetBackgroundColor(ColorDef color) {m_backgroundColor = color;}
DG_INLINE bool ViewController::IsLoaded() const { return m_baseModelId.IsValid();}
DG_INLINE void ViewController::ChangeModelDisplay(DgnModelId modelId, bool onOff) {_ChangeModelDisplay(modelId, onOff);}
DG_INLINE bool ViewController::Allow3dManipulations() const {return _Allow3dManipulations();}
DG_INLINE DgnModelP ViewController::GetTargetModel() const {return _GetTargetModel();}
DG_INLINE bool ViewController::OnGeoLocationEvent(GeoLocationEventStatus& status, GeoPointCR point) {return _OnGeoLocationEvent(status, point);}
DG_INLINE void ViewController::OnViewOpened(DgnViewportR vp) {_OnViewOpened(vp);}

DG_INLINE ClipVectorPtr PhysicalViewController::GetClipVector() const {return _GetClipVector();}
DG_INLINE void CameraViewController::SetClipVector(ClipVectorR c) {m_clipVector = &c;}
DG_INLINE void CameraViewController::ClearClipVector() {m_clipVector=NULL;}
DG_INLINE ClipVectorPtr CameraViewController::_GetClipVector() const {return m_clipVector;}
DG_INLINE IAuxCoordSysP PhysicalViewController::GetAuxCoordinateSystem() const {return _GetAuxCoordinateSystem();}

DG_INLINE void            HitDetail::GetInfoString(Utf8StringR pathDescr, Utf8CP delimiter) const {_GetInfoString(pathDescr, delimiter);}
DG_INLINE HitSource       HitDetail::GetLocateSource() const {return m_locateSource;}
DG_INLINE DPoint3dCR      HitDetail::GetTestPoint() const {return m_testPoint;}
DG_INLINE GeomDetailCR    HitDetail::GetGeomDetail() const {return m_geomDetail;}
DG_INLINE GeomDetailR     HitDetail::GetGeomDetailW() {return m_geomDetail;}
DG_INLINE ViewFlagsCR     HitDetail::GetViewFlags() const {return m_viewFlags; }
DG_INLINE IElemTopologyCP HitDetail::GetElemTopology() const {return(m_elemTopo.IsValid() ? m_elemTopo.get() : nullptr);}
DG_INLINE void            HitDetail::SetElemTopology(IElemTopologyP topo) {m_elemTopo = topo;}
DG_INLINE DgnViewportR    HitDetail::GetViewport() const {return m_viewport;}
DG_INLINE DgnElementId    HitDetail::GetElementId() const {return m_elementId;}

DG_INLINE SnapMode        SnapDetail::GetSnapMode() const {return m_snapMode;}
DG_INLINE SnapMode        SnapDetail::GetOriginalSnapMode() const {return m_originalSnapMode;}
DG_INLINE void            SnapDetail::SetSnapMode(SnapMode s, bool isOriginal) {m_snapMode=s; if(isOriginal) m_originalSnapMode=s;}
DG_INLINE void            SnapDetail::SetSnapDivisor(int divisor) {m_divisor = divisor ? divisor : 2;}
DG_INLINE void            SnapDetail::SetAdjustedPoint(DPoint3dCR adjustedPt) {m_adjustedPt = adjustedPt;}
DG_INLINE void            SnapDetail::SetHeat(SnapHeat isHot) {m_heat = isHot;}
DG_INLINE bool            SnapDetail::IsHot() const {return m_heat != SNAP_HEAT_None;}
DG_INLINE bool            SnapDetail::IsPointOnCurve() const {return m_heat == SNAP_HEAT_InRange;}
DG_INLINE SnapHeat        SnapDetail::GetHeat() const {return m_heat;}
DG_INLINE DPoint3dCR      SnapDetail::GetAdjustedPoint() const {return m_adjustedPt;}
DG_INLINE DPoint3dCR      SnapDetail::GetSnapPoint() const {return m_snapPoint;}
DG_INLINE int             SnapDetail::GetSnapDivisor() const {return m_divisor;}
DG_INLINE double          SnapDetail::GetMinScreenDist() const {return m_minScreenDist;}
DG_INLINE Point2d const&  SnapDetail::GetScreenPoint() const {return m_screenPt;}

DG_INLINE IACSManagerR IACSManager::GetManager() {return T_HOST.GetAcsManager();}

DG_INLINE void IAuxCoordSys::DisplayInView(DgnViewportP vp, ACSDisplayOptions options, bool drawName) const {return _DisplayInView(vp, options, drawName);}
DG_INLINE WString IAuxCoordSys::GetName() const {return _GetName();}
DG_INLINE WString IAuxCoordSys::GetDescription() const {return _GetDescription();}
DG_INLINE ACSType IAuxCoordSys::GetType() const {return _GetType();}
DG_INLINE WString IAuxCoordSys::GetTypeName() const {return _GetTypeName();}
DG_INLINE uint32_t IAuxCoordSys::GetExtenderId() const {return _GetExtenderId();}
DG_INLINE uint32_t IAuxCoordSys::GetSerializedSize() const {return _GetSerializedSize();}
DG_INLINE StatusInt IAuxCoordSys::Serialize(void *buffer, uint32_t maxSize) const {return _Serialize(buffer, maxSize);}
DG_INLINE double IAuxCoordSys::GetScale() const {return _GetScale();}
DG_INLINE DPoint3dR IAuxCoordSys::GetOrigin(DPoint3dR pOrigin) const {return _GetOrigin(pOrigin);}
DG_INLINE bool IAuxCoordSys::GetIsReadOnly() const {return _GetIsReadOnly();}
DG_INLINE RotMatrixR IAuxCoordSys::GetRotation(RotMatrixR pRot) const {return _GetRotation(pRot);}
DG_INLINE RotMatrixR IAuxCoordSys::GetRotation(RotMatrixR pRot, DPoint3dR pPosition) const {return _GetRotation(pRot, pPosition);}
DG_INLINE ACSFlags IAuxCoordSys::GetFlags() const {return _GetFlags();}
DG_INLINE StatusInt IAuxCoordSys::SetName(WCharCP name) {return _SetName(name);}
DG_INLINE StatusInt IAuxCoordSys::SetDescription(WCharCP descr) {return _SetDescription(descr);}
DG_INLINE StatusInt IAuxCoordSys::SetType(ACSType type) {return _SetType(type);}
DG_INLINE StatusInt IAuxCoordSys::SetScale(double scale) {return _SetScale(scale);}
DG_INLINE StatusInt IAuxCoordSys::SetOrigin(DPoint3dCR pOrigin) {return _SetOrigin(pOrigin);}
DG_INLINE StatusInt IAuxCoordSys::SetRotation(RotMatrixCR pRot) {return _SetRotation(pRot);}
DG_INLINE StatusInt IAuxCoordSys::PointFromString(DPoint3dR outPoint, WStringR errorMsg, WCharCP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) {return _PointFromString(outPoint, errorMsg, inString, relative, lastPoint, modelRef);}
DG_INLINE StatusInt IAuxCoordSys::StringFromPoint(WStringR outString, WStringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFormatter, DirectionFormatterR directionFormatter)
                                                        { return _StringFromPoint(outString, errorMsg, inPoint, delta, deltaOrigin, modelRef, distanceFormatter, directionFormatter); }
DG_INLINE StatusInt IAuxCoordSys::SetFlags(ACSFlags flags) {return _SetFlags(flags);}
DG_INLINE void IAuxCoordSys::DrawGrid(DgnViewportP viewport) const {return _DrawGrid(viewport);}
DG_INLINE void IAuxCoordSys::PointToGrid(DgnViewportP viewport, DPoint3dR point) const {_PointToGrid(viewport, point);}
DG_INLINE StatusInt IAuxCoordSys::CompleteSetupFromViewController(PhysicalViewControllerCP info) {return _CompleteSetupFromViewController(info);}
DG_INLINE bool IAuxCoordSys::Equals(IAuxCoordSysCP other) const {return _Equals(other);}
DG_INLINE IAuxCoordSysPtr IAuxCoordSys::Clone() const {return _Clone();}

DG_INLINE StatusInt IAuxCoordSys::GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, uint32_t& gridPerRef) const {return _GetStandardGridParams(gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}
DG_INLINE StatusInt IAuxCoordSys::SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, uint32_t gridPerRef) {return _SetStandardGridParams(gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}

DG_INLINE DgnDbR ViewContext::GetDgnDb() const {BeAssert(nullptr != m_dgnDb); return *m_dgnDb;}
DG_INLINE void ViewContext::SetDgnDb(DgnDbR dgnDb) {return _SetDgnDb(dgnDb);}
DG_INLINE GeometricElementCP ViewContext::GetCurrentElement() const {return (m_currentElement.IsValid() ? m_currentElement->ToGeometricElement() : nullptr);}
DG_INLINE void ViewContext::SetCurrentElement(GeometricElementCP element) {_SetCurrentElement(element);}

DG_INLINE DrawPurpose ViewContext::GetDrawPurpose() const {return m_purpose;}
DG_INLINE bool ViewContext::IsCameraOn() const {return m_isCameraOn;}
DG_INLINE bool ViewContext::Is3dView() const {return m_is3dView;}

DG_INLINE ElemMatSymbP ViewContext::GetElemMatSymb() {return &m_elemMatSymb;}
DG_INLINE OvrMatSymb* ViewContext::GetOverrideMatSymb() {return &m_ovrMatSymb;}
DG_INLINE IViewDrawR ViewContext::GetIViewDraw() {BeAssert(NULL != m_IViewDraw); return *m_IViewDraw;}
DG_INLINE IDrawGeomR ViewContext::GetIDrawGeom() {BeAssert(NULL != m_IDrawGeom); return *m_IDrawGeom;}
DG_INLINE ICachedDrawP ViewContext::GetICachedDraw() {return m_ICachedDraw;}
DG_INLINE bool ViewContext::CheckICachedDraw() {return m_creatingCacheElem;}
DG_INLINE Byte& ViewContext::GetFilterLODFlag() {return m_filterLOD;}
DG_INLINE void ViewContext::SetFilterLODFlag(FilterLODFlags flags) { m_filterLOD =(Byte) flags; }
DG_INLINE bool ViewContext::GetIgnoreScaleForDimensions() {return m_ignoreScaleForDimensions;}
DG_INLINE bool ViewContext::GetIgnoreScaleForMultilines() {return m_ignoreScaleForMultilines;}
DG_INLINE bool ViewContext::GetApplyRotationToDimView() {return m_applyRotationToDimView;}
DG_INLINE void ViewContext::SetIgnoreScaleForDimensions(bool ignore) {m_ignoreScaleForDimensions = ignore;}
DG_INLINE void ViewContext::SetIgnoreScaleForMultilines(bool ignore) {m_ignoreScaleForMultilines = ignore;}
DG_INLINE void ViewContext::SetApplyRotationToDimView(bool rotateDimView) {m_applyRotationToDimView = rotateDimView;}
DG_INLINE size_t ViewContext::GetTransClipDepth() {return m_transformClipStack.GetSize(); }
DG_INLINE size_t ViewContext::GetRefTransClipDepth() {return m_refTransClipDepth;}
DG_INLINE RangeResult ViewContext::GetCurrParentRangeResult() {return m_parentRangeResult; }
DG_INLINE void ViewContext::SetCurrParentRangeResult(RangeResult val) {m_parentRangeResult = val;}
DG_INLINE bool& ViewContext::GetUseCachedGraphics() {return m_useCachedGraphics;}
DG_INLINE DgnViewportP ViewContext::GetViewport() const {return m_viewport;}
DG_INLINE double ViewContext::GetArcTolerance() const {return m_arcTolerance;}
DG_INLINE double ViewContext::GetMinLOD() const {return m_minLOD;}
DG_INLINE void ViewContext::SetMinLOD(double lod) { m_minLOD = lod; }
DG_INLINE ScanCriteriaCP ViewContext::GetScanCriteria() const {return m_scanCriteria;}
DG_INLINE ViewFlagsCP ViewContext::GetViewFlags() const {return m_IDrawGeom ? m_IDrawGeom->GetDrawViewFlags() : NULL;}
DG_INLINE void ViewContext::SetViewFlags(ViewFlagsCP flags) {if (NULL != m_IDrawGeom) m_IDrawGeom->SetDrawViewFlags(flags);}
DG_INLINE ElemDisplayParamsP ViewContext::GetCurrentDisplayParams() {return &m_currDisplayParams;}
DG_INLINE bool ViewContext::GetDisplayPriorityRange(int32_t& low, int32_t& high) const {if (NULL == m_viewport) return false; low = m_displayPriorityRange[0]; high = m_displayPriorityRange[1]; return true;}
DG_INLINE void ViewContext::SetArcTolerance(double tol) {m_arcTolerance = tol;}
DG_INLINE uint32_t ViewContext::GetRasterPlane() const {return m_rasterPlane;}
DG_INLINE StatusInt ViewContext::Attach(DgnViewportP vp, DrawPurpose purpose) {return _Attach(vp,purpose);}
DG_INLINE void ViewContext::Detach() {_Detach();}
DG_INLINE bool ViewContext::CheckStop() {return _CheckStop();}
DG_INLINE void ViewContext::PopTransformClip() {_PopTransformClip();}
DG_INLINE void ViewContext::PushTransform(TransformCR trans) {_PushTransform(trans);}
DG_INLINE void ViewContext::PushClip(ClipVectorCR clip) {_PushClip(clip);}
DG_INLINE IPickGeomP ViewContext::GetIPickGeom() {return _GetIPickGeom();}
DG_INLINE void ViewContext::DrawSymbol(IDisplaySymbol* symb, TransformCP trans, ClipPlaneSetP clip, bool ignoreColor, bool ignoreWeight) {_DrawSymbol(symb, trans, clip, ignoreColor, ignoreWeight);}
DG_INLINE void ViewContext::InitScanRangeAndPolyhedron() {_InitScanRangeAndPolyhedron();}
DG_INLINE void ViewContext::DrawStyledLineString2d(int nPts, DPoint2dCP pts, double zDepth, DPoint2dCP range, bool closed){_DrawStyledLineString2d(nPts, pts, zDepth, range, closed);}
DG_INLINE void ViewContext::DrawStyledLineString3d(int nPts, DPoint3dCP pts, DPoint3dCP range, bool closed){_DrawStyledLineString3d(nPts, pts, range, closed);}
DG_INLINE void ViewContext::DrawStyledArc2d(DEllipse3dCR ellipse, bool isEllipse, double zDepth, DPoint2dCP range) {_DrawStyledArc2d(ellipse, isEllipse, zDepth, range);}
DG_INLINE void ViewContext::DrawStyledArc3d(DEllipse3dCR ellipse, bool isEllipse, DPoint3dCP range) {_DrawStyledArc3d(ellipse, isEllipse, range);}
DG_INLINE void ViewContext::DrawStyledBSplineCurve2d(MSBsplineCurveCR curve, double zDepth) {_DrawStyledBSplineCurve2d(curve, zDepth);}
DG_INLINE void ViewContext::DrawStyledBSplineCurve3d(MSBsplineCurveCR curve) {_DrawStyledBSplineCurve3d(curve);}
DG_INLINE void ViewContext::PushViewIndependentOrigin(DPoint3dCP origin) {_PushViewIndependentOrigin(origin);}
DG_INLINE StatusInt ViewContext::VisitElement(GeometricElementCR elem) {return _VisitElement(elem);}
DG_INLINE void ViewContext::AllocateScanCriteria(){_AllocateScanCriteria();}
DG_INLINE void ViewContext::VisitDgnModel(DgnModelP modelRef) {_VisitDgnModel(modelRef);}
DG_INLINE void ViewContext::SetScanReturn() {_SetScanReturn();}
DG_INLINE QvElem* ViewContext::DrawCached(IStrokeForCache& stroker) {return _DrawCached(stroker);}
DG_INLINE void ViewContext::VisitTransientGraphics(bool isPreUpdate) {_VisitTransientGraphics(isPreUpdate);}
DG_INLINE void ViewContext::CookDisplayParams(ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) {_CookDisplayParams(elParams, elMatSymb);}
DG_INLINE void ViewContext::CookDisplayParamsOverrides(ElemDisplayParamsR elParams, OvrMatSymbR ovrMatSymb) {_CookDisplayParamsOverrides(elParams, ovrMatSymb);}
DG_INLINE StatusInt ViewContext::InitContextForView() {return _InitContextForView();}
DG_INLINE bool ViewContext::VisitAllModelElements(bool includeTransients) { return _VisitAllModelElements(includeTransients); }
DG_INLINE void ViewContext::ClearZ() { _ClearZ(); }
DG_INLINE void ViewContext::DeleteSymbol(IDisplaySymbol* symbol) {_DeleteSymbol(symbol);}
DG_INLINE bool ViewContext::_WantSaveQvElem(DrawExpense expense) {return T_HOST.GetGraphicsAdmin()._WantSaveQvElem(static_cast<int>(expense));}
DG_INLINE BentleyStatus ViewContext::GetCurrLocalToWorldTrans(TransformR trans) const { return m_transformClipStack.GetTransform(trans); }
DG_INLINE BentleyStatus ViewContext::GetCurrWorldToLocalTrans(TransformR trans) const { return m_transformClipStack.GetInverseTransform(trans); }
DG_INLINE TransformCP ViewContext::GetCurrLocalToWorldTransformCP() const { return m_transformClipStack.GetTransformCP(); }
DG_INLINE void ViewContext::DrawTextString(TextStringCR textString) {_DrawTextString(textString);}
DG_INLINE BentleyStatus ViewContext::GetLocalToWorldTrans(TransformR trans, size_t index) const { return m_transformClipStack.GetTransformFromIndex(trans, index); }
DG_INLINE bool ViewContext::IsViewIndependent() { return m_transformClipStack.IsViewIndependent(); }
DG_INLINE bool ViewContext::WantUndisplayedClips() { return _WantUndisplayedClips(); }

DG_INLINE double ViewContext::GetCurrentLevelOfDetail() const { return m_levelOfDetail; }
DG_INLINE void ViewContext::SetCurrentLevelOfDetail(double levelOfDetail) { m_levelOfDetail = levelOfDetail; }
DG_INLINE ViewContext::ContextMark::~ContextMark() {Pop();}

DG_INLINE IElemTopologyCP   ViewContext::GetElemTopology() const {return(m_currElemTopo.IsValid() ? m_currElemTopo.get() : nullptr);}
DG_INLINE void              ViewContext::SetElemTopology(IElemTopologyP topo) {m_currElemTopo = topo;}
DG_INLINE GeomStreamEntryId ViewContext::GetGeomStreamEntryId() const {return m_currGeomStreamEntryId;}
DG_INLINE void              ViewContext::SetGeomStreamEntryId(GeomStreamEntryId geomId) {m_currGeomStreamEntryId = geomId;}

DG_INLINE IDrawGeom::IDrawGeom() { }

DG_INLINE ViewFlagsCP IDrawGeom::GetDrawViewFlags() {return _GetDrawViewFlags();}
DG_INLINE void IDrawGeom::SetDrawViewFlags(ViewFlagsCP flags) {_SetDrawViewFlags(flags);}
DG_INLINE void IDrawGeom::ActivateMatSymb(ElemMatSymbCP matSymb){_ActivateMatSymb(matSymb);}
DG_INLINE void IDrawGeom::ActivateOverrideMatSymb(OvrMatSymbCP ovrMatSymb){_ActivateOverrideMatSymb(ovrMatSymb);}

DG_INLINE void IDrawGeom::DrawLineString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) {_DrawLineString3d(numPoints, points, range);}
DG_INLINE void IDrawGeom::DrawLineString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) {_DrawLineString2d(numPoints, points, zDepth, range);}
DG_INLINE void IDrawGeom::DrawPointString3d(int numPoints, DPoint3dCP points, DPoint3dCP range) {_DrawPointString3d(numPoints, points, range);}
DG_INLINE void IDrawGeom::DrawPointString2d(int numPoints, DPoint2dCP points, double zDepth, DPoint2dCP range) {_DrawPointString2d(numPoints, points, zDepth, range);}
DG_INLINE void IDrawGeom::DrawShape3d(int numPoints, DPoint3dCP points, bool filled, DPoint3dCP range) {_DrawShape3d(numPoints, points, filled, range);}
DG_INLINE void IDrawGeom::DrawShape2d(int numPoints, DPoint2dCP points, bool filled, double zDepth, DPoint2dCP range) {_DrawShape2d(numPoints, points, filled, zDepth, range);}
DG_INLINE void IDrawGeom::DrawTriStrip3d(int numPoints, DPoint3dCP points, int32_t usageFlags, DPoint3dCP range) {_DrawTriStrip3d(numPoints, points, usageFlags, range);}
DG_INLINE void IDrawGeom::DrawTriStrip2d(int numPoints, DPoint2dCP points, int32_t usageFlags, double zDepth, DPoint2dCP range) {_DrawTriStrip2d(numPoints, points, usageFlags, zDepth, range);}
DG_INLINE void IDrawGeom::DrawArc3d(DEllipse3dCR ellipse, bool isEllipse, bool filled, DPoint3dCP range) {_DrawArc3d(ellipse, isEllipse, filled, range);}
DG_INLINE void IDrawGeom::DrawArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double zDepth, DPoint2dCP range) {_DrawArc2d(ellipse, isEllipse, filled, zDepth, range);}
DG_INLINE void IDrawGeom::DrawBSplineCurve(MSBsplineCurveCR curve, bool filled) {_DrawBSplineCurve(curve, filled);}
DG_INLINE void IDrawGeom::DrawBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) { _DrawBSplineCurve2d(curve, filled, zDepth); }
DG_INLINE void IDrawGeom::DrawCurveVector(CurveVectorCR curves, bool isFilled) {_DrawCurveVector(curves, isFilled);}
DG_INLINE void IDrawGeom::DrawCurveVector2d(CurveVectorCR curves, bool isFilled, double zDepth) {_DrawCurveVector2d(curves, isFilled, zDepth);}
DG_INLINE void IDrawGeom::DrawSolidPrimitive(ISolidPrimitiveCR primitive) {_DrawSolidPrimitive(primitive);}
DG_INLINE void IDrawGeom::DrawBSplineSurface(MSBsplineSurfaceCR surface) {_DrawBSplineSurface(surface);}
DG_INLINE void IDrawGeom::DrawPolyface(PolyfaceQueryCR meshData, bool filled) { _DrawPolyface(meshData, filled); }
DG_INLINE StatusInt IDrawGeom::DrawBody(ISolidKernelEntityCR entity, double pixelSize) { return _DrawBody(entity, pixelSize); }
DG_INLINE void IDrawGeom::DrawTextString(TextStringCR text, double* zDepth) {_DrawTextString(text, zDepth);}
DG_INLINE void IDrawGeom::DrawMosaic(int numX, int numY, uintptr_t const* tileIds, DPoint3d const* verts) {_DrawMosaic(numX,numY,tileIds,verts);}
 DG_INLINE RangeResult IDrawGeom::PushBoundingRange3d(DPoint3dCP range){return _PushBoundingRange3d(range);}
DG_INLINE RangeResult IDrawGeom::PushBoundingRange2d(DPoint2dCP range, double zDepth){return _PushBoundingRange2d(range, zDepth);}
DG_INLINE void IDrawGeom::PopBoundingRange(){_PopBoundingRange();}
DG_INLINE void IDrawGeom::DrawTorus(DPoint3dCR center, DVec3dCR vectorX, DVec3dCR vectorY, double majorRadius, double minorRadius, double sweepAngle, bool capped) { DrawSolidPrimitive(*ISolidPrimitive::CreateDgnTorusPipe(DgnTorusPipeDetail(center, vectorX, vectorY, majorRadius, minorRadius, sweepAngle, capped))); }
DG_INLINE void IDrawGeom::DrawBox(DVec3dCR primary, DVec3dCR secondary, DPoint3dCR basePoint, DPoint3dCR topPoint, double baseWidth, double baseLength, double topWidth, double topLength, bool capped) { DrawSolidPrimitive(*ISolidPrimitive::CreateDgnBox(DgnBoxDetail::InitFromCenters(basePoint, topPoint, primary, secondary, baseWidth, baseLength, topWidth, topLength, capped))); }
DG_INLINE size_t IDrawGeom::GetMethodIndex() { return _GetMethodIndex(); }
DG_INLINE void IDrawGeom::PushMethodState() { _PushMethodState(); }
DG_INLINE void IDrawGeom::PopMethodState() { _PopMethodState(); }
 DG_INLINE bool IViewDraw::IsOutputQuickVision() const { return _IsOutputQuickVision(); }
DG_INLINE bool IViewDraw::ApplyMonochromeOverrides(ViewFlagsCR flags) const { return _ApplyMonochromeOverrides(flags); }

DG_INLINE void IViewDraw::SetToViewCoords(bool yesNo){_SetToViewCoords(yesNo);}
DG_INLINE void IViewDraw::SetSymbology(ColorDef lineColor, ColorDef fillColor, int lineWidth, uint32_t linePattern) {_SetSymbology(lineColor, fillColor, lineWidth, linePattern);}
DG_INLINE void IViewDraw::DrawGrid(bool doIsoGrid, bool drawDots, DPoint3dCR gridOrigin, DVec3dCR xVector, DVec3dCR yVector,uint32_t gridsPerRef, Point2dCR repetitions){_DrawGrid(doIsoGrid, drawDots, gridOrigin, xVector, yVector, gridsPerRef, repetitions);}
DG_INLINE bool IViewDraw::DrawSprite(ISprite* sprite, DPoint3dCP location, DPoint3dCP xVec, int transparency){return _DrawSprite(sprite, location, xVec, transparency);}
DG_INLINE void IViewDraw::DrawTiledRaster(ITiledRaster* tiledRaster){_DrawTiledRaster(tiledRaster);}
DG_INLINE void IViewDraw::DrawRaster2d(DPoint2d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, double zDepth, DPoint2dCP range) { _DrawRaster2d(points, pitch, numTexelsX, numTexelsY, enableAlpha, format, texels, zDepth, range); }
DG_INLINE void IViewDraw::DrawRaster(DPoint3d const points[4], int pitch, int numTexelsX, int numTexelsY, int enableAlpha, int format, Byte const* texels, DPoint3dCP range) { _DrawRaster(points, pitch, numTexelsX, numTexelsY, enableAlpha, format, texels, range); }
DG_INLINE void IViewDraw::DrawDgnOle(IDgnOleDraw* ole) {_DrawDgnOle(ole);}
DG_INLINE void IViewDraw::DrawPointCloud(IPointCloudDrawParams* drawParams) {_DrawPointCloud(drawParams);}
DG_INLINE void IViewDraw::DrawQvElem(QvElem* qvElem, int subElemIndex){_DrawQvElem(qvElem, subElemIndex);}
DG_INLINE void IViewDraw::ClearZ() {_ClearZ(); }
DG_INLINE StatusInt IViewDraw::TestOcclusion(int numVolumes, DPoint3dP verts, int* results){return _TestOcclusion(numVolumes, verts, results);}
DG_INLINE uintptr_t IViewDraw::DefineQVTexture(WCharCP textureName, DgnDbP dgnFile) {return _DefineQVTexture(textureName, dgnFile);}
DG_INLINE void IViewDraw::DefineQVGeometryMap(MaterialCR material, IStrokeForCache& stroker, DPoint2dCP spacing, bool useCellColors, ViewContextR seedContext, bool forAreaPattern) {return _DefineQVGeometryMap(material, stroker, spacing, useCellColors, seedContext, forAreaPattern);}
DG_INLINE void IViewDraw::PushClipStencil(QvElem* qvElem) {_PushClipStencil(qvElem);}
DG_INLINE void IViewDraw::PopClipStencil() {_PopClipStencil(); }

DG_INLINE void ICachedDraw::BeginCacheElement(QvCache* qvCache, bool is3d, double zDepth, uintptr_t elementId){_BeginCacheElement(qvCache, is3d, zDepth, elementId);}
DG_INLINE QvElem* ICachedDraw::EndCacheElement() {return _EndCacheElement();}
DG_INLINE void ICachedDraw::AssignElementToView(QvView* qvView, QvElem* qvElem, int viewMode) {_AssignElementToView(qvView, qvElem, viewMode);}
DG_INLINE QvElem* ICachedDraw::GetCacheElement() { return _GetCacheElement(); }
DG_INLINE void ICachedDraw::SetCacheElement(QvElem* qvElem) { _SetCacheElement(qvElem); }
DG_INLINE void ICachedDraw::PushTransform(TransformCR trans) {_PushTransClip(&trans);}
DG_INLINE void ICachedDraw::PopTransform() {_PopTransClip();}

DG_INLINE void IViewOutput::AdjustBrightness(bool useFixedAdaptation, double brightness) {_AdjustBrightness(useFixedAdaptation, brightness);}
DG_INLINE uint64_t IViewOutput::GetLightStamp() {return _GetLightStamp();}
DG_INLINE void IViewOutput::SetViewAttributes(ViewFlags viewFlags, ColorDef bgColor, bool usebgTexture, AntiAliasPref aaLines, AntiAliasPref aaText) {_SetViewAttributes(viewFlags, bgColor, usebgTexture, aaLines, aaText);}
DG_INLINE DgnDisplayCoreTypes::DeviceContextP      IViewOutput::GetScreenDC() const {return _GetScreenDC();}
DG_INLINE StatusInt IViewOutput::AssignDC(DgnDisplayCoreTypes::DeviceContextP dc){return _AssignDC(dc);}
DG_INLINE void IViewOutput::AddLights(bool threeDview, const RotMatrix* rotMatrixP, DgnModelP model){_AddLights(threeDview, rotMatrixP, model);}
DG_INLINE void IViewOutput::DefineFrustum(DPoint3dCR frustPts, double fraction, bool is2d){_DefineFrustum(frustPts, fraction, is2d);}
DG_INLINE void IViewOutput::SetDrawBuffer(DgnDrawBuffer drawBuffer, BSIRectCP subRect){_SetDrawBuffer(drawBuffer, subRect);}
DG_INLINE DgnDrawBuffer IViewOutput::GetDrawBuffer() const{return _GetDrawBuffer();}
DG_INLINE void IViewOutput::SetEraseMode(bool newMode){_SetEraseMode(newMode);}
DG_INLINE void IViewOutput::SetFlashMode(bool newMode){_SetFlashMode(newMode);}
DG_INLINE StatusInt IViewOutput::SynchDrawingFromBackingStore(){return _SynchDrawingFromBackingStore();}
DG_INLINE void IViewOutput::SynchDrawingFromBackingStoreAsynch(){_SynchDrawingFromBackingStoreAsynch();}
DG_INLINE StatusInt IViewOutput::SynchScreenFromDrawing(){return _SynchScreenFromDrawing();}
DG_INLINE void IViewOutput::SynchScreenFromDrawingAsynch(){_SynchScreenFromDrawingAsynch();}
DG_INLINE bool IViewOutput::IsScreenDirty(BSIRectP rect){return _IsScreenDirty(rect);}
DG_INLINE void IViewOutput::ShowProgress(){_ShowProgress();}
DG_INLINE bool IViewOutput::IsBackingStoreValid() const{return _IsBackingStoreValid();}
DG_INLINE bool IViewOutput::IsAccelerated() const{return _IsAccelerated();}
DG_INLINE void IViewOutput::ScreenDirtied(BSIRectCP rect){_ScreenDirtied(rect);}
DG_INLINE bool IViewOutput::EnableZTesting(bool yesNo){return _EnableZTesting(yesNo);}
DG_INLINE bool IViewOutput::EnableZWriting(bool yesNo){return _EnableZWriting(yesNo);}
DG_INLINE void IViewOutput::SetProjectDepth(double depth){_SetProjectDepth(depth);}
DG_INLINE StatusInt IViewOutput::BeginDraw(bool eraseBefore){return _BeginDraw(eraseBefore);}
DG_INLINE void IViewOutput::EndDraw(QvPaintOptions const& opts){_EndDraw(opts);}
DG_INLINE StatusInt IViewOutput::BeginDrawCapture(){return _BeginDrawCapture();}
DG_INLINE StatusInt IViewOutput::EndDrawCapture(){return _EndDrawCapture();}
DG_INLINE bool IViewOutput::HaveCapture() const {return _HaveCapture();}
DG_INLINE void IViewOutput::ResetCapture(){_ResetCapture();}
DG_INLINE StatusInt IViewOutput::DisplayCaptured(ViewFlags flags, DPoint2dCP origin, DPoint2dCP extent, int(*stopProc)()){ return _DisplayCaptured(flags, origin, extent, stopProc); }
DG_INLINE bool IViewOutput::IsDrawActive(){return _IsDrawActive();}
DG_INLINE void IViewOutput::ShowTransparent(){_ShowTransparent();}
DG_INLINE void IViewOutput::AccumulateDirtyRegion(bool val){_AccumulateDirtyRegion(val);}
DG_INLINE void IViewOutput::ClearHealRegion(){_ClearHealRegion();}
DG_INLINE void IViewOutput::SetNeedsHeal(BSIRectCP dirty){_SetNeedsHeal(dirty);}
DG_INLINE void IViewOutput::HealComplete(bool aborted){_HealComplete(aborted);}
DG_INLINE bool IViewOutput::CheckNeedsHeal(BSIRectP rect){return _CheckNeedsHeal(rect);}
DG_INLINE void IViewOutput::BeginDecorating(BSIRectCP rect){_BeginDecorating(rect);}
DG_INLINE void IViewOutput::BeginOverlayMode(){_BeginOverlayMode();}
DG_INLINE bool IViewOutput::LocateQvElem(QvElem* qvElem, DPoint2dCR borePt, double radius, DPoint3dR hitPt, DVec3dP hitNormal, int(*stopProc)(CallbackArgP), CallbackArgP arg) {return _LocateQvElem(qvElem, borePt, radius, hitPt, hitNormal, stopProc, arg);}
DG_INLINE void IViewOutput::AbortOutstandingOperations(){_AbortOutstandingOperations();}
DG_INLINE void IViewOutput::SetIdleCallback(bool(*callback)(CallbackArgP), CallbackArgP userData){_SetIdleCallback(callback, userData);}
DG_INLINE QvView* IViewOutput::GetQvView() const{return _GetQvView();}
DG_INLINE void IViewOutput::PushTransClip(TransformCP trans, ClipPlaneSetCP clip) {_PushTransClip(trans, clip);}
DG_INLINE void IViewOutput::PopTransClip() {_PopTransClip();}
DG_INLINE BentleyStatus IViewOutput::FillImageCaptureBuffer(bvector<unsigned char>& buffer, CapturedImageInfo& info, DRange2dCR screenBufferRange, Point2dCR outputImageSize, bool topDown) {return _FillImageCaptureBuffer(buffer, info, screenBufferRange, outputImageSize, topDown);}
DG_INLINE int IViewOutput::GetVisibleTiles(QvMRImageP mri, size_t bufSize, int* lrc) { return _GetVisibleTiles(mri, bufSize, lrc); }
DG_INLINE void ITiledRaster::DrawRaster(IViewOutputR viewOutput) { return _DrawRaster(viewOutput); }
DG_INLINE void ITiledRaster::PrintRaster(IViewOutputR viewOutput) { return _DrawRaster(viewOutput); }

DG_INLINE GradientSymb::GradientSymb() {memset(&m_mode, 0, offsetof(GradientSymb, m_values) + sizeof(m_values) - offsetof(GradientSymb, m_mode));}
DG_INLINE int GradientSymb::GetNKeys() const {return m_nKeys;}
DG_INLINE GradientMode GradientSymb::GetMode() const {return m_mode;}
DG_INLINE uint16_t GradientSymb::GetFlags() const {return m_flags;}
DG_INLINE double GradientSymb::GetShift() const {return m_shift;}
DG_INLINE double GradientSymb::GetTint() const {return m_tint;}
DG_INLINE double GradientSymb::GetAngle() const {return m_angle;}
DG_INLINE void GradientSymb::GetKey(ColorDef& color, double& value, int index) const {color = m_colors[index], value = m_values[index];}
DG_INLINE void GradientSymb::SetMode(GradientMode mode) {m_mode = mode;}
DG_INLINE void GradientSymb::SetFlags(uint16_t flags) {m_flags = flags;}
DG_INLINE void GradientSymb::SetAngle(double angle) {m_angle = angle;}
DG_INLINE void GradientSymb::SetTint(double tint) {m_tint = tint;}
DG_INLINE void GradientSymb::SetShift(double shift) {m_shift = shift;}

DG_INLINE void PatternParams::SetModifiers(PatternParamsModifierFlags value) {modifiers = value;}
DG_INLINE void PatternParams::SetOrientation(RotMatrixCR value) {rMatrix = value; modifiers = modifiers |PatternParamsModifierFlags::RotMatrix;}
DG_INLINE void PatternParams::SetOffset(DPoint3dCR value) {offset = value; modifiers = modifiers |PatternParamsModifierFlags::Offset;}
DG_INLINE void PatternParams::SetPrimarySpacing(double value) {space1 = value; modifiers = modifiers |PatternParamsModifierFlags::Space1;}
DG_INLINE void PatternParams::SetPrimaryAngle(double value) {angle1 = value; modifiers = modifiers |PatternParamsModifierFlags::Angle1;}
DG_INLINE void PatternParams::SetSecondarySpacing(double value) {space2 = value; modifiers = modifiers |PatternParamsModifierFlags::Space2;}
DG_INLINE void PatternParams::SetSecondaryAngle(double value) {angle2 = value; modifiers = modifiers |PatternParamsModifierFlags::Angle2;}
DG_INLINE void PatternParams::SetScale(double value) {scale = value; modifiers = modifiers |PatternParamsModifierFlags::Scale;}
DG_INLINE void PatternParams::SetTolerance(double value) {tolerance = value; modifiers = modifiers |PatternParamsModifierFlags::Tolerance;}
DG_INLINE void PatternParams::SetCellId(DgnElementId value) {cellId = value.GetValue(); modifiers = modifiers |PatternParamsModifierFlags::Cell;}
DG_INLINE void PatternParams::SetMinLine(int32_t value) {minLine = value; modifiers = modifiers |PatternParamsModifierFlags::Multiline;}
DG_INLINE void PatternParams::SetMaxLine(int32_t value) {maxLine = value; modifiers = modifiers |PatternParamsModifierFlags::Multiline;}
DG_INLINE void PatternParams::SetColor(ColorDef value) {color = value; modifiers = modifiers |PatternParamsModifierFlags::Color;}
DG_INLINE void PatternParams::SetWeight(uint32_t value) {weight = value; modifiers = modifiers |PatternParamsModifierFlags::Weight;}
DG_INLINE void PatternParams::SetStyle(int32_t value) {style = value; modifiers = modifiers |PatternParamsModifierFlags::Style;}
DG_INLINE void PatternParams::SetHoleStyle(PatternParamsHoleStyleType value) {holeStyle = static_cast<int16_t>(value); modifiers = modifiers |PatternParamsModifierFlags::HoleStyle;}
DG_INLINE void PatternParams::SetDwgHatchDef(DwgHatchDefCR value) {dwgHatchDef = value; modifiers = modifiers |PatternParamsModifierFlags::DwgHatchDef;}
DG_INLINE void PatternParams::SetOrigin(DPoint3dCR value) {origin = value; modifiers = modifiers |PatternParamsModifierFlags::Origin;}
DG_INLINE void PatternParams::SetAnnotationScale(double scale) {annotationscale = scale; modifiers = modifiers |PatternParamsModifierFlags::AnnotationScale;}
DG_INLINE PatternParamsModifierFlags PatternParams::GetModifiers() const {return modifiers;}
DG_INLINE RotMatrixCR PatternParams::GetOrientation() const {return rMatrix;}
DG_INLINE DPoint3dCR PatternParams::GetOffset() const {return offset;}
DG_INLINE double PatternParams::GetPrimarySpacing() const {return space1;}
DG_INLINE double PatternParams::GetPrimaryAngle() const {return angle1;}
DG_INLINE double PatternParams::GetSecondarySpacing() const {return space2;}
DG_INLINE double PatternParams::GetSecondaryAngle() const {return angle2;}
DG_INLINE double PatternParams::GetScale() const {return scale;}
DG_INLINE double PatternParams::GetTolerance() const {return tolerance;}
DG_INLINE WCharCP PatternParams::GetCellName() const {return cellName;}
DG_INLINE DgnElementId PatternParams::GetCellId() const {return DgnElementId(cellId);}
DG_INLINE int32_t PatternParams::GetMinLine() const {return minLine;}
DG_INLINE int32_t PatternParams::GetMaxLine() const {return maxLine;}
DG_INLINE ColorDef PatternParams::GetColor() const {return color;}
DG_INLINE uint32_t PatternParams::GetWeight() const {return weight;}
DG_INLINE int32_t PatternParams::GetStyle() const {return style;}
DG_INLINE PatternParamsHoleStyleType PatternParams::GetHoleStyle() const {return static_cast<PatternParamsHoleStyleType>(holeStyle);}
DG_INLINE DwgHatchDefCR PatternParams::GetDwgHatchDef() const {return dwgHatchDef;}
DG_INLINE DPoint3dCR PatternParams::GetOrigin() const {return origin;}
DG_INLINE double PatternParams::GetAnnotationScale() const {return annotationscale;}

DG_INLINE void ElemMatSymb::SetIndexedRasterPattern(int32_t index, uint32_t rasterPat) {m_elementStyle = IS_LINECODE(index) ? index : 0; m_rasterPat = rasterPat; m_extSymbID = 0;}

DG_INLINE ColorDef ElemDisplayParams::GetLineColor() const {BeAssert(m_appearanceOverrides.m_color || m_resolved); return m_lineColor;}
DG_INLINE ColorDef ElemDisplayParams::GetFillColor() const {BeAssert(m_appearanceOverrides.m_fill || m_resolved); return m_fillColor;}
DG_INLINE FillDisplay ElemDisplayParams::GetFillDisplay() const {return m_fillDisplay;}
DG_INLINE GradientSymbCP ElemDisplayParams::GetGradient() const {return m_gradient.get(); }
DG_INLINE PatternParamsCP ElemDisplayParams::GetPatternParams() const {return m_pattern.get();}
DG_INLINE LineStyleInfoCP ElemDisplayParams::GetLineStyle() const {BeAssert(m_appearanceOverrides.m_style || m_resolved); return m_styleInfo.get();}
DG_INLINE uint32_t ElemDisplayParams::GetWeight() const {BeAssert(m_appearanceOverrides.m_weight || m_resolved); return m_weight;}
DG_INLINE DgnGeometryClass ElemDisplayParams::GetGeometryClass() const {return m_geometryClass;}
DG_INLINE DgnMaterialId ElemDisplayParams::GetMaterial() const {BeAssert(m_appearanceOverrides.m_material || m_resolved); return m_material;}
DG_INLINE double ElemDisplayParams::GetTransparency() const {return m_elmTransparency;}
DG_INLINE double ElemDisplayParams::GetFillTransparency() const {return m_fillTransparency;}
DG_INLINE int32_t ElemDisplayParams::GetDisplayPriority() const {return m_elmPriority;}
DG_INLINE double ElemDisplayParams::GetNetTransparency() const {BeAssert(m_resolved); return m_netElmTransparency;}
DG_INLINE double ElemDisplayParams::GetNetFillTransparency() const {BeAssert(m_resolved); return m_netFillTransparency;}
DG_INLINE int32_t ElemDisplayParams::GetNetDisplayPriority() const {BeAssert(m_resolved); return m_netPriority;}

DG_INLINE void ElemDisplayParams::SetLineColor(ColorDef color) {m_appearanceOverrides.m_color = true; m_lineColor = color;}
DG_INLINE void ElemDisplayParams::SetFillColor(ColorDef color) {m_appearanceOverrides.m_fill = true; m_fillColor = color;}
DG_INLINE void ElemDisplayParams::SetFillDisplay(FillDisplay display) {m_fillDisplay = display;}
DG_INLINE void ElemDisplayParams::SetGradient(GradientSymbP gradient) {m_gradient = gradient;}
DG_INLINE void ElemDisplayParams::SetPatternParams (PatternParamsP pattern) {m_pattern = pattern;}
DG_INLINE void ElemDisplayParams::SetLineStyle(LineStyleInfoP styleInfo) {m_appearanceOverrides.m_style = true; m_styleInfo = styleInfo;}
DG_INLINE void ElemDisplayParams::SetWeight(uint32_t weight) {m_appearanceOverrides.m_weight = true; m_weight = weight;}
DG_INLINE void ElemDisplayParams::SetGeometryClass(DgnGeometryClass geomClass) {m_geometryClass = geomClass;}
DG_INLINE void ElemDisplayParams::SetMaterial(DgnMaterialId material) {m_appearanceOverrides.m_material = true; m_material = material;}
DG_INLINE void ElemDisplayParams::SetTransparency(double transparency) {m_elmTransparency = m_netElmTransparency = m_fillTransparency = m_netFillTransparency = transparency;}
DG_INLINE void ElemDisplayParams::SetFillTransparency(double transparency) {m_fillTransparency = m_netFillTransparency = transparency;}
DG_INLINE void ElemDisplayParams::SetDisplayPriority(int32_t priority) {m_elmPriority = m_netPriority = priority;}
DG_INLINE void ElemDisplayParams::SetNetDisplayPriority(int32_t priority) {m_netPriority = priority;}
DG_INLINE bool ElemDisplayParams::IsLineColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_color;}
DG_INLINE bool ElemDisplayParams::IsWeightFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_weight;}
DG_INLINE bool ElemDisplayParams::IsLineStyleFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_style;}
DG_INLINE bool ElemDisplayParams::IsMaterialFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_material;}
DG_INLINE bool ElemDisplayParams::IsFillColorFromSubCategoryAppearance() const {return !m_appearanceOverrides.m_fill;}
DG_INLINE void ElemDisplayParams::SetCategoryId(DgnCategoryId categoryId) {m_categoryId = categoryId; m_subCategoryId = DgnCategories::DefaultSubCategoryId(categoryId); memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;}
DG_INLINE void ElemDisplayParams::SetSubCategoryId(DgnSubCategoryId subCategoryId) {m_subCategoryId = subCategoryId; memset(&m_appearanceOverrides, 0, sizeof(m_appearanceOverrides)); m_resolved = false;}
DG_INLINE DgnCategoryId ElemDisplayParams::GetCategoryId() const {return m_categoryId;}
DG_INLINE DgnSubCategoryId ElemDisplayParams::GetSubCategoryId() const {return m_subCategoryId;}

DG_INLINE bool PlotInfo::IsScreeningSet() const {return m_hasScreening;}
DG_INLINE double PlotInfo::GetScreening() const {return m_screening;}
DG_INLINE void PlotInfo::SetScreening(double screen, bool set) {if (set) m_screening = screen; m_hasScreening = set;};
DG_INLINE bool PlotInfo::IsLineJoinSet() const {return m_hasLineJoin;}
DG_INLINE LineJoin PlotInfo::GetLineJoin() const {return m_lineJoin;}
DG_INLINE void PlotInfo::SetLineJoin(LineJoin join, bool set) {if (set) m_lineJoin = join; m_hasLineJoin = set;};
DG_INLINE bool PlotInfo::IsLineCapSet() const {return m_hasLineCap;}
DG_INLINE LineCap PlotInfo::GetLineCap() const {return m_lineCap;}
DG_INLINE void PlotInfo::SetLineCap(LineCap cap, bool set) {if (set) m_lineCap = cap; m_hasLineCap = set;};
DG_INLINE bool PlotInfo::IsLineWeightMMSet() const {return m_hasLineWeightMM;}
DG_INLINE double PlotInfo::GetLineWeightMM() const {return m_widthMM;}
DG_INLINE void PlotInfo::SetLineWeightMM(double mm, bool set) {if (set) m_widthMM = mm; m_hasLineWeightMM = set;};

DG_INLINE void LineStyleParams::SetScale(double inScale) { modifiers |= STYLEMOD_SCALE; scale = inScale; }

DG_INLINE StatusInt     ViewController::VisitHit(HitDetailCR hit, ViewContextR context) const{return _VisitHit(hit, context);}
DG_INLINE void ViewController::DrawView(ViewContextR context) {return _DrawView(context);}
DG_INLINE void ViewController::VisitElements(ViewContextR context) {return _VisitElements(context);}

DG_INLINE SnapDetailP     SnapContext::GetSnapDetail() {return m_snapPath;}
DG_INLINE SnapMode      SnapContext::GetSnapMode() {return m_snapMode;}
DG_INLINE int           SnapContext::GetSnapDivisor() {return m_snapDivisor;}

DG_INLINE DgnPlatformLib::Host& DgnPlatformLib::GetHost() {return *static_cast<DgnPlatformLib::Host*>(g_hostForThread.GetValueAsPointer());}

//DG_INLINE void          IDrawElementAgenda::DrawElementAgenda(ElementAgendaR agenda, DgnDrawMode drawMode, DrawPurpose purpose) {_DrawElementAgenda(agenda, drawMode, purpose);}
DG_INLINE Utf8StringCR Material::GetName() const { return m_name; }
DG_INLINE Utf8StringR Material::GetNameR() { return m_name; }
DG_INLINE Utf8StringCR Material::GetPalette() const { return m_palette; }
DG_INLINE Utf8StringR Material::GetPaletteR() { return m_palette; }
DG_INLINE DgnDbR Material::GetDgnProjectR() const { return *m_dgnProject; }
DG_INLINE MaterialSettingsCR Material::GetSettings() const { return m_settings; }
DG_INLINE MaterialSettingsR Material::GetSettingsR() { return m_settings; }
DG_INLINE bool Material::GetSentToQV() const { return m_sentToQV; }
DG_INLINE void Material::SetSentToQV(bool sentToQV) const { m_sentToQV = sentToQV; }

DG_INLINE WChar AngleFormatter::GetDecimalSeparator() const { return m_decimalSeparator; }
DG_INLINE AngleMode AngleFormatter::GetAngleMode() const { return m_angleMode; }
DG_INLINE AnglePrecision AngleFormatter::GetAnglePrecision() const { return m_precision; }
DG_INLINE bool AngleFormatter::GetLeadingZero() const { return m_leadingZero; }
DG_INLINE bool AngleFormatter::GetTrailingZeros() const { return m_trailingZeros; }
DG_INLINE bool AngleFormatter::GetAllowNegative() const { return m_allowNegative; }
DG_INLINE bool AngleFormatter::GetAllowUnclamped() const { return m_allowUnclamped; };
DG_INLINE void AngleFormatter::SetAngleMode(AngleMode newVal) { m_angleMode = newVal; }
DG_INLINE void AngleFormatter::SetAnglePrecision(AnglePrecision newVal) { m_precision = newVal; }
DG_INLINE void AngleFormatter::SetDecimalSeparator(WChar newVal) { m_decimalSeparator = newVal; }
DG_INLINE void AngleFormatter::SetLeadingZero(bool newVal) { m_leadingZero = newVal; }
DG_INLINE void AngleFormatter::SetTrailingZeros(bool newVal) { m_trailingZeros = newVal; }
DG_INLINE void AngleFormatter::SetAllowNegative(bool newVal) { m_allowNegative = newVal; }
DG_INLINE void AngleFormatter::SetAllowUnclamped(bool newVal) { m_allowUnclamped = newVal; }

DG_INLINE AngleFormatterR DirectionFormatter::GetAngleFormatter() { return *m_angleFormatter; }
DG_INLINE DirectionMode DirectionFormatter::GetDirectionMode() const { return m_mode; }
DG_INLINE bool DirectionFormatter::GetAddTrueNorth() const { return m_addTrueNorth; }
DG_INLINE double DirectionFormatter::GetTrueNorthValue() const { return m_trueNorth; }
DG_INLINE double DirectionFormatter::GetBaseDirection() const { return m_baseDirection; }
DG_INLINE bool DirectionFormatter::GetClockwise() const { return m_clockwise; }
DG_INLINE bool DirectionFormatter::GetBearingSpaces() const { return m_bearingSpaces; }
DG_INLINE void DirectionFormatter::SetDirectionMode(DirectionMode newVal) { m_mode = newVal; }
DG_INLINE void DirectionFormatter::SetAddTrueNorth(bool newVal) { m_addTrueNorth = newVal; }
DG_INLINE void DirectionFormatter::SetTrueNorthValue(double newVal) { m_trueNorth = newVal; }
DG_INLINE void DirectionFormatter::SetBaseDirection(double newVal) { m_baseDirection = newVal; }
DG_INLINE void DirectionFormatter::SetClockwise(bool newVal) { m_clockwise = newVal; }
DG_INLINE void DirectionFormatter::SetBearingSpaces(bool newVal) { m_bearingSpaces = newVal; }

DG_INLINE DgnUnitFormat DistanceFormatter::GetUnitFormat() const { return m_unitFormat; }
DG_INLINE UnitDefinitionCR DistanceFormatter::GetMasterUnits() const { return m_masterUnit; }
DG_INLINE UnitDefinitionCR DistanceFormatter::GetSubUnits() const { return m_subUnit; }
DG_INLINE double DistanceFormatter::GetScaleFactor() const { return m_scaleFactor; }
DG_INLINE bool DistanceFormatter::GetUnitLabelFlag() const { return m_unitFlag; }
DG_INLINE bool DistanceFormatter::GetSuppressZeroMasterUnits() const { return m_suppressZeroMasterUnits; }
DG_INLINE bool DistanceFormatter::GetSuppressZeroSubUnits() const { return m_suppressZeroSubUnits; }
DG_INLINE bool DistanceFormatter::GetIsDgnCoordReadOutCapable() const { return m_isDgnCoordReadOutCapable; }
DG_INLINE void DistanceFormatter::SetUnitFormat(DgnUnitFormat newVal) { m_unitFormat = newVal; }
DG_INLINE void DistanceFormatter::SetUnitLabelFlag(bool newVal) { m_unitFlag = newVal; }
DG_INLINE void DistanceFormatter::SetSuppressZeroMasterUnits(bool newVal) { m_suppressZeroMasterUnits = newVal; }
DG_INLINE void DistanceFormatter::SetSuppressZeroSubUnits(bool newVal) { m_suppressZeroSubUnits = newVal; }
DG_INLINE void DistanceFormatter::SetIsDgnCoordReadOutCapable(bool newVal) { m_isDgnCoordReadOutCapable = newVal; }
DG_INLINE void DistanceFormatter::SetPrecisionByte(Byte newVal) { m_precisionByte = newVal; }
DG_INLINE bool DistanceFormatter::GetUseDWGFormattingLogic() const { return m_useDWGFormattingLogic; }
DG_INLINE void DistanceFormatter::SetUseDWGFormattingLogic(bool newVal) { m_useDWGFormattingLogic = newVal; }
DG_INLINE DwgUnitFormat DistanceFormatter::GetDWGUnitFormat() const { return m_dwgUnitFormat; }

DG_INLINE bool PointFormatter::GetIs3d() const { return m_is3d; }
DG_INLINE void PointFormatter::SetIs3d(bool newVal) { m_is3d = newVal; }

DG_INLINE UnitDefinitionCR AreaOrVolumeFormatterBase::GetMasterUnits() const { return m_masterUnit; }
DG_INLINE double AreaOrVolumeFormatterBase::GetScaleFactor() const { return m_scaleFactor; }
DG_INLINE bool AreaOrVolumeFormatterBase::GetShowUnitLabel() const { return m_showUnitLabel; }
DG_INLINE bool AreaOrVolumeFormatterBase::GetLabelDecoratorAsSuffix() const { return m_labelDecoratorAsSuffix; }
DG_INLINE void AreaOrVolumeFormatterBase::SetShowUnitLabel(bool newVal) { m_showUnitLabel = newVal; }
DG_INLINE bool AreaOrVolumeFormatterBase::GetUseDWGFormattingLogic() const { return m_useDWGFormattingLogic; }
DG_INLINE void AreaOrVolumeFormatterBase::SetUseDWGFormattingLogic(bool newVal) { m_useDWGFormattingLogic = newVal; }
DG_INLINE void AreaOrVolumeFormatterBase::SetLabelDecoratorAsSuffix(bool newVal) { m_labelDecoratorAsSuffix = newVal; }
DG_INLINE DwgUnitFormat AreaOrVolumeFormatterBase::GetDWGUnitFormat() const { return m_dwgUnitFormat; }

DG_INLINE void AreaFormatter::SetPrecisionByte(Byte newVal) { m_precisionByte = newVal; }

DG_INLINE void AngleParser::SetAngleMode(AngleMode mode) { m_angleMode = mode; }
DG_INLINE AngleMode AngleParser::GetAngleMode() { return m_angleMode; }

DG_INLINE AngleParserR DirectionParser::GetAngleParser() { return *m_angleParser; }
DG_INLINE void DirectionParser::SetTrueNorthValue(double trueNorth) { m_trueNorth = trueNorth; }
DG_INLINE double DirectionParser::GetTrueNorthValue() { return m_trueNorth; }
DG_INLINE void DirectionParser::SetDirectionMode(DirectionMode newVal) { m_mode = newVal; }
DG_INLINE DirectionMode DirectionParser::GetDirectionMode() { return m_mode; }
DG_INLINE void DirectionParser::SetClockwise(bool isCw) { m_isClockwise = isCw; }
DG_INLINE bool DirectionParser::GetClockwise() { return m_isClockwise; }
DG_INLINE void DirectionParser::SetBaseDirection(double newVal) { m_baseDirection = newVal; }
DG_INLINE double DirectionParser::GetBaseDirection() { return m_baseDirection; }

DG_INLINE void DistanceParser::SetMasterUnitLabel(WCharCP label) { m_masterUnitLabel = label; }
DG_INLINE WCharCP DistanceParser::GetMasterUnitsLabel() { return m_masterUnitLabel.c_str(); }
DG_INLINE void DistanceParser::SetSubUnitLabel(WCharCP label) { m_subUnitLabel = label; }
DG_INLINE WCharCP DistanceParser::GetSubUnitsLabel() { return m_subUnitLabel.c_str(); }
DG_INLINE void DistanceParser::SetMasterUnitScale(double scale) { m_masterUnitScale = scale; }
DG_INLINE double DistanceParser::GetMasterUnitsScale() { return m_masterUnitScale; }
DG_INLINE void DistanceParser::SetSubUnitScale(double scale) { m_subUnitScale = scale; }
DG_INLINE double DistanceParser::GetSubUnitsScale() { return m_subUnitScale; }
DG_INLINE void DistanceParser::SetScale(double scale) { m_scale = scale; }
DG_INLINE double DistanceParser::GetScale() { return m_scale; }

DG_INLINE DistanceParserR PointParser::GetDistanceParser() { return *m_distanceParser; }
DG_INLINE void PointParser::SetIs3d(bool is3d) { m_is3d = is3d; }
DG_INLINE bool PointParser::GetIs3d() const { return m_is3d; }

DG_INLINE void AreaOrVolumeParser::SetMasterUnitScale(double scale) { m_masterUnitScale = scale; }
DG_INLINE double AreaOrVolumeParser::GetMasterUnitsScale() { return m_masterUnitScale; }
DG_INLINE void AreaOrVolumeParser::SetScale(double scale) { m_scale = scale; }
DG_INLINE double AreaOrVolumeParser::GetScale() { return m_scale; }
