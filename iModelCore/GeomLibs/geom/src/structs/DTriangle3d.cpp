/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DTriangle3d::DTriangle3d
(
DPoint3dCR xyz00,
DPoint3dCR xyz10,
DPoint3dCR xyz01
)
    {
    point[0] = xyz00;
    point[1] = xyz10;
    point[2] = xyz01;
    }


DTriangle3d::DTriangle3d
(
DPoint2dCR xyz00,
DPoint2dCR xyz10,
DPoint2dCR xyz01
)
    {
    point[0].Init (xyz00);
    point[1].Init (xyz10);
    point[2].Init (xyz01);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DTriangle3d::DTriangle3d ()
    {
    point[0].Zero ();
    point[1].Zero ();
    point[2].Zero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DTriangle3d::GetVectorsFromOrigin (DVec3dR vectorU, DVec3dR vectorV) const
    {
    vectorU.DifferenceOf (point[1], point[0]);
    vectorV.DifferenceOf (point[2], point[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DTriangle3d::Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const
    {
    GetVectorsFromOrigin (dXdu, dXdv);
    xyz.SumOf (point[0], dXdu, u, dXdv, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DTriangle3d::EvaluateNormal (double u, double v, DPoint3dR xyz, DVec3dR unitNormal) const
    {
    DVec3d dXdu, dXdv;
    GetVectorsFromOrigin (dXdu, dXdv);
    unitNormal.NormalizedCrossProduct (dXdu, dXdv);
    xyz.SumOf (point[0], dXdu, u, dXdv, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ValidatedDRay3d DTriangle3d::EvaluateUnitNormalRay (double u, double v)  const
    {
    DVec3d dXdu, dXdv;
    GetVectorsFromOrigin (dXdu, dXdv);
    DRay3d ray;
    ray.direction.CrossProduct (dXdu, dXdv);
    ray.origin.SumOf (point[0], dXdu, u, dXdv, v);
    double magnitude;
    bool valid = ray.direction.TryNormalize (ray.direction, magnitude);
    return ValidatedDRay3d (ray, valid);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DVec3d DTriangle3d::CrossVectorsFromOrigin () const
    {
    DVec3d dXdu, dXdv;
    GetVectorsFromOrigin (dXdu, dXdv);
    return DVec3d::FromCrossProduct (dXdu, dXdv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3d DTriangle3d::Evaluate (double u, double v) const
    {
    DPoint3d xyz;
    DVec3d dXdu, dXdv;
    GetVectorsFromOrigin (dXdu, dXdv);
    xyz.SumOf (point[0], dXdu, u, dXdv, v);
    return xyz;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3d DTriangle3d::EvaluateBarycentric (double u, double v, double w) const
    {
    return DPoint3d::FromSumOf (point[0], u, point[1], v, point[2], w);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3d DTriangle3d::EvaluateBarycentric (DPoint3dCR uvw) const
    {
    return DPoint3d::FromSumOf (point[0], uvw.x, point[1], uvw.y, point[2], uvw.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DTriangle3d::ClosestPointUnbounded
(
DPoint3dCR spacePoint,
DPoint2d &uv
) const
    {
    DVec3d vectorU, vectorV;     
    GetVectorsFromOrigin (vectorU, vectorV);
    DVec3d vectorQ = DVec3d::FromStartEnd (point[0], spacePoint);
    bool stat = vectorQ.ProjectToPlane (vectorU, vectorV, uv);
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DTriangle3d::ClosestPointUnbounded
(
DPoint3dCR spacePoint,
DPoint2d &uv,
DPoint3d &xyz
) const
    {
    DVec3d vectorU, vectorV;     
    GetVectorsFromOrigin (vectorU, vectorV);
    DVec3d vectorQ = DVec3d::FromStartEnd (point[0], spacePoint);
    bool stat = vectorQ.ProjectToPlane (vectorU, vectorV, uv);
    xyz.SumOf (point[0], vectorU, uv.x, vectorV, uv.x);
    return stat;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DSegment3d DTriangle3d::GetCCWEdgeDSegment3d (int i) const
    {
    return DSegment3d::From (point[Angle::Cyclic3dAxis (i)], point[Angle::Cyclic3dAxis (i+1)]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DVec3d DTriangle3d::GetCCWEdgeDVec3d (int i) const
    {
    return DVec3d::FromStartEnd (point[Angle::Cyclic3dAxis (i)], point[Angle::Cyclic3dAxis (i+1)]);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DRay3d DTriangle3d::GetCCWEdgeDRay3d (int i) const
    {
    return DRay3d::FromOriginAndTarget (point[Angle::Cyclic3dAxis (i)], point[Angle::Cyclic3dAxis (i+1)]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DTriangle3d::AspectRatio () const
    {
    DVec3d vectorU, vectorV;     
    GetVectorsFromOrigin (vectorU, vectorV);
    DVec3d cross = DVec3d::FromCrossProduct (vectorU, vectorV);
    double a = cross.Magnitude ();
    double b = vectorU.MagnitudeSquared () + vectorV.MagnitudeSquared () + vectorU.DistanceSquared (vectorV);
    double c;
    DoubleOps::SafeDivideParameter (c, a, b, 0.0);
    return c;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DTriangle3d::IsPointInOrOnXY (DPoint3dCR xyz) const
    {
    return PolygonOps::IsPointInOrOnXYTriangle (xyz, point[0], point[1], point[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DTriangle3d::IsBarycentricInteriorUV (double u, double v)
    {
    return u >= 0.0 && v >= 0.0 && u + v <= 1.0;
    }

bool DTriangle3d::TransverseIntersection
(
DRay3dCR     ray,
DPoint3dR    xyzIntersection,
DPoint3dR    uvwIntersection,
double       &rayParameter
) const
    {
    DVec3d vector1, vector2;
    GetVectorsFromOrigin (vector1, vector2);
    DVec3d vectorA = ray.origin - point[0];
    DVec3d uvq;
    RotMatrix Q = RotMatrix::FromColumnVectors (vector1, vector2, ray.direction);
    RotMatrix QI;
    if (QI.InverseOf (Q))
        {
        uvq = QI * vectorA;
        uvwIntersection.Init (1.0 - uvq.x - uvq.y, uvq.x, uvq.y);
        rayParameter = - uvq.z;
        xyzIntersection = Evaluate (uvq.x, uvq.y);
        return true;
        }
    xyzIntersection = point[0];
    uvwIntersection.Zero ();
    rayParameter = 0.0;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d::DPoint3dDVec3dDVec3d (DPoint3dCR _origin, DVec3dCR _vectorU, DVec3dCR _vectorV)
    : origin (_origin), vectorU(_vectorU), vectorV (_vectorV)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d::DPoint3dDVec3dDVec3d (
double ax, double ay, double az,
double ux, double uy, double uz,
double vx, double vy, double vz
)
    {
    origin.x = ax; origin.y = ay; origin.z = az;
    vectorU.x = ux; vectorU.y = uy; vectorU.z = uz;
    vectorV.x = vx; vectorV.y = vy; vectorV.z = vz;
    }
DPoint3dDVec3dDVec3d::DPoint3dDVec3dDVec3d (DEllipse3dCR ellipse)
    : origin (ellipse.center), vectorU(ellipse.vector0), vectorV (ellipse.vector90)
    {
    }	
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d::DPoint3dDVec3dDVec3d ()
    : origin (DPoint3d::From (0,0,0)), vectorU(DVec3d::From (1,0,0)), vectorV (DVec3d::From (0,1,0))
    {
    }    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3dDVec3dDVec3d::Evaluate (double u, double v) const
    {
    DPoint3d xyz;
    xyz.SumOf (origin, vectorU, u, vectorV, v);
    return xyz;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DVec3d DPoint3dDVec3dDVec3d::EvaluateVectorOnly (double u, double v) const
    {
    return DVec3d::FromSumOf (vectorU, u, vectorV, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3dDVec3dDVec3d::Evaluate (DPoint2dCR uv) const
    {
    DPoint3d xyz;
    xyz.SumOf (origin, vectorU, uv.x, vectorV, uv.y);
    return xyz;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DVec3d DPoint3dDVec3dDVec3d::EvaluateVectorOnly (DPoint2dCR uv) const
    {
    return DVec3d::FromSumOf (vectorU, uv.x, vectorV, uv.y);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::EvaluateTangents (double u, double v) const
    {
    return DPoint3dDVec3dDVec3d (DPoint3d::FromSumOf (origin, vectorU, u, vectorV, v), vectorU, vectorV);
    }
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DPoint3dDVec3dDVec3d::MaxDiff (DPoint3dDVec3dDVec3dCR other) const
    {
    return DoubleOps::Max (origin.MaxDiff (other.origin), vectorU.MaxDiff (other.vectorU), vectorV.MaxDiff (other.vectorV));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndParallelToXY (DPoint3d origin, double sizeU = 1.0, double sizeV = 1.0)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::From (sizeU, 0,0), DVec3d::From (0, sizeV, 0));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndParallelToYZ (DPoint3d origin, double sizeU = 1.0, double sizeV = 1.0)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::From (0, sizeU, 0), DVec3d::From (0, 0, sizeV));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndParallelToXZ (DPoint3d origin, double sizeU = 1.0, double sizeV = 1.0)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::From (sizeU, 0, 0), DVec3d::From (0, 0, sizeV));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromXYPlane ()
    {
    return DPoint3dDVec3dDVec3d (
        0,0,0,
        1,0,0,
        0,1,0
        );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndTargets (DPoint3dCR origin, DPoint3dCR UTarget, DPoint3dCR VTarget)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::FromStartEnd (origin, UTarget), DVec3d::FromStartEnd (origin, VTarget));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromXZPlane ()
    {
    return DPoint3dDVec3dDVec3d (
        0,0,0,
        1,0,0,
        0,0,1
        );
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromYZPlane ()
    {
    return DPoint3dDVec3dDVec3d (
        0,0,0,
        0,1,0,
        0,0,1
        );
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool DPoint3dDVec3dDVec3d::GetTransformsUnitZ (TransformR localToWorld, TransformR worldToLocal) const
    {
    auto unitNormal = DVec3d::FromCrossProduct (vectorU, vectorV).ValidatedNormalize ();
    localToWorld.InitFromOriginAndVectors (origin, vectorU, vectorV, unitNormal);
    return worldToLocal.InverseOf (localToWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
//! Compute the simple intersection of a segment with the plane.
//! Returns false in any parallel case (both in-plane and off-plane)
bool DPoint3dDVec3dDVec3d::TransverseIntersection (DSegment3dCR segment, DPoint2dR uv, double &segmentFraction) const
    {
    RotMatrix matrix = RotMatrix::FromColumnVectors (vectorU, vectorV, DVec3d::FromStartEnd (segment.point[0], segment.point[1]));
    DVec3d uvw;
    DVec3d rhs = DVec3d::FromStartEnd (origin, segment.point[0]);
    if (matrix.Solve (uvw, rhs))
        {
        uv.x = uvw.x;
        uv.y = uvw.y;
        segmentFraction = -uvw.z;
        return true;
        }
    segmentFraction = 0.0;
    uv.Zero ();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ValidatedDPoint2d DPoint3dDVec3dDVec3d::ProjectPointToUV (DPoint3dCR xyz) const
    {
    DVec3d vectorQ = DVec3d::FromStartEnd (origin, xyz);
    DPoint2d uv;
    if (vectorQ.ProjectToPlane (vectorU, vectorV, uv))
        return ValidatedDPoint2d (uv, true);
    else
        return ValidatedDPoint2d (DPoint2d::From (0,0), false);

    }
// If source is valid return its ValidatedInverse.
// Otherwise return transform that just subtracts out the given origin (and is marked invalid)
static ValidatedTransform ConditionalInvert (ValidatedTransform const &source, DPoint3dCR origin)
    {
    if (source.IsValid ())
        return source.Value ().ValidatedInverse ();
    return ValidatedTransform (
            Transform::From (-origin.x, -origin.y, -origin.z),
            false);
    }
//! return a coordinate frame with normalized X axis along vectorU, normalized Y axis in plane of vectorV and vectorV, unit vector perpendicular.
ValidatedTransform DPoint3dDVec3dDVec3d::NormalizedLocalToWorldTransform () const
    {
    Transform localToWorld;
    bool stat = localToWorld.InitFromOriginXVectorYVectorSquareAndNormalize (origin, vectorU, vectorV);
    return ValidatedTransform (localToWorld, stat);
    }

//! return a transform into the NormalizedLocal frame.
ValidatedTransform DPoint3dDVec3dDVec3d::WorldToNormalizedLocalTransform () const
    {
    return ConditionalInvert (NormalizedLocalToWorldTransform (), origin);
    }

//! Return a transform with the full length VectorU and VectorV axes as X and Y axes, unit vector perpendicular.
ValidatedTransform DPoint3dDVec3dDVec3d::LocalToWorldTransform () const
    {
    auto unitNormal = DVec3d::FromCrossProduct (vectorU, vectorV).ValidatedNormalize ();
    if(unitNormal.IsValid ())
        return ValidatedTransform (Transform::FromOriginAndVectors (origin, vectorU, vectorV, unitNormal), true);
    // Maybe one of the vectors is 0.  Maybe they are nonzero but parallel....get a non-canceling sum 
    double a = vectorU.DotProduct (vectorV) > 0.0 ? 1.0 : -1.0;
    DVec3d vector = DVec3d::FromSumOf (vectorU, vectorV, a);
    DVec3d xAxis, yAxis, zAxis;
    vector.GetNormalizedTriad (yAxis, zAxis, xAxis); // that puts zAxis along the vector . . 
    return ValidatedTransform (Transform::FromOriginAndVectors (origin, vectorU, vectorV, zAxis), false);
    }

//! Return a the inverse of the LocalToWorldTransform.
ValidatedTransform DPoint3dDVec3dDVec3d::WorldToLocalTransform () const
    {
    return ConditionalInvert (LocalToWorldTransform (), origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
UVBoundarySelect::UVBoundarySelect (int selector) : m_selector (selector) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool UVBoundarySelect::IsInOrOn (double u, double v) const
    {
    switch (m_selector)
        {
        case Triangle: return u >= 0.0 && v >= 0.0 && u + v <= 1.0;
        case UnitSquare:   return u >= 0.0 && v >= 0.0 && u <= 1.0 && v <= 1.0;
        case CenteredSquare:   return u >= -1.0 && v >= -1.0 && u <= 1.0 && v <= 1.0;
        case UnitCircle: return u*u + v*v <= 1.0;
        }
    return true;
    }

bool UVBoundarySelect::IsInOrOn (DPoint2dCR uv) const {return IsInOrOn (uv.x, uv.y);}

/*----------------------------------------------------------------------------+
| BARYCENTRIC COORDINATE FUNCTIONS:
|
| For a given triangle T with vertices v0, v1, v2, every point q in the plane
| of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
| relative to T:
|
| q = b0 * v0 + b1 * v1 + b2 * v2,
| 1 = b0 + b1 + b2.
|
+----------------------------------------------------------------------------*/
static bool    barycentricFromDPoint2dTriangleVectors
(
DPoint3dP pInstance,
double    *pArea,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    bool        status = true;
    double      denom, numer1, numer2;

    /*
    Let vectors v1 and v2 emanating from the plane's origin define triangle T,
    and let q be a point in the plane.  Then the system for finding the
    barycentric coordinates of q relative to T reduces from 3x3 to 2x2:
      -             - -    -   -     -      -            - -    -   -     -
      | 0 v1.x v2.x | | b0 |   | q.x |      | v1.x  v2.x | | b1 |   | q.x |
      | 0 v1.y v2.y | | b1 | = | q.y |  =>  | v1.y  v2.y | | b2 | = | q.y |
      | 1    1    1 | | b2 |   | 1   |      -            - -    -   -     -
      -             - -    -   -     -
    We use Cramer's Rule to solve this system for b1 and b2; b0 can be found
    by subtracting the sum of the other two coords from 1.0.
    */

    /* Calculate numerators of Cramer's Rule formulae */
    if (pOrigin)
        {
        /*
        Since barycoords are invariant under affine transformation, we can
        translate the triangle and point so that pOrigin is the origin.  This
        gives us the dimension reduction detailed above.
        */
        numer1 = (pPoint->x - pOrigin->x) * pVector2->y -
                 (pPoint->y - pOrigin->y) * pVector2->x;
        numer2 = (pPoint->y - pOrigin->y) * pVector1->x -
                 (pPoint->x - pOrigin->x) * pVector1->y;
        }
    else
        {
        numer1 = pPoint->x * pVector2->y - pPoint->y * pVector2->x;
        numer2 = pPoint->y * pVector1->x - pPoint->x * pVector1->y;
        }

    /*
    Calculate denominator of Cramer's Rule formulae.  On a good day, denom is
    twice the signed area of T.  On a bad day (i.e. when T is long and skinny)
    we get subtractive cancellation, but there's no way around it!
    */
    denom  = pVector1->x * pVector2->y - pVector2->x * pVector1->y;

    /* Return false and barycoords (1,0,0) if denom relatively close to zero */
    if (! DoubleOps::SafeDivide (pInstance->y, numer1, denom, 0.0))
        status = false;

    if (! DoubleOps::SafeDivide (pInstance->z, numer2, denom, 0.0))
        status = false;

    pInstance->x = 1.0 - pInstance->y - pInstance->z;

    if (pArea)
        *pArea = denom / 2.0;
    return status;
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public   bool    bsiDPoint3d_barycentricFromDPoint2dTriangle
(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;

    return barycentricFromDPoint2dTriangleVectors(pInstance, NULL, &q, NULL, &v1, &v2);
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint2dTriangle
(
DPoint3dR uvw,
DPoint3dR dUVWdX,
DPoint3dR dUVWdY,
double   &area,
DPoint2dCR point,
DPoint2dCR vertex0,
DPoint2dCR vertex1,
DPoint2dCR vertex2
)
    {
    DPoint2d        q, v1, v2;

    q.x  = point.x   - vertex0.x;
    q.y  = point.y   - vertex0.y;
    v1.x = vertex1.x - vertex0.x;
    v1.y = vertex1.y - vertex0.y;
    v2.x = vertex2.x - vertex0.x;
    v2.y = vertex2.y - vertex0.y;
    double divArea;
    if (barycentricFromDPoint2dTriangleVectors(&uvw, &area, &q, NULL, &v1, &v2)
        && DoubleOps::SafeDivide (divArea, 1.0, 2.0 * area, 0.0))
        {

        DVec2d edgeVector[3];
        edgeVector[0].DifferenceOf (vertex2, vertex1);  // vector opposite point0
        edgeVector[1].DifferenceOf (vertex0, vertex2);  // vector opposite point1
        edgeVector[2].DifferenceOf (vertex1, vertex0);  // vector opposite point2
        dUVWdX.Init (-edgeVector[0].y * divArea, -edgeVector[1].y * divArea, -edgeVector[2].y * divArea);
        dUVWdY.Init ( edgeVector[0].x * divArea,  edgeVector[1].x * divArea, edgeVector[2].x * divArea);
        return true;
        }
    uvw.Zero ();
    dUVWdX.Zero ();
    dUVWdY.Zero ();
    area = 0.0;
    return false;
    }

/*-----------------------------------------------------------------*//**
* @description Compute the minimum distance from a point to a triangle.
* @instance pSpacePoint   <= point in space
* @param pVertex0       => vertex of T
* @param pVertex1       => vertex of T
* @param pVertex2       => vertex of T
* @param pClosePoint    <= projection of space point onto plane of triangle
* @param pBCoords       <= barycentric coordinates of closest point
* @return minimum distance
* @group "DPoint3d Barycentric"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDPoint3d_minDistToTriangle
(
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pClosePoint,
DPoint3dP pBoundedUVW,
DPoint3dP pUnboundedUVW
)
    {
    DPoint3d planePoint;
    double s, t;
    DPoint3d xyz[3];
    static DPoint3d uvwCorner[3] =
        {
            {1,0,0},
            {0,1,0},
            {0,0,1}
        };

    /* Ugh.  Compute for each edge independently.  */
    DSegment3d testSeg[3];
    DPoint3d   testPoint[3];
    double     testParam[3];
    double     testDistanceSquared[3];

    int i, iMin, jMin;
    xyz[0] = *pVertex0;
    xyz[1] = *pVertex1;
    xyz[2] = *pVertex2;
    if (pUnboundedUVW)
        pUnboundedUVW->Zero ();

    if (bsiGeom_closestPointOnSkewedPlane (&planePoint, &s, &t, xyz, pSpacePoint))
        {
        if (pUnboundedUVW)
            pUnboundedUVW->Init ( 1.0 - s - t, s, t);
        if (s >= 0.0
        &&  t >= 0.0
        &&  s + t <= 1.0)
            {
            if (pBoundedUVW)
                pBoundedUVW->Init ( 1.0 - s - t, s, t);
            if (pClosePoint)
                *pClosePoint = planePoint;
            return planePoint.Distance (*pSpacePoint);
            }
        }

    testSeg[0].Init (*pVertex0, *pVertex1);
    testSeg[1].Init (*pVertex1, *pVertex2);
    testSeg[2].Init (*pVertex2, *pVertex0);

    for (i = 0; i < 3; i++)
        {
        testSeg[i].ProjectPointBounded (testPoint[i], testParam[i], *pSpacePoint);
        testDistanceSquared[i] = testPoint[i].DistanceSquared (*pSpacePoint);
        }
    iMin = 0;
    if (testDistanceSquared[1] < testDistanceSquared[0])
        iMin = 1;
    if (testDistanceSquared[2] < testDistanceSquared[iMin])
        iMin = 2;

    jMin = iMin + 1;
    if (jMin == 3)
        jMin = 0;

    if (pClosePoint)
        *pClosePoint = testPoint[iMin];
    if (pBoundedUVW)
        pBoundedUVW->Interpolate (uvwCorner[iMin], testParam[iMin], uvwCorner[jMin]);
    return sqrt (testDistanceSquared[iMin]);
    }

/*-----------------------------------------------------------------*//**
* @description Applies transformation to simplify the problem of finding the barycentric
* coordinates of a 3d point relative to a triangle.  Returned are the
* components of the new 2d problem.
*
* @instance pInstance   => point to find barycentric coords of
* @param pNewPoint      <= point in plane of new triangle with same barycoords
* @param pNewVector1    <= side of new triangle
* @param pNewVector2    <= side of new triangle
* @param pVector1       => side of old triangle
* @param pVector2       => side of old triangle
* @group "DPoint3d Barycentric"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
static void transformBarycentric3dSystemTo2d
(
DPoint3dCP pInstance,
DPoint2dP pNewPoint,
DPoint2dP pNewVector1,
DPoint2dP pNewVector2,
DPoint3dCP pVector1,
DPoint3dCP pVector2
)
    {
    /*
    Projecting the 3D point q onto the plane spanned by 3D vectors v1, v2
    (which form triangle T) is a least squares problem:

      [v1 v2] [b1 b2]^ = q

    The resulting normal equations (below) determine the barycoords b1, b2
    (corresponding to v1, v2) of the projection relative to T:
      -              - -    -   -      -
      | v1.v1  v1.v2 | | b1 | = | q.v1 |
      | v1.v2  v2.v2 | | b2 |   | q.v2 |
      -              - -    -   -      -

    or equivalently, [newVector1 newVector2] [b1 b2]^ = newPoint.

    This latter form shows that the 3D problem reduces to a 2D problem:
    finding the barycentric coordinates of newPoint relative to the
    triangle in the xy-plane spanned by vectors newVector1, newVector2.
    */
    pNewVector1->x =    pVector1->x * pVector1->x +
                        pVector1->y * pVector1->y +
                        pVector1->z * pVector1->z;
    pNewVector1->y =
    pNewVector2->x =    pVector1->x * pVector2->x +
                        pVector1->y * pVector2->y +
                        pVector1->z * pVector2->z;

    pNewVector2->y =    pVector2->x * pVector2->x +
                        pVector2->y * pVector2->y +
                        pVector2->z * pVector2->z;

    pNewPoint->x =     pInstance->x * pVector1->x +
                       pInstance->y * pVector1->y +
                       pInstance->z * pVector1->z;

    pNewPoint->y =     pInstance->x * pVector2->x +
                       pInstance->y * pVector2->y +
                       pInstance->z * pVector2->z;
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in space.
* Points p and r in space have the same barycentric coordinates relative to
* T if and only if they project to the same point q in the plane of T;
* then their barycentric coordinates relative to T are those of q.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in space
* @param pVertex0       => vertex of triangle T
* @param pVertex1       => vertex of triangle T
* @param pVertex2       => vertex of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint3d_fromBarycentricAndDPoint3dTriangle
* @return true if and only if the area of T is sufficiently large.
* @group "DPoint3d Barycentric"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint3dTriangle
(
DPoint3dP pInstance,
DPoint3dCP pPoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
)
    {
    DPoint3d        q, v1, v2;
    DPoint2d        newPoint, newV1, newV2;

    /*
    Translating by pVertex0 compresses the triangle definition from 3 points
    to 2 vectors while preserving barycentric coords.
    */
    q.x  = pPoint->x   - pVertex0->x;
    q.y  = pPoint->y   - pVertex0->y;
    q.z  = pPoint->z   - pVertex0->z;
    v1.x = pVertex1->x - pVertex0->x;
    v1.y = pVertex1->y - pVertex0->y;
    v1.z = pVertex1->z - pVertex0->z;
    v2.x = pVertex2->x - pVertex0->x;
    v2.y = pVertex2->y - pVertex0->y;
    v2.z = pVertex2->z - pVertex0->z;

    /* decrement dimension of problem */
    transformBarycentric3dSystemTo2d (&q, &newPoint, &newV1, &newV2, &v1, &v2);

    return bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
            (pInstance, &newPoint, NULL, &newV1, &newV2);
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pOrigin, pVector1-pOrigin, pVector2-pOrigin)
* in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pArea          <= area of triangle.
* @param pPoint         => point in plane
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangleVectors
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
)
    {
    return barycentricFromDPoint2dTriangleVectors(pInstance, NULL, pPoint, pOrigin, pVector1, pVector2);
    }

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane with the given barycentric
* coordinates relative to triangle T (pVertex0, pVertex1, pVertex2).
*
* @instance pInstance   <= point with given barycoords relative to T
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @group "DPoint2d Barycentric"
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
Public  void bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
(
DPoint2dP pInstance,
DPoint3dCP pBaryCoords,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
)
    {
    pInstance->x =  pBaryCoords->x * pVertex0->x +
                    pBaryCoords->y * pVertex1->x +
                    pBaryCoords->z * pVertex2->x;
    pInstance->y =  pBaryCoords->x * pVertex0->y +
                    pBaryCoords->y * pVertex1->y +
                    pBaryCoords->z * pVertex2->y;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
