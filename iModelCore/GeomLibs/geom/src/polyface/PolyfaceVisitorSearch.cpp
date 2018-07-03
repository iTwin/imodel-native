/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceVisitorSearch.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/BinaryRangeHeap.h>
#include "../DeprecatedFunctions.h"

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryFindFacetRayIntersection (DRay3dCR ray, double tolerance, DPoint3dR facetPoint, double &rayFraction) const
    {
#ifdef BUILD_FOR_811
    return false;
#else
    double edgeFraction;
    DPoint3d rayPoint;
    int baseVertexIndex;
    int code = bsiPolygon_piercePoint (&facetPoint, &edgeFraction, &baseVertexIndex,
                    &m_point[0], (int)m_point.size (),
                    NULL, &ray.origin, &ray.direction,
                    tolerance * tolerance);
    ray.ProjectPointUnbounded (rayPoint, rayFraction, facetPoint);
    return code >= 0;
#endif
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryFindCloseFacetPoint (DPoint3dCR xyz, double tolerance, DPoint3dR facetPoint) const
    {
#ifdef BUILD_FOR_811
    return false;
#else
    int code = bsiPolygon_closestPoint (&facetPoint, &m_point[0], (int)m_point.size (), &xyz);
    double distance = xyz.Distance (facetPoint);
    if (code >= 0 && distance <= tolerance)
        return true;
    return false;
#endif
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::AdvanceToFacetBySearchPoint (DPoint3dCR xyz, double tolerance, DPoint3dR facetXYZ, ptrdiff_t &edgeIndex, double &edgeFraction)
    {
#ifdef BUILD_FOR_811
    return false;
#else
    edgeIndex = -1;
    edgeFraction = 0.0;
    for (int faceIndex = 0; AdvanceToNextFace (); faceIndex++)
        {
        int code = bsiPolygon_closestPointExt (facetXYZ, edgeIndex, edgeFraction, &m_point[0], (int)m_point.size (), xyz);
        double distance = xyz.Distance (facetXYZ);
        if (code >= 0 && distance <= tolerance)
            return true;
        }
#endif
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::AdvanceToFacetBySearchPoint (DPoint3dCR xyz, double tolerance, DPoint3dR facetXYZ)
    {
    for (int faceIndex = 0; AdvanceToNextFace (); faceIndex++)
        {
        if (TryFindCloseFacetPoint (xyz, tolerance, facetXYZ))
            return true;
        }
    return false;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::AdvanceToFacetBySearchRay (DRay3dCR ray, double tolerance, DPoint3dR facetXYZ, double &rayFraction)
    {
    while (AdvanceToNextFace ())
        {
        if (TryFindFacetRayIntersection (ray, tolerance, facetXYZ, rayFraction))
            return true;
        }
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::AdvanceToFacetBySearchRay (DRay3dCR ray, FacetLocationDetail& detail)
    {
    while (AdvanceToNextFace ())
        {
        if (TryDRay3dIntersectionToFacetLocationDetail (ray, detail))
            return true;
        }
    return false;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryGetLocalFrame
(
TransformR localToWorld,
TransformR worldToLocal,
LocalCoordinateSelect selector
)
    {
    uint32_t numEdgesThisFace = NumEdgesThisFace ();
    bvector<DPoint3d> const&    points = Point ();

    return numEdgesThisFace >= 1 && PolygonOps::CoordinateFrame
            (
            &points[0], (size_t)numEdgesThisFace,
            localToWorld, worldToLocal, selector
            );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::AdvanceToFacetBySearchRay
(
DRay3d ray,
double tolerance,
DPoint3dR facetXYZ,
double &rayFraction,
ptrdiff_t &edgeIndex,
double &edgeFraction,
DPoint3dR edgeXYZ,
double &edgeDistance
)
    {
    edgeDistance = DBL_MAX;
    edgeIndex = -1;
    edgeFraction = 0.0;
    while (AdvanceToNextFace ())
        {
        if (TryFindFacetRayIntersection (ray, tolerance, facetXYZ, rayFraction))
            {
            DPoint3d xyzA, xyzB;
            double fractionA, fractionB;
            DPoint3d xyz0 = m_point.back ();
            size_t n = m_point.size ();
            DPoint3d xyz1;
            for (size_t i = 0, iEdge = n - 1; i < n; iEdge = i, i++, xyz0 = xyz1)
                {
                xyz1 = m_point[i];
                DSegment3d segment= DSegment3d::From (xyz0, xyz1);
                DRay3d::ClosestApproachUnboundedRayBoundedSegment (fractionA, fractionB, xyzA, xyzB, ray, segment);
                double d = xyzA.Distance (xyzB);
                if (d <= edgeDistance)
                    {
                    edgeIndex = iEdge;
                    edgeDistance = d;
                    edgeFraction = fractionB;
                    edgeXYZ = xyzB;
                    }
                }
            return true;
            }
        }
    return false;
    }

// Update 3 detail structures:
//  A -- to left of refCoordinate
//  0 -- at refCoordinate
//  B -- to right of refCoordinate
struct ScanSearcher
{
FacetLocationDetail m_detailA, m_detailB, m_detail0;
double m_xPick;
double m_yTarget;
PolyfaceVisitor &m_visitor;
ScanSearcher (PolyfaceVisitor &visitor)
    : m_visitor (visitor)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IsActive(FacetLocationDetailCR detail) const
    {
    return detail.a < DBL_MAX && detail.a > -DBL_MAX;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void InitScanPair(size_t readIndex, double xPick, double yTarget)
    {
    m_detailA = FacetLocationDetail (readIndex,  -DBL_MAX);
    m_detail0 = FacetLocationDetail (readIndex,  -DBL_MAX);
    m_detailB = FacetLocationDetail (readIndex,   DBL_MAX);
    m_xPick = xPick;
    m_yTarget = yTarget;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IsRealInterval()
    {
    return m_detailA.a > -DBL_MAX && m_detailB.a < DBL_MAX && m_detailA.a <= m_detailB.a;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void UpdateInterval(double x, size_t edgeIndex, double fraction)
    {
    if (x == m_xPick)
        {
        m_visitor.IntepolateDataOnEdge (m_detail0, edgeIndex, fraction, x);
        }
    else
        {
        if (x <= m_xPick && x > m_detailA.a)
            m_visitor.IntepolateDataOnEdge (m_detailA, edgeIndex, fraction, x);
        if (x >= m_xPick && x < m_detailB.a)
            m_visitor.IntepolateDataOnEdge (m_detailB, edgeIndex, fraction, x);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TestAndUpdateCrossing(double x0, double y0, double x1, double y1, size_t edgeIndex)
    {
    y0 -= m_yTarget;
    y1 -= m_yTarget;
    if (y0 * y1 > 0.0)
        return false;
    if (y0 == 0.0 && y1 == 0.0)
        {
        UpdateInterval (x0, edgeIndex, 0.0);
        UpdateInterval (x1, edgeIndex, 1.0);
        return true;
        }
    else    // not both zero, division is safe ...
        {
        double fraction = -y0 / (y1 - y0);
        double x = DoubleOps::Interpolate (x0, fraction, x1);
        UpdateInterval (x, edgeIndex, fraction);
        return true;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool GetPair(FacetLocationDetailPairP pair)
    {
    if (NULL != pair)
        {
        if (IsActive (m_detail0))
            {
            if (IsActive (m_detailA))
                {
                pair->detailA = m_detailA;
                pair->detailB = m_detail0;
                return true;
                }
            if (IsActive (m_detailA))
                {
                pair->detailA = m_detail0;
                pair->detailB = m_detailB;
                return true;
                }
            }
        else
            {
            if (IsActive (m_detailA) && IsActive (m_detailB))
                {
                pair->detailA = m_detailA;
                pair->detailB = m_detailB;
                return true;
                }
            }
        }
    return false;
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::IntepolateDataOnEdge (FacetLocationDetailR detail, size_t vertexIndex, double edgeFraction, double a)
    {
    uint32_t numEdge = NumEdgesThisFace ();
    if (numEdge <= 0)
        return false;
    size_t i0 = vertexIndex % numEdge;
    size_t i1 = (vertexIndex + 1) % numEdge;
    detail.Zero ();
    AccumulateScaledData (detail, i0, 1.0 - edgeFraction);
    AccumulateScaledData (detail, i1, edgeFraction);
    detail.a = a;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::AccumulateScaledData
(
FacetLocationDetailR detail,
size_t index, 
double fraction
)
    {
    FacetLocationDetail detailA;
    if (LoadVertexData (detailA, index))
        {
        detail.AccumulateScaledData (detailA, fraction);
        return true;
        }
    return false;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryParamToScanBrackets
    (
    DPoint2dCR uvParam,
    FacetLocationDetailPairP horizontalScanBracket,
    FacetLocationDetailPairP verticalScanBracket
    )
    {
    size_t numEdgesThisFace = (size_t)NumEdgesThisFace ();
    if (numEdgesThisFace < 2)
        return false;
    if (m_param.size () < numEdgesThisFace)
        return false;
    ScanSearcher hScan(*this), vScan (*this);
    hScan.InitScanPair (0, uvParam.x, uvParam.y);
    vScan.InitScanPair (0, uvParam.y, uvParam.x);
    size_t i0 = numEdgesThisFace - 1;
    for (size_t i1 = 0; i1 < numEdgesThisFace; i0 = i1++)
        {
        DPoint2d uv0 = m_param[i0];
        DPoint2d uv1 = m_param[i1];    
        hScan.TestAndUpdateCrossing (uv0.x, uv0.y, uv1.x, uv1.y, i0);
        vScan.TestAndUpdateCrossing (uv0.y, uv0.x, uv1.y, uv1.x, i0);
        }
    return hScan.GetPair (horizontalScanBracket)
        && vScan.GetPair (verticalScanBracket);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool InterpolateDetail
(
double a,
FacetLocationDetailCR detailA,
FacetLocationDetailCR detailB,
FacetLocationDetailR  detail,
DVec3dR dXda
)
    {
    double f;
    double delta = detailB.a - detailA.a;
    if (DoubleOps::SafeDivideParameter (f, a - detailA.a, delta, 0.0))
        {
        detail.Zero ();
        detail.a = a;
        detail.AccumulateScaledData (detailA, 1.0 - f);
        detail.AccumulateScaledData (detailB, f);
        dXda.DifferenceOf (detailB.point, detailA.point);
        dXda.Scale (1.0 / delta);   // Assume it's safe because of SafeDivideParameter?
        return true;
        }
    return false;
    }

// Compute barycentric coordinates of uvParam wrt the triangle on (locally numbered) vertices i0,i1,i2.
// Return false if i0,i1,i2 not a valid triangle.
// Set isInteriorPoint to true if interior to the triangle. (i.e. points outside the triangle return true for the function but the outside state is noted within the structure.)
static bool TryTriangleParamToFacetLocationDetail
(
PolyfaceVisitorR visitor,
DPoint2d uvParam,
FacetLocationDetailR detail,
size_t i0,
size_t i1,
size_t i2
)
    {
    size_t numEdgesThisFace = (size_t)visitor.NumEdgesThisFace ();
    if (numEdgesThisFace != 3)
        return false;
    if (!(i0 < numEdgesThisFace && i1 < numEdgesThisFace && i2 < numEdgesThisFace))
        return false;
    DPoint3d U;   // local barycentrics, not the params of the mesh.
    DVec3d dUdx;
    DVec3d dUdy;
    DPoint3d ddx, ddy;  // ugh. Have to form sums as points.
    double area;
    bvector<DPoint3d>&point = visitor.Point ();
    bvector<DPoint2d>&param = visitor.Param ();
    if (bsiDPoint3d_barycentricFromDPoint2dTriangle (U, dUdx, dUdy, area, uvParam, param[i0], param[i1], param[i2]))
        {
        visitor.AccumulateScaledData (detail, i0, U.x);
        visitor.AccumulateScaledData (detail, i1, U.y);
        visitor.AccumulateScaledData (detail, i2, U.z);
        ddx.SumOf (point[i0], dUdx.x, point[i1], dUdx.y, point[i2], dUdx.z);
        ddy.SumOf (point[i0], dUdy.x, point[i1], dUdy.y, point[i2], dUdy.z);
        detail.dXdu = DVec3d::From (ddx);
        detail.dXdv = DVec3d::From (ddy);
        detail.SetIsInterior(DoubleOps::IsIn01 (U));
        detail.SetReadIndex (visitor.GetReadIndex ());
        return true;
        }
    return false;
    }

// Compute point and normal at uvParam wrt the triangle on (locally numbered) vertices 0,i1,(i1+1),
// i.e. 
// Return false if i0,i1,i2 not a valid triangle with non-colinear points
ValidatedDRay3d PolyfaceVisitor::TryTriangleParamToPerpendicularRay
(
DPoint2d uvParam,
size_t i1
) const
    {
    size_t numEdgesThisFace = (size_t)NumEdgesThisFace ();
    if (i1 == 0 || i1 + 1 >= numEdgesThisFace)
        return ValidatedDRay3d ();
    DTriangle3d triangle (m_point[0], m_point[i1], m_point[i1+1]);
    return triangle.EvaluateUnitNormalRay (uvParam.x, uvParam.y);
    }

// Compute barycentric coordinates of uvParam wrt the triangle on (locally numbered) vertices i0,i1,i2.
// Return false if i0,i1,i2 not a valid triangle.
// Set isInteriorPoint to true if interior to the triangle. (i.e. points outside the triangle return true for the function but the outside state is noted within the structure.)
static bool TryTriangleDRay3dIntersectionToFacetLocationDetail
(
PolyfaceVisitorR visitor,
DRay3d ray,
FacetLocationDetailR detail,
size_t i0,
size_t i1,
size_t i2
)
    {
    size_t numEdgesThisFace = (size_t)visitor.NumEdgesThisFace ();
    if (!(i0 < numEdgesThisFace && i1 < numEdgesThisFace && i2 < numEdgesThisFace))
        return false;
    bvector<DPoint3d>&point = visitor.Point ();
    bvector<DPoint2d>&param = visitor.Param ();
    DPoint3d xyzHit;
    DPoint3d U;   // local barycentrics, not the params of the mesh.
    double rayFraction;
    DTriangle3d triangle (point[i0], point[i1], point[i2]);
    if (triangle.TransverseIntersection (ray, xyzHit, U, rayFraction))
        {
        detail.Zero ();
        visitor.AccumulateScaledData (detail, i0, U.x);
        visitor.AccumulateScaledData (detail, i1, U.y);
        visitor.AccumulateScaledData (detail, i2, U.z);

        detail.SetIsInterior (DoubleOps::IsIn01 (U));
        detail.SetReadIndex (visitor.GetReadIndex ());
        detail.a = rayFraction;
        DVec3d normal;
        if (!detail.TryGetNormal (normal))
            detail.SetNormal (DVec3d::FromNormalizedCrossProductToPoints (triangle.point[0], triangle.point[1], triangle.point[2]));

        DPoint2d uvParam;
        DPoint3d U1;    // should match U when recomputed relative to facet's parameterization.
        DVec3d dUdx;
        DVec3d dUdy;
        DPoint3d ddx, ddy;  // ugh. Have to form sums as points.
        double area;
        if (detail.TryGetParam (uvParam)
            && bsiDPoint3d_barycentricFromDPoint2dTriangle
                    (
                    U1, dUdx, dUdy, area,
                    uvParam,
                    param[i0], param[i1], param[i2]))
            {
            ddx = triangle.EvaluateBarycentric (dUdx);
            ddy = triangle.EvaluateBarycentric (dUdy);
            detail.dXdu = DVec3d::From (ddx);
            detail.dXdv = DVec3d::From (ddy);
            }
        else
            {
            // visitor does not have parameters.  Put the directions along 
            // the edges
            detail.dXdu = DVec3d::FromStartEnd (triangle.point[0], triangle.point[1]);
            detail.dXdv = DVec3d::FromStartEnd (triangle.point[0], triangle.point[2]);
            }
        return true;
        }
    return false;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryParamToFacetLocationDetail
(
DPoint2d uvParam,
FacetLocationDetailR detail
)
    {
    if (Param().size () == 0)
        return false;
    size_t numEdgesThisFace = (size_t)NumEdgesThisFace ();
    if (numEdgesThisFace == 3)
        {
        if (TryTriangleParamToFacetLocationDetail (*this, uvParam, detail, 0, 1, 2))
            return detail.GetIsInterior ();
        }

    FacetLocationDetailPair hPair;
    FacetLocationDetailPair vPair;
    FacetLocationDetail detailY;
    DVec3d dXdu, dXdv;
    if (TryParamToScanBrackets (uvParam, &hPair, &vPair))
        {
        if (InterpolateDetail (uvParam.x, hPair.detailA, hPair.detailB, detail, dXdu)
            && InterpolateDetail (uvParam.y, vPair.detailA, vPair.detailB, detailY, dXdv))
            {
            detail.dXdu = dXdu;
            detail.dXdv = dXdv;
            detail.SetIsInterior (true);
            detail.SetReadIndex (GetReadIndex ());
            return true;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceVisitor::TryDRay3dIntersectionToFacetLocationDetail
(
DRay3dCR ray,
FacetLocationDetailR detail
)
    {
    // EDL -- Nov 14 2013 --- What about more vertices?  InOut Test? parameterization?
    size_t numEdgesThisFace = (size_t)NumEdgesThisFace ();
    if (numEdgesThisFace == 3)
        {
        return TryTriangleDRay3dIntersectionToFacetLocationDetail
                    (*this, ray, detail, 0, 1, 2)
            && detail.GetIsInterior ();
        }

    // This is right only for convex ...
    for (size_t i0 = 1; i0 + 1 < numEdgesThisFace; i0++)
        {
        if (TryTriangleDRay3dIntersectionToFacetLocationDetail (*this, ray, detail, 0, i0, i0 + 1)
                    && detail.GetIsInterior ())
            return true;
        }
    return false;
    }


static void CopyWithInterpolatedUVXYZ (FacetLocationDetail &detail, FacetLocationDetail const &detailA, double f, FacetLocationDetail const &detailB)
    {
    detail = detailA;
    detail.point.Interpolate (detailA.point, f, detailB.point);
    detail.param.Interpolate (detailA.param, f, detailB.param);
    }

bool PolyfaceQuery::PickFacetsByStroke
(
DPoint4dCR eyePoint,
DPoint3dCR point0,
DPoint3dCR point1,
bvector<FacetLocationDetail> &pickData,
bool exitAtFirstPick
)
    {
    pickData.clear ();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*this, false);
    DVec3d vector0E = DVec3d::FromStartEnd (point0, point1);
    DVec3d vector01 = DVec3d::FromWeightedDifferenceOf (eyePoint, point0);
    DVec3d normal   = DVec3d::FromNormalizedCrossProduct (vector0E, vector01);

    bvector<CurveLocationDetail> slicePoints;
    bvector<CurveLocationDetail> touchPoints;

    DMatrix4d triangleToWorld = DMatrix4d::FromRowValues
        (
        eyePoint.x, point0.x, point1.x, normal.x,
        eyePoint.y, point0.y, point1.y, normal.y,
        eyePoint.z, point0.z, point1.z, normal.z,
        eyePoint.w,      1.0,      1.0,      0.0
        );
    // in a flat view, eyePoint.w == 0.
    DMatrix4d worldToTriangle;
    if (!worldToTriangle.QrInverseOf (triangleToWorld))
        return false;
    static double s_abstol = 1.0e-10;
    DPlane3d plane = DPlane3d::FromOriginAndNormal (point0, normal);
    bvector<FacetLocationDetail> strokeParameters;
    bvector<DPoint3d> &points = visitor->Point ();
    DRange1d altitudeRange;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        PolygonOps::PlaneIntersectionPoints (points, plane, s_abstol, slicePoints, &touchPoints, altitudeRange);

        if (slicePoints.size () > 1)
            {
            strokeParameters.clear ();
            bool error = false;
            strokeParameters.clear ();
            for (size_t i = 0; i < slicePoints.size (); i++)
                {
                DPoint4d Q = worldToTriangle.Multiply (slicePoints[i].point, 1.0);
                double alpha = Q.y;
                double beta  = Q.z;
                double zeta  = Q.x; // towards eye
                double lambda;
                if (!DoubleOps::SafeDivide (lambda, beta, alpha + beta, 0.0))
                    {
                    error = true;
                    break;
                    }
                FacetLocationDetail detail (visitor->GetReadIndex ());
                detail.point = slicePoints[i].point;
                detail.param = DPoint2d::From (lambda, zeta);
                strokeParameters.push_back (detail);
                }
            if (!error)
                {
                FacetLocationDetail::SortUV (strokeParameters);

                for (size_t i = 0; i + 1< strokeParameters.size (); i += 2)
                    {
                    double u0 = strokeParameters[i].param.x;
                    double u1 = strokeParameters[i+1].param.x;
                    if (u1 < 0.0)
                        {
                        }
                    else if (u0 > 1.0)
                        {
                        }
                    else
                        {
                        FacetLocationDetail detailA = strokeParameters[i];
                        double f;
                        if (u0 < 0.0 && DoubleOps::SafeDivide (f, -u0, u1 - u0))
                            CopyWithInterpolatedUVXYZ (detailA, strokeParameters[i], f, strokeParameters[i+1]);
                        FacetLocationDetail detailB = strokeParameters[i+1];
                        if (u1 > 1.0 && DoubleOps::SafeDivide (f, 1.0-u0, u1 - u0))
                            CopyWithInterpolatedUVXYZ (detailB, strokeParameters[i], f, strokeParameters[i+1]);
                        pickData.push_back (detailA);
                        pickData.push_back (detailB);
                        if (exitAtFirstPick)
                            return true;
                        }
                    }
                }
            }
        }
    return pickData.size () > 0;
    }


// Search for closest approach between two meshes.
//
static bool SearchClosestApproach_allPairs (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment                 //!< shortest segment.
)
    {
    PolyfaceVisitorPtr visitorA = PolyfaceVisitor::Attach (polyfaceA);
    PolyfaceVisitorPtr visitorB = PolyfaceVisitor::Attach (polyfaceB);

    bvector<DPoint3d> &facetPointA = visitorA->Point ();
    bvector<DPoint3d> &facetPointB = visitorB->Point ();
    DPoint3d pointA, pointB;

    double minDistance = maxDistance;
    if (minDistance <= 0.0)
        minDistance = DBL_MAX;
    bool found = false;
    for (visitorA->Reset (); visitorA->AdvanceToNextFace ();)
        {
        DRange3d rangeA = DRange3d::From (facetPointA);
        for (visitorB->Reset (); visitorB->AdvanceToNextFace ();)
            {
            DRange3d rangeB = DRange3d::From (facetPointB);
            //if (rangeA.IntersectsWith (rangeB, minDistance, 3))
            if (rangeA.DistanceSquaredTo (rangeB) <= minDistance * minDistance)
                {
                if (bsiPolygon_closestApproachBetweenPolygons (&pointA, &pointB,
                          &facetPointA.front (), (int)facetPointA.size (),
                          &facetPointB.front (), (int)facetPointB.size ()))
                    {
                    double d = pointA.Distance (pointB);
                    if (d < minDistance)
                        {
                        found = true;
                        minDistance = d;
                        segment.point[0] = pointA;
                        segment.point[1] = pointB;
                        }
                    }
                }
            }
        }
    return found;
    }

// Search for closest approach between mesh and linestring
//
bool PolyfaceQuery::SearchClosestApproachToLinestring (
PolyfaceQueryR polyfaceA,           //!< first polyface
bvector<DPoint3d> const &pointsB,
DSegment3dR segment                 //!< shortest segment.
)
    {
    PolyfaceVisitorPtr visitorA = PolyfaceVisitor::Attach (polyfaceA);
    bvector<DPoint3d> &facetPointA = visitorA->Point ();
    DPoint3d pointA, pointB;
    DRange3d rangeB = DRange3d::From (pointsB);
    double minDistance = DBL_MAX;
    if (minDistance <= 0.0)
        minDistance = DBL_MAX;
    bool found = false;
    // linestring pointer does not change during the mesh visits ...
    DPoint3d const *pPointsB = &pointsB.front ();
    int numPointsB = (int)pointsB.size ();

    for (visitorA->Reset (); visitorA->AdvanceToNextFace ();)
        {
        DRange3d rangeA = DRange3d::From (facetPointA);
        if (!found || rangeA.IntersectsWith (rangeB, minDistance, 3))
            {
            if (bsiPolygon_closestApproachBetweenPolygonAndLineString (
                pointA, pointB,
                &facetPointA.front (), (int)facetPointA.size (),
                pPointsB, numPointsB)
                )
                {
                double d = pointA.Distance (pointB);
                if (d < minDistance)
                    {
                    found = true;
                    minDistance = d;
                    segment.point[0] = pointA;
                    segment.point[1] = pointB;
                    }
                }
            }
        }
    return found;
    }





struct ClosestApproachPairTester : public IndexedRangeHeap::PairProcessor
{
PolyfaceVisitorPtr m_visitorA;
PolyfaceVisitorPtr m_visitorB;
double m_maxDistance;   // as requested by caller
double m_minDistanceFound;   // min found so far.
bool   m_looking;
size_t m_found;
DSegment3d m_segment;
PolyfaceIndexedHeapRangeTree *m_treeA;
PolyfaceIndexedHeapRangeTree *m_treeB;

size_t m_numReduce;
BoolCounter m_numNeedProcessing;
size_t m_numProcess;

ClosestApproachPairTester (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
PolyfaceIndexedHeapRangeTree *treeA,
PolyfaceIndexedHeapRangeTree *treeB,
double maxDistance                  //!< ignore larger distances.  Send 0 to consider all pairs.
)
    : m_maxDistance (maxDistance),
    m_treeA (treeA),
    m_treeB (treeB),
    m_numReduce (0),
    m_numProcess (0)
    {
    m_visitorA = PolyfaceVisitor::Attach (polyfaceA);
    m_visitorB = PolyfaceVisitor::Attach (polyfaceB);


    m_minDistanceFound = DBL_MAX;
    if (m_maxDistance > 0.0)
        m_minDistanceFound = m_maxDistance;
    m_looking = true;
    m_found = false;
    }

    bool NeedProcessing (
            DRange3dCR rangeA, size_t iA0, size_t iA1,
            DRange3dCR rangeB, size_t iB0, size_t iB1
            ) override
            {
            return m_numNeedProcessing.Count (
                //rangeA.IntersectsWith (rangeB, m_minDistanceFound, 3));
                rangeA.DistanceSquaredTo (rangeB) <= m_minDistanceFound * m_minDistanceFound);
            } 

    void Process (size_t iA, size_t iB) override 
            {
            size_t readIndexA, readIndexB;
            m_numProcess++;
            if (!m_treeA->TryGetReadIndex (iA, readIndexA))
                return;
            if (!m_treeB->TryGetReadIndex (iB, readIndexB))
                return;
            if (!m_visitorA->MoveToFacetByReadIndex (readIndexA))
                return;
            if (!m_visitorB->MoveToFacetByReadIndex (readIndexB))
                return;
                
            DPoint3d pointA, pointB;
            if (bsiPolygon_closestApproachBetweenPolygons (
                        &pointA, &pointB,
                        &m_visitorA->Point().front (), (int)m_visitorA->Point().size (),
                        &m_visitorB->Point().front (), (int)m_visitorB->Point().size ()
                        ))
                {
                double d = pointA.Distance (pointB);
                if (d < m_minDistanceFound)
                    {
                    m_numReduce++;
                    m_found = true;
                    m_minDistanceFound = d;
                    m_segment.point[0] = pointA;
                    m_segment.point[1] = pointB;
                    } 
                }        
            }
            
    bool IsLive () const override
        {
        return m_looking;
        }
    bool GetSegment (DSegment3dR segment)
        {
        if (m_found)
            {
            segment = m_segment;
            return true;
            }
        return false;
        }
};
// Search for closest approach between two meshes.
//
GEOMDLLIMPEXP bool PolyfaceQuery::SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment,                 //!< shortest segment.
struct PolyfaceIndexedHeapRangeTree *treeA,    //! optional range tree for polyfaceA
struct PolyfaceIndexedHeapRangeTree *treeB     //! optional range tree for polyfaceB
)
    {
    if (nullptr == treeA || nullptr == treeB)
        return SearchClosestApproach_allPairs (polyfaceA, polyfaceB, maxDistance, segment);
    ClosestApproachPairTester tester (polyfaceA, polyfaceB, treeA, treeB, maxDistance);
    static size_t s_sampleStep = 197;
    // do "a few" samples to get an idea of what a "small" distance is ...
    for (size_t readIndexA, iA = 0; treeA->TryGetReadIndex (iA, readIndexA); iA += s_sampleStep)
        for (size_t readIndexB, iB = 0; treeB->TryGetReadIndex (iB, readIndexB); iB += s_sampleStep)
            {
            tester.Process (iA, iB);
            }
    
    IndexedRangeHeap::Search (treeA->GetHeapR (), treeB->GetHeapR (), tester, 1);
    return tester.GetSegment (segment);
    }

// Search for closest approach between two meshes.
//
GEOMDLLIMPEXP bool PolyfaceQuery::SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< first polyface
PolyfaceQueryR polyfaceB,           //!< second polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment                 //!< shortest segment.
)
    {
    size_t numA = polyfaceA.GetPointCount ();
    size_t numB = polyfaceB.GetPointCount ();
    static size_t s_pointLimit = 10;
    if (numA > s_pointLimit && numB > s_pointLimit)
        {
        PolyfaceIndexedHeapRangeTreePtr treeA = PolyfaceIndexedHeapRangeTree::CreateForPolyface (polyfaceA);
        PolyfaceIndexedHeapRangeTreePtr treeB = PolyfaceIndexedHeapRangeTree::CreateForPolyface (polyfaceB);
        return SearchClosestApproach (polyfaceA, polyfaceB, maxDistance, segment, treeA.get (), treeB.get ());
        }
    else
        {
        return SearchClosestApproach_allPairs (polyfaceA, polyfaceB, maxDistance, segment);
        }
    }




template <typename T>
bool AreAnyValuesShared (bvector<T> &indexA, bvector<T> &indexB)
    {
    for (T a : indexA)
        {
        for (T b : indexB)
            {
            if (a == b)
                return true;
            }
        }
    return false;
    }

// Search for closest approach of the polyface to itself
//
GEOMDLLIMPEXP bool PolyfaceQuery::SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< first polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment,                 //!< shortest segment.
double normalTestRadians
)
    {
    PolyfaceVisitorPtr visitorA = PolyfaceVisitor::Attach (polyfaceA);
    PolyfaceVisitorPtr visitorB = PolyfaceVisitor::Attach (polyfaceA);

    bvector<DPoint3d> &facetPointA = visitorA->Point ();
    bvector<DPoint3d> &facetPointB = visitorB->Point ();
    bvector<int> &indexA = visitorA->ClientPointIndex ();
    bvector<int> &indexB = visitorB->ClientPointIndex ();

    DPoint3d pointA, pointB;
    if (normalTestRadians < 0.0)
        normalTestRadians = 0.0;
    double minDistance = maxDistance;
    if (minDistance <= 0.0)
        minDistance = DBL_MAX;
    bool found = false;
    double radiansPerp = Angle::PiOver2 ();
    DVec3d normalA, normalB;
    for (visitorA->Reset (); visitorA->AdvanceToNextFace ();)
        {
        DRange3d rangeA = DRange3d::From (facetPointA);
        for (visitorB->Reset (); visitorB->AdvanceToNextFace ();)
            {
            if (AreAnyValuesShared<int> (indexA, indexB))
                continue;
            DRange3d rangeB = DRange3d::From (facetPointB);
            //if (rangeA.IntersectsWith (rangeB, minDistance, 3))
            if (rangeA.DistanceSquaredTo (rangeB) < minDistance * minDistance)
                {
                if (bsiPolygon_closestApproachBetweenPolygons (&pointA, &pointB, &normalA, &normalB,
                          &facetPointA.front (), (int)facetPointA.size (),
                          &facetPointB.front (), (int)facetPointB.size ()))
                    {
                    double d = pointA.Distance (pointB);
                    if (d < minDistance)
                        {
                        bool accept = true;
                        if (d > 0.0 && normalTestRadians > 0.0)
                            {
                            DVec3d edgeVector = DVec3d::FromStartEnd (pointA, pointB);
                            double thetaA = fabs (edgeVector.AngleTo(normalA) - radiansPerp);
                            double thetaB = fabs (edgeVector.AngleTo(normalB) - radiansPerp);
                            if (thetaA < normalTestRadians || thetaB < normalTestRadians)
                                accept = false;
                            }
                        if (accept)
                            {                            
                            found = true;
                            minDistance = d;
                            segment.point[0] = pointA;
                            segment.point[1] = pointB;
                            }
                        }
                    }
                }
            }
        }
    return found;
    }
    
GEOMDLLIMPEXP bool PolyfaceQuery::SearchClosestApproach (
PolyfaceQueryR polyfaceA,           //!< first polyface
double maxDistance,                 //!< ignore larger distances.  Send 0 to consider all pairs.
DSegment3dR segment                 //!< shortest segment.
)
    {
    return SearchClosestApproach (polyfaceA, maxDistance, segment, 0.0);
    }    

/*=================================================================================**//**
* Filter class for selecting facets presented by a visitor.
+===============+===============+===============+===============+===============+======*/

PolyfaceEdgeSearcher::PolyfaceEdgeSearcher (PolyfaceQueryCR polyface, DMatrix4dCR worldToLocal, double localAperture, bool ignoreInvisibleEdges)
    {
    m_visitor = PolyfaceVisitor::Attach (polyface, false);
    m_worldToLocal = worldToLocal;
    m_localAperture = localAperture;
    m_ignoreInvisibleEdges = ignoreInvisibleEdges;
    m_visitor->SetNumWrap (1);
    m_visitor->Reset ();
    }

void PolyfaceEdgeSearcher::Reset (){m_visitor->Reset ();}

PolyfaceEdgeSearcher::SearchState PolyfaceEdgeSearcher::AppendHits (
DPoint3dCR localPick,
bvector<FacetLocationDetail> *edgeHits,
bvector<FacetLocationDetail> *facetHits,
size_t maxTest,
size_t maxNewHit
)
    {
    size_t numNewHit = 0;
    size_t numTest = 0;
    bvector<DPoint3d> &points = m_visitor->Point ();
    bvector<bool> &visible = m_visitor->Visible ();

    // NEEDSWORK: TryDRay3dIntersectionToFacetLocationDetail currently needs "world" DRay3d...
    DRay3d boresite;
    if (nullptr != facetHits)
        {
        DMatrix4d   localToWorld;
        DPoint3d    worldPt;

        localToWorld.QrInverseOf(m_worldToLocal);
        localToWorld.MultiplyAndRenormalize(&worldPt, &localPick, 1);

        DPoint4d    eyePoint;
        double      aa;

        boresite.origin = worldPt;
        localToWorld.GetColumn(eyePoint, 2);
        boresite.direction.Init(eyePoint.x, eyePoint.y, eyePoint.z);

        if (DoubleOps::SafeDivide(aa, 1.0, eyePoint.w, 1.0))
            {
            DPoint3d  xyzEye;

            xyzEye.Scale(boresite.direction, aa);
            boresite.direction.DifferenceOf(xyzEye, boresite.origin);
            }

        boresite.direction.Normalize();
        boresite.direction.Negate();
        }

    while (m_visitor->AdvanceToNextFace ())
        {
        if (numTest >= maxTest)
            return SearchState::SuspendByMaxTest;
        numTest++;

        if (nullptr != edgeHits)
            {            
            for (size_t i = 0, numEdge = m_visitor->NumEdgesThisFace (); i < numEdge; i++)
                {
                if (!m_ignoreInvisibleEdges || visible[i])
                    {
                    DSegment4d hSegment = DSegment4d::From (points[i], points[i+1]);
                    m_worldToLocal.Multiply (hSegment.point, hSegment.point, 2);
                    DPoint3d closePoint;
                    double closeParam, distanceXY;
                    if (hSegment.ClosestPointBoundedXY (closePoint, closeParam, distanceXY, localPick, nullptr, false, false)
                        && distanceXY < m_localAperture)
                        {
                        FacetLocationDetail detail (m_visitor->GetReadIndex (), distanceXY);
                        detail.point = DPoint3d::FromInterpolate (points[i], closeParam, points[i+1]);
                        detail.sourceIndex[0] = i;
                        detail.sourceFraction[0] = closeParam;
                        detail.numSourceIndex = 1;
                        detail.dXdu = DVec3d::From (points[i]);
                        detail.dXdv = DVec3d::From (points[i+1]);
                        edgeHits->push_back (detail);
                        numNewHit++;
                        if (numNewHit > maxNewHit)
                            return SearchState::SuspendByMaxNewHit;
                        }
                    }
                }
            }
        
        if (nullptr != facetHits)
            {
            FacetLocationDetail facetDetail;
            if (m_visitor->TryDRay3dIntersectionToFacetLocationDetail(boresite, facetDetail))
                {
                facetHits->push_back (facetDetail);
                numNewHit++;
                if (numNewHit > maxNewHit)
                    return SearchState::SuspendByMaxNewHit;
                }
            }
        }
    return SearchState::Complete;
    }



PolyfacePolygonPicker::PolyfacePolygonPicker (PolyfaceQueryCR polyface, DMatrix4dCR worldToLocal)
    {
    m_visitor = PolyfaceVisitor::Attach (polyface, false);
    m_worldToLocal = worldToLocal;
    m_visitor->SetNumWrap (1);
    m_visitor->Reset ();
    m_shiftFactor = 0.001;
    }

void PolyfacePolygonPicker::Reset (){m_visitor->Reset ();}

PolyfacePolygonPicker::SearchState PolyfacePolygonPicker::AppendHitsByStroke
(
DPoint3dCR strokeStart,             //!< [in] local coordinates of stroke start
DPoint3dCR strokeEnd,               //< [in] local coordinates of stroke end
bvector<size_t> *facets,            //!< [inout] growing array of readIndices of facets touched by the edge.
bvector<StrokePick> *strokePicks,   // [inout] growing array of readIndices and sort data for facets touched by the edge
size_t maxTest,                     //!< [in] maximum number of new facets to test
size_t maxNewHit                    //!< [in] maximum number of new hits to record
)
    {
    size_t numNewHit = 0;
    size_t numTest = 0;
    bvector<DPoint3d> &points = m_visitor->Point ();
    DVec3d strokeVector = strokeEnd - strokeStart;
    strokeVector.z = 0.0;
    strokeVector.Normalize ();
    DVec3d perpVector = DVec3d::From (-m_shiftFactor * strokeVector.y, m_shiftFactor * strokeVector.x, 0.0);
    ClipPlane linePlane (perpVector, strokeStart);
    ClipPlane leftClip (strokeVector, strokeStart);
    DVec3d reverseStrokeVector;
    reverseStrokeVector.Negate (strokeVector);
    ClipPlane rightClip (reverseStrokeVector, strokeEnd);

    ConvexClipPlaneSet convexSet;
    convexSet.push_back (leftClip);
    convexSet.push_back (rightClip);

    uint32_t axisSelect = fabs (strokeEnd.x - strokeStart.x) > fabs (strokeEnd.y - strokeStart.y) ? 0 : 1;

    bvector<DPoint3d> edgePoints, insidePoints;
    while (m_visitor->AdvanceToNextFace ())
        {
        size_t ri = m_visitor->GetReadIndex ();
        if (numTest >= maxTest)
            return SearchState::SuspendByMaxTest;
        numTest++;
        m_worldToLocal.MultiplyAndRenormalize (points);
        linePlane.PolygonCrossings (points, edgePoints);
        convexSet.ClipPointsOnOrInside (edgePoints, &insidePoints, nullptr);
        if (insidePoints.size () > 0)
            {
            if (nullptr != facets)
                facets->push_back (ri);
            if (nullptr != strokePicks)
                AppendStrokePick (*strokePicks, axisSelect, insidePoints, ri);
            numNewHit++;
            if (numNewHit > maxNewHit)
                return SearchState::SuspendByMaxNewHit;
            }
        }
    return SearchState::Complete;
    }

void PolyfacePolygonPicker::AppendStrokePick
(
bvector<StrokePick> &data,
uint32_t axisSelect,
bvector<DPoint3d> const &points,
size_t readIndex
)
    {
    if (points.size () > 1)
        {
        StrokePick strokeData;
        strokeData.u0 = DBL_MAX;
        strokeData.u1 = -DBL_MAX;
        strokeData.z0 = -DBL_MAX;
        strokeData.z1 = -DBL_MAX;
        strokeData.index = readIndex;
        for (auto xyz : points)
            {
            double u = axisSelect == 0 ? xyz.x : xyz.y;
            if (u < strokeData.u0)
                {
                strokeData.u0 = u;
                strokeData.z0 = xyz.z;
                }
            if (u > strokeData.u1)
                {
                strokeData.u1 = u;
                strokeData.z1 = xyz.z;
                }
            }
        data.push_back (strokeData);
        }
    }

//!<ul>
//!<li> if u is a simple interior coordinate in the pick, interpolate to its z as a valid result
//!<li> if u exactly matches both u0 and u1, return the z0..z1 range sorted as a valid result
//!<li> if u is outside and u0==u1, return the z0..z1 range sorted but not valid
//!<li> if u is outside and u0 differs from u1 return the nearer z but not valid
//!<li> i.e. IsValid means "strictly in the interval"
//!</ul>
ValidatedDSegment1d PolyfacePolygonPicker::StrokePick::InteriorZAtU (double u) const
    {
    if (u0 == u1)
        {
        return ValidatedDSegment1d (
                DSegment1d (DoubleOps::Min (z0, z1), DoubleOps::Max (z0, z1)),
                u == u0);
        }
    else
        {
        // u0 < u1, strictly .
        BeAssert (u0 < u1);
        if (u < u0)
            return ValidatedDSegment1d (DSegment1d (z0), false);
        else if (u > u1)
            return ValidatedDSegment1d (DSegment1d (z1), false);
        else
            {
            double s = (u-u0) / (u1 - u0);
            double z = DoubleOps::Interpolate (z0, s, z1);
            return ValidatedDSegment1d (DSegment1d (z, z), true);
            }
        }
    }

static bool cb_strokePickU0_LT
(
const PolyfacePolygonPicker::StrokePick &dataA,
const PolyfacePolygonPicker::StrokePick &dataB
)
    {
    return dataA.u0 < dataB.u0;
    }


static void CompressPicksEntirelyToLeft (bvector<PolyfacePolygonPicker::StrokePick> &activePicks, double u0)
    {
    size_t numAccept = 0;
    for (size_t i = 0; i < activePicks.size (); i++)
        {
        if (activePicks[i].u1 >= u0)
            activePicks[numAccept++] = activePicks[i];
        }
    activePicks.resize (numAccept);
    }

bool AnySegmentHides (
double uTest,
ValidatedDSegment1d &zRangeA,
bvector<PolyfacePolygonPicker::StrokePick> &hiders
)
    {
    if (zRangeA.IsValid ())
        {
        double zA = zRangeA.Value ().GetEnd (); // highest z on segment A.
        for (auto &h : hiders)
            {
            ValidatedDSegment1d zRangeB = h.InteriorZAtU (uTest);
            if (zRangeB.IsValid ())
                {
                if (zRangeB.Value ().GetEnd () > zA)
                    return true;        // The B segment has a z above the top of A, so it hides A
                }
            }
        }
    return false;
    }
//! Set visibility bits in an array of stroke picks.   The array order may be changed.
//!<ul>
//!<li>isVisible is true if any portion of this facet is visible.
//!<li>isHidden is true if any portion of this facet is hidden.
//!</ul>
void PolyfacePolygonPicker::SetVisibilityBits (bvector<StrokePick> &picks)
    {
    // left = evolving array of picks that start to the left and have not ended at u0 of sweep through sorted picks.
    // right = reused array of picks that are found by lookahead from current pick.
    bvector<StrokePick> left, right;    
    bvector<double> uBreak;
    // Sort by left ends.  The right ends are ragged
    std::sort (picks.begin (), picks.end (), cb_strokePickU0_LT);
    left.clear ();

    for (size_t indexB = 0; indexB < picks.size (); indexB++)
        {
        // left carries picks "from left"
        right.clear ();
        uBreak.clear ();
        StrokePick &pickB = picks[indexB];
        pickB.isVisible = pickB.isHidden = false;
        CompressPicksEntirelyToLeft (left, pickB.u0);
        double u1 = pickB.u1;
        uBreak.push_back (pickB.u0);
        uBreak.push_back (u1);
        for (size_t indexC = indexB + 1; indexC < picks.size () && picks[indexC].u0 <= u1; indexC++)
            {
            uBreak.push_back (picks[indexC].u0);
            if (picks[indexC].u1 < u1)
                uBreak.push_back (picks[indexC].u1);
            right.push_back (picks[indexC]);
            }

        std::sort (uBreak.begin (), uBreak.end ());
        pickB.isHidden = false;
        pickB.isVisible = false;
        bool done = false;
        for (size_t i = 0; !done && i + 1 < uBreak.size (); i++)
            {
            double u0 = uBreak[i];
            double u1 = uBreak[i+1];
            if (!DoubleOps::AlmostEqual (u0, u1))
                {
                double uB = 0.5 * (u0 + u1);
                ValidatedDSegment1d zB = pickB.InteriorZAtU (uB);
                if (zB.IsValid ())
                    {
                    if (AnySegmentHides (uB, zB, left) || AnySegmentHides (uB, zB, right))
                        pickB.isHidden = true;
                    else
                        pickB.isVisible = true;
                    done = pickB.isVisible && pickB.isHidden;   // if both have gone true, nothing more to learn.
                    }
                }
            }
        left.push_back (pickB);
        }
    }


PolyfacePolygonPicker::SearchState PolyfacePolygonPicker::AppendHitsByBox
(
DRange2dCR pickBox,                 //!< [in] local coordinates of pick box
bvector<size_t> *allIn,             //!< [inout] (optional) growing array of readIndices of facets completely inside
bvector<size_t> *allOut,            //!< [inout] (optional) growing array of readIndices of facets completely outside
bvector<size_t> *crossing,          //!< [inout] (optional) growing array of readIndices of facets that have both inside and outside portions.
size_t maxTest,                     //!< [in] maximum number of new facets to test
size_t maxNewHit                    //!< [in] maximum number of new hits to record
)
    {
    size_t numNewHit = 0;
    size_t numTest = 0;
    bvector<DPoint3d> &points = m_visitor->Point ();


    bvector<DPoint3d> convexPoints
        {
        DPoint3d::From (pickBox.low.x, pickBox.low.y, 0.0),
        DPoint3d::From (pickBox.high.x, pickBox.low.y, 0.0),
        DPoint3d::From (pickBox.high.x, pickBox.high.y, 0.0),
        DPoint3d::From (pickBox.low.x, pickBox.high.y, 0.0),
        DPoint3d::From (pickBox.low.x, pickBox.low.y, 0.0)
        };

    ConvexClipPlaneSet convexSet;
    convexSet.ReloadSweptConvexPolygon (convexPoints, DVec3d::From (0,0,1), 0);
    bvector<DPoint3d> clippedPolygon, work;
    while (m_visitor->AdvanceToNextFace ())
        {
        if (numTest >= maxTest)
            return SearchState::SuspendByMaxTest;
        size_t ri = m_visitor->GetReadIndex ();
        m_worldToLocal.MultiplyAndRenormalize (points);
        numTest++;
        convexSet.ConvexPolygonClip (points, clippedPolygon, work);
        bvector<size_t> *dest = nullptr;
        if (clippedPolygon.size () == 0)
            dest = allOut;
        else
            {
            double area0 = PolygonOps::AreaXY (points);
            double area1 = PolygonOps::AreaXY (clippedPolygon);
            if (DoubleOps::AlmostEqual (area1, area0))
                dest = allIn;
            else
                dest = crossing;
            }

        if (dest != nullptr)
            {
            dest->push_back (ri);
            numNewHit++;
            if (numNewHit > maxNewHit)
                return SearchState::SuspendByMaxNewHit;
            }
        }
    return SearchState::Complete;
    }

void PolyfacePolygonPicker::SelectVisibleFacets
(
bvector<size_t> *candidateReadIndex,  //!< [inout] read indices of candidates.  This will be sorted !!!
bvector<size_t> &visibleReadIndex       //!< [out] read indices of visible candidates.
)
    {
    visibleReadIndex.clear ();
    // sort to prevent duplicates.
    TaggedPolygonVector polygons, visiblePolygons;
    bvector<DPoint3d> &visitorPoints = m_visitor->Point ();
    if (nullptr != candidateReadIndex)
        {
        std::sort (candidateReadIndex->begin (), candidateReadIndex->end ());

        // only visit each facet once ...
        size_t readIndex0 = SIZE_MAX;
        for (size_t readIndex : *candidateReadIndex)
            {
            // (ignore repetitions and misses ...)
            if (readIndex != readIndex0 && m_visitor->MoveToFacetByReadIndex (readIndex))
                {
                PolygonVectorOps::AddPolygon (
                        polygons,
                        visitorPoints, 0, readIndex);
                }
            readIndex0 = readIndex;
            }
        }
     else
        {
        for (m_visitor->Reset (); m_visitor->AdvanceToNextFace ();)
                PolygonVectorOps::AddPolygon (polygons,
                        visitorPoints, 0, m_visitor->GetReadIndex ());
        }

    // find ALL visibles ..
    PolygonVectorOps::MultiplyAndRenormalize (polygons, m_worldToLocal);
    bsiPolygon_clipByXYVisibility (polygons, visiblePolygons, true, false);

    size_t readIndex0 = SIZE_MAX;
    for (auto &v : visiblePolygons)
        {
        size_t readIndex1 = v.GetIndexB ();
        if (readIndex1 != readIndex0)
            visibleReadIndex.push_back (readIndex1);
        readIndex0 = readIndex1;
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
