/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Geom/BinaryRangeHeap.h>
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL


// static int s_noisy = 0;    
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
    DSegment3d segment = segment0;
    for (int i = 0; i < numSegment; i++)
        {
        cv.Add (ICurvePrimitive::CreateLine (segment));
        segment.point[0].Add (shift);
        segment.point[1].Add (shift);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,RangeTree)
    {
    CurveVectorPtr cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    AddSegments (*cv, 5, DSegment3d::From (DPoint3d::From (0,0,0), DPoint3d::From (1,1,0)), DVec3d::From (2,0,0));
    CurveVectorRangeData cvRanges;
    cvRanges.Install (cv);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (CurveCurve, TransverseIntersection)
    {
    for (int numB = 1; numB < 6; numB += 2)
        {
        SaveAndRestoreCheckTransform shifter(0, 40, 0);

        auto waveA = CurveVector::CreateRectangle (0,0, numB * 8,6, 0.0);
        auto waveB = SquareWavePolygon (numB, 1.0, 2.0, 3.0, -1.0, 2.0, true, -3.0);
        Transform transform = Transform::FromRowValues
            (
            1,0,0, -1,
            0,-0.2,-1, 1,
            0,1,0, -1
            );
        waveA->TransformInPlace (transform);
        bvector<DSegment3d> segments;
        Check::SaveTransformed (waveA);
        Check::SaveTransformed (waveB);
        Check::Shift (0,10,0);
        Check::SaveTransformed(waveB);

        Check::True (CurveCurve::TransverseRegionIntersectionSegments (*waveA, *waveB, segments));
        for (auto &segment : segments)
            Check::SaveTransformed (segment);
        auto cvSegments = CurveVector::Create (segments);
        // CHECK:  All segment midpoints are IN
        for (DSegment3d segment : segments)
            {
            DPoint3d midPoint = segment.FractionToPoint (0.5);
            DPoint3d pointA, pointB;
            Check::SaveTransformedMarker (midPoint, 0.02);
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
            Check::SaveTransformedMarker(midPoint, 0.02);
            Check::True
                (inA == CurveVector::INOUT_On || inB == CurveVector::INOUT_On, "known exterior point");
            }
        }
    Check::ClearGeometry ("CurveCurve.TransverseIntersection");
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
* @bsimethod
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
    for (ICurvePrimitivePtr curve1 : *right)
        result->Add (curve1);
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
CurveVectorPtr IndexedCurveInJson(BeFileNameCR path, size_t index)
    {
    Utf8String pathStr;
    if (!ReadAsString(pathStr, path.c_str()))
        return nullptr;

    bvector<IGeometryPtr> allGeometry;
    if (BentleyGeometryJson::TryJsonStringToGeometry(pathStr, allGeometry) && index < allGeometry.size())
        return allGeometry[index]->GetAsCurveVector();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CCI,RangeCheck)
    {
    auto v = Check::SetMaxVolume (1000);
    auto arc = ICurvePrimitive::CreateArc(DEllipse3d::From(
        5, 5, 0,
        5, 0, 0,
        0, 5, 0,
        0, Angle::TwoPi()));
    double lineFraction = 0.323423;
    for (DVec3d vectorA :   // to become the full length vector along the segment.
        {
        DVec3d::From (0,1,0),
        DVec3d::From (1,2,4),
        DVec3d::From (1,0,0),
        DVec3d::From (0,0,1),
        DVec3d::From (2,1,0.0001)})
        {
        Check::StartScope ("Line Vector", vectorA);
        for (double arcFraction :  {0.0, 1.0, 0.5, 0.2, 0.6})
            {
            Check::StartScope ("arcFraction", arcFraction);
            DPoint3d commonPoint;
            arc->FractionToPoint (arcFraction, commonPoint);
            auto line = ICurvePrimitive::CreateLine (DSegment3d::From (
                    commonPoint - (1.0 - lineFraction) * vectorA,
                    commonPoint + lineFraction * vectorA));
            auto pointsOnA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
            auto pointsOnB = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
            CurveCurve::CloseApproach (*pointsOnA, *pointsOnB, line.get (), arc.get (),
                        1.0e-8);
            if (Check::True(pointsOnA->size() > 0, "At least one point reported for CC approach"))
                {
                uint32_t numMatch = 0;
                for (uint32_t i = 0; i < pointsOnA->size (); i++)
                    {
                    DPoint3d xyz;
                    auto p = pointsOnA->at (i);
                    p->GetStartPoint (xyz);
                    if (xyz.AlmostEqual(commonPoint))
                        numMatch++;
                    }
                Check::True(numMatch > 0, "Expected intersection point found by closest approach");
                }
/*
auto pointsOnAXY = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
            auto pointsOnBXY = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

            CurveCurve::IntersectionsXY (*pointsOnAXY, *pointsOnBXY, line.get (), arc.get (), nullptr, false);
            printf("\n     CurveCurve::IntersectionsXY counts %d %d \n", (int)pointsOnAXY->size(), (int)pointsOnBXY->size());
*/
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    Check::SetMaxVolume (v);
    }
#ifdef CompileCCADebugTracking
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
GEOMDLLIMPEXP void SetCCADebug(int value);
GEOMDLLIMPEXP void GrabCCADebugDetails(bvector<bvector<CurveLocationDetailPair>> &data);
END_BENTLEY_GEOMETRY_NAMESPACE
void SaveTransformed(bvector<CurveLocationDetailPair> const &data)
    {
    auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    for (auto &pair : data)
        cv->push_back(ICurvePrimitive::CreateLine(DSegment3d::From(pair.detailA.point, pair.detailB.point)));
    Check::SaveTransformed(*cv);
    }
void SaveTransformed(bvector<bvector<CurveLocationDetailPair>> const &data)
    {
    for (auto &d : data)
        SaveTransformed (d);
    }
#endif
TEST(CloseApproach,Bsplines)
    {
    bvector<DPoint3d> pointA {
        {3815316856.9794154, -15026759129.438129, 0.0},
        {3815369311.7554502, -15026768363.876534, 0.0},
        {3815473830.0117874, -15026861966.106886, 0.0},
        {3815488771.3555403, -15026913088.84770, 0.0},
        {3815497475.3889918, -15026942870.241766, 0.0},
        {3815399027.1612139, -15026966235.007328, 0.0},
        {3815387793.1110296, -15027001841.842773, 0.0},
        {3815377930.4541130, -15027033101.984585, 0.0},
        {3815401740.9556241, -15027090486.904747, 0.0},
        {3815425481.2342372, -15027113689.779469, 0.0},
        {3815442689.8381410, -15027130508.835682, 0.0},
        {3815491676.5367379, -15027106349.188074, 0.0},
        {3815510639.7585812, -15027121907.635582, 0.0},
        {3815533323.1736822, -15027140518.329802, 0.0},
        {3815558827.1268096, -15027192099.177509, 0.0},
        {3815550421.1450701, -15027216197.204651, 0.0},
        {3815541836.4731503, -15027240807.495672, 0.0},
        {3815468427.8065457, -15027238561.451620, 0.0},
        {3815459667.7976031, -15027268032.590067, 0.0},
        {3815439740.6395755, -15027335073.164673, 0.0},
        {3815493686.2618842, -15027449759.637545, 0.0},
        {3815464359.6441588, -15027505732.343809, 0.0},
        {3815445188.4106684, -15027542322.510153, 0.0},
        {3815349002.3690953, -15027567933.325439, 0.0},
        {3815314174.2439556, -15027545721.207893, 0.0}
        };
    bvector<double> knotA{
        0.0,0.0,0.0,0.0,0.19744579268019086,0.19744579268019086,0.19744579268019086,0.31246722668159571,
        0.31246722668159571,0.31246722668159571,0.4134474540254810,0.4134474540254810,0.4134474540254810,0.48664494099677769,0.48664494099677769,0.48664494099677769,
        0.57420226617402070,0.57420226617402070,0.57420226617402070,0.66362084143597977,0.66362084143597977,0.66362084143597977,0.86702909039404663,0.86702909039404663,
        0.86702909039404663,1.0,
        1.0, 1.0, 1.0};

    bvector<DPoint3d> pointB {
         {3815247549.1124158,-15027576712.271242,0.0},
         {3815223497.8392797,-15027567860.907219,0.0},
         {3815208071.6604347,-15027401697.379820,0.0},
         {3815230082.4610553,-15027388569.735683,0.0},
         {3815251583.5478344,-15027375746.094206,0.0},
         {3815341003.3158565,-15027500575.705017,0.0},
         {3815378084.7746148,-15027498858.414398,0.0},
         {3815403205.1765943,-15027497695.055887,0.0},
         {3815426055.8244605,-15027413908.506632,0.0},
         {3815416688.0432682,-15027379927.788296,0.0},
         {3815406719.2475233,-15027343766.947922,0.0},
         {3815322966.0562596,-15027319574.361773,0.0},
         {3815320075.0438032,-15027288433.738268,0.0},
         {3815317562.4299130,-15027261369.045761,0.0},
         {3815412333.8053026,-15027216538.117579,0.0},
         {3815400477.1642280,-15027205311.840261,0.0},
         {3815383305.0832152,-15027189052.720808,0.0},
         {3815270796.7155857,-15027228513.081705,0.0},
         {3815232988.8775411,-15027205977.547960,0.0},
         {3815205981.9367952,-15027189879.938152,0.0},
         {3815192838.6999912,-15027120572.433996,0.0},
         {3815206032.8278561,-15027089412.409607,0.0},
         {3815222358.6828012,-15027050856.31530,0.0},
         {3815296114.3040614,-15027035167.100290,0.0},
         {3815321548.8259711,-15026996829.191872,0.0},
         {3815339664.5545816,-15026969523.031101,0.0},
         {3815387501.2845893,-15026953776.127850,0.0},
         {3815336683.5794177,-15026892480.202038,0.0},
         {3815313605.5191088,-15026847624.959509,0.0},
         {3815208852.0491328,-15026817556.245234,0.0},
         {3815209485.7121844,-15026779019.805809,0.0},
         {3815210159.2816954,-15026738056.441160,0.0},
         {3815297337.4553547,-15026658694.623476,0.0},
         {3815340605.2771049,-15026653980.789814,0.0},
         {3815374335.0236430,-15026650306.086346,0.0},
         {3815401731.0313845,-15026731255.958553,0.0},
         {3815440478.4170504,-15026753854.194414,0.0},
         {3815467776.3164954,-15026769774.865179,0.0},
         {3815507843.6381702,-15026776169.848623,0.0},
         {3815538741.1324377,-15026769537.509691,0.0}
        };
    bvector<double> knotB {0.0, 0.0, 0.0, 0.0, 0.10049212758974138, 0.10049212758974138, 0.10049212758974138, 0.19865711454932439,
        0.19865711454932439, 0.19865711454932439, 0.26515784066088627, 0.26515784066088627, 0.26515784066088627, 0.33592509513647956, 0.33592509513647956, 0.33592509513647956,
        0.39742977186801465, 0.39742977186801465, 0.39742977186801465, 0.48650755408385132, 0.48650755408385132, 0.48650755408385132, 0.55013770355770197, 0.55013770355770197,
        0.55013770355770197, 0.62887095528027137, 0.62887095528027137, 0.62887095528027137, 0.68494868404143305, 0.68494868404143305, 0.68494868404143305, 0.77559991814175677,
        0.77559991814175677, 0.77559991814175677, 0.87196013236258874, 0.87196013236258874, 0.87196013236258874, 0.94707844131751084, 0.94707844131751084, 0.94707844131751084,
        1.0, 1.0, 1.0, 1.0};

    double scale = 1.0 / 10000.0;
    for (auto &p : pointA)
        p.Scale (scale);
    for (auto &p : pointB)
        p.Scale(scale);
    auto curveA = ICurvePrimitive::CreateBsplineCurve(
        MSBsplineCurve::CreateFromPolesAndOrder (pointA, nullptr, &knotA, 4, false, false));
    auto curveB = ICurvePrimitive::CreateBsplineCurve(
        MSBsplineCurve::CreateFromPolesAndOrder (pointB, nullptr, &knotB, 4, false, false));
    CurveLocationDetail pointOnA, pointOnB;
#ifdef CompileCCADebugTracking
    SetCCADebug(1);
#endif
    Check::True (CurveCurve::ClosestApproach (pointOnA, pointOnB, curveA.get (), curveB.get ()));
#ifdef CompileCCADebugTracking
    bvector<bvector<CurveLocationDetailPair>> searchData;
    GrabCCADebugDetails (searchData);
    SaveTransformed (searchData);
    SetCCADebug(0);
#endif
    Check::SaveTransformed (*curveA);
    Check::SaveTransformed (*curveB);
    Check::SaveTransformed (DSegment3d::From (pointOnA.point, pointOnB.point));

    auto strokeOptions = IFacetOptions::CreateForCurves();
    strokeOptions->SetAngleTolerance(0.1);
    bvector<DPoint3d> strokeA, strokeB;
    curveA->AddStrokes (strokeA, *strokeOptions);
    curveB->AddStrokes(strokeB, *strokeOptions);
    auto range = DRange3d::From (strokeA);
    CurveLocationDetail locationA, locationB;
    if (PolylineOps::ClosestApproach(strokeA, strokeB, locationA, locationB))
        {
        Check::Shift (range.XLength (), 0, 0);
        Check::SaveTransformed(strokeA);
        Check::SaveTransformed(strokeB);
        Check::SaveTransformed(DSegment3d::From(locationA.point, locationB.point));
        }
    Check::ClearGeometry("CloseApproach.Bsplines");
    }
TEST(CloseApproach, BsplineWithInflection)
    {
    double a = 0.425;  
    bvector<DPoint3d> pointA{
        {-4.0, -a, 0},
        {-1.0,  a, 0},
        { 1.0, -a, 0},
        { 5.0,  a, 0},
        };

    for (double y1 : { 0.25, 0.5, 1.0, 2.0})
        {
        SaveAndRestoreCheckTransform shifter1(0, 15, 0);
        for (double e : {-0.5, -0.2, 0.0, 0.3, 0.5, 1.0})
            {
            SaveAndRestoreCheckTransform shifter2 (15,0,0);
            double b = 3.0;
            bvector<DPoint3d> pointB{
                {-2.0 + e, y1+b, 0},
                {-1.0 + e,  y1, 0},
                { 1.0+ e,  y1, 0},
                { 2.0+ e,  y1+b, 0},

                };
            auto curveA = ICurvePrimitive::CreateBsplineCurve(
                MSBsplineCurve::CreateFromPolesAndOrder(pointA, nullptr, nullptr, 4, false, false));
            auto curveB = ICurvePrimitive::CreateBsplineCurve(
                MSBsplineCurve::CreateFromPolesAndOrder(pointB, nullptr, nullptr, 4, false, false));
            CurveLocationDetail pointOnA, pointOnB;
            Check::True(CurveCurve::ClosestApproach(pointOnA, pointOnB, curveA.get(), curveB.get()));
            Check::SaveTransformed(*curveA);
            Check::SaveTransformed(*curveB);
            Check::SaveTransformed(DSegment3d::From(pointOnA.point, pointOnB.point));

            auto strokeOptions = IFacetOptions::CreateForCurves();
            strokeOptions->SetAngleTolerance(0.1);
            bvector<DPoint3d> strokeA, strokeB;
            curveA->AddStrokes(strokeA, *strokeOptions);
            curveB->AddStrokes(strokeB, *strokeOptions);
            auto range = DRange3d::From(strokeA);
            CurveLocationDetail locationA, locationB;
            if (PolylineOps::ClosestApproach(strokeA, strokeB, locationA, locationB))
                {
                Check::Shift(range.XLength(), 0, 0);
                Check::SaveTransformed(strokeA);
                Check::SaveTransformed(strokeB);
                Check::SaveTransformed(DSegment3d::From(locationA.point, locationB.point));
                }
            }
        }
    Check::ClearGeometry("CloseApproach.BsplineWithInflection");
    }

TEST(CloseApproach, LineArcCombinations)
    {
    double a = 2.0;
    double b = 3.0 * a;
    auto arcA = ICurvePrimitive::CreateArc (DEllipse3d::From (0,0,0,   a,0,0,  0,a,0, -0.1, 3.5));
    auto trace = DEllipse3d::From (1,1,0,   b,0,0,  0,b,0,  0, Angle::TwoPi ());    // just to get points for segments.
    double df = 0.2;
    // close approach of arc to various lines.
    {
    SaveAndRestoreCheckTransform shifter1(0, 20, 0);
    for (double f0 = 0.0; f0 < 1.0; f0 += 0.09)
        {
        SaveAndRestoreCheckTransform shifter2 (10, 0, 0);
        DPoint3d xyz0 = trace.FractionToPoint (f0);
        DPoint3d xyz1 = trace.FractionToPoint(f0 + df);
        auto segmentB = ICurvePrimitive::CreateLine (DSegment3d::From (xyz0, xyz1));
        CurveLocationDetail pointOnA, pointOnB;
        Check::True(CurveCurve::ClosestApproach(pointOnA, pointOnB, arcA.get(), segmentB.get()));
        Check::SaveTransformed (*arcA);
        Check::SaveTransformed (*segmentB);
        Check::SaveTransformed(DSegment3d::From(pointOnA.point, pointOnB.point));
        }
    }
    // and all those lines approach to the chord of the arc

    DPoint3d xyzA0, xyzA1;
    arcA->FractionToPoint (0.0, xyzA0);
    arcA->FractionToPoint(1.0, xyzA1);
    auto lineA = ICurvePrimitive::CreateLine (DSegment3d::From (xyzA0, xyzA1));
    for (double f0 = 0.0; f0 < 1.0; f0 += 0.09)
        {
        SaveAndRestoreCheckTransform shifter(10, 0, 0);
        DPoint3d xyz0 = trace.FractionToPoint(f0);
        DPoint3d xyz1 = trace.FractionToPoint(f0 + df);
        auto segmentB = ICurvePrimitive::CreateLine(DSegment3d::From(xyz0, xyz1));
        CurveLocationDetail pointOnA, pointOnB;
        Check::True(CurveCurve::ClosestApproach(pointOnA, pointOnB, lineA.get(), segmentB.get()));
        Check::SaveTransformed(*lineA);
        Check::SaveTransformed(*segmentB);
        Check::SaveTransformed(DSegment3d::From(pointOnA.point, pointOnB.point));
        }

    Check::ClearGeometry("CloseApproach.LineArcCombinations");
    }