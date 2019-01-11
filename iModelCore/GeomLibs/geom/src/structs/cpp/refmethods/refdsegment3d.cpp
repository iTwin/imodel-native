/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/refdsegment3d.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*---------------------------------------------------------------------------------**//**
 @description Find the closest approach of two (unbounded) lines.
 @instance pSegment IN segment
 @param pParam0 <= parameter on first ray.
 @param pParam1 <= parameter on second ray.
 @param pPoint0 <= point on first ray.
 @param pPoint1 <= point on second ray.
 @param pSegment0   <= first segment
 @param pSegment1   <= second segment
 @return false if the segments are parallel.
 @group "DSegment3d Closest Point"
 @bsimethod                                                     EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool            bsiDSegment3d_closestApproach

(
double          *pParam0,
double          *pParam1,
DPoint3dP pPoint0,
DPoint3dP pPoint1,
DSegment3dCP pSegment0,
DSegment3dCP pSegment1
)
    {
    return bsiGeom_closestApproachOfLines
                (
                pParam0,
                pParam1,
                pPoint0,
                pPoint1,
                &pSegment0->point[0],
                &pSegment0->point[1],
                &pSegment1->point[0],
                &pSegment1->point[1]
                );
    }
/*-----------------------------------------------------------------*//**
*
* Compute the parameters and points where the xy projections of two rays intersect.
*
* @param pPoint0 <= intersection point on line 0.
* @param s0P <= parametric coordinate on segment 0
* @param pPoint1 <= intesection point on line 1.
* @param s1P <= parametric coordinate on segment 1
* @param pStart0 => start of first ray
* @param pVec0 => first vector
* @param pStart1 => start of second ray
* @param pVec1 => second vector
* @return true unless rays are parallel
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_intersectXYRays


(
DPoint3dP pPoint0,
double          *pParam0,
DPoint3dP pPoint1,
double          *pParam1,
DPoint3dCP pStart0,
DPoint3dCP pVec0,
DPoint3dCP pStart1,
DPoint3dCP pVec1
)

    {
    double param0, param1;
    bool    result = bsiSVD_solve2x2 (&param0, &param1, pVec0->x, -pVec1->x, pVec0->y, -pVec1->y,
                                pStart1->x - pStart0->x,pStart1->y - pStart0->y);

    if (result)
        {
        if (pPoint0)
            *pPoint0 = (*pStart0) + (*(DVec3d const*)pVec0) * param0;//            bsiDPoint3d_addScaledDPoint3d (pPoint0, pStart0, pVec0, param0);
        if (pPoint1)
            *pPoint1 = (*pStart1) + (*(DVec3d const*)pVec1) * param1;//            bsiDPoint3d_addScaledDPoint3d (pPoint1, pStart1, pVec1, param1);
        if (pParam0)
            *pParam0 = param0;
        if (pParam1)
            *pParam1 = param1;
        }
    return result;
    }

/*-----------------------------------------------------------------*//**
* finds the point of closest approach between two (possibly skewed,
* nonintersecting) straight lines in 3 space.  The lines are describe
* by two endpoints.  The return points are described by their parametric
* and cartesian coordinates on the respective lines.
* Input maxParam is the largest parameter value permitted in the output
* points.    If only points within the lines are ever of interest, send
* a smallish maxParam, e.g. 10 or 100.  A zero parameter value indicates
* large parameters are acceptable.
*
* @param pParamA <= parametric coordinate on line A
* @param pParamB <= parametric coordinate on line B
* @param pPointAP <= closest point on A. May be NULL
* @param pPointBP <= closest point on B. May be NULL
* @param pStartA => start point of line A
* @param pEndA => end point of line A
* @param pStartB => start point of line B
* @param pEndB => end point of line B
* @return true if lines are non-parallel
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiGeom_closestApproachOfLines

(
double      *pParamA,
double      *pParamB,
DPoint3dP pPointAP,
DPoint3dP pPointBP,
DPoint3dCP pStartA,
DPoint3dCP pEndA,
DPoint3dCP pStartB,
DPoint3dCP pEndB
)
    {
    DVec3d    tangentA = (*pEndA) - (*pStartA);   // bsiDPoint3d_subtractDPoint3dDPoint3d (&tangentA, pEndA, pStartA);
    DVec3d    tangentB = (*pEndB) - (*pStartB);   //bsiDPoint3d_subtractDPoint3dDPoint3d (&tangentB, pEndB, pStartB);

    return bsiGeom_closestApproachOfRays (pParamA, pParamB, pPointAP, pPointBP,
                pStartA, &tangentA, pStartB, &tangentB);
    }

/*---------------------------------------------------------------------------------**//**
* @description Return a segment defined by start point and extent vector.
*
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::FromOriginAndDirection
(
DPoint3dCR      point0,
DPoint3dCR      tangent
)
    {
    DSegment3d segment;
    segment.InitFromOriginAndDirection (point0, tangent);
    return segment;
    }

/*---------------------------------------------------------------------------------**//**
* Initialize a segment from a ray.
*
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::From
(
DRay3dCR        ray
)
    {
    DSegment3d segment;
    segment.InitFrom (ray);
    return segment;
    }

/*---------------------------------------------------------------------------------**//**
* Initialize a segment from a ray.
*
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::FromZero ()
    {
    DSegment3d segment;
    segment.point[0].Zero ();
    segment.point[1].Zero ();
    return segment;
    }

/*---------------------------------------------------------------------------------**//**
* Initialize a segment from a ray.
*
+--------------------------------------------------------------------------------------*/
void DSegment3d::InitZero ()
    {
    point[0].Zero ();
    point[1].Zero ();
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::FromFractionInterval (DSegment3dCR parent, double startFraction, double endFraction)
    {
    if (DoubleOps::IsExact01 (startFraction, endFraction))
        return parent;
    DSegment3d segment = parent;
    DVec3d extent;
    extent.DifferenceOf (parent.point[1], parent.point[0]);
    segment.point[0].SumOf (parent.point[0], extent, startFraction);
    segment.point[1].SumOf (parent.point[0], extent, endFraction);
    return segment;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::FromFractionInterval (DSegment3dCR parent, DSegment1dCR interval)
    {
    return DSegment3d::FromFractionInterval (parent, interval.GetStart (), interval.GetEnd ());
    }



/*---------------------------------------------------------------------------------**//**
* @description Returns a lines segment defined by its start and end coordinates.
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::From
(
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
)
    {
    DSegment3d segment;
    segment.Init (x0, y0, z0, x1, y1, z1);
    return segment;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::From
(
DPoint3dCR      point0,
DPoint3dCR      point1
)
    {
    DSegment3d segment;
    segment.Init (point0, point1);
    return segment;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a segment defined by its endpoints.
*
+--------------------------------------------------------------------------------------*/
DSegment3d DSegment3d::From
(
DPoint2dCR      point0,
DPoint2dCR      point1
)
    {
    DSegment3d segment;
    segment.Init (point0, point1);
    return segment;
    }


/*---------------------------------------------------------------------------------**//**
* @description Returns a lines segment defined by its start and end coordinates.
* @indexVerb init
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void DSegment3d::Init
(

      double            x0,
      double            y0,
      double            z0,
      double            x1,
      double            y1,
      double            z1

)
    {
    this->point[0].x = x0;
    this->point[0].y = y0;
    this->point[0].z = z0;

    this->point[1].x = x1;
    this->point[1].y = y1;
    this->point[1].z = z1;
    }


/*---------------------------------------------------------------------------------**//**
* Down-weight a DSegment4d back to DSegment3d.  Beware that this can fail
* (if weights are zero), and changes parameterization when weights of the two
* points are different.
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
bool DSegment3d::Init
(

DSegment4dCR source

)
    {
    bool stat = true;
    if (!source.point[0].GetProjectedXYZ (point[0]))
        stat = false;
    if (!source.point[1].GetProjectedXYZ (point[1]))
        stat = false;
    return stat;
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from endpoints.
* @indexVerb init
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void DSegment3d::Init
(

DPoint3dCR point0,
DPoint3dCR point1

)
    {
    this->point[0] = point0;
    this->point[1] = point1;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a segment defined by its endpoints.
*
* @indexVerb init
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void DSegment3d::Init
(

DPoint2dCR point0,
DPoint2dCR point1

)
    {
    this->point[0].x = point0.x;
    this->point[0].y = point0.y;

    this->point[1].x = point1.x;
    this->point[1].y = point1.y;

    this->point[0].z = this->point[1].z = 0.0;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a segment defined by start point and extent vector.
*
* @indexVerb init
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void DSegment3d::InitFromOriginAndDirection
(

DPoint3dCR point0,
DPoint3dCR tangent

)
    {
    point[0] = point0;
    point[1].SumOf (point0, tangent);
    }


/*---------------------------------------------------------------------------------**//**
* Initialize a segment from a ray.
*
* @indexVerb init
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void DSegment3d::InitFrom
(

DRay3dCR ray

)
    {
    point[0] = ray.origin;
    point[1].SumOf (ray.origin, ray.direction);
    }


/*---------------------------------------------------------------------------------**//**
* Return the endpoints of the segment.
*
* @indexVerb parameterization
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
void DSegment3d::GetEndPoints
(

DPoint3dR point0,
DPoint3dR point1

) const
    {
    point0 = point[0];
    point1 = point[1];
    }


/*---------------------------------------------------------------------------------**//**
* @return squared length of the segment.
*
* @indexVerb magnitude
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
double DSegment3d::LengthSquared
(

) const
    {
    return point[0].DistanceSquared (point[1]);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
bool DSegment3d::GetRange (DRange3dR range) const
    {
    range.InitFrom (point[0], point[1]);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/12
+--------------------------------------------------------------------------------------*/
DRange1d DSegment3d::ProjectedParameterRange (DRay3dCR ray) const
    {
    double divAA = ray.direction.SafeOneOverMagnitudeSquared (0.0);
    double a0 = ray.DirectionDotVectorToTarget (point[0]) * divAA;
    double a1 = ray.DirectionDotVectorToTarget (point[1]) * divAA;
    return DRange1d::From (a0, a1);
    }


/*---------------------------------------------------------------------------------**//**
* Project a point onto the extended line in 3D.
* @param [out] closestPoint point on extended line
* @param [out] closestparam parameter for closest point
* @param [in] point space point.
* @indexVerb projection
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
bool DSegment3d::ProjectPoint
(

DPoint3dR closestPoint,
double          &closestParam,
DPoint3dCR point

) const
    {
    double param;
    bool result = this->PointToFractionParameter (param, point);

    closestParam = param;

    closestPoint.Interpolate (this->point[0], param, this->point[1]);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Project a point onto the extended line using only xy parts.
* @param [out] closestPoint point on extended line
* @param [out] closestparam parameter for closest point
* @param [in] point space point.* @indexVerb projection
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
bool DSegment3d::ProjectPointXY
(

DPoint3dR closestPoint,
double          &closestParam,
DPoint3dCR point

) const
    {
    double param;
    DSegment3d xySegment = *this;
    DPoint3d xyPoint = point;
    xySegment.point[0].z = xySegment.point[1].z = xyPoint.z = 0.0;
    bool result = xySegment.PointToFractionParameter (param, xyPoint);

    closestParam = param;

    closestPoint.Interpolate (this->point[0], param, this->point[1]);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* Project a point onto the bounded line in 3D.  If nearest point of extended line
* is outside the 0..1 parameter range, returned values are for nearest endpoint.
* @param [out] closestPoint point on extended line
* @param [out] closestparam parameter for closest point
* @param [in] point space point.* @indexVerb projection
* @indexVerb projection
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
bool DSegment3d::ProjectPointBounded
(
DPoint3dR closestPoint,
double          &closestParam,
DPoint3dCR spacePoint,
bool extend0,
bool extend1
) const
    {
    bool result = PointToFractionParameter (closestParam, spacePoint);

    if (closestParam < 0.0 && !extend0)
        {
        closestParam = 0.0;
        closestPoint = point[0];
        }
    else if (closestParam > 1.0 && !extend1)
        {
        closestParam = 1.0;
        closestPoint = point[1];
        }
    else
        {
        closestPoint.Interpolate (point[0], closestParam, point[1]);
        }
    return result;
    }

bool DSegment3d::ProjectPointBounded
(
DPoint3dR closestPoint,
double          &closestParam,
DPoint3dCR point
) const
    {
    return ProjectPointBounded (closestPoint, closestParam, point, false, false);
    }
/*---------------------------------------------------------------------------------**//**
* Return the intersection of the (unbounded) segment with a plane.
* @param [in] intPoint intersection point
* @param [in] intParam parameter along the line
* @param [in] plane plane (origin and normal)
* @return false if line, plane are parallel.
* @indexVerb intersection
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
bool DSegment3d::Intersect
(
DPoint3dR intPoint,
double   &intParam,
DPlane3dCR plane
) const
    {
    DRay3d ray; 
    ray.InitFrom (*this);
    return ray.Intersect (intPoint, intParam, plane);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
bool DSegment3d::IntersectXY
(
double          &fractionA,
double          &fractionB,
DPoint3dR       pointA,
DPoint3dR       pointB,
DSegment3dCR    segmentA,
DSegment3dCR    segmentB
)
    {

    DPoint3d vector0 = DVec3d::FromStartEnd (segmentA.point[0], segmentA.point[1]);
    DPoint3d vector1 = DVec3d::FromStartEnd (segmentB.point[0], segmentB.point[1]);

    return bsiGeom_intersectXYRays
            (
            &pointA, &fractionA,
            &pointB, &fractionB,
            &segmentA.point[0], &vector0,
            &segmentB.point[0], &vector1
            );
    }

/*---------------------------------------------------------------------------------**//**
* Return the intersection of the (unbounded) segment with a circle, using only
* xy coordinates.
* @param [out] intPoint 0, 1, or 2 intersection points.
* @param [out] pInParam 0, 1, or 2 intersection parameters.
* @param [in] pIntParam parameter along the line
* @param [in] center circle center.
* @param [in] radius circle radius.
* @return   number of intersections.
* @indexVerb intersection
* @bsimethod                                                    EarlinLutz      10/98
+--------------------------------------------------------------------------------------*/
int DSegment3d::IntersectCircleXY
(

DPoint3dP intPoint,
                double          *pIntParam,
DPoint3dCR center,
                double          radius

) const
    {
    DRay3d ray;
    ray.InitFrom (*this);
    return ray.IntersectCircleXY (intPoint, pIntParam, center, radius);
    }


/*-----------------------------------------------------------------*//**
*
* Apply a transformation to the source segment.
* @param [in] transform transformation to apply.
* @param [in] source source segment
* @indexVerb transform
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DSegment3d::InitProduct
(

TransformCR transform,
DSegment3dCR source

)
    {
    transform.Multiply (this->point, source.point, 2);
    }

/*---------------------------------------------------------------------------------**//**
* Get start point for a linear object. Only valid if isLinear is true.
* @param        start point of object.
* @indexVerb get
* @indexVerb parameterization
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
void DSegment3d::GetStartPoint
(

DPoint3dR pt

) const
    {
    pt = this->point[0];
    }


/*---------------------------------------------------------------------------------**//**
* Get end point for a linear object. Only valid if isLinear is true.
* @param        end point of object.
* @indexVerb get
* @indexVerb parameterization
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
void DSegment3d::GetEndPoint
(

DPoint3dR pt

) const
    {
    pt = this->point[1];
    }


/*---------------------------------------------------------------------------------**//**
* Set the "start" point for a linear object. Only valid if isLinear is true.
* @param        point          new start point.
* @indexVerb set
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
void DSegment3d::SetStartPoint
(

DPoint3dCR point

)
    {
    this->point[0] = point;
    }


/*---------------------------------------------------------------------------------**//**
* Set the "end" point for a linear object. Only valid if isLinear is true.
* @param        point          new end point.
* @indexVerb set
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
void DSegment3d::SetEndPoint
(

DPoint3dCR point

)
    {
    this->point[1] = point;
    }


/*---------------------------------------------------------------------------------**//**
* @param [out] pPointOut coordinates at fractional parameter.
* @param [in] param fractional parameter
* @indexVerb parameterization
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DSegment3d::FractionParameterToPoint
(

DPoint3dR point,
double  param

) const
    {
    point.Interpolate (this->point[0], param, this->point[1]);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/16
+--------------------------------------------------------------------------------------*/
DVec3d DSegment3d::VectorStartToEnd () const
    {
    return point[1] - point[0];
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2015
+--------------------------------------------------------------------------------------*/
DPoint3d DSegment3d::FractionToPoint (double  fraction) const
    {
    return DPoint3d::FromInterpolate (point[0], fraction, point[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @description Returns the parameter at which a point projects to the *unbounded)
*   line containing the segment.  Parameters less than zero and greater than one
*   mean the projection is outside the bounds of the line segment.
* @param [out] param fraction where point projects to the line.
* @param [in] point point to project to the line.
* @indexVerb projection
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DSegment3d::PointToFractionParameter
(
double &param,
DPoint3dCR spacePoint

) const
    {
    return DVec3d::FromStartEnd(point[0], point[1]).ValidatedFractionOfProjection(
                    DVec3d::FromStartEnd(point[0], spacePoint),
                    0.0).IsValid (param);
    }


/*---------------------------------------------------------------------------------**//**
* @param [out] pPointOut point on line at fractional parameter.
* @param [out] pTangentOut tangent vector at fractional parameter.
* @param [in] param fractional parameter.
* @indexVerb parameterization
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DSegment3d::FractionParameterToTangent
(

DPoint3dR point,
DVec3dR tangent,
        double      param

) const
    {
    point.Interpolate (this->point[0], param, this->point[1]);

    tangent.DifferenceOf (this->point[1], this->point[0]);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @param [out] pt returned point.
* @param [in] index index of point to return.
* @indexVerb get
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DSegment3d::GetPoint
(

DPoint3dR pt,
        int             index

) const
    {
    if (index == 0 || index == 1)
        {
        pt = this->point[index];
        return true;
        }
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @param [out] pt returned point.
* @param [in] index index of point to return.
* @indexVerb set
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DSegment3d::SetPoint
(

DPoint3dCR pt,
        int             index

)
    {
    if (index == 0 || index == 1)
        {
        this->point[index] = pt;
        return true;
        }
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DSegment3d::InterpolateUniformArray
(
bvector<DPoint3d>&points,
size_t          numPoints
) const
    {
    DPoint3d xyz;

    points.clear ();
    //if (numPoints < 0)        unsigned value is never < 0
    //    return;
    if (numPoints == 1)
        {
        xyz.Interpolate (point[0], 0.5, point[1]);
        points.push_back (xyz);
        return;
        }

    points.push_back (point[0]);

    double df = 1.0 / ((double) (numPoints - 1));

    /* compute only interior pts */
    for (size_t i = 1; i < numPoints - 1; i++)
        {
        xyz.Interpolate (point[0], i * df, point[1]);
        points.push_back (xyz);
        }

    points.push_back (point[1]);
    }



/*---------------------------------------------------------------------------------*//**\
* @return line segment length.
* @indexVerb magnitude
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
double DSegment3d::Length
(

) const
    {
    return point[0].Distance (point[1]);
    }


/*---------------------------------------------------------------------------------**//**
* Compute the (signed) arc length between specified fractional parameters.
* @param [out] arcLength computed arc length.  Negative if fraction1 < fraction0.
* @param [in] fraction0 start fraction for interval to measure.
* @param [in] fraction1 end fraction for interval to measure.
* @return true if the arc length was computed.
* @indexVerb arcLength
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DSegment3d::FractionToLength
(

double      &arcLength,
double      fraction0,
double      fraction1

) const
    {
    arcLength = (fraction1 - fraction0) * this->Length ();
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* Compute the fraction parameter corresponding to a specified arc length away from
*   a specified start fraction. (inverse of fractions to arcStep)
* @param [out] fraction1 fraction at end of interval.
* @param [in] fraction0 start fraction for interval to measure.
* @param [in] arcStep arc length to move.  Negative arc length moves backwards.
* @return true if the fractional step was computed.
* @indexVerb arcLength
* @bsimethod                                                    EarlinLutz      03/99
+--------------------------------------------------------------------------------------*/
bool DSegment3d::LengthToFraction
(

double      &fraction1,
double      fraction0,
double      arcStep

) const
    {
    double df = 0.0;
    bool stat = DoubleOps::SafeDivideParameter (df, arcStep, this->Length (), 0.0);
    fraction1 = fraction0 + df;
    return stat;
    }



/*---------------------------------------------------------------------------------**//**
@description Compute projection
+--------------------------------------------------------------------------------------*/
bool DSegment3d::ClosestPointBoundedXY
(
DPoint3dR   closePoint,
double&     closeParam,
double&     distanceXY,
DPoint3dCR  spacePoint,
DMatrix4dCP worldToLocal
) const
    {
    return ClosestPointBoundedXY (closePoint, closeParam, distanceXY, spacePoint, worldToLocal, false, false);
    }


/*---------------------------------------------------------------------------------**//**
@description Compute projection
+--------------------------------------------------------------------------------------*/
bool DSegment3d::ClosestPointBoundedXY
(
DPoint3dR   closePoint,
double&     closeParam,
double&     distanceXY,
DPoint3dCR  spacePoint,
DMatrix4dCP worldToLocal,
bool extend0,
bool extend1
) const
    {
    DSegment4d segment4d;
    DPoint4d closePoint4d;
    DPoint4d spacePoint4d;
    spacePoint4d.InitFrom (spacePoint, 1.0);
    segment4d.point[0].InitFrom (point[0], 1.0);
    segment4d.point[1].InitFrom (point[1], 1.0);

    if (worldToLocal != NULL)
        {
        worldToLocal->Multiply (segment4d.point, segment4d.point, 2);
//        bsiDSegment4d_transformDMatrix4d (&segment4d, worldToLocal, &segment4d);
        worldToLocal->Multiply (spacePoint4d, spacePoint4d);
        }

    // When this fails (zero length) it still returns zero parameter ...
    segment4d.ProjectDPoint4dCartesianXYW (closePoint4d, closeParam, spacePoint4d);
    // hm.. is it this simple?  Are there spooky effects where the line cuts the eyeplane?
    if (closeParam > 1.0 && !extend1)
        {
        closeParam = 1.0;
        closePoint4d = segment4d.point[1];
        }
    else if (closeParam < 0.0 && !extend0)
        {
        closeParam = 0.0;
        closePoint4d = segment4d.point[0];            
        }
    FractionParameterToPoint (closePoint, closeParam);
    return spacePoint4d.RealDistanceXY (distanceXY, closePoint4d);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DSegment3d::WireCentroid
(
double     &length,
DPoint3dR   centroid,
double      fraction0,
double     fraction1
) const
    {
    DPoint3d pointA, pointB;
    pointA.Interpolate (point[0], fraction0, point[1]);
    pointB.Interpolate (point[0], fraction1, point[1]);
    length = pointA.Distance (pointB);
    centroid.Interpolate (pointA, 0.5, pointB);
    }


// Find min dist from given point to segmentB.
// Update min dist and fraction/point data.
static void TryMinDist (
double &minDist,
double testFraction,
DPoint3dCR testPoint,
DSegment3dCR segmentB,
double &fractionA,
DPoint3dR pointA,
double &fractionB,
DPoint3dR pointB
)
    {
    DPoint3d closePoint;
    double   closeFraction;
    if (segmentB.ProjectPointBounded (closePoint, closeFraction, testPoint))
        {
        double d = testPoint.Distance (closePoint);
        if (d < minDist)
            {
            minDist = d;
            fractionA = testFraction;
            pointA    = testPoint;
            fractionB = closeFraction;
            pointB    = closePoint;
            }
        }
    }

static void TryMinDistXY (
double &minDist,
double testFraction,
DPoint3dCR testPoint,
DSegment3dCR segmentB,
double &fractionA,
DPoint3dR pointA,
double &fractionB,
DPoint3dR pointB
)
    {
    DPoint3d closePoint;
    double   closeFraction;
    double d;
    if (segmentB.ClosestPointBoundedXY (closePoint, closeFraction, d, testPoint, NULL, false, false))
        {
        if (d < minDist)
            {
            minDist = d;
            fractionA = testFraction;
            pointA    = testPoint;
            fractionB = closeFraction;
            pointB    = closePoint;
            }
        }
    }



#ifdef ComipileAll
// Analysis of overlap portions of parallel or coincident segments.
// Given fractions where segmentB endpoints project to segmentA.
// Determine fractions on A and B for closest approach.
static void AnalyzeFractionProximityRange
(
double fractionB0onA,
double fractionB1onA,
double &fractionA,
double &fractionB
)
    {
    double delta = fractionB1onA - fractionB0onA;
    if (fractionB0onA < 0.0)
        {
        fractionA = 0.0;
        if (fractionB1onA >= 0.0)
            fractionB = -fractionB0onA / delta;
        else if (delta > 0.0)
            fractionB = 1.0;
        else
            fractionB = 0.0;            
        }
    else if (fractionB0onA <= 1.0)
        {
        fractionB = 0.0;
        fractionA = fractionB0onA;
        }
    else    // fractionB0onA > 1.0
        {
        fractionA = 1.0;
        if (fractionB1onA <= 1.0)
            fractionB = (1.0 - fractionB0onA) / delta;
        else if (delta > 0.0)
            fractionB = 0.0;
        else
            fractionB = 1.0;
        }
    }
#endif



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DSegment3d::ClosestApproachBounded
(
double      &fractionA,
double      &fractionB,
DPoint3dR    pointA,
DPoint3dR    pointB,
DSegment3dCR segmentA,
DSegment3dCR segmentB
)
    {
    DRay3d rayA = DRay3d::From (segmentA);
    DRay3d rayB = DRay3d::From (segmentB);

    if (DRay3d::ClosestApproachUnboundedRayUnboundedRay (
                fractionA, fractionB, pointA, pointB, rayA, rayB)
        && fractionA >= 0.0 && fractionA <= 1.0
        && fractionB >= 0.0 && fractionB <= 1.0)
            {
            }
    else
        {
        double dMin = DBL_MAX;
        TryMinDist (dMin, 0.0, segmentA.point[0], segmentB, fractionA, pointA, fractionB, pointB);
        TryMinDist (dMin, 1.0, segmentA.point[1], segmentB, fractionA, pointA, fractionB, pointB);
        TryMinDist (dMin, 0.0, segmentB.point[0], segmentA, fractionB, pointB, fractionA, pointA);
        TryMinDist (dMin, 1.0, segmentB.point[1], segmentA, fractionB, pointB, fractionA, pointA);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DSegment3d::ClosestApproachBoundedXY
(
double      &fractionA,
double      &fractionB,
DPoint3dR    pointA,
DPoint3dR    pointB,
DSegment3dCR segmentA,
DSegment3dCR segmentB
)
    {
    if (DSegment3d::IntersectXY (fractionA, fractionB, pointA, pointB, segmentA, segmentB)
        && fractionA >= 0.0 && fractionA <= 1.0
        && fractionB >= 0.0 && fractionB <= 1.0)
            {
            }
    else
        {
        double dMin = DBL_MAX;
        TryMinDistXY (dMin, 0.0, segmentA.point[0], segmentB, fractionA, pointA, fractionB, pointB);
        TryMinDistXY (dMin, 1.0, segmentA.point[1], segmentB, fractionA, pointA, fractionB, pointB);
        TryMinDistXY (dMin, 0.0, segmentB.point[0], segmentA, fractionB, pointB, fractionA, pointA);
        TryMinDistXY (dMin, 1.0, segmentB.point[1], segmentA, fractionB, pointB, fractionA, pointA);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool DSegment3d::IsAlmostEqual (DSegment3d other, double tolerance) const
    {
    return DPoint3dOps::AlmostEqual (point[0], other.point[0], tolerance)
        && DPoint3dOps::AlmostEqual (point[1], other.point[1], tolerance)
        ;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool DSegment3d::IsAlmostSinglePoint () const
    {
    return point[0].AlmostEqual (point[1]);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool DSegment3d::IsSinglePoint () const
    {
    return point[0].IsEqual (point[1]);
    }


/*---------------------------------------------------------------------------------**//**
* Find the closest approach of two (unbounded) lines.
*
* @param [out] pParam0 parameter on first ray.
* @param [out] pParam1 parameter on second ray.
* @param [out] pPoint0 point on first ray.
* @param [out] pPoint1 point on second ray.
* @param [out] pSegment0 first segment
* @param [out] pSegment1 second segment
* @indexVerb closestApproach
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DSegment3d::ClosestApproachUnbounded
(
double          &param0,
double          &param1,
DPoint3dR point0,
DPoint3dR point1,
DSegment3dCR segment0,
DSegment3dCR segment1
)
    {
    return bsiDSegment3d_closestApproach
                (&param0, &param1, &point0, &point1, &segment0, &segment1);
    }

/*---------------------------------------------------------------------------------**//**
* Construct a segment tangent to two circles in xy plane.
* @bsihdr                                                       EarlinLutz      10/98
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDSegment3d  DSegment3d::ConstructTangent_CircleCircleXY
(
DPoint3dCR centerA,
double     radiusA,
DPoint3dCR centerB,
double     radiusB
)
    {
    DPoint2d uvA, uvB;
    bool outerTangents = radiusA * radiusB > 0.0;
    if (radiusA == 0.0 && radiusB == 0.0)
        return ValidatedDSegment3d (DSegment3d::From (centerA, centerB), true);
    if (DEllipse3d::ConstructTangentLineRatios (centerA.Distance (centerB), fabs (radiusA), fabs (radiusB), outerTangents, uvA, uvB))
        {
        double signA = radiusA > 0.0 ? 1.0 : -1.0;
        double signB = radiusB > 0.0 ? 1.0 : -1.0;
        if (radiusA == 0.0)
            signB = -signB;
        else if (!outerTangents)
            signB = -signB;
        return ValidatedDSegment3d (
                DSegment3d::From
                    (
                    DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvA.x, centerB, signA * uvA.y),
                    DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvB.x, centerB, signB * uvB.y)
                    ),
                true
                );
        }
    return ValidatedDSegment3d (DSegment3d::From (centerA, centerB), false);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
