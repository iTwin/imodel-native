/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
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

    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 0, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, false, 0, 0, 1, 0, arcPoints.data(), (int)arcPoints.size(),
        nullptr, 0, &startTangent, &endTangent))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));

    allCurves.push_back (nullptr);

    if (SUCCESS == newCurve.Populate(4, true, 0, 0, 0, 0, circlePoints.data(), (int)circlePoints.size(),
        nullptr, 0, nullptr, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));
    if (SUCCESS == newCurve.Populate(4, true, 1, 0, 0, 0, circlePoints.data(), (int)circlePoints.size(),
        nullptr, 0, nullptr, nullptr))
        allCurves.push_back(ICurvePrimitive::CreateInterpolationCurveSwapFromSource(newCurve));

    allCurves.push_back(nullptr);

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

TEST(InterpolationCurve, PointFormat)
    {
    // Test alternative tagged json point format (not 2D number array)
    // Unescaped raw_json_text is in between the following multicharacter delimiters: foo( raw_json_text )foo
    static Utf8Chars s_input(u8R"foo({"interpolationCurve":{"fitPoints":[{"x":0,"y":0},{"x":2,"y":1,"z":1},{"x":3,"y":2,"z":2},{"x":4,"y":3,"z":1},{"x":5,"y":4}],"startTangent":{"x":1,"y":0},"endTangent":{"x":-1,"y":0}}})foo");
    bvector<IGeometryPtr> inputs;
    if (Check::True(IModelJson::TryIModelJsonStringToGeometry(&s_input, inputs), "Parse input") &&
        Check::Size(inputs.size(), 1, "Have one input"))
        {
        auto prim = inputs[0]->GetAsICurvePrimitive();
        if (Check::True(prim.IsValid(), "Input is valid"))
            {
            auto interpCurve = prim->GetInterpolationCurveCP();
            if (Check::True(interpCurve != 0, "Input is an interpolation curve"))
                {
                Check::Int(4, interpCurve->params.order, "Order");
                Check::Int(5, interpCurve->params.numPoints, "Fit points count");
                Check::Int(0, interpCurve->params.isPeriodic, "Not periodic");
                Check::Int(0, interpCurve->params.isChordLenTangents, "Bessel tangent length");
                Check::Exact(DPoint3d::From(0,0), interpCurve->fitPoints[0], "First point");
                Check::Exact(DPoint3d::From(2,1,1), interpCurve->fitPoints[1], "Second point");
                Check::Exact(DPoint3d::From(3,2,2), interpCurve->fitPoints[2], "Third point");
                Check::Exact(DPoint3d::From(4,3,1), interpCurve->fitPoints[3], "Fourth point");
                Check::Exact(DPoint3d::From(5,4), interpCurve->fitPoints[4], "Fifth point");
                Check::Exact(DVec3d::From(1,0), interpCurve->startTangent, "Start tangent");
                Check::Exact(DVec3d::From(-1,0), interpCurve->endTangent, "End tangent");
                }
            }
        }
    }
