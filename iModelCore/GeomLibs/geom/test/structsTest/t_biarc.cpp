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

TEST(Biarc,TEST0)
    {
    testBiarc (DPoint3d::From (1,0,0), DPoint3d::From (0,1,0), Angle::FromDegrees (10.0), Angle::FromDegrees (15.0));
    testBiarc (DPoint3d::From (1,0,0), DPoint3d::From (0,1,0), Angle::FromDegrees (45.0), Angle::FromDegrees (45.0));
    testBiarc (DPoint3d::From (1,0,0), DPoint3d::From (0,1,0), Angle::FromDegrees (-45.0), Angle::FromDegrees (45.0));
    Check::ClearGeometry ("Biarc.TEST0");
    }
