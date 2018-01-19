//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PseudoSpiral,Serialize)
    {
    double spiralLength = 100.0;
    double dyShift = 40.0;
    double dxShift = 250.0;
    bvector<double> radiusMeters {1000, 400, 200 };
    //bvector<double> pseudoLengthMeters {100.02506262460328, 100.15873011778872, 100.66666663992606};
    //bvector<double> edgeCount {15, 25, 35};
    double yShift = 0.0;
    for (int spiralType : {
                DSpiral2dBase::TransitionType_Clothoid,
#ifdef doAll
                DSpiral2dBase::TransitionType_Bloss,
                DSpiral2dBase::TransitionType_Biquadratic,
                DSpiral2dBase::TransitionType_Cosine,
                DSpiral2dBase::TransitionType_Sine,
#endif
                //DSpiral2dBase::TransitionType_Viennese,
                //DSpiral2dBase::TransitionType_WeightedViennese,
                DSpiral2dBase::TransitionType_NewSouthWales,
                //DSpiral2dBase::TransitionType_Czech,
                DSpiral2dBase::TransitionType_Australian,
                //DSpiral2dBase::TransitionType_Italian,
                //DSpiral2dBase::TransitionType_Polish
                })
        {
        double xShift = -dxShift;
        for (double radius1 : {1000.0, 400.0, 200.0})
            {
            xShift += dxShift;
            yShift = 50.0;  // start up to avoid reference data placed at (0,0), (0,10), (0,20)
            for (double fraction0 : {0.0, 0.6})
                {
                auto frame = Transform::From (xShift, yShift, 0);
                yShift += dyShift;
                for (double bearing0 : {0.0,  Angle::DegreesToRadians (10), Angle::DegreesToRadians (135)})
                    {
                    for (double radiusFactor : {-1.0, 1.0})
                        {
                        auto spiral = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (
                                spiralType,
                                bearing0, 0.0, spiralLength, radiusFactor / radius1,
                                frame,
                                fraction0, 1.0
                                );
                        if (spiral.IsValid ())
                            {
                            Check::SaveTransformed (*spiral);
                            auto bcurve = spiral->GetProxyBsplineCurvePtr ();
                            if (bcurve.IsValid ())
                                {
                                auto bprim = ICurvePrimitive::CreateBsplineCurve (bcurve);
                                Check::SaveTransformed (*bprim);
                                }
                            }
                        }
                    }
                }
            }
        char fileName[1024];
        Utf8String typeName;
        DSpiral2dBase::TransitionTypeToString (spiralType, typeName);
        sprintf (fileName, "Spiral.Serialize.%s", typeName.c_str());
        Check::ClearGeometry (fileName);
        }
    }

ICurvePrimitivePtr ConstructSpiralRadiusRadiusLength (int typeCode, Angle bearingA, double radiusA, double radiusB, double lengthAB)
    {
    double curvatureA = DoubleOps::ValidatedDivideDistance (1.0, radiusA);
    double curvatureB = DoubleOps::ValidatedDivideDistance (1.0, radiusB);

    // extrapolate to inflection, assuming clothoid (linear) curvature function
    //   (curvatureB - curvatureA) / lengthAB = curvatureB / length0B
    double length0B = curvatureB * lengthAB / (curvatureB - curvatureA);
    double length0A = length0B - lengthAB;
    double fractionA = curvatureA / curvatureB;
    double fractionB = 1.0;
    auto referenceSpiralA = DSpiral2dBase::Create (typeCode);
    auto referenceSpiral = dynamic_cast <DSpiral2dDirectEvaluation *>(referenceSpiralA);
    if (referenceSpiral == nullptr)
        return nullptr;
    double bearing0 = 0.0;
    double curvature0 = 0.0;
    referenceSpiral->SetBearingCurvatureLengthCurvature (0.0, curvature0, length0B, curvatureB);
    DPoint2d uvA;
    DVec2d   duvA;
    referenceSpiral->EvaluateAtDistance (length0A, uvA, &duvA, nullptr, nullptr);
    DVec3d unitTangentA =  DVec3d::From (uvA.x, uvA.y, 0.0).ValidatedNormalize ();
    DVec3d unitPerpA = DVec3d::From (-unitTangentA.y, unitTangentA.x);
    auto frameAtA = Transform::FromOriginAndVectors (DPoint3d::From (uvA), unitTangentA, unitPerpA, DVec3d::UnitZ ());
    auto inverseA = frameAtA.ValidatedInverse ();
    if (!inverseA.IsValid ())
        return nullptr;
    if (bearingA.Radians () != 0.0)
        {
        auto rotation = Transform::FromMatrixAndFixedPoint (RotMatrix::FromAxisAndRotationAngle (2, bearingA.Radians ()), DPoint3d::FromZero ());
        inverseA = rotation * inverseA;
        }

    GEOMAPI_PRINTF ("RRL Spiral type %d\n", typeCode);
    for (double curvatureTarget : { curvatureA, curvatureB})
        {
        double f = curvatureTarget/ curvatureB;
        double s = referenceSpiral->FractionToDistance (f);
        double curvatureF = referenceSpiral->DistanceToCurvature (s);
        double radiusF = DoubleOps::ValidatedDivideDistance (1.0, curvatureF, 0.0);
        double radiusTarget = DoubleOps::ValidatedDivideDistance (1.0, curvatureTarget, 0.0);
        GEOMAPI_PRINTF(" (f %g) (s %g) (r %g (er %g)) (q %g)\n",
                        f, s, radiusF, radiusF - radiusTarget, referenceSpiral->DistanceToLocalAngle (s));
        }

    auto fullSpiral = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (typeCode, 0.0, 0.0, length0B, curvatureB, inverseA,
            fractionA, fractionB);
    return fullSpiral;
    }
    
TEST(Spiral,TwoRadiusConstruction)
    {
    double radiusB = 1000.0;
    double length0B = 100.0;
    int typeCode = DSpiral2dBase::TransitionType_NewSouthWales;
    double yShiftB = 45.0;
    double yShiftA = 5.0;
    for (double bearing0Degrees : {0.0, 10.0})
        {
        SaveAndRestoreCheckTransform shifter (2.0 * length0B, 0,0);
        for (double fraction : {0.0, 0.25, 0.5, 0.75})
            {
            double curvatureB = DoubleOps::ValidatedDivideDistance (1.0, radiusB);
            double curvatureA = fraction * curvatureB;
            double radiusA = DoubleOps::ValidatedDivideDistance (1.0, curvatureA);
            double lengthAB = (1.0 - fraction) * length0B;
            auto spiral0B = ConstructSpiralRadiusRadiusLength (typeCode, Angle::FromDegrees (bearing0Degrees),
                    0.0, radiusB,
                    lengthAB * curvatureB / (curvatureB- curvatureA));
            // auto spiralAB = ConstructSpiralRadiusRadiusLength (typeCode, Angle::FromDegrees (bearing0Degrees), radiusA, radiusB, lengthAB);
            auto placement = Transform::FromMatrixAndFixedPoint (RotMatrix::FromAxisAndRotationAngle (2, Angle::DegreesToRadians (bearing0Degrees)), DPoint3d::From (0, yShiftA, 0));
            auto spiralAB = ICurvePrimitive::CreatePseudoSpiralRadiusLengthRadius (typeCode, placement, radiusA, lengthAB, radiusB);
            Check::SaveTransformed (*spiral0B);
            Check::SaveTransformed (*spiralAB);
            }
        }
    Check::ClearGeometry ("Spiral.TwoRadiusConstruction");
    }

void ShowFrame (TransformCR frame, double ax, double ay)
    {
    auto origin = frame * DPoint3d::From (0,0,0);
    auto xPoint = frame * DPoint3d::From (ax,0,0);
    auto yPoint = frame * DPoint3d::From (0,ay,0);
    Check::SaveTransformed (bvector<DPoint3d> {yPoint, origin, xPoint});
    }

Transform HeadsUpFrameOnCurve (ICurvePrimitiveCR curve, double fraction)
    {
    auto frame = curve.FractionToFrenetFrame (fraction);
    if (frame.Value ().form3d[2][2] < 0.0)
        frame.Value ().ScaleMatrixColumns (1.0, -1.0, -1.0);     // flip from "right handed with negative Z" to "right handed with positive Z" -- preserve X direction
    return frame.Value ();
    }
TEST(Spiral,FrenetFrameConstruction)
    {
    int typeCode = DSpiral2dBase::TransitionType_NewSouthWales;
    double radiusA = 200.0;
    double turnDegrees = 90.0;
    double lengthAB = 25.0;
    double lengthArc0 = 50.0;
    double gridStep = 5.0 * lengthArc0;
    for (double radiusA : {200.0, 100.0, 0.0, -100.0, -200.0})
        {
        SaveAndRestoreCheckTransform shifter (0.0, gridStep, 0);
        for (double radiusB : {200.0, 100.0, 0.0, -100.0, -200.0})
            {
            SaveAndRestoreCheckTransform shifter (gridStep, 0, 0);
            // a subtle way to allow both line and arc as entry ...
            auto curve0 = ICurvePrimitive::CreatePseudoSpiralRadiusLengthRadius (typeCode, Transform::FromIdentity (), radiusA, lengthArc0, radiusA);
            auto frame0 = HeadsUpFrameOnCurve (*curve0, 1.0);
            ShowFrame (frame0, 2, 1);
            Check::SaveTransformed (*curve0);
            auto spiralAB = ICurvePrimitive::CreatePseudoSpiralRadiusLengthRadius (typeCode, frame0, radiusA, lengthAB, radiusB);
            if (spiralAB.IsValid ())
                {
                auto frame1 = HeadsUpFrameOnCurve (*spiralAB, 1.0);
                ShowFrame (frame1, 2, 1);
                    // frame1 has radius1 sign built in .. use positive radius
                auto curve1 = ICurvePrimitive::CreatePseudoSpiralRadiusLengthRadius (typeCode, frame1, radiusB, lengthArc0, radiusB);
                Check::SaveTransformed (*spiralAB);
                Check::SaveTransformed (*curve1);
                }
            }
        }
    Check::ClearGeometry ("Spiral.FrenetFrameConstruction");
    }