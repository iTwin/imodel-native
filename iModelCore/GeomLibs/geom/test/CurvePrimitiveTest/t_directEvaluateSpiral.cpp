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

ICurvePrimitivePtr ConstructSpiralRadiusRadiusLength (int typeCode, double radiusA, double radiusB, double lengthAB)
    {
    double curvatureA = DoubleOps::ValidatedDivideDistance (1.0, radiusA);
    double curvatureB = DoubleOps::ValidatedDivideDistance (1.0, radiusB);

    // extrapolate to inflection, assuming clothoid (linear) curvature function
    //   (curvatureB - curvatureA) / lengthAB = curvatureB / length0B
    double length0B = curvatureB * lengthAB / (curvatureB - curvatureA);
    double fractionA = curvatureA / curvatureB;
    double fractionB = 1.0;
    auto referenceSpiral = DSpiral2dBase::Create (typeCode);
    double bearing0 = 0.0;
    double curvature0 = 0.0;
    referenceSpiral->SetBearingCurvatureLengthCurvature (bearing0, curvature0, length0B, curvatureB);
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
    auto fullSpiral = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (typeCode, 0.0, 0.0, length0B, curvatureB, Transform::FromIdentity (),
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
    for (double fraction : {0.0, 0.25, 0.5, 0.75})
        {
        double curvatureB = DoubleOps::ValidatedDivideDistance (1.0, radiusB);
        double curvatureA = fraction * curvatureB;
        double radiusA = DoubleOps::ValidatedDivideDistance (1.0, curvatureA);
        double lengthAB = (1.0 - fraction) * length0B;
        auto spiral0B = ConstructSpiralRadiusRadiusLength (typeCode, 0.0, radiusB,
                lengthAB * curvatureB / (curvatureB- curvatureA));
        auto spiralAB = ConstructSpiralRadiusRadiusLength (typeCode, radiusA, radiusB, lengthAB);
        Check::SaveTransformed (*spiralAB);
        Check::Shift (0, yShiftA, 0);
        Check::SaveTransformed (*spiral0B);
        Check::Shift (0, yShiftB, 0);
        }
    Check::ClearGeometry ("Spiral.TwoRadiusConstruction");
    }