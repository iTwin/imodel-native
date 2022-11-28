/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
* @bsimethod
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
                DSpiral2dBase::TransitionType_Viennese,
                //DSpiral2dBase::TransitionType_WeightedViennese,
                DSpiral2dBase::TransitionType_WesternAustralian,
                DSpiral2dBase::TransitionType_Czech,
                DSpiral2dBase::TransitionType_AustralianRailCorp,
                DSpiral2dBase::TransitionType_Italian,
                DSpiral2dBase::TransitionType_MXCubicAlongArc,
                DSpiral2dBase::TransitionType_DirectHalfCosine,
                DSpiral2dBase::TransitionType_ChineseCubic,
                DSpiral2dBase::TransitionType_JapaneseCubic,
                DSpiral2dBase::TransitionType_Arema,
                DSpiral2dBase::TransitionType_PolishCubic
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
        snprintf (fileName, sizeof(fileName), "Spiral.Serialize.%d%s", 1000+(int)spiralType, typeName.c_str());
        Check::ClearGeometry (fileName);
        }
    }

/*---------------------------------------------------------------------------------**//**
                                                                                      * @bsimethod
                                                                                      +---------------+---------------+---------------+---------------+---------------+------*/
TEST (PseudoSpiral, SerializeB)
    {
    auto options = IFacetOptions::CreateForCurves ();
    double strokeShiftX = 200.0;
    double strokeShiftY = 0;
    double startY = -60.0;
    double yShift = 10.0;
    // preshift to avoid direct overlap with primary data ....
    Check::Shift (0, startY, 0);
    int numViennese = 0;
    for (int spiralType : {
        DSpiral2dBase::TransitionType_Clothoid,
            DSpiral2dBase::TransitionType_Bloss,
            DSpiral2dBase::TransitionType_Biquadratic,
            DSpiral2dBase::TransitionType_Cosine,
            DSpiral2dBase::TransitionType_Sine,
            DSpiral2dBase::TransitionType_Viennese,         // ZERO CANT
            DSpiral2dBase::TransitionType_Viennese,         // TRICKY TRICKY -- this one gets nonzero cant. (1x)
            DSpiral2dBase::TransitionType_Viennese,         // TRICKY TRICKY -- this one gets nonzero cant. (2x)
            DSpiral2dBase::TransitionType_Viennese,         // TRICKY TRICKY -- this one gets nonzero cant. (4x)
            DSpiral2dBase::TransitionType_Viennese,         // TRICKY TRICKY -- this one gets nonzero cant. (9x)
            //DSpiral2dBase::TransitionType_WeightedViennese,
            DSpiral2dBase::TransitionType_WesternAustralian,
            DSpiral2dBase::TransitionType_Czech,
            DSpiral2dBase::TransitionType_AustralianRailCorp,
            DSpiral2dBase::TransitionType_Italian,
            DSpiral2dBase::TransitionType_MXCubicAlongArc,
            DSpiral2dBase::TransitionType_DirectHalfCosine,
            DSpiral2dBase::TransitionType_ChineseCubic,
            DSpiral2dBase::TransitionType_JapaneseCubic,
            DSpiral2dBase::TransitionType_PolishCubic,
            DSpiral2dBase::TransitionType_Arema,
        })
        {
        Check::SetTransform (Transform::FromIdentity ());
        if (spiralType == DSpiral2dBase::TransitionType_Viennese)
            numViennese++;
        for (auto bundle : bvector<DPoint3d> {
            DPoint3d::From (0,-1000, 100),
            DPoint3d::From (0,-500,20),
            DPoint3d::From (1000,0,100),
            DPoint3d::From (-500,0,20),
            DPoint3d::From (500,400,20),
            DPoint3d::From (0,800,200),
            })
            {
            SaveAndRestoreCheckTransform shifter (0, yShift, 0);
            double radius0 = bundle.x;
            double radius1 = bundle.y;
            double curvature0, curvature1;
            DoubleOps::SafeDivide (curvature0, 1.0, radius0, 0.0);
            DoubleOps::SafeDivide (curvature1, 1.0, radius1, 0.0);
            double spiralLength  = bundle.z;
            double bearing0 = 0.0;
            ICurvePrimitivePtr spiral;
            if (spiralType == DSpiral2dBase::TransitionType_Viennese)
                {
                static double s_cantFactor = 0.05;      // Make cant a small fraction of actual curvature change
                double curvatureSum = fabs (curvature0) + fabs (curvature1);
                double q = numViennese - 1;
                double cant = curvatureSum * q * q * s_cantFactor;
                double e = 1.0;
                double h = 1.0;
                bvector<double>extraData;
                DSPiral2dViennese::FillExtraDataArray (extraData, cant, h, e);
                DSpiral2dBaseP xySpiral = DSpiral2dBase::Create (spiralType, extraData);
                if (xySpiral == nullptr)
                    continue;
                xySpiral->SetBearingCurvatureLengthCurvature(bearing0, curvature0, spiralLength, curvature1);
                spiral = ICurvePrimitive::CreateSpiral(*xySpiral, Transform::FromIdentity(), 0.0, 1.0);
                delete xySpiral;
                }
            else if (spiralType < 50)
                spiral = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (
                    spiralType,
                    bearing0, curvature0, spiralLength, curvature1,
                    Transform::FromIdentity (),
                    0.0, 1.0
                );
            else
                spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (spiralType,
                    DPoint3d::From (0, 0, 0), bearing0, radius0, spiralLength, radius1);


            if (spiral.IsValid ())
                {
                Check::SaveTransformed (*spiral);
                auto bcurve = spiral->GetProxyBsplineCurvePtr ();
                Check::Shift (strokeShiftX, strokeShiftY, 0);
                if (bcurve.IsValid ())
                    {
                    auto bprim = ICurvePrimitive::CreateBsplineCurve (bcurve);
                    Check::SaveTransformed (*bprim);
                    }
                // This will closely match samples from SS3
                size_t numStrokes = spiralLength < 30 ? 8 : 16;
                bvector<DPoint3d> strokes;
                for (size_t i = 0; i <= numStrokes; i++)
                    {
                    DPoint3d xyz;
                    spiral->FractionToPoint ((double)i/(double)numStrokes, xyz);
                    strokes.push_back (xyz);
                    }
                int errors = 0;
                if (strokes.size () == 0)
                    errors++;
                else
                    {
                    for (auto &xyz : strokes)
                        {
                        if (BeNumerical::BeIsnan (xyz.x) || BeNumerical::BeIsnan(xyz.y) || BeNumerical::BeIsnan(xyz.z))
                            errors++;
                        }
                    }
                Check::Shift (strokeShiftX, strokeShiftY, 0);
                if (errors == 0)
                    Check::SaveTransformed (strokes);
                }               
            }
        char fileName[1024];
        Utf8String typeName;
        DSpiral2dBase::TransitionTypeToString (spiralType, typeName);
        // add 1000 to ensure alphabetic sort maintains numeric order
        if (spiralType == DSpiral2dBase::TransitionType_Viennese)
            snprintf(fileName, sizeof(fileName), "Spiral.SerializeSetB.%d%s%d", 1000 + (int)spiralType, typeName.c_str(), numViennese);
        else
            snprintf (fileName, sizeof(fileName), "Spiral.SerializeSetB.%d%s", 1000 + (int)spiralType, typeName.c_str ());
        Check::ClearGeometry (fileName);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(PseudoSpiral,ExerciseDistances)
    {
    //bvector<double> pseudoLengthMeters {100.02506262460328, 100.15873011778872, 100.66666663992606};
    //bvector<double> edgeCount {15, 25, 35};
    double smallYShift = 1.0;
    auto options = IFacetOptions::CreateForCurves ();
    bvector<double> breakFractions {0.25, 0.54, 0.98, 1.0}; // implied 0 at start
    for (int spiralType : {
        DSpiral2dBase::TransitionType_PolishCubic,
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
                DSpiral2dBase::TransitionType_ChineseCubic,
                DSpiral2dBase::TransitionType_PolishCubic,
                DSpiral2dBase::TransitionType_Arema,
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
                    for (double bearing0 : {Angle::DegreesToRadians (135.0), 0.0,  Angle::DegreesToRadians (10), Angle::DegreesToRadians (135)})
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
        snprintf (fileName, sizeof(fileName), "Spiral.Exercise.%d%s", 1000+(int)spiralType, typeName.c_str());
        Check::ClearGeometry (fileName);
        Check::EndScope();
        }
    }


   
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        SaveAndRestoreCheckTransform shifter1 (2.0 * length0B, 0,0);
        for (double fraction : {0.0, 0.25, 0.5, 0.75})
            {
            SaveAndRestoreCheckTransform shifter2 (0, yShiftB, 0);

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
        SaveAndRestoreCheckTransform shifter1 (2.0 * lengthAB, 0, 0);
        for (double radiusB : {1000.0, 0.0})
            {
            SaveAndRestoreCheckTransform shifter2 (0, yShiftB, 0);
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
                delete baseSpiral;
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
        SaveAndRestoreCheckTransform shifter1 (2.0 * length0B, 0,0);
        for (double fraction : {0.0, 0.25, 0.5, 0.75})
            {
            SaveAndRestoreCheckTransform shifter2 (0, yShiftB, 0);

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
        SaveAndRestoreCheckTransform shifter1 (0.0, gridStep, 0);
        for (double radiusB : radiiB)
            {
            SaveAndRestoreCheckTransform shifter2 (gridStep, 0, 0);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,MixedRadiusConstructionsA)
    {
    // mixed radius constructions with varying magnitudes.
    bvector<double> radii {100.0, 50.0, 0.0, -50.0, -100.0};
    testRadiusCombinations (radii, radii);
    Check::ClearGeometry ("Spiral.MixedRadiusConstructionsA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
ValidatedDouble (*inverseMap)(double, double, double, double, double &),
bvector<DPoint3d> &xyFromS
)
    {
    UsageSums positiveSums, negativeSums;
    UsageSums positiveRoundTripErrors, negativeRoundTripErrors;
    bvector<double> positiveFailures, negativeFailures;

    for (double i = 0; i <= numSamples; i++)
        {
        double u = DoubleOps::Interpolate (-L, i / numSamples, L);
        double coffPositive;
        auto fPositive = inverseMap (1.0, R, L, u, coffPositive);
        if (fPositive.IsValid ())
            {
            positiveSums.Accumulate (ClothoidCosineApproximation::s_evaluationCount);
            auto g = forwardMap (-1.0, R, L, fPositive.Value ());
            positiveRoundTripErrors.Accumulate (u - g);
            }
        else
            positiveFailures.push_back (u);
        double coffNegative;
        auto fNegative = inverseMap (-1.0, R, L, u, coffNegative);
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
* @bsimethod
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
    // unused - DVec3d dYA = (X[1] - X[0]) / df;
    // unused - DVec3d dYB = (X[2] - X[1]) / df;
    // unused - DVec3d ddYAB = (dYB - dYA) / df;
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
* @bsimethod
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
        DSpiral2dBase::TransitionType_Czech,
        DSpiral2dBase::TransitionType_JapaneseCubic,
        DSpiral2dBase::TransitionType_Italian,
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,AustralianReversal)
    {
    double R1 = 0.0;
    double L = 20.0;
    double tolFactor = 1000.0;      // aussRail is painfully grainy & does not match derivatives.
    for (double R0 : {-350.0, -200.0, -100.0, 100.0, 200.0, 350.0})
        {
        SaveAndRestoreCheckTransform shifter1 (0, 10.0 * L,0);
        for (double beta0 : {0.0, 1.0, 3.0, 4.74, 6.0 })
            {
            SaveAndRestoreCheckTransform shifter2 (10.0 * L,0,0);
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
        SaveAndRestoreCheckTransform shifter1 (displayShift, 0, 0);
        for (double r1 : { 600, 800, 1000})
            {
            Check::StartScope ("R1", r1);
            SaveAndRestoreCheckTransform shifter2 (0, displayShift , 0);
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
void RadiansToDMSString (double radians, Utf8String &string)
    {
    char buffer[1024];
    double s = radians >= 0.0 ? 1.0 : -1.0;
    double degrees = Angle::RadiansToDegrees (fabs (radians));
    int     iDegrees = (int)floor( degrees);
    double minutes = 60.0 * (degrees - iDegrees);
    int    iMinutes = (int)floor (minutes);
    double seconds = 60.0 * (minutes - iMinutes);
    snprintf (buffer, sizeof(buffer), "[%c%d deg   %d min   %6.3lf] [%.10lg deg]", s > 0.0 ? '+' : '-', iDegrees, iMinutes, seconds, degrees);
    string = buffer;
    }
void PrintTangents (DVec3dCR chordAB, double bearingChangeAB, double startCurvature, double endCurvature)
    {
    double shortTangentOUT = chordAB.x - chordAB.y / tan (bearingChangeAB);
    double shortTangentIn  = chordAB.y / sin (bearingChangeAB);
    double longTangentOUT = chordAB.y / sin (bearingChangeAB);
    double longTangentIN  = chordAB.x - shortTangentIn * cos (bearingChangeAB);
    bool outGoing = fabs(endCurvature) < fabs (startCurvature);
    if (outGoing)
        printf ("   OUT (Long,Short) tangents (%.6lg, %6lg)\n", longTangentOUT, shortTangentOUT);
    else
        printf ("    IN (Long,Short) tangents (%.6lg, %6lg)\n", longTangentIN, shortTangentIn);
    }

void Claude_DoCubicParabolaSpiralCheck(int type, int ctorSelect, double startBearing, double startRadius, double endRadius, double length)
    {
    DPoint3d  startPoint = DPoint3d::From (0,0,0);
    RotMatrix localAxes = RotMatrix::FromAxisAndRotationAngle (2, startBearing);
    Transform frameA = Transform::FromIdentity ();
    ICurvePrimitivePtr spiral  = nullptr;
    if (ctorSelect == 1)
        spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius(type, startPoint, startBearing, startRadius, length, endRadius);
    else 
        spiral = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius(type,
         startBearing, startRadius, length, endRadius, frameA, 0, 1);
    Utf8String typeName;
    DSpiral2dBase::TransitionTypeToString (type, typeName);

    printf (" *** SPIRAL TYPE %s\n", typeName.c_str ());

    printf ("  Entrance Bearing:(requested)                 %.6lg\n", startBearing);
    printf ("  Entrance Radius:(requested)                 %.3lf\n", startRadius);
    printf ("      Exit Radius:(requested)                 %.3lf\n", endRadius);
    printf ("   nominal length:                            %.3lf\n", length);
    double startCurvature, endCurvature;
    DoubleOps::SafeDivide (startCurvature, 1.0, startRadius, 0.0);
    DoubleOps::SafeDivide (endCurvature, 1.0, endRadius, 0.0);
    Utf8String stringA1, stringB1, stringAB1, stringAC1;
    DVec3d tangentA, tangentB;
    DPoint3d pointA, pointB;
    spiral->FractionToPoint (0.0, pointA, tangentA);
    spiral->FractionToPoint (1.0, pointB, tangentB);
    double bearingA = atan2 (tangentA.y, tangentA.x);
    double bearingB = atan2 (tangentB.y, tangentB.x);
    RadiansToDMSString (bearingA, stringA1);
    RadiansToDMSString (bearingB, stringB1);
    RadiansToDMSString (bearingB- bearingA, stringAB1);
    DVec3d chordAB = pointB - pointA;
    DVec3d chordABLocal = chordAB * localAxes;
    double evaluatedLength;
    spiral->Length (evaluatedLength);
    printf ("    Start Bearing:(on CurvePrimitive)                    %s\n", stringA1.c_str ());
    printf ("      End Bearing:(on CurvePrimitive)                    %s\n", stringB1.c_str());
    printf ("       Turn Angle:(on CurvePrimitive)                    %s\n", stringAB1.c_str());
    printf ("           Length:(on CurvePrimitive)                    %.6lg\n", evaluatedLength);
    printf ("  Start X,Y (on CurvePrimitive)   (%.8lg,%.8lg)\n", pointA.x, pointA.y);
    printf ("  Final X,Y (on CurvePrimitive)   (%.8lg,%.8lg)\n", pointB.x, pointB.y);
    printf ("  Local chord (on CurvePrimitive)   (%.8lg,%.8lg)\n", chordABLocal.x, chordABLocal.y);

    double bearingChangeAB = bearingB - bearingA;

    printf ("   Long and Short Tangents with angle from CurvePrimitive (arctan of actual bearing) %s\n", stringAB1.c_str());
    PrintTangents (chordAB, bearingChangeAB, startCurvature, endCurvature);
    printf ("    chord length    %.6lg\n", chordAB.Magnitude ());
    double radiusA1, radiusB1, curvatureA1, torsionA1, curvatureB1, torsionB1;
    Transform frameA1, frameB1;
    spiral->FractionToFrenetFrame (0.0, frameA1, curvatureA1, torsionA1);
    spiral->FractionToFrenetFrame (1.0, frameB1, curvatureB1, torsionB1);
    DoubleOps::SafeDivide (radiusA1, 1.0, curvatureA1, 0.0);
    DoubleOps::SafeDivide (radiusB1, 1.0, curvatureB1, 0.0);
    printf ("  Start and end radii: (on CurvePrimitive)      %.4lg   %.4lg\n", radiusA1, radiusB1);

    CurveLocationDetail curveLocationC;
    spiral->PointAtSignedDistanceFromFraction (0.0, length, true, curveLocationC);
    DPoint3d pointC;
    DVec3d   tangentC;
    spiral->FractionToPoint (curveLocationC.fraction, pointC, tangentC);
    double bearingC = atan2 (tangentC.y, tangentC.x);
    RadiansToDMSString (bearingC- bearingA, stringAC1);
    printf ("  fraction to nominal length %lf     tangent at nominal length %s\n", curveLocationC.fraction,stringAC1.c_str());

    if (startRadius == 0.0 && type == DSpiral2dBase::TransitionType_WesternAustralian)
        {
        printf ("  Final X,Y (evaluated)        %.6lf, %.6lf\n", pointB.x, pointB.y);
        double Lt = length;
        double Lt2 = Lt * Lt;
        double Lt5 = Lt2 * Lt2 * Lt;
        double perpDenominator = (6.0 * endRadius * Lt);
        double tangentDenominator = (40.0 * endRadius * endRadius * Lt2);
        double formulaPerpCoordinate = Lt * Lt * Lt / perpDenominator;
        double formulaTangentCoordinate = Lt - Lt5 / tangentDenominator;
        double formulaPerpDerivative = 3.0 * Lt2 / perpDenominator;
        double formulaTangentDerivative = 1.0 - 5.0 * Lt2 * Lt2 / tangentDenominator;
        double finalRadiansFromDerivative = atan2 (formulaPerpDerivative, formulaTangentDerivative);
        printf ("  Final X,Y (final point formulas)        %.6lg, %.6lg\n", formulaTangentCoordinate, formulaPerpCoordinate);
        double formulaFinalRadians = Lt * Lt / (2.0 * endRadius * Lt);
        Utf8String formulaFinalRadiansDMS, formulaFinalRadiansFromDerivativesDMS;
        RadiansToDMSString (formulaFinalRadians, formulaFinalRadiansDMS);
        RadiansToDMSString (finalRadiansFromDerivative, formulaFinalRadiansFromDerivativesDMS);
        printf ("   final point radian approximation formula   %s\n", formulaFinalRadiansDMS.c_str());
        printf ("   final point radian by arctan from perp, tangent derivatives %s\n", formulaFinalRadiansFromDerivativesDMS.c_str());
        printf ("   Long and Short Tangents with angle approximation bearignChange %s\n", formulaFinalRadiansDMS.c_str());
        PrintTangents (chordAB, formulaFinalRadians, startCurvature, endCurvature);
        }

    if (startRadius == 0.0
        && (type == DSpiral2dBase::TransitionType_AustralianRailCorp || (type == DSpiral2dBase::TransitionType_WesternAustralian))
        )
        {
        double Xc = chordAB.x;
        double root3 = sqrt(3.0);
        double phi = asin ((2.0 / root3) *
            cos (    (1.0 / 3.0) * acos ( -0.75 * root3 * (Xc / endRadius)) + Angle::DegreesToRadians (240.0)));
        Utf8String phiDMS;
        RadiansToDMSString (phi, phiDMS);
        double clothoidRadians = length * length / (2.0 * endRadius * length);
        Utf8String clothoidDMS;
        RadiansToDMSString (clothoidRadians, clothoidDMS);

        if (type == DSpiral2dBase::TransitionType_AustralianRailCorp)
            {
            printf (" RailCorp explicit PHI %s\n", phiDMS.c_str());
            printf (" ?? Tangents with clothoid angle (L/(2R)) (clothoid) and RailCorp final XY\n");
            PrintTangents (chordAB, clothoidRadians, startCurvature, endCurvature);
            }
        else
            {
            printf ("  Clothoid sweep %s  (%.10g)\n", clothoidDMS.c_str(), clothoidRadians);
            }
        }



    Check::SaveTransformed (*spiral);
    CurveLocationDetail sLocDetail;
    for (double distanceAlong : {10, 20, 30, 40, 50, 60, 70, 80, 90})
        {
        if (distanceAlong > length)
            break;
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
    DPoint3dDoubleUVCurveArrays strokeA;
    bvector<DPoint3d> strokeB;
    spiral->AddStrokes(strokeA, *opt, 0, 1);

    spiral->AddStrokes (strokeB, *opt, true, 0.0, 1.0);
    printf ("   (AddStrokes counts %d %d\n", (int)strokeA.m_xyz.size (), (int)strokeB.size ());

    double dy = 20;
    Check::Shift (0, -dy,0);
    Check::SaveTransformed(strokeA.m_xyz);
    
    Check::Shift (200,dy,0);
    }

double DMSToRadians (int degrees, int minutes, double seconds)
    {
    return Angle::DegreesToRadians ((double)(degrees + minutes/ 60.0 + seconds / 3600.0));
    }
TEST(Spiral,IntermdediateCurvature)
    {
    Claude_DoCubicParabolaSpiralCheck (DSpiral2dBase::TransitionType_WesternAustralian, 1, DMSToRadians (60,17,20.9), 0, 400, 40);
    Claude_DoCubicParabolaSpiralCheck (DSpiral2dBase::TransitionType_WesternAustralian, 1, DMSToRadians (60,17,20.888), 0, 400, 40);

    for (int type :{
            DSpiral2dBase::TransitionType_AustralianRailCorp,
            DSpiral2dBase::TransitionType_WesternAustralian,
            DSpiral2dBase::TransitionType_MXCubicAlongArc,
            DSpiral2dBase::TransitionType_Clothoid})
        {
        printf ("\n\n\n");
        SaveAndRestoreCheckTransform shifter (1000,0,0);
        printf ("\n\n (Paul Cusack tests)\n");
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 0, 300, 60);
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 0, 300, 60);
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 0, 400, 50.424);
        printf ("\n\n (Claude G tests)\n");
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 500, 0, 100);
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 400, 0, 100);
        printf ("\n\n\n");
        }
    Check::ClearGeometry ("Spiral.IntermdediateCurvature");
    }

TEST(Spiral,AlignmentChecks)
    {
    for (int type :{
            DSpiral2dBase::TransitionType_WesternAustralian,
            DSpiral2dBase::TransitionType_AustralianRailCorp
        })
        {
        SaveAndRestoreCheckTransform shifter (1000,0,0);
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 0, 500, 50.134);
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 0, 500, 50);
        Claude_DoCubicParabolaSpiralCheck (type, 1, 0.0, 400, 500, 50);

        }
    Check::ClearGeometry ("Spiral.AlignmentChecks");
    }

TEST(Spiral,PartialCubics)
    {
    const  double  endRadius = 400;
    const  double  startRadius = 500;
    const double length = 50;
    double startAngle = 0;
    double angleSS4 = 0.11343;

    DPoint3d  startPoint = DPoint3d::From (1000, 1000, 1000);
    ICurvePrimitivePtr spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius(DSpiral2dBase::TransitionType_AustralianRailCorp,
        startPoint, startAngle, startRadius, length, endRadius);

    CurveLocationDetail sLocDetail;
    spiral->PointAtSignedDistanceFromFraction(0, 0, true, sLocDetail);

    CurveLocationDetail eLocDetail;
    spiral->PointAtSignedDistanceFromFraction(0, 50, true, eLocDetail);

    DVec3d  dXDfStart;
    DPoint3d point;
    sLocDetail.TryFractionToPoint(point, dXDfStart);
    dXDfStart.Normalize();

    DVec3d  dXDfEnd;
    eLocDetail.TryFractionToPoint(point, dXDfEnd);
    dXDfEnd.Normalize();
    double angleAtNominalLength = fabs (dXDfEnd.AngleToXY(dXDfStart));

    double actualLength;
    spiral->Length (actualLength);
    DPoint3d xyz0, xyz1;
    DVec3d   tangent0, tangent1;
    spiral->FractionToPoint (0.0, xyz0, tangent0);
    spiral->FractionToPoint (1.0, xyz1, tangent1);
    double radians01 = tangent0.AngleToXY (tangent1);
    printf (" partial cubic (nominal length %.8g) (actual %.8g) (SS4 angle %.8g) (actual angle %.8g) (angleDelta %.8g)\n", 
                        length,
                        actualLength,
                        angleSS4,
                        radians01, radians01 - angleSS4
                        );

    printf ("        angle at nominal length %.8g (delta %.8g)\n", angleAtNominalLength, angleAtNominalLength - angleSS4);
//    ASSERT_TRUE((angle >= angleSS4 - tolerance && angle <= angleSS4 + tolerance)) << "Angle is not correct";

    Check::ClearGeometry ("Spiral.PartialCubics");
    }



// Vector integrands for testing if quadrature applied to derivatives reproduces primaries.
struct CurvePrimitiveTangentIntegrands : BSIIncrementalVectorIntegrand
{
ICurvePrimitiveCR m_cp;

CurvePrimitiveTangentIntegrands (ICurvePrimitiveCR &cp) : m_cp(cp)
    {
    }
int GetVectorIntegrandCount () override { return 9;}
void EvaluateVectorIntegrand (double t, double *pF) override
    {
    DPoint3d X;
    DVec3d  dX;
    DVec3d ddX;
    DVec3d dddX;
    m_cp.FractionToPoint (t, X, dX, ddX, dddX);
    pF[0] = dX.x;
    pF[1] = dX.y;
    pF[2] = dX.z;
    pF[3] = ddX.x;
    pF[4] = ddX.y;
    pF[5] = ddX.z;
    pF[6] = dddX.x;
    pF[7] = dddX.y;
    pF[8] = dddX.z;
    }
bvector<double> integralFraction;
bvector<DVec3d> integralOfDX;
bvector<DVec3d>  integralOfDDX;
bvector<DVec3d>  integralOfDDDX;


bool AnnounceIntermediateIntegral (double t, double *pIntegrals) override
    {
    integralFraction.push_back (t);
    integralOfDX.push_back (DVec3d::From (pIntegrals[0], pIntegrals[1], pIntegrals[2]));
    integralOfDDX.push_back (DVec3d::From (pIntegrals[3], pIntegrals[4], pIntegrals[5]));
    integralOfDDDX.push_back (DVec3d::From (pIntegrals[6], pIntegrals[7], pIntegrals[8]));
    return true;
    }

void GetFinalIntegrals (DVec3dR dX, DVec3dR ddX, DVec3dR dddX)
    {
    dX = integralOfDX.back ();
    ddX = integralOfDDX.back ();
    dddX = integralOfDDDX.back ();
    }
};


void CheckCCLSpiral (ICurvePrimitiveCR spiral, double radiusA, double radiusB, double length, double errorFactor = 1.0)
    {
    Transform frame0, frame1;
    double k0, k1;
    double t0, t1;
    // factors to loosen up tolerances
    double s_radiusFactor = 1.0e4 * errorFactor;
    double s_lengthFactor = 1.0e3 * errorFactor;
    double r0, r1, l01;
    if (Check::True (spiral.FractionToFrenetFrame (0.0, frame0, k0, t0))
        && Check::True (spiral.FractionToFrenetFrame (1.0, frame1, k1, t1))
        && Check::True (spiral.Length (l01)))
        {
        DoubleOps::SafeDivide (r0, 1.0, k0, 0.0);
        DoubleOps::SafeDivide (r1, 1.0, k1, 0.0);
        Check::Near (fabs (radiusA), r0, "entry radius", s_radiusFactor);
        Check::Near (fabs (radiusB), r1, "exit radius", s_radiusFactor);
        Check::Near (length, l01, "length", s_lengthFactor);
        }

    BSIQuadraturePoints quadraturePoints;
    quadraturePoints.InitGauss (5);

    CurvePrimitiveTangentIntegrands integrands (spiral);
    DPoint3d X0, X1;
    DVec3d   dX0, dX1, ddX0, ddX1, chord01A, chord01B, turn01A, turn01B, torque01A, torque01B;
    double errorBound;
    int numInterval = 8;
    quadraturePoints.IntegrateWithRombergExtrapolation (integrands, 0.0, 1.0, numInterval, errorBound);
    spiral.FractionToPoint (0.0, X0, dX0, ddX0);
    spiral.FractionToPoint (1.0, X1, dX1, ddX1);
    chord01A = X1 - X0;
    turn01A = dX1 - dX0;
    torque01A = ddX1 - ddX0;
    integrands.GetFinalIntegrals (chord01B, turn01B, torque01B);
    DVec3d chordErrorAB = chord01B - chord01A;
    DVec3d turnErrorAB = turn01B - turn01A;
    DVec3d torqueErrorAB = torque01B - torque01A;
    chordErrorAB.Magnitude ();  // to suppress unused var warning
    turnErrorAB.Magnitude ();   // to suppress unused var warning
    torqueErrorAB.Magnitude ();   // to suppress unused var warning
    Check::Near (chord01A, chord01B, "Chord integration");
    Check::Near (turn01A, turn01B, "turn integration");
    Check::Near (torque01A, torque01B, "torque integration");
    }
TEST(Spiral,IterateForPartialSpiralProperties)
    {
    double reversalStepY = 100.0;
    double ticSize = 3.0;
    for (int type :{
            DSpiral2dBase::TransitionType_WesternAustralian,
            DSpiral2dBase::TransitionType_AustralianRailCorp,
            DSpiral2dBase::TransitionType_ChineseCubic,
            DSpiral2dBase::TransitionType_MXCubicAlongArc,
            DSpiral2dBase::TransitionType_Czech,
            DSpiral2dBase::TransitionType_Arema,
            DSpiral2dBase::TransitionType_PolishCubic
        })
        {
        SaveAndRestoreCheckTransform shifter1 (0, 6000, 0);
        double errorFactor = 1.0;
        if (type == DSpiral2dBase::TransitionType_AustralianRailCorp)
            errorFactor = 5000.0;
        for (DVec3d bundle : {
            DVec3d::From (0, 1000, 100),
            DVec3d::From (1000, 500, 50),
            DVec3d::From (1000, 500, 35),
            DVec3d::From (1000, 800, 50),
            DVec3d::From (400, 500, 35)
            })
            {
            SaveAndRestoreCheckTransform shifter2 (3000, 0, 0);
            for (double signX : {1.0, -1.0})
                {
                for (double signY : {1.0, -1.0})
                    {
                    SaveAndRestoreCheckTransform shifter3 (0, 1000, 0);
                    double radiusA = bundle.x * signX;
                    double radiusB = bundle.y * signY;
                    double requestedLength = bundle.z;

                    Check::SaveTransformed (bvector<DPoint3d> {
                            DPoint3d::From (requestedLength, 0, 0),
                            DPoint3d::From (0,0,0),
                            DPoint3d::From (0, reversalStepY,0)}
                            );

                    auto spiral = ICurvePrimitive::CreatePseudolSpiralWithTrueRadiusLengthRadius (type, DPoint3d::From (0,0,0), 0.0, radiusA, requestedLength, radiusB);
                    if (Check::True (spiral.IsValid ()))
                        {
                        double lengthAB;
                        spiral->Length (lengthAB);
                        //printf (" Requested (rA, rB, length) (%.4g, %.4g, %.4g) (actual length %.10g)\n", radiusA, radiusB, requestedLength, lengthAB);
                        //auto placement = spiral->GetSpiralPlacementCP ();
                        //printf ("       (Fractions %.15g, %.15g)\n", placement->fractionA, placement->fractionB);
                        Check::SaveTransformed (*spiral);
                        DPoint3d xyz0;
                        spiral->FractionToPoint (0.0, xyz0);
                        Check::SaveTransformedMarker (xyz0, -ticSize);
                        CheckCCLSpiral (*spiral, radiusA, radiusB, requestedLength, errorFactor);
                        }
                    else
                        {
                        printf ("\n\nFORWARD  (%d) Spiral search failed: (R0 R1 L) %g %g %g\n", type, radiusA, radiusB, requestedLength);
                        Check::SaveTransformedMarker (DPoint3d::From (0,0,0), -ticSize);
                        }

                    auto reversedSpiral = ICurvePrimitive::CreatePseudolSpiralWithTrueRadiusLengthRadius (type, DPoint3d::From (0,0,0), 0.0, radiusB, requestedLength, radiusA);
                    Check::Shift (0, reversalStepY, 0);
                    if (Check::True (reversedSpiral.IsValid ()))
                        {
                        //auto reversedPlacement = spiral->GetSpiralPlacementCP ();
                        //printf ("       (Reversed Fractions %.15g, %.15g)\n", reversedPlacement->fractionA, reversedPlacement->fractionB);
                        Check::SaveTransformed (*reversedSpiral);
                        DPoint3d xyz0;
                        reversedSpiral->FractionToPoint (0.0, xyz0);
                        Check::SaveTransformedMarker (xyz0, -ticSize);
                        CheckCCLSpiral (*reversedSpiral, radiusB, radiusA, requestedLength, errorFactor);
                        }
                    else
                        {
                        printf ("\n\nREVERSED (%d) Spiral search failed: (R0 R1 L) %g %g %g\n", type, radiusB, radiusA, requestedLength);
                        Check::SaveTransformedMarker (DPoint3d::From (0,0,0), -ticSize * 2.0);
                        }
                    }
                }
            }
        }
    Check::ClearGeometry ("Spiral.IterateForPartialSpiralProperties");
    }


TEST(Spiral,IterateForNSWProperties)
    {
    for (DVec3d bundle : {
                DVec3d::From (0,-1000, 100),
                DVec3d::From (0,-500,20),
                DVec3d::From (1000,0,100),
                DVec3d::From (-500,0,20),
                DVec3d::From (-500,-400,20)
        //DVec3d::From (2000, 1000, 100),
        //DVec3d::From (1000, 500, 100),
        //DVec3d::From (1000, 500, 100),
        //DVec3d::From (1000, 800, 50),
        //DVec3d::From (500, 400, 20),
        //DVec3d::From (500, 400, 30),
        //DVec3d::From (500, 400, 40),
        //DVec3d::From (500, 400, 50)
        })
        {
        SaveAndRestoreCheckTransform shifter (0, 10, 0);
        double radius0 = bundle.x;
        double radius1 = bundle.y;
        double requestedLength = bundle.z;
        auto spiral = ICurvePrimitive::CreateAustralianRailCorpBearingRadiusLengthRadius (DPoint3d::From (0,0,0),
                0.0,
                radius0,
                requestedLength,
                radius1
                );
        if (spiral.IsValid ())
            {
            double actualLength;
            spiral->Length (actualLength);
            printf ("   (requested, actual length) (%.10g, %.10g)\n", requestedLength, actualLength);
            Check::SaveTransformed (*spiral);
            }
        }
    Check::ClearGeometry ("Spiral.IterateForNSWProperties");
    }

TEST(Spiral,HugeTurn)
    {
    const double length = 50;
    const double startAngle = 0;
    double  startRadius = 0.0;
    DPoint3d  startPoint = DPoint3d::From (1000, 1000, 1000);
    bvector<double> radiusFractions {2.0, 1.0, 0.75, 0.60, 0.51, 0.49, 0.4, 0.2};
    for (double endRadiusFraction : radiusFractions)
        {
        double endRadius = endRadiusFraction * length;
        auto spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (DSpiral2dBase::TransitionType_Czech,
            startPoint, startAngle, startRadius, length, endRadius);
        if (2 * endRadius < length)
            Check::False (spiral.IsValid ());
        if (spiral.IsValid ())
            {
            auto bcurve = spiral->GetProxyBsplineCurvePtr ();
            if (bcurve.IsValid ())
                {
                Check::SaveTransformed (*bcurve);
                }
            }
        // ASSERT_TRUE (spiral.IsNull ());
        }

    Check::Shift (100,0,0);
    for (double endRadiusFraction : radiusFractions)
        {
        double endRadius = endRadiusFraction * length;
        auto spiral = ICurvePrimitive::CreatePseudolSpiralWithTrueRadiusLengthRadius (DSpiral2dBase::TransitionType_Czech,
            startPoint, startAngle, startRadius, length, endRadius);
        if (2 * endRadius < length)
            Check::False (spiral.IsValid ());
        if (spiral.IsValid ())
            {
            auto bcurve = spiral->GetProxyBsplineCurvePtr ();
            if (bcurve.IsValid ())
                {
                Check::SaveTransformed (*bcurve);
                }
            }
        }
    Check::ClearGeometry ("Spiral.HugeTurn");
    }

TEST (Spiral, WesternAustralianRoundTripCase)
    {
    double startRadius = 500;
    double endRadius = 150;
    double startAngle = 0;
    DPoint3d startPoint = DPoint3d::From (0, 0, 0);
    double length1;
    for (int scale : {1, 10000})
        {
        for (double length : {25, 50, 70, 75, 100, 100, 100})
            {
            ICurvePrimitivePtr spiral = ICurvePrimitive::CreatePseudolSpiralWithTrueRadiusLengthRadius (DSpiral2dBase::TransitionType_AustralianRailCorp,
                startPoint, startAngle * scale, startRadius * scale, length * scale, endRadius * scale);
            if (spiral.IsValid ())
                {
                Check::SaveTransformed (*spiral);
                spiral->Length (length1);
                printf (" (RRL %lg %lg %lg   L1 %lg\n", startRadius, endRadius, length, length1);
                for (double fraction : {0.0, 0.25, 0.75, 1.0})
                    {
                    Transform frame;
                    double curvature, torsion;
                    spiral->FractionToFrenetFrame (fraction, frame, curvature, torsion);
                    printf ("  (f %lg)  (k %lg) (r %lg)\n", fraction, curvature,
                                    curvature == 0.0 ? 0.0 : 1.0 / curvature);
                    }
                }
            }
        char filename[1024];
        snprintf (filename, sizeof(filename), "Spiral.WesternAustralianRoundTripCase%d", scale);
        Check::ClearGeometry (filename);
        }
    }


TEST(Spiral, PolishHello)
    {
    double maxErr = DSpiral2dPolish::ValidateSeriesInversion();
    Check::LE_Toleranced (maxErr, 1.0e-14 * 100.0);
    }
void ShowProps(char const * name, ICurvePrimitiveR curve, double fraction, DPoint3dCP expectedXYZ = nullptr)
    {
    Transform frame;
    double curvature;
    double torsion;
    curve.FractionToFrenetFrame (fraction, frame, curvature, torsion);
    double realLength;
    curve.SignedDistanceBetweenFractions(0.0, fraction, realLength);
    Check::PrintIndent (1); printf (" %s", name);
    Check::PrintIndent(3); Check::Print (fraction, "fraction");
    DPoint3d xyz;
    frame.GetTranslation (xyz);
    Check::PrintIndent(3); Check::Print(xyz, "point");
    Check::PrintIndent(3); Check::Print (realLength, "distance from 0");
    Check::PrintIndent(3); Check::Print (curvature, "curvature");
    if (curvature != 0.0)
        {
        Check::PrintIndent(3); Check::Print(1.0 / curvature, "radius");
        }
    if (nullptr != expectedXYZ)
        {
        Check::PrintIndent(3); Check::Print(*expectedXYZ, "expectedXYZ");
        Check::PrintIndent(3); Check::Print (expectedXYZ->Distance (xyz), "xyz difference");
        }
    }

TEST(Spiral, MXCubicExample)
    {
    const double length = 20;
    // const double startDirection = 3.397200148;
    double  endRadius = 160.0;
    double  startRadius = 0.0;
    Check::Print (length, "\nNominal Length");
    Check::Print(endRadius, "\nNominal EndRadius");
    printf ("\n\n");
    DPoint3d  startPoint = DPoint3d::From(397857.825178359, 796669.946775749, 0);
    Check::Shift (-startPoint.x, -startPoint.y, -startPoint.z);
    DPoint3d  piPoint = DPoint3d::From(397854.455385710, 796657.051683136, 0);
    DPoint3d  endPoint = DPoint3d::From(397853.173303618, 796650.498871944, 0);
    Check::SaveTransformed (std::vector<DPoint3d>({startPoint, piPoint, endPoint}));
    double startAngle = atan2((piPoint.y - startPoint.y) , (piPoint.x - startPoint.x));
    // Case 1 : OK
    ICurvePrimitivePtr spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius(DSpiral2dBase::TransitionType_MXCubicAlongArc,
        startPoint, startAngle, startRadius, length, endRadius);
    Check::True(spiral.IsValid());
    Check::SaveTransformed (*spiral);
    double realLengthA;
    DPoint3d endA1;
    spiral->FractionToPoint (1.0, endA1);
    spiral->SignedDistanceBetweenFractions(0.0, 1.0, realLengthA);
    CurveLocationDetail sLocDetail;
    spiral->PointAtSignedDistanceFromFraction(0, realLengthA, true, sLocDetail);
    Check::True(sLocDetail.point.IsEqual(endPoint, 0.0001), "CreatePseudoSpiralPointBearingRadiusLengthRadius distance to end");
    ShowProps ("spiralA", *spiral, 1.0, &endPoint);
    // Case 2 : NOT OK
    spiral = ICurvePrimitive::CreatePseudolSpiralWithTrueRadiusLengthRadius(DSpiral2dBase::TransitionType_MXCubicAlongArc,
        startPoint, startAngle, startRadius, length, endRadius);
    Check::SaveTransformed(*spiral);
    Check::True(spiral.IsValid());
    double realLengthB;
    DPoint3d endB;
    spiral->FractionToPoint(1.0, endB);
    spiral->SignedDistanceBetweenFractions(0.0, 1.0, realLengthB);
    spiral->PointAtSignedDistanceFromFraction(0, realLengthB, true, sLocDetail);
    ShowProps("spiralB", *spiral, 1.0, &endPoint);
    Check::False(sLocDetail.point.IsEqual(endPoint, 0.0001), "CreatePseudolSpiralWithTrueRadiusLengthRadius distance to end -- expected a little different");
    Check::ClearGeometry ("Spiral.MXCubicExample");
    }

TEST(Spiral, DirectSpiralWithViewingTransform)
    {
    auto oldVolume = Check::SetMaxVolume (100);
    const double length = 20;
    // const double startDirection = 3.397200148;
    double  endRadius = 160.0;
    double  startRadius = 0.0;
    double startBearingRadians = 0.0;
    double refLength = 1.0e8; // force approximate length comparisons with many fewer digits than usual
    for (double scale : {1.0, 0.5})
        {
        Check::PrintIndent(1);  Check::Print(scale, "scale");

        // This transform is diagonal with 0 in zz spot, so it is not a uniform scale.
        // But if the geometry starts in xy plane it really is uniform, and length computations "as transformed"
        //   should match non-transform methods. (But we know that the direct spiral might use the approximate
        //   bspline instead of the direct equations, so accuracy is dubious)
        RotMatrix projectToXY = RotMatrix::FromRowValues(
            scale,0,0,
            0,scale,0,
            0,0,0
            );
        ICurvePrimitivePtr spiral = ICurvePrimitive::CreatePseudolSpiralWithTrueRadiusLengthRadius(
            DSpiral2dBase::TransitionType_MXCubicAlongArc,
            DPoint3d::FromZero (),
            startBearingRadians, startRadius,
            length, endRadius);
        double lengthA, lengthB;
        Check::SaveTransformed (*spiral);
        auto bcurve = spiral->GetProxyBsplineCurvePtr();
        if (bcurve.IsValid ())
            Check::SaveTransformed (*bcurve);
        // evaluate directly to be sure we bypass the bcurve ..
        bvector<DPoint3d> strokes;
        for (double f = 0.0; f <= 1.000001; f += 1.0 / 32.0)
            {
            DPoint3d xyz;
            spiral->FractionToPoint (f, xyz);
            strokes.push_back(xyz);
            }
        Check::SaveTransformed (strokes);
        spiral->Length (lengthA);
        Check::Near (length, lengthA, "simple length");
        spiral->Length (&projectToXY, lengthB);
        Check::Near (scale *length, lengthB, "full projected length", refLength);

        bvector<double> fractions;
        bvector<double> length1FromStart;
        bvector<double> length2FromStart;
        for (auto fraction : { 0.25, 0.5, 0.75, 0.875 }) {
            double lengthC1, lengthC2;
            Check::PrintIndent(2);  Check::Print (fraction, "fraction");
            Check::True (spiral->SignedDistanceBetweenFractions(0, fraction, lengthC1), "SignedDistanceBetweenFractions");
            Check::PrintIndent(3);  Check::Print (lengthC1, "lengthC1");
            Check::True (spiral->SignedDistanceBetweenFractions (&projectToXY, 0, fraction, lengthC2), "SignedDistanceBetweenFractions (projectToXYZ)");
            Check::PrintIndent(3);  Check::Print(lengthC2, "lengthC2");
            Check::Near (scale * lengthC1, lengthC2, "fractional length", refLength);
            fractions.push_back (fraction);
            length1FromStart.push_back (lengthC1);
            length2FromStart.push_back(lengthC2);
            }
        auto bsplinePrimitive = ICurvePrimitive::CreateBsplineCurve(bcurve);
        for (size_t i = 0; i < fractions.size(); i++)
            {
            double startFraction = fractions[i];
            Check::StartScope("startFraction", startFraction);
            for (size_t j = 0; j < fractions.size(); j++)
                {
                double endFraction = fractions[j];
                Check::StartScope("endFraction", endFraction);
                double signedDistance1 = length1FromStart[j] - length1FromStart[i];
                double signedDistance2;
                spiral->SignedDistanceBetweenFractions (startFraction, endFraction, signedDistance2);
                Check::Near(signedDistance1, signedDistance2, "signed distance from start versus internal", refLength);
                Check::StartScope("length1Between", signedDistance1);
                CurveLocationDetail detail1, detail2, detail3;
                Check::True(spiral->PointAtSignedDistanceFromFraction(startFraction, signedDistance1, false, detail1), "PointAtSignedDistanceBetweenFractions");
                Check::Near(endFraction, detail1.fraction, "point at signed distance");
                Check::True(spiral->PointAtSignedDistanceFromFraction(&projectToXY, startFraction, scale * signedDistance1, false, detail2), "PointAtSignedDistanceBetweenFractions projected");
                Check::Near (endFraction, detail2.fraction, "point at signed distance with projection");
                Check::EndScope ();
                Check::EndScope ();
                }
            Check::EndScope();
            }
        Check::Shift (0, 5, 0);
        }
    Check::ClearGeometry("Spiral.DirectSpiralWithViewingTransform");
    Check::SetMaxVolume(oldVolume);
    }
TEST(Spiral, DirectSpiralWithDubiousArgs)
    {
    double spiralLength = 50.0;
    double radiusB = 400.0;
    {
    auto pointA = DPoint3d::From(3, 3.5, 0);
    auto spiral = ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius(
        DSpiral2dBase::TransitionType_Arema,
        pointA, 0, 0, spiralLength, radiusB);
     Check::SaveTransformed (bvector<DPoint3d>{pointA, {pointA.x + spiralLength,pointA.y,0}});
    if (spiral.IsValid ())
        {
        Check::SaveTransformed (*spiral);
        auto bcurve = spiral->GetProxyBsplineCurvePtr();
        if (bcurve.IsValid ())
            Check::SaveTransformed (*bcurve);
        }
    }

    {
    auto pointA1 = DPoint3d::From(3, 5.5, 0);
    auto frame1 = Transform::From (pointA1);
    auto spiral1 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius(
        DSpiral2dBase::TransitionType_Clothoid,
        0, 0, spiralLength, radiusB,
        frame1,
        0,1);
    Check::SaveTransformed(bvector<DPoint3d>{pointA1, { pointA1.x + spiralLength,pointA1.y,0 }});
    if (spiral1.IsValid())
        Check::SaveTransformed(*spiral1);
    }
    Check::ClearGeometry ("Spiral.DirectSpiralWithDubiousArgs");
    }

/**
 * Newton evaluator for Spiral by Point+Bearing+Kurvature+Point+Bearing
 * With control parameters
 *   u = curve length
 *   v = end radius (0 for flat)
 * The computed spirals are linear curvature passing through (length, curvature) pairs
 *  (0,curvature0)  (L/2, curvature1) (L,curvature2)
 *
 */
class SpiralPBKPBEvaluator : FunctionRRToRR
{
int m_transitionType;
DPoint3d m_point0;
double m_bearingRadians0;
double m_curvature0;
DPoint3d m_point2;
double m_bearingRadians2;
// results of most recent evaluation.
ICurvePrimitivePtr m_spiralA, m_spiralB;
// integral under (length,curvature) is angle change.  This provides curvature1 ...
//  0.5 * (length01 * (curvature0 + curvature1)  + length12 * (curvature1 + curvature2)) = deltaRadians
ValidatedDouble computeCurvature1(double curvature0, double length01, double length12, double curvature2, double deltaRadians)
    {
    double a = 2.0 * deltaRadians - (length01 * curvature0 + length12 * curvature2);
    return DoubleOps::ValidatedDivide (a, length01 + length12);
    }
public:
    SpiralPBKPBEvaluator(int transitionType, DPoint3dCR point0, double bearingRadians0, double curvature0, DPoint3dCR point2, double bearingRadians2)
        {
        m_transitionType = transitionType;
        m_point0 = point0;
        m_bearingRadians0 = bearingRadians0;
        m_curvature0 = curvature0;
        m_point2 = point2;
        m_bearingRadians2 = bearingRadians2;
        }
    // Virtual function
    // @param [in] uvw current values of 3 independent variables.
    // @param [out]f  3 computed function values
    bool EvaluateRRToRR
    (
    double totalLength,
    double finalRadius,
    double &xError,
    double &yError
    ) override
        {
        double length01 = totalLength * 0.5;
        double length12 = totalLength * 0.5;
        double curvature2;
        DoubleOps::SafeDivide (curvature2, 1.0, finalRadius, 0.0);
        m_spiralA = m_spiralB = nullptr;
        double curvature1 = computeCurvature1 (m_curvature0, length01, length12, curvature2, m_bearingRadians2 - m_bearingRadians0);
        auto spiralA = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (m_transitionType,
                    m_bearingRadians0, m_curvature0,
                    length01, curvature1,
                    Transform::From (m_point0), 0, 1);

        if (spiralA.IsValid())
            {
            DPoint3d point1;
            DVec3d tangent1;
            spiralA->FractionToPoint (1.0, point1, tangent1);
            double bearingRadians1 = atan2 (tangent1.y, tangent1.x);
            auto spiralB = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature(m_transitionType, bearingRadians1, curvature1, length12, curvature2,
                Transform::From(point1), 0, 1);
            if (spiralB.IsValid())
                {
                m_spiralA = spiralA;
                m_spiralB = spiralB;
                DPoint3d point2;
                DVec3d tangent2;
                spiralB->FractionToPoint(1.0, point2, tangent2);
                xError = point2.x - m_point2.x;
                yError = point2.y - m_point2.y;
                return true;
                }
            }
        return false;
        }
ICurvePrimitivePtr RecentSpiralA () const {return m_spiralA;}
ICurvePrimitivePtr RecentSpiralB() const { return m_spiralB; }
};

//! Create one or more spirals which have
//! <ul>
//! <li> Start at pointA with direction bearingA and radiusA
//! <li> end at pointB with direction bearingB.
//! <li> if more than one spiral is required, join with curvature continuity at some intermediate point
//! <ul>
//! The spiral type must be a "true" spiral.

//CurveVectorPtr CreateInterpolatingSpiralsPointBearingCurvaturePointBearing(int spiralType, DPoint3dCR pointA, double bearingRadiansA, double radiusA, DPoint3dCR pointB, double bearingRadiansB)
TEST(Spiral, CreateInterpolatingSpiralsPointBearingCurvaturePointBearing)
    {
    DPoint3d pointA = DPoint3d::From (0,0,0);
    double curvatureA = 0.0;
    int transitionType = DSpiral2dBase::TransitionType_Clothoid;
    for (double radius2 : {100.0, 200.0, 300.0})
        {
        SaveAndRestoreCheckTransform radiusShift(200, 0, 0);
        auto spiralAB = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius(transitionType, 0.0, 0.0, 100.0, radius2,
            Transform::From(pointA), 0, 1);
        DPoint3d pointB;
        DVec3d tangentB;
        spiralAB->FractionToPoint(1.0, pointB, tangentB);
        double bearingRadiansB = atan2(tangentB.y, tangentB.x);
        Check::SaveTransformed(*spiralAB);
        Check::Shift(0, 10, 0);

        for (double deltaRadians : { -0.2, 0.0, 0.2})
            {
            Check::Shift(0, 10, 0);
            auto bearingRadiansB1 = bearingRadiansB + deltaRadians;
            SpiralPBKPBEvaluator evaluator(transitionType, pointA, 0.0, curvatureA, pointB, bearingRadiansB1);
            auto vectorB = DVec3d::FromXYAngleAndMagnitude (bearingRadiansB1, 5.0);
            for (double length02 : {98.0, 100.0, 102.0})
                {
                double errorX, errorY;
                if (evaluator.EvaluateRRToRR(length02, radius2, errorX, errorY))
                    {
                    auto spiralA = evaluator.RecentSpiralA();
                    auto spiralB = evaluator.RecentSpiralB();
                    if (spiralA.IsValid ())
                        Check::SaveTransformed (*spiralA);
                    if (spiralB.IsValid())
                        {
                        Check::SaveTransformed(*spiralB);
                        DPoint3d xyzB;
                        spiralB->FractionToPoint (1.0, xyzB);
                        Check::SaveTransformed (DSegment3d::From (xyzB, xyzB + vectorB));
                        }
                    }
                }
            }
        }
    Check::ClearGeometry("Spiral.CreateInterpolatingSpiralsPointBearingCurvaturePointBearing");
    }