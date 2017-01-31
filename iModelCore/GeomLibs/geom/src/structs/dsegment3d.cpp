/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/dsegment3d.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------*//**
 @struct DSegment3d
 A DRay3d structure defines a 3 dimensional line segment start and end points.
 @fields
 @field DPoint3d point[2] array of two points.
 @endfields
 @bsistruct                                         EarlinLutz      09/00
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*----------------------------------------------------------------------+
|                                                                       |
|   macro definitions                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define FIX_MIN(value, min)          if (value < min) min = value
#define FIX_MAX(value, max)          if (value > max) max = value
#define FIX_MINMAX(value, min, max)  FIX_MIN(value, min); else FIX_MAX(value, max)

/*----------------------------------------------------------------------+
|                                                                       |
|   external variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|       local function definitions                                      |
|                                                                       |
+----------------------------------------------------------------------*/

/* VBSUB(Segment3dFromXYZXYZ) */

/*---------------------------------------------------------------------------------**//**
 @description Returns a lines segment defined by its start and end coordinates.
 @instance pSegment OUT initialized segment
 @param x0 IN x coordinate of point 0.
 @param y0 IN y coordinate of point 0.
 @param z0 IN z coordinate of point 0.
 @param x1 IN x coordinate of point 1.
 @param y1 IN y coordinate of point 1.
 @param z1 IN z coordinate of point 1.
  @group "DSegment3d Initialization"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromStartEndXYZXYZ

(
DSegment3dP pSegment,
double            x0,
double            y0,
double            z0,
double            x1,
double            y1,
double            z1
)
    {
    pSegment->point[0].x = x0;
    pSegment->point[0].y = y0;
    pSegment->point[0].z = z0;

    pSegment->point[1].x = x1;
    pSegment->point[1].y = y1;
    pSegment->point[1].z = z1;
    }


/*---------------------------------------------------------------------------------**//**
 @description Down-weight a DSegment4d back to DSegment3d.  Beware that this can fail
 (if weights are zero), and changes parameterization when weights of the two
 points are different.
 @instance pSegment OUT initialized segment
 @param pSource IN source segment.
 @return false if either point of the source segment has zero weights.
 @group "DSegment3d Initialization"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDSegment3d_initFromDSegment4d

(
DSegment3dP pSegment,
DSegment4dCP pSource
)
    {
    return bsiDPoint4d_normalize (&pSource->point[0], &pSegment->point[0])
        && bsiDPoint4d_normalize (&pSource->point[1], &pSegment->point[1]);
    }


/* VBSUB(Segment3dFromPoint3dStartEnd) */

/*---------------------------------------------------------------------------------**//**
 @description Initialize a segment from endpoints.
 @instance pSegment OUT initialized segment.
 @param pPoint0 IN start point.
 @param pPoint1 IN end point.
 @group "DSegment3d Initialization"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDPoint3d

(
DSegment3dP pSegment,
DPoint3dCP pPoint0,
DPoint3dCP pPoint1
)
    {
    pSegment->point[0] = *pPoint0;
    pSegment->point[1] = *pPoint1;
    }


/*---------------------------------------------------------------------------------**//**
 @description Return a segment defined by its 2d endpoints with z=0.
 @instance pSegment OUT initialized segment.
 @param pPoint0 IN start point.
 @param pPoint1 IN end point.
 @group "DSegment3d Initialization"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDPoint2d

(
DSegment3dP pSegment,
DPoint2dCP pPoint0,
DPoint2dCP pPoint1
)
    {
    pSegment->point[0].x = pPoint0->x;
    pSegment->point[0].y = pPoint0->y;

    pSegment->point[1].x = pPoint1->x;
    pSegment->point[1].y = pPoint1->y;

    pSegment->point[0].z = pSegment->point[1].z = 0.0;
    }

/* VBSUB(Segment3dFromPoint3dOriginExtent) */

/*---------------------------------------------------------------------------------**//**
 @description Return a segment defined by start point and extent vector.
 @instance pSegment OUT initialized segment.
 @param pPoint0 IN start point.
 @param pTangent IN extent vector.
 @group "DSegment3d Initialization"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDPoint3dTangent

(
DSegment3dP pSegment,
DPoint3dCP pPoint0,
DPoint3dCP pTangent
)
    {
    pSegment->point[0] = *pPoint0;
    bsiDPoint3d_addDPoint3dDPoint3d (&pSegment->point[1], pPoint0, pTangent);
    }


/*---------------------------------------------------------------------------------**//**
 @description Initialize a segment from a ray.  Ray origin becomes start point. Ray point at parameter 1 becomes end point.
 @instance pSegment OUT initialized segment.
 @param pRay IN source segment.
 @group "DSegment3d Initialization"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_initFromDRay3d

(
DSegment3dP pSegment,
DRay3dCP pRay
)
    {
    pSegment->point[0] = pRay->origin;
    bsiDPoint3d_addDPoint3dDPoint3d (&pSegment->point[1], &pRay->origin, &pRay->direction);
    }

/* VBSUB(Point3dFromSegment3dFractionParameter) */
/*---------------------------------------------------------------------------------**//**
 @description Return a point at a fractional position along a segment.
 @instance pSegment OUT initialized segment.
 @param pPoint OUT computed point
 @param param IN fractional parameter
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_evaluateDPoint3d

(
DSegment3dCP pSegment,
DPoint3dP pPoint,
double            param
)
    {
    bsiDPoint3d_interpolate (pPoint, &pSegment->point[0], param, &pSegment->point[1]);
    }


/*---------------------------------------------------------------------------------**//**
 @description Return the endpoints of the segment.
 @instance pSegment IN segment
 @param pPoint0 OUT start point
 @param pPoint1 OUT end point
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_evaluateEndPoints

(
DSegment3dCP pSegment,
DPoint3dP pPoint0,
DPoint3dP pPoint1
)
    {
    *pPoint0 = pSegment->point[0];
    *pPoint1 = pSegment->point[1];
    }

/* VBSUB(Point3dFromDSegment3dTangent) */
/*---------------------------------------------------------------------------------**//**
 @description Return the (unnormalized) tangent vector along the segment.
 @instance pSegment IN segment
 @param pTangent OUT tangent vector
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            bsiDSegment3d_evaluateTangent

(
DSegment3dCP pSegment,
DPoint3dP pTangent
)
    {
    bsiDPoint3d_subtractDPoint3dDPoint3d (pTangent, &pSegment->point[1], &pSegment->point[0]);
    }

/* VBSUB(Segment3dLengthSquared) */

/*---------------------------------------------------------------------------------**//**
 @description Compute the squared length of a segment.
 @instance pSegment IN segment
 @return squared length of the segment.
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double          bsiDSegment3d_lengthSquared

(
DSegment3dCP pSegment
)
    {
    return bsiDPoint3d_distanceSquared (&pSegment->point[0], &pSegment->point[1]);
    }


/*---------------------------------------------------------------------------------**//**
 @description Compute the range box around a segment.
 @instance pSegment IN segment
 @param pRange <= range of segment.
 @return always true
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDSegment3d_getRange

(
DSegment3dCP pSegment,
DRange3dP pRange
)
    {
    bsiDRange3d_initFrom2Points (pRange, &pSegment->point[0], &pSegment->point[1]);
    return true;
    }



/*---------------------------------------------------------------------------------**//**
 @description Project a point onto the extended line in 3D.
 @instance pSegment IN segment
 @param pClosestPoint OUT computed point
 @param pClosestParam OUT parameter of closest point
 @param pPoint IN space point.
 @return false if the segment degenerates to a point.
 @group "DSegment3d Closest Point"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDSegment3d_projectPoint

(
DSegment3dCP pSegment,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
)
    {
    double param;
    bool    result = bsiDSegment3d_dPoint3dToFractionParameter (pSegment, &param, pPoint);

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        bsiDPoint3d_interpolate (pClosestPoint, &pSegment->point[0], param, &pSegment->point[1]);
    return result;
    }



/*---------------------------------------------------------------------------------**//**
 @description Project a point onto the extended line, using only xy parts for distance calculation.
 @instance pSegment IN segment
 @param pClosestPoint OUT computed point
 @param pClosestParam OUT parameter of closest point
 @param pPoint IN space point.
 @return false if the segment degenerates to a point.
 @group "DSegment3d Closest Point"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDSegment3d_projectPointXY
(
DSegment3dCP pSegment,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
)
    {
    double param;
    DSegment3d xySegment = *pSegment;
    DPoint3d xyPoint = *pPoint;
    xySegment.point[0].z = xySegment.point[1].z = xyPoint.z = 0.0;
    bool    result = bsiDSegment3d_dPoint3dToFractionParameter (&xySegment, &param, &xyPoint);

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        bsiDPoint3d_interpolate (pClosestPoint, &pSegment->point[0], param, &pSegment->point[1]);
    return result;
    }



/*---------------------------------------------------------------------------------**//**
 @description Project a point onto the bounded line in 3D.  If nearest point of extended line
 is outside the 0..1 parameter range, returned values are for nearest endpoint.
 @instance pSegment IN segment
 @param pClosestPoint OUT computed point
 @param pClosestParam OUT parameter of closest point
 @param pPoint IN space point.
 @return false if the segment degenerates to a point.
 @group "DSegment3d Closest Point"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDSegment3d_projectPointBounded

(
DSegment3dCP pSegment,
DPoint3dP pClosestPoint,
double          *pClosestParam,
DPoint3dCP pPoint
)
    {
    double param;
    bool    result = bsiDSegment3d_dPoint3dToFractionParameter (pSegment, &param, pPoint);

    if (param < 0.0)
        param = 0.0;
    if (param > 1.0)
        param = 1.0;

    if (pClosestParam)
        *pClosestParam = param;

    if (pClosestPoint)
        {
        if (param <= 0.0)
            *pClosestPoint = pSegment->point[0];
        else if (param >= 1.0)
            *pClosestPoint = pSegment->point[1];
        else
            bsiDPoint3d_interpolate (pClosestPoint, &pSegment->point[0], param, &pSegment->point[1]);
        }
    return result;
    }

/* VBSUB(Point3dFromSegment3dIntersectPlane3d) */

/*---------------------------------------------------------------------------------**//**
 @description Return the intersection of the (unbounded) segment with a plane.
 @instance pSegment IN segment
 @param pIntPoint => intersection point
 @param pIntParam => parameter along the line
 @param pPlane => plane (origin and normal)
 @return false if line, plane are parallel.
 @group "DSegment3d Intersection"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDSegment3d_intersectDPlane3d

(
DSegment3dCP pSegment,
DPoint3dP pIntPoint,
double          *pIntParam,
DPlane3dCP pPlane
)
    {
    DRay3d ray;
    bsiDRay3d_initFromDSegment3d (&ray, pSegment);
    return bsiDRay3d_intersectDPlane3d (&ray, pIntPoint, pIntParam, pPlane);
    }


/*---------------------------------------------------------------------------------**//**
 @description Return the intersection of the (unbounded) segment with a circle, using only
 xy coordinates.
 @instance pSegment IN segment
 @param pIntPoint <= 0, 1, or 2 intersection points.
 @param pInParam  <= 0, 1, or 2 intersection parameters.
 @param pIntParam => parameter along the line
 @param pCenter => circle center.
 @param radius  => circle radius.
 @return   number of intersections.
 @group "DSegment3d Intersection"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int             bsiDSegment3d_intersectCircleXY

(
DSegment3dCP pSegment,
DPoint3dP pIntPoint,
double          *pIntParam,
DPoint3dCP pCenter,
double          radius
)
    {
    DRay3d ray;
    bsiDRay3d_initFromDSegment3d (&ray, pSegment);
    return bsiDRay3d_intersectCircleXY (&ray, pIntPoint, pIntParam,
                        pCenter, radius);
    }



/*-----------------------------------------------------------------*//**
 @description Apply a transformation to the source segment.
 @instance pDest OUT transformed segment
 @param pTransform => transformation to apply.
 @param pSource => source segment
 @return always true
 @group "DSegment3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDSegment3d_multiplyTransformDSegment3d

(
DSegment3dP pDest,
TransformCP pTransform,
DSegment3dCP pSource
)
    {
    bsiTransform_multiplyDPoint3dArray (pTransform, pDest->point, pSource->point, 2);
    return true;
    }



/*-----------------------------------------------------------------*//**
 @description Compute the parameters and points where the xy projections of two rays intersect.
 @instance pSegment IN segment
 @param pPoint0 <= intersection point on segment 0.
 @param pParam0 <= parametric coordinate on segment 0
 @param pPoint1 <= intesection point on segment 1.
 @param pParam1 <= parametric coordinate on segment 1
 @param pSegment0 => segment 0
 @param pSegment1 => segment 1
 @return true unless lines are parallel
 @group "DSegment3d Intersection"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_intersectXYDSegment3dDSegment3d


(
DPoint3dP pPoint0,
double              *pParam0,
DPoint3dP pPoint1,
double              *pParam1,
DSegment3dCP pSegment0,
DSegment3dCP pSegment1
)

    {
    DPoint3d vector0, vector1;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector0, &pSegment0->point[1], &pSegment0->point[0]);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vector1, &pSegment1->point[1], &pSegment1->point[0]);
    return bsiGeom_intersectXYRays
                        (
                        pPoint0, pParam0,
                        pPoint1, pParam1,
                        &pSegment0->point[0], &vector0,
                        &pSegment1->point[0], &vector1
                        );
    }



/*---------------------------------------------------------------------------------**//**
 @description get the start point of the segment.
 @instance pSegment IN segment
 @param pPoint OUT start point
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDSegment3d_getStartPoint

(
DSegment3dCP pSegment,
DPoint3dP pPoint
)
    {
    *pPoint = pSegment->point[0];
    }


/*---------------------------------------------------------------------------------**//**
 @description return the end point of the segment.
 @instance pSegment IN segment
 @param pPoint OUT end point
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDSegment3d_getEndPoint

(
DSegment3dCP pSegment,
DPoint3dP pPoint
)
    {
    *pPoint = pSegment->point[1];
    }



/*---------------------------------------------------------------------------------**//**
 Set the "start" point for the segment.
 @instance pSegment IN segment
 @param pPoint IN new start point.
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDSegment3d_setStartPoint

(
DSegment3dP pSegment,
DPoint3dCP pPoint
)
    {
    pSegment->point[0] = *pPoint;
    }


/*---------------------------------------------------------------------------------**//**
 Set the "end" point for the segment.
 @instance pSegment IN segment
 @param pPoint  IN new end point.
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bsiDSegment3d_setEndPoint

(
DSegment3dP pSegment,
DPoint3dCP pPoint
)
    {
    pSegment->point[1] = *pPoint;
    }


/*---------------------------------------------------------------------------------**//**
 @description Evaluate the point at a fractional position along a segment.
 @instance pSegment IN segment
 @param pPoint  <= coordinates at fractional parameter.
 @param param      => fractional parameter
 @return always true
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDSegment3d_fractionParameterToDPoint3d

(
DSegment3dCP pSegment,
DPoint3dP pPoint,
double  param
)
    {
    bsiDPoint3d_interpolate (pPoint, &pSegment->point[0], param, &pSegment->point[1]);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
 @description Returns the parameter at which a point projects to the *unbounded)
   line containing the segment.  Parameters less than zero and greater than one
   mean the projection is outside the bounds of the line segment.
 @instance pSegment IN segment
 @param pParam  <= where pPoint projects to the line.
 @param pPoint     => point to project to the line.
 @return false if segment degenerates to a point.
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiDSegment3d_dPoint3dToFractionParameter

(
DSegment3dCP pSegment,
double          *pParam,
DPoint3dCP pPoint
)
    {
    DPoint3d vectorU, vectorV;
    double UdotU, UdotV, param;
    bool    result;

    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorU, &pSegment->point[1], &pSegment->point[0]);
    bsiDPoint3d_subtractDPoint3dDPoint3d (&vectorV, pPoint, &pSegment->point[0]);
    UdotU = bsiDPoint3d_dotProduct (&vectorU, &vectorU);
    UdotV = bsiDPoint3d_dotProduct (&vectorU, &vectorV);

    result = bsiTrig_safeDivide (&param, UdotV, UdotU, 0.0);

    if (pParam)
        *pParam = param;

    return result;
    }


/*---------------------------------------------------------------------------------**//**
 @description Evaluate the point and tangent vector at a fractional parameter.
 @instance pSegment IN segment
 @param pPoint      <= point on line at fractional parameter.
 @param pTangent    <= tangent vector at fractional parameter.
 @param param           => fractional parameter.
 @return always true
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDSegment3d_fractionParameterToDPoint3dTangent

(
DSegment3dCP pSegment,
DPoint3dP pPoint,
DPoint3dP pTangent,
double      param
)
    {

    if (pPoint)
        bsiDPoint3d_interpolate (pPoint, &pSegment->point[0], param, &pSegment->point[1]);

    if (pTangent)
        bsiDPoint3d_subtractDPoint3dDPoint3d (pTangent, &pSegment->point[1], &pSegment->point[0]);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
 @description return one of the endpoints of a segment, selected by integer index 0 or 1.
 @instance pSegment IN segment
 @param pPt <= returned point.
 @param index => index of point to return.
 @return false if index is other than 0 or 1.  In this case the point is left unassigned.
 @group "DSegment3d Evaluate"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool             bsiDSegment3d_getIndexedDPoint3d

(
DSegment3dCP pSegment,
DPoint3dP pPt,
int             index
)
    {
    if (index == 0 || index == 1)
        {
        *pPt = pSegment->point[index];
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
 @description Set one of the endpoints of a segment, selected by integer 0 or 1.
 @instance pSegment IN segment
 @param pPt IN point to copy
 @param index => index of point to update.
 @return false if index is other than 0 or 1.  In this case the point is left unassigned.
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool             bsiDSegment3d_setIndexedDPoint3d

(
DSegment3dP pSegment,
DPoint3dCP pPt,
int             index
)
    {
    if (index == 0 || index == 1)
        {
        pSegment->point[index] = *pPt;
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
 @description Computes equidistant points along this instance.
 @instance pSegment IN segment
 @param    pPoints     <= array of computed points
 @param    numPoints   => # points to compute
 @param    bInclusive   => true to include endpoints in numPoints count
 @group "DSegment3d Evaluate"
 @bsimethod                                                     DavidAssaf      08/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void             bsiDSegment3d_interpolateUniformDPoint3dArray

(
DSegment3dCP pSegment,
DPoint3dP pPoints,
int             numPoints,
bool            bInclusive
)
    {
    double  scalarInc = 1.0, scalar = 1.0;
    int     mult;

    if (numPoints <= 0) return;

    /* if inclusive, return endpts exactly */
    if (bInclusive)
        {
        pPoints[0] = pSegment->point[0];

        if (numPoints > 1)
            {
            pPoints[numPoints - 1] = pSegment->point[1];
            scalar = scalarInc = 1.0 / ((double) (numPoints - 1));
            }

        numPoints -= 2;
        pPoints++;
        }
    else
        scalar = scalarInc = 1.0 / ((double) (numPoints + 1));

    /* compute only interior pts */
    for (mult = 1; mult <= numPoints; scalar = ++mult * scalarInc)
        bsiDSegment3d_fractionParameterToDPoint3d (pSegment, pPoints++, scalar);
    }

/* VBSUB(Segment3dLength) */

/*---------------------------------------------------------------------------------*//**
 @description compute the length of a line segment.
 @instance pSegment IN segment
 @return line segment length.
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDSegment3d_length

(
DSegment3dCP pSegment
)
    {
    return bsiDPoint3d_distance (&pSegment->point[0], &pSegment->point[1]);
    }


/*---------------------------------------------------------------------------------**//**
 Compute the (signed) arc length between specified fractional parameters.
 @instance pSegment IN segment to query
 @param pArcLength <= computed arc length.  Negative if fraction1 < fraction0.
 @param fraction0 => start fraction for interval to measure.
 @param fraction1 => end fraction for interval to measure.
 @return true if the arc length was computed.
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDSegment3d_fractionToLength

(
DSegment3dCP pSegment,
double      *pArcLength,
double      fraction0,
double      fraction1
)
    {
    if (pArcLength)
        *pArcLength = (fraction1 - fraction0) * bsiDSegment3d_length (pSegment);
    return true;
    }



/*---------------------------------------------------------------------------------**//**
 Compute the fraction parameter corresponding to a specified arc length away from
   a specified start fraction. (inverse of fractions to arcStep)
@instance pSegment IN segment to query
 @param pFraction1 <= fraction at end of interval.
 @param fraction0 => start fraction for interval to measure.
 @param arcStep => arc length to move.  Negative arc length moves backwards.
 @return true if the fractional step was computed.
 @group "DSegment3d Queries"
 @bsimethod                                                     EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     bsiDSegment3d_lengthToFraction

(
DSegment3dCP pSegment,
double      *pFraction1,
double      fraction0,
double      arcStep
)
    {
    double df = 0.0;
    bool    stat = bsiTrig_safeDivide (&df, arcStep, bsiDSegment3d_length (pSegment), 0.0);
    if (pFraction1)
        *pFraction1 = fraction0 + df;
    return stat;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
