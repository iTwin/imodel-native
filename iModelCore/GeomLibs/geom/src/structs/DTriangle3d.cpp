/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/DTriangle3d.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DTriangle3d::DTriangle3d ()
    {
    point[0].Zero ();
    point[1].Zero ();
    point[2].Zero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
void DTriangle3d::GetVectorsFromOrigin (DVec3dR vectorU, DVec3dR vectorV) const
    {
    vectorU.DifferenceOf (point[1], point[0]);
    vectorV.DifferenceOf (point[2], point[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
void DTriangle3d::Evaluate (double u, double v, DPoint3dR xyz, DVec3dR dXdu, DVec3dR dXdv) const
    {
    GetVectorsFromOrigin (dXdu, dXdv);
    xyz.SumOf (point[0], dXdu, u, dXdv, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
void DTriangle3d::EvaluateNormal (double u, double v, DPoint3dR xyz, DVec3dR unitNormal) const
    {
    DVec3d dXdu, dXdv;
    GetVectorsFromOrigin (dXdu, dXdv);
    unitNormal.NormalizedCrossProduct (dXdu, dXdv);
    xyz.SumOf (point[0], dXdu, u, dXdv, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DVec3d DTriangle3d::CrossVectorsFromOrigin () const
    {
    DVec3d dXdu, dXdv;
    GetVectorsFromOrigin (dXdu, dXdv);
    return DVec3d::FromCrossProduct (dXdu, dXdv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3d DTriangle3d::EvaluateBarycentric (double u, double v, double w) const
    {
    return DPoint3d::FromSumOf (point[0], u, point[1], v, point[2], w);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3d DTriangle3d::EvaluateBarycentric (DPoint3dCR uvw) const
    {
    return DPoint3d::FromSumOf (point[0], uvw.x, point[1], uvw.y, point[2], uvw.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DSegment3d DTriangle3d::GetCCWEdgeDSegment3d (int i) const
    {
    return DSegment3d::From (point[Angle::Cyclic3dAxis (i)], point[Angle::Cyclic3dAxis (i+1)]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DVec3d DTriangle3d::GetCCWEdgeDVec3d (int i) const
    {
    return DVec3d::FromStartEnd (point[Angle::Cyclic3dAxis (i)], point[Angle::Cyclic3dAxis (i+1)]);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DRay3d DTriangle3d::GetCCWEdgeDRay3d (int i) const
    {
    return DRay3d::FromOriginAndTarget (point[Angle::Cyclic3dAxis (i)], point[Angle::Cyclic3dAxis (i+1)]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
bool DTriangle3d::IsPointInOrOnXY (DPoint3dCR xyz) const
    {
    return PolygonOps::IsPointInOrOnXYTriangle (xyz, point[0], point[1], point[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d::DPoint3dDVec3dDVec3d (DPoint3dCR _origin, DVec3dCR _vectorU, DVec3d _vectorV)
    : origin (_origin), vectorU(_vectorU), vectorV (_vectorV)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d::DPoint3dDVec3dDVec3d ()
    : origin (DPoint3d::From (0,0,0)), vectorU(DVec3d::From (1,0,0)), vectorV (DVec3d::From (0,1,0))
    {
    }    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3dDVec3dDVec3d::Evaluate (double u, double v) const
    {
    DPoint3d xyz;
    xyz.SumOf (origin, vectorU, u, vectorV, v);
    return xyz;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DVec3d DPoint3dDVec3dDVec3d::EvaluateVectorOnly (double u, double v) const
    {
    return DVec3d::FromSumOf (vectorU, u, vectorV, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3dDVec3dDVec3d::Evaluate (DPoint2dCR uv) const
    {
    DPoint3d xyz;
    xyz.SumOf (origin, vectorU, uv.x, vectorV, uv.y);
    return xyz;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DVec3d DPoint3dDVec3dDVec3d::EvaluateVectorOnly (DPoint2dCR uv) const
    {
    return DVec3d::FromSumOf (vectorU, uv.x, vectorV, uv.y);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::EvaluateTangents (double u, double v) const
    {
    return DPoint3dDVec3dDVec3d (DPoint3d::FromSumOf (origin, vectorU, u, vectorV, v), vectorU, vectorV);
    }
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
double DPoint3dDVec3dDVec3d::MaxDiff (DPoint3dDVec3dDVec3dCR other) const
    {
    return DoubleOps::Max (origin.MaxDiff (other.origin), vectorU.MaxDiff (other.vectorU), vectorV.MaxDiff (other.vectorV));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndParallelToXY (DPoint3d origin, double sizeU = 1.0, double sizeV = 1.0)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::From (sizeU, 0,0), DVec3d::From (0, sizeV, 0));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndParallelToYZ (DPoint3d origin, double sizeU = 1.0, double sizeV = 1.0)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::From (0, sizeU, 0), DVec3d::From (0, 0, sizeV));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndParallelToXZ (DPoint3d origin, double sizeU = 1.0, double sizeV = 1.0)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::From (sizeU, 0, 0), DVec3d::From (0, 0, sizeV));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/16
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
DPoint3dDVec3dDVec3d DPoint3dDVec3dDVec3d::FromOriginAndTargets (DPoint3dCR origin, DPoint3dCR UTarget, DPoint3dCR VTarget)
    {
    return DPoint3dDVec3dDVec3d (origin, DVec3d::FromStartEnd (origin, UTarget), DVec3d::FromStartEnd (origin, VTarget));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/16
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
* @bsimethod                                                    Earlin.Lutz     01/16
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
bool DPoint3dDVec3dDVec3d::GetTransformsUnitZ (TransformR localToWorld, TransformR worldToLocal) const
    {
    auto unitNormal = DVec3d::FromCrossProduct (vectorU, vectorV).ValidatedNormalize ();
    localToWorld.InitFromOriginAndVectors (origin, vectorU, vectorV, unitNormal);
    return worldToLocal.InverseOf (localToWorld);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
* @bsimethod                                                    Earlin.Lutz     01/16
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
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
UVBoundarySelect::UVBoundarySelect (int selector) : m_selector (selector) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
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
END_BENTLEY_GEOMETRY_NAMESPACE
