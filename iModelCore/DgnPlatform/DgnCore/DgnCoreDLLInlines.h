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

DG_INLINE void DgnViewport::SetFrustumFromRootCorners(DPoint3dCP rootBox, double compressionFraction) {_SetFrustumFromRootCorners(rootBox, compressionFraction);}
DG_INLINE void DgnViewport::SetNeedsRefresh() {_SetNeedsRefresh();}
DG_INLINE void DgnViewport::SetNeedsHeal() {_SetNeedsHeal();}
DG_INLINE double DgnViewport::GetMinimumLOD() const {return _GetMinimumLOD();}
DG_INLINE BSIRect DgnViewport::GetClientRect() const {return _GetClientRect();}
DG_INLINE Point2d DgnViewport::GetScreenOrigin() const {return _GetScreenOrigin();}
DG_INLINE DVec2d DgnViewport::GetDpiScale() const    {return _GetDpiScale();}
DG_INLINE ColorDef DgnViewport::GetWindowBgColor() const {return _GetWindowBgColor();}
DG_INLINE ColorDef DgnViewport::GetHiliteColor() const {return _GetHiliteColor();}
DG_INLINE StatusInt DgnViewport::RefreshViewport(bool always, bool synchHealingFromBs, bool& stopFlag) {return _RefreshViewport(always, synchHealingFromBs, stopFlag);}
DG_INLINE void DgnViewport::DrawStandardGrid(DPoint3dR origin, RotMatrixR rMatrix, DPoint2d spacing, uint32_t gridsPerRef, bool isoGrid, Point2dCP fixedRepetitions) {_DrawStandardGrid(origin, rMatrix, spacing, gridsPerRef, isoGrid, fixedRepetitions);}

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
DG_INLINE DgnElementId PatternParams::GetCellId() const {return DgnElementId((uint64_t)cellId);}
DG_INLINE int32_t PatternParams::GetMinLine() const {return minLine;}
DG_INLINE int32_t PatternParams::GetMaxLine() const {return maxLine;}
DG_INLINE ColorDef PatternParams::GetColor() const {return color;}
DG_INLINE uint32_t PatternParams::GetWeight() const {return weight;}
DG_INLINE int32_t PatternParams::GetStyle() const {return style;}
DG_INLINE PatternParamsHoleStyleType PatternParams::GetHoleStyle() const {return static_cast<PatternParamsHoleStyleType>(holeStyle);}
DG_INLINE DwgHatchDefCR PatternParams::GetDwgHatchDef() const {return dwgHatchDef;}
DG_INLINE DPoint3dCR PatternParams::GetOrigin() const {return origin;}
DG_INLINE double PatternParams::GetAnnotationScale() const {return annotationscale;}

DG_INLINE void ElemMatSymb::SetIndexedRasterPattern(int32_t index, uint32_t rasterPat) {m_elementStyle = IS_LINECODE(index) ? index : 0; m_rasterPat = rasterPat; }

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

DG_INLINE SnapDetailP     SnapContext::GetSnapDetail() {return m_snapPath;}
DG_INLINE SnapMode      SnapContext::GetSnapMode() {return m_snapMode;}
DG_INLINE int           SnapContext::GetSnapDivisor() {return m_snapDivisor;}

DG_INLINE DgnPlatformLib::Host& DgnPlatformLib::GetHost() {return *static_cast<DgnPlatformLib::Host*>(g_hostForThread.GetValueAsPointer());}

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
