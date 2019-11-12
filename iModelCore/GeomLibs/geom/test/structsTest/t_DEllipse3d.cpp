/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Bentley/BeNumerical.h>

static int s_noisy = 0;

bool SolveAlignedEllipseCenter (double x0, double y0, double x1, double y1, double a, double b, bool ccw, bool largeArc, double &cx, double &cy)
    {
    if (a <= 0.0 || b <= 0.0)
        return false;
    double dx = (x1 - x0) / a;
    double dy = (y1 - y0) / b;
    // Now we are working in stretched coordinates where the ellipse is a unit circle.
    double d = sqrt (dx * dx + dy * dy);
    double ux = dx / d; // unit vector from X0 to X1
    double uy = dy / d;
    double vx = - uy;   // perpendicular to unit bisector
    double vy = ux;
    
    double e1 = 0.5 * d;
    if (e1 > 1.0)
        return false;
    // vector to bisector midpoint
    double bx = dx * 0.5;
    double by = dy * 0.5;
    double e2 = sqrt (1.0 - e1 * e1);
    double direction = 1.0;
    if (ccw == largeArc)
        direction = -1.0;
    double cxLocal = bx + direction * e2 * vx;
    double cyLocal = by + direction * e2 * vy;
    // map back to prescale
    cx = x0 + cxLocal * a;
    cy = y0 + cyLocal * b;
    return true;
    }


//! @param [in] x0 start point x coordinate (global)
//! @param [in] y0 start point y coordinate (global)
//! @param [in] x1 end point x coordinate
//! @param [in] y1 end point y coordinate
//! @param ]in] theta angle from global x axis to ellipse a axis
//! @param [in] a ellipse length on local horizontal axis
//! @param [in] b ellipse radius on local vertical axis
//! @param [in] ccw true for elliptical arc traveling counter clockwise from start to end
//! @param [in] largeArc true for larger part of ellipse
//! @param [out] cx ellipse center x (global)
//! @param [out] cy ellipse center y (global)
bool SolveRotatedEllipseCenter (double x0, double y0, double x1, double y1, double theta, double a, double b, bool ccw, bool largeArc, double &cx, double &cy)
    {
    // vector from point 0 to point 1 in global coordinates ...
    double dx = x1 - x0;
    double dy = y1 - y0;
    double c = cos (theta);
    double s = sin (theta);
    // change to local coordinates ...
    double ex =   dx * c + dy * s;
    double ey = - dx * s + dy * c;
    double cxLocal, cyLocal;
    if (!SolveAlignedEllipseCenter (0,0, ex, ey, a, b, ccw, largeArc, cxLocal, cyLocal))
        return false;
    cx = x0 + cxLocal * c - cyLocal * s;
    cy = y0 + cxLocal * s + cyLocal * c;
    return true;
    }

void CheckEllipsePoint (double x, double y, double cx, double cy, double a, double b)
    {
    double u = (x - cx) / a;
    double v = (y - cy) / b;
    double f = u * u + v * v;
    Check::Near (f, 1.0, "Point on Ellipse");
    }



bool Check_SolveAlignedEllipseCenter (double x0, double y0, double x1, double y1, double a, double b,
            bool ccw, bool largeArc, double &cx, double &cy)
    {
    if (SolveAlignedEllipseCenter (x0, y0, x1, y1, a, b, ccw, largeArc, cx, cy))
        {
        CheckEllipsePoint (x0, y0, cx, cy, a, b);
        CheckEllipsePoint (x1, y1, cx, cy, a, b);
        return true;
        }
    return false;
    }

void RotateAroundOrigin (double x, double y, double theta, double &xOut, double &yOut)
    {
    double c = cos (theta);
    double s = sin (theta);
    xOut = x * c - y * s;
    yOut = x * s + y * c;
    }

bool Check_SolveRotatedEllipseCenter (double x0, double y0, double x1, double y1, double a, double b)
    {
    double cx0, cy0;
    if (!SolveAlignedEllipseCenter (x0, y0, x1, y1, a, b, true, true, cx0, cy0))
        return false;
    for (double theta = -0.8; theta < 1.0; theta += 0.324)
        {
        double u0, v0, u1, v1, cx1, cy1, cx2,cy2;
        RotateAroundOrigin (x0, y0, theta, u0, v0);
        RotateAroundOrigin (x1, y1, theta, u1, v1);
        if (!SolveRotatedEllipseCenter (u0, v0, u1,v1, theta, a, b, true, true, cx1, cy1))
            return false;
        RotateAroundOrigin (cx1, cy1, -theta, cx2, cy2);
        Check::Near (cx2, cx0, "rotated cx");
        Check::Near (cy2, cy0, "rotated cy");
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Ellipse, Microsoft)
{
double cx0, cy0, cx1, cy1;
Check_SolveAlignedEllipseCenter (1,2,  4,5, 3, 4, true,  true, cx0, cy0);
Check_SolveAlignedEllipseCenter (1,2,  4,5, 3, 4, false, true, cx1, cy1);

Check_SolveRotatedEllipseCenter (1,2,  4,5, 3, 4);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, Init)
    {
    DEllipse3d ellipse0, ellipse1, ellipse2, ellipse3, ellipse4, ellipse5, ellipse6, ellipse7, ellipse8;
    DPoint3d center0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d center1 = DPoint3d::FromXYZ (0.5, 0.0, 0.0);
    DPoint3d vector0 = DPoint3d::FromXYZ (1.0, 1.0, 1.0);
    DPoint3d vector90 = DPoint3d::FromXYZ (2.0, 2.0, 2.0);
    DPoint3d vector01 = DPoint3d::FromXYZ (cos (0.4), sin (0.4), 0.0);
    DPoint3d vector901 = DPoint3d::FromXYZ (-sin (0.4), cos (0.4), 0.0);
    DPoint3d start0 = DPoint3d::FromXYZ (1.0, 0.0, 0.0);
    DPoint3d middle0 = DPoint3d::FromXYZ (0.0, 1.0, 0.0);
    DPoint3d middle1 = DPoint3d::FromXYZ (0.0, 2.0, 0.0);
    DPoint3d end0 = DPoint3d::FromXYZ (-1.0, 0.0, 0.0);
    DVec3d vector2 = DVec3d::From (0.0, 1.0, 0.0);
    DVec3d vector3 = DVec3d::From (1.0, 1.0, 1.0);
    DVec3d vector4 = DVec3d::From (2.0, 2.0, 2.0);
    DPoint3d pointArray[3];
    pointArray[0] = center0;
    pointArray[1] = vector0;
    pointArray[2] = vector90;
    ellipse0.Init (0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 0.7, 0.5);
    ellipse1 = DEllipse3d::FromXYMajorMinor (0.0, 0.0, 0.0, 1.0, 1.0, 0.4, 0.7, 0.5);
    ellipse2.InitFromPoints (center0, vector0, vector90, 0.7, 0.5); 
    Check::True (ellipse3.InitFromPointsOnArc (start0, middle0, end0));
    Check::True (!(ellipse3.InitFromPointsOnArc (start0, start0, start0)));
    Check::True (ellipse4.InitArcFromPointTangentPoint (center0, vector2, start0));
    Check::True (!(ellipse5.InitFromArcCenterStartEnd (center0, start0, end0)));
    Check::True (ellipse6.InitFromArcCenterStartEnd (center0, start0, middle0), "c0m");
    Check::True (ellipse7.InitFromArcCenterStartEnd (center0, start0, middle1));
    DEllipse3d ellipse7A = DEllipse3d::FromArcCenterStartEnd (center0, start0, middle1);
    Check::Near (ellipse7, ellipse7A, "Init/From CenterStartEnd");
    ellipse8.InitFromVectors (center0, vector3, vector4, 0.7, 0.5);
    Check::Near (center0, ellipse0.center);
    Check::Near (vector0, ellipse0.vector0);
    Check::Near (vector90, ellipse0.vector90);
    Check::Near (0.7, ellipse0.start);
    Check::Near (0.5, ellipse0.sweep);
    Check::Near (center0, ellipse1.center);
    Check::Near (vector01, ellipse1.vector0);
    Check::Near (vector901, ellipse1.vector90);
    Check::Near (0.7, ellipse1.start);
    Check::Near (0.5, ellipse1.sweep);
    Check::Near (center0, ellipse2.center);
    Check::Near (vector0, ellipse2.vector0);
    Check::Near (vector90, ellipse2.vector90);
    Check::Near (0.7, ellipse2.start);
    Check::Near (0.5, ellipse2.sweep);
    Check::Near (center0, ellipse3.center);
    Check::Near (0.0, ellipse3.start);
    Check::Near (PI, ellipse3.sweep);
    Check::Near (center1, ellipse4.center);
    Check::Near (0.0, ellipse4.start);
    Check::Near (PI, ellipse4.sweep);
    Check::Near (0.0, ellipse6.start);
    Check::Near (PI/2, ellipse6.sweep);
    Check::Near (middle0, ellipse7.vector90);
    Check::Near (0.0, ellipse7.start);
    Check::Near (PI/2, ellipse7.sweep);
    Check::Near (center0, ellipse8.center);
    Check::Near (vector3, ellipse8.vector0);
    Check::Near (vector4, ellipse8.vector90);
    Check::Near (0.7, ellipse8.start);
    Check::Near (0.5, ellipse8.sweep);
#ifdef DEllipse3d_has_initfromPointArray
    DEllipse3d ellipse9;
    ellipse9.InitFromPointArray (pointArray, 0.7, 0.5);
    Check::Near (center0, ellipse9.center);
    Check::Near (vector0, ellipse9.vector0);
    Check::Near (vector90, ellipse9.vector90);
    Check::Near (0.7, ellipse9.start);
    Check::Near (0.5, ellipse9.sweep);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, SetStartEnd)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 2*PI);
    DPoint3d start0 = DPoint3d::FromXYZ (1.0, 0.0, 0.0);
    DPoint3d end0 = DPoint3d::FromXYZ (0.0, 1.0, 0.0);
    bool ccw = true;
    Check::True (ellipse0.SetStartEnd (start0, end0, ccw));
    Check::Near (0.0, ellipse0.start);
    Check::Near (PI/2, ellipse0.sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, InitFromScaledRotMatrix)
    {
    DEllipse3d ellipse0;
    DEllipse3d ellipse1;
    DPoint3d center0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DVec3d vectorU = DVec3d::From (1.0, 0.0, 0.0);
    DVec3d vectorV = DVec3d::From (0.0, 1.0, 0.0);
    DVec3d vector1 = DVec3d::From (2.0, 0.0, 0.0);
    DVec3d vector2 = DVec3d::From (0.0, 2.0, 0.0);
    RotMatrix matrix0 = RotMatrix::FromRowValues (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0);
    ellipse0.InitFromScaledRotMatrix (center0, matrix0, 1.0, 1.0, 0.0, PI/2);
    ellipse1.InitFromScaledVectors (center0, vectorU, vectorV, 2.0, 2.0, 0.0, PI/2);
    Check::Near (center0, ellipse0.center);
    Check::Near (vectorU, ellipse0.vector0);
    Check::Near (vectorV, ellipse0.vector90);
    Check::Near (0.0, ellipse0.start);
    Check::Near (PI/2, ellipse0.sweep);
    Check::Near (center0, ellipse1.center);
    Check::Near (vector1, ellipse1.vector0);
    Check::Near (vector2, ellipse1.vector90);
    Check::Near (0.0, ellipse1.start);
    Check::Near (PI/2, ellipse1.sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, GetScaledRotMatrix)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI/2);
    DPoint3d center0;
    DPoint3d center1 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    RotMatrix matrix0;
    DVec3d vector1 = DVec3d::From (1.0, 0.0, 0.0);
    DVec3d vector2 = DVec3d::From (0.0, 1.0, 0.0);
    DVec3d vector3 = DVec3d::From (0.0, 0.0, 1.0);
    DVec3d vector4;
    DVec3d vector5;
    DVec3d vector6;
    double r0;
    double r1;
    double theta0;
    double sweep0;
    ellipse0.GetScaledRotMatrix (center0, matrix0, r0, r1, theta0, sweep0);
    vector4.InitFromColumn (matrix0, 0);
    vector5.InitFromColumn (matrix0, 1);
    vector6.InitFromColumn (matrix0, 2);
    Check::Near (center1, center0);
    Check::Near (vector1, vector4);
    Check::Near (vector2, vector5);
    Check::Near (vector3, vector6);
    Check::Near (1.0, r0);
    Check::Near (1.0, r1);
    Check::Near (0.0, theta0);
    Check::Near (PI/2, sweep0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, InitFromCenterNormalRadius)
    {
    DEllipse3d ellipse0;
    DPoint3d center0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DVec3d normal0 = DVec3d::From (1.0, 0.0, 0.0);
    DPoint3d vector1 = DPoint3d::FromXYZ (0.0, 1.0, 0.0);
    DPoint3d vector2 = DPoint3d::FromXYZ (0.0, 0.0, 1.0);
    double radius0 = 1;
    ellipse0.InitFromCenterNormalRadius (center0, normal0, radius0);
    Check::Near (center0, ellipse0.center);
    Check::Near (vector1, ellipse0.vector0);
    Check::Near (vector2, ellipse0.vector90);
    Check::Near (0.0, ellipse0.start);
    Check::Near (2*PI, ellipse0.sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IsFullEllipse)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI/2);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI/2);
    Check::True (!(ellipse0.IsFullEllipse ()));
    Check::True (ellipse1.IsFullEllipse ());
    ellipse0.MakeFullSweep ();
    ellipse1.ComplementSweep ();
    ellipse2.ComplementSweep ();
    Check::Near (2*PI, ellipse0.sweep);
    Check::Near (2*PI, ellipse1.sweep);
    Check::Near (-(3*PI)/2, ellipse2.sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, Evaluate)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DPoint3d point0;
    DPoint3d point1;
    DPoint3d point2 = DPoint3d::FromXYZ (0.0, 1.0, 0.0);
    DPoint4d point3;
    DPoint4d point4 = DPoint4d::From (0.0, 1.0, 0.0, 1.0);
    DPoint3d point5;
    DPoint3d point6;
    DPoint3d point7 = DPoint3d::FromXYZ (1.0, 0.0, 0.0);
    DPoint3d point8;
    DVec3d dX;
    DVec3d ddX;
    DVec3d point9 = DVec3d::From (-1.0, 0.0, 0.0);
    DVec3d point10 = DVec3d::From (0.0, -1.0, 0.0);
    double theta = PI/2;
    double cosTheta = cos (PI/2);
    double sinTheta = sin (PI/2);
    ellipse0.Evaluate (point0, theta);
    ellipse0.Evaluate (point1, cosTheta, sinTheta);
    ellipse0.Evaluate (point3, theta);
    ellipse0.EvaluateEndPoints (point5, point6);
    ellipse0.Evaluate (point8, dX, ddX, theta);
    Check::Near (point2, point0);
    Check::Near (point1, point2);
    Check::Near (point3, point4);
    Check::Near (point7, point5);
    Check::Near (point7, point6);
    Check::Near (point2, point8);
    Check::Near (point9, dX);
    Check::Near (point10, ddX);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, FractionParameterToPoint)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DPoint3d point0;
    DPoint3d point1 = DPoint3d::FromXYZ (-1.0, 0.0, 0.0);
    DPoint3d point2;
    DVec3d point3;
    DVec3d point4;
    DVec3d point5 = DVec3d::From (0.0, -2*PI, 0.0);
    DVec3d point6 = DVec3d::From (4*PI*PI, 0.0, 0.0);
    DPoint3d point7[3];
    DPoint3d point8 = DPoint3d::FromXYZ (0.0, -1.0, 0.0);
    DPoint3d point9 = DPoint3d::FromXYZ (1.0, 0.0, 0.0);
    double fraction0 = 0.5;
    double theta = PI;
    ellipse0.FractionParameterToPoint (point0, fraction0);
    ellipse0.FractionParameterToDerivatives (point2, point3, point4, fraction0);
    ellipse0.Evaluate (point7, 2, theta);
    double theta1 = ellipse0.FractionToAngle (fraction0);
    double determinant = ellipse0.DeterminantJXY (); 
    Check::Near (point1, point0);
    Check::Near (point1, point2);
    Check::Near (point5, point3);
    Check::Near (point6, point4);
    Check::Near (point1, point7[0]);
    Check::Near (point8, point7[1]);
    Check::Near (point9, point7[2]);
    Check::Near (theta, theta1);
    Check::Near (1.0, determinant);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, GetLocalFrame)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    Transform frame0;
    Transform frame1;
    Transform frame2;
    Transform inverse0;
    Transform inverse1;
    Transform inverse2;
    DPoint3d center0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DVec3d xVector = DVec3d::From (1.0, 0.0, 0.0);
    DVec3d yVector = DVec3d::From (0.0, 1.0, 0.0);
    DVec3d zVector = DVec3d::From (0.0, 0.0, 1.0);
    DPoint3d pointIn0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d pointIn1 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d pointOut0;
    DPoint3d pointOut1;
    DPoint3d pointIn2 = DPoint3d::FromXYZ (0.0, 2.0, 0.0);
    DPoint3d xYZNear;
    DPoint3d pointOut3 = DPoint3d::FromXYZ (0.0, 2.0, 0.0);
    double coff0, coff90;
    frame1.InitFromOriginAndVectors (center0, xVector, yVector, zVector);
    Check::True (inverse1.InverseOf (frame1));
    Check::True (ellipse0.GetLocalFrame (frame0, inverse0));
    Check::True (ellipse0.GetXYLocalFrame (frame2, inverse2));
    inverse1.Multiply (&pointOut1, &pointIn1, 1);
    Check::True (ellipse0.PointToXYLocal (pointOut0, pointIn0));
    double theta = ellipse0.PointToAngle (pointIn2);
    ellipse0.ProjectPointToPlane (xYZNear, coff0, coff90, pointIn2);
    Check::Near (frame1, frame0);
    Check::Near (inverse1, inverse0);
    Check::Near (frame1, frame2);
    Check::Near (inverse1, inverse2);
    Check::Near (pointOut1, pointOut0);
    Check::Near (PI/2, theta);
    Check::Near (pointOut3, xYZNear);
    Check::Near (0.0, coff0);
    Check::Near (2.0, coff90);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, GetStrokeCount)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    double tol0 = 1 - cos (PI/4);
    double tol1 = 1 - cos (PI/8);
    int n0 = 0;
    int max0 = 8;
    int max1 = 16;
    int max2 = 4;
    int n1 = ellipse0.GetStrokeCount (n0, max0, tol0);
    int n2 = ellipse0.GetStrokeCount (n0, max1, tol1);
    int n3 = ellipse0.GetStrokeCount (n0, max2, tol1);
    Check::Near (4, n1);
    Check::Near (8, n2);
    Check::Near (4, n3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, EvaluateTrigPairs)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DPoint3d points0[3];
    DPoint3d points1[3];
    DPoint2d trig[3];
    trig[0] = DPoint2d::From (cos (0.0), sin (0.0));
    trig[1] = DPoint2d::From (cos (PI), sin (PI));
    trig[2] = DPoint2d::From (cos ((3*PI)/2), sin ((3*PI)/2));
    int numPoints = 3;
    ellipse0.EvaluateTrigPairs (points0, trig, numPoints);
    int returnedNumPoints = ellipse1.TestAndEvaluateTrigPairs (points1, trig, numPoints);
    Check::Near (DPoint3d::FromXYZ (1.0, 0.0, 0.0), points0[0]);
    Check::Near (DPoint3d::FromXYZ (-1.0, 0.0, 0.0), points0[1]);
    Check::Near (DPoint3d::FromXYZ (0.0, -1.0, 0.0), points0[2]);
    Check::Near (2, returnedNumPoints);
    Check::Near (DPoint3d::FromXYZ (1.0, 0.0, 0.0), points1[0]);
    Check::Near (DPoint3d::FromXYZ (-1.0, 0.0, 0.0), points1[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IsAngleInSweep)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI/2);
    double angle0 = 0;
    double angle1 = PI/2;
    double angle2 = PI;
    double angle3 = 3*PI;
    Check::True (ellipse0.IsAngleInSweep (angle0));
    Check::True (ellipse0.IsAngleInSweep (angle1));
    Check::True (ellipse0.IsAngleInSweep (angle2));
    Check::True (ellipse0.IsAngleInSweep (angle3));
    Check::True (ellipse1.IsAngleInSweep (angle0));
    Check::True (ellipse1.IsAngleInSweep (angle1));
    Check::True (ellipse1.IsAngleInSweep (angle2));
    Check::True (ellipse1.IsAngleInSweep (angle3));
    Check::True (ellipse2.IsAngleInSweep (angle0));
    Check::True (ellipse2.IsAngleInSweep (angle1));
    Check::True (!(ellipse2.IsAngleInSweep (angle2)));
    Check::True (!(ellipse2.IsAngleInSweep (angle3)));
    double fraction0 = ellipse0.AngleToFraction (angle0);
    double fraction1 = ellipse0.AngleToFraction (angle1);
    double fraction2 = ellipse0.AngleToFraction (angle2);
    double fraction3 = ellipse0.AngleToFraction (angle3);
    Check::Near (0.0, fraction0);
    Check::Near (0.25, fraction1);
    Check::Near (0.5, fraction2);
    Check::Near (0.5, fraction3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, GetLimits)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, PI/4, 2*PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, PI/2, PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI/2);
    double startAngle0, startAngle1, startAngle2, startAngle3, startAngle4, endAngle0, endAngle1, endAngle2, endAngle3, endAngle4;
    ellipse0.GetLimits (startAngle0, endAngle0);
    ellipse1.GetLimits (startAngle1, endAngle1);
    ellipse2.GetLimits (startAngle2, endAngle2);
    ellipse0.GetSweep (startAngle3, endAngle3);
    ellipse1.GetSweep (startAngle4, endAngle4);
    Check::Near (PI/4, startAngle0);
    Check::Near ((9*PI)/4, endAngle0);
    Check::Near (PI/2, startAngle1);
    Check::Near ((3*PI)/2, endAngle1);
    Check::Near (0.0, startAngle2);
    Check::Near (PI/2, endAngle2);
    Check::Near (PI/4, startAngle3);
    Check::Near (2*PI, endAngle3);
    Check::Near (PI/2, startAngle4);
    Check::Near (PI, endAngle4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, SetLimits)
    {
    DEllipse3d ellipse0, ellipse1, ellipse2, ellipse3;
    double startAngle0 = PI/4;
    double startAngle1 = PI/2;
    double endAngle0 = 2*PI;
    double endAngle1 = PI;
    ellipse0.SetLimits (startAngle0, endAngle0);
    ellipse1.SetLimits (startAngle1, endAngle1);
    ellipse2.SetSweep (startAngle0, endAngle0);
    ellipse3.SetSweep (startAngle1, endAngle1);
    Check::Near (PI/4, ellipse0.start);
    Check::Near ((7*PI)/4, ellipse0.sweep);
    Check::Near (PI/2, ellipse1.start);
    Check::Near (PI/2, ellipse1.sweep);
    Check::Near (PI/4, ellipse2.start);
    Check::Near (2*PI, ellipse2.sweep);
    Check::Near (PI/2, ellipse3.start);
    Check::Near (PI, ellipse3.sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, InitWithPerpendicularAxes)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse1;
    DVec3d crossBefore, crossAfter;
    ellipse1.InitWithPerpendicularAxes (ellipse0);
    crossBefore.CrossProduct (ellipse0.vector0, ellipse0.vector90);
    crossAfter.CrossProduct (ellipse1.vector0, ellipse1.vector90);
    Check::True (ellipse1.vector0.IsPerpendicularTo (ellipse1.vector90));
    Check::Near (crossBefore, crossAfter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, GetMajorMinorRangeMidlines)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DSegment3d longSegment, shortSegment;
    DSegment3d longSegment1 = DSegment3d::From (-1.0, 0.0, 0.0, 1.0, 0.0, 0.0);
    DSegment3d shortSegment1 = DSegment3d::From (0.0, -1.0, 0.0, 0.0, 1.0, 0.0);
    double length = ellipse0.GetMajorMinorRangeMidlines (longSegment, shortSegment);
    Check::Near (longSegment1, longSegment);
    Check::Near (shortSegment1, shortSegment);
    Check::Near (2.0, length);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, InitReversed)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse1;
    ellipse1.InitReversed (ellipse0);
    Check::Near (2*PI, ellipse1.start);
    Check::Near (-2*PI, ellipse1.sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, FromCopyWithPositiveSweep)
    {
    DPoint3d xyz0, xyz1;
    for (double sweepFraction = -2.0; sweepFraction <= 2.0; sweepFraction += 0.25)  // sweep by this fraction of PI
        {
        if (sweepFraction == 0.0)
            continue;
        for (double startFraction = -2.0; startFraction <= 2.0; startFraction += 0.25)  // start at this fraction of PI
            {
            DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, startFraction * PI, sweepFraction*PI);
            DEllipse3d ellipse1 = DEllipse3d::FromCopyWithPositiveSweep (ellipse0);
            for (double f = 0.0; f <= 1.0; f += 1.0/8.0)
                {
                ellipse0.FractionParameterToPoint (xyz0, f);
                ellipse1.FractionParameterToPoint (xyz1, f);
                Check::Near (xyz0, xyz1, "FromCopyWithPositiveSweep preserves fractional coordinates.");
                Check::True (ellipse1.sweep > 0.0, "FromCopyWithPositiveSweep forces positive sweep");
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, TangentMagnitude)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    double theta = 0.0;
    double magnitudeTangent = ellipse0.TangentMagnitude (theta);
    Check::Near (1.0, magnitudeTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, ArcLength)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, PI, 0.6*PI);
    double arclength1 = 2*PI;
    double arclength2 = -PI;
    double arclength3 = 0.0;
    double fraction0 = 0.5;
    double fraction1 = 0.8;
    double arclength = ellipse0.ArcLength ();
    double sweepAngle0 = ellipse0.InverseArcLength (arclength1);
    double sweepAngle1 = ellipse0.InverseArcLength (arclength2);
    Check::True (ellipse1.FractionToLength (arclength3, fraction0, fraction1), "fraction to length");
    double arcLength4 = ellipse2.ArcLength ();
    Check::Near (2*PI, arclength);
    Check::Near (2*PI, sweepAngle0);
    Check::Near (-PI, sweepAngle1);
    Check::Near (arcLength4, arclength3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, ArcLengthVsBspline)
    {
    double r0 = 20.0;
    
    for (double f : bvector<double> {0.1, 0.2, 0.4, 0.8, 1.0, 1.2, 1.4, 1.8, 2.0})
        {
        Check::StartScope ("fraction", f);
        double r90 = f * r0;
        DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, r0, 0.0, 0.0, 0.0, r90, 0.0, 0.0, 2*PI);
        double a = ellipse0.ArcLength ();
        MSBsplineCurve bcurve;
        bcurve.InitFromDEllipse3d (ellipse0);
        double b = bcurve.Length ();
        double e = fabs (b-a) / a;
        if (s_noisy)
            {
            printf (" (r0 %lg) (r90 %lg) (arcLength %20.15lg)     ", r0, r90, a);
            printf (" (bcurve arc length %20.15lg) (reldiff %le)\n", b, e);
            }
        double split = 0.35;
        double tol = 1.0e-10;
        if (f > split && f < 1.0 / split)
            tol = 1.0e-13;
        Check::LessThanOrEqual (e, tol, "ellipse arc length vs bspline arc length");
        bcurve.ReleaseMem ();
        Check::EndScope ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, ArcLengthVsBsplineSubDivide)
    {
    double r0 = 20.0;
    
    for (double f : bvector<double> {0.1, 0.2, 0.4, 0.8, 1.0, 1.2, 1.4, 1.8, 2.0})
        {
        Check::StartScope ("fraction", f);
        double r90 = f * r0;
        DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, r0, 0.0, 0.0, 0.0, r90, 0.0, 0.0, 2*PI);
        double a = ellipse0.ArcLength ();
        MSBsplineCurve bcurve;
        bcurve.InitFromDEllipse3d (ellipse0);
        double delta = 1.0 / 6.0;
        for (int i = 1; i < 6; i+= 2)
            bspknot_addKnot (&bcurve, delta * i, 0., 1, true);
        double b = bcurve.Length ();
        double e = fabs (b-a) / a;
        if (s_noisy)
            {
            printf (" (r0 %lg) (r90 %lg) (arcLength %20.15lg)     ", r0, r90, a);
            printf (" (bcurve arc length %20.15lg) (reldiff %le)\n", b, e);
            }
        double split = 0.35;
        double tol = 1.0e-10;
        if (f > split && f < 1.0 / split)
            tol = 1.0e-13;
        Check::LessThanOrEqual (e, tol, "ellipse arc length vs bspline arc length");
        bcurve.ReleaseMem ();
        Check::EndScope ();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, GetRange)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DRange3d range0;
    DRange3d range2;
    DRange2d range1;
    ellipse0.GetRange (range0);
    ellipse1.GetRange (range2);
    ellipse0.GetLocalRange (range1);
    Check::Near (-1.0, range0.low.x);
    Check::Near (-1.0, range0.low.y);
    Check::Near (0.0, range0.low.z);
    Check::Near (1.0, range0.high.x);
    Check::Near (1.0, range0.high.y);
    Check::Near (0.0, range0.high.z);
    Check::Near (-1.0, range2.low.x);
    Check::Near (0.0, range2.low.y);
    Check::Near (0.0, range2.low.z);
    Check::Near (1.0, range2.high.x);
    Check::Near (1.0, range2.high.y);
    Check::Near (0.0, range2.high.z);
    Check::Near (-1.0, range1.low.x);
    Check::Near (-1.0, range1.low.y);
    Check::Near (1.0, range1.high.x);
    Check::Near (1.0, range1.high.y);
    }

bool checkIsTrigPoint (DPoint3dCR xyTheta, double expectedRadians, char const *message)
    {
    // construct a trig point from z:
    DPoint3dCR xyTheta1 = DPoint3d::From (cos (xyTheta.z), sin(xyTheta.z), xyTheta.z);
    return Check::Near (xyTheta, xyTheta1, message)
        && Check::True (Angle::NearlyEqualAllowPeriodShift (expectedRadians, xyTheta.z), message);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IntersectPlane)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DPoint3d trigPoints0[3], trigPoints1[3], trigPoints2[3];
    DPoint4d plane0 = DPoint4d::From (0.0, 0.0, 1.0, 1.0);
    DPoint4d plane1 = DPoint4d::From (1.0, 0.0, 0.0, 1.0);
    DPoint4d plane2 = DPoint4d::From (1.0, 1.0, 0.0, 1.0);
    int numPoints0 = ellipse0.IntersectPlane (trigPoints0, plane0);
    int numPoints1 = ellipse0.IntersectPlane (trigPoints1, plane1);
    int numPoints2 = ellipse0.IntersectPlane (trigPoints2, plane2);
    Check::Near (0, numPoints0);
    Check::Near (1, numPoints1);

    // Sept 2017 this was testing for -PI as the angle.
    // but Angle::Atan2 has been changed to force the (-PI,+PI) ambiguous value to positive.
    // Feb 2018 angles at 180 degrees are still ambiguous ... do the tests with period shift ...
    checkIsTrigPoint (trigPoints1[0], PI, "Tangency point");
    Check::Near (2, numPoints2);
    // we know the angles, but order is ambiguous, so tricky test . . .
    checkIsTrigPoint (trigPoints2[0], trigPoints2[0].z, "simple intersection trig point");
    checkIsTrigPoint (trigPoints2[1], trigPoints2[1].z, "simple intersection trig point");
    double q0 = PI;
    double q1 = -0.5 * PI;
    Check::True (
           Angle::NearlyEqualAllowPeriodShift (q0, trigPoints2[0].z)
        || Angle::NearlyEqualAllowPeriodShift (q1, trigPoints2[0].z), "known angle solution");
    Check::True (
           Angle::NearlyEqualAllowPeriodShift (q0, trigPoints2[1].z)
        || Angle::NearlyEqualAllowPeriodShift (q1, trigPoints2[1].z), "known angle solution");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IntersectXYLine)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DPoint3d cartesianPoints0[2], cartesianPoints1[2], cartesianPoints2[2], ellipseCoffs0[2], ellipseCoffs1[2], ellipseCoffs2[2];
    double lineParams0[2], lineParams1[2], lineParams2[2], ellipseAngle0[2], ellipseAngle1[2], ellipseAngle2[2];
    DPoint3d startPoint0 = DPoint3d::FromXYZ (-2.0, 0.0, 0.0);
    DPoint3d endPoint0 = DPoint3d::FromXYZ (2.0, 0.0, 0.0);
    DPoint3d startPoint1 = DPoint3d::FromXYZ (-1.0, 1.0, 0.0);
    DPoint3d endPoint1 = DPoint3d::FromXYZ (0.0, 2.0, 0.0);
    DPoint3d endPoint2 = DPoint3d::FromXYZ (1.0, 1.0, 0.0);
    int numPoints0 = ellipse0.IntersectXYLine (cartesianPoints0, lineParams0, ellipseCoffs0, ellipseAngle0, startPoint0, endPoint0);
    int numPoints1 = ellipse0.IntersectXYLine (cartesianPoints1, lineParams1, ellipseCoffs1, ellipseAngle1, startPoint0, endPoint1);
    int numPoints2 = ellipse0.IntersectXYLine (cartesianPoints2, lineParams2, ellipseCoffs2, ellipseAngle2, startPoint1, endPoint2);
    Check::Near (2, numPoints0);
    Check::Near (DPoint3d::FromXYZ (-1.0, 0.0, 0.0), cartesianPoints0[0]);
    Check::Near (DPoint3d::FromXYZ (1.0, 0.0, 0.0), cartesianPoints0[1]);
    Check::Near (0.25, lineParams0[0]);
    Check::Near (0.75, lineParams0[1]);
    Check::Near (DPoint3d::FromXYZ (-1.0, 0.0, 0.0), ellipseCoffs0[0]);
    Check::Near (DPoint3d::FromXYZ (1.0, 0.0, 0.0), ellipseCoffs0[1]);
    Check::Near (PI, ellipseAngle0[0]);
    Check::Near (0.0, ellipseAngle0[1]);
    Check::Near (0, numPoints1);
    Check::Near (1, numPoints2);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), cartesianPoints2[0]);
    Check::Near (0.5, lineParams2[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipseCoffs2[0]);
    Check::Near (PI/2, ellipseAngle2[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IsCircular)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 2*PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 3.0, 1.0, 0.0, 0.0, 5.0, 1.0, 0.0, 2*PI);
    Check::True (ellipse0.IsCircular ());
    Check::True (!(ellipse1.IsCircular ()));
    Check::True (ellipse0.IsCircularXY ());
    Check::True (!(ellipse1.IsCircularXY ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IntersectXYDEllipse3d)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 3.0, 1.0, 0.0, 0.0, 5.0, 1.0, 0.0, PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse3 = DEllipse3d::From (0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 0.0, 0.0, PI);
    DEllipse3d ellipse4 = DEllipse3d::From (0.0, -1.0, 0.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DPoint3d cartesianPoints0[5], cartesianPoints1[5], cartesianPoints2[5], cartesianPoints3[5], ellipse0Params0[5], ellipse0Params1[5], ellipse1Params[5], ellipse2Params0[5], ellipse2Params1[5], ellipse3Params0[5], ellipse3Params1[5], ellipse4Params0[5];
    int numPoints0 = ellipse0.IntersectXYDEllipse3d (cartesianPoints0, ellipse0Params0, ellipse1Params, ellipse1);
    int numPoints1 = ellipse0.IntersectXYDEllipse3d (cartesianPoints1, ellipse0Params1, ellipse2Params0, ellipse2);
    int numPoints2 = ellipse2.IntersectXYDEllipse3d (cartesianPoints2, ellipse2Params1, ellipse3Params0, ellipse3);
    int numPoints3 = ellipse3.IntersectXYDEllipse3d (cartesianPoints3, ellipse3Params1, ellipse4Params0, ellipse4);
    Check::Near (0, numPoints0);
    Check::Near (2, numPoints1);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), cartesianPoints1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, -1.0, 0.0), cartesianPoints1[1]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse0Params1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, -1.0, 0.0), ellipse0Params1[1]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse2Params0[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, -1.0, 0.0), ellipse2Params0[1]);
    Check::Near (4, numPoints2);
    Check::Near (3, numPoints3);
    for (int i = 0; i < numPoints2; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse2.Evaluate (xyz0, ellipse2Params1[i].x, ellipse2Params1[i].y);
        ellipse3.Evaluate (xyz1, ellipse3Params0[i].x, ellipse3Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    for (int i = 0; i < numPoints3; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse3.Evaluate (xyz0, ellipse3Params1[i].x, ellipse3Params1[i].y);
        ellipse4.Evaluate (xyz1, ellipse4Params0[i].x, ellipse4Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IntersectXYDEllipse3dBounded)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 3.0, 1.0, 0.0, 0.0, 5.0, 1.0, 0.0, PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse3 = DEllipse3d::From (0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 0.0, 0.0, PI);
    DEllipse3d ellipse4 = DEllipse3d::From (0.0, -1.0, 0.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DPoint3d cartesianPoints0[4], cartesianPoints1[4], cartesianPoints2[4], cartesianPoints3[4], ellipse0Params0[4], ellipse0Params1[4], ellipse1Params[4], ellipse2Params0[4], ellipse2Params1[4], ellipse3Params0[4], ellipse3Params1[4], ellipse4Params0[4];
    double ellipse0Angle0[4], ellipse0Angle1[4], ellipse1Angle0[4], ellipse2Angle0[4], ellipse2Angle1[4], ellipse3Angle0[4], ellipse3Angle1[4], ellipse4Angle0[4];
    int numPoints0 = ellipse0.IntersectXYDEllipse3dBounded (cartesianPoints0, ellipse0Params0, ellipse0Angle0, ellipse1Params, ellipse1Angle0, ellipse1);
    int numPoints1 = ellipse0.IntersectXYDEllipse3dBounded (cartesianPoints1, ellipse0Params1, ellipse0Angle1, ellipse2Params0, ellipse2Angle0, ellipse2);
    int numPoints2 = ellipse2.IntersectXYDEllipse3dBounded (cartesianPoints2, ellipse2Params1, ellipse2Angle1, ellipse3Params0, ellipse3Angle0, ellipse3);
    int numPoints3 = ellipse3.IntersectXYDEllipse3dBounded (cartesianPoints3, ellipse3Params1, ellipse3Angle1, ellipse4Params0, ellipse4Angle0, ellipse4);
    Check::Near (0, numPoints0);
    Check::Near (1, numPoints1);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), cartesianPoints1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse0Params1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse2Params0[0]);
    Check::Near (1, numPoints2);
    Check::Near (1, numPoints3);
    for (int i = 0; i < numPoints2; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse2.Evaluate (xyz0, ellipse2Params1[i].x, ellipse2Params1[i].y);
        ellipse3.Evaluate (xyz1, ellipse3Params0[i].x, ellipse3Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    for (int i = 0; i < numPoints3; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse3.Evaluate (xyz0, ellipse3Params1[i].x, ellipse3Params1[i].y);
        ellipse4.Evaluate (xyz1, ellipse4Params0[i].x, ellipse4Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IntersectSweptDEllipse3d)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 3.0, 1.0, 0.0, 0.0, 5.0, 1.0, 0.0, PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse3 = DEllipse3d::From (0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 0.0, 0.0, PI);
    DEllipse3d ellipse4 = DEllipse3d::From (0.0, -1.0, 0.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DPoint3d cartesianPoints0[5], cartesianPoints1[5], cartesianPoints2[5], cartesianPoints3[5], ellipse0Params0[5], ellipse0Params1[5], ellipse1Params[5], ellipse2Params0[5], ellipse2Params1[5], ellipse3Params0[5], ellipse3Params1[5], ellipse4Params0[5];
    int numPoints0 = ellipse0.IntersectSweptDEllipse3d (cartesianPoints0, ellipse0Params0, ellipse1Params, ellipse1);
    int numPoints1 = ellipse0.IntersectSweptDEllipse3d (cartesianPoints1, ellipse0Params1, ellipse2Params0, ellipse2);
    int numPoints2 = ellipse2.IntersectSweptDEllipse3d (cartesianPoints2, ellipse2Params1, ellipse3Params0, ellipse3);
    int numPoints3 = ellipse3.IntersectSweptDEllipse3d (cartesianPoints3, ellipse3Params1, ellipse4Params0, ellipse4);
    Check::Near (0, numPoints0);
    Check::Near (2, numPoints1);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), cartesianPoints1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, -1.0, 0.0), cartesianPoints1[1]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse0Params1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, -1.0, 0.0), ellipse0Params1[1]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse2Params0[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, -1.0, 0.0), ellipse2Params0[1]);
    Check::Near (4, numPoints2);
    Check::Near (3, numPoints3);
    for (int i = 0; i < numPoints2; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse2.Evaluate (xyz0, ellipse2Params1[i].x, ellipse2Params1[i].y);
        ellipse3.Evaluate (xyz1, ellipse3Params0[i].x, ellipse3Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    for (int i = 0; i < numPoints3; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse3.Evaluate (xyz0, ellipse3Params1[i].x, ellipse3Params1[i].y);
        ellipse4.Evaluate (xyz1, ellipse4Params0[i].x, ellipse4Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, IntersectSweptDEllipse3dBounded)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse1 = DEllipse3d::From (0.0, 0.0, 0.0, 3.0, 1.0, 0.0, 0.0, 5.0, 1.0, 0.0, PI);
    DEllipse3d ellipse2 = DEllipse3d::From (0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DEllipse3d ellipse3 = DEllipse3d::From (0.0, 0.0, 0.0, 0.0, 2.0, 0.0, 1.0, 0.0, 0.0, 0.0, PI);
    DEllipse3d ellipse4 = DEllipse3d::From (0.0, -1.0, 0.0, 1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0, PI);
    DPoint3d cartesianPoints0[4], cartesianPoints1[4], cartesianPoints2[4], cartesianPoints3[4], ellipse0Params0[4], ellipse0Params1[4], ellipse1Params[4], ellipse2Params0[4], ellipse2Params1[4], ellipse3Params0[4], ellipse3Params1[4], ellipse4Params0[4];
    double ellipse0Angle0[4], ellipse0Angle1[4], ellipse1Angle0[4], ellipse2Angle0[4], ellipse2Angle1[4], ellipse3Angle0[4], ellipse3Angle1[4], ellipse4Angle0[4];
    int numPoints0 = ellipse0.IntersectSweptDEllipse3dBounded (cartesianPoints0, ellipse0Params0, ellipse0Angle0, ellipse1Params, ellipse1Angle0, ellipse1);
    int numPoints1 = ellipse0.IntersectSweptDEllipse3dBounded (cartesianPoints1, ellipse0Params1, ellipse0Angle1, ellipse2Params0, ellipse2Angle0, ellipse2);
    int numPoints2 = ellipse2.IntersectSweptDEllipse3dBounded (cartesianPoints2, ellipse2Params1, ellipse2Angle1, ellipse3Params0, ellipse3Angle0, ellipse3);
    int numPoints3 = ellipse3.IntersectSweptDEllipse3dBounded (cartesianPoints3, ellipse3Params1, ellipse3Angle1, ellipse4Params0, ellipse4Angle0, ellipse4);
    Check::Near (0, numPoints0);
    Check::Near (1, numPoints1);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), cartesianPoints1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse0Params1[0]);
    Check::Near (DPoint3d::FromXYZ (0.0, 1.0, 0.0), ellipse2Params0[0]);
    Check::Near (1, numPoints2);
    Check::Near (1, numPoints3);
    for (int i = 0; i < numPoints2; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse2.Evaluate (xyz0, ellipse2Params1[i].x, ellipse2Params1[i].y);
        ellipse3.Evaluate (xyz1, ellipse3Params0[i].x, ellipse3Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    for (int i = 0; i < numPoints3; i++)
        {
        DPoint3d xyz0;
        DPoint3d xyz1;
        ellipse3.Evaluate (xyz0, ellipse3Params1[i].x, ellipse3Params1[i].y);
        ellipse4.Evaluate (xyz1, ellipse4Params0[i].x, ellipse4Params0[i].y);
        xyz1.z = xyz0.z;	// They might be at different heights, force them together for comparison
        Check::Near (xyz0, xyz1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, ConstructABXPointEdgePoint)
    {
    bvector<DEllipse3d> ellipses;
    double aa[] = {2.0, 2.0, 4.0, 0.0};
    double bb[] = {2.0, 1.0, 5.0, 0.0};
    for (int radiusSelect = 0; aa[radiusSelect] > 0.0; radiusSelect++)
        {
        double a = aa[radiusSelect];
        double b = bb[radiusSelect];
        for (double e = 0.0; e < 2.0; e += 0.25)
            {
            DPoint3d xPoint = DPoint3d::From (2.0 + e, 0.0, 0.0);
            DPoint3d edgePoint = DPoint3d::From (e*e,1,0);
            DEllipse3d::Construct_XRadius_YRadius_XPoint_EdgePoint(ellipses,
                    a, b,
                    xPoint, edgePoint);
            static int s_doChecks = 1;
            for (size_t i = 0;i < ellipses.size (); i++)
                {
                DEllipse3d ellipse = ellipses[i];
                DPoint3d point0, point1;
                ellipse.FractionParameterToPoint(point0, 0.0);
                ellipse.FractionParameterToPoint(point1, 1.0);
                DPoint3d point1Local;
                ellipse.PointToXYLocal (point1Local, edgePoint);
                double a1 = ellipse.vector0.Magnitude ();
                double b1 = ellipse.vector90.Magnitude ();
                if (s_doChecks)
                    {
                    Check::Near (xPoint, point0, "Ellipse x point");
                    Check::Near (edgePoint, point1, "Ellipse edge point");
                    Check::Near (a, a1, "Ellipse a");
                    Check::Near (b, b1, "Ellipse b");
                    Check::Near (0.0, ellipse.start, "Ellipse start angle");
                    Check::True (ellipse.vector0.IsPerpendicularTo (ellipse.vector90), "Right ellipse");
                    Check::True (ellipse.vector0.CrossProductXY(ellipse.vector90) > 0.0, "Right handed");
                    }
                }
            }
        }
    }



void ConstructXPointEdgePointEdgePointAngle_go (double d, double y0, double y1, double alpha)
    {
    static bool s_doChecks = true;
    DPoint3d edgePoint0 = DPoint3d::From (0,y0,0);
    DPoint3d xPoint = DPoint3d::From (d,0,0);
    DPoint3d edgePoint1 = DPoint3d::From (-1, y1,0);

    DEllipse3d ellipse;
    if (DEllipse3d::TryConstruct_EdgePoint_XPoint_EdgePoint_XAngle
        (
        ellipse,
        edgePoint0,
        xPoint,
        edgePoint1,
        alpha
        ))
        {
        DPoint3d point0, point1, xPointOut;
        ellipse.FractionParameterToPoint(point0, 0.0);
        ellipse.FractionParameterToPoint(point1, 1.0);
        xPointOut = DPoint3d::FromSumOf (ellipse.center, ellipse.vector0, 1.0);
        DPoint3d point1Local;
        ellipse.PointToXYLocal (point1Local, point1);
        //double a1 = ellipse.vector0.Magnitude ();
        //double b1 = ellipse.vector90.Magnitude ();
        double theta = atan2 (ellipse.vector0.y, ellipse.vector0.x);
        if (s_doChecks)
            {
            Check::Near (xPoint, xPointOut, "Ellipse x point");
            Check::Near (edgePoint0, point0, "Ellipse edge point");
            Check::Near (edgePoint1, point1, "Ellipse edge point");
            Check::True (
                    Angle::NearlyEqualAllowPeriodShift (alpha, theta)
                 || Angle::NearlyEqualAllowPeriodShift (alpha, theta + Angle::Pi ()),"Ellipse angle");
            Check::True (ellipse.vector0.IsPerpendicularTo (ellipse.vector90), "Right ellipse");
            Check::True (ellipse.vector0.CrossProductXY(ellipse.vector90) > 0.0, "Right handed");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, ConstructXPointEdgePointEdgePointAngle)
    {
    for (double d = 2.0; d < 5.0; d+= 0.5)
        {
        for (double alpha = 0.0; alpha < 0.1; alpha += 0.03)
            {
            ConstructXPointEdgePointEdgePointAngle_go (d, -1.0, 1.0, alpha);
            ConstructXPointEdgePointEdgePointAngle_go (d,  1.0, 1.0, alpha);
            ConstructXPointEdgePointEdgePointAngle_go (-d, -1.0, 1.0, alpha);
            ConstructXPointEdgePointEdgePointAngle_go (-d,  1.0, 1.0, alpha);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, ConstructXPointEdgePointEdgePointAxis)
    {
    // These coordinates have (by hand construction) 3 solutions:
    DPoint3d xPoint = DPoint3d::From (0,0,0);
    DPoint3d edgePoint0 = DPoint3d::From (41.4739, -80.847, 0);
    DPoint3d edgePoint1 = DPoint3d::From (2520.7458, -70.4044, 0);
    double a = 2003.0662;
    bvector<DEllipse3d> ellipse;
    DEllipse3d::Construct_XPoint_EdgePoint_EdgePoint_XRadius (
            ellipse, xPoint, edgePoint0, edgePoint1, a);
    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, FractionInterval)
    {
    DEllipse3d ellipse0 = DEllipse3d::From (0,0,0, 1,0,0, 0,1,0, 1,3);
    double a0 = 0.72;
    double a1 = 0.301;
    DEllipse3d ellipse1 = DEllipse3d::FromFractionInterval (ellipse0, a0, a1);
    Check::Near (ellipse0.FractionToAngle (a0), ellipse1.FractionToAngle (0.0), "FractionEllipse start");
    Check::Near (ellipse0.FractionToAngle (a1), ellipse1.FractionToAngle (1.0), "FractionEllipse start");
    }


void ExerciseEllipseConstructions (DEllipse3dCR ellipse0)
    {
    double theta0 = ellipse0.start;
    double dTheta = ellipse0.sweep;
    DPoint3d point0, point90, A0, A90;
    DVec3d dA0, ddA0, dA90, ddA90;
    ellipse0.Evaluate (point0, 0.0);
    ellipse0.Evaluate (point90, Angle::PiOver2 ());
    ellipse0.Evaluate (A0, dA0, ddA0, 0.0);
    ellipse0.Evaluate (A90, dA90, ddA90, -Angle::PiOver2 ());  // negative 90 degrees to get tangent forward
    DEllipse3d ellipse1 = DEllipse3d::FromVectors (ellipse0.center, dA90, dA0, theta0, dTheta);
    Check::Near (ellipse0, ellipse1, "tangent relations");
    DEllipse3d ellipse2 = DEllipse3d::FromPoints (ellipse0.center, point0, point90, theta0, dTheta);
    Check::Near (ellipse0, ellipse2, "point construction");

    double a0 = 1.2, a90 = 1.3;
    DEllipse3d ellipse3 = DEllipse3d::FromScaledVectors (
                ellipse0.center, ellipse0.vector0, ellipse0.vector90, a0, a90, theta0, dTheta);
    DVec3d normal0 = DVec3d::FromCrossProduct (ellipse0.vector0, ellipse0.vector90);
    DVec3d normal3 = DVec3d::FromCrossProduct (ellipse3.vector0, ellipse3.vector90);
    Check::Near (normal0.Magnitude () * a0 * a90, normal3.Magnitude (), "scaled vectors");
    Check::Near (ellipse0.vector0.MagnitudeSquared () * a0,
                 ellipse3.vector0.DotProduct (ellipse0.vector0),
                "scaled vectors");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, Init2)
    {
    double theta0 = 0.1;
    double dTheta = 1.2;
    DPoint3d center = DPoint3d::From (1,2,3);
    DEllipse3d ellipse0 = DEllipse3d::FromScaledRotMatrix (center,
                RotMatrix::FromPrincipleAxisRotations(RotMatrix::FromIdentity (), 1,2,3),
                2,3,   theta0, dTheta);
    ExerciseEllipseConstructions (ellipse0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, Init3)
    {
    
    DEllipse3d ellipse0 = DEllipse3d::FromPointsOnArc (
                DPoint3d::From (1,2,3),
                DPoint3d::From (4,0.3,1),
                DPoint3d::From (2,9,3));
    Check::True (ellipse0.IsCircular (), "3point arc");
    ExerciseEllipseConstructions (ellipse0);
    }

void testDEllipse3d_Construct_Point_Direction_TangentXY (
double x0, double y0,
double theta0,
double x1, double y1,
double theta1
)
    {
    Check::StartScope ("Circle Pt+tangent+ray");
    bvector<DEllipse3d> ellipse;
    bvector<double>    rayFraction;
    DPoint3d startA = DPoint3d::From (x0, y0, 0.0);
    DVec3d tangentA = 
          DVec3d::From (cos (theta0), sin(theta0), 0.0);
    DRay3d ray = DRay3d::FromOriginAndVector (
              DPoint3d::From (x1, y1, 0.0),
              DVec3d::From (cos(theta1), sin(theta1), 0.0)
              );
    DEllipse3d::Construct_Point_Direction_TangentXY
          (
          ellipse, rayFraction,
          startA,
          tangentA,
          ray
          );
    DVec3d dummy;
    Check::True (ellipse.size () > 0, "numEllipse > 0");
    for (size_t i = 0; i < ellipse.size (); i++)
        {
        DPoint3d startB, endB;
        DVec3d tangentStartB, tangentEndB;
        ellipse[i].FractionParameterToDerivatives (startB, tangentStartB, dummy, 0.0);
        ellipse[i].FractionParameterToDerivatives (endB, tangentEndB, dummy, 1.0);
        Check::Near (startB, startA, "through start");
        DPoint3d endC = ray.FractionParameterToPoint (rayFraction[i]);
        Check::Near (endB, endC, "ends on ray");
        Check::Parallel (tangentStartB, tangentA, "start tangent");
        Check::Parallel (tangentEndB, ray.direction, "end tangent");
        }
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d, StartDirection_TangentToRay)
    {
    testDEllipse3d_Construct_Point_Direction_TangentXY (1,1, 0.0, 0,0, 0.0);
    testDEllipse3d_Construct_Point_Direction_TangentXY (1,1, 1.0, 0,0, 0.0);
    testDEllipse3d_Construct_Point_Direction_TangentXY (100,105, 1.0, 54, 29, 0.10);
    }

void testParallelLineLinePoint (DPoint3dCR pointA, DPoint3dCR pointB, double angle, double signDirectionB, double fraction, int selectAB)
    {
    int numOut;
    Check::StartScope ("circleTTTLineAWithPointParallelLineB");
    DVec3d directionA = DVec3d::From (cos(angle), sin(angle), 0.0);
    DVec3d directionB = DVec3d::FromScale (directionA, signDirectionB);
    DPoint3d pointC;
    if (selectAB == 0)
        pointC = DPoint3d::FromSumOf (pointA, directionA, fraction);
    else 
        pointC = DPoint3d::FromSumOf (pointB, directionB, fraction);
    DPoint3d tangentA[8], tangentB[8], tangentC[8];
    DPoint3d center[8];
    double   radius[8];
    DRay3d rayA = DRay3d::FromOriginAndVector (pointA, directionA);
    DRay3d rayB = DRay3d::FromOriginAndVector (pointB, directionB);
    bsiGeom_circleTTTLineLineCircleConstruction
      (
      center, radius, tangentA, tangentB, tangentC, numOut, 8,
      pointA, directionA, pointB, directionB, pointC, 0.0);
    if (Check::Int (1, numOut, "degenerate intersection count"))
        {
        DPoint3d pointA1, pointB1;
        double fractionA1, fractionB1;
        Check::True (rayA.ProjectPointUnbounded (pointA1, fractionA1, center[0]), "project to rayA");
        Check::True (rayB.ProjectPointUnbounded (pointB1, fractionB1, center[0]), "project to rayA");
        Check::Near (pointA1, tangentA[0], "tangency point on A");
        Check::Near (pointB1, tangentB[0], "tangency point on B");
        if (selectAB == 0)
            Check::Near (pointA1, pointC, "known tangency point");
        else
            Check::Near (pointB1, pointC, "known tangency point");
        Check::Near (center[0].Distance (pointA1), center[0].Distance (pointB1), "equal radii");
        }
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(TTT,ParallelLineLinePoint)
    {
    static double fMax = 1.0e7;
    static double fFactor = 3.0 + sqrt (7.0);
    for (int selectAB = 0; selectAB < 2; selectAB++)
        {
        for (int selectDir = -1; selectDir < 2; selectDir += 2)
            {
            double s = (double) selectDir;
            testParallelLineLinePoint (
                DPoint3d::From (0,0,0),
                DPoint3d::From (0,1,0),
                0.0, s,
                0.0, selectAB);

            for (double f = 0.0; f < fMax; f = (f + 1.0) * fFactor)
                {
                testParallelLineLinePoint (
                    DPoint3d::From (110,45.233,0),
                    DPoint3d::From (4000.1232, 32.99,0),
                    1.529, s,
                    f, selectAB);
                  }
            }
        }
    }



#ifdef abc
-		conic	0x001c7230 {center={...} vector0={...} vector90={...} ...}	_dConic4d *
-		center	{x=1000000.0000000001 y=5055613.9615411852 z=-1434614.6192090772 ...}	_dPoint4d
		x	1000000.0000000001	double
		y	5055613.9615411852	double
		z	-1434614.6192090772	double
		w	1.0000000000000000	double
-		vector0	{x=181261.55740733005 y=-84523.652348139804 z=5.3010665289492597e-012 ...}	_dPoint4d
		x	181261.55740733005	double
		y	-84523.652348139804	double
		z	5.3010665289492597e-012	double
		w	0.00000000000000000	double
-		vector90	{x=-84523.652348139891 y=-181261.55740732988 z=2.3911565870609734e-011 ...}	_dPoint4d
		x	-84523.652348139891	double
		y	-181261.55740732988	double
		z	2.3911565870609734e-011	double
		w	0.00000000000000000	double
		start	3.1415926535897931	double
		sweep	3.1950068192435337	double
#endif

void CheckConicPoints (DConic4dCR conic0, DConic4dCR conic1, double fraction)
    {
    double angle0 = conic0.start + fraction * conic0.sweep;
    double angle1 = conic1.start + fraction * conic1.sweep;
    DPoint3d xyz0, xyz1;
    DVec3d tangent0, tangent1;
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&conic0, &xyz0, &tangent0, NULL, angle0);
    bsiDConic4d_angleParameterToDPoint3dDerivatives (&conic1, &xyz1, &tangent1, NULL, angle1);
    CHECK_EQ (Near, xyz0, xyz1);
    CHECK_EQ (Parallel, tangent0, tangent1);
    double sweepSign = conic0.sweep * conic1.sweep;
    CHECK_EXPR(True, sweepSign * tangent0.DotProduct (tangent1) > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DConic4d, CommonAxesFail0)
    {
    DConic4d conic;
    bsiDConic4d_init (&conic,
                      0,0,0,
                      1,0,0,
                      0,1,0,
                      3.1415926535897931,
                      3.1950068192435337);
    DConic4d axes;
    RotMatrix basis;
    int curveType;
    CHECK_EXPR (True, bsiDConic4d_getCommonAxes (&conic, &axes, &basis, &curveType));
    CheckConicPoints (conic, axes, 0.0);
    CheckConicPoints (conic, axes, 1.0);

    for (double sweep = -6.0; sweep < 6.1; sweep += 0.9)
        {
        conic.sweep = sweep;
        CHECK_EXPR (True, bsiDConic4d_getCommonAxes (&conic, &axes, &basis, &curveType));
        CheckConicPoints (conic, axes, 0.0);
        CheckConicPoints (conic, axes, 1.0);
        }

    for (double weight0 = 0.01; weight0 < 1.0; weight0 += 0.3)
        {
        conic.vector0.w = weight0;
        CHECK_EXPR (True, bsiDConic4d_getCommonAxes (&conic, &axes, &basis, &curveType));
        CheckConicPoints (conic, axes, 0.0);
        CheckConicPoints (conic, axes, 1.0);
        }
    }




double TestExactClosestPoint (DEllipse3dR ellipse, double f)
    {
    DPoint3d xyz;
    ellipse.FractionParameterToPoint (xyz, f);
    DPoint3d xyzA[5];
    double thetaA[5];
    int numA = ellipse.ProjectPointBounded (xyzA, thetaA, xyz);
    Check::True (numA > 0);
    double thetaB, d2B;
    DPoint3d xyzB;
    Check::True (ellipse.ClosestPointXYBounded (thetaB, d2B, xyzB, xyz));
    Check::Near (xyz, xyzB);
    return xyz.Distance (xyzB);
    }
double TestExactClosestPoint (DEllipse3dR ellipse, int numStep, int numDelta, double delta)
    {
    double maxDistance = 0.0;
    for (int i = 0; i <= numStep; i++)
        {
        double f = i / (double)numStep;
        maxDistance = DoubleOps::Max (maxDistance, TestExactClosestPoint (ellipse, f));
        for (int k = 0; k < numDelta; k++)
          {
          maxDistance = DoubleOps::Max (maxDistance, TestExactClosestPoint (ellipse, f + k * delta));
          maxDistance = DoubleOps::Max (maxDistance, TestExactClosestPoint (ellipse, f - k * delta));
          }
        }
    return maxDistance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,ClosestPointA)
    {
    DEllipse3d ellipse;
    double a = 1.0;
    double b = 2.0;
    ellipse.Init (0,a,0,     0,-a,0,   b,0,0, 0.0, Angle::TwoPi ());
    double epsilon = BeNumerical::BeNextafter (1.0, 100.0) - 1.0;
    TestExactClosestPoint (ellipse, 16, 10, epsilon);
    TestExactClosestPoint (ellipse, 7, 10, epsilon);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,ClosestPointB)
    {
    DEllipse3d ellipse;
    for (double a = 1.0; a < 1000.0; a *= 2.62137129678632)
        {
        ellipse.Init (a * 2.4, a * 4.8, a * -2.4,     3.9, 2.1, 0.2,   -1.2, 4.2, -0.3, 0.0, Angle::TwoPi ());
        double epsilon = BeNumerical::BeNextafter (1.0, 100.0) - 1.0;
        TestExactClosestPoint (ellipse, 16, 10, epsilon);
        TestExactClosestPoint (ellipse, 7, 10, epsilon);
        }
    }

void CheckIntersectionOfTangents(DEllipse3dCR baseEllipse, double startDegrees, double sweepDegrees, bool expectSuccess)
    {
    DEllipse3d ellipse = baseEllipse;
    ellipse.start = Angle::DegreesToRadians(startDegrees);
    ellipse.sweep = Angle::DegreesToRadians(sweepDegrees);
    DPoint3d xyzA, xyz0, xyz1;
    DVec3d tangent0, tangent1, kurl0, kurl1, vector0A, vector1A;
    bool stat = ellipse.IntersectionOfStartAndEndTangents(xyzA);
    if (Check::Bool(stat, expectSuccess) && stat)
        {
        ellipse.FractionParameterToDerivatives(xyz0, tangent0, kurl0, 0.0);
        ellipse.FractionParameterToDerivatives(xyz1, tangent1, kurl1, 1.0);
        vector0A = DVec3d::FromStartEnd(xyz0, xyzA);
        vector1A = DVec3d::FromStartEnd(xyz1, xyzA);
        if (ellipse.IsFullEllipse())
            {
            Check::Near(xyzA, xyz0, "Full Ellipse IntersectionOfStartEndTangents matches start point.");
            }
        else
            {
            Check::Parallel(tangent0, vector0A);
            Check::Parallel(tangent1, vector1A);
            }
      }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d, IntersectionOfEndTangents)
    {
    bvector<double> sweepDegreesArray {10, 20, 30, 45, 60, 85, 100, 125, 178};
    auto baseEllipse = DEllipse3d::From(1,2,3,
                5,1,0.2,
                2,8,-0.2,
                0,0);
    for (double startDegrees = -360; startDegrees <= 360; startDegrees += 45.0)
        {
        for (double sweepDegrees : sweepDegreesArray)
            {
            CheckIntersectionOfTangents(baseEllipse, startDegrees, sweepDegrees, true);
            CheckIntersectionOfTangents(baseEllipse, startDegrees, -sweepDegrees, true);
            CheckIntersectionOfTangents(baseEllipse, startDegrees, 180 + sweepDegrees, true);
            CheckIntersectionOfTangents(baseEllipse, startDegrees, -(180 + sweepDegrees), true);
            }
        CheckIntersectionOfTangents(baseEllipse, startDegrees, 180, false);
        CheckIntersectionOfTangents(baseEllipse, startDegrees, -180, false);
        CheckIntersectionOfTangents(baseEllipse, startDegrees, 360, true);
        CheckIntersectionOfTangents(baseEllipse, startDegrees, -360, true);
        }
    }
#ifdef IsEllipseCollapsedToLine
//! @return true if the ellipse is degenerate (i.e. vector0 and vector90 are colinear, or one is zero)
//! @param [out] startExtremaEnd linestring from startpoint to endpoint touching any major or minor axis points contained by the properly stroked ellipse.
//!   This may have 2 to 6 points.
//! @param [out] strokeA single line segment that is the projection of the ellipse onto its longest axis.
//! @param [out] strokeB single line segment that is the projection of the ellipse onto its shortest axis.
bool IsEllipseCollapsedToLine (DEllipse3dCR ellipse, bvector<DPoint3d> &startExtremaEnd, DSegment3dR stroke)
    {
    DEllipse3d majorMinorEllipse = DEllipse3d::FromPerpendicularAxes (ellipse);
    double a = vector0.Magnitude ();
    double b = vector90.Magnitude ();
    if (b > a)
        majorMinorEllipse = DEllipse3d::FromRotatedAxes (majorMinorEllipse, Angle::PiOver2 ());
    majorMinorEllipse = DEllipse3d::FromCopyWithPositiveSweep (majorMinorEllispe);
    DRange2d localRange;
    localRange.Init ();
    for (int step = -2; step < 

    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d, ClassifyDegenerate)
    {
    
    DSegment3d segment0X, segment0Y;
    DEllipse3d ellipse0 = DEllipse3d::From (0,0,0,    1,0,0,    0,0,0, Angle::DegreesToRadians (-45), Angle::DegreesToRadians (340));
    double d0 = ellipse0.GetMajorMinorRangeMidlines (segment0X, segment0Y);
    Check::True (d0 < Angle::SmallAngle () * segment0X.Length ());

    DSegment3d segment1X, segment1Y;
    DEllipse3d ellipse1 = DEllipse3d::From (0,0,0,    1,1,0,    -1,-1,0,
                Angle::DegreesToRadians (-45), Angle::DegreesToRadians (20));
    double d1 = ellipse1.GetMajorMinorRangeMidlines (segment1X, segment1Y);
    Check::True (d1 < Angle::SmallAngle () * segment1X.Length ());

    DSegment3d segment2X, segment2Y;
    DEllipse3d ellipse2 = DEllipse3d::From (1,1,1, 1,0.4,0, -0.2, 0.8,0, 0.0, Angle::TwoPi ());
    double d2 = ellipse2.GetMajorMinorRangeMidlines (segment2X, segment2Y);
    Check::True (d2 > Angle::SmallAngle () * segment2X.Length ());
    Check::True (segment2X.Length () > segment2Y.Length ());
    }

void TestSymmetricLineLine (double totalTurnRadians, double length)
    {

    DSpiral2dBaseP spiralA = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralB = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralC = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralD = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    double a = 100.0;
    double singleTurnRadians = totalTurnRadians * 0.5;
    DPoint3d lineA = DPoint3d::From (-a,0,0);
    DPoint3d lineB = DPoint3d::From (a * cos (totalTurnRadians), a * sin (totalTurnRadians), 0.0);
    DPoint3d lineAB = DPoint3d::From (0,0,0);
    DPoint3d lineA1, lineB1, jointAB;
    double rAB;
    DSpiral2dBase::SymmetricLineSpiralSpiralLineTransition (lineA, lineB, lineAB, length,
                *spiralA, *spiralB,
                lineA1, lineB1, jointAB, rAB);
    double s = totalTurnRadians > 0.0 ? 1.0 : -1.0;
    Check::True (lineA1.x < 0.0);
    Check::Near (lineA1.y, 0.0);
    Check::True (jointAB.y * s > 0.0);
    Check::True (jointAB.x  < 0.0);
    Check::True (lineB1.y * s > 0.0);
    Check::True (lineB1.x  > 0.0);

    Check::Near (lineAB.Distance (lineA1), lineAB.Distance (lineB1), "PI distance to tangency");
    Check::Near (spiralA->mCurvature1, -spiralB->mCurvature1, "JunctionCurvatures");
    double q = 0.2;
    Transform placement = Transform::FromRowValues (
                cos (q), -sin(q), 0, 2.3,
                sin(q) , cos(q), 0, 1.5,
                0,0,1,0
                );

    DPoint3d lineC = placement * lineA;
    DPoint3d lineD = placement * lineB;
    DPoint3d lineCD = placement * lineAB;
    DPoint3d lineC1, lineD1, jointCD;
    double rCD;
    DSpiral2dBase::SymmetricLineSpiralSpiralLineTransition (lineC, lineD, lineCD, length,
                *spiralC, *spiralD,
                lineC1, lineD1, jointCD, rCD);
    Check::Near (rAB, rCD, "transformed radius");
    Check::Near (placement * lineA1, lineC1, "lineC1");
    Check::Near (placement * lineB1, lineD1, "lineD1");
    Check::Near (placement * jointAB, jointCD, "jointCD");
    Check::Near (spiralA->mCurvature1, spiralC->mCurvature1);
    Check::Near (spiralB->mCurvature1, spiralD->mCurvature1);

    // spiral magic ....
    Check::Near (singleTurnRadians * 2.0 * rAB, length);
    delete spiralA; delete spiralB; delete spiralC; delete spiralD;
    }


void TestLineSpiralArcSpiralLine(double totalTurnRadians, double lengthA, double lengthB, double radiusAB)
    {

    DSpiral2dBaseP spiralA = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralB = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
#ifdef abc
    DSpiral2dBaseP spiralC = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBaseP spiralD = DSpiral2dBase::Create(DSpiral2dBase::TransitionType_Clothoid);
#endif
    DEllipse3d arcAB;
    double a = 100.0;
    DPoint3d lineA = DPoint3d::From (-a,0,0);
    DPoint3d lineB = DPoint3d::From (a * cos (totalTurnRadians), a * sin (totalTurnRadians), 0.0);
    DPoint3d lineAB = DPoint3d::From (0,0,0);
    DPoint3d lineA1, lineB1, spiralA1, spiralB1;
    DSpiral2dBase::LineSpiralArcSpiralLineTransition (lineA, lineB, lineAB,
                radiusAB, lengthA, lengthB,
                *spiralA, *spiralB,
                lineA1, lineB1, spiralA1, spiralB1, arcAB);
#ifdef abc
    double s = totalTurnRadians > 0.0 ? 1.0 : -1.0;
    Check::True (lineA1.x < 0.0);
    Check::Near (lineA1.y, 0.0);
    Check::True (jointAB.y * s > 0.0);
    Check::True (jointAB.x  < 0.0);
    Check::True (lineB1.y * s > 0.0);
    Check::True (lineB1.x  > 0.0);
    Check::Near (lineAB.Distance (lineA1), lineAB.Distance (lineB1), "PI distance to tangency");
    Check::Near (spiralA->mCurvature1, -spiralB->mCurvature1, "JunctionCurvatures");
    double q = 0.2;
    Transform placement = Transform::FromRowValues (
                cos (q), -sin(q), 0, 2.3,
                sin(q) , cos(q), 0, 1.5,
                0,0,1,0
                );

    DPoint3d lineC = placement * lineA;
    DPoint3d lineD = placement * lineB;
    DPoint3d lineCD = placement * lineAB;
    DPoint3d lineC1, lineD1, jointCD;
    double rCD;
    DEllipse3d arcCD;
    DSpiral2dBase::SymmetricLineSpiralSpiralLineTransition (lineC, lineD, lineCD, length,
                *spiralC, *spiralD,
                lineC1, lineD1, jointCD, rCD);
    Check::Near (rAB, rCD, "transformed radius");
    Check::Near (placement * lineA1, lineC1, "lineC1");
    Check::Near (placement * lineB1, lineD1, "lineD1");
    Check::Near (placement * jointAB, jointCD, "jointCD");
    Check::Near (spiralA->mCurvature1, spiralC->mCurvature1);
    Check::Near (spiralB->mCurvature1, spiralD->mCurvature1);
#endif
    delete spiralA; delete spiralB;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSpiral2D,SymmetricLineLine)
    {
    TestSymmetricLineLine (0.2, 10.0);
    TestSymmetricLineLine (-0.4, 10.0);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSpiral2D,LineSpiralArcSpiralLine)
    {
    TestLineSpiralArcSpiralLine (0.2, 10.0, 10, 20);
    TestLineSpiralArcSpiralLine (-0.4, 10.0, 10, 20);
    }

void TestSpiralStartShouderTargetA (DPoint2dCR start, DPoint2dCR shoulder, DPoint2dCR target)
    {
    DSpiral2dBase *spiralA = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);
    DSpiral2dBase *spiralB = DSpiral2dBase::Create (DSpiral2dBase::TransitionType_Clothoid);
    DPoint2d pointP, pointQ;
    if (Check::True (DSpiral2dBase::SymmetricPointShoulderTargetTransition (
                    start, shoulder, target,
                    *spiralA,
                    *spiralB,
                    pointP, pointQ), "DSpiral2dBase::SymmetricPointShoulderTargetTransition"))
        {
        DVec2d xVec = DVec2d::From (1.0, 0.0);

        Transform frameA = Transform::FromOriginAndXVector (start, xVec);
        Transform frameB = Transform::FromOriginAndXVector (pointP, xVec);

        auto cpA = ICurvePrimitive::CreateSpiral (*spiralA, frameA, 0.0, 1.0);
        auto cpB = ICurvePrimitive::CreateSpiral (*spiralB, frameB, 0.0, 1.0);
        auto cpM0 = ICurvePrimitive::CreateLine (DSegment3d::From (start, shoulder));
        auto cpM1 = ICurvePrimitive::CreateLine (DSegment3d::From (shoulder, target));
        Check::SaveTransformed (*cpM0);
        Check::SaveTransformed (*cpM1);
        Check::SaveTransformed (*cpA);
        Check::SaveTransformed (*cpB);
        }

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSpiral2D,StartShoulderTargetA)
    {
    TestSpiralStartShouderTargetA (DPoint2d::From (0,0), DPoint2d::From (10,0), DPoint2d::From (10,10));
    Check::Shift (15.0, 0.0, 0.0);
    TestSpiralStartShouderTargetA (DPoint2d::From (0,0), DPoint2d::From (5,-10), DPoint2d::From (10,5));
    Check::Shift (15.0, 0.0, 0.0);
    Check::ClearGeometry ("DSpiral2d.StartShoulderTargetA");
    }

void TestLineShift (bvector<DPoint3d> const &points)
    {
    auto c1 = CurveVector::CreateSpiralLineToLineShift (DSpiral2dBase::TransitionType_Clothoid, points[0], points[1], points[2], points[3]);
    auto c2 = ICurvePrimitive::CreateLineString (points);
    if (c1.IsValid ())
    Check::SaveTransformed (*c1);
    Check::SaveTransformed (*c2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSpiral2D,LineToLineShift)
    {
    TestLineShift (bvector<DPoint3d>
        {
        DPoint3d::From (0,0, 0),
        DPoint3d::From (1,0, 0),
        DPoint3d::From (3,4,0),
        DPoint3d::From (5,4,0)
        }
    );
    Check::Shift (10, 0, 0);
    TestLineShift(bvector<DPoint3d>
        {
        DPoint3d::From (-1,-1,0),
        DPoint3d::From (0,-1,0),
        DPoint3d::From (5,2,0),
        DPoint3d::From (7,2.5,0)
        }
        );
    Check::Shift (10,0,0);

    TestLineShift (bvector<DPoint3d>
        {
        DPoint3d::From (-1, -1, 0),
            DPoint3d::From (0,-0.85, 0),
            DPoint3d::From (5, 2, 0),
            DPoint3d::From (6, 1.0, 0)
        }
    );
    Check::Shift (10, 0, 0);

    Check::ClearGeometry("DSpiral2d.LineToLineShift");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSpiral2D,ScaledTransform)
    {
    Transform frame0 = Transform::FromRowValues
            (
            0,-1, 0, 5,
            1, 0, 0, 7,
            0, 0, 1, 0
            );
            
    Transform rotate = Transform::FromLineAndRotationAngle (DPoint3d::From (1,1,0), DPoint3d::From (1,1,5), 0.4);
    Transform rotateAndScale;
    double s = 12.0;
    rotateAndScale.ScaleMatrixColumns (rotate, s, s, s);
    auto spiral = ICurvePrimitive::CreateSpiralBearingRadiusBearingRadius
            (
            DSpiral2dBase::TransitionType_Clothoid,
            0.1, 100.0,
            0.2, 200.0,
            frame0,
            0.0, 1.0
            );
    auto spiral0 = spiral->Clone ();
    bvector<DPoint3d> stroke0, stroke1;
    auto options = IFacetOptions::CreateForCurves ();
    spiral->AddStrokes (stroke0, *options);
    double length0, length1;
    spiral0->Length (length0);
    spiral->TransformInPlace (rotateAndScale);
    spiral->AddStrokes (stroke1, *options);
    spiral->Length (length1);
    Check::Near (length0 * s, length1);
    Check::Near (rotateAndScale * stroke0.front (), stroke1.front ());
    Check::Near (rotateAndScale * stroke0.back(), stroke1.back());
    DSpiral2dPlacementCP placement0 = spiral0->GetSpiralPlacementCP ();
    DSpiral2dPlacementCP placement  = spiral->GetSpiralPlacementCP ();
    Check::Near (placement0->spiral->mLength * s, placement->spiral->mLength);
    Check::Near (placement0->spiral->mCurvature0, s * placement->spiral->mCurvature0);
    Check::Near (placement0->spiral->mCurvature1, s * placement->spiral->mCurvature1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MoveSemantics,BVectorDestructorAfterSwap)
    {
    bvector <double> a {1,2,3};
    bvector<double> b{5,4,6};
    std::swap (a,b);
    }



// Check if there is C1 continuity at end of primitive i . .
void CheckC1 (CurveVectorCR curves, size_t i)
    {
    auto ray0End = curves.at(i)->FractionToPointAndUnitTangent (1.0).Value ();
    auto ray1Start = curves.at(i+1)->FractionToPointAndUnitTangent (0.0).Value ();
    Check::Near (ray0End.origin, ray1Start.origin, "Tangent point A");
    Check::Parallel (ray0End.direction, ray1Start.direction, "Confirm tangency A");

    }

void TestMultiRadiusFillet (DPoint3dCR pointA, DPoint3dCR pointB, DPoint3dCR pointC, bool reverse)
    {
    double a = 15.0;
    SaveAndRestoreCheckTransform shifter (a,0,0);
    auto options = IFacetOptions::CreateForCurves ();
    bvector <double> radii;
    bvector<DPoint3d> cornerPoints {pointB, pointA, pointC,
            DPoint3d::FromInterpolate (pointC, 0.1, pointB)
            };
    for (size_t i = 2; i < 5; i++)
        {
        Check::SaveTransformed (cornerPoints);
        radii.push_back ((double)i);
        auto vectorA = DVec3d::FromStartEnd (pointA, pointB);
        auto vectorB = DVec3d::FromStartEnd (pointA, pointC);
        auto curves = CurveCurve::ConstructMultiRadiusBlend (pointA, vectorA, vectorB, radii, reverse);
        if (Check::True (curves.IsValid ()))
            {
            Check::SaveTransformed (*curves);
            bvector<DPoint3d> strokes;
            curves->AddStrokePoints (strokes, *options);
            for (size_t i = 0; i + 1 < curves->size (); i++)
                CheckC1 (*curves, i);
            }
        Check::Shift (0,a,0);
        }
    }

void testMultiRadiusFillet (bool reverse)
    {
    DPoint3d pointA = DPoint3d::From ( 0, 0,0);
    DPoint3d pointB = DPoint3d::From (10, 0,0);
    DPoint3d pointC = DPoint3d::From ( 0,10,0);
    for (auto delta : bvector<DVec3d> {DVec3d::From (0,0,0), DVec3d::From (2,1,0), DVec3d::From (-1,2,0)})
        {
        TestMultiRadiusFillet (pointA + delta, pointB, pointC, reverse);
        TestMultiRadiusFillet (pointA + delta, pointC, pointB, reverse);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,MultiRadiusFilletShortTurn)
    {
    testMultiRadiusFillet (false);
    Check::ClearGeometry ("DEllipse3d.MultiRadiusFilletShortTurn");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,MultiRadiusFilletLongTurn)
    {
    testMultiRadiusFillet (true);
    Check::ClearGeometry ("DEllipse3d.MultiRadiusFilletLongTurn");
    }


void testSymmetricFilletWithTaper
(
DPoint3dCR pointBefore,
DPoint3dCR corner,          //!< [in] corner
DPoint3dCR pointAfter,
double setback0,
double distance0,
double filletRadius,        //!< [in] fillet radius
double setback1,        //!< [in] distance from fillet tangency to nominal curb
double distance1,        //!< [in] length of (projection of) taper along the nominal curb
double offset0 = 0.0,
double offset1 = 0.0
)
    {
    DVec3d vectorA = pointBefore - corner;
    DVec3d vectorB = pointAfter - corner;
    Check::StartScope ("TaperFilletTaper in Corner");


    auto chain = CurveCurve::ConstructTaperFilletTaper
                    (
                    corner, vectorA, vectorB,
                    setback0,
                    distance0,
                    filletRadius,
                    setback1,
                    distance1,
                    offset0,
                    offset1);
    if (    Check::True (chain.IsValid (), "Chain returned")
        &&  Check::Size (3, chain->size (), "5 primitives")
        )
        {
        double length0, length2;
        chain->at(0)->Length (length0);
        chain->at(2)->Length (length2);
        double h0 = hypot (setback0, distance0);
        double h2 = hypot (setback1, distance1);
        Check::Near (h0, length0);
        Check::Near (h2, length2);
        DVec3d planeNormal = DVec3d::FromCrossProduct (vectorA, vectorB);
        auto ray0End = chain->at(0)->FractionToPointAndUnitTangent (1.0).Value ();
        auto ray1Start = chain->at(1)->FractionToPointAndUnitTangent (0.0).Value ();

        auto ray2End = chain->at(1)->FractionToPointAndUnitTangent (1.0).Value ();
        auto ray3Start = chain->at(2)->FractionToPointAndUnitTangent (0.0).Value ();
        Check::Near (ray0End.origin, ray1Start.origin, "Tangent point A");
        Check::Parallel (ray0End.direction, ray1Start.direction, "Confirm tangency A");

        Check::Near (ray2End.origin, ray3Start.origin, "Tangent point B");
        Check::Parallel (ray2End.direction, ray3Start.direction, "Confirm tangency B");

        DPoint3d xyzStart, xyzEnd;
        chain->GetStartEnd (xyzStart, xyzEnd);
        DRay3d rayA = DRay3d::FromOriginAndVector (corner, vectorA);
        DRay3d rayB = DRay3d::FromOriginAndVector (corner, vectorB);
        DPoint3d xyzProjectedStart, xyzProjectedEnd;
        double fA, fB;
        Check::True (rayA.ProjectPointUnbounded (xyzProjectedStart, fA, xyzStart));
        Check::True (rayB.ProjectPointUnbounded (xyzProjectedEnd, fB, xyzEnd));
        Check::Near (fabs (offset0), xyzStart.Distance (xyzProjectedStart), "start point");
        Check::Near (fabs (offset1), xyzEnd.Distance (xyzProjectedEnd), "end point");
        if (offset0 != 0.0)
            Check::True (vectorA.TripleProduct (DVec3d::FromStartEnd (corner, xyzStart), planeNormal) * offset0 > 0.0, "offset direction 0");
        if (offset1 != 0.0)
            Check::True (vectorB.TripleProduct (DVec3d::FromStartEnd (corner, xyzEnd), planeNormal) * offset1 < 0.0, "offset direction 1");
        if (s_noisy)
            Check::Print (chain, "SymmetricTaper");
        }
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,SymmetricFilletWithTaper)
    {
    for (double offset : bvector<double> {0.0, 0.1})
        {
        testSymmetricFilletWithTaper (DPoint3d::From (5,0,0), DPoint3d::From (0,0,0), DPoint3d::From (0,10,0), 0.1, 2.0, 1.5, 0.3, 2.0, offset, offset);
        testSymmetricFilletWithTaper (DPoint3d::From (5,0,0), DPoint3d::From (0,0,0), DPoint3d::From (0,10,0), 0.1, 2.0, 1.5, 0.3, 2.0, offset, -offset);
        testSymmetricFilletWithTaper (DPoint3d::From (5,0,0), DPoint3d::From (0.2,0.4,0), DPoint3d::From (0,10,0), 0.1, 2.0, 1.5, 0.3, 2.0, offset, 2.0 * offset);
        testSymmetricFilletWithTaper (DPoint3d::From (5,0,0), DPoint3d::From (0.2,0.4,0), DPoint3d::From (0,10,0), 0.1, 2.0, 1.5, 0.3, 2.0, offset, -2.0 * offset);
        }
    }

void TestPseudoOffsetConstruction (DEllipse3dCR arc0)   // ASSUME arc0.start == 0
    {
    DPoint3d pointA, pointB;
    DVec3d tangentA, tangentB, twistA, twistB;
    arc0.FractionParameterToDerivatives (pointA, tangentA, twistA, 0.0);
    arc0.FractionParameterToDerivatives (pointB, tangentB, twistB, 1.0);
    auto result = DEllipse3d::FromStartTangentSweepEndTangentXY (pointA, tangentA, pointB, tangentB, arc0.sweep);

    if (Check::True (result.IsValid (), "construction completed"))
        {
        if (!Check::Near (arc0, result.Value (), "ellipse match") || s_noisy)
            {
            Check::Print (arc0, "Input Arc");
            Check::Print (result.Value (), "Matched arc");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,PseudoOffsetConstruction)
    {
    for (auto b : bvector<double>{1.0, 1.5, 2.0, 0.5})
        {
        for (auto sweepDegrees : bvector<double>{90.0, 45.0, 132.0})
            {
            TestPseudoOffsetConstruction (
                DEllipse3d::From (
                    0,0,0,
                    1,0,0,
                    0,b,0,
                    0.0,
                    Angle::DegreesToRadians (sweepDegrees)
                    ));
            }
        }

    // And skewed vector cases (but always start at theta = 0)
        for (auto sweepDegrees : bvector<double>{90.0, 45.0, 132.0})
            {
            TestPseudoOffsetConstruction (
                DEllipse3d::From (
                    1,2,0,
                    3.4, 0.6,0,
                    -0.3, 2.0, 0,
                    0.0,
                    Angle::DegreesToRadians (sweepDegrees)
                    ));
            }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,FromStartTangentNormalRadiusSweep)
    {
    // Create short ellipses starting along x axis, with various combinations of 
    // sign of normal, radius, sweep
    for (double zSign: bvector<double>{1,-1})
        {
        DVec3d normal = DVec3d::From (0,0,zSign);
        for (double radius : bvector<double> {2.0, -2.0})
            {
            for (double sweep : bvector<double> {0.4, -0.4})
                {
                auto arc = DEllipse3d::FromStartTangentNormalRadiusSweep (
                        DPoint3d::From (0,0,0),
                        DVec3d::UnitX (),
                        normal,
                        radius,
                        sweep
                        );
                // unused - auto endPoint = arc.Value ().FractionToPoint (1.0);
                Check::Print (normal, "normal");
                Check::Print(radius, "radius");
                Check::Print(sweep, "sweep");
                Check::Print (arc.Value (), "arc");
                Check::LessThanOrEqual (0, 
                        radius * arc.Value ().vector0.TripleProduct (arc.Value ().vector90, normal) > 0.0,
                        "Arc Frame is orientation follows radius, normal"
                        );
                Check::Near (sweep, arc.Value ().sweep, "sweep applied directly");
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,CenterStartEnd)
  {
  DEllipse3d ellipse = DEllipse3d::From (0,10,0,   10,0,0, 0,10,0, 0.0, Angle::TwoPi ());
  for (double endDegrees = -50.0; endDegrees < 60.0; endDegrees += 20.0)
      {
      Angle startAngle = Angle::FromDegrees (0);
      Angle endAngle   = Angle::FromDegrees (endDegrees);
      DPoint3d startA, endA;
      ellipse.Evaluate (startA, startAngle.Radians ());
      ellipse.Evaluate (endA, endAngle.Radians ());
      DEllipse3d ellipseB =DEllipse3d::FromArcCenterStartEnd (ellipse.center, startA, endA);
      Check::Near (ellipse.center, ellipseB.center);
      DPoint3d startB, endB;
      ellipse.Evaluate (startB, startAngle.Radians ());
      ellipse.Evaluate (endB, endAngle.Radians ());
      Check::Near (startA, startB);
      Check::Near (endA, endB);
      }
  }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,Scott)
  {
  DPoint3d cc = DPoint3d::From (51934.74932643221, 57759.754516860063,0);
  DPoint3d pc = DPoint3d::From (52055.976152000003, 57957.891192000003,0);
  DPoint3d pt = DPoint3d::From (51860.118358500002, 57979.718849999997);
  double thetaA = DVec3d::FromStartEnd (cc, pc).AngleTo (DVec3d::FromStartEnd (cc, pt));
  double rA0 = cc.Distance (pc);
  double rA1 = cc.Distance (pt);
  Check::PrintIndent (2);    Check::Print (Angle::RadiansToDegrees (thetaA), "input sweep");
  Check::PrintIndent (2);    Check::Print ((rA1-rA0) / rA0, "relative radius difference (fractional)");
  DEllipse3d ellipseB =DEllipse3d::FromArcCenterStartEnd (cc, pc, pt);
  Check::PrintIndent (2);    Check::Print (pc, "startA");
  Check::PrintIndent (2);    Check::Print (pt,   "endA");


  Check::PrintIndent (2);    Check::Print (ellipseB.center,   "center");
  Check::PrintIndent (2);    Check::Print (ellipseB.vector0,  "vector0");
  Check::PrintIndent (2);    Check::Print (ellipseB.vector90, "vector90");
  Check::PrintIndent (2);    Check::Print (Angle::RadiansToDegrees (ellipseB.sweep),    "sweep degrees");
  DPoint3d startB, endB;
  ellipseB.EvaluateEndPoints (startB, endB);

  Check::PrintIndent (2);    Check::Print (startB, "startB");
  Check::PrintIndent (2);    Check::Print (endB,   "endB");
  Check::PrintIndent (2);    Check::Near (pc, startB);
//  Check::Near (pt, endB);
  Check::PrintIndent (2);    Check::Near (thetaA, ellipseB.sweep, "Angle Sweep");
  ICurvePrimitivePtr primitive = ICurvePrimitive::CreateArc (ellipseB);
  DEllipse3d ellipseC;
  primitive->TryGetArc (ellipseC);
  Check::PrintIndent (2);    Check::Near (ellipseB, ellipseC);
  double d = pt.Distance (endB);
  Check::PrintIndent (2);    Check::True (d < 1.0e-7 * rA0, "Radial shift -- input is slightly off radius");
  DPoint3d startPrim, endPrim;
  primitive->GetStartEnd (startPrim, endPrim);
  Check::PrintIndent (2);    Check::Near (startB, startPrim);
  Check::PrintIndent (2);    Check::Near (endB, endPrim);
  }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d,IsCCWSweepXY)
  {

  bvector <double> yValues {-1,1};
  bvector <double> sweepValues {-0.2, 0.2};
  for (double y: yValues)
      {
      for (double sweep : sweepValues)
          {
          DEllipse3d ellipse = DEllipse3d::From (
                0,0,0,
                1,0,0,
                0,y,0, 0.0, sweep);
          Check::Bool (sweep * y > 0, ellipse.IsCCWSweepXY ());
          }
      }
    }

// Construct a circular arcfrom center, start, and end, with flag to select among two possible orientations.
// (endpoint will not be on the arc if it is at a different radius than the start.)
DEllipse3d ConstructCircularArcWithXYOrientation
(
DPoint3dCR centerPt,    // center point of circular arc.
DPoint3dCR startPt,     // start point of circular arc
DPoint3dCR endPt,       // nominal end point.  If at different radius, it is projected onto the circle through the start point.
bool isCCW              // true for arc runnign CCW viewed from above.
)
    {
    auto arc = DEllipse3d::FromArcCenterStartEnd (centerPt, startPt, endPt);
    // This computes an ellispe with the SHORTER sweep from startPt to endPt.
    auto ellipseNormal = DVec3d::FromCrossProduct (arc.vector0, arc.vector90);
    // Force the ellipse axes to be right handed when viewed from above the xy plane
    if (ellipseNormal.z < 0.0)
        arc = arc.FromNegateVector90 (arc);
    // The short sweep may be going the opposite direction around the circle . . . . 
    // sign of sweep indicates whether the ellipse is ccw or cw
    bool isCCW0 = arc.sweep > 0;
    if (isCCW0 != isCCW)
        arc.ComplementSweep ();
    return arc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DEllipse3d,ConstructWithXYOrientation)
    {
    DPoint3d center = DPoint3d::From (1,2,3);
    DVec3d   vector0 = DVec3d::From (1,3,0);
    DVec3d   vector90 = DVec3d::FromCrossProduct (DVec3d::UnitZ (), vector0);

    for (bool ccw : bvector<bool> { true, false})
        {
        for (double ccwSweep : bvector<double> { 60.0, 90.0, 170.0, 225.0, 332.0})
            {
            DPoint3d startA = center + vector0;
            DPoint3d endA = center + cos (ccwSweep) * vector0 + sin (ccwSweep) * vector90;
            DEllipse3d arc = ConstructCircularArcWithXYOrientation (center, startA, endA, ccw);
            Check::Bool (ccw, arc.IsCCWSweepXY (), "sweep direction check");
            DPoint3d startB, endB;
            arc.FractionParameterToPoint (startB, 0.0);
            arc.FractionParameterToPoint (endB, 1.0);
            Check::Near (startA, startB);
            Check::Near (endA, endB);
            }
        }
    }

bool testConstructStartPointStartTangentCircleTangency
(
DPoint3dCR pointA,
DVec3dCR   tangentA,
DPoint3dCR centerB,
double     radiusB,
DPoint3d   biasPoint
)
    {
    bvector<DEllipse3d> ellipse;
    static double tangentMultiplier = pointA.Distance (centerB);
    DEllipse3d::Construct_Point_Direction_TangentToCircleXY (ellipse, pointA, tangentA, centerB, radiusB);
    if (ellipse.empty ())
        return false;
    auto lineA = ICurvePrimitive::CreateLine (DSegment3d::From (pointA, pointA + tangentMultiplier * tangentA));
    Check::SaveTransformed (*lineA);
    auto circleB = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (centerB, radiusB));
    Check::SaveTransformed (*circleB);
    for (auto &e : ellipse)
        {
        auto arc = ICurvePrimitive::CreateArc (e);
        Check::SaveTransformed (*arc);
        }

    // Find the arc that ends closest to the bias point.
    // choose its smaller part.
    // scale to make it visually different
    auto biasCircle = ICurvePrimitive::CreateArc (DEllipse3d::FromCenterRadiusXY (biasPoint, 0.1 * radiusB));
    Check::SaveTransformed (*biasCircle);

    auto closestArc = DEllipse3d::ClosestEllipse (ellipse, 1.0, biasPoint).Value ();
    closestArc.SelectSmallerSweep ();
    // unused - double displayScale = 0.95;
    closestArc.vector0.Scale (0.95);
    closestArc.vector90.Scale (0.95);
    auto displayArc = ICurvePrimitive::CreateArc (closestArc);
    Check::SaveTransformed (*displayArc);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,ConstructStartPointStartTangentCircleTangency)
    {
    double shiftY = -30.0;
    double shfitX = 60.0;
    // unused - double b = 10.0;
    for (auto b : bvector<double> {10.0, 5.0, 1.0, -1.0, -5.0, -10.0})
        {
        auto pointA = DPoint3d::From (0,0,0);
        auto directionA = DVec3d::From (0,1,0);
        auto centerB = DPoint3d::From (1,b,0);
        auto directionBA = pointA - centerB;
        auto biasPoint = pointA + DVec3d::FromCCWPerpendicularXY (directionBA);
        auto radiusB = 2.0;
        testConstructStartPointStartTangentCircleTangency (pointA, directionA, centerB, radiusB, biasPoint);
        Check::Shift (0,shiftY,0);
        testConstructStartPointStartTangentCircleTangency (pointA, -1.0 * directionA, centerB, radiusB, biasPoint);
        Check::Shift (shfitX, -shiftY, 0);
        }
    Check::ClearGeometry ("DEllipse3d.ConstructStartPointStartTangentCircleTangency");
    }

void TestSweepToPlane (DEllipse3dCR spaceEllipse, DPlane3dCR plane, DVec3d sweepDirection)
    {
    auto targetEllipse = DEllipse3d::FromEllipseSweptToPlane (spaceEllipse, plane, sweepDirection);
    for (double f : bvector<double>{0,0.25, 0.5, 0.78, 1.0})
        {
        DPoint3d pointA = spaceEllipse.FractionToPoint (f);
        DPoint3d pointB = targetEllipse.Value ().FractionToPoint (f);
        DVec3d pointSweep = pointB - pointA;
        Check::Parallel (sweepDirection, pointSweep, "fractional ellipse point moves in sweep direction");
        Check::NearZero (plane.Evaluate (pointB), "Swept ellipse point is on plane");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,SweepToPlane)
    {
    DVec3d unitZ = DVec3d::UnitZ ();
    DVec3d slant = DVec3d::From (1,2,9);
    DPlane3d targetZ1 = DPlane3d::FromOriginAndNormal (DPoint3d::From (0,0,1),  unitZ);
    DEllipse3d baseEllipse = DEllipse3d::From (0,0,0,  1,0,0,  0,1,0, 0, 2);
    TestSweepToPlane (baseEllipse, targetZ1, unitZ);
    TestSweepToPlane (baseEllipse, targetZ1, slant);
    DPlane3d skewTarget = DPlane3d::FromOriginAndNormal (1,2,3, 1,-2,1);
    // unused - DEllipse3d skewEllipse = DEllipse3d::From (0.2,2,1,   4,5,-1, 3,8,11, 0.0, Angle::Pi ());
    TestSweepToPlane (baseEllipse, skewTarget, slant);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Earlin.Lutz                    05/17
//---------------------------------------------------------------------------------------
TEST(DEllipse3d, ArcLengthTolerance) 
    {
    auto arc = DEllipse3d::From
        (
        302415.36920969415, 256189.85380689989, 0.00000000000000000,
        -68.208760403853375, -73.127047008398222, 0.00000000000000000,
        73.127047008398208, -68.208760403853361, -0.00000000000000000,
        -0.95777356280688086,
        6.2831853071795862
        );
    auto cp = ICurvePrimitive::CreateArc (arc);
    for (double fraction0 : bvector<double>{1.0, 0.9})
        {
        for (auto signedDistanceA :
            bvector<double> {-10.0, -1.0, -1.4210854715202004e-14, 1.4210854715202004e-14, 1.0})
            {
            CurveLocationDetail location;
            Check::True (cp->PointAtSignedDistanceFromFraction
                    (
                fraction0, signedDistanceA, false,
                location
                ));
            double signedDistanceB;
            Check::True (cp->SignedDistanceBetweenFractions (fraction0, location.fraction, signedDistanceB));
            if (location.a < signedDistanceA)
                {
                Check::Near (1.0, location.fraction);
                }
            else if (location.a > signedDistanceA)
                {
                Check::Near (0.0, location.fraction);
                }
            else
                {
                Check::Near (signedDistanceA, signedDistanceB);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,CircleCircleTangentLines)
    {
    DPoint3d centerA = DPoint3d::From (0,0,0);
    DPoint3d centerB = DPoint3d::From (10,0,0);

    for (bool flip : bvector<bool>{false, true})
        {
        SaveAndRestoreCheckTransform shifter (0,120,0);
        for (double rB : bvector<double> {1.0, 0.0, 4.0})
            {
            SaveAndRestoreCheckTransform shifter (0,30,0);
            for (double rA : bvector<double> {1.0, 0.0, 4.0})
                {
                SaveAndRestoreCheckTransform shifter (20,0,0);
                Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (centerA, rA));
                Check::SaveTransformed (DEllipse3d::FromCenterRadiusXY (centerB, rB));
                DPoint2d uvA, uvB;
                if (DEllipse3d::ConstructTangentLineRatios (centerA.Distance (centerB), rA, rB, flip, uvA, uvB))
                    {
                    auto pointA = DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvA.x, centerB, uvA.y);
                    auto pointB = DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvB.x, centerB, uvB.y);
                    Check::SaveTransformed (bvector<DPoint3d>{pointA, pointB});
                    auto pointA1 = DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvA.x, centerB, -uvA.y);
                    auto pointB1 = DPoint3d::FromInterpolateAndPerpendicularXY (centerA, uvB.x, centerB, -uvB.y);
                    Check::SaveTransformed (bvector<DPoint3d>{pointA1, pointB1});
                    }
                }
            }
        }
    Check::ClearGeometry ("DEllipse3d.CircleCircleTangentLines");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,STurn)
    {
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DVec3d tangentA = DVec3d::From (0,1,0);
    DPoint3d pointB0 = DPoint3d::From (8,10,0);
    DVec3d tangentB0  = DVec3d::From (0,1,0);
    double tangentLength = 3.0;

    for (double uxB : bvector<double>{0.0, 0.1, -1.0, })
        {
        SaveAndRestoreCheckTransform shifter (150, 0 ,0);
        for (double rA : bvector<double> {1.0, 4.0, -1.0, -4.0})
            {
            for (double rB : bvector<double> {1.0, 3.5, -3.5, -1.0})
                {
                SaveAndRestoreCheckTransform shifter (0,45,0);
                for (double sBX : bvector<double> {1.0, -1.0})
                    {
                    for (double sBY : bvector<double> {1.0, -1.0})
                        {
                        SaveAndRestoreCheckTransform shifter (25,0,0);
                        auto pointB = pointB0;
                        pointB.x *= sBX;
                        pointB.y *= sBY;
                        DVec3d tangentB = tangentB0;
                        tangentB.x = uxB;
                        Check::SaveTransformed (bvector<DPoint3d>{ pointA, pointA - tangentLength * tangentA});
                        Check::SaveTransformed (bvector<DPoint3d>{ pointB, pointB + tangentLength * tangentB});
                        DEllipse3d arcA, arcB;
                        DSegment3d segment;
                        if (DEllipse3d::Construct_ArcLineArc_PointTangentRadius_PointTangentRadiusXY
                                (
                                pointA, tangentA, rA,
                                pointB, tangentB, rB,
                                arcA, segment, arcB
                                )
                            )
                            {
                            Check::SaveTransformed (arcA);
                            Check::SaveTransformed (bvector<DPoint3d>{segment.point[0], segment.point[1]});
                            Check::SaveTransformed (arcB);
                            }
                        }
                    }
                }
            }
        }

    Check::ClearGeometry ("DEllipse3d.STurn");
    }

#ifdef VerifyCivilArcConstruction
// This xml fragment is given (Scott Devoe, Oct 22 2017)
// The code below makes an arc that matches.
#ifdef RawXMLArcDefinitions
<Curve rot="ccw" crvType="arc" radius="550.000000000" length="1558.934318418" chord="1087.052172758" dirStart="2.201767682562" dirEnd="5.650526956255" staStart="363.352882000"> 
<Start code="PC"> 
 1495977.055739300 556616.979535724 105.109956645 
</Start> 
<Center code="CC"> 
 1496421.155911220 556941.440684858 0.000000000 
</Center> 
<End code="PT"> 
 1496746.365793140 557384.992860710 124.597816952 
</End>

#endif

// return the angle (in Radians) from the positive Y axis to the xy part of the vector.
double CCWAngleFromY (DVec3dCR vector)
    {
    double radians = atan2 (-vector.x, vector.y);
    if (radians < 0.0)
        return radians + Angle::TwoPi ();
    return radians;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  11/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,ScottDConstructionFromXML)
    {
    auto start = DPoint3d::From ( 1495977.055739300, 556616.979535724,105.109956645);
    auto center = DPoint3d::From ( 1496421.155911220, 556941.440684858, 0.000000000);
    auto end = DPoint3d::From (1496746.365793140, 557384.992860710,124.597816952);

    auto centerToStart = DVec3d::FromStartEnd (center, start);
    auto centerToEnd   = DVec3d::FromStartEnd (center, end);

    auto radius = 550.0;
    double dirStart = 2.201767682562;
    double dirEnd   = 5.650526956255;

    // length of vectors for visualizing start end bearings:
    double startTangentLength = radius * 0.4;
    double endTangentLength = 0.25 * startTangentLength;

    auto startTarget = DPoint3d::FromSumOf (start, DVec3d::From (-startTangentLength * cos (dirStart), -startTangentLength * sin (dirStart)));
    auto endTarget = DPoint3d::FromSumOf (end, DVec3d::From ( -endTangentLength * cos (dirEnd), -endTangentLength * sin (dirEnd)));


    auto startRadiusLines = bvector<DPoint3d>
        {
        DPoint3d::FromInterpolateAndPerpendicularXY (start, 0.8, startTarget, -0.2),
        startTarget,
        start,
        center,
        };

    auto endRadiusLines = bvector<DPoint3d>
        {
        center,
        end,
        endTarget,
        DPoint3d::FromInterpolateAndPerpendicularXY (end, 0.8, endTarget, -0.2)
        };


    auto xyArc = DEllipse3d::FromCenterRadiusXY (center, radius);
    Check::SaveTransformed (startRadiusLines);
    Check::SaveTransformed (endRadiusLines);
    double markerSize = 10.0;
    Check::SaveTransformedMarker (center, markerSize);
    Check::SaveTransformed (xyArc);

    auto radius0 = center.Distance (start);
    auto radius1 = center.Distance (end);

    GEOMAPI_PRINTF (" Stated Radius %.17le\n", radius);
    GEOMAPI_PRINTF (" Actual center to start %.17le     XY %.17le\n", center.Distance (start), center.DistanceXY (start));
    GEOMAPI_PRINTF (" Actual center to start %.17le     XY %.17le\n", center.Distance (end), center.DistanceXY (end));

    GEOMAPI_PRINTF (" Stated start and end bearing %.17le %.17le\n", dirStart, dirEnd);
    GEOMAPI_PRINTF (" Angle:  X to Center-to-start %.17le\n", CCWAngleFromY (centerToStart));
    GEOMAPI_PRINTF (" Angle:  X to Center-to-end %.17le\n", CCWAngleFromY (centerToEnd));

    auto xyArcAtStartZ = DEllipse3d::From (
                center.x, center.y, start.z,        // center point, but z from start
                0, radius, 0,                       // positive Y direction is 0-degrees for bearing measurements
                -radius, 0, 0,                      // negative X direction is 90-degrees for bearing measurements
                dirStart,                           // start bearing (FROM NORTH TO WEST)
                dirEnd - dirStart                   // sweep bearing (NORTH TO WEST (i.e. CCW) is positive)
                );
    Check::SaveTransformed (xyArcAtStartZ);
    Check::ClearGeometry ("DEllipse3d.ScottDConstructionFromXML");

    }
#endif
void ShowCircles (DPoint3dCP center, DPoint3dCP edgeA, DPoint3dCP edgeB, DPoint3dCP edgeC, int n)
    {
    for (int i = 0; i < n; i++)
        {
        auto circle = DEllipse3d::FromCenterRadiusXY (center[i], center[i].Distance (edgeA[i]));
        Check::SaveTransformed (circle);
        if (edgeA)
            Check::SaveTransformedMarker (edgeA[i], 0.02);
        if (edgeB)
            Check::SaveTransformedMarker (edgeB[i], 0.02);
        if (edgeC)
            Check::SaveTransformedMarker (edgeC[i], 0.02);
        }
    }
#define MAX_TANGENT 20
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,Circle_CCR)
    {

    double rA = 1.0;
    double rB = 0.5;
    double rC = 2.0;    
    for (double a : {0.75, 1.25, 1.50, 1.75, 4.0})
        {
        SaveAndRestoreCheckTransform shifter (10,0,0);
        auto circleA = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (1,1), rA);
        auto circleB = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (a,a), rB);
        Check::SaveTransformed (circleA);
        Check::SaveTransformed (circleB);
        DPoint3d centers[MAX_TANGENT];
        DPoint3d tangentA[MAX_TANGENT];
        DPoint3d tangentB[MAX_TANGENT];
        int n;
        bsiGeom_circleTTCircleCircleConstruction (centers, tangentA, tangentB, n, MAX_TANGENT,
                    circleA.center, rA, circleB.center, rB, rC);
        ShowCircles (centers, tangentA, tangentB, nullptr, n);
        }
    Check::ClearGeometry ("DEllipse3d.Circle_CCR");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,Circle_CLR)
    {

    double rA = 1.0;
    double rC = 3.0;
    for (double xB : {1.0, 4.0})
        {
        SaveAndRestoreCheckTransform shifter (10,0,0);
        auto circleA = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (1,1), rA);
        Check::SaveTransformed (circleA);
        auto segmentB = DSegment3d::From (xB, -10, 0, xB + 1, 10.0, 0);
        Check::SaveTransformed (segmentB);
        DPoint3d centers[MAX_TANGENT];
        DPoint3d tangentA[MAX_TANGENT];
        DPoint3d tangentB[MAX_TANGENT];
        int n;
        bsiGeom_circleTTLineCircleConstruction (centers, tangentA, tangentB, n, MAX_TANGENT,
                    segmentB.point[0],
                    segmentB.point[1] - segmentB.point[0],
                    circleA.center, rA,
                    rC);
        ShowCircles (centers, tangentA, tangentB, nullptr, n);
        }
    Check::ClearGeometry ("DEllipse3d.Circle_CLR");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,Circle_CCC)
    {

    double rA = 1.0;
    double rB = 2.0;
    double rC = 3.0;
    for (double xB : {2.0, 5.0, 8.0})
        {
        SaveAndRestoreCheckTransform shifter (30,0,0);
        auto circleA = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (1,1), rA);
        auto circleB = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (xB,0), rB);
        auto circleC = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (5,0), rC);
        Check::SaveTransformed (circleA);
        Check::SaveTransformed (circleB);
        Check::SaveTransformed (circleC);
        DPoint3d centers[MAX_TANGENT];
        double radii[MAX_TANGENT];
        DPoint3d tangentA[MAX_TANGENT];
        DPoint3d tangentB[MAX_TANGENT];
        DPoint3d tangentC[MAX_TANGENT];
        int n;
        bsiGeom_circleTTTCircleCircleCircleConstruction (centers, radii, tangentA, tangentB, tangentC, n, MAX_TANGENT,
                    circleA.center, rA,
                    circleB.center, rB,
                    circleC.center, rC);
        ShowCircles (centers, tangentA, nullptr, nullptr, n);
        }
    Check::ClearGeometry ("DEllipse3d.Circle_CCC");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,Circle_LLCParallel)
    {
    double rC = 1.5;
    for (double xC : {0.0, 1.0, 2.0, 2.5, 6.0})
        {
        SaveAndRestoreCheckTransform shifter (30,0,0);
        auto segmentA = DSegment3d::From (0,0,0,  0,10,0);
        auto segmentB = DSegment3d::From (5,0,0,  5,10,0);
        auto circleC = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (xC,4), rC);
        Check::SaveTransformed (segmentA);
        Check::SaveTransformed (segmentB);
        Check::SaveTransformed (circleC);
        DPoint3d centers[MAX_TANGENT];
        double radii[MAX_TANGENT];
        DPoint3d tangentA[MAX_TANGENT];
        DPoint3d tangentB[MAX_TANGENT];
        DPoint3d tangentC[MAX_TANGENT];
        int n;
        bsiGeom_circleTTTLineLineCircleConstruction (centers, radii, tangentA, tangentB, tangentC, n, MAX_TANGENT,
                    segmentA.point[0], segmentA.point[1] - segmentA.point[0],
                    segmentB.point[0], segmentB.point[1] - segmentB.point[0],
                    circleC.center, rC);
        ShowCircles (centers, tangentA, nullptr, nullptr, n);
        }
    Check::ClearGeometry ("DEllipse3d.Circle_LLCParallel");
    }

void ShowEllipseLineData (
DEllipse3dCR arc,
DSegment3dCR segment,
DPoint3d xyz[],
DPoint3d ellipseParams[],
double lineParams[],
int n,
DVec3dCR viewVector     // Expect intersection in this view direction.
)
    {
    Check::SaveTransformed (arc);
    Check::SaveTransformed (segment);

    double a = 5.0;
    for (auto i = 0; i < n; i++)
        {
        DPoint3d xyzA = segment.FractionToPoint (lineParams[i]);
        DPoint3d xyzB;
        arc.Evaluate (xyzB, ellipseParams[i].x, ellipseParams[i].y);
        Check::SaveTransformedMarker (xyzA);
        Check::SaveTransformedMarker (xyzB);
        DPoint3d xyzC = xyz[i] + viewVector * a;
        DPoint3d xyzD = xyz[i] - viewVector * a;
        Check::SaveTransformed (DSegment3d::From (xyzC, xyzD));
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,IntersectSweptLineSegment)
    {
    double lineParams[5];
    DPoint3d ellipseParams[5];
    double ellipseAngle[5];
    DPoint3d xyz[5];
    bool isTangency[5];
    auto arc = DEllipse3d::From (1,0,0, 4,1,1, -1,-5,1,
            Angle::DegreesToRadians (-20),
            Angle::DegreesToRadians (220));
    auto unitNormal = DVec3d::FromNormalizedCrossProduct (arc.vector0, arc.vector90);
    auto unitZ = DVec3d::From (0,0,1);
    for (double xB : {0.0, 4.0, 8.0})
        {
        SaveAndRestoreCheckTransform shifter (0, 30, 0);
        auto segment = DSegment3d::From (-8,0,0,  xB,1,0);
        auto n = arc.IntersectSweptDSegment3d (xyz, ellipseParams, lineParams, segment);
        ShowEllipseLineData (arc, segment, xyz, ellipseParams, lineParams, n, unitNormal);
        Check::Shift (30,0,0);
        n = arc.IntersectSweptDSegment3dBounded (xyz, ellipseParams, lineParams, segment);
        ShowEllipseLineData (arc, segment, xyz, ellipseParams, lineParams, n, unitNormal);

        Check::Shift (30, 0, 0);
        n = arc.IntersectXYLineBounded (xyz, lineParams, ellipseParams, ellipseAngle, isTangency,
                        segment.point[0], segment.point[1]);
        ShowEllipseLineData (arc, segment, xyz, ellipseParams, lineParams, n, unitZ);
        }
    Check::ClearGeometry ("DEllipse3d.IntersectSweptLineSegment");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,ClosestApproachDRay3d)
    {
    double rayFraction[5];
    double arcAngle[5];
    DPoint3d xyzArc[5];
    DPoint3d xyzRay[5];
    auto arc = DEllipse3d::From (1,0,0, 4,1,1, -1,-5,1,
            Angle::DegreesToRadians (-20),
            Angle::DegreesToRadians (220));
    DPoint3d rayOrigin = DPoint3d::From(-2,0,1);
    for (auto vector : {
                DVec3d::From (4,0,3),
                DVec3d::From (1,5,-3)
                })
        {
        SaveAndRestoreCheckTransform shifter (0, 30, 0);
        auto ray = DRay3d::FromOriginAndVector (rayOrigin, vector);

        auto n = arc.ClosestApproach (arcAngle, rayFraction, xyzArc, xyzRay, ray);
        Check::SaveTransformed (arc);
        Check::SaveTransformed (DSegment3d::From (ray.origin, ray.origin + ray.direction));
        for (int i = 0; i < n; i++)
            {
            Check::SaveTransformed (DSegment3d::From (xyzArc[i], xyzRay[i]));
            DPoint3d xyzA;
            DVec3d   tangentA, kurvA;
            arc.Evaluate (xyzA, tangentA, kurvA, arcAngle[i]);
            Check::Near (xyzArc[i], xyzA);
            Check::Perpendicular (vector, xyzRay[i] - xyzArc[i]);
            }
        }
    Check::ClearGeometry ("DEllipse3d.ClosestApproachDRay3d");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,InitArcFromPointPointArcLength)
    {
    DPoint3d pointA = DPoint3d::From (1,3,2);
    DPoint3d pointB = DPoint3d::From (4,5,1);
    double a = pointA.Distance (pointB);
    DVec3d planeVector = DVec3d::From (3,6,1);
    double markerSize = 0.05;
    // shorter arc length should fail ..
    DEllipse3d arc;
    Check::False (arc.InitArcFromPointPointArcLength (pointA, pointB, 0.5 * a, planeVector));
    for (double factor : {3.0, 1.1, 5.0})
        {
        SaveAndRestoreCheckTransform shifter (10, 0, 0);
        double arcLength = factor * a;

        Check::SaveTransformedMarker (pointA, markerSize);
        Check::SaveTransformedMarker (pointB, markerSize);
        if (Check::True (arc.InitArcFromPointPointArcLength (pointA, pointB, arcLength, planeVector)))
            {
            Check::Near (arcLength, arc.ArcLength ());
            Check::SaveTransformed (arc);
            DPoint3d midPoint = DPoint3d::FromInterpolate (pointA, 0.5, pointB);
            Check::SaveTransformed (
                DSegment3d::From (midPoint, midPoint + planeVector));
            auto normal = DVec3d::FromCrossProduct (arc.vector0, arc.vector90);
            Check::Near (pointA, arc.FractionToPoint (0.0));
            Check::Near (pointB, arc.FractionToPoint (1.0));
            Check::Perpendicular (normal, planeVector);
            }
        }
    Check::ClearGeometry ("DEllipse3d.InitArcFromPointPointArcLength");
    }
bool VerifyArcMatch (DEllipse3dCR arc0, DEllipse3dCR arc1)
    {
    // 5 points define an unbounded conic.
    // fractional parameterization is preserved by axis "rotations"
    for (auto f : {0.0, 0.2, 0.4, 0.6, 0.8, 1.0})
        {
        auto point0 = arc0.FractionToPoint (f);
        auto point1 = arc1.FractionToPoint (f);
        if (!Check::Near (point0, point1))
            return false;
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,DgnFields3d)
    {
    for (double sweepDegrees : {360, 290 , -81})
        {
        SaveAndRestoreCheckTransform shifter (20, 0, 0);
        DEllipse3d arc0 = DEllipse3d::From (
            3,4,5,
            6,1,0,
            -1,4,0,
            Angle::DegreesToRadians (20), Angle::DegreesToRadians (sweepDegrees)
            );

        double quat [4];
        DVec3d vector0, vector90;
        double r0, r90;
        DPoint3d center;
        double start, sweep;
        arc0.GetDGNFields3d (center, quat, vector0, vector90, r0, r90, start, sweep);
        DEllipse3d arc1;
        arc1.InitFromDGNFields3d (center, quat, r0, r90, start, sweep);
        VerifyArcMatch (arc0, arc1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,DgnFields2d)
    {
    for (double sweepDegrees : {360, 290 , -81})
        {
        SaveAndRestoreCheckTransform shifter (20, 0, 0);
        DEllipse3d arc0 = DEllipse3d::From (
            3,4,0,
            6,1,0,
            -1,4,0,
            Angle::DegreesToRadians (20), Angle::DegreesToRadians (sweepDegrees)
            );

        double xAngle;
        DVec2d direction0;
        double r0, r90;
        DPoint2d center;
        double start, sweep;
        arc0.GetDGNFields2d (center, xAngle, direction0, r0, r90, start, sweep);
        DEllipse3d arc1;
        arc1.InitFromDGNFields2d (center, xAngle, r0, r90, start, sweep, 0.0);
        VerifyArcMatch (arc0, arc1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,InitFromCenterMajorAxisPointAndThirdPoint)
    {
    DEllipse3d arc0;
    auto center = DPoint3d::From (1,2,2);
    auto majorPoint = DPoint3d::From (8,5,1);
    auto pointA = DPoint3d::From (5,3,0);

    if (Check::True (arc0.InitFromCenterMajorAxisPointAndThirdPoint (
            center, majorPoint, pointA)))
        {
        Check::Near (center, arc0.center);
        Check::Near (majorPoint, arc0.FractionToPoint (0.0));
        Check::Near (pointA, arc0.FractionToPoint (1.0));
        Check::ExactDouble (arc0.start, 0.0);
        }
    }

struct ArcPoints
{
DPoint3d m_xyz[10];
double   m_a[10];
int m_numPoints;
int findDouble (double r, double tol = 1.0e-12)
    {
    for (int i = 0; i < m_numPoints; i++)
        {
        if (fabs (r - m_a[i]) <= tol)
            return i;
        }
    return -1;
    }
int findXYZ (DPoint3dCR xyz, double tol = 1.0e-12)
    {
    for (int i = 0; i < m_numPoints; i++)
        {
        if (xyz.Distance (m_xyz[i]) <= tol)
            return i;
        }
    return -1;
    }

};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,ProjectPointXY)
    {
    double sweepDegrees = 185.0;
    DEllipse3d arc0 = DEllipse3d::From (
        3,4,5,
        6,1,0,
        -1,4,1,
        Angle::DegreesToRadians (20), Angle::DegreesToRadians (sweepDegrees)
        );
    for (auto fraction : {0.0, 0.5, 1.5})
        {
        auto frenetFrame1 = arc0.FractionToFrenetFrame(fraction);
        Check::True (frenetFrame1.IsValid ());
        auto frenetFrame = frenetFrame1.Value ();
        auto arcPoint = frenetFrame.Origin ();
        ArcPoints searchPoints;
        searchPoints.m_numPoints = arc0.ProjectPointXY (searchPoints.m_xyz, searchPoints.m_a, arcPoint);
        Check::True (searchPoints.findXYZ (arcPoint) >= 0);
        DPoint3d xyzA;
        frenetFrame.Multiply (xyzA, 0, -0.1, 0); // outside point
        searchPoints.m_numPoints = arc0.ProjectPoint (searchPoints.m_xyz, searchPoints.m_a, xyzA);
        Check::True (searchPoints.findXYZ (arcPoint) >= 0);


        }
    }
void ShowLocalRange (TransformCR transform, DRange3dCR localRange)
    {
    DPoint3d corners[8];
    localRange.Get8Corners (corners);
    transform.Multiply (corners, 8);
    Check::SaveTransformed (bvector<DPoint3d> {
            corners[0], corners[1], corners[3], corners[2], corners[0]});
    Check::SaveTransformed (bvector<DPoint3d> {
            corners[4], corners[5], corners[7], corners[6], corners[4]});
    Check::SaveTransformed (bvector<DPoint3d> {
            corners[0], corners[4], corners[6], corners[2], corners[0]});
    Check::SaveTransformed (bvector<DPoint3d> {
            corners[1], corners[5], corners[7], corners[3], corners[1]});
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,AlignedRange)
    {
    for (double sweepDegrees: {360, 10, 90, 180, 275 })
        {
        SaveAndRestoreCheckTransform shifter (0, 10, 0);
        for (double startDegrees: {360, 10, 90, 180, 275, })
            {
            SaveAndRestoreCheckTransform shifter (10, 0, 0);
            DEllipse3d arc0 = DEllipse3d::From (
                3,4,5,
                4,1,0,
                -1,4,1,
                Angle::DegreesToRadians (startDegrees), Angle::DegreesToRadians (sweepDegrees)
                );
           DRange3d localRange;
           Transform localToWorld, worldToLocal;
           arc0.AlignedRange (localToWorld, worldToLocal, localRange);
           Check::SaveTransformed (arc0);
           ShowLocalRange (localToWorld, localRange);
           }
        }
    Check::ClearGeometry ("DEllipse3d.AlignedRange");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,FilletInBoundedCorner)
    {
    DPoint3d pointA = DPoint3d::From (0,0,0);
    DPoint3d pointC = DPoint3d::From (2,2,0);
    double r = 1.0;
    for (double fraction : {0.3, 0.5, 0.8})
        {
        SaveAndRestoreCheckTransform shifter (15, 0, 0);
        for (double perp : {0.1, 0.3, 0.8})
            {
            SaveAndRestoreCheckTransform shifter (10, 0, 0);
            DPoint3d pointB = DPoint3d::FromInterpolateAndPerpendicularXY (pointA, fraction, pointC, -perp);
            bvector<DPoint3d> corner {pointA, pointB, pointC};
            Check::SaveTransformed (corner);
            auto arc = DEllipse3d::FromFilletInBoundedCorner (pointA, pointB, pointC);
            if (Check::True (arc.IsValid ()))
                Check::SaveTransformed (arc);
            Check::Shift (0,10,0);
            Check::SaveTransformed (corner);
            auto arc1 = DEllipse3d::FromFilletInCorner (pointA, pointB, pointC, r);
            if (Check::True (arc1.IsValid ()))
                Check::SaveTransformed (arc1);
            }
        }
    Check::ClearGeometry ("DEllipse3d.FilletInBoundedCorner");
    }

void AnnotateArc (DEllipse3dCR arc, double fA = 1.05, double fB = 1.4, double yShift = -5.0,
uint32_t select = 0xFF
)
    {
    Check::SaveTransformed (arc);
    // primary vector linework

    DPoint3d key00 = arc.center + arc.vector0;
    DPoint3d key90 = arc.center + arc.vector90;
    DPoint3d key180 = arc.center - arc.vector0;
    DPoint3d key270 = arc.center - arc.vector90;

    if (select & 0x01)
        {
        // Add primary vectors
        Check::SaveTransformed (DSegment3d::From (arc.center, key00));
        Check::SaveTransformed (DSegment3d::From (arc.center, key90));
        }

    if (select & 0x02)
        {
        // Add negative vectors
        Check::SaveTransformed (DSegment3d::From (arc.center, key180));
        Check::SaveTransformed (DSegment3d::From (arc.center, key270));
        }

    if (select & 0x04)
        {
    // add polygon
    // enclosing parallelogram linework
        bvector<DPoint3d> parallelogram {
            key00 + arc.vector90,
            key90 - arc.vector0,
            key180 - arc.vector90,
            key270 + arc.vector0,
            key00 + arc.vector90};
        Check::SaveTransformed (parallelogram);
        }


    if (select & 0x08)
        {
        // add outer tics for start end
        // start and endpoint linework ouside of ellipse
        auto pointP = arc.FractionToPoint (0.0);
        auto pointQ = arc.FractionToPoint (1.0);
        Check::SaveTransformed (DSegment3d::From (
            DPoint3d::FromInterpolate (arc.center, fA, pointP),
            DPoint3d::FromInterpolate (arc.center, fB, pointP)));
        Check::SaveTransformed (DSegment3d::From (
            DPoint3d::FromInterpolate (arc.center, fA, pointQ),
            DPoint3d::FromInterpolate (arc.center, fB, pointQ)));
        }
    Check::Shift (0, yShift, 0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,EllipseExamples)
    {
    for (auto sweep : {Angle::DegreesToRadians (360), Angle::DegreesToRadians (135)})
        {
        SaveAndRestoreCheckTransform shifter (10, 0, 0);
        // a circular arc
        auto arcA = DEllipse3d::From (0,0,0, 1,0,0, 0,1,0, 0.0, sweep);
        AnnotateArc (arcA);
        // 2:1 major:minor
        auto arcB = DEllipse3d::From (0,0,0, 2,0,0, 0,1,0, 0.0, sweep);
        AnnotateArc (arcB);
        // Same xyz points, but with parameter space shifts applied to defining vectors.
        for (auto degrees : { 10, 30, 45})
            {
            auto arcC = DEllipse3d::FromRotatedAxes (arcB, Angle::DegreesToRadians (degrees));
            AnnotateArc (arcC);
            }
        }
    Check::ClearGeometry ("DEllipse3d.EllipseExamples");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  05/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DEllipse3d,CutConeExamples)
    {
    bvector<double> allSweep {Angle::DegreesToRadians (360), Angle::DegreesToRadians (360), Angle::DegreesToRadians (90)};
    bvector<double> allDf {0.25, 0.125, 0.25};
    for (size_t k = 0; k < allSweep.size (); k++)
        {
        double sweep = allSweep[k];
        double df = allDf[k];
        SaveAndRestoreCheckTransform shifter (10, 0, 0);
        auto baseArc = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0,0,0), 2.0);
        baseArc.SetSweep (0, sweep);
        bvector<DEllipse3d> arcs;
        arcs.push_back (baseArc);
        for (size_t i = 0; i < 5; i ++)
            {
            auto arc = baseArc;
            double z = 6.0 * (i + 1);
            arc.center.z += z;
            arc.vector0.z = 2.0 * cos ((double) i * i);
            arc.vector90.z += 2.5 * cos (2.0 + (double) i * i);
            arcs.push_back (arc);
            }

        for (size_t i = 0; i < arcs.size (); i++)
            {
            AnnotateArc (arcs[i], 1, 2, 0.0, 0x01);
            if (i & 0x01)
                {
                for (double f = 0; f <= 1.0 + 0.99999 * df; f += df)
                    {
                    DPoint3d pointA = arcs[i-1].FractionToPoint (f);
                    DPoint3d pointB = arcs[i].FractionToPoint (f);
                    Check::SaveTransformed (DSegment3d::From (pointA, pointB));
                    }
                }
            }
        }
    Check::ClearGeometry ("DEllipse3d.CutConeExamples");
    }
//
// Flatten the z of an ellipse.
// Return the flattened ellipse, with axes rotated to major, minor structure (longest first)
// Returned ellipse always has the flattened vectors, but is marked invalid if it has degnerated into a line.
// @param singleStroke if true, the strokes array can contain only 2 points which are the extrema of the line.
//           if false, the strokes start and end with the arc start end, and may contain additional points where
//           the projection reached an extrema other than at its endpoints
// @param arc [in] arc to flatten
// @param strokes [out] if flattened the ellipse is a line, these are the viewed strokes on z=0
// @return the flattened ellipse.
//
ValidatedDEllipse3d FlattenDEllipse3dZ (DEllipse3d arc, bvector<DPoint3d> &strokes, bool singleStroke)
    {
    auto arc0 = arc;
    arc0.center.z = arc0.vector0.z = arc0.vector90.z = 0.0;
    auto arc1 = DEllipse3d::FromMajorMinor (arc0);
    strokes.clear ();
    static double s_flattenFactor = 1.0e-8;
    if (arc1.vector90.Magnitude () > s_flattenFactor * arc1.vector0.Magnitude ())
        return ValidatedDEllipse3d (arc1, true);
    // arc1 strokes from center+vector0 to center-vector0 and back.
    // Get all the relevant points ....
    double pi = Angle::Pi ();
    bvector<double>fractions;   // fractional positions on the ray <center + alpha * vector0>
    DPoint3d point0 = arc1.FractionToPoint (0.0);
    DPoint3d point1 = arc1.FractionToPoint (1.0);
    DPoint3d pointA = arc1.center + arc1.vector0;
    DPoint3d pointB = arc1.center - arc1.vector0;
    strokes.push_back (point0);
    fractions.push_back(0.0);
    double factor = 1.0 / arc1.vector0.Magnitude ();
    fractions.push_back (factor * arc1.vector0.DotProduct (point0 - arc1.center));
    if (arc1.IsAngleInSweep (0.0))
        {
        strokes.push_back (pointA);
        fractions.push_back (1.0);
        }
    if (arc1.IsAngleInSweep (pi))
        {
        strokes.push_back (pointB);
        fractions.push_back (-1.0);
        }
    strokes.push_back (point1);
    fractions.push_back (factor * arc1.vector0.DotProduct (point1 - arc1.center));
    if (strokes.size () != 2 && singleStroke)
        {
        // at least one of the major and minor axis points is within the arc.
        // sort the fractions.
        std::sort (fractions.begin (), fractions.end ());
        if (singleStroke)
            {
            strokes.clear ();
            strokes.push_back (arc1.center + fractions.front () * arc1.vector0);
            strokes.push_back (arc1.center + fractions.back () * arc1.vector0);
            }
        }
    return ValidatedDEllipse3d (arc1, false);
    }
TEST(DEllipse3d,FlattenZ)
    {
    double z0 = 0.3;
    double xShift = 0.1;
    DVec3d ticShift = DVec3d::From (0.5 * xShift, 0,0);
    for (double y0 :{0.0, 0.2}) // for y0=0, the 3d starter arc is circular.
        {
        SaveAndRestoreCheckTransform shifter (0,3,0);
        for (double x0 : {1.0, 0.5, 0.0, -0.5, -1.0})
            {
            SaveAndRestoreCheckTransform shifter (3,0,0);
            // make a tilted true circle
            auto arc = DEllipse3d::From (0,0,1,
                            x0, y0, z0, 
                            0, sqrt (x0 * x0 + z0 * z0), 0.0,
                            0.4, 6.0);
            Check::SaveTransformed (arc);
            bvector<DPoint3d> strokes;
            auto arc0 = FlattenDEllipse3dZ (arc, strokes, true);
            if (arc0.IsValid ())
                Check::SaveTransformed (arc0);
            else
                {
                // Draw the strokes of the (true, false) projection flavors, shifted to left and right, with extra line shooting up to show true start of strokes.
                Check::Shift (-xShift, 0, 0);
                Check::SaveTransformed (DSegment3d::From (strokes.front (), strokes.front () - ticShift));
                Check::SaveTransformed (DSegment3d::From (strokes.back (), strokes.back () + ticShift));
                Check::SaveTransformed (strokes);
        
                Check::Shift (2 * xShift, 0, 0);
                /* unused - auto arcB = */FlattenDEllipse3dZ (arc, strokes, false);
                Check::SaveTransformed (DSegment3d::From (strokes.front (), strokes.front () - ticShift));
                Check::SaveTransformed (DSegment3d::From (strokes.back (), strokes.back () + ticShift));
                Check::SaveTransformed (strokes);
                }
            }
        }
    Check::ClearGeometry ("DEllipse3d.FlattenZ");
    }
// Return (X, a * dX/ds) for s theta = theta0 + s * dTheta
static DRay3d ScaledTangentAtCirclePoint (
double r,       // circle radius
double theta0, // angular position on circle (radians)
double dTheta, // angular step for approximation interval. (radians)
double a        // additional scale factor for derivative -- e.g. to scale for bezier edge
)
    {
    double b = a * r * dTheta;
    return DRay3d::FromOriginAndVector (
        DPoint3d::From (r * cos(theta0), r * sin(theta0), 0),
        DVec3d::From (- b * sin(theta0), b * cos(theta0), 0));
    }
static bvector<DPoint3d> QuadraticBezierCirclePoints (double r, double theta0, double dTheta)
    {
    auto ray0 = ScaledTangentAtCirclePoint (r, theta0, dTheta, 0.5);
    auto ray1 = ScaledTangentAtCirclePoint (r, theta0 + dTheta, -dTheta, 0.5);
    // mid point of bezier is intersecton of  the rays ...
    double fractionA, fractionB;
    DPoint3d pointA, pointB;
    DRay3d::ClosestApproachUnboundedRayUnboundedRay (fractionA, fractionB, pointA, pointB, ray0, ray1);
    return bvector<DPoint3d> {ray0.origin, pointA, ray1.origin};
    }
TEST(DEllipse3d,BezierApproximation)
    {
    // quadratic bezier . . .
    double r = 1.0;
    for (double dTheta : {0.4, 0.2, 0.1})
        {
        SaveAndRestoreCheckTransform shifter (2,0,0);
        double dTheta1 = 0.5 * dTheta;
        auto bezier02 = QuadraticBezierCirclePoints (r, 0.0, dTheta);
        auto bezier01 = QuadraticBezierCirclePoints (r, 0.0, dTheta1);
        auto bezier12 = QuadraticBezierCirclePoints (r, dTheta1, dTheta1);
        auto arc = DEllipse3d::From (0,0,0, r,0,0, 0,r,0, 0.0, dTheta);
        Check::SaveTransformed (bezier02);
        Check::SaveTransformed (bezier01);
        Check::SaveTransformed (bezier12);
        Check::SaveTransformed (arc);
        double d02 = PolylineOps::Length (bezier02);
        double d012 = PolylineOps::Length (bezier01) + PolylineOps::Length (bezier12);
        printf (" (dTheta %.8lg) (bezierHalvingLengthRatio %.8lg) (tangent ratio %.6g)\n", dTheta,
                    d012/ d02,
                    2.0 * tan (dTheta1/2.0) / tan (dTheta/2)
                    );
        }
    Check::ClearGeometry ("DEllipse3d.BezierApproximation");
    }