//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

/* unused - static int s_noisy = 0;*/
// ASSUME the curve is not significantly wandering . ..
//   * plane through splitFracitonA or splitFractionB will only intersect once.

void ExerciseVirtualsA (ICurvePrimitivePtr &curve, double splitFractionA, double splitFractionB)
    {
    auto clone = curve->Clone ();
    if (!Check::True (clone.IsValid (), "Clone.IsValid ()"))
        return;
    Check::True (clone->IsSameStructure (*curve), "Clone has same structure");
    // Check that point and derivatives from various fraction evaluators agree ..
    DPoint3d pointA0, pointB0;
    curve->FractionToPoint (splitFractionA, pointA0);
    curve->FractionToPoint (splitFractionB, pointB0);
    DPoint3d pointA1, pointB1;
    DVec3d d1A1, d1B1;
    curve->FractionToPoint (splitFractionA, pointA1, d1A1);
    curve->FractionToPoint (splitFractionB, pointB1, d1B1);

    DPoint3d pointA2, pointB2;
    DVec3d d1A2, d2A2, d1B2, d2B2;
    curve->FractionToPoint (splitFractionA, pointA2, d1A2, d2A2);
    curve->FractionToPoint (splitFractionB, pointB2, d1B2, d2B2);

    Check::Near (pointA0, pointA1);
    Check::Near (pointB0, pointB1);

    Check::Near (pointA0, pointA2);
    Check::Near (pointB0, pointB2);

    Check::Near (d1A1, d1A2);
    Check::Near (d1B1, d1B2);

    auto cloneAB = curve->CloneBetweenFractions (splitFractionA, splitFractionB, false  );
    double lengthAB, lengthABClone;
    Check::True (cloneAB->Length (lengthABClone));
    Check::True (curve->SignedDistanceBetweenFractions  (splitFractionA, splitFractionB, lengthAB));
    Check::Near (lengthAB, lengthABClone);

    double fastLengthAB;
    Check::True (cloneAB->FastLength(fastLengthAB));
    Check::LessThanOrEqual (lengthAB, fastLengthAB);


    DPoint3d pointA10, pointB10;
    Check::True (cloneAB->GetStartEnd (pointA10, pointB10));
    Check::Near (pointA0, pointA10);
    Check::Near (pointB0, pointB10);

    DPoint3d pointA11, pointB11;
    DVec3d unitTangentA11, unitTangentB11;
    Check::True (cloneAB->GetStartEnd (pointA11, pointB11, unitTangentA11, unitTangentB11));
    Check::Near (pointA0, pointA10);
    Check::Near (pointB0, pointB10);
    Check::Parallel (unitTangentA11, d1A1);
    Check::Parallel (unitTangentB11, d1B1);
    Check::Near (1.0, unitTangentA11.Magnitude ());
    Check::Near (1.0, unitTangentB11.Magnitude ());

    DRange3d range01, rangeAB;
    Check::True (curve->GetRange (range01));
    Check::True (cloneAB->GetRange (rangeAB));
    Check::True (rangeAB.IsContained (range01));
    }
void ExerciseContentQueries (ICurvePrimitivePtr &curve)
    {
    auto type = curve->GetCurvePrimitiveType ();
    if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        {
        auto segmentP = curve->GetLineCP ();
        Check::True (segmentP != nullptr);
        }
    else if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc)
        {
        auto arcP = curve->GetArcCP ();
        Check::True (arcP != nullptr);
        }
    else if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        {
        auto childP = curve->GetChildCurveVectorCP ();
        auto childPtr = curve->GetChildCurveVectorP (); // bad name -- should be Ptr
        Check::True (childP != nullptr);
        Check::True (childPtr.IsValid ());
        }
    }

void ExerciseVirtualsOffset (ICurvePrimitivePtr &curve)
    {
    double length;
    curve->Length (length);
    double offsetDistance = 0.02 * length;
    CurveOffsetOptions options(offsetDistance);
    auto offset = curve->CloneAsSingleOffsetPrimitiveXY (options);
    if (Check::True (offset.IsValid (), "SingleOffsetPrimitive"))
        {
        DPoint3d pointA0, pointB0, pointA1, pointB1;
        DVec3d   unitA0, unitB0, unitA1, unitB1;
        curve->GetStartEnd (pointA0, pointA1, unitA0, unitA1);
        offset->GetStartEnd (pointB0, pointB1, unitB0, unitB1);
        Check::Near (pointA0.Distance (pointB0), offsetDistance);
        Check::Near (pointA1.Distance (pointB1), offsetDistance);
        Check::Parallel (unitA0, unitB0);
        Check::Parallel (unitA1, unitB1);
        }
    }
void ExerciseAllVirtuals (ICurvePrimitivePtr &curve, double splitFractionA, double splitFractionB)
    {
    ExerciseVirtualsA (curve, splitFractionA, splitFractionB);
    ExerciseVirtualsOffset (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive, ExerciseVirtuals)
    {
    auto c = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3, 5,-1,3));
    ExerciseAllVirtuals (c, 0.25, 0.8);

    }
    
