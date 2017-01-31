/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/PolylineOffset.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_maxMiterRadians = 2.0;
static double s_minMiterRadians = 0.15;


struct SimpleOffsetContext
{
DVec3d m_planeNormal;
double m_offsetDistance;
double m_maxMiterRadians;

SimpleOffsetContext (DVec3dCR planeNormal, double offsetDistance, double maxMiterRadians)
    :
    m_planeNormal (planeNormal),
    m_offsetDistance (offsetDistance)
    {
    m_maxMiterRadians =  DoubleOps::Max (DoubleOps::Min (maxMiterRadians, s_maxMiterRadians), s_minMiterRadians);
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitVectorToRightOfEdge
(
DVec3dCR    edgeVector,
DVec3dR     perpVector
)
    {
    DVec3d vector = DVec3d::FromCrossProduct (edgeVector, m_planeNormal);
    double magnitude;
    return perpVector.TryNormalize (vector, magnitude);
    }

/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitVectorToOffsetSide
(
DVec3dCR    edgeVector,
DVec3dR     perpVector
)
    {
    DVec3d vector = DVec3d::FromCrossProduct (edgeVector, m_planeNormal);
    double magnitude;
    if (m_offsetDistance < 0.0)
        vector.Negate ();
    return perpVector.TryNormalize (vector, magnitude);
    }


/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
bool UnitVectorToOffsetSide
(
DPoint3dCR point0,
DPoint3dCR point1,
DVec3dR     perpVector
)
    {
    DVec3d edgeVector = DVec3d::FromStartEnd (point0, point1);
    DVec3d vector = DVec3d::FromCrossProduct (edgeVector, m_planeNormal);
    if (m_offsetDistance < 0.0)
        vector.Negate ();
    double magnitude;
    return perpVector.TryNormalize (vector, magnitude);
    }


bool ConstructUnitBisectorOnOffsetSide
(
DVec3dR bisector,
double &sweepAngle,
DPoint3dCR point0,
DPoint3dCR point1,
DPoint3dCR point2
)
    {
    DVec3d perp01, perp12;
    if ( UnitVectorToOffsetSide (point0, point1, perp01)
       && UnitVectorToOffsetSide (point1, point2, perp12)
       )
        {
        double a;
        sweepAngle = perp01.PlanarAngleTo (perp01, m_planeNormal);
        bisector = DVec3d::FromSumOf (perp01, perp12);
        if (bisector.TryNormalize (bisector, a))
            {
            return true;
            }
        }
    return false;
    }

static void RemovePoints (bvector<DPoint3d> &points, size_t index0, size_t numRemove)
    {
    size_t n = points.size ();
    if (index0 < n)
        {
        size_t index1 = index0 + numRemove;
        if (index1 > n)
            index1 = n;
        points.erase (points.begin () + index0, points.begin () + index1);
        }
    }

bool RemoveAnyInteriorVertexBy3PointNeighborhood (bvector<DPoint3d> &points)
    {
    // Look for a the smallest triangle elimianated by a single projection.
    size_t indexQ = SIZE_MAX;
    DPoint3d pointQ, pointR;
    pointR.Zero ();
    pointQ.Zero ();
    double rr;
    size_t numPoints = points.size ();
    double squaredOffset = m_offsetDistance * m_offsetDistance;
    double rrMin = DBL_MAX;
    double fraction;
    double sweptAngle;
    static double s_maximumSweptAngle = 1.0;    
    for (size_t i = 1; i + 1 < numPoints; i++)
        {
        DVec3d unitBisector;
        DPoint3d pointA = points[i - 1];
        DPoint3d pointB = points[i];
        DPoint3d pointC = points[i+1];
        // umm.. This is a little wrong -- fires one point across perpendicular to bisector
        //   (that's ok) and compares that to the offset -- but there's a cosine factor???
        if (ConstructUnitBisectorOnOffsetSide (unitBisector, sweptAngle, pointA, pointB, pointC)
         && sweptAngle < s_maximumSweptAngle)
            {
            double dA = pointA.DotDifference (pointB, unitBisector);
            double dC = pointC.DotDifference (pointB, unitBisector);
            if (dA > 0 && dC > 0)
                {
                if (dA < dC)
                    {
                    // move B along edge BC
                    if (DoubleOps::SafeDivide (fraction, dA, dC, 0.0))
                        {
                        pointR = DPoint3d::FromInterpolate (pointB, fraction, pointC);
                        rr = pointR.DistanceSquared (pointA);
                        if (rr > squaredOffset && rr < rrMin)
                            {
                            rrMin = rr;
                            pointQ = pointR;
                            indexQ = i;
                            }
                        }
                    }
                else
                    {
                    // move B along edge BA
                    if (DoubleOps::SafeDivide (fraction, dC, dA, 0.0))
                        {
                        pointR = DPoint3d::FromInterpolate (pointB, fraction, pointA);
                        rr = pointR.DistanceSquared (pointC);
                        if (rr > squaredOffset && rr < rrMin && i > 1)
                            {
                            rrMin = rr;
                            pointQ = pointR;
                            indexQ = i;
                            }
                        }
                    }
                }
            }
        }

    if (indexQ != SIZE_MAX)
        {
        points[indexQ] = pointQ;
        return true;
        }
    return false;
    }

bool RemoveAnyInteriorVertex (bvector<DPoint3d> &points)
    {
    size_t indexToRemove = SIZE_MAX;
    double minDistance = DBL_MAX;
    DVec3d bisectorA, bisectorB, perpAB;
    size_t numPoints = points.size ();
    DPoint3d pointA, pointB;
    double fractionA, fractionB;
    double sweptAngle;
    // For each (interior) edge, find the smallest offset distance that would swallow the edge.
    for (size_t i=1; i + 2 < numPoints; i++)
        {
        if (   ConstructUnitBisectorOnOffsetSide (bisectorA, sweptAngle, points[i-1], points[i], points[i+1])
            && ConstructUnitBisectorOnOffsetSide (bisectorB, sweptAngle, points[i], points[i+1], points[i+2])
            && DRay3d::ClosestApproachUnboundedRayUnboundedRay
                  (
                  fractionA, fractionB,
                  pointA, pointB,
                  DRay3d::FromOriginAndVector (points[i], bisectorA),
                  DRay3d::FromOriginAndVector (points[i+1], bisectorB)
                  )
            && fractionA > 0.0
            && fractionB > 0.0
            && UnitVectorToOffsetSide(points[i], points[i+1], perpAB)
            )
            {
            double distanceToIntersection = pointA.DotDifference (points[i], perpAB);
            if (//distanceToIntersection > fabs (m_offsetDistance) && 
                distanceToIntersection < minDistance)
                {
                minDistance = distanceToIntersection;
                indexToRemove = i;
                }
            }
        }

    if (fabs (minDistance) < fabs (m_offsetDistance))
        {
        // Extend prior and following edges to intersection.
        DRay3d rayA = DRay3d::FromOriginAndTarget (points[indexToRemove], points[indexToRemove - 1]);
        DRay3d rayB = DRay3d::FromOriginAndTarget (points[indexToRemove + 1], points[indexToRemove + 2]);
        if (DRay3d::ClosestApproachUnboundedRayUnboundedRay
                  (
                  fractionA, fractionB,
                  pointA, pointB,
                  rayA, rayB
                  ))
            {

            if (UnitVectorToOffsetSide(points[indexToRemove], points[indexToRemove + 1], perpAB))
                {
                double distanceToIntersection = pointA.DotDifference (points[indexToRemove], perpAB);
                if (distanceToIntersection < 0.0)
                    {
                    points[indexToRemove] = pointA;
                    RemovePoints (points, indexToRemove, 1);
                    return true;
                    }
                else
                    {
                    static int s_cutbackSelect = 0;
                    if (s_cutbackSelect == 1)
                        {
                        // Just close the inset??
                        RemovePoints (points, indexToRemove, 2);
                        }
                    else
                        {
                        // The two side segments intersect in front -- e.g. converging.
                        // replace the back segment by a parallel segment at the nearer point.
                        pointA = points[indexToRemove - 1];
                        pointB = points[indexToRemove + 2];
                        double distanceA = pointA.DotDifference (points[indexToRemove], perpAB);
                        double distanceB = pointB.DotDifference (points[indexToRemove], perpAB);
                        if (DoubleOps::AlmostEqual (distanceA, distanceB))
                            {
                            // remove both points of this segment . .
                            RemovePoints (points, indexToRemove, 2);
                            return true;
                            }
                        else if (distanceA < distanceB)
                            {
                            RemovePoints (points, indexToRemove, 1);
                            points[indexToRemove] = pointA;
                            return true;
                            }
                        else // distanceB < distanceA
                            {
                            RemovePoints (points, indexToRemove, 1);
                            points[indexToRemove] = pointB;
                            return true;
                            }
                        }
                    }
                }
#ifdef abc
                if (points[mv - 1].distance (&points[mv]) <= tolerance
                    || points[mv].distance (&points[mv+1]) <= tolerance)
                    removePoint(points, mv, numVerts, 2);
#endif
            }
        else
            {
            // The rays are parallel. This is a "deep gouge" with parallel sides, [indexToRemove,indexToRemove+1] at the bottom.
            // remove one or both.
            static double s_largeFraction = 0.99;
            rayA.ProjectPointUnbounded (pointA, fractionA, points[indexToRemove+2]);
            rayB.ProjectPointUnbounded (pointB, fractionB, points[indexToRemove-1]);
            if (fractionA > s_largeFraction && fractionB > s_largeFraction)
                {
                RemovePoints (points, indexToRemove, 2);
                return true;
                }
            else if (fractionA > 0.0 && fractionA < fractionB && fractionA < 1.0)
                {
                RemovePoints (points, indexToRemove, 1);
                points[indexToRemove] = pointA;
                return true;
                }
            else if (fractionB > 0.0 && fractionB < fractionA && fractionB < 1.0)
                {
                RemovePoints (points, indexToRemove, 1);
                points[indexToRemove] = pointB;
                return true;
                }
            }
        }
    return false;
    }


// Input vectors already have sign of offset in effect -- only apply absolute offset.
bool ConstructBisectionVectorFromAverageDirectedUnits
(
DVec3dCR perp0,
DVec3dCR perp1,
bool scaleForAbsoluteOffsetDistance,
DVec3dR bisector,
double  &sweptAngle
)
    {
    DVec3d angleBisector = DVec3d::FromSumOf (perp0, perp1);
    DVec3d unitBisector;
    double a;
    sweptAngle = 0.0;
    if (unitBisector.TryNormalize (angleBisector, a))
        {
        sweptAngle = perp0.PlanarAngleTo (perp1, m_planeNormal);
        if (scaleForAbsoluteOffsetDistance)
            {
            double d = unitBisector.DotProduct (perp0);
            double bisectorDistance;
            bool stat = DoubleOps::SafeDivide (bisectorDistance, fabs (m_offsetDistance), d, fabs (m_offsetDistance));
            if (stat)
                {
                bisector.Scale (unitBisector, bisectorDistance);
                return true;
                }
            }
        else
            {
            bisector = unitBisector;
            return true;
            }
        }
    return false;
    }
#define MAX_MITER_COUNT 24
// return number of steps and reference vector for angle sweeping...
// assume perp0, perp1 are unit vectors 
// dTheta is the "half" angle step for first and last.  Internal steps are 2*dTheta
int AnalyzeMiterSteps (DVec3dCR perpX, DVec3dCR perp1, DVec3dR perpY, double &dTheta)
    {
    perpY.CrossProduct (m_planeNormal, perpX);  // Z cross X is Y
    if (m_offsetDistance < 0.0)
        perpY.Negate ();
    double co = perpX.DotProduct (perp1);
    double si = perpY.DotProduct (perp1);
    double theta = atan2 (si, co);    // This is signed !!!
    if (fabs (si) < 1.0e-14 * fabs (co) && co < 0.0)
        {
        // force near-reversal to PI sweep.
        theta = fabs (theta);
        }
    if (theta <= m_maxMiterRadians)
        return 1;
    int n = (int)ceil (theta / m_maxMiterRadians);
    if (n > MAX_MITER_COUNT)
        n = MAX_MITER_COUNT;
    dTheta = 0.5 * theta / (double)n;
    return n;
    }
/*---------------------------------------------------------------------------------**//**
+---------------+---------------+---------------+---------------+---------------+------*/
bool AppendOffsetPoints
(
DPoint3d   xyzBase,
DVec3dCR   edge0,
DVec3dCR   edge1,
bvector<DPoint3d> &points
)
    {
    DVec3d perp0, perp1;

    bool stat0 = UnitVectorToOffsetSide (edge0, perp0);
    bool stat1 = UnitVectorToOffsetSide (edge1, perp1);
    double sweepAngle = 0.0;
    if (stat0 && stat1)
        {
        DVec3d perpY;
        double dTheta;
        int numberOfMiterSteps = AnalyzeMiterSteps (perp0, perp1, perpY, dTheta);
        if (numberOfMiterSteps == 1)
            {
            DVec3d bisector;
            if (ConstructBisectionVectorFromAverageDirectedUnits (perp0, perp1, true, bisector, sweepAngle))
                points.push_back (DPoint3d::FromSumOf (xyzBase, bisector));
            else
                points.push_back (xyzBase);
            return true;
            }
        else
            {
            double a = fabs (m_offsetDistance) / cos (dTheta);
            for (int i = 0; i < numberOfMiterSteps; i++)
                {
                double theta = (1 + 2 * i) * dTheta;
                double co = cos (theta);
                double si = sin (theta);
                points.push_back (DPoint3d::FromSumOf (xyzBase, perp0, co * a, perpY, si * a));
                }
            }
        }
    else if (stat0)
        {
        points.push_back (DPoint3d::FromSumOf (xyzBase, perp0, fabs (m_offsetDistance)));
        return true;
        }
    else if (stat1)
        {
        points.push_back (DPoint3d::FromSumOf (xyzBase, perp1, fabs (m_offsetDistance)));
        return true;
        }
    return false;
    }

};


bool PolylineOps::OffsetLineStringXY
(
bvector<DPoint3d> &out,        //!< offset point.
bvector<DPoint3d> const &in,  //!< input points
double offsetDistance,        //!< signed offset distance
bool periodic,                //!< true to make offset joint for closure
double maxMiterRadians        //!< largest angle of turn for an outside miter.  Suggested value is no more than
                              //!             Angle:DegreesToRadians (95.0)
)
    {
    return OffsetLineString (out, in, offsetDistance, DVec3d::From (0,0,1), periodic, maxMiterRadians);
    }

static bool s_justClean = false;
bool PolylineOps::OffsetLineString
(
bvector<DPoint3d> &out,        //!< offset point.
bvector<DPoint3d> const &in,  //!< input points
double offsetDistance,        //!< signed offset distance
DVec3dCR planeNormal,         //!< normal towards eye
bool periodic,                //!< true to make offset joint for closure
double maxMiterRadians        //!< largest angle of turn for an outside miter.  Suggested value is no more than
                              //!             Angle:DegreesToRadians (95.0).  Large miter angles will create sharp
                              //!             joints to miter points far away from the vertex.
)
    {
    out.clear ();
    DVec3d unitNormal;
    double magnitude;
    if (unitNormal.TryNormalize (planeNormal, magnitude) && in.size () >= 2)
        {
        DVec3d edge0, edge1;
        SimpleOffsetContext context (unitNormal, offsetDistance, maxMiterRadians);

        bvector<DPoint3d> pointA = in;
        // repeatedly remove or adjust points to eliminate obviously conflicting offsets . ..
        static size_t s_maxIt = SIZE_MAX;
        size_t maxIt = pointA.size ();
        if (maxIt > s_maxIt)
            maxIt = s_maxIt;
        static bool s_do3Point = false;
        static bool s_do4Point = true;
        size_t numIt = 0;
        while (
                numIt++ < maxIt &&
                (   (s_do3Point && context.RemoveAnyInteriorVertexBy3PointNeighborhood (pointA))
                ||  (s_do4Point && context.RemoveAnyInteriorVertex (pointA))
                )
              )
            {
            }

        if (s_justClean)
            {
            out = pointA;
            return true;
            }
        edge0 = DVec3d::FromStartEnd (pointA[0], pointA[1]);
        DVec3d edge0Save = edge0;
        context.AppendOffsetPoints (pointA[0], edge0, edge0, out);

        for (size_t i = 1; i + 1 < pointA.size (); i++, edge0 = edge1)
            {
            edge1 = DVec3d::FromStartEnd (pointA[i], pointA[i+1]);
            context.AppendOffsetPoints (pointA[i], edge0, edge1, out);
            }
        if (!periodic)
            context.AppendOffsetPoints (pointA.back (), edge1, edge1, out);
        else
            {
            context.AppendOffsetPoints (pointA.back (), edge1, edge0Save, out);
            out[0] = out.back ();
            }
        return true;
        }
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE

