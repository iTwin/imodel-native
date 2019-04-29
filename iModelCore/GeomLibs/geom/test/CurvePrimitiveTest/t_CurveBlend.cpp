/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include "testHarness.h"
#include <GeomSerialization/GeomSerializationApi.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

static int s_noisy = 0;

#if defined(BENTLEY_WIN32)
static bool s_readFile = 0;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve, FilletSegmentSegment)
    {
    ICurvePrimitivePtr line1 = ICurvePrimitive::CreateLine (
            DSegment3d::From (0,0,0, 10,0,0));
    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine (
            DSegment3d::From (5,0,0, 5, 10,0));
    Check::StartScope ("FilletBetweenLines");            
    bvector<CurveCurve::FilletDetail> arc0, arc1;
    double radius = 3;
    CurveCurve::CollectFilletArcs (*line1, *line2, radius, true, arc0);
    CurveCurve::CollectFilletArcs (*line1, *line2, radius, false, arc1);
    
    Check::Size (4, arc0.size (), "ExtendedLines");
    Check::Size (2, arc1.size (), "BoundedLines");

    Check::EndScope ();
               
        
    Check::StartScope ("ParabolicBlendBetweenLines");            

    bvector<BlendDetail> parabola0, parabola1;
    CurveCurve::CollectBlends (*line1, *line2, CURVE_CURVE_BLEND_BisectorParabola, radius, radius, true, parabola0);
    CurveCurve::CollectBlends (*line1, *line2, CURVE_CURVE_BLEND_BisectorParabola, radius, radius, false, parabola1);
    // Peter  --- This should produce 4 extended blends and 2 for nonextended .......
    Check::Size (4, parabola0.size (), "Extended");
    Check::Size (2, parabola1.size (), "Bounded");    
    
    Check::EndScope ();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve, FilletLinestring)
    {
    bvector<DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (5,0,0));
    points.push_back (DPoint3d::From (0,5,0));
    points.push_back (DPoint3d::From (10,5,0));
    double radius = 0.5;
    ICurvePrimitivePtr linestring = ICurvePrimitive::CreateLineString (points);
    Check::StartScope ("FilletWithinLinestring");
    bvector<CurveCurve::FilletDetail> arcs;

    CurveCurve::CollectFilletArcs (*linestring, *linestring, radius, false, arcs);
    // We expect a single fillet at each interior vertex ....    
    Check::Size (points.size () - 2, arcs.size (), "Internal fillets");
    
    Check::EndScope ();
    
    
    Check::StartScope ("ParabolicBlendWithinLinestring");

    bvector<BlendDetail> parabolas;
    CurveCurve::CollectBlends (*linestring, *linestring, CURVE_CURVE_BLEND_BisectorParabola, radius, radius, false, parabolas);
    Check::Size (points.size () - 2, parabolas.size (), "Internal blends");
    
    Check::EndScope ();
    
    
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve, FilletLineArc)
    {
    double rA = 10.0;
    DEllipse3d ellipseA = DEllipse3d::From (
                    5,2,0,
                    rA,0,0,
                    0,rA,0,
                    0.0, 0.75 * Angle::Pi ()
                    );
    ICurvePrimitivePtr arcA = ICurvePrimitive::CreateArc (ellipseA);
    DPoint3d pointA0, pointA1;
    arcA->GetStartEnd (pointA0, pointA1);
    ICurvePrimitivePtr lineB = ICurvePrimitive::CreateLine (DSegment3d::From (pointA0, pointA1));
    Check::StartScope ("FilletLineArc");            
    bvector<CurveCurve::FilletDetail> arc0, arc1;
    double rF = 0.5;

    CurveCurve::CollectFilletArcs (*arcA, *lineB, rF, true, arc0);
    CurveCurve::CollectFilletArcs (*arcA, *lineB, rF, false, arc1);

    for (size_t i = 0; i < arc0.size (); i++)
        {
        DPoint3d center = arc0[i].arc.center;
        double dC = center.Distance (ellipseA.center);
        Check::Near (rF, fabs (dC - rA), "confirm tangency");
        }
    Check::Size (8, arc0.size (), "Extended");
    Check::Size (2, arc1.size (), "Bounded");
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve, FilletLineStringArc)
    {
    double rA = 10.0;
    DEllipse3d ellipseA = DEllipse3d::From (
                    5,2,0,
                    rA,0,0,
                    0,rA,0,
                    0.0, 0.75 * Angle::Pi ()
                    );
    ICurvePrimitivePtr arcA = ICurvePrimitive::CreateArc (ellipseA);
    DPoint3d pointA0, pointA1;
    arcA->GetStartEnd (pointA0, pointA1);

    bvector<DPoint3d> points;
    points.push_back (pointA0);
    points.push_back (ellipseA.center);
    points.push_back (pointA1);



    ICurvePrimitivePtr lineB = ICurvePrimitive::CreateLineString (points);
    Check::StartScope ("FilletLineArc");            
    bvector<CurveCurve::FilletDetail> arc0, arc1;
    double rF = 0.5;

    CurveCurve::CollectFilletArcs (*lineB, *arcA, rF, true, arc0);
    CurveCurve::CollectFilletArcs (*lineB, *arcA, rF, false, arc1);

    for (size_t i = 0; i < arc0.size (); i++)
        {
        DPoint3d center = arc0[i].arc.center;
        double dC = center.Distance (ellipseA.center);
        Check::Near (rF, fabs (dC - rA), "confirm tangency");
        }
    Check::Size (8, arc0.size (), "Extended");
    Check::Size (2, arc1.size (), "Bounded");
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve, FilletArcArc)
    {
    double rA = 12;
    double rB = 7;
    double rF = 0.5;
    Check::StartScope ("FilletArcArc");
    DEllipse3d ellipseA = DEllipse3d::From (1,1,0,  rA, 0, 0,  0,rA,0,    -0.25 * Angle::Pi (), Angle::Pi ());
    DEllipse3d ellipseB = DEllipse3d::From (rA,-rB,0,   rB, 0, 0,   0,rB, 0,    0.0, Angle::Pi ());
    ICurvePrimitivePtr prim1 = ICurvePrimitive::CreateArc (ellipseA);
    ICurvePrimitivePtr prim2 = ICurvePrimitive::CreateArc (ellipseB);
    bvector<CurveCurve::FilletDetail> arc0, arc1;    
    CurveCurve::CollectFilletArcs (*prim1, *prim2, rF, true, arc0);
    CurveCurve::CollectFilletArcs (*prim1, *prim2, rF, false, arc1);
    
    Check::Size (8, arc0.size (), "Extended");
    Check::Size (4, arc1.size (), "Bounded");
    
    
    Check::EndScope ();
    }

void AddPoint (bvector<DPoint3d> &points, double x, double y, double z = 0.0)
    {
    points.push_back (DPoint3d::From (x, y, z));
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Curve,CloneWithFilletsA)
    {
    bvector<DPoint3d> points;
    AddPoint (points, 1,1);
    AddPoint (points, 4,1);
    AddPoint (points, 4,3);
    AddPoint (points, 8,3);
    double radius = 0.25;
    CurveVectorPtr pathA = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr pathB = pathA->CloneWithFillets (radius);
    Check::Size (3, pathB->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line), "Post-Fillet lines");
    Check::Size (2, pathB->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc), "Post-Fillet arcs");
    
    CurveVectorPtr pathC = pathA->CloneWithBlends (CURVE_CURVE_BLEND_BisectorParabola, radius, radius);
    Check::Size (3, pathC->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line), "Post-Blend lines");
    Check::Size (2, pathC->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve), "Post-Blend parabolas");
    // Path D has strictly alternating arcs and lines -- consolidate should have no effect ...
    CurveVectorPtr pathD = pathB->Clone ();
    size_t sizeD0 = pathB->size ();
    pathD->ConsolidateAdjacentPrimitives ();
    size_t sizeD1 = pathD->size ();
    Check::Size (sizeD0, sizeD1, "ConsolidateAdjacentPrimitives should have no effect here");
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Curve,CloneWithFilletsB)
    {
    bvector<DPoint3d> points;
    double ax = 4.0;
    double ay = 3.0;
    double radius = 1.0;
    double x0 = 1.0;
    double x1 = x0 + ax;
    double y0 = 2.0;
    double y1 = y0 + ay;
    AddPoint (points, x0, y0);
    AddPoint (points, x1, y0);
    AddPoint (points, x1, y1);
    AddPoint (points, x0, y1);
    AddPoint (points, x0, y0);
    CurveVectorPtr pathA = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
    CurveVectorPtr pathB = pathA->CloneWithFillets (radius);
    Check::Size (4, pathB->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line), "Post-Fillet lines");
    Check::Size (4, pathB->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc), "Post-Fillet arcs");
    double expectedLengthA = 2.0 * (ax + ay);
    double lengthA = pathA->Length ();
    double expectedLengthB = expectedLengthA - 8.0 * radius + 4.0 * Angle::PiOver2 () * radius;
    double lengthB = pathB->Length ();
    Check::Near (expectedLengthA, lengthA, "Rectangle Perimeter");
    Check::Near (expectedLengthB, lengthB, "Filleted Rectangle Perimeter");
    
    // Increase fillet size so no fillets ....
    double bigRadius = 10.0;
    CurveVectorPtr pathC = pathA->CloneWithFillets (bigRadius);
    Check::Size (4, pathC->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line), "Post-Fillet lines");
    Check::Size (0, pathC->CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc), "Post-Fillet arcs");    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef TestRemainderArithmetic
TEST(RemainderArithmetic,Test)
    {
    size_t n_as_size_t = 10;
    ptrdiff_t n_as_ptrdiff_t = 10;
    for (ptrdiff_t i = -20; i < 20; i+=3)
        {
        size_t m1 = i % n_as_size_t;
        ptrdiff_t m2 = i % n_as_size_t;
        ptrdiff_t m3 = i % n_as_ptrdiff_t;
        size_t m4 = i % n_as_ptrdiff_t;
        size_t m5;
        if (i < 0)
            {
            size_t j = -i;
            m5 = n_as_size_t - (j % n_as_size_t);
            if (m5 >= n_as_size_t)
                m5 -= n_as_size_t;
            }
        else
            m5 = i % n_as_size_t;
        printf ("(i %Id) ((i modulo %Id) (s = (d modulo s) %Id) (d = (d modulo s) %Id) (d = (d modulo d) %Id) (s = (d modulo d) %Id) (tested %Id)/n",
                    (int)i, (int)n_as_size_t, (int)m1, (int)m2, (int)m3, (int)m4, (int)m5);
        }
    }
#endif

void GetOffsets (CurveVectorPtr pathA, double offsetDistance, double arcAngle, CurveVectorPtr &pathB)
    {
    // unused - static double s_chamferAngle = -1.0;
    CurveOffsetOptions options (offsetDistance);
    options.SetArcAngle (arcAngle);
    pathB = pathA->CloneOffsetCurvesXY(options);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CircleChords,TriplePointsA)
    {
    bvector<DPoint3d> points{
           DPoint3d::From (299700.15037042351,256513.87138485495,152.19232662650333),
           DPoint3d::From (299703.51244670496,256512.38272162498,152.19316990035730),
           DPoint3d::From (299706.94521635934,256511.06523627986,152.19457535678063),
           DPoint3d::From (299710.43997044861,256509.92227128040,152.19654299577331),
           DPoint3d::From (299713.98784278031,256508.95672632931,152.19907281733526),
           DPoint3d::From (299717.57983240084,256508.17105101451,152.20216482146657),
           DPoint3d::From (299721.20682643115,256507.56723859458,152.20581900816720),
           DPoint3d::From (299724.85962318594,256507.14682094182,152.21003537743712)
   };
   DVec3d shift = DVec3d::From (points[0]);
   for (auto &point : points)
    point = point - shift;
   auto cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
   cvA->push_back(
    ICurvePrimitive::CreateLineString (points));
    double offset = -0.20812;
            CurveOffsetOptions options (offset);
        CurveVectorPtr cvB = cvA->CloneOffsetCurvesXY(options);
    Check::SaveTransformed (*cvA);
    Check::SaveTransformed (*cvB);
    Check::ClearGeometry ("CircleChords.TriplePointsA");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CircleChords,TriplePointsB)
    {
    bvector<DPoint3d> points{
            DPoint3d::From (0,0),
            DPoint3d::From (10, 0),
            DPoint3d::From (10,10),
            DPoint3d::From (20,10),
            DPoint3d::From (30, 10.1),
            DPoint3d::From (40, 10.1),
            DPoint3d::From (50, 10.11),
            DPoint3d::From (60,10.11),
            DPoint3d::From (70,12)
            };

    // transform to create some slope . .
    double dzdx = 0.01;
    double dy = 20.0;
    for (auto transform : bvector<Transform>
        {
        Transform::FromIdentity (),
        Transform::FromRowValues (
                1, 0, 0, 0,
                0, 1, 0, 0,
                dzdx, 0, 1, 0),
        Transform::FromRowValues (
                1, 0, 0, 0,
                0, 1, 0, 0,
                10.0 * dzdx, 0, 1, 0),
        Transform::FromRowValues (
                1, 0, 0, 0,
                0, 1, 0, 0,
                100.0 * dzdx, 0, 1, 0)

        })
        {
        SaveAndRestoreCheckTransform shifter (0, dy,0);

        auto cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        cvA->push_back(
        ICurvePrimitive::CreateLineString (points));
        cvA->TransformInPlace (transform);
        double offset = -0.20812;
                CurveOffsetOptions options (offset);
            CurveVectorPtr cvB = cvA->CloneOffsetCurvesXY(options);
        Check::SaveTransformed (*cvA);
        Check::SaveTransformed (*cvB);
        }
    Check::ClearGeometry ("CircleChords.TriplePointsB");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CloneOffset, RectangleOffset)
    {
    bvector<DPoint3d> points;
    double ax = 4.0;
    double ay = 3.0;
    double x0 = 1.0;
    double x1 = x0 + ax;
    double y0 = 2.0;
    double y1 = y0 + ay;
    AddPoint (points, x0, y0);
    AddPoint (points, x1, y0);
    AddPoint (points, x1, y1);
    AddPoint (points, x0, y1);
    AddPoint (points, x0, y0);
    for (double chamferAngle = 0.0; chamferAngle < 1.0; chamferAngle += 0.85)
        {
        for (double offset = 0.5; offset > -2.0; offset -= 1.0)
            {
            CurveVectorPtr pathA = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
            CurveVectorPtr pathB;
            GetOffsets (pathA, offset, -1.0, pathB);
            if (s_noisy > 0)
                {
                printf ("\n\n OFFSET DISTANCE %#.17g\n", offset);        

                Check::Print (pathA, "BaseCurve");
                Check::Print (pathB, "Line Offset");
                }
            if (offset > -0.45 * ax && offset > -0.45 * ay)
                {
                Check::Near (2.0 * ax + 2.0 * ay, pathA->Length (), "Pre offset length");
                Check::Near (2.0 * ax + 2.0 * ay + 8.0 * offset, pathB->Length (), "Line offset length");
                }
                        
            CurveVectorPtr pathC;
            GetOffsets (pathA, offset, 0.001, pathC);
            if (s_noisy > 0)
                {
                Check::Print (pathC, "Arc Offset");
                }
            if (offset > 0.0)
                Check::Near (2.0 * ax + 2.0 * ay + 4.0 * offset * Angle::PiOver2 (), pathC->Length (), "Arc offset length");
                
            CurveOffsetOptions options (offset);
            options.SetChamferAngle (chamferAngle);
            if (chamferAngle > 0.0)
                options.SetArcAngle (0.0);
            else
                options.SetArcAngle (0.1); 
            CurveVectorPtr pathD = pathA->AreaOffset(options);
            if (s_noisy > 0)
                Check::Print (pathD, "Area Offset");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CloneOffset, ArcOffset0)
    {
    bvector<DPoint3d> points;
    double ax = 4.0;
    double ay = 3.0;
    double b = 1.0;
    AddPoint (points, b, 0.0);
    AddPoint (points, ax, 0.0);
    AddPoint (points, ax, ay);
    AddPoint (points, 0.0, ay);
    AddPoint (points, 0.0, b);
    
    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    pathA->push_back (ICurvePrimitive::CreateLineString (points));
    pathA->push_back (ICurvePrimitive::CreateArc (DEllipse3d::FromVectors
                (
                DPoint3d::From (b,b,0),
                DVec3d::From (-b,0,0),
                DVec3d::From (0, -b, 0),
                0.0, Angle::PiOver2 ()
                )));

    for (double offset = 1.5; offset > -2.0; offset -= 1.0)
        {
        Check::StartScope ("Offset from arc", offset);
        CurveOffsetOptions options (offset);
        CurveVectorPtr pathB = pathA->AreaOffset(options);
        if (s_noisy > 4)
            {
            Check::Print (pathA, "BaseCurve");
            Check::Print (pathB, "Offset");
            }
        Check::EndScope ();
        }
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CloneOffset, ArcOffset1)
    {
    
    CurveVectorPtr diskA= CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    diskA->Add (ICurvePrimitive::CreateArc (DEllipse3d::FromXYMajorMinor (0,0, 0, 5,5, 0, 0.0, Angle::TwoPi ())));
    CurveVectorPtr diskB= CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);
    diskB->Add (ICurvePrimitive::CreateArc (DEllipse3d::FromXYMajorMinor (0,0, 0, 1,1, 0, 0.0, -Angle::TwoPi ())));
    
    CurveVectorPtr parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    parityRegion->Add (diskA);
    parityRegion->Add (diskB);

    for (double offset = 1.5; offset > -2.0; offset -= 1.0)
        {
        Check::StartScope ("Offset from arc", offset);
        CurveOffsetOptions options (offset);
        CurveVectorPtr regionB = parityRegion->AreaOffset(options);
        if (s_noisy > 0)
            {
            Check::Print (parityRegion, "BaseCurve");
            Check::Print (regionB, "Offset");
            }
        regionB->ConsolidateAdjacentPrimitives ();
        if (s_noisy > 0)
            Check::Print (regionB, "ConsolidatedOffset");
        Check::EndScope ();
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CloneOffset, RightTriangleOffset)
    {
    bvector<DPoint3d> points;
    double ax = 4.0;
    double ay = 3.0;
    AddPoint (points, 0.0, 0.0);
    AddPoint (points, ax, 0.0);
    AddPoint (points, 0.0, ay);
    AddPoint (points, 0.0, 0.0);

    for (double offset = 0.5; offset > -1.0; offset -= 1.0)
        {
        CurveVectorPtr pathA = CurveVector::CreateLinear (points, CurveVector::BOUNDARY_TYPE_Outer);
        CurveOffsetOptions options (offset);
        CurveVectorPtr pathB = pathA->CloneOffsetCurvesXY(options);
        if (s_noisy > 0)
            {
            Check::Print (pathA, "BaseCurve");
            Check::Print (pathB, "Offset");
            }
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CloneOffset, BsplineOffset)
    {
    bvector<DPoint3d> points;
    double offset = 0.2;
    for (int order = 3; order < 6; order++)
        {
        points.clear ();
        double x = 0.0;
        double y = 0.0;
        double dx = 0.3;
        for (int k = 0; k < 2 * order; k++)
            {
            x = k * dx;
            y = sin (x);
            AddPoint (points, x, y);
            }
        MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (points, NULL, NULL, order, false, false);
        CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open,
                ICurvePrimitive::CreateBsplineCurve (*curve));
        CurveOffsetOptions options (offset);
        options.SetBCurveMethod (0);

        for (int numPerKnot = 2; numPerKnot < 20; numPerKnot = (int) (1.5 * numPerKnot))
            {
            options.SetBCurvePointsPerKnot (numPerKnot);
            CurveVectorPtr pathB = pathA->CloneOffsetCurvesXY(options);
            if (s_noisy > 9)
                {
                Check::Print (pathA, "BaseCurveBspline");
                Check::Print (pathB, "Offset");
                }
            int numTest = 40;
            if (Check::Size (1, pathB->size (), "Bcurve offsets to single primitive"))
                {
                DRange1d distanceRange = DRange1d ();
                for (int i = 0; i <= numTest; i++)
                    {

                    double f = i / (double)numTest;
                    DPoint3d offsetPoint;
                    CurveLocationDetail location;
                    pathB->at(0)->FractionToPoint (f, offsetPoint);
                    if (pathA->ClosestPointBounded (offsetPoint, location))
                        {
                        distanceRange.Extend (offsetPoint.Distance (location.point));
                        }
                    }
                double eMax = DoubleOps::MaxAbs (distanceRange.low - offset, distanceRange.high - offset);
                static double s_offsetTolerance = 2.0e-3;
                if (!Check::True (eMax < s_offsetTolerance))
                    printf ("BCurve order %d  poles %d numPerknot %d offset error range (%.3le   %.3le)\n",
                        (int)curve->GetOrder (),
                        curve->NumberAllocatedPoles (),
                        numPerKnot,
                        distanceRange.low - offset, distanceRange.high - offset);
                }
            }
        }
    }

ICurvePrimitivePtr ConstructBsplineCurveFromEndsAndTangents (DPoint3dCR pointA, DVec3dCR tangentA, DPoint3dCR pointB, DVec3dCR tangentB)
    {
    DRay3d rayA = DRay3d::FromOriginAndVector (pointA, tangentA);
    DRay3d rayB = DRay3d::FromOriginAndVector (pointB, tangentB);
    double fractionA, fractionB;
    DPoint3d approachA, approachB;
    if (DRay3d::ClosestApproachUnboundedRayUnboundedRay (fractionA, fractionB, approachA, approachB, rayA, rayB))
        {
        DPoint3d midPoint = DPoint3d::FromInterpolate (approachA, 0.5, approachB);
        bvector<DPoint3d> poles;
        poles.push_back (pointA);
        poles.push_back (midPoint);
        poles.push_back (pointB);
        bvector<double>weights;
        DVec3d vectorA = DVec3d::FromStartEnd (pointA, approachA);
        DVec3d vectorB = DVec3d::FromStartEnd (approachB, pointB);
        double theta = vectorA.AngleTo (vectorB);
        double w = cos (theta * 0.5);
        weights.push_back (1.0);
        weights.push_back (w);
        weights.push_back (1.0);
        MSBsplineCurvePtr curvePtr = MSBsplineCurve::CreateFromPolesAndOrder (poles, &weights, NULL, 3, false, false);
        return ICurvePrimitive::CreateBsplineCurve (curvePtr);        
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST(Bspline, ConstructFromEnds)
    {
    ICurvePrimitivePtr curve = ConstructBsplineCurveFromEndsAndTangents (
            DPoint3d::From (1,2,0),
            DVec3d::From (0,1, 0),
            DPoint3d::From (2,3,0),
            DVec3d::From (1,0,0)
            );
    CurveVectorPtr chain = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    chain->push_back (curve);
    if (s_noisy > 0)
        Check::Print (chain, "unitTurn curve");
    
    bvector<DPoint3d> strokes;
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    curve->AddStrokes (strokes, *options);
    if (s_noisy > 0)
        {
        for (size_t i = 0; i < strokes.size (); i++)
            {
            Check::Print (strokes[i], "xyz");
            }
        }
    }


void ExciseParts (CurveVectorWithDistanceIndexPtr path, bvector<PathLocationDetailPair> &pairs, bvector<CurveVectorPtr> &parts)
    {
    for (PathLocationDetailPair &pair : pairs)
        {
        parts.push_back (path->CloneBetweenPathLocations (pair.DetailA(), pair.DetailB ()));
        }
    }


void TestPathMatch (CurveVectorPtr &cvA, CurveVectorPtr &cvB, ValidatedDouble expectedOverlap)
    {
    CurveVectorWithDistanceIndexPtr pathA  = CurveVectorWithDistanceIndex::Create ();
    CurveVectorWithDistanceIndexPtr pathB  = CurveVectorWithDistanceIndex::Create ();
    pathA->SetPath (cvA);
    pathB->SetPath (cvB);
    bvector<PathLocationDetailPair> pathAIntervals;
    bvector<PathLocationDetailPair> pathBIntervals;
    
    static double distanceTol = Angle::SmallAngle ();
    DRange1d strictPositive (distanceTol, DBL_MAX);
    DRange1d nonNegative (-distanceTol, DBL_MAX);
    DRange1d nearZero (-distanceTol, distanceTol);
    bvector<bool> falseTrue {false,true};
    for (bool compress : falseTrue)
        {
        for (bool includeGaps : falseTrue)
            {
            CurveVectorWithDistanceIndex::FindCommonSubPaths (*pathA, *pathB, pathAIntervals,pathBIntervals, includeGaps, compress);
            Check::Size (pathAIntervals.size (), pathBIntervals.size ());
            Check::ValidateDistances (pathAIntervals,
                            includeGaps ? nearZero : nonNegative,
                            nonNegative,
                            strictPositive
                            );

            Check::ValidateDistances (pathBIntervals,
                            includeGaps ? nearZero : nonNegative,
                            nonNegative,
                            strictPositive
                            );

            if (s_noisy)
                {
                GEOMAPI_PRINTF("\n\nCommonSubPath  (includeGaps %s) (compress %s)\n",
                            includeGaps ? "true" : "false",
                            compress ? "true" : "false"
                            );
                Check::Print (cvA, "pathA");
                Check::Print (cvB, "pathB");
                Check::Print (pathAIntervals, pathBIntervals);
                }
            bvector<CurveVectorPtr> partsA, partsB;
            ExciseParts (pathA, pathAIntervals, partsA);
            ExciseParts (pathB, pathBIntervals, partsB);
            if (Check::Size (partsA.size (), partsB.size ())
                && Check::Size (pathAIntervals.size (), partsA.size ()))
                {
                double totalOverlap = 0.0;
                for (size_t i = 0; i < partsA.size (); i++)
                    {
                    if (pathAIntervals[i].GetTagA () == 1)
                        {
                        double lengthA = partsA[i]->Length ();
                        double lengthB = partsB[i]->Length ();
                        Check::Near (lengthA, lengthB, "Length of overlap");
                        totalOverlap += lengthA;
                        }
                    }
                if (expectedOverlap.IsValid ())
                    {
                    if (s_noisy)
                        {
                        Check::PrintCoordinate ("\n(overlap expected ", expectedOverlap, ") ");
                        Check::PrintCoordinate (" (actual ", totalOverlap, ") ");
                        }
                    Check::Near (expectedOverlap, totalOverlap);
                    }
                }
            }
        }
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,MatchXYLineSegments)
    {
    CurveVectorPtr cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr cvB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    bvector<DPoint3d> pointA
        {
        DPoint3d::From ( 0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (20,0,0),
        DPoint3d::From (30,0,0),
        DPoint3d::From (60,0,0)
        };

    bvector<DPoint3d> pointB        
        {
        DPoint3d::From (5,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (15,0,0),
        DPoint3d::From (15,5,0),
        DPoint3d::From (25,5,0),
        DPoint3d::From (25,0,0),
        DPoint3d::From (50,0,0)
        };
    double expectedOverlap = 35.0;  // gotta look at the numbers.
    for (size_t i = 0; i + 1 < pointA.size (); i++)
        cvA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA[i], pointA[i+1])));
    for (size_t i = 0; i + 1 < pointB.size (); i++)
        cvB->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointB[i], pointB[i+1])));

    TestPathMatch (cvA, cvB, ValidatedDouble (expectedOverlap));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,MatchXYLineSegments_AlmostParallel)
    {


    for (double yy : bvector<double> {0, 1.0e-15, 1.0e-13, 1.0e-12, 2.0e-8})
        {
    Check::StartScope ("yy", yy);            
        CurveVectorPtr cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        CurveVectorPtr cvB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        bvector<DPoint3d> pointA
            {
            DPoint3d::From ( 0,0,0),
            DPoint3d::From (20,0,0),
            };
        // make the "other" line just a little non-parallel
        bvector<DPoint3d> pointB        
            {
            DPoint3d::From (3,yy,0),
            DPoint3d::From (8,-yy * 0.2342367,0),
            DPoint3d::From (18,1.0e-8,0),   // "clearly" not parallel?
            };

        for (size_t i = 0; i + 1 < pointA.size (); i++)
            cvA->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA[i], pointA[i+1])));
        for (size_t i = 0; i + 1 < pointB.size (); i++)
            cvB->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointB[i], pointB[i+1])));
        double expectedOverlap = 5.0;  // gotta look at the numbers.
        if (yy > 1.0e-10)
            expectedOverlap = 0.0;
        TestPathMatch (cvA, cvB, ValidatedDouble (expectedOverlap));
    Check::EndScope ();            
        }
    }

#define TEST_ALL_CURVE_MATCH_not
#ifdef TEST_ALL_CURVE_MATCH
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,MatchXYArc0)
    {
    size_t numTest = 0;
    CurveVectorPtr cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr cvB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    int numPerQuadrant = 2;
    int numPerCircle = 4 * numPerQuadrant;
    double radiansPerSector = Angle::TwoPi () / (double)numPerCircle;
    int sectorsInA = 2 * numPerQuadrant;
    double sweepA = sectorsInA * radiansPerSector;
    double startRadiansA = 0.1;
    for (int numSweptSector = 1; numSweptSector <= numPerCircle; numSweptSector++)
        {
        DSegment1d intervalA (startRadiansA, startRadiansA + sweepA);
        double startRadiansA1 = startRadiansA + Angle::TwoPi ();
        DSegment1d intervalA1 (startRadiansA1, startRadiansA1 + sweepA);     // Another copy of A, advanced 360 degrees.
        DEllipse3d ellipseA = DEllipse3d::From (0,0,0,   1,0,0,  0,1,0,   startRadiansA,  sweepA);
        // B is trickier ...
        for (int i0 = -numPerQuadrant; i0 < numPerCircle; i0++)
            {
            double startB = startRadiansA + i0 * radiansPerSector;
            double sweepB = numSweptSector * radiansPerSector;
            DSegment1d intervalB (startB, startB + sweepB);
            DEllipse3d ellipseB = DEllipse3d::From (0,0,0,   1,0,0,  0,1,0,   startB, sweepB);
            cvA->clear ();
            cvB->clear ();
            cvA->push_back (ICurvePrimitive::CreateArc (ellipseA));
            cvB->push_back (ICurvePrimitive::CreateArc (ellipseB));
            // Don't consider anything that leaves and reenters -- the path logic just doesn't do that . ..
            ValidatedDSegment1d intervalC1 = intervalA1.NonZeroDirectedOverlap (intervalB);
            if (!intervalC1.IsValid ())
               {
                double expectedOverlap = 0.0;
                auto intervalC = intervalA.DSegment1d::NonZeroDirectedOverlap (intervalB);
                if (intervalC.IsValid ())
                    {
                    expectedOverlap = intervalC.Value ().Length ();
                    }
                if (s_noisy)
                    printf (" (numSwept %d i0 %d  numA %d)\n", numSweptSector, i0, sectorsInA);
                TestPathMatch (cvA, cvB, ValidatedDouble (expectedOverlap));
                numTest ++;
               }
           }
        }
    }
#endif

#define TEST_ALL_CURVE_MATCH_not
#ifdef TEST_ALL_CURVE_MATCH
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,MatchXYLineBspline0)
    {
    CurveVectorPtr cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr cvB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    bvector<DPoint3d> pointA
        {
        DPoint3d::From ( 0,0,0),
        DPoint3d::From (10,0,0),
        DPoint3d::From (20,0,0),
        DPoint3d::From (30,0,0),
        DPoint3d::From (40,5,0),
        DPoint3d::From (50,5,0)
        };

    MSBsplineCurvePtr bcurveA = MSBsplineCurve::CreateFromInterpolationAtBasisFunctionPeaks (pointA, 3, 0);
    MSBsplineCurvePtr bcurveB = MSBsplineCurve::CreateFromInterpolationAtBasisFunctionPeaks (pointA, 3, 0);
    cvA->push_back (ICurvePrimitive::CreateBsplineCurve (bcurveA));
    cvB->push_back (ICurvePrimitive::CreateBsplineCurve (bcurveB));
    auto expectedOverlap = cvB->Length ();
    TestPathMatch (cvA, cvB, ValidatedDouble (expectedOverlap));
    }
#endif

void AppendStroke (CurveVectorPtr &cv, DVec3dCR vector)
    {
    DPoint3d point0, point1;
    cv->GetStartEnd (point0, point1);
    cv->push_back (ICurvePrimitive::CreateLine (
            DSegment3d::From (point1, point1 + vector)
            ));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,MatchXYSpiral)
    {
    CurveVectorPtr cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr cvB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);

    Transform transform = Transform::FromIdentity ();
    ICurvePrimitivePtr spiral0 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,0.0,
            20.0, 100.0,
            transform,
            0.0, 1.0
            );

    cvA->push_back (spiral0);
    cvB->push_back (spiral0->Clone ());
    auto expectedOverlap = cvB->Length ();
    AppendStroke (cvA, DVec3d::From (10,0,0));
    AppendStroke (cvB, DVec3d::From (10,10,0));
    TestPathMatch (cvA, cvB, ValidatedDouble (expectedOverlap));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveOffset,OffsetXYSpiral)
    {

    Transform transform = Transform::FromIdentity ();

    for (double finalRadius : bvector<double> {300.0, -300.0})
        {
        CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        Check::Print (finalRadius, "final radius");
        ICurvePrimitivePtr spiral0 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
                (
                DSpiral2dBase::TransitionType_Clothoid,
                0.0,finalRadius * 1000.0,
                20.0, finalRadius,
                transform,
                0.0, 1.0
                );

        pathA->push_back (ICurvePrimitive::CreateArc (
                DEllipse3d::From (-20, 10,0, -10,0,0,    0,-10,0,    0.0, Angle::DegreesToRadians (90))));
        pathA->push_back (ICurvePrimitive::CreateLine (
                    DSegment3d::From (DPoint3d::From (-20, 0, 0), DPoint3d::From (0,0,0))));
        pathA->push_back (spiral0);
        int numPerKnot = 3;

        for (double d : bvector<double> {2.0, -2.0})
            {
            Check::Print (d, "Offset distance");
            CurveOffsetOptions options (d);
            options.SetBCurvePointsPerKnot (numPerKnot);
            CurveVectorPtr pathB = pathA->CloneOffsetCurvesXY(options);
            for (size_t i = 0; i + 1 < pathB->size (); i++)
                {
                DPoint3d xyzA, xyzB;
                pathB->at(i)->FractionToPoint (1.0, xyzA);
                pathB->at(i+1)->FractionToPoint (0.0, xyzB);\
                // Force fluffy tolerance ...
                Check::Near (xyzA, xyzB, "OffsetPath start End Match", 1.0e9);
                }
            Check::Print (pathA, "basecurve");
            Check::Print (pathB, "Offset");
            }
        }

    }

static double s_dumpCurves = false;
double ApproximateOffsetArea (char const * name, ICurvePrimitivePtr const &baseCurve, double offsetDistance)
    {
    if (!baseCurve.IsValid ())
        return 0.0;
    Check::PrintHeading ("***  ApproximateAreaOffset", name);
    CurveVectorPtr pathA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    pathA->push_back (baseCurve);

    int numPerKnot = 3;
    CurveOffsetOptions offsetOptions (offsetDistance);
    offsetOptions.SetBCurvePointsPerKnot (numPerKnot);

    CurveVectorPtr pathB = pathA->CloneOffsetCurvesXY(offsetOptions);
    bvector<DPoint3d> points;
    if (s_dumpCurves)
        {
        Check::Print (pathA, "basecurve");
        Check::Print (pathB, "offset");
        }

    IFacetOptionsPtr strokeOptions = IFacetOptions::CreateForCurves ();
    pathA->AddStrokePoints (points, *strokeOptions);
    DPoint3dOps::Reverse (points);
    pathB->AddStrokePoints (points, *strokeOptions);
    double area = PolygonOps::AreaXY (points);
    double length;
    baseCurve->Length (length);
    Check::Print (length, "baseLength");
    Check::Print (offsetDistance, "offset");
    Check::Print (area, "area");
    return area;    
    }

ICurvePrimitivePtr CloneAsBspline (ICurvePrimitivePtr const &source)
    {
    MSBsplineCurve bcurve;
    if (source.IsValid () && source->GetMSBsplineCurve (bcurve))
        return ICurvePrimitive::CreateBsplineCurve (bcurve);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveOffset,OffsetXYSpiralSenseCheck)
    {

    Transform transform = Transform::FromIdentity ();
    double length = 20.0;
    for (double finalRadius : bvector<double> {300.0, -300.0, 500.0, -500.0})
        {
        Check::PrintIndent (2);
        Check::Print (finalRadius, "final radius");
        ICurvePrimitivePtr spiral0 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
                (
                DSpiral2dBase::TransitionType_Clothoid,
                0.0,finalRadius * 1000.0,
                length, finalRadius,
                transform,
                0.0, 1.0
                );
        double theta = length / finalRadius;
        auto arc = DEllipse3d::From (0, finalRadius, 0,   0,-finalRadius,0,   finalRadius, 0,0,   0.0, theta);
        ICurvePrimitivePtr line0 = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, length,0,0));

        ICurvePrimitivePtr arc0 = ICurvePrimitive::CreateArc (arc);
        auto spiral0BCurve = CloneAsBspline (spiral0);
        auto line0BCurve = CloneAsBspline (line0);
        for (double d : bvector<double> {2.0, -2.0})
            {
            Check::PrintIndent (2);
            double aSpiral = ApproximateOffsetArea ("Spiral", spiral0, d);
            double aSpiralBCurve = ApproximateOffsetArea ("SpiralAsBCurve", spiral0BCurve, d);
            double aLine = ApproximateOffsetArea ("line", line0, d);
            // double aLineBCurve = ApproximateOffsetArea ("lineAsBCurve", line0BCurve, d);
            double aArc = ApproximateOffsetArea ("arc", arc0, d);
            Check::PrintIndent (3);
            Check::Print ((aSpiral - aLine)/ DoubleOps::Max (aSpiral, aLine), "Spiral::Line Area Diff");
            Check::Print ((aSpiralBCurve - aLine)/ DoubleOps::Max (aSpiral, aLine), "Spiral::SpiralBCurve Area Diff");
            Check::True (aSpiral * aLine > 0.0, "Spiral, line offsets to same side.");
            Check::True (aSpiralBCurve * aSpiral > 0.0, "Spiral vs spiral BCurve");
            Check::True (aSpiral * aArc > 0.0, "Spiral vs arc");
            }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,OffsetIntersection)
    {
    bvector<DPoint3d> xyzA {DPoint3d::From (50,0,0), DPoint3d::From (50,100,0)};
    bvector<DPoint3d> xyzB {DPoint3d::From ( 0,20,0), DPoint3d::From (100,20,0)};
    CurveVectorPtr clA = CurveVector::CreateLinear (xyzA, CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr clB = CurveVector::CreateLinear (xyzB, CurveVector::BOUNDARY_TYPE_Open);

    CurveVectorPtr intersectionsA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    CurveVectorPtr intersectionsB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);


    bvector<double> signs {1.0, -1.0};
    double wA = 5.0;
    double wB = 4.0;
    double filletRadius = 2.0;
    for (double sA : signs)
        {
        for (double sB : signs)
            {
            CurveOffsetOptions optionsA (sA * (wA + filletRadius));
            CurveOffsetOptions optionsB (sB * (wB + filletRadius));
            auto offsetA = clA->CloneOffsetCurvesXY (optionsA);
            auto offsetB = clB->CloneOffsetCurvesXY (optionsB);
            CurveCurve::IntersectionsXY (*intersectionsA, *intersectionsB, *offsetA, *offsetB, nullptr);
            for (size_t i = 0; i < intersectionsA->size (); i++)
                {
                DPoint3d pointA, pointB;
                double fractionA, fractionB;
                CurveLocationDetail detailA, detailB;
                if (CurveCurve::IsSinglePointPair (
                            *intersectionsA, *intersectionsB, i,
                            fractionA, pointA, fractionB, pointB))
                    {
                    Check::Near (pointA, pointB);
                    clA->ClosestPointBoundedXY (pointA, nullptr, detailA);
                    clB->ClosestPointBoundedXY (pointB, nullptr, detailB);
                    DVec3d unitA = DVec3d::FromStartEnd (pointA, detailA.point).ValidatedNormalize ();
                    DVec3d unitB = DVec3d::FromStartEnd (pointB, detailB.point).ValidatedNormalize ();
                    DPoint3d filletPointA = pointA + filletRadius * unitA;
                    DPoint3d filletPointB = pointB + filletRadius * unitB;
                    // meaningless tests, but it keeps the vars alive for debugger.
                    Check::Near (pointB.Distance(filletPointB), pointA.Distance (filletPointA));
                    }
                }
            }
        }
    }

void TestDistanceAlong (CurveVectorR curve)
    {
    double length = curve.Length ();
    CurveLocationDetail startDetail, endDetail;
    curve.GetStartEnd (startDetail, endDetail);
    double d0 = 0.0;
    CurveLocationDetail baseDetail = startDetail;
    for (double f : bvector<double>{0.1, 0.25, 0.8, 0.5, 1.0})
        {
        double d1 = f * length;
        double delta = d1 - d0;
        auto nextDetail = curve.PointAtSignedDistanceAlong (baseDetail, delta);
        if (!Check::True (nextDetail.IsValid ()))
            return;
        baseDetail = nextDetail;
        d0 = d1;
        }
    Check::Near (endDetail.point, baseDetail.point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,PointAtDistanceAlong)
    {
    bvector<CurveVectorPtr> curves;
    SampleGeometryCreator::AddMultiPrimitiveXYOpenPaths (curves);
    for (CurveVectorPtr &curve : curves)
        TestDistanceAlong (*curve);
    }



void testCurveCurveTaperFilletTaper
(
ICurvePrimitiveR curveA,
ICurvePrimitiveR curveB,
double fractionA,
double fractionB,
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB              //!< [in] taper distance along vectorB
)
    {

    auto chain = CurveCurve::CurveCurve::ConstructTaperFilletTaper (curveA, curveB, setbackA, taperA, filletRadius, setbackB, taperB, fractionA, fractionB);
    if (
            Check::True (chain.IsValid (), "Chain returned")
        &&  Check::Size (3, chain->size (), "3 primitives")
        )
        {
        auto ray0End = chain->at(0)->FractionToPointAndUnitTangent (1.0).Value ();
        auto ray1Start = chain->at(1)->FractionToPointAndUnitTangent (0.0).Value ();

        auto ray2End = chain->at(1)->FractionToPointAndUnitTangent (1.0).Value ();
        auto ray3Start = chain->at(2)->FractionToPointAndUnitTangent (0.0).Value ();

        Check::Near (ray0End.origin, ray1Start.origin, "Tangent point A");
        Check::Parallel (ray0End.direction, ray1Start.direction, "Confirm tangency A");

        Check::Near (ray2End.origin, ray3Start.origin, "Tangent point B");
        Check::Parallel (ray2End.direction, ray3Start.direction, "Confirm tangency B");
        Check::Print (curveA);
        Check::Print (curveB);
        Check::Print (chain, "SymmetricTaper");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,TaperFilletTaper)
    {
    auto lineA = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 10,0,0));
    auto lineB = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 0,10,0));
    testCurveCurveTaperFilletTaper (*lineA, *lineB, 0.3, 0.3, 0.1, 1.5, 1.0, 0.2, 1.0);
    auto arcA = ICurvePrimitive::CreateArc (DEllipse3d::FromStartTangentNormalRadiusSweep (
                        DPoint3d::From (0,0,0),
                        DVec3d::From (1,-0.3,0),
                        DVec3d::From (0,0,1),
                        20.0, Angle::DegreesToRadians (30)
                        ));
    auto arcB = ICurvePrimitive::CreateArc (DEllipse3d::FromStartTangentNormalRadiusSweep (
                        DPoint3d::From (0,0,0),
                        DVec3d::From (0,1,0),
                        DVec3d::From (0,0,1),
                        -10.0, Angle::DegreesToRadians (30)
                        ));
    testCurveCurveTaperFilletTaper (*arcA, *arcB, 0.3, 0.3, 0.1, 1.5, 1.0, 0.2, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,MultiRadiusBlendInCorner)
    {
    double dx = 15.0;
    double dy = 15.0;
    DPoint3d origin = DPoint3d::From (0,0,0);
    DVec3d   vectorU = DVec3d::From (10,0,0);
    DVec3d   vectorV = DVec3d::From (-2,10,0);
    for (bool reverse : bvector<bool> {false, true})
        {
        SaveAndRestoreCheckTransform shifter (0, dy,0);
        for (double thetaDegrees : bvector<double> { 0.0, 10.0, 30.0})
            {
            SaveAndRestoreCheckTransform shifter (dx,0,0);
            bvector<double> radii {5.0};
            Angle theta = Angle::FromDegrees (thetaDegrees);
            auto curves = CurveCurve::ConstructMultiRadiusBlend (origin, vectorU, vectorV,
                        theta, 1.0,
                        radii,
                        1.0, theta,
                        reverse
                        );
            Check::SaveTransformed (bvector<DPoint3d> {origin + vectorV, origin, origin + vectorU});
            if (curves.IsValid ())
                Check::SaveTransformed (*curves);
            }
        }
    Check::ClearGeometry ("CurveCurve.MultiRadiusBlendInCorner");
    }


void RunTaperFilletTaper (char const * jsonA, char const * jsonB,
double setbackA,
double taperA,
double filletRadius,
double setbackB,
double taperB,
double &distanceA,
double &distanceB,
double offsetA,
double offsetB,
bool  setExtendedPath = false   // false forces new logic to use tangent extension
)
    {
    bvector<IGeometryPtr> geometryA, geometryB;
    BentleyGeometryJson::TryJsonStringToGeometry (jsonA, geometryA);
    BentleyGeometryJson::TryJsonStringToGeometry (jsonB, geometryB);
    Check::SaveTransformed (geometryA);
    Check::SaveTransformed (geometryB);

    for (auto &gA : geometryA)
        {
        for (auto &gB : geometryB)
            {
            auto cvA = gA->GetAsCurveVector ();
            auto cvB = gB->GetAsCurveVector ();
            if (cvA.IsValid () && cvB.IsValid ())
                {
                PathLocationDetail startA, endA, startB, endB;
                auto pathA = CurveVectorWithDistanceIndex::Create ();
                auto pathB = CurveVectorWithDistanceIndex::Create ();

                if (setExtendedPath)
                    {
                    pathA->SetExtendedPath (cvA,
                        1.0,
                        startA, endA, false, 2.0);
                    pathB->SetExtendedPath (cvB,
                        1.0,
                        startB, endB, false, 2.0);
                    }
                else
                    {
                    pathA->SetPath (cvA);
                    pathB->SetPath (cvB);
                    }
                auto rayA = pathA->DistanceAlongToPointAndUnitTangent (distanceA);
                auto rayB = pathB->DistanceAlongToPointAndUnitTangent (distanceB);
                double markerSize = pathA->TotalPathLength () * 0.04;
                Check::SaveTransformedMarkers (
                        bvector<DPoint3d> {rayA.Value ().origin, rayB.Value ().origin}, markerSize);
                auto blend = CurveCurve::ConstructTaperFilletTaper
                    (
                    *pathA, *pathB,
                    setbackA, taperA, filletRadius, setbackB, taperB,
                    distanceA, distanceB,
                    offsetA, offsetB
                    );
                if (blend.IsValid ())
                    Check::SaveTransformed (*blend);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,TaperFilletTaperBad)
    {
    char const * jsonA = "{\"DgnCurveVector\":{\"Member\":[{\"LineSegment\":{\"endPoint\":[591651.21287459158,4615479.0561747560,0.0],\"startPoint\":[591709.10318203433,4615489.1559713706,0.0]}}],\"boundaryType\":1}}\n";
    char const * jsonB = "{\"DgnCurveVector\":{\"Member\":[{\"LineSegment\":{\"endPoint\":[591716.22036095220,4615457.3934983332,0.0],\"startPoint\":[591709.10318083060,4615489.1559721744,0.0]}}],\"boundaryType\":1}}\n";

    //double distanceA = 25.64;
    //double distanceB = 32.55;
    double offsetA = 12;
    double offsetB = 4.3;
    double filletRadius  = 15.239999999943588;
    double distanceA = 25.635999999943589;
    double distanceB = 32.550099806237583;

    RunTaperFilletTaper (jsonA, jsonB,
                    0.6096, 6.096,
                    filletRadius,
                    0.6096, 6.096,
                    distanceA, distanceB,
                    offsetA, offsetB, false
                    );
    Check::ClearGeometry ("CurveCurve.TaperFilletTaperBad");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,TaperFilletTaperOutOfBounds)
    {

    char const * jsonA = "{\"DgnCurveVector\":{\"Member\":[{\"LineSegment\":{\"endPoint\":[591709.10318203433,4615489.1559713706,0.0],\"startPoint\":[591766.28518248675,4615499.1321936445,0.0]}}],\"boundaryType\":1}}\n";
    char const * jsonB = "{\"DgnCurveVector\":{\"Member\":[{\"LineSegment\":{\"endPoint\":[591709.10318083060,4615489.1559721744,0.0],\"startPoint\":[591699.79062477022,4615530.7159458166,0.0]}}],\"boundaryType\":1}}\n";
    double a = 200.0;
    double b = 3000.0;
    bvector<double> signs {1.0, -1.0};
    // These two paths are perpendicular lines, with distance 0 at their intersection.
    // The outer loops select combinations of positive and negative signs for offset.
    // The inner loops run taper search from start spots before, along, and after the curve bodies.
    // All cases converge to "first quadrant" arcs solutions (with negative offset, parts of the solution curve are 
    //   shifted to the other side -- but the arcs still go to the quadrant.)
    for (double signA : signs)
        {
        SaveAndRestoreCheckTransform shifter (0,b,0);
        for (double signB : signs)
            {
            SaveAndRestoreCheckTransform shifter (b, 0, 0);
            for (double distanceA : bvector<double> {-1.0, 1.0, 10.0, 20.0, 40.0, 45.0,  50.0, 80.0})
                {
                SaveAndRestoreCheckTransform shifter (0,a,0);

                for (double distanceB : bvector<double> {-1.0, 1.0, 10.0, 20.0, 40.0, 40.0, 45.0, 80.0 })
                    {
                    SaveAndRestoreCheckTransform shifter (a, 0, 0);
                    double offsetA = signA * 12;
                    double offsetB = signB * 4.3;
                    double dA = distanceA;
                    double dB = distanceB;
                    RunTaperFilletTaper (jsonA, jsonB,
                                    0.6096, 6.096,
                                    22.86,
                                    0.6096, 6.096,
                                    dA, dB,
                                    offsetA, offsetB, false
                                    );
                    }
                }
            }
        }
    Check::ClearGeometry ("CurveCurve.TaperFilletTaperOutOfBounds");
    }

bool testCurveCurveConstructMultiRadiusBlend
(
ICurvePrimitiveR curveA,
ICurvePrimitiveR curveB,
double fractionA,
double fractionB,
Angle thetaA,
double distanceA,
bvector<double> radii,
double distanceB,
Angle thetaB,
bool reverse
)
    {
    double a = 40.0;
    SaveAndRestoreCheckTransform shifter (0,a,0);
    Check::SaveTransformed (curveA);
    Check::SaveTransformed (curveB);
    auto chain = CurveCurve::ConstructMultiRadiusBlend (curveA, curveB, thetaA, distanceA, radii, distanceB, thetaB, fractionA, fractionB, reverse);
    if (!chain.IsValid ())
        return false;
    Check::SaveTransformed (*chain);
    size_t expectedArcs = radii.size ();
    if (distanceA != 0.0)
        expectedArcs++;
    if (distanceB != 0.0)
        expectedArcs++;
    if (    Check::True (chain.IsValid (), "Chain returned")
        &&  Check::Size (expectedArcs, chain->size (), "num arcs")
        )
        {
        bvector<DPoint3d> markers;
        for (size_t i = 0; i + 1 < chain->size (); i++)
            {
            auto end   = chain->at(i)->FractionToPointAndUnitTangent (1.0).Value ();
            auto start = chain->at(i+1)->FractionToPointAndUnitTangent (0.0).Value ();
            Check::Parallel (end.direction, start.direction, "Confirm tangency A");
            Check::Near (end.origin, start.origin, "Tangent point A");
            markers.push_back (end.origin);
            }
        Check::SaveTransformedMarkers (markers, 0.1);
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,MultiRadiusBlend)
    {
    double b = 40.0;
    double r = 2.0;
    Angle zeroAngle = Angle::FromDegrees (0.0);
    Angle angleA = Angle::FromDegrees (10.0);
    Angle angleA1 = Angle::FromDegrees (-15.0);
    Angle angleB = Angle::FromDegrees (3.0);

    double distanceA = 1.25;
    double distanceA1 = 2.0;
    double distanceB = 0.5;
    size_t errors = 0;
    for (bool reverse : bvector<bool> {false, true})
        {
        bvector<double> radii;
        for (size_t i = 0; i < 3; i++, r += 0.5)
            {
            SaveAndRestoreCheckTransform shifter (b,0,0);
            radii.push_back (r);
            auto lineA = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 10,0,0));
            auto lineB = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 0,10,0));
            if (!testCurveCurveConstructMultiRadiusBlend (*lineA, *lineB, 0.3, 0.3, zeroAngle, 0.0, radii, 0.0, zeroAngle, reverse))
                errors++;
            if (!testCurveCurveConstructMultiRadiusBlend (*lineA, *lineB, 0.3, 0.3, angleA, distanceA, radii, distanceB, angleB, reverse))
                errors++;

            auto arcA = ICurvePrimitive::CreateArc (DEllipse3d::FromStartTangentNormalRadiusSweep (
                                DPoint3d::From (0,0,0),
                                DVec3d::From (1,-0.3,0),
                                DVec3d::From (0,0,1),
                                50.0, Angle::DegreesToRadians (40)
                                ));
            auto arcB = ICurvePrimitive::CreateArc (DEllipse3d::FromStartTangentNormalRadiusSweep (
                                DPoint3d::From (0,0,0),
                                DVec3d::From (0,1,0),
                                DVec3d::From (0,0,1),
                                -20.0, Angle::DegreesToRadians (40)
                                ));
            if (!testCurveCurveConstructMultiRadiusBlend (*arcA, *arcB, 0.3, 0.3, zeroAngle, 0.0, radii, 0.0, zeroAngle, reverse))
                errors++;
            if (!testCurveCurveConstructMultiRadiusBlend (*arcA, *arcB, 0.3, 0.3, angleA, distanceA, radii, distanceB, angleB, reverse))
                errors++;
            if (!testCurveCurveConstructMultiRadiusBlend (*arcA, *arcB, 0.3, 0.3, angleA1, distanceA1, radii, distanceB, 2.0 * angleB, reverse))
                errors++;
            
            }
        }
    Check::ClearGeometry ("CurveCurve.MultiRadiusBlend");
    Check::Size (0, errors, "Failed blend construction");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorWithDistanceIndex,XYLength1)
    {
    double ax = 4.0;
    double az = 3.0;
    auto lineA = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, ax,0,az));
    auto cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cvA->push_back (lineA);
    auto pathA = CurveVectorWithDistanceIndex::Create ();
    pathA->SetPath (cvA);
    
    Check::Near (ax, pathA->TotalPathLengthXY ());
    Check::Near (hypot (ax, az), pathA->TotalPathLength ());

    auto cvB = cvA->CloneAsBsplines ();
    auto pathB = CurveVectorWithDistanceIndex::Create ();
    pathB->SetPath (cvB);
    
    Check::Near (ax, pathB->TotalPathLengthXY ());
    Check::Near (hypot (ax, az), pathB->TotalPathLength ());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorWithDistanceIndex,XYLength2)
    {
    double aSpiral = 20.0;
    // tip the spiral over to the xz plane.
    Transform transform = Transform::FromRowValues (
            1,0,0,0,
            0,0,-1,0,
            0,1,0,0
            );    
    auto spiral = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,0.0,
            aSpiral, 100.0,
            transform,
            0.0, 1.0
            );

    
    auto cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cvA->push_back (spiral);
    auto pathA = CurveVectorWithDistanceIndex::Create ();
    pathA->SetPath (cvA);
    double a = pathA->TotalPathLength ();
    double b = pathA->TotalPathLengthXY ();
    Check::PushTolerance (ToleranceSelect_Loose);
    Check::Near (aSpiral, a,"tilted spiral length");
    DPoint3d xyz0, xyz1;
    spiral->GetStartEnd (xyz0, xyz1);
    Check::Near (xyz1.x, b,"xy spiral length");
    Check::PopTolerance ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef DoConceptstationExample_bcurveLooksTooTileted
TEST(CurveCurve,TaperFilletTaperOnCurveVectorWithDistanceIndex_ConceptStationExample)
    {
    auto lineA = ICurvePrimitive::CreateLine (
            DSegment3d::From (
            291259.27273372654, 259426.78423992204, 0,
            291241.64096290554, 259543.75078044418, 0)
            );

    auto cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cvA->push_back (lineA);

    auto lineB = ICurvePrimitive::CreateLine (
            DSegment3d::From (
                290920.90443317365, 259517.45283875987, 0.00000000000000000,
//                291348.54982715682, 259534.91373305192, 0.00000000000000000));
                291348.54982715682, 259634.91373305192, 0.00000000000000000));

    auto cvB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cvB->push_back (lineB);

    if (s_noisy)
        {
        Check::Print (cvA, "X path");
        Check::Print (cvB, "Y path");
        }
    auto pathA = CurveVectorWithDistanceIndex::Create ();
    auto pathB = CurveVectorWithDistanceIndex::Create ();
    pathA->SetPath (cvA);
    pathB->SetPath (cvB);
    double filletRadius = 13.716000000000001;
    bvector<double> tapers {6.0960000000000001, 1.0, 2.0, 4.0, 8.0, 12.0 };
    for (double taper : tapers)
        {
        double e = 0.60960000000000003;
        
        double dA = 111.65317409719319;
        double dB = 111.65317309718409;
        if (s_noisy)
            {
            PathLocationDetail detailA, detailB;
            pathA->SearchByDistanceFromPathStart (dA, detailA);
            pathB->SearchByDistanceFromPathStart (dB, detailB);
            Check::Print (taper, "taper");
            Check::Print (e, "setback");
            Check::Print (filletRadius, "filletRadius");
            }
        
        auto blend = CurveCurve::ConstructTaperFilletTaper (*pathA, *pathB, e, taper, filletRadius, e, taper,
                dA, dB);
        if (Check::True (blend.IsValid ()))
            {
            if (s_noisy)
                Check::Print (*blend, "constructed taper");
            }
        }
    }
#endif //#ifdef DoConceptstationExample_bcurveLooksTooTileted

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveCurve,TaperFilletTaperOnCurveVectorWithDistanceIndex)
    {
    double ax = 10.0;
    double ar = 120.0;
    double offset = 1.0;
    double aRadians = Angle::DegreesToRadians (20);
    auto lineA = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, ax,0,0));
    auto arcA = ICurvePrimitive::CreateArc (DEllipse3d::From (ax, ar, 0.0,
                        0.0, -ar, 0.0,
                        ar, 0.0, 0.0,
                        0.0, aRadians));
    auto cvA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cvA->push_back (lineA);
    cvA->push_back (arcA);

    double by = 4.0;
    double br = 60.0;
    double bRadians = Angle::DegreesToRadians (30);
    auto lineB = ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 0,by,0));
    auto arcB = ICurvePrimitive::CreateArc (DEllipse3d::From (-br, by, 0.0,
                                                    br, 0,0,
                                                    0,br,0,
                                                    0, bRadians));
    auto cvB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cvB->push_back (lineB);
    cvB->push_back (arcB);
    auto options = IFacetOptions::CreateForCurves ();
    options->SetMaxEdgeLength (1.0);
    auto strokeA = cvA->Stroke (*options);
    auto strokeB = cvB->Stroke (*options);
    if (s_noisy)
        {
        Check::Print (cvA, "X path");
        Check::Print (strokeA, "X strokes");
        Check::Print (cvB, "Y path");
        Check::Print (strokeB, "Y strokes");
        }
    auto pathA = CurveVectorWithDistanceIndex::Create ();
    auto pathB = CurveVectorWithDistanceIndex::Create ();
    pathA->SetPath (cvA);
    pathB->SetPath (cvB);
    double filletRadius = 2.0;
    bvector<double> tapers {1.0, 2.0, 4.0, 8.0, 12.0, 0.0 };
    for (double taper : tapers)
        {
        double e = 0.1 * taper;
        if (s_noisy)
            {
            Check::Print (taper, "taper");
            Check::Print (e, "setback");
            Check::Print (filletRadius, "filletRadius");
            }
        double dA = taper + 0.1 * filletRadius;
        double dB = taper + 0.1 * filletRadius;
        auto blend = CurveCurve::ConstructTaperFilletTaper (*pathA, *pathB, e, taper, filletRadius, e, taper,
                dA, dB, offset, offset);
        if (Check::True (blend.IsValid ()))
            {
            if (s_noisy)
                Check::Print (*blend, "constructed taper");
            auto blendStrokes = blend->Stroke (*options);
            if (s_noisy)
                Check::Print (*blendStrokes, "stroked taper");
            }
        }
    }

 bool ReadAsString (char const *filenameChar, Utf8String &string)
    {

    string.clear ();
    BeFile file;
    Utf8String filename (filenameChar);
    if (BeFileStatus::Success == file.Open (filename, BeFileAccess::Read))
        {
        bvector<Byte> bytes;
        if (BeFileStatus::Success == file.ReadEntireFile (bytes))
            {
            for (auto b : bytes)
                string.push_back (b);
            return true;
            }
        }
    return false;
    }

bool DGNJSFileToGeometry (char const *filename, bvector<IGeometryPtr> &geometry)
    {
    geometry.clear ();
    Utf8String string;
    return ReadAsString (filename, string)
        && BentleyGeometryJson::TryJsonStringToGeometry (string, geometry);
    }

bool ReadAsString (BeFileName &filename, Utf8String &string)
    {

    string.clear ();
    BeFile file;
    if (BeFileStatus::Success == file.Open (filename, BeFileAccess::Read))
        {
        bvector<Byte> bytes;
        if (BeFileStatus::Success == file.ReadEntireFile (bytes))
            {
            for (auto b : bytes)
                string.push_back (b);
            return true;
            }
        }
    return false;
    }

bool DGNJSFileToGeometry (BeFileName &filename, bvector<IGeometryPtr> &geometry)
    {
    geometry.clear ();
    Utf8String string;
    return ReadAsString (filename, string)
        && BentleyGeometryJson::TryJsonStringToGeometry (string, geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined(BENTLEY_WIN32)
TEST(CurveVector,BuildDistanceIndexFromFile)
    {
    Utf8String string;
    if (s_readFile
        && ReadAsString ("d:/tmp/spline1.js", string))
        {
        bvector<IGeometryPtr> allGeometry;
        auto strokeOptions = IFacetOptions::CreateForCurves ();
        if (BentleyGeometryJson::TryJsonStringToGeometry (string, allGeometry))
            {
            for (auto g : allGeometry)
                {
                CurveVectorPtr cv = g->GetAsCurveVector();
                auto strokes = cv->Stroke (*strokeOptions);
                // double a = strokes->Length ();
                // double b = cv->Length ();
                if (cv.IsValid ())
                    {
                    int numPass = 10;
                    TimeAccumulator timer;
                    timer.Reset ();
                    for (int i = 0; i < numPass; i++)
                        {
                        auto path = CurveVectorWithDistanceIndex::Create ();
                        path->SetPath (cv);
                        }
                    timer.AccumulateAndReset ();
                    double t = timer.Sum ();
                    printf (" (SetPath %d %g\n", numPass, t);
                    }
                }
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined(BENTLEY_WIN32)
TEST(CurveVector,CCI)
    {
    Utf8String string1, string2;
    if (s_readFile
        && ReadAsString ("d:/tmp/sandy/spline.js", string1)
        && ReadAsString ("d:/tmp/sandy/secondary.js", string2))
        {
        bvector<IGeometryPtr> allGeometry1, allGeometry2;
        auto strokeOptions = IFacetOptions::CreateForCurves ();
        if (   BentleyGeometryJson::TryJsonStringToGeometry (string1, allGeometry1)
            && BentleyGeometryJson::TryJsonStringToGeometry (string2, allGeometry2)
            )
            {
            for (auto g1 : allGeometry1) for (auto g2: allGeometry2)
                {
                CurveVectorPtr cv1 = g1->GetAsCurveVector();
                CurveVectorPtr cv2 = g2->GetAsCurveVector();
                if (   cv1.IsValid ()
                    && cv2.IsValid ())
                    {
                    int numPass = 10;
                    auto intersection1 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
                    auto intersection2 = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
                    TimeAccumulator timer;
                    timer.Reset ();
                    for (int i = 0; i < numPass; i++)
                        {
                        CurveCurve::IntersectionsXY (*intersection1, *intersection2, *cv1, *cv2, nullptr);
                        }
                    timer.AccumulateAndReset ();
                    double t = timer.Sum ();
                    printf (" (CCI %d %g)\n", numPass, t);
                    }
                }
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined(BENTLEY_WIN32)
TEST(CurveVector,SpiralCuts)
    {
    Utf8String string1, string2;
    if (s_readFile
        && ReadAsString ("d:/dgndb0601dev/src/geomlibs/geom/test/data/CurveVector/03CPartialSpiral/partialSpiral.js", string1)
        )
        {
        bvector<IGeometryPtr> allGeometry1;
        auto strokeOptions = IFacetOptions::CreateForCurves ();
        if (   BentleyGeometryJson::TryJsonStringToGeometry (string1, allGeometry1))
            {
            for (auto g1 : allGeometry1)
                {
                CurveVectorPtr cv1 = g1->GetAsCurveVector();
                if (cv1.IsValid ())
                    {
                    double distanceA = 44.358130211521399;
                    for (auto distanceB : bvector<double>{ 91.277302578162477 , 190.0, 270.0, 300.0})
                        {
                        auto parentPath = CurveVectorWithDistanceIndex::Create ();
                        parentPath->SetPath (cv1);
                        Check::Print (*cv1, "FULL PATH");
                        Check::PrintIndent (1); Check::Print (cv1->Length (), "Full path length");
                        PathLocationDetail detailA, detailB, detailAXY, detailBXY;
                        //auto locationA = parentPath->SearchByDistanceFromPathStart (distanceA, detailA);
                        //auto locationB = parentPath->SearchByDistanceFromPathStart (distanceB, detailB);
                        //auto locationAXY = parentPath->SearchByDistanceFromPathStart (distanceA, detailAXY);
                        //auto locationBXY = parentPath->SearchByDistanceFromPathStart (distanceB, detailBXY);

                        printf ("  Total parentPath length %.17g\n", parentPath->TotalPathLength ());
                        Check::Print ("\ndetailA"); Check::Print (detailA);
                        Check::Print ("\ndetailAXY");Check::Print (detailAXY);
                        Check::Print ("\ndetailB");Check::Print (detailB);
                        Check::Print ("\ndetailBXY");Check::Print (detailBXY);

                        auto cv2 = parentPath->CloneBetweenDistances (distanceA, distanceB);
                        Check::Print (*cv2, "CLIPPED PATH");
                        Check::PrintIndent (1); Check::Print(distanceA, "distanceA");
                        Check::PrintIndent (1); Check::Print(distanceB, "distanceB");

                        Check::PrintIndent (1); Check::Print(distanceB - distanceA, "distanceB - distanceA");
                        Check::PrintIndent (1); Check::Print(cv2->Length (), "clippedPath->TotalPathLength ()");

                        bvector<PathLocationDetailPair> parentIntervals, childIntervals;
                        auto childPath = CurveVectorWithDistanceIndex::Create ();
                        childPath->SetPath (cv2);
                        CurveVectorWithDistanceIndex::FindCommonSubPaths (*parentPath, *childPath, parentIntervals, childIntervals, true, false);
                        for (size_t i = 0; i < parentIntervals.size (); i++)
                            {
                            printf ("\n** %s *******\n", parentIntervals[i].GetTagA () == 0 ? "split" : "shared");
                            Check::PrintIndent (2);
                                Check::Print (parentIntervals[i].DetailA());
                                Check::Print (parentIntervals[i].DetailB());
                            Check::PrintIndent (2);
                                Check::Print (childIntervals[i].DetailA());
                                Check::Print (childIntervals[i].DetailB());

                            }
                        }
                    }
                }
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVectorWithDistanceIndex,SpiralCut0)
    {
    double d1 = 100.0;
    double r1 = 1000.0;
    auto spiral0 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,0.0,
            d1, r1,
            Transform::FromIdentity (),
            0.0, 1.0
            );
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cv->push_back (spiral0);
    auto path = CurveVectorWithDistanceIndex::Create ();
    path->SetPath (cv);
    for (auto f : bvector<double> {0.1, 0.6, 0.8})
        {
        Check::PrintIndent (0);
        Check::Print (f, "fraction along spiral");
        PathLocationDetail detailA;
        double dA = f * d1;
        Check::Print (dA, "distance along spiral");
        if (Check::True (path->SearchByDistanceFromPathStart (dA, detailA)))
            {
            Check::PrintIndent (1);
            Check::Print (detailA);
            DPoint3d pointA = detailA.Point ();
            auto detailB = path->SearchClosestPointBounded (pointA, true);
            Check::PrintIndent (1);
            Check::Print (detailB);
            }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined(BENTLEY_WIN32)
TEST(CurveVector,StrokeAlignment)
    {
    static int s_readFile;
    Utf8String string1, string2;
    IFacetOptionsPtr curveOptions = IFacetOptions::CreateForCurves();
    double angleTol = 0.08;
    curveOptions->SetAngleTolerance(angleTol);
    if (s_readFile)
        {
        if (ReadAsString ("d:/dgndb06devA/src/GeomLibs/geom/test/data/CurveVector/SpiralStroke/spiralButConvertedToBspline.js", string1))
            {
            bvector<IGeometryPtr> allGeometry;
            if (BentleyGeometryJson::TryJsonStringToGeometry (string1, allGeometry))
                {
                for (auto g : allGeometry)
                    {
                    CurveVectorPtr cv = g->GetAsCurveVector();
                    if (cv.IsValid ())
                        {
                        for (size_t i = 0; i + 1 < cv->size (); i++)
                            {
                            auto frameA = cv->at(i)->FractionToFrenetFrame (1.0);
                            auto frameB = cv->at(i)->FractionToFrenetFrame (0.0);
                            DPoint3d pointA, pointB;
                            frameA.Value ().GetTranslation (pointA);
                            frameB.Value ().GetTranslation (pointB);
                            Check::Near (pointA, pointB);
                            }
                        auto index = CurveVectorWithDistanceIndex::Create ();
                        index->SetPath (cv);
                        bvector<DPoint3d> strokes;
                        cv->AddStrokePoints(strokes, *curveOptions);
                        //Check::Print (strokes, "Stroke LineSpiralLine");
                        // confirm that stroking has reasonable angles..
                        UsageSums sum;
                        for (size_t i = 0; i + 2 < strokes.size (); i++)
                            {
                            auto vector0 = strokes[i+1] - strokes[i];
                            auto vector1 = strokes[i+2] - strokes[i+1];
                            double theta = vector0.AngleTo (vector1);
                            sum.Accumulate (theta);
                            }
                        Check::LessThanOrEqual (sum.Max (), angleTol, "spiral stroke range");
                        }
                    }
                }
            }
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,Stroke)
    {
    double y = 1000.0;
    auto line0 = ICurvePrimitive::CreateLine (DSegment3d::From (-10,y,0, 10,y,0));
    Transform frame01, frame12;
    line0->FractionToFrenetFrame (1.0, frame01);
    auto spiral1 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,0.0,
            20.0, 100.0,
            frame01,
            0.0, 1.0
            );
    spiral1->FractionToFrenetFrame (1.0, frame12);
    DPoint3d point2A = frame12 * DPoint3d::From ( 0,0,0);
    DPoint3d point2B = frame12 * DPoint3d::From (10,0,0);
    auto line2 = ICurvePrimitive::CreateLine (DSegment3d::From (point2A, point2B));
    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    cv->push_back (line0);
    cv->push_back (spiral1);
    cv->push_back (line2);
    Utf8String json;
    auto g = IGeometry::Create (cv);
    BentleyGeometryJson::TryGeometryToJsonString (json, *g, true);
    //BentleyGeometryJson::DumpJson (json);

    IFacetOptionsPtr curveOptions = IFacetOptions::CreateForCurves();
    double angleTol = 0.04;
    curveOptions->SetAngleTolerance(angleTol);

    bvector<DPoint3d> strokes;
    cv->AddStrokePoints(strokes, *curveOptions);
    //Check::Print (strokes, "Stroke LineSpiralLine");
    // confirm that stroking has reasonable angles..
    UsageSums angleSum;
    for (size_t i = 0; i + 2 < strokes.size (); i++)
        {
        auto vector0 = strokes[i+1] - strokes[i];
        auto vector1 = strokes[i+2] - strokes[i+1];
        double theta = vector0.AngleTo (vector1);
        angleSum.Accumulate (theta);
        }
    Check::LessThanOrEqual (angleSum.Max (), angleTol, "spiral stroke range");

    UsageSums strokeLengthSum;
    UsageSums chordErrorSum;
    UsageSums xyISum, xy2Sum;
    DMatrix4d matrix2 = 
        DMatrix4d::FromRowValues
            (
            2,0,0,0,
            0,2,0,0,
            0,0,2,0,
            0,0,0,2
            );      // Effectively an identity matrix, but will be treated as nontrivial perspective.

    for (size_t i = 0; i + 2 < strokes.size (); i++)
        {
        DPoint3d midPoint = DPoint3d::FromInterpolate (strokes[i], 0.5, strokes[i+1]);
        CurveLocationDetail curvePoint, curvePointXYI, curvePointXY2;
        cv->ClosestPointBounded (midPoint, curvePoint);
        cv->ClosestPointBoundedXY (midPoint, nullptr, curvePointXYI);
        cv->ClosestPointBoundedXY (midPoint, &matrix2, curvePointXY2);
        xyISum.Accumulate (curvePoint.point.Distance (curvePointXYI.point));
        xy2Sum.Accumulate (curvePoint.point.Distance (curvePointXY2.point));
        chordErrorSum.Accumulate (midPoint.Distance (curvePoint.point));
        strokeLengthSum.Accumulate (strokes[i].Distance (strokes[i+1]));
        }
    Check::LessThanOrEqual (chordErrorSum.Max (),  0.01);
    Check::Near (1.0 + xyISum.Max (),  1.0);
    Check::Near (1.0 + xy2Sum.Max (),  1.0);




    auto index = CurveVectorWithDistanceIndex::Create ();
    index->SetPath (cv);
    double length = index->TotalPathLengthXY ();
    auto cv1 = cv->Clone ();
    double a = 12.0 * 2.54 / 100.0;
    auto transform = Transform::FromRowValues
            (
            a,0,0,1,
            0,a,0,2,
            0,0,a,3
            );
    cv1->TransformInPlace (transform);
    bvector<DPoint3d> stroke1;
    cv1->AddStrokePoints(stroke1, *curveOptions);
    //Check::Print (stroke1, "strokes after transform");

    auto cv2 = cv1->Clone ();
    auto index2 = CurveVectorWithDistanceIndex::Create ();
    index2->SetPath (cv2);
    double length2 = index2->TotalPathLengthXY ();
    Check::Near (length * a, length2);
    //Check::Print (*cv2, "cv2");
    bvector<DPoint3d> stroke2;
    cv2->AddStrokePoints(stroke2, *curveOptions);
    //Check::Print (stroke2, "strokes after transform");

    }

bool TestCCI (ICurvePrimitiveR curve1, ICurvePrimitiveR curve2, double f1, double f2)
    {
    auto curve3 = curve2.Clone ();

    DPoint3d point1, point2, point2B;
    curve1.FractionToPoint (f1, point1);
    curve2.FractionToPoint (f2, point2);
//            Check::Print (point1, "Forced intersection point");
    // move and rotate curve3 so that its point at f2 matches curve1 at f1
    // Create an intersection at point 2, with each fraction as the known result ..
    auto shift = Transform::From (point1 - point2);
    auto rotate = Transform::FromAxisAndRotationAngle (
            DRay3d::FromOriginAndVector (point1, DVec3d::UnitZ()), Angle::DegreesToRadians (90));
    auto product = rotate * shift;
    curve3->TransformInPlace (product);
    point2B = product * point2;

    DPoint3d point3A;
    curve3->FractionToPoint (f2, point3A);
    Check::Near (point1, point3A);


    // We know the two intersect at fractions f1,f2 and at point2.
    auto intersectionsA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    auto intersectionsB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveCurve::IntersectionsXY (
            *intersectionsA, *intersectionsB,
            &curve1, curve3.get (), nullptr, false);
    //Check::Print (*intersectionsA, "IntersectA");
    //Check::Print (*intersectionsB, "IntersectB");
    static double s_fractionTol = 1.0e-7;
    size_t numMatch = 0;
    for (size_t i = 0; i < intersectionsA->size (); i++)
        {
        double fQ1, fQ2;
        DPoint3d pointQ1, pointQ2;
        if (Check::True (CurveCurve::IsSinglePointPair (
                *intersectionsA, *intersectionsB, 0,
                fQ1, pointQ1, fQ2, pointQ2)))
            {
            if (   DoubleOps::AlmostEqual (f1, fQ1, s_fractionTol)
                && DoubleOps::AlmostEqual (f2, fQ2, s_fractionTol))
                numMatch++;
            }
        }
    return numMatch >= 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,Intersection)
    {
    // Spiral 
    auto spiral1 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,0.0,
            20.0, 100.0,
            Transform::FromIdentity (),
            0.0, 1.0
            );

    auto spiral2 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.0,0.0,
            20.0, 100.0,
            Transform::FromIdentity (),
            0.38, 0.70
            );


    auto line = ICurvePrimitive::CreateLine (
            DSegment3d::From (0,0,0, 20,0,0));
    
    auto arc = ICurvePrimitive::CreateArc (
            DEllipse3d::FromStartTangentNormalRadiusSweep (
                DPoint3d::From(0,0,0),
                DVec3d::UnitX (),
                DVec3d::UnitZ (),
                30.0,
                1.0));
    for(double f1 : bvector<double> {0.1, 0.3, 0.9})
        {
        for (double f2 : bvector<double> {0.2, 0.6})
            {
            Check::True(TestCCI (*spiral1, *spiral1, f1, f2), "CCI spiral-spiral");
            Check::True(TestCCI (*spiral1, *line, f1, f2), "CCI spiral-line");
            Check::True(TestCCI (*spiral1, *arc, f1, f2), "CCI spiral-arc");

            Check::True(TestCCI (*spiral2, *spiral2, f1, f2), "CCI spiral-spiral");
            Check::True(TestCCI (*spiral2, *line, f1, f2), "CCI spiral-line");
            Check::True(TestCCI (*spiral2, *arc, f1, f2), "CCI spiral-arc");

            }
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,PartialSpiralMappings)
    {
    // Spiral 
    auto spiral1 = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.24,1000.0,
            20.0, 100.0,
            Transform::FromIdentity (),
            0.0, 1.0
            );

    DSegment1d interval (0.25, 0.5);
    auto spiral2 = spiral1->CloneBetweenFractions (interval.GetStart (), interval.GetEnd (), false);
    for (double f2: bvector<double>{0.0, 0.40, 0.80, 1.0})
        {
        DPoint3d xyz1, xyz2;
        double f1 = interval.FractionToPoint (f2);
        spiral1->FractionToPoint (f1, xyz1);
        spiral2->FractionToPoint (f2, xyz2);
        Check::Near (xyz1, xyz2);
        }

    double length2;
    spiral2->Length (length2);
    double a = 0.25 * length2;
    double f2 = 0.5;
    double f1 = interval.FractionToPoint (f2);
    for (double b: bvector<double>{a, -a})
        {
        CurveLocationDetail detail1, detail2;
        spiral1->PointAtSignedDistanceFromFraction (f1, b, false, detail1);
        spiral2->PointAtSignedDistanceFromFraction (f2, b, false, detail2);
        Check::Near (detail1.point, detail2.point);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(VariableRadiusOffset,Arc)
    {
    double r0 = 10.0;
    auto masterArc = ICurvePrimitive::CreateArc
            (
            DEllipse3d::From 
                (
                0,0,0,
                r0,0,0,
                0,r0,0,
                0.0, Angle::DegreesToRadians (90)
                )
            );
    auto numPointsAlong = 10;

    double distance0 = 1.0;
    double distance1 = 2.0;
    double endToleranceScale = 1000.0;
//    double interiorToleranceScale = 1.0e8;
    size_t numTest = 45;
    auto bcurveA = MSBsplineCurve::CreateFromInterpolationAtGrevilleKnots(*masterArc, numPointsAlong, 4, true);
    //bcurveA->NormalizeKnots ();
    auto primitiveA = ICurvePrimitive::CreateBsplineCurve (bcurveA);
    Check::Print (*primitiveA);
    if (Check::True (primitiveA.IsValid ()))  
        {
        CurveOffsetOptions options (0.0);    // That’s an offset distance arg, but you will supply distinct start and end distances later .
        options.SetBCurveMethod (0);
        options.SetBCurvePointsPerKnot(1);
       
        auto offsetAsBCurve = bcurveA->CreateCopyOffsetXY(distance0, distance1, options);
        if (Check::True (offsetAsBCurve.IsValid ()))
            {
            auto primitiveB = ICurvePrimitive::CreateBsplineCurve(offsetAsBCurve);
            if (Check::True (primitiveB.IsValid ()))
                {
                Check::Print (*primitiveB);
                double arcLength0, arcLength1;
                Check::True (primitiveA->Length (arcLength0));
                Check::True (primitiveB->Length (arcLength1));
                DPoint3d startA, endA, startB, endB;
                primitiveA->GetStartEnd (startA, endA);
                primitiveB->GetStartEnd (startB, endB);
                // At start and end we expect really good match..
                double d0 = startA.Distance (startB);
                double d1 = endB.Distance (endA);
                Check::Near (d0, distance0, "start distance", endToleranceScale);
                Check::Near (d1, distance1, "end distance", endToleranceScale);
                UsageSums interiorErrors;
                UsageSums interiorFractionDifferences;
                UsageSums primaryRadius;
                for (size_t i = 1; i < numTest; i++)
                    {
                    double f = i / (double)numTest;
                    DPoint3d xyzA, xyzB;
                    primitiveA->FractionToPoint (f, xyzA);
                    double fB;
                    primaryRadius.Accumulate (xyzA.Magnitude () - r0);
                    primitiveB->ClosestPointBounded (xyzA, fB, xyzB);
                    interiorFractionDifferences.Accumulate (f - fB);
                    double distance3 = DoubleOps::Interpolate (distance0, f, distance1);
                    double distance4 = xyzA.Distance (xyzB);
                    interiorErrors.Accumulate (distance3-distance4);
                    }
                double e = interiorErrors.MaxAbs ();
                // we have really pathetic expectations of offset distances ..
                Check::LessThanOrEqual (e, 0.01, "interior distance error");
                PrintUsageSums (interiorErrors, "Interior Error Range");
                PrintUsageSums (interiorFractionDifferences, "Interior Fraction DIfference Range");
                PrintUsageSums (primaryRadius, "primary bspline error");
                }
            }
        }
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CloneOffset, AcuteLinestrings)
    {
    bvector<double> signs;
    signs.push_back (-1.0);
    signs.push_back (1.0);

    double displayShift = 12000.0;
    for (double ySign : signs)
        {
        auto baseTransform = Check::GetTransform ();
        bvector<DPoint3d> points;
        points.push_back(DPoint3d::From(0, 0, 0));
        points.push_back(DPoint3d::From(10000, 0, 0));
        points.push_back(DPoint3d::From(0, ySign * 10000, 0));
        Check::SaveTransformed (points);
        bvector<DPoint3d> pointB;
        double distance = 2000.0;
        PolylineOps::OffsetLineStringXY (pointB, points, distance, false, Angle::FromDegrees (91).Radians ());
        Check::SaveTransformed (pointB);
        Check::Shift (0, 2.0 * displayShift, 0);

        CurveVectorPtr  cv0 = CurveVector::CreateLinear(points);
        for (double chamferRadians = 0.0; chamferRadians < 5.0; chamferRadians += 3.1)
            {
            CurveOffsetOptions  options(distance);
            if (chamferRadians > 0.0)
              options.SetChamferAngle (chamferRadians);
            CurveVectorPtr      cv1 = cv0->CloneOffsetCurvesXY(options);
            Check::SaveTransformed (*cv0);
            Check::SaveTransformed (*cv1);
            Check::Print (cv0, "base linestring");
            Check::Print (cv1, "offset linestring");
            Check::Shift (0, displayShift, 0);
            }
        Check::SetTransform (baseTransform);
        Check::Shift (displayShift, 0,0);
        }
    Check::ClearGeometry ("CloneOffset.AcuteLinestring");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CloneOffset, AcuteCornerRemoval)
    {
    bvector<double> distances;
    distances.push_back (-2.0);
    distances.push_back (-1.0);
    distances.push_back (-0.5);
    distances.push_back (-0.25);
    distances.push_back (0.25);
    distances.push_back (0.5);
    distances.push_back ( 1.0);
    distances.push_back ( 2.0);
    bvector<DPoint3d> points;
    points.push_back(DPoint3d::From(0, 0, 0));
    points.push_back(DPoint3d::From(1, 0, 0));
    points.push_back(DPoint3d::From(6, 0, 0));
    points.push_back(DPoint3d::From(3, 2.5, 0));
    points.push_back(DPoint3d::From(3, 10, 0));
    points.push_back (DPoint3d::From (10, 10, 0));
    points.push_back (DPoint3d::From (10, 1, 0));
    points.push_back (DPoint3d::From (40, 1, 0));
    points.push_back (DPoint3d::From (20, 10, 0));
    points.push_back (DPoint3d::From (100, 10, 0));
    points.push_back (DPoint3d::From (80, 1, 0));
    points.push_back (DPoint3d::From (120,1,0));
    Check::SaveTransformed (points);
    Check::Print (points, "Points for acute corner removal");
    for (double d : distances)
        {
        bvector<DPoint3d> pointB;
        PolylineOps::OffsetLineStringXY (pointB, points, d, false, Angle::FromDegrees (91).Radians ());
        Check::Print (d, "offset distance");
        Check::Print (pointB, "offset points");
        Check::SaveTransformed (pointB);
        }
    Check::ClearGeometry ("CloneOffset.AcuteCornerRemoval");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined(BENTLEY_WIN32)
TEST(CurveVector,OffsetLineArcJaggies)
    {
    Utf8String string;
    if (s_readFile
        && ReadAsString ("d:/bim0200dev/src/geomlibs/geom/test/data/CurveVector/1701CurveOffsetZigZagInput/Centerline.dgnjs", string))
        {
        double offsetDistanceA = -5.7;
        double offsetDistanceB = -0.5;
        bvector<IGeometryPtr> allGeometry;
        auto strokeOptions = IFacetOptions::CreateForCurves ();
        if (BentleyGeometryJson::TryJsonStringToGeometry (string, allGeometry))
            {
            for (auto g : allGeometry)
                {
                CurveVectorPtr cv = g->GetAsCurveVector();
                Check::SaveTransformed (*cv);
                auto strokes = cv->Stroke (*strokeOptions);
                double a = strokes->Length ();
                double b = cv->Length ();
                Check::True (fabs (a-b) < 1.0e-4 * a);
                CurveOffsetOptions options (offsetDistanceA);
                auto offsetA = cv->CloneOffsetCurvesXY (options);
                Check::SaveTransformed (*offsetA);


                options.SetOffsetDistance (offsetDistanceB);
                auto offsetB = offsetA->CloneOffsetCurvesXY (options);
                Check::SaveTransformed (*offsetB);
                }
            }
        }
    Check::ClearGeometry ("CurveVector.OffsetLineArcJaggies");
    }
#endif
	
void Test_ConstructArcs_PointTangentCurveTangent (DPoint3dCR pointA, DVec3dCR tangentA, ICurvePrimitiveCR primitive)
    {
    bvector<CurveCurve::FilletDetail> arcs;
    CurveCurve::ConstructArcs_PointTangentCurveTangent (arcs, pointA, tangentA, primitive);
    Check::SaveTransformed (primitive);
    auto line = ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointA + 4.0 * tangentA));
    Check::SaveTransformed (*line);
    for (auto &detail: arcs)
        {
        Check::Print (detail.arc, "TangentArc");
        auto arcPrim = ICurvePrimitive::CreateArc (detail.arc);
        Check::SaveTransformed (*arcPrim);
        DRay3d ray0 = arcPrim->FractionToPointAndUnitTangent (0.0);
        DRay3d ray1 = arcPrim->FractionToPointAndUnitTangent (1.0);
        DRay3d ray2 = primitive.FractionToPointAndUnitTangent (detail.detailB.fraction);
        Check::Near (pointA, ray0.origin, "Tangent arc starts at pointA");
        Check::Near (tangentA, ray0.direction, "Tangent arc start direction");
        Check::Parallel (ray0.direction, tangentA, "Tangent arc start direction");
        Check::Parallel (ray1.direction, ray2.direction, "Tangent arc end direction");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurvePrimitive,ConstructArcs_PointTangentCurveTangent)
    {
    DPoint3d pointC = DPoint3d::From (6,5,0);
    double   radiusC = 3.0;
    DPoint3d pointA = DPoint3d::From (1,-1,0);
    DVec3d   tangentA = DVec3d::From (0,1,0);
    DEllipse3d ellipse0 = DEllipse3d::FromCenterRadiusXY (pointC, radiusC);
    auto baseTransform = Check::GetTransform ();
    for (auto axisFactor : bvector<double> {1.0, 0.9, 1.1, 0.5})
        {
        auto ellipse = ellipse0;
        ellipse.vector0.Scale (axisFactor);
        auto curve = ICurvePrimitive::CreateArc (ellipse);
        Test_ConstructArcs_PointTangentCurveTangent (pointA, tangentA, *curve);
        Check::Shift (10.0, 0,0);
        }

    auto bcurve = MSBsplineCurve::CreateFromPolesAndOrder
        (
        bvector <DPoint3d> {
            DPoint3d::From (0,5,0),
            DPoint3d::From (2,5,0),
            DPoint3d::From (5,4,0),
            DPoint3d::From (3,10,0),
            DPoint3d::From (0,7,0)
            },
        nullptr, nullptr,
        4,
        true,       // Closed !!! not a common thing to do ..
        true);
    auto bcurvePrimitive = ICurvePrimitive::CreateBsplineCurve (bcurve);
    Check::SetTransform (baseTransform);
    Check::Shift (0,20,0);

    Test_ConstructArcs_PointTangentCurveTangent (pointA, tangentA, *bcurvePrimitive);

    Check::ClearGeometry ("CurvePrimitive.ConstructArcs_PointTangentCurveTangent");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(CurveVector,ConstructArcs_PointTangentCurveTangent_Sandy)
    {
    char const * cvJson =
        "{\"DgnCurveVector\":"
        "{\"Member\":["
        "{\"LineSegment\":{\"endPoint\":[302202.34846006555,255412.64833109025,0.0],\"startPoint\":[302312.87741261825,255330.46328081773,0.0]}},"
        "{\"CircularArc\":{\"placement\":{\"origin\":[302145.36392289313,255336.01102072198,0.0],\"vectorX\":[-0.13772806853932984,0.99047007987946101,0.0],\"vectorZ\":[0.0,0.0,-1.0]},\"radius\":95.501386467645830,\"startAngle\":44.549464581921455,\"sweepAngle\":-44.549464581921455}},"
        "{\"LineSegment\":{\"endPoint\":[302110.71335630986,255427.61301123278,0.0],\"startPoint\":[302132.21070139209,255430.60228660519,0.0]}}],"
        "\"boundaryType\":1}"
        "}";
    bvector<IGeometryPtr> cvGeometry;
    BentleyGeometryJson::TryJsonStringToGeometry (cvJson, cvGeometry);
    Check::SaveTransformed (cvGeometry);

    auto cv = cvGeometry[0]->GetAsCurveVector ();
    int numShift = 20;
    double totalLengthShiftFraction = 1.5;
    if (Check::True (cv.IsValid (), "Valid CurveVector from Json data"))
        {
        double a = cv->FastLength ();
        auto pointA = DPoint3d::From (302312.39839566144, 255343.65445345294, 0);
        auto tangentA = DVec3d::From (-0.10008298498874627, -0.99497909330585543, 0);
        auto shiftVector = DVec3d::FromXYAngleAndMagnitude (Angle::DegreesToRadians (120.0), totalLengthShiftFraction * a / numShift);
        for (size_t i = 0; i < numShift; i++)
            {
            DPoint3d pointB = pointA + (double)i * shiftVector;
            Check::SaveTransformed (
                bvector<DPoint3d> {pointB, pointB + tangentA * 0.25 * shiftVector.Magnitude ()});
            bvector<CurveCurve::FilletDetail> arcs;
            CurveCurve::ConstructArcs_PointTangentCurveTangent (arcs, pointB, tangentA, *cv);
                for (auto detail : arcs)
                    Check::SaveTransformed (detail.arc);
            }
        }
    Check::ClearGeometry ("CurveVector.ConstructArcs_PointTangentCurveTangent_Sandy");
    }

CurveVectorPtr createLinStringSandyB (int compress)
    {
    bvector<DPoint3d> pointA {
        DPoint3d::From (426516.5253791220,4502207.7760169217,1307.8360079348492),
        DPoint3d::From (426516.52414777537,4502207.4768121997,1307.8360480111091),
        DPoint3d::From (426516.53759919188,4502207.1779074622,1307.8361682398890),
        DPoint3d::From (426516.56570096582,4502206.8800227977,1307.8363686211890),
        DPoint3d::From (426516.60838539765,4502206.5838758387,1307.8366491550089),
        DPoint3d::From (426516.66554965673,4502206.2901800266,1307.8370098413486),
        DPoint3d::From (426516.73705602944,4502205.9996429021,1307.8374506802083),
        DPoint3d::From (426516.82273225061,4502205.7129643932,1307.8379716715881),
        DPoint3d::From (426516.92237191874,4502205.4308351343,1307.8385728154876),
        DPoint3d::From (426517.03573499317,4502205.1539347982,1307.8392541119072),
        // 10
        DPoint3d::From (426517.16254837223,4502204.8829304632,1307.8400155608467),
        DPoint3d::From (426517.30250655144,4502204.6184750041,1307.8408571623063),
        DPoint3d::From (426517.45527235937,4502204.3612055136,1307.8417789162854),
        DPoint3d::From (426517.62047776970,4502204.1117417794,1307.8427808227848),
        DPoint3d::From (426517.79772478808,4502203.8706847802,1307.8438628818042),
        DPoint3d::From (426517.98658641113,4502203.6386152441,1307.8450250933436),
        DPoint3d::From (426518.18660765438,4502203.4160922458,1307.8462674574025),
        DPoint3d::From (426518.39730664942,4502203.2036518641,1307.8475899739819),
        DPoint3d::From (426518.61817580368,4502203.0018058857,1307.8489926430809),
        DPoint3d::From (426518.84868302412,4502202.8110405747,1307.8504754646999),
        // 20
        DPoint3d::From (426519.08827299863,4502202.6318155024,1307.8520384388389),
        DPoint3d::From (426519.33636853407,4502202.4645624375,1307.8536815654979),
        DPoint3d::From (426519.59237194643,4502202.3096843073,1307.8554048446767),
        DPoint3d::From (426519.85566650121,4502202.1675542267,1307.8572082763756),
        DPoint3d::From (426520.12561789853,4502202.0385145992,1307.8590918605944),
        DPoint3d::From (426520.40157580207,4502201.9228762928,1307.8610555973330),
        DPoint3d::From (426520.68287540501,4502201.8209178904,1307.8630994865916),
        DPoint3d::From (426520.96883903188,4502201.7328850189,1307.8652235283703),
        DPoint3d::From (426521.25877777138,4502201.6589897564,1307.8674277226689),
        DPoint3d::From (426521.55199313560,4502201.5994101251,1307.8697120694876),
        // 30
        DPoint3d::From (426521.84777874302,4502201.5542896548,1307.8720765688261),
        DPoint3d::From (426522.14542202029,4502201.5237370459,1307.8745212206843),
        DPoint3d::From (426522.44420591864,4502201.5078259027,1307.8770460250628),
        DPoint3d::From (426522.44420591864,4502201.5078259027,1307.8770460250628),
        DPoint3d::From (426522.44420591864,4502201.5078259027,1307.8770460250628),
        DPoint3d::From (426522.74341064150,4502201.5065945564,1307.8795708294408),
        DPoint3d::From (426523.04231537844,4502201.5200459724,1307.8820154812986),
        DPoint3d::From (426523.34020004189,4502201.5481477464,1307.8843799806370),
        DPoint3d::From (426523.63634700142,4502201.5908321785,1307.8866643274550),
        DPoint3d::From (426523.93004281324,4502201.6479964377,1307.8888685217532),
        // 40
        DPoint3d::From (426524.22057993832,4502201.7195028104,1307.8909925635317),
        DPoint3d::From (426524.50725844729,4502201.8051790316,1307.8930364527901),
        DPoint3d::From (426524.78938770655,4502201.9048186997,1307.8950001895284),
        DPoint3d::From (426525.0662880420,4502202.0181817738,1307.8968837737468),
        DPoint3d::From (426525.33729237638,4502202.1449951539,1307.8986872054454),
        DPoint3d::From (426525.60174783657,4502202.2849533325,1307.9004104846240),
        DPoint3d::From (426525.85901732626,4502202.4377191402,1307.9020536112826),
        DPoint3d::From (426526.10848106071,4502202.602924550,1307.9036165854213),
        DPoint3d::From (426526.34953805985,4502202.7801715694,1307.9050994070399),
        DPoint3d::From (426526.58160759631,4502202.9690331919,1307.9065020761386),
        // 50
        DPoint3d::From (426526.80413059425,4502203.1690544356,1307.9078245927176),
        DPoint3d::From (426527.01657097624,4502203.3797534304,1307.9090669567763),
        DPoint3d::From (426527.21841695486,4502203.6006225850,1307.9102291683155),
        DPoint3d::From (426527.40918226534,4502203.8311298052,1307.9113112273342),
        DPoint3d::From (426527.58840733755,4502204.0707197795,1307.9123131338333),
        DPoint3d::From (426527.75566040236,4502204.3188153151,1307.9132348878127),
        DPoint3d::From (426527.91053853265,4502204.5748187276,1307.9140764892718),
        DPoint3d::From (426528.05266861338,4502204.8381132828,1307.9148379382109),
        DPoint3d::From (426528.18170824071,4502205.1080646794,1307.9155192346304),
        DPoint3d::From (426528.29734654695,4502205.3840225833,1307.9161203785293),
        // 60
        DPoint3d::From (426528.39930494915,4502205.6653221864,1307.9166413699090),
        DPoint3d::From (426528.48733782081,4502205.9512858130,1307.9170822087683),
        DPoint3d::From (426528.56123308284,4502206.2412245525,1307.9174428951080),
        DPoint3d::From (426528.62081271515,4502206.5344399167,1307.9177234289277),
        DPoint3d::From (426528.66593318520,4502206.8302255236,1307.9179238102270),
        DPoint3d::From (426528.69648579386,4502207.1278688014,1307.9180440390066),
        DPoint3d::From (426528.71239693707,4502207.4266526997,1307.9180841152665),
        DPoint3d::From (426528.71239693707,4502207.4266526997,1307.9180841152665),
        DPoint3d::From (426528.71239693696,4502207.4266526997,1307.9180841152665),
        DPoint3d::From (426528.71342346567,4502207.7355080638,1307.9180414117384),
        // 70
        DPoint3d::From (426528.69880540937,4502208.0440190090,1307.9179133011548),
        DPoint3d::From (426528.66858029232,4502208.3513935879,1307.9176997835154),
        DPoint3d::From (426528.62282570201,4502208.6568427784,1307.9174008588207),
        DPoint3d::From (426528.56165908981,4502208.9595824936,1307.9170165270702),
        DPoint3d::From (426528.48523746949,4502209.2588356072,1307.9165467882644),
        DPoint3d::From (426528.39375701430,4502209.5538339401,1307.9159916424030),
        DPoint3d::From (426528.28745255328,4502209.8438202366,1307.9153510894857),
        DPoint3d::From (426528.16659696848,4502210.1280501038,1307.9146251295133),
        DPoint3d::From (426528.03150049428,4502210.4057939295,1307.9138137624850),
        DPoint3d::From (426527.88250992162,4502210.6763387490,1307.9129169884011),
        // 80
        DPoint3d::From (426527.72000770696,4502210.9389900770,1307.9119348072616),
        DPoint3d::From (426527.54441099131,4502211.1930736927,1307.9108672190666),
        DPoint3d::From (426527.35617052880,4502211.4379373658,1307.9097142238161),
        DPoint3d::From (426527.15576953016,4502211.6729525346,1307.9084758215099),
        DPoint3d::From (426526.94372242171,4502211.897515920,1307.9071520121481),
        DPoint3d::From (426526.72057352547,4502212.1110510696,1307.9057427957312),
        DPoint3d::From (426526.48689566128,4502212.3130098423,1307.9042481722579),
        DPoint3d::From (426526.24328867707,4502212.5028738128,1307.9026681417297),
        DPoint3d::From (426525.99037790828,4502212.6801556023,1307.9010027041456),
        DPoint3d::From (426525.72881257342,4502212.8444001330,1307.8992518595057),
        // 90
        DPoint3d::From (426525.45926410682,4502212.9951857897,1307.8974156078109),
        DPoint3d::From (426525.18242443557,4502213.1321255071,1307.8954939490598),
        DPoint3d::From (426524.89900420315,4502213.2548677651,1307.8934868832537),
        DPoint3d::From (426524.60973094514,4502213.3630974852,1307.8913944103917),
        DPoint3d::From (426524.31534722203,4502213.4565368444,1307.8892165304744),
        DPoint3d::From (426524.01660871244,4502213.5349459816,1307.8869532435012),
        DPoint3d::From (426523.71428227390,4502213.5981236259,1307.8846045494724),
        DPoint3d::From (426523.40914397402,4502213.6459075985,1307.8821704483883),
        DPoint3d::From (426523.10197709856,4502213.6781752389,1307.8796509402484),
        DPoint3d::From (426522.79357014032,4502213.6948437179,1307.8770460250528),
        DPoint3d::From (426522.79357014032,4502213.6948437179,1307.8770460250528),
        DPoint3d::From (426522.79357014032,4502213.6948437179,1307.8770460250528),
        DPoint3d::From (426522.48471477564,4502213.6958702467,1307.8744411098580),
        DPoint3d::From (426522.17620383174,4502213.6812521899,1307.8719216017184),
        DPoint3d::From (426521.86882925185,4502213.6510270732,1307.8694875006347),
        DPoint3d::From (426521.56338006206,4502213.6052724831,1307.8671388066064),
        DPoint3d::From (426521.26064034615,4502213.5441058706,1307.8648755196334),
        DPoint3d::From (426520.96138723247,4502213.4676842513,1307.8626976397165),
        DPoint3d::From (426520.66638889961,4502213.3762037959,1307.8606051668548),
        DPoint3d::From (426520.37640260404,4502213.2698993348,1307.8585981010490),
        DPoint3d::From (426520.09217273636,4502213.1490437491,1307.8566764422983),
        DPoint3d::From (426519.81442891061,4502213.0139472755,1307.8548401906035),
        DPoint3d::From (426519.54388409120,4502212.8649567030,1307.8530893459645),
        DPoint3d::From (426519.28123276279,4502212.7024544878,1307.8514239083804),
        DPoint3d::From (426519.02714914759,4502212.5268577719,1307.8498438778522),
        DPoint3d::From (426518.78228547459,4502212.3386173099,1307.8483492543796),
        DPoint3d::From (426518.54727030528,4502212.1382163120,1307.8469400379629),
        DPoint3d::From (426518.32270692033,4502211.9261692027,1307.8456162286016),
        DPoint3d::From (426518.10917177063,4502211.7030203063,1307.8443778262956),
        DPoint3d::From (426517.90721299784,4502211.4693424432,1307.8432248310451),
        DPoint3d::From (426517.71734902734,4502211.2257354585,1307.8421572428506),
        DPoint3d::From (426517.54006723716,4502210.9728246890,1307.8411750617115),
        DPoint3d::From (426517.37582270714,4502210.7112593548,1307.8402782876281),
        DPoint3d::From (426517.22503705055,4502210.4417108875,1307.83946692060),
        DPoint3d::From (426517.08809733205,4502210.1648712168,1307.8387409606275),
        DPoint3d::From (426516.96535507397,4502209.8814509846,1307.8381004077107),
        DPoint3d::From (426516.85712535388,4502209.5921777263,1307.8375452618493),
        DPoint3d::From (426516.7636859960,4502209.2977940030,1307.8370755230437),
        DPoint3d::From (426516.68527685769,4502208.9990554946,1307.8366911912938),
        DPoint3d::From (426516.62209921423,4502208.6967290556,1307.8363922665994),
        DPoint3d::From (426516.57431524160,4502208.3915907554,1307.8361787489603),
        DPoint3d::From (426516.54204760067,4502208.0844238801,1307.8360506383769),
        DPoint3d::From (426516.52537912189,4502207.7760169217,1307.8360079348492),
        DPoint3d::From (426516.52537912189,4502207.7760169217,1307.8360079348492)
        };

    if (compress == 1)
        {
        bvector<DPoint3d> pointB;
        DPoint3dOps::Compress (pointA, pointB, 0.001);
        return CurveVector::CreateLinear (pointB);
        }
    else if (compress == 2)
        {
        auto cvA = CurveVector::CreateLinear (pointA);
        cvA->SimplifyLinestrings (0.001, true, false);
        return cvA;
        }
    else
        {
        return CurveVector::CreateLinear (pointA);
        }
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(OffsetLinestring,SandyB)
    {
    for (auto compress : bvector<int> {0, 1, 2})
        {
        SaveAndRestoreCheckTransform shifter (20,0,0);
        auto pathA = createLinStringSandyB (compress);
        Check::SaveTransformed (*pathA);
        for (auto offsetDistance : bvector<double> {1, -1, -2.4162400000000002})
            {
            CurveOffsetOptions options (offsetDistance);
            options.SetArcAngle (0.0);
            auto pathB = pathA->CloneOffsetCurvesXY(options);
            Check::SaveTransformed (*pathB);

            }
        }
    Check::ClearGeometry ("OffsetLineString.SandyB");
    }

struct MatrixWeightedBezier2d
{
bvector<DVec3d> m_cp;
bvector<RotMatrix> m_matrix;
double m_mu;
double m_w;
MatrixWeightedBezier2d (double w = 1.0, double mu = 1.0) : m_w(w), m_mu(mu){}
void AddCP (double x, double y, double tangentRadians)
    {
    double c = cos (tangentRadians);
    double s = sin (tangentRadians);
    double nx = -s;
    double ny = c;
    m_cp.push_back(DVec3d::From (x,y,0));

    m_matrix.push_back (RotMatrix::FromRowValues(
        m_w * (1.0 + m_mu * nx * nx), m_w * m_mu * nx * ny, 0,
        m_w * m_mu * nx * ny, m_w * (1.0 + m_mu * ny * ny), 0,
        0,                0,                        1)); 
    }

void AppendBezcoffs2d (bvector<double> &coffs, DVec3dCR xy, RotMatrixCR matrix)
    {
        DVec3d Q = matrix * xy;
        coffs.push_back (Q.x);
        coffs.push_back (Q.y);
        coffs.push_back (matrix.form3d[0][0]);
        coffs.push_back (matrix.form3d[0][1]);
        coffs.push_back (matrix.form3d[1][0]);
        coffs.push_back (matrix.form3d[1][1]);
    }
void FormBezcoffs2d (bvector<double> &bezcoffs)
    {
    bezcoffs.clear ();
    size_t order = m_cp.size ();
    for (size_t i = 0; i < order; i++)
        {
        AppendBezcoffs2d (bezcoffs, m_cp[i], m_matrix[i]);
        }
    }
    // M = w * (I + mu P)   where P should be orthogonal with sum of diagaonl squares (x,y only!!) equal 1.
    // But M is a linear interpoland.
    // experiments show the diagonal condition is maintained anyway !!!!
RotMatrix RescaleMatrix (RotMatrixCR M)
    {
    double a = 1.0 / (m_mu * m_w);
    double b = -1.0 / m_mu;
    RotMatrix I = RotMatrix::FromIdentity ();
    RotMatrix P;
    for (int i = 0; i <3; i++)
        for (int j = 0; j < 3; j++)
            {
            P.form3d[i][j] = a * M.form3d[i][j] + b * I.form3d[i][j];
            }
    double s = P.form3d[0][0] + P.form3d[1][1];
    double mu1 = m_mu / s;
    RotMatrix M1 = M;
    for (int i = 0; i < 2; i++)
        {
        for (int j = 0; j < 2; j++)
            {
            M1.form3d[i][j] = m_w * (I.form3d[i][j] + mu1 * P.form3d[i][j]);
            }
        }
    return M1;
    }
void EvaluateBezcoffs2d (bvector<double> &bezcoffs, int n, bvector<DPoint3d> &points, bool rescale = false)
    {
    if (n < 2)
        n = 2;
    double bezval[6];
    int order = (int)bezcoffs.size() / 6;
    for (int i = 0; i <= n; i++)
        {
        double f = (double)i / (double) n;
        bsiBezier_evaluate (bezval, &bezcoffs[0], order, 6, f);
        DVec3d Q = DVec3d::From (bezval[0], bezval[1], 0.0);
        RotMatrix M = RotMatrix::FromRowValues (
            bezval[2], bezval[3], 0,
            bezval[4], bezval[5], 0,
            0,         0,         1
            );
        if (rescale)
            M = RescaleMatrix (M);
        DVec3d P;
        if (M.Solve (P, Q))
            points.push_back (P);
        }
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MatrixWeightedBezier,HelloWorld)
    {
    for (int order = 2; order < 5; order++)
        {
        SaveAndRestoreCheckTransform shifter (5,0,0);
        for (double alpha = 0.25; alpha < 1.1; alpha *= 2.0)
            {
            SaveAndRestoreCheckTransform shifter (0.1,0.1,0);
            MatrixWeightedBezier2d bezier;
            double degreeStep = alpha * 30.0 / (double)(order - 1.0);
            bvector<DPoint3d> circlePoints;
            for (int i = 0; i < order; i++)
                {
                double degrees = i * degreeStep;
                double radians = Angle::DegreesToRadians (degrees);
                double tangentRadians = Angle::DegreesToRadians (degrees + 90.0);
                bezier.AddCP (cos(radians), sin(radians), tangentRadians);
                circlePoints.push_back (DPoint3d::From (cos(radians), sin(radians),0));
                }
            bvector<double> coffs;
            bezier.FormBezcoffs2d (coffs);
            bvector<DPoint3d> points;
            bezier.EvaluateBezcoffs2d (coffs, 4 * order, points);
            double eMax = 0.0;
            for (auto &xyz : points)
                eMax = DoubleOps::Max (eMax, 1.0 - xyz.MagnitudeXY ());
            GEOMAPI_PRINTF ("(order %d) (degreeSweep %g) (eMax %.2g)\n", order, degreeStep, eMax);
            Check::SaveTransformed (points);
            Check::SaveTransformed (circlePoints);
            Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0,0,0), 1.0));
            }
        }
    Check::ClearGeometry ("MatrixWeightedBezier.HelloWorld");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MatrixWeightedBezier,RadiusEffects)
    {
    int order = 2;
    for (double radius : bvector<double> { 0.5, 1.0, 1.5, 2.0, 3.0})
        {
        SaveAndRestoreCheckTransform shifter (5,0,0);
        for (double alpha = 0.25; alpha < 1.1; alpha *= 2.0)
            {
            SaveAndRestoreCheckTransform shifter (0.1,0.1,0);
            MatrixWeightedBezier2d bezier;
            double degreeStep = alpha * 30.0 / (double)(order - 1.0);
            bvector<DPoint3d> circlePoints;
            for (int i = 0; i < order; i++)
                {
                double degrees = i * degreeStep;
                double radians = Angle::DegreesToRadians (degrees);
                double tangentRadians = Angle::DegreesToRadians (degrees + 90.0);
                bezier.AddCP (radius * cos(radians), radius * sin(radians), tangentRadians);
                circlePoints.push_back (DPoint3d::From (radius * cos(radians), sin(radius * radians),0));
                }
            bvector<double> coffs;
            bezier.FormBezcoffs2d (coffs);
            bvector<DPoint3d> points;
            bezier.EvaluateBezcoffs2d (coffs, 4 * order, points);
            double eMax = 0.0;
            for (auto &xyz : points)
                eMax = DoubleOps::Max (eMax, 1.0 - xyz.MagnitudeXY ());
            GEOMAPI_PRINTF ("(radius %g) (order %d) (degreeSweep %g) (eMax %.2g)\n", radius, order, degreeStep, eMax);
            Check::SaveTransformed (points);
            Check::SaveTransformedMarkers (circlePoints, 0.05);
            Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0,0,0), radius));
            }
        }
    Check::ClearGeometry ("MatrixWeightedBezier.RadiusEffects");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MatrixWeightedBezier,EllipseSections)
    {
    int order = 2;
    double radiusA = 1.0;
    for (double radiusB : bvector<double> { 0.5, 1.0, 1.5, 2.0, 3.0, 5.0})
        {
        SaveAndRestoreCheckTransform shifter (5,0,0);
        auto arc = DEllipse3d::From (
                0,0,0,
                radiusA, 0, 0,
                0, radiusB, 0,
                0.0, Angle::DegreesToRadians (90.0)
                );
        for (double alpha = 0.25; alpha < 1.1; alpha *= 2.0)
            {
            SaveAndRestoreCheckTransform shifter (0.1,0.1,0);
            MatrixWeightedBezier2d bezier;
            double degreeStep = alpha * 30.0 / (double)(order - 1.0);
            bvector<DPoint3d> circlePoints;
            for (int i = 0; i < order; i++)
                {
                double degrees = i * degreeStep;
                double radians = Angle::DegreesToRadians (degrees);
                DVec3d dX, ddX;
                DPoint3d X;
                arc.Evaluate (X, dX, ddX, radians);
                bezier.AddCP (X.x, X.y, atan2 (dX.y, dX.x));
                circlePoints.push_back (X);
                }
            bvector<double> coffs;
            bezier.FormBezcoffs2d (coffs);
            bvector<DPoint3d> points;
            bezier.EvaluateBezcoffs2d (coffs, 4 * order, points);
            GEOMAPI_PRINTF ("(radiusA %g) (radiusB %g) (order %d) (degreeSweep %g) \n", radiusA, radiusB, order, degreeStep);
            Check::SaveTransformed (points);
            Check::SaveTransformedMarkers (circlePoints, 0.05);
            Check::SaveTransformed (arc);
            }
        }
    Check::ClearGeometry ("MatrixWeightedBezier.EllipseSections");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MatrixWeightedBezier,WobbleStress)
    {
    double xB = 1.0;
    double radiansB = Angle::DegreesToRadians (90.0);
    int numEval = 40;
    double tickSize = 0.1;
    bool rescale = false;
    for (double mu : bvector<double>{1,2,4,8,16})
        {
        SaveAndRestoreCheckTransform (0, 20,0);
        for (double yShoulder : bvector<double> {0.1, 0.5, 1.0, 3.0})
            {
            SaveAndRestoreCheckTransform shifter (2,0,0);
            for (double fB : bvector<double> { 1.5, 2.0, 3.0, 5.0})
                {
                double radiansA = atan2 (yShoulder, xB);
                MatrixWeightedBezier2d bezier (1.0, mu);
                bezier.AddCP (0.0, 0.0, radiansA);
                double yC = fB * yShoulder;
                bezier.AddCP (xB, yC, radiansB);
                bvector<double> coffs;
                bezier.FormBezcoffs2d (coffs);
                bvector<DPoint3d> points;
                bezier.EvaluateBezcoffs2d (coffs, numEval, points, rescale);
                Check::SaveTransformed (points);
                Check::SaveTransformed (bvector<DPoint3d> {DPoint3d::From (0,0), DPoint3d::From (xB, yShoulder),
                    DPoint3d::From (xB, yC),
                    DPoint3d::From (xB + tickSize, yC)
                    });
                }
            }
        }
    Check::ClearGeometry ("MatrixWeightedBezier.WobbleStress");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(LineString,OffsetA)
    {
    auto pointA = bvector<DPoint3d> {
        DPoint3d::From (427369.30676537694,4502732.6026837705,1319.3960517455339),
        DPoint3d::From (427371.99373941735,4502732.5242798980,1319.4318933145964),
        DPoint3d::From (427372.14851206850,4502732.5133412220,1319.4384689670742),
        DPoint3d::From (427372.30185038043,4502732.4896433270,1319.4450446195526),
        DPoint3d::From (427372.45270432608,4502732.4533484895,1319.4516202720311),
        DPoint3d::From (427372.60004089068,4502732.4047052478,1319.4581959245097),
        DPoint3d::From (427372.74285114580,4502732.3440467017,1319.4647715769881),
        DPoint3d::From (427372.88015715819,4502732.2717882274,1319.4713472294666),
        DPoint3d::From (427373.01101868640,4502732.1884246338,1319.4779228819450),
        DPoint3d::From (427373.07375084038,4502732.1427539438,1319.4812107081843),
        DPoint3d::From (427373.13453961944,4502732.0945267789,1319.4844985344234),
        DPoint3d::From (427373.24987411301,4502731.9907376524,1319.4910741869021),
        DPoint3d::From (427373.35623238189,4502731.8777679801,1319.4976498393805),
        DPoint3d::From (427373.45288610796,4502731.7563913539,1319.5042254918590),
        DPoint3d::From (427373.53917342779,4502731.6274389345,1319.5108011443376),
        DPoint3d::From (427373.61450346466,4502731.4917937582,1319.5173767968161),
        DPoint3d::From (427373.67836037517,4502731.3503846945,1319.5239525618533),
        DPoint3d::From (427373.73030688107,4502731.2041800823,1319.5305284730707),
        DPoint3d::From (427373.79967465845,4502730.9857642436,1319.5306565102262),
        DPoint3d::From (427373.87231885275,4502730.7684161626,1319.5307555235697),
        DPoint3d::From (427373.91649177019,4502730.6196782002,1319.5242357537195),
        DPoint3d::From (427373.94821572211,4502730.4677972635,1319.5177159838729),
        DPoint3d::From (427373.96727346996,4502730.3138133986,1319.5111962140263),
        DPoint3d::From (427373.97353451035,4502730.1587810554,1319.5046764441795),
        DPoint3d::From (427373.96695596923,4502730.0037618605,1319.4981566743331),
        DPoint3d::From (427373.94758289482,4502729.8498173505,1319.4916369044865),
        DPoint3d::From (427373.91554794967,4502729.6980017032,1319.4851171346399),
        DPoint3d::From (427373.89484747837,4502729.6232178444,1319.4818572497165),
        DPoint3d::From (427373.87107050221,4502729.5493545206,1319.4785973647931),
        DPoint3d::From (427373.81445512420,4502729.4048937056,1319.4720775949468),
        DPoint3d::From (427373.74608950526,4502729.2656084942,1319.4655578251002),
        DPoint3d::From (427373.66644179809,4502729.1324526789,1319.4590380552536),
        DPoint3d::From (427373.57605741278,4502729.0063380841,1319.4525182854070),
        DPoint3d::From (427373.47555528180,4502728.8881283142,1319.4459985155604),
        DPoint3d::From (427373.36562362156,4502728.7786328420,1319.4394785114293),
        DPoint3d::From (427373.24701521971,4502728.6786014717,1319.4329696665602),
        DPoint3d::From (427366.15277712437,4502723.1812729146,1319.3133222813860)
        };

    auto cvA = CurveVector::CreateLinear (pointA);
    DRange3d range;
    cvA->GetRange (range);
    auto dX = range.XLength ();
    auto dY = range.YLength ();
    double offsetDistance = -0.079999999548142409;
    CurveOffsetOptions options (offsetDistance);
    auto cvB = cvA->CloneOffsetCurvesXY (options);
    static double tol = 1.0e-3;

    {
    SaveAndRestoreCheckTransform shifter (dX, 0, 0);
    Check::SaveTransformed (*cvA);
    Check::SaveTransformed (*cvB);
    cvB->SimplifyLinestrings (tol, true, true, true);
    Check::Shift (0, dY, 0);
    Check::SaveTransformed (*cvA);
    Check::SaveTransformed (*cvB);
    }

    for (double f : bvector<double>{1.0, 2.0, 3.0, 5.0, 10.0})
        {
        bvector<DPoint3d> pointB;
        PolylineOps::OffsetLineString (pointB, pointA, f * offsetDistance, DVec3d::From (0,0,1), false, 1.0);
        Check::SaveTransformed (pointA);
        Check::SaveTransformed (pointB);
        Check::Shift (0, dY, 0);
        }
    Check::ClearGeometry ("LineString.OffsetA");
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(LineString,OffsetB)
    {
    double x0 = 427369.30676537694;
    double y0 = 4502732.6026837705;
    double a = 0.1;
    auto points = bvector<DPoint3d> {
        DPoint3d::From (x0, y0),
        DPoint3d::From (x0 + 2 * a, y0),
        DPoint3d::From (x0 + 4 * a, y0 + 1 * a),
        DPoint3d::From (x0 + 5 * a, y0 + 3 * a),
        DPoint3d::From (x0 + 7 * a, y0 + 4 * a),
        DPoint3d::From (x0 + 8 * a, y0 + 6 * a),
        DPoint3d::From (x0 + 8 * a, y0 + 8 * a)
        };
    // create various inflections at point3 
    for (auto f : bvector<double> { 0.1, 0.01, 0.001, 0.0001})
        {
        points[3] = DPoint3d::FromInterpolateAndPerpendicularXY (points[2], 0.5, points[4], f);
        auto cvA = CurveVector::CreateLinear (points);
        double offsetDistance = -0.079999999548142409;
        CurveOffsetOptions options (offsetDistance);
        auto cvB = cvA->CloneOffsetCurvesXY (options);
        Check::SaveTransformed (*cvA);
        Check::SaveTransformed (*cvB);
        Check::Shift (0, 10.0 * a, 0);
        }
    Check::ClearGeometry ("LineString.OffsetB");
    }

struct FrameWithGeometry
{
Transform localToWorld;
Transform worldToLocal;
bvector<DPoint3d>  samplePoints;
IGeometryPtr geometry;
};
//!
//! Place frames along a CurveVectorWithDistanceIndex, at true arc length distance {frameStep}
//! The frame origins are equally spaced by arc length
//! Each frame's z direction is forward along the curve tangent.
//! Each frame's x direction is perpendicular and in the global xy plane.
void PlaceFramesAtUniformArcLength
(
bvector<FrameWithGeometry> &frames,          //!< [out] frames with Transforms and local geoemtry
CurveVectorWithDistanceIndexCR path,    //!< [in] track curve.
IGeometryPtr const &geometry,           //!< [in] optional geometry to place in the local frame.
double frameStep                        //!< [in] arc length between stations.
)
    {
    DVec3d vectorZ = DVec3d::From (0,0,1);
    frames.clear ();
    double totalPathLength = path.TotalPathLength ();
    for (size_t i = 0; i * frameStep <= totalPathLength; i++)
        {
        double d = i * frameStep;
        auto ray = path.DistanceAlongToPointAndUnitTangent (d).Value ();
        DVec3d sideVector = DVec3d::FromNormalizedCrossProduct (vectorZ, ray.direction);
        DVec3d upVector =  DVec3d::FromNormalizedCrossProduct (ray.direction, sideVector);
        FrameWithGeometry planeData;
        planeData.localToWorld = Transform::FromOriginAndVectors (ray.origin,
                        sideVector, upVector, ray.direction);
        planeData.worldToLocal = planeData.localToWorld.ValidatedInverse ().Value ();
        if (geometry.IsValid ())
            planeData.geometry = geometry->Clone (planeData.localToWorld);
        frames.push_back (planeData);
        }
    }

//!
//! Place rearTrackFrames along a CurveVectorWithDistanceIndex, at true arc length distance {frameStep}
//! The origin of a rearTrackFrame is the position of the bogie center.
//! For each rearTrackFrame, also create a corresponging carFrame that points (at an angle to the track) from
//! the rear bogie center to the forward bogie center.
void PlaceFramesForBogieCenters
(
bvector<FrameWithGeometry> &rearTrackFrames,
bvector<FrameWithGeometry> &carFrames,
CurveVectorWithDistanceIndexCR path,
IGeometryPtr const& carGeometry,
double frameStep,       // step between placements of the rear bogie.
double bogieDistance,        // bogie-to-bogie distance
double maxTurnAngle = Angle::DegreesToRadians (45.0)    // maximum plausible turn angle 
)
    {
    // make an arc to be placed in each frame and the intersected with the curve ahead.
    // REMARK: This intersection is as viewed in xy.  If the curve has elevation changes, do some
    // work to iterate to a spherical intersection instead of the xy cylinder intersection.
    // (That may also have to consider distance from track to frame vector?)
    auto referenceArc = ICurvePrimitive::CreateArc (DEllipse3d::From (
            0,0,0,
            0,0,bogieDistance,
            bogieDistance,0,0,
            -maxTurnAngle, 2.0 * maxTurnAngle
            ));
    DVec3d vectorZ = DVec3d::From (0,0,1);

    bvector<FrameWithGeometry> candidateRearTrackFrames;
    rearTrackFrames.clear ();
    carFrames.clear ();
    PlaceFramesAtUniformArcLength (candidateRearTrackFrames, path, nullptr, frameStep);

    CurveVectorPtr intersectionsA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionsB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);\
    // bypass const pointer ...
    CurveVectorP pathCurveVector = const_cast <CurveVectorP> (path.GetCurveVector().get ());
    for (auto rearFrame : candidateRearTrackFrames)
        {
        auto localArc = referenceArc->Clone (rearFrame.localToWorld);
        intersectionsA->clear ();
        intersectionsB->clear ();
        // Check::SaveTransformed (*localArc);
        CurveCurve::IntersectionsXY (
            *intersectionsA, *intersectionsB,
            *localArc,
            *pathCurveVector,
            nullptr);
        DPoint3d pointA, forwardPoint;
        double fractionA, fractionB;
        size_t numIntersection = intersectionsB->size ();
        if (numIntersection != 1)
            {
            }
        else if (CurveCurve::IsSinglePointPair (
                *intersectionsA, *intersectionsB, 0,
                fractionA, pointA, fractionB, forwardPoint
            ))
            {
            auto rearPoint = rearFrame.localToWorld.Origin ();
            DVec3d carAxisVector = DVec3d::FromStartEndNormalize (rearPoint, forwardPoint);
            DVec3d sideVector = DVec3d::FromNormalizedCrossProduct (vectorZ, carAxisVector);
            DVec3d upVector = DVec3d::FromNormalizedCrossProduct (carAxisVector, sideVector);
            FrameWithGeometry carFrame;
            carFrame.localToWorld = Transform::FromOriginAndVectors (rearPoint,
                        sideVector, upVector, carAxisVector);
            carFrame.worldToLocal = carFrame.localToWorld.ValidatedInverse ().Value ();
            if (carGeometry.IsValid ())
                carFrame.geometry = carGeometry->Clone (carFrame.localToWorld);
            carFrames.push_back (carFrame);
            rearTrackFrames.push_back (rearFrame);
            }
        }
    }

void Save (bvector <FrameWithGeometry> &frames)
    {
    for (auto &frame : frames)
        if (frame.geometry.IsValid ())
            Check::SaveTransformed (frame.geometry);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Train,Envelope)
    {
    CurveVectorPtr path = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    path->Add (ICurvePrimitive::CreateLine (DSegment3d::From (0,0,0, 100,0,0)));

    auto endPoint = path->at(0)->FractionToPointAndUnitTangent (1.0).Value ();
    path->Add(ICurvePrimitive::CreateArc (DEllipse3d::FromStartTangentNormalRadiusSweep (
                endPoint.origin, endPoint.direction, DVec3d::From (0,0,1), 150.0, Angle::DegreesToRadians (45.0))));
    auto pathWithDistanceIndex = CurveVectorWithDistanceIndex::Create ();
    pathWithDistanceIndex->SetPath (path);

    double dx = 3.0;
    double dy = 10.0;
    auto crossSectionGeometry = IGeometry::Create (
        CurveVector::CreateRectangle (-dx, 0.0, dx, dy, 0.0,
            CurveVector::BOUNDARY_TYPE_Open));
    // just a bogie-to-bogie box for the car -- the geometry used for sweep has to extend beyond the bogies.
    // (yes, the car is flipped up in the z direction -- thats the way the placements work.
    double width = 2.5;
    double halfWidth = width * 0.5;
    double height = 3.5;
    double bogieDistance = 15.0;
    auto bogieToBogieBox = IGeometry::Create (ISolidPrimitive::CreateDgnBox (
            DgnBoxDetail (
                DPoint3d::From (-halfWidth,0,0),
                DPoint3d::From (-halfWidth,0,bogieDistance),
                DVec3d::From (1,0,0), DVec3d::From (0,1,0),
                width, height, width, height, true)));

    Check::SaveTransformed (*path);

    bvector <FrameWithGeometry> allPlanes, carRear, carOnTrack;
    double crossSectionStep = 2.0;
    PlaceFramesAtUniformArcLength (allPlanes, *pathWithDistanceIndex, crossSectionGeometry, crossSectionStep);
    Save(allPlanes);
    double carStep = 5.0;
    PlaceFramesForBogieCenters (carRear, carOnTrack, *pathWithDistanceIndex, bogieToBogieBox, carStep, bogieDistance);

    Check::Shift (0,100,0);
    Check::SaveTransformed (*path);
    Save(carOnTrack);


    Check::ClearGeometry ("Train.Envelope");
    }
CurveVectorPtr  ThickenPathToRegion(CurveVectorPtr const& plineCurve, double halfWidth)
    {

    double  pointTolerance = 0.01 * halfWidth;

    // move the curve towards left by half width
    CurveOffsetOptions  offsetOptions(halfWidth);
    offsetOptions.SetTolerance(pointTolerance);

    auto left = plineCurve->CloneOffsetCurvesXY(offsetOptions);
    if (!left.IsValid())
        return  nullptr;

    // move the curve towards right by half width
    offsetOptions.SetOffsetDistance(-halfWidth);

    auto right = plineCurve->CloneOffsetCurvesXY(offsetOptions);
    if (!right.IsValid())
        return  nullptr;
    if (plineCurve->IsClosedPath())
        {
        auto parityRegion = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
        parityRegion->Add(right);
        parityRegion->Add(left);
        parityRegion->FixupXYOuterInner(false);
        return parityRegion;
        }
    else
        {
        auto shape = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);

        ICurvePrimitivePtr  top, bottom;
        // get start and end points
        DPoint3d    starts[2], ends[2];
        if (!left->GetStartEnd(starts[0], ends[0]) || !right->GetStartEnd(starts[1], ends[1]))
            return  nullptr;

        // bottom cap to connect left->right
        bottom = ICurvePrimitive::CreateLine(DSegment3d::From(ends[0], ends[1]));
        if (!bottom.IsValid())
            return  nullptr;

        // top cap to connect right->left
        top = ICurvePrimitive::CreateLine(DSegment3d::From(starts[1], starts[0]));
        if (!top.IsValid())
            return  nullptr;

        // reverse the right curve
        right->ReverseCurvesInPlace();

        // add and orient them to complete a loop:
        shape->AddPrimitives(*left);
        shape->Add(bottom);
        shape->AddPrimitives(*right);
        shape->Add(top);
        if (shape->IsClosedPath())
            return  shape;

        CurveGapOptions gapOptions(pointTolerance, 1.0e-4, 1.0e-4);
        return shape->CloneWithGapsClosed(gapOptions);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ThickenPathToRegion, openPolylines)
    {
    bvector<bvector<DPoint3d>> paths{
        {
        DPoint3d::From(-10, 0, 0),
        DPoint3d::From(0, 0, 0),
        DPoint3d::From(4, -5, 0),
        DPoint3d::From(-10,-5,0)
        },
        {
        DPoint3d::From(-10, 0, 0),
        DPoint3d::From(0, 0, 0),
        DPoint3d::From(8, -5, 0),
        DPoint3d::From(-10,-5,0),
        DPoint3d::From(-10, 0, 0)
        }

        /* ,
        {
        DPoint3d::From (0, 0, 0),
        DPoint3d::From (10, 0, 0),
        DPoint3d::From (10, 5, 0)
        },
        {
        DPoint3d::From (0, 0, 0),
        DPoint3d::From (10, 0, 0),
        DPoint3d::From (12, 5, 0)
        },
        {
        DPoint3d::From (0, 0, 0),
        DPoint3d::From (10, 0, 0),
        DPoint3d::From (0, 5, 0),
        DPoint3d::From (10,10,0),
        DPoint3d::From ( 0,10,0),
        DPoint3d::From (8,15,0)
        } */
        };
    double verticalShift = 0.001;
    for (DPoint2d offsets : {
        DPoint2d::From (0.5, 0.5),
        DPoint2d::From (0.0, 0.25),
        DPoint2d::From (0.25, 0.0),
            DPoint2d::From(-0.25, 0.5),
        })
        {
        double leftOffset  = offsets.x;
        double rightOffset = offsets.y;
        double radius = 1.0;
        for (auto & points : paths)
            {
            SaveAndRestoreCheckTransform shifter(30, 0, 0);
            auto cv = CurveVector::CreateLinear(points, 
                points.back ().AlmostEqual (points.front ()) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
            Check::SaveTransformed(*cv);
            Check::Shift(0, 20, 0);

            auto region = CurveVector::ThickenXYPathToArea(cv, leftOffset, rightOffset);

            Check::Shift(0, 0, verticalShift);
            Check::SaveTransformed(points);
            Check::Shift(0, 0, -verticalShift);
            if (region.IsValid())
                Check::SaveTransformed(*region);

            Check::Shift (0,20,0);

            auto cvB = cv->CloneWithFillets (radius);
            auto regionB = CurveVector::ThickenXYPathToArea(cvB, leftOffset, rightOffset);
            Check::Shift(0, 0, verticalShift);
            Check::SaveTransformed(points);
            Check::Shift(0, 0, -verticalShift);
            if (regionB.IsValid ()  )
                Check::SaveTransformed (*regionB);
            }
        }
    Check::ClearGeometry("ThickenPathToRegion.openPolylines");
    }


CurveVectorPtr CurveVector19Feb00()
    {
    auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);


    cv->push_back(ICurvePrimitive::CreateLine(
        DSegment3d::From(
            DPoint3d::From(226818.62784515313, 68416.728429918381, 0.0),
            DPoint3d::From(226818.45465399697, 68416.753526201544, 0.0))));

    return cv;
    }
CurveVectorPtr CurveVector19Feb01()
    {
    auto cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    cv->push_back(ICurvePrimitive::CreateArc(
        DEllipse3d::FromScaledRotMatrix(
            DPoint3d::From(226917.43056137353, 68710.221406539262, 0.0),
            RotMatrix::FromColumnVectors(
                DVec3d::From(0.11039277888089406, -0.99388803915277812, 0.0),
                DVec3d::FromCrossProduct(DVec3d::From(0.0, -0.0, -1.0), DVec3d::From(0.11039277888089406, -0.99388803915277812, 0.0)),
                DVec3d::From(0.0, -0.0, -1.0)),
            309.67741935489840, 309.67741935489840,
            Angle::DegreesToRadians(24.943467150543480), Angle::DegreesToRadians(-24.943467150543480))));


    cv->push_back(ICurvePrimitive::CreateLine(
        DSegment3d::From(
            DPoint3d::From(226951.61671225278, 68402.436723446735, 0.0),
            DPoint3d::From(227237.84239773938, 68434.228281009811, 0.0))));

    return cv;
    }

TEST(CurveCurve, TaperFilletTaperSmallRadius)
    {
    char const *subAStr = R"({"DgnCurveVector":{"Member":[{"LineSegment":{"endPoint":[226818.45465399697,68416.753526201544,0.0],"startPoint":[226818.62784515313,68416.728429918381,0.0]}}],"boundaryType":1}})";
    char const *subBStr = R"({"DgnCurveVector":{"Member":[{"CircularArc":{"placement":{"origin":[226917.43056137353,68710.221406539262,0.0],"vectorX":[0.11039277888089406,-0.99388803915277812,0.0],"vectorZ":[0.0,-0.0,-1.0]},"radius":309.67741935489840,"startAngle":24.943467150543480,"sweepAngle":-24.943467150543480}},{"LineSegment":{"endPoint":[227237.84239773938,68434.228281009811,0.0],"startPoint":[226951.61671225278,68402.436723446735,0.0]}}],"boundaryType":1}})";
    for (double radius : { 0.5, 0.1 })
        {
        SaveAndRestoreCheckTransform shifter(0, 40, 0);
        for (double distanceFactor : {0.2, 0.5, 1.0, 2.0, 5.0})
            {
            SaveAndRestoreCheckTransform shifter(0, 2, 0);
            double distanceA = 0.0090659115617812997 * distanceFactor;
            double distanceB = 0.0090659115681263475 * distanceFactor;
            RunTaperFilletTaper(subAStr, subBStr,
                        0, 0,
                        radius,
                        0.0, 0.0,
                        distanceA, distanceB,
                        0.0, 0.0);
            }
        }

#ifdef abc
    void RunTaperFilletTaper(char const * jsonA, char const * jsonB,
        double setbackA,
        double taperA,
        double filletRadius,
        double setbackB,
        double taperB,
        double &distanceA,
        double &distanceB,
        double offsetA,
        double offsetB,
        bool  setExtendedPath = false   // false forces new logic to use tangent extension



    CurveVectorPtr subA = GeometryHelper::DeserializeCurveVector(subAStr);
    CurveVectorPtr subB = GeometryHelper::DeserializeCurveVector(subBStr);

    auto pathA = CurveVectorWithDistanceIndex::Create();
    pathA->SetPath(*subA);
    auto pathB = CurveVectorWithDistanceIndex::Create();
    pathB->SetPath(*subB);

    double distanceA = 0.0090659115617812997;
    double distanceB = 0.0090659115681263475;
    for (double radius : { 0.5, 0.1 })
        {
        SaveAndRestoreCheckTransform shifter (0, 20, 0);
        Check::SaveTransformed (*subA);
        Check::SaveTransformed (*subB);
        auto filletPath = GeometryHelper::ConstructTaperFilletTaperEx(*pathA, *pathB, 0.0, 0.0, .1, 0.0, 0.0, distanceA, distanceB);
        if (filletPath.IsValid ())
            Check::SaveTransformed (*filletPath);
        }
#endif
    Check::ClearGeometry ("CurveCurve.TaperFilletTaperSmallRadius");
    }
#ifdef abc
double distanceA = 0.0090659115617812997;
double distanceB = 0.0090659115681263475;
auto filletPath = GeometryHelper::ConstructTaperFilletTaperEx(*pathA, *pathB, 0.0, 0.0, .1, 0.0, 0.0, distanceA, distanceB);

ICurvePrimitiveR curveA,
ICurvePrimitiveR curveB,
double fractionA,
double fractionB,
double setbackA,            //!< [in] setback distance from vectorA
double taperA,              //!< [in] taper distance along vectorA
double filletRadius,        //!< [in] fillet radius
double setbackB,            //!< [in] setback distance from vectorB
double taperB

#endif