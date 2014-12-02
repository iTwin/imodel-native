/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IGeoCoordReproject.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

GEOCOORD_TYPEDEFS (DgnGCS)

BEGIN_BENTLEY_API_NAMESPACE;

enum ReprojectStatus;

END_BENTLEY_API_NAMESPACE;

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef struct IGeoCoordinateReprojectionSettings* IGeoCoordinateReprojectionSettingsP;

enum ReprojectionOption
{
ReprojectionOptionNever   = 0,
ReprojectionOptionAlways  = 1,
ReprojectionOptionIfLarge = 2,
};

/*=================================================================================**//**
* Interface that exposes methods that are needed by DisplayHandler's to reproject
*  graphic elements from their coordinate system to a different geographic coordinate system.
*  The IGeoCoordinateReprojectionHelper object is passed as an argument to the DisplayHandler's
*  _OnGeoCoordinateReprojection method. That method is called when MicroStation is reprojecting
*  elements from one GeoCoordinate system to another, either while preparing a reference for "on-the-fly"
*  reprojection display, or when changing the GeoCoordinate system associated with a model.
*  When the IGeoCoordinateReprojectionHelper object is passed to the DisplayHandler's _OnGeoCoordinateReprojection
*  method, it is set up to accept input in the UORS of the model that contains the element and
*  convert to UORS in the desired destination coordinate system.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  IGeoCoordinateReprojectionHelper
{
/*---------------------------------------------------------------------------------**//**
* Reprojects an array of 3D points from the host models UORs to the destination models UORs.
* @param        outUors       OUT an array to receive the points transformed to the target cartesian coordinate system.
*                                 Must be non-NULL and must be dimensioned to hold \c numPts points.
* @param        outLatLong    OUT an array to receive the points transform to the target latitude longitude coordinate system.
*                                 Must be dimensioned to hold \c numPts points.
*                                 Can be NULL if the target latitude longitude points are not needed.
* @param        inLatLong     OUT an array to receive the points transformed to latitude longitude in the source coordinate system.
*                                 Must be dimensioned to hold \c numPts points.
*                                 Can be NULL if the source latitude longitude points are not needed.
* @param        inUors        IN  an array that contains the points in UORs of the source model.
* @param        numPoints     IN  number of points in the inCartesian array.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ReprojectStatus         ReprojectPoints (DPoint3dP outUors, GeoPointP outLatLong, GeoPointP inLatLong, DPoint3dCP inUors, int numPoints)=0;

/*---------------------------------------------------------------------------------**//**
* Reprojects an array of 2D points from the host models UORs to the destination models UORs.
* @param        outUors       OUT an array to receive the points transformed to the target cartesian coordinate system.
*                                 Must be non-NULL and must be dimensioned to hold \c numPts points.
* @param        outLatLong    OUT an array to receive the points transform to the target latitude longitude coordinate system.
*                                 Must be dimensioned to hold \c numPts points.
*                                 Can be NULL if the target latitude longitude points are not needed.
* @param        inLatLong     OUT an array to receive the points transformed to latitude longitude in the source coordinate system.
*                                 Must be dimensioned to hold \c numPts points.
*                                 Can be NULL if the source latitude longitude points are not needed.
* @param        inUors        IN  an array that contains the points in UORs of the source model.
* @param        numPoints     IN  number of points in the inCartesian array.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ReprojectStatus         ReprojectPoints2D (DPoint2dP outUors, GeoPoint2dP outLatLong, GeoPoint2dP inLatLong, DPoint2dCP inUors, int numPoints)=0;

/*---------------------------------------------------------------------------------**//**
* Reprojects an array of 3D points from the host models UORs to the destination models UORs, with the option of adding intermediate points if needed.
* @param        outUors           OUT the output point array. May contain more than numPoint points. The caller is not responsible for freeing the points.
* @param        outPointCount     OUT the number of points in outUors.
* @param        inUors            IN  an array that contains the points in UORs of the source model.
* @param        numPoints         IN  number of points in the inCartesian array.
* @param        tolerance         IN  the tolerance required.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ReprojectStatus         ReprojectPointsMoreDetail (DPoint3dP *outUors, int *outPointCount, DPoint3dCP inUors, int numPoints, double tolerance)=0;

/*---------------------------------------------------------------------------------**//**
* Reprojects an array of 2D points from the host models UORs to the destination models UORs, with the option of adding intermediate points if needed.
* @param        outUors           OUT the output point array. May contain more than numPoint points. The caller is not responsible for freeing the points.
* @param        outPointCount     OUT the number of points in outUors.
* @param        inUors            IN  an array that contains the points in UORs of the source model.
* @param        numPoints         IN  number of points in the inCartesian array.
* @param        tolerance         IN  the tolerance required.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ReprojectStatus         ReprojectPointsMoreDetail2D (DPoint3dP *outUors, int *outPointCount, DPoint2dCP inUors, int numPoints, double tolerance)=0;

/*---------------------------------------------------------------------------------**//**
* Calculates a linear transform at the specified point that can be used to transform an element (approximately) to the new GeoCoordinate system.
* @param        outTransform  OUT the computed transform.
* @param        elementOrigin IN  the point, in UORS of the source model, that the transform is to be computed for.
* @param        extent        IN  the extent, in UORS of the source model, of the input element. Can be NULL, in which case a reasonable default is used.
* @param        doRotate      IN  include rotation in the transform.
* @param        doScale       IN  include scale in the transform.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ReprojectStatus         GetLocalTransform (TransformP outTransform, DPoint3dCR elementOrigin, DPoint3dCP extent, bool doRotate, bool doScale)=0;

/*---------------------------------------------------------------------------------**//**
* Returns the settings for the reprojection.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IGeoCoordinateReprojectionSettingsP GetSettings()=0;

/*---------------------------------------------------------------------------------**//**
* Returns the tolerance in destination model UORS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double                  GetStrokeToleranceDestUors()=0;

/*---------------------------------------------------------------------------------**//**
* Returns the tolerance in source model UORS
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double                  GetStrokeToleranceSourceUors()=0;

/*---------------------------------------------------------------------------------**//**
* Returns the GeoCoordinateSystem for the source (reference) cache.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnGCSP                 GetSourceGCS ()=0;

/*---------------------------------------------------------------------------------**//**
* Returns the GeoCoordinateSystem for the destination (master) cache.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnGCSP                 GetDestinationGCS ()=0;

/*---------------------------------------------------------------------------------**//**
* Returns true if the elemHandle passed in should be stroked, given its range the option
*  appropriate to that element type.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool                    ShouldStroke (EditElementHandleR elemHandle, ReprojectionOption option)=0;

/*---------------------------------------------------------------------------------**//**
* Returns the source modelRef.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnModelP            GetSourceDgnModel ()=0;

/*---------------------------------------------------------------------------------**//**
* Returns the destination modelRef.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnModelP            GetDestinationDgnModel ()=0;

/*---------------------------------------------------------------------------------**//**
* Returns the unit ratio (destination modelRef uor/meter divided by source modelRef uor/meter)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual double                  GetUnitRatio ()=0;

};

struct  IGeoCoordinateReprojectionSettings
{
virtual double                  StrokeTolerance()=0;
virtual ReprojectionOption      DoCellElementsIndividually()=0;
virtual ReprojectionOption      DoMultilineTextElementsIndividually()=0;
virtual bool                    ScaleText()=0;
virtual bool                    RotateText()=0;
virtual bool                    ScaleCells()=0;
virtual bool                    RotateCells()=0;
virtual ReprojectionOption      StrokeArcs()=0;
virtual ReprojectionOption      StrokeEllipses()=0;
virtual ReprojectionOption      StrokeCurves()=0;

virtual bool                    PostStrokeLinear()=0;
virtual bool                    ReprojectElevation()=0;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
