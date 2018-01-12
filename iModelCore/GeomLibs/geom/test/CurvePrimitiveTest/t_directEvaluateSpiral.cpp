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
    
