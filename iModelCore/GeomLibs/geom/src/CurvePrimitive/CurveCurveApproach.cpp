/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <algorithm>
#include <numeric>
#include <set>

#include "CurveCurveProcessor.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#ifdef CompileCCADebugTracking
// These functions are marked GEOMDLLIMPEXP so a unit test can extract every single iteration.
// They are NOT in an h file, so the unit test has to redeclare.
static int s_collectDebugDetails = 0;
static bvector<bvector<CurveLocationDetailPair>> s_debugDetails;

GEOMDLLIMPEXP void SetCCADebug(int value)
    {
    s_collectDebugDetails = value;
    }
GEOMDLLIMPEXP void GrabCCADebugDetails(bvector<bvector<CurveLocationDetailPair>> &data)
    {
    data.swap(s_debugDetails);
    }
void StartNewDebug ()
    {
    s_debugDetails.push_back (bvector<CurveLocationDetailPair> ());
    }
#endif
// Newton iteration function for "closest approach between smooth curves".
struct CurveCurveApproachIterate : FunctionRRToRRD
{
ICurvePrimitiveP m_curveA;
ICurvePrimitiveP m_curveB;

CurveCurveApproachIterate (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB) : m_curveA(curveA), m_curveB (curveB)
    {
    }

// Virtual function
// @param [in] u  first variable
// @param [in] v  second variable
// @param [out] f  first function value
// @param [out] g  second function value
// @param [out] dfdu  derivative of f wrt u
// @param [out] dfdv  derivative of f wrt v
// @param [out] dgdu  derivative of g wrt u
// @param [out] dgdv  derivative of g wrt v
// @return true if function was evaluated.
virtual bool EvaluateRRToRRD(double uA, double uB, double &f, double &g, double &dfdu, double &dfdv, double &dgdu, double &dgdv) override
   {
    DPoint3d pointA, pointB;
    DVec3d d1A, d1B, d2A, d2B;
    if (    m_curveA->FractionToPoint (uA, pointA, d1A, d2A)
        &&  m_curveB->FractionToPoint (uB, pointB, d1B, d2B))
        {
#ifdef CompileCCADebugTracking
        if (s_collectDebugDetails && s_debugDetails.size() > 0)
            {
            CurveLocationDetailPair pair (m_curveA, uA, pointA, uB, pointB);
            s_debugDetails.back().push_back(pair);
            }
#endif
        DVec3d chord = DVec3d::FromStartEnd (pointA, pointB);
        f = d1A.DotProduct (chord);
        dfdu = d2A.DotProduct (chord) - d1A.DotProduct (d1A);
        dfdv = d1A.DotProduct (d1B);
        g = d1B.DotProduct (chord);
        dgdu = - d1B.DotProduct (d1A);
        dgdv = d2B.DotProduct (chord) + d1B.DotProduct (d1B);
        return true;
        }
    return false;
    }

};



/*--------------------------------------------------------------------------------**//**
* @bsistruct
+--------------------------------------------------------------------------------------*/
struct CCAXYZProcessor : public CurveCurveProcessAndCollectCloseApproaches
{
int m_maxIterations;
double m_newtonFractionTol;

/*--------------------------------------------------------------------------------**//**
* @param [in] maxDistance if positive, collect all approaches within this distance; otherwise collect only the closest approach.
* When Newton iteration is employed to compute the closest approach, two sub-options are available to control how many seeds are used.
* When maxDistance = 0 (default), use only one Newton seed, the closest approach between the stroked inputs.
* When maxDistance < 0, use all stroked close approaches within -maxDistance as Newton seeds (slower but more accurate).
* @param [in] strokeOptions optional stroking parameters to use for nonlinear curves. Default is nullptr (use default stroking parameters).
* @param [in] maxIterations maximum number of Newton iterations to run per seed for nonlinear curves. Default/negative is 20.
* @param [in] fractionTol parametric (fractional) convergence tolerance for Newton iteration. Default/negative is 1.0e-12.
* @param [in] coordTol absolute and relative tolerance for clustering solutions. Typically this is a coarse tolerance. Default/negative is 1.0e-4.
* @bsimethod
+--------------------------------------------------------------------------------------*/
CCAXYZProcessor (double maxDistance = 0.0, IFacetOptionsCP strokeOptions = nullptr, int maxIterations = -1.0, double fractionTol = -1.0, double coordTol = -1.0) :
    CurveCurveProcessAndCollectCloseApproaches(maxDistance, nullptr, coordTol)
    {
    if (strokeOptions)
        SetStrokeOptions(*strokeOptions);
    m_maxIterations = maxIterations > 0 ? maxIterations : 20;
    m_newtonFractionTol = fractionTol >= 0.0 ? fractionTol : 1.0e-12;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void TestEndPoints(ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverseOrder)
    {
    DPoint3d startA, endA, startB, endB; // closed curve duplicate approaches will be filtered
    if (!curveA->GetStartEnd(startA, endA) || !curveB->GetStartEnd(startB, endB))
        return;
    CurveLocationDetail detail;
    if (curveB->ClosestPointBounded(startA, detail))
        CollectPair(curveA, &startA, 0.0, curveB, &detail.point, detail.fraction, bReverseOrder);
    if (curveB->ClosestPointBounded(endA, detail))
        CollectPair(curveA, &endA, 1.0, curveB, &detail.point, detail.fraction, bReverseOrder);
    if (curveA->ClosestPointBounded(startB, detail))
        CollectPair(curveA, &detail.point, detail.fraction, curveB, &startB, 0.0, bReverseOrder);
    if (curveA->ClosestPointBounded(endB, detail))
        CollectPair(curveA, &detail.point, detail.fraction, curveB, &endB, 1.0, bReverseOrder);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bvector<DPoint3d> StrokeCurveForNewtonSeed(ICurvePrimitiveCR curve)
    {
    auto saveMaxEdgeLength = GetStrokeOptions()->GetMaxEdgeLength();
    if (IsLinear(&curve) && saveMaxEdgeLength > 0.0)
        GetStrokeOptions()->SetMaxEdgeLength(0.0);

    bvector<DPoint3d> strokes;
    curve.AddStrokes(strokes, *GetStrokeOptions());

    // ADO#1229059: 3 strokes is not enough to avoid local minima for an arc
    static size_t s_minStrokeCount = 5;
    if (!IsLinear(&curve) && strokes.size() < s_minStrokeCount)
        {
        auto segData = PolylineOps::SumSegmentLengths(strokes);
        auto maxEdgeLength = segData.Mean() / (s_minStrokeCount / segData.Count());
        GetStrokeOptions()->SetMaxEdgeLength(maxEdgeLength);
        strokes.clear();
        curve.AddStrokes(strokes, *GetStrokeOptions());
        }

    GetStrokeOptions()->SetMaxEdgeLength(saveMaxEdgeLength);
    return strokes;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessPrimitivePrimitive(ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverseOrder) override
    {
    bvector<DPoint3d> strokesA = StrokeCurveForNewtonSeed(*curveA);
    bvector<DPoint3d> strokesB = StrokeCurveForNewtonSeed(*curveB);

    // seeds are solutions to the discrete problem on the strokesA/B linestrings
    bvector<CurveLocationDetail> seedA;
    bvector<CurveLocationDetail> seedB;
    if (RefineClosestSeedOnly())
        {
        CurveLocationDetail detailA, detailB;
        if (PolylineOps::ClosestApproach(strokesA, strokesB, detailA, detailB))
            {
            seedA.push_back(detailA);
            seedB.push_back(detailB);
            }
        }
    else
        {
        if (!PolylineOps::AddCloseApproaches(strokesA, strokesB, seedA, seedB, GetMaxDistance()))
            return;
        }

    CurveCurveApproachIterate iterate(curveA, curveB);
    NewtonIterationsRRToRR newton(m_newtonFractionTol, m_newtonFractionTol);
    newton.SetMaxIterations(m_maxIterations);
    CurveLocationDetail detail;
    bool needNewton = !IsLinear(curveA) || !IsLinear(curveB);

    for (size_t i = 0; i < seedA.size(); i++)
        {
        double u = seedA[i].fraction;
        double v = seedB[i].fraction;

        // convert from linestring to curve fractions
        if (curveA->ClosestPointBounded(seedA[i].point, detail))
            u = detail.fraction;
        if (curveB->ClosestPointBounded(seedB[i].point, detail))
            v = detail.fraction;

        if (needNewton)
            newton.RunNewton(u, v, iterate);

        if (DoubleOps::IsIn01(u, v))
            CollectPair(curveA, curveB, u, v, bReverseOrder);
        }

    TestEndPoints(curveA, curveB, bReverseOrder);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
virtual void ProcessLineLine(ICurvePrimitiveP curveA, DSegment3dCR segmentA, ICurvePrimitiveP curveB, DSegment3dCR segmentB, bool bReverseOrder) override
    {
    DPoint3d pointA, pointB;
    double fractionA, fractionB;
    DSegment3d::ClosestApproachBounded (fractionA, fractionB, pointA, pointB, segmentA, segmentB);
    CollectPair(curveA, curveB, fractionA, fractionB, bReverseOrder);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessLineArc(
    ICurvePrimitiveP curveA, DSegment3dCR segmentA,
    ICurvePrimitiveP curveB, DEllipse3dCR arcB,
    bool bReverseOrder) override
    {
    auto segmentDirection = segmentA.VectorStartToEnd();

    // special case: closest approaches that are intersections with cylinder
    Transform arcToWorld, worldToArc;
    bool foundCylinderIntersections = false;
    if (arcB.GetLocalFrame(arcToWorld, worldToArc))
        {
        auto arcZ = arcToWorld.ColumnZ();
        if (arcZ.IsPerpendicularTo(segmentDirection))
            {
            // The segment is parallel to (but possibly above or below) the plane of the arc.
            // find intersections of the swept arc with the segment.
            // These are closest approaches
            double segmentFractions[10];
            DPoint3d arcXYZ[10];
            DPoint3d points[10];
            auto numIntersection = arcB.IntersectSweptDSegment3dBounded (points, arcXYZ, segmentFractions, segmentA);
            for (int i = 0; i < numIntersection; i++)
                {
                double fractionSegment = segmentFractions[i];
                double fractionArc = arcB.AngleToFraction (atan2 (arcXYZ[i].y, arcXYZ[i].x));
                CollectPair(curveA, curveB, fractionSegment, fractionArc, bReverseOrder);
                }
            foundCylinderIntersections = numIntersection > 0;
            }
        }

    // general case: if special case found an intersection, we can skip
    if (!foundCylinderIntersections)
        {
        RotMatrix localToWorldMatrix = RotMatrix::From1Vector(segmentDirection, 2, true);
        auto localToWorld = Transform::From(localToWorldMatrix, segmentA.point[0]);
        Transform worldToLocal;
        if (worldToLocal.InverseOf(localToWorld))
            {
            // convert to local frame with segment as z axis.
            DEllipse3d arcBLocal;
            worldToLocal.Multiply(arcBLocal, arcB);

            DPoint3d _[4]; // projections to local arc, unused
            double radians[4];
            int numProjections = arcBLocal.ProjectPointXYBounded(_, radians, DPoint3d::FromZero());
            for (int i = 0; i < numProjections; i++)
                {
                double arcFraction = arcB.AngleToFraction(radians[i]);
                DPoint3d arcPoint = arcB.RadiansToPoint(radians[i]);
                // project back to the world segment
                DPoint3d segmentPoint;
                double segmentFraction;
                if (segmentA.ProjectPointBounded(segmentPoint, segmentFraction, arcPoint))
                    CollectPair(curveA, &segmentPoint, segmentFraction, curveB, &arcPoint, arcFraction, bReverseOrder);
                }
            }
        }

    TestEndPoints(curveA, curveB, bReverseOrder);
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveCurve::AnnounceCloseApproaches
(
CurveVectorCR chainA,
CurveVectorCR chainB,
ICloseApproachAnnouncer const& announce,
double maxDistance,
IFacetOptionsCP strokeOptions,
int maxIterations,
double fractionTol,
double coordTol
)
    {
    CCAXYZProcessor processor(maxDistance, strokeOptions, maxIterations, fractionTol, coordTol);
    for(auto const& curveA : chainA)
        for(auto const& curveB : chainB)
            processor.Process(const_cast<ICurvePrimitiveP>(curveA.get()), const_cast<ICurvePrimitiveP>(curveB.get()));

    return processor.GetResults(announce);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveCurve::AnnounceCloseApproaches
(
ICurvePrimitiveCR curveA,
ICurvePrimitiveCR curveB,
ICloseApproachAnnouncer const& announce,
double maxDistance,
IFacetOptionsCP strokeOptions,
int maxIterations,
double fractionTol,
double coordTol
)
    {
    CCAXYZProcessor processor(maxDistance, strokeOptions, maxIterations, fractionTol, coordTol);

    processor.Process(const_cast<ICurvePrimitiveP>(&curveA), const_cast<ICurvePrimitiveP>(&curveB));

    return processor.GetResults(announce);
    }

// Announcer that collects all approaches into two parallel arrays of partial curves.
struct AllCloseApproaches
    {
    CurveVectorR m_pointsOnA;
    CurveVectorR m_pointsOnB;
    AllCloseApproaches(CurveVectorR a, CurveVectorR b) : m_pointsOnA(a), m_pointsOnB(b)
        {
        m_pointsOnA.clear();
        m_pointsOnB.clear();
        }
    CurveCurve::ICloseApproachAnnouncer GetAnnouncer()
        {
        return [this](CurveLocationDetailPairCR approach) -> void
            {
            double u = approach.detailA.fraction;
            double v = approach.detailB.fraction;
            m_pointsOnA.push_back(ICurvePrimitive::CreatePartialCurve(const_cast<ICurvePrimitiveP>(approach.detailA.curve), u, u));
            m_pointsOnB.push_back(ICurvePrimitive::CreatePartialCurve(const_cast<ICurvePrimitiveP>(approach.detailB.curve), v, v));
            };
        }
    };

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurve::CloseApproach
(
CurveVectorR pointsOnA,
CurveVectorR pointsOnB,
ICurvePrimitiveP curveA,
ICurvePrimitiveP curveB,
double maxDist
)
    {
    AnnounceCloseApproaches(*curveA, *curveB, AllCloseApproaches(pointsOnA, pointsOnB).GetAnnouncer(), maxDist <= 0 ? DBL_MAX : maxDist);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurve::CloseApproach
(
CurveVectorR pointsOnA,
CurveVectorR pointsOnB,
CurveVectorCR chainA,
CurveVectorCR chainB,
double maxDist
)
    {
    AnnounceCloseApproaches(chainA, chainB, AllCloseApproaches(pointsOnA, pointsOnB).GetAnnouncer(), maxDist <= 0 ? DBL_MAX : maxDist);
    }

// Announcer that collects the closest approach into two details.
struct TheClosestApproach
    {
    CurveLocationDetailR m_detailA;
    CurveLocationDetailR m_detailB;
    TheClosestApproach(CurveLocationDetailR a, CurveLocationDetailR b) : m_detailA(a), m_detailB(b) {}
    CurveCurve::ICloseApproachAnnouncer GetAnnouncer()
        {
        return [this](CurveLocationDetailPairCR approach) { m_detailA = approach.detailA; m_detailB = approach.detailB; };
        }
    };

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveCurve::ClosestApproach
(
CurveLocationDetailR detailA,
CurveLocationDetailR detailB,
ICurvePrimitiveP    curveA,
ICurvePrimitiveP    curveB
)
    {
    return AnnounceCloseApproaches(*curveA, *curveB, TheClosestApproach(detailA, detailB).GetAnnouncer());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool CurveCurve::ClosestApproach
(
CurveLocationDetailR detailA,
CurveLocationDetailR detailB,
CurveVectorCR    chainA,
CurveVectorCR    chainB
)
    {
    return AnnounceCloseApproaches(chainA, chainB, TheClosestApproach(detailA, detailB).GetAnnouncer());
    }

//! Search for locations where there is a local min or max in the Z distance between
//! curves that are montone increasing in X.
//! ASSUME no disconnects !!!
void CurveCurve::CollectLocalZApproachExtremaOrderedX
(
CurveVectorCR curveA,
CurveVectorCR curveB,
bvector<CurveLocationDetailPair> &localMin,   //!< [out] local minima
bvector<CurveLocationDetailPair> &localMax   //!< [out] local maxima
)
    {
    bvector<CurveLocationDetailPair> allPairs;
    bvector<DPoint3d>strokeA, strokeB;
    auto options = IFacetOptions::CreateForCurves ();
    curveA.AddStrokePoints (strokeA, *options);
    curveB.AddStrokePoints (strokeB, *options);
    PolylineOps::CollectZPairsForOrderedXPoints (strokeA, strokeB, allPairs);
    localMax.clear ();
    localMin.clear ();
    if (allPairs.size () == 0)
        return;
    if (allPairs.size () == 1)
        {
        localMin.push_back (allPairs.front ());
        localMax.push_back (allPairs.front ());
        return;
        }
    double d0 = allPairs[0].DeltaZ ();
    double d1 = allPairs[1].DeltaZ ();
    if (d0 <= d1)
        localMin.push_back (allPairs[0]);
    if (d0 >= d1)
        localMax.push_back (allPairs[0]);
    double d2 = 0.0;
    for (size_t i = 2; i < allPairs.size (); i++, d0 = d1, d1 = d2)
        {
        d2 = allPairs[i].DeltaZ ();
        double e01 = d1 - d0;
        double e12 = d2 - d1;
        double a = e01 * e12;
        if (a > 0.0)
            {
            // same direction, not an extrema
            }
        else if (a < 0.0)
            {
            // simple min or max
            if (e01 > 0)
                localMax.push_back (allPairs[i-1]);
            else
                localMin.push_back (allPairs[i-1]);
            }
        else
            {
            // least one is true zero ... record if one is not zero
            if (e01 > 0.0 || e12 < 0.0)
                localMax.push_back (allPairs[i-1]);
            else if (e01 < 0.0 || e12 > 0.0)
                localMax.push_back (allPairs[i-1]);
            }
        }
        if (d0 <= d1)
            localMax.push_back (allPairs.back ());
        if (d0 >= d1)
            localMin.push_back (allPairs.back ());
    }


//! Search for locations where there is a local min or max in the Z distance between
//! curves that are montone increasing in X.
//! ASSUME no disconnects !!!
void CurveCurve::CollectOrderedXVerticalStrokes
(
CurveVectorCR curveA,
CurveVectorCR curveB,
bvector<CurveLocationDetailPair> &allPairs
)
    {
    allPairs.clear ();
    bvector<DPoint3d>strokeA, strokeB;
    auto options = IFacetOptions::CreateForCurves ();
    curveA.AddStrokePoints (strokeA, *options);
    curveB.AddStrokePoints (strokeB, *options);
    PolylineOps::CollectZPairsForOrderedXPoints (strokeA, strokeB, allPairs);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
