//
//
#include "testHarness.h"
#include <Geom/BinaryRangeHeap.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

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

TEST (CurveOffsetClosed, Bspline)
    {
    bvector<DPoint3d> pole0
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (-1,1,0),
        DPoint3d::From (-1,3,0),
        DPoint3d::From (2,3, 0),
        DPoint3d::From (1,0,0)
        };

    MSBsplineCurvePtr bcurve0 = MSBsplineCurve::CreateFromPolesAndOrder (pole0, nullptr, nullptr, 4, false, true);
    ICurvePrimitivePtr curve = ICurvePrimitive::CreateBsplineCurve (bcurve0);
    Check::SaveTransformed (*curve);

    CurveVectorPtr centerLine = CurveVector::Create (curve);

    CurveVectorPtr left = centerLine->CloneOffsetCurvesXY (CurveOffsetOptions (0.5));
    CurveVectorPtr right = centerLine->CloneOffsetCurvesXY (CurveOffsetOptions (-0.5));

    Check::SaveTransformed (*left);
    Check::SaveTransformed (*right);

    Check::Shift (DVec3d::From (5, 0, 0));

    Check::True (right->ReverseCurvesInPlace ());

    DPoint3d leftStart, leftEnd, rightStart, rightEnd;
    Check::True (left->GetStartEnd (leftStart, leftEnd));
    Check::True (right->GetStartEnd (rightStart, rightEnd));

    CurveVectorPtr result = left->Clone ();
    result->Add (ICurvePrimitive::CreateLine (leftEnd, rightStart));
    for (ICurvePrimitivePtr curve : *right)
        result->Add (curve);
    result->Add (ICurvePrimitive::CreateLine (rightEnd, leftStart));

    Check::SaveTransformed (*result);

    Check::ClearGeometry ("CurveOffsetClosed.Bspline");
    }

CurveVectorPtr RebarCurve ()
    {
    CurveVectorPtr curve = CurveVector::Create (
        ICurvePrimitive::CreateLine (
            DPoint3d::From (-21908.999, 4111.625, 0),
            DPoint3d::From (-22956.749, 4111.625, 0)
        )
    );

    //top left corner
    curve->Add (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromArcCenterStartEnd (
                DPoint3d::From (-22956.749, 3714.623, 0), //center
                DPoint3d::From (-22956.749, 4111.625, 0), //start
                DPoint3d::From (-23353.751, 3714.623, 0) //end
            )
        )
    );

    curve->Add (
        ICurvePrimitive::CreateLine (
            DPoint3d::From (-23353.751, 3714.623, 0),
            DPoint3d::From (-23353.751, -3587.75, 0)
        )
    );

    //lower left corner
    curve->Add (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromArcCenterStartEnd (
                DPoint3d::From (-22956.876, -3587.75, 0), //center
                DPoint3d::From (-23353.751, -3587.75, 0),
                DPoint3d::From (-22956.876, -3984.625, 0)
            )
        )
    );

    curve->Add (
        ICurvePrimitive::CreateLine (
            DPoint3d::From (-22956.876, -3984.625, 0),
            DPoint3d::From (-18575.376, -3984.625, 0)
        )
    );

    //lower right corner
    curve->Add (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromArcCenterStartEnd (
                DPoint3d::From (-18575.376, -3587.750, 0), //center
                DPoint3d::From (-18575.376, -3984.625, 0),
                DPoint3d::From (-18178.501, -3587.750, 0)
            )
        )
    );

    curve->Add (
        ICurvePrimitive::CreateLine (
            DPoint3d::From (-18178.501, -3587.75, 0),
            DPoint3d::From (-18178.501, 3714.750, 0)
        )
    );

    //upper right corner
    curve->Add (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromArcCenterStartEnd (
                DPoint3d::From (-18575.376, 3714.750, 0), //center
                DPoint3d::From (-18178.501, 3714.750, 0),
                DPoint3d::From (-18575.376, 4111.625, 0)
            )
        )
    );

    curve->Add (
        ICurvePrimitive::CreateLine (
            DPoint3d::From (-18575.376, 4111.625, 0),
            DPoint3d::From (-22956.749, 4111.625, 0)
        )
    );

    //upper left corner, shorter arc (angle < pi / 2)
    curve->Add (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromArcCenterStartEnd (
                DPoint3d::From (-22956.749, 3714.623, 0),
                DPoint3d::From (-22956.749, 4111.625, 0),
                DPoint3d::From (-23323.531, 3866.549, 0)
            )
        )
    );

    curve->Add (
        ICurvePrimitive::CreateLine (
            DPoint3d::From (-23323.531, 3866.549, 0),
            DPoint3d::From (-23323.531, 3866.549, 0)
        )
    );

    curve->Add (
        ICurvePrimitive::CreateArc (
            DEllipse3d::FromArcCenterStartEnd (
                DPoint3d::From (-22956.749, 3714.623, 0), //center
                DPoint3d::From (-23323.531, 3866.549, 0),
                DPoint3d::From (-23237.471, 3433.9, 0)
            )
        )
    );

    curve->Add (
        ICurvePrimitive::CreateLine (
            DPoint3d::From (-23237.471, 3433.900, 0),
            DPoint3d::From (-22586.403, 2782.832, 0)
        )
    );

    return curve;
    }

BeFileStatus ReadEntireFile (BeFile & file, bvector<Byte>& buffer)
    {
    buffer.clear();

    uint64_t fileSize;
    BeFileStatus stat = file.GetSize (fileSize);

    if (BeFileStatus::Success != stat)
        return  stat;

    buffer.resize(static_cast <uint32_t> (fileSize));
    return file.Read(buffer.data(), NULL, static_cast <uint32_t> (fileSize));
    }

 bool ReadAsString (Utf8String &string, WCharCP filename)
    {
    string.clear ();
    BeFile file;
    if (BeFileStatus::Success == file.Open (filename, BeFileAccess::Read))
        {
        bvector<Byte> bytes;
        if (BeFileStatus::Success == ReadEntireFile (file, bytes))
            {
            for (auto b : bytes)
                string.push_back (b);
            return true;
            }
        }
    return false;
    }

CurveVectorPtr FirstCurveInJson (BeFileNameCR path)
    {
    Utf8String pathStr;
    if (!ReadAsString (pathStr, path.c_str ()))
        return nullptr;

    bvector<IGeometryPtr> allGeometry;
    if (BentleyGeometryJson::TryJsonStringToGeometry (pathStr, allGeometry) && !allGeometry.empty ())
        return allGeometry[0]->GetAsCurveVector ();
    return nullptr;
    }

CurveVectorPtr FirstCurveInJson (WCharCP path)
    {
    return
        FirstCurveInJson (
            BeFileName (
                path
            )
        );
    }

//Interfaces
struct ICurveProcessor : ICurvePrimitiveProcessor, RefCountedBase
    {};
using ICurveProcessorPtr = RefCountedPtr<ICurveProcessor>;

struct ICurveVector : RefCountedBase
    {
    virtual CurveVectorPtr Curve () = 0;
    };
using ICurveVectorPtr = RefCountedPtr<ICurveVector>;

struct CurveVectorFunc final : ICurveVector
    {
private:
    ICurveVectorPtr m_origin;
    std::function<CurveVectorPtr (CurveVectorPtr)> m_curve;

public:
    CurveVectorFunc (ICurveVectorPtr origin, std::function<CurveVectorPtr (CurveVectorPtr)> const& curve)
        :m_origin (origin), m_curve (curve)
        {}

    CurveVectorPtr Curve () override
        {
        return m_curve (m_origin->Curve ());
        }
    };

struct CurveVectorDefault final : ICurveVector
    {
private:
    CurveVectorPtr m_curve;

public:
    CurveVectorDefault (CurveVectorPtr curve)
        :m_curve (curve)
        {}

    CurveVectorPtr Curve () override
        {
        return m_curve;
        }
    };

struct CurveVectorCapped final : ICurveVector
    {
private:
    ICurveVectorPtr m_left;
    ICurveVectorPtr m_right;

public:
    CurveVectorCapped (ICurveVectorPtr left, ICurveVectorPtr right)
        :m_left (left), m_right (right)
        {}

    CurveVectorPtr Curve () override
        {
        DPoint3d leftStart, leftEnd, rightStart, rightEnd;
        CurveVectorPtr left = m_left->Curve ();
        CurveVectorPtr right = m_right->Curve ();
        if (!left.IsValid() || !right.IsValid())
            return nullptr;

        CurveVectorPtr offset = left->CloneReversed ();
        if (!offset.IsValid ())
            return nullptr;

        if (!offset->GetStartEnd (leftStart, leftEnd) || !right->GetStartEnd (rightStart, rightEnd))
            return nullptr;

        offset->Add (ICurvePrimitive::CreateLine (DSegment3d::From (leftEnd, rightStart)));
        for (ICurvePrimitivePtr curve : *right)
            {
            offset->Add (curve);
            }
        offset->Add (ICurvePrimitive::CreateLine (DSegment3d::From (rightEnd, leftStart)));
        return offset;
        }
    };

struct ClonedOffset final : ICurveVector
    {
private:
    ICurveVectorPtr m_origin;
    CurveOffsetOptions const m_opts;

public:
    ClonedOffset (CurveVectorPtr origin, double const& dist)
        :
        ClonedOffset (
            new CurveVectorDefault (
                origin
            ),
            CurveOffsetOptions (
                dist
            )
        )
        {}
    ClonedOffset (ICurveVectorPtr origin, CurveOffsetOptions const& opts)
        :m_origin (origin), m_opts (opts)
        {}

    CurveVectorPtr Curve () override
        {
        CurveVectorPtr curve = m_origin->Curve ();
        if (!curve.IsValid ())
            return curve;
        return curve->CloneOffsetCurvesXY (m_opts);
        }
    };

CurveVectorPtr Saved (CurveVectorPtr curve)
    {
    if (curve.IsValid ())
        Check::SaveTransformed (*curve);
    return curve;
    }

CurveVectorPtr SavedWithShift (CurveVectorPtr curve)
    {
    CurveVectorPtr saved = Saved (curve);
    DRange3d range;
    if (saved.IsValid () && saved->GetRange (range))
        Check::Shift (range.MaxAbs () / 10.0, 0, 0);
    return saved;
    }

struct DoubleSidedOffset final : ICurveVector
    {
private:
    ICurveVectorPtr m_origin;
    double const m_dist;

public:
    DoubleSidedOffset (CurveVectorPtr origin, double const& dist)
        :DoubleSidedOffset (
            new CurveVectorDefault (
                origin
            ),
            dist
        )
        {}
    DoubleSidedOffset (ICurveVectorPtr origin, double const& dist)
        :m_origin (origin), m_dist (dist)
        {}
    CurveVectorPtr Curve () override
        {
        ICurveVectorPtr left =
            new ClonedOffset (
                m_origin->Curve (),
                -m_dist
            );

        /*ICurveVectorPtr right =
            new ClonedOffset (
                m_origin->Curve (),
                -m_dist
            );*/

        ICurveVectorPtr right =
            new ClonedOffset (
                left->Curve(),
                2.0 * m_dist
            );

        //Saved (left->Curve ());
        //Saved (right->Curve ());

        return CurveVectorCapped (
            left,
            right
        ).Curve ();
        }
    };

// TEST (CurveOffset, RebarJson)
//     {
//     Saved (
//         CurveVector::ThickenXYPathToArea(
//             Saved (
//                 FirstCurveInJson (
//                     BeFileName (
//                         L"D:\\tmp\\original.dgnjs"
//                     )
//                 )
//             ),
//             80, 80
//         )
//     );

//     Check::ClearGeometry ("Offset.RebarJson");
//     }

TEST (CurveOffset, IsFlat)
    {
    CurveVectorPtr offset = CurveVector::ThickenXYPathToArea (
        CurveVector::Create (
            ICurvePrimitive::CreateLine (DPoint3d::FromZero (), DPoint3d::From (100, 0, 0)),
            CurveVector::BOUNDARY_TYPE_Open
        ),
        80, 80
    );

    for (ICurvePrimitivePtr& primitive : *offset)
        Check::True (primitive->GetCurvePrimitiveType () != ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_CurveVector);
    }

TEST (CurveOffset, IsNone)
    {
    CurveVectorPtr offset = CurveVector::ThickenXYPathToArea(
        CurveVector::Create (
            ICurvePrimitive::CreateLine (DPoint3d::FromZero (), DPoint3d::From (100, 0, 0)),
            CurveVector::BOUNDARY_TYPE_Open
        ),
        80, 80
    );

    Check::True (offset->GetBoundaryType () == CurveVector::BOUNDARY_TYPE_Outer);
    }

TEST (CurveOffset, RebarSimple)
    {
    Saved (
        CurveVector::ThickenXYPathToArea(
            CurveVector::Create (
                ICurvePrimitive::CreateLine (DPoint3d::FromZero (), DPoint3d::From (100, 0, 0)),
                CurveVector::BOUNDARY_TYPE_Open
            ),
            80, 80
        )
    );

    Check::ClearGeometry ("Offset.RebarSimple");
    }

TEST (CurveOffset, RebarHard)
    {
    Saved (
        CurveVector::ThickenXYPathToArea(
            RebarCurve (),
            80, 80
        )
    );

    Check::ClearGeometry ("Offset.RebarHard");
    }

struct OffsetOnebyOne final : ICurveProcessor, ICurveVector
    {
private:
    ICurveVectorPtr m_origin;
    std::function<CurveVectorPtr (CurveVectorPtr)> m_offset;
    CurveVectorPtr m_result;

    void Add (CurveVectorPtr curve)
        {
        if (!m_result.IsValid ())
            m_result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
        m_result->Add (curve);
        }

    void _ProcessDefault (ICurvePrimitiveCR prim, DSegment1dCP) override
        {
        CurveVectorPtr curve = CurveVector::Create (
            prim.Clone (),
            CurveVector::BOUNDARY_TYPE_Outer
        );

        if (!curve.IsValid ())
            return;

        CurveVectorPtr offset = m_offset (curve);

        if (offset.IsValid())
            Add (offset);
        }

public:
    OffsetOnebyOne (CurveVectorPtr origin, std::function<CurveVectorPtr (CurveVectorPtr)> const& offset)
        :
        OffsetOnebyOne (
            new CurveVectorDefault (
                origin
            ),
            offset
        )
        {}
    OffsetOnebyOne (ICurveVectorPtr origin, std::function<CurveVectorPtr (CurveVectorPtr)> const& offset)
        :m_origin (origin), m_offset (offset)
        {}

    CurveVectorPtr Curve () override
        {
        _ProcessCurveVector (*m_origin->Curve (), nullptr);
        return m_result;
        }
    };

TEST (CurveOffset, RebarReduceCCW)
    {
    std::function<CurveVectorPtr (CurveVectorPtr, CurveVector::BoundaryType)> changeType = 
        [] (CurveVectorPtr curve, CurveVector::BoundaryType boundaryType)
        {
        curve->SetBoundaryType (boundaryType);
        return curve;
        };

    Saved (
        CurveVector::ReduceToCCWAreas (
            *changeType (
                Saved (
                    RebarCurve ()
                ),
                CurveVector::BoundaryType::BOUNDARY_TYPE_Outer
            )
        )
    );

    Check::ClearGeometry ("Offset.RebarReduceCCW");
    }

TEST (CurveOffset, RebarDoubleSide)
    {
    Saved (
        DoubleSidedOffset (
            RebarCurve (),
            80
        ).Curve ()
    );

    Check::ClearGeometry ("DoubleOffset");
    }

TEST (CurveOffset, RebarUnion)
    {
    std::function<CurveVectorPtr (CurveVectorPtr)> offset =
        [] (CurveVectorPtr curve)
        {
        return
            Saved (
                DoubleSidedOffset (curve, 80).Curve ()
            );
        };

    Saved (
        OffsetOnebyOne (
            RebarCurve (),
            offset
        ).Curve()
    );

    Check::ClearGeometry ("UnionAll");
    }
