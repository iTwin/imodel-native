/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
#include <Bentley/BeNumerical.h>
#include <SampleGeometryCreator.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

// markup = 0 is just edge output
// markup = 1 adds all cube diagonals, extending to points at fractional coordinate
//      along the full diagonal
void SaveEdges(TransformedDRange3dPtr const & range, double xShift, double yShift, int markup = 0, double fraction = 1.0)
    {
    bvector<DPoint3d> corners;
    range->GetWorldCorners (corners);
    for (auto &c : corners)
        {
        c.x += xShift;
        c.y += yShift;
        }
    Check::SaveTransformedEdges (corners.data ());
    if (markup == 1)
        {
for (auto i0 : { 0,1,2,3 })
    {
    auto point0 = corners[i0];
    auto point1 = corners[7 - i0];
    Check::SaveTransformed(
        DSegment3d::From(
            DPoint3d::FromInterpolate(point0, fraction, point1),
            DPoint3d::FromInterpolate(point1, fraction, point0)));
    }
        }
    }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    TEST(TransformedRange, HelloWorld)
        {
        double a = 2.0;
        auto baseRange = DRange3d::From(-1 * a, -2 * a, -5 * a, 5 * a, 6 * a, 10 * a);
        // shift between outputs of "true" and "false" results
        double yShift = 30.0 * a;
        double xShift = 30.0 * a;
        auto outputRange = TransformedDRange3d::Create(baseRange, Transform::FromIdentity());
        auto baseClip = ConvexClipPlaneSet(baseRange,
            true, true,
            true, true,
            true, true);
        auto pointZ = baseClip.FindAnyVertex();
        if (pointZ.IsValid())
            Check::SaveTransformedMarker(pointZ, -0.10);

        double w0Step = 0.01;
        double shiftB1 = 5.0;
        double shiftB0 = 7.0;

        for (double lengthFactor : {1.0, 5.0})
            {
            double w0 = 0.01;
            for (double rotateDegrees : {0.0, 35.0, 225.0})
                {
                for (auto localTestRange : {
                    DRange3d::From(-1,-1,0, 1,lengthFactor * 2.5,1),
                    DRange3d::From(0,0,0, lengthFactor * 5,1,1)
                    }
                    )
                    {
                    SaveAndRestoreCheckTransform shifter(3 * xShift, 0);
                    SaveEdges(outputRange, 0, 0);
                    SaveEdges(outputRange, 0, yShift);
                    SaveEdges(outputRange, xShift, 0);
                    SaveEdges(outputRange, xShift, yShift);
                    SaveEdges(outputRange, xShift, 2 * yShift);
                    SaveEdges(outputRange, xShift, shiftB0 * yShift);
                    SaveEdges(outputRange, xShift, shiftB1 * yShift);

                    auto axes = RotMatrix::FromVectorAndRotationAngle(DVec3d::From(-0.1, 0.2, 0.9), Angle::DegreesToRadians(rotateDegrees));
                    for (double uFraction = -0.5; uFraction < 1.401; uFraction += 0.35)
                        {
                        for (double vFraction = -0.41; vFraction < 1.301; vFraction += 0.3)
                            {
                            double wFraction = w0 + 0.3 * vFraction;
                            auto origin = baseRange.LocalToGlobal(uFraction, vFraction, wFraction);
                            auto testRange = TransformedDRange3d::Create(localTestRange, Transform::From(axes, origin));
                            bool allIn = testRange->IsAllInside(baseClip);
                            SaveEdges(testRange, 0.0, allIn ? 0.0 : yShift, allIn ? 0 : 0, 1.5);

                            auto classify = testRange->Classify(baseClip);
                            auto classifyAny = testRange->IsAnyPointInsideClipper(baseClip);

                            if (classify.IsAllInside ())
                                SaveEdges(testRange, xShift, 0, 0);
                            else if (classify.IsAllOutside ())
                                SaveEdges(testRange, xShift, yShift, 0, 1.0);
                            else
                                SaveEdges(testRange, xShift, 2 * yShift, 0, 1.5);

                            SaveEdges(testRange, xShift,
                                (classifyAny ? shiftB1 : shiftB0) * yShift,
                                0, 1.0);
                            Check::Bool (classifyAny, !classify.IsAllOutside ());
                            }
                        }
                    w0 += w0Step;
                    }
                }
            }
        Check::ClearGeometry("TransformedRange.HelloWorld");
        }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TransformedRange, FindAnyVertexOfConvexClipPlaneSet)
    {
    double r = 3.0;
    auto zVector = DVec3d::From (0,0,1);
    for (int numSides : {6, 3, 2})
        {
        ConvexClipPlaneSet clipper;
        bvector<DPoint3d> points;
        double radiansStep = 0.4;
        // multiple planes with parallel intersections
        for (int i = 0; i < numSides; i++)
            {
            points.push_back (DPoint3d::From (r * cos (i * radiansStep), r * sin (i * radiansStep), 0));
            }
        for (int i = 0; i + 1 < numSides; i++)
            {
            auto plane = ClipPlane::FromEdgeAndUpVector(points[i+1], points[i], zVector, Angle::FromDegrees (0));
            clipper.push_back (plane.Value ());
            }
        for (auto & xyz : points)
            {
            DPoint3d xyzA = xyz; xyzA.x += 0.3;
            Check::SaveTransformedMarker (xyzA,
                clipper.IsPointInside(xyzA) ? 0.2 : -0.1);
            xyzA.x -= 2.0;
            Check::SaveTransformedMarker(xyzA,
                clipper.IsPointInside(xyzA) ? 0.2 : -0.1);
            }
        // and finally a transverse
        auto pointQ = DPoint3d::From(1, 2, 0.3);
        Check::SaveTransformedMarker(pointQ,
            clipper.IsPointInside(pointQ) ? 0.2 : -0.1);

        ClipPlane planeA (DVec3d::From(0, 0, 1), pointQ);
        double q = planeA.EvaluatePoint (pointQ);
        Check::SaveTransformedMarker(pointQ,
            q >= 0.0 ? 0.2 : -0.1);


        clipper.push_back (planeA);
        Check::SaveTransformed (points);

        auto pointZ = clipper.FindAnyVertex ();
        if (pointZ.IsValid ())
            Check::SaveTransformedMarker (pointZ, -0.10);
        Check::Shift (10,0,0);
        }
    Check::ClearGeometry("TransformedRange.FindVertexInClipPlaneSet");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TransformedRange, ClipSet)
    {
    auto range0 = DRange3d::From(0,0,0, 5,5,2);
    auto range1 = DRange3d::From (6,0,0, 10,3,2);
    // shift between outputs of "true" and "false" results
    double yShift = 10.0;
    double xShift = 30.0;
    ClipPlaneSet clipper;
    double w0Step = 0.01;
    bvector<DRange3d> activeRanges;
    for (auto &addedRange : {range0, range1})
        {
        auto convexClip = ConvexClipPlaneSet (addedRange);
        clipper.push_back (convexClip);
        activeRanges.push_back (addedRange);
        double shiftB1 = 5.0;
        double shiftB0 = 7.0;
        for (double lengthFactor : {0.1, 1.0, 5.0})
            {
            double w0 = 0.01;
            for (auto localTestRange : {
                DRange3d::From(-1,-1,0, 1,lengthFactor * 2.5,1),
                DRange3d::From(0,0,0, lengthFactor * 5,1,1)
                }
                )
                {
                SaveAndRestoreCheckTransform shifter(3 * xShift, 0);
                auto compositeRange = DRange3d::NullRange ();
                for (auto &range : activeRanges)
                    {
                    auto outputRange = TransformedDRange3d::Create (range, Transform::FromIdentity ());
                    SaveEdges(outputRange, 0, 0);
                    SaveEdges(outputRange, 0, yShift);
                    SaveEdges(outputRange, 0, 2 * yShift);
                    SaveEdges(outputRange, xShift, 0);
                    SaveEdges(outputRange, xShift, yShift);
                    SaveEdges(outputRange, xShift, 2 * yShift);
                    SaveEdges(outputRange, xShift, shiftB0 * yShift);
                    SaveEdges(outputRange, xShift, shiftB1 * yShift);
                    compositeRange.Extend (range);
                    }
                for (double uFraction = -0.5; uFraction < 1.401; uFraction += 0.35)
                    {
                    for (double vFraction = -0.41; vFraction < 1.301; vFraction += 0.3)
                        {
                        double wFraction = w0 + 0.3 * vFraction;
                        auto origin = compositeRange.LocalToGlobal(uFraction, vFraction + 0.1 * uFraction, wFraction);
                        auto testRange = TransformedDRange3d::Create(localTestRange, Transform::From(origin));

                        auto classify = testRange->Classify(clipper);
                        auto classifyAny = testRange->IsAnyPointInsideClipper(clipper);

                        if (classify.IsAllInside())
                            SaveEdges(testRange, xShift, 0, 0);
                        else if (classify.IsAllOutside())
                            SaveEdges(testRange, xShift, yShift, 0, 1.0);
                        else
                            SaveEdges(testRange, xShift, 2 * yShift, 0, 1.5);

                        SaveEdges (testRange, xShift,
                                    (classifyAny ? shiftB1 : shiftB0) * yShift, 
                                    0, 1.0);
                        Check::Bool(classifyAny, !classify.IsAllOutside());
                        }
                    }
                w0 += w0Step;
                }
            }
        }
    Check::ClearGeometry("TransformedRange.ClipPlaneSet");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TransformedRange, PolygonClipSet)
    {
    double a = 12.0;
    double b = 6.0;
    bvector<DPoint3d> rightLeg, upperLeg;
    ClipPlaneSet clipper;
    SampleGeometryCreator::BuildLShapedClipper (a,a, b, b, clipper, rightLeg, upperLeg);
    double xShift = 4 * a;
    double yShift = 2 * a;
    for (double rotateDegrees : {0.0, 10.0})
        {
        SaveAndRestoreCheckTransform shifter1(xShift, 0, 0);
        {
            SaveAndRestoreCheckTransform shifter2 (0,0,0);
            for (int i = 0; i < 3; i++)
                {
                Check::SaveTransformed(rightLeg);
                Check::SaveTransformed(upperLeg);
                Check::Shift (0, yShift, 0);
                }
            }
        auto localRange = DRange3d::From(0,0,0, 2,1,0);
        for (double x0 = -3; x0 < a + 4; x0 += 3)
            {
            for (double y0 = -2.5; y0 < a + 3; y0 += 2.1)
                {
                auto testRange = TransformedDRange3d::Create (localRange, Transform::From (
                        RotMatrix::FromVectorAndRotationAngle(DVec3d::From(-0.1, 0.2, 0.9), Angle::DegreesToRadians(rotateDegrees)),
                        DPoint3d::From (x0 + 0.15 * y0, y0, 0.0)));
                auto classify = testRange->Classify(clipper);
                if (classify.IsAllInside())
                    SaveEdges(testRange, 0, 0, 0);
                else if (classify.IsAllOutside())
                    SaveEdges(testRange, 0, yShift, 0, 1.0);
                else
                    SaveEdges(testRange, 0, 2 * yShift, 0, 1.5);
                }
            }
        }
    Check::ClearGeometry("TransformedRange.PolygonClipSet");

    }
