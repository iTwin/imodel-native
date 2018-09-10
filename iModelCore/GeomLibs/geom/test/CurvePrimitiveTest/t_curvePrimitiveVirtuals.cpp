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
    Check::True (clone->IsSameStructureAndGeometry (*curve), "Clone has same structure and geometry");
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
    CurveLocationDetail detailAtoB, detailBtoA;
    Check::True (curve->PointAtSignedDistanceFromFraction (splitFractionA,  lengthAB, false, detailAtoB));
    Check::True (curve->PointAtSignedDistanceFromFraction (splitFractionB,  -lengthAB, false, detailBtoA));
    Check::Near (splitFractionB, detailAtoB.fraction, "PointAtSignedDistance positive");
    Check::Near (splitFractionA, detailBtoA.fraction, "PointAtSignedDistance negative");

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
    
    double fastMaxAbs = curve->FastMaxAbs ();
    double rangeMaxAbs = range01.MaxAbs ();
    Check::LessThanOrEqual (pointA0.MaxAbs (), fastMaxAbs);
    Check::LessThanOrEqual (pointA0.MaxAbs (), rangeMaxAbs);
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
    else if (type == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve)
        {
        auto dataPtr = curve->GetMSBsplineCurvePtr ();
        Check::True (dataPtr.IsValid ());
        MSBsplineCurve bcurve;
        bool hasBCurve = curve->GetMSBsplineCurve (bcurve);
        Check::True (hasBCurve);
        }

    if (type != ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        {
        Check::True (curve->IsFractionSpace ());
        }

    }
bool CheckWithinFactor (double a, double b, double factor, char const *message)
    {
    return Check::LessThanOrEqual (a, factor * b, message)
        && Check::LessThanOrEqual (b, factor * a, message);
    }
void ExerciseStrokeVirtuals (ICurvePrimitivePtr &curve, double fractionA, double fractionB)
    {
    bvector<DPoint3d> strokes;
    auto options = IFacetOptions::CreateForCurves ();
    curve->AddStrokes (strokes, *options, true, fractionA, fractionB);
    auto strokeCurve = ICurvePrimitive::CreateLineString (strokes);
    double lengthAB, lengthStrokes;
    curve->SignedDistanceBetweenFractions (fractionA, fractionB, lengthAB);
    strokeCurve->Length (lengthStrokes);
    double f1 = 1.0 + Angle::SmallAngle ();
    Check::LessThanOrEqual (lengthStrokes, f1 * lengthAB);
    Check::LessThanOrEqual (0.95 * lengthAB, lengthStrokes);

    size_t numStrokesA = curve->GetStrokeCount (*options, fractionA, fractionB);
    size_t numPoints = strokes.size ();
    // hm... no exact expectations, and spirals have issues . . .
    if (curve->GetCurvePrimitiveType () != ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral)
        CheckWithinFactor ((double)(numStrokesA + 1) , (double)numPoints, 1.5, "predicted and actual stroke counts similar");
    }

void ExercisePlaneIntersection (ICurvePrimitivePtr &curve, double fraction)
    {
    DPoint3d xyzA;
    DVec3d tangentA;
    curve->FractionToPoint (fraction, xyzA, tangentA);
    auto plane = DPlane3d::FromOriginAndNormal (xyzA, tangentA);
    bvector<CurveLocationDetailPair> details;
    curve->AppendCurvePlaneIntersections (plane, details);
    auto pair = CurveLocationDetail::ClosestPoint (details, xyzA, 1.0e-8);
    Check::True (pair.IsValid ()  && Check::Near (xyzA, pair.Value().detailA.point));
    }

void ExerciseClosestPoint (ICurvePrimitivePtr &curve, double fraction)
    {
    DPoint3d xyzA;
    DVec3d tangentA;
    curve->FractionToPoint (fraction, xyzA, tangentA);
    auto plane = DPlane3d::FromOriginAndNormal (xyzA, tangentA);
    CurveLocationDetail detail;
    if (Check::True (curve->ClosestPointBounded (xyzA, detail, false, false), "ClosestPoint"))
        Check::Near (detail.fraction, fraction, "ClosestPoint fraction");
    }


void ExerciseComponentVirtuals (ICurvePrimitivePtr &curve)
    {
    size_t numComponents = curve->NumComponent ();
    if (numComponents > 1)
        {
        double length0, length1;
        bvector<DPoint3d> points;
        DPoint3d xyz;
        curve->ComponentFractionToPoint (0, 0.0, xyz);
        points.push_back (xyz);
        for (size_t i = 0; i < numComponents; i++)
            {
            double df = 0.125;
            for (double f = df; f <= 1.0; f += df)
                {
                curve->ComponentFractionToPoint (0, 0.0, xyz);
                points.push_back (xyz);
                }
            double breakFraction0, breakFraction1;
            Check::True (curve->GetBreakFraction (i, breakFraction0));
            if (i + 1 < numComponents)
                {
                Check::True (curve->GetBreakFraction (i + 1, breakFraction1));
                }
            else
                breakFraction1 = 1.0;

            }
        curve->Length (length0);
        length1 = PolylineOps::Length (points);
        CheckWithinFactor (length0, length1, 1.1, "Component stroke length versus curve stroker");
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
    ExerciseStrokeVirtuals(curve, splitFractionA, splitFractionB);
    ExercisePlaneIntersection (curve, splitFractionA);
    ExercisePlaneIntersection (curve, splitFractionB);

    ExerciseClosestPoint (curve, splitFractionA);
    ExerciseClosestPoint (curve, splitFractionB);
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
    auto frame = Transform::FromIdentity ();
    for (int spiralType : {
            DSpiral2dBase::TransitionType_Clothoid,
            DSpiral2dBase::TransitionType_Bloss,
            DSpiral2dBase::TransitionType_Biquadratic,
            DSpiral2dBase::TransitionType_Cosine,
            DSpiral2dBase::TransitionType_Sine,
            //DSpiral2dBase::TransitionType_Viennese,
            //DSpiral2dBase::TransitionType_WeightedViennese,
            DSpiral2dBase::TransitionType_WesternAustralian,
            //DSpiral2dBase::TransitionType_Czech,
            DSpiral2dBase::TransitionType_AustralianRailCorp,
            //DSpiral2dBase::TransitionType_Italian,
            //DSpiral2dBase::TransitionType_Polish
            DSpiral2dBase::TransitionType_MXCubicAlongArc,
            DSpiral2dBase::TransitionType_DirectHalfCosine,
            DSpiral2dBase::TransitionType_ChineseCubic,
            })
        {
        auto c3 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (
            spiralType,
            0,0,100,1000,
            frame, 0, 1
            );
        ExerciseAllVirtuals (c3, 0.25, 0.75);
        }
    }
    
