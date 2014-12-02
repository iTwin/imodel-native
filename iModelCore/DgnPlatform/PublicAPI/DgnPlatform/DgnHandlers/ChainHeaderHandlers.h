/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ChainHeaderHandlers.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>
#include "IAreaFillProperties.h"
#include "IManipulator.h"
#include "ComplexHeaderHandler.h"

//__PUBLISH_SECTION_END__
#include "IGeoCoordReproject.h"
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


/// @addtogroup CurveElements
/// @beginGroup
#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* Base class with behavior common to complex strings and shapes. A complex chain
* collects a series of ordered and oriented open curve types into a single path or region.
* @note ChainHeaderHandler is never the element handler for any element.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          ChainHeaderHandler : ComplexHeaderDisplayHandler,
                                     ICurvePathEdit
{
    DEFINE_T_SUPER(ComplexHeaderDisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ChainHeaderHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void                _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual ScanTestResult      _DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual SnapStatus          _OnSnap (SnapContextP, int snapPathIndex) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _ValidateElementRange (EditElementHandleR elHandle) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus       _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new CMPLX_STRING_ELM or CMPLX_SHAPE_ELM header.
* After creating the chain header the application should use AddComponentElement to add
* open curve elements to the chain, and AddComponentComplete once all components
* have been added to finish the chain creation.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  isClosed      true for a CMPLX_SHAPE_ELM and false for a CMPLX_STRING_ELM.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Will be returned from eeh.GetDgnModel ()
*                           and later used by AddComponentComplete to update the chain's range.
* @note Chain components can only be open curves and only non-extended types, this is enforced
*       by AddComponentElement.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static void  CreateChainHeaderElement (EditElementHandleR eeh, ElementHandleCP templateEh, bool isClosed, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Check the supplied element to determine if it is an acceptable type for inclusion
* in a complex string or shape. Valid components are open curves of the following types:
* \li LINE_ELM
* \li LINE_STRING_ELM
* \li CURVE_ELM
* \li ARC_ELM
* \li BSPLINE_CURVE_ELM
* <p>
* @param[in] eh The element to check.
* @return true if element is valid candidate.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static bool  IsValidChainComponentType (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Add another element as a component of the chain. Only open curves will be accepted.
* The components are expected to be properly oriented, head to tail. A complex chain will
* stroke across gaps during display.
* @param[out] eeh           The chain to add the component to.
* @param[in]  componentEeh  The chain element to add.
* @return SUCCESS if component is suitable for a chain component and was successfully added.
* @note componentEeh will be invalid after this call unless it represents a persistent element.
* @see IsValidChainComponentType
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus AddComponentElement (EditElementHandleR eeh, EditElementHandleR componentEeh);

/*---------------------------------------------------------------------------------**//**
* Update the chains's range once all component elements have been added.
* @param[out] eeh       The chain element.
* @return SUCCESS if the chain has components and it's range was sucessfully updated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus AddComponentComplete (EditElementHandleR eeh);

}; // ChainHeaderHandler

/*=================================================================================**//**
* The default type handler for the CMPLX_STRING_ELM type. Complex strings are open
* curves (paths). The element data is stored using the Complex_string structure.
* The same data structure is used in both 2d and 3d. A complex string need not be
* planar.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          ComplexStringHandler : ChainHeaderHandler
{
    DEFINE_T_SUPER(ChainHeaderHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ComplexStringHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void     _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual bool     _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void     _Draw (ElementHandleCR, ViewContextR) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // ComplexStringHandler

/*=================================================================================**//**
* The default type handler for the CMPLX_SHAPE_ELM type. Complex shapes are closed
* curves (paths). The element data is stored using the Complex_string structure.
* The same data structure is used in both 2d and 3d. The creation of non-planar complex
* shapes is not disallowed but it is discouraged, complex shapes are intended to be
* planar.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          ComplexShapeHandler : ChainHeaderHandler,
                                      IAreaFillPropertiesEdit
{
    DEFINE_T_SUPER(ChainHeaderHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ComplexShapeHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void     _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual bool     _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
virtual bool                        _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void     _Draw (ElementHandleCR, ViewContextR) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // ComplexShapeHandler

//=======================================================================================
// @bsiclass                                                    MattGooding     11/13
//=======================================================================================
struct CivilComplexStringHandler : ComplexStringHandler // new in graphite
{
    DEFINE_T_SUPER(ComplexStringHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (CivilComplexStringHandler, DGNPLATFORM_EXPORT)

//__PUBLISH_CLASS_VIRTUAL__

    DGNPLATFORM_EXPORT static ElementHandlerId GetElemHandlerId();

}; // CivilComplexStringHandler

//=======================================================================================
// @bsiclass                                                    MattGooding     11/13
//=======================================================================================
struct CivilComplexShapeHandler : ComplexShapeHandler // new in graphite
{
    DEFINE_T_SUPER(ComplexShapeHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (CivilComplexShapeHandler, DGNPLATFORM_EXPORT)

//__PUBLISH_CLASS_VIRTUAL__

    DGNPLATFORM_EXPORT static ElementHandlerId GetElemHandlerId();

}; // CivilComplexShapeHandler
#endif

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
