//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

/* unused - static int s_noisy = 0;*/

#if defined(BENTLEY_WIN32)
static bool s_readFile = 0;
#endif

void TestTrackingCurveOffset (char const *message, ICurvePrimitiveR curve0, double d, double expectedLength, bool isCurved)
    {
    Check::StartScope (message);
    ICurvePrimitivePtr curve1 = ICurvePrimitive::CreateTrackingCurveXY (&curve0, d);
    for (double f : bvector<double> {0,0.2, 0.5, 0.9})
        {
        DRay3d xyz0 = curve0.FractionToPointAndUnitTangent (f);
        DRay3d xyz1 = curve1->FractionToPointAndUnitTangent (f);
        Check::Near (fabs (d), xyz0.origin.Distance (xyz1.origin), "Offset Curve point");
        Check::Near (xyz0.direction, xyz1.direction, "Offset curve unit tangent");
        }


    auto options = IFacetOptions::CreateForCurves ();
    static double s_defaultMaxEdgeLength = 4.0;
    options->SetMaxEdgeLength (s_defaultMaxEdgeLength);
    bvector<DPoint3d> stroke0, stroke1, stroke1A;
    curve0.AddStrokes (stroke0, *options);
    curve1->AddStrokes (stroke1, *options);
    auto strokeLengthSum1 = PolylineOps::SumSegmentLengths (stroke1);

    static double s_strokeLengthFraction = 0.45;
    //Check::Size (stroke0.size (), stroke1.size (), "Match strokes between base and offset");

    options->SetMaxEdgeLength (s_strokeLengthFraction * strokeLengthSum1.Max ());
    //Check::True (strokeLengthSum.Max () < 1.42 * s_defaultMaxEdgeLength);   // allow some fluff in max edge length!!!
    curve1->AddStrokes (stroke1A, *options);    // should have more points !!!
    double length1;
    curve1->Length (length1);


    auto strokeLengthSum1A = PolylineOps::SumSegmentLengths (stroke1A);
    double strokeLength1 = strokeLengthSum1.Sum ();
    double strokeLength1A = strokeLengthSum1A.Sum ();
    static double s_lengthFactor = 1.0 + Angle::SmallAngle ();
    if (Check::True (curve1->Length (length1), "Evaluate length"))
        {
        if (expectedLength > 0.0)
            Check::Near (expectedLength, length1, "Offset length");

        Check::True (strokeLength1 <= length1 * s_lengthFactor, "strokeLength < length");
        Check::True (strokeLength1A <= length1 * s_lengthFactor, "strokeLength < length");
        if (isCurved)
            {
            // finer stroking approaches length from below ..
            Check::True (strokeLength1 < strokeLength1A, "tighter stroke length");
            Check::True (strokeLength1A < length1, "strokeLength < length");
            }
        }
    
    Check::EndScope ();

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive, Tracking)
    {
    double r0 = 10.0;
    double dr = 1.0;
    double radians = Angle::DegreesToRadians (90.0);
    auto curve0 = ICurvePrimitive::CreateArc
                    (
                    DEllipse3d::From (0,0,0, r0,0,0, 0,r0,0, 0.0, radians)
                    );
    TestTrackingCurveOffset ("Arc offset outside", *curve0, dr, radians * (r0 + dr), true);
    TestTrackingCurveOffset ("Arc offset inside", *curve0, -dr, radians * (r0 - dr), true);

    auto curve1 = ICurvePrimitive::CreateLine
                    (
                    DSegment3d::From (0,0,0, r0,0,0)
                    );
    TestTrackingCurveOffset ("Line offset outside", *curve1, dr, r0, false);
    TestTrackingCurveOffset ("Line offset inside", *curve1, -dr, r0, false);

    auto curve2 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
                (
                DSpiral2dBase::TransitionType_Clothoid,
                0.0, 0.0,
                100.0, 500.0,
                Transform::FromIdentity (),
                0.0, 1.0
                );
    TestTrackingCurveOffset ("Spiral right", *curve2, dr, 0.0, true);
    
    }
    
    
