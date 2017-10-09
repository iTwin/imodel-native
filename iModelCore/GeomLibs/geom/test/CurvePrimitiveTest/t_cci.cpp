//
//
#include "testHarness.h"
#include <Geom/BinaryRangeHeap.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL


static int s_noisy = 0;    
// Support data for fast search of curve vectors.
// This has two levels:
// 1) range tree with ranges of primitives.
// 2) For (large) linestring primitives, an additional range tree for the linestring segments.
struct CurveVectorRangeData :  IndexedRangeHeap::IndexedRangeSource
{
private:
CurveVectorPtr m_curveVector;
// The master curve vector is a Ptr.  So are all its internals.
// So the raw pointers in this index are safe . . .
bvector<ICurvePrimitive*> m_leafPrimitives;
BENTLEY_GEOMETRY_INTERNAL_NAMESPACE_NAME::IndexedRangeHeap m_heap;
bvector<size_t>  m_heapIndexToReadIndex;
public:
CurveVectorRangeData (){}

void AppendPrimitives (CurveVectorCR parent)
    {
    for (ICurvePrimitivePtr const &primitive : parent)
        {
        CurveVectorPtr child = primitive->GetChildCurveVectorP ();
        if (child.IsValid ())
            AppendPrimitives (*child);
        else
            m_leafPrimitives.push_back (primitive.get ());
        }
    }

virtual bool GetRange (size_t i0, size_t i1, DRange3d &range) const override
    {
    range.Init ();
    if (m_leafPrimitives.size () < i1)
        i1 = m_leafPrimitives.size ();
     
    for (size_t i = i0; i <= i1; i++)
        {
        DRange3d rangeI;
        if (m_leafPrimitives[i]->GetRange (rangeI))
            range.Extend (rangeI);
        }
    return !range.IsNull ();        
    }
// Construct a range tree for this curve vector.
bool Install (CurveVectorPtr &curveVector)
    {
    m_leafPrimitives.clear ();
    m_curveVector = curveVector;
    AppendPrimitives (*curveVector);
    if (m_leafPrimitives.size () == 0)
        return false;
    m_heap.Build (1, this, 0, m_leafPrimitives.size () - 1);
    return true;
    }
};

void AddSegments (CurveVectorR cv, int numSegment, DSegment3dCR segment0, DVec3dCR shift)
    {
    DSegment3d segment;
    for (int i = 0; i < numSegment; i++)
        {
        cv.Add (ICurvePrimitive::CreateLine (segment));
        segment.point[0].Add (shift);
        segment.point[1].Add (shift);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,RangeTree)
{
CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
AddSegments (*cv, 5, DSegment3d::From (DPoint3d::From (0,0,0), DPoint3d::From (1,1,0)), DVec3d::From (2,0,0));
CurveVectorRangeData cvRanges;
cvRanges.Install (cv);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveCurve, TransverseIntersection)
    {
    for (int numB = 1; numB < 6; numB += 2)
        {
        auto waveA = CurveVector::CreateRectangle (0,0, numB * 8,6, 0.0);
        auto waveB = SquareWavePolygon (numB, 1.0, 2.0, 3.0, -1.0, 2.0, true, -3.0);
        Transform transform = Transform::FromRowValues
            (
            1,0,0, -1,
            0,0,-1, 1,
            0,1,0, -1
            );
        waveA->TransformInPlace (transform);
        bvector<DSegment3d> segments;
        if (s_noisy)
            {
            Check::Print (*waveA, "XZ Plane Rectangle");
            Check::Print (*waveB, "WaveB");
            }
        Check::True (CurveCurve::TransverseRegionIntersectionSegments (*waveA, *waveB, segments));
        auto cvSegments = CurveVector::Create (segments);
        if (s_noisy)
            Check::Print (*cvSegments, "Segments");
        // CHECK:  All segment midpoints are IN
        for (DSegment3d segment : segments)
            {
            DPoint3d midPoint = segment.FractionToPoint (0.5);
            DPoint3d pointA, pointB;
            Check::True
                (
                CurveVector::INOUT_In == waveA->ClosestCurveOrRegionPoint (midPoint, pointA),
                "interior point of A"
                );
            Check::True
                (
                CurveVector::INOUT_In == waveB->ClosestCurveOrRegionPoint (midPoint, pointB),
                "interior point of B"
                );
            }
        // Assume segments are ordered.
        // Check that midpoints between segments are OUT of at least 1
        for (size_t i = 0; i + 1 < segments.size (); i++)
            {
            DPoint3d midPoint = DPoint3d::FromInterpolate (segments[i].point[1], 0.5, segments[i + 1].point[0]);
            DPoint3d pointA, pointB;
            // This classification is for the returned pointA and pointB, not for the midPoint  Hence an "outside" midpoint
            //  gets and "On" for closest point.
            auto inA = waveA->ClosestCurveOrRegionPoint (midPoint, pointA);
            auto inB = waveB->ClosestCurveOrRegionPoint (midPoint, pointB);
            Check::True
                (inA == CurveVector::INOUT_On || inB == CurveVector::INOUT_On, "known exterior point");
            }
        }
    }

void AddSelfIntersectingCurves (bvector<CurveVectorPtr> &curves)
    {
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DPoint3d pointAB = DPoint3d::From (10,0,0);
    DPoint3d pointC = DPoint3d::From (5,10,0);
    DPoint3d pointQ = DPoint3d::From (3,-2,0);
    auto cv0 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cv0->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointAB)));
    cv0->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointAB, pointC)));
    cv0->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointC, pointQ)));
    curves.push_back (cv0);

    auto cv1 = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cv1->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointAB)));
    cv1->push_back (ICurvePrimitive::CreateArc (
            DEllipse3d::FromPointsOnArc (
                    pointAB,
                    DPoint3d::FromInterpolateAndPerpendicularXY (pointAB, 0.3, pointQ, -0.3),
                    pointQ
                    )));
    cv1->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointQ, pointC)));
    curves.push_back (cv1);

    }
void SavePartialCurveMarkers (CurveVectorR curves, double markerSize = 0.2)
    {
    bvector<DPoint3d> markers;
    for (auto cp : curves)
        {
        double fractionA, fractionB;
        ICurvePrimitivePtr parent;
        int64_t tag;
        DPoint3d xyzA, xyzB;
        if (cp->TryGetPartialCurveData (fractionA, fractionB, parent, tag))
            {
            cp->FractionToPoint (0.0, xyzA);
            cp->FractionToPoint (1.0, xyzB);
            if (fractionA == fractionB)
                markers.push_back (xyzA);
            else
                {
                auto cp1 = cp->CloneDereferenced ();
                if (cp1.IsValid ())
                    Check::SaveTransformed (*cp1);
                }
            }
        }
    Check::SaveTransformedMarkers (markers, markerSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveCurve, SelfIntersecton)
    {
    bvector<CurveVectorPtr> curves;
    AddSelfIntersectingCurves (curves);
    for (auto curve : curves)
        {
        SaveAndRestoreCheckTransform shifter (20,0,0);
        CurveVectorPtr intersectionsA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr intersectionsB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveCurve::SelfIntersectionsXY (*intersectionsA, *intersectionsB, *curve, nullptr);
        Check::SaveTransformed (*curve);
        SavePartialCurveMarkers (*intersectionsA);
        }
    Check::ClearGeometry ("CurveCurve.SelfIntersection");
    }