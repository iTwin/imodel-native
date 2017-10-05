#include "testHarness.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, MagnitudeSquared)
    {
    DPoint2d point0 = DPoint2d::From (0.0, 1.0);
    DPoint2d point1 = DPoint2d::From (1.0, 1.0);
    DPoint2d point2 = DPoint2d::From (0.0, 0.0);
    DPoint2d point3;
    DPoint2d point4 = DPoint2d::From (2.0, 2.0);
    DPoint2d point5;
    DPoint2d point6 = DPoint2d::From (0.0, 0.5);
    DPoint2d point7;
    DPoint2d point8 = DPoint2d::From (1.0, 2.0);
    DPoint2d point9 = DPoint2d::From (0.0, 1.0);
    DPoint2d point10 = DPoint2d::From (1.0, 1.0);
    DPoint2d point11 = DPoint2d::From (1.0, 0.0);
    DPoint2d point12;
    DPoint2d point13 = DPoint2d::From (-1.0, 0.0);
    DPoint2d point14;
    double scale = 2.0;
    double s = 0.5;
    double magnitudeSqr0 = point0.MagnitudeSquared ();
    double cross0 = point0.CrossProduct (point1);
    double cross1 = point2.CrossProductToPoints (point0, point1);
    double dot0 = point0.DotProduct (point1);
    double dot1 = point2.DotProductToPoints (point0, point1);
    double distanceSqr0 = point0.DistanceSquared (point1);
    point3.SumOf (point2, point1, scale);
    point5.Interpolate (point2, s, point0);
    point7.SumOf (point0, point1);
    point9.Add (point1);
    point10.Subtract (point0);
    point12.DifferenceOf (point0, point1);
    point14.Scale (point1, scale);
    Check::Near (1.0, magnitudeSqr0);
    Check::Near (-1.0, cross0);
    Check::Near (-1.0, cross1);
    Check::Near (1.0, dot0);
    Check::Near (1.0, dot1);
    Check::Near (1.0, distanceSqr0);
    Check::Near (point4, point3);
    Check::Near (point6, point5);
    Check::Near (point8, point7);
    Check::Near (point8, point9);
    Check::Near (point11, point10);
    Check::Near (point13, point12);
    Check::Near (point4, point14);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, Normalize)
    {
    DPoint2d point0 = DPoint2d::From (0.0, 0.0);
    DPoint2d point1 = DPoint2d::From (-1.0, 2.0);
    DPoint2d point5 = DPoint2d::From (-1.0, 2.0);
    DPoint2d point2 = DPoint2d::From (-1.0 / (sqrt (5.0)), 2.0 / (sqrt (5.0)));
    DPoint2d point3;
    DPoint2d point4;
    double magnitude0 = point0.Normalize ();
    double magnitude1 = point1.Normalize ();
    double magnitude2 = point3.Normalize (point0);
    double magnitude3 = point4.Normalize (point5);
    Check::Near (0.0, magnitude0);
    Check::Near (sqrt (5.0), magnitude1);
    Check::Near (point2, point1);
    Check::Near (0.0, magnitude2);
    Check::Near (sqrt (5.0), magnitude3);
    Check::Near (point2, point4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, Rotate90)
    {
    DPoint2d point0, point4, point5;
    DPoint2d point1 = DPoint2d::From (1.0, 0.0);
    DPoint2d point2 = DPoint2d::From (0.0, 1.0);
    DPoint2d point6 = DPoint2d::From (0.0, -1.0);
    double radian0 = PI/2;
    double radian1 = -PI/2;
    point0.Rotate90 (point1);
    point4.RotateCCW (point1, radian0);
    point5.RotateCCW (point1, radian1);
    Check::Near (point2, point0);
    Check::Near (point2, point4);
    Check::Near (point6, point5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, Zero)
    {
    DPoint2d point0 = DPoint2d::From (12.9, -4.3);
    DPoint2d point1 = DPoint2d::From (0.0, 0.0);
    DPoint2d point2 = DPoint2d::From (1.0, 1.0);
    DPoint2d point3 = DPoint2d::From (-54.0, 3.6);
    point0.Zero ();
    point3.One ();
    Check::Near (point1, point0);
    Check::Near (point2, point3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, InitFromArray)
    {
    DPoint2d point0, point2;
    DPoint2d point1 = DPoint2d::From (2.0, -7.9);
    DPoint3d point3 = DPoint3d::From (2.0, -7.9, 8.9);
    double pxy[2];
    pxy[0] = 2.0;
    pxy[1] = -7.9;
    point0.InitFromArray (pxy);
    point2.Init (point3);
    Check::Near (point1, point0);
    Check::Near (point1, point2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, SetComponents)
    {
    DPoint2d point0;
    DPoint2d point1 = DPoint2d::From (2.0, -7.9);
    DPoint2d point2 = DPoint2d::From (4.0, -7.9);
    DPoint2d point3 = DPoint2d::From (2.0, 9.3);
    DPoint2d point4 = DPoint2d::From (-5.1, -7.9);
    double xx = 2.0;
    double yy = -7.9;
    int index0 = 0;
    int index1 = 1;
    int index2 = 2;
    double xCoord, yCoord;
    point0.Init(xx, yy);
    point2.SetComponent (xx, index0);
    point3.SetComponent (yy, index1);
    point4.SetComponent (xx, index2);
    double component0 = point1.GetComponent (index0);
    double component1 = point1.GetComponent (index1);
    double component2 = point1.GetComponent (index2);
    point1.GetComponents (xCoord, yCoord);
    Check::Near (point1, point0);
    Check::Near (point1, point2);
    Check::Near (point1, point3);
    Check::Near (point1, point4);
    Check::Near (xx, component0);
    Check::Near (yy, component1);
    Check::Near (xx, component2);
    Check::Near (xx, xCoord);
    Check::Near (yy, yCoord);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, SumOf)
    {
    DPoint2d point0, point6;
    DPoint2d point1 = DPoint2d::From (0.0, 0.0);
    DPoint2d point2 = DPoint2d::From (2.0, -7.9);
    DPoint2d point3 = DPoint2d::From (4.0, -7.9);
    DPoint2d point4 = DPoint2d::From (-8.0, 7.9);
    DPoint2d point5 = DPoint2d::From (3.0, 2.0);
    DPoint2d point7 = DPoint2d::From (-5.0, 9.9);
    double scale0 = 2.0;
    double scale1 = -3.0;
    double scale2 = 1.0;
    point0.SumOf (point1, point2, scale0, point3, scale1);
    point6.SumOf (point1, point2, scale0, point3, scale1, point5, scale2);
    Check::Near (point4, point0);
    Check::Near (point7, point6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, NormalizedDifferenceOf)
    {
    DPoint2d point0;
    DPoint2d point1 = DPoint2d::From (2.0, -7.9);
    DPoint2d point2 = DPoint2d::From (4.0, -7.9);
    DPoint2d point3 = DPoint2d::From (-1.0, 0.0);
    double magnitude0 = point0.NormalizedDifferenceOf (point1, point2);
    double magnitude1 = point0.Magnitude ();
    Check::Near (2.0, magnitude0);
    Check::Near (1.0, magnitude1);
    Check::Near (point3, point0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, AngleTo)
    {
    DPoint2d point0 = DPoint2d::From (1.0, 0.0);
    DPoint2d point1 = DPoint2d::From (0.0, 1.0);
    DPoint2d point2 = DPoint2d::From (1.0, 1.0);
    double angle0 = point0.AngleTo (point1);
    double angle1 = point0.AngleTo (point2);
    double angle2 = point2.AngleTo (point0);
    Check::Near (PI/2, angle0);
    Check::Near (PI/4, angle1);
    Check::Near (-PI/4, angle2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, Distance)
    {
    DPoint2d point0 = DPoint2d::From (1.0, 0.0);
    DPoint2d point1 = DPoint2d::From (2.0, 0.0);
    DPoint2d point2 = DPoint2d::From (-2.0, 0.0);
    DPoint2d point3;
    double distance0 = point0.Distance (point1);
    double distance1 = point0.Distance (point2);
    double magnitude0 = point0.Magnitude ();
    double magnitude1 = point2.Magnitude ();
    point3.Negate (point2);
    Check::Near (1.0, distance0);
    Check::Near (3.0, distance1);
    Check::Near (1.0, magnitude0);
    Check::Near (2.0, magnitude1);
    Check::Near (point1, point3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, IsParallelTo)
    {
    DPoint2d point0 = DPoint2d::From (2.0, 2.0);
    DPoint2d point1 = DPoint2d::From (1.0, 1.0);
    DPoint2d point2 = DPoint2d::From (1.0, 0.0);
    DPoint2d point3 = DPoint2d::From (0.0, 1.0);
    Check::True (point0.IsParallelTo (point1));
    Check::True (!(point0.IsParallelTo (point2)));
    Check::True (point2.IsPerpendicularTo (point3));
    Check::True (!(point2.IsPerpendicularTo (point1)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, IsEqual)
    {
    DPoint2d point0 = DPoint2d::From (2.0, 2.0);
    DPoint2d point1 = DPoint2d::From (2.0, 2.0);
    DPoint2d point2 = DPoint2d::From (1.0, 1.0);
    DPoint2d point3 = DPoint2d::From (0.0, 0.0);
    DPoint2d point4 = DPoint2d::From (0.00001, 0.00001);
    double tol0 = 0.01;
    double tol1 = -0.0001;
    Check::True (point0.IsEqual (point1));
    Check::True (!(point0.IsEqual (point2)));
    Check::True (point3.IsEqual (point4, tol0));
    Check::True (!(point3.IsEqual (point4, tol1)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, MaxAbs)
    {
    DPoint2d point0 = DPoint2d::From (-2.0, -15.0);
    DPoint2d point1 = DPoint2d::From (4.0, -5.0);
    double maxVal0 = point0.MaxAbs ();
    double maxVal1 = point1.MaxAbs ();
    Check::Near (15.0, maxVal0);
    Check::Near (5.0, maxVal1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, IsDisconnect)
    {
    DPoint2d point0;
    DPoint2d point1 = DPoint2d::From (-2.0, -15.0);
    point0.InitDisconnect ();
    Check::True (point0.IsDisconnect ());
    Check::True (!(point1.IsDisconnect ()));
    }

void TestBaryCentric (DPoint2d pointA, DPoint2d pointB, DPoint2d pointC)
    {
    double a0 = 0.2;
    double b0 = 0.25;
    double c0 = 1.0 - a0 - b0;
    DPoint3d abc0 = DPoint3d::From (a0, b0, c0);
    DPoint3d abc1, d1x, d1y;
    DPoint3d abc2, d2x, d2y;
    DPoint3d abc3, d3x, d3y;
    DPoint2d xy1, xy2, xy3;
    double area;
    xy1.SumOf (pointA, a0, pointB, b0, pointC, c0);
    xy2 = xy3 = xy1;
    xy2.x += 1.0;
    xy3.y += 1.0;
    Check::True (bsiDPoint3d_barycentricFromDPoint2dTriangle
                (abc1, d1x, d1y, area, xy1, pointA, pointB, pointC));
    Check::True (bsiDPoint3d_barycentricFromDPoint2dTriangle
                (abc2, d2x, d2y, area, xy2, pointA, pointB, pointC));
    Check::True (bsiDPoint3d_barycentricFromDPoint2dTriangle
                (abc3, d3x, d3y, area, xy3, pointA, pointB, pointC));
    DVec3d ex = DVec3d::FromStartEnd (abc1, abc2);
    DVec3d ey = DVec3d::FromStartEnd (abc1, abc3);
    Check::Near (abc0, abc1, "barycentric inversion");
    Check::Near (ex, d1x, "barycentric ddx");
    Check::Near (ey, d1y, "barycentric ddy");
    Check::Near (1.0, abc0.x + abc0.y + abc0.z, "convex sum");
    Check::Near (0.0, d1x.x + d1x.y + d1x.z, "derivative of convex sum");
    Check::Near (0.0, d1y.x + d1y.y + d1y.z, "derivative of convex sum");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, BaryCentric)
    {
    TestBaryCentric (DPoint2d::From (0,0), DPoint2d::From(1,0), DPoint2d::From (0,1));
    TestBaryCentric (DPoint2d::From (1,2), DPoint2d::From(3,6), DPoint2d::From (2,-1));
    }

double signOf (double a) { return a < 0.0 ? -1.0 : 1.0;}
    
template <typename PrimaryType, typename VectorType>
bool test_interpolate (PrimaryType const &pointA, PrimaryType const &pointB)
    {
    int errors = 0;
    VectorType vectorAB = VectorType::FromStartEnd (pointA, pointB);
    double dotUU = vectorAB.DotProduct (vectorAB);
    double dAB = pointA.Distance (pointB);            

    for (double f = -0.75; f < 1.8; f += 0.25)
        {
        PrimaryType pointC = PrimaryType::FromInterpolate (pointA, f, pointB);
        VectorType vectorAC = VectorType::FromStartEnd (pointA, pointC);
        double dotUV = vectorAB.DotProduct (vectorAC);
        double g = dotUV / dotUU;   // assume dotUU != 0
        if (!DoubleOps::AlmostEqualFraction (f, g))
            errors++;
        double dAC = pointA.Distance (pointC);
        double dBC = pointB.Distance (pointC);
        double e = signOf (f) * dAC + signOf (1.0 - f) * dBC;        
        if (!DoubleOps::AlmostEqual (e, dAB))
            errors++;
        PrimaryType pointD;
        pointD.Interpolate (pointA, f, pointB);
        if (!pointC.AlmostEqual (pointD))
            errors ++;
        }
    
    return errors == 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint2d, Interpolate)
    {
    Check::True (test_interpolate <DPoint2d, DVec2d> (DPoint2d::From (1,3), DPoint2d::From (5,11)), "DPoint2d interpolations");
    Check::True (test_interpolate <DPoint3d, DVec3d> (DPoint3d::From (1,3,-7), DPoint3d::From (5,11,13)), "DPoint3d interpolations");
    }    
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint2d, Swap)
    {
    DPoint2d dpFirst = DPoint2d::From(3, 5);
    DPoint2d dpSecond = DPoint2d::From(6, 8);
    dpFirst.Swap(dpSecond);
    Check::ExactDouble(6, dpFirst.x);
    Check::ExactDouble(8, dpFirst.y);
    Check::ExactDouble(3, dpSecond.x);
    Check::ExactDouble(5, dpSecond.y);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint2d, SumOfTwoVectorsWithScale)
    {
    DPoint2d dpMain;
    DPoint2d dpFirst = DPoint2d::From(3, 5);
    double scale1 = 2;
    DPoint2d dpSecond = DPoint2d::From(6, 8);
    double scale2 = 3;
    dpMain.SumOf(dpFirst, scale1, dpSecond, scale2);
    Check::Near(24, dpMain.x);
    Check::Near(34, dpMain.y);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint2d, ScaleToLength)
    {
    DPoint2d dpMain;
    DPoint2d dpOriginal1 = DPoint2d::From(6, 8);
    double length = 5;
    Check::Near(10, dpMain.ScaleToLength(dpOriginal1, length));
    Check::Near(3, dpMain.x);
    Check::Near(4, dpMain.y);
    length = 0;
    Check::Near(5, dpMain.ScaleToLength(length));
    Check::Near(0, dpMain.x);
    Check::Near(0, dpMain.y);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint2d, MaxDiff)
    {
    double pXY[] = { 3, 2 };
    DPoint2d pnt = DPoint2d::FromArray(pXY);

    DPoint2d pnt2 = DPoint2d::FromOne();
    double diff = pnt.MaxDiff(pnt2);
    Check::Near(2, diff);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint2d, IsConvexPair)
    {
    DPoint2d point1 = DPoint2d::From(0, 0.4);
    Check::False (point1.IsConvexPair());
    DPoint2d point2 = DPoint2d::From(0.2, 0.4);
    Check::False(point2.IsConvexPair());
    for (double f: bvector <double> {-0.3, 0, 0.2, 0.4, 0.99, 1.0, 1.2})
        {
        DPoint2d xy = DPoint2d::From (f, 1.0 - f);
        if (DoubleOps::IsIn01 (f))
            {
            Check::True (xy.IsConvexPair (false), "Good Convex pair (false)");
            Check::True (xy.IsConvexPair (true), "Good Convex pair (true)");
            }
        else
            {
            Check::False (xy.IsConvexPair (false), "Extrapolated pair (false)");
            Check::True  (xy.IsConvexPair (true), "Extapolated pair (true)");
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint2d, FromOne)
    {
    DPoint2d point2 = DPoint2d::FromOne();
    Check::True(point2.x * point2.y == 1);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint2d, FromArray)
    {
    double pts[] = { 3, 4 };
    DPoint2d point2 = DPoint2d::FromArray(pts);
    point2.ScaleToLength(point2, 2);
    Check::ExactDouble(0.4 * 3, point2.x);
    Check::ExactDouble(0.4 * 4, point2.y);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DPoint2d, Resultant)
    {
    DPoint2d pnt0 = DPoint2d::FromOne();
    DPoint2d pnt1 = DPoint2d::From(22, 24);
    pnt0.ScaleToLength(22);

    DPoint2d res;
    res.SumOf(pnt0, pnt1);
    DPoint2d resEq;
    pnt0.Swap(pnt1);
    resEq.SumOf(pnt0, pnt1);
    Check::Near(res, resEq);
    }