/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/BSplineSurfaceHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>
#include "ComplexHeaderHandler.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/// @addtogroup 3DElements
/// @beginGroup
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* The default type handler for the BSPLINE_SURFACE_ELM type that corresponds to the 
* Bspline_surface structure. This is a 3d only element type, it has no 2d representation
* and can not be added to a 2d model.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE BSplineSurfaceHandler : ComplexHeaderDisplayHandler,
                                                        IBsplineSurfaceEdit
{
    DEFINE_T_SUPER(ComplexHeaderDisplayHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (BSplineSurfaceHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void        _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void        _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

// DisplayHandler
virtual bool                           _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void        _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual void        _GetOrientation (ElementHandleCR, RotMatrixR) override;
DGNPLATFORM_EXPORT virtual SnapStatus  _OnSnap (SnapContextP, int snapPathIndex) override;
DGNPLATFORM_EXPORT virtual StatusInt   _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// IBsplineSurfaceEdit
DGNPLATFORM_EXPORT virtual BentleyStatus   _SetBsplineSurface (EditElementHandleR eeh, MSBsplineSurfaceCR surface) override;
DGNPLATFORM_EXPORT virtual BentleyStatus   _GetBsplineSurface (ElementHandleCR source, MSBsplineSurfacePtr& surface) override;

public:

/*---------------------------------------------------------------------------------**//**
* Initialize the MSBsplineSurface structure from the supplied BSPLINE_SURFACE_ELM.
* @param[out] surface       MSBsplineSurface representation.
* @param[in]  eh            The element to extract from.
* @return true if eh is the correct type and the requested information is valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  SurfaceFromElement (MSBsplineSurfaceR surface, ElementHandleCR eh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Quick verification that the input MSBsplineSurface has all the required information.
* @param[in]  surface    The surface to check.
* @return true if input surface appears valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BSplineStatus  IsValidSurface (MSBsplineSurfaceCR surface);

/*---------------------------------------------------------------------------------**//**
* Create a BSPLINE_SURFACE_ELM element from the supplied MSBsplineSurface.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  surface       The MSBsplineSurface parameters.
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BSplineStatus  CreateBSplineSurfaceElement (EditElementHandleR eeh, ElementHandleCP templateEh, MSBsplineSurfaceCR surface, DgnModelR modelRef);

}; // BSplineSurfaceHandler
#endif

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
