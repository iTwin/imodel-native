/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,SweptPolygon)
    {
    ConvexClipPlaneSet convexSetA, convexSetB, convexSet0;
    bvector<DPoint3d> triangle {
        DPoint3d::From (1,1,1),
        DPoint3d::From (3,1,2),
        DPoint3d::From (2,2,1)
        };
    auto sweepDirection = DVec3d::From (0,0,1);
    convexSetA.ReloadSweptConvexPolygon (triangle, sweepDirection, 1);
    convexSetB.ReloadSweptConvexPolygon (triangle, sweepDirection, -1);
    convexSet0.ReloadSweptConvexPolygon (triangle, sweepDirection, 0);
    double tolerance = 1.0e-10;
    bvector<double> values  {-0.5, 0.3, 0.5, 0.8, 1.1};
    for (double u : values)
        {
        for (double v : values)
            {
            double w = 1.0 - u - v;
            bool inside = DoubleOps::IsIn01 (u)
                && DoubleOps::IsIn01 (v)
                && DoubleOps::IsIn01 (w);
            DPoint3d planePoint = DPoint3d::FromSumOf (triangle[0], u, triangle[1], v, triangle[2], w);
            DPoint3d abovePoint = planePoint + sweepDirection;
            DPoint3d belowPoint = planePoint - sweepDirection;

            Check::Bool (inside, convexSetA.IsPointOnOrInside (abovePoint, tolerance));
            Check::False (convexSetA.IsPointOnOrInside (belowPoint, tolerance));
            Check::Bool (inside, convexSetA.IsPointOnOrInside (planePoint, tolerance));

            Check::Bool (inside, convexSetB.IsPointOnOrInside (belowPoint, tolerance));
            Check::False (convexSetB.IsPointOnOrInside (abovePoint, tolerance));
            Check::Bool (inside, convexSetB.IsPointOnOrInside (planePoint, tolerance));

            Check::Bool (inside, convexSet0.IsPointOnOrInside (abovePoint, tolerance));
            Check::Bool (inside, convexSet0.IsPointOnOrInside (belowPoint, tolerance));
            Check::Bool (inside, convexSet0.IsPointOnOrInside (planePoint, tolerance));

            }
        }
    }
void testConvexClipXY (double x0, double y0, double ux, double uy, bvector<DPoint3d> &xyz)
    {
    DPlane3d plane0 = DPlane3d::FromOriginAndNormal (x0, y0, 0.0, ux, uy, 0.0);
    DPlane3d plane1 = DPlane3d::FromOriginAndNormal (x0, y0, 0.0, -ux, -uy, 0.0);
    ClipPlane clip0 (plane0);
    ClipPlane clip1 (plane1);
    bvector<DPoint3d> xyz0 = xyz, xyz1 = xyz;
    DVec3d normal = PolygonOps::AreaNormal (xyz);
    double area = normal.Magnitude ();
    bvector<DPoint3d> work;
    clip0.ConvexPolygonClipInPlace (xyz0, work);
    clip1.ConvexPolygonClipInPlace (xyz1, work);
    DVec3d normal0 = PolygonOps::AreaNormal (xyz0);
    DVec3d normal1 = PolygonOps::AreaNormal (xyz1);
    double area0 = normal0.Magnitude ();
    double area1 = normal1.Magnitude ();
    Check::Near (area, area0 + area1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlane,ConvexPolygonClip)
    {
    for (auto &polygon :
        bvector<bvector<DPoint3d>>
            {
            bvector<DPoint3d>
                {
                DPoint3d::From (0,0,0),
                DPoint3d::From (1,0,0),
                DPoint3d::From (1,1,0),
                DPoint3d::From (0,1,0)
                },
            bvector<DPoint3d>
                {
                DPoint3d::From (0.2,0,0),
                DPoint3d::From (1-0.3,0),
                DPoint3d::From (1,1.2,0),
                }
            })
        {
        testConvexClipXY (0,0.5,   0,1  , polygon);
        testConvexClipXY (1,0.5,   0,1  , polygon);
        testConvexClipXY (0,0.5,   1,0  , polygon);
        testConvexClipXY (1,0.5,   1,0  , polygon);
        testConvexClipXY (0.5, 0.6, 0.3, 0.2, polygon);
        testConvexClipXY (0,2,   0,1  , polygon);
        testConvexClipXY (0,-3,   0,1  , polygon);

        testConvexClipXY (1,1, 1,1, polygon);
        testConvexClipXY (1,1, -1,2, polygon);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,Undercut1)
    {
    printf (" enter undercut\n");
    auto roadSurface = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  0,0,1,    10,0,0,   0,4,0),
            3,1, true
            );
    roadSurface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    auto bridge = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  1,1,0.5,    5,0,1,   0,2,0),
            1,1, true
            );
    bridge->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    printf (" road points %d\n", (int)roadSurface->Point ().size ());
    printf (" bridge points %d\n", (int)bridge->Point ().size());
    PolyfaceHeaderPtr undercut;
    PolyfaceHeader::ComputeUndercut (*roadSurface, nullptr, *bridge, nullptr, undercut);

    Check::SaveTransformed (*roadSurface);
    Check::SaveTransformed (*bridge);

    if (Check::True (undercut.IsValid ()))
        {
        printf (" undercut points %d\n", (int)undercut->Point ().size());
        Check::SaveTransformed (*undercut);
        }
    Check::ClearGeometry ("ClipPlaneSet.Undercut1");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,Undercut2)
    {
    printf (" enter undercut\n");
    auto dtm = PolyfaceWithSinusoidalGrid (20, 20,
                    0.0, 0.1, 
                    0.9, 0.15,
                    true);
    dtm->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    auto bridge = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  1,1,0,    5,0,0,   0,2,0.2),
            3,3, true
            );
    bridge->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    printf (" dtm points %d\n", (int)dtm->Point ().size ());
    printf (" bridge points %d\n", (int)bridge->Point ().size());
    PolyfaceHeaderPtr undercut;
    PolyfaceHeader::ComputeUndercut (*dtm, nullptr, *bridge, nullptr, undercut);

    Check::SaveTransformed (*dtm);
    Check::SaveTransformed (*bridge);

    if (Check::True (undercut.IsValid ()))
        {
        printf (" undercut points %d\n", (int)undercut->Point ().size());
        Check::SaveTransformed (*undercut);
        }
    Check::ClearGeometry ("ClipPlaneSet.Undercut2");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,MingagusDoubleVertexHit)
    {
    // rectangle cuts diamond in half.
    // With exact hits on diamond points, the "cut" count in the plane clip was never incremented,
    // and the (properly!!) cut polygon was not swapped in as the right one.
    // verify that the output area is half the input ...
    bvector<DPoint3d> poly1 {{0,10,0},{-10,0,0},{0,-10,0},{10,0,0},{0,10,0}};
    bvector<DPoint3d> poly2 {{-20,-20,0},{0,-20,0},{0,20,0},{-20,20,0},{-20,-20,0}};
    Check::SaveTransformed (poly1);
    Check::SaveTransformed (poly2);
    ConvexClipPlaneSet clipPlaneSet;
    clipPlaneSet.ReloadSweptConvexPolygon(poly2, DVec3d::From(0, 0, 1), 0);
    bvector<DPoint3d> clippedRegion, work;
    clipPlaneSet.ConvexPolygonClip(poly1, clippedRegion, work);
    Check::Shift (30,0,0);
    Check::SaveTransformed (clippedRegion);

    Check::Near (0.5 * PolygonOps::AreaXY (poly1), PolygonOps::AreaXY (clippedRegion), "Diamond cut in half");
    Check::ClearGeometry ("ClipPlaneSet.MingaagusDoubleVertexHit");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,ClassifyCurveVectorInSetDifference)
    {
    double a = 20.0;
    double b = 5.0;
    double bc = 7.5;
    double c = 10.0;
    double h1 = 16.0;
    double h = 15;

    auto convexOuterClip = ConvexClipPlaneSet::FromXYBox (0,0,a,a);
    auto convexInnerClip = ConvexClipPlaneSet::FromXYBox (b,b,c,c);

    ClipPlaneSet outerClip (convexOuterClip);
    ClipPlaneSet innerClip (convexInnerClip);

    auto outsideLine = ICurvePrimitive::CreateLine (DSegment3d::From (-10,h,0, -5,h,0));
    auto insideLine = ICurvePrimitive::CreateLine (DSegment3d::From (2,h,0, 10,h,0));
    auto crossingLine = ICurvePrimitive::CreateLine (DSegment3d::From (-2,h1,0, 10,h1,0));
    auto holeLine = ICurvePrimitive::CreateLine (DSegment3d::From (b+1, bc, 0, c-1, bc, 0));

    auto cOut =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*outsideLine, outerClip, &innerClip);
    auto cIn =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*insideLine, outerClip, &innerClip);
    auto cCrossing =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*crossingLine, outerClip, &innerClip);
    auto cHole =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*holeLine, outerClip, &innerClip);

    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_StronglyOutside, (int)cOut, "Expect OUT");
    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_StronglyInside, (int)cIn, "Expect IN");
    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_Ambiguous, (int)cCrossing, "Expect CROSSING");
    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_StronglyOutside, (int)cHole, "Expect OUT(hole)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,ClassifyRegion)
    {
    double a = 0.0;
    double b0 = 5.0;
    double b1 = 10.0;
    double c = 20.0;

    auto convexClipPoints = ConvexClipPlaneSet::FromXYBox (b0, b0, b1, b1);

    ClipPlaneSet outerClip (convexClipPoints);

    auto outsideBox = CurveVector::CreateRectangle (a, a, c, c, 0.0, CurveVector::BOUNDARY_TYPE_Outer);

    auto cCrossing =  ClipPlaneSet::ClassifyCurveVectorInSetDifference (*outsideBox, outerClip, nullptr, true);

    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_Ambiguous, (int)cCrossing, "Expect CROSSING");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,LineSelect)
    {
    double a = 0.0;
    double c = 20.0;
    // make a region in the (a,a) (c,c) sqaure . . .
    auto box = CurveVector::CreateRectangle (a, a, c, c, 0.0, CurveVector::BOUNDARY_TYPE_Outer);
    // unused - DVec3d vectorToEye = DVec3d::From (0,0,1);
    bvector<BoolTypeForVector> hiddenEdge {false, false, false, false};
    static double offset = -0.0001;
    for (auto yClip : {-5.0, 5.0, 25.0})
        {
        for (auto xRange : {
                    DSegment1d (-5,-1),
                    DSegment1d (-5,5),
                    DSegment1d (5,10),
                    DSegment1d (10,30),
                    DSegment1d (25,30)})
            {
            // endpoints of a select stroke ...
            DPoint3d point0 = DPoint3d::From (xRange.GetStart (), yClip, 0);
            DPoint3d point1 = DPoint3d::From (xRange.GetEnd (), yClip, 0);
            // put a double-back line in the clip set, then offset it (negative!!) to create some space. . .
            bvector<DPoint3d> strokePoints = bvector<DPoint3d> {point0, point1, point0};
            auto convexSet = ConvexClipPlaneSet::FromXYPolyLine (strokePoints, hiddenEdge, true);
            for (auto &plane : convexSet)
                {
                plane.OffsetDistance (offset);
                }
            // add clip planes that point back into the interior of the line . . .
            convexSet.push_back (ClipPlane (DVec3d::FromStartEndNormalize (point0, point1), point0, 0,0));
            convexSet.push_back (ClipPlane (DVec3d::FromStartEndNormalize (point1, point0), point1, 0,0));
            // wrap the convex plane set in a ClipPlaneSet
            ClipPlaneSet clipSet (convexSet);

            auto cCrossing =  ClipPlaneSet::ClassifyCurveVectorInSetDifference (*box, clipSet, nullptr, true);
            auto cExpected = ClipPlaneContainment::ClipPlaneContainment_Ambiguous;
            if (yClip < a || yClip > c || xRange.GetEnd () < a || xRange.GetStart () > c)
                cExpected = ClipPlaneContainment::ClipPlaneContainment_StronglyOutside;
            Check::Int ((int)cCrossing, (int)cExpected, "containment by sliver region");
#ifdef noisyLineSelect
            GEOMAPI_PRINTF (" (xy0 %g,%g) (xy1 %g,%g) (offset %g) (cCrossing %d)  \n", 
                        point0.x, point0.y,
                        point1.x, point1.y,
                        offset, cCrossing);
#endif
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,ClassifySetDifference_ManyLines)
    {
    double a = 30.0;
    double b = 5.0;
    //unusued: double bc = 7.5;
    double c = 15.0;
    //unusued: double h1 = 16.0;
    //unusued: double h = 15;

    double shift = 200.0;
    bvector<DPoint3d> outerBox {
        DPoint3d::From (0,0),
        DPoint3d::From (a,0),
        DPoint3d::From (a,a),
        DPoint3d::From (0,a),
        DPoint3d::From (0,0),
        };
    bvector<DPoint3d> innerBox {
        DPoint3d::From (b,b),
        DPoint3d::From (c,b),
        DPoint3d::From (c,c),
        DPoint3d::From (b,c),
        DPoint3d::From (b,b),
        };
    
    ConvexClipPlaneSet convexOuterClip, convexInnerClip;
    convexOuterClip.AddSweptPolyline (outerBox, DVec3d::From (0,0,1), Angle::FromDegrees (0.0));
    convexInnerClip.AddSweptPolyline (innerBox, DVec3d::From (0,0,1), Angle::FromDegrees (0.0));

    // Make curves that dance in and out ....
    auto traceA = ICurvePrimitive::CreateArc (DEllipse3d::From (b, b, 0, 1.5 * a, 0, 0, 0, b, 0, 0.0, Angle::TwoPi ()));
    auto traceB = ICurvePrimitive::CreateArc (DEllipse3d::From (c, c, 0, 2.0 *c, 0, 0, 0,  0.55 * c, 0, -0.75 * Angle::Pi (), Angle::Pi ()));
    auto traceC = ICurvePrimitive::CreateArc (DEllipse3d::From (b, b, 0, 1.5 * c, 0, 0, 0, 1.2 * c, 0, -0.75 * Angle::Pi (), Angle::Pi ()));

#define NumGeometrySet 3
    Transform transforms[NumGeometrySet][4];
    for (int pass = 0; pass < NumGeometrySet; pass++)
        {
        // output gridd has a column for each variety of geometry (lines, arcs, filleted boxes ...)
        // In each column the blocks are:
        // 0) clipper boxes and trace geometry
        // 1) "inside" geometry
        // 2) "mixed" geometry
        // 3) "outside" geometry
        for (size_t i = 0; i < 4; i++)
            {
            transforms[pass][i] = Transform::From (shift * pass, shift * i, 0);
            Check::SetTransform (transforms[pass][i]);
            Check::SaveTransformed (outerBox);
            Check::SaveTransformed (innerBox);
            }
        }
    ClipPlaneSet outerClip (convexOuterClip);
    ClipPlaneSet innerClip (convexInnerClip);

    // Make lines that join them in all combinations.
    // test in/out for each.
    // output with shift driven by the in out:
    bvector<IGeometryPtr> lines, testCurves;

    size_t numSample0 = 10;
    size_t numSample1 = 10;
    size_t numSample2 = 10;
    for (int pass = 0; pass < NumGeometrySet; pass++)
        {
        testCurves.clear ();
        lines.clear ();
        Check::SetTransform (transforms[pass][0]);
        if (pass == 1)
            {
            Check::SaveTransformed (*traceA);
            Check::SaveTransformed (*traceB);
            SampleGeometryCreator::AddLinesBetweenFractions (lines, *traceA, numSample1, *traceB, numSample1 * 2, 0.03);
            SampleGeometryCreator::AddArcsFromMajorAxisLines (testCurves, lines,
                    0.2, Angle::FromDegrees (10.0), Angle::FromDegrees (60.0));
            }
        else if (pass == 2)
            {
            Check::SaveTransformed (*traceA);
            Check::SaveTransformed (*traceC);
            SampleGeometryCreator::AddLinesBetweenFractions (lines, *traceA, numSample2, *traceC, numSample2, 0.03);
            SampleGeometryCreator::AddFilletedBoxesOnLines(testCurves, lines, 0.5, 0.25);
            }
        else
            {
            Check::SaveTransformed (*traceA);
            Check::SaveTransformed (*traceB);
            SampleGeometryCreator::AddLinesBetweenFractions (lines, *traceA, numSample0, *traceB, numSample0, 0.03);
            testCurves = lines;
            }
        size_t i0 = 0;
        size_t i1 = testCurves.size ();  // assign outside for easy debugger reset to target specific lines.
        for (size_t i = i0; i < i1; i++)
            {
            auto testGeometry = testCurves[i];
            ICurvePrimitivePtr testCurve = testGeometry->GetAsICurvePrimitive ();
            CurveVectorPtr     testCV    = testGeometry->GetAsCurveVector ();
            ClipPlaneContainment classification;
            if (testCurve.IsValid ())
                classification = ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*testCurve, outerClip, &innerClip);
            else if (testCV.IsValid ())
                classification = ClipPlaneSet::ClassifyCurveVectorInSetDifference (*testCV, outerClip, &innerClip, true);    // BUT as of now we know that it ignores the region condition.
            else
                continue;

            uint32_t index = (uint32_t)classification;
            if (index > 3) index = 0;       // should not happen --- classifications are 1,2,3
            Check::SetTransform (transforms[pass][index]);
            Check::SaveTransformed (testGeometry);
            }
        }

    Check::ClearGeometry ("ClipPlaneSet.ClassifySetDifference_ManyLines");
    }

void PushIfDistinct (bvector<DPoint3d> &points, DPoint3dCR xyz)
    {
    static double s_tolerance = 1.0e-12;
    if (points.size () == 0 || points.back ().DistanceXY (xyz) > s_tolerance)
        points.push_back (xyz);
    }

/*---------------------------------------------------------------------------------**//**
* On the line from pointA to pointB, evaluate each coordinate in lineCoordinates as interpolateAndPerpendicular.
* Append to points.
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void appendToFractal (bvector<DPoint3d> &points, DPoint3dCR pointA, DPoint3dCR pointB, bvector<DPoint2d> &pattern, int numRecursion, double perpendicularFactor)
    {
    DPoint3d point0 = pointA;
    PushIfDistinct (points, pointA);

    for (auto &uv : pattern)
        {
        DPoint3d point1 = DPoint3d::FromInterpolateAndPerpendicularXY (pointA, uv.x, pointB, perpendicularFactor * uv.y);
        if (numRecursion > 0)
            appendToFractal (points, point0, point1, pattern, numRecursion - 1, perpendicularFactor);
        PushIfDistinct (points, point1);
        point0 = point1;
        }
    PushIfDistinct (points, pointB);
    }

/*---------------------------------------------------------------------------------**//**
* Map the pattern onto each line segment in poles.
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void appendToFractal (bvector<DPoint3d> &points, bvector<DPoint3d> &poles, bvector<DPoint2d> &pattern, int numRecursion, double perpendicularFactor)
    {
    PushIfDistinct (points, poles[0]);
    for (size_t i = 0; i + 1 < poles.size (); i++)
        {
        if (numRecursion > 0)
            appendToFractal (points, poles[i], poles[i+1], pattern, numRecursion - 1, perpendicularFactor);
        PushIfDistinct (points, poles[i+1]);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Save the polyline data at each level of the tree.
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SaveTree (AlternatingConvexClipTreeNode const &root)
    {
    Check::SaveTransformed (root.m_points);
    for (auto &child: root.m_children)
        SaveTree (child);
    }

/*---------------------------------------------------------------------------------**//**
* outputLevel = (0 none, 1 diagonals and scatter samples, 2 all)
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void TestClipper (bvector<DPoint3d> const &points, AlternatingConvexClipTreeNode &root, int outputLevel = 1)
    {
    bvector<double> fractions;
    auto range = DRange3d::From (points);
    int halfCount = 20;
    double df = 0.8 / halfCount;
    for (int i = -halfCount; i <= halfCount; i++)
        fractions.push_back (0.5 - i * df);
    auto a = range.XLength () * 0.004;
    ClipPlane::GetEvaluationCount (true);
    UsageSums inSum, outSum;
    size_t id = 0;
    static size_t s_idPeriod = 29;
    for (auto fx : fractions)
        {
        for (auto fy : fractions)
            {
            id++;
            auto xyz = range.LocalToGlobal (fx, fy, 0);
            bool doOutput = outputLevel == 2
                || (outputLevel == 1 &&
                    DoubleOps::AlmostEqual (fabs(fx - 0.5), fabs (fy - 0.5)))
                || (outputLevel == 1 && (id % s_idPeriod) == 0);
            if (root.IsPointOnOrInside (xyz))
                {
                if (doOutput)
                    Check::SaveTransformedMarker (xyz, a);
                inSum.Accumulate (ClipPlane::GetEvaluationCount (true));
                }
            else
                {
                if (doOutput)
                    Check::SaveTransformedMarker (xyz, -a);
                outSum.Accumulate (ClipPlane::GetEvaluationCount (true));
                }
            }
        }
    size_t numTest = fractions.size () * fractions.size ();
    printf (" ClipperTest  (polygonPoints %d) (TestPoint %d) (IN %lf avg %lf max %lf) (OUT %lf avg %lf max %lf)\n",
            (int)points.size (),
            (int)numTest,
            inSum.Count (), inSum.Mean (), inSum.Max (),
            outSum.Count (), outSum.Mean (), inSum.Max ());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RecursveClipSets, Test1)
    {
    for (size_t numPoints : bvector<size_t>{5, 8, 12, 15, 23, 37, 67})
        {
        SaveAndRestoreCheckTransform shifter(10,0,0);
        bvector<DPoint3d> points;
        SampleGeometryCreator::StrokeUnitCircle (points, numPoints);
        points.pop_back ();
        double f0 = 0.4;
        double f = 0.3;
        double af = 1.4;
        for (size_t i = 1; i < numPoints;)
            {
            points[i].Scale (f0);
            if (numPoints > 10 && i + 2 < numPoints)
                {
                auto vector = points[i+1] - points[i];
                points[i+1] = points[i+1] + f * vector;
                f *= af;
                points[i+2] = points[i+2] + f * vector;
                if (f > 2.0)
                    f = 0.1;
                i += 4;
                }
            else
                i += 3;
            }
        Check::SaveTransformed (points);
        Check::Shift (0,5,0);
        AlternatingConvexClipTreeNode root;
        AlternatingConvexClipTreeNode::CreateTreeForPolygon (points, root);
        Check::Shift (0,5,0);
        SaveTree (root);
        TestClipper (points, root);
        }
    Check::ClearGeometry ("RecursiveClipSets.Test1");
    }

// Primary shape is a "triangle" with lower edge pushed in so it becomes a mild nonconvex quad.
// Fractal effects are gentle.
void NonConvexQuadSimpleFractal(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor)
    {
    points.clear ();
    bvector<DPoint2d> pattern {
        DPoint2d::From (0, 0),
        DPoint2d::From (0.5, 0.1),
        DPoint2d::From (1.0, 0.0)
        };
    bvector<DPoint3d> poles {
        DPoint3d::From (0,0,0),
        DPoint3d::From (0.6,0.1,0),
        DPoint3d::From (1,0.1,0),
        DPoint3d::From (0.6,1,0),
        DPoint3d::From (0,0,0)
        };
    appendToFractal (points, poles, pattern, numRecursion, perpendicularFactor);
    }

void Fractal0 (bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor)
    {
    points.clear ();
    bvector<DPoint2d> pattern {
        DPoint2d::From (0, 0),
        DPoint2d::From (0.25, 0),
        DPoint2d::From (0.5, 0.2),
        DPoint2d::From (0.75, -0.1),
        DPoint2d::From (1.0, 0.0)
        };
    bvector<DPoint3d> poles {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (1,1,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (0,0,0)
        };
    appendToFractal (points, poles, pattern, numRecursion, perpendicularFactor);
    }

void Fractal1 (bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor)
    {
    points.clear ();
    bvector<DPoint2d> pattern {
        DPoint2d::From (0, 0),
        DPoint2d::From (0.25, 0),
        DPoint2d::From (0.5, 0.2),
        DPoint2d::From (0.75, -0.1),
        DPoint2d::From (1.0, 0.0)
        };
    bvector<DPoint3d> poles {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (1,1,0),
        DPoint3d::From (2,2,0),
        DPoint3d::From (2,3,0),
        DPoint3d::From (0,3,0),
        DPoint3d::From (0,0,0)
        };
    appendToFractal (points, poles, pattern, numRecursion, perpendicularFactor);
    }

// A fractal with fewer concavity changes...
void Fractal2 (bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor)
    {
    points.clear ();
    bvector<DPoint2d> pattern {
        DPoint2d::From (0, 0),
        DPoint2d::From (0.25, 0.1),
        DPoint2d::From (0.5, 0.15),
        DPoint2d::From (0.75, 0.1),
        DPoint2d::From (1.0, 0.0)
        };
    bvector<DPoint3d> poles {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (1,1,0),
        DPoint3d::From (2,2,0),
        DPoint3d::From (2,3,0),
        DPoint3d::From (0,3,0),
        DPoint3d::From (0,0,0)
        };
    appendToFractal (points, poles, pattern, numRecursion, perpendicularFactor);
    }


// A diamond with simple wave fractal
void FractalA (bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor)
    {
    points.clear ();
    bvector<DPoint2d> pattern {
        DPoint2d::From (0, 0),
        DPoint2d::From (0.3, 0.1),
        DPoint2d::From (0.5, 0.15),
        DPoint2d::From (0.7, 0.1),
        DPoint2d::From (1.0, 0.0)
        };
    bvector<DPoint3d> poles {
        DPoint3d::From (0,-1,0),
        DPoint3d::From (1,0,0),
        DPoint3d::From (0,1,0),
        DPoint3d::From (-1,0,0),
        DPoint3d::From (0,-1,0)
        };
    appendToFractal (points, poles, pattern, numRecursion, perpendicularFactor);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RecursiveClipSets,Test2)
    {
    for (auto perpendicularFactor : {-1.0, 1.0})
        {
        for (auto generatorFunction : {FractalA, Fractal0, Fractal1, Fractal2})
            {
            SaveAndRestoreCheckTransform shifter(0,20,0);
            for (int numRecursion = 0; numRecursion < 4; numRecursion++)
                {
                SaveAndRestoreCheckTransform shifter(10,0,0);
                bvector<DPoint3d> points;
                generatorFunction (points, numRecursion, perpendicularFactor);
                Check::SaveTransformed (points);
                Check::Shift (0,5,0);
                AlternatingConvexClipTreeNode root;
                AlternatingConvexClipTreeNode::CreateTreeForPolygon (points, root);
                SaveTree (root);
                TestClipper (points, root);
                }
            }
        }
    Check::ClearGeometry ("RecursiveClipSets.Test2");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RecursiveClipSets,Test3)
    {
    // A diamond, but with the diagonals pushed inward so no full edge of the polygon is on the hull.
    bvector<DPoint3d> points
        {
        DPoint3d::From (5,0,0),
        DPoint3d::From (2,1,0),
        DPoint3d::From (1,2,0),

        DPoint3d::From (0,5,0),

        DPoint3d::From (-1,2,0),
        DPoint3d::From (-2,1,0),

        DPoint3d::From (-5,0,0),

        DPoint3d::From (-2,-1,0),
        DPoint3d::From (-1,-2,0),

        DPoint3d::From (0,-5,0),

        DPoint3d::From (1,-2,0),
        DPoint3d::From (2,-1,0)

        //DPoint3d::From (5,0,0),
        };

    Check::SaveTransformed (points);
    Check::Shift (0,25,0);
    AlternatingConvexClipTreeNode root;
    AlternatingConvexClipTreeNode::CreateTreeForPolygon (points, root);
    SaveTree (root);
    TestClipper (points, root);
    Check::ClearGeometry ("RecursiveClipSets.Test3");
    }


void ClipPathA (DVec3dCR shift, double scale, bvector<DPoint3d> &points)
    {
    points.clear ();
    bvector<DPoint3d> clipPathA {
        DPoint3d::From (-1,-1),
        DPoint3d::From (0.5,-1),
        DPoint3d::From (0.5,0.5),
        DPoint3d::From (0.6,0.6),
        DPoint3d::From (1.3,0.8),
        DPoint3d::From (0.9, 1.4)
    };
    for (auto &xyz : clipPathA)
        {
        points.push_back (xyz + scale * shift);
        }
    }

void ClipAndSave (AlternatingConvexClipTreeNode &root, ICurvePrimitiveCR curve)
    {
    bvector<CurveLocationDetailPair> inside, outside;
    root.AppendCurvePrimitiveClipIntervals (curve, &inside, &outside);
    for (auto &pair : inside)
        {
        auto r = curve.CloneBetweenFractions (pair.detailA.fraction, pair.detailB.fraction, true);
        if (r.IsValid ())
            Check::SaveTransformed (*r);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RecursiveClipSets,LineClip0)
    {
    bvector<DPoint3d> polygon;
    bvector<DPoint3d> linesToClip;
    DVec3d baseShift = DVec3d::From (-0.1, -0.1, 0);
    for (auto perpendicularFactor : {-1.0, 1.0})
        {
        for (auto generatorFunction : {NonConvexQuadSimpleFractal, FractalA, Fractal0, Fractal1, Fractal2})
            {
            SaveAndRestoreCheckTransform shifter(50,0,0);
            for (int depth = 0; depth < 3; depth++)
                {
                SaveAndRestoreCheckTransform shifter(5,0,0);
                generatorFunction (polygon, depth, perpendicularFactor);
                AlternatingConvexClipTreeNode root;
                AlternatingConvexClipTreeNode::CreateTreeForPolygon (polygon, root);
                SaveTree (root);

                ClipPathA (baseShift, 0.0, linesToClip);
                Check::SaveTransformed (linesToClip);
                Check::Shift (0,4,0);
                Check::SaveTransformed (polygon);
                for (double s : {0.0, 1.1, 2.3})
                    {
                    ClipPathA (baseShift, s, linesToClip);

                    for (size_t i0 = 0; i0 + 1 < linesToClip.size (); i0++)
                        {
                        auto lineSegment = ICurvePrimitive::CreateLine (DSegment3d::From (linesToClip[i0], linesToClip[i0+1]));
                        ClipAndSave (root, *lineSegment);
                        }
                    }
                Check::Shift (0,5,0);
                Check::SaveTransformed (polygon);
                ClipPathA (baseShift, 0.0, linesToClip);
                Check::SaveTransformed (linesToClip);
                Check::Shift (0,4,0);
                Check::SaveTransformed (polygon);
                ClipAndSave (root, *ICurvePrimitive::CreateLineString (linesToClip));

                Check::Shift (0,5,0);
                Check::SaveTransformed (polygon);
                DEllipse3d arc0 = DEllipse3d::From (
                    0.5, 0.5, 0,
                    0.5, 1,0,
                    -0.2, 0.2, 0,
                    Angle::DegreesToRadians (-120),
                    Angle::DegreesToRadians (240));
                auto cpArc = ICurvePrimitive::CreateArc (arc0);
                Check::SaveTransformed (*cpArc);
                Check::Shift (0,4,0);
                Check::SaveTransformed (polygon);
                ClipAndSave (root, *cpArc);

                Check::Shift (0,5,0);
                Check::SaveTransformed (polygon);
                auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder (
                        bvector<DPoint3d>{
                        DPoint3d::From (0, -0.2),
                        DPoint3d::From (1,0.3),
                        DPoint3d::From (1.2, 0.8),
                        DPoint3d::From (0.5,1.0),
                        //DPoint3d::From (-0.1, 0.1),   // some plane cuts missed with sharp cusp?
                        DPoint3d::From (-0.3, 0.1),
                        //DPoint3d::From (0, 0.5),
                        DPoint3d::From (0, 0.8),
                        DPoint3d::From (0.5,1.3)
                            },
                        nullptr, nullptr,
                        4, false, true);
                auto cpBCurve = ICurvePrimitive::CreateBsplineCurve (bcurve);
                Check::SaveTransformed (*cpBCurve);
                Check::Shift (0,4,0);
                Check::SaveTransformed (polygon);
                ClipAndSave (root, *cpBCurve);

                Check::Shift (0,5,0);
                Check::SaveTransformed (polygon);
                auto cpLineString = ICurvePrimitive::CreateLineString (
                bvector<DPoint3d>{
                      DPoint3d::From(0, -0.2),
                      DPoint3d::From(1, 0.3),
                      DPoint3d::From(1.2, 0.8),
                      DPoint3d::From(0.5, 1.0),
                      DPoint3d::From(-0.3, 0.1),
                      DPoint3d::From(0, 0.8),
                      DPoint3d::From(0.5, 1.3)}
                    );
                Check::SaveTransformed (*cpLineString);
                Check::Shift (0,4,0);
                Check::SaveTransformed (polygon);
                ClipAndSave (root, *cpLineString);
                }
            }
        }
    Check::ClearGeometry ("RecursiveClipSets.LineClip0");
    }

// Output the clip of a rectangle with each convex set.
void ShowClipperXY (ClipPlaneSetCR clipper, double x0, double y0, double x1, double y1, double z = 0.0)
    {
    auto points = bvector<DPoint3d> {
        DPoint3d::From (x0, y0, z),
        DPoint3d::From (x1, y0, z),
        DPoint3d::From (x1, y1, z),
        DPoint3d::From (x0, y1, z)
        //, DPoint3d::From (x0, y0, 0)
        };
    bvector<DPoint3d> inside;
    bvector<DPoint3d> work;
    for (auto &convexSet : clipper)
        {
        convexSet.ConvexPolygonClip (points, inside, work);
        Check::SaveTransformed (inside, true);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,ClipToSetDifference_MutlipleClips_NoMasks)
    {
    auto polyface = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  0,0,1,    10,0,0,   0,4,0),
            3,2, true
            );
    polyface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    auto convexClipA = ConvexClipPlaneSet::FromXYBox (1,-1, 3,20);
    auto convexClipB = ConvexClipPlaneSet::FromXYBox (5,0.1, 12,2.8);
    bvector<BoolTypeForVector> hidden;   // empty vector for arg
    auto convexClipC = ConvexClipPlaneSet::FromXYPolyLine (
            bvector<DPoint3d> {DPoint3d::From (12, 6, 0), DPoint3d::From (15,0,0)}, 
            hidden, true);
    ClipPlaneSet clipPlaneSet;
    clipPlaneSet.push_back (convexClipA);
    clipPlaneSet.push_back (convexClipB);
    clipPlaneSet.push_back (convexClipC);
    ShowClipperXY (clipPlaneSet, -1,-1, 40, 10, 2);
    Check::SaveTransformed (*polyface);

    PolyfaceHeaderPtr inside, outside;
    ClipPlaneSet::ClipToSetDifference (*polyface, clipPlaneSet, nullptr, &inside, &outside);
    if (inside.IsValid ())
        {
        Check::Shift (0, 15,0);
        Check::SaveTransformed (*inside);
        }
    if (outside.IsValid ())
        {
        Check::Shift (0, 15,0);
        Check::SaveTransformed (*outside);
        }
    Check::ClearGeometry ("ClipPlaneSet.ClipToSetDifference_MutlipleClips_NoMasks");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,ClipToSetDifference_OneClip_MutlipleMasks)
    {
    auto polyface = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  0,0,1,    10,0,0,   0,4,0),
            3,2, true
            );
    polyface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    auto convexClipA = ConvexClipPlaneSet::FromXYBox (1,-1, 3,4);
    auto convexClipB = ConvexClipPlaneSet::FromXYBox (5,0.1, 12,2.8);
    bvector<BoolTypeForVector> hidden;   // empty vector for arg
    ClipPlaneSet masks;
    masks.push_back (convexClipA);
    masks.push_back (convexClipB);

    ClipPlaneSet clip;
    auto outer = ConvexClipPlaneSet::FromXYPolyLine (
            bvector<DPoint3d> {
                DPoint3d::From (-2, -3, 0),
                DPoint3d::From (18,-3,0),
                DPoint3d::From (10,8,0),
                DPoint3d::From (0,8,0),
                DPoint3d::From (-2, -3, 0),
                }, 
            hidden, true);
    clip.push_back (outer);

    ShowClipperXY (clip, -10,-10, 40, 10, 2);
    ShowClipperXY (masks, -10,-10, 40, 10, 2);

    Check::Shift (20, 10,0);
    PolyfaceHeaderPtr inside, outside;
    ClipPlaneSet::ClipToSetDifference (*polyface, clip, &masks, &inside, &outside);
    Check::SaveTransformed (*polyface);
    if (inside.IsValid ())
        {
        Check::Shift (0, 10,0);
        Check::SaveTransformed (*inside);
        }
    if (outside.IsValid ())
        {
        Check::Shift (0, 10,0);
        Check::SaveTransformed (*outside);
        }
    Check::ClearGeometry ("ClipPlaneSet.ClipToSetDifference_OneClip_MutlipleMasks");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet,ClipToSetDifference_MultipleClips_MutlipleMasks)
    {
    auto polyface = UnitGridPolyface
            (
            DPoint3dDVec3dDVec3d (  0,0,1,    10,0,0,   0,4,0),
            6,4, true
            );
    auto range = polyface->PointRange ();
    double yStep = range.YLength () + 2;
    polyface->ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    auto convexClipA = ConvexClipPlaneSet::FromXYBox (1,-1, 3,4);
    auto convexClipB = ConvexClipPlaneSet::FromXYBox (5,0.1, 12,2.8);
    bvector<BoolTypeForVector> hidden;   // empty vector for arg
    ClipPlaneSet masks;
    masks.push_back (convexClipA);
    masks.push_back (convexClipB);

    ClipPlaneSet clip;
    auto outerA = ConvexClipPlaneSet::FromXYPolyLine (
            bvector<DPoint3d> {
                DPoint3d::From (-2, -3, 0),
                DPoint3d::From (18,-3,0),
                DPoint3d::From (10,8,0),
                DPoint3d::From (0,8,0),
                DPoint3d::From (-2, -3, 0),
                }, 
            hidden, true);
    auto outerB = ConvexClipPlaneSet::FromXYPolyLine (
            bvector<DPoint3d> {
                DPoint3d::From (11,8,0),
                DPoint3d::From (19,-3,0),
                DPoint3d::From (30,0,0),
                DPoint3d::From (30,1,0),
                }, 
            hidden, true);
    clip.push_back (outerA);
    clip.push_back (outerB);

    ShowClipperXY (clip, -10,-10, 40, 10, 2);
    ShowClipperXY (masks, -10,-10, 40, 10, 2);
    Check::SaveTransformed (*polyface);

    PolyfaceHeaderPtr inside, outside;
    ClipPlaneSet::ClipToSetDifference (*polyface, clip, &masks, &inside, &outside);
    if (inside.IsValid ())
        {
        Check::Shift (0, yStep,0);
        Check::SaveTransformed (*inside);
        }
    if (outside.IsValid ())
        {
        Check::Shift (0, yStep,0);
        Check::SaveTransformed (*outside);
        }


    // extract each pair of consecutive triangles as a small mesh.  Do both clip and full classify and confirm same classification
    size_t targetPerBlock = 2;
    auto visitor = PolyfaceVisitor::Attach (*polyface);
    PolyfaceHeaderPtr smallPolyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        smallPolyface->AddPolygon (visitor->Point ());
        if (smallPolyface->GetNumFacet () >= targetPerBlock)
            {
            ClipPlaneSet::ClipToSetDifference (*smallPolyface, clip, &masks, &inside, &outside);
            size_t numIn = inside->GetNumFacet ();
            size_t numOut = outside->GetNumFacet ();
            auto c0 = ClipPlaneSet::ClassifyPolyfaceInSetDifference (*smallPolyface, clip, &masks);
            auto c1 = ClipPlaneContainment::ClipPlaneContainment_Ambiguous;
            if (numIn > 0 && numOut == 0)
                c1 = ClipPlaneContainment::ClipPlaneContainment_StronglyInside;
            if (numOut > 0 && numIn == 0)
                c1 = ClipPlaneContainment::ClipPlaneContainment_StronglyOutside;
            Check::Int ((int)c0, (int)c1, "polyface in/out clipSets via clip, classify");
            PolyfaceHeaderPtr smallPolyface = PolyfaceHeader::CreateVariableSizeIndexed ();
            }
        }
    Check::ClearGeometry ("ClipPlaneSet.ClipToSetDifference_MultipleClips_MutlipleMasks");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlane,ClipPlaneToRange)
    {
    // A range .. happens to contain the origin, but other than that avoids unit points . . 
    DRange3d range = DRange3d::From (-3,-2,-4, 1.5, 2.2, 3.2);
    // A segment with non-principal direction . .
    auto segment = DSegment3d::From (-12,-3,4, 5,10, -3);

    bvector<DSegment3d> edges;
    range.GetEdges (edges);

    // Various normals ....
    for (auto normal : {
            DVec3d::From (1,0,0),
            DVec3d::From (0,1,0),
            DVec3d::From (0,0,1),
            DVec3d::From (1,1,1),
            DVec3d::From (-1,1,1),
            DVec3d::From (-1,1,-1),
            })
        {
        SaveAndRestoreCheckTransform (0,50,0);
        bvector<DPoint3d> outerPolygon;
        for (auto &e : edges)
            Check::SaveTransformed (e);
        Check::SaveTransformed (segment);
        bvector<DPoint3d> firstOuter, lastOuter;
        // scatter plane origins along the segment ..
        // clip each plane to the range
        for (double fraction = 0.0; fraction<= 1.0; fraction+= 1.0 / 32.0)
            {
            auto plane = DPlane3d::FromOriginAndNormal (
                    segment.FractionToPoint (fraction),
                    normal
                    );
            bvector<DPoint3d> polygon;
            ClipPlane::ClipPlaneToRange (range, plane, polygon, &outerPolygon);
            if (polygon.size () > 0)
                {
                DPoint3dOps::AppendClosure (polygon);
                Check::SaveTransformed (polygon);

                if (firstOuter.size () == 0)
                    firstOuter = outerPolygon;
                lastOuter = outerPolygon;
                }
            }
        Check::SaveTransformed (firstOuter);
        Check::SaveTransformed (lastOuter);
        }
    Check::ClearGeometry ("ClipPlaneSet.ClipPlaneToRange");
    }
TEST(ClipPlaneSet,ClipRegion)
    {
    auto shapeL = CreateL (-4,0, 4, 1, 2,6);
    auto holeL = CreateL (-2, 1, 0, 2, -1, 3);
    ClipPlaneSet clipper = ClipPlaneSet::FromSweptPolygon (&shapeL[0], shapeL.size ());
    ClipPlaneSet holeClipper = ClipPlaneSet::FromSweptPolygon (&holeL[0], holeL.size ());
    DPoint3d xyzA = shapeL.front (); shapeL.push_back (xyzA);
    DPoint3d xyzB = holeL.front (); holeL.push_back (xyzB);

    for (auto useHole : {false, true})
        {
        auto region = CreateFilletedSymmetricT (5.0, 8.0, 1.0, 4.0, 0.5, 0.4);
        SaveAndRestoreCheckTransform shifter (30,0,0);
        auto baseTransform = Check::GetTransform ();
        for (double dz = 0.0; dz < 3.2; dz += 0.5)
            {
            Check::Shift (0,0,dz);
            Check::SaveTransformed (shapeL);
            if (useHole)
                Check::SaveTransformed (holeL);
            }
        Check::SetTransform (baseTransform);
    
        //Check::SaveTransformed (region);
        DSegment3d segment = DSegment3d::From (DPoint3d::From (5,-10, 0), DPoint3d::From (4,10,1));
        Check::SaveTransformed (segment);
        DVec3d vector = segment.VectorStartToEnd ();
        vector.Normalize ();
        for (auto degrees : {0.0, 5.0, 10.0, 20.0, 30.0, 40.0})
            {
            SaveAndRestoreCheckTransform shifter (0,0,0);
            auto transform = Transform::FromMatrixAndFixedPoint (
                    RotMatrix::FromVectorAndRotationAngle (vector, Angle::DegreesToRadians (degrees)),
                    segment.point[0]);
            region->TransformInPlace (transform);
            auto clipped = clipper.ClipAndMaskPlanarRegion (
                &clipper,
                useHole ? &holeClipper : nullptr,
                *region);
            if (clipped.IsValid ())
                {
                Check::SaveTransformed (*clipped);
                }
            Check::Shift (0,10,0);
            Check::SaveTransformed (*region);
            }
        }
    Check::ClearGeometry ("ClipPlaneSet.ClipRegion");
    }