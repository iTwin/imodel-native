/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "CurveCurveProcessor.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

double Snap01(double x, double tolerance = 1.0e-10)
    {
    if (fabs (x) < tolerance)
        return 0.0;
    if (fabs (x - 1) < tolerance)
        return 1.0;
    return x;
    }
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

int m_fixedCurve;

CurveCurveApproachIterate (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB) :
    m_curveA(curveA), m_curveB (curveB), m_fixedCurve (0)
    {
    }
public:
void SetFixedCurve (int select)
    {
    m_fixedCurve = select;
    }
public:
// Virtual function
// @param [in] u  first variable
// @param [in] v  second variable
// @param [out]f  first function value
// @param [out]g  second function value
// @param [out]dfdu  derivative of f wrt u
// @param [out]dfdv  derivative of f wrt v
// @param [out]dgdu  derivative of g wrt u
// @param [out]dgdv  derivative of g wrt v
// @return true if function was evaluated.
bool EvaluateRRToRRD
(
double uA,
double uB,
double &f,
double &g,
double &dfdu,
double &dfdv,
double &dgdu,
double &dgdv
) override
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
        // To restrict iteration to one curve:
        // 1) both offdiagonals zero.
        // 2) "other" curve's functino zero
        // 3) "other" diagonal 1.0    (so jacobian is invertible)
        if (m_fixedCurve == 1)
            {
            dfdv = dgdu = 0.0;
            dgdv = 1.0;
            g = 0.0;
            }
        else if (m_fixedCurve == 2)
            {
            dfdv = dgdu = 0.0;
            dfdu = 1.0;
            f = 0.0;
            }
        return true;
        }
    return false;
    }

};



/*--------------------------------------------------------------------------------**//**
* @bsistruct
+--------------------------------------------------------------------------------------*/
struct CCAXYZProcessor : public CurveCurveProcessAndCollect
{
double m_maxDistance;
bool m_testEndPoints;

CCAXYZProcessor (CurveVectorR resultA, CurveVectorR resultB,
            double tol,
            bool   testEndpoints = true,
            double maxDistance = DBL_MAX) :
    CurveCurveProcessAndCollect (resultA, resultB, NULL, tol)
    {
    m_maxDistance = maxDistance;
    m_testEndPoints = testEndpoints;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CollectApproach(
ICurvePrimitiveP curveA,
ICurvePrimitiveP curveB,
double fractionA,
double fractionB,
DPoint3dCR pointA,
DPoint3dCR pointB,
bool bReverseOrder
)
    {
    double d = pointA.Distance (pointB);
    if (d < m_maxDistance)
        {
        CollectPair (curveA, curveB, fractionA, fractionB, bReverseOrder);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CollectApproach(CurveLocationDetailPairCR location, bool bReverseOrder)
    {
    if (location.detailA.a < m_maxDistance)
        {
        CollectPair (
                const_cast <ICurvePrimitiveP>(location.detailA.curve),
                const_cast <ICurvePrimitiveP>(location.detailB.curve),
                location.detailA.fraction, location.detailB.fraction,
                bReverseOrder);
        }
    }

bvector<DPoint3d> m_strokeA;
bvector<DPoint3d> m_strokeB;
bvector <CurveLocationDetail> m_locationA;
bvector <CurveLocationDetail> m_locationB;


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessPrimitivePrimitive(ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverse) override
    {
    static double s_absTol = 1.0e-14;
    m_strokeA.clear ();
    m_strokeB.clear ();
    m_locationA.clear ();
    m_locationB.clear ();    
    curveA->AddStrokes (m_strokeA, *GetStrokeOptions ());
    curveB->AddStrokes (m_strokeB, *GetStrokeOptions ());
    bool needNewton = !IsLinear (curveA) || !IsLinear (curveB);
    CurveCurveApproachIterate iterate (curveA, curveB);
    NewtonIterationsRRToRR newton (s_absTol);
    if (PolylineOps::AddCloseApproaches (m_strokeA, m_strokeB, m_locationA, m_locationB, m_maxDistance))
        {
        for (size_t i = 0; i < m_locationA.size (); i++)
            {
            if (needNewton)
                newton.RunNewton (m_locationA[i].fraction, m_locationB[i].fraction, iterate);
            CurveLocationDetailPair location (m_locationA[i], m_locationB[i]);
            location.Set ((ICurvePrimitiveP)curveA, (ICurvePrimitiveP)curveB);
            CollectApproach (location, bReverse);
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void UpdateMin(
CurveLocationDetailPairR location,
double fractionA,
DPoint3dCR pointA,
double fractionB,
DPoint3dCR pointB
)
    {
    double d = pointA.Distance (pointB);
    if (d < location.detailA.a)
        {
        location.Set (fractionA, pointA, d, fractionB, pointB, d);
        }        
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void TestEndPoints
(
ICurvePrimitiveP curveA,
ICurvePrimitiveP curveB,
bool reversed
)
    {
    double endFraction[2] = {0.0, 1.0};
    DPoint3d endPointA[2], endPointB[2];
    for (int i = 0; i < 2; i++)
        {
        curveA->FractionToPoint (endFraction[i], endPointA[i]);
        curveB->FractionToPoint (endFraction[i], endPointB[i]);
        }

    CurveLocationDetailPair location (curveA, DBL_MAX, curveB, DBL_MAX);
    //double dMin = DBL_MAX;
    for (int i = 0; i < 2; i++)
        {
#ifdef explicitEndpointTest
        for (int j = 0; j < 2; j++)
            {
            Update (location, endFraction[i], endPointA[i]
                              endFraction[j], endPointB[j]);
            }
#endif
        DPoint3d pointA, pointB;
        double fractionA, fractionB;
        if (curveB->ClosestPointBounded (endPointA[i], fractionB, pointB))
            UpdateMin (location, endFraction[i], endPointA[i], fractionB, pointB);
        if (curveA->ClosestPointBounded (endPointB[i], fractionA, pointA))
            UpdateMin (location, fractionA, pointA, endFraction[i], endPointB[i]);
        }

    CollectApproach (location, reversed);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessLineLine(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DSegment3dCR segmentB,
        bool bReverseOrder) override
    {
    DPoint3d pointA, pointB;
    double fractionA, fractionB;
    // A true closest approach wins.
    // Closest endpoint approach follows.
    if (DSegment3d::ClosestApproachUnbounded
                (
                fractionA, fractionB,
                pointA, pointB,
                segmentA, segmentB)
        && validEdgeFractionWithinLinestring (fractionA, 0, 2)
        && validEdgeFractionWithinLinestring (fractionB, 0, 2))
        {
        CollectApproach (curveA, curveB, fractionA, fractionB, pointA, pointB, bReverseOrder);
        }
    else
        {
        TestEndPoints (curveA, curveB, bReverseOrder);
        }
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
    BentleyApi::Transform arcToWorld, worldToArc;
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
                double fractionArc = arcB.AngleToFraction (
                            atan2 (arcXYZ[i].y, arcXYZ[i].x));
                DPoint3d xyzArc = arcB.FractionToPoint (fractionArc);
                DPoint3d xyzSegment = segmentA.FractionToPoint (fractionSegment);
                CollectApproach(
                    curveA, curveB,
                    fractionSegment, fractionArc,
                    xyzSegment, xyzArc,
                    bReverseOrder);

                }
            return;
            }
        }
    RotMatrix localToWorldMatrix = RotMatrix::From1Vector(segmentDirection, 2, true);
    auto localToWorld = BentleyApi::Transform::From(localToWorldMatrix, segmentA.point[0]);
    BentleyApi::Transform worldToLocal;
    if (worldToLocal.InverseOf(localToWorld))
        {
        DEllipse3d arcBLocal;
        // convert to local frame with segment as z axis.
        worldToLocal.Multiply(arcBLocal, arcB);
        DPoint3d closestPointLocal;
        double fractionArc, distanceXY;
        // find closest point in the projection
        if (arcBLocal.ClosestPointBoundedXY(closestPointLocal, fractionArc, distanceXY,
            DPoint3d::From(0, 0, 0), nullptr, false, false))
            {
            // Evaluate 3d ellipse point.
            auto closePointArc = arcB.FractionToPoint(fractionArc);
            // project back to segment (within the extend of the segment)
            DPoint3d closePointSegment;
            double fractionSegment;
            segmentA.ProjectPointBounded(closePointSegment, fractionSegment, closePointArc, false, false);
            CollectApproach(
                curveA, curveB, 
                fractionSegment, fractionArc,
                closePointSegment, closePointArc,
                bReverseOrder);
            }
        }
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
    double tol = 0.0;
    CCAXYZProcessor processor (pointsOnA, pointsOnB, tol, true, maxDist);
    processor.Process (curveA, curveB);
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
    double tol = 0.0;
    CCAXYZProcessor processor (pointsOnA, pointsOnB, tol, true, maxDist);
    for(ICurvePrimitivePtr curveA : chainA)
        for(ICurvePrimitivePtr curveB : chainB)
            processor.Process (curveA.get (), curveB.get ());                        
    }


/*--------------------------------------------------------------------------------**//**
* @bsistruct
+--------------------------------------------------------------------------------------*/
struct CCAMinXYZProcessor :
    public CurveCurveProcessAndSelectMinimum <CurveLocationDetail>
{

CCAMinXYZProcessor () :
    CurveCurveProcessAndSelectMinimum <CurveLocationDetail> (NULL, 0.0)
    {
#ifdef CompileCCADebugTracking
    if (s_collectDebugDetails)
        {
        s_debugDetails.clear ();
        }
#endif
    }


bvector<DPoint3d> m_strokeA;
bvector<DPoint3d> m_strokeB;

// INSTALL curve pointers.
// REJECT if fractions outside 01
// EVALUATE (at current fraction)
// 
void EvaluateAndTestMin (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverse,
      CurveLocationDetailR locationA, CurveLocationDetailR locationB)
      {
#ifdef CompileCCADebugTracking
      if (s_collectDebugDetails && s_debugDetails.size () > 0)
        s_debugDetails.back ().push_back (CurveLocationDetailPair(locationA, locationB));
#endif
    if (DoubleOps::IsIn01 (Snap01 (locationA.fraction), Snap01 (locationB.fraction)))
          {
          locationA.curve = (ICurvePrimitiveP)curveA;
          locationB.curve = (ICurvePrimitiveP)curveB;
          curveA->FractionToPoint (locationA.fraction, locationA.point);
          curveB->FractionToPoint (locationB.fraction, locationB.point);
          TestAndCollectMin (locationA.Distance (locationB), locationA, locationB, bReverse);
          }
      }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessPrimitivePrimitive(ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverse) override
    {
    static double s_absTol = 1.0e-10;
    static double s_softTol = 1.0e-6;
    m_strokeA.clear ();
    m_strokeB.clear ();
    curveA->AddStrokes (m_strokeA, *GetStrokeOptions ());
    curveB->AddStrokes (m_strokeB, *GetStrokeOptions ());
    bool needNewton = !IsLinear (curveA) || !IsLinear (curveB);
    CurveLocationDetail locationA, locationB;
    if (PolylineOps::ClosestApproach (m_strokeA, m_strokeB, locationA, locationB))
        {
        DPoint3d pointA = locationA.point;
        DPoint3d pointB = locationB.point;
        if (  curveA->ClosestPointBounded(pointA, locationA, false, false)
           && curveB->ClosestPointBounded(pointB, locationB, false, false))
           {
#ifdef CompileCCADebugTracking
            if (s_collectDebugDetails)
               s_debugDetails.push_back(bvector<CurveLocationDetailPair>());
#endif
            EvaluateAndTestMin (curveA, curveB, bReverse, locationA, locationB);
            if (needNewton)
                {
                NewtonIterationsRRToRR newton (s_absTol);
                newton.SetSoftTolerance (s_softTol, s_softTol);
                double fA0 = locationA.fraction;
                double fB0 = locationB.fraction;
                CurveCurveApproachIterate iterate (curveA, curveB);
                for (int select = 0; select < 3; select++)
                    {
#ifdef CompileCCADebugTracking
                    StartNewDebug ();
#endif
                    locationA.fraction = fA0;
                    locationB.fraction = fB0;
                    iterate.SetFixedCurve (select);
                    newton.RunNewton (locationA.fraction, locationB.fraction, iterate);
                    EvaluateAndTestMin (curveA, curveB, bReverse, locationA, locationB);
                    }
                }
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ProcessLineLine(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DSegment3dCR segmentB,
        bool bReverseOrder) override
    {
    DPoint3d pointA, pointB;
    double fractionA, fractionB;
    DSegment3d::ClosestApproachBounded (fractionA, fractionB, pointA, pointB, segmentA, segmentB);
    TestAndCollectMin (
        pointA.Distance (pointB),
        CurveLocationDetail (curveA, fractionA, pointA),
        CurveLocationDetail (curveB, fractionB, pointB),
        bReverseOrder);
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
    CCAMinXYZProcessor searcher;
    searcher.Process (curveA, curveB);
    double d;
    return searcher.GetResult (d, detailA, detailB);
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
    CCAMinXYZProcessor searcher;
    for(ICurvePrimitivePtr curveA : chainA)
        for(ICurvePrimitivePtr curveB : chainB)
            searcher.Process (curveA.get (), curveB.get ()); 
    double d;
    return searcher.GetResult (d, detailA, detailB);    
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
