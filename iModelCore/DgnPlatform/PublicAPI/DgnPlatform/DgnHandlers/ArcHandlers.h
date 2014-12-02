/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ArcHandlers.h $
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
//__PUBLISH_SECTION_END__
#include "IGeoCoordReproject.h"
#include "DelegateMaps.h"
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/** @addtogroup CurveElements */
/** @beginGroup */

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* Base class with behavior common to ellipse and arc elements.
* @note EllipticArcBaseHandler is never the element handler for any element.
* @bsiclass                                                     EarlinLutz  05/06
+===============+===============+===============+===============+===============+======*/
struct          EllipticArcBaseHandler : DisplayHandler,
                                         ICurvePathEdit
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (EllipticArcBaseHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void             _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void             _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual bool             _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void             _GetTransformOrigin (ElementHandleCR, DPoint3dR) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus    _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus    _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // EllipticArcBaseHandler

/*=================================================================================**//**
* The default type handler for the ELLIPSE_ELM type that corresponds to the
* Ellipse_3d and Ellipse_2d structures.
* @bsiclass                                                     EarlinLutz  05/06
+===============+===============+===============+===============+===============+======*/
struct          EllipseHandler : EllipticArcBaseHandler,
                                 IAreaFillPropertiesEdit
{
    DEFINE_T_SUPER(EllipticArcBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (EllipseHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void             _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void             _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt        _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus  _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt        _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void             _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void             _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

// DisplayHandler
virtual bool                                _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void             _Draw (ElementHandleCR, ViewContextR) override;
virtual bool                                _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}
DGNPLATFORM_EXPORT virtual void             _GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation) override;

public:

/*---------------------------------------------------------------------------------**//**
* Extract the requested parameters from the supplied ELLIPSE_ELM.
* @param[out] sf_pt         start/end point of ellipse.
* @param[out] majorAxis     size of ellipse primary axis.
* @param[out] minorAxis     size of ellipse secondary axis.
* @param[out] trans         ellipse rotation.
* @param[out] rot2d         ellipse rotation angle (2d only).
* @param[out] center        ellipse center point.
* @param[in]  eh            The element to extract from.
* @return SUCCESS if eh is the correct type and the requested information is valid.
* @see IEllipticArcQuery
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  Extract (DPoint3dP sf_pt, double* majorAxis, double* minorAxis, RotMatrixP trans, double* rot2d, DPoint3dP center, ElementHandleCR eh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new ELLIPSE_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  center        ellipse center
* @param[in]  axis1         size of the primary ellipse axis
* @param[in]  axis2         size of the secondary ellipse axis
* @param[in]  rotation      ellipse rotation 
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreateEllipseElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR center, double axis1, double axis2, RotMatrixCR rotation, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Create a new ELLIPSE_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  center        ellipse center
* @param[in]  axis1         size of the primary ellipse axis
* @param[in]  axis2         size of the secondary ellipse axis
* @param[in]  rotation      ellipse rotation angle
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreateEllipseElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR center, double axis1, double axis2, double rotation, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Create a new ELLIPSE_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  ellipse       ellipse parameters
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreateEllipseElement (EditElementHandleR eeh, ElementHandleCP templateEh, DEllipse3dCR ellipse, bool is3d, DgnModelR modelRef);

}; // EllipseHandler

/*=================================================================================**//**
* The default type handler for the ARC_ELM type that corresponds to the
* Arc_3d and Arc_2d structures.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          ArcHandler : EllipticArcBaseHandler
{
    DEFINE_T_SUPER(EllipticArcBaseHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (ArcHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void             _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt        _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus  _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt        _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt        _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void             _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void             _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;


// DisplayHandler
DGNPLATFORM_EXPORT virtual void             _Draw (ElementHandleCR, ViewContextR) override;
virtual bool                                _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}
DGNPLATFORM_EXPORT virtual void             _GetOrientation (ElementHandleCR elHandle, RotMatrixR orientation) override;

public:

/*---------------------------------------------------------------------------------**//**
* Extract the requested parameters from the supplied ARC_ELM.
* @param[out] startPt       arc start point.
* @param[out] endPt         arc end point.
* @param[out] start         arc start angle in radians.
* @param[out] sweep         arc sweep angle in radians.
* @param[out] majorAxis     size of arc primary axis.
* @param[out] minorAxis     size of arc secondary axis.
* @param[out] trans         arc rotation.
* @param[out] rot2d         arc rotation angle (2d only).
* @param[out] center        arc center point.
* @param[in]  eh            The element to extract from.
* @return SUCCESS if eh is the correct type and the requested information is valid.
* @see IEllipticArcQuery
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  Extract (DPoint3dP startPt, DPoint3dP endPt, double* start, double* sweep, double* majorAxis, double* minorAxis, RotMatrixP trans, double* rot2d, DPoint3dP center, ElementHandleCR eh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new ARC_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  center        arc center
* @param[in]  axis1         size of the primary arc axis
* @param[in]  axis2         size of the secondary arc axis
* @param[in]  rotation      arc rotation 
* @param[in]  start         arc starting angle in radians
* @param[in]  sweep         arc sweep angle in radians
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreateArcElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR center, double axis1, double axis2, RotMatrixCR rotation, double start, double sweep, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Create a new ARC_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  center        arc center
* @param[in]  axis1         size of the primary arc axis
* @param[in]  axis2         size of the secondary arc axis
* @param[in]  rotation      arc rotation angle
* @param[in]  start         arc starting angle in radians
* @param[in]  sweep         arc sweep angle in radians
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreateArcElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCR center, double axis1, double axis2, double rotation, double start, double sweep, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Create a new ARC_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  ellipse       arc parameters
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreateArcElement (EditElementHandleR eeh, ElementHandleCP templateEh, DEllipse3dCR ellipse, bool is3d, DgnModelR modelRef);

}; // ArcHandler
#endif

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
