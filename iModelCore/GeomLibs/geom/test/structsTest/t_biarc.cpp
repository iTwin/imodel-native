/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Bentley/BeNumerical.h>
void testBiarc (DPoint3dCR pointA, DPoint3dCR pointB, Angle tangentAToChord, Angle tangentBToChord)
    {
    SaveAndRestoreCheckTransform shifter (5,0,0);
    DVec3d unitA, unitB, chord, unitChord;
    chord = DVec3d::FromStartEnd (pointA, pointB);
    double chordLength = chord.Magnitude ();
    double a = chordLength * 0.25;
    unitChord.Normalize (chord);
    unitA.RotateXY (unitChord, -tangentAToChord.Radians ());
    unitB.RotateXY (unitChord, tangentBToChord.Radians ());
    DEllipse3d ellipseA, ellipseB;
    if (  Check::True (DEllipse3d::Construct_Biarcs (ellipseA, ellipseB, pointA, unitA, pointB, unitB), "Biarc"))
        {
        auto cpA = ICurvePrimitive::CreateArc (ellipseA);
        auto cpB = ICurvePrimitive::CreateArc (ellipseB);
        
        DPoint3d pointA0, pointA1, pointB0, pointB1;
        DVec3d   tangentA0, tangentA1, tangentB0, tangentB1;
        DVec3d   kurvA, kurvB;
        ellipseA.FractionParameterToDerivatives (pointA0, tangentA0, kurvA, 0.0);
        ellipseA.FractionParameterToDerivatives (pointA1, tangentA1, kurvA, 1.0);

        ellipseB.FractionParameterToDerivatives (pointB0, tangentB0, kurvA, 0.0);
        ellipseB.FractionParameterToDerivatives (pointB1, tangentB1, kurvB, 1.0);

        Check::Near (pointA0, pointA);
        Check::Near (pointB1, pointB);
        Check::Near (pointA1, pointB0);

        Check::True (unitA.IsParallelTo (tangentA0),     "start Tangent");
        Check::True (tangentA1.IsParallelTo (tangentB0), "interior tangent");
        Check::True (unitB.IsParallelTo (tangentB1),     "end Tangent");
        Check::SaveTransformed (bvector<DPoint3d> {pointA, pointA + a * unitA});
        Check::SaveTransformed (bvector<DPoint3d> {pointB, pointB - a * unitB});
        Check::SaveTransformed (*cpA);
        Check::SaveTransformed (*cpB);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Biarc,TEST0)
    {
    testBiarc (DPoint3d::From (1,0,0), DPoint3d::From (0,1,0), Angle::FromDegrees (10.0), Angle::FromDegrees (15.0));
    testBiarc (DPoint3d::From (1,0,0), DPoint3d::From (0,1,0), Angle::FromDegrees (45.0), Angle::FromDegrees (45.0));
    testBiarc (DPoint3d::From (1,0,0), DPoint3d::From (0,1,0), Angle::FromDegrees (-45.0), Angle::FromDegrees (45.0));
    Check::ClearGeometry ("Biarc.TEST0");
    }

void testBiarcFamily (DPoint3dCR pointA0, Angle bearingA, DVec3dCR deltaA, size_t numA, DPoint3dCR pointB, Angle bearingB)
    {
    SaveAndRestoreCheckTransform shifter (5,0,0);
    double a = 0.05 * pointA0.Distance (pointB);
    auto unitB = DVec3d::From (bearingB.Cos (), bearingB.Sin ());
    Check::SaveTransformed (bvector<DPoint3d> {pointB, pointB + a * unitB});
    bvector<DPoint3d> breakpoints;
    for (size_t i = 0; i < numA; i++)
        {
        auto pointA = pointA0 + (double)i * deltaA;
        auto unitA = DVec3d::From (bearingA.Cos (), bearingA.Sin ());
        Check::SaveTransformed (bvector<DPoint3d> {pointA, pointA - a * unitA});
        DEllipse3d ellipseA, ellipseB;
        if (DEllipse3d::Construct_Biarcs (ellipseA, ellipseB, pointA, unitA, pointB, unitB))
            {
            breakpoints.push_back (ellipseA.FractionToPoint (1.0));
            auto cpA = ICurvePrimitive::CreateArc (ellipseA);
            auto cpB = ICurvePrimitive::CreateArc (ellipseB);
            Check::SaveTransformed (*cpA);
            Check::SaveTransformed (*cpB);
            }
        }
    Check::SaveTransformed (breakpoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Biarc,TEST1)
    {
    testBiarcFamily
            (
            DPoint3d::From (0,0,0), Angle::FromDegrees (90), DVec3d::From (0,1,0), 10,
            DPoint3d::From (2,10,0), Angle::FromDegrees (0)
            );
    testBiarcFamily
            (
            DPoint3d::From (0,0,0), Angle::FromDegrees (85), DVec3d::From (0,1,0), 10,
            DPoint3d::From (2,10,0), Angle::FromDegrees (0)
            );

    testBiarcFamily
            (
            DPoint3d::From (0,0,0), Angle::FromDegrees (60), DVec3d::From (0,1,0), 10,
            DPoint3d::From (5,5,0), Angle::FromDegrees (0)
            );

    Check::ClearGeometry ("Biarc.TEST1");
    }

void testTangentArcChain (bvector<double> const &radii, bvector<Angle> const &angles, size_t negateIndex = SIZE_MAX)
    {
    bvector<double>radiiA = radii;
    if (negateIndex < radiiA.size ())
        radiiA[negateIndex] *= -1.0;
    DPoint3d startPoint = DPoint3d::From (0,0,0);
    auto cv = CurveCurve::ConstructTangentArcChain (
            DPoint3d::From (0,0,0),
            DVec3d::From (0,1,0), DVec3d::UnitZ (),
            radiiA, angles);
    if (Check::True (cv.IsValid ()))
        {
        DRange3d range;
        cv->GetRange (range);
        Check::Shift (startPoint - range.low);
        double ticSize = 0.04 * range.low.Distance (range.high);
        SaveAndRestoreCheckTransform shifter (1.0 + range.XLength (),0,0);
        Check::SaveTransformed (*cv);
        for (size_t i = 1; i < cv->size (); i++)
            {
            auto ray = cv->at(i)->FractionToPointAndUnitTangent (0.0);
            if (ray.IsValid ())
                {
                auto perp = DVec3d::FromCrossProduct (ray.Value ().direction, DVec3d::UnitZ ());
                Check::SaveTransformed (bvector<DPoint3d> {ray.Value ().origin, ray.Value ().origin + ticSize * perp});
                }
            }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Biarc,TangentArcChain)
    {
    // simple S curves ..
    for (double degrees : bvector<double> {10,20,40})
        testTangentArcChain (
            bvector<double> {-5, 5},
            bvector<Angle> {Angle::FromDegrees (degrees), Angle::FromDegrees (degrees)}
            );


    bvector<double>  radii {1,2,1, 4};
    bvector<Angle> angles {Angle::FromDegrees (25), Angle::FromDegrees (40), Angle::FromDegrees (40), Angle::FromDegrees (20)};
    for (size_t i = 0; i < radii.size () + 1; i++)
        testTangentArcChain (radii, angles, i == 0 ? radii.size () : i - 1);

    Check::ClearGeometry ("Biarc.TangentArcChain");
    }

bvector<DPoint3d> CreateArrow (DPoint3dCR startPoint, DVec3dCR vectorA, double headSize, double vectorLength = 0.0)
    {
    DVec3d vectorU, vectorV;
    DVec3d vector = vectorA;
    if (vectorLength != 0.0)
        vector.ScaleToLength (vectorLength);
    vectorU.Normalize (vector);
    vectorU.ScaleToLength (headSize);
    vectorV = DVec3d::FromCCWPerpendicularXY (vectorU);
    DPoint3d headPoint = startPoint + vector;
    double a = 2 * headSize;
    return bvector<DPoint3d> {startPoint, headPoint,
        headPoint - a * vectorU + headSize * vectorV,
        headPoint - a * vectorU - headSize * vectorV,
        headPoint
        };
    }


#ifdef MultiRadiusRamp
CurveVectorPtr ConstructMultiRadiusTurnToStraightRampPerpendicularIntersection
(
DPoint3dCR startPoint,
DVec3dCR   startTangent,
double     startDistance,
bvector<double> const &radii,
DPoint3dCR    intersectionLinePoint,
DVec3dCR      intersectionLineTangent
)
    {

    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Biarc,LoopBack)
    {
    DPoint3d startPoint = DPoint3d::From (0,0,0);
    DVec3d   startTangent = DVec3d::From (-1,2,0);
    DVec3d   endTangent = DVec3d::From (0, -1, 0);
    Check::SaveTransformed (CreateArrow(startPoint, startTangent, 0.2, 1.0));
    for (double a : bvector<double> {-3,-2,-1,1,2,3})
        {
        DVec3d   endTangentShift = DVec3d::From (a,0,0);
        Check::SaveTransformed (CreateArrow(startPoint + endTangentShift, endTangent, 0.2, 1.0));
        }

    double refRadians = startTangent.AngleToXY (endTangent);
    if (refRadians < 0.0)
        refRadians = Angle::TwoPi () + refRadians;

    Angle q1 = Angle::FromRadians (refRadians / 3.0);
    auto curve1 = CurveCurve::ConstructTangentArcChain (
                startPoint, startTangent, DVec3d::UnitZ (),
                bvector<double> {5,3,4},
                bvector<Angle> {q1,q1,q1}
                );
    Check::SaveTransformed (*curve1);
// reverse by radius negation 
    Angle q2 = Angle::FromRadians ((Angle::TwoPi () - refRadians) / 3.0);
    auto curve2 = CurveCurve::ConstructTangentArcChain (
                startPoint, startTangent, DVec3d::UnitZ (),
                bvector<double> {-5,-3,-4},
                bvector<Angle> {q2,q2,q2}
                );
    Check::SaveTransformed (*curve2);



    Check::ClearGeometry ("Biarc.LoopBack");
    }