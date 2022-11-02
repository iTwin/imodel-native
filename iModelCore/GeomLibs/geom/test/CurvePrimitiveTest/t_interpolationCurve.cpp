/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
bool ReadDgnjsGeometry(bvector<IGeometryPtr> &geometry, size_t minGeometry, WCharCP nameA, WCharCP nameB, WCharCP nameC);
void DoRoundTrip(IGeometryPtr g0, bool emitGeometry, int serializerSelect);


TEST(InterpolationCurve, HelloWorld)
    {
    double circleRadius = 1.0;
    bvector<DPoint3d> circlePoints, arcPoints;
    int numEdges = 8;
    double yScale = 1.4;
    double radiansStep = Angle::TwoPi () / numEdges;
    DPoint3d xyz;
    for (int i = 0; i < 8; i++)
        {
        double radians = i * radiansStep;
        xyz = DPoint3d::From (circleRadius * cos (radians), yScale * circleRadius * sin(radians));
        arcPoints.push_back (xyz);
        }
    circlePoints = arcPoints;
    circlePoints.push_back (arcPoints.front ());
    bvector<double> fitParams {0, 0.03, 0.1, 0.17, 0.3, 0.44, 0.72, 1.0};
    auto startTangent = DVec3d::From (1, 1, 0);
    auto endTangent = DVec3d::From (1, -1, 0);
    double delta = 3.0 * circleRadius;
    Check::SaveTransformed(circlePoints);
    Check::Shift(0, delta);
    Check::SaveTransformed(arcPoints);
    Check::Shift(delta, -delta);


    bvector<ICurvePrimitivePtr> allCurves;
    MSInterpolationCurve newCurve;
    if (SUCCESS == newCurve.Populate (4, false, 0, 0, 0, 0, arcPoints.data (), (int)arcPoints.size (),
                nullptr, 0, nullptr, nullptr))
        allCurves.push_back (ICurvePrimitive::CreateInterpolationCurveSwapFromSource (newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 0, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    allCurves.push_back(nullptr);

    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 0, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 1, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 1, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 0, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 1, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 1, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    allCurves.push_back(nullptr);


    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 0, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 1, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 1, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 0, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 1, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 1, 1, 1, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, nullptr, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    allCurves.push_back(nullptr);

/*
InterpolationCurve3dOptions.create({ fitPoints: arcPoints, startTangent : startTan, endTangent : endTan, isChordLenTangents : 0 }),
InterpolationCurve3dOptions.create({ fitPoints: arcPoints, startTangent : startTan, endTangent : endTan, isChordLenTangents : 1 }),
*/

    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 1, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    allCurves.push_back (nullptr);
/*
InterpolationCurve3dOptions.create({ fitPoints: circlePoints, closed : true, isChordLenKnots : 0 }),
InterpolationCurve3dOptions.create({ fitPoints: circlePoints, closed : true, isChordLenKnots : 1 }),
*/
    if (SUCCESS == newCurve.Populate(4, true, 0, 0, 0, 0, circlePoints.data(), (int)circlePoints.size(),
        nullptr, 0, nullptr, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, true, 1, 0, 0, 0, circlePoints.data(), (int)circlePoints.size(),
        nullptr, 0, nullptr, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    allCurves.push_back(nullptr);

    /*
InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, isColinearTangents : 0, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, isColinearTangents : 0, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, isColinearTangents : 1, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, isColinearTangents : 1, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 0, isChordLenTangents : 0, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 0, isChordLenTangents : 0, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 0, isChordLenTangents : 1, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 0, isChordLenTangents : 1, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 1, isChordLenTangents : 0, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 1, isChordLenTangents : 0, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 1, isChordLenTangents : 1, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, isColinearTangents : 1, isChordLenTangents : 1, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 0, isChordLenTangents : 0, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 0, isChordLenTangents : 0, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 0, isChordLenTangents : 1, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 0, isChordLenTangents : 1, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 1, isChordLenTangents : 0, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 1, isChordLenTangents : 0, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 1, isChordLenTangents : 1, isNaturalTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, endTangent : endTan, isColinearTangents : 1, isChordLenTangents : 1, isNaturalTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, endTangent : endTan, isChordLenTangents : 0 }),
        InterpolationCurve3dOptions.create({ fitPoints: arcPoints, knots : fitParams, startTangent : startTan, endTangent : endTan, isChordLenTangents : 1 }),
        InterpolationCurve3dOptions.create({ fitPoints: circlePoints, knots : fitParams, closed : true }),
        */
    for (auto &cp : allCurves) {
        SaveAndRestoreCheckTransform shifter (delta, 0, 0);
        if (cp.IsValid ())
            {
            auto proxy = cp->GetProxyBsplineCurvePtr();
            Check::SaveTransformed (*cp);
            Check::Shift (0, delta, 0);
            Check::SaveTransformed (*proxy);
            auto g = IGeometry::Create(cp);
            DoRoundTrip (g, false, 0);
            }
        }
    Check::ClearGeometry("InterpolationCurve.HelloWorld");
    }




