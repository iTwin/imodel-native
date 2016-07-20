/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCoreDLLInlines.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
// WIP_MERGE_John_Patterns - DG_INLINE double          ViewController::GetPatternZOffset(ViewContextR context, ElementHandleCR eh) const {return _GetPatternZOffset(context, eh);}
DG_INLINE StatusInt IViewClipObject::GetCuttingPlane(DPlane3dR cutPlane, DVec3dR xDir, DVec3dR yDir, ClipMask& clipMask, DRange2dR clipRange, bool& forwardFacing, int index, ViewContextR context) const {return _GetCuttingPlane(cutPlane,xDir,yDir,clipMask,clipRange,forwardFacing,index,context);}
DG_INLINE bool IViewClipObject::GetAuxTransform(TransformR t, ClipVolumePass p) const {return _GetAuxTransform(t,p);}
DG_INLINE StatusInt IViewClipObject::GetTransform(TransformR trans) const {return _GetTransform(trans);}
DG_INLINE size_t IViewClipObject::GetPrimaryCutPlaneCount() const {return _GetPrimaryCutPlaneCount();}
DG_INLINE StatusInt IViewClipObject::ApplyTransform(TransformCR t) {return _ApplyTransform(t);}
DG_INLINE void IViewClipObject::Draw(ViewContextR c) {return _Draw(c);}

DG_INLINE IACSManagerR IACSManager::GetManager() {return T_HOST.GetAcsManager();}
DG_INLINE void IAuxCoordSys::DisplayInView(DecorateContextR context, ACSDisplayOptions options, bool drawName) const {return _DisplayInView(context, options, drawName);}
DG_INLINE Utf8String IAuxCoordSys::GetName() const {return _GetName();}
DG_INLINE Utf8String IAuxCoordSys::GetDescription() const {return _GetDescription();}
DG_INLINE ACSType IAuxCoordSys::GetType() const {return _GetType();}
DG_INLINE Utf8String IAuxCoordSys::GetTypeName() const {return _GetTypeName();}
DG_INLINE uint32_t IAuxCoordSys::GetExtenderId() const {return _GetExtenderId();}
DG_INLINE uint32_t IAuxCoordSys::GetSerializedSize() const {return _GetSerializedSize();}
DG_INLINE StatusInt IAuxCoordSys::Serialize(void *buffer, uint32_t maxSize) const {return _Serialize(buffer, maxSize);}
DG_INLINE double IAuxCoordSys::GetScale() const {return _GetScale();}
DG_INLINE DPoint3dR IAuxCoordSys::GetOrigin(DPoint3dR pOrigin) const {return _GetOrigin(pOrigin);}
DG_INLINE bool IAuxCoordSys::GetIsReadOnly() const {return _GetIsReadOnly();}
DG_INLINE RotMatrixR IAuxCoordSys::GetRotation(RotMatrixR pRot) const {return _GetRotation(pRot);}
DG_INLINE RotMatrixR IAuxCoordSys::GetRotation(RotMatrixR pRot, DPoint3dR pPosition) const {return _GetRotation(pRot, pPosition);}
DG_INLINE ACSFlags IAuxCoordSys::GetFlags() const {return _GetFlags();}
DG_INLINE StatusInt IAuxCoordSys::SetName(Utf8CP name) {return _SetName(name);}
DG_INLINE StatusInt IAuxCoordSys::SetDescription(Utf8CP descr) {return _SetDescription(descr);}
DG_INLINE StatusInt IAuxCoordSys::SetType(ACSType type) {return _SetType(type);}
DG_INLINE StatusInt IAuxCoordSys::SetScale(double scale) {return _SetScale(scale);}
DG_INLINE StatusInt IAuxCoordSys::SetOrigin(DPoint3dCR pOrigin) {return _SetOrigin(pOrigin);}
DG_INLINE StatusInt IAuxCoordSys::SetRotation(RotMatrixCR pRot) {return _SetRotation(pRot);}
DG_INLINE StatusInt IAuxCoordSys::PointFromString(DPoint3dR outPoint, Utf8StringR errorMsg, Utf8CP inString, bool relative, DPoint3dCP lastPoint, DgnModelR modelRef) {return _PointFromString(outPoint, errorMsg, inString, relative, lastPoint, modelRef);}
DG_INLINE StatusInt IAuxCoordSys::StringFromPoint(Utf8StringR outString, Utf8StringR errorMsg, DPoint3dCR inPoint, bool delta, DPoint3dCP deltaOrigin, DgnModelR modelRef, DistanceFormatterR distanceFormatter, DirectionFormatterR directionFormatter)
                                                        { return _StringFromPoint(outString, errorMsg, inPoint, delta, deltaOrigin, modelRef, distanceFormatter, directionFormatter); }
DG_INLINE StatusInt IAuxCoordSys::SetFlags(ACSFlags flags) {return _SetFlags(flags);}
DG_INLINE void IAuxCoordSys::DrawGrid(DecorateContextR context) const {return _DrawGrid(context);}
DG_INLINE void IAuxCoordSys::PointToGrid(DgnViewportR viewport, DPoint3dR point) const {_PointToGrid(viewport, point);}
DG_INLINE StatusInt IAuxCoordSys::CompleteSetupFromViewController(SpatialViewControllerCP info) {return _CompleteSetupFromViewController(info);}
DG_INLINE bool IAuxCoordSys::Equals(IAuxCoordSysCP other) const {return _Equals(other);}
DG_INLINE IAuxCoordSysPtr IAuxCoordSys::Clone() const {return _Clone();}

DG_INLINE StatusInt IAuxCoordSys::GetStandardGridParams(Point2dR gridReps, Point2dR gridOffset, double& uorPerGrid, double& gridRatio, uint32_t& gridPerRef) const {return _GetStandardGridParams(gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}
DG_INLINE StatusInt IAuxCoordSys::SetStandardGridParams(Point2dCR gridReps, Point2dCR gridOffset, double uorPerGrid, double gridRatio, uint32_t gridPerRef) {return _SetStandardGridParams(gridReps, gridOffset, uorPerGrid, gridRatio, gridPerRef);}

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
DG_INLINE DPoint3dCR PatternParams::GetOrigin() const {return origin;}
DG_INLINE double PatternParams::GetAnnotationScale() const {return annotationscale;}

DG_INLINE void LineStyleParams::SetScale(double inScale) { modifiers |= STYLEMOD_SCALE; scale = inScale; }
DG_INLINE DgnPlatformLib::Host& DgnPlatformLib::GetHost() {return *static_cast<DgnPlatformLib::Host*>(g_hostForThread.GetValueAsPointer());}

