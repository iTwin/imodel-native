//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
#include <Bentley/BeNumerical.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PseudoSpiral,Serialize)
    {
    //bvector<double> pseudoLengthMeters {100.02506262460328, 100.15873011778872, 100.66666663992606};
    //bvector<double> edgeCount {15, 25, 35};
    double smallYShift = 1.0;
    auto options = IFacetOptions::CreateForCurves ();
    double strokeShiftX = 500.0;
    double strokeShiftY = 0;
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
        Check::SetTransform (Transform::FromIdentity ());
        double xShift = 1500.0;
        SaveAndRestoreCheckTransform typeShift (xShift, 0, 0);
        double yShift = 200.0;
        for (double spiralLength : {100.0, 200.0})
            {
            SaveAndRestoreCheckTransform shifter (0, yShift, 0);
            for (double radius1 : {800.0, 400.0, 200.0, 50.0})
                {
                if (spiralType == DSpiral2dBase::TransitionType_AustralianRailCorp
                    && radius1 <= 1.8 * spiralLength)
                        continue;
                for (double fraction0 : {0.0, 0.6})
                    {
                    for (double bearing0 : {0.0,  Angle::DegreesToRadians (60), Angle::DegreesToRadians (135)})
                        {
                        for (double radiusFactor : {-1.0, 1.0})
                            {
                            // This shift moves the partial curves away from the full curve, in the vertical direction inward curve.
                            auto frame = Transform::From (0, smallYShift * radiusFactor * fraction0, 0);

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
                                bvector<DPoint3d> strokes;
                                spiral->AddStrokes (strokes, *options, true, 0.0, 1.0);
                                int errors = 0;
                                if (strokes.size () == 0)
                                    errors++;
                                else
                                    {
                                    for (auto &xyz : strokes)
                                        {
                                        if (BeNumerical::BeIsnan (xyz.x) || BeNumerical::BeIsnan (xyz.y) || BeNumerical::BeIsnan (xyz.z))
                                            errors++;
                                        }
                                    }
                                Check::Shift (strokeShiftX, strokeShiftY, 0);
                                if (errors == 0)
                                    Check::SaveTransformed (strokes);
                                Check::Shift (-strokeShiftX, -strokeShiftY, 0);
                                }
                            }
                        }
                    }
                }
            }
        char fileName[1024];
        Utf8String typeName;
        DSpiral2dBase::TransitionTypeToString (spiralType, typeName);
        // add 1000 to ensure alphabetic sort maintains numeric order
        sprintf (fileName, "Spiral.Serialize.%d%s", 1000+(int)spiralType, typeName.c_str());
        Check::ClearGeometry (fileName);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PseudoSpiral,ExerciseDistances)
    {
    //bvector<double> pseudoLengthMeters {100.02506262460328, 100.15873011778872, 100.66666663992606};
    //bvector<double> edgeCount {15, 25, 35};
    double smallYShift = 1.0;
    auto options = IFacetOptions::CreateForCurves ();
    bvector<double> breakFractions {0.25, 0.54, 0.98, 1.0}; // implied 0 at start
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
                //-----------DSpiral2dBase::TransitionType_WesternAustralian,
                //DSpiral2dBase::TransitionType_Czech,
                //-----------DSpiral2dBase::TransitionType_AustralianRailCorp,
                //DSpiral2dBase::TransitionType_Italian,
                //DSpiral2dBase::TransitionType_Polish
                //-----------DSpiral2dBase::TransitionType_MXCubicAlongArc,
                DSpiral2dBase::TransitionType_DirectHalfCosine,
                DSpiral2dBase::TransitionType_ChineseCubic
                })
        {
        Check::StartScope ("ExerciseDistance", (size_t)spiralType);
        Check::SetTransform (Transform::FromIdentity ());
        double xShift = 1500.0;
        SaveAndRestoreCheckTransform typeShift (xShift, 0, 0);
        double yShift = 200.0;
        for (double spiralLength : {100.0, 200.0})
            {
            SaveAndRestoreCheckTransform shifter (0, yShift, 0);
            for (double radius1 : {800.0, 400.0, 200.0})
                {
                if (spiralType == DSpiral2dBase::TransitionType_AustralianRailCorp
                    && radius1 <= 1.8 * spiralLength)
                        continue;
                for (double fraction0 : {0.0, 0.6})
                    {
                    for (double bearing0 : {0.0,  Angle::DegreesToRadians (10), Angle::DegreesToRadians (135)})
                        {
                        for (double radiusFactor : {-1.0, 1.0})
                            {
                            // This shift moves the partial curves away from the full curve, in the vertical direction inward curve.
                            auto frame = Transform::From (0, smallYShift * radiusFactor * fraction0, 0);

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
                                double accumulatedDistance = 0.0;
                                double totalDistance;
                                if (Check::True (spiral->Length (totalDistance)))
                                    {
                                    double f0 = 0.0;
                                    double maxDiff = 0;
                                    for (auto f1 : breakFractions)
                                        {
                                        double d01;
                                        if (Check::True (spiral->SignedDistanceBetweenFractions (f0, f1, d01)))
                                            {
                                            accumulatedDistance += d01;
                                            CurveLocationDetail detail;
                                            if (Check::True (spiral->PointAtSignedDistanceFromFraction (f0, d01, true, detail)))
                                                {
                                                maxDiff = DoubleOps::Max (maxDiff, fabs (f1 - detail.fraction));
                                                }
                                            }
                                        f0 = f1;
                                        }
                                    static double s_fractionErrorTol = 1.0e-6;       // allow fluffy distances
                                    static double s_distanceFactor = 1.0e-6;
                                    Check::LessThanOrEqual (maxDiff, s_fractionErrorTol, "Signed distance inversion");
                                    Check::LessThanOrEqual (fabs (totalDistance - accumulatedDistance), s_distanceFactor * totalDistance, "Confirm split distance sum");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        char fileName[1024];
        Utf8String typeName;
        DSpiral2dBase::TransitionTypeToString (spiralType, typeName);
        // add 1000 to ensure alphabetic sort maintains numeric order
        sprintf (fileName, "Spiral.Exercise.%d%s", 1000+(int)spiralType, typeName.c_str());
        Check::ClearGeometry (fileName);
        Check::EndScope();
        }
    }


   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,TwoRadiusConstruction)
    {
    double radiusB = 1000.0;
    double length0B = 100.0;
    int typeCode = DSpiral2dBase::TransitionType_WesternAustralian;
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

void SaveSpiralAndStrokes (ICurvePrimitiveCR primitive, double dy)
    {
    bvector<DPoint3d> strokes;
    auto options = IFacetOptions::CreateForCurves ();

    Check::SaveTransformed (primitive);
    Check::Shift (0, dy, 0);
    primitive.AddStrokes (strokes, *options);
    Check::SaveTransformed (strokes);
    Check::Shift (0, dy, 0);
    auto bcurve = primitive.GetProxyBsplineCurvePtr ();
    if (bcurve.IsValid ())
        {
        auto bprim = ICurvePrimitive::CreateBsplineCurve (bcurve);
        Check::SaveTransformed (*bprim);
        }
    }
TEST (Spiral, ConstructPseudoSpiralViaSimpleCreate)
    {
    double lengthAB = 100.0;
    int typeCode = DSpiral2dBase::TransitionType_WesternAustralian;
    double yShiftB = 45.0;
    //double yShiftA = 5.0;
    double yShift = 1.0;
    double zShift = 0.0;
    double bearing0Degrees = 0.0;
    auto bearing0Radians = Angle::DegreesToRadians (bearing0Degrees);
    for (double radiusA : {0.0, 1000.0})
        {
        SaveAndRestoreCheckTransform shifter (2.0 * lengthAB, 0, 0);
        for (double radiusB : {1000.0, 0.0})
            {
            SaveAndRestoreCheckTransform shifter (0, yShiftB, 0);
            if (radiusA != radiusB)
                {
                double curvatureA = DoubleOps::ValidatedDivideDistance (1.0, radiusA);
                double curvatureB = DoubleOps::ValidatedDivideDistance (1.0, radiusB);
                DSpiral2dBaseP baseSpiral = DSpiral2dBase::CreateBearingCurvatureLengthCurvature (typeCode,
                            bearing0Radians, curvatureA, lengthAB, curvatureB);
                auto frame = Transform::FromIdentity ();
                DPoint3d originZ = DPoint3d::From (0,yShift,zShift);
                auto spiralX = ICurvePrimitive::CreateSpiral (*baseSpiral, frame, 0.0, 1.0);
                auto spiralZ = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode,
                    originZ, bearing0Radians, radiusA, lengthAB, radiusB);
                SaveSpiralAndStrokes (*spiralX, 1);
                Check::Shift (0,10,0);
                SaveSpiralAndStrokes (*spiralZ, 1);


                }
            }
        }
    Check::ClearGeometry ("Spiral.ConstructPseudoSpiralViaSimpleCreate");
    }



TEST(Spiral,StrokeA)
    {
    //double radiusB = 1000.0;
    double radiusB = -2000.0;
    double length0B = 200.0;
    int typeCode = DSpiral2dBase::TransitionType_WesternAustralian;
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

            IFacetOptionsPtr opt = IFacetOptions::CreateForCurves();
            auto maxChord = 0.01;
            opt->SetChordTolerance(maxChord);
            bvector <DPoint3d> points;
            if (spiralAB->AddStrokes(points, *opt, true, 0.0, 1.0))
                {
                Check::Shift (0, yShiftA * 0.1, 0);
                Check::SaveTransformed (points);
                }
            }
        }
    Check::ClearGeometry ("Spiral.StrokeA");
    }

TEST(Spiral,ConstructionCrash)
    {
    double radiusA = 0.0;
    double radiusB = 121.752286;
    double radiansA = Angle::Pi ();
    double length0B = 100.0;
    int typeCode = DSpiral2dBase::TransitionType_AustralianRailCorp;
    double yShiftB = -6.0;
    for (double radiusBFraction : {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.925, 0.950, 0.975, 1.0})
        {
        SaveAndRestoreCheckTransform shifter (0, yShiftB, 0);
        auto spiral0B = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode,
            DPoint3d::From (0, 0, 0), radiansA, radiusA, length0B, radiusB / radiusBFraction);
        Check::SaveTransformed (*spiral0B);
        }
    Check::ClearGeometry ("Spiral.ConstructionCrash");
    }




void testRadiusCombinations (bvector<double> radiiA, bvector<double> radiiB)
    {
    int typeCode = DSpiral2dBase::TransitionType_WesternAustralian;
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
struct RadiusAndCount
{
double R1;
double L1;
uint32_t stepCount;
RadiusAndCount (double radius1, double length1, uint32_t count) : R1(radius1), L1(length1), stepCount (count) {}
};
bool DerivativesToFrenetFrame(DPoint3dCR point, DVec3dCR deriv1, DVec3dCR deriv2, DVec3dCR deriv3,
TransformR frame, double &curvature, double &torsion)
    {
    curvature = torsion = 0.0;
    
    RotMatrix rMatrix1, rMatrix2;
    rMatrix1.InitFrom2Vectors (deriv1, deriv2);
    rMatrix2.SquareAndNormalizeColumns (rMatrix1, 0, 1);
    frame.InitFrom (rMatrix2, point);
    double a = deriv1.Magnitude ();
    double a3 = a * a * a;
    DVec3d cross12 = DVec3d::FromCrossProduct (deriv1, deriv2);
    double tripleProduct = cross12.DotProduct (deriv3);
    double crossmag2 = cross12.MagnitudeSquared ();
    double crossmag  = sqrt (crossmag2);
    DoubleOps::SafeDivide (curvature, crossmag, a3, 0.0);
    DoubleOps::SafeDivide (torsion, tripleProduct, crossmag2, 0.0);
    return true;
    }
void CheckDerivatives (ICurvePrimitivePtr curve, double f, double tolFactor = 1.0)
    {
    static double df = 1.0e-8;
    double fA = f - df;
    double fB = f + df;
    double   gg[3];
    DPoint3d X[3];
    DVec3d   dX[3];
    DVec3d   ddX[3];
    int i = 0;
    for (double g : {fA, f, fB})
        {
        gg[i] = g;
        curve->FractionToPoint (g, X[i], dX[i], ddX[i]);
        i++;
        }
    DVec3d dY = (X[2] - X[0]) / (2.0 * df);
    DVec3d dYA = (X[1] - X[0]) / df;
    DVec3d dYB = (X[2] - X[1]) / df;
    DVec3d ddYAB = (dYB - dYA) / df;
    DVec3d ddZ = (dX[2] - dX[0]) / (2.0 * df);
    double toleranceScale1 = 1.0e7 * tolFactor;
    double toleranceScale2 = 1.0e7 * tolFactor;
    Check::Near (dX[1], dY, "approx dY", toleranceScale1);
    //Check::Near (ddX[1], ddYAB, "approx ddYAB", toleranceScale2);
    Check::Near (ddX[1], ddZ, "approx ddZ", toleranceScale2);
    Transform frame1, frame2, frame3;
    double curvature1, curvature2, curvature3;
    double torsion1, torsion2, torsion3;
    DVec3d dddX = DVec3d::From (0,0,0);
    DerivativesToFrenetFrame (X[1], dX[1], ddX[1], dddX, frame1, curvature1, torsion1);
    DerivativesToFrenetFrame (X[1], dY, ddZ, dddX, frame2, curvature2, torsion2);
    curve->FractionToFrenetFrame (f, frame3, curvature3, torsion3);
    curve->FractionToFrenetFrame (f, frame3, curvature3, torsion3);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  03/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,DocCheck)
    {
    double R0 = 0.0;
    double startRadians = 0.0;
    for (auto spiralType :
        {
        DSpiral2dBase::TransitionType_Clothoid,
        DSpiral2dBase::TransitionType_WesternAustralian,
        DSpiral2dBase::TransitionType_MXCubicAlongArc,
        DSpiral2dBase::TransitionType_DirectHalfCosine,
        DSpiral2dBase::TransitionType_AustralianRailCorp,
        })
        {
        Check::Shift (0, 40, 0);
        Utf8String typeName;
        DSpiral2dBase::TransitionTypeToString (spiralType, typeName);
        for (RadiusAndCount radiusAndCount : {
                RadiusAndCount(800, 200, 35),
                RadiusAndCount(500, 200, 44),
                RadiusAndCount(300, 300, 86)
                }
                )
            {
            double R1 = radiusAndCount.R1;
            double L  = radiusAndCount.L1   ;
            SaveAndRestoreCheckTransform shifter (0, 20, 0);
            Check::SaveTransformed (
                bvector<DPoint3d> {
                            DPoint3d::From (0,0),
                            DPoint3d::From ((double)spiralType, 0),
                            DPoint3d::From ((double)spiralType, -2),
                            });
            ICurvePrimitivePtr curve1 = nullptr;
            if (spiralType >= DSpiral2dBase::TransitionType_FirstDirectEvaluate)
                curve1 = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (
                        spiralType,
                        DPoint3d::From (0,0,0),
                        startRadians,
                        R0,
                        L,
                        R1
                        );
            else
                curve1 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (
                        spiralType,
                        startRadians,
                        R0,
                        L,
                        R1,
                        Transform::FromIdentity (),
                        0.0, 1.0
                        );
            // QUIET skip ... 
            if (curve1 == nullptr)
                continue;
            double stationStep = L / (double)radiusAndCount.stepCount;
            DPoint3d xyz;
            DVec3d   tangent;
            printf ("\n\n **** SPIRAL TYPE %d (%s) *********\n", spiralType, typeName.c_str ());
            bvector<DPoint3d> strokes;
            double c0, c1;
            DoubleOps::SafeDivide (c0, 1.0, R0, 0.0);
            DoubleOps::SafeDivide (c1, 1.0, R1, 0.0);
            double tolFactor = 1.0;
            if (spiralType == DSpiral2dBase::TransitionType_AustralianRailCorp)
                tolFactor = 1.0e5;  // horrible things happen numerically in ARC
            for (double stationDistance = 0.0; stationDistance < 1.001 * L; stationDistance += stationStep)
                {
                double f = stationDistance / L;
                curve1->FractionToPoint (f, xyz, tangent);
                if (spiralType != DSpiral2dBase::TransitionType_AustralianRailCorp)
                    CheckDerivatives (curve1, f, tolFactor);
                Transform frame;
                double curvature, torsion;
                curve1->FractionToFrenetFrame (f, frame, curvature, torsion);
                strokes.push_back (xyz);
                printf ("    %10.4lf   (kFraction %10.2le)  %20.4f   %20.4lf    %20.5lf\n", stationDistance,
                    (curvature - c0) / (c1 - c0),
                    xyz.x, xyz.y,
                    Angle::RadiansToDegrees (atan2 (tangent.y, tangent.x)));
                }
             Check::SaveTransformed (strokes);
             }
        Utf8String fileName;
        fileName.Sprintf ("Spiral.DocCheck.%s", typeName.c_str ());
        Check::ClearGeometry (fileName.c_str());
        }
    }
#ifdef TestLangham
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  03/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,SignedDistanceLangham)
    {
    double beta1 = Angle::DegreesToRadians (14.323944878270584);
    Transform frame = Transform::FromIdentity ();
    double r1 = 100.0;
    RotMatrix worldToLocal = RotMatrix::FromIdentity ();
    for (ICurvePrimitivePtr spiral : {
            ICurvePrimitive::CreateSpiralBearingRadiusBearingRadius (
                        DSpiral2dBase::TransitionType_Clothoid,
                        0.0, 0.0,
                        beta1, r1,
                        frame,
                        0.0, 1.0),
            ICurvePrimitive::CreateSpiralBearingRadiusBearingRadius (
                        DSpiral2dBase::TransitionType_Clothoid,
                        0.0, r1,
                        beta1, 0.0,
                        frame,
                        0.0, 1.0)}
            )
        {
        Check::SaveTransformed (*spiral);
        double spiralLength;
        spiral->Length (spiralLength);
        for (auto s : {-1.0, 1.0})
            {
            double startFraction = 0.5 * (1.0 + s);
            for (double stepFraction : {0.1, 0.5})
                {
                CurveLocationDetail detail;
                double signedDistance = stepFraction * spiralLength;
                if (Check::True (spiral->PointAtSignedDistanceFromFraction (&worldToLocal, startFraction, signedDistance, true, detail)))
                    {
                    Check::SaveTransformedMarker (detail.point);
                    }
                }
            }
        }
    Check::ClearGeometry ("Spiral.SignedDistanceAlongLangham");
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,AustralianReversal)
    {
    double R1 = 0.0;
    double L = 20.0;
    double tolFactor = 1000.0;      // aussRail is painfully grainy & does not match derivatives.
    for (double R0 : {-350.0, -200.0, -100.0, 100.0, 200.0, 350.0})
        {
        SaveAndRestoreCheckTransform shifter (0, 10.0 * L,0);
        for (double beta0 : {0.0, 1.0, 3.0, 4.74, 6.0 })
            {
            SaveAndRestoreCheckTransform shifter (10.0 * L,0,0);
            DPoint3d origin = DPoint3d::From (0,0,0);
            auto curve1 = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (
                    DSpiral2dBase::TransitionType_AustralianRailCorp,
                    DPoint3d::From (0,0,0),
                    beta0,
                    R0,
                    L,
                    R1
                    );
            DVec3d startTangent = DVec3d::From (-L * cos (beta0), -L * sin (beta0), 0);
            auto arc = DEllipse3d::FromStartTangentNormalRadiusSweep (origin, startTangent,
                        DVec3d::From (0,0,1), -R0, 0.5);


            for (double f: {0.3, 0.6, 0.99})
                {
                CheckDerivatives (curve1, f, tolFactor);
                }


            Check::SaveTransformed (*curve1);
            Check::SaveTransformed (arc);
            }
        }
    double radius = 25.0;
    auto arc = ICurvePrimitive::CreateArc (
                        DEllipse3d::FromCenterRadiusXY (DPoint3d::From (1,1,0), radius));
    for (double f: {0.3, 0.6, 0.99})
        {
        CheckDerivatives (arc, f, tolFactor);
        double curvature, torsion;
        Transform frame;
        arc->FractionToFrenetFrame(f, frame, curvature, torsion);
        }
    Check::ClearGeometry ("Spiral.AustralianReversal");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,SpiralStartEndMatch)
    {
    double tol = 1e-4;

    DPoint3d startPoint = DPoint3d::From(1000, 1000, 1000);
    double  length = 50.7574;
    double startAngle = Angle::Pi ();
    double endRadius = 720.0;
    double startRadius = 0.0;
    ICurvePrimitivePtr curvePrimiP = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius(DSpiral2dBase::TransitionType_WesternAustralian,
        startPoint, startAngle, startRadius, length, endRadius);
    double spiralLength = curvePrimiP->GetSpiralPlacementCP()->SpiralLength01();
    double position;
    curvePrimiP->SignedDistanceBetweenFractions(0.0, 1.0, position);
    Check::True (fabs (spiralLength - position) <= tol);

    startRadius = 720.0;
    endRadius = 0.0;
    curvePrimiP = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius(DSpiral2dBase::TransitionType_WesternAustralian,
        startPoint, startAngle, startRadius, length, endRadius);
    spiralLength = curvePrimiP->GetSpiralPlacementCP()->SpiralLength01();
    curvePrimiP->SignedDistanceBetweenFractions(0.0, 1.0, position);
    Check::True (fabs (spiralLength - position) <= tol);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,HalfCosineEvaluation)
    {
    bvector<DPoint3d> stroke, strokeD1, strokeD2, strokeD3;
    double tangentLength = 200.0;
    double r1 = 800.0;
    DPoint2d xy;
    DVec2d d1xy, d2xy, d3xy;
    int numStep = 25;
    for (int step = 0; step <= numStep; step++)
        {
        double f = (double)step / (double)numStep;
        DSpiral2dDirectHalfCosine::EvaluateAtFractionInStandardOrientation (
            f, tangentLength, r1,
            xy, &d1xy, &d2xy, &d3xy);
        double x = xy.x;
        Check::Near (x, f * tangentLength);
        stroke.push_back (DPoint3d::From (x, xy.y));
        strokeD1.push_back (DPoint3d::From (x, d1xy.y));
        strokeD2.push_back (DPoint3d::From (x, d2xy.y));
        strokeD3.push_back (DPoint3d::From (x, d3xy.y));
        }
    double dy = tangentLength * 0.75;
    bvector<DPoint3d> axis {
        DPoint3d::From (0,10,0),
        DPoint3d::From (0,0,0),
        DPoint3d::From (tangentLength, 0, 0)
        };
    Check::SaveTransformed (stroke);
    Check::SaveTransformed (axis);
    Check::Shift (0,dy,0);

    Check::SaveTransformed (strokeD1);
    Check::SaveTransformed (axis);
    Check::Shift (0,dy,0);

    Check::SaveTransformed (strokeD2);
    Check::SaveTransformed (axis);
    Check::Shift (0,dy,0);

    Check::SaveTransformed (strokeD3);
    Check::SaveTransformed (axis);
    Check::ClearGeometry ("Spiral.HalfCosineEvaluation");
    }
// Vector integrands for testing
struct HalfCosineIntegrands : BSIIncrementalVectorIntegrand
{
double m_tangentLength;
double m_r1;

HalfCosineIntegrands (double tangentLength, double r1) :
    m_tangentLength (tangentLength),
    m_r1 (r1)
    {
    }

int GetVectorIntegrandCount () override { return 4;}
void EvaluateVectorIntegrand (double t, double *pF) override
    {
    DPoint2d xy;
    DVec2d d1xy, d2xy, d3xy;
    DSpiral2dDirectHalfCosine::EvaluateAtFractionInStandardOrientation (
            t, m_tangentLength, m_r1,
            xy, &d1xy, &d2xy, &d3xy);
    pF[0] = d1xy.y;
    pF[1] = d2xy.y;
    pF[2] = d3xy.y;
    pF[3] = d1xy.Magnitude ();
    }
bvector<DPoint3d> summedD1Y;    // should match Y
bvector<DPoint3d> summedD2Y;    // shoudl match d1Y
bvector<DPoint3d> summedD3Y;    // should match d2Y
bvector<DPoint3d> summedDistance;
bool AnnounceIntermediateIntegral (double t, double *pIntegrals) override
    {
    double x = t * m_tangentLength;
    summedD1Y.push_back (DPoint3d::From (x, pIntegrals[0]));
    summedD2Y.push_back (DPoint3d::From (x, pIntegrals[1]));
    summedD3Y.push_back (DPoint3d::From (x, pIntegrals[2]));
    summedDistance.push_back (DPoint3d::From (x, pIntegrals[3]));
    return true;
    }
};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,HalfCosineIntegrals)
    {
    bvector<DPoint3d> exactXY, exactD1, exactD2, exactD3;
    double tangentLength = 200.0;
    double r1 = 800.0;
    DPoint2d xy;
    DVec2d d1xy, d2xy, d3xy;
    bvector<HalfCosineIntegrands> allIntegrands;


    BSIQuadraturePoints quadraturePoints;
    quadraturePoints.InitGauss (5);
    double errorBound;

    for (int numInterval : {16, 8, 4})
        {
        allIntegrands.push_back (HalfCosineIntegrands (tangentLength, r1));
        quadraturePoints.IntegrateWithRombergExtrapolation (allIntegrands.back (), 0.0, 1.0, numInterval, errorBound);
        }
    double df = 1.0 / 16.0;
    for (double f = 0.0; f < 1.0 + df * 0.00000001; f += df)
        {
        double x = f * tangentLength;
        DSpiral2dDirectHalfCosine::EvaluateAtFractionInStandardOrientation (
            f, tangentLength, r1,
            xy, &d1xy, &d2xy, &d3xy);
        exactXY.push_back (DPoint3d::From (x, xy.y));
        exactD1.push_back (DPoint3d::From (x, d1xy.y));
        exactD2.push_back (DPoint3d::From (x, d2xy.y));
        exactD3.push_back (DPoint3d::From (x, d3xy.y));
        }

    double dy = 80.0;
    double dy1 = 2.0;
    double dx = tangentLength;
    {
    SaveAndRestoreCheckTransform shifter (0, dy, 0);
    Check::SaveTransformed (DSegment3d::From (0,0,0, dx, 0, 0));
    Check::SaveTransformed (exactXY);
    double y0 = exactXY[0].y;
    Check::Shift (0,y0,0);
    for (auto &integrands : allIntegrands)
        {
        Check::Shift (0,dy1,0);
        Check::SaveTransformed (integrands.summedD1Y);
        Check::Near (exactXY.back ().y, integrands.summedD1Y.back ().y + y0, "Integrated Y");
        }
    }

    {
    SaveAndRestoreCheckTransform shifter (0, dy, 0);
    Check::SaveTransformed (DSegment3d::From (0,0,0, dx, 0, 0));
    Check::SaveTransformed (exactD1);
    Check::Shift (0,exactD1[0].y,0);
    double y0 = exactD1[0].y;
    Check::Shift (0,y0,0);
    for (auto &integrands : allIntegrands)
        {
        Check::Shift (0,dy1,0);
        Check::SaveTransformed (integrands.summedD2Y);
        Check::Near (exactD1.back ().y, integrands.summedD2Y.back ().y + y0, "Integrated DY");
        }
    }

    {
    SaveAndRestoreCheckTransform shifter (0, dy, 0);
    Check::SaveTransformed (DSegment3d::From (0,0,0, dx, 0, 0));
    Check::SaveTransformed (exactD2);
    double y0 = exactD2[0].y;
    Check::Shift (0,y0,0);
    for (auto &integrands : allIntegrands)
        {
        Check::Shift (0,dy1,0);
        Check::SaveTransformed (integrands.summedD3Y);
        Check::Near (exactD2.back ().y, integrands.summedD3Y.back ().y + y0, "Integrated D2Y");
        }
    }

    {
    SaveAndRestoreCheckTransform shifter (0, dy,0);
    Check::SaveTransformed (DSegment3d::From (0,0,0, 2 * dx, 0, 0));
    Check::SaveTransformed (exactD3);
    }

    {
    SaveAndRestoreCheckTransform shifter (0, dy, 0);
    Check::SaveTransformed (DSegment3d::From (0,0,0, dx, 0, 0));
    // expect integrals to be a little above this.
    Check::SaveTransformed (DSegment3d::From (0,0,0, dx, dx, 0));
    DVec3d delta1 = DVec3d::From (0, -dy1, 0);
    DVec3d delta2 = DVec3d::From (dy1, -dy1, 0);

    for (auto &integrands : allIntegrands)
        {
        Check::Shift (0,dy1,0);
        auto xyz = integrands.summedDistance.back ();
        Check::SaveTransformed (bvector<DPoint3d> {xyz, xyz + delta1, xyz + delta2});
        Check::SaveTransformed (integrands.summedDistance);
        }
    }

    Check::ClearGeometry ("Spiral.HalfCosineIntegrals");
    }


// Vector integrands for testing
struct WesternAustraliaDistanceTestIntegrands: BSIIncrementalVectorIntegrand
{
double m_l1;    // nominal length
double m_r1;    // end radius

WesternAustraliaDistanceTestIntegrands (double l1, double r1) :
    m_l1 (l1),
    m_r1 (r1)
    {
    }
DPoint3d SToXY (double s, int numTerms = 1)
    {
    double s2 = s * s;
    double s4 = s2 * s2;
    double RL = m_r1 * m_l1;
    auto xyz = DPoint3d::From (s * ( 1.0 - s4 / (40.0 * RL * RL)), s * s2 / (6.0 * RL));
    if (numTerms > 1)
        {
        double s6 = s4 * s2;
        double R3L3 = RL * RL * RL;
        xyz.y -= s6 * s / (48.0 * 7.0 * R3L3);
        xyz.x += s6 * s * s2 / (9.0 * 24.0 * 16.0 * R3L3 * RL);
        }
    return xyz;
    }

DVec2d SToDXY (double s)
    {
    double s2 = s * s;
    double s4 = s2 * 2;
    double RL = m_r1 * m_l1;
    return DVec2d::From (1.0 - s4 / (8.0 * RL * RL), s2 / (2.0 * RL));
    }
int GetVectorIntegrandCount () override { return 4;}
void EvaluateVectorIntegrand (double t, double *pF) override
    {
    double x = t * m_l1;
    DVec2d Duv = SToDXY (x);
    double theta = 0.5 * x * x  / (m_l1 * m_r1);
    double dx = cos(theta) * m_l1;
    double dy = sin(theta) * m_l1;
    pF[0] = dx;
    pF[1] = dy;
    pF[2] =  Duv.Magnitude () * m_l1;   // integrated distance along the approximate curve
    pF[3] = sqrt (dx * dx + dy * dy);   // integrated distance along true curve.
    }

bvector<DPoint3d> distanceError;
bvector<DPoint3d> clothoidXY;
bvector<DPoint3d> series1XY;
bvector<DPoint3d> series2XY;


bool AnnounceIntermediateIntegral (double t, double *pIntegrals) override
    {
    double s = t * m_l1;
    DPoint3d XY1 = SToXY (s);
    DPoint3d XY2 = SToXY (s, 2);
    // <requestedDistance, relative distance error for 1 term, relative distance error for 2 term>
    distanceError.push_back (DPoint3d::From (s, (pIntegrals[2] - s) / m_l1, (pIntegrals[3] - s) / m_l1));
    series1XY.push_back (XY1);   // approximate curve
    series2XY.push_back (XY2);
    clothoidXY.push_back (DPoint3d::From (pIntegrals[0], pIntegrals[1])); // true curve
    return true;
    }
};
// extend points by adding a point at (dx,dy) from the last point.
// (If several linestrings are very close to one another, adding strokes with different dx,dy makes them easier to mouse-pick)
void AppendVector (bvector<DPoint3d> &points, double dx, double dy)
    {
    if (points.size () > 0)
        {
        DPoint3d xyz = points.back ();
        xyz.x += dx;
        xyz.y += dy;
        points.push_back (xyz);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  07/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,WesternAustralianDistanceErrors)
    {
    BSIQuadraturePoints quadraturePoints;
    quadraturePoints.InitGauss (5);
    double errorBound;
    uint32_t numInterval = 16;
    Check::StartScope ("Series Approximations for Clothoid");
    for (double l1 : { 100, 200, 400})
        {
        Check::StartScope ("L", l1);
        double displayShift = 1.25 * l1;
        SaveAndRestoreCheckTransform shifter (displayShift, 0, 0);
        for (double r1 : { 600, 800, 1000})
            {
            Check::StartScope ("R1", r1);
            SaveAndRestoreCheckTransform shifter (0, displayShift , 0);
            WesternAustraliaDistanceTestIntegrands integrands (l1, r1);
            quadraturePoints.IntegrateWithRombergExtrapolation (integrands, 0.0, 1.0, numInterval, errorBound);
            DPoint3d xyz = integrands.clothoidXY.back ();
            DPoint3d xyz0 = DPoint3d::From (xyz.x, 0, 0);

            AppendVector (integrands.clothoidXY, 1,0);
            AppendVector (integrands.series1XY, 1,1);
            AppendVector (integrands.series2XY, 1,2);

            Check::SaveTransformed (integrands.distanceError);
            Check::SaveTransformed (integrands.clothoidXY);
            Check::SaveTransformed (integrands.series1XY);
            Check::SaveTransformed (integrands.series2XY);
            Check::SaveTransformed (DSegment3d::From (xyz, xyz0));
            double d1 = integrands.distanceError.back ().y;    // integration of tangent lengths for 1-term sine.
            double d2 = integrands.distanceError.back ().z;     // integrationi of tangent lengths for 2-term sine.
            if (l1 < 0.5 * r1)  // Don't check if obviously too long a spiral
                Check::LessThanOrEqual (d1, 0.008, "1-term sine approximation erorr limit");
            Check::LessThanOrEqual (d2, 1.0e-8, "2-term sine approximation erorr limit");
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    Check::EndScope ();
    Check::ClearGeometry ("Spiral.WesternAustralianDistanceErrors");
    }

void Claude_DoCubicParabolaSpiralCheck(int type, int ctorSelect, double startRadius, double endRadius)
    {
    const double length = 100;
    double startAngle = 0;
    DPoint3d  startPoint = DPoint3d::From (0,0,0);
    Transform frame = Transform::FromIdentity ();
    ICurvePrimitivePtr spiral  = nullptr;
    if (ctorSelect == 1)
        spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius(type, startPoint, startAngle, startRadius, length, endRadius);
    else 
        spiral = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius(type,
         startAngle, startRadius, length, endRadius, frame, 0, 1);
    Check::SaveTransformed (*spiral);
    CurveLocationDetail sLocDetail;
    for (double distanceAlong : {10, 20, 30, 40, 50, 60, 70, 80, 90})
        {
        spiral->PointAtSignedDistanceFromFraction(0, distanceAlong, true, sLocDetail);
        double curvature, torsion;
        Transform frame;
        spiral->FractionToFrenetFrame(sLocDetail.fraction, frame, curvature, torsion);
        DPoint3d origin = frame * DPoint3d::From (0,0,0);
        Check::SaveTransformedMarker (origin, -0.1);
        if (fabs (curvature) > 1.0e-5)
            {
            DPoint3d center = frame * DPoint3d::From (0,1/curvature,0);
            Check::SaveTransformed (DSegment3d::From (origin, center));
            }
        }    
    static double tolerance = 0.001;
    IFacetOptionsPtr opt = IFacetOptions::CreateForCurves();
    opt->SetChordTolerance(tolerance);
    DPoint3dDoubleUVCurveArrays strokes;

    spiral->AddStrokes(strokes, *opt, 0, 1);
    double dy = 20;
    Check::Shift (0, -dy,0);
    Check::SaveTransformed(strokes.m_xyz);
    
    Check::Shift (200,dy,0);
    }
TEST(Spiral,IntermdediateCurvature)
    {
    for (int type :{DSpiral2dBase::TransitionType_AustralianRailCorp})
        {
        for (int select : {1})
            {
            SaveAndRestoreCheckTransform shifter (1000,0,0);
            Claude_DoCubicParabolaSpiralCheck (type, select, 500, 0);
            Claude_DoCubicParabolaSpiralCheck (type, select, 400, 0);
            Claude_DoCubicParabolaSpiralCheck (type, select, 0, 500);
            }
        }
    Check::ClearGeometry ("Spiral.IntermdediateCurvature");
    }