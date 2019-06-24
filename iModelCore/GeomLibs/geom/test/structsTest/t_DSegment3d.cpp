/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "testHarness.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, Init)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::FromXYZ (1.0, 1.0, 1.0);
    DPoint3d point2 = DPoint3d::FromXYZ (2.0, 3.0, 4.0);
    DPoint3d point3 = DPoint3d::FromXYZ (5.0, 6.0, 7.0);
    DPoint3d point4 = DPoint3d::FromXYZ (7.5, 8.0, 6.3);
    DPoint3d point5 = DPoint3d::FromXYZ (3.5, 1.8, 4.7);
    DPoint2d point6 = DPoint2d::From (3.5, 1.8);
    DPoint2d point7 = DPoint2d::From (9.4, 3.6);
    DPoint3d point8 = DPoint3d::FromXYZ (3.5, 1.8, 0.0);
    DPoint3d point9 = DPoint3d::FromXYZ (9.4, 3.6, 0.0);
    DPoint3d point10 = DPoint3d::FromXYZ (1.4, 5.6, 10.0);
    DPoint3d point11 = DPoint3d::FromXYZ (5.7, 2.4, 7.8);
    DPoint3d point12 = DPoint3d::FromXYZ (7.1, 8.0, 17.8);
    DRay3d ray0 = DRay3d::FromOriginAndTarget (point2, point3);
    DRay3d ray1 = DRay3d::FromOriginAndTarget (point4, point5);
    DSegment3d segment0 = DSegment3d::From (point0, point1);
    DSegment3d segment1 = DSegment3d::From (point2, point3);
    DSegment3d segment2 = DSegment3d::From (point4, point5);
    DSegment3d segment3 = DSegment3d::From (point1, point5);
    DSegment3d segment4 = DSegment3d::From (point8, point9);
    DSegment3d segment5 = DSegment3d::From (point3, point5);
    DSegment3d segment6 = DSegment3d::From (point2, point8);
    DSegment3d segment7 = DSegment3d::From (point0, point11);
    DSegment3d segment8 = DSegment3d::From (point2, point3);
    DSegment3d segment9 = DSegment3d::From (point1, point3);
    DSegment3d segment10 = DSegment3d::From (point4, point5);
    segment1.Init (0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
    segment3.Init (point4, point5);
    segment5.Init (point6, point7);
    segment6.InitFromOriginAndDirection (point10, point11);
    segment7.InitFrom (ray0);
    segment9.InitFrom (ray1);
    Check::Near (segment0, segment1);
    Check::Near (segment2, segment3);
    Check::Near (segment4, segment5);
    Check::Near (point10, segment6.point[0]);
    Check::Near (point12, segment6.point[1]);
    Check::Near (point2, segment7.point[0]);
    Check::Near (point3, segment7.point[1]);
    Check::Near (segment8, segment7);
    Check::Near (segment9, segment10);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, GetEndPoints)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::FromXYZ (1.0, 1.0, 1.0);
    DPoint3d point2 = DPoint3d::FromXYZ (2.0, 3.0, 4.0);
    DPoint3d point3 = DPoint3d::FromXYZ (5.0, 6.0, 7.0);
    DSegment3d segment0 = DSegment3d::From (point0, point1);
    segment0.GetEndPoints (point2, point3);
    Check::Near (point0, point2);
    Check::Near (point1, point3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, LengthSquared)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::FromXYZ (1.0, 1.0, 1.0);
    DPoint3d point2 = DPoint3d::FromXYZ (2.0, 3.0, 4.0);
    DPoint3d point3 = DPoint3d::FromXYZ (-2.3, -3.0, -7.4);
    DSegment3d segment0 = DSegment3d::From (point0, point0);
    DSegment3d segment1 = DSegment3d::From (point0, point1);
    DSegment3d segment2 = DSegment3d::From (point1, point2);
    DSegment3d segment3 = DSegment3d::From (point2, point3);
    double lengthSquaredSeg0 = segment0.LengthSquared ();
    double lengthSquaredSeg1 = segment1.LengthSquared ();
    double lengthSquaredSeg2 = segment2.LengthSquared ();
    double lengthSquaredSeg3 = segment3.LengthSquared ();
    Check::Near (0.0, lengthSquaredSeg0);
    Check::Near (3.0, lengthSquaredSeg1);
    Check::Near (14.0, lengthSquaredSeg2);
    Check::Near (184.45, lengthSquaredSeg3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, GetRange)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::FromXYZ (1.0, 5.0, 9.0);
    DPoint3d point2 = DPoint3d::FromXYZ (2.0, 3.0, 4.0);
    DRange3d range0 = DRange3d::From (point0, point1, point2);
    DSegment3d segment0 = DSegment3d::From (point1, point2);
    Check::True (segment0.GetRange (range0));
    Check::Near (1.0, range0.low.x);
    Check::Near (3.0, range0.low.y);
    Check::Near (4.0, range0.low.z);
    Check::Near (2.0, range0.high.x);
    Check::Near (5.0, range0.high.y);
    Check::Near (9.0, range0.high.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, ProjectPoint)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (4.7, 2.4, 9.3);
    DPoint3d point1 = DPoint3d::FromXYZ (8.1, 5.6, 7.2);
    DPoint3d point2 = DPoint3d::FromXYZ (5.1, 8.7, 4.3);
    DPoint3d point3 = DPoint3d::FromXYZ (6.5, 9.6, 3.7);
    DPoint3d point4 = DPoint3d::FromXYZ (1.0, 2.0, 3.0);
    DPoint3d point5 = DPoint3d::FromXYZ (1.0, 2.0, 3.0);
    DPoint3d point6 = DPoint3d::FromXYZ (3.0, 5.0, 7.0);
    DPoint3d point7 = DPoint3d::FromXYZ (6.0, 15.0, 8.0);
    DPoint3d point8 = DPoint3d::FromXYZ (8.0, 15.0, 5.0);
    DPoint3d point9 = DPoint3d::FromXYZ (7.0, 14.0, 3.0);
    DPoint3d point10 = DPoint3d::FromXYZ (4.0, 3.0, 98.0);
    DPoint3d point11 = DPoint3d::FromXYZ (6.0, 18.0, 32.0);
    double closestParam0 = 1.0;
    double closestParam1 = 2.0;
    double closestParam2 = 3.0;
    DSegment3d segment0 = DSegment3d::From (point0, point1);
    DSegment3d segment1 = DSegment3d::From (point4, point5);
    DSegment3d segment2 = DSegment3d::From (point8, point9);
    Check::True (segment0.ProjectPoint (point2, closestParam0, point3));
    Check::True (!(segment1.ProjectPoint (point6, closestParam1, point7)));
    Check::True (segment2.ProjectPointXY (point10, closestParam2, point11));
    Check::Near ((40.92/26.21), closestParam0);
    Check::Near ((4.7 + (40.92/26.21) * 3.4), point2.x);
    Check::Near ((2.4 + (40.92/26.21) * 3.2), point2.y);
    Check::Near ((9.3 + (40.92/26.21) * -2.1), point2.z);
    Check::Near ((-1.0/2.0), closestParam2);
    Check::Near ((8.0 + (-1.0/2.0) * -1.0), point10.x);
    Check::Near ((15.0 + (-1.0/2.0) * -1.0), point10.y);
    Check::Near ((5.0 + (-1.0/2.0) * -2.0), point10.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, ProjectPointBounded)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (4.7, 2.4, 9.3);
    DPoint3d point1 = DPoint3d::FromXYZ (8.1, 5.6, 7.2);
    DPoint3d point2 = DPoint3d::FromXYZ (5.1, 8.7, 4.3);
    DPoint3d point3 = DPoint3d::FromXYZ (6.5, 9.6, 3.7);
    DPoint3d point4 = DPoint3d::FromXYZ (8.0, 15.0, 0.0);
    DPoint3d point5 = DPoint3d::FromXYZ (7.0, 14.0, 0.0);
    DPoint3d point6 = DPoint3d::FromXYZ (4.0, 3.0, 98.0);
    DPoint3d point7 = DPoint3d::FromXYZ (6.0, 18.0, 0.0);
    DPoint3d point8 = DPoint3d::FromXYZ (7.0, 6.0, 1.0);
    DPoint3d point9 = DPoint3d::FromXYZ (8.0, 7.0, 1.0);
    DPoint3d point10 = DPoint3d::FromXYZ (4.0, 3.0, 98.0);
    DPoint3d point11 = DPoint3d::FromXYZ (5.0, 9.0, 1.0);
    double closestParam0 = 0.0;
    double closestParam1 = 0.0;
    double closestParam2 = 0.0;
    DSegment3d segment0 = DSegment3d::From (point0, point1);
    DSegment3d segment1 = DSegment3d::From (point4, point5);
    DSegment3d segment2 = DSegment3d::From (point8, point9);
    Check::True (segment0.ProjectPointBounded (point2, closestParam0, point3));
    Check::True (segment1.ProjectPointBounded (point6, closestParam1, point7));
    Check::True (segment2.ProjectPointBounded (point10, closestParam2, point11));
    Check::Near (1.0, closestParam0);
    Check::Near (point2, point1);
    Check::Near (0.0, closestParam1);
    Check::Near (point4, point6);
    Check::Near (0.5, closestParam2);
    Check::Near (7.5, point10.x);
    Check::Near (6.5, point10.y);
    Check::Near (1.0, point10.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, Intersect)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (4.7, 2.4, 9.3);
    DPoint3d point1 = DPoint3d::FromXYZ (8.1, 5.6, 7.2);
    DPoint3d point2 = DPoint3d::FromXYZ (5.1, 8.7, 4.3);
    DPoint3d point3 = DPoint3d::FromXYZ (-6.5, -9.6, -3.7);
    DPoint3d point4 = DPoint3d::FromXYZ (6.5, 9.6, 3.7);
    DPoint3d point5 = DPoint3d::FromXYZ (6.5, 9.6, 3.7);
    DPoint3d point6;
    DPoint3d point7;
    DPoint3d point8 = DPoint3d::FromXYZ (8.5, 4.6, 0.0);
    DPoint3d point9 = DPoint3d::FromXYZ (3.7, 2.4, 0.0);
    DPoint3d point10;
    DPoint3d point11;
    double initParam0 = 0.0;
    double initParam1 = 0.0;
    double initParam2 = 0.0;
    DPlane3d plane0 = DPlane3d::FromOriginAndNormal (0.0, 0.0, 1.0, 0.0, 0.0, 1.0);
    DSegment3d segment0 = DSegment3d::From (point0, point1);
    DSegment3d segment1 = DSegment3d::From (point3, point4);
    DSegment3d segment2 = DSegment3d::From (point8, point9);
    Check::True (segment0.Intersect (point2, initParam0, plane0));
    Check::True (segment1.Intersect (point5, initParam1, plane0));
    Check::True (!(segment2.Intersect (point10, initParam2, plane0)));
    segment0.FractionParameterToPoint (point7, initParam0);
    segment1.FractionParameterToPoint (point6, initParam1);
    segment2.FractionParameterToPoint (point11, initParam2);
    Check::Near (0.0, plane0.Evaluate (point7));
    Check::Near (0.0, plane0.Evaluate (point6));
    Check::True (!(0.0 == plane0.Evaluate (point11)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, IntersectCircleXY)
    {
    DPoint3d point0[2];
    DPoint3d center0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point2 = DPoint3d::FromXYZ (2.0, 0.0, 0.0);
    DPoint3d point3 = DPoint3d::FromXYZ (1.0, 0.0, 0.0);
    DPoint3d point4 = DPoint3d::FromXYZ (-1.0, 0.0, 0.0);
    DPoint3d point5 = DPoint3d::FromXYZ (-1.0, 1.0, 0.0);
    DPoint3d point6 = DPoint3d::FromXYZ (0.0, 1.0, 0.0);
    DPoint3d point7[2];
    DPoint3d point8 = DPoint3d::FromXYZ (-2.0, 2.0, 0.0);
    DPoint3d point9 = DPoint3d::FromXYZ (0.0, 2.0, 0.0);
    DPoint3d point10[2];
    double intParam0[2];
    double intParam1[2];
    double intParam2[2];
    double radius0 = 1.0;
    DSegment3d segment0 = DSegment3d::From (point1, point2);
    DSegment3d segment1 = DSegment3d::From (point5, point6);
    DSegment3d segment2 = DSegment3d::From (point8, point9);
    int counter0 = segment0.IntersectCircleXY (point0, intParam0, center0, radius0);
    int counter1 = segment1.IntersectCircleXY (point7, intParam1, center0, radius0);
    int counter2 = segment2.IntersectCircleXY (point10, intParam2, center0, radius0);
    Check::Near (2, counter0);
    Check::Near (-0.5, intParam0[0]);
    Check::Near ( 0.5, intParam0[1]);
    Check::Near (point4, point0[0]);
    Check::Near (point3, point0[1]);
    Check::Near (1, counter1);
    Check::Near (1.0, intParam1[0]);
    Check::Near (point6, point7[0]);
    Check::Near (0, counter2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, InitProduct)
    {
    DPoint3d point1 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point2 = DPoint3d::FromXYZ (2.0, 1.0, 3.0);
    //DPoint3d point3 = DPoint3d::FromXYZ (1.0, 4.0, 9.0);
    //DPoint3d point4 = DPoint3d::FromXYZ (3.0, 5.0, 7.0);
    DPoint3d point5;
    DPoint3d point6;
    DPoint3d point7;
    double frac0 = 0.7;
    Transform transform0 = Transform::FromRowValues (0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0);
    DSegment3d segment0 = DSegment3d::From (point1, point2);
    DSegment3d segment1;
    segment0.FractionParameterToPoint (point5, frac0);
    transform0.Multiply (segment1, segment0);
    segment1.FractionParameterToPoint (point6, frac0);
    transform0.Multiply (point7, point5);
    Check::Near (point7, point6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, GetStartPoint)
    {
    DSegment3d segment0 = DSegment3d::From (8.9, -3.4, 5.7, -1.2, -2.3, 9.3);
    DSegment3d segment1 = DSegment3d::From (3.9, -2.6, 8.4, 3.2, -5.8, 2.7);
    DPoint3d point0;
    DPoint3d point1;
    DPoint3d point2 = DPoint3d::FromXYZ (8.9, -3.4, 5.7);
    DPoint3d point3 = DPoint3d::FromXYZ (-1.2, -2.3, 9.3);
    segment0.GetStartPoint (point0);
    segment0.GetEndPoint (point1);
    segment1.SetStartPoint (point2);
    segment1.SetEndPoint (point3);
    Check::Near (point2, point0);
    Check::Near (point3, point1);
    Check::Near (point2, segment1.point[0]);
    Check::Near (point3, segment1.point[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, FractionParameterToPoint)
    {
    DSegment3d segment0 = DSegment3d::From (0.0, 0.0, 0.0, 0.0, 4.0, 0.0);
    DPoint3d point0 = DPoint3d::FromXYZ (0.0, 2.0, 0.0);
    DPoint3d point1;
    DPoint3d point2 = DPoint3d::FromXYZ (0.0, 1.0, 0.0);
    DPoint3d point3;
    DPoint3d point4;
    DVec3d vector0;
    DVec3d vector1 = DVec3d::From (0.0, 4.0, 0.0);
    double param0 = 0.5;
    double param1 = 0.25;
    Check::True (segment0.FractionParameterToPoint (point1, param0));
    Check::True (segment0.FractionParameterToPoint (point3, param1));
    Check::True (segment0.FractionParameterToTangent (point4, vector0, param0));
    Check::Near (point0, point1);
    Check::Near (point3, point2);
    Check::Near (point0, point4);
    Check::Near (vector1, vector0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, GetPoint)
    {
    DSegment3d segment0 = DSegment3d::From (1.5, 6.2, 3.7, 6.9, 4.0, 10.0);
    DSegment3d segment1 = DSegment3d::From (-3.5, 5.7, -3.9, -2.6, -2.0, 4.1);
    DPoint3d point0;
    DPoint3d point1;
    DPoint3d point2 = DPoint3d::FromXYZ (1.5, 6.2, 3.7);
    DPoint3d point3 = DPoint3d::FromXYZ (6.9, 4.0, 10.0);
    DPoint3d point4 = DPoint3d::FromXYZ (7.4, -2.8, 4.9);
    DPoint3d point5 = DPoint3d::FromXYZ (3.5, 7.0, -8.1);
    int index0 = 0;
    int index1 = 1;
    int index2 = 2;
    Check::True (segment0.GetPoint (point0, index0));
    Check::True (segment0.GetPoint (point1, index1));
    Check::True (!(segment0.GetPoint (point1, index2)));
    Check::True (segment1.SetPoint (point4, index0));
    Check::True (segment1.SetPoint (point5, index1));
    Check::True (!(segment1.SetPoint (point4, index2)));
    Check::Near (point2, point0);
    Check::Near (point3, point1);
    Check::Near (point4, segment1.point[0]);
    Check::Near (point5, segment1.point[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, InterpolateUniformArray)
    {
    DSegment3d segment0 = DSegment3d::From (0.0, 0.0, 0.0, 0.0, 4.0, 0.0);
    bvector <DPoint3d> points1;
    bvector <DPoint3d> points2;
    bvector <DPoint3d> points3;
    DPoint3d point0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::FromXYZ (0.0, 1.0, 0.0);
    DPoint3d point2 = DPoint3d::FromXYZ (0.0, 2.0, 0.0);
    DPoint3d point3 = DPoint3d::FromXYZ (0.0, 3.0, 0.0);
    DPoint3d point4 = DPoint3d::FromXYZ (0.0, 4.0, 0.0);
    size_t numPoints1 = 5;
    size_t numPoints2 = 3;
    size_t numPoints3 = 1;
    segment0.InterpolateUniformArray (points1, numPoints1);
    segment0.InterpolateUniformArray (points2, numPoints2);
    segment0.InterpolateUniformArray (points3, numPoints3);
    Check::Near (point0, points1[0]);
    Check::Near (point1, points1[1]);
    Check::Near (point2, points1[2]);
    Check::Near (point3, points1[3]);
    Check::Near (point4, points1[4]);
    Check::Near (point0, points2[0]);
    Check::Near (point2, points2[1]);
    Check::Near (point4, points2[2]);
    Check::Near (point2, points3[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, Length)
    {
    DSegment3d segment0 = DSegment3d::From (0.0, 0.0, 0.0, 0.0, 4.0, 0.0);
    DSegment3d segment1 = DSegment3d::From (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    DSegment3d segment2 = DSegment3d::From (0.0, 0.0, 0.0, 0.0, -4.0, 0.0);
    double length0 = segment0.Length ();
    double length1 = segment1.Length ();
    double length2 = segment2.Length ();
    Check::Near (4.0, length0);
    Check::Near (0.0, length1);
    Check::Near (4.0, length2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, FractionToLength)
    {
    DSegment3d segment0 = DSegment3d::From (0.0, 0.0, 0.0, 0.0, 4.0, 0.0);
    DSegment3d segment1 = DSegment3d::From (0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    double arcLength0 = 0.0;
    double arcLength1 = 0.0;
    double arcLength2 = 0.0;
    double arcLength3 = 0.0;
    double arcStep0 = 4.0;
    double arcStep1 = -4.0;
    double arcStep2 = 1.0;
    double fraction0 = 0.0;
    double fraction1 = 0.25;
    double fraction2 = 0.75;
    double fraction3 = 1.25;
    double fraction4 = 0.0;
    double fraction5 = 0.0;
    double fraction6 = 0.0;
    double fraction7 = 0.0;
    Check::True (segment0.FractionToLength (arcLength0, fraction0, fraction1), "fraction to length");
    Check::True (segment0.FractionToLength (arcLength1, fraction1, fraction2), "fraction to length");
    Check::True (segment0.FractionToLength (arcLength2, fraction2, fraction1), "fraction to length");
    Check::True (segment0.FractionToLength (arcLength3, fraction1, fraction3), "fraction to length");
    Check::True (segment0.LengthToFraction (fraction4, fraction0, arcStep0), "fraction to length");
    Check::True (segment0.LengthToFraction (fraction5, fraction0, arcStep1), "fraction to length");
    Check::True (segment0.LengthToFraction (fraction6, fraction1, arcStep2), "fraction to length");
    Check::True (!(segment1.LengthToFraction (fraction7, fraction0, arcStep1)), "fraction to length");
    Check::Near (1.0, arcLength0);
    Check::Near (2.0, arcLength1);
    Check::Near (-2.0, arcLength2);
    Check::Near (4.0, arcLength3);
    Check::Near (1.0, fraction4);
    Check::Near (-1.0, fraction5);
    Check::Near (0.5, fraction6);
    }

void TrimSegments (bvector <DSegment3d> &segments, double tolerance)
    {
    for (size_t i = 0; i < segments.size () - 1; i++)
        {
        size_t j = i + 1;
        DPoint3d pointi, pointj;
        double parami, paramj;
        DPoint3d starti, endi, startj, endj;
        segments[i].GetStartPoint (starti);
        segments[j].GetStartPoint (startj);
        segments[i].GetEndPoint (endi);
        segments[j].GetEndPoint (endj);
        segments[i].ProjectPoint (pointi, parami, startj);
        segments[j].ProjectPoint (pointj, paramj, endi);
        if (pointi.Distance (startj) < tolerance && pointj.Distance(endi) < tolerance)
 	    { /// This is the collinear case
   	    if (0.0 < parami && parami <= 1.0    && 0.0 < paramj && paramj <= 1.0)
  		{
  		segments[j].SetStartPoint (endi);
  		}
  	    }
        else
            {
            bool result = DSegment3d::IntersectXY (parami, paramj, pointi, pointj, segments[i], segments[j]);
            //bsiGeom_intersectXYDSegment3dDSegment3d (&pointi, &parami, &pointj, &paramj, &segments[i], &segments[j]);
            if (result)
                {
                if (0.0 <= parami && parami <= 1.0    && 0.0 <= paramj && paramj <= 1.0)
                    {
                    DPoint3d pointk, pointl;
                    DVec3d tangentk, tangentl;
                    segments[i].FractionParameterToTangent (pointk, tangentk, 0.0);
                    segments[j].FractionParameterToTangent (pointl, tangentl, 0.0);
                    if (tangentk.DotProduct (tangentl) > 0)
                        {
                        segments[i].SetEndPoint (pointi);
                        segments[j].SetStartPoint (pointj);
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DSegment3d, TrimSegments)
    {
    DPoint3d point0 = DPoint3d::FromXYZ (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::FromXYZ (0.0, 4.0, 0.0);
    DPoint3d point2 = DPoint3d::FromXYZ (-1.0, 2.0, 0.0);
    DPoint3d point3 = DPoint3d::FromXYZ (3.0, 6.0, 0.0);
    DPoint3d point4 = DPoint3d::FromXYZ (0.0, 6.0, 0.0);
    DPoint3d point5 = DPoint3d::FromXYZ (4.0, 4.0, 0.0);
    DPoint3d point6 = DPoint3d::FromXYZ (3.0, 4.5, 0.0);
    DPoint3d point7 = DPoint3d::FromXYZ (6.0, 3.0, 0.0);
    DPoint3d point8 = DPoint3d::FromXYZ (0.0, 3.0, 0.0);
    DPoint3d point9 = DPoint3d::FromXYZ (2.0, 5.0, 0.0);
    DSegment3d segment0 = DSegment3d::From (point0, point1);
    DSegment3d segment1 = DSegment3d::From (point2, point3);
    DSegment3d segment2 = DSegment3d::From (point4, point5);
    DSegment3d segment3 = DSegment3d::From (point6, point7);
    bvector <DSegment3d> segments;
    segments.push_back (segment0);
    segments.push_back (segment1);
    segments.push_back (segment2);
    segments.push_back (segment3);
    double tolerance = 1.0E-10;
    TrimSegments (segments, tolerance);
    Check::Near (point8, segments[0].point[1]);
    Check::Near (point8, segments[1].point[0]);
    Check::Near (point9, segments[1].point[1]);
    Check::Near (point9, segments[2].point[0]);
    Check::Near (point5, segments[3].point[0]);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRange3d,intersectsWith)
    {
    DRange3d A;
    A.Init ();
    A.Extend (0,0,0);
    A.Extend (1,1,0);
    Check::True (A.IntersectsWith (A));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DRay3d,IntersectZPlane)
    {
    Transform frame = Transform::FromRowValues (
                    5,0.2, 0.4,     1,
                    -0.2,7, 0.45,   2,
                    0.01, 0.8, -11, 3);
    DRay3d ray = DRay3d::FromOriginAndVector (DPoint3d::From (3,0.3,-0.4), DVec3d::From (0.2, 0.8, 5));
    DPoint3d uvw;
    double t;
    double z = 0.42;
    Check::True(ray.IntersectZPlane (frame, z, uvw, t), "Ray intersect z plane");
    DPoint3d xyzFrame;
    frame.Multiply (xyzFrame, uvw);
    Check::Near (xyzFrame, ray.FractionParameterToPoint (t), "Ray intersect z plane -- world point");
    Check::Near (z, uvw.z, "Ray intersect z plane -- z part");
    }

//! @param [out] transform transform that picks up one segment and puts it down (optionally stretched) on a second segment.
//! @param [in] pickup first segment.
//! @param [in] putdown second segment.
//! @param [in] includeScale true to stretch the segment in addtion to move and rotate.  false to just move and rotate.
bool bsiTransform_initPickupPutdownXY (TransformR transform, DSegment3dCR pickup, DSegment3dCR putdown, bool includeScale)
    {
    DVec3d vectorX0 = DVec3d::FromStartEnd (pickup.point[0],  pickup.point[1]);
    DVec3d vectorX1 = DVec3d::FromStartEnd (putdown.point[0], putdown.point[1]);
    vectorX0.z = vectorX1.z = 0;
    transform.InitIdentity ();
    DPoint3d origin0 = pickup.point[0];
    DPoint3d origin1 = putdown.point[0];


    if (!includeScale)
        {
        if (vectorX0.Normalize () == 0.0 || vectorX1.Normalize () == 0.0)
            return false;
        }

    // Make coordinate frames with origin at start of the respective segments and x axis along the segment.
    Transform frame0 = Transform::FromRowValues
            (
            vectorX0.x, -vectorX0.y, 0.0, origin0.x,
            vectorX0.y,  vectorX0.x, 0.0, origin0.y,
            0.0,         0.0,        1.0, origin0.z
            );
    Transform frame1 = Transform::FromRowValues
            (
            vectorX1.x, -vectorX1.y, 0.0, origin1.x,
            vectorX1.y,  vectorX1.x, 0.0, origin1.y,
            0.0,         0.0,        1.0, origin1.z
            );
    Transform worldToFrame0;
    // compose a transform that takes world coordinates into frame0 locals and then uses them as frame1 locals for putdown:
    if (worldToFrame0.InverseOf (frame0))
        {
        transform.InitProduct (frame1, worldToFrame0);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment3d, PickupPutdown)
    {
    DSegment3d segment0 = DSegment3d::From (0,0,0, 1,0,0);
    DSegment3d segment1 = DSegment3d::From (1,1,0, 1,2,0);
    Transform transform;
    if (Check::True (bsiTransform_initPickupPutdownXY (transform, segment0, segment1, true), "PickupPutdown Transform"))
        {
        DPoint3d point0, point1;
        transform.Multiply (point0, segment0.point[0]);
        transform.Multiply (point1, segment0.point[1]);
        Check::Near (segment1.point[0], point0, "Start point");
        Check::Near (segment1.point[1], point1, "End point");
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DSegment3d, StarttoEnd)
    {
    DSegment3d seg3 = DSegment3d::FromZero();
    Check::True(seg3.IsSinglePoint());

    DRay3d ray = DRay3d::FromOriginAndTarget(DPoint3d::From(2, 2, 2), DPoint3d::From(8, 8, 8));
    DSegment3d segm = DSegment3d::From(ray);
    Check::False(segm.IsSinglePoint());
    DVec3d vec3d = segm.VectorStartToEnd();
    double mag1 = vec3d.Magnitude();
    DRange3d range;
    segm.GetRange(range);
    double mag2 = range.high.Magnitude() - range.low.Magnitude();
    Check::Near(mag1, mag2);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DSegment3d, AlmostSingle)
    {
    DSegment3d seg3d = DSegment3d::From(DPoint3d::From(3, 2, 1), DPoint3d::From(8, 8, 6));
    Check::False(seg3d.IsAlmostSinglePoint());
    seg3d.SetEndPoint(DPoint3d::From(3.0000000000005, 2.00000000000005, 1.00000000000005));
    Check::True(seg3d.IsAlmostSinglePoint());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DSegment3d, Zero)
    {
    DSegment3d seg3d;
    seg3d.InitZero();
    Check::True(seg3d.IsSinglePoint());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DSegment3d, TransformedSegment)
    {
    DSegment3d originalSeg = DSegment3d::From(DPoint3d::From(7, 7, 7), DPoint3d::From(12, 12, 12));
    DSegment3d afterTransform;
    afterTransform.InitProduct(Transform::From(RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(360).Radians())), originalSeg);

    Check::Near(originalSeg, afterTransform);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Earlin.Lutz                   06/17
//---------------------------------------------------------------------------------------
TEST(DSegment3d, TangentCC)
    {
    auto centerA = DPoint3d::From (-2,0,0);
    auto centerB = DPoint3d::From ( 2,0,0);

    bvector<double> radii {-1.1, -0.5, 0.0, 0.5, 1.1};
    double a = 10.0;
    for (auto rB : radii)
        {
        SaveAndRestoreCheckTransform shifter (0,a,0);
        for (auto rA : radii)
            {
            Check::SaveTransformed (DEllipse3d::FromVectors (
                            centerA, rA * DVec3d::UnitX (), rA *DVec3d::UnitY (), 0.0, Angle::Pi ()));
            Check::SaveTransformed (DEllipse3d::FromVectors (
                            centerB, rB * DVec3d::UnitX (), rB *DVec3d::UnitY (), 0.0, Angle::Pi ()));
            SaveAndRestoreCheckTransform shifter (a,0,0);
            auto segment = DSegment3d::ConstructTangent_CircleCircleXY (centerA, rA, centerB, rB);
            if (Check::True (segment.IsValid ()))
                {
                Check::SaveTransformed (segment.Value ());
                auto pointA = segment.Value ().point[0];
                auto pointB = segment.Value ().point[1];
                DVec3d vectorAB = pointB - pointA;
                Check::Near (pointA.Distance (centerA), fabs (rA));
                Check::Near (pointB.Distance (centerB), fabs (rB));
                if (rA != 0.0)
                    {
                    Check::Perpendicular (pointA - centerA, vectorAB);
                    Check::True (pointA.y * rA > 0.0, "side selection at A");
                    }
                if (rB != 0.0)
                    {
                    Check::Perpendicular (pointB - centerB, vectorAB);
                    Check::True (pointB.y * rB > 0.0, "side selection at B");
                    }
                }
            }
        }
    Check::ClearGeometry ("DSegment3d.TangentCC");
    }