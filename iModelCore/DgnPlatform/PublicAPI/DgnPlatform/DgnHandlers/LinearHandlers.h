/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/LinearHandlers.h $
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

/// @addtogroup DisplayHandler
/// @beginGroup
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* The default type handler for the POINT_STRING_ELM type. The element data is stored using
* the Line_String_3d and Line_String_2d structures.
* @bsiclass                                                     RichardTrefz    08/02
+===============+===============+===============+===============+===============+======*/
struct          PointStringHandler : DisplayHandler,
                                     ICurvePathEdit
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (PointStringHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void            _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt       _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt       _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt       _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void            _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void            _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual bool            _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void            _Draw (ElementHandleCR, ViewContextR) override;
virtual bool                               _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus   _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus   _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new POINT_STRING_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  points        input point buffer.
* @param[in]  matrices      optional array of per-point rotations (usually NULL).
* @param[in]  numVerts      number of points (and matrices if not NULL)
* @param[in]  disjoint      Whether point displays as disjoint or a continous linestring.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus  CreatePointStringElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCP points, RotMatrixCP matrices, size_t numVerts, bool disjoint, bool is3d, DgnModelR modelRef);

}; // PointStringHandler

/// @endGroup

/// @addtogroup CurveElements
/// @beginGroup

/*=================================================================================**//**
* The default type handler for the LINE_ELM type that corresponds to the
* Line_3d and Line_2d structures.
* @bsiclass                                                     EarlinLutz  05/06
+===============+===============+===============+===============+===============+======*/
struct          LineHandler : DisplayHandler,
                              ICurvePathEdit
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (LineHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT void                 GetStartEnd (ElementHandleCR, DPoint3dR point0, DPoint3dR point1, TransformCP transform=NULL);
DGNPLATFORM_EXPORT void                 SetStartEnd (EditElementHandleR, DPoint3dCR point0, DPoint3dCR point1);

// Handler
DGNPLATFORM_EXPORT virtual void                _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt           _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;
DGNPLATFORM_EXPORT virtual void                _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// DisplayHandler
DGNPLATFORM_EXPORT virtual void                _Draw (ElementHandleCR, ViewContextR) override;
virtual bool                                   _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}
DGNPLATFORM_EXPORT virtual void                _GetTransformOrigin(ElementHandleCR, DPoint3dR) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus       _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new LINE_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  segment       start and end points of line.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateLineElement (EditElementHandleR eeh, ElementHandleCP templateEh, DSegment3dCR segment, bool is3d, DgnModelR modelRef);

}; // LineHandler

/*=================================================================================**//**
* Base class with behavior common to linestring and shape elements.
* @note LineStringBaseHandler is never the element handler for any element.
* @bsiclass                                                     EarlinLutz  05/06
+===============+===============+===============+===============+===============+======*/
struct          LineStringBaseHandler : DisplayHandler,
                                        ICurvePathEdit
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (LineStringBaseHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
friend struct PointStringHandler;
friend struct BSplinePoleHandler;
friend struct CurveHandler;

protected:

DGNPLATFORM_EXPORT int                  GetPointCount (ElementHandleCR);
DGNPLATFORM_EXPORT StatusInt            GetPoint (ElementHandleCR, DPoint3dR point, int index, TransformCP pTransform = NULL);
DGNPLATFORM_EXPORT StatusInt            GetPoint (ElementHandleCR, DPoint3dR point, int index, double edgeFraction);
DGNPLATFORM_EXPORT StatusInt            GetPoints (ElementHandleCR, DPoint3dP pPointBuffer, int index, int count);
DGNPLATFORM_EXPORT StatusInt            SetPoint (EditElementHandleR, DPoint3dCR point, int index);
DGNPLATFORM_EXPORT StatusInt            EncodeStringFraction (ElementHandleCR, double&, int, double);
DGNPLATFORM_EXPORT StatusInt            DecodeStringFraction (ElementHandleCR, int&, double&, double);

DGNPLATFORM_EXPORT static StatusInt     TransformLineString (EditElementHandleR eeh, TransformInfoCR trans, bool canContainDisconnects);

// Handler
DGNPLATFORM_EXPORT virtual ReprojectStatus     _OnGeoCoordinateReprojection (EditElementHandleR, IGeoCoordinateReprojectionHelper&, bool inChain) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void                _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;
DGNPLATFORM_EXPORT virtual void                _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void                _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual bool                _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

// Handler
DGNPLATFORM_EXPORT virtual StatusInt           _OnDrop (ElementHandleCR eh, ElementAgendaR dropGeom, DropGeometryCR geometry) override;

// ICurvePathEdit
DGNPLATFORM_EXPORT virtual BentleyStatus       _GetCurveVector (ElementHandleCR eh, CurveVectorPtr& curves) override;
DGNPLATFORM_EXPORT virtual BentleyStatus       _SetCurveVector (EditElementHandleR eeh, CurveVectorCR path) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

}; // LineStringBaseHandler

/*=================================================================================**//**
* The default type handler for the LINE_STRING_ELM type. The element data is stored using
* the Line_String_3d and Line_String_2d structures.
* @bsiclass                                                     EarlinLutz  05/06
+===============+===============+===============+===============+===============+======*/
struct          LineStringHandler : LineStringBaseHandler
{
    DEFINE_T_SUPER(LineStringBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (LineStringHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void         _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;

// DisplayHandler

DGNPLATFORM_EXPORT virtual void         _Draw (ElementHandleCR, ViewContextR) override;
virtual bool                            _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new LINE_STRING_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  points        input point buffer.
* @param[in]  numVerts      number of points.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateLineStringElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCP points, size_t numVerts, bool is3d, DgnModelR modelRef);

}; // LineStringHandler

/*=================================================================================**//**
* The default type handler for the SHAPE_ELM type. The element data is stored using
* the Line_String_3d and Line_String_2d structures. The creation of non-planar
* shapes is not disallowed but it is discouraged, shapes are intended to be
* planar.
* @bsiclass                                                     KeithBentley    04/01
+===============+===============+===============+===============+===============+======*/
struct          ShapeHandler : LineStringBaseHandler,
                               IAreaFillPropertiesEdit
{
    DEFINE_T_SUPER(LineStringBaseHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ShapeHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void         _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnTransform (EditElementHandleR, TransformInfoCR) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnFenceStretch (EditElementHandleR, TransformInfoCR, FenceParamsP, FenceStretchFlags) override;
DGNPLATFORM_EXPORT virtual StatusInt    _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR, FenceParamsP, FenceClipFlags) override;

// DisplayHandler
virtual bool                            _IsRenderable (ElementHandleCR) override {return true;}
DGNPLATFORM_EXPORT virtual void         _Draw (ElementHandleCR, ViewContextR) override;
virtual bool                            _FilterLevelOfDetail (ElementHandleCR, ViewContextR) override {return false;}

public:

DGNPLATFORM_EXPORT static void          ExtractVertexData (ElementHandleCR eh, bvector<DVec3d>* normals, bvector<DPoint2d>* params, bvector<RgbColorDef>* colors);
DGNPLATFORM_EXPORT static void          AppendVertexData (EditElementHandleR eeh, DVec3dCP normals, DPoint2dCP params, RgbColorDefCP colors);
DGNPLATFORM_EXPORT static void          DeleteVertexData (EditElementHandleR eeh, bool normals, bool params, bool colors);

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:

/*---------------------------------------------------------------------------------**//**
* Create a new SHAPE_ELM with the supplied parameters.
* @param[out] eeh           The new element.
* @param[in]  templateEh    Template element to use for symbology; if NULL defaults are used.
* @param[in]  points        input point buffer.
* @param[in]  numVerts      number of points.
* @param[in]  is3d          Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
* @param[in]  modelRef      Model to associate this element with. Required to compute range.
* @return SUCCESS if a valid element is created and range was sucessfully calculated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DGNPLATFORM_EXPORT static BentleyStatus CreateShapeElement (EditElementHandleR eeh, ElementHandleCP templateEh, DPoint3dCP points, size_t numVerts, bool is3d, DgnModelR modelRef);

}; // ShapeHandler

/// @endGroup
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
