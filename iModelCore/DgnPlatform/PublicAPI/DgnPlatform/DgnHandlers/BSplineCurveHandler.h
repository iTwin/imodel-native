/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/BSplineCurveHandler.h $
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
//__PUBLISH_SECTION_END__
/*=================================================================================**//**
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          BSplinePoleHandler : DisplayHandler
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (BSplinePoleHandler, DGNPLATFORM_EXPORT)
protected:

// Handler
DGNPLATFORM_EXPORT virtual void                _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual void                _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;

}; // BSplinePoleHandler

//__PUBLISH_SECTION_START__
/*=================================================================================**//**
* The default type handler for the BSPLINE_CURVE_ELM type that corresponds to the
* Bspline_curve structure.
* @bsiclass                                                    EarlinLutz   06/06
+===============+===============+===============+===============+===============+======*/
struct          BSplineCurveHandler : ComplexHeaderDisplayHandler,
                                      ICurvePathEdit,
                                      IAreaFillPropertiesEdit
{
    DEFINE_T_SUPER(ComplexHeaderDisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (BSplineCurveHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void                _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void                _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

DGNPLATFORM_EXPORT virtual bool                _IsRenderable (ElementHandleCR elHandle) override;
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _ValidateElementRange (EditElementHandleR elHandle) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus       _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

// IAreaFillPropertiesEdit
DGNPLATFORM_EXPORT virtual bool                _GetAreaType (ElementHandleCR eh, bool* isHoleP) const override;
DGNPLATFORM_EXPORT virtual bool                _GetSolidFill (ElementHandleCR eh, UInt32* fillColorP, bool* alwaysFilledP) const override;
DGNPLATFORM_EXPORT virtual bool                _GetGradientFill (ElementHandleCR eh, GradientSymbPtr& symb) const override;
DGNPLATFORM_EXPORT virtual bool                _GetPattern (ElementHandleCR eh, PatternParamsPtr& params, bvector<DwgHatchDefLine>* hatchDefLinesP, DPoint3dP originP, int index) const override;

DGNPLATFORM_EXPORT virtual bool                _SetAreaType (EditElementHandleR eeh, bool isHole) override;
DGNPLATFORM_EXPORT virtual bool                _RemoveAreaFill (EditElementHandleR eeh) override;
DGNPLATFORM_EXPORT virtual bool                _RemovePattern (EditElementHandleR eeh, int index) override;
DGNPLATFORM_EXPORT virtual bool                _AddSolidFill (EditElementHandleR eeh, UInt32* fillColorP, bool* alwaysFilledP) override;
DGNPLATFORM_EXPORT virtual bool                _AddGradientFill (EditElementHandleR eeh, GradientSymbCR symb) override;
DGNPLATFORM_EXPORT virtual bool                _AddPattern (EditElementHandleR eeh, PatternParamsR params, DwgHatchDefLineP hatchDefLinesP, int index) override;


public:

/*---------------------------------------------------------------------------------**//**
* Initialize the MSBSplineCurve structure from the supplied element.
* @param[out] curve         MSBsplineCurve representation.
* @param[in]  eh            The element to extract from.
* @return true if eh is a valid element that defines a single curve path.
* @see ICurvePathQuery
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CurveFromElement (MSBsplineCurveR curve, ElementHandleCR eh);

/*---------------------------------------------------------------------------------**//**
* Initialize the MSInterpolationCurve structure from the supplied BSPLINE_CURVE_ELM.
* @param[out] fitCurve      MSInterpolationCurve representation.
* @param[in]  eh            The element to extract from.
* @return true if eh is a valid interpolation curve of type BSPLINE_CONSTRUCTION_INTERPOLATION.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus InterpolationCurveFromElement (MSInterpolationCurveR fitCurve, ElementHandleCR eh);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Quick verification that the input MSBSplineCurve has all the required information.
* @param[in]  curve         The curve to check.
* @return true if input curve appears valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BSplineStatus IsValidCurve (MSBsplineCurveCR curve);

/*---------------------------------------------------------------------------------**//**
* Create a new pole based BSPLINE_CURVE_ELM element from the supplied MSBsplineCurve.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  curve         The MSBsplineCurve parameters.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BSplineStatus CreateBSplineCurveElement (EditElementHandleR eeh, ElementHandleCP templateEh, MSBsplineCurveCR curve, bool is3d, DgnModelR modelRef);

/*---------------------------------------------------------------------------------**//**
* Quick verification that the input MSInterpolationCurve has all the required information.
* @param[in]  curve         The curve to check.
* @return true if input curve appears valid.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BSplineStatus IsValidInterpolationCurve (MSInterpolationCurveCR curve);

/*---------------------------------------------------------------------------------**//**
* Create a new interpolation BSPLINE_CURVE_ELM element from the supplied MSInterpolationCurve.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  curve         The MSInterpolationCurve parameters.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BSplineStatus CreateBSplineCurveElement (EditElementHandleR eeh, ElementHandleCP templateEh, MSInterpolationCurveCR curve, bool is3d, DgnModelR modelRef);

}; // BSplineCurveHandler
#endif

/** @endGroup */

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
