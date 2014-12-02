/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnHandlersDLLInlines.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*--------------------------------------------------------------------------------------+
/* This file exists to allow non-virtual-interface methods to be inlined in
/* the production builds of Ustation.dll, but non-inlined for debug builds (otherwise you can't step into them).
+--------------------------------------------------------------------------------------*/

#if !defined (__DGNPLATFORM_BUILD__)
    #error This file is only valid inside DgnHandlers.dll
#endif

// DG_INLINE is defined by dgncoredllinlines.h, and that is *always* included just before this is included
//#if defined (NDEBUG) && !defined (__DGNHANDLERS_DONT_INLINE__)
//    #define DG_INLINE inline
//#else
//    #define DG_INLINE
//#endif

DG_INLINE void Handler::_OnConvertTo3d (EditElementHandleR eeh, double elevation){}
DG_INLINE void Handler::_OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) {}
DG_INLINE void Handler::_QueryProperties (ElementHandleCR eh, PropertyContextR context) {}
DG_INLINE void Handler::_EditProperties (EditElementHandleR eeh, PropertyContextR context) {}
DG_INLINE StatusInt Handler::_OnTransform (EditElementHandleR element, TransformInfoCR transform) {return SUCCESS;}
DG_INLINE bool Handler::_IsTransformGraphics (ElementHandleCR elemHandle, TransformInfoCR) {return true; }
DG_INLINE StatusInt Handler::_OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options) {return ERROR;}
DG_INLINE StatusInt Handler::_OnFenceStretch (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options) {return ERROR;}

DG_INLINE bool                  DisplayHandler::_IsRenderable (ElementHandleCR) {return false;}
DG_INLINE StatusInt             DisplayHandler::_EvaluateCustomKeypoint (ElementHandleCR elHandle, DPoint3dP outPoint, byte* customKeypointData) {return ERROR;}
DG_INLINE IAnnotationHandlerP   DisplayHandler::_GetIAnnotationHandler (ElementHandleCR) {return NULL;}

DG_INLINE ITextQueryCP  ElementHandle::GetITextQuery () const {return dynamic_cast <ITextQueryCP> (&GetHandler());}
DG_INLINE ITextEditP    ElementHandle::GetITextEdit() const {return dynamic_cast <ITextEditP> (&GetHandler());}

DG_INLINE bool  IAreaFillPropertiesQuery::GetAreaType (ElementHandleCR eh, bool* isHoleP) const {return _GetAreaType (eh, isHoleP);}
DG_INLINE bool  IAreaFillPropertiesQuery::GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const {return _GetSolidFill (eh, fillColorP, alwaysFilledP);}
DG_INLINE bool  IAreaFillPropertiesQuery::GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const {return _GetGradientFill (eh, symb);}
DG_INLINE bool  IAreaFillPropertiesQuery::GetPattern (ElementHandleCR eh, PatternParamsPtr& params, bvector<DwgHatchDefLine>* hatchDefLinesP, DPoint3dP originP, int index) const {return _GetPattern (eh, params, hatchDefLinesP, originP, index);}

DG_INLINE bool  IAreaFillPropertiesEdit::RemoveAreaFill (EditElementHandleR eeh) {return _RemoveAreaFill (eeh);}
DG_INLINE bool  IAreaFillPropertiesEdit::RemovePattern (EditElementHandleR eeh, int index) {return _RemovePattern (eeh, index);}
DG_INLINE bool  IAreaFillPropertiesEdit::AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP) {return _AddSolidFill (eeh, fillColorP, alwaysFilledP);}
DG_INLINE bool  IAreaFillPropertiesEdit::AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb) {return _AddGradientFill (eeh, symb);}
DG_INLINE bool  IAreaFillPropertiesEdit::AddPattern (EditElementHandleR eeh, PatternParamsR params, DwgHatchDefLineP hatchDefLinesP, int index) {return _AddPattern (eeh, params, hatchDefLinesP, index);}
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
DG_INLINE void      IDragManipulator::OnDraw (ElementHandleCR elHandle, ViewportP vp) {return _OnDraw (elHandle, vp);}
DG_INLINE WString   IDragManipulator::OnGetDescription (ElementHandleCR elHandle) {return _OnGetDescription (elHandle);}
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
DG_INLINE bool      IDragManipulator::IsViewDynamicsControl (ElementHandleCR elHandle, ViewportP vp) {return _IsViewDynamicsControl (elHandle, vp);}
DG_INLINE StatusInt IDragManipulator::OnViewMotion (EditElementHandleR elHandle,  DgnButtonEventCR ev) {return _OnViewMotion (elHandle, ev);}
DG_INLINE bool      IDragManipulator::OnClick(DgnButtonEventCR ev, EditElementHandleR eeh) { return _OnClick(ev, eeh); }

DG_INLINE bool      ITransformManipulator::AcceptElement (ElementHandleCR elHandle, WStringP cantAcceptReason, ElementAgendaP agenda) {return _AcceptElement (elHandle, cantAcceptReason, agenda);}
DG_INLINE bool      ITransformManipulator::OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify) {return _OnModifyChanged (elHandle, agenda, opType, modify);}
DG_INLINE bool      ITransformManipulator::IsTransformGraphics (ElementHandleCR elHandle, TransformInfoCR tInfo) const {return _IsTransformGraphics (elHandle, tInfo);}
DG_INLINE StatusInt ITransformManipulator::OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo, ViewContextP context) {return _OnTransform (elHandle, tInfo, context);}
DG_INLINE StatusInt ITransformManipulator::OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return _OnFenceClip (elHandle, fp, flags);}
DG_INLINE bool      ITransformManipulator::GetAboutCenterPoint (DPoint3d& pt, ElementHandleCR elHandle) {return _GetAboutCenterPoint (pt, elHandle);}
DG_INLINE bool      ITransformManipulator::GetAlignExtents (DRange3d* range, EditElementHandleR elHandle, Transform const* transform) {return _GetAlignExtents (range, elHandle, transform);}

DG_INLINE bool      IDeleteManipulator::AcceptElement (ElementHandleCR elHandle, WStringP cantAcceptReason, ElementAgendaP agenda) {return _AcceptElement (elHandle, cantAcceptReason, agenda);}
DG_INLINE StatusInt IDeleteManipulator::OnPreDelete (EditElementHandleR elHandle) {return _OnPreDelete (elHandle);}
DG_INLINE StatusInt IDeleteManipulator::OnDelete (EditElementHandleR elHandle) {return _OnDelete (elHandle);}
DG_INLINE StatusInt IDeleteManipulator::OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return _OnFenceClip (elHandle, fp, flags);}

DG_INLINE bool      IPropertyManipulator::AcceptElement (ElementHandleCR elHandle, WStringP cantAcceptReason, ElementAgendaP agenda) {return _AcceptElement (elHandle, cantAcceptReason, agenda);}
DG_INLINE bool      IPropertyManipulator::OnModifyChanged (ElementHandleCR elHandle, ElementAgendaP agenda, AgendaOperation opType, AgendaModify modify) {return _OnModifyChanged (elHandle, agenda, opType, modify);}
DG_INLINE StatusInt IPropertyManipulator::OnFenceClip (EditElementHandleR elHandle, FenceParamsP fp, FenceClipFlags flags) {return _OnFenceClip (elHandle, fp, flags);}
DG_INLINE StatusInt IPropertyManipulator::OnPropertyChange (EditElementHandleR elHandle, PropertyContextR context) {return _OnPropertyChange (elHandle, context);}

DG_INLINE bool      IVertexManipulator::AcceptElement (ElementHandleCR eh, WStringP cantAcceptReason) {return _AcceptElement (eh, cantAcceptReason);}
DG_INLINE StatusInt IVertexManipulator::OnModifyElement (EditElementHandleR eeh, DgnButtonEventCP ev, DPoint3dCR locatePoint, ViewContextP context) {return _OnModifyElement (eeh, ev, locatePoint, context);}

DG_INLINE bool      IPopupDialogManipulator::DoPopup (ElementHandleCR eh) {return _DoPopup (eh);}
DG_INLINE void      IPopupDialogManipulator::ShowPopupDialog (ElementHandleCR elHandle, ViewportP viewport, Point2dCR point) {return _ShowPopupDialog (elHandle, viewport, point);}

#if defined (NEEDS_WORK_DGNITEM)
DG_INLINE StatusInt             DisplayHandler::_OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) {return ERROR;};
DG_INLINE BentleyStatus IAssocRegionQuery::GetParams (ElementHandleCR eh, RegionParams& params) const {return _GetParams (eh, params);}
DG_INLINE BentleyStatus IAssocRegionQuery::GetSeedPoints (ElementHandleCR eh, bvector<DPoint3d>* points) const {return _GetSeedPoints (eh, points);}
DG_INLINE BentleyStatus IAssocRegionQuery::GetRoots (ElementHandleCR eh, bvector<DependencyRoot>* boundaryRoots) const {return _GetRoots (eh, boundaryRoots);}
DG_INLINE BentleyStatus IAssocRegionQuery::GetLoopData (ElementHandleCR eh, bvector<LoopData>* loopData) const {return _GetLoopData (eh, loopData);}

DG_INLINE bool                    IMultilineQuery::IsClosed (ElementHandleCR source) const {return _IsClosed (source);}
DG_INLINE UInt32                  IMultilineQuery::GetProfileCount (ElementHandleCR source) const {return _GetProfileCount (source);}
DG_INLINE UInt32                  IMultilineQuery::GetPointCount (ElementHandleCR source) const {return _GetPointCount (source);}
DG_INLINE MultilinePointPtr       IMultilineQuery::GetPoint (ElementHandleCR source, UInt32 pointNumber) const {return _GetPoint (source, pointNumber);}
DG_INLINE JointDef                IMultilineQuery::ExtractJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const {return _ExtractJointDefinition (source, pts, pointNo);}
DG_INLINE JointDef                IMultilineQuery::ExtractCapJointDefinition (ElementHandleCR source, DPoint3dCP pts, int pointNo) const {return _ExtractCapJointDefinition (source, pts, pointNo);}
DG_INLINE BentleyStatus           IMultilineQuery::ExtractPoints (ElementHandleCR source, DPoint3dP pXYZBuffer, size_t& numPoints, size_t maxOut) const {return _ExtractPoints (source, pXYZBuffer, numPoints, maxOut);}
DG_INLINE MultilineStylePtr       IMultilineQuery::GetStyle (ElementHandleCR eh, MultilineStyleCP seedStyle, UInt32 options) const {return _GetStyle (eh, seedStyle, options);}
DG_INLINE double                  IMultilineQuery::GetStyleScale (ElementHandleCR element) const {return _GetStyleScale (element);}
DG_INLINE MultilineProfilePtr     IMultilineQuery::GetProfile (ElementHandleCR source, int index) const {return _GetProfile (source, index); }
DG_INLINE MultilineSymbologyPtr   IMultilineQuery::GetOriginCap (ElementHandleCR source) const {return _GetOriginCap (source); }
DG_INLINE MultilineSymbologyPtr   IMultilineQuery::GetEndCap (ElementHandleCR source) const {return _GetEndCap (source); }
DG_INLINE MultilineSymbologyPtr   IMultilineQuery::GetMidCap (ElementHandleCR source) const {return _GetMidCap (source); }
DG_INLINE MlineOffsetMode         IMultilineQuery::GetOffsetMode (ElementHandleCR source) const {return _GetOffsetMode (source); }
DG_INLINE double                  IMultilineQuery::GetPlacementOffset (ElementHandleCR source) const {return _GetPlacementOffset (source); }
DG_INLINE UInt32                  IMultilineQuery::GetBreakCount (ElementHandleCR source) const {return _GetBreakCount (source); }
DG_INLINE MultilineBreakPtr       IMultilineQuery::GetBreak (ElementHandleCR source, UInt32 segmentNumber, UInt32 segBreakNumber) const {return _GetBreak (source, segmentNumber, segBreakNumber); }
DG_INLINE double                  IMultilineQuery::GetOriginAngle (ElementHandleCR source) const {return _GetOriginAngle (source); }
DG_INLINE double                  IMultilineQuery::GetEndAngle (ElementHandleCR source) const {return _GetEndAngle (source); }
DG_INLINE LevelId                 IMultilineQuery::GetEffectiveSymbologyLevel (ElementHandleCR source, UInt32 index, bool isCap) const { return _GetEffectiveSymbologyLevel (source, index, isCap); }
DG_INLINE UInt32                  IMultilineQuery::GetEffectiveSymbologyColor (ElementHandleCR source, UInt32 index, bool isCap) const { return _GetEffectiveSymbologyColor (source, index, isCap); }
DG_INLINE UInt32                  IMultilineQuery::GetEffectiveSymbologyWeight (ElementHandleCR source, UInt32 index, bool isCap) const { return _GetEffectiveSymbologyWeight (source, index, isCap); }
DG_INLINE Int32                   IMultilineQuery::GetEffectiveSymbologyStyle (ElementHandleCR source, UInt32 index, bool isCap) const { return _GetEffectiveSymbologyStyle (source, index, isCap); }

DG_INLINE BentleyStatus           IMultilineEdit::ReplacePoint (EditElementHandleR source, DPoint3dCR newPoint, UInt32 pointNum, MlineModifyPoint options) {return _ReplacePoint (source, newPoint, pointNum, options);}
DG_INLINE BentleyStatus           IMultilineEdit::InsertPoint (EditElementHandleR source, DPoint3dCR newPoint, AssocPoint const * assocPt, UInt32 pointNum) {return _InsertPoint (source, newPoint, assocPt, pointNum);}
DG_INLINE BentleyStatus           IMultilineEdit::DeletePoint (EditElementHandleR source, UInt32 pointNum) {return _DeletePoint (source, pointNum);}
DG_INLINE BentleyStatus           IMultilineEdit::SetZVector (EditElementHandleR element, DVec3dCR normal) {return _SetZVector (element, normal);}
DG_INLINE BentleyStatus           IMultilineEdit::SetClosed (EditElementHandleR element, bool isClosed) {return _SetClosed (element, isClosed);}
DG_INLINE BentleyStatus           IMultilineEdit::ApplyStyle (EditElementHandleR element, MultilineStyleCR mlineStyle, double styleScale) {return _ApplyStyle (element, mlineStyle, styleScale);}
DG_INLINE BentleyStatus           IMultilineEdit::SetProfile (EditElementHandleR element, UInt32 index, MultilineProfileCR profile) {return _SetProfile (element, index, profile);}
DG_INLINE BentleyStatus           IMultilineEdit::SetOriginCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) {return _SetOriginCap (element, capSymbology);}
DG_INLINE BentleyStatus           IMultilineEdit::SetEndCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) {return _SetEndCap (element, capSymbology);}
DG_INLINE BentleyStatus           IMultilineEdit::SetMidCap (EditElementHandleR element, MultilineSymbologyCR capSymbology) {return _SetMidCap (element, capSymbology);}
DG_INLINE BentleyStatus           IMultilineEdit::SetOffsetMode (EditElementHandleR element, MlineOffsetMode offsetMode) {return _SetOffsetMode (element, offsetMode);}
DG_INLINE BentleyStatus           IMultilineEdit::SetPlacementOffset (EditElementHandleR element, double placementOffset) {return _SetPlacementOffset (element, placementOffset);}
DG_INLINE BentleyStatus           IMultilineEdit::InsertBreak (EditElementHandleR element, MultilineBreakCR mlbreak, UInt32 segment) {return _InsertBreak (element, mlbreak, segment);}
DG_INLINE BentleyStatus           IMultilineEdit::DeleteBreak (EditElementHandleR element, UInt32 segment, UInt32 breakNo) {return _DeleteBreak (element, segment, breakNo);}
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
DG_INLINE BentleyStatus         IDimensionQuery::GetProxyCell (ElementHandleCR dimElement, DgnPlatform::ElementId& proxyCellId, DPoint3dP origin, RotMatrixP rotMatrix) const {return _GetProxyCell(dimElement, proxyCellId, origin, rotMatrix);}
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
DG_INLINE DgnHandlersStatus IDimensionEdit::SetProxyCell (EditElementHandleR dimElement, DgnPlatform::ElementId const& proxyCellId, DPoint3dCR origin, RotMatrixCR rotMatrix) {return _SetProxyCell(dimElement, proxyCellId, origin, rotMatrix);}
DG_INLINE BentleyStatus     IDimensionEdit::SetTextOffset (EditElementHandleR dimElement, int segmentNo, DPoint2dCR offset) {return _SetTextOffset(dimElement, segmentNo, offset);}
DG_INLINE BentleyStatus     IDimensionEdit::SetViewRotation (EditElementHandleR dimElement, RotMatrixCR rMatrix) {return _SetViewRotation(dimElement, rMatrix);}
DG_INLINE void              IDimensionEdit::SetAngularDimensionClockWiseSweep (EditElementHandleR dimElement, bool value) {return _SetAngularDimensionClockWiseSweep(dimElement, value);}
DG_INLINE BentleyStatus     IDimensionEdit::SetWitnessVisibility (EditElementHandleR eeh, int pointNo, bool value) {return _SetWitnessVisibility(eeh, pointNo, value);}
DG_INLINE BentleyStatus     IDimensionEdit::SetWitnessUseAltSymbology (EditElementHandleR eeh, int pointNo, bool value) {return _SetWitnessUseAltSymbology(eeh, pointNo, value);}
DG_INLINE BentleyStatus     IDimensionEdit::SetPointsForLabelLine (EditElementHandleR eeh, DSegment3dCP segment, HitPathP hitPath, double offset, RotMatrixCR viewRMatrix, DimensionStyleCR dimStyle) {return _SetPointsForLabelLine (eeh, segment, hitPath, offset, viewRMatrix, dimStyle);}


DG_INLINE StatusInt DimensionStyle::SetAccuracyProp (byte valueIn, DimStyleProp iProp) {return _SetAccuracyProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetBooleanProp (bool valueIn, DimStyleProp iProp) {return _SetBooleanProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetCharProp (UShort valueIn, DimStyleProp iProp) {return _SetCharProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetDoubleProp (double valueIn, DimStyleProp iProp) {return _SetDoubleProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetIntegerProp (int valueIn, DimStyleProp iProp) {return _SetIntegerProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetLevelProp (LevelId valueIn, DimStyleProp iProp) {return _SetLevelProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetStringProp (WCharCP valueIn, DimStyleProp iProp) {return _SetStringProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetFontProp (UInt32 valueIn, DimStyleProp iProp) {return _SetFontProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetColorProp (UInt32 valueIn, DimStyleProp iProp) {return _SetColorProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetWeightProp (UInt32 valueIn, DimStyleProp iProp) {return _SetWeightProp (valueIn, iProp);} 
DG_INLINE StatusInt DimensionStyle::SetLineStyleProp (Int32 valueIn, DimStyleProp iProp) {return _SetLineStyleProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetTextStyleProp (DgnTextStyleCR valueIn, DimStyleProp iProp) {return _SetTextStyleProp (valueIn, iProp);}
DG_INLINE StatusInt DimensionStyle::SetTemplateFlagProp(UShort     valueIn,  DimensionType dimCmdIndex, DimStyleProp iProp) {return _SetTemplateFlagProp (valueIn, dimCmdIndex, iProp);}
DG_INLINE void DimensionStyle::CopyValues (DimensionStyleCR source) {return _CopyValues(source);}
DG_INLINE StatusInt DimensionStyle::SetUnitsProp (UnitDefinitionCR masterUnit, UnitDefinitionCR subUnit, DimStyleProp iProperty){return _SetUnitsProp(masterUnit, subUnit, iProperty);}
DG_INLINE StatusInt DimensionStyle::GetTextStyleProp (DgnTextStylePtr& textStyle, DimStyleProp iProperty) const {return _GetTextStyleProp(textStyle, iProperty);}

DG_INLINE UInt32 DimensionTextPartId::GetPartSegment () const {return m_partSegment;}
DG_INLINE DimensionTextPartType DimensionTextPartId::GetPartType () const {return m_partType;}
DG_INLINE DimensionTextPartSubType DimensionTextPartId::GetPartSubType () const {return m_partSubType;}
#endif

DG_INLINE MaterialCP    IMaterialPropertiesExtension::FindMaterialAttachment (ElementHandleCR eh) const { return _FindMaterialAttachment (eh); }
DG_INLINE BentleyStatus IMaterialPropertiesExtension::AddMaterialAttachment (EditElementHandleR eeh, DgnMaterialId id) {return _AddMaterialAttachment (eeh, id);}

DG_INLINE bool          IMaterialPropertiesExtension::SupportsSubEntityAttachments (ElementHandleCR eh) const {return _SupportsSubEntityAttachments (eh);}
DG_INLINE bool          IMaterialPropertiesExtension::HasSubEntityAttachments (ElementHandleCR eh) const {return _HasSubEntityAttachments (eh);}
DG_INLINE BentleyStatus IMaterialPropertiesExtension::GetSubEntityAttachments (ElementHandleCR eh, MaterialSubEntityAttachments& attachments) const {return _GetSubEntityAttachments (eh, attachments);}

DG_INLINE ITextPartIdPtr ITextPartId::Create () { return new ITextPartId (); }
DG_INLINE ITextQueryOptions::ITextQueryOptions () : m_includeEmptyParts (false), m_requireFieldSupport (false) { }
DG_INLINE bool ITextQueryOptions::ShouldIncludeEmptyParts () const { return m_includeEmptyParts; }
DG_INLINE void ITextQueryOptions::SetShouldIncludeEmptyParts (bool value) { m_includeEmptyParts = value; }
DG_INLINE bool ITextQueryOptions::ShouldRequireFieldSupport () const { return m_requireFieldSupport; }
DG_INLINE void ITextQueryOptions::SetShouldRequireFieldSupport (bool value) { m_requireFieldSupport = value; }
DG_INLINE ITextQueryOptionsPtr ITextQueryOptions::CreateDefault () { return new ITextQueryOptions (); }
DG_INLINE bool ITextQuery::_IsTextElement (ElementHandleCR) const { return false; }
DG_INLINE bool ITextQuery::_DoesSupportFields (ElementHandleCR) const { return false; }
DG_INLINE bool ITextQuery::IsTextElement (ElementHandleCR eh) const { return _IsTextElement (eh); }
DG_INLINE ITextPartIdPtr ITextQuery::GetTextPartId (ElementHandleCR eh, HitPathCR hitPath) const { return _GetTextPartId (eh, hitPath); }
DG_INLINE void ITextQuery::GetTextPartIds (ElementHandleCR eh, ITextQueryOptionsCR options, T_ITextPartIdPtrVectorR ids) const { _GetTextPartIds (eh, options, ids); }
DG_INLINE TextBlockPtr ITextQuery::GetTextPart (ElementHandleCR eh, ITextPartIdCR id) const { return _GetTextPart (eh, id); }
DG_INLINE ITextEdit::ReplaceStatus ITextEdit::ReplaceTextPart (EditElementHandleR eeh, ITextPartIdCR id, TextBlockCR textBlock) { return _ReplaceTextPart (eeh, id, textBlock); }

DG_INLINE WString   IEcPropertyHandler::GetEcPropertiesClassName (ElementHandleCR eh) {return _GetEcPropertiesClassName (eh);}
DG_INLINE StatusInt IEcPropertyHandler::GetEcProperties (T_EcCategories& t, ElementHandleCR eh) {return _GetEcProperties (t, eh);}
DG_INLINE bool      IEcPropertyHandler::IsPropertyReadOnly (ElementHandleCR eh, UInt32 propId, size_t arrayIndex) {return _IsPropertyReadOnly (eh, propId, arrayIndex);}
DG_INLINE bool      IEcPropertyHandler::IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName) {return _IsNullProperty (enabler, className, propName);}
DG_INLINE IsNullReturnType IEcPropertyHandler::IsNullProperty (ElementHandleCR eh, UInt32 propId, size_t arrayIndex) {return _IsNullProperty (eh, propId, arrayIndex);}

#if defined (NEEDS_WORK_DGNITEM)
DG_INLINE RasterFrameElm const* RasterFrameHandler::GetFrameElmCP(ElementHandleCR eh) const
    {
    BeAssert(eh.IsValid() && eh.GetLegacyType() == RASTER_FRAME_ELM);
    return reinterpret_cast<RasterFrameElm const*>(eh.GetElementCP());
    }
DG_INLINE RasterFrameElm* RasterFrameHandler::GetFrameElmP(EditElementHandleR eeh)
    {
    BeAssert(eeh.IsValid() && eeh.GetLegacyType() == RASTER_FRAME_ELM);
    return reinterpret_cast<RasterFrameElm*>(eeh.GetElementP());
    }
#endif

DG_INLINE BentleyStatus IRasterSourceFileQuery::InitFrom(ElementHandleCR eh) { return _InitFrom(eh); }
DG_INLINE UInt64 IRasterSourceFileQuery::GetBitmapHeight () const { return _GetBitmapHeight(); }
DG_INLINE UInt64 IRasterSourceFileQuery::GetBitmapWidth () const { return _GetBitmapWidth(); }
DG_INLINE BentleyStatus IRasterSourceFileQuery::ReadToRGBA (byte *RgbaBuffer, size_t maxBufferLength, bool useBgIfNoAlpha) const { return _ReadToRGBA(RgbaBuffer,maxBufferLength,useBgIfNoAlpha); }

/*--------------------------------------------------------------------------------------+
| ANDROID_NONPORT_WIP 
|
| The functions below are inlined in DgnHandlersDLLInlines.h in Vancouver.  Something about
| this class (and only this class) or its method of inlining caused problems for 
| optimized Android builds.  For now, we've taken the quick fix of never inlining these
| methods.
+---------------+---------------+---------------+---------------+---------------+------*/
//DG_INLINE DropGeometryR         DropToElementDrawGeom::GetDropGeometry () {return m_geometry;}
//DG_INLINE DropGraphicsR         DropToElementDrawGeom::GetDropGraphics () {return m_graphics;}
//DG_INLINE bool                  DropToElementDrawGeom::_DoTextGeometry () const {return (0 != (m_geometry.GetOptions() & DropGeometry::OPTION_Text)) || m_inSymbolDraw;}
//DG_INLINE bool                  DropToElementDrawGeom::_DoSymbolGeometry () const {return true;} // Shouldn't see pattern/linestyle symbols unless option is set to drop them...
//DG_INLINE bool                  DropToElementDrawGeom::_ProcessAsBody (bool isCurved) const {return m_geometry.GetSolidsOptions() != DropGeometry::SOLID_Wireframe;}
//DG_INLINE bool                  DropToElementDrawGeom::_DoClipping () const {return true;}
//DG_INLINE void                  DropToElementDrawGeom::SetElementNonSnappable (bool nonsnappable) {m_nonsnappable = nonsnappable;}
//DG_INLINE void                  DropToElementDrawGeom::SetPreserveClosedCurves (bool preserveClosed) {m_preserveClosedCurves = preserveClosed;}
//DG_INLINE void                  DropToElementDrawGeom::OnElementCreated (EditElementHandleR eeh) {_OnElementCreated (eeh);}
//DG_INLINE void                  DropToElementDrawGeom::ProcessAreaPattern (ElementHandleCR thisElm, DgnPlatform::ViewContext::ClipStencil& boundary, DgnPlatform::ViewContext::PatternParamSource& source) {_ProcessAreaPattern (thisElm, boundary, source);}
