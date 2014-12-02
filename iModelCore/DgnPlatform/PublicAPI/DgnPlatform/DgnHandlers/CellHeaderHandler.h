/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/CellHeaderHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_END__

#include <DgnPlatform/DgnCore/PropertyContext.h>

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>
#include <DgnPlatform/DgnHandlers/IManipulator.h>
#include <DgnPlatform/DgnCore/IAnnotationHandler.h>
#include "ComplexHeaderHandler.h"

#ifdef WIP_ECENABLERS
//__PUBLISH_SECTION_END__
#include "DelegatedElementECEnabler.h"
//__PUBLISH_SECTION_START__
#endif

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*=================================================================================**//**
* @addtogroup Cells
* The CELL_HEADER_ELM type that corresponds to the Cell_3d and Cell_2d structures is 
* a general purpose complex header element whose children can be most other displayable
* element types.
* <p>
* Prior to the advent of element handlers CELL_HEADER_ELM was a convenient way for 
* many applications to represent their graphical application data. This leads to two 
* distinct types of cells, those that want to be treated as a single object and those that
* are just a collection of other "top-level" elements.
* <p>
* SmartSolids and Notes are examples of cells that do not have public children. All operations
* on these application defined types needs to go through the element handler as direct 
* modification of the child elements may produce an invalid element.
* <p>
* User defined cells and group cells (Edit->Group) are collections of other 
* element types and do have public child elements that may be of interest to
* applications.
* <p>
* Type2Handler is the base class for all cell sub-types. The Type2Handler class will never
* be the element handler for any element, it will always be a sub-class. The Type2Handler class 
* itself does not report having any public children. Application defined cells (like SmartSolids) 
* that want to be treated as a single object will be a direct sub-class of Type2Handler.
* <p>
* Whether a particular Type2Handler sub-class has public children is enforced through the use 
* of child element iterators. A ChildElemIter can be used to iterate over the public children of 
* a cell to extract information. A ChildEditElemIter can be used to iterate over the public children 
* of a cell to make modifications like adding or removing area fill. 
* @note Since cells can be nested inside other cells you will also need to ask the children at any
*       given depth if they also have public children.
* <p>
* \code
static bool queryCellChildren (ElementHandleCR eh)
    {
    if (NULL != dynamic_cast <"SomeInterface*"> (&eh.GetHandler ()))
        return true;

    for (ChildElemIter childEh (eh, false); childEh.IsValid (); childEh = childEh.ToNext ())
        if (queryCellChildren (childEh))
            return true;

    return false;
    }
* \endcode
* <p>
* NormalCellHeaderHandler is a sub-class of Type2Handler that is used for user defined cells and
* group cells. The NormalCellHeaderHandler class represents a public collection of other 
* elements and will return a child iterator for both query and edit.
* @bsiclass
+===============+===============+===============+===============+===============+======*/

/// @addtogroup Cells
/// @beginGroup

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* Base class for application defined cells that want to be treated as a single object.
* The element data is stored using the Cell_3d and Cell_2d structures. 
* @note Applications should leverage element Handlers, XAttributes, and XGraphics in 
*       lieu of creating a new cell type application element. Type2Handler is never the element 
*       handler for any element, it is simply a base class that applications can sub-class from.
* @bsiclass                                                     Brien.Bastings  03/07
+===============+===============+===============+===============+===============+======*/
struct          Type2Handler : ComplexHeaderDisplayHandler
                               //ITransactionHandler removed in Graphite
{
    DEFINE_T_SUPER(ComplexHeaderDisplayHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (Type2Handler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT  void                ValidateRangeDiagonal (EditElementHandleR elHandle);
DGNPLATFORM_EXPORT  void                TransformCellHeader (EditElementHandleR eeh, TransformCR trans);

// Handler
DGNPLATFORM_EXPORT virtual void                 _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void                 _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt            _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt            _OnFenceStretchFinish (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual void                 _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                 _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual ScanTestResult       _DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP) override;
// DGNPLATFORM_EXPORT virtual ITransactionHandlerP _GetITransactionHandler() override; removed in Graphite

// DisplayHandler
DGNPLATFORM_EXPORT virtual bool                _IsVisible (ElementHandleCR, ViewContextR, bool testRange, bool testLevel, bool testClass) override;
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void                _DrawFiltered (ElementHandleCR, ViewContextR, DPoint3dCP, double size) override;
DGNPLATFORM_EXPORT virtual void                _GetOrientation (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual void                _GetTransformOrigin (ElementHandleCR, DPoint3dR) override;
virtual void                                   _GetSnapOrigin (ElementHandleCR el, DPoint3dR origin) override {_GetTransformOrigin(el, origin);}
DGNPLATFORM_EXPORT virtual SnapStatus          _OnSnap (SnapContextP, int snapPathIndex) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _ValidateElementRange (EditElementHandleR elHandle) override;


// ComplexHeaderDisplayHandler
DGNPLATFORM_EXPORT virtual void                _QueryHeaderProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditHeaderProperties (EditElementHandleR eeh, PropertyContextR context) override;

public:

DGNPLATFORM_EXPORT BentleyStatus        SetRange (EditElementHandleR eeh);
DGNPLATFORM_EXPORT BentleyStatus        SetOriginAndRange (EditElementHandleR eeh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // Type2Handler

/*=================================================================================**//**
* Class for user defined cells and groups. The NormalCellHeaderHandler represents a
* public collection of other "top-level" elements.
* @see ChildElemIter, ChildEditElemIter, and ExposeChildrenReason.
* @note NormalCellHeaderHandler is the type handler for any CELL_HEADER_ELM element not 
*       claimed by any other element handler.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          NormalCellHeaderHandler : Type2Handler,
                                          ICellQuery
{
    DEFINE_T_SUPER(Type2Handler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (NormalCellHeaderHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

void                                           PickOrigin (ElementHandleCR, ViewContextR);
DGNPLATFORM_EXPORT virtual bool                _WantFastCell (DgnElementCP);

// Handler
DGNPLATFORM_EXPORT virtual bool                _IsTransformGraphics (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceStretchFinish (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual bool                _ExposeChildren (ElementHandleCR, ExposeChildrenReason reason) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnChildrenModified (EditElementHandleR el, ExposeChildrenReason reason) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;
//DGNPLATFORM_EXPORT virtual StatusInt           _DrawCut (ElementHandleCR el, ICutPlaneR, ViewContextR context) override;  removed in graphtie
DGNPLATFORM_EXPORT virtual void                _GetElemDisplayParams (ElementHandleCR, ElemDisplayParams&, bool wantMaterials = false) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _ValidateElementRange (EditElementHandleR elHandle) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

public:

DGNPLATFORM_EXPORT void VisitChildren (ElementHandleCR eh, ViewContextR context);

DGNPLATFORM_EXPORT static BentleyStatus  SetCellRange (EditElementHandleR eeh);
DGNPLATFORM_EXPORT static BentleyStatus  SetCellOriginAndRange (EditElementHandleR eeh);
DGNPLATFORM_EXPORT static BentleyStatus  AdjustScale (EditElementHandleR, double scale);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Set the given cell element's point cell status when creating a new element.
* @param[out] eeh           The cell element.
* @param[in]  isPointCell   New point cell status.
* @return SUCCESS if eeh is a CELL_HEADER_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetPointCell (EditElementHandleR eeh, bool isPointCell);

/*---------------------------------------------------------------------------------**//**
* Set the given cell element's name when creating a new element.
* @param[out] eeh           The cell element.
* @param[in]  cellName      New cell name.
* @return SUCCESS if eeh is a CELL_HEADER_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetName (EditElementHandleR eeh, WCharCP cellName);

/*---------------------------------------------------------------------------------**//**
* Set the given cell element's description when creating a new element.
* @param[out] eeh           The cell element.
* @param[in]  descr         New cell description.
* @return SUCCESS if eeh is a CELL_HEADER_ELM and element was updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus SetDescription (EditElementHandleR eeh, WCharCP descr);

/*---------------------------------------------------------------------------------**//**
* Create a new group cell (orphan cell) from a collection of elements.
* @param[out] eeh       The new element.
* @param[in]  agenda    The child elements.
* @param[in]  cellName  The cell name (optional)
* @note All child elements must be from the same model, the model to associate the new cell element with.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateGroupCellElement (EditElementHandleR eeh, ElementAgendaR agenda, WCharCP cellName = NULL);

/*---------------------------------------------------------------------------------**//**
* Create a new anonymous orphan CELL_HEADER_ELM with origin at zero and identity rotation.
* After creating the cell header the application should use AddChildElement to add 
* child elements to the cell, and AddChildComplete once all children have been added 
* to finish the cell creation.
* @param[out] eeh       The new element.
* @param[in]  cellName  Name of the new cell element which may be NULL.
* @param[in]  is3d      Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef  Model to associate this element with. Will be returned from eeh.GetDgnModel ()
*                       and later used by AddChildComplete to update the cell's range. 
* @note An orphan (or anonymous) cell is a cell without a name or whose name doesn't necessarily 
*       identify a unique set of geometry. A user defined cells are NOT orphan cells.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void CreateOrphanCellElement (EditElementHandleR eeh, WCharCP cellName, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Create a new named CELL_HEADER_ELM with supplied origin and rotation.
* After creating the cell header the application should use AddChildElement to add 
* child elements to the cell, and AddChildComplete once all children have been added 
* to finish the cell creation.
* @param[out] eeh       The new element.
* @param[in]  cellName  Name of the new cell element.
* @param[in]  origin    new cell's origin.
* @param[in]  rMatrix   new cell's rotation.
* @param[in]  is3d      Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef  Model to associate this element with. Will be returned from eeh.GetDgnModel ()
*                       and later used by AddChildComplete to update the cell's range. 
* @note Use this method to create cells where the cellName represents a unique set of geometry
*       such as a user defined cell. For application defined cells and groups orphan cells should
*       be created instead.
* @see CreateOrphanCellElement
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void CreateCellElement (EditElementHandleR eeh, WCharCP cellName, DPoint3dCR origin, RotMatrixCR rMatrix, bool is3d, DgnModelR modelRef);
                                            
/*---------------------------------------------------------------------------------**//**
* Add another element as a child of the cell.
* @param[out] eeh       The cell to add the child to.
* @param[in]  childEeh  The child element to add.
* @return SUCCESS if child is suitable for a cell component and was successfully added.
* @note childEeh will be invalid after this call unless it represents a persistent element.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus AddChildElement (EditElementHandleR eeh, EditElementHandleR childEeh);

/*---------------------------------------------------------------------------------**//**
* Update the cell's range, origin, range diagonal, and component count once all child
* elements have been added.
* @param[out] eeh       The cell element.
* @return SUCCESS if the cell has children and it's range was sucessfully updated. 
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus AddChildComplete (EditElementHandleR eeh);

}; // NormalCellHeaderHandler

//__PUBLISH_SECTION_END__

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  08/12
+===============+===============+===============+===============+===============+======*/
struct NormalCellHeaderHandlerPathEntryExtension : IDisplayHandlerPathEntryExtension
{
DGNPLATFORM_EXPORT virtual void _PushDisplayEffects (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual bool _GetElemHeaderOverrides (ElementHandleCR el, ElemHeaderOverridesR ovr) override;
};
/*=================================================================================**//**
* An Annotation Cell is-a a sub-type of cell.
* @bsiclass                                                     Sunand.Sandurkar 06/2006
+===============+===============+===============+===============+===============+======*/
class           AnnotationCellHeaderHandler : public NormalCellHeaderHandler,
                                              public IAnnotationHandler,
                                              public ISubTypeHandlerQuery
{
    DEFINE_T_SUPER(NormalCellHeaderHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (AnnotationCellHeaderHandler, DGNPLATFORM_EXPORT)

protected:

StatusInt ApplyAnnotationScaleTransform (EditElementHandleR eh, TransformInfoCR trans, bool& newScaleFlag, double& newScaleValue);

DGNPLATFORM_EXPORT virtual void        _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void        _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual bool        _IsTransformGraphics (ElementHandleCR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual void        _Draw (ElementHandleCR, ViewContextR) override;
//DGNPLATFORM_EXPORT virtual StatusInt   _OnPreprocessCopy (EditElementHandleR symbolEH, ElementCopyContextP ccP) override; remmoved in graphite

// IAnnotationhandler Interface methods
DGNPLATFORM_EXPORT virtual IAnnotationHandlerP _GetIAnnotationHandler (ElementHandleCR)  override {return this;}
DGNPLATFORM_EXPORT virtual bool        _GetAnnotationScale (double* annotationScale, ElementHandleCR element) const override;
DGNPLATFORM_EXPORT virtual StatusInt   _ComputeAnnotationScaledRange (ElementHandleCR, DRange3dR elemRangeOut, double scaleFactor) override;

public:
virtual bool _ClaimElement (ElementHandleCR el) override;

DGNPLATFORM_EXPORT static StatusInt    CreateFromNormalCell (EditElementHandleR cellEEH); 

}; // AnnotationCellHeaderHandler

//__PUBLISH_SECTION_START__

/** @endGroup */
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */

