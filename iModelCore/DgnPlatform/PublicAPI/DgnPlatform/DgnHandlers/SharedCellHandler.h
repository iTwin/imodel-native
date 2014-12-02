/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/SharedCellHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @addtogroup SharedCells
* Shared Cells are an efficient way to store repeated displayable geometry. A shared
* cell instance is a light-weight non-complex displayable element. Multiple instances 
* can all refer to a common shared definition that holds the complete set of geometry.
* A shared cell instance displays its definition through the instance's transform that
* can apply translation, rotation, and scaling. Each instance can also override some
* element properties like color and level so that these properties are taken from the 
* instance instead of using the properties of the elements in the definition.
* @bsiclass
+===============+===============+===============+===============+===============+======*/

/// @addtogroup SharedCells
/// @beginGroup

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* The default type handler for the SHARED_CELL_ELM type that corresponds to the 
* SharedCell structure. A shared cell instance will either find its definition by
* name in the case of a named shared cell definition, or by an explicit dependency,
* as in the case of an anonymous shared cell definition.
* @remarks Applications can use a ChildElemIter to iterate the children of the shared cell 
* definition for a given shared cell instances by requesting the inclusion 
* of \em shared children.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          SharedCellHandler : DisplayHandler, 
                                    ISharedCellQuery,
                                    IAnnotationHandler
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (SharedCellHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
friend struct SharedCellHandlerPathEntryExtension;
protected:

DGNPLATFORM_EXPORT void                 PushSC (ElementHandleCR scell, ElementHandleCR scDef, DgnModelP modelRef, ViewContextR context, bool pushTransform);
DGNPLATFORM_EXPORT bool                 PopulateElemHeaderOverrides (ElemHeaderOverridesR, SharedCell const*);
DGNPLATFORM_EXPORT void                 PickOrigin (ElementHandleCR, ViewContextR);
DGNPLATFORM_EXPORT bool                 IsVisibleLevels (ElementHandleCR, ViewContextR);
                   BentleyStatus        DropOneLevel (ElementAgendaR agenda, ElementHandleCR eh);
                   BentleyStatus        DropOneLevelToNormalCell (EditElementHandleR cellEeh, ElementHandleCR eh);
                   void                 DropNestedToNormalCell (EditElementHandleR cellEeh);

// Handler
DGNPLATFORM_EXPORT virtual void                _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void                _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual bool                _IsTransformGraphics (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual void                _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

DGNPLATFORM_EXPORT virtual ScanTestResult      _DoScannerTests (ElementHandleCR eh, BitMaskCP, UInt32 const*, ViewContextP) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void                _DrawFiltered (ElementHandleCR el, ViewContextR context, DPoint3dCP pts, double size) override;
DGNPLATFORM_EXPORT virtual bool                _IsVisible (ElementHandleCR, ViewContextR, bool testRange, bool testLevel, bool testClass) override;
DGNPLATFORM_EXPORT virtual SnapStatus          _OnSnap (SnapContextP, int snapPathIndex) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _ValidateElementRange (EditElementHandleR elHandle) override;
DGNPLATFORM_EXPORT virtual void                _GetTransformOrigin (ElementHandleCR, DPoint3dR) override;
virtual void                                   _GetSnapOrigin (ElementHandleCR el, DPoint3dR origin) override {_GetTransformOrigin(el, origin);}
DGNPLATFORM_EXPORT virtual void                _GetOrientation (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual bool                _IsPlanar (ElementHandleCR, DVec3dP normal, DPoint3dP point, DVec3dCP inputDefaultNormal) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// IAnnotationHandler
virtual IAnnotationHandlerP                    _GetIAnnotationHandler (ElementHandleCR)  override {return this;}
DGNPLATFORM_EXPORT virtual bool                _GetAnnotationScale (double* annotationScale, ElementHandleCR element) const override;
DGNPLATFORM_EXPORT virtual StatusInt           _ComputeAnnotationScaledRange (ElementHandleCR, DRange3dR elemRangeOut, double scaleFactor) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Set the shared cell override values and flags that will be used when visiting
* the components of the shared cell definition when creating a new element.
* @param[out]  eeh          The element to modify.
* @param[in]   overrides    The new shared cell overrides.
* @return SUCCESS if eeh is a SHARED_CELL_ELM and element was updated.
* @note Could affect shared cell's range. If creating a new instance or modifying an existing
*       instance caller is expected to call CreateSharedCellComplete or ValidateElementRange after
*       making all changes and before adding or replacing element in model.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetSharedCellOverrides (EditElementHandleR eeh, SCOverride* overrides);

/*---------------------------------------------------------------------------------**//**
* Set the given shared cell element's point cell status when creating a new element.
* @param[out] eeh           The element to modify.
* @param[in]  isPointCell   New point cell status.
* @return SUCCESS if eeh is a SHARED_CELL_ELM and element was updated.
* @note Could affect shared cell's range. If creating a new instance or modifying an existing
*       instance caller is expected to call CreateSharedCellComplete or ValidateElementRange after
*       making all changes and before adding or replacing element in model.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetPointCell (EditElementHandleR eeh, bool isPointCell);

/*---------------------------------------------------------------------------------**//**
* Set the given shared cell element's name when creating a new element.
* @param[out] eeh           The element to modify.
* @param[in]  cellName      New cell name.
* @return SUCCESS if eeh is a SHARED_CELL_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetName (EditElementHandleR eeh, WCharCP cellName);

/*---------------------------------------------------------------------------------**//**
* Set the given shared cell element's description when creating a new element.
* @param[out] eeh           The element to modify.
* @param[in]  descr         New cell description.
* @return SUCCESS if eeh is a SHARED_CELL_ELM and element was updated.
* @note A description that is common to all shared cell intances should be stored once
*       on the shared cell definition, not on each instance.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetDescription (EditElementHandleR eeh, WCharCP descr);

/*---------------------------------------------------------------------------------**//**
* Set the shared cell definition id for the supplied shared cell instance when creating a new element.
* @param[out] eeh       The element to modify.
* @param[in]  elemID    ElementId of the shared cell defintion to associate this instance to.
* @return SUCCESS if eeh is a SHARED_CELL_ELM and element was updated.
* @note Named shared cells may have an explicit dependency to their definition, however
*       this is not required and normally reserved only for anonymous shared cells where
*       it is required.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  SetDefinitionID (EditElementHandleR eeh, ElementId elemID);

/*---------------------------------------------------------------------------------**//**
* Create a new SHARED_CELL_ELM with the supplied information.
* After creating the shared cell element and setting up all options the application 
* should call CreateSharedCellComplete in order to compute the range for this instance.
* It is expected/required that the shared cell definition already exist in the destination
* model for the range to be properly calculated.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  cellName      Name of the new shared cell element which may be NULL if creating an instance for an anonymous shared cell definition.
* @param[in]  origin        Point to use for origin of the new shared cell instance.
* @param[in]  rMatrix       Rotation to use for the new shared cell instance.
* @param[in]  scale         Scale to use for the new shared cell instance.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Will be returned from eeh.GetDgnModel ()
*                           and later used by AddChildComplete to find the shared cell definition for 
*                           updating the cell's range. 
* @note When creating an instance for an anonymous shared cell definition you must call SetDefinitionID 
*       to associate the newly created instance with an existing shared cell definition.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void CreateSharedCellElement (EditElementHandleR eeh, ElementHandleCP templateEh, WCharCP cellName, 
                                                        DPoint3dCP origin, RotMatrixCP rMatrix, DPoint3dCP scale, 
                                                        bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Update the shared cell's range from the instance's transform and shared cell definition.
* @param[out] eeh       The shared cell element.
* @return SUCCESS if the shared cell definition exists and the range was sucessfully updated. 
* @note Requires that the shared cell definition already exists in the destination model.
* @remarks This is merely a helper function that calls DisplayHandler::ValidateElementRange.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateSharedCellComplete (EditElementHandleR eeh);

//__PUBLISH_SECTION_END__

// Hack around CopyContext only being available in ForeignFormat.
BentleyStatus DropSharedCellInstance (ElementHandleCR eh, ElementAgendaR dropGeom);

//__PUBLISH_SECTION_START__
}; // SharedCellHandler

/*=================================================================================**//**
* The default type handler for the SHAREDCELL_DEF_ELM type that corresponds to the 
* SharedCellDef structure. A shared cell definition is part of the dictionary model
* and may be used by shared cell instances from any model cache. A shared cell
* definition can be anonymous or named. If named, the name is expected to represent
* unique geometry, ex. user defined cell. 
* @remarks A shared cell definition is a complex element whose children can be any 
* element type that can be part of a normal cell. A shared cell definition may not 
* contain other shared cell definitions but it may contain shared cell instances 
* for \em other shared cell definitions. 
* @remarks Applications can use a ChildElemIter to iterate the children of a shared 
* cell definition.
* @note Since a shared cell definition is a non-model element and not tied to any
* particular model each definition can be either 2d or 3d and need not match the
* dimension of the shared cell instances that refer to it. For the purpose of units
* the shared cell definition is considered to use the unit definition of the default
* model.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          SharedCellDefHandler : ComplexHeaderDisplayHandler,
                                       //ITransactionHandler, removed in Graphite
                                       ISharedCellQuery
{
    DEFINE_T_SUPER(ComplexHeaderDisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (SharedCellDefHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void                _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void                _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnChangeOfUnits (EditElementHandleR, DgnModelP source, DgnModelP dest) override;
DGNPLATFORM_EXPORT virtual bool                _ExposeChildren (ElementHandleCR, ExposeChildrenReason reason) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;
//DGNPLATFORM_EXPORT virtual StatusInt           _DrawCut (ElementHandleCR, ICutPlaneR, ViewContextR) override; removeed in Graphite
DGNPLATFORM_EXPORT virtual SnapStatus          _OnSnap (SnapContextP, int snapPathIndex) override;
virtual bool                                   _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}
DGNPLATFORM_EXPORT virtual void                _GetTransformOrigin (ElementHandleCR, DPoint3dR) override;
virtual void                                   _GetSnapOrigin (ElementHandleCR el, DPoint3dR origin) override {_GetTransformOrigin(el, origin);}
DGNPLATFORM_EXPORT virtual void                _GetOrientation (ElementHandleCR, RotMatrixR) override;
// DGNPLATFORM_EXPORT virtual CopyContextRemapKey _OnPreProcessDeepCopy (EditElementHandleR copyOfRoot, CopyContextRemapKey oldRootKey, ElementHandleCR dependent, ElementCopyContextR elementCopier, bool copyAlways) override;removed in Graphite
// DGNPLATFORM_EXPORT virtual StatusInt           _OnPreprocessCopy (EditElementHandleR symbolEH, ElementCopyContextP ccP) override;                                                                                            removed in Graphite
// virtual ITransactionHandlerP _GetITransactionHandler() override {return this;}                                                                                                                                               removed in Graphite
// DGNPLATFORM_EXPORT virtual void                _OnElementLoaded (ElementHandleCR) override;                                                                                                                                  removed in Graphite
// DGNPLATFORM_EXPORT virtual void                _OnDeleted (ElementHandleP element) override;                                                                                                                                 removed in Graphite
// DGNPLATFORM_EXPORT virtual PreActionStatus     _OnAdd (EditElementHandleR eh) override;                                                                                                                                      removed in Graphite
// DGNPLATFORM_EXPORT virtual void                _OnAdded (ElementHandleP element) override;                                                                                                                                   removed in Graphite
// DGNPLATFORM_EXPORT virtual void                _OnModified (ElementHandleP newElement, ElementHandleP oldElement, ChangeTrackAction action, bool* cantBeUndoneFlag) override;                                                removed in Graphite
// DGNPLATFORM_EXPORT virtual void                _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override;                          removed in Graphite

    void ClearEntryFromCache (PersistentElementRefR el);                // added in graphite
    virtual PreActionStatus _OnDelete (PersistentElementRefR el) override {ClearEntryFromCache(el); return PRE_ACTION_Ok;}// added in graphite
    virtual PreActionStatus _OnAdd (EditElementHandleR eh) override;    // added in graphite
    virtual void _OnAdded (PersistentElementRefR element) override; // added in graphite

    virtual void _OnTxnReverse_Add (PersistentElementRefR el, bool isUndo) override {ClearEntryFromCache(el);}
    virtual void _OnTxnReversed_Delete (PersistentElementRefR el, bool isUndo) override {ClearEntryFromCache(el);} // this is necessary because we cache misses

public:

DGNPLATFORM_EXPORT static BentleyStatus AdjustScale (EditElementHandleR, double scale);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Set the point cell status when creating a new shared cell definition.
* @param[out] eeh           The element to modify.
* @param[in]  isPointCell   New point cell status.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetPointCell (EditElementHandleR eeh, bool isPointCell);

/*---------------------------------------------------------------------------------**//**
* Set the annotation cell status when creating a new shared cell definition.
* @param[out] eeh   The element to modify.
* @param[in] isAnnotation  New annotation cell status.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetAnnotation (EditElementHandleR eeh, bool isAnnotation);

/*---------------------------------------------------------------------------------**//**
* Set the anonymouse cell status when creating a new shared cell definition.
* @param[out] eeh   The element to modify.
* @param[in] isAnonymous  false for named shared cell, true for anonymous.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetAnonymous (EditElementHandleR eeh, bool isAnonymous);

/*---------------------------------------------------------------------------------**//**
* Set the given shared cell definition element's name when creating a new element.
* @param[out] eeh           The element to modify.
* @param[in]  cellName      New shared cell definition name.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetName (EditElementHandleR eeh, WCharCP cellName);

/*---------------------------------------------------------------------------------**//**
* Set the given shared cell definition element's description when creating a new element.
* @param[out] eeh           The element to modify.
* @param[in]  descr         New shared cell definition description.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetDescription (EditElementHandleR eeh, WCharCP descr);

/*---------------------------------------------------------------------------------**//**
* Set the flag that controls the display behavior of Multiline offsets when displayed
* through a scaled shared cell instance when creating a new element.
* @param[out] eeh   The element to modify.
* @param[in]  scaleMultilines  Multiline offset scale flag.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetMlineScaleOption (EditElementHandleR eeh, bool scaleMultilines);

/*---------------------------------------------------------------------------------**//**
* Set the flag that controls the display behavior of dimensions when displayed
* through a scaled shared cell instance when creating a new element.
* @param[out] eeh   The element to modify.
* @param[in] nondefaultScaleForDims  Dimenion scale flag.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetDimScaleOption (EditElementHandleR eeh, bool nondefaultScaleForDims);

/*---------------------------------------------------------------------------------**//**
* Set the flag that controls the display behavior of dimensions in nested shared cells with
* regards to whether to use the instance rotation or parent rotation when creating a new element.
* @param[out] eeh   The element to modify.
* @param[in] rotateDimView  Dimension rotation flag.
* @return SUCCESS if eeh is a SHAREDCELL_DEF_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetDimRotationOption (EditElementHandleR eeh, bool rotateDimView);

/*---------------------------------------------------------------------------------**//**
* Create a new SHAREDCELL_DEF_ELM with the supplied information.
* After creating the shared cell definitionheader the application should use AddChildElement to add 
* child elements to the definition, and AddChildComplete once all children have been added 
* to finish the shared cell definition creation.
* @param[out] eeh           The new element.
* @param[in]  cellName      Name of the new shared cell definition element which may be NULL if creating an anonymous shared cell definition.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Will be returned from eeh.GetDgnModel ()
*                           and later used by AddChildComplete to updating the element's range. 
* @note To create an anonymous shared cell definition call SetAnonymous on the new element.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void          CreateSharedCellDefElement (EditElementHandleR eeh, WCharCP cellName, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Add another element as a child of the shared cell definition.
* @param[out] eeh       The shared cell definition to add the child to.
* @param[in]  childEeh  The child element to add.
* @return SUCCESS if child is suitable for a shared cell definition component and was successfully added.
* @note childEeh will be invalid after this call unless it represents a persistent element.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus AddChildElement (EditElementHandleR eeh, EditElementHandleR childEeh);

/*---------------------------------------------------------------------------------**//**
* Update the shared cell definitions range and component count once all child elements have been added.
* @param[out] eeh       The shared cell definition element.
* @return SUCCESS if the shared cell definition has children and it's range was sucessfully updated. 
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus AddChildComplete (EditElementHandleR eeh);

}; // SharedCellDefHandler
#endif

/// @endGroup


END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
