//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
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
                DSpiral2dBase::TransitionType_MXCubic
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
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,TwoRadiusConstruction)
    {
    double radiusB = 1000.0;
    double length0B = 100.0;
    int typeCode = DSpiral2dBase::TransitionType_NewSouthWales;
    double yShiftB = 45.0;
    double yShiftA = 5.0;
    for (double bearing0Degrees : {0.0, 10.0})
        {
        auto bearing0Radians = Angle::DegreesToRadians (bearing0Degrees);
        SaveAndRestoreCheckTransform shifter (2.0 * length0B, 0,0);
        for (double fraction : {0.0, 0.25, 0.5, 0.75})
            {
            SaveAndRestoreCheckTransform shifter (0, yShiftB, 0);

            double curvatureB = DoubleOps::ValidatedDivideDistance (1.0, radiusB);
            double curvatureA = fraction * curvatureB;
            double radiusA = DoubleOps::ValidatedDivideDistance (1.0, curvatureA);
            double lengthAB = (1.0 - fraction) * length0B;
            auto spiral0B = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode,
                DPoint3d::From (0, 0, 0), bearing0Radians, 0.0, lengthAB, radiusB);

            auto spiralAB = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode,
                DPoint3d::From (0, yShiftA, 0), bearing0Radians, radiusA, lengthAB, radiusB);
            Check::SaveTransformed (*spiral0B);
            Check::SaveTransformed (*spiralAB);
            }
        }
    Check::ClearGeometry ("Spiral.TwoRadiusConstruction");
    }

void ShowFrame (DPoint3dCR origin, double bearingRadians, double sizeX, double sizeY)
    {
    if (sizeY != 0.0)
        {
        auto xPoint = origin + DVec3d::FromXYAngleAndMagnitude (bearingRadians, sizeX);
        auto xPoint1 = origin - DVec3d::FromXYAngleAndMagnitude (bearingRadians, sizeX);
        auto yPoint = origin + DVec3d::FromXYAngleAndMagnitude (bearingRadians + Angle::PiOver2 (), sizeY);
        Check::SaveTransformed (bvector<DPoint3d> {yPoint, origin, xPoint, xPoint1});
        }
    else
        {
        auto xPoint = origin + DVec3d::FromXYAngleAndMagnitude (bearingRadians, sizeX);
        auto xPoint1 = origin - DVec3d::FromXYAngleAndMagnitude (bearingRadians, sizeX);
        auto yPoint = origin + DVec3d::FromXYAngleAndMagnitude (bearingRadians + Angle::PiOver2 (), sizeX);
        auto yPoint1 = origin - DVec3d::FromXYAngleAndMagnitude (bearingRadians + Angle::PiOver2 (), sizeX);
        Check::SaveTransformed (bvector<DPoint3d> {yPoint, yPoint1, origin, xPoint, xPoint1});
        }
    }

void testRadiusCombinations (bvector<double> radiiA, bvector<double> radiiB)
    {
    int typeCode = DSpiral2dBase::TransitionType_NewSouthWales;
    double lengthAB = 25.0;
    double lengthArc0 = 50.0;
    double gridStep = 400.0;
    double xAxisSize = 20.0;
    double directStrokeFractionStep = 1.0 / 32.0;
    Transform smallShift = Transform::From (0, 5,0);
    for (double radiusA : radiiA)
        {
        SaveAndRestoreCheckTransform shifter (0.0, gridStep, 0);
        for (double radiusB : radiiB)
            {
            SaveAndRestoreCheckTransform shifter (gridStep, 0, 0);
            // a subtle way to allow both line and arc as entry ...
            auto curve0 = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode, DPoint3d::FromZero (), 0.0, radiusA, lengthArc0, radiusA);
            DPoint3d pointA, pointB;
            DVec3d tangentA, tangentB;
            curve0->FractionToPoint (1.0, pointA, tangentA);
            double bearingA = tangentA.AngleXY ();
            ShowFrame (pointA, bearingA, xAxisSize, radiusA);
            Check::SaveTransformed (*curve0);
            auto spiralAB = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode, pointA, bearingA, radiusA, lengthAB, radiusB);
            if (spiralAB.IsValid ())
                {
                auto bcurve = spiralAB->GetProxyBsplineCurvePtr ();
                if (bcurve.IsValid ())
                    {
                    auto bprim = ICurvePrimitive::CreateBsplineCurve (bcurve);
                    bprim->TransformInPlace (smallShift);
                    Check::SaveTransformed (*bprim);
                    bvector<DPoint3d> strokes;
                    for (auto f = 0.0; f <= 1.0000001; f += directStrokeFractionStep)
                        {
                        DPoint3d xyz;
                        spiralAB->FractionToPoint (f, xyz);
                        xyz = smallShift * xyz;
                        strokes.push_back (xyz);
                        }
                        Check::SaveTransformed (*bprim);
                        Check::SaveTransformed (strokes);
                    }
                spiralAB->FractionToPoint (1.0, pointB, tangentB);
                double bearingB = tangentB.AngleXY ();
                ShowFrame (pointB, bearingB, xAxisSize, radiusB);
                    // frame1 has radius1 sign built in .. use positive radius
                auto curve1 = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode, pointB, bearingB, radiusB, lengthArc0, radiusB);
                Check::SaveTransformed (*spiralAB);
                Check::SaveTransformed (*curve1);
                }
            }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,MixedRadiusConstructionsA)
    {
    // mixed radius constructions with varying magnitudes.
    bvector<double> radii {100.0, 50.0, 0.0, -50.0, -100.0};
    testRadiusCombinations (radii, radii);
    Check::ClearGeometry ("Spiral.MixedRadiusConstructionsA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,MixedRadiusConstructionsB)
    {
    // mixed radius constructions with nearly equal magnitudes
    bvector<double> radiiA {100.0, 50.0};
    bvector<double> radiiB {-50.0, -100.0};
    testRadiusCombinations (radiiA, radiiB);
    Check::ClearGeometry ("Spiral.MixedRadiusConstructionsB");
    }

void testClothoidCosineApproximation (
char const *name,
double numSamples,
double R,
double L,
double yFactor,
double (*forwardMap)(double, double, double, double),
ValidatedDouble (*inverseMap)(double, double, double, double),
bvector<DPoint3d> &xyFromS
)
    {
    UsageSums positiveSums, negativeSums;
    UsageSums positiveRoundTripErrors, negativeRoundTripErrors;
    bvector<double> positiveFailures, negativeFailures;

    for (double i = 0; i <= numSamples; i++)
        {
        double u = DoubleOps::Interpolate (-L, i / numSamples, L);
        auto fPositive = inverseMap (1.0, R, L, u);
        if (fPositive.IsValid ())
            {
            positiveSums.Accumulate (ClothoidCosineApproximation::s_evaluationCount);
            auto g = forwardMap (-1.0, R, L, fPositive.Value ());
            positiveRoundTripErrors.Accumulate (u - g);
            }
        else
            positiveFailures.push_back (u);
        auto fNegative = inverseMap (-1.0, R, L, u);
        if (fNegative.IsValid ())
            {
            negativeSums.Accumulate (ClothoidCosineApproximation::s_evaluationCount);
            auto g = forwardMap (1.0, R, L, fNegative.Value ());
            negativeRoundTripErrors.Accumulate (u - g);
            }
        else
            negativeFailures.push_back (u);
        // Treat u as DISTANCE ALONG SPIRAL ..
        double xFromS = forwardMap (-1.0, R, L, u);
        double yFromS = yFactor * u * u * u;
        xyFromS.push_back (DPoint3d::From (xFromS, yFromS, 0));
        }
    Check::Size (0, positiveFailures.size (), "Iterative failures with positive second term");
    Check::Size (0, negativeFailures.size (), "Iterative failures with negative second term");
    static double s_cosineSeriesRelTol = 1.0e-3;
    Check::LessThanOrEqual (positiveRoundTripErrors.MaxAbs (), s_cosineSeriesRelTol * L, "cosine series axis-curve-axis roundtrip relative error");
    Check::LessThanOrEqual (negativeRoundTripErrors.MaxAbs (), s_cosineSeriesRelTol * L, "cosine series curve-axis-curve roundtrip relative error");
    GEOMAPI_PRINTF ("Positive Iterations: %s (max %g) (mean %g) (roundTrip Error Range %7.1le %7.1le)\n",
        name,
        positiveSums.Max (), positiveSums.Mean (), positiveRoundTripErrors.Min (), positiveRoundTripErrors.Max ()
        );
    GEOMAPI_PRINTF ("Negative Iterations: %s (max %g) (mean %g) (roundTrip Error Range %7.1le %7.1le)\n",
        name,
        negativeSums.Max (), negativeSums.Mean (), negativeRoundTripErrors.Min (), negativeRoundTripErrors.Max ()
        );

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,ClothoidCosineApproximation)
    {
    double R = 1000.0;
    double L = 100.0;
    // verify some algebra . .
    // constants in czech/italian code:
    double lambda = asin(L / (2.0 * R));
    double gamma = 1.0 / cos (lambda);
    double gamma1 = 2.0 * R / sqrt ( 4.0 * R * R - L * L);
    double gamma2 = ClothoidCosineApproximation::CzechGamma (R, L);
    Check::Near (gamma, gamma1, "beta formulations");
    Check::Near (gamma1, gamma2, "beta formulations");
    double QQ = 4.0 * R * R - L * L;
    double A0 = 40.0 * R * R * L * L;
    double A1 = 10.0 * QQ * L * L;
    Check::True (fabs (A1/A0 - 1.0) < 0.01, "factors are similar");

    double numSamples = 100;
    bvector<DPoint3d> xyFromS, xyFromSItalian;
    testClothoidCosineApproximation (
        "COSINE SERIES 40 RRLL",
        numSamples,
        R, L, 1.0 / (6.0 * R * L),
        ClothoidCosineApproximation::Evaluate40R2L2Map,
        ClothoidCosineApproximation::Invert40R2L2Map,
        xyFromS);
    Check::SaveTransformed (xyFromS);

    testClothoidCosineApproximation (
        "ITALIAN COSINE SERIES 10(4 RR - LL) LL",
        numSamples,
        R, L, 1.0 / (6.0 * R * L),
        ClothoidCosineApproximation::EvaluateItalianCzechR2L2Map,
        ClothoidCosineApproximation::InvertItalianCzechR2L2Map,
        xyFromSItalian);
    Check::SaveTransformed (xyFromSItalian);

    Check::ClearGeometry ("Spiral.ClothoidCosineApproximation");
    }
