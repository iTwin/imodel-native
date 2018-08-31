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
    double period;
    if (curve->IsPeriodicFractionSpace (period))
        {
        for (double step : {1.0, 2.0, -1.0})
            {
            double f = 0.23;
            DPoint3d xyz0, xyz1;
            DVec3d tangent0, tangent1;
            curve->FractionToPoint (f, xyz0, tangent0);
            curve->FractionToPoint (f + step * period, xyz1, tangent1);
            Check::Near (xyz0, xyz1);
            Check::Near (tangent0, tangent1);
            }
        }
    }
void ExerciseContentQueries (ICurvePrimitivePtr &curve)
    {
    auto type = curve->GetCurvePrimitiveType ();
    if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        {
        auto dataP = curve->GetLineCP ();
        Check::True (dataP != nullptr);
        }
    else if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc)
        {
        auto dataP = curve->GetArcCP ();
        Check::True (dataP != nullptr);
        }
    else if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        {
        auto childP = curve->GetChildCurveVectorCP ();
        auto childPtr = curve->GetChildCurveVectorP (); // bad name -- should be Ptr
        Check::True (childP != nullptr);
        Check::True (childPtr.IsValid ());
        }
    else if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        {
        auto dataCP = curve->GetLineStringCP ();
        Check::True (dataCP != nullptr);
        auto dataP = curve->GetLineStringCP ();
        Check::True (dataP != nullptr);
        }
    }

void ExerciseVirtualsOffset (ICurvePrimitivePtr &curve)
    {
    double length;
    curve->Length (length);
    double offsetDistance = 0.02 * length;
    CurveOffsetOptions options(offsetDistance);
    auto flattenZ = Transform::From (RotMatrix::FromRowValues (1,0,0,   0,1,0,   0,0,0));
    auto xyCurve = curve->Clone ();
    double angleTol = 0.00001;        // fluffy tol for offsets
    if (Check::True (xyCurve->TransformInPlace (flattenZ)))
        {
        ICurvePrimitivePtr offset = xyCurve->CloneAsSingleOffsetPrimitiveXY (options);

        if (offset.IsValid ())     // ignore failure quietly !!
            {
            DPoint3d pointA0, pointB0, pointA1, pointB1;
            DVec3d   unitA0, unitB0, unitA1, unitB1;
            xyCurve->GetStartEnd (pointA0, pointA1, unitA0, unitA1);
            offset->GetStartEnd (pointB0, pointB1, unitB0, unitB1);
            Check::Near (pointA0.Distance (pointB0), offsetDistance);
            Check::Near (pointA1.Distance (pointB1), offsetDistance);
            auto type = curve->GetCurvePrimitiveType ();
            if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line
                || (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc
                    && xyCurve->GetArcCP ()->IsCircular ())
                )
                {
                // offset are hard .. be really fluffy on angle between . . .
                double angle0 = unitA0.AngleTo(unitB0);
                double angle1 = unitA0.AngleTo(unitB0);
                Check::LessThanOrEqual (angle0, angleTol);
                Check::LessThanOrEqual (angle1, angleTol);
                }
            }
        }
    }
void ExerciseAllVirtuals (ICurvePrimitivePtr &curve, double splitFractionA, double splitFractionB)
    {
    ExerciseVirtualsA (curve, splitFractionA, splitFractionB);
    ExerciseVirtualsOffset (curve);
    ExerciseContentQueries (curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  7/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive, ExerciseVirtuals)
    {
    auto c0 = ICurvePrimitive::CreateLine (DSegment3d::From (1,2,3, 5,-1,3));
    ExerciseAllVirtuals (c0, 0.25, 0.8);
    auto c1 = ICurvePrimitive::CreateArc (DEllipse3d::From (1,1,2, 0,-1,0, 3,0,0, 0.0, 0.3));
    ExerciseAllVirtuals (c1, 0.25, 0.8);
    bvector<DPoint3d> points {
                DPoint3d::From (0,0,0),
                DPoint3d::From (2,0,0),
                DPoint3d::From (4,1,0),
                DPoint3d::From (6,3,0)
                };
    auto c2 = ICurvePrimitive::CreateLineString (points);
    ExerciseAllVirtuals (c2, 0.25, 0.8);

    }
    
