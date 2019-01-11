/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/funcs/DPoint3dOpsRangeHeap.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/BinaryRangeHeap.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static bool s_debug = false;
static size_t s_polylinePolylineHeapTransition = 20;

void PolylineOps::SetPolylinePolylineHeapTrigger (size_t mn)
    {
    s_polylinePolylineHeapTransition = mn;
    }

struct PolylineSubRangeServer
    : IndexedRangeHeap::IndexedRangeSource
{
bvector <DPoint3d> const &m_points;
bool m_bXYOnly;
PolylineSubRangeServer (bvector <DPoint3d> const &points, bool bXYOnly) : m_points(points), m_bXYOnly (bXYOnly) {}
// Get range of segments (not points) from i0 to i1 inclusive.
//  Hence points i0..i1+1
bool GetRange (size_t i0, size_t i1, DRange3dR range) const override
    {
    range = DRange3d::NullRange ();
    size_t n = m_points.size ();
    i1++;  // End index for final semgent.
    if (i1 >= n)
        i1 = n - 1;
    if (i1 < i0)
        return false;
    range = DRange3d::From (m_points.data () + i0, (int)(i1 - i0 + 1));
    if (m_bXYOnly)
        {
        range.low.z  = 0.0;
        range.high.z = 1.0;
        }
    return true;
    }
};


double ResolveFraction (double componentFraction, size_t baseIndex, double df, bvector<double> const * params)
    {
    if (params != NULL && baseIndex + 1 < params->size ())
        return (1.0 - componentFraction) * params->at (baseIndex)
                + componentFraction * params->at (baseIndex + 1);
    return ((double)baseIndex + componentFraction) * df;
    }

struct PolylineCloseApproach_HeapSearch : IndexedRangeHeap::PairProcessor
{
GEOMAPI_VIRTUAL ~PolylineCloseApproach_HeapSearch(){}

bvector<DPoint3d> const &m_xyzA;
bvector<double>  const *m_paramA;

bvector<DPoint3d> const &m_xyzB;
bvector<double>  const *m_paramB;

bvector<CurveLocationDetail> *m_locationA;
bvector<CurveLocationDetail> *m_locationB;
CurveLocationDetail m_finalLocationA;
CurveLocationDetail m_finalLocationB;
size_t m_hitCount;
double m_searchDistance;
bool  m_collectorMode;

PolylineSubRangeServer m_rangeServerA;
PolylineSubRangeServer m_rangeServerB;
IndexedRangeHeap   m_heapA;
IndexedRangeHeap   m_heapB;
size_t m_test;
size_t m_leafProcess;
bool m_bXYOnly;

// If the bvector pointers are provided, the given maxDist is used unchanged throughout the search.
// If no bvector pointers are provided, maxDist is reduced as the search progresses, and only the last

// "collector" constructor.   maxDist is left unchanged during the search.
PolylineCloseApproach_HeapSearch
(
bvector<DPoint3d> const &xyzA,
bvector<double> const *paramA,
bvector<DPoint3d> const &xyzB,
bvector<double> const *paramB,
bvector< CurveLocationDetail> *locationA,
bvector< CurveLocationDetail> *locationB,
double    maxDist,
bool bXYOnly = false
) : m_xyzA (xyzA), m_xyzB (xyzB),
    m_locationA (locationA), m_locationB (locationB),
    m_rangeServerA (xyzA, bXYOnly), m_rangeServerB (xyzB, bXYOnly),
    m_heapA (), m_heapB (),
    m_searchDistance (maxDist),
    m_paramA(paramA), m_paramB(paramB),
    m_bXYOnly (bXYOnly)
    {
    m_test = 0;
    m_leafProcess = 0;
    m_collectorMode = true;
    }

// "Single point" constructor.  maxDist starts at DBL_MAX, decreases as closer approaches are found.
PolylineCloseApproach_HeapSearch
(
bvector<DPoint3d> const &xyzA,
bvector<double> const *paramA,
bvector<DPoint3d> const &xyzB,
bvector<double> const *paramB,
bool  bXYOnly
) : m_xyzA (xyzA), m_xyzB (xyzB),
    m_locationA (NULL), m_locationB (NULL),
    m_rangeServerA (xyzA, bXYOnly), m_rangeServerB (xyzB, bXYOnly),
    m_heapA (), m_heapB (),
    m_paramA(paramA), m_paramB(paramB),
    m_bXYOnly (bXYOnly)
    {
    m_searchDistance = DBL_MAX;
    m_test = 0;
    m_leafProcess = 0;
    m_collectorMode = false;
    }

bool GetFinalLocations (CurveLocationDetailR locationA, CurveLocationDetailR locationB)
    {
    if (m_hitCount > 0)
        {
        locationA = m_finalLocationA;
        locationB = m_finalLocationB;
        return true;
        }
    return false;
    }




bool IsLive () const override {return true;}
bool NeedProcessing (DRange3dCR rangeA, size_t iA0, size_t iA1,
                        DRange3dCR rangeB, size_t iB0, size_t iB1) override
    {
    m_test++;
    if (m_searchDistance == DBL_MAX)
        return true;
    bool b = rangeA.IntersectsWith (rangeB, m_searchDistance, 3);
    return b;
    };

void Process (size_t iA, size_t iB) override 
    {
    m_leafProcess++;
    DSegment3d segmentA = DSegment3d::From (m_xyzA[iA], m_xyzA[iA + 1]);
    DSegment3d segmentB = DSegment3d::From (m_xyzB[iB], m_xyzB[iB + 1]);
    double fractionA, fractionB;
    DPoint3d pointA, pointB;
    double d;
    if (m_bXYOnly)
        {
        DSegment3d::ClosestApproachBoundedXY (fractionA, fractionB,
                pointA,   pointB,
                segmentA, segmentB);
        d = pointA.DistanceXY (pointB);
        if (pointA.AlmostEqualXY (pointB))
            d = 0.0;
        }
    else
        {
        DSegment3d::ClosestApproachBounded (fractionA, fractionB,
                pointA,   pointB,
                segmentA, segmentB);
        d = pointA.Distance (pointB);
        if (pointA.AlmostEqual (pointB))
            d = 0.0;
        }
    if (d <= m_searchDistance)
        {
        double dfA = 1.0 / (double)(m_xyzA.size () - 1);
        double dfB = 1.0 / (double)(m_xyzB.size () - 1);

        CurveLocationDetail detailA, detailB;
        detailA.a               = detailB.a = d;
        detailA.fraction        = ResolveFraction (fractionA, iA, dfA, m_paramA);
        detailB.fraction        = ResolveFraction (fractionB, iB, dfB, m_paramB);
        detailA.componentIndex  = iA;
        detailB.componentIndex  = iB;
        detailA.componentFraction = fractionA;
        detailB.componentFraction = fractionB;
        detailA.numComponent    = m_xyzA.size () - 1;
        detailB.numComponent    = m_xyzB.size () - 1;
        detailA.point           = pointA;
        detailB.point           = pointB;
        m_finalLocationA = detailA;
        m_finalLocationB = detailB;
        m_hitCount++;
        if (m_collectorMode)
            {
            m_locationA->push_back (detailA);
            m_locationB->push_back (detailB);
            }
        else
            {
            m_searchDistance = d;
            }
        }
    }

void Search ()
    {
    m_hitCount = 0;
    if (m_xyzA.size () < 2 || m_xyzB.size () < 2)
        return;
    if (m_collectorMode)
        {
        }
    else
        {
        m_searchDistance = DBL_MAX;
        }
    m_heapA.Build (1, &m_rangeServerA, 0, m_xyzA.size () - 2);
    m_heapB.Build (1, &m_rangeServerB, 0, m_xyzB.size () - 2);
    IndexedRangeHeap::Search (m_heapA, m_heapB, *this);    
    }

};

bool PolylineOps::AddCloseApproaches
(
bvector<DPoint3d> const &xyzA,
bvector<DPoint3d> const &xyzB,
bvector< CurveLocationDetail> &locationA,
bvector< CurveLocationDetail> &locationB,
double    maxDist
)
    {
    return PolylineOps::AddCloseApproaches (xyzA, NULL, xyzB, NULL, locationA, locationB, maxDist);
    }


bool PolylineOps::AddCloseApproaches
(
bvector<DPoint3d> const &xyzA,
bvector<double> const *paramA,
bvector<DPoint3d> const &xyzB,
bvector<double> const *paramB,
bvector< CurveLocationDetail> &locationA,
bvector< CurveLocationDetail> &locationB,
double    maxDist
)
    {
    if (xyzA.size () * xyzB.size () > s_polylinePolylineHeapTransition
        && xyzA.size () > 1 && xyzB.size () > 1)
        {
        PolylineCloseApproach_HeapSearch searcher (xyzA, paramA, xyzB, paramB, &locationA, &locationB, maxDist);
        searcher.Search ();
        size_t mn = (xyzA.size () - 1) * (xyzB.size () - 1);
        if (s_debug)
            printf (" Search m=%d n=%d mn=%d rangeTests=%d (leaf %d) (fI %g) (fL %g)\n",
                    (int)(xyzA.size () - 1), (int)(xyzB.size () - 1),
                    (int)mn,
                    (int)searcher.m_test, (int)searcher.m_leafProcess,
                    (double)searcher.m_test / (double)mn,
                    (double)searcher.m_leafProcess / (double)mn
                    );
        return true;
        }

    size_t numA = xyzA.size ();
    size_t numB = xyzB.size ();
    bool found = false;
    double dfA = 1.0 / (numA - 1);
    double dfB = 1.0 / (numB - 1);
    for (size_t iA = 1; iA < numA; iA++)
        {
        DSegment3d segmentA = DSegment3d::From (xyzA[iA-1], xyzA[iA]);
        for (size_t iB = 1; iB < numB; iB++)
            {
            DSegment3d segmentB = DSegment3d::From (xyzB[iB-1], xyzB[iB]);
            double fractionA, fractionB;
            DPoint3d pointA, pointB;
            DSegment3d::ClosestApproachBounded (fractionA, fractionB,
                        pointA,   pointB,
                        segmentA, segmentB);
            double d = pointA.Distance (pointB);
            if (d < maxDist)
                {
                CurveLocationDetail detailA, detailB;
                detailA.a               = detailB.a = d;
                detailA.fraction        = ResolveFraction (fractionA, iA - 1, dfA, paramA);
                detailB.fraction        = ResolveFraction (fractionB, iB - 1, dfB, paramB);
                detailA.componentIndex  = iA - 1;
                detailB.componentIndex  = iB - 1;
                detailA.componentFraction = fractionA;
                detailB.componentFraction = fractionB;
                detailA.numComponent    = numA - 1;
                detailB.numComponent    = numB - 1;
                detailA.point           = pointA;
                detailB.point           = pointB;
                locationA.push_back (detailA);
                locationB.push_back (detailB);
                found = true;
                }
            }
        }
    return found;
    }


bool PolylineOps::CollectIntersectionsAndCloseApproachesXY
(
bvector<DPoint3d> const &xyzA,
bvector<double> const *paramA,
bvector<DPoint3d> const &xyzB,
bvector<double> const *paramB,
bvector< CurveLocationDetail> &locationA,
bvector< CurveLocationDetail> &locationB,
double    maxDist
)
    {
    locationA.clear ();
    locationB.clear ();
    if (xyzA.size () * xyzB.size () > s_polylinePolylineHeapTransition
        && xyzA.size () > 1 && xyzB.size () > 1)
        {
        PolylineCloseApproach_HeapSearch searcher (xyzA, paramA, xyzB, paramB, &locationA, &locationB, maxDist, true);
        searcher.Search ();
        size_t mn = (xyzA.size () - 1) * (xyzB.size () - 1);
        if (s_debug)
            printf (" Search m=%d n=%d mn=%d rangeTests=%d (leaf %d) (fI %g) (fL %g)\n",
                    (int)(xyzA.size () - 1), (int)(xyzB.size () - 1),
                    (int)mn,
                    (int)searcher.m_test, (int)searcher.m_leafProcess,
                    (double)searcher.m_test / (double)mn,
                    (double)searcher.m_leafProcess / (double)mn
                    );
        return true;
        }

    size_t numA = xyzA.size ();
    size_t numB = xyzB.size ();
    bool found = false;
    double dfA = 1.0 / (numA - 1);
    double dfB = 1.0 / (numB - 1);
    for (size_t iA = 1; iA < numA; iA++)
        {
        DSegment3d segmentA = DSegment3d::From (xyzA[iA-1], xyzA[iA]);
        for (size_t iB = 1; iB < numB; iB++)
            {
            DSegment3d segmentB = DSegment3d::From (xyzB[iB-1], xyzB[iB]);
            double fractionA, fractionB;
            DPoint3d pointA, pointB;
            DSegment3d::ClosestApproachBoundedXY (fractionA, fractionB,
                        pointA,   pointB,
                        segmentA, segmentB);
            double d = pointA.DistanceXY (pointB);
            if (d < maxDist || pointA.AlmostEqualXY (pointB))
                {
                CurveLocationDetail detailA, detailB;
                detailA.a               = detailB.a = d;
                detailA.fraction        = ResolveFraction (fractionA, iA - 1, dfA, paramA);
                detailB.fraction        = ResolveFraction (fractionB, iB - 1, dfB, paramB);
                detailA.componentIndex  = iA - 1;
                detailB.componentIndex  = iB - 1;
                detailA.componentFraction = fractionA;
                detailB.componentFraction = fractionB;
                detailA.numComponent    = numA - 1;
                detailB.numComponent    = numB - 1;
                detailA.point           = pointA;
                detailB.point           = pointB;
                locationA.push_back (detailA);
                locationB.push_back (detailB);
                found = true;
                }
            }
        }
    return found;
    }




bool PolylineOps::ClosestApproach
(
bvector<DPoint3d> const &xyzA,
bvector<DPoint3d> const &xyzB,
CurveLocationDetailR locationA,
CurveLocationDetailR locationB
)
    {
    if (xyzA.size () > 1 && xyzB.size () > 1)
        {
        PolylineCloseApproach_HeapSearch searcher (xyzA, NULL, xyzB, NULL, false);
        searcher.Search ();
        if (searcher.GetFinalLocations (locationA, locationB))
            return true;
        }
    return false;    
    }

END_BENTLEY_GEOMETRY_NAMESPACE
