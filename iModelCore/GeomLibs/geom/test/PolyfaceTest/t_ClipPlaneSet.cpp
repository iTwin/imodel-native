/*--------------------------------------------------------------------------------------+
|
|  $Source: geom/test/PolyfaceTest/t_ClipPlaneSet.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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

    auto cOut =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*outsideLine, outerClip, innerClip);
    auto cIn =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*insideLine, outerClip, innerClip);
    auto cCrossing =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*crossingLine, outerClip, innerClip);
    auto cHole =  ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*holeLine, outerClip, innerClip);

    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_StronglyOutside, (int)cOut, "Expect OUT");
    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_StronglyInside, (int)cIn, "Expect IN");
    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_Ambiguous, (int)cCrossing, "Expect CROSSING");
    Check::Int ((int)ClipPlaneContainment::ClipPlaneContainment_StronglyOutside, (int)cHole, "Expect OUT(hole)");
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
                classification = ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*testCurve, outerClip, innerClip);
            else if (testCV.IsValid ())
                classification = ClipPlaneSet::ClassifyCurveVectorInSetDifference (*testCV, outerClip, innerClip, true);    // BUT as of now we know that it ignores the region condition.
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