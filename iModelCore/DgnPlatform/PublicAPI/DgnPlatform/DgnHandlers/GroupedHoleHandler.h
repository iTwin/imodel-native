/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/GroupedHoleHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/ElementGeometry.h>
#include "CellHeaderHandler.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup RegionElements
/// @beginGroup

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* A grouped hole is a sub-type of cell. A grouped hole cell must have a first
* child that is a closed curve with solid area type followed by at least one
* closed curve with hole area type. Displays fill using parity rules.
* @bsiclass                                                     Sam.Wilson      10/2004
+===============+===============+===============+===============+===============+======*/
struct          GroupedHoleHandler : Type2Handler,
                                     ICurvePathEdit,
                                     IAreaFillPropertiesEdit
                                     //__PUBLISH_SECTION_END__
                                     ,ISubTypeHandlerQuery
                                     //__PUBLISH_SECTION_START__
{
    DEFINE_T_SUPER(Type2Handler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (GroupedHoleHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void             _GetTypeName (WStringR string, UInt32 desiredLength) override;
virtual void                                _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override {GetTypeName (descr, desiredLength);};
DGNPLATFORM_EXPORT virtual ReprojectStatus  _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;

// DisplayHandler
virtual bool                                _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void             _GetElemDisplayParams (ElementHandleCR, ElemDisplayParams&, bool wantMaterials = false) override;
DGNPLATFORM_EXPORT virtual bool             _IsVisible (ElementHandleCR, ViewContextR, bool testRange, bool testLevel, bool testClass) override;
DGNPLATFORM_EXPORT virtual void             _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual StatusInt        _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus    _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus    _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

// IAreaFillPropertiesEdit
virtual bool                                _GetAreaType (ElementHandleCR eh, bool* isHoleP) const override {if (isHoleP) *isHoleP = false; return true;} // Always solid...
virtual bool                                _SetAreaType (EditElementHandleR eeh, bool isHole) override {return false;}

DGNPLATFORM_EXPORT virtual bool             _GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const override;
DGNPLATFORM_EXPORT virtual bool             _GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const override;
DGNPLATFORM_EXPORT virtual bool             _RemoveAreaFill (EditElementHandleR eeh) override;
DGNPLATFORM_EXPORT virtual bool             _AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP) override;
DGNPLATFORM_EXPORT virtual bool             _AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb) override;

// ISubTypeHandlerQuery
virtual bool _ClaimElement (ElementHandleCR) override; // new in Graphite

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Check the supplied element to see if it represents a grouped hole.
* @param[in] eh The element to check.
* @return true if element is a grouped hole.
* @see IPlanarRegionQuery::HasHoles
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static bool          IsGroupedHole (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Check the supplied element to determine if it is an acceptable type for inclusion
* in a grouped hole, either as a solid or hole loop. Valid components are closed 
* curves of the following types:
* \li SHAPE_ELM
* \li CMPLX_SHAPE_ELM
* \li ELLIPSE_ELM
* \li BSPLINE_CURVE_ELM
* <p>
* @param[in] eh The element to check.
* @return true if element is valid candidate.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static bool          IsValidGroupedHoleComponentType (ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Create a new grouped hole cell from the supplied solid element and collection of
* hole elements. Only closed curves are acceptable for the solid and hole loops.
* @param[out] eeh       The new element.
* @param[in]  solidEeh  Element to use for solid loop.
* @param[in]  holes     Elements to use for hole loops, requires a minimum of one.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @note solidEeh and hole agenda entries will be invalid after this call unless they 
* represent persistent elements.
* @see IsValidGroupedHoleComponentType
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateGroupedHoleElement (EditElementHandleR eeh, EditElementHandleR solidEeh, ElementAgendaR holes);

}; // GroupedHoleHandler

#endif

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
