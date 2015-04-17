/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnHandlersDLLInlines.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------+
/* This file exists to allow non-virtual-interface methods to be inlined in
/* the production builds of Ustation.dll, but non-inlined for debug builds (otherwise you can't step into them).
+--------------------------------------------------------------------------------------*/

#if !defined (__DGNPLATFORM_BUILD__)
    #error This file is only valid inside DgnPlatform.dll
#endif

DG_INLINE bool  IAreaFillPropertiesQuery::GetAreaType (ElementHandleCR eh, bool* isHoleP) const {return _GetAreaType (eh, isHoleP);}
DG_INLINE bool  IAreaFillPropertiesQuery::GetSolidFill (ElementHandleCR eh, uint32_t* fillColorP, bool* alwaysFilledP) const {return _GetSolidFill (eh, fillColorP, alwaysFilledP);}
DG_INLINE bool  IAreaFillPropertiesQuery::GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const {return _GetGradientFill (eh, symb);}
DG_INLINE bool  IAreaFillPropertiesQuery::GetPattern (ElementHandleCR eh, PatternParamsPtr& params, DPoint3dP originP, int index) const {return _GetPattern (eh, params, originP, index);}

DG_INLINE bool  IAreaFillPropertiesEdit::RemoveAreaFill (EditElementHandleR eeh) {return _RemoveAreaFill (eeh);}
DG_INLINE bool  IAreaFillPropertiesEdit::RemovePattern (EditElementHandleR eeh, int index) {return _RemovePattern (eeh, index);}
DG_INLINE bool  IAreaFillPropertiesEdit::AddSolidFill (EditElementHandleR eeh, ColorDef* fillColorP, bool* alwaysFilledP) {return _AddSolidFill (eeh, fillColorP, alwaysFilledP);}
DG_INLINE bool  IAreaFillPropertiesEdit::AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb) {return _AddGradientFill (eeh, symb);}
DG_INLINE bool  IAreaFillPropertiesEdit::AddPattern (EditElementHandleR eeh, PatternParamsR params, int index) {return _AddPattern (eeh, params, index);}
DG_INLINE bool  IAreaFillPropertiesEdit::SetAreaType (EditElementHandleR eeh, bool isHole) {return _SetAreaType (eeh, isHole);}

DG_INLINE IDragManipulatorPtr       IDragManipulatorExtension::GetIDragManipulator (ElementHandleCR elHandle, DisplayPathCP path) {return _GetIDragManipulator (elHandle, path);}
DG_INLINE ITransformManipulatorPtr  ITransformManipulatorExtension::GetITransformManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) {return _GetITransformManipulator (elHandle, opType, modify, path);}
DG_INLINE IDeleteManipulatorPtr     IDeleteManipulatorExtension::GetIDeleteManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) {return _GetIDeleteManipulator (elHandle, opType, modify, path);}
DG_INLINE IPropertyManipulatorPtr   IPropertyManipulatorExtension::GetIPropertyManipulator (ElementHandleCR elHandle, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) {return _GetIPropertyManipulator (elHandle, opType, modify, path);}
DG_INLINE IVertexManipulatorPtr     IVertexManipulatorExtension::GetIVertexManipulator (ElementHandleCR eh, bool insert, HitPathCR path) {return _GetIVertexManipulator (eh, insert, path);}
DG_INLINE IPopupDialogManipulatorPtr IPopupDialogManipulatorExtension::GetIPopupDialogManipulator (ElementHandleCR eh, AgendaOperation opType, AgendaModify modify, DisplayPathCP path) {return _GetIPopupDialogManipulator (eh, opType, modify, path);}

DG_INLINE bool      IDragManipulator::OnCreateControls (ElementHandleCR elHandle) {return _OnCreateControls (elHandle);}
DG_INLINE void      IDragManipulator::OnCleanupControls (ElementHandleCR elHandle) {_OnCleanupControls (elHandle);}
DG_INLINE bool      IDragManipulator::OnSelectControl (ElementHandleCR elHandle,  DgnButtonEventCR ev) {return _OnSelectControl (elHandle, ev);}
DG_INLINE bool      IDragManipulator::OnSelectControl (ElementHandleCR elHandle, HitPathCP path) {return _OnSelectControl (elHandle, path);}
DG_INLINE bool      IDragManipulator::OnMultiSelectControls (ElementHandleCR elHandle,  DgnButtonEventCR ev, SelectionMode mode) {return _OnMultiSelectControls (elHandle, ev, mode);}
DG_INLINE bool      IDragManipulator::OnMultiSelectControls (ElementHandleCR elHandle, FenceParamsP fp, SelectionMode mode) {return _OnMultiSelectControls (elHandle, fp, mode);}
DG_INLINE bool      IDragManipulator::HasSelectedControls (ElementHandleCR elHandle) {return _HasSelectedControls (elHandle);}
DG_INLINE void      IDragManipulator::OnDraw (ElementHandleCR elHandle, DgnViewportP vp) {return _OnDraw (elHandle, vp);}
DG_INLINE Utf8String   IDragManipulator::OnGetDescription (ElementHandleCR elHandle) {return _OnGetDescription (elHandle);}
DG_INLINE bool      IDragManipulator::OnRightClick (ElementHandleCR elHandle,  DgnButtonEventCR ev) {return _OnRightClick (elHandle, ev);}
DG_INLINE void      IDragManipulator::OnNewPath (ElementHandleCR elHandle, DisplayPathCP path) {_OnNewPath (elHandle, path);}
DG_INLINE bool      IDragManipulator::OnDoubleClick (ElementHandleCR elHandle, DisplayPathCP path) {return _OnDoubleClick (elHandle, path);}
DG_INLINE bool      IDragManipulator::OnSetupDrag (DgnButtonEventR ev, EditElementHandleR elHandle) {return _OnSetupDrag (ev, elHandle);}
DG_INLINE void      IDragManipulator::OnStartDrag (ElementHandleCR elHandle,  DgnButtonEventCR ev) {_OnStartDrag (elHandle, ev);}
DG_INLINE void      IDragManipulator::OnCancelDrag (ElementHandleCR elHandle,  DgnButtonEventCR ev) {_OnCancelDrag (elHandle, ev);}
DG_INLINE StatusInt IDragManipulator::OnDrag (EditElementHandleR elHandle,  DgnButtonEventCR ev) {return _OnDrag (elHandle, ev);}
DG_INLINE StatusInt IDragManipulator::OnEndDrag (EditElementHandleR elHandle,  DgnButtonEventCR ev) {return _OnEndDrag (elHandle, ev);}
DG_INLINE StatusInt IDragManipulator::DoDragControls (EditElementHandleR elHandle,  DgnButtonEventCR ev, bool isDynamics) {return _DoDragControls (elHandle, ev, isDynamics);}
DG_INLINE void      IDragManipulator::OnDragModifierKeyTransition (ElementHandleCR elHandle, bool wentDown, int key) {_OnDragModifierKeyTransition (elHandle, wentDown, key);}
DG_INLINE bool      IDragManipulator::IsViewDynamicsControl (ElementHandleCR elHandle, DgnViewportP vp) {return _IsViewDynamicsControl (elHandle, vp);}
DG_INLINE StatusInt IDragManipulator::OnViewMotion (EditElementHandleR elHandle,  DgnButtonEventCR ev) {return _OnViewMotion (elHandle, ev);}
DG_INLINE bool      IDragManipulator::OnClick(DgnButtonEventCR ev, EditElementHandleR eeh) { return _OnClick(ev, eeh); }

DG_INLINE bool      ITransformManipulator::AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda) {return _AcceptElement (elHandle, cantAcceptReason, agenda);}
DG_INLINE bool      ITransformManipulator::OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify) {return _OnModifyChanged (elHandle, agenda, opType, modify);}
DG_INLINE bool      ITransformManipulator::IsTransformGraphics (ElementHandleCR elHandle, TransformInfoCR tInfo) const {return _IsTransformGraphics (elHandle, tInfo);}
DG_INLINE StatusInt ITransformManipulator::OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo, ViewContextP context) {return _OnTransform (elHandle, tInfo, context);}
DG_INLINE StatusInt ITransformManipulator::OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return _OnFenceClip (elHandle, fp, flags);}
DG_INLINE bool      ITransformManipulator::GetAboutCenterPoint (DPoint3d& pt, ElementHandleCR elHandle) {return _GetAboutCenterPoint (pt, elHandle);}
DG_INLINE bool      ITransformManipulator::GetAlignExtents (DRange3d* range, EditElementHandleR elHandle, Transform const* transform) {return _GetAlignExtents (range, elHandle, transform);}

DG_INLINE bool      IDeleteManipulator::AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda) {return _AcceptElement (elHandle, cantAcceptReason, agenda);}
DG_INLINE StatusInt IDeleteManipulator::OnPreDelete (EditElementHandleR elHandle) {return _OnPreDelete (elHandle);}
DG_INLINE StatusInt IDeleteManipulator::OnDelete (EditElementHandleR elHandle) {return _OnDelete (elHandle);}
DG_INLINE StatusInt IDeleteManipulator::OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return _OnFenceClip (elHandle, fp, flags);}

DG_INLINE bool      IPropertyManipulator::AcceptElement (ElementHandleCR elHandle, Utf8StringP cantAcceptReason, ElementAgendaP agenda) {return _AcceptElement (elHandle, cantAcceptReason, agenda);}
DG_INLINE bool      IPropertyManipulator::OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify) {return _OnModifyChanged (elHandle, agenda, opType, modify);}
DG_INLINE StatusInt IPropertyManipulator::OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return _OnFenceClip (elHandle, fp, flags);}
DG_INLINE StatusInt IPropertyManipulator::OnPropertyChange (EditElementHandleR elHandle, PropertyContextR context) {return _OnPropertyChange (elHandle, context);}

DG_INLINE bool      IVertexManipulator::AcceptElement (ElementHandleCR eh, Utf8StringP cantAcceptReason) {return _AcceptElement (eh, cantAcceptReason);}
DG_INLINE StatusInt IVertexManipulator::OnModifyElement (EditElementHandleR eeh, DgnButtonEventCP ev, DPoint3dCR locatePoint, ViewContextP context) {return _OnModifyElement (eeh, ev, locatePoint, context);}

DG_INLINE bool      IPopupDialogManipulator::DoPopup (ElementHandleCR eh) {return _DoPopup (eh);}
DG_INLINE void      IPopupDialogManipulator::ShowPopupDialog (ElementHandleCR elHandle, DgnViewportP viewport, Point2dCR point) {return _ShowPopupDialog (elHandle, viewport, point);}

#if defined (NEEDS_WORK_DGNITEM)
DG_INLINE StatusInt             DisplayHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) {return ERROR;};
DG_INLINE BentleyStatus IAssocRegionQuery::GetParams (ElementHandleCR eh, RegionParams& params) const {return _GetParams (eh, params);}
DG_INLINE BentleyStatus IAssocRegionQuery::GetSeedPoints (ElementHandleCR eh, bvector<DPoint3d>* points) const {return _GetSeedPoints (eh, points);}
DG_INLINE BentleyStatus IAssocRegionQuery::GetRoots (ElementHandleCR eh, bvector<DependencyRoot>* boundaryRoots) const {return _GetRoots (eh, boundaryRoots);}
DG_INLINE BentleyStatus IAssocRegionQuery::GetLoopData (ElementHandleCR eh, bvector<LoopData>* loopData) const {return _GetLoopData (eh, loopData);}

DG_INLINE bool                    IMultilineQuery::IsClosed (ElementHandleCR source) const {return _IsClosed (source);}
DG_INLINE uint32_t                  IMultilineQuery::GetProfileCount (ElementHandleCR source) const {return _GetProfileCount (source);}
DG_INLINE uint32_t                  IMultilineQuery::GetPointCount (ElementHandleCR source) const {return _GetPointCount (source);}
DG_INLINE MultilinePointPtr       IMultilineQuery::GetPoint (ElementHandleCR source, uint32_t pointNumber) const {return _GetPoint (source, pointNumber);}
DG_INLINE JointDef                IMultilineQuery::ExtractJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const {return _ExtractJointDefinition (source, pts, pointNo);}
DG_INLINE JointDef                IMultilineQuery::ExtractCapJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const {return _ExtractCapJointDefinition (source, pts, pointNo);}
DG_INLINE BentleyStatus           IMultilineQuery::ExtractPoints (ElementHandleCR source, DPoint3dP pXYZBuffer, size_t& numPoints, size_t maxOut) const {return _ExtractPoints (source, pXYZBuffer, numPoints, maxOut);}
DG_INLINE MultilineStylePtr       IMultilineQuery::GetStyle (ElementHandleCR eh, MultilineStyleCP seedStyle, uint32_t options) const {return _GetStyle (eh, seedStyle, options);}
DG_INLINE double                  IMultilineQuery::GetStyleScale (ElementHandleCR element) const {return _GetStyleScale (element);}
DG_INLINE MultilineProfilePtr     IMultilineQuery::GetProfile (ElementHandleCR source, int index) const {return _GetProfile (source, index); }
DG_INLINE MultilineSymbologyPtr   IMultilineQuery::GetOriginCap (ElementHandleCR source) const {return _GetOriginCap (source); }
DG_INLINE MultilineSymbologyPtr   IMultilineQuery::GetEndCap (ElementHandleCR source) const {return _GetEndCap (source); }
DG_INLINE MultilineSymbologyPtr   IMultilineQuery::GetMidCap (ElementHandleCR source) const {return _GetMidCap (source); }
DG_INLINE MlineOffsetMode         IMultilineQuery::GetOffsetMode (ElementHandleCR source) const {return _GetOffsetMode (source); }
DG_INLINE double                  IMultilineQuery::GetPlacementOffset (ElementHandleCR source) const {return _GetPlacementOffset (source); }
DG_INLINE uint32_t                  IMultilineQuery::GetBreakCount (ElementHandleCR source) const {return _GetBreakCount (source); }
DG_INLINE MultilineBreakPtr       IMultilineQuery::GetBreak (ElementHandleCR source, uint32_t segmentNumber, uint32_t segBreakNumber) const {return _GetBreak (source, segmentNumber, segBreakNumber); }
DG_INLINE double                  IMultilineQuery::GetOriginAngle (ElementHandleCR source) const {return _GetOriginAngle (source); }
DG_INLINE double                  IMultilineQuery::GetEndAngle (ElementHandleCR source) const {return _GetEndAngle (source); }
DG_INLINE LevelId                 IMultilineQuery::GetEffectiveSymbologyLevel (ElementHandleCR source, uint32_t index, bool isCap) const { return _GetEffectiveSymbologyLevel (source, index, isCap); }
DG_INLINE uint32_t                  IMultilineQuery::GetEffectiveSymbologyColor (ElementHandleCR source, uint32_t index, bool isCap) const { return _GetEffectiveSymbologyColor (source, index, isCap); }
DG_INLINE uint32_t                  IMultilineQuery::GetEffectiveSymbologyWeight (ElementHandleCR source, uint32_t index, bool isCap) const { return _GetEffectiveSymbologyWeight (source, index, isCap); }
DG_INLINE int32_t                   IMultilineQuery::GetEffectiveSymbologyStyle (ElementHandleCR source, uint32_t index, bool isCap) const { return _GetEffectiveSymbologyStyle (source, index, isCap); }

DG_INLINE BentleyStatus           IMultilineEdit::ReplacePoint (EditElementHandleR source, DPoint3dCR newPoint, uint32_t pointNum, MlineModifyPoint options) {return _ReplacePoint (source, newPoint, pointNum, options);}
DG_INLINE BentleyStatus           IMultilineEdit::InsertPoint (EditElementHandleR source, DPoint3dCR newPoint, AssocPoint const * assocPt, uint32_t pointNum) {return _InsertPoint (source, newPoint, assocPt, pointNum);}
DG_INLINE BentleyStatus           IMultilineEdit::DeletePoint (EditElementHandleR source, uint32_t pointNum) {return _DeletePoint (source, pointNum);}
DG_INLINE BentleyStatus           IMultilineEdit::SetZVector (EditElementHandleR element, DVec3dCR normal) {return _SetZVector (element, normal);}
DG_INLINE BentleyStatus           IMultilineEdit::SetClosed (EditElementHandleR element, bool isClosed) {return _SetClosed (element, isClosed);}
DG_INLINE BentleyStatus           IMultilineEdit::ApplyStyle (EditElementHandleR element, MultilineStyleCR mlineStyle, double styleScale) {return _ApplyStyle (element, mlineStyle, styleScale);}
DG_INLINE BentleyStatus           IMultilineEdit::SetProfile (EditElementHandleR element, uint32_t index, MultilineProfileCR profile) {return _SetProfile (element, index, profile);}
DG_INLINE BentleyStatus           IMultilineEdit::SetOriginCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) {return _SetOriginCap (element, capSymbology);}
DG_INLINE BentleyStatus           IMultilineEdit::SetEndCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) {return _SetEndCap (element, capSymbology);}
DG_INLINE BentleyStatus           IMultilineEdit::SetMidCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) {return _SetMidCap (element, capSymbology);}
DG_INLINE BentleyStatus           IMultilineEdit::SetOffsetMode (EditElementHandleR element, MlineOffsetMode offsetMode) {return _SetOffsetMode (element, offsetMode);}
DG_INLINE BentleyStatus           IMultilineEdit::SetPlacementOffset (EditElementHandleR element, double placementOffset) {return _SetPlacementOffset (element, placementOffset);}
DG_INLINE BentleyStatus           IMultilineEdit::InsertBreak (EditElementHandleR element, MultilineBreakCR mlbreak, uint32_t segment) {return _InsertBreak (element, mlbreak, segment);}
DG_INLINE BentleyStatus           IMultilineEdit::DeleteBreak (EditElementHandleR element, uint32_t segment, uint32_t breakNo) {return _DeleteBreak (element, segment, breakNo);}
DG_INLINE BentleyStatus           IMultilineEdit::SetOriginAngle (EditElementHandleR element, double angle) {return _SetOriginAngle (element, angle);}
DG_INLINE BentleyStatus           IMultilineEdit::SetEndAngle (EditElementHandleR element, double angle) {return _SetEndAngle (element, angle);}
DG_INLINE DimensionStylePtr     IDimensionQuery::GetDimensionStyle (ElementHandleCR eh) const {return _GetDimensionStyle (eh);}
DG_INLINE DimStylePropMaskPtr   IDimensionQuery::GetOverrideFlags (ElementHandleCR eh) const {return _GetOverrideFlags (eh);}
DG_INLINE int                   IDimensionQuery::GetNumPoints (ElementHandleCR eh) const {return _GetNumPoints (eh);}
DG_INLINE int                   IDimensionQuery::GetNumSegments (ElementHandleCR eh) const {return _GetNumSegments (eh);}
DG_INLINE BentleyStatus         IDimensionQuery::ExtractPoint (ElementHandleCR eh, DPoint3dR point, int iPoint) const {return _ExtractPoint (eh, point, iPoint);}
DG_INLINE BentleyStatus         IDimensionQuery::GetHeight (ElementHandleCR eh, double& height) const {return _GetHeight (eh, height);}
DG_INLINE BentleyStatus         IDimensionQuery::GetJustification (ElementHandleCR eh, int segmentNo, DimStyleProp_Text_Justification& just) const{return _GetJustification (eh, segmentNo, just);}
DG_INLINE DimensionType         IDimensionQuery::GetDimensionType (ElementHandleCR eh) const {return _GetDimensionType(eh);}
DG_INLINE BentleyStatus         IDimensionQuery::GetStackHeight (ElementHandleCR eh, int segmentNo, double& height) const {return _GetStackHeight (eh, segmentNo, height);}
DG_INLINE BentleyStatus         IDimensionQuery::GetRotationMatrix (ElementHandleCR eh, RotMatrixR rmatrix) const {return _GetRotationMatrix (eh, rmatrix);}
DG_INLINE bool                  IDimensionQuery::IsTextOutside (ElementHandleCR eh) const {return _IsTextOutside (eh);}
DG_INLINE BentleyStatus         IDimensionQuery::IsPointAssociative (ElementHandleCR eh, int pointNo, bool& flag) const {return _IsPointAssociative(eh, pointNo, flag);}
DG_INLINE bool                  IDimensionQuery::IsVertexInsertable (ElementHandleCR eh) const {return _IsVertexInsertable (eh);}
DG_INLINE bool                  IDimensionQuery::IsVertexDeletable (ElementHandleCR eh, HitPathCR hitPath) const {return _IsVertexDeletable (eh, hitPath);}
DG_INLINE BentleyStatus         IDimensionQuery::GetProxyCell (ElementHandleCR dimElement, DgnPlatform::DgnElementId& proxyCellId, DPoint3dP origin, RotMatrixP rotMatrix) const {return _GetProxyCell(dimElement, proxyCellId, origin, rotMatrix);}
DG_INLINE BentleyStatus         IDimensionQuery::GetTextOffset (ElementHandleCR dimElement, int segmentNo, DPoint2dR offset) const {return _GetTextOffset(dimElement, segmentNo, offset);}
DG_INLINE BentleyStatus         IDimensionQuery::GetViewRotation (ElementHandleCR dimElement, RotMatrixR rmatrix) const {return _GetViewRotation(dimElement, rmatrix);}
DG_INLINE bool                  IDimensionQuery::GetAngularDimensionClockWiseSweep (ElementHandleCR dimElement) const {return _GetAngularDimensionClockWiseSweep(dimElement);}
DG_INLINE BentleyStatus         IDimensionQuery::GetWitnessVisibility (ElementHandleCR eh, int pointNo, bool& value) const {return _GetWitnessVisibility (eh, pointNo, value);}
DG_INLINE BentleyStatus         IDimensionQuery::GetWitnessUseAltSymbology (ElementHandleCR eh, int pointNo, bool& value) const {return _GetWitnessUseAltSymbology (eh, pointNo, value);}

DG_INLINE void              IDimensionEdit::ApplyDimensionStyle (EditElementHandleR eeh, DimensionStyleCR dimStyle, bool retainOverrides) { _ApplyDimensionStyle (eeh, dimStyle, retainOverrides);}
DG_INLINE BentleyStatus     IDimensionEdit::SetPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, int pointNo) { return _SetPoint (eeh, point, assocPt, pointNo); }
DG_INLINE BentleyStatus     IDimensionEdit::InsertPoint (EditElementHandleR eeh, DPoint3dCP point, AssocPoint const* assocPt, DimensionStyleCR dimStyle, int pointNo) { return _InsertPoint (eeh, point, assocPt, dimStyle, pointNo); }
DG_INLINE BentleyStatus     IDimensionEdit::SetHeight (EditElementHandleR eeh, double height) { return _SetHeight (eeh, height); }
DG_INLINE BentleyStatus     IDimensionEdit::SetJustification (EditElementHandleR eeh, int segmentNo, DimStyleProp_Text_Justification just) {return _SetJustification (eeh, segmentNo, just);}
DG_INLINE BentleyStatus     IDimensionEdit::SetRotationMatrix (EditElementHandleR eh, RotMatrixCR rmatrix) {return _SetRotationMatrix (eh, rmatrix);}
DG_INLINE BentleyStatus     IDimensionEdit::DeletePoint (EditElementHandleR eh, int pointNo) {return _DeletePoint (eh, pointNo);}
DG_INLINE BentleyStatus     IDimensionEdit::InsertVertex (EditElementHandleR eh, HitPathCR hitPath, DPoint3dCR point) {return _InsertVertex (eh, hitPath, point);}
DG_INLINE BentleyStatus     IDimensionEdit::DeleteVertex (EditElementHandleR eh, HitPathCR hitPath) {return _DeleteVertex (eh, hitPath);}
DG_INLINE DgnHandlersStatus IDimensionEdit::SetProxyCell (EditElementHandleR dimElement, DgnPlatform::DgnElementId const& proxyCellId, DPoint3dCR origin, RotMatrixCR rotMatrix) {return _SetProxyCell(dimElement, proxyCellId, origin, rotMatrix);}
DG_INLINE BentleyStatus     IDimensionEdit::SetTextOffset (EditElementHandleR dimElement, int segmentNo, DPoint2dCR offset) {return _SetTextOffset(dimElement, segmentNo, offset);}
DG_INLINE BentleyStatus     IDimensionEdit::SetViewRotation (EditElementHandleR dimElement, RotMatrixCR rMatrix) {return _SetViewRotation(dimElement, rMatrix);}
DG_INLINE void              IDimensionEdit::SetAngularDimensionClockWiseSweep (EditElementHandleR dimElement, bool value) {return _SetAngularDimensionClockWiseSweep(dimElement, value);}
DG_INLINE BentleyStatus     IDimensionEdit::SetWitnessVisibility (EditElementHandleR eeh, int pointNo, bool value) {return _SetWitnessVisibility(eeh, pointNo, value);}
DG_INLINE BentleyStatus     IDimensionEdit::SetWitnessUseAltSymbology (EditElementHandleR eeh, int pointNo, bool value) {return _SetWitnessUseAltSymbology(eeh, pointNo, value);}
DG_INLINE BentleyStatus     IDimensionEdit::SetPointsForLabelLine (EditElementHandleR eeh, DSegment3dCP segment, HitPathP hitPath, double offset, RotMatrixCR viewRMatrix, DimensionStyleCR dimStyle) {return _SetPointsForLabelLine (eeh, segment, hitPath, offset, viewRMatrix, dimStyle);}


DG_INLINE StatusInt DimensionStyle::SetAccuracyProp (Byte valueIn, DimStyleProp iProp) {return _SetAccuracyProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetBooleanProp (bool valueIn, DimStyleProp iProp) {return _SetBooleanProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetCharProp (unsigned short valueIn, DimStyleProp iProp) {return _SetCharProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetDoubleProp (double valueIn, DimStyleProp iProp) {return _SetDoubleProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetIntegerProp (int valueIn, DimStyleProp iProp) {return _SetIntegerProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetLevelProp (LevelId valueIn, DimStyleProp iProp) {return _SetLevelProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetStringProp (WCharCP valueIn, DimStyleProp iProp) {return _SetStringProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetFontProp (uint32_t valueIn, DimStyleProp iProp) {return _SetFontProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetColorProp (uint32_t valueIn, DimStyleProp iProp) {return _SetColorProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetWeightProp (uint32_t valueIn, DimStyleProp iProp) {return _SetWeightProp (valueIn, iProp);} 
DG_INLINE StatusInt DimensionStyle::SetLineStyleProp (int32_t valueIn, DimStyleProp iProp) {return _SetLineStyleProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetTextStyleProp (DgnTextStyleCR valueIn, DimStyleProp iProp) {return _SetTextStyleProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetTemplateFlagProp(unsigned short     valueIn,  DimensionType dimCmdIndex, DimStyleProp iProp) {return _SetTemplateFlagProp (valueIn, dimCmdIndex, iProp);}
DG_INLINE void DimensionStyle::CopyValues (DimensionStyleCR source) {return _CopyValues(source);}
DG_INLINE StatusInt DimensionStyle::SetUnitsProp (UnitDefinitionCR masterUnit, UnitDefinitionCR subUnit, DimStyleProp iProperty){return _SetUnitsProp(masterUnit, subUnit, iProperty);}
DG_INLINE StatusInt DimensionStyle::GetTextStyleProp (DgnTextStylePtr& textStyle, DimStyleProp iProperty) const {return _GetTextStyleProp(textStyle, iProperty);}

DG_INLINE uint32_t DimensionTextPartId::GetPartSegment () const {return m_partSegment;}
DG_INLINE DimensionTextPartType DimensionTextPartId::GetPartType () const {return m_partType;}
DG_INLINE DimensionTextPartSubType DimensionTextPartId::GetPartSubType () const {return m_partSubType;}
#endif

#if defined (NEEDS_WORK_CONVERTER)
DG_INLINE MaterialCP    IMaterialPropertiesExtension::FindMaterialAttachment (ElementHandleCR eh) const { return _FindMaterialAttachment (eh); }
DG_INLINE BentleyStatus IMaterialPropertiesExtension::AddMaterialAttachment (EditElementHandleR eeh, DgnMaterialId id) {return _AddMaterialAttachment (eeh, id);}
DG_INLINE bool          IMaterialPropertiesExtension::SupportsSubEntityAttachments (ElementHandleCR eh) const {return _SupportsSubEntityAttachments (eh);}
DG_INLINE bool          IMaterialPropertiesExtension::HasSubEntityAttachments (ElementHandleCR eh) const {return _HasSubEntityAttachments (eh);}
DG_INLINE BentleyStatus IMaterialPropertiesExtension::GetSubEntityAttachments (ElementHandleCR eh, MaterialSubEntityAttachments& attachments) const {return _GetSubEntityAttachments (eh, attachments);}
#endif

DG_INLINE WString   IEcPropertyHandler::GetEcPropertiesClassName (ElementHandleCR eh) {return _GetEcPropertiesClassName (eh);}
DG_INLINE StatusInt IEcPropertyHandler::GetEcProperties (T_EcCategories& t, ElementHandleCR eh) {return _GetEcProperties (t, eh);}
DG_INLINE bool      IEcPropertyHandler::IsPropertyReadOnly (ElementHandleCR eh, uint32_t propId, size_t arrayIndex) {return _IsPropertyReadOnly (eh, propId, arrayIndex);}
DG_INLINE bool      IEcPropertyHandler::IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName) {return _IsNullProperty (enabler, className, propName);}
DG_INLINE IsNullReturnType IEcPropertyHandler::IsNullProperty (ElementHandleCR eh, uint32_t propId, size_t arrayIndex) {return _IsNullProperty (eh, propId, arrayIndex);}

DG_INLINE BentleyStatus IRasterSourceFileQuery::InitFrom(ElementHandleCR eh) { return _InitFrom(eh); }
DG_INLINE uint64_t IRasterSourceFileQuery::GetBitmapHeight () const { return _GetBitmapHeight(); }
DG_INLINE uint64_t IRasterSourceFileQuery::GetBitmapWidth () const { return _GetBitmapWidth(); }
DG_INLINE BentleyStatus IRasterSourceFileQuery::ReadToRGBA (Byte *RgbaBuffer, size_t maxBufferLength, bool useBgIfNoAlpha) const { return _ReadToRGBA(RgbaBuffer,maxBufferLength,useBgIfNoAlpha); }

