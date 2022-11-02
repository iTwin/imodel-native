/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*-----------------------------------------------------------------*//**
* Reutrn world-to-local and local-to-world transforms for translating to/from
* a local origin.  The local origin is taken as the 3d (cartesian) normalized
* image of a 4D point.
* @param pWorldToLocal OUT transformation from world to local coordiantes
* @param pLocalToWorld OUT transformation from local to world coordinates
* @param pPoint IN Homogeneous origin.
* @return false if homogeneous origin cannot be normalized to xyz.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool    bsiDMatrix4d_initForDPoint4dOrigin

(
DMatrix4dP   pWorldToLocal,
DMatrix4dP   pLocalToWorld,
DPoint4dCP pPoint
)
    {
    DPoint3d xyz;
    bool    stat = pPoint->GetProjectedXYZ (xyz);

    if (pWorldToLocal)
        pWorldToLocal->InitIdentity ();

    if (pLocalToWorld)
        pLocalToWorld->InitIdentity ();

    if (stat)
        {
        if (pWorldToLocal)
            {
            pWorldToLocal->coff[0][3] = -xyz.x;
            pWorldToLocal->coff[1][3] = -xyz.y;
            pWorldToLocal->coff[2][3] = -xyz.z;
            }

        if (pLocalToWorld)
            {
            pLocalToWorld->coff[0][3] = xyz.x;
            pLocalToWorld->coff[1][3] = xyz.y;
            pLocalToWorld->coff[2][3] = xyz.z;
            }
        }
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* Get DPoint4d plane coefficients which represent the (perspsective, i.e. normalized weight)
* view of the line as a plane.  This vector has z=0 so that z parts of other geometry
* are irrelevant to intersection calculations
* @param        start point of object.
* @indexVerb get
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void       bsiDSegment4d_getXYWImplicitDPoint4dPlane

(
DSegment4dCP pInstance,
DPoint4dP pPlaneCoffs
)
    {
    DPoint3d xyw0, xyw1, cross;
    double a, b;
    pInstance->point[0].GetXYW (xyw0);
    pInstance->point[1].GetXYW (xyw1);

    cross.CrossProduct (xyw0, xyw1);
    /* Try to normalize.  If fail, just pass along original data */
    a = sqrt (cross.x * cross.x + cross.y * cross.y);
    DoubleOps::SafeDivide (b, 1.0, a, 1.0);

    pPlaneCoffs->x = b * cross.x;
    pPlaneCoffs->y = b * cross.y;
    pPlaneCoffs->z = 0.0;
    pPlaneCoffs->w = b * cross.z;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DSegment4d::Init
(
double            x0,
double            y0,
double            z0,
double            x1,
double            y1,
double            z1
)
    {
    point[0].Init (x0, y0, z0, 1.0);
    point[1].Init (x1, y1, z1, 1.0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DSegment4d::Init
(
double  x0,
double  y0,
double  z0,
double  w0,
double  x1,
double  y1,
double  z1,
double  w1
)
    {
    point[0].Init (x0, y0, z0, w0);
    point[1].Init (x1, y1, z1, w1);    
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DSegment4d::Init (DSegment3dCR source)
    {
    point[0].Init (source.point[0], 1.0);
    point[1].Init (source.point[1], 1.0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DSegment4d::Init (DPoint3dCR point0, DPoint3dCR point1)
    {
    point[0].Init (point0, 1.0);
    point[1].Init (point1, 1.0);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DSegment4d::Init (DPoint4dCR point0, DPoint4dCR point1)
    {
    point[0] = point0;
    point[1] = point1;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DSegment4d::FractionParameterToPoint (DPoint4dR outPoint, double fraction) const
    {
    outPoint.Interpolate (point[0], fraction, point[1]);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DSegment4d::ProjectDPoint4dCartesianXYW
(
DPoint4dR closestPoint,
double      &closestParam,
DPoint4dCR spacePoint
) const
    {
    DPoint4d vectorU;
    DVec3d diffPA, diffUP, vectorUBar;

    double dot0, dot1;
    bool    result;
    
    vectorU.DifferenceOf (point[1], point[0]);
    
    
    vectorUBar.WeightedDifferenceOf (point[1], point[0]);

    diffPA.WeightedDifferenceOf (spacePoint, point[0]);
    diffUP.WeightedDifferenceOf (vectorU, spacePoint);

    dot0 = diffPA.DotProductXY (vectorUBar);
    dot1 = diffUP.DotProductXY (vectorUBar);
    result = DoubleOps::SafeDivide (closestParam, dot0, dot1, 0.0);
    closestPoint.SumOf (point[0], vectorU, closestParam);
    return result;
    }


/*---------------------------------------------------------------------------------**//**
* @description Return a segment defined by its endpoints.
*
+---------------+---------------+---------------+---------------+---------------+------*/

DSegment4d DSegment4d::From 
(
DPoint4dCR pointA, 
DPoint4dCR pointB
)
    {
    DSegment4d segment;
    segment.Init (pointA, pointB);
    return segment;
    }

DSegment4d DSegment4d::From
(
DPoint3dCR pointA, 
DPoint3dCR pointB
)
    {
   DSegment4d segment;
   segment.Init (pointA.x, pointA.y, pointA.z, pointB.x, pointB.y, pointB.z);
   return segment;
    }

DSegment4d DSegment4d::From (double xA, double yA, double zA, double wA, double xB, double yB, double zB, double wB)
    {
   DSegment4d segment;
   segment.Init (xA, yA, zA, wA, xB, yB, zB, wB);
   return segment;
    }


DSegment4d DSegment4d::From (double xA, double yA, double zA, double xB, double yB, double zB)
    {
   DSegment4d segment;
   segment.Init (xA, yA, zA, 1.0, xB, yB, zB, 1.0);
   return segment;
    }

DSegment4d DSegment4d::From (DSegment3dCR segment)
    {
    DPoint3d       point0;
    DPoint3d       point1;
    DSegment4d      segment4;

    segment.GetEndPoints(point0, point1);
    
    segment4.Init(point0, point1);
    return segment4;
    }

//DSegment4d DSegment4d::FromFractionInterval (DSegment4d parent, double startFraction, double endFraction)
//     {
//     DSegment4d segment;
//
//    parent.FractionParameterToPoint (segment.point[0], startFraction);
//    parent.FractionParameterToPoint (segment.point[1], endFraction);
//
//    return segment;
//     }
     
     

bool DSegment4d::GetEndPoints (DPoint3dR point0, DPoint3dR point1) const
    {
    auto stat0 = point[0].GetProjectedXYZ(point0);
    auto stat1 = point[1].GetProjectedXYZ(point1);
    return stat0 && stat1;
    }

bool DSegment4d::GetRange (DRange3dR range3d) const
    {
    range3d.Init ();
    range3d.Extend (point, 2);
    return true;
    }

void DSegment4d::InitProduct (TransformCR transform, DSegment4dCR source)
    {
    transform.Multiply (this->point, source.point, 2);
    }

void DSegment4d::InitProduct (DMatrix4dCR mat, DSegment4dCR source)
    {
    mat.Multiply (this->point, source.point, 2);
    }
    
void DSegment4d::GetStartPoint (DPoint3dR xyz) const
    {
    point[0].GetProjectedXYZ (xyz);
    }
    
void DSegment4d::GetEndPoint (DPoint3dR xyz) const
    {
    point[1].GetProjectedXYZ (xyz);
    }
    
DSegment4d DSegment4d::FromFractionInterval (DSegment4d parent, double startFraction, double endFraction)
    {
      if (DoubleOps::IsExact01 (startFraction, endFraction))
        return parent; 
      point[0].Interpolate(parent.point[0], startFraction, parent.point[1]);
      point[1].Interpolate(parent.point[0], endFraction, parent.point[1]);
      DSegment4d segment = DSegment4d::From (point[0], point[1]);
      return segment;
    }
    
DPoint4d DSegment4d::FractionParameterToPoint(double fraction) const
    {
    DPoint4d pnt;
    pnt.Interpolate (this->point[0], fraction, this->point[1]);
    return pnt;
    }

bool DSegment4d::FractionParameterToPoint(DPoint3d &pnt, double fraction) const
    {
    DPoint4d pnt4d = DPoint4d::From (pnt, 1);
    pnt4d.Interpolate (this->point[0], fraction, this->point[1]);
    return pnt4d.GetProjectedXYZ(pnt);
    }

bool DSegment4d::PointToFractionParameter (double &fraction, DPoint3d spacePoint) const
    {
    DPoint4d directionU, direction0, direction1;
    
    directionU.WeightedDifferenceOf (point[1], point[0]);
    direction0.WeightedDifferenceOf (point[0], spacePoint, 1.0);
    direction1.WeightedDifferenceOf (point[1], spacePoint, 1.0);
    double dot0 = direction0.DotProduct (directionU);
    double dot1 = direction1.DotProduct (directionU);
    return DoubleOps::SafeDivideParameter (fraction, -dot0, dot1 - dot0, 0.0);
    }
   
bool DSegment4d::FractionParameterToTangent (DPoint3d &spacepoint, DVec3dR tangent, double param) const
    {
    DPoint4d point4d = DPoint4d::From (spacepoint, 1);
    point4d.Interpolate (this->point[0], param, this->point[1]);
    spacepoint = DPoint3d::From (point4d.x, point4d.y, point4d.z);

    tangent.WeightedDifferenceOf (this->point[1], this->point[0]);

    return true;
    }
    
DPoint4d DSegment4d::FractionParameterToTangent  (DPoint4d spacepoint, DPoint4d &tangent, double param) const
    {
    spacepoint.Interpolate (this->point[0], param, this->point[1]);
    tangent.DifferenceOf (this->point[1], this->point[0]);
    
    return spacepoint;
    }
    
bool DSegment4d::FractionToLength (double &arcLength, double fraction0, double fraction1) const
    {
    DPoint4d pnt0;
    pnt0.Interpolate (this->point[0], fraction0, this->point[1]);
    DPoint4d pnt1;
    pnt1.Interpolate (this->point[0], fraction1, this->point[1]);
    arcLength = pnt1.RealDistance (pnt0);
    return true;
    
    /*DPoint4d pnt0 = this->FractionParameterToPoint (fraction0);
    DPoint4d pnt1 = this->FractionParameterToPoint (fraction1);*/
    }
    
bool DSegment4d::LengthToFraction (double &fraction1, double fraction0, double arcStep) const
    {
    DPoint4d vectorU;
    DVec3d w1, w2;
    vectorU.DifferenceOf (this->point[1], this->point[0]);
    double f1;
    DPoint4d P0 = this->point[0];
    DPoint3d A;
    this->FractionParameterToPoint (A, fraction0);
    DPoint4d A4d = DPoint4d::From (A, 1);
    
    w1.WeightedDifferenceOf (P0, A4d);
    w2.WeightedDifferenceOf (vectorU, A4d); 
    
    double w1sq = w1.DotProduct (w1);
    double w2sq = w2.DotProduct (w2);
    double w1w2 = w1.DotProduct (w2);
    double arcLsq = arcStep * arcStep;
    double deltaw = this->point[1].w - this->point[0].w;
    
    double a = w2sq - arcLsq*deltaw*deltaw;
    double b = 2.0 * (w1w2 - arcLsq*(P0.w)*(deltaw));
    double c = w1sq - arcLsq*P0.w*P0.w;
    
    double signterm = b*b - 4*a*c;
    
    if (signterm<0)
        {
        return false;
        }
    
    double quadnump = -b + sqrt(signterm);
    //varunused double quadnumn = -b - sqrt(signterm);
    double quadden = 2*a;
    bool result = DoubleOps::SafeDivide (f1, quadnump, quadden, 0.0);
    fraction1 = f1;
    return result;
    }    

bool DSegment4d::ClosestPointBoundedXY (DPoint3d &closePoint, double &closeParam, double &distanceXY, DPoint3d spacePoint, DMatrix4dCP worldToLocal, bool extend0, bool extend1) const
    {
    DPoint4d closePoint4d;
    DPoint4d spacePoint4d;
    spacePoint4d.InitFrom (spacePoint, 1.0);
    DSegment4d xySegment = *this;
    if (worldToLocal != NULL)
        {
        worldToLocal->Multiply (xySegment.point, xySegment.point, 2);
        worldToLocal->Multiply (spacePoint4d, spacePoint4d);
        }
        
    if (xySegment.ProjectDPoint4dCartesianXYW (closePoint4d, closeParam, spacePoint4d))
        {
        if (closeParam > 1.0 && !extend1)
            {
            closeParam = 1.0;
            closePoint4d = this->point[1];
            }
        else if (closeParam < 0.0 && !extend0)
            {
            closeParam = 0.0;
            closePoint4d = this->point[0];            
            }
        FractionParameterToPoint (closePoint, closeParam);
        double testDistance;
        spacePoint4d.RealDistanceXY (testDistance, closePoint4d);
        if (spacePoint4d.RealDistanceXY (distanceXY, closePoint4d))
            return true;
        }
        
    closeParam = 0.0;
    point[0].GetProjectedXYZ (closePoint);
    return false;
    }

bool DSegment4d::ClosestPointBoundedXY (DPoint3d &closePoint, double &closeParam, double &distanceXY, DPoint3d spacePoint, DMatrix4dCP worldToLocal) const
    {
    return ClosestPointBoundedXY (closePoint, closeParam, distanceXY, spacePoint, worldToLocal, false, false);
    }
    
bool DSegment4d::ProjectPoint (DPoint3d &closestPoint, double &f, DPoint3d spacePoint) const
    {
    bool proj = this->PointToFractionParameter (f, spacePoint);
    FractionParameterToPoint (closestPoint, f);
    return proj;
    }

bool DSegment4d::ProjectPointBounded (DPoint3dR closestPoint, double &f, DPoint3d spacePoint) const
    {
    bool simpleProjection = this->PointToFractionParameter (f, spacePoint);
    if (!simpleProjection)
        f = 0.0;
    else if (f < 0.0)
        f = 0.0;
    else if (f > 1.0)
        f = 0.0;
    FractionParameterToPoint (closestPoint, f);
    return simpleProjection;
    }


static double bsiDPoint4d_dotProductAbsValues


(
DPoint4dCP pPoint0,
DPoint4dCP pPoint1
)
    {
    return fabs (pPoint0->x * pPoint1->x)
        +  fabs (pPoint0->y * pPoint1->y)
        +  fabs (pPoint0->z * pPoint1->z)
        +  fabs (pPoint0->w * pPoint1->w);
    }


static bool ExtractCartesianRay
(
DSegment4dCR segment,
DRay3dR ray,
DPoint3dR endPoint
)
    {
    if (segment.point[0].w == 1.0
        && segment.point[1].w == 1.0)
        {
        ray.origin.x = segment.point[0].x;
        ray.origin.y = segment.point[0].y;
        ray.origin.z = segment.point[0].z;
        endPoint.x = segment.point[1].x;
        endPoint.y = segment.point[1].y;
        endPoint.z = segment.point[1].z;
        ray.direction.x = endPoint.x - ray.origin.x;
        ray.direction.y = endPoint.y - ray.origin.y;
        ray.direction.z = endPoint.z - ray.origin.z;
        return true;
        }
    return false;
    }





/*-----------------------------------------------------------------*//**
* Compute the parameters and points where the xy projections of two rays intersect.
*
* @param pPoint0 <= intersection point on line 0.
* @param s0P <= parametric coordinate on segment 0
* @param pPoint1 <= intesection point on line 1.
* @param s1P <= parametric coordinate on segment 1
* @param pStart0 => start of first line segment
* @param pEnd0 => end of first line
* @param pStart1 => start of second segment
* @param pEnd1 => end of second segment
* @see
* @return true unless lines are parallel
* @indexVerb intersection
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
bool DSegment4d::IntersectXY
(
DPoint4dR   point01,
double      &param01,
DPoint4dR   point23,
double      &param23,
DSegment4dCR inSegment01,
DSegment4dCR inSegment23
)
    {
    // If it's all cartesian do the simplest thing ...
    DRay3d ray01, ray23;
    DPoint3d point1, point3;
    if (    ExtractCartesianRay (inSegment01, ray01, point1)
        && ExtractCartesianRay (inSegment23, ray23, point3))
        {
        DPoint3d xyz01, xyz23;
        bool stat = bsiGeom_intersectXYRays
                            (
                            &xyz01, &param01,
                            &xyz23, &param23,
                            &ray01.origin, &ray01.direction,
                            &ray23.origin, &ray23.direction
                            );
        if (!stat)
            return false;
        // recompute at maximum accuracy ...
        xyz01.Interpolate (ray01.origin, param01, point1);
        xyz23.Interpolate (ray23.origin, param23, point3);
        point01.Init (xyz01, 1.0);
        point23.Init (xyz23, 1.0);
        return true;
        }

    DPoint4d plane01, plane23;
    double h0, h1, h2, h3;
    bool    b0, b1;
    double a01, a23;
    double d01, d23;
    double dummy0, dummy1;
    DMatrix4d worldToLocal, localToWorld;

    DSegment4d segment01;
    DSegment4d segment23;

    bsiDMatrix4d_initForDPoint4dOrigin (&worldToLocal, &localToWorld, &inSegment01.point[0]);

    segment01.InitProduct (worldToLocal, inSegment01);
    segment23.InitProduct (worldToLocal, inSegment23);

    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&segment01, &plane01);
    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&segment23, &plane23);

    h0 = segment01.point[0].DotProduct (plane23);
    h1 = segment01.point[1].DotProduct (plane23);

    h2 = segment23.point[0].DotProduct (plane01);
    h3 = segment23.point[1].DotProduct (plane01);

    // h0,h1,h2,h3 are (scaled) heights of endpoints above planes.
    // To get a sense of what is a "big" number at this scale, recompute
    // with absolute values imposed on all the altitude terms.   Height
    //  differences are then significant only if large compared to the big number.
    a01 = bsiDPoint4d_dotProductAbsValues (&segment01.point[0], &plane23)
        + bsiDPoint4d_dotProductAbsValues (&segment01.point[1], &plane23);

    a23 = bsiDPoint4d_dotProductAbsValues (&segment23.point[0], &plane01)
        + bsiDPoint4d_dotProductAbsValues (&segment23.point[1], &plane01);

    d01 = h1 - h0;
    d23 = h3 - h2;

    b0 =  DoubleOps::SafeDivide (param01, - h0, d01, 0.0)
       && DoubleOps::SafeDivide (dummy0, a01, d01, 0.0);

    b1 =  DoubleOps::SafeDivide (param23, - h2, d23, 0.0)
       && DoubleOps::SafeDivide (dummy1, a23, d23, 0.0);

    /* Use original (untransformed) segments for coordinate calculations. */
    point01 = inSegment01.FractionParameterToPoint (param01);
    point23 = inSegment23.FractionParameterToPoint (param23);

    return b0 && b1;
    }


/*-----------------------------------------------------------------*//**
* Compute the parameters and points where the xy projections of two rays intersect.
* Only return points in 0..1 parameter range.  Output parameters are
* untouched (undefined) if no intersections occur in range.
*
* @param pPoint0 <= intersection point on line 0.
* @param s0P <= parametric coordinate on segment 0
* @param pPoint1 <= intesection point on line 1.
* @param s1P <= parametric coordinate on segment 1
* @param pStart0 => start of first line segment
* @param pEnd0 => end of first line
* @param pStart1 => start of second segment
* @param pEnd1 => end of second segment
* @see
* @return true unless lines are parallel
* @indexVerb intersection
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
bool DSegment4d::IntersectXYBounded
(
DPoint4dR   point01,
double      &param01,
DPoint4dR   point23,
double      &param23,
DSegment4dCR inSegment01,
DSegment4dCR inSegment23
)

    {
    DPoint4d plane01, plane23;
    double h0, h1, h2, h3;
    bool    boolstat = false;

    /* This could be implemented by calling the unbounded intersector and checking
        parameters.  However, looking directly at the altitudes allows no-intersect
        cases to be filtered with no divisions. */

    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&inSegment01, &plane01);
    bsiDSegment4d_getXYWImplicitDPoint4dPlane (&inSegment23, &plane23);

    h0 = inSegment01.point[0].DotProduct (plane23);
    h1 = inSegment01.point[1].DotProduct (plane23);

    if (h0 * h1 <= 0.0)
        {
        h2 = inSegment23.point[0].DotProduct (plane01);
        h3 = inSegment23.point[1].DotProduct (plane01);

        if (h2 * h3 <= 0.0
            && DoubleOps::SafeDivide (param01, - h0, h1 - h0, 0.0)
            && DoubleOps::SafeDivide (param23, - h2, h3 - h2, 0.0))
            {
            point01 = inSegment01.FractionParameterToPoint (param01);
            point23 = inSegment23.FractionParameterToPoint (param23);
            boolstat = true;
            }
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Project a point onto the extended, cartesian line using only xyw parts of the line.
*
* @indexVerb projection
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool               DSegment4d::ProjectPointUnboundedCartesianXYW
(
DPoint4dR closestPoint,
double    &closestParam,
DPoint4dCR   spacePoint
) const
    {
    DPoint4d vectorU;
    DPoint3d diffPA, diffUP, vectorUBar;

    double dot0, dot1, param;
    bool    result;

    vectorU.DifferenceOf (point[1], point[0]);
    vectorUBar.WeightedDifferenceOf(point[1], point[0]);
    diffPA.WeightedDifferenceOf(spacePoint, point[0]);
    diffUP.WeightedDifferenceOf(vectorU, spacePoint);

    dot0 = diffPA.DotProductXY (vectorUBar);
    dot1 = diffUP.DotProductXY (vectorUBar);
    result = DoubleOps::SafeDivide (param, dot0, dot1, 0.0);

    closestParam = param;

    closestPoint.SumOf(point[0], vectorU, param);
    return result;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
