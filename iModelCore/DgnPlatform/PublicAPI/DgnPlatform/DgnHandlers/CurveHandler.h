/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/CurveHandler.h $
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
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (NEEDS_WORK_DGNITEM)
/*=================================================================================**//**
* @addtogroup CurveElements
* Curve elements are either open or closed elements that can describe a path.
* The following standard types always represent a path:
* \li LINE_ELM
* \li LINE_STRING_ELM
* \li SHAPE_ELM
* \li CURVE_ELM
* \li ARC_ELM
* \li ELLIPSE_ELM
* \li BSPLINE_CURVE_ELM
* \li CMPLX_STRING_ELM
* \li CMPLX_SHAPE_ELM
* <p>
* These types support the ICurvePathQuery interface and can return linear properties
* such as length and answer other curve type queries. An EXTENDED_ELM can also represent
* a path by supporting the ICurvePathQuery interface. For this reason an application that
* wants to support any element that can describe a path should be written to dynamic_cast
* the element handler to the ICurvePathQuery interface instead of switching on element type.
* @bsiclass                                                     Brien.Bastings  04/2009
+===============+===============+===============+===============+===============+======*/

/// @addtogroup CurveElements
/// @beginGroup

/*=================================================================================**//**
* The default type handler for the CURVE_ELM type. The element data is stored using
* the Line_String_3d and Line_String_2d structures. The points are displayed using
* akima spline interpolation; this type is usually referred to as an akima curve.
* @bsiclass                                                     EarlinLutz  06/08
+===============+===============+===============+===============+===============+======*/
struct          CurveHandler : DisplayHandler,
                               ICurvePathEdit
{
    DEFINE_T_SUPER(DisplayHandler)    
    ELEMENTHANDLER_DECLARE_MEMBERS (CurveHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void                _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus       _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

public:

DGNPLATFORM_EXPORT static void               ResetHiddenPoints (EditElementHandleR eeh);
DGNPLATFORM_EXPORT static AkimaCurveStatus   SegmentAkimaCurveByPointCount (EditElementHandleR eeh, ElementHandleCR eh, int* endIndex, int startIndex, int maxPoints, double tolerance);
DGNPLATFORM_EXPORT static AkimaCurveStatus   SegmentAkimaCurve (EditElementHandleR eeh, ElementHandleCR eh, TransformCP, double param0, double param1, double tolerance);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Initialize the default start tangent points from the array of curve points.
* @param[out] tangentPts    tangent points, supply as index 0 and 1 to CreateCurveElement.
* @param[in]  points        curve points.
* @param[in]  numPoints     number of curve points (does not include 4 points for start and end tangents).
* @return SUCCESS if tangent points could be calculated from input points.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus GetStartTangentPoints (DPoint3d tangentPts[2], DPoint3d points[], size_t numPoints);

/*---------------------------------------------------------------------------------**//**
* Initialize the default end tangent points from the array of curve points.
* @param[out] tangentPts    tangent points, supply as index n-1 and n-2 to CreateCurveElement.
* @param[in]  points        curve points.
* @param[in]  numPoints     number of curve points (does not include 4 points for start and end tangents).
* @return SUCCESS if tangent points could be calculated from input points.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus GetEndTangentPoints (DPoint3d tangentPts[2], DPoint3d points[], size_t numPoints);

/*---------------------------------------------------------------------------------**//**
* Create a new CURVE_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  points        curve points. Must be 6 &lt;=numVerts &lt;=MAX_VERTICES. The
*                           first 2 points and last 2 points control the end tangents.
* @param[in]  numVerts      number of curve points.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateCurveElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCP points, size_t numVerts, bool is3d, DgnModelR modelRef);

}; // CurveHandler

#endif
/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
