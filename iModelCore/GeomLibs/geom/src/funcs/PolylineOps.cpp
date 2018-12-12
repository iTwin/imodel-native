/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/PolylineOps.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

double PolylineOps::SegmentFractionToPolylineFraction (size_t segmentIndex, size_t numSegment, double segmentFraction)
    {
    if (numSegment == 0)
        return segmentFraction;
    return ((double)segmentIndex + segmentFraction) / (double) numSegment;
    }


bool PolylineOps::PolylineFractionsOnSameSegment
(
double f0,
double f1,
size_t numVertex
)
    {
    if (numVertex <= 2)
        return true;
    size_t numSegment = numVertex - 1;
    double df = 1.0 / (double)numSegment;
    if (f0 <= df && f1 <= df)
        return true;
    if (f0 >= 1.0 - df && f1 >= 1.0 - df)
        return true;
    size_t segmentIndex0 = (size_t)(f0 * numSegment);
    size_t segmentIndex1 = (size_t)(f1 * numSegment);
    return segmentIndex0 == segmentIndex1;
    }

bool PolylineOps::PolylineFractionToSegmentData
(
size_t numVertex,
double fraction,
size_t &segmentIndex,
size_t &numSegment,
double &segmentFraction,
bool &isExtrapolated
)
    {
    segmentIndex = 0;
    segmentFraction = 0.0;
    isExtrapolated = false;
    numSegment = 0;
    if (numVertex < 2)
        return false;
    numSegment = numVertex - 1;
    double df = 1.0 / (double) numSegment;
    if (fraction < df)
        {
        segmentIndex = 0;
        segmentFraction = fraction * numSegment;
        isExtrapolated = true;
        return true;
        }
    else if (fraction >= 1.0 - df)
        {
        segmentIndex = numVertex - 2;
        segmentFraction = (fraction - (1.0 - df)) / df;
        isExtrapolated = false;
        return true;
        }
    segmentIndex = (int)(fraction * numSegment);
    double localFraction = fraction - segmentIndex * df;
    segmentFraction = localFraction / df;
    if (segmentFraction >= 1.0)
        {
        segmentIndex++;
        segmentFraction -= 1.0;
        }
    isExtrapolated = false;
    return true;
    }

// Find a point, strictly before index i0, to complete a frame definition.
// @param [in] frameOrigin frame origin point.  May be different from point0.
// @param [in] point0 start point for x axis
// @param [in] point1 end point for x axis
// @param [out] frame computed frame
// @param [out] i1 index of point used.
bool SearchForFrameCompletionBefore
(
bvector<DPoint3d> const&points,
size_t i0,
DPoint3dCR frameOrigin,
DPoint3dCR point0,
DPoint3dCR point1,
TransformR frame,
size_t &i1
)
    {
    size_t n = points.size ();
    i1 = i0;
    if (i1 >= n)
        i1 = n;
    while (i1 > 0)
        {
        i1--;
        if (frame.InitNormalizedFrameFromOriginXPointYPoint (point0, point1, points[i1]))
            {
            frame.SetTranslation (frameOrigin);
            return true;
            }
        }
    return false;
    }

// Find a point, strictly before index i0, to complete a frame definition.
// @param [in] frameOrigin frame origin point.  May be different from point0.
// @param [in] point0 start point for x axis.  May be different from frameOrigin.
// @param [in] point1 end point for x axis
// @param [out] frame computed frame
// @param [out] i1 index of point used.
bool SearchForFrameCompletionAtOrAfter
(
bvector<DPoint3d> const&points,
size_t i0,
DPoint3dCR frameOrigin,
DPoint3dCR point0,
DPoint3dCR point1,
TransformR frame,
size_t &i1
)
    {
    size_t n = points.size ();
    i1 = i0;
    while (i1 < n)
        {
        if (frame.InitNormalizedFrameFromOriginXPointYPoint (point0, point1, points[i1]))
            {
            frame.SetTranslation (frameOrigin);
            return true;
            }
        i1++;
        }
    return false;
    }

//! Search for closest point on edge. Optionally add closure edge.
bool PolylineOps::ClosestPoint
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        double &globalFraction,
        DPoint3dR curvePoint
        )
    {
    size_t edgeIndex, numEdge;
    double edgeFraction;
    return ClosestPoint (xyz, addClosurePoint, spacePoint, globalFraction, curvePoint, edgeIndex, numEdge, edgeFraction, false, false);
    }

//! Search for closest point on edge. Optionally add closure edge.
bool PolylineOps::ClosestPoint
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        double &globalFraction,
        DPoint3dR curvePoint,
        size_t &edgeIndex,
        size_t &numEdge,
        double &edgeFraction,
        bool extend0,
        bool extend1
        )
    {
    size_t n = xyz.size ();
    globalFraction = edgeFraction = 0.0;
    edgeIndex = numEdge = 0;
    curvePoint.Zero ();
    if (n == 0)
        return false;
    numEdge    = n - 1;

    double d2Min = spacePoint.DistanceSquared (xyz[0]);
    double d2;
    DVec3d edgeVector;
    DVec3d spaceVector;
    curvePoint = xyz[0];    // default (zeros) for edgeIndex, edgeFraction are ok.

    if (n == 1)
        return true;

    double df = 1.0 / (double)(n - 1);
    for (size_t i = 1; i < n; i++)
        {
        spaceVector = DVec3d::FromStartEnd (xyz[i-1], spacePoint);
        edgeVector = DVec3d::FromStartEnd (xyz[i-1], xyz[i]);

        double edgeDot;
        double spaceDot = edgeVector.DotProduct (spaceVector);
        if (spaceDot < 0.0)
            {
            // Start point is closest point of this edge.  It has already been considered.
            }
        else if ((edgeDot = edgeVector.DotProduct (edgeVector)) <= spaceDot)
            {
            // End point is closest point of this edge.
            d2 = xyz[i].DistanceSquared (spacePoint);
            if (d2 < d2Min)
                {
                d2Min = d2;
                edgeIndex = i - 1;
                edgeFraction = 1.0;
                globalFraction = (i < n - 1) ? i * df : 1.0;
                curvePoint = xyz[i];
                }
            }
        else
            {
            double thisEdgeFraction = spaceDot / edgeDot;
            DPoint3d curveCandidate;
            curveCandidate.Interpolate (xyz[i-1], thisEdgeFraction, xyz[i]);
            d2 = curveCandidate.DistanceSquared (spacePoint);
            if (d2 < d2Min)
                {
                d2Min = d2;
                edgeIndex = i - 1;
                edgeFraction = thisEdgeFraction;
                globalFraction = (edgeIndex + edgeFraction) * df;
                curvePoint = curveCandidate;
                }
            }
        }

    // consider extension only from an endpoint that is closest
    DSegment3d segmentA;
    DPoint3d pointA;
    double fractionA;
    if (extend0 && n >= 2)
        {
        // consider extension if point 0 is the close point
        if (edgeIndex == 0 && edgeFraction == 0.0)
            {
            segmentA = DSegment3d::From (xyz[0], xyz[1]);
            segmentA.ProjectPointBounded (pointA, fractionA, spacePoint, true, false);
            if (fractionA < 0.0)
                {
                edgeFraction = fractionA;
                globalFraction = fractionA * df;
                curvePoint = pointA;
                }
            }
        }

    if (extend1 && n >= 2)
        {
        if (edgeIndex == n - 2 && edgeFraction == 1.0)
            {
            segmentA = DSegment3d::From (xyz[n-2], xyz[n-1]);
            segmentA.ProjectPointBounded (pointA, fractionA, spacePoint, false, true);
            if (fractionA > 1.0)
                {
                edgeFraction = fractionA;
                globalFraction = (edgeIndex + edgeFraction) * df;
                curvePoint = pointA;
                }
            }
        }
        
    return true;
    }

//! return true if there are an intersections within the linestring.
bool PolylineOps::IsSelfIntersectingXY (bvector<DPoint3d> const &xyz, bool addClosure)
    {
    return xyz.size () > 2
        && bsiDPoint3dArray_isSelfIntersectingXY (&xyz[0], (int)xyz.size (), addClosure);
    }
//! return true if there are any intersections within the linestring.
bool PolylineOps::IsSelfIntersectingXY (DPoint3d *points, size_t n, bool addClosure)
    {
    return n > 2 
        && bsiDPoint3dArray_isSelfIntersectingXY (points, (int)n, addClosure);
    }

bool PolylineOps::Are4EdgesPerpendicular (DPoint3dCP points)
    {
    DVec3d vector[5];
    vector[0] = DVec3d::FromStartEnd (points[3], points[0]);
    vector[1] = DVec3d::FromStartEnd (points[0], points[1]);
    vector[2] = DVec3d::FromStartEnd (points[1], points[2]);
    vector[3] = DVec3d::FromStartEnd (points[2], points[3]);
    vector[4] = vector[0];
    // DVec3d vectorX = vector[1];
    // DVec3d vectorY = DVec3d::FromStartEnd (points[0], points[3]);
    double thetaMax = 0.0;
    // static double s_toleranceFactor = 100.0;
    for (int i = 0; i < 4; i++)
        thetaMax = DoubleOps::Max (thetaMax, vector[i].AngleFromPerpendicular (vector[i+1]));
    return thetaMax <= DoubleOps::SmallCoordinateRelTol ();
    }

bool PolylineOps::IsRectangle
(
DPoint3dCP points, size_t n,
TransformR localToWorld,
TransformR worldToLocal,
bool requireClosurePoint
)
    {
    localToWorld.InitIdentity ();
    worldToLocal.InitIdentity ();
    
    if (requireClosurePoint && n != 5)
        return false;
    if (!requireClosurePoint)
        {
        if (n != 4 && n != 5)
           return false;
        }
    if (n == 5)
        if (!DPoint3dOps::AlmostEqual (points[0], points[4]))
            return false;

    if (!Are4EdgesPerpendicular (points))
        return false;
    DVec3d vectorX = DVec3d::FromStartEnd (points[0], points[1]);
    DVec3d vectorY = DVec3d::FromStartEnd (points[0], points[3]);

    DVec3d vectorZ = DVec3d::FromNormalizedCrossProduct (vectorX, vectorY);
    localToWorld.InitFromOriginAndVectors (points[0], vectorX, vectorY, vectorZ);
    return worldToLocal.InverseOf (localToWorld);
    }

bool PolylineOps::IsRectangle
(
bvector<DPoint3d> const &points,
TransformR localToWorld,
TransformR worldToLocal,
bool requireClosurePoint
)
    {
    size_t n = points.size ();
    if (n > 0)
        return IsRectangle (&points[0], n, localToWorld, worldToLocal, requireClosurePoint);
    return false;
    }

bool PolylineOps::ClosestPointXY
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        DMatrix4dCP worldToLocal,
        double &globalFraction,
        DPoint3dR curvePoint,
        size_t &edgeIndex,
        size_t &numEdge,
        double &edgeFraction,
        double &xyDistance
        )
    {
    return ClosestPointXY (xyz, addClosurePoint, spacePoint, worldToLocal, globalFraction,
                curvePoint, edgeIndex, numEdge, edgeFraction, xyDistance,
                false, false);
    }

//! Search for closest point on edge, using xy coordinates after optional projection. Optionally add closure edge.
bool PolylineOps::ClosestPointXY
        (
        bvector<DPoint3d> const &xyz,
        bool addClosurePoint,
        DPoint3dCR spacePoint,
        DMatrix4dCP worldToLocal,
        double &globalFraction,
        DPoint3dR curvePoint,
        size_t &edgeIndex,
        size_t &numEdge,
        double &edgeFraction,
        double &xyDistance,
        bool extend0,
        bool extend1
        )
    {
    size_t n = xyz.size ();
    globalFraction = edgeFraction = 0.0;
    edgeIndex = numEdge = 0;
    curvePoint.Zero ();
    xyDistance = DBL_MAX;
    if (n == 0)
        return false;
    numEdge    = n - 1;
    double paramB, distanceB;
    DPoint3d pointB;

    curvePoint = xyz[0];    // default (zeros) for edgeIndex, edgeFraction are ok.

    if (n == 1)
        return true;

    if (addClosurePoint)
        {
        extend0 = extend1 = false;
        }
    double df = 1.0 / (double)(n - 1);
    DPoint3d basePoint;
    size_t i;
    if (addClosurePoint)
        {
        i = 0;
        basePoint = xyz[n-1];
        }
    else
        {
        i = 1;
        basePoint = xyz[0];
        }
    for (size_t i = 1; i < n; i++)
        {
        DSegment3d segment;
        segment.point[0] = xyz[i-1];
        segment.point[1] = xyz[i];
        segment.ClosestPointBoundedXY (pointB, paramB, distanceB, spacePoint, worldToLocal,
                i == 1 ? extend0 : false,
                i+1 == n ? extend1 : false
                );
        if (distanceB < xyDistance)
            {
            xyDistance = distanceB;
            edgeFraction = paramB;
            edgeIndex    = i - 1;
            globalFraction = (i - 1 + edgeFraction)*df;
            curvePoint = pointB;
            }
        }
    return true;
    }

double PolylineOps::Length (bvector<DPoint3d> const &xyz, bool addClosure)
    {
    size_t count = xyz.size ();
    double a = 0.0;
    DPoint3d xyz0, xyz1;
    // Sum in blocks starting with a non-disconnect
    for (size_t i = 0; i < count; i++)
        {
        if (!xyz[i].IsDisconnect ())
            {
            xyz0 = xyz1 = xyz[i++];
            for (;i < count && !xyz[i].IsDisconnect (); i++)
                {
                a += xyz1.Distance (xyz[i]);
                xyz1 = xyz[i];
                }
            if (addClosure)
                a += xyz1.Distance (xyz0);
            }
        }
    return a;
    }

double PolylineOps::Length (bvector<bvector<DPoint3d>> const &xyz, bool addClosure)
    {
    size_t numLoops = xyz.size ();
    double a = 0.0;
    for (size_t i = 0; i < numLoops; i++)
        a += PolylineOps::Length (xyz[i], addClosure);
    return a;
    }

static double Distance (RotMatrixCP worldToLocal, DPoint3dCR point0, DPoint3dCR point1)
    {
    DVec3d vector = DVec3d::FromStartEnd (point0, point1);
    if (nullptr != worldToLocal)
        worldToLocal->Multiply (vector);
    return vector.Magnitude ();
    }

double PolylineOps::Length (RotMatrixCP worldToLocal, bvector<DPoint3d> const &xyz, bool addClosure)
    {
    size_t count = xyz.size ();
    double a = 0.0;
    DPoint3d xyz0, xyz1;
    // Sum in blocks starting with a non-disconnect
    for (size_t i = 0; i < count; i++)
        {
        if (!xyz[i].IsDisconnect ())
            {
            xyz0 = xyz1 = xyz[i++];
            for (;i < count && !xyz[i].IsDisconnect (); i++)
                {
                a += Distance (worldToLocal, xyz1, xyz[i]);
                xyz1 = xyz[i];
                }
            if (addClosure)
                a += Distance (worldToLocal, xyz1, xyz0);
            }
        }
    return a;
    }

static double s_divideLimit = 1.0e-13;
static bool unweight (DPoint3dCR xyzw, double w, DPoint3dR xyz)
    {
    xyz = xyzw;
    if (fabs (w) <= s_divideLimit)
        return false;
    if (w == 1.0)
        {
        // leave it alone.
        }
    else
        {
        double a = 1.0 / w;
        xyz.x = xyzw.x * a;
        xyz.y = xyzw.y * a;
        xyz.z = xyzw.z * a;
        }
    return true;
    }

static bool unweight (DPoint3dCP xyzw, double const *weight, size_t index, DPoint3dR xyz)
    {
    xyz = xyzw [index];
    double w = weight[index];
    if (fabs (w) <= s_divideLimit)
        return false;
    if (w == 1.0)
        {
        // leave it alone.
        }
    else
        {
        double a = 1.0 / w;
        xyz.x *= a;
        xyz.y *= a;
        xyz.z *= a;
        }
    return true;
    }


double PolylineOps::Length (DPoint3dCP xyz, double const *weight, ptrdiff_t step, size_t count, bool addClosure)
    {
    double a = 0.0;
    if (count == 0.0)
        {
        }
    else if (NULL == weight)
        {
        for (size_t i = 1, k = step; i < count; i++, k += step)
            a += xyz[k - step].Distance (xyz[k]);
        if (addClosure)
            a += xyz[(count - 1) * step].Distance (xyz[0]);
        }
    else
        {
        DPoint3d xyz0, xyzWrap, xyz1;
        unweight (xyz[0], weight[0], xyz0);
        xyzWrap = xyz0;
        for (size_t i = 1, k = step; i < count; i++, xyz0 = xyz1, k += step)
            {
            unweight (xyz[k], weight[k], xyz1);
            a += xyz0.Distance (xyz1);
            }
        if (addClosure)
            a += xyz0.Distance (xyzWrap);
        }
    return a;
    }

static double TurnAngle (DPoint3dCR xyz0, DPoint3dCR xyz1, DPoint3dCR xyz2)
    {
    DVec3d vector01 = DVec3d::FromStartEnd (xyz0, xyz1);
    DVec3d vector12 = DVec3d::FromStartEnd (xyz1, xyz2);
    return vector01.AngleTo (vector12);
    }

double PolylineOps::SumAbsoluteAngles (DPoint3dCP xyz, double const *weight, ptrdiff_t step, size_t count, bool addClosure, double minEdgeLength)
    {
    double a = 0.0;
    if (count < 3)
        return 0.0;
    DPoint3d xyz0, xyz1, xyz2;
    double d01, d12;
    size_t iStart;
    if (NULL == weight)
        {
        if (addClosure)
            {
            iStart = 0;
            xyz0 = xyz[step * (count-2)];
            xyz1 = xyz[step * (count-1)];
            }
        else
            {
            iStart = 2;
            xyz0 = xyz[0];
            xyz1 = xyz[step];
            }
        d01 = xyz0.Distance (xyz1);
        for (size_t i = iStart, k = step * iStart; i < count; i++, xyz0 = xyz1, xyz1 = xyz2, k += step, d01 = d12)
            {
            xyz2 = xyz[k];
            d12 = xyz1.Distance (xyz2);
            if (d12 > minEdgeLength && d01 > minEdgeLength)
                a += TurnAngle (xyz0, xyz1, xyz2);
            }
        }
    else
        {
        if (addClosure)
            {
            iStart = 0;
            unweight (xyz, weight, step * (count - 2), xyz0);
            unweight (xyz, weight, step * (count - 1), xyz1);
            }
        else
            {
            iStart = 2;
            unweight (xyz, weight, 0, xyz0);
            unweight (xyz, weight, step, xyz1);
            }
        d01 = xyz0.Distance (xyz1);
        for (size_t i = iStart, k = step * iStart; i < count; i++, xyz0 = xyz1, xyz1 = xyz2, k += step, d01 = d12)
            {
            unweight (xyz, weight, k, xyz2);
            d12 = xyz1.Distance (xyz2);
            if (d12 > minEdgeLength && d01 > minEdgeLength)
                a += TurnAngle (xyz0, xyz1, xyz2);
            }
        }
    return a;
    }


double PolylineOps::SegmentLength (bvector<DPoint3d> const &points, size_t index)
    {
    if (index + 1 < points.size ())
        {
        return points[index].Distance(points[index+1]);
        }
    return 0.0;
    }

double PolylineOps::SegmentLength (RotMatrixCP worldToLocal, bvector<DPoint3d> const &points, size_t index)
    {
    if (index + 1 < points.size ())
        {
        return Distance (worldToLocal, points[index], points[index+1]);
        }
    return 0.0;
    }

ValidatedDRay3d PolylineOps::PolylineFractionToRay (bvector<DPoint3d> const &points, double polylineFraction)
    {
    size_t numPoints = points.size ();
    if (numPoints == 0)
        return ValidatedDRay3d (DRay3d::FromOriginAndVector (DPoint3d::From (0,0,0), DVec3d::From (0,0,0)));
    else if (numPoints == 1)
        return ValidatedDRay3d (DRay3d::FromOriginAndVector (points[0], DVec3d::From (0,0,0)));

    size_t numSegment, segmentStartIndex;
    double segmentFraction;
    bool isExtrapolated;
    // this is guaranteed to succeed ..
    PolylineFractionToSegmentData (numPoints, polylineFraction, segmentStartIndex, numSegment, segmentFraction, isExtrapolated);
    return ValidatedDRay3d (DRay3d::FromIinterpolateWithScaledDifference (
                points[segmentStartIndex],
                segmentFraction,
                points[segmentStartIndex + 1],
                (double)numSegment)
                );
    }

DPoint3d PolylineOps::SegmentFractionToPoint (bvector<DPoint3d> const &points, size_t index, double edgeFraction)
    {
    size_t numVertex = points.size ();
    if (numVertex > 1)
        return DPoint3d::FromInterpolate (points[index], edgeFraction, points[index+1]);
    else if (numVertex == 1)
        return points[0];
    DPoint3d zero;
    zero.Zero ();
    return zero;
    }

bool PushDistinct (bvector<DPoint3d> &points, DPoint3dCR xyz)
    {
    if (points.size () == 0
        || !xyz.IsEqual (points.back ()))
        {
        points.push_back (xyz);
        return true;
        }
    return false;
    }

//! Search (forward or reverse, according to sign of requested distance).
void PolylineOps::CopyBetweenFractions
(
bvector <DPoint3d> const &points,
bvector<DPoint3d> &dest,
double fractionA,
double fractionB
)
    {
    size_t numVertex = points.size ();
    dest.clear ();
    double uA, uB;
    size_t indexA, indexB, numSegment;
    bool extrapolatedA, extrapolatedB;
    if (   PolylineFractionToSegmentData (numVertex, fractionA, indexA, numSegment, uA, extrapolatedA)
        && PolylineFractionToSegmentData (numVertex, fractionB, indexB, numSegment, uB, extrapolatedB))
        {
        dest.push_back (DPoint3d::FromInterpolate (points[indexA], uA, points[indexA+1]));
        if (indexA < indexB)
            {
            size_t lastIndex = uB > 0.0 ? indexB : indexB - 1;
            for (size_t index = indexA + 1; index <= lastIndex; index++)
                PushDistinct (dest, points[index]);
            }
        else if (indexA > indexB)
            {
            size_t index = uA > 0.0 ? indexA : indexA - 1;
            for (; index > indexB; index--)
                PushDistinct (dest, points[index]);
            }
        PushDistinct (dest, DPoint3d::FromInterpolate (points[indexB], uB, points[indexB+1]));
        }
    }

bool PushDistinctXYZ (bvector<CurveLocationDetail> &points, CurveLocationDetailCR data)
    {
    if (points.size () == 0 || !data.point.AlmostEqual (points.back ().point))
        {
        points.push_back (data);
        return true;
        }
    return false;
    }

//! Search (forward or reverse, according to sign of requested distance).
void PolylineOps::CopyBetweenFractions
(
bvector <DPoint3d> const &points,
bvector<CurveLocationDetail> &dest,
double fractionA,
double fractionB
)
    {
    size_t numVertex = points.size ();
    dest.clear ();
    double uA, uB;
    size_t indexA, indexB, numSegment;
    bool extrapolatedA, extrapolatedB;
    if (   PolylineOps::PolylineFractionToSegmentData (numVertex, fractionA, indexA, numSegment, uA, extrapolatedA)
        && PolylineOps::PolylineFractionToSegmentData (numVertex, fractionB, indexB, numSegment, uB, extrapolatedB))
        {
        double df = 1.0 / numSegment;
        PushDistinctXYZ (dest,CurveLocationDetail (nullptr, fractionA,
                        DPoint3d::FromInterpolate (points[indexA], uA, points[indexA+1]),
                        indexA, numSegment, uA));

        if (indexA < indexB)
            {
            size_t lastIndex = uB > 0.0 ? indexB : indexB - 1;
            for (size_t index = indexA + 1; index <= lastIndex; index++)
                  PushDistinctXYZ (dest,CurveLocationDetail (nullptr, index * df,
                        points[index],
                        index, numSegment, 0.0));
            }
        else if (indexA > indexB)
            {
            size_t index = uA > 0.0 ? indexA : indexA - 1;
            for (; index > indexB; index--)
                  PushDistinctXYZ (dest,CurveLocationDetail (nullptr, index * df,
                        points[index],
                        index, numSegment, 0.0));
            }
        PushDistinctXYZ (dest,CurveLocationDetail (nullptr, fractionB,
                        DPoint3d::FromInterpolate (points[indexB], uB, points[indexB+1]),
                        indexB, numSegment, uB));
        }
    }



double PolylineOps::SignedDistanceBetweenFractions
(
bvector <DPoint3d> const &points,
double fraction0,
double fraction1
)
    {
    size_t index0, index1;
    double segmentFraction0, segmentFraction1;
    size_t numSegment;
    double s = fraction0 < fraction1 ? 1.0 : -1.0;
    double length = 0.0;
    bool extrapolate0, extrapolate1;
    if (   PolylineFractionToSegmentData (points.size (), DoubleOps::Min (fraction0, fraction1), index0, numSegment, segmentFraction0, extrapolate0)
        && PolylineFractionToSegmentData (points.size (), DoubleOps::Max (fraction0, fraction1), index1, numSegment, segmentFraction1, extrapolate1))
        {
        if (index0 == index1)
            {
            length = SegmentLength (points, index0) * (segmentFraction1 - segmentFraction0);
            }
        else
            {
            length = (1.0 - segmentFraction0) * SegmentLength(points, index0);
            for (size_t index = index0 + 1; index < index1; index++)
                length += SegmentLength (points, index);
            length += segmentFraction1 * SegmentLength (points, index1);
            }
        }
    return s * length;
    }

double PolylineOps::SignedDistanceBetweenFractions
(
RotMatrixCP worldToLocal, 
bvector <DPoint3d> const &points,
double fraction0,
double fraction1
)
    {
    size_t index0, index1;
    double segmentFraction0, segmentFraction1;
    size_t numSegment;
    double s = fraction0 < fraction1 ? 1.0 : -1.0;
    double length = 0.0;
    bool extrapolate0, extrapolate1;
    if (   PolylineFractionToSegmentData (points.size (), DoubleOps::Min (fraction0, fraction1), index0, numSegment, segmentFraction0, extrapolate0)
        && PolylineFractionToSegmentData (points.size (), DoubleOps::Max (fraction0, fraction1), index1, numSegment, segmentFraction1, extrapolate1))
        {
        if (index0 == index1)
            {
            length = SegmentLength (worldToLocal, points, index0) * (segmentFraction1 - segmentFraction0);
            }
        else
            {
            length = (1.0 - segmentFraction0) * SegmentLength(worldToLocal, points, index0);
            for (size_t index = index0 + 1; index < index1; index++)
                length += SegmentLength (worldToLocal, points, index);
            length += segmentFraction1 * SegmentLength (worldToLocal, points, index1);
            }
        }
    return s * length;
    }


static bool PolylineFractionAtSignedDistanceFromFraction
(
RotMatrixCP worldToLocal, 
bvector<DPoint3d> const &xyz,
double startFraction,
double signedDistance,
double &endFraction,
size_t &endSegmentIndex,
double &segmentFraction,
double &actualDistance
)
    {
    size_t      currSegmentIndex;
    double      startFractionInSegment;
    bool        isExtrapolated;

    size_t      numVertex = xyz.size ();
    size_t      numSegment;
    actualDistance = 0.0;

    endFraction = startFraction;
    endSegmentIndex = 0;
    segmentFraction = 0.0;
    if (!PolylineOps::PolylineFractionToSegmentData (numVertex, startFraction, currSegmentIndex, numSegment, startFractionInSegment, isExtrapolated))
        return false;

    //double accumulatedDistance = 0.0;
    double absoluteDistanceRemaining = fabs (signedDistance);
    double segmentStep;
    if (signedDistance >=  0.0)
        {
        // Forward walk ...
        for (;; currSegmentIndex++, startFractionInSegment = 0.0)
            {
            double segmentLength = Distance (worldToLocal, xyz[currSegmentIndex], xyz[currSegmentIndex + 1]);
            double availableInSegment = (1.0 - startFractionInSegment ) * segmentLength;
            if (availableInSegment >= absoluteDistanceRemaining || currSegmentIndex == (numSegment - 1))
                {
                actualDistance = signedDistance;
                DoubleOps::SafeDivideParameter (segmentStep, absoluteDistanceRemaining, segmentLength, 0.0);
                segmentFraction = startFractionInSegment + segmentStep;
                endSegmentIndex = currSegmentIndex;
                endFraction  = PolylineOps::SegmentFractionToPolylineFraction (currSegmentIndex, numSegment, segmentFraction);
                return true;  // Interesting case:   segmentLength==0.  (Necessarily absoluteDistanceRemaining is 0)  Accept the fallback 0.0.
                }
            actualDistance += availableInSegment;
            absoluteDistanceRemaining -= availableInSegment;
            }
        }
    else
        {
        // Reverse walk ...
        for (;; currSegmentIndex--, startFractionInSegment = 1.0)
            {
            double segmentLength = Distance (worldToLocal, xyz[currSegmentIndex], xyz[currSegmentIndex + 1]);
            double availableInSegment = startFractionInSegment * segmentLength;
            if (availableInSegment >= absoluteDistanceRemaining || currSegmentIndex == 0)
                {
                actualDistance = signedDistance;
                DoubleOps::SafeDivideParameter (segmentStep, absoluteDistanceRemaining, segmentLength, 0.0);
                endSegmentIndex = currSegmentIndex;
                segmentFraction = startFractionInSegment - segmentStep;
                endFraction  = PolylineOps::SegmentFractionToPolylineFraction (currSegmentIndex, numSegment, segmentFraction);
                return true;
                }
            actualDistance -= availableInSegment;
            absoluteDistanceRemaining -= availableInSegment;
            }
        }
    // arg.  really should not get here...
    //return false;
    }

bool PolylineOps::FractionAtSignedDistanceFromFraction
(
RotMatrixCP worldToLocal,
bvector<DPoint3d> const &xyz,
double startFraction,
double signedDistance,
double &endFraction,
size_t &endSegmentIndex,
double &segmentFraction,
double &actualDistance
)
    {
    return PolylineFractionAtSignedDistanceFromFraction (worldToLocal, xyz, startFraction, signedDistance, endFraction, endSegmentIndex, segmentFraction, actualDistance);
    }

bool PolylineOps::FractionAtSignedDistanceFromFraction
(
bvector<DPoint3d> const &xyz,
double startFraction,
double signedDistance,
double &endFraction,
size_t &endSegmentIndex,
double &segmentFraction,
double &actualDistance
)
    {
    return PolylineFractionAtSignedDistanceFromFraction (nullptr, xyz, startFraction, signedDistance, endFraction, endSegmentIndex, segmentFraction, actualDistance);
    }


bool PolylineOps::FractionToFrenetFrame
(
bvector <DPoint3d> const & points,
double f,
TransformR frame
)
    {
    size_t      segmentStartIndex;
    double      segmentFraction;
    bool        isExtrapolated;
    size_t      numSegment, numPoints = points.size ();
    DPoint3d    point, p0, p1;

    if (numPoints < 2)
        return false;
    
    if (!PolylineFractionToSegmentData (numPoints, f, segmentStartIndex, numSegment, segmentFraction, isExtrapolated))
        return false;

    point = SegmentFractionToPoint (points, segmentStartIndex, segmentFraction);

    // We need another point to determine the plane.
    // Hmmm.. Fussing about coincident points is annoying.
    //        Fussing about colinear vectors is annoying.
    //        Fussign about both at once is REALLY annoying.
    // A simplyfing rule: "Early" data is more important than late. (We first try to answer the query using only data from the current segment and before.)
    // Note that the "origin point for vectors" is always one of the polyline points -- which is different from the interpolated edge point
    //          which is the origin point of the frame
    size_t distinctPointIndex;
    size_t segmentEndIndex = segmentStartIndex + 1;
    size_t framePointIndex;
    if (DPoint3dOps::FindNotAlmostEqualBefore (points, points[segmentEndIndex], segmentEndIndex, distinctPointIndex, p0))
        {
        p1 = points[segmentEndIndex];
        if (SearchForFrameCompletionBefore (points, distinctPointIndex, point, p0, p1, frame, framePointIndex))
            return true;
        if (SearchForFrameCompletionAtOrAfter (points, segmentEndIndex + 1, point, p0, p1, frame, framePointIndex))
            return true;
        }
    else if (DPoint3dOps::FindNotAlmostEqualAtOrAfter (points, points[segmentStartIndex], segmentEndIndex, distinctPointIndex, p1))
        {
        p0 = points[segmentStartIndex];
#ifdef THIS_CONDITION_CANT_HAPPEN
        // This is No point in looking backward for off-line point -- everything was declared equal to endpoint.  Agree?
        // if (SearchForFrameCompletionBefore (points, segmentStartIndex, point, p0, p1, frame, framePointIndex))
        //    return true;
#endif
        if (SearchForFrameCompletionAtOrAfter (points, distinctPointIndex + 1, point, p0, p1, frame, framePointIndex))
            return true;
        }
    else
        {
        // No distinct points before or after ....
        frame.InitFrom (p0);
        return false;
        }

    // Distinct points found but none off of that line....
    frame.InitFromPlaneNormalToLine (p0, p1, 0, true);
    frame.SetTranslation (point);
    return true;
    }

bool PolylineOps::WireCentroid
(
bvector <DPoint3d> const &points,
double &length,
DPoint3dR centroid,
double fraction0,
double fraction1
)
    {
    size_t n = points.size ();
    length = 0.0;    
    centroid.Zero ();
    if (n == 0)
        return false;
    if (n == 1)
        {
        length = 0.0;
        centroid = points[0];
        return true;
        }

    DRange1d activeGlobalFractions = DRange1d::From (fraction0, fraction1);
    DRange1d activeLocalFractions;
    DVec3d sum;
    DVec3d baseVector;
    DVec3d edgePoint;
    sum.Zero ();
    double a;
    double intervalFraction = 1.0 / (n - 1);
    for (size_t i = 1; i < n; i++)
        {
        DRange1d intervalFractions = DRange1d::From ((i-1) * intervalFraction, i * intervalFraction);
        if (intervalFractions.StrictlyNonEmptyFractionalIntersection (activeGlobalFractions, activeLocalFractions))
            {
            DSegment3d segment;
            segment.point[0] = points[i-1];
            segment.point[1] = points[i];
            segment.WireCentroid (a, edgePoint, activeLocalFractions.low, activeLocalFractions.high);
            baseVector.DifferenceOf (edgePoint, points[0]);
            sum.SumOf (sum, baseVector, a);
            length += a;
            }
        }
    DVec3d centroidVector;
    if (centroidVector.SafeDivide (sum, length))
        centroid.SumOf (points[0], centroidVector);
    else
        centroid = points[0];
    return true;
    }

DRange1d PolylineOps::ProjectedParameterRange
(
bvector <DPoint3d> const &points,
DRay3dCR ray,
double fraction0,
double fraction1
)
    {
    DRange1d range;
    range.InitNull ();
    double divAA = ray.direction.SafeOneOverMagnitudeSquared (0.0);
    size_t n = points.size ();
    if (n == 0)
        return range;
    if (n == 1)
        {
        return DRange1d::From (divAA * ray.DirectionDotVectorToTarget (points[0]));
        }

    fraction0 = DoubleOps::Max (fraction0, 0.0);
    fraction1 = DoubleOps::Min (fraction1, 1.0);
    if (fraction0 > fraction1)
        std::swap (fraction0, fraction1);

    size_t i0, i1;
    size_t numSegment;
    double f0, f1;  // fractions within interval.
    bool isExtrapolated;
    PolylineFractionToSegmentData (n, fraction0, i0, numSegment, f0, isExtrapolated);
    PolylineFractionToSegmentData (n, fraction1, i1, numSegment, f1, isExtrapolated);
    DPoint3d xyz;
    xyz.Interpolate (points[i0], f0, points[i1]);
    range.Extend (divAA * ray.DirectionDotVectorToTarget (xyz));
    for (size_t i = i0 + 1; i <= i1; i++)
        range.Extend (divAA * ray.DirectionDotVectorToTarget (points[i]));
    xyz.Interpolate (points[i1], f1, points[i1]);
    range.Extend (divAA * ray.DirectionDotVectorToTarget (xyz));
    return range;
    }






void PolylineOps::AddContinuationStartPoint (bvector<DPoint3d>&points, DPoint3dCR xyz, bool forceIncludeStartPoint)
    {
    
    if (   forceIncludeStartPoint
        || points.size () == 0
        || !DPoint3dOps::AlmostEqual (xyz, points.back ())
       )
        {
        DPoint3d _xyz = xyz;
        points.push_back (_xyz);
        }
    }

bool PolylineOps::AddStrokes
(
DEllipse3dCR arc,
bvector <DPoint3d> & strokes, 
IFacetOptionsCR options,
bool includeStartPoint,
double startFraction,
double endFraction
)
    {
    DEllipse3d arc1;
    if (startFraction != 0.0 || endFraction != 1.0) // exact is ok -- default calls will be exact 0 and 1
        {
        arc1 = DEllipse3d::FromFractionInterval (arc, startFraction, endFraction);
        }
    else
        arc1 = arc;

    size_t n = options.EllipseStrokeCount (arc1);
    if (n < 1)
        n = 1;
    size_t i0 = includeStartPoint ? 0 : 1;
    double df = 1.0 / (double)n;
    for (size_t i = i0; i <= n; i++)
        {
        double f = i * df;
        strokes.push_back (arc1.FractionToPoint (f));
        }
    return true;
    }


bool PolylineOps::AddStrokes
(
bvector <DPoint3d> const &points,
bvector <DPoint3d> & strokes, 
IFacetOptionsCR options,
bool includeStartPoint,
double startFraction,
double endFraction
)
    {
    if (points.size () == 0)
        {
        return false;
        }
    else if (DoubleOps::IsExact01 (startFraction, endFraction))
        {
        AddContinuationStartPoint (strokes, points[0], includeStartPoint);
        for (size_t i = 1; i < points.size (); i++)
            {
            DSegment3d localSegment = DSegment3d::From (points[i-1], points[i]);
            size_t edgeCount = options.SegmentStrokeCount (localSegment);
            if (edgeCount >= 2)
                {
                double df = 1.0 / (double) (edgeCount);
                DVec3d extent = DVec3d::FromStartEnd (localSegment.point[0], localSegment.point[1]);
                for (size_t i = 1; i < edgeCount; i++)
                    {
                    DPoint3d xyz;
                    xyz.SumOf (localSegment.point[0], extent, i * df);
                    strokes.push_back (xyz);
                    }
                }
            strokes.push_back (localSegment.point[1]);
            }
        return true;
        }
    else
        {
        bvector<DPoint3d> newPoints;
        PolylineOps::CopyBetweenFractions (points, newPoints, startFraction, endFraction);
        AddContinuationStartPoint (strokes, newPoints[0], includeStartPoint);
        for (size_t i = 1; i < newPoints.size (); i++)
            {
            DSegment3d localSegment = DSegment3d::From (newPoints[i-1], newPoints[i]);
            size_t edgeCount = options.SegmentStrokeCount (localSegment);
            if (edgeCount > 2)
                {
                double df = 1.0 / (double) (edgeCount - 1);
                DVec3d extent = DVec3d::FromStartEnd (localSegment.point[0], localSegment.point[1]);
                for (size_t i = 1; i < edgeCount - 1; i++)
                    {
                    DPoint3d xyz;
                    xyz.SumOf (localSegment.point[0], extent, i * df);
                    strokes.push_back (xyz);
                    }
                }
            strokes.push_back (localSegment.point[1]);
            }
        return true;
        }
    }

bool PolylineOps::AddStrokes
(
bvector <DPoint3d> const &points,
DPoint3dDoubleUVCurveArrays & strokes, 
IFacetOptionsCR options,
double startFraction,
double endFraction,
ICurvePrimitiveCP curve
)
    {
    ICurvePrimitiveP curve1 = const_cast <ICurvePrimitiveP> (curve);    // for saving in the stroke array
    bool reverse = false;
    if (startFraction > endFraction)
        {
        std::swap (startFraction, endFraction);
        reverse = true;
        }
    if (startFraction < 0.0)
        startFraction = 0.0;
    if (endFraction > 1.0)
        endFraction = 1.0;
    size_t size0 = strokes.m_xyz.size ();
    if (points.size () == 0)
        {
        }
    else if (points.size () == 1)
        {
        strokes.Add (points[0], 0.0, DVec3d::From (0,0,0), curve1);
        }
    else if (DoubleOps::IsExact01 (startFraction, endFraction))
        {
        double fraction0 = 0.0;
        double dfOuter = 1.0 / (double)(points.size () - 1);
        for (size_t i = 1; i < points.size (); i++)
            {
            double fraction1 = i * dfOuter;
            if (i + 1 == points.size ())
                fraction1 = 1.0;        // prevent roundoff
            DSegment3d localSegment = DSegment3d::From (points[i-1], points[i]);
            DVec3d segmentVector = DVec3d::FromStartEnd (localSegment.point[0], localSegment.point[1]);
            DVec3d segmentDerivative = segmentVector / dfOuter;
            size_t edgeCount = options.SegmentStrokeCount (localSegment);
            strokes.Add (localSegment.point[0], fraction0, segmentDerivative, curve1);
            if (edgeCount > 2)
                {
                double dfInner = 1.0 / (double) (edgeCount - 1);
                for (size_t j = 1; j < edgeCount - 1; j++)
                    {
                    double segmentFraction = j * dfInner;
                    strokes.Add (localSegment.point[0] + segmentFraction * segmentVector,
                                fraction0 + segmentFraction * dfOuter,
                                segmentDerivative,
                                curve1);
                    }
                }
            strokes.Add (localSegment.point[1], fraction1, segmentDerivative, curve1);
            fraction0 = fraction1;
            }
        }
    else
        {
        strokes.Add (PolylineFractionToRay (points, startFraction), startFraction, curve1);
        double outerFractionTol = Angle::SmallAngle ();

        size_t numVertex = points.size ();
        size_t numSegment, segmentIndex0, segmentIndex1;
        double segmentFraction0, segmentFraction1;
        bool isExtrapolated0, isExtrapolated1;
        PolylineFractionToSegmentData (numVertex, startFraction, segmentIndex0, numSegment, segmentFraction0, isExtrapolated0);
        PolylineFractionToSegmentData (numVertex, endFraction, segmentIndex1, numSegment, segmentFraction1, isExtrapolated1);
        double dfOuter = 1.0 / (double)numSegment;
        double tangentScale = (double)numSegment;
        for (size_t segmentIndex = segmentIndex0; segmentIndex < segmentIndex1; segmentIndex++)
            {
            double fraction0 = segmentIndex * dfOuter;
            double fraction1 = (segmentIndex + 1) * dfOuter;
            if (segmentIndex + 1 == points.size ())
                fraction1 = 1.0;        // prevent roundoff

            DSegment3d localSegment = DSegment3d::From (points[segmentIndex], points[segmentIndex + 1]);
            DVec3d segmentVector = DVec3d::FromStartEnd (localSegment.point[0], localSegment.point[1]);
            DVec3d segmentDerivative = segmentVector * tangentScale;
            size_t edgeCount = options.SegmentStrokeCount (localSegment);
            if (DoubleOps::ClearlyIncreasing (startFraction, fraction0, outerFractionTol))
                strokes.Add (localSegment.point[0], fraction0, segmentDerivative, curve1);
            if (edgeCount > 2)  // interior points of the segment.
                {
                double dfInner = 1.0 / (double) (edgeCount - 1);
                for (size_t j = 0; j < edgeCount; j++)
                    {
                    double segmentFraction = j * dfInner;
                    double outerFraction = fraction0 + segmentFraction * dfOuter;
                    if (DoubleOps::ClearlyIncreasing (startFraction, outerFraction, outerFractionTol)
                        && DoubleOps::ClearlyIncreasing (outerFraction, endFraction, outerFractionTol)
                        )
                        {
                        strokes.Add (localSegment.point[0] + segmentFraction * segmentVector,
                                outerFraction,
                                segmentDerivative,
                                curve1);
                        }
                    }
                }
            if (DoubleOps::ClearlyIncreasing (fraction1, endFraction, outerFractionTol))
                strokes.Add (localSegment.point[1], fraction1, segmentDerivative, curve1);
            }

        strokes.Add (PolylineFractionToRay (points, endFraction), endFraction, curve1);
        }
    size_t size1 = strokes.m_xyz.size ();
    if (reverse)
        strokes.ReverseXFUVC (size0, size1, false, true, true);
    return size1 > size0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2012
+--------------------------------------------------------------------------------------*/
size_t PolylineOps::GetStrokeCount
(
bvector <DPoint3d> const &points,
IFacetOptionsCR options,
double startFraction,
double endFraction
)
    {
    size_t count = 0, n = points.size ();
    if (n < 2)
        return n;

    if (DoubleOps::IsExact01 (startFraction, endFraction))
        {
        for (size_t i = 1; i < points.size (); i++)
            {
            DSegment3d localSegment = DSegment3d::From (points[i-1], points[i]);
            size_t edgeCount = options.SegmentStrokeCount (localSegment);
            if (i==1)
                count += edgeCount;
            else
                count += edgeCount;
            }
        return count;
        }
    else
        {
        bvector<DPoint3d> newPoints;
        PolylineOps::CopyBetweenFractions (points, newPoints, startFraction, endFraction);

        for (size_t i = 1; i < newPoints.size (); i++)
            {
            DSegment3d localSegment = DSegment3d::From (newPoints[i-1], newPoints[i]);
            size_t edgeCount = options.SegmentStrokeCount (localSegment);
            if (i==1)
                count += edgeCount;
            else
                count += edgeCount;
            }
        return count;
        }
    }

/*--------------------------------------------------------------------------------**//**
* Test if pointA is on line of origin..pointB and can be ignored.
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/
static bool IsColinearPointToEliminate (DPoint3dCR origin, DPoint3dCR pointA, DPoint3dCR pointB, double tol2, bool eliminateOverdraw, bool xyOnly)
    {
    DVec3d vectorA, vectorB;
    vectorA.DifferenceOf (pointA, origin);
    if (xyOnly)
        vectorA.z = 0.0;
    if (vectorA.MagnitudeSquared () < tol2)
        return true;
    vectorB.DifferenceOf (pointB, origin);
    if (xyOnly)
        vectorB.z = 0.0;
    double dot = vectorA.DotProduct (vectorB);
    double fraction;
    DVec3d perpendicularPart, parallelPart;
    if (dot >= 1.0) // pointB is "beyond" pointA ...
        {
        vectorB.GetPerpendicularParts (vectorA, fraction, parallelPart, perpendicularPart);
        return perpendicularPart.MagnitudeSquared () <= tol2;
        }
    if (!eliminateOverdraw)
        return false;
    vectorB.GetPerpendicularParts (vectorA, fraction, parallelPart, perpendicularPart);
    return perpendicularPart.MagnitudeSquared () <= tol2;            
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/    
void PolylineOps::CompressColinearPoints
(
bvector <DPoint3d> &points,
double absTol,
bool eliminateOverdraw,
bool wrap,
bool xyOnly
)
    {
        
    size_t n = points.size ();
    if (n < 2)
        return;

    double tol = absTol;
    if (absTol < 0.0)
        tol = DPoint3dOps::Tolerance (points, 0.0, Angle::SmallAngle ());
    double tol2 = tol * tol;
    DPoint3d lastAcceptedPoint = points[0];
    size_t numAccepted = 1;
    // !! lastAcceptedPoint is always the last accepted coordinate (lastAcceptedPoint == points[numAccepted - 1])
    // look ahead to points j and k==j+1
    //  (j can get multiple steps ahead.  k is one beyond j)
    for (size_t j = 1; j < n;)
        {
        size_t k = j + 1;
        if (k < n)
            {
            if (IsColinearPointToEliminate (lastAcceptedPoint, points[j], points[k], tol2, eliminateOverdraw, xyOnly))
                {
                // and leave lastAcceptedPoint alone ...
                }
            else
                {
                lastAcceptedPoint = points[numAccepted++] = points[j];
                }
            }
        j = k;
        }
 
    // The lastAcceptedPoint is NOT the last point.
    // The last point has NOT been accepted ...
    
    if (   wrap)
        {
        double d0 = points[n - 1].DistanceSquared(points[0]);
        if (d0 <= tol2 && numAccepted > 1
            && IsColinearPointToEliminate (lastAcceptedPoint, points[0], points[1], tol2, eliminateOverdraw, xyOnly)
            )
            {
            // move point[0] back to lastAcceptedPoint ...
            points[0] = lastAcceptedPoint;
            }
        else
            {
            points[numAccepted++] = points[n-1];
            }
       }
    else
        {
        if (points[n-1].DistanceSquared (lastAcceptedPoint) > tol2)
            {
            points[numAccepted++] = points[n-1];
            }
        }
    points.resize (numAccepted);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2012
+--------------------------------------------------------------------------------------*/    
void PolylineOps::AppendToChains (bvector<bvector<DPoint3d>> &chains, DSegment3dCR segment)
    {
    if (chains.empty ()     // really first call.
        || chains.back ().empty ()  // first call with empty back chain?  Doesn't happen in normal use but have to test it ...
        || !segment.point[0].AlmostEqual (chains.back ().back ())  // new segment is discontinuous from those before.
        )
        {
        chains.push_back (bvector<DPoint3d>());
        chains.back().push_back(segment.point [0]);
        }
    chains.back ().push_back (segment.point[1]);
    }

bool PolylineOps__IsStrictIncreasingX (bvector<DPoint3d>const &point)
    {
    for (size_t i = 0; i + 1 < point.size (); i++)
        if (point[i].x >= point[i+1].x)
            return false;
    return true;
    }
static CurveLocationDetail ComponentDetail (
DPoint3dCR point, size_t componentIndex, double componentFraction
)
    {
    CurveLocationDetail detail;
    detail.point = point;
    detail.componentFraction = componentFraction;
    detail.componentIndex = componentIndex;
    return detail;
    }

static void CompleteComponentDetails (bvector<CurveLocationDetailPair> &data, size_t numPointA, size_t numPointB)
    {
    double fA = numPointA < 2 ? 1.0 : DoubleOps::ValidatedDivide (1.0, ((double)numPointA) - 1.0).Value();
    size_t numComponentA = numPointA > 1 ? numPointA - 1 : 1;
    double fB = numPointB < 2 ? 1.0 : DoubleOps::ValidatedDivide (1.0, ((double)numPointB) - 1.0).Value();
    size_t numComponentB = numPointB > 1 ? numPointB - 1 : 1;
    for (CurveLocationDetailPair &pair : data)
        {
        pair.detailA.fraction = fA * (pair.detailA.componentIndex + pair.detailA.componentFraction);
        pair.detailA.numComponent = numComponentA;
        pair.detailB.fraction = fB * (pair.detailB.componentIndex + pair.detailB.componentFraction);
        pair.detailB.numComponent = numComponentB;
        }
    }    
void AppendPair (bvector<CurveLocationDetailPair> &pairs, CurveLocationDetail const &detailA, CurveLocationDetail const &detailB, bool reverse, bool skipIfAlmostEqual)
    {
    if (reverse)
        pairs.push_back (CurveLocationDetailPair (detailB, detailA));
    else
        pairs.push_back (CurveLocationDetailPair (detailA, detailB));
    if (skipIfAlmostEqual && pairs.size () > 1)
        {
        size_t n = pairs.size ();
        CurveLocationDetailPair &pair0 = pairs.at(n-2);
        CurveLocationDetailPair &pair1 = pairs.at(n-1);
        if (pair0.detailA.point.AlmostEqual (pair1.detailA.point)
            && pair0.detailB.point.AlmostEqual (pair1.detailB.point))
            {
            pairs.pop_back ();
            }
        }
    }

// ASSUME xyzA0.x <= xyzB0.x
// AT LEAST ONE OF iA, iB is always incremented !!!
static void RecordApproachPerpZOrderedX
(
DPoint3dCR xyzA0, DPoint3dCR xyzA1, size_t &iA,
DPoint3dCR xyzB0, DPoint3dCR xyzB1, size_t &iB,
bvector<CurveLocationDetailPair> &pairs,
bool reverse,
bool skipIfAlmostEqual
)
    {
    double xA0 = xyzA0.x;
    double xB0 = xyzB0.x;
    double xA1 = xyzA1.x;
    double xB1 = xyzB1.x;
    if (xA0 >= xA1)
        {
        iA++;
        return; // tolerable error?
        }
    if (xB0 >= xB1)
        {
        iB++;
        return;     // tolerable error?
        }
    if (xB0 < xA0)
        {
        iB++;
        return; // CALLER ERROR.
        }
    if (xB0 > xA1)  // no overlap advance iA without any pair entry ...
        {
        iA++;
        return;
        }
    // xA0..xB0..xA1
    auto fractionB0 = DoubleOps::InverseInterpolate (xA0, xB0, xA1);
    DPoint3d xyzAB0 = DPoint3d::FromInterpolate (xyzA0, fractionB0, xyzA1);
    if (xB1 <= xA1)
        {
        auto fractionB1 = DoubleOps::InverseInterpolate (xA0, xB1, xA1);
        DPoint3d xyzAB1 = DPoint3d::FromInterpolate (xyzA0, fractionB1, xyzA1);
        AppendPair (pairs,
                ComponentDetail (xyzAB0, iA, fractionB0),
                ComponentDetail (xyzB0, iB, 0.0),
                reverse, skipIfAlmostEqual);
        AppendPair (pairs,
                ComponentDetail (xyzAB1, iA, fractionB1),
                ComponentDetail (xyzB1, iB, 1.0), reverse, skipIfAlmostEqual
                );
        iB++;
        return;
        }
    else    // xA0 .. xB0_______xA1.. xB1
        {
        auto fractionA1 = DoubleOps::InverseInterpolate (xB0, xA1, xB1);
        DPoint3d xyzBA1 = DPoint3d::FromInterpolate (xyzB0, fractionA1, xyzB1);
        AppendPair (pairs,
                ComponentDetail (xyzAB0, iA, fractionB0),
                ComponentDetail (xyzB0, iB, 0.0),
                reverse, skipIfAlmostEqual);
        AppendPair (pairs,
                ComponentDetail (xyzA1, iA, 1.0),
                ComponentDetail (xyzBA1, iB, fractionA1),
                reverse, skipIfAlmostEqual);
        iA++;
        return;
        }
    }

void PolylineOps::CollectZPairsForOrderedXPoints (
bvector<DPoint3d>const &pointA,
bvector<DPoint3d> const &pointB,
bvector<CurveLocationDetailPair> &pairs
)
    {
    pairs.clear ();

    for (size_t iA = 0, iB = 0;
            iA + 1 < pointA.size () && iB + 1 < pointB.size ();
            )
        {
        if (pointA[iA].x <= pointB[iB].x)
            RecordApproachPerpZOrderedX (pointA[iA], pointA[iA+1], iA, pointB[iB], pointB[iB+1], iB, pairs, false, true);
        else
            RecordApproachPerpZOrderedX (pointB[iB], pointB[iB+1], iB, pointA[iA], pointA[iA+1], iA, pairs, true, true);
        }
    CompleteComponentDetails (pairs, pointA.size (), pointB.size ());
    }



void PolylineOps::AddPointIfDistinctFromBack (bvector<DPoint3d> &points, DPoint3d xyz)
    {
    if (points.size () == 0 || !points.back ().AlmostEqual (xyz))
        points.push_back (xyz);
    }

void PolylineOps::EnforceClosure(bvector<DPoint3d> &points)
    {
    if (points.size () > 1)
        {
        DPoint3d xyz = points.front ();
        if (xyz.AlmostEqual (points.back ()))
            points.back () = xyz;       // Enforce bit equality
        else
            points.push_back (xyz);
        }
    }

void PolylineOps::PackAlmostEqualAfter (bvector<DPoint3d> &points, size_t initialCount)
    {
    if (points.size () < 2)
        return;
    if (initialCount >= points.size ())
        return;
    size_t tailIndex = initialCount == 0 ? 0 : initialCount - 1;
    for (size_t i = tailIndex + 1; i < points.size (); i++)
        {
        if (i == 0 || !points[i].AlmostEqual (points[tailIndex]))
            {
            tailIndex++;
            points[tailIndex] = points[i];
            }
        }
    points.resize (tailIndex + 1);
    }

struct TestTriangle : DTriangle3d
{
DTriangle3d m_triangle;
double m_aspectRatio;
int m_id;
bool m_isValid;
int m_indices[3];   // ONE BASED indices, sign controlled by signA
TestTriangle (
bvector<DPoint3d> const &data0, size_t index0A, size_t index0B,
bvector<DPoint3d> const &data1, size_t index1,
int id
)
    {
    m_id = id;
    int signA = id < 0 ? -1 : 1;
    if (index0A < data0.size () && index0B < data0.size ()
        && index1 < data1.size ())
        {
        m_isValid = true;
        m_triangle = DTriangle3d (data0[index0A], data0[index0B], data1[index1]);
        m_indices[0] =   signA * (int)(index0A + 1);
        m_indices[1] =   signA * (int)(index0B + 1);
        m_indices[2] = - signA * (int)(index1 + 1);
        m_aspectRatio = m_triangle.AspectRatio ();
        }
    else
        {
        m_isValid = false;
        m_aspectRatio = -1.0;
        }
    }

// @param [in] first a valid triangle
// @param [in] other a triangle that maybe invalid
// @return first triangle with possibly reduced aspect ratio
// 
static TestTriangle MergeAspectRatio (TestTriangle const &first, TestTriangle const &other)
    {
    TestTriangle result = first;
    if (other.m_isValid)
        {
        double d = first.CrossVectorsFromOrigin ().DotProduct (other.CrossVectorsFromOrigin ());
        // find smaller of plain aspect ratios . . .
        if(other.m_aspectRatio < result.m_aspectRatio)
            result.m_aspectRatio = other.m_aspectRatio;
        // remoreselessly reduce if normals flip . ..
        if (d < 0.0)
            result.m_aspectRatio -= 1.0;
        }
    return result;
    }

static void UpdateIfOtherHasLargerAspectRatio (TestTriangle &triangle0, TestTriangle const &triangle1)
    {
    if (triangle1.m_isValid && triangle1.m_aspectRatio > triangle0.m_aspectRatio)
        triangle0 = triangle1;
    }
};

UsageSums PolylineOps::SumSegmentLengths (bvector<DPoint3d> const &xyz)
    {
    UsageSums sums;

    for (size_t i = 0, n = xyz.size (); i + 1 < n; i++)
        {
        sums.Accumulate (xyz[i].Distance (xyz[i+1]));
        }
    return sums;
    }


static void AddGreedyTriangulationBetweenLinestrings
(
bvector<DPoint3d> const & linestringA,  size_t &baseA, size_t limitA,
bvector<DPoint3d> const &linestringB,   size_t &baseB, size_t limitB,
bvector<DTriangle3d> &triangles,
bvector<int> *oneBasedIndexAB,
size_t maxAdd = SIZE_MAX
)
    {
    if (limitA > linestringA.size ()) 
        limitA = linestringA.size ();
    if (limitB > linestringB.size ()) 
        limitB = linestringB.size ();
    size_t count0 = triangles.size ();
    while (baseA + 1 < limitA && baseB + 1 < limitB && (triangles.size () - count0) < maxAdd)
        {
        if (linestringA[baseA].AlmostEqual (linestringA[baseA+1]))
            {
            baseA++;
            continue;
            }
        if (linestringB[baseB].AlmostEqual (linestringB[baseB+1]))
            {
            baseB++;
            continue;
            }

        // triangles A1 and B1 are always valid.
        TestTriangle triangleA1 (linestringA, baseA, baseA+1, linestringB, baseB, 1);
        TestTriangle triangleB1 (linestringB, baseB+1, baseB, linestringA, baseA, -1);
        TestTriangle triangleA2 (linestringA, baseA+1, baseA+2, linestringB, baseB, 2);
        TestTriangle triangleB2 (linestringB, baseB+2, baseB+1, linestringA, baseA, -2);
        TestTriangle triangleA3 (linestringA, baseA, baseA+1, linestringB, baseB + 1, 3);
        TestTriangle triangleB3 (linestringB, baseB+1, baseB, linestringA, baseA + 1, -3);
        // Look at pairs of 2 triangles.
        // (each pair begins with 1 or -1)
        // For each pair find the smallest aspect ratio of its two triangles.  (Small is bad)
        // Choose the pair where that (smaller aspect ratio of two) is largest.
        // Advance in that direction.
        TestTriangle bestTriangle = TestTriangle::MergeAspectRatio (triangleA1, triangleB3);
        TestTriangle::UpdateIfOtherHasLargerAspectRatio (bestTriangle, TestTriangle::MergeAspectRatio (triangleB1, triangleA3));
        //TestTriangle::UpdateIfOtherHasLargerAspectRatio (bestTriangle, TestTriangle::MergeAspectRatio (triangleB1, triangleB2));
        //TestTriangle::UpdateIfOtherHasLargerAspectRatio (bestTriangle, TestTriangle::MergeAspectRatio (triangleA1, triangleA2));

        if (bestTriangle.m_id > 0)
            {
            baseA++;
            triangles.push_back (bestTriangle.m_triangle);
            }
        else
            {
            baseB++;
            triangles.push_back (bestTriangle.m_triangle);
            }

        if (oneBasedIndexAB != nullptr)
            {
            oneBasedIndexAB->push_back (bestTriangle.m_indices[0]);
            oneBasedIndexAB->push_back (bestTriangle.m_indices[1]);
            oneBasedIndexAB->push_back (bestTriangle.m_indices[2]);
            }

        }
    // sweep in trailing points from either side.  At lesat one of baseA, baseB is at its limit, so only one of these will execute any bodies.
    if (baseA + 1 == limitA)
        {
        while (baseB + 1 < limitB && (triangles.size () - count0) < maxAdd)
            {
            triangles.push_back (DTriangle3d (linestringB[baseB+1], linestringB[baseB], linestringA[baseA]));
            if (oneBasedIndexAB != nullptr)
                {
                oneBasedIndexAB->push_back (- (int)(baseB + 2));
                oneBasedIndexAB->push_back (- (int)(baseB + 1));
                oneBasedIndexAB->push_back (  (int)(baseA + 1));
                }
            baseB++;
            }
        }

    // sweep in trailing points from either side.  At lesat one of baseA, baseB is at its limit, so only one of these will execute any bodies.
    if (baseB + 1 == limitB)
        {
        while (baseA + 1 < limitA && (triangles.size () - count0) < maxAdd)
            {
            triangles.push_back (DTriangle3d (linestringA[baseA], linestringA[baseA+1], linestringB[baseB]));
            if (oneBasedIndexAB != nullptr)
                {
                oneBasedIndexAB->push_back (  (int)(baseA + 1));
                oneBasedIndexAB->push_back (  (int)(baseA + 2));
                oneBasedIndexAB->push_back ( -(int)(baseB + 1));
                }
            baseA++;
            }
        }
    }

bool IsForwardVector (DVec3dCR candidate, DVec3dCR forward, DVec3dCR perp, double tiltRadians)
    {
    if (candidate.DotProduct (forward) <= 0.0)
        return false;
    double theta = candidate.AngleFromPerpendicular (perp);
    if (fabs (theta) > tiltRadians)
        return false;
    return true;
    }
size_t AdvanceToPlanarLimit
(
bvector<DPoint3d> const &xyz,
size_t i0,          // examine indices starting here
DPoint3dCR xyzA,
DVec3dCR perpA,
DVec3dCR forwardA,
DPoint3dCR xyzB,
DVec3dCR perpB,
DVec3dCR forwardB,
double radians
)
    {
    size_t n = xyz.size ();
    size_t i1 = i0;
    while (i1 < n)
        {
        DVec3d vectorA = xyz[i1] - xyzA;
        //DVec3d vectorB = xyz[i1] - xyzB;
        if (!IsForwardVector (vectorA, forwardA, perpA, radians))
            break;
       if (!IsForwardVector (vectorA, forwardB, perpB, radians))
           break;
        if (i1 > 0 && !IsForwardVector (xyz[i1] - xyz[i1-1], forwardA, perpA, radians))
            break;
        i1++;
        }
    return i1;
    }


void PolylineOps::GreedyTriangulationBetweenLinestrings
(
bvector<DPoint3d> const & linestringA,
bvector<DPoint3d> const &linestringB,
bvector<DTriangle3d> &triangles,
bvector<int> *oneBasedIndexAB
)
    {
    triangles.clear ();
    if (oneBasedIndexAB != nullptr)
        oneBasedIndexAB->clear ();
    size_t baseA = 0;
    size_t baseB = 0;
    AddGreedyTriangulationBetweenLinestrings (
            linestringA, baseA, linestringA.size (),
            linestringB, baseB, linestringB.size (),
            triangles,
            oneBasedIndexAB);
    }

bool IsPlanarBase
(
bvector<DPoint3d> const & linestringA,
size_t baseA,
bvector<DPoint3d> const &linestringB,
size_t baseB,
double radians,
DPoint3dR xyzA,
DVec3dR crossA,
DVec3dR forwardA,
DPoint3dR xyzB,
DVec3dR crossB,
DVec3dR forwardB
)
    {
    if (baseA + 1 < linestringA.size ()
        && baseB + 1 < linestringB.size ())
        {
        xyzA = linestringA[baseA];
        xyzB = linestringB[baseB];
        forwardA = linestringA[baseA+1] - xyzA;
        forwardB = linestringB[baseB+1] - xyzB;
        DVec3d   chord = xyzB - xyzA;
        crossA = DVec3d::FromCrossProduct (chord, forwardA);
        crossB = DVec3d::FromCrossProduct (chord, forwardB);
        if (!xyzA.AlmostEqual (xyzB) && crossA.AngleTo (crossB) < radians)
            return true;
        }
    return false;
    }


void PolylineOps::GreedyTriangulationBetweenLinestrings
(
bvector<DPoint3d> const & linestringA,
bvector<DPoint3d> const &linestringB,
bvector<DTriangle3d> &triangles,
bvector<int> *oneBasedIndexAB,
Angle planarContinuationAngle
)
    {
    triangles.clear ();
    if (oneBasedIndexAB != nullptr)
        oneBasedIndexAB->clear ();
    size_t baseA = 0;
    size_t baseB = 0;
    size_t nA = linestringA.size ();
    size_t nB = linestringB.size ();
    double radians = DoubleOps::MaxAbs (planarContinuationAngle.Radians (), Angle::MediumAngle ());
    DPoint3d xyzA, xyzB;
    DVec3d   crossA, crossB, forwardA, forwardB;
#ifdef PrintGreedyTriangles
    GEOMAPI_PRINTF ("\nGreedyTriangulation %d %d\n", (int)linestringA.size (), (int)linestringB.size ());
#endif
    while (baseA + 1 < nA && baseB + 1 < nB)
        {
        if (IsPlanarBase (linestringA, baseA, linestringB, baseB, radians, xyzA, crossA, forwardA, xyzB, crossB, forwardB))
            {
            size_t limitA = AdvanceToPlanarLimit (linestringA, baseA + 1, xyzA, crossA, forwardA, xyzB, crossB, forwardB, radians);
            size_t limitB = AdvanceToPlanarLimit (linestringB, baseB + 1, xyzB, crossB, forwardB, xyzA, crossA, forwardA, radians);
#ifdef PrintGreedyTriangles
            GEOMAPI_PRINTF ("(n+n) (A %d %d)      (B %d %d)\n",
                                (int)baseA, (int)limitA,
                                (int)baseB, (int)limitB);
#endif
            AddGreedyTriangulationBetweenLinestrings (
                    linestringA, baseA, limitA,
                    linestringB, baseB, limitB,
                    triangles,
                    oneBasedIndexAB);
            }
        else if (IsPlanarBase (linestringA, baseA + 1, linestringB, baseB, radians, xyzA, crossA, forwardA, xyzB, crossB, forwardB))
            {
#ifdef PrintGreedyTriangles
            GEOMAPI_PRINTF ("    (A %d (+2))      (B %d (+1))\n",  (int)baseA,  (int)baseB);
#endif
            AddGreedyTriangulationBetweenLinestrings (
                    linestringA, baseA, baseA + 2,
                    linestringB, baseB, baseB + 1,
                    triangles,
                    oneBasedIndexAB);
            }
        else if (IsPlanarBase (linestringA, baseA, linestringB, baseB + 1, radians, xyzA, crossA, forwardA, xyzB, crossB, forwardB))
            {
#ifdef PrintGreedyTriangles
            GEOMAPI_PRINTF ("    (A %d (+1))      (B %d (+2))\n",  (int)baseA,  (int)baseB);
#endif
            AddGreedyTriangulationBetweenLinestrings (
                    linestringA, baseA, baseA + 1,
                    linestringB, baseB, baseB + 2,
                    triangles,
                    oneBasedIndexAB);
            }
        else 
            {
#ifdef PrintGreedyTriangles
                GEOMAPI_PRINTF ("SINGLE    (A %d)      (B %d)\n",  (int)baseA,  (int)baseB);
#endif
                AddGreedyTriangulationBetweenLinestrings (
                        linestringA, baseA, baseA + 2,
                        linestringB, baseB, baseB + 2,
                        triangles,
                        oneBasedIndexAB, 1);
#ifdef PrintGreedyTriangles
                GEOMAPI_PRINTF ("after    (A %d)      (B %d)\n",  (int)baseA,  (int)baseB);
#endif
            }
        }
#ifdef PrintGreedyTriangles
    GEOMAPI_PRINTF ("final (A %d %d)      (B %d %d)\n",
                        (int)baseA, (int)linestringA.size (),
                        (int)baseB, (int)linestringB.size ());
#endif
    // catch final ..
    AddGreedyTriangulationBetweenLinestrings (
                    linestringA, baseA, linestringA.size (),
                    linestringB, baseB, linestringB.size (),
                    triangles,
                    oneBasedIndexAB);
    }


void PolylineOps::CopyCyclicPointsFromStartIndex (bvector<DPoint3d> const &source, size_t startIndex, bvector<DPoint3d> &dest)
    {
    dest.clear ();
    size_t n = source.size ();
    for (size_t i = 0; i < n; i++)
        {
        size_t j = (startIndex + i) % n;
        if (i == 0 || !source[j].AlmostEqual (dest.back ()))
            dest.push_back (source[j]);
        }
    PolylineOps::EnforceClosure (dest);
    }
END_BENTLEY_GEOMETRY_NAMESPACE

