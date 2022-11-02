/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <GeomSerialization/GeomLibsSerialization.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL

static bool s_noisy = false;
static void AddPoint(bvector<DPoint3d> &data, double x, double y, double z)
    {
    data.push_back (DPoint3d::From (x, y, z));
    }

MSBsplineCurvePtr CreateCurve (bvector<DPoint3d> &poles, int order)
    {
    return MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, order, false, false);
    }

void PrintLengthFractions (bvector<MSBsplineCurvePtr> &curveA, bvector<double> &lengthA)
    {
    size_t numCurves = curveA.size ();
    int numKnots0 = curveA[0]->NumberAllocatedKnots ();
    if (s_noisy)
        printf ("Corresponding length fractions\n");
    int order = (int)curveA[0]->GetOrder ();
    double knot0 = curveA[0]->knots[order-1];
    DRange1d fractionRange;
    for (int i = order; i + order - 1 < numKnots0; i++)
        {
        if (s_noisy)
            printf ("(knotIndex %d)", i);
        fractionRange.InitNull ();
        for (int k = 0; k < numCurves; k++)
            {
            double f = curveA[k]->knots[i];
            double a = curveA[k]->LengthBetweenKnots (knot0, f);
            double g = a / lengthA[k];
            fractionRange.Extend (g);
            if (s_noisy)
                printf ("(%.17g %.17g)", f, g);
            }
        if (s_noisy)
            printf ("    (arcFractionRange %g %.17g\n", fractionRange.low, fractionRange.Length ());
        }
  }


void PrintKnots (MSBsplineCurve *curve, char const *name, int index)
    {
    if (!s_noisy)
        return;
    printf ("Knots %s [%d] (order %d numPoles %d)", name, index, curve->params.order, curve->params.numPoles);
    size_t numKnots = curve->NumberAllocatedKnots ();
    for (size_t i = 0; i < numKnots; i++)
        {
        if ((i % 5) == 0)
            printf ("\n         ");
        printf (" %.17g", curve->GetKnot (i));
        }
    printf ("\n");
    }


void ShowGrid (bvector<bvector<DPoint3d>> &points, bool showU = true, bool showV = true)
    {
    if (showV)
        {
        for (size_t i = 1; i < points.size (); i++)
            {
            for (size_t j = 0; j < points[i].size (); j++)
                {
                Check::SaveTransformed (DSegment3d::From (points[i-1][j], points[i][j]));
                }
            }
        }
    if (showU)
        for (auto &p : points)
            Check::SaveTransformed (p);
    }
void CheckCompatibleCurves (bvector<MSBsplineCurvePtr> &curveA, bvector<MSBsplineCurvePtr> &curveB)
    {
    if (!Check::Size (curveA.size (), curveB.size (), "matched curve counts"))
        return;
    bvector<bvector <DPoint3d>> pointA, pointA1, pointA2, pointB;
    size_t numEdge = 10;
    for (auto &c: curveA)
        {
        Check::SaveTransformed (c);

        bvector<DPoint3d> strokes;
        bvector<double> params;
        c->StrokeFixedNumberWithEqualFractionLength (strokes, params, numEdge);
        pointA.push_back (strokes);

        c->StrokeFixedNumberWithEqualChordLength (strokes, params, numEdge);
        pointA1.push_back (strokes);

        if (c->StrokeFixedNumberWithEqualChordError (strokes, params, numEdge))
            pointA2.push_back (strokes);
        }
    ShowGrid (pointA);
    Check::Shift (0,1,0);
    ShowGrid (pointA1);
    Check::Shift (0,1,0);
    ShowGrid (pointA2);

    Check::Shift (0,5,0);
    for (auto &c: curveB)
        {
        bvector<DPoint3d> points;
        bvector<double> params;
        c->StrokeFixedNumberWithEqualFractionLength (points, params, numEdge);
        pointB.push_back (points);
        Check::SaveTransformed (c);

        bvector<DPoint3d> strokes;
        c->StrokeFixedNumberWithEqualChordLength (strokes, params, numEdge);
        Check::SaveTransformed (c);
        Check::SaveTransformed (strokes);
        }
    ShowGrid (pointB);
    Check::Shift (0,10,0);


    bvector<double> lengthA, lengthB;
    bool ok = true;
    int numKnots0 = curveB[0]->NumberAllocatedKnots ();
    size_t numCurves = curveA.size ();
    for (int i = 0; i < numCurves; i++)
        {
        lengthA.push_back (curveA[i]->Length ());
        lengthB.push_back (curveB[i]->Length ());
        Check::Near (lengthA[i], lengthB[i], "Compatible curve maintains length");
        PrintKnots (curveA[i].get (), "curveA", i);
        PrintKnots (curveB[i].get (), "curveB", i);
        ok &= curveB[i]->NumberAllocatedKnots () == numKnots0;
        }
    if (Check::True (ok, "ArcCompatible curves have same knot counts"))
        PrintLengthFractions (curveB, lengthB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, CompatibleByArcLenth)
    {
    bvector<DPoint3d> pointA, pointB, pointC, pointD;
    AddPoint (pointA, 0,0,0);
    AddPoint (pointA, 1,0,0);
    AddPoint (pointA, 2,1,0);
    AddPoint (pointA, 3,0,0);
    AddPoint (pointA, 4,1,0);


    AddPoint (pointB, 0,0,1);
    AddPoint (pointB, 1,0,1);
    AddPoint (pointB, 2,1,1);
    AddPoint (pointB, 3,0,1);
    AddPoint (pointB, 4,1,1);       // curveB just like curveA, shifted up to z=1

    AddPoint (pointC, 0,0,2);
    AddPoint (pointC, 1,0,2);
    AddPoint (pointC, 3,1,2);
    AddPoint (pointC, 4,1,2);       // curveC lifted to z=2, no interior knot
    
    AddPoint (pointD, 0,0,3);
    AddPoint (pointD, 1,0,3);
    AddPoint (pointD, 2,4,3);
    AddPoint (pointD, 2.5,4,3);
    AddPoint (pointD, 3,1,3);
    AddPoint (pointD, 4,1,3);       // curveD lifted to z=3, no interior knot, will be built as order 3

    MSBsplineCurvePtr inputA = CreateCurve (pointA, 4);
    MSBsplineCurvePtr inputB = CreateCurve (pointB, 4);
    MSBsplineCurvePtr inputC = CreateCurve (pointC, 4);
    MSBsplineCurvePtr inputD = CreateCurve (pointD, 3);
    bvector<MSBsplineCurvePtr> inputCurves;
    bvector<MSBsplineCurvePtr> outputCurves;
    
    Check::StartScope ("bspcurv_makeCompatibleByArcLength");
    inputCurves.push_back (inputA);
    inputCurves.push_back (inputB);
    if (Check::True (MSBsplineCurve::CloneArcLengthCompatibleCurves (outputCurves, inputCurves, false, false), "2 identical curves"))
      CheckCompatibleCurves (inputCurves, outputCurves);

    inputCurves.push_back (inputC);
    if (Check::True (MSBsplineCurve::CloneArcLengthCompatibleCurves (outputCurves, inputCurves, false, false), "3 curves"))
      CheckCompatibleCurves (inputCurves, outputCurves);

    inputCurves.push_back (inputD);
    if (Check::True (MSBsplineCurve::CloneArcLengthCompatibleCurves (outputCurves, inputCurves, false, false), "4 curves"))
      CheckCompatibleCurves (inputCurves, outputCurves);

    Check::EndScope ();
    Check::ClearGeometry ("Bspline.CompatibleByArcLength");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Bspline, FromCoplanarTangents)
    {
    bvector<DRay3d> tangents;
    double a = 1.0;
    double b = 2.0;
    for (double theta = 0.0; theta < 3.0;  theta += 0.25)
        {
        double c = cos (theta);
        double s = sin (theta);
        auto ray = DRay3d::FromOriginAndVector (DPoint3d::From (a * c,b * s,0), DVec3d::From (-s * a, c * b,0));
        tangents.push_back (ray);
        Check::SaveTransformed (DSegment3d::From (ray.FractionParameterToPoint (-0.1), ray.FractionParameterToPoint (0.1)));
        }
    double tolerance = 1.0e-5;
    MSBsplineCurve curve;
    if (Check::Int (0, mdlBspline_interpolateCoplanarTangents (&curve, &tangents[0], (int)tangents.size (), false, false, tolerance)))
        {
        auto curve1 = curve.CreateCapture ();
        Check::SaveTransformed (curve1);
        curve1->ElevateDegree (3);
        Check::Shift (2 * a, 0,0);
        Check::SaveTransformed (curve1);
        for (double toleranceA : {0.001, 0.01, 0.05, 0.1, 0.5})
            {
            Check::Shift (0,2 * b, 0);
            MSBsplineCurve curveB;
            if (Check::Int (0, bspcurv_cubicDataReduce (&curveB, curve1.get (), toleranceA)))
                {
                Check::SaveTransformed (curve1);
                auto curveB1 = curveB.CreateCapture ();
                Check::SaveTransformed (curveB1);
                }
            }
        }

    Check::ClearGeometry ("Bspline.FromCoplanarTangents");
    }

void PrintBearingCurvature (char const *title, DSpiral2dBase &spiral, double fraction)
    {
    double distance = spiral.FractionToDistance (fraction);
    double turnDegrees = Angle::RadiansToDegrees (spiral.DistanceToLocalAngle (distance));
    double curvature = spiral.DistanceToCurvature (distance);
    double radius;
    DoubleOps::SafeDivide (radius, 1.0, curvature, 0.0);
    if (s_noisy)
        printf ("  at %s    (fraction %#.17g) (distance %#.17g)\n        (turn since start %#.17g deg) (radius %#.17g)\n", title, fraction, distance, turnDegrees, radius);
    }
    
static void PrintSpiral (char const *title, DSpiral2dBase &spiral, size_t numStep, DPoint2dCP xy0 = NULL)
    {
    if (!s_noisy)
        return;
    printf ("\n **** %s ***** \n", title);
    DPoint2d origin = DPoint2d::FromZero ();
    if (xy0 != NULL)
        origin = *xy0;
    PrintBearingCurvature ("start", spiral, 0.0);
    PrintBearingCurvature ("end", spiral, 1.0);
    for (size_t i = 0; i <= numStep; i++)
        {
        double f = i / (double)numStep;
        double d = f * spiral.mLength;
        double e, r;
        double c = spiral.DistanceToCurvature (d);
        DoubleOps::SafeDivide (r, 1.0, c, 0.0);
        DVec2d xy;
        DSpiral2dBase::Stroke (spiral, 0.0, f, 0.0, xy, e, DEFAULT_SPIRAL_MAX_STROKE_LENGTH);
        xy.x += origin.x;
        xy.y += origin.y;
        printf (" (f %#.17g) (d %#.17g) (r %#.17g) (xy %#.17g  %#.17g)\n",
                    f, d, r, xy.x, xy.y);
        }
    }

void CheckSpiralStroke (DSpiral2dBaseR spiral)
    {
    //double length = spiral.FractionToDistance (1.0);
    bvector <DVec2d> xy0,xy1, xy2;
    bvector <double> f0, f1, f2;
    double e0, e1, e2;
    double strokeFraction1 = 0.5;   
    double strokeFraction2 = strokeFraction1 * 0.5;
    double strokeRadians1 = 1.0;        // try to eliminate this from participating
    DSpiral2dBase::Stroke (spiral, 0.0, 1.0, 0.0, xy0, f0, e0, DEFAULT_SPIRAL_MAX_STROKE_LENGTH);
    DSpiral2dBase::Stroke (spiral, 0.0, 1.0, strokeRadians1, xy1, f1, e1, strokeFraction1 * DEFAULT_SPIRAL_MAX_STROKE_LENGTH);
    DSpiral2dBase::Stroke (spiral, 0.0, 1.0, strokeRadians1, xy2, f2, e2, strokeFraction2 * DEFAULT_SPIRAL_MAX_STROKE_LENGTH);
    size_t size0 = f0.size ();
    size_t size1 = f1.size ();
    size_t size2 = f2.size ();
    Check::True (size1 >= size0);
    Check::True (size2 >= size1);
    double d01 = xy0.back ().Distance (xy1.back ());
    double d12 = xy1.back ().Distance (xy2.back ());
    // REMARK
    // stroke0 is defaults.    In examples, size0 is "small" -- e.g. 11
    // if stroke0 is driven by the max edge length (rather than angle), stroke1 and 2 are respectively 2x and 4X points.
    // Difference d12 is "usually" smaller -- one exception being BiQuadratic.   Is the discontinuity affecting things?
    printf("(d01 %8.2g) (d12 %8.2g)\n",d01, d12);
    Check::NearZero (d01, "stroke spirals match", 1000.0);
    Check::NearZero (d12, "Heavily stroke spirals match", 100.0);
    }
void PrintSpiralAtDistances (char const*title, DSpiral2dBase &spiral, bvector<double>distances)
    {
    CheckSpiralStroke (spiral);
    if (s_noisy)
        printf ("\n **** %s ***** \n", title);
    PrintBearingCurvature ("start", spiral, 0.0);
    PrintBearingCurvature ("end", spiral, 1.0);
    for (size_t i = 0; i < distances.size (); i++)
        {
        double d = distances[i];
        double f = spiral.DistanceToFraction (d);
        double e, r;
        double c = spiral.DistanceToCurvature (d);
        DoubleOps::SafeDivide (r, 1.0, c, 0.0);
        DVec2d xy;
        DSpiral2dBase::Stroke (spiral, 0.0, f, 0.0, xy, e, DEFAULT_SPIRAL_MAX_STROKE_LENGTH);
        if (s_noisy)
            printf (" (f %#.17g) (d %#.17g) (r %#.17g) (xy %#.17g  %#.17g)\n",
                    f, d, r, xy.x, xy.y);
        }
    }

double Series2Term_yAtx (double x, double R, double L)
    {
    return x * x * x / (6.0 * R * L);
    }

double Series3Term_yAtx (double x, double R, double L)
    {
    return x * x * x / (6.0 * R * L);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,CubicTransition)
    {
    double a = 100.0;    // cubic is defined over this x length
    double d0 = a;
    double R = 1500.0;
    double L = 100.0;
    double b = a * a * a / (6.0 * R * L);     // nominal height.
    bvector<DPoint3d> cubicPoles;
    AddPoint (cubicPoles, 0,0,0);
    AddPoint (cubicPoles, a / 3.0, 0, 0);
    AddPoint (cubicPoles, 2.0 * a / 3.0, 0, 0);
    AddPoint (cubicPoles, a, b, 0.0);

    bvector<double> distances;
    distances.push_back (a);

    DSpiral2dClothoid clothoid;
    double radiusB = R;
    double curvatureB = 1.0 / radiusB;
    clothoid.SetBearingCurvatureLengthCurvature (0.0, 0.0, a, curvatureB);
    PrintSpiralAtDistances ("clothoid", clothoid, distances);

    static double strokeRadians = 0.4;

    MSBsplineCurvePtr cubic = MSBsplineCurve::CreateFromPolesAndOrder (cubicPoles, NULL, NULL, 4, false, false);
    double f1, d1;
    Check::True (cubic->FractionAtSignedDistance (0.0, d0, f1, d1), "Compute end of transition cubic");
    DPoint3d xyz1;
    DVec3d tangent1;
    cubic->FractionToPoint (xyz1, tangent1, f1);
    if (s_noisy)
        {
        printf (" (a %g) (RL %g) (b %lg) (f1 %#.17g)\n (xyz1 %#.17g %#.17g\n",
                  a, R * L, b, f1, xyz1.x, xyz1.y);
        printf (" (tangent1 %#.17g %#.17g) (angle1 %#.17g)\n",
                  tangent1.x, tangent1.y, 
                  Angle::RadiansToDegrees (atan2 (tangent1.y, tangent1.x)));
        }
    bvector<DPoint3d> pointA;
    bvector<double>   paramA;
    for (double x = 5.0; x < 100.0; x += 5.0)
        {
        DPlane3d plane = DPlane3d::FromOriginAndNormal (x,0,0, 1,0,0);
        cubic->AddPlaneIntersections (&pointA, &paramA, plane);
        }
    for (size_t i = 0; i < pointA.size (); i++)
        {
        double distance = cubic->LengthBetweenFractions (0.0, paramA[i]);

        DVec2d uv;
        double e;
        double f = clothoid.DistanceToFraction (distance);
        DSpiral2dBase::Stroke (clothoid, 0.0, f, strokeRadians, uv, e);
        if (s_noisy)
            {
            printf (" (cubcxy@X      %#.17g,  %#.17g)   (Dalong %#.17g)\n (clXY@Distance %#.17g,  %#.17g)                                       (err %.5g, %.5g)\n",
                  pointA[i].x, pointA[i].y, distance,
                  uv.x, uv.y, uv.x - pointA[i].x, uv.y - pointA[i].y);
            }
        }
    DVec3d axes1[3];
    DPoint3d origin1;
    double curvature1, torsion1;
    cubic->GetFrenetFrame (axes1, origin1, curvature1, torsion1, f1);
    if (s_noisy)
        printf (" (curvature1 %#17g)\n", curvature1);



    DSpiral2dBloss bloss;
    bloss.SetBearingCurvatureLengthCurvature (0.0, 0.0, a, curvatureB);
    PrintSpiralAtDistances ("bloss", bloss, distances);

    DSpiral2dBiQuadratic biquadratic;
    biquadratic.SetBearingCurvatureLengthCurvature (0.0, 0.0, a, curvatureB);
    PrintSpiralAtDistances ("biquadratic", biquadratic, distances);

    DSpiral2dCosine cosine;
    cosine.SetBearingCurvatureLengthCurvature (0.0, 0.0, a, curvatureB);
    PrintSpiralAtDistances ("cosine", cosine, distances);

    DSpiral2dCosine sine;
    sine.SetBearingCurvatureLengthCurvature (0.0, 0.0, a, curvatureB);
    PrintSpiralAtDistances ("sine", sine, distances);

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral, L100R1500)
    {
    double Theta0 = 0.0;
    double C0 = 0.0;
    double R1 = 1500.0;
    double L1 = 100.0;
    DSpiral2dClothoid clothoid;
    clothoid.SetBearingCurvatureLengthCurvature (Theta0, C0, L1, 1.0 / R1);
    PrintSpiral ("Clothoid", clothoid, 20);
    bvector<double> distances;
    distances.push_back(0);
    distances.push_back (5.000000003);
    distances.push_back (10.00000011);
    distances.push_back (15.00000084);
    distances.push_back (20.00000356);
    distances.push_back (25.00001085);
    distances.push_back (30.000027);
    distances.push_back (35.00005836);
    distances.push_back (40.00011378);
    distances.push_back (45.00020503);
    distances.push_back (50.00034722);
    distances.push_back (55.0005592);
    distances.push_back (60.00086398);
    distances.push_back (65.00128918);
    distances.push_back (70.00186738);
    distances.push_back (75.00263659);
    distances.push_back (80.00364066);
    distances.push_back (85.00492966);
    distances.push_back (90.00656034);
    distances.push_back (95.00859649);
    distances.push_back (100.0111094);
    PrintSpiralAtDistances ("clothoidl100R1500 with distances", clothoid, distances);
  }

  void applyDistance01Radius01 (DSpiral2dBase &spiral, double bearingRadians0, double distance0, double distance1, double radius0, double radius1)
    {
    double curvature0, curvature1;
    DoubleOps::SafeDivide (curvature0, 1.0, radius0, 0.0);
    DoubleOps::SafeDivide (curvature1, 1.0, radius1, 0.0);
    double distance01 = distance1 - distance0;
    spiral.SetBearingCurvatureLengthCurvature (bearingRadians0, curvature0, distance01, curvature1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
  TEST(Spiral, CompoundB)
    {
    DSpiral2dClothoid clothoid;
    double startAngle = 0.0;
    double radius0 = 0.0;;
    double radius1 = 1500.0;
    double distance1 = 255.0;
   double curvature0, curvature1;
    DoubleOps::SafeDivide (curvature0, 1.0, radius0, 0.0);
    DoubleOps::SafeDivide (curvature1, 1.0, radius1, 0.0);
    clothoid.SetBearingCurvatureLengthCurvature (startAngle, curvature0, distance1, curvature1);
    PrintSpiral ("compound example CLOTHOID", clothoid, 255/ 5);


    bvector<double>distances;

    distances.push_back (0);
    distances.push_back (5.000000003);
    distances.push_back (10.00000011);
    distances.push_back (15.00000084);
    distances.push_back (20.00000356);
    distances.push_back (25.00001085);
    distances.push_back (30.000027);
    distances.push_back (35.00005836);
    distances.push_back (40.00011378);
    distances.push_back (45.00020503);
    distances.push_back (50.00034722);
    distances.push_back (55.0005592);
    distances.push_back (60.00086398);
    distances.push_back (65.00128918);
    distances.push_back (70.00186738);
    distances.push_back (75.00263659);
    distances.push_back (80.00364066);
    distances.push_back (85.00492966);
    distances.push_back (90.00656034);
    distances.push_back (95.00859649);
    distances.push_back (100.0111094);
    distances.push_back (105.0141782);
    distances.push_back (110.0178905);
    distances.push_back (115.0223424);
    distances.push_back (120.0276392);
    distances.push_back (125.0338957);
    distances.push_back (130.0412366);
    distances.push_back (135.0497971);
    distances.push_back (140.0597228);
    distances.push_back (145.0711708);
    distances.push_back (150.0843092);
    distances.push_back (155.0993184);
    distances.push_back (160.1163909);
    distances.push_back (165.1357318);
    distances.push_back (170.1575592);
    distances.push_back (175.1821046);
    distances.push_back (180.2096132);
    distances.push_back (185.2403443);
    distances.push_back (190.2745716);
    distances.push_back (195.3125833);
    distances.push_back (200.354683);
    distances.push_back (205.4011895);
    distances.push_back (210.4524371);
    distances.push_back (215.5087765);
    distances.push_back (220.5705741);
    distances.push_back (225.6382132);

    distances.push_back (225.0000526);
    distances.push_back (255.0001622);
    distances.push_back (225.098475);
    distances.push_back (255.1840531);

    PrintSpiralAtDistances ("clothoidl100R1500 with distances", clothoid, distances);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
  TEST(Spiral, LengthTest)
    {
    DSpiral2dClothoid clothoid;
    double startAngle = 0.0;
    double radius0 = 0.0;;
    double radius1 = 500.0;
    double distance1 = 200.0;
    DPoint2d origin = DPoint2d::From (1392.6572, 1000.0);
    double curvature0, curvature1;
    DoubleOps::SafeDivide (curvature0, 1.0, radius0, 0.0);
    DoubleOps::SafeDivide (curvature1, 1.0, radius1, 0.0);
    clothoid.SetBearingCurvatureLengthCurvature (startAngle, curvature0, distance1, curvature1);
    PrintSpiral ("calibration clothoid", clothoid, (int)(distance1 / 5.0), &origin);
    }



void AddKnots (bvector<double> &knots, double a, int count)
    {
    for (int i = 0; i < count; i++)
      knots.push_back (a);
    }

DPoint3d LinearPoles (double a)
    {
    return DPoint3d::From (a, a, 0.0);
    }

DPoint3d QuadraticPoles (double a)
    {
    return DPoint3d::From (a, a * a, 0.0);
    }


MSBsplineCurvePtr MakeBsplineForDiscontinuityTests (int order, int numInterior, int repeatedInteriorIndex, DPoint3d  (*PoleFunction)(double a))
  {
  bvector<double> knots;
  bvector<DPoint3d> poles;

  double a = 0.0;
  AddKnots (knots, a++, order);
  for (int i = 0; i < numInterior; i++)
      {
      int n = i == repeatedInteriorIndex ? order -1 : 1;
      AddKnots (knots, a++, n);
      }
  AddKnots (knots, a++, order);

  int actualInteriorKnots = (int)knots.size () - 2 * order;
  int numPoles = order + actualInteriorKnots;
  for (int i = 0; i < numPoles; i++)
      {
      poles.push_back (PoleFunction ((double)i));
      }
  return MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, &knots, order, false, true);
  }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineCurve,C1Discontinuity)
    {
    bvector<double> fraction1, fraction2;
    bvector<MSBsplineCurvePtr> segment1, segment2;
    for (int order = 2; order < 5; order++)
        {
        for (int numInterior = 0; numInterior < 6; numInterior++)
            {
            for (int iRepeat = 0; iRepeat <= numInterior; iRepeat++)
                {
                MSBsplineCurvePtr curve1 = MakeBsplineForDiscontinuityTests (order, numInterior, iRepeat, LinearPoles);
                MSBsplineCurvePtr curve2 = MakeBsplineForDiscontinuityTests (order, numInterior, iRepeat, QuadraticPoles);
                curve1->GetC1DiscontinuousCurves (fraction1, segment1);
                curve2->GetC1DiscontinuousCurves (fraction2, segment2);

                bool hasInteriorDoubleKnot = iRepeat < numInterior;
                // The curve with linear poles never has a discontinuity ...
                Check::Size (2, fraction1.size (), "Curve with double knot but linear poles has no discontinuities");
                if (order == 2)
                    Check::Size (curve1->GetNumPoles (), fraction2.size (), "order 2 with non-colinear poles is D1 at every pole");
                else if (hasInteriorDoubleKnot)
                    Check::Size (3, fraction2.size (), "Curve with double knot and quadratic poles poles has 1 discontinuity");
                else
                    Check::Size (2, fraction2.size (), "Simple quadratic has no interior discontinuities");
                }
            }
        }

    for (int order = 3; order < 6; order++)
        {
        int numInterior = 10;    // Lots of interior knots and poles !!!!
        size_t k0 = 3;
        for (int numRepeatedPole = 1; numRepeatedPole < order; numRepeatedPole++)
            {
            MSBsplineCurvePtr curve = MakeBsplineForDiscontinuityTests (order, numInterior, -1, QuadraticPoles);
            DPoint3d xyz = curve->GetPole (k0);
            for (int k = 1; k < numRepeatedPole; k++)
                {
                curve->SetPole (k0 + k, xyz);
                }
            curve->GetC1DiscontinuousCurves (fraction1, segment1);
            if (numRepeatedPole + 1 < order)
                Check::Size (2, fraction1.size (), "Multiple poles < order -1 does not make D1");
            else
                Check::Size (3, fraction1.size (), "Multiple poles < order -1 does not make D1");

            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (bspcurve,reduceKnots)
    {
    bvector<DPoint3d> poles;
    poles.push_back (DPoint3d::From (10,3,4));
    poles.push_back (DPoint3d::From (11,1,2));
    poles.push_back (DPoint3d::From (12,5,2));
    poles.push_back (DPoint3d::From (13,3,2));
    poles.push_back (DPoint3d::From (14,9,4));
    poles.push_back (DPoint3d::From (15,1,7));
    poles.push_back (DPoint3d::From (16,5,9));
    double reltol = 1.0e-10;

    bvector<double> auxTolerance (10, reltol);

    for (size_t order = 4; order < poles.size (); order++)
        {
        Check::StartScope ("Add knots to curve order ", order);
        MSBsplineCurvePtr curve = MSBsplineCurve::CreateFromPolesAndOrder (poles, NULL, NULL, (int)order, false, false);
        size_t numKnots0 = curve->GetNumKnots ();
        curve->AddKnot (0.219, 1);
        curve->AddKnot (0.483, 1);

        // make auxPoles match y,z pole data.
        size_t numPoles = curve->GetNumPoles ();
        int auxDimension = 2;
        double *auxData = (double*)BSIBaseGeom::Malloc (sizeof (double) * auxDimension * (int)(numPoles));
        int k = 0;
        for (size_t p = 0; p < numPoles; p++)
            {
            auxData[k++] = curve->poles[p].y;
            auxData[k++] = curve->poles[p].z;
            }
        size_t numKnots1 = curve->GetNumKnots ();
        Check::Size (numKnots0 + 2, numKnots1, "Add 2 knots");
        DPoint3d tolerance = DPoint3d::From (reltol, reltol, reltol);
        bspknotrm_reduceKnots (curve.get (), &auxData, &tolerance, &auxTolerance[0], (int)auxDimension);
        size_t numKnots2 = curve->GetNumKnots ();
        bvector<DPoint3d> newPoles;
        curve->GetPoles (newPoles);
        k = 0;
        for (size_t p = 0; p < curve->GetNumPoles (); p++)
            {
            Check::Near (curve->poles[p].y, auxData[k++], "aux y");
            Check::Near (curve->poles[p].z, auxData[k++], "aux y");
            }
        BSIBaseGeom::Free (auxData);
        Check::Size (numKnots0, numKnots2, "Remove 2 knots");
        Check::Near (poles, newPoles, "poles returned to normal");
        Check::EndScope();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Bspline, IsParabola)
        {
        bvector<DPoint3d> points;
        points.push_back (DPoint3d::From (1,1,0));
        points.push_back (DPoint3d::From (2,1,0));
        points.push_back (DPoint3d::From (4, 2,0));

        MSBsplineCurvePtr bcurve3 = MSBsplineCurve::CreateFromPolesAndOrder (points, NULL, NULL, 3, false, false);
        Transform worldToLocal3, localToWorld3;
        DPoint3d pointA3, pointB3;
        double t3, c3;
        Check::True (bcurve3->IsParabola (localToWorld3, worldToLocal3, t3, pointA3, pointB3, c3), "parabola system");
        for (double tSplit = 0.1; tSplit < 0.99; tSplit += 0.25)
            {
            Transform worldToLocal2, localToWorld2;
            DPoint3d shoulder0 = DPoint3d::FromInterpolate (points[0], tSplit, points[1]);
            DPoint3d shoulder1 = DPoint3d::FromInterpolate (points[1], tSplit, points[2]);
            DPoint3d point01 = DPoint3d::FromInterpolate (shoulder0, tSplit, shoulder1);
            bvector<DPoint3d> points2;
            points2.push_back (point01);
            points2.push_back (shoulder1);
            points2.push_back (points[2]);

            DPoint3d pointA2, pointB2;
            double t2, c2;
            MSBsplineCurvePtr bcurve2 = MSBsplineCurve::CreateFromPolesAndOrder (points2, NULL, NULL, 3, false, false);

            Check::True (bcurve2->IsParabola (localToWorld2, worldToLocal2, t2, pointA2, pointB2, c2), "parabola system");
            Check::Near (c2, c3, "parabola coefficients");
            Check::Near (localToWorld2, localToWorld3, "parabola frames");
            }
        }


void AppendBoundary100 (MSBsplineSurfaceR surface)
    {
    bvector<DPoint2d> boundary;
    boundary.push_back (DPoint2d::From (1.000000, 1.000000));
    boundary.push_back (DPoint2d::From (0.978836, 1.000000));
    boundary.push_back (DPoint2d::From (0.957671, 1.000000));
    boundary.push_back (DPoint2d::From (0.936507, 1.000000));
    boundary.push_back (DPoint2d::From (0.915342, 1.000000));
    boundary.push_back (DPoint2d::From (0.894178, 1.000000));
    boundary.push_back (DPoint2d::From (0.873014, 1.000000));
    boundary.push_back (DPoint2d::From (0.851849, 1.000000));
    boundary.push_back (DPoint2d::From (0.830685, 1.000000));
    boundary.push_back (DPoint2d::From (0.809521, 1.000000));
    boundary.push_back (DPoint2d::From (0.788356, 1.000000));
    boundary.push_back (DPoint2d::From (0.767192, 1.000000));
    boundary.push_back (DPoint2d::From (0.746027, 1.000000));
    boundary.push_back (DPoint2d::From (0.724863, 1.000000));
    boundary.push_back (DPoint2d::From (0.703699, 1.000000));
    boundary.push_back (DPoint2d::From (0.682534, 1.000000));
    boundary.push_back (DPoint2d::From (0.661370, 1.000000));
    boundary.push_back (DPoint2d::From (0.659877, 0.995193));
    boundary.push_back (DPoint2d::From (0.658273, 0.990450));
    boundary.push_back (DPoint2d::From (0.656559, 0.985773));
    boundary.push_back (DPoint2d::From (0.654734, 0.981160));
    boundary.push_back (DPoint2d::From (0.652798, 0.976611));
    boundary.push_back (DPoint2d::From (0.650751, 0.972128));
    boundary.push_back (DPoint2d::From (0.648593, 0.967709));
    boundary.push_back (DPoint2d::From (0.646325, 0.963355));
    boundary.push_back (DPoint2d::From (0.643946, 0.959066));
    boundary.push_back (DPoint2d::From (0.641426, 0.954792));
    boundary.push_back (DPoint2d::From (0.638588, 0.950269));
    boundary.push_back (DPoint2d::From (0.635638, 0.945854));
    boundary.push_back (DPoint2d::From (0.632575, 0.941550));
    boundary.push_back (DPoint2d::From (0.629401, 0.937355));
    boundary.push_back (DPoint2d::From (0.626114, 0.933269));
    boundary.push_back (DPoint2d::From (0.622714, 0.929293));
    boundary.push_back (DPoint2d::From (0.619203, 0.925426));
    boundary.push_back (DPoint2d::From (0.615579, 0.921669));
    boundary.push_back (DPoint2d::From (0.611843, 0.918022));
    boundary.push_back (DPoint2d::From (0.607875, 0.914378));
    boundary.push_back (DPoint2d::From (0.603446, 0.910565));
    boundary.push_back (DPoint2d::From (0.598918, 0.906928));
    boundary.push_back (DPoint2d::From (0.594293, 0.903467));
    boundary.push_back (DPoint2d::From (0.589568, 0.900181));
    boundary.push_back (DPoint2d::From (0.584745, 0.897071));
    boundary.push_back (DPoint2d::From (0.579824, 0.894136));
    boundary.push_back (DPoint2d::From (0.574804, 0.891377));
    boundary.push_back (DPoint2d::From (0.569686, 0.888793));
    boundary.push_back (DPoint2d::From (0.564469, 0.886385));
    boundary.push_back (DPoint2d::From (0.558917, 0.884058));
    boundary.push_back (DPoint2d::From (0.552909, 0.881800));
    boundary.push_back (DPoint2d::From (0.546856, 0.879794));
    boundary.push_back (DPoint2d::From (0.540758, 0.878038));
    boundary.push_back (DPoint2d::From (0.534615, 0.876533));
    boundary.push_back (DPoint2d::From (0.528427, 0.875278));
    boundary.push_back (DPoint2d::From (0.522194, 0.874274));
    boundary.push_back (DPoint2d::From (0.515916, 0.873521));
    boundary.push_back (DPoint2d::From (0.509593, 0.873019));
    boundary.push_back (DPoint2d::From (0.503225, 0.872767));
    boundary.push_back (DPoint2d::From (0.496825, 0.872766));
    boundary.push_back (DPoint2d::From (0.490461, 0.873016));
    boundary.push_back (DPoint2d::From (0.484142, 0.873515));
    boundary.push_back (DPoint2d::From (0.477867, 0.874266));
    boundary.push_back (DPoint2d::From (0.471638, 0.875266));
    boundary.push_back (DPoint2d::From (0.465453, 0.876517));
    boundary.push_back (DPoint2d::From (0.459314, 0.878019));
    boundary.push_back (DPoint2d::From (0.453219, 0.879771));
    boundary.push_back (DPoint2d::From (0.447169, 0.881773));
    boundary.push_back (DPoint2d::From (0.441164, 0.884026));
    boundary.push_back (DPoint2d::From (0.435610, 0.886350));
    boundary.push_back (DPoint2d::From (0.430388, 0.888758));
    boundary.push_back (DPoint2d::From (0.425265, 0.891341));
    boundary.push_back (DPoint2d::From (0.420240, 0.894099));
    boundary.push_back (DPoint2d::From (0.415314, 0.897034));
    boundary.push_back (DPoint2d::From (0.410487, 0.900145));
    boundary.push_back (DPoint2d::From (0.405758, 0.903431));
    boundary.push_back (DPoint2d::From (0.401127, 0.906893));
    boundary.push_back (DPoint2d::From (0.396596, 0.910531));
    boundary.push_back (DPoint2d::From (0.392162, 0.914344));
    boundary.push_back (DPoint2d::From (0.388202, 0.917979));
    boundary.push_back (DPoint2d::From (0.384477, 0.921614));
    boundary.push_back (DPoint2d::From (0.380863, 0.925357));
    boundary.push_back (DPoint2d::From (0.377360, 0.929209));
    boundary.push_back (DPoint2d::From (0.373969, 0.933170));
    boundary.push_back (DPoint2d::From (0.370689, 0.937240));
    boundary.push_back (DPoint2d::From (0.367521, 0.941418));
    boundary.push_back (DPoint2d::From (0.364465, 0.945706));
    boundary.push_back (DPoint2d::From (0.361520, 0.950102));
    boundary.push_back (DPoint2d::From (0.358686, 0.954608));
    boundary.push_back (DPoint2d::From (0.356153, 0.958893));
    boundary.push_back (DPoint2d::From (0.353759, 0.963199));
    boundary.push_back (DPoint2d::From (0.351477, 0.967570));
    boundary.push_back (DPoint2d::From (0.349306, 0.972007));
    boundary.push_back (DPoint2d::From (0.347248, 0.976509));
    boundary.push_back (DPoint2d::From (0.345301, 0.981076));
    boundary.push_back (DPoint2d::From (0.343465, 0.985709));
    boundary.push_back (DPoint2d::From (0.341742, 0.990407));
    boundary.push_back (DPoint2d::From (0.340130, 0.995171));
    boundary.push_back (DPoint2d::From (0.338630, 1.000000));
    boundary.push_back (DPoint2d::From (0.317466, 1.000000));
    boundary.push_back (DPoint2d::From (0.296301, 1.000000));
    boundary.push_back (DPoint2d::From (0.275137, 1.000000));
    boundary.push_back (DPoint2d::From (0.253973, 1.000000));
    boundary.push_back (DPoint2d::From (0.232808, 1.000000));
    boundary.push_back (DPoint2d::From (0.211644, 1.000000));
    boundary.push_back (DPoint2d::From (0.190479, 1.000000));
    boundary.push_back (DPoint2d::From (0.169315, 1.000000));
    boundary.push_back (DPoint2d::From (0.148151, 1.000000));
    boundary.push_back (DPoint2d::From (0.126986, 1.000000));
    boundary.push_back (DPoint2d::From (0.105822, 1.000000));
    boundary.push_back (DPoint2d::From (0.084658, 1.000000));
    boundary.push_back (DPoint2d::From (0.063493, 1.000000));
    boundary.push_back (DPoint2d::From (0.042329, 1.000000));
    boundary.push_back (DPoint2d::From (0.021164, 1.000000));
    boundary.push_back (DPoint2d::From (0.000000, 1.000000));
    boundary.push_back (DPoint2d::From (0.000000, 0.979167));
    boundary.push_back (DPoint2d::From (0.000000, 0.958333));
    boundary.push_back (DPoint2d::From (0.000000, 0.937500));
    boundary.push_back (DPoint2d::From (0.000000, 0.916667));
    boundary.push_back (DPoint2d::From (0.000000, 0.895833));
    boundary.push_back (DPoint2d::From (0.000000, 0.875000));
    boundary.push_back (DPoint2d::From (0.000000, 0.854167));
    boundary.push_back (DPoint2d::From (0.000000, 0.833333));
    boundary.push_back (DPoint2d::From (0.000000, 0.812500));
    boundary.push_back (DPoint2d::From (0.000000, 0.791667));
    boundary.push_back (DPoint2d::From (0.000000, 0.770833));
    boundary.push_back (DPoint2d::From (0.000000, 0.750000));
    boundary.push_back (DPoint2d::From (0.000000, 0.729167));
    boundary.push_back (DPoint2d::From (0.000000, 0.708333));
    boundary.push_back (DPoint2d::From (0.000000, 0.687500));
    boundary.push_back (DPoint2d::From (0.000000, 0.666667));
    boundary.push_back (DPoint2d::From (0.000000, 0.645833));
    boundary.push_back (DPoint2d::From (0.000000, 0.625000));
    boundary.push_back (DPoint2d::From (0.000000, 0.604167));
    boundary.push_back (DPoint2d::From (0.000000, 0.583333));
    boundary.push_back (DPoint2d::From (0.000000, 0.562500));
    boundary.push_back (DPoint2d::From (0.000000, 0.541667));
    boundary.push_back (DPoint2d::From (0.000000, 0.520833));
    boundary.push_back (DPoint2d::From (0.000000, 0.500000));
    boundary.push_back (DPoint2d::From (0.000000, 0.479167));
    boundary.push_back (DPoint2d::From (0.000000, 0.458333));
    boundary.push_back (DPoint2d::From (0.000000, 0.437500));
    boundary.push_back (DPoint2d::From (0.000000, 0.416667));
    boundary.push_back (DPoint2d::From (0.000000, 0.395833));
    boundary.push_back (DPoint2d::From (0.000000, 0.375000));
    boundary.push_back (DPoint2d::From (0.000000, 0.354167));
    boundary.push_back (DPoint2d::From (0.000000, 0.333333));
    boundary.push_back (DPoint2d::From (0.000000, 0.312500));
    boundary.push_back (DPoint2d::From (0.000000, 0.291667));
    boundary.push_back (DPoint2d::From (0.000000, 0.270833));
    boundary.push_back (DPoint2d::From (0.000000, 0.250000));
    boundary.push_back (DPoint2d::From (0.000000, 0.229167));
    boundary.push_back (DPoint2d::From (0.000000, 0.208333));
    boundary.push_back (DPoint2d::From (0.000000, 0.187500));
    boundary.push_back (DPoint2d::From (0.000000, 0.166667));
    boundary.push_back (DPoint2d::From (0.000000, 0.145833));
    boundary.push_back (DPoint2d::From (0.000000, 0.125000));
    boundary.push_back (DPoint2d::From (0.000000, 0.104167));
    boundary.push_back (DPoint2d::From (0.000000, 0.083333));
    boundary.push_back (DPoint2d::From (0.000000, 0.062500));
    boundary.push_back (DPoint2d::From (0.000000, 0.041667));
    boundary.push_back (DPoint2d::From (0.000000, 0.020833));
    boundary.push_back (DPoint2d::From (0.000000, 0.000000));
    boundary.push_back (DPoint2d::From (0.020833, 0.000000));
    boundary.push_back (DPoint2d::From (0.041667, 0.000000));
    boundary.push_back (DPoint2d::From (0.062500, 0.000000));
    boundary.push_back (DPoint2d::From (0.083333, 0.000000));
    boundary.push_back (DPoint2d::From (0.104167, 0.000000));
    boundary.push_back (DPoint2d::From (0.125000, 0.000000));
    boundary.push_back (DPoint2d::From (0.145833, 0.000000));
    boundary.push_back (DPoint2d::From (0.166667, 0.000000));
    boundary.push_back (DPoint2d::From (0.187500, 0.000000));
    boundary.push_back (DPoint2d::From (0.208333, 0.000000));
    boundary.push_back (DPoint2d::From (0.229167, 0.000000));
    boundary.push_back (DPoint2d::From (0.250000, 0.000000));
    boundary.push_back (DPoint2d::From (0.270833, 0.000000));
    boundary.push_back (DPoint2d::From (0.291667, 0.000000));
    boundary.push_back (DPoint2d::From (0.312500, 0.000000));
    boundary.push_back (DPoint2d::From (0.333333, 0.000000));
    boundary.push_back (DPoint2d::From (0.354167, 0.000000));
    boundary.push_back (DPoint2d::From (0.375000, 0.000000));
    boundary.push_back (DPoint2d::From (0.395833, 0.000000));
    boundary.push_back (DPoint2d::From (0.416667, 0.000000));
    boundary.push_back (DPoint2d::From (0.437500, 0.000000));
    boundary.push_back (DPoint2d::From (0.458333, 0.000000));
    boundary.push_back (DPoint2d::From (0.479167, 0.000000));
    boundary.push_back (DPoint2d::From (0.500000, 0.000000));
    boundary.push_back (DPoint2d::From (0.520833, 0.000000));
    boundary.push_back (DPoint2d::From (0.541667, 0.000000));
    boundary.push_back (DPoint2d::From (0.562500, 0.000000));
    boundary.push_back (DPoint2d::From (0.583333, 0.000000));
    boundary.push_back (DPoint2d::From (0.604167, 0.000000));
    boundary.push_back (DPoint2d::From (0.625000, 0.000000));
    boundary.push_back (DPoint2d::From (0.645833, 0.000000));
    boundary.push_back (DPoint2d::From (0.666667, 0.000000));
    boundary.push_back (DPoint2d::From (0.687500, 0.000000));
    boundary.push_back (DPoint2d::From (0.708333, 0.000000));
    boundary.push_back (DPoint2d::From (0.729167, 0.000000));
    boundary.push_back (DPoint2d::From (0.750000, 0.000000));
    boundary.push_back (DPoint2d::From (0.770833, 0.000000));
    boundary.push_back (DPoint2d::From (0.791667, 0.000000));
    boundary.push_back (DPoint2d::From (0.812500, 0.000000));
    boundary.push_back (DPoint2d::From (0.833333, 0.000000));
    boundary.push_back (DPoint2d::From (0.854167, 0.000000));
    boundary.push_back (DPoint2d::From (0.875000, 0.000000));
    boundary.push_back (DPoint2d::From (0.895833, 0.000000));
    boundary.push_back (DPoint2d::From (0.916667, 0.000000));
    boundary.push_back (DPoint2d::From (0.937500, 0.000000));
    boundary.push_back (DPoint2d::From (0.958333, 0.000000));
    boundary.push_back (DPoint2d::From (0.979167, 0.000000));
    boundary.push_back (DPoint2d::From (1.000000, 0.000000));
    boundary.push_back (DPoint2d::From (1.000000, 0.020833));
    boundary.push_back (DPoint2d::From (1.000000, 0.041667));
    boundary.push_back (DPoint2d::From (1.000000, 0.062500));
    boundary.push_back (DPoint2d::From (1.000000, 0.083333));
    boundary.push_back (DPoint2d::From (1.000000, 0.104167));
    boundary.push_back (DPoint2d::From (1.000000, 0.125000));
    boundary.push_back (DPoint2d::From (1.000000, 0.145833));
    boundary.push_back (DPoint2d::From (1.000000, 0.166667));
    boundary.push_back (DPoint2d::From (1.000000, 0.187500));
    boundary.push_back (DPoint2d::From (1.000000, 0.208333));
    boundary.push_back (DPoint2d::From (1.000000, 0.229167));
    boundary.push_back (DPoint2d::From (1.000000, 0.250000));
    boundary.push_back (DPoint2d::From (1.000000, 0.270833));
    boundary.push_back (DPoint2d::From (1.000000, 0.291667));
    boundary.push_back (DPoint2d::From (1.000000, 0.312500));
    boundary.push_back (DPoint2d::From (1.000000, 0.333333));
    boundary.push_back (DPoint2d::From (1.000000, 0.354167));
    boundary.push_back (DPoint2d::From (1.000000, 0.375000));
    boundary.push_back (DPoint2d::From (1.000000, 0.395833));
    boundary.push_back (DPoint2d::From (1.000000, 0.416667));
    boundary.push_back (DPoint2d::From (1.000000, 0.437500));
    boundary.push_back (DPoint2d::From (1.000000, 0.458333));
    boundary.push_back (DPoint2d::From (1.000000, 0.479167));
    boundary.push_back (DPoint2d::From (1.000000, 0.500000));
    boundary.push_back (DPoint2d::From (1.000000, 0.520833));
    boundary.push_back (DPoint2d::From (1.000000, 0.541667));
    boundary.push_back (DPoint2d::From (1.000000, 0.562500));
    boundary.push_back (DPoint2d::From (1.000000, 0.583333));
    boundary.push_back (DPoint2d::From (1.000000, 0.604167));
    boundary.push_back (DPoint2d::From (1.000000, 0.625000));
    boundary.push_back (DPoint2d::From (1.000000, 0.645833));
    boundary.push_back (DPoint2d::From (1.000000, 0.666667));
    boundary.push_back (DPoint2d::From (1.000000, 0.687500));
    boundary.push_back (DPoint2d::From (1.000000, 0.708333));
    boundary.push_back (DPoint2d::From (1.000000, 0.729167));
    boundary.push_back (DPoint2d::From (1.000000, 0.750000));
    boundary.push_back (DPoint2d::From (1.000000, 0.770833));
    boundary.push_back (DPoint2d::From (1.000000, 0.791667));
    boundary.push_back (DPoint2d::From (1.000000, 0.812500));
    boundary.push_back (DPoint2d::From (1.000000, 0.833333));
    boundary.push_back (DPoint2d::From (1.000000, 0.854167));
    boundary.push_back (DPoint2d::From (1.000000, 0.875000));
    boundary.push_back (DPoint2d::From (1.000000, 0.895833));
    boundary.push_back (DPoint2d::From (1.000000, 0.916667));
    boundary.push_back (DPoint2d::From (1.000000, 0.937500));
    boundary.push_back (DPoint2d::From (1.000000, 0.958333));
    boundary.push_back (DPoint2d::From (1.000000, 0.979167));
    boundary.push_back (DPoint2d::From (1.000000, 1.000000));
    surface.AddTrimBoundary (boundary);
    }

MSBsplineSurfacePtr CreateGridSurface (DPoint3dCR origin, double dx, double dy, size_t order, size_t numX, size_t numY)
    {
    bvector<DPoint3d> poles;
    for (size_t j = 0; j < numY; j++)
        {
        for (size_t i = 0; i < numX; i++)
            poles.push_back (DPoint3d::From (origin.x + i * dx, origin.y + j * dy, origin.z));
        }
    return MSBsplineSurface::CreateFromPolesAndOrder (poles,  NULL, 
                NULL, (int)order, (int)numX, false,
                NULL, (int)order, (int)numY, false, true
                );                
    }

void AppendRepeated (bvector<double> &data, size_t n, double value)
    {
    for (size_t i = 0; i < n; i++)
        data.push_back (value);
    }
void BuildKnots (bvector<double>&data, size_t order, size_t numInterval, size_t interiorMultiplicity)
    {
    double dx = 1.0 / (double)numInterval;
    AppendRepeated (data, order, 0.0);
    for(size_t i = 1; i < numInterval; i++)
        AppendRepeated (data, interiorMultiplicity, i * dx);
    AppendRepeated (data, order, 1.0);
    }

// knot interval counts are the "non null" intervals ..
MSBsplineSurfacePtr CreateGridSurfaceWithMultipleKnots (DPoint3dCR origin, double dx, double dy, size_t order, size_t interiorMultiplicity, size_t numKnotIntervalX, size_t numKnotIntervalY)
    {
    bvector<DPoint3d> poles;
    bvector<double> uKnots, vKnots;

    BuildKnots (uKnots, order, numKnotIntervalX, interiorMultiplicity);
    BuildKnots (vKnots, order, numKnotIntervalY, interiorMultiplicity);
    size_t numXPoles = uKnots.size () - order;
    size_t numYPoles = vKnots.size () - order;

    for (size_t j = 0; j < numYPoles; j++)
        {
        for (size_t i = 0; i < numXPoles; i++)
            poles.push_back (DPoint3d::From (origin.x + i * dx, origin.y + j * dy, origin.z));
        }
    return MSBsplineSurface::CreateFromPolesAndOrder (poles,  NULL, 
                &uKnots, (int)order, (int)numXPoles, false,
                &vKnots, (int)order, (int)numYPoles, false, true
                );                
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BsplineSurface, Boundary100)
    {
    double a = 1000.0 / 3.0;
    MSBsplineSurfacePtr surface = CreateGridSurface (DPoint3d::From (0,0,0), a, a, 3, 4, 4);
    AppendBoundary100 (*surface);
    surface->SetOuterBoundaryActive (false);
    CurveVectorPtr curves = surface->GetUnstructuredBoundaryCurves (0.0, false, true);
    double length = curves->Length ();
    Check::True (length > 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BSurfPatch,UniformKnotIntervals)
    {
    for (size_t order = 2; order < 5; order++)
        {
        size_t numX = 5, numY = 7;
        MSBsplineSurfacePtr surface = CreateGridSurface (DPoint3d::From(0,0,0), 10,10, order, numX + order - 1, numY + order - 1);
        bvector<double> uKnots, vKnots;
        surface->GetUKnots (uKnots);
        surface->GetVKnots (vKnots);
        BSurfPatch patch;
        double dx = 1.0 / (double)numX;
        double dy = 1.0 / (double)numY;
        for (size_t i = 0; i < numX; i++)
            {
            Check::False (surface->GetPatch (patch, i, numY + 1), "Reject invalid patch");
            }

        for (size_t j = 0; j < numX; j++)
            {
            Check::False (surface->GetPatch (patch, numX + 1, j), "Reject invalid patch");
            }



        for (size_t i = 0; i < numX; i++)
            {
            for (size_t j = 0; j < numY; j++)
                {
                if (Check::True (surface->GetPatch (patch, i, j)))
                    {
                    Check::Near (i * dx, patch.uMin);
                    Check::Near ((i+1) * dx, patch.uMax);
                    Check::Near (j * dy, patch.vMin);
                    Check::Near ((j+1) * dy, patch.vMax);
                    }
                }
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BSurfPatch,DoubledKnotIntervals)
    {
    for (size_t order = 3; order < 5; order++)
        {
        size_t numXKnotInterval = 5, numYKnotInterval = 7;
        MSBsplineSurfacePtr surface = CreateGridSurfaceWithMultipleKnots (
                    DPoint3d::From(0,0,0),
                    10,10,
                    order, 2,
                    numXKnotInterval, numYKnotInterval);
        auto numXPoles = surface->GetNumUPoles ();
        auto numYPoles = surface->GetNumVPoles ();
        bvector<double> uKnots, vKnots;
        surface->GetUKnots (uKnots);
        surface->GetVKnots (vKnots);
        BSurfPatch patch;
        double dx = 1.0 / (double)numXKnotInterval;
        double dy = 1.0 / (double)numYKnotInterval;
        for (size_t i = 0; i + 1 < numXPoles; i++)
            {
            Check::False (surface->GetPatch (patch, i, numYPoles), "Reject invalid patch");
            }

        for (size_t j = 0; j + 1 < numYPoles; j++)
            {
            Check::False (surface->GetPatch (patch, numXPoles, j), "Reject invalid patch");
            }

        for (size_t i = 0; i + order < numXPoles + 1; i++)
            {
            for (size_t j = 0; j + order < numYPoles + 1; j++)
                {
                char message [1024];
                sprintf (message, " interval %d %d\n", (int)i, (int)j);
                Check::Print (message);
                bool nullI = 0 != (i & 0x01);
                bool nullJ = 0 != (j & 0x01);
                if (Check::True (surface->GetPatch (patch, i, j)))
                    {
                    Check::Near (patch.uMin, patch.uKnots[order-2]);
                    Check::Near (patch.uMax, patch.uKnots[order-1]);

                    Check::Near (patch.vMin, patch.vKnots[order-2]);
                    Check::Near (patch.vMax, patch.vKnots[order-1]);

                    // with multiplicity 2 interior knots, every other knot interval is null ...
                    if (Check::Bool (nullI, patch.isNullU)
                        && Check::Bool (nullJ, patch.isNullV)
                        )
                        {
                        if (nullI || nullJ)
                            {

                            }
                        else
                            {
                            size_t ii = i / 2;
                            size_t jj = j / 2;
                            Check::Near (ii * dx, patch.uMin);
                            Check::Near ((ii+1) * dx, patch.uMax);
                            Check::Near (jj * dy, patch.vMin);
                            Check::Near ((jj+1) * dy, patch.vMax);
                            }
                        }
                    }
                }
            }
        }
    }

void TestFastIntersectCurveSurface (MSBsplineSurfacePtr &surface)
    {
    PatchRangeDataPtr surfaceCache;     // defaults to uninitialized !!!
    double gU = 0.5;
    double gV = 0.75;   // Fractions within control grid polygons.
    size_t numUEdge = surface->GetNumUPoles () + 1 - surface->GetUOrder ();
    size_t numVEdge = surface->GetNumVPoles () + 1 - surface->GetVOrder ();
    double xyzTolerance = 1.0e-8;
    bvector<int> doCurve {1,1, 1}; // For selective testing : line, arc, ....

    for (size_t i = 0; i < numUEdge; i++)
        {
        Check::StartScope ("Grid i", i);
        for (size_t j = 0; j  < numVEdge; j++)
            {
            Check::StartScope ("Grid j", j);
            auto uv0 = surface->ControlPolygonFractionToKnot (i,j,gU,gV);
            DPoint3d xyz0;
            surface->EvaluatePoint (xyz0, uv0.Value().x, uv0.Value().y);
            // xyz0 is on the surface .. ..
            DPoint3d xyzA = xyz0, xyzB = xyz0;
            // xyzA,xyzB here are directly above and below ...
            xyzA.z += 4;
            xyzB.z -= 1;

            DSegment3d segment = DSegment3d::From (xyzA, xyzB);
            bvector<ICurvePrimitivePtr> curves;
            curves.push_back (ICurvePrimitive::CreateLine (segment));

            // now shift to the side and build an arc ...
            xyzA.x += 1;
            xyzB.x += 1;
            DEllipse3d arc = DEllipse3d::FromPointsOnArc (xyzA, xyz0, xyzB);


            curves.push_back (ICurvePrimitive::CreateArc (arc));

            auto bcurve = curves.back()->GetMSBsplineCurvePtr ();
            curves.push_back (ICurvePrimitive::CreateBsplineCurve (bcurve));

            for (size_t k = 0; k < curves.size (); k++)
                {
                if (doCurve[k] != 0)
                    {
                    ICurvePrimitivePtr curve = curves[k];
                    Check::StartScope ("Curve",  k);
                    bvector<CurveAndSolidLocationDetail> intersections;
                    surface->FastIntersectCurve (*curve, surfaceCache, intersections);
                    size_t numMatch = 0;
                    for (auto &hit : intersections)
                        {
                        if (hit.m_curveDetail.point.Distance (xyz0) <= xyzTolerance)
                            numMatch++;
                        }
                    Check::True (numMatch > 0);
                    Check::EndScope ();
                    }

                }
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MSBsplineSurface,FastIntersectCurve0)
    {
    double q0I = 0.0;
    double q0J = 0.0;

    for (size_t order = 2; order < 5; order++)
        {
        auto surface0 = CreateGridSurface (DPoint3d::From (0,0,0), 2.0, 4.0, order, 5,6);
        Check::StartScope ("Planar surface order", order);
        TestFastIntersectCurveSurface (surface0);
        Check::EndScope ();
        auto surface1 = SurfaceWithSinusoidalControlPolygon ((int)order, (int)order, 5, 7, q0I, 0.22, q0J, 0.1);
        Check::StartScope ("Sinusoidal surface order", order);
        TestFastIntersectCurveSurface (surface1);
        Check::EndScope ();
        }
    }



struct MSBsplineSurfaceProjectionFunction : FunctionRRToRR
{
MSBsplineSurfaceCR m_surface;
DPoint3d m_spacePoint;
MSBsplineSurfaceProjectionFunction (MSBsplineSurfaceCR surface) : m_surface (surface)
    {
    }
void SetSpacePoint (DPoint3dCR xyz) {m_spacePoint = xyz;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool EvaluateRRToRR
(
double u,
double v,
double &f,
double &g
) override
    {
    DPoint3d xyz;
    DVec3d U, V, W;
    m_surface.EvaluatePoint (xyz, U, V, u, v);
    W.DifferenceOf (m_spacePoint, xyz);
    f = W.DotProduct (U);
    g = W.DotProduct (V);
    return true;
    }
};


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(BSurfFastProjection,Test1)
    {
    // Place a modestly curved bspline clearly out of the 0..1 range
    MSBsplineSurfacePtr surface = MSBsplineSurface::CreateFromPolesAndOrder
        (
        bvector<DPoint3d>
            {
            DPoint3d::From (10, 10,0),   DPoint3d::From (20,10,0),    DPoint3d::From (30, 10, 2), DPoint3d::From (40,10,2),
            DPoint3d::From (10, 20,0),   DPoint3d::From (20,20,1),    DPoint3d::From (30, 20, 3), DPoint3d::From (40,20,3),
            },
        nullptr,
        nullptr, 3, 4, false,
        nullptr, 2, 2, false,
        true
        );

    DPoint2d uv0 = DPoint2d::From (0.1, 0.2);
    DPoint2d uv1 = DPoint2d::From (0.8, 0.9);
    // Evaluate a bunch of points on the surface:
    double df = 1.0 / 32.0;
    bvector<DPoint2d> knownUV;
    bvector<DPoint3d> knownXYZ;
    for (double f = 0.0; f <= 1.0; f += df)
        {
        DPoint3d xyz;
        DPoint2d uv = DPoint2d::FromInterpolate (uv0, f, uv1);
        surface->EvaluatePoint (xyz, uv.x, uv.y);
        knownUV.push_back (uv);
        knownXYZ.push_back (xyz);
        }

    MSBsplineSurfaceProjectionFunction F (*surface);
    
    double uvTol = 1.0e-10;
    double maxStep = 0.20;
    NewtonIterationsRRToRR newton (uvTol);


    // Use the FIRST point as a seed for iterating to all later ones:
    double u = knownUV.front ().x;
    double v = knownUV.front ().y;
    for (size_t i = 1; i < knownUV.size (); i++)
        {
        F.SetSpacePoint (knownXYZ[i]);
        // The u,v values "now" are from the prior point ... the newton solver will mvoe them forward to the new xyz
        if (!Check::True (newton.RunApproximateNewton (u, v, F, maxStep, maxStep)))
            break;
        if (!Check::Near (u, knownUV[i].x))
            break;
        if (!Check::Near (v, knownUV[i].y))
            break;
        }
   
    }

struct NewtonTestNonLinear3 : FunctionRRRToRRR
{
virtual bool EvaluateRRRToRRR (DVec3dCR uvw, DVec3dR fgh)
    {
    // Test functions:   x=3, x+2y=4, x^3-z = 1
    fgh = DVec3d::From(
        uvw.x - 3.0,
        uvw.x + 2.0 * uvw.y - 4.0,
        0.1 * (uvw.x * uvw.x * uvw.x - uvw.z - 1.0)
        );
    return true;
    }
};

struct NewtonTestLinear3 : FunctionRRRToRRR
    {
    virtual bool EvaluateRRRToRRR(DVec3dCR uvw, DVec3dR fgh)
        {
        // Test functions:   x=3, x+2y=4, x+4y-6z=2
        fgh = DVec3d::From(
            uvw.x - 3.0,
            uvw.x + 2.0 * uvw.y - 4.0,
            uvw.x + 4.0 * uvw.y - 6.0 * uvw.z - 2.0
            );
        return true;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Newton,Approx3)	
    {
    NewtonTestNonLinear3 funcB;
    NewtonTestLinear3 funcA;
    DVec3d dUMax = DVec3d::From (3,3,3);

    DVec3d Ua = DVec3d::From (1,2,3);
    NewtonIterationsRRToRR newton (1.0e-12);
    Check::True (newton.RunApproximateNewton (Ua, funcA, dUMax), "Linear Newton, 3dofs");
    DVec3d Fa, Fb;
    funcA.EvaluateRRRToRRR (Ua, Fa);
    Check::Near (DVec3d::FromZero (), Fa);
    DVec3d Ub = DVec3d::From(1, 2, 3);
    Check::True(newton.RunApproximateNewton (Ub, funcB, dUMax), "Nonlinear Newton, 3dofs");
    funcB.EvaluateRRRToRRR(Ub, Fb);
    Check::Near(DVec3d::FromZero(), Fb);
    }

static double s_maxRadians = 0.05;
// Shared data class for 3-piece hermite various spiral constructions.
struct HermiteSpiral3Data : FunctionRRRToRRR
{
DPoint2d m_xy0;     // given start point
DPoint2d m_xy3;     // given endpoint
double m_curvature0, m_curvature3;  // Given end curvatures
double m_bearing0, m_bearing3; // Given end bearings
DSpiral2dClothoid m_spiral01;   // forwards from m_xy0 to (interior) point1
DSpiral2dClothoid m_spiral12;   // reverse from (interior) point1 to (interior) point2
DSpiral2dClothoid m_spiral23;   // reverse from (interior) point1 to (interior) point2

DPoint2d m_xy[4];   // array of both imposed and computed points
double m_curvature[4];   // array of both imposed and computed curvatures
double m_bearing[4];   // array of both imposed and computed bearings
double m_arcLength[3]; // per-spiral arc length

size_t m_evaluations;
HermiteSpiral3Data(DPoint2dCR xyA, DPoint2dCR xyB, double betaA, double betaB, double curvatureA, double curvatureB)
    : m_xy0 (xyA),
      m_xy3 (xyB),
      m_bearing0 (betaA),
      m_bearing3 (betaB),
      m_curvature0 (curvatureA),
      m_curvature3(curvatureB),
      m_spiral01 (),
      m_spiral12 (),
      m_spiral23 ()
    {
    }

// On input the following are assumed to be assigned:
//  1) m_xy[index0], m_bearing[index0]
//  2) m_curvature[index0] and m_curvature[index1]
//  3) m_arLength[index0] 
// (with index1 = index0 + 1)
// Todo:
//   1) "Set" data in clothoid
//   2) compute m_xy[index1], m_bearing[index1]
//
void ComputeStep (uint32_t index0, DSpiral2dClothoid &clothoid)
    {
    int index1 = index0 + 1;
    clothoid.SetBearingCurvatureLengthCurvature
            (
            m_bearing[index0],
            m_curvature[index0],
            m_arcLength[index0],
            m_curvature[index1]
            );
    m_bearing[index1] = clothoid.mTheta1;   // The "set" computed this
    DVec2d delta;
    double error;
    DSpiral2dBase::Stroke(clothoid, 0.0, 1.0, s_maxRadians, delta, error);
    m_xy[index1] = DPoint2d::FromSumOf (m_xy[index0], delta, 1.0);
    }

void ClearCounters ()   { m_evaluations = 0;}
void RecordEvaluation (DVec3dCR uvw) {m_evaluations++;}
virtual char const *SolverName () const {return "HermiteSpiral3dData";}
virtual bool InitialGuess(DVec3dR uvw){ uvw.Zero (); return false;}

};


struct HermiteSpiralEqualArcLengthFunction : HermiteSpiral3Data
    {
    virtual char const *SolverName() const { return "HermiteSpiralEqualArcLengthFunction"; }

    HermiteSpiralEqualArcLengthFunction(DPoint2dCR xyA, DPoint2dCR xyB, double betaA, double betaB, double curvatureA, double curvatureB)
        : HermiteSpiral3Data (xyA, xyB, betaA, betaB, curvatureA, curvatureB)
        {
        }
    // Compute an intial guess at the three degrees of freedom --
    // uvw.x = curvature at interior point 1    (at 1/3 of total arc length)
    // uvw.y = curvature at interior point 2    (at 2/3 of total arc length)
    // uvw.z = total arc length
    virtual bool InitialGuess(DVec3dR uvw)
        {
        uvw.x = DoubleOps::Interpolate(m_curvature0, 1.0 / 3.0, m_curvature3);
        uvw.y = DoubleOps::Interpolate(m_curvature0, 2.0 / 3.0, m_curvature3);
        uvw.z = m_xy0.Distance(m_xy3);
        ClearCounters();
        return true;
        }


    virtual bool EvaluateRRRToRRR(DVec3dCR uvw, DVec3dR fgh)
        {
        RecordEvaluation (uvw);
        // Save all curvatures ...
        m_curvature[0] = m_curvature0;
        m_curvature[1] = uvw.x;
        m_curvature[2] = uvw.y;
        m_curvature[3] = m_curvature3;
        double totalLength = uvw.z;
        m_arcLength[0] = totalLength / 3.0;
        m_arcLength[1] = totalLength / 3.0;
        m_arcLength[2] = totalLength - (m_arcLength[0] + m_arcLength[1]);
        // Initialize first bearing and point:
        m_xy[0] = m_xy0;
        m_bearing[0] = m_bearing0;
        ComputeStep(0, m_spiral01);
        ComputeStep(1, m_spiral12);
        ComputeStep(2, m_spiral23);
        fgh.x = m_xy[3].x - m_xy3.x;
        fgh.y = m_xy[3].y - m_xy3.y;
        fgh.z = m_bearing[3] - m_bearing3;
        return true;
        }

    };

struct HermiteSpiralMonotoneCurvatureFunction: HermiteSpiral3Data
    {
    virtual char const *SolverName() const { return "HermiteSpiralMonotoneCurvatureFunction "; }
    
    HermiteSpiralMonotoneCurvatureFunction(DPoint2dCR xyA, DPoint2dCR xyB, double betaA, double betaB, double curvatureA, double curvatureB)
        : HermiteSpiral3Data(xyA, xyB, betaA, betaB, curvatureA, curvatureB)
        {
        }
    // Compute an intial guess at the three arc lengths
    virtual bool InitialGuess(DVec3dR uvw)
        {
        double a = m_xy0.Distance(m_xy3) / 3.0;
        uvw.x = a;
        uvw.y = a;
        uvw.z = a;
        ClearCounters ();
        return true;
        }


    virtual bool EvaluateRRRToRRR(DVec3dCR uvw, DVec3dR fgh)
        {
        RecordEvaluation(uvw);
        // Set monotone curvatures
        m_curvature[0] = m_curvature0;
        m_curvature[1] = DoubleOps::Interpolate (m_curvature0, 1.0 / 3.0, m_curvature3);
        m_curvature[2] = DoubleOps::Interpolate(m_curvature0, 2.0 / 3.0, m_curvature3);
        m_curvature[3] = m_curvature3;
        // The candiate uvw has 3 arc lengths ...
        m_arcLength[0] = uvw.x;
        m_arcLength[1] = uvw.y;
        m_arcLength[2] = uvw.z;
        // Initialize first bearing and point:
        m_xy[0] = m_xy0;
        m_bearing[0] = m_bearing0;
        ComputeStep(0, m_spiral01);
        ComputeStep(1, m_spiral12);
        ComputeStep(2, m_spiral23);
        fgh.x = m_xy[3].x - m_xy3.x;
        fgh.y = m_xy[3].y - m_xy3.y;
        fgh.z = m_bearing[3] - m_bearing3;
        return true;
        }

    };

void SaveSpiral(DPoint2dCR origin, DSpiral2dBaseCR spiral)
    {
    auto cp = ICurvePrimitive::CreateSpiral
        (
            spiral,
            Transform::From (DPoint3d::From (origin)),
            0.0, 1.0
            );
    Check::SaveTransformed(*cp);
    }

void RunSpliceSolverTest (HermiteSpiral3Data &spliceFunction, DVec3dCR maxDU)
    {
    bool doPrint = Check::PrintDeepStructs ();
    NewtonIterationsRRToRR newton(1.0e-12);
    DVec3d U;
    spliceFunction.InitialGuess(U);
    if (doPrint)
        {
        printf("***Hermite spline solution (%s)\n",
            spliceFunction.SolverName());
        printf ("  (xyA %.17g,%.17g) (kA %.17g) (betaA %.17g)\n",
                spliceFunction.m_xy0.x,
                spliceFunction.m_xy0.y,
                spliceFunction.m_curvature0,
                spliceFunction.m_bearing0);
        printf("  (xyB %.17g,%.17g) (kB %.17g) (betaB %.17g)\n",
            spliceFunction.m_xy3.x,
            spliceFunction.m_xy3.y,
            spliceFunction.m_curvature3,
            spliceFunction.m_bearing3
            );
        }
    // If the solver succeeds, we print and Check:: its results.
    // If the solver fails, we print a notice but do NOT call it a "Check::True" failure.   
    if (newton.RunApproximateNewton(U, spliceFunction, maxDU))
        {
        if (doPrint)
            printf("   (%d evaluations)\n",
                (int)spliceFunction.m_evaluations);

        DVec3d F;
        spliceFunction.EvaluateRRRToRRR(U, F);
        if (doPrint)
            {
            for (int i = 0; i < 3; i++)
                {
                printf(" (D %.17lg) (curvature %.16g) (dK/dL %.16g)\n",
                    spliceFunction.m_arcLength[i],
                    spliceFunction.m_curvature[i],
                    (spliceFunction.m_curvature[i + 1] - spliceFunction.m_curvature[i]) / spliceFunction.m_arcLength[i]
                    );
                }
            printf(" (curvature %.16g)\n",
                spliceFunction.m_curvature[3]);
            }
        Check::Near (spliceFunction.m_xy3, spliceFunction.m_xy[3], "spliced spiral endpoint");
        Check::Near(spliceFunction.m_bearing3, spliceFunction.m_bearing[3], "spliced spiral endpoint");
        SaveSpiral (spliceFunction.m_xy[0], spliceFunction.m_spiral01);
        SaveSpiral(spliceFunction.m_xy[1], spliceFunction.m_spiral12);
        SaveSpiral(spliceFunction.m_xy[2], spliceFunction.m_spiral23);
        }
    else
        {
        if (doPrint)
            printf ("\n ************************ No solution from newton solver !!!\n\n");
        }
    }

void SaveArcByCurvaturePassThrough (DPoint2d xy, double bearingRadians, double curvature,
    double refSize = 1.0
    )
    {
    ICurvePrimitivePtr cp;
    double c = cos (bearingRadians);
    double s = sin (bearingRadians);
    DVec3d tangent = DVec3d::From(c, s, 0.0);
    auto radial = DVec3d::From(-s, c, 0.0);
    if (curvature == 0.0)
        {
        cp = ICurvePrimitive::CreateLine (
                DSegment3d::From (
                    DPoint3d::From (xy),
                    DPoint3d::From(xy) - tangent * refSize
                    )
                );
        Check::SaveTransformed(*cp);
        }
    else
        {
        double radius = 1.0 / curvature;
        DPoint3d center = DPoint3d::From(xy) + radial * radius;
        cp = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (center, radius));
        Check::SaveTransformed (*cp);
        Check::SaveTransformed(*ICurvePrimitive::CreateLine(
            DSegment3d::From(DPoint3d::From(xy), center))
            );
        }
    // ticmark to outside of circle.
    cp = ICurvePrimitive::CreateLine(
        DSegment3d::From(
            DPoint3d::From(xy),
            DPoint3d::From(xy) - radial * refSize * 0.5
            )
        );
    Check::SaveTransformed(*cp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral,HermiteSplice)
    {
    bool doPrint = Check::PrintDeepStructs ();
    //double dy = 3.0;
    double radiusB = 100.0;
    double lengthAB = 40.0;
    for (DVec2d endPointFactor : bvector<DVec2d> {
            DVec2d::From (1.0, 1.0),        // The input is perfect clothoid
            DVec2d::From(0.98, 0.98),       // The request is for a quicker turn than the clothoid
            DVec2d::From(1.02, 1.02),        // The request is for a longer turn than the clothoid.
            DVec2d::From(2.0, 2.0),         // really stretch it out.  Don't even know if this should work..
            DVec2d::From(1.1, 1.0),         // stretch x, leave y alone.
            DVec2d::From(1.3, 1.0),         // stretch x, leave y alone.
            DVec2d::From(2.0, 1.0),         // stretch x, leave y alone.
            DVec2d::From(1.0, 1.3)         // stretch y, leave x alone.
        }
            )   
        {
        if (doPrint)
            printf ("\n\n**** (endPointFactor %.17g, %.17g)\n", endPointFactor.x, endPointFactor.y);
        DSpiral2dClothoid clothoid;
        double curvatureA = 0.0;
        double curvatureB = 1.0 / radiusB;
        double bearingA = 0.0;
        clothoid.SetBearingCurvatureLengthCurvature(bearingA, curvatureA, lengthAB, curvatureB);
        double e;
        DVec2d delta;




        DSpiral2dBase::Stroke(clothoid, 0.0, 1.0, s_maxRadians, delta, e);
        DPoint2d xyA = DPoint2d::From(0, 0);
        DPoint2d xyB = DPoint2d::From(
                    xyA.x + delta.x * endPointFactor.x,
                    xyA.y + delta.y * endPointFactor.y
                    );


        DVec3d maxDU;

            {
            SaveArcByCurvaturePassThrough(xyA, clothoid.mTheta0, clothoid.mCurvature0);
            SaveArcByCurvaturePassThrough(xyB, clothoid.mTheta1, clothoid.mCurvature1);


            HermiteSpiralEqualArcLengthFunction spliceFunction
                (
                xyA, xyB,
                clothoid.mTheta0, clothoid.mTheta1,
                curvatureA,
                curvatureB
                );
            double maxdK = DoubleOps::Max(curvatureA, curvatureB);
            maxDU.Init(maxdK, maxdK, xyA.Distance(xyB));
            RunSpliceSolverTest(spliceFunction, maxDU);
            Check::Shift(0.0, 2.1 * radiusB);
        }
        // *****************
            {
            SaveArcByCurvaturePassThrough(xyA, clothoid.mTheta0, clothoid.mCurvature0);
            SaveArcByCurvaturePassThrough(xyB, clothoid.mTheta1, clothoid.mCurvature1);


            HermiteSpiralMonotoneCurvatureFunction spliceFunction
                (
                    xyA, xyB,
                    clothoid.mTheta0, clothoid.mTheta1,
                    curvatureA,
                    curvatureB
                    );
            double ab = xyA.Distance(xyB);
            maxDU.Init(ab, ab, ab);
            RunSpliceSolverTest(spliceFunction, maxDU);
            Check::Shift(0.0, 3.0 * radiusB);
            }
        }    
    Check::ClearGeometry ("Spiral.HermiteSplice");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Spiral, InflectedSpirals)
    {
    // q = (startCurvature, endCurvature) factors
    double refRadius = 100.0;
    double refLength = 100.0;
    double toleranceLength = 1.0; // 1.0e5;        // allow lots of digit fluff . . . "Curve1" spiral would do better !!!
    double dy1 = refLength;
    for (double bearing0 : {0.0, 0.3})
        {
        SaveAndRestoreCheckTransform typeShift(refLength, 0, 0);
        for (auto q : bvector<DPoint2d> {
                    DPoint2d::From (1.0, 2.0),
                    DPoint2d::From (1.0,-2.0),
                    DPoint2d::From (-1.0,2.0),
                    DPoint2d::From (-1.0,-2.0)
                    })
            {
            DSpiral2dClothoid clothoid;
            double k0 = q.x / refRadius;
            double k1 = q.y / refRadius;
            clothoid.SetBearingCurvatureLengthCurvature (bearing0, k0, refLength, k1);
            auto cp = ICurvePrimitive::CreateSpiral
                    (
                    clothoid,
                    Transform::From(DPoint3d::From(0,0,0)),
                    0.0, 1.0
                    );
            double a;
            cp->Length (a);
            Check::Near (refLength, a, "computed spiral length", toleranceLength);
            Check::SaveTransformed (*cp);
            auto proxy = cp->GetProxyBsplineCurvePtr ();
            auto cp1 = ICurvePrimitive::CreateBsplineCurve(proxy);
            Check::SaveTransformed (*cp1);
            Check::SaveTransformed(*cp);
            Check::SaveTransformed (DSegment3d::From (DPoint3d::From (0,0,0), DPoint3d::From (refLength * cos (bearing0), refLength*sin (bearing0))));
            Check::Shift (0, dy1);
            }
        }
    Check::ClearGeometry("Spiral.InflectedSpiral");
    }
char const * bsurfFromLuca = R"([{"bsurf":{"orderU":4, "orderV" : 4, "points" : [[[305994.0, -4504.7299999999996, 24019.330000000002, 1.0], [303977.0, -5994.5309999999999, 24052.840, 1.0], [301913.29999999999, -7337.3140000000003, 24087.110000000001, 1.0], [299719.0, -8595.4860000000026, 24123.560000000001, 1.0]], [[305932.60000000003, -4432.3750, 24189.550000000003, 1.0], [303923.20000000001, -5920.4340000000011, 24211.889999999999, 1.0], [301854.70000000001, -7263.3559999999998, 24234.740000000005, 1.0], [299666.0, -8507.0939999999991, 24259.040000000005, 1.0]], [[305871.40000000002, -4360.0219999999999, 24359.779999999999, 1.0], [303869.40000000002, -5846.3379999999997, 24370.940000000002, 1.0], [301796.20000000001, -7189.3950000000013, 24382.369999999999, 1.0], [299613.0, -8418.6970000000001, 24394.520000000004, 1.0]], [[305810.0, -4287.6690000000008, 24530.0, 1.0], [303815.60000000003, -5772.2410000000009, 24530.0, 1.0], [301737.70000000001, -7115.4340000000002, 24530.0, 1.0], [299560.0, -8330.3000000000011, 24530.0, 1.0]]], "uKnots" : [0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0], "vKnots" : [0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0]}}])";

void AppendSurfacePointTriad
(
    bvector<DPoint3d> &points,
    MSBsplineSurface &surface,
    double u,
    double v,
    double uScale = 0.002,
    double vScale = 0.005,
    double wScale = 0.01    // times geometric mean cross product !
)
    {
    DPoint3d xyzQ;
    DVec3d dQdu, dQdv;
    surface.EvaluatePoint (xyzQ,
        dQdu, dQdv, u, v);
    DVec3d scaledNormal;
    scaledNormal.GeometricMeanCrossProduct(dQdu, dQdv);
    points.push_back (xyzQ);
    points.push_back(xyzQ + dQdu * uScale);
    points.push_back(xyzQ + dQdv * vScale);
    points.push_back(xyzQ);
    if (wScale != 0.0)
        points.push_back (xyzQ + scaledNormal * wScale);
    }
void MSBsplineSurface_ClosestPointOnBoundary(
    MSBsplineSurfaceCR surface,
    DPoint3dCR spacePoint,
    DPoint3dR boundaryPoint
)
    {
    DRange3d poleRange;
    surface.GetPoleRange (poleRange);
    static double s_reltol = 1.0e-8;
    double tolerance = s_reltol * poleRange.low.Distance (poleRange.high);
    auto curves = surface.GetUnstructuredBoundaryCurves( tolerance, true, true);
    CurveLocationDetail detail;
    curves->ClosestPointBounded (spacePoint, detail);
    boundaryPoint = detail.point;
    }
/*
* Construct the boundary curves along edge of the 4 edges of the surface.
* Find the closest point on any such edge.
* Return that point (boudaryPoint) and its UV coordinates on the surface (boundaryUV)
* @param [in] surface  bspline surface to search
* @param [in] spacePoint any point
* @param [out] boundaryPoint closest point on any edge of the surface
* @param [out] boundaryUV the boundaryPoint UV coordinates.
*/
void MSBsplineSurface_ClosestPointOnBoundaryIsoCurves(
    MSBsplineSurfaceCR surface,
    DPoint3dCR spacePoint,
    DPoint3dR boundaryPoint,
    DPoint2dR boundaryUV
)
    {
    auto curveU0 = surface.GetIsoUCurve (0.0);
    auto curveU1 = surface.GetIsoUCurve(1.0);
    auto curveV0 = surface.GetIsoVCurve(0.0);
    auto curveV1 = surface.GetIsoVCurve(1.0);
    DPoint3d pointA, pointB;    // pointB is the best known ..
    double fraction;
    DPoint2d uvB;

    curveU0->ClosestPoint(pointB, fraction, spacePoint);
    uvB = DPoint2d::From (0.0, fraction);

    curveU1->ClosestPoint(pointA, fraction, spacePoint);
    if (spacePoint.Distance(pointA) < spacePoint.Distance(pointB))
        {
        pointB = pointA;
        uvB = DPoint2d::From(1.0, fraction);
        }

    curveV0->ClosestPoint(pointA, fraction, spacePoint);
    if (spacePoint.Distance(pointA) < spacePoint.Distance(pointB))
        {
        pointB = pointA;
        uvB = DPoint2d::From(fraction, 0.0);
        }

    curveV1->ClosestPoint(pointA, fraction, spacePoint);
    if (spacePoint.Distance(pointA) < spacePoint.Distance(pointB))
        {
        pointB = pointA;
        uvB = DPoint2d::From(fraction, 1.0);
        }
    boundaryPoint = pointB;
    boundaryUV = uvB;
    }

bool IsEdgePoint(DPoint2dCR uv)
    {
    return uv.x <= 0.0 || uv.y <= 0.0 || uv.x >= 1.0 || uv.y >= 1.0;
    }
#ifdef BuildImproverClass
/*
Class to run a Newton-Raphson iteration to improve accuracy of points projected onto
a bspline surface.

Create the class with the surface reference.

Run the ImproveSurfacePoint method for as many points as need improvement.

****** PROBLEM: When run at the edge of the surface, the newton step fails because of clamping.
*/
struct MSBsplineSurfaceProjectedPointImprover : FunctionRRToRR
    {
    /*
    CONSTRUCTOR -- capture the surface pointer.
    Caller must not release surface memory during ImproveSurfacePoint calls!!!
    */
    MSBsplineSurfaceProjectedPointImprover(MSBsplineSurfaceCR surface) : m_surface(surface) {}

    /* Surface being searched. */
    MSBsplineSurfaceCR m_surface;
    /* space point for current search */
    DPoint3d m_spacePoint;
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool EvaluateRRToRR
    (
        double u,
        double v,
        double &f,
        double &g
    ) override
        {
        //! Calculate the position and partial derivatives of a B-spline surface at a particular uv parameter value pair.
        DPoint3d xyz;
        DVec3d U, V, W;
        m_surface.EvaluatePoint(xyz, U, V, u, v);
        W.DifferenceOf(m_spacePoint, xyz);
        f = W.DotProduct(U);
        g = W.DotProduct(V);
        return true;
        }
    bool RunImprovementStep
    (
        DPoint3dCR spacePoint,
        DPoint2dCR initialUV,
        DPoint3dR finalSurfacePoint,
        DPoint2dR finalSurfaceUV
    )
        {
        static double s_paramTol = 1.0e-8;
        NewtonIterationsRRToRR newton(s_paramTol);
        m_spacePoint = spacePoint;
        double u = initialUV.x;
        double v = initialUV.y;
        static double s_maxStep = 0.04;
        bool iterationOK = newton.RunApproximateNewton(u, v, *this, s_maxStep, s_maxStep);
        finalSurfaceUV.x = u;
        finalSurfaceUV.y = v;
        m_surface.EvaluatePoint(finalSurfacePoint, finalSurfaceUV.x, finalSurfaceUV.y);
        return iterationOK;
        }

    };
#endif
TEST(MSBsplineSurface, ClosestPoint)
    {
    IFacetOptionsPtr options = IFacetOptions::Create();
    bvector<IGeometryPtr> g1Array;
    BentleyGeometryJson::TryJsonStringToGeometry(bsurfFromLuca, g1Array);
    if (g1Array.size() == 1)
        {
        MSBsplineSurfacePtr bsurf = g1Array[0]->GetAsMSBsplineSurface();
        if (bsurf.IsValid())
            {
            Check::SaveTransformed(bsurf);
            DRange3d poleRange;
            bsurf->GetPoleRange (poleRange);
#ifdef BuildImproverClass
            MSBsplineSurfaceProjectedPointImprover improveSurfacePoint (*bsurf);
#endif
            auto spacePoint0 = DPoint3d::From (304146.26066172763, -5480.0000000047057, 24530.000000000000);
            DPoint3d surfacePoint;
            DPoint2d surfaceUV;
            bsurf->ClosestPoint (surfacePoint, surfaceUV, spacePoint0);
            Check::SaveTransformed (DSegment3d::From (spacePoint0, surfacePoint));
            Check::Near (surfacePoint, spacePoint0, "Specific failure point from luca");
            {
            DPoint2d edgeUV4;
            DPoint3d edgePoint4;
            MSBsplineSurface_ClosestPointOnBoundaryIsoCurves(
                *bsurf, spacePoint0, edgePoint4, edgeUV4);
            double d4 = spacePoint0.Distance(edgePoint4);
            printf (" ClosestPointOnBoundaryIsoCurves (%lg, %lg) (d %lg)\n",
                        edgeUV4.x, edgeUV4.y, d4);
            }
#ifdef BuildImproverClass
            DPoint2d surfaceUVB;
            DPoint3d surfacePointB;
            improveSurfacePoint.RunImprovementStep (spacePoint0, surfaceUV,
                surfacePointB, surfaceUVB);
            printf (" (bsurf->ClosestPoint distance %lf\n", spacePoint0.Distance (surfacePoint));
            printf(" (RunImprovementStep distance %lf\n", spacePoint0.Distance(surfacePointB));
#endif
            DVec3d dQdu, dQdv;
            DPoint3d surfacePointQ;
            bsurf->EvaluatePoint (surfacePointQ, dQdu, dQdv, surfaceUV.x, surfaceUV.y);
            DVec3d scaledNormal;
            scaledNormal.GeometricMeanCrossProduct (dQdu, dQdv);
            for (double zFactor : {0.0, 0.05, -0.05})
                {
                double maxError2 = 0.0;
                double maxError3 = 0.0;
                double maxError4 = 0.0;
                if (zFactor != 0.0)
                    Check::SaveTransformed(bsurf);
                bvector<double> steps{ 0.0, 0.1, 0.01, 0.001 , -0.1, -0.01, -0.001};
                DPoint3d surfacePoint0 = surfacePoint;
                for (double du : steps)
                    {
                    for (double dv : steps)
                        {
                        // test with space points shifted on the plane at the initial point.
                        DPoint3d spacePoint = surfacePoint0 + du * dQdu + dv * dQdv + zFactor * scaledNormal;
                        DPoint3d surfacePoint1;
                        DPoint2d surfaceUV1;
                        bsurf->ClosestPoint (surfacePoint1, surfaceUV1, spacePoint);
                        bvector<DPoint3d> displayPoints {spacePoint, surfacePoint};
                        AppendSurfacePointTriad (displayPoints, *bsurf, surfaceUV1.x, surfaceUV1.y, 0.002, 0.005, 0.0);
                        Check::SaveTransformed(displayPoints);

                        DPoint3d surfacePoint2;
                        DPoint2d surfaceUV2;
                        bsurf->ClosestPoint(surfacePoint2, surfaceUV2, surfacePoint1);
                        Check::Near (surfacePoint1, surfacePoint2, "reprojected surface point");
                        maxError2 = DoubleOps::Max (maxError2, surfacePoint1.Distance (surfacePoint2));

                        if (IsEdgePoint(surfaceUV1))
                            {
                            DPoint3d edgePoint;
                            MSBsplineSurface_ClosestPointOnBoundary(*bsurf, spacePoint, edgePoint);
                            maxError3 = DoubleOps::Max(maxError3, surfacePoint1.Distance(edgePoint));

                            DPoint2d edgeUV4;
                            DPoint3d edgePoint4;
                            MSBsplineSurface_ClosestPointOnBoundaryIsoCurves(
                                *bsurf, spacePoint, edgePoint4, edgeUV4);
                            double d4 = surfacePoint1.Distance(edgePoint4);
                            maxError4 = DoubleOps::Max(maxError4, d4);

                            }
                        }
                    }
                BeConsole::Printf("  at zFactor = %lf max reprojection distance %lg\n", zFactor, maxError2);
                BeConsole::Printf("  at zFactor = %lf max edge distance %lg\n", zFactor, maxError3);
                BeConsole::Printf("  at zFactor = %lf max isocurve distance %lg\n", zFactor, maxError4);
                Check::Shift(0, poleRange.YLength(), 0);
                }
            }
        }
    Check::ClearGeometry("MSBsplineSurface.ClosestPoint");
    }

TEST(BsplineCurve, overStrokeFromLuca)
    {
    // Unescaped raw_json_text is in between the following multicharacter delimiters: foo( raw_json_text )foo
    static Utf8CP s_inputs = u8R"foo([{"path":[{"bcurve":{"closed":false,"knots":[0.0,0.0,0.0,0.0,1.0,1.0,1.0,1.0],"order":4,"points":[[7565.44000,1134.0261211764747,165.43999999999956],[7597.0133333334103,1134.0261203431699,165.44000],[7628.5866666667879,1134.0261195604125,165.44000],[7660.1600000002071,1134.0261188244293,165.44000000011865]]}}]},{"path":[{"bcurve":{"closed":false,"knots":[0.0,0.0,0.0,0.0,1.0,1.0,1.0,1.0],"order":4,"points":[[7565.44000,1134.0261211764747,165.43999999999956],[7597.0133333334103,1124.0261203431699,165.44000],[7628.5866666667879,1144.0261195604125,165.44000],[7660.1600000002071,1134.0261188244293,165.44000000011865]]}}]}])foo";
    bvector<IGeometryPtr> inputs;
    if (Check::True(IModelJson::TryIModelJsonStringToGeometry(s_inputs, inputs), "Parse inputs") &&
        Check::Size(inputs.size(), 2, "Have two inputs"))
        {
        auto straight = inputs[0]->GetAsCurveVector();  // Luca's test case is a very nearly straight cubic Bezier curve in UOR
        auto curved = inputs[1]->GetAsCurveVector();
        if (Check::True(straight.IsValid() && curved.IsValid(), "Inputs are valid"))
            {
            DPoint3d start, end;
            bvector<DPoint3d> pts;
            auto opts = IFacetOptions::CreateForCurves();
            opts->SetChordTolerance(19200); // from Luca
            straight->AddStrokePoints(pts, *opts);
            if (Check::Size(pts.size(), 2, "Stroker returns 2 points for straight Bezier"))
                {
                straight->GetStartEnd(start, end);
                Check::True(pts.front() == start, "Stroker returns start");
                Check::True(pts.back() == end, "Stroker returns end");
                }
            pts.clear();
            curved->AddStrokePoints(pts, *opts);
            if (Check::True(pts.size() > 2, "Stroker returns more than 2 points for curved Bezier"))
                {
                curved->GetStartEnd(start, end);
                Check::True(pts.front() == start, "Stroker returns start");
                Check::True(pts.back() == end, "Stroker returns end");
                }
            }
        }
    }

TEST(BsplineCurve, UnclampedKnots)
    {
    // Unescaped raw_json_text is in between the following multicharacter delimiters: foo( raw_json_text )foo
    static Utf8CP s_inputs = u8R"foo([{"bcurve":{"points":[[0,0,0,1],[5,5,0,1],[10,-5,0,1],[15,10,0,1],[20,-10,0,1],[25,15,0,1]],"knots":[1,1,2,3,4,5,6,7,8,8],"closed":false,"order":4}},{"bcurve":{"points":[[5,2.5,0,1],[6.66666666666,1.66666666667,0,1],[10,-5,0,1],[15,10,0,1],[18.33333333332,-3.33333333332,0,1],[20,-2.5,0,1]],"knots":[3,3,3,3,4,5,6,6,6,6],"closed":false,"order":4}}])foo";
    bvector<IGeometryPtr> inputs;
    if (Check::True(IModelJson::TryIModelJsonStringToGeometry(s_inputs, inputs), "Parse inputs") &&
        Check::Size(inputs.size(), 2, "Have two inputs"))
        {
        // verify that the unclamped IMJS curve imports as a clamped curve 
        auto unclampedCP = inputs[0]->GetAsICurvePrimitive();
        auto clampedCP = inputs[1]->GetAsICurvePrimitive();
        if (Check::True(unclampedCP.IsValid() && clampedCP.IsValid(), "Inputs are valid"))
            {
            auto unclamped = unclampedCP->GetBsplineCurveCP();
            auto clamped = clampedCP->GetBsplineCurveCP();
            if (Check::True(unclamped != 0 && clamped != 0, "Inputs are B-spline curves"))
                {
                Check::True(unclamped->AlmostEqual(*clamped), "Unclamped curve imports as clamped");
                }
            }
        }
    }

TEST(Spiral, ClothoidProjectionFromClaude)
    {
    // distances in meters
    DPoint3d startPoint = DPoint3d::From(38.049581317, 391.30711461591967);
    double startCurvature = 10;
    double endCurvature = 0;
    double length = 0.013;
    double startAngle = 1.0877954002626353;

    DPoint3d spacePoint = DPoint3d::From(36.079385122364806, 392.05867428412268);
    DSpiral2dClothoid clothoid;
    clothoid.SetBearingCurvatureLengthCurvature(0, startCurvature, length, endCurvature);

    RotMatrix rMatrix;
    rMatrix.InitFromAxisAndRotationAngle(2, startAngle);
    Transform spiralToWorld;
    spiralToWorld.InitFrom(rMatrix, startPoint);

    DPoint3d pointOn;
    double fraction, distance;
    bool result = DSpiral2dBase::ClosestPoint(clothoid, -7, 1, spiralToWorld, spacePoint, pointOn, fraction, distance);
    if (Check::True(result, "Clothoid ClosestPoint 1 of 2 succeeded."))
        Check::Near(fraction, -5.9028694715602592, "Found closest point in extended clothoid.");

    result = DSpiral2dBase::ClosestPoint(clothoid, -0.01, 1.01, spiralToWorld, spacePoint, pointOn, fraction, distance);
    if (Check::True(result, "Clothoid ClosestPoint 2 of 2 succeeded."))
        Check::Near(fraction, -0.01, "Found closest point at start point.");
    }
