/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <stdio.h>
IPolyfaceConstructionPtr CreateBuilder(bool normals, bool params);
#include <Geom/BinaryRangeHeap.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SaveTree (AlternatingConvexClipTreeNode const &root)
    {
    Check::SaveTransformed (root.m_points);
    for (auto &child: root.m_children)
        SaveTree (child);
    }

/*---------------------------------------------------------------------------------**//**
* outputLevel = (0 none, 1 diagonals and scatter samples, 2 all)
* @bsimethod
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
* @bsimethod
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

// A fractal with deep interior gouge in reverse C shape ...
void Fractal3(bvector<DPoint3d> &points, int numRecursion, double perpendicularFactor)
    {
    points.clear();
    bvector<DPoint2d> pattern{
        DPoint2d::From(0, 0),
        DPoint2d::From(0.25, 0.1),
        DPoint2d::From(0.5, 0.15),
        DPoint2d::From(0.75, 0.1),
        DPoint2d::From(1.0, 0.0)
        };
    bvector<DPoint3d> poles{
        DPoint3d::From(0,0,0),
        DPoint3d::From(4,0,0),
        DPoint3d::From(4,4,0),
        DPoint3d::From(0,4,0),
        DPoint3d::From(0,3.0,0),
        DPoint3d::From(3,2.5,0),
        DPoint3d::From(3,1.5,0),
        DPoint3d::From(0,1,0),
        DPoint3d::From(0,0,0),
        };
    appendToFractal(points, poles, pattern, numRecursion, perpendicularFactor);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RecursiveClipSets,Test2)
    {
    for (auto perpendicularFactor : {-1.0, 1.0})
        {
        for (auto generatorFunction : {FractalA, Fractal0, Fractal1, Fractal2})
            {
            SaveAndRestoreCheckTransform shifter1(0,20,0);
            for (int numRecursion = 0; numRecursion < 4; numRecursion++)
                {
                SaveAndRestoreCheckTransform shifter2(10,0,0);
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
* @bsimethod
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
* @bsimethod
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
            SaveAndRestoreCheckTransform shifter1(50,0,0);
            for (int depth = 0; depth < 3; depth++)
                {
                SaveAndRestoreCheckTransform shifter2(5,0,0);
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
            PolyfaceHeaderPtr smallPolyface1 = PolyfaceHeader::CreateVariableSizeIndexed ();
            }
        }
    Check::ClearGeometry ("ClipPlaneSet.ClipToSetDifference_MultipleClips_MutlipleMasks");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        auto region = CreateFilletedSymmetricT (5.0, 8.0, 1.0, 4.0, 0.5, 0.4, false);
        SaveAndRestoreCheckTransform shifter1 (30,0,0);
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
            SaveAndRestoreCheckTransform shifter2 (0,0,0);
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet, MultiConvexClassify)
    {
    ConvexClipPlaneSet convexSetA, convexSetB;
    double a = 12.0;
    double b = 6.0;
    DPoint3d interiorPoint = DPoint3d::From (b,b);
    bvector<DPoint3d> rightLeg{
        DPoint3d::From(0,0),
        DPoint3d::From(a,0),
        DPoint3d::From(a,b),
        interiorPoint,      // return to origin is back() which is interior
        DPoint3d::From(0,0)
        };
    bvector<DPoint3d> upperLeg{
        DPoint3d::From(0,0),    // front() should be the interior plane.
        interiorPoint,
        DPoint3d::From(b,a),
        DPoint3d::From(0,a),
        DPoint3d::From(0,0)
        };
    DVec3d sweepDirection = DVec3d::From (0,0,1);
    convexSetA.ReloadSweptConvexPolygon(rightLeg, sweepDirection, 0);
    uint32_t interiorFlags = 0x03; // ClipPlane::PlaneMask_Interior | ClipPlane::PlaneMask_Invisible;
    convexSetA.back ().SetFlags(interiorFlags);
    convexSetB.ReloadSweptConvexPolygon(upperLeg, sweepDirection, 0);
    convexSetB.front ().SetFlags (interiorFlags);
    ClipPlaneSet clipper;
    clipper.push_back (convexSetA);
    clipper.push_back(convexSetB);
    auto bigBox = ISolidPrimitive::CreateDgnBox(DgnBoxDetail(
        DPoint3d::From(-1, -1, -3),
        DPoint3d::From(-1, -1, 4),
        DVec3d::From(1, 0, 0),
        DVec3d::From(0, 1, 0),
        12, 12, 12, 12, true
    ));
    auto boxA = ISolidPrimitive::CreateDgnBox(DgnBoxDetail(
        DPoint3d::From (1,1,0),
        DPoint3d::From (1,1,1),
        DVec3d::From (1,0,0),
        DVec3d::From (0,1,0),
        2,2,1,1, true
        ));
    auto boxB = ISolidPrimitive::CreateDgnBox(DgnBoxDetail(
        DPoint3d::From(1, 1, 0),
        DPoint3d::From(1, 1, 1),
        DVec3d::From(1, 1, 0),
        DVec3d::From(-1, 1, 0),
            3, 4, 1, 1, true
            ));
    double e = b/2.0;
    auto solidC = ISolidPrimitive::CreateDgnTorusPipe(DgnTorusPipeDetail(
        DPoint3d::From(e, e, 1),
        DVec3d::From(1, 0,  0),
        DVec3d::From(0, 1, 0),
        5.0, 1.5, Angle::TwoPi(), false));
    double theta = Angle::DegreesToRadians (42);
    auto solidD = ISolidPrimitive::CreateDgnTorusPipe(DgnTorusPipeDetail(
        DPoint3d::From(e, e, 1),
        DVec3d::From(cos(theta), sin(theta), 0),
        DVec3d::From(0, 0, 1),
        1.1, 0.5, Angle::TwoPi(), false));

    auto builder0 = CreateBuilder(false, false);
    builder0->AddSolidPrimitive(*bigBox);
    auto mesh0 = builder0->GetClientMeshPtr();

    auto builderA = CreateBuilder (false, false);
    builderA->AddSolidPrimitive (*boxA);
    auto meshA = builderA->GetClientMeshPtr ();

    auto builderB = CreateBuilder(false, false);
    builderB->AddSolidPrimitive(*boxB);
    auto meshB = builderB->GetClientMeshPtr();

    auto builderC = CreateBuilder(false, false);
    builderC->AddSolidPrimitive(*solidC);
    auto meshC = builderC->GetClientMeshPtr();

    auto builderD = CreateBuilder(false, false);
    // builderD->GetFacetOptionsR().SetAngleTolerance (Angle::DegreesToRadians (45.0));
    builderD->AddSolidPrimitive(*solidD);
    auto meshD = builderD->GetClientMeshPtr();

    for (auto &mesh : { meshD, mesh0, meshA, meshB, meshC, meshD})
        {
        SaveAndRestoreCheckTransform shifter (2 * a, 0, 0);
        Check::SaveTransformed(DSegment3d::From(0, 0, 0, 0, 2 * a, 0));
        Check::SaveTransformed (rightLeg);
        Check::SaveTransformed (upperLeg);
        Check::SaveTransformed (*mesh);
        auto inside = PolyfaceHeader::CreateVariableSizeIndexed ();
        auto outside = PolyfaceHeader::CreateVariableSizeIndexed();
        ClipPlaneSet::ClipPlaneSetIntersectPolyface (*mesh, clipper, true, &inside, &outside);
        auto containment = ClipPlaneSet::ClassifyPolyfaceInSetDifference(*mesh, clipper, nullptr);
        printf (" containment %d\n", containment);
        Check::Shift (0, containment == 1 ? a : 3 * a, 0);
        Check::SaveTransformed (*inside);
        Check::Shift(0, 1.5 * a, 0);
        Check::SaveTransformed(*outside);

        bvector<bvector<DPoint3d>> sectionLines;
        // auto section = PolyfaceHeader::CreateVariableSizeIndexed();
        // auto edgeTolerance = ValidatedDouble (DoubleOps::SmallMetricDistance ());
        // ClipPlaneSet::ClipPlaneSetSectionPolyface(*mesh, clipper, &section, &sectionLines, edgeTolerance);
        ClipPlaneSet::ClipPlaneSetPolyfaceIntersectionEdges (*mesh, clipper, sectionLines);

        // Check::Shift(0, 1.5 * a, 0);
        // Check::SaveTransformed(*section);
        Check::Shift(0, 1.5 * a, 0);
        for (auto &s : sectionLines)
            {
            Check::SaveTransformed(s);
            }

        }
    Check::ClearGeometry ("ClipPlaneSet.MultiConvexClassify");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ClipPlaneSet, ClassifySetDifference)
    {
    double a = 15.0;
    double b = 6.0;
    double c = 18.0;
    bvector<DPoint3d> outerClip {
        DPoint3d::From(0,0),
        DPoint3d::From(a,0),
        DPoint3d::From(a,a),
        DPoint3d::From(0,a),
        DPoint3d::From(0,0)
        };
    bvector<DPoint3d> chopper {
        DPoint3d::From(b,b),
        DPoint3d::From(c,b),
        DPoint3d::From(c,c),
        DPoint3d::From(b,c),
        DPoint3d::From(b,b)
        };
    DVec3d sweepDirection = DVec3d::From(0, 0, 1);

    ConvexClipPlaneSet convexSetA, convexSetB;
    convexSetA.ReloadSweptConvexPolygon(outerClip, sweepDirection, 0);
    convexSetB.ReloadSweptConvexPolygon(chopper, sweepDirection, 0);

    ClipPlaneSet clipperA, clipperB;
    clipperA.push_back(convexSetA);
    clipperB.push_back(convexSetB);

    auto bigBox = ISolidPrimitive::CreateDgnBox(DgnBoxDetail(
        DPoint3d::From(-1, -1, -1),
        DPoint3d::From(-1, -1, 2),
        DVec3d::From(1, 0, 0),
        DVec3d::From(0, 1, 0),
        12, 12, 12, 12, true
    ));
    auto boxA = ISolidPrimitive::CreateDgnBox(DgnBoxDetail(
        DPoint3d::From(1, 1, 0),
        DPoint3d::From(1, 1, 1),
        DVec3d::From(1, 0, 0),
        DVec3d::From(0, 1, 0),
        2, 2, 1, 1, true
    ));
    auto boxB = ISolidPrimitive::CreateDgnBox(DgnBoxDetail(
        DPoint3d::From(1.1, 1, 0),
        DPoint3d::From(1.1, 1, 1),
        DVec3d::From(1, 1, 0),
        DVec3d::From(-1, 1, 0),
        3, 4, 1, 1, true
    ));

    auto builder0 = CreateBuilder(false, false);
    builder0->AddSolidPrimitive(*bigBox);
    auto mesh0 = builder0->GetClientMeshPtr();

    auto builderA = CreateBuilder(false, false);
    builderA->AddSolidPrimitive(*boxA);
    auto meshA = builderA->GetClientMeshPtr();
    auto builderB = CreateBuilder(false, false);
    builderB->AddSolidPrimitive(*boxB);
    auto meshB = builderB->GetClientMeshPtr();

    for (auto &mesh : { mesh0, meshA, meshB })
        {
        SaveAndRestoreCheckTransform shifter(2 * a, 0, 0);
        Check::SaveTransformed (DSegment3d::From (0,-3,0,2 *a, -3,0));
        Check::SaveTransformed(*mesh);
        auto inside = PolyfaceHeader::CreateVariableSizeIndexed();
        auto outside = PolyfaceHeader::CreateVariableSizeIndexed();
        ClipPlaneSet::ClipToSetDifference(*mesh, clipperA, &clipperB, &inside, &outside);
        Check::Shift(0, 2 * a, 0);
        Check::SaveTransformed(*inside);
        }
    Check::ClearGeometry("ClipPlaneSet.ClassifySetDifference");
    }


// Demonstrate "ClipPlaneSet in range" clip where the ClipPlaneSet is two disjoint ranges.
TEST(ClipPlaneSet, BoundedClipperFacesInRange)
    {
    ConvexClipPlaneSet clipper1 (DRange3d::From (2,2,2, 6,5,3));
    ConvexClipPlaneSet clipper2 (DRange3d::From (7,7,7, 10,8,40));
    ClipPlaneSet clipper;
    clipper.push_back (clipper1);
    clipper.push_back (clipper2);

    auto bigBoxMesh = PolyfaceHeader::CreateDRange3dFaces(DRange3d::From(1, 2, 1, 15, 20, 20), true);

    auto inside = PolyfaceHeader::CreateVariableSizeIndexed();
    ClipPlaneSet::ClipPlaneSetIntersectPolyface(*bigBoxMesh, clipper, true, &inside, nullptr);
    Check::SaveTransformed (*inside);
    Check::SaveTransformed(*bigBoxMesh);


    Check::ClearGeometry("ClipPlaneSet.BoundedClipperFacesInRange");
    }


// Demonstrate "ClipPlaneSet in range" clip where the ClipPlaneSet is an unbounded swept polyline.
TEST(ClipPlaneSet, UnboundedClipperFacesInRange)
    {
    Check::Shift (40,0,0);
    bvector<DPoint3d> clipperContour;
    clipperContour.push_back(DPoint3d::From(27, 26, 0));
    clipperContour.push_back(DPoint3d::From(7, 4, 0));
    clipperContour.push_back(DPoint3d::From(-8, 4, 0));

    ConvexClipPlaneSet clipper1;
    clipper1.AddSweptPolyline(clipperContour, DVec3d::From(0, 0.7, 1), Angle::FromDegrees(0.0));
    ClipPlaneSet clipper;
    clipper.push_back (clipper1);
    Transform placement = Transform::From (
        RotMatrix::FromVectorAndRotationAngle(DVec3d::From (1,2,5), 0.8),
        DPoint3d::From (1,2,3)
    );
    auto bigBoxMesh = PolyfaceHeader::CreateDRange3dFaces (DRange3d::From(1, 2, 1, 15, 20, 20), true, &placement);
    //PolyfaceFromRange(DRange3d::From(1, 2, 1, 15, 20, 20));

    auto inside = PolyfaceHeader::CreateVariableSizeIndexed();
    ClipPlaneSet::ClipPlaneSetIntersectPolyface(*bigBoxMesh, clipper, true, &inside, nullptr);
    Check::SaveTransformed(*inside);
    Check::SaveTransformed (*bigBoxMesh);
    Check::SaveTransformed (clipperContour);

    Check::ClearGeometry("ClipPlaneSet.UnboundedClipperFacesInRange");
    }


TEST(ClipPlaneSet, ClipSegmentToIntersection)
    {
    double yShift = 20.0;
    double xShift = 20.0;
    bvector<DPoint3d> world{
        DPoint3d::From (-4,-4,0),
        DPoint3d::From(10,-4,0),
        DPoint3d::From (10,10,0),
        DPoint3d::From(-4,10,0),
        DPoint3d::From(-4,-4,0),
        };
    auto convexA0 = ConvexClipPlaneSet::FromXYBox(0, 0.1, 3,3.8);
    auto convexA1 = ConvexClipPlaneSet::FromXYBox(4, 0, 6, 1);
    auto convexB  = ConvexClipPlaneSet::FromXYBox (1,-1, 5,4);
    ClipPlaneSetCompoundClipContext clipContext;
    for (auto &convexClipper : { convexA0, convexA1, convexB })
        {
        bvector<DPoint3d> profile;
        bvector<DPoint3d> work;
        convexClipper.ConvexPolygonClip (world, profile, work);
        DPoint3d xyz = profile.front ();
        profile.push_back(xyz);
        for (int i = 0; i < 3; i++)
            {
            Check::Shift(i * xShift, 0);
            Check::SaveTransformed (profile);
            Check::Shift (0, yShift);
            Check::SaveTransformed(profile);
            Check::Shift(0, -yShift);
            Check::Shift(- i * xShift, 0);
            }
        }
    ClipPlaneSet setA, setB;
setA.push_back(convexA0);
    setA.push_back(convexA1);
    setB.push_back(convexB);

    ClipPlaneSetsWithOpcodes clippers (true);
    clippers.AppendClipStep (&setA, 1);
    clippers.AppendClipStep(&setB, 1);

    double x0 = -1.0;
    double x1 = 8.0;
    for (double y : {-2.0, -0.5, 0.5, 1.5, 4.0})
        {
        auto segment = DSegment3d::From (x0, y, 0, x1, y + 0.2, 0);
        bvector<DSegment3d> clippedSegments;
    clipContext.BooleanCombinationOfLineClips(clippers, segment, nullptr, &clippedSegments);
        Check::SaveTransformed (segment);
        Check::Shift (0, yShift);
        for (auto &s : clippedSegments)
            Check::SaveTransformed(s);
        Check::Shift (0, -yShift);
        }

    auto arc = ICurvePrimitive::CreateArc (
        DEllipse3d::From (3,2,0, 3,0,0, 0,2, 0, 0.0, Angle::TwoPi ()));
    bvector<DPoint3d> strokes;
    auto options = IFacetOptions::CreateForCurves ();
    arc->AddStrokes (strokes, *options);
    for (uint32_t i = 0; i < 2; i++)
        {
        bvector<bvector<DPoint3d>> chains;
        clipContext.ClipPolylineToBooleanCombinationOfLineClips(clippers, strokes, chains);
        Check::Shift(20, 0);
        Check::SaveTransformed(strokes);
        Check::Shift(0, yShift);
        for (auto &chain : chains)
            Check::SaveTransformed(chain);
        Check::Shift(0, -yShift);
        std::reverse(std::begin(strokes), std::end(strokes));
        }
    Check::ClearGeometry("ClipPlaneSet.ClipSegmentToIntersection");
    }
TEST(ClipPlaneSet, ClipSegmentToBoolean)
    {
    double yShift = 20.0;
    double xShift = 20.0;
    bvector<DPoint3d> world{
        DPoint3d::From(-4,-4,0),
        DPoint3d::From(10,-4,0),
        DPoint3d::From(10,10,0),
        DPoint3d::From(-4,10,0),
        DPoint3d::From(-4,-4,0),
        };
    auto convexA0 = ConvexClipPlaneSet::FromXYBox(0, 0.1, 3, 3.8);
    auto convexA1 = ConvexClipPlaneSet::FromXYBox(4, 0, 6, 1);
    auto convexB = ConvexClipPlaneSet::FromXYBox(1, -1, 5, 4);
    bvector<bvector<DPoint3d>> allLoops;
    for (auto &convexClipper : { convexA0, convexA1, convexB })
        {
        bvector<DPoint3d> profile;
        bvector<DPoint3d> work;
        convexClipper.ConvexPolygonClip(world, profile, work);
        DPoint3d xyz = profile.front();
        profile.push_back(xyz);
        allLoops.push_back(profile);
        }
    ClipPlaneSet setA, setB;
    setA.push_back(convexA0);
    setA.push_back(convexA1);
    setB.push_back(convexB);
    bvector<int> allOpCodes { -4, -3, -2,-1,0,1,2, 3, 4 };
    double yA = -5.5;
    bvector<DSegment3d> segments;
    double x0 = -1.0;
    double x1 = 8.0;
    for (double y : {-2.0, -0.5, 0.5, 1.5, 3.0, 4.0})
        segments.push_back (DSegment3d::From(x0, y, 0, x1, y + 0.2, 0));
    for (auto &loop : allLoops)
        Check::SaveTransformed(loop);
    for (auto &segment : segments)
        Check::SaveTransformed(segment);
    Check::Shift (xShift, 0, 0);
    ClipPlaneSetCompoundClipContext clipContext;
    for (bool initialState : {false, true})
        {
        SaveAndRestoreCheckTransform  rangeShifter1 ((allOpCodes.size () + 2) * xShift,0, 0);
        for (int opcode0 : allOpCodes)
            {
            SaveAndRestoreCheckTransform  rangeShifter2(xShift, 0, 0);
            for (int opcode1 : allOpCodes)
                {
                SaveAndRestoreCheckTransform opcodeShifter(0, yShift,0);

                ClipPlaneSetsWithOpcodes clippers(initialState);
                clippers.AppendClipStep(&setA, opcode0);
                clippers.AppendClipStep(&setB, opcode1);

                for (auto &loop : allLoops)
                    {
                    Check::SaveTransformed (loop);
                    }
                Check::SaveTransformed ({
                        DPoint3d::From(3.0,yA),
                        DPoint3d::From(0.0,yA),
                        DPoint3d::From(1.0, yA + (double) opcode0),
                        DPoint3d::From (2.0, yA + (double) opcode1)});
                for (auto segment : segments)
                    {
                    bvector<DSegment3d> clippedSegments;
                    clipContext.BooleanCombinationOfLineClips(clippers, segment, nullptr, &clippedSegments);
                    for (auto &s : clippedSegments)
                        Check::SaveTransformed(s);
                    }
                }
            }
        }
    Check::ClearGeometry("ClipPlaneSet.ClipSegmentToBoolop");
    }

TEST(ClipPlaneSet, ClipMeshToBoolean)
    {
    double yShift = 20.0;
    double xShift = 20.0;
    bvector<DPoint3d> world{
        DPoint3d::From(-4,-4,0),
        DPoint3d::From(10,-4,0),
        DPoint3d::From(10,10,0),
        DPoint3d::From(-4,10,0),
        DPoint3d::From(-4,-4,0),
        };
    auto convexA0 = ConvexClipPlaneSet::FromXYBox(0, 0.1, 3, 3.8);
    auto convexA1 = ConvexClipPlaneSet::FromXYBox(4, 0, 6, 1);
    auto convexB = ConvexClipPlaneSet::FromXYBox(1, -1, 5, 4);
    bvector<bvector<DPoint3d>> allLoops;
    for (auto &convexClipper : { convexA0, convexA1, convexB })
        {
        bvector<DPoint3d> profile;
        bvector<DPoint3d> work;
        convexClipper.ConvexPolygonClip(world, profile, work);
        DPoint3d xyz = profile.front();
        profile.push_back(xyz);
        allLoops.push_back(profile);
        }
    ClipPlaneSet setA, setB;
    setA.push_back(convexA0);
    setA.push_back(convexA1);
    setB.push_back(convexB);
    bvector<int> allOpCodes{ -4, -3, -2,-1,0,1,2, 3, 4 };
    double yA = -5.5;

    double x0 = -1.0;
    double x1 = 8.0;
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed();
    double c = 0.2;
    for (double y : {-2.0, -0.5, 0.5, 1.5, 3.0, 4.0})
        {
        bvector<DPoint3d> points = 
            {
            DPoint3d::From (x0, y, 0),
            DPoint3d::From (x1, y, 0),
            DPoint3d::From (x1, y + c, 0),
            DPoint3d::From (x0, y + c, 0)
            };
        mesh->AddPolygon (points);
        }

    for (auto &loop : allLoops)
        Check::SaveTransformed(loop);
    Check::SaveTransformed(*mesh);
    Check::Shift(xShift, 0, 0);
    ClipPlaneSetCompoundClipContext clipContext;
    for (bool initialState : {false, true})
        {
        SaveAndRestoreCheckTransform  rangeShifter1((allOpCodes.size() + 2) * xShift, 0, 0);
        for (int opcode0 : allOpCodes)
            {
            SaveAndRestoreCheckTransform  rangeShifter2(xShift, 0, 0);
            for (int opcode1 : allOpCodes)
                {
                SaveAndRestoreCheckTransform opcodeShifter(0, yShift, 0);

                ClipPlaneSetsWithOpcodes clippers(initialState);
                clippers.AppendClipStep(&setA, opcode0);
                clippers.AppendClipStep(&setB, opcode1);

                for (auto &loop : allLoops)
                    {
                    Check::SaveTransformed(loop);
                    }
                Check::SaveTransformed({
                        DPoint3d::From(3.0,yA),
                        DPoint3d::From(0.0,yA),
                        DPoint3d::From(1.0, yA + (double)opcode0),
                        DPoint3d::From(2.0, yA + (double)opcode1) });
                bvector<PolyfaceHeaderPtr> clippedMeshes;
                clipContext.BooleanCombinationOfMeshClips(clippers, mesh, clippedMeshes);
                for (auto &s : clippedMeshes)
                    Check::SaveTransformed(*s);
                }
            }
        }
    Check::ClearGeometry("ClipPlaneSet.ClipMeshToBoolop");
    }
// These points are on the perimiter of an alignmnet.
// BEWARE -- they are badly ordered for polygon definition.
//  There are N upper points (to the left of stations 0..N-1
//        then N lower points (to the right of the stations 0..N-1
// to make a polyong, reverse half of them.
static bvector<DPoint3d> s_upperAlignmentPoints{
    {281654.58498939429, -5273921.0128923692, 0.00000000000000000},
    {281669.32784762711, -5273921.3925883612, 0.00000000000000000},
    {281684.08616223175, -5273921.7236364437, 0.00000000000000000},
    {281698.88170756353, -5273921.9375032736, 0.00000000000000000},
    {281714.12686322944, -5273917.9440042200, 0.00000000000000000},
    {281729.07471956604, -5273916.7384848697, 0.00000000000000000},
    {281743.21183971688, -5273922.7569458690, 0.00000000000000000},
    {281757.97209963796, -5273923.0818711957, 0.00000000000000000},
    {281772.73405566881, -5273923.4014581311, 0.00000000000000000},
    {281787.50808554154, -5273923.6830434445, 0.00000000000000000},
    {281802.27756365773, -5273923.9789551124, 0.00000000000000000},
    {281817.05208453815, -5273924.2589950124, 0.00000000000000000},
    {281831.82978056022, -5273924.5290413657, 0.00000000000000000},
    {281846.61079521384, -5273924.7886425434, 0.00000000000000000},
    {281861.38771798403, -5273925.0611226540, 0.00000000000000000},
    {281876.15327815304, -5273925.3693658020, 0.00000000000000000},
    {281890.92880656780, -5273925.6462345561, 0.00000000000000000},
    {281905.69466766529, -5273925.9535305481, 0.00000000000000000},
    {281920.46713593754, -5273926.2400308987, 0.00000000000000000},
    {281935.23777017556, -5273926.5280322703, 0.00000000000000000},
    {281950.01739501162, -5273926.7724821940, 0.00000000000000000},
    {281964.80063822377, -5273926.9897347940, 0.00000000000000000},
    {281979.59308978077, -5273927.1621960029, 0.00000000000000000},
    {281994.39082913706, -5273927.3022054825, 0.00000000000000000},
    {282009.20048431703, -5273927.3889019638, 0.00000000000000000},
    {282024.01605536952, -5273927.4411698990, 0.00000000000000000},
    {282038.83123916597, -5273927.4788479637, 0.00000000000000000},
    {282053.64031390904, -5273927.5199451437, 0.00000000000000000},
    {282068.43485033035, -5273927.5909919990, 0.00000000000000000},
    {282083.20626048569, -5273927.7190185199, 0.00000000000000000},
    {282097.95065431530, -5273927.9162684102, 0.00000000000000000},
    {282112.66976738663, -5273928.1772790868, 0.00000000000000000},
    {282127.36439327977, -5273928.4995528106, 0.00000000000000000},
    {282141.98435397667, -5273929.0410215762, 0.00000000000000000},
    {282156.60685388267, -5273929.5173288509, 0.00000000000000000},
    {282171.60990192683, -5273927.5284049651, 0.00000000000000000},
    {282186.64961857861, -5273925.2109868648, 0.00000000000000000},
    {282201.71828856017, -5273922.6300638216, 0.00000000000000000},
    {282216.72275496100, -5273920.5703386012, 0.00000000000000000},
    {282232.23891262896, -5273918.5905892625, 0.00000000000000000},
    {282247.68063304469, -5273917.0796970893, 0.00000000000000000},
    {282263.05947038019, -5273916.0193215078, 0.00000000000000000},
    {282277.98382506042, -5273919.7150471993, 0.00000000000000000},
    {282292.67067683663, -5273926.7258769525, 0.00000000000000000},
    {282307.87384566944, -5273928.7770951809, 0.00000000000000000},
    {282323.43159621663, -5273927.6487977533, 0.00000000000000000},
    {282338.77416112355, -5273927.4204400508, 0.00000000000000000},
    {282354.11912754527, -5273927.1836068686, 0.00000000000000000},
    {282369.62852112553, -5273926.0450281873, 0.00000000000000000},
    {282384.96965900797, -5273923.4907245906, 0.00000000000000000},
    {282400.35799683700, -5273922.7018650062, 0.00000000000000000},
    {282415.75605147739, -5273921.5037415605, 0.00000000000000000},
    {282431.11763650633, -5273921.2650754657, 0.00000000000000000},
    {282446.49028567027, -5273920.7733303532, 0.00000000000000000},
    {282461.84946725203, -5273921.3294390719, 0.00000000000000000},
    {282476.80871318991, -5273920.5971492808, 0.00000000000000000},
    {282491.98641860439, -5273916.6914547738, 0.00000000000000000},
    {282507.49694255530, -5273909.2677896703, 0.00000000000000000},
    {282522.54723777808, -5273906.5120995780, 0.00000000000000000},
    {282537.53995630285, -5273904.2681535063, 0.00000000000000000},
    {282552.35978149599, -5273903.7262659995, 0.00000000000000000},
    {282566.97066306381, -5273905.2585532656, 0.00000000000000000},
    {282581.47402350826, -5273907.8190888232, 0.00000000000000000},
    {282596.05250104610, -5273909.5746611115, 0.00000000000000000},
    {282611.08706170530, -5273895.0209031869, 0.00000000000000000},
    {282626.29461213748, -5273874.3839448728, 0.00000000000000000},
    {282641.27895296004, -5273867.6616672017, 0.00000000000000000},
    {282656.23832794069, -5273862.6978041371, 0.00000000000000000},
    {282671.17466121458, -5273859.3743616352, 0.00000000000000000},
    {282686.05808837153, -5273859.5227191709, 0.00000000000000000},
    {282700.96395390539, -5273858.5225156834, 0.00000000000000000},
    {282715.86313132278, -5273858.1598792495, 0.00000000000000000},
    {282730.59266077029, -5273868.4280199697, 0.00000000000000000},
    {282745.43734848336, -5273871.8617248461, 0.00000000000000000},
    {282760.22930918669, -5273878.7562463405, 0.00000000000000000},
    {282774.89519933262, -5273893.6092411485, 0.00000000000000000},
    {282789.86962286977, -5273893.9133451656, 0.00000000000000000},
    {282804.99779976800, -5273891.0204462158, 0.00000000000000000},
    {282820.01926130324, -5273889.2689123601, 0.00000000000000000},
    {282834.97681289783, -5273888.2408098970, 0.00000000000000000},
    {282849.86162469396, -5273888.0461086575, 0.00000000000000000},
    {282864.70192703174, -5273888.3934110869, 0.00000000000000000},
    {282879.52563784819, -5273888.9945443114, 0.00000000000000000},
    {282894.34396299964, -5273889.7338398956, 0.00000000000000000},
    {282909.50346855773, -5273876.0047800466, 0.00000000000000000},
    {282924.52886498754, -5273870.9158536270, 0.00000000000000000},
    {282940.28764056158, -5273821.0812451886, 0.00000000000000000},
    {282955.26751824166, -5273819.2385329688, 0.00000000000000000},
    {282970.30575377971, -5273814.0445643356, 0.00000000000000000},
    {282985.19609168940, -5273818.1475735717, 0.00000000000000000},
    {283000.09357705060, -5273822.0397093771, 0.00000000000000000},
    {283014.99513087375, -5273825.9097850528, 0.00000000000000000},
    {283029.89546751522, -5273830.0819328828, 0.00000000000000000},
    {283044.76108101162, -5273836.6108437814, 0.00000000000000000},
    {283059.63821101491, -5273842.6609570412, 0.00000000000000000},
    {283074.51232594100, -5273849.1233950490, 0.00000000000000000},
    {283089.42318914121, -5273853.5597442081, 0.00000000000000000},
    {283104.31260107970, -5273859.5389828673, 0.00000000000000000},
    {283119.20931351616, -5273865.2979627810, 0.00000000000000000},
    {283134.08618825092, -5273872.5008836417, 0.00000000000000000},
    {283148.97477857186, -5273879.2127986820, 0.00000000000000000},
    {283163.89424188226, -5273884.2589157792, 0.00000000000000000},
    {283178.82529746683, -5273888.8215889595, 0.00000000000000000},
    {283195.32548308710, -5273874.2227569530, 0.00000000000000000},
    {283210.48873392201, -5273873.1356672011, 0.00000000000000000},
    {283225.46238499309, -5273874.0882237051, 0.00000000000000000},
    {283240.30279578082, -5273876.4986759564, 0.00000000000000000},
    {283253.89026105450, -5273889.9232041547, 0.00000000000000000},
    {283269.03550655092, -5273891.3538257768, 0.00000000000000000},
    {283284.27182949765, -5273889.8480778607, 0.00000000000000000},
    {283299.49815520155, -5273888.3631527219, 0.00000000000000000},
    {283314.59105498996, -5273888.1728979172, 0.00000000000000000},
    {283329.60675282625, -5273888.6969617680, 0.00000000000000000},
    {283344.63330164936, -5273889.0264494400, 0.00000000000000000},
    {283359.67285969737, -5273889.1390832821, 0.00000000000000000},
    {283374.60455221147, -5273890.2825495964, 0.00000000000000000},
    {283389.54061442160, -5273891.2983405227, 0.00000000000000000},
    {283404.45207487233, -5273892.4855035096, 0.00000000000000000},
    {283419.36465414451, -5273893.5785470875, 0.00000000000000000},
    {283434.24261573737, -5273894.9463484632, 0.00000000000000000},
    {283449.26443190384, -5273894.7466918835, 0.00000000000000000},
    {283464.29524782917, -5273894.3715677606, 0.00000000000000000},
    {283479.22517786070, -5273894.9552312754, 0.00000000000000000},
    {283493.96108293207, -5273897.4590770053, 0.00000000000000000},
    {283508.82219035109, -5273896.0394246979, 0.00000000000000000},
    {283523.89690395503, -5273889.8002127279, 0.00000000000000000},
    {283539.00147539168, -5273881.5028058141, 0.00000000000000000},
    {283554.03149269184, -5273877.5500742281, 0.00000000000000000},
    {283568.97426767170, -5273878.7200974869, 0.00000000000000000},
    {283583.82158586581, -5273885.5166127253, 0.00000000000000000},
    {283598.01401543734, -5273891.1360840425, 0.00000000000000000},
    {283611.88844890823, -5273895.2685498074, 0.00000000000000000},
    {283625.82563882682, -5273896.4141352847, 0.00000000000000000},
    {283639.61982688331, -5273897.8593602600, 0.00000000000000000},
    {283653.41419141809, -5273898.8002964156, 0.00000000000000000},
    {283667.48598503682, -5273899.4483411051, 0.00000000000000000},
    {283681.49950002995, -5273899.9853152186, 0.00000000000000000},
    {283694.94421794941, -5273901.4051850680, 0.00000000000000000},
    {283707.91434244916, -5273903.3682283163, 0.00000000000000000},
    {283721.38874169462, -5273904.3344418257, 0.00000000000000000},
    {283735.03939427680, -5273906.1302028466, 0.00000000000000000},
    {283750.17843829119, -5273906.4470521640, 0.00000000000000000},
    {283763.24104092643, -5273908.8365252474, 0.00000000000000000},
    {283776.10446549999, -5273911.5133951604, 0.00000000000000000},
    {283790.56897372811, -5273913.2559356969, 0.00000000000000000},
    {283804.70213219547, -5273915.5221070126, 0.00000000000000000},
    {283816.02557677438, -5273919.6851698626, 0.00000000000000000},
    {283825.17625503405, -5273925.0551103242, 0.00000000000000000},
    {283834.19158496626, -5273930.2078013923, 0.00000000000000000},
    {283847.79937513545, -5273931.8471477870, 0.00000000000000000},
    {283858.41670781688, -5273937.1295472262, 0.00000000000000000},
    {283869.23306801263, -5273942.0774423378, 0.00000000000000000},
    {283878.94764314999, -5273948.0957602020, 0.00000000000000000},
    {283889.37706670724, -5273953.4575413074, 0.00000000000000000},
    {283899.99654134351, -5273958.6546949670, 0.00000000000000000},
    {283909.94004685141, -5273964.2091353610, 0.00000000000000000},
    {283923.26525753341, -5273968.9798508035, 0.00000000000000000},
    {283932.20190533355, -5273975.0766908461, 0.00000000000000000},
    {283940.49430319306, -5273981.3772480581, 0.00000000000000000},
    {283948.37226661306, -5273987.8090097923, 0.00000000000000000},
    {283955.88356674626, -5273994.3530242834, 0.00000000000000000},
    {283963.01068060799, -5274001.0053572785, 0.00000000000000000},
    {283970.68653678300, -5274007.2727188673, 0.00000000000000000},
    {283979.36126057676, -5274013.1834676396, 0.00000000000000000},
    {283986.52969458979, -5274019.9601025013, 0.00000000000000000},
    {283993.83310122986, -5274026.5749507919, 0.00000000000000000},
    {283999.17492523557, -5274034.2161148079, 0.00000000000000000},
    {284005.95379092346, -5274041.0314987209, 0.00000000000000000},
    {284011.52171998238, -5274048.5146257123, 0.00000000000000000},
    {284017.67678116611, -5274055.6513418481, 0.00000000000000000},
    {284024.14565979945, -5274062.5958030997, 0.00000000000000000},
    {284030.24475770001, -5274069.7523478335, 0.00000000000000000},
    {284037.28623258788, -5274076.4041016614, 0.00000000000000000},
    {284043.14845013304, -5274083.7056848630, 0.00000000000000000},
    {284049.20233876829, -5274091.0186977843, 0.00000000000000000},
    {284056.04388169188, -5274097.9217915442, 0.00000000000000000},
    {284062.93805333937, -5274104.8051329292, 0.00000000000000000},
    {284069.51778330246, -5274111.7023855941, 0.00000000000000000},
    {284075.43100486591, -5274118.6458372260, 0.00000000000000000},
    {284082.18772499543, -5274125.5000351295, 0.00000000000000000},
    {284088.23153844767, -5274132.4186297767, 0.00000000000000000},
    {284094.34351060470, -5274139.3309736568, 0.00000000000000000},
    {284100.66018494312, -5274146.1525106728, 0.00000000000000000}
    };
static bvector<DPoint3d> s_lowerAlignmentPoints {
    
    {281653.41510980553, -5273927.0351621676, 0.00000000000000000},
    {281668.06767662102, -5273927.6990444325, 0.00000000000000000},
    {281682.81578242057, -5273928.0622240538, 0.00000000000000000},
    {281697.52908268746, -5273928.5349517921, 0.00000000000000000},
    {281711.85622926312, -5273930.2230727645, 0.00000000000000000},
    {281727.24351724680, -5273928.5744686862, 0.00000000000000000},
    {281741.55256457685, -5273930.3195558377, 0.00000000000000000},
    {281756.51538429363, -5273930.0069375876, 0.00000000000000000},
    {281771.38469994464, -5273929.9886172106, 0.00000000000000000},
    {281786.19169533555, -5273930.1664457340, 0.00000000000000000},
    {281801.10290101124, -5273930.0162792718, 0.00000000000000000},
    {281815.95885327307, -5273930.0400192775, 0.00000000000000000},
    {281830.70056035940, -5273930.7140961727, 0.00000000000000000},
    {281845.51568622317, -5273930.5755768260, 0.00000000000000000},
    {281860.29499214725, -5273930.8408825453, 0.00000000000000000},
    {281874.98443163792, -5273931.3883840693, 0.00000000000000000},
    {281889.73810069682, -5273931.7340537030, 0.00000000000000000},
    {281904.34879488719, -5273932.5297272876, 0.00000000000000000},
    {281919.30551979202, -5273932.2362920567, 0.00000000000000000},
    {281933.87217540469, -5273933.1663026754, 0.00000000000000000},
    {281948.69862437551, -5273933.2633767305, 0.00000000000000000},
    {281963.47226683598, -5273933.5108470591, 0.00000000000000000},
    {281978.38677237229, -5273933.2991514327, 0.00000000000000000},
    {281992.89133598364, -5273934.3619122030, 0.00000000000000000},
    {282007.67890296108, -5273934.5181298424, 0.00000000000000000},
    {282022.49268893118, -5273934.5760162063, 0.00000000000000000},
    {282037.48482352286, -5273934.0567533327, 0.00000000000000000},
    {282051.69685826637, -5273935.9769944707, 0.00000000000000000},
    {282066.45590669848, -5273936.1597374277, 0.00000000000000000},
    {282081.15324375930, -5273936.5209041201, 0.00000000000000000},
    {282096.37701773283, -5273935.2093366748, 0.00000000000000000},
    {282111.29449634417, -5273934.9872825220, 0.00000000000000000},
    {282126.00060288829, -5273935.5584849808, 0.00000000000000000},
    {282140.83045237674, -5273935.0677267313, 0.00000000000000000},
    {282155.54772281658, -5273935.5056547336, 0.00000000000000000},
    {282170.31484565034, -5273935.2790539600, 0.00000000000000000},
    {282185.08982285659, -5273935.2851923117, 0.00000000000000000},
    {282199.73481951735, -5273935.7002547923, 0.00000000000000000},
    {282214.44444014074, -5273936.0157212233, 0.00000000000000000},
    {282229.97666404396, -5273935.6112944959, 0.00000000000000000},
    {282245.43293951615, -5273935.3952133115, 0.00000000000000000},
    {282260.83226203104, -5273935.3343609218, 0.00000000000000000},
    {282276.18201833067, -5273936.0825928804, 0.00000000000000000},
    {282291.52729351539, -5273936.8488562629, 0.00000000000000000},
    {282306.89040556527, -5273937.5584423896, 0.00000000000000000},
    {282322.29422722734, -5273937.8972340105, 0.00000000000000000},
    {282337.83854458621, -5273936.2960320497, 0.00000000000000000},
    {282353.40083097760, -5273934.1818653801, 0.00000000000000000},
    {282368.66378130077, -5273934.1823117062, 0.00000000000000000},
    {282383.76503596362, -5273935.8325490998, 0.00000000000000000},
    {282398.81165505724, -5273938.1487421170, 0.00000000000000000},
    {282414.29402008117, -5273937.8107277099, 0.00000000000000000},
    {282429.97913391236, -5273935.8387113055, 0.00000000000000000},
    {282445.51219491358, -5273934.8615706870, 0.00000000000000000},
    {282461.13176995039, -5273932.8807295840, 0.00000000000000000},
    {282476.16620515590, -5273930.3555716975, 0.00000000000000000},
    {282491.40508587740, -5273927.0382515015, 0.00000000000000000},
    {282506.26521039801, -5273928.8255393952, 0.00000000000000000},
    {282521.13472298079, -5273929.8096910575, 0.00000000000000000},
    {282535.97594575316, -5273932.3012320353, 0.00000000000000000},
    {282550.80391448631, -5273935.3781215735, 0.00000000000000000},
    {282565.62169581244, -5273938.8523041848, 0.00000000000000000},
    {282580.48540053912, -5273939.2855629921, 0.00000000000000000},
    {282595.41625054693, -5273935.5197492242, 0.00000000000000000},
    {282610.43181663426, -5273926.5588793559, 0.00000000000000000},
    {282625.44955802499, -5273913.7496055141, 0.00000000000000000},
    {282640.35738710780, -5273912.2231090367, 0.00000000000000000},
    {282655.25205178116, -5273910.9150682259, 0.00000000000000000},
    {282670.20061154780, -5273909.1332854312, 0.00000000000000000},
    {282685.12192235829, -5273907.7153409738, 0.00000000000000000},
    {282700.03449429129, -5273906.4701712113, 0.00000000000000000},
    {282714.72337427165, -5273919.4096519612, 0.00000000000000000},
    {282729.66421029530, -5273916.7198096942, 0.00000000000000000},
    {282744.61176885339, -5273913.8451512242, 0.00000000000000000},
    {282759.60623405222, -5273908.3214609530, 0.00000000000000000},
    {282774.11680017563, -5273932.6994127147, 0.00000000000000000},
    {282789.03348524647, -5273932.4003056958, 0.00000000000000000},
    {282803.95501486503, -5273932.0309412675, 0.00000000000000000},
    {282818.87862282468, -5273931.6438443810, 0.00000000000000000},
    {282833.74566578760, -5273934.9010654977, 0.00000000000000000},
    {282848.61690810206, -5273938.1281999219, 0.00000000000000000},
    {282863.49234976817, -5273941.3252476584, 0.00000000000000000},
    {282878.37199078628, -5273944.4922087062, 0.00000000000000000},
    {282893.51462032890, -5273931.7593180696, 0.00000000000000000},
    {282909.00559199078, -5273897.8924252465, 0.00000000000000000},
    {282923.81738743209, -5273896.2519345507, 0.00000000000000000},
    {282938.56399339030, -5273897.9792380631, 0.00000000000000000},
    {282953.21794581087, -5273900.7454952234, 0.00000000000000000},
    {282968.00020662486, -5273902.2699058335, 0.00000000000000000},
    {282982.74178073893, -5273904.2968610683, 0.00000000000000000},
    {282997.18694121391, -5273909.4660104197, 0.00000000000000000},
    {283011.76731114078, -5273913.3220795300, 0.00000000000000000},
    {283026.28270285239, -5273917.9314337038, 0.00000000000000000},
    {283040.59424028324, -5273924.7275712611, 0.00000000000000000},
    {283055.05755170295, -5273930.0396470875, 0.00000000000000000},
    {283069.78868283075, -5273932.6698209178, 0.00000000000000000},
    {283085.20834925096, -5273928.2754071988, 0.00000000000000000},
    {283100.68930345261, -5273923.3309431346, 0.00000000000000000},
    {283116.17453709617, -5273918.4248768399, 0.00000000000000000},
    {283131.70984902198, -5273913.0844662283, 0.00000000000000000},
    {283146.98834292195, -5273910.4775382383, 0.00000000000000000},
    {283162.18271046062, -5273908.8215451771, 0.00000000000000000},
    {283177.46130357462, -5273906.3787346296, 0.00000000000000000},
    {283192.71006907616, -5273904.3263796717, 0.00000000000000000},
    {283207.85816853773, -5273903.3956845319, 0.00000000000000000},
    {283223.50881091610, -5273897.3602413591, 0.00000000000000000},
    {283238.04319482495, -5273902.9295461830, 0.00000000000000000},
    {283253.47933836794, -5273899.2733317539, 0.00000000000000000},
    {283268.54559464962, -5273904.1148895454, 0.00000000000000000},
    {283283.50192528847, -5273908.5068530450, 0.00000000000000000},
    {283298.46044366655, -5273912.5377792697, 0.00000000000000000},
    {283313.45888802106, -5273913.8928884845, 0.00000000000000000},
    {283328.47256403172, -5273914.0865139533, 0.00000000000000000},
    {283343.45665272081, -5273915.8670961568, 0.00000000000000000},
    {283358.51493840257, -5273912.8702549767, 0.00000000000000000},
    {283373.56165376212, -5273910.3555115601, 0.00000000000000000},
    {283388.58403529460, -5273909.1055641137, 0.00000000000000000},
    {283403.59252696915, -5273908.4799556360, 0.00000000000000000},
    {283418.59441332717, -5273908.0319751576, 0.00000000000000000},
    {283433.56948314980, -5273909.0010381462, 0.00000000000000000},
    {283448.53721431864, -5273910.1926996829, 0.00000000000000000},
    {283463.50375301589, -5273911.2300565783, 0.00000000000000000},
    {283478.50136068545, -5273910.1347357547, 0.00000000000000000},
    {283493.55910500977, -5273905.1242156290, 0.00000000000000000},
    {283508.31055472721, -5273906.9761214266, 0.00000000000000000},
    {283522.62741817289, -5273913.6402323246, 0.00000000000000000},
    {283537.26597731444, -5273916.9011841463, 0.00000000000000000},
    {283552.30995569064, -5273915.8014010284, 0.00000000000000000},
    {283567.40066696552, -5273914.1331470460, 0.00000000000000000},
    {283582.45189841941, -5273912.7898513917, 0.00000000000000000},
    {283595.88693827798, -5273909.8754231911, 0.00000000000000000},
    {283609.93155345984, -5273906.4568416839, 0.00000000000000000},
    {283623.97727378248, -5273904.7505911672, 0.00000000000000000},
    {283637.84833287180, -5273904.2512194486, 0.00000000000000000},
    {283651.56175707350, -5273904.4045646926, 0.00000000000000000},
    {283664.77587696339, -5273906.2110804748, 0.00000000000000000},
    {283677.71234795405, -5273908.5488837082, 0.00000000000000000},
    {283691.04318013333, -5273909.2217916856, 0.00000000000000000},
    {283704.56408771209, -5273909.3483533897, 0.00000000000000000},
    {283717.63671090954, -5273910.2018936146, 0.00000000000000000},
    {283730.50653375860, -5273912.4915736550, 0.00000000000000000},
    {283741.90484260244, -5273917.2794460198, 0.00000000000000000},
    {283754.53822767595, -5273919.2710820958, 0.00000000000000000},
    {283767.29499001522, -5273921.0601702463, 0.00000000000000000},
    {283778.43274148484, -5273925.5748101436, 0.00000000000000000},
    {283789.75195217610, -5273929.5600239234, 0.00000000000000000},
    {283801.98028715362, -5273932.0944730220, 0.00000000000000000},
    {283815.29229307547, -5273933.2258921061, 0.00000000000000000},
    {283826.80591816618, -5273935.5854570940, 0.00000000000000000},
    {283836.66103527125, -5273940.2249121899, 0.00000000000000000},
    {283849.01466817711, -5273943.8464467917, 0.00000000000000000},
    {283860.27629377355, -5273948.1288980404, 0.00000000000000000},
    {283871.75407110190, -5273952.7943980051, 0.00000000000000000},
    {283880.95470532926, -5273958.8188937632, 0.00000000000000000},
    {283890.78128599620, -5273964.1332945768, 0.00000000000000000},
    {283899.80819044705, -5273970.1160498625, 0.00000000000000000},
    {283909.14271269168, -5273975.7396197906, 0.00000000000000000},
    {283918.16867434268, -5273981.5209846599, 0.00000000000000000},
    {283927.52678134944, -5273986.9516991805, 0.00000000000000000},
    {283936.85934493574, -5273992.3504909175, 0.00000000000000000},
    {283944.58906496852, -5273998.8431652049, 0.00000000000000000},
    {283952.81137341633, -5274004.8857496027, 0.00000000000000000},
    {283959.78341671382, -5274011.6658842629, 0.00000000000000000},
    {283966.72179200908, -5274018.3459017770, 0.00000000000000000},
    {283973.82904160360, -5274024.8239153652, 0.00000000000000000},
    {283980.48864465184, -5274031.4792592954, 0.00000000000000000},
    {283987.27297412930, -5274038.4286986347, 0.00000000000000000},
    {283994.01603415492, -5274045.3861225210, 0.00000000000000000},
    {284000.72223724192, -5274052.3490421968, 0.00000000000000000},
    {284007.21540484694, -5274059.3641335536, 0.00000000000000000},
    {284013.61436150933, -5274066.4088831199, 0.00000000000000000},
    {284019.96477281733, -5274073.4657133985, 0.00000000000000000},
    {284026.06321470859, -5274080.6493604388, 0.00000000000000000},
    {284032.31571736617, -5274087.7308134846, 0.00000000000000000},
    {284038.51620046445, -5274094.8263067771, 0.00000000000000000},
    {284044.74393265299, -5274101.8911309019, 0.00000000000000000},
    {284050.96076759335, -5274108.9468013039, 0.00000000000000000},
    {284057.12147496850, -5274116.0188289685, 0.00000000000000000},
    {284064.13941273699, -5274122.5920581147, 0.00000000000000000},
    {284071.06610995973, -5274129.2045535343, 0.00000000000000000},
    {284078.36031792103, -5274135.6087562284, 0.00000000000000000},
    {284084.88566596422, -5274142.2119123377, 0.00000000000000000},
    {284091.69127289939, -5274149.0606436711, 0.00000000000000000}
    };
bvector<DPoint3d> AlignmentPolygon ()
    {
    bvector<DPoint3d> points = s_lowerAlignmentPoints;
    std::copy (s_upperAlignmentPoints.rbegin (), s_upperAlignmentPoints.rend (), std::back_inserter (points));
    return points;
    }
TEST(ClipPlaneSet, AlignmentOrdering)
    {

    ConvexClipPlaneSet convexSetA, convexSetB, convexSet0;


    Check::SaveTransformed(s_upperAlignmentPoints);
    Check::SaveTransformed(s_lowerAlignmentPoints);

    auto correctedAlignmentPolygon = AlignmentPolygon ();
    DRange3d range = DRange3d::From(correctedAlignmentPolygon);
    double yStep = 1.5 * (range.high.y - range.low.y);

    Check::Shift (0, yStep, 0);
    Check::SaveTransformed(correctedAlignmentPolygon);
    AlternatingConvexClipTreeNode rootNode;
    AlternatingConvexClipTreeNode::CreateTreeForPolygon(correctedAlignmentPolygon, rootNode);
#define debugTriangulation
#ifdef debugTriangulation
    bvector<DTriangle3d> triangles;
    bvector<bvector<DPoint3d>> loops;
    loops.push_back (correctedAlignmentPolygon);
    PolygonOps::FixupAndTriangulateSpaceLoops (loops, triangles);
    Check::Shift(0, yStep, 0);
    Check::SaveTransformed (triangles);
#endif
    DVec3d zVector = DVec3d::From (0,0,1);
    ClipPlaneSet convexSets = ClipPlaneSet::FromSweptPolygon (&correctedAlignmentPolygon[0], correctedAlignmentPolygon.size (), &zVector);

    bvector<DSegment3d> segmentsA, segmentsB;
    segmentsA.reserve (1000);
    segmentsB.reserve(1000);
    size_t numScan = 100;
    bvector<CurveLocationDetailPair> inside;
    for (size_t i = 0; i < numScan; i++)
        {
        auto f = (double)i / (double) numScan;
        DPoint3d pointA = range.LocalToGlobal (f,0,0);
        DPoint3d pointB = range.LocalToGlobal (f,1,0);
        auto cp = ICurvePrimitive::CreateLine (pointA, pointB);
        inside.clear ();
        rootNode.AppendCurvePrimitiveClipIntervals (*cp, &inside, nullptr);
        for (auto &pair : inside)
            {
            segmentsA.push_back (DSegment3d::From(pair.detailA.point, pair.detailB.point));
            }
        }
    bvector<DSegment1d> inside1d;
    for (size_t i = 0; i < numScan; i++)
        {
        auto f = (double)i / (double)numScan;
        DPoint3d pointA = range.LocalToGlobal(f-0.0232131, 0, 0);
        DPoint3d pointB = range.LocalToGlobal(f, 1, 0);
        auto segment = DSegment3d::From(pointA, pointB);
        inside1d.clear();
        convexSets.AppendIntervals(segment, inside1d);
        for (auto &interval : inside1d)
            {
            segmentsB.push_back(DSegment3d::FromFractionInterval (segment, interval));
            }
        }

    Check::Shift(0, yStep, 0);
    Check::SaveTransformed(correctedAlignmentPolygon);
    Check::SaveTransformed (segmentsA);
    Check::Shift(0, yStep, 0);
    Check::SaveTransformed(correctedAlignmentPolygon);
    Check::SaveTransformed(segmentsB);
#define debugClipPlaneSet
#ifdef debugClipPlaneSet
    range.low.z -= 10.0;
    range.high.z += 10.0;
    bvector<DPoint3d> bigFacet{
        DPoint3d::From(range.low.x, range.low.y, 0.0),
        DPoint3d::From(range.high.x, range.low.y, 0.0),
        DPoint3d::From(range.high.x, range.high.y, 0.0),
        DPoint3d::From(range.low.x, range.high.y, 0.0),
        DPoint3d::From(range.low.x, range.low.y, 0.0),
        };

#ifdef debugClipPlaneSetIntersectPolyface
    auto bigBoxMesh = PolyfaceHeader::CreateDRange3dFaces(range, true);
    Check::Shift(0, yStep, 0);
    auto convexInside = PolyfaceHeader::CreateVariableSizeIndexed();
    ClipPlaneSet::ClipPlaneSetIntersectPolyface(*bigBoxMesh, convexSets, true, &convexInside, nullptr);
    Check::SaveTransformed(*convexInside);

    auto bigFacetMesh = PolyfaceHeader::CreateVariableSizeIndexed();
    bigFacetMesh->AddPolygon (bigFacet);
    auto bigFacetClip = PolyfaceHeader::CreateVariableSizeIndexed();

    Check::Shift(0, yStep, 0);
    ClipPlaneSet::SweptPolygonClipPolyface (*bigFacetMesh, correctedAlignmentPolygon, zVector, false, &bigFacetClip, nullptr);
    Check::SaveTransformed(*bigFacetClip);
#endif
    Check::Shift(0, yStep, 0);
    bvector <DPoint3d> work, singleClip;
    for (auto &convex : convexSets)
        {
        convex.ConvexPolygonClip (bigFacet, singleClip, work);
        DPoint3d xyz = singleClip.front ();
        singleClip.push_back (xyz);
        Check::SaveTransformed(singleClip);
        }
#endif
    Check::ClearGeometry ("ClipPlaneSet.AlignmentOrdering");
    }

TEST(ClipPlaneSet, AlignmentFiltering)
    {
    auto ccwPoints = AlignmentPolygon ();
    bvector<DPoint3d> upper = s_upperAlignmentPoints;
    bvector<DPoint3d> lower = s_lowerAlignmentPoints;

    auto range = DRange3d::From(ccwPoints);
    auto diagonal = range.low.Distance (range.high);
    double yStep = 2.0 * (range.high.y - range.low.y);
    double yStep1 = 0.3 * yStep;
    double xStep = 1.2 * (range.high.x - range.low.x);
    Check::SaveTransformed (ccwPoints);

    auto builder0 = CreateBuilder(false, false);
    auto curveA = CurveVector::CreateLinear(lower);
    auto curveB = CurveVector::CreateLinear(upper);
    builder0->AddRuledBetweenCorrespondingCurves({ curveA, curveB }, false);
    Check::Shift(0, yStep1, 0);
    auto quadMesh = builder0->GetClientMeshPtr();

    Check::Shift(0, 3 * yStep);
    Check::SaveTransformed(*quadMesh);

    for (double fraction : {1.0e-5, 1.0e-4, 1.0e-3, 2.5e-3, 5.0e-3, 1.0e-2})
        {
        SaveAndRestoreCheckTransform shifter (0, yStep);
//        auto points = ccwPoints;
//        PolylineOps::CompressColinearPoints (points, fraction * diagonal, true, false, true);
        bvector<DPoint3d> points;
        DPoint3dOps::CompressByChordError (points, ccwPoints, fraction * diagonal);
        Check::SaveTransformed(points);
        Check::Shift(xStep, 0, 0);
        bvector<DPoint3d> compressedLower, compressedUpper;
        DPoint3dOps::CompressByChordError(compressedUpper, upper, fraction * diagonal);
        DPoint3dOps::CompressByChordError(compressedLower, lower, fraction * diagonal);
        Check::SaveTransformed(compressedUpper);
        Check::SaveTransformed(compressedLower);
        auto loop = compressedLower;
        std::copy (compressedUpper.rbegin (), compressedUpper.rend (), std::back_inserter (loop));

        Check::Shift(xStep, 0, 0);
        bvector<DTriangle3d> triangles;
        bvector<bvector<DPoint3d>> loops;
        loops.push_back(loop);
        PolygonOps::FixupAndTriangulateSpaceLoops(loops, triangles);
        Check::SaveTransformed(triangles);

        Check::Shift(xStep, 0, 0);
        triangles.clear ();
        PolylineOps::GreedyTriangulationBetweenLinestrings (compressedLower, compressedUpper, triangles);
        Check::SaveTransformed(triangles);
        Check::Shift(0,yStep1,0);
        Check::SaveTransformed (compressedLower);
        Check::SaveTransformed (compressedUpper);

        auto compressedLowerB = lower;
        auto compressedUpperB = upper;
        auto builder = CreateBuilder(false, false);
        builder->GetFacetOptionsR ().SetAngleTolerance (3.0);
        PolylineOps::CompressPairedColinearPoints(compressedLowerB, compressedUpperB,
                fraction * diagonal, false, false, true);
        if (Check::Size (compressedLowerB.size(), compressedUpperB.size ()))
            {
            auto curveA1 = CurveVector::CreateLinear(compressedLowerB);
            auto curveB1 = CurveVector::CreateLinear(compressedUpperB);
            builder->AddRuledBetweenCorrespondingCurves({ curveA1, curveB1 }, false);
            auto quadMesh1 = builder->GetClientMeshPtr();
            Check::Shift(0, yStep1, 0);
            Check::SaveTransformed(*quadMesh1);
            }
        }
    Check::ClearGeometry("ClipPlaneSet.AlignmentFiltering");
    }

static double ClipGridInRange
(
DRange3dCR range,
ClipPlaneSetP clipper1,
ClipPlaneSetWithIndexedRangeHeap * clipper2,
size_t numScanX,
size_t numScanY,
bool doSave = false,
double fractionTilt = 0.0
)
    {
    bvector<DSegment3d> segmentsA;
    segmentsA.reserve (2 * (numScanX + numScanY));
    bvector<DSegment1d> inside1d;
    inside1d.reserve (100);
    TimeAccumulator timer;
    timer.Reset();
    for (size_t i = 0; i < numScanX; i++)
        {
        auto f = (double)i / (double)numScanX;
        DPoint3d pointA = range.LocalToGlobal(f - fractionTilt, 0, 0);
        DPoint3d pointB = range.LocalToGlobal(f, 1, 0);
        auto segment = DSegment3d::From(pointA, pointB);
        inside1d.clear();
        if (clipper1)
            clipper1->AppendIntervals(segment, inside1d);
        if (clipper2)
            clipper2->AppendDSegment3dIntervals(segment, inside1d);
        for (auto &interval : inside1d)
            {
            segmentsA.push_back(DSegment3d::FromFractionInterval(segment, interval));
            }
        }

    for (size_t i = 0; i < numScanY; i++)
        {
        auto f = (double)i / (double)numScanY;
        DPoint3d pointA = range.LocalToGlobal(0, f - fractionTilt, 0);
        DPoint3d pointB = range.LocalToGlobal(1.0, f, 0);
        auto segment = DSegment3d::From(pointA, pointB);
        inside1d.clear();
        if (clipper1)
            clipper1->AppendIntervals(segment, inside1d);
        if (clipper2)
            clipper2->AppendDSegment3dIntervals(segment, inside1d);
        for (auto &interval : inside1d)
            {
            segmentsA.push_back(DSegment3d::FromFractionInterval(segment, interval));
            }
        }
    timer.AccumulateAndReset();
    double t = timer.Sum();
    if (doSave)
        Check::SaveTransformed(segmentsA);
    return t;
    }

TEST(ClipPlaneSet, AlignmentByMatchedArrays)
    {
    bvector<DPoint3d> upper = s_upperAlignmentPoints;
    bvector<DPoint3d> lower = s_lowerAlignmentPoints;

    double fractionTilt = 0.0123287987932423;
    Check::SaveTransformed (s_upperAlignmentPoints);
    Check::SaveTransformed (s_lowerAlignmentPoints);
    auto range = DRange3d::From (s_upperAlignmentPoints);
    range.Extend (s_lowerAlignmentPoints);
    double yStep = 2.0 * range.YLength();
    double diagonal = range.low.Distance (range.high);

    ClipPlaneSet clipperA;
    clipperA.AddMatchedXYPointVectors(upper, lower);
    ClipGridInRange (range, &clipperA, nullptr, 100, 20, true, fractionTilt);

    Check::Shift(0, yStep, 0);
    for (double compressionFraction : {1.0e-5, 3.0e-5, 9.0e-5, 27.0e-5, 80.0e-5})
        {
        SaveAndRestoreCheckTransform shifter(0, yStep, 0);
        double tol = compressionFraction * diagonal;
        auto compressedLower = lower;
        auto compressedUpper = upper;
        PolylineOps::CompressPairedColinearPoints(compressedLower, compressedUpper,
            tol, false, false, true);
        Check::SaveTransformed(compressedUpper);
        Check::SaveTransformed(compressedLower);

        ClipPlaneSet clipperB;
        // Remark: pass lower,upper in reverse order here to exercise quad logic in both orientations
        clipperB.AddMatchedXYPointVectors(compressedLower, compressedUpper);
        ClipGridInRange(range, &clipperB, nullptr, 100, 20, true, fractionTilt);
        }

    ClipPlaneSetWithIndexedRangeHeap indexedClipSet;
    indexedClipSet.InitFromMatchedXYPointVectors (upper, lower);
    Check::Shift(0, yStep, 0);
    ClipGridInRange(range, nullptr, &indexedClipSet, 100, 20, true, fractionTilt);
#ifdef doTimingTests
    indexedClipSet.m_work.reserve (100);
    for (size_t numY : {200, 200, 2000, 2000, 20000, 20000})
        {
        size_t numX = numY * 5;
        double tIndexed = ClipGridInRange(range, nullptr, &indexedClipSet, numX, numY, false, fractionTilt);
        double tFull  = ClipGridInRange(range, &indexedClipSet, nullptr, numX, numY, false, fractionTilt);
        double t0 = ClipGridInRange(range, nullptr, nullptr, numX, numY, false, fractionTilt);
        printf (" (numY %4d)  (fullScanTime %10.2lf)  (IndexedScanTime %10.2lf) (t0 %10.2lf)\n",
                    (int)numY, tFull, tIndexed, t0);
        }
// timing 11/20/20 
// numY = 200 => (tFull=73,tIndexed=43) (69,18)        ?? 1st pass difference in indexedClipSet repeatable over multiple runs
// numY = 2000 => (706,173) (703,179)
// numY = 20000 => (7014,1776) (7003,1751)
// m_hitData (n=266520, hits=1448789, max=84)
// with t0 about numY/400, i.e. 1% of tFull and 3% of tIndexed
// average range hits per call is 5 ... compare to 183 without binary.
// so the 5X performance difference seems small compared to 30X reduction in leaf calls.
//   Tree depth is 8.  Apparently the range check going down is significant part of time.
//   Clipping a mesh will have larger time 'at leaf' so probably better than 5X.
//   That's an experiment for another day ...
#endif
    Check::ClearGeometry("ClipPlaneSet.AlignmentByMatchedArrays");
    }
