#include "testHarness.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, FromArray)
    {
    DPoint3d point0 = DPoint3d::From (1.0, 1.0, 1.0);
    double xyz[3];
    xyz[0] = xyz[1] = xyz[2] = 1.0;
    DPoint3d point1 = DPoint3d::FromArray (xyz);
    Check::Near (point0, point1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, CrossProductToPointsXY)
    {
    DPoint3d point0 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::From (1.0, 1.0, 1.0);
    DPoint3d point2 = DPoint3d::From (4.0, 5.0, 6.0);
    DPoint3d point3 = DPoint3d::From (3.0, 4.0, 8.0);
    DVec3d vec0 = DVec3d::From (2.0, 3.0, 4.0);
    double cross = point0.CrossProductToPointsXY (point1, point2);
    double dot0 = point0.DotProductToPoints (point1, point2);
    double dot1 = point0.DotProductToPointsXY (point1, point2); 
    double dot2 = point2.DotDifference (point0, vec0);
    double triple = point0.TripleProductToPoints (point1, point2, point3);
    Check::Near (1.0, cross);
    Check::Near (15.0, dot0);
    Check::Near (9.0, dot1);
    Check::Near (47.0, dot2);
    Check::Near (3.0, triple);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, Zero)
    {
    DPoint3d point0 = DPoint3d::From (12.9, -4.3, -3.4);
    DPoint3d point1 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point2 = DPoint3d::From (1.0, 1.0, 1.0);
    DPoint3d point3 = DPoint3d::From (-54.0, 3.6, -78.9);
    point0.Zero ();
    point3.One ();
    Check::Near (point1, point0);
    Check::Near (point2, point3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, InitFromArray)
    {
    DPoint3d point0, point2, point3, point6;
    DPoint3d point1 = DPoint3d::From (2.0, -7.9, 8.9);
    DPoint3d point4 = DPoint3d::From (2.0, -7.9, 0.0);
    DPoint4d point5 = DPoint4d::From (2.0, -7.9, 8.9, 1.0);
    double pxy[3];
    pxy[0] = 2.0;
    pxy[1] = -7.9;
    pxy[2] = 8.9;
    point0.InitFromArray (pxy);
    point2.Init (2.0, -7.9, 8.9);
    point3.Init (2.0, -7.9);
    point6.XyzOf (point5);
    Check::Near (point1, point0);
    Check::Near (point1, point2);
    Check::Near (point4, point3);
    Check::Near (point1, point6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, SetComponent)
    {
    DPoint3d point1 = DPoint3d::From (2.0, -7.9, 5.0);
    DPoint3d point2 = DPoint3d::From (4.0, -7.9, 5.0);
    DPoint3d point3 = DPoint3d::From (2.0, 9.3, 5.0);
    DPoint3d point4 = DPoint3d::From (-5.1, -7.9, 5.0);
    DPoint3d point5 = DPoint3d::From (2.0, -7.9, 9.0);
    double xx = 2.0;
    double yy = -7.9;
    double zz = 5.0;
    int index0 = 0;
    int index1 = 1;
    int index2 = 2;
    int index3 = 3;
    double xCoord, yCoord, zCoord;
    point2.SetComponent (xx, index0);
    point3.SetComponent (yy, index1);
    point4.SetComponent (xx, index3);
    point5.SetComponent (zz, index2);
    double component0 = point1.GetComponent (index0);
    double component1 = point1.GetComponent (index1);
    double component2 = point1.GetComponent (index2);
    double component3 = point1.GetComponent (index3);
    point1.GetComponents (xCoord, yCoord, zCoord);
    Check::Near (point1, point2);
    Check::Near (point1, point3);
    Check::Near (point1, point4);
    Check::Near (point1, point5);
    Check::Near (xx, component0);
    Check::Near (yy, component1);
    Check::Near (zz, component2);
    Check::Near (xx, component3);
    Check::Near (xx, xCoord);
    Check::Near (yy, yCoord);
    Check::Near (zz, zCoord);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, Interpolate)
    {
    DPoint3d point0;
    DPoint3d point1 = DPoint3d::From (0.0, 1.0, 0.0);
    DPoint3d point2 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point3 = DPoint3d::From (0.0, 0.5, 0.0);
    double s = 0.5;
    point0.Interpolate (point2, s, point1);
    Check::Near (point3, point0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, IsPointInSmallerSector)
    {
    DPoint3d point0 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d vector0 = DPoint3d::From (1.0, 0.0, 0.0);
    DPoint3d vector1 = DPoint3d::From (0.0, 1.0, 0.0);
    DPoint3d vector2 = DPoint3d::From (0.5, 0.5, 0.0);
    DPoint3d vector3 = DPoint3d::From (-0.5, 0.5, 0.0);
    DPoint3d vector4 = DPoint3d::From (-0.5, -0.5, 0.0);
    DPoint3d vector5 = DPoint3d::From (0.5, -0.5, 0.0);
    DVec3d upVector = DVec3d::From (0.0, 1.0, 0.0);
    Check::True(vector2.IsPointInSmallerSector (point0, vector0, vector1));
    Check::True(!(vector3.IsPointInSmallerSector (point0, vector0, vector1)));
    Check::True(!(vector4.IsPointInSmallerSector (point0, vector0, vector1)));
    Check::True(!(vector5.IsPointInSmallerSector (point0, vector0, vector1)));
    Check::True(!(vector2.IsPointInCCWector (point0, vector0, vector1, upVector)));
    Check::True(vector3.IsPointInCCWector (point0, vector0, vector1, upVector));
    Check::True(vector4.IsPointInCCWector (point0, vector0, vector1, upVector));
    Check::True(vector5.IsPointInCCWector (point0, vector0, vector1, upVector));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, Distance)
    {
    DPoint3d vector0 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d vector1 = DPoint3d::From (4.0, 5.0, 0.0);
    DPoint3d vector2 = DPoint3d::From (-1.0, -7.8, 1.0);
    DMatrix4d matrix0 = DMatrix4d::FromRowValues (1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    double distance = 0.0;
    double distanceBetweenVec0Vec0 = vector0.Distance (vector0);
    double distanceBetweenVec1Vec2 = vector1.Distance (vector2);
    double distanceSquaredBetweenVec1Vec2 = vector1.DistanceSquared (vector2);
    double distanceSquaredXYBetweenVec1Vec2 = vector1.DistanceSquaredXY (vector2);
    double distanceXYBetweenVec1Vec2 = vector1.DistanceXY (vector2);
    Check::True (vector1.DistanceXY (vector2, &matrix0, distance), "distance with transform");
    Check::Near (0.0, distanceBetweenVec0Vec0);
    Check::Near (sqrt (189.84), distanceBetweenVec1Vec2);
    Check::Near (189.84, distanceSquaredBetweenVec1Vec2);
    Check::Near (188.84, distanceSquaredXYBetweenVec1Vec2);
    Check::Near (sqrt (188.84), distanceXYBetweenVec1Vec2);
    Check::Near (sqrt (188.84), distance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, MaxAbs)
    {
    DPoint3d point0 = DPoint3d::From (-2.0, -15.0, 1.0);
    DPoint3d point1 = DPoint3d::From (4.0, -5.0, 1.0);
    double maxVal0 = point0.MaxAbs ();
    double maxVal1 = point1.MaxAbs ();
    Check::Near (15.0, maxVal0);
    Check::Near (5.0, maxVal1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, IsEqual)
    {
    DPoint3d point0 = DPoint3d::From (2.0, 2.0, 2.0);
    DPoint3d point1 = DPoint3d::From (2.0, 2.0, 2.0);
    DPoint3d point2 = DPoint3d::From (1.0, 1.0, 1.0);
    DPoint3d point3 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point4 = DPoint3d::From (0.00001, 0.00001, 0.00001);
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
TEST(DPoint3d, NpcCoordinatesOf)
    {
    DPoint3d point0 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::From (2.0, 2.0, 2.0);
    DPoint3d point2, point3, point6;
    DPoint3d point4 = DPoint3d::From (1.0, 1.0, 1.0);
    DPoint3d point5 = DPoint3d::From (0.5, 0.5, 0.5);
    DRange3d range0 = DRange3d::From (point0, point1);
    point2.NpcCoordinatesOf (point0, range0);
    point3.NpcCoordinatesOf (point1, range0);
    point6.NpcCoordinatesOf (point4, range0);
    Check::Near (point0, point2);
    Check::Near (point4, point3);
    Check::Near (point5, point6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, IsDisconnect)
    {
    DPoint3d point0;
    DPoint3d point1 = DPoint3d::From (-2.0, -15.0, 9.0);
    point0.InitDisconnect ();
    Check::True (point0.IsDisconnect ());
    Check::True (!(point1.IsDisconnect ()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, Init)
    {
    DPoint3d point0, point2, point4, point6, point8, point10;
    DPoint3d point1 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point3 = DPoint3d::From (2.0, 4.0, 6.0);
    DPoint3d point5 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point7 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point9 = DPoint3d::From (11.0, 16.0, 21.0);
    DPoint3d point11 = DPoint3d::From (-9.0, 8.0, 9.0);
    DVec3d vector0 = DVec3d::From (1.0, 2.0, 3.0);
    DVec3d vector1 = DVec3d::From (3.0, 4.0, 5.0);
    DVec3d vector2 = DVec3d::From (-5.0, -2.0, -3.0);
    double scale0 = 2.0;
    double scale1 = 3.0;
    double scale2 = 4.0;
    point0.Init (vector0);
    point2.SumOf (point1, vector0);
    point4.Subtract (point3, vector0);
    point5.Add (vector0);
    point6.SumOf (point7, vector0, scale0);
    point8.SumOf (point7, vector0, scale0, vector1, scale1);
    point10.SumOf (point7, vector0, scale0, vector1, scale1, vector2, scale2);
    Check::Near (point1, point0);
    Check::Near (point3, point2);
    Check::Near (point1, point4);
    Check::Near (point3, point5);
    Check::Near (point3, point6);
    Check::Near (point9, point8);
    Check::Near (point11, point10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, CrossProduct)
    {
    DPoint3d point0, point6, point7, point8, point9, point10;
    DPoint3d point1 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point3 = DPoint3d::From (2.0, 5.0, 6.0);
    DPoint3d point4 = DPoint3d::From (-3.0, 0.0, 1.0);
    DPoint3d point5 = DPoint3d::From (0.0, 0.0, 0.0);
    double c = sqrt (sqrt (910.0));
    point0.CrossProduct (point1, point3);
    point6.CrossProductToPoints (point5, point1, point3);
    double cross0 = point1.CrossProductXY (point3);
    double magnitude0 = point7.NormalizedCrossProduct (point1, point3);
    double magnitude2 = point8.SizedCrossProduct (point1, point3, 1.0);
    double magnitude3 = point9.GeometricMeanCrossProduct (point1, point3);
    point10.Scale (point4, c/magnitude3);
    Check::Near (point4, point0);
    Check::Near (point4, point6);
    Check::Near (1.0, cross0);
    double magnitude1 = point4.Normalize ();
    Check::Near (point4, point7);
    Check::Near (magnitude1, magnitude0);
    Check::Near (point4, point8);
    Check::Near (magnitude1, magnitude2);
    Check::Near (magnitude1, magnitude3);
    Check::Near (point10, point9);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, DotProduct)
    {
    DPoint3d point0 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point1 = DPoint3d::From (2.0, 5.0, 6.0);
    DPoint3d point2;
    DPoint3d point3 = DPoint3d::From (1.0, 3.0, 3.0);
    double dot0 = point0.DotProduct (point1);
    double dot1 = point0.DotProductXY (point1);
    double dot2 = point0.DotProduct (2.0, 5.0, 6.0);
    double length0 = point2.NormalizedDifference (point1, point0);
    double length1 = point3.Normalize ();
    Check::Near (30.0, dot0);
    Check::Near (12.0, dot1);
    Check::Near (30.0, dot2);
    Check::Near (length1, length0);
    Check::Near (point3, point2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, AngleTo)
    {
    DPoint3d point0 = DPoint3d::From (1.0, 0.0, 0.0);
    DPoint3d point1 = DPoint3d::From (0.0, 1.0, 0.0);
    DPoint3d point2 = DPoint3d::From (1.0, 1.0, 0.0);
    DPoint3d point3 = DPoint3d::From (1.0, -1.0, 0.0);
    DPoint3d point4 = DPoint3d::From (0.0, 0.0, 1.0);
    double angle0 = point0.AngleTo (point1);
    double angle1 = point0.AngleTo (point2);
    double angle2 = point2.AngleTo (point0);
    double angle3 = point1.SmallerUnorientedAngleTo (point3);
    double angle4 = point0.AngleToXY (point1);
    double angle5 = point0.AngleToXY (point2);
    double angle6 = point2.AngleToXY (point0);
    double angle7 = point1.SmallerUnorientedAngleToXY (point3);
    double angle8 = point2.SignedAngleTo (point0, point4);
    double angle9 = point0.PlanarAngleTo (point1, point4);
    double angle10 = point1.PlanarAngleTo (point0, point4);
    Check::Near (PI/2, angle0);
    Check::Near (PI/4, angle1);
    Check::Near (PI/4, angle2);
    Check::Near (PI/4, angle3);
    Check::Near (PI/2, angle4);
    Check::Near (PI/4, angle5);
    Check::Near (-PI/4, angle6);
    Check::Near (PI/4, angle7);
    Check::Near (-PI/4, angle8);
    Check::Near (PI/2, angle9);
    Check::Near (-PI/2, angle10);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, IsVectorInSmallerSector)
    {
    //DPoint3d point0 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d vector0 = DPoint3d::From (1.0, 0.0, 0.0);
    DPoint3d vector1 = DPoint3d::From (0.0, 1.0, 0.0);
    DPoint3d vector2 = DPoint3d::From (0.5, 0.5, 0.0);
    DPoint3d vector3 = DPoint3d::From (-0.5, 0.5, 0.0);
    DPoint3d vector4 = DPoint3d::From (-0.5, -0.5, 0.0);
    DPoint3d vector5 = DPoint3d::From (0.5, -0.5, 0.0);
    DVec3d upVector = DVec3d::From (0.0, 1.0, 0.0);
    Check::True(vector2.IsVectorInSmallerSector (vector0, vector1));
    Check::True(!(vector3.IsVectorInSmallerSector (vector0, vector1)));
    Check::True(!(vector4.IsVectorInSmallerSector (vector0, vector1)));
    Check::True(!(vector5.IsVectorInSmallerSector (vector0, vector1)));
    Check::True(!(vector2.IsVectorInCCWSector (vector0, vector1, upVector)));
    Check::True(vector3.IsVectorInCCWSector (vector0, vector1, upVector));
    Check::True(vector4.IsVectorInCCWSector (vector0, vector1, upVector));
    Check::True(vector5.IsVectorInCCWSector (vector0, vector1, upVector));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, RotateXY)
    {
    DPoint3d point0,point5;
    DPoint3d point1 = DPoint3d::From (1.0, 0.0, 0.0);
    DPoint3d point2 = DPoint3d::From (0.0, 1.0, 0.0);
    DPoint3d point6 = DPoint3d::From (0.0, -1.0, 0.0);
    double radian0 = PI/2;
    double radian1 = -PI;
    point0.RotateXY (point1, radian0);
    point1.RotateXY (radian0);
    point5.RotateXY (point2, radian1);
    Check::Near (point2, point0);
    Check::Near (point2, point1);
    Check::Near (point6, point5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, WeightedDifferenceOf)
    {
    DPoint3d point0, point5, point7;
    DPoint4d point1 = DPoint4d::From (5.0, 6.0, 3.0, 7.0);
    DPoint4d point2 = DPoint4d::From (2.0, 6.0, 3.0, 4.0);
    DPoint3d point3 = DPoint3d::From (6.0, -18.0, -9.0);
    DPoint4d point4 = DPoint4d::From (3.0, 5.0, 1.0, 2.0);
    //DPoint3d point6 = DPoint3d::From (6.0, -18.0, -9.0);
    DPoint3d point8 = DPoint3d::From (-11.0, -23.0, -1.0);    
    DPoint3d point9 = DPoint3d::From (-8.0, -8.0, 2.0);
    point0.WeightedDifferenceOf (point1, point2);
    point5.WeightedDifferenceCrossProduct (point4, point1, point2);
    point7.CrossProduct (point8, point9);
    Check::Near (point3, point0);
    Check::Near (point7, point5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, MagnitudeSquared)
    {
    DPoint3d point0 = DPoint3d::From (6.0, -18.0, -9.0);
    DPoint3d point1;
    DPoint3d point2 = DPoint3d::From (12.0, -36.0, -18.0);
    DPoint3d point3 = DPoint3d::From (11.0, -4.0, -16.0);
    DPoint3d point4 = DPoint3d::From (22.0, -8.0, -32.0);
    double scale0 = 2.0;
    double magnitude0 = point0.MagnitudeSquared ();
    double magnitude1 = point0.MagnitudeXY ();
    double magnitude2 = point0.MagnitudeSquaredXY ();
    double magnitude3 = point0.Magnitude ();
    point1.Scale (point0, scale0);
    point3.Scale (scale0);
    Check::Near (441.0, magnitude0);
    Check::Near (sqrt (360.0), magnitude1);
    Check::Near (360.0, magnitude2);
    Check::Near (sqrt (441.0), magnitude3);
    Check::Near (point2, point1);
    Check::Near (point4, point3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, UnitPerpendicularXY)
    {
    DPoint3d point0;
    DPoint3d point1 = DPoint3d::From (2.0, 0.0, 0.0);
    DPoint3d point2 = DPoint3d::From (0.0, 1.0, 0.0);
    Check::True (point0.UnitPerpendicularXY (point1));
    Check::Near (point2, point0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, Negate)
    {
    DPoint3d point0, point3;
    DPoint3d point1 = DPoint3d::From (2.0, -5.0, 1.0);
    DPoint3d point2 = DPoint3d::From (-2.0, 5.0, -1.0);
    point0.Negate (point1);
    Check::Near (point2, point0);
    point2.Negate ();
    Check::Near (point1, point2);
    double length0 = point3.Normalize (point1);
    double length1 = point3.Magnitude ();
    double f = 1.0 / (sqrt (30.0));
    DPoint3d point4 = DPoint3d::From (2.0 * f, -5.0 * f, 1.0 * f);
    Check::Near (sqrt (30.0), length0);
    Check::Near (1.0, length1);
    Check::Near (point4, point3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, ScaleToLength)
    {
    DPoint3d point0;
    DPoint3d point1 = DPoint3d::From (1.0, 0.0, 0.0);
    DPoint3d point2 = DPoint3d::From (2.0, 0.0, 0.0);
    DPoint3d point3 = DPoint3d::From (1.0, 0.0, 0.0);
    double scale0 = 2.0;
    double length0 = point0.ScaleToLength (point1, scale0);
    Check::Near (point2, point0);
    Check::Near (1.0, length0);
    double length1 = point1.ScaleToLength (scale0);
    Check::Near (point2, point1);
    Check::Near (1.0, length1);
    double length2 = point2.Normalize ();
    Check::Near (2.0, length2);
    Check::Near (point3, point2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, IsParallelTo)
    {
    DPoint3d point0 = DPoint3d::From (2.0, 2.0, 2.0);
    DPoint3d point1 = DPoint3d::From (1.0, 1.0, 1.0);
    DPoint3d point2 = DPoint3d::From (1.0, 0.0, 0.0);
    DPoint3d point3 = DPoint3d::From (0.0, 1.0, 0.0);
    Check::True (point0.IsParallelTo (point1));
    Check::True (!(point0.IsParallelTo (point2)));
    Check::True (point2.IsPerpendicularTo (point3));
    Check::True (!(point2.IsPerpendicularTo (point1)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, SafeDivide)
    {
    DPoint3d point0, point3;
    DPoint3d point1 = DPoint3d::From (1.0, 1.0, 1.0);
    DPoint3d point2 = DPoint3d::From (0.5, 0.5, 0.5);
    DPoint3d point5 = DPoint3d::From (2.0, 3.0, 4.0);
    double denominator0 = 0.0;
    double denominator1 = 2.0;
    Check::True (!(point0.SafeDivide (point1, denominator0)));
    Check::True (point3.SafeDivide (point1, denominator1));
    double product = point2.TripleProduct (point1, point5);
    Check::Near (point2, point3);
    Check::Near (0.0, product);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, Subtract)
    {
    DPoint3d point0 = DPoint3d::From (1.0, 1.0, 1.0);
    DPoint3d point1 = DPoint3d::From (0.5, 0.5, 0.5);
    DPoint3d point2, point4, point7, point11, point15;
    DPoint3d point3 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point5 = DPoint3d::From (1.5, 1.5, 1.5);
    DPoint3d point6 = DPoint3d::From (2.0, 3.0, 4.0);
    DPoint3d point8 = DPoint3d::From (5.5, 7.5, 9.5);
    DPoint3d point9 = DPoint3d::From (8.0, 7.0, 5.0);
    DPoint3d point10 = DPoint3d::From (37.5, 35.5, 29.5);
    DPoint3d point12 = DPoint3d::From (5.0, 2.0, 1.0);
    DPoint3d point13 = DPoint3d::From (2.0, 8.0, 4.0);
    DPoint3d point14 = DPoint3d::From (7.0, 10.0, 5.0);
    DPoint3d point16 = DPoint3d::From (9.0, 18.0, 9.0);
    double scale0 = 3.0;
    double scale1 = 2.0;
    double scale2 = 4.0;
    point0.Subtract (point1);
    point2.DifferenceOf (point0, point1);
    point4.SumOf (point3, point1, scale0);
    point7.SumOf (point3, point1, scale0, point6, scale1);
    point11.SumOf (point3, point1, scale0, point6, scale1, point9, scale2);
    point12.Add (point13);
    point15.SumOf (point12, point13);
    Check::Near (point1, point0);
    Check::Near (point3, point2);
    Check::Near (point5, point4);
    Check::Near (point8, point7);
    Check::Near (point10, point11);
    Check::Near (point14, point12);
    Check::Near (point16, point15);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, MultiplyArrayByScales)
    {
    DPoint3d point0[2];
    DPoint3d point1[2];
    point1[0] = DPoint3d::From (0.5, 0.5, 0.5);
    point1[1] = DPoint3d::From (2.0, 3.0, 4.0);
    DPoint3d point2 = DPoint3d::From (1.5, 1.5, 1.5);
    DPoint3d point3 = DPoint3d::From (6.0, 9.0, 12.0);
    double scale0[2];
    scale0[0] = scale0[1] = 3.0;
    DPoint3d::MultiplyArrayByScales (point0, point1, scale0, 2);
    Check::Near (point2, point0[0]);
    Check::Near (point3, point0[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, DivideArrayByScales)
    {
    DPoint3d point0[2];
    DPoint3d point1[2];
    point1[0] = DPoint3d::From (1.5, 1.5, 1.5);
    point1[1] = DPoint3d::From (6.0, 9.0, 12.0);
    DPoint3d point2 = DPoint3d::From (0.5, 0.5, 0.5);
    DPoint3d point3 = DPoint3d::From (2.0, 3.0, 4.0);
    double scale0[2];
    scale0[0] = scale0[1] = 3.0;
    DPoint3d::DivideArrayByScales (point0, point1, scale0, 2);
    Check::Near (point2, point0[0]);
    Check::Near (point3, point0[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, FromProduct)
    {
    Transform transform0 = Transform::FromIdentity ();
    DPoint3d point0 = DPoint3d::FromProduct (transform0, 2.0, 3.0, 4.0);
    DPoint3d point1 = DPoint3d::From (2.0, 3.0, 4.0);
    DPoint3d point2 = DPoint3d::From (0.0, 0.0, 0.0);
    RotMatrix matrix0 = RotMatrix::FromIdentity ();
    DPoint3d point3 = DPoint3d::FromProduct (point2, matrix0, 2.0, 3.0, 4.0);
    DPoint3d point4 = DPoint3d::FromProduct (transform0, point1);
    Check::Near (point1, point0);
    Check::Near (point1, point3);
    Check::Near (point1, point4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, FromInterpolate)
    {
    DPoint3d point0 = DPoint3d::From (2.0, 3.0, 4.0);
    DPoint3d point1 = DPoint3d::From (5.0, 8.0, 6.0);
    double fraction = 0.5;
    DPoint3d point2 = DPoint3d::FromInterpolate (point0, fraction, point1);
    DPoint3d point3 = DPoint3d::From (3.5, 5.5, 5.0);
    Check::Near (point3, point2);
    }

void Create (DPoint4d &data, double x, double y, double z, double w)    {data.Init (x,y,z, w);}
void Create (DPoint3d &data, double x, double y, double z, double w)    {data.Init (x,y,z);}
void Create (DVec3d   &data, double x, double y, double z, double w)    {data.Init (x,y,z);}
void Create (DPoint2d &data, double x, double y, double z, double w)    {data.Init (x,y);}
void Create (DVec2d   &data, double x, double y, double z, double w)    {data.Init (x,y);}

template<typename PointType, typename VectorType>
void TestSums (double ax, double ay, double az)
    {
    // Create instances with full xyz.  Collapse to instantiation type via "From" method...
    PointType origin;
    VectorType xVec;
    VectorType yVec;
    VectorType zVec;

    
    Create (origin, 1,2,3, 1);

    Create (xVec, 4,7,8, 1.1);
    Create (yVec, -2,3,1.5,0.9);
    Create (zVec, 1.3, 2.34, -0.4, 0);

    //VectorType xScale = VectorType::FromScale (xVec, ax);
    //VectorType yScale = VectorType::FromScale (yVec, ay);
    PointType sum1    = PointType::FromSumOf (origin, xVec, ax);
    PointType sum2    = PointType::FromSumOf (origin, xVec, ax, yVec, ay);
    PointType sum3    = PointType::FromSumOf (origin, xVec, ax, yVec, ay, zVec, az);
    PointType sum12   = PointType::FromSumOf (sum2, zVec, az);
    PointType sum2a   = PointType::FromSumOf (sum1, yVec, ay);
    Check::Near (sum3, sum12, "FromSumOf");
    Check::Near (sum2, sum2a, "FromSumOf");


    PointType alpha0 = PointType::FromScale (xVec, ax);
    PointType beta0  = PointType::FromSumOf (xVec, ax, yVec, ay);
    PointType gamma0 = PointType::FromSumOf (xVec, ax, yVec, ay, zVec, az);
    PointType zeta0  = PointType::FromScale (zVec, az);
    PointType mu0    = PointType::FromSumOf (beta0, 1.0, zeta0, 1.0);
    Check::Near (mu0, gamma0, "PointType::FromSumOf");

    VectorType alpha1 = VectorType::FromScale (xVec, ax);
    VectorType beta1  = VectorType::FromSumOf (xVec, ax, yVec, ay);
    VectorType gamma1 = VectorType::FromSumOf (xVec, ax, yVec, ay, zVec, az);
    VectorType mu1    = VectorType::FromSumOf (beta1, 1.0, zVec, az);
    Check::Near (mu1, gamma1, "VectorType::FromSumOf");

    Check::Near (alpha0, alpha1, "FromScale as point, vector");
    Check::Near (beta0, beta1, "FromScale as point, vector");
    Check::Near (gamma0, gamma1, "FromScale as point, vector");

    PointType target = PointType::FromSumOf (origin, xVec, 1.0);
    for (double f = -0.25; f < 1.5; f += 0.25)
        {
        PointType point0 = PointType::FromSumOf (origin, xVec, f);
        PointType point1 = PointType::FromInterpolate (origin, f, target);
        Check::Near (point0, point1, "Interpolation vs direct sum");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DPoint3d, SumOf_Variants)
    {
    TestSums <DPoint3d, DVec3d> (2, 3, 0.12312);
    TestSums <DPoint2d, DVec2d> (2, 3, 0.12312);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d,AppendInterpolated)
    {
    bvector<DPoint3d> points;
    DPoint3d pointA = DPoint3d::From (1,0,0);
    DPoint3d pointB = DPoint3d::From (2,0,0);
    DPoint3d pointC = DPoint3d::From (3,3,1);
    
    size_t n = 4;
    DPoint3dOps::AppendInterpolated (points, pointA, pointB, 4, true);
    Check::Size (n, points.size (), "First AppendInterpolated");
    Check::Near (pointA, points.front (), "front");
    Check::Near (pointB, points.back (), "back");
    DPoint3dOps::AppendInterpolated (points, pointB, pointC, 4, false);
    Check::Size (2 * n - 1, points.size (), "Second AppendInterpolated");
    Check::Near (pointC, points.back (), "back");
    
    }
    

/// @param [IN] planeOrigin 00 point of the plane
/// @param [IN] planeXPoint (s=1,t=0) point of the plane -- target along nominal X direction.
/// @parma [IN] planeYPoint (s=0,t=1) point of the plane -- target along nominal Y direction.
/// @param [IN] planeXFraction fractional position along the plane X direction
/// @param [IN] planeYFraction fractional postion along the plane Y direction
DPoint3d PlaneFractionsToPoint
(
DPoint3dCR planeOrigin,
DPoint3dCR planeXTarget,
DPoint3dCR planeYTarget,
double planeXFraction,
double planeYFraction
)
    {
    DVec3d planeXDirection = DVec3d::FromStartEnd (planeOrigin, planeXTarget);
    DVec3d planeYDirection = DVec3d::FromStartEnd (planeOrigin, planeYTarget);
    return DPoint3d::FromSumOf (planeOrigin, planeXDirection, planeXFraction, planeYDirection, planeYFraction);
    }

/// Intersect a ray (defined by origin and target) with a plane (defined by 3 points)
/// @param [IN] rayOrigin origin of ray.
/// @param [IN] rayTarget target point of the ray.
/// @param [IN] planeOrigin 00 point of the plane
/// @param [IN] planeXPoint (s=1,t=0) point of the plane -- target along nominal X direction.
/// @parma [IN] planeYPoint (s=0,t=1) point of the plane -- target along nominal Y direction.
/// @param [OUT] rayFraction fractional position (possibly negative or larger than 1) along the ray.    
/// @param [OUT] planeXFraction fractional position along the plane X direction
/// @param [OUT] planeYFraction fractional postion along the plane Y direction
bool RayBy2PointsIntersectPlaneBy3Points
(
DPoint3dCR rayOrigin,
DPoint3dCR rayTarget,
DPoint3dCR planeOrigin,
DPoint3dCR planeXTarget,
DPoint3dCR planeYTarget,
double &rayFraction,
double &planeXFraction,
double &planeYFraction
)
    {
    DVec3d rayDirection = DVec3d::FromStartEnd (rayOrigin, rayTarget);
    DVec3d planeXDirection = DVec3d::FromStartEnd (planeOrigin, planeXTarget);
    DVec3d planeYDirection = DVec3d::FromStartEnd (planeOrigin, planeYTarget);
    RotMatrix matrix = RotMatrix::FromColumnVectors (planeXDirection, planeYDirection, rayDirection);
    DVec3d rhs = DVec3d::FromStartEnd (planeOrigin, rayOrigin);
    DVec3d stLambda;
    if (matrix.Solve (stLambda, rhs))
        {
        planeXFraction = stLambda.x;
        planeYFraction = stLambda.y;
        rayFraction    = -stLambda.z;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d,RayPlaneExample)
    {
    DPoint3d rayOrigin = DPoint3d::From (1,2,3);
    DPoint3d rayTarget = DPoint3d::From (5,2,9);
    DPoint3d planeOrigin = DPoint3d::From (10,12,0);
    DPoint3d planeXTarget = DPoint3d::From (10,0,0);
    DPoint3d planeYTarget = DPoint3d::From (10,12,20);
    double rayFraction, planeXFraction, planeYFraction;
    Check::True (RayBy2PointsIntersectPlaneBy3Points (rayOrigin, rayTarget, planeOrigin, planeXTarget, planeYTarget, rayFraction, planeXFraction, planeYFraction), "ray plane intersection");
    DPoint3d planePoint = PlaneFractionsToPoint (planeOrigin, planeXTarget, planeYTarget, planeXFraction, planeYFraction);
    DPoint3d rayPoint   = DPoint3d::FromInterpolate (rayOrigin, rayFraction, rayTarget);

    Check::Near (rayPoint, planePoint, "rayPlane intersection resolved");    
    
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint4d,Interpolate)
    {
    DPoint4d pointA = DPoint4d::From (1,2,3, 1.25);
    DPoint4d pointB = DPoint4d::From (7,5,11, 0.75);
    DPoint4d deltaAB, deltaAC;
    DPoint4d pointC, pointD;
    for (double f = -2.0; f < 3.0; f += 0.25)
        {
        pointC.Interpolate (pointA, f, pointB);
        pointD.Interpolate (pointB, 1.0 - f, pointA);
        Check::Near (pointC, pointD, "Reverse interpolation");
        deltaAB.DifferenceOf (pointB, pointA);
        deltaAC.DifferenceOf (pointC, pointA);
        Check::Near (deltaAC.MagnitudeXYZW (), fabs (f) * deltaAB.MagnitudeXYZW (), "interpolated distance");
        }
    }



void CheckEvaluatedPointExtents (DEllipse3dCR ellipse)
    {
    bvector<DPoint3d> points;
    DVec3d perp;
    perp.GeometricMeanCrossProduct (ellipse.vector0, ellipse.vector90);
    double dTheta = 0.20;
    size_t numTheta = 40;
    for (size_t i = 0; i < numTheta; i++)
        {
        double theta = i * dTheta;
        points.push_back (DPoint3d::FromSumOf (ellipse.center, ellipse.vector0, cos (theta), ellipse.vector90, sin (theta)));
        }

    Transform principalExtents;
    if (Check::True (DPoint3dOps::PrincipalExtents (points, principalExtents)))
        {
        DVec3d xAxis, yAxis, zAxis;
        principalExtents.GetMatrixColumn (xAxis, 0);
        principalExtents.GetMatrixColumn (yAxis, 1);
        principalExtents.GetMatrixColumn (zAxis, 2);
        Check::True (xAxis.Magnitude () >= yAxis.Magnitude ());
        DPoint3d origin;
        principalExtents.GetTranslation (origin);
        Check::True (xAxis.IsPerpendicularTo (perp));
        Check::True (yAxis.IsPerpendicularTo (perp));
        Check::True (xAxis.IsPerpendicularTo (yAxis));
        Transform npcToWorld;
        npcToWorld.InitFromOriginAndVectors (origin, xAxis, yAxis, perp);   // zAxis is zero because data really is planar.
        Transform worldToNPC;
        worldToNPC.InverseOf (npcToWorld);
        DRange3d npcRange = DPoint3dOps::Range (&points, worldToNPC);
        // The local range should achieve its minmax claims...
        //double tol = Angle::SmallAngle ();
        Check::Near (npcRange.low.x,  0.0);
        Check::Near (npcRange.low.y,  0.0);
        Check::Near (npcRange.high.x, 1.0);
        Check::Near (npcRange.high.y, 1.0);
        Check::Near (npcRange.low.z,  0.0);
        Check::Near (npcRange.high.z, 0.0); // Because the data really is planar.
        // Introduce some z variation ...
        static double s_zFraction = 0.1;
        static size_t zStep = 5;
        for (size_t i = 0; i < numTheta; i += zStep)
          points[i] = DPoint3d::FromSumOf (points[i], perp, s_zFraction);
        Transform principalExtents2;
        Check::True (DPoint3dOps::PrincipalExtents (points, principalExtents2));
        // A few points have moved a little off the plane.
        // Expect the z to show just a little magnitude ..
        principalExtents2.GetMatrixColumn (xAxis, 0);
        principalExtents2.GetMatrixColumn (yAxis, 1);
        principalExtents2.GetMatrixColumn (zAxis, 2);
        Check::True (xAxis.Magnitude () >= yAxis.Magnitude ());
        double s_zAxisFractionAcceptFraction = 3.0 * s_zFraction;
        double az = zAxis.Magnitude ();
        double aPerp = perp.Magnitude ();
        Check::True (az < s_zAxisFractionAcceptFraction * aPerp);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d,PrincipalExtents)
  {

  DEllipse3d ellipse = DEllipse3d::From (1,2,3, 6,2,0, -1, 3,0, 0.0, Angle::DegreesToRadians (360));
  CheckEvaluatedPointExtents (ellipse);
  ellipse.vector90.z += 1.0;
  CheckEvaluatedPointExtents (ellipse);
  }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d,PolygonLinestringApproach)
  {
  bvector<DPoint3d> polygon;
  bvector<DPoint3d> linestring;

  polygon.push_back (DPoint3d::From (1,1,0));
  polygon.push_back (DPoint3d::From (-1,1,0));
  polygon.push_back (DPoint3d::From (-1,-1,0));
  polygon.push_back (DPoint3d::From (1,-1,0));
  polygon.push_back (DPoint3d::From (1,1,0));
  DPoint3d pointA, pointB, pointB0;
  pointB0.Zero ();
  for (int i = 0 ; i < 4; i++)  // build tests at corner point i
      {
      for (double z0 = -1.5; z0 < 1.0; z0 += 1.0)
          {
          DPoint3d cornerPoint = polygon[i];
          DPoint3d planePoint = cornerPoint;
          planePoint.Scale (2.0);
          for (int numB = 2; numB < 4; numB++)
              {
              double dz = 1.0 / (numB - 1);
              linestring.clear ();
              for (size_t k = 0; (int)k < numB; k++)
                  linestring.push_back (DPoint3d::From (planePoint.x, planePoint.y, z0 + k * dz));
              bsiPolygon_closestApproachBetweenPolygonAndLineString (pointA, pointB,
                      &polygon.front (), (int)polygon.size (), &linestring.front (), (int)linestring.size());

              Check::Near (cornerPoint, pointA);

              // confirm pointB not affected by splitting the B line
              if (numB == 2)
                  {
                  pointB0 = pointB;
                  }
              else
                  {
                  Check::Near (pointB0, pointB);
                  }
              }
          }
      }
  }

void testCrossProduct (DPoint4dCR a, DPoint4dCR b, DPoint4dCR c, DPoint4dCR d, double expectedSign)
    {
    DPoint4d d1 = DPoint4d::FromCrossProduct (a, b, c);
    DPoint4d d2 = DPoint4d::FromCrossProduct (b, c, a);
    DPoint4d d3 = DPoint4d::FromCrossProduct (c, a, b);
    DPoint4d ds = d;
    ds.Scale (expectedSign);
    if (!Check::Near (ds, d1))
        {
        Check::PrintIndent (2);Check::Print (a, "a");
        Check::PrintIndent (2);Check::Print (b, "b");
        Check::PrintIndent (2);Check::Print (c, "c");
        Check::PrintIndent (2);Check::Print (d, "expected d");
        Check::PrintIndent (2);Check::Print (expectedSign, "expectedSign");
        Check::PrintIndent (2);Check::Print (d1, "computed d");
        }

    Check::Near (d1, d2);
    Check::Near (d1, d3);
    Check::Near (0.0, d.DotProduct (a));
    Check::Near (0.0, d.DotProduct (b));
    Check::Near (0.0, d.DotProduct (c));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
 TEST(DPoint4d,CrossProductA)
  {
  DPoint4d unitX = DPoint4d::From (1,0,0,0);
  DPoint4d unitY = DPoint4d::From (0,1,0,0);
  DPoint4d unitZ = DPoint4d::From (0,0,1,0);
  DPoint4d unitW = DPoint4d::From (0,0,0,1);

// The sign of the result seems to be such that the 
// determinant [result a b c] is positive.
// hence determinant [a b c result] which is 3 swaps away is negative.
  testCrossProduct (unitX, unitY, unitZ, unitW, -1.0);
  testCrossProduct (unitY, unitX, unitZ, unitW, -1.0);
  testCrossProduct (unitX, unitZ, unitY, unitW, -1.0);
  testCrossProduct (unitZ, unitY, unitX, unitW, -1.0);

  testCrossProduct (unitY, unitZ, unitW, unitX, 1.0);
  testCrossProduct (unitZ, unitY, unitW, unitX, -1.0);
  testCrossProduct (unitY, unitW, unitZ, unitX, -1.0);
  testCrossProduct (unitW, unitZ, unitY, unitX, -1.0);

  testCrossProduct (unitZ, unitW, unitX, unitY, -1.0);
  testCrossProduct (unitZ, unitX, unitW, unitY,  1.0);
  testCrossProduct (unitX, unitW, unitZ, unitY,  1.0);
  testCrossProduct (unitW, unitZ, unitX, unitY,  1.0);

  testCrossProduct (unitX, unitY, unitW, unitZ, 1.0);
  testCrossProduct (unitY, unitX, unitW, unitZ, -1.0);
  testCrossProduct (unitW, unitY, unitX, unitZ, -1.0);
  testCrossProduct (unitY, unitX, unitW, unitZ, -1.0);

  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint4d,CrossProductB)
  {
  Check::StartScope ("CrossProduct is perpendicular to its args");
  // Make 3 dpoint4d that are clearly independent
  DPoint4d X = DPoint4d::From (5,2,1,1);
  DPoint4d Y = DPoint4d::From (0.35,4.0,1,1);
  DPoint4d Z = DPoint4d::From (0.2, 0.4, 5.0, 0.3);
  DPoint4d W = DPoint4d::FromCrossProduct (X, Y, Z);
  double dx = W.DotProduct (X);
  Check::Near (0.0, dx);
  Check::Near (0.0, W.DotProduct (Y));
  Check::Near (0.0, W.DotProduct (Z));
  Check::EndScope ();
  }

void CollectGFormat (bvector<Utf8String> &strings)
    {
    bvector<double> values {1.0, 2.0, 1.25, 2.242342319438947348972389172389, 4.0 / 3.0, 9.0/7.0};
    char buffer[1024];
    for (double s : bvector<double>{1.0, 10.0, 1000.0, 1.0e6})
        {
//        printf ("| Scale Factor %.17g| | |\n", s);
//        printf ("| (17g) | (g) | (8g)|\n");
        for (auto x : values)
            {
            double y = x * s;
            sprintf (buffer, "| %.17g  | %g | %.8g|", y, y, y);
            strings.push_back (Utf8String(buffer));
            }
//        printf ("| Scale Factor 1.0/%.17g | | |\n", s);
//        printf ("| (17g) | (g) | (8g)|\n");
        for (auto x : values)
            {
            double y = x / s;
            sprintf (buffer, "| %.17g  | %g | %.8g|", y, y, y);
            strings.push_back (Utf8String(buffer));
            }
        }
    }
static bvector<Utf8String> s_baseline
    {
 Utf8String ("| 1  | 1 | 1|"),
 Utf8String ("| 2  | 2 | 2|"),
 Utf8String ("| 1.25  | 1.25 | 1.25|"),
 Utf8String ("| 2.2423423194389471  | 2.24234 | 2.2423423|"),
 Utf8String ("| 1.3333333333333333  | 1.33333 | 1.3333333|"),
 Utf8String ("| 1.2857142857142858  | 1.28571 | 1.2857143|"),
 Utf8String ("| 1  | 1 | 1|"),
 Utf8String ("| 2  | 2 | 2|"),
 Utf8String ("| 1.25  | 1.25 | 1.25|"),
 Utf8String ("| 2.2423423194389471  | 2.24234 | 2.2423423|"),
 Utf8String ("| 1.3333333333333333  | 1.33333 | 1.3333333|"),
 Utf8String ("| 1.2857142857142858  | 1.28571 | 1.2857143|"),
 Utf8String ("| 10  | 10 | 10|"),
 Utf8String ("| 20  | 20 | 20|"),
 Utf8String ("| 12.5  | 12.5 | 12.5|"),
 Utf8String ("| 22.423423194389471  | 22.4234 | 22.423423|"),
 Utf8String ("| 13.333333333333332  | 13.3333 | 13.333333|"),
 Utf8String ("| 12.857142857142858  | 12.8571 | 12.857143|"),
 Utf8String ("| 0.10000000000000001  | 0.1 | 0.1|"),
 Utf8String ("| 0.20000000000000001  | 0.2 | 0.2|"),
 Utf8String ("| 0.125  | 0.125 | 0.125|"),
 Utf8String ("| 0.22423423194389472  | 0.224234 | 0.22423423|"),
 Utf8String ("| 0.13333333333333333  | 0.133333 | 0.13333333|"),
 Utf8String ("| 0.12857142857142859  | 0.128571 | 0.12857143|"),
 Utf8String ("| 1000  | 1000 | 1000|"),
 Utf8String ("| 2000  | 2000 | 2000|"),
 Utf8String ("| 1250  | 1250 | 1250|"),
 Utf8String ("| 2242.342319438947  | 2242.34 | 2242.3423|"),
 Utf8String ("| 1333.3333333333333  | 1333.33 | 1333.3333|"),
 Utf8String ("| 1285.7142857142858  | 1285.71 | 1285.7143|"),
 Utf8String ("| 0.001  | 0.001 | 0.001|"),
 Utf8String ("| 0.002  | 0.002 | 0.002|"),
 Utf8String ("| 0.00125  | 0.00125 | 0.00125|"),
 Utf8String ("| 0.0022423423194389471  | 0.00224234 | 0.0022423423|"),
 Utf8String ("| 0.0013333333333333333  | 0.00133333 | 0.0013333333|"),
 Utf8String ("| 0.0012857142857142859  | 0.00128571 | 0.0012857143|"),
 Utf8String ("| 1000000  | 1e+06 | 1000000|"),
 Utf8String ("| 2000000  | 2e+06 | 2000000|"),
 Utf8String ("| 1250000  | 1.25e+06 | 1250000|"),
 Utf8String ("| 2242342.3194389469  | 2.24234e+06 | 2242342.3|"),
 Utf8String ("| 1333333.3333333333  | 1.33333e+06 | 1333333.3|"),
 Utf8String ("| 1285714.2857142859  | 1.28571e+06 | 1285714.3|"),
 Utf8String ("| 9.9999999999999995e-07  | 1e-06 | 1e-06|"),
 Utf8String ("| 1.9999999999999999e-06  | 2e-06 | 2e-06|"),
 Utf8String ("| 1.2500000000000001e-06  | 1.25e-06 | 1.25e-06|"),
 Utf8String ("| 2.2423423194389473e-06  | 2.24234e-06 | 2.2423423e-06|"),
 Utf8String ("| 1.3333333333333332e-06  | 1.33333e-06 | 1.3333333e-06|"),
 Utf8String ("| 1.2857142857142858e-06  | 1.28571e-06 | 1.2857143e-06|"),
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(General,GFormat)
    {
    bvector <Utf8String> strings;
    CollectGFormat (strings);
#define PrintBvectorInitialization_not
#ifdef PrintBvectorInitialization
    for (auto &s : strings)
        {
        printf (" Utf8String (\"%s\"),\n", s.c_str());
        }
#endif
#define CompareToBaseline
#ifdef CompareToBaseline
    LOG.tracev (L"g format string count %d", (int)strings.size ());
    if (Check::Size (s_baseline.size (), strings.size (), "formatted strings counts should match"))
        {
        size_t numError = 0;
        for (size_t i = 0; i < strings.size (); i++)
            if (s_baseline[i] != strings[i])
                {
                numError++;
                printf ("g format string mismatch:\n(baseline %s)\n(runtime  %s)\n", s_baseline[i].c_str(), strings[i].c_str());
                LOG.tracev (L"g format string mismatch:\n(baseline %s)\n(runtime  %s)\n", s_baseline[i].c_str(), strings[i].c_str());
                }
#if EmitErrors
        Check::Size (0, numError, "Formatted string matching");
#endif
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d,MaxDistanceFromRay)
    {
    double rMax = 78;
    double theta = 0.92;
    DPoint3d disconnect;
    disconnect.InitDisconnect ();
    bvector<DPoint3d> pointA
        {
        DPoint3d::From (2,3,9),
        disconnect,
        DPoint3d::From (4,3,2),
        DPoint3d::From (rMax * cos(theta), rMax * sin(theta),0),           // And we know that this is the farthest point from Z, and it is at index0==3 ..
        DPoint3d::From (4,3,5)
        };
    size_t index0 = 3;
    double dMaxA, dMaxB;
    size_t indexA, indexB;
    DRay3d rayA = DRay3d::FromOriginAndVector (DPoint3d::From(0,0,0),  DVec3d::From (0,0,1));
    if (Check::True (DPoint3dOps::MaxDistanceFromUnboundedRay (pointA, rayA, indexA, dMaxA))
            && Check::Near (rMax, dMaxA)
            && Check::Size (index0, indexA)
            )
        {
        auto transform = Transform::FromLineAndRotationAngle (DPoint3d::From (1,5,9), DPoint3d::From (1,2,3), 0.4);
        bvector<DPoint3d> pointB;
        transform.Multiply (pointB, pointA);
        DRay3d rayB;
        transform.Multiply (rayB, rayA);
        if (Check::True (DPoint3dOps::MaxDistanceFromUnboundedRay (pointB, rayB, indexB, dMaxB)))
            {
            Check::Size (indexA, indexB);
            Check::Near (dMaxA, dMaxB);
            }
        }
    }


#define MGeometryPort_not
#ifdef MGeometryPort
//! @return true if a is "to the left of, or below with equal x"
static bool DPoint2d__LexicalXYLessThan (DPoint2d const &a, DPoint2d const &b)
    {
    if (a.x < b.x)
        return true;
    if (a.x > b.x)
        return false;
    return a.y < b.y;
    }
#define CrossProductToTargets CrossProductToPoints
//! methods treating an array of DPoint2d as counterclockwise order around a convex polygon
struct MConvexPolygon2d
{
private:
// hull points in CCW order, WITHOUT final duplicate . ..
bvector<DPoint2d> m_hullPoints;
public: 
// Create the hull.
MConvexPolygon2d (bvector<DPoint2d> &points)
    {
    ComputeConvexHull(points, m_hullPoints);
    }

// Return a copy of the hull points.
bvector<DPoint2d> Points () {return m_hullPoints;}

// test if the hull points are a convex, CCW polygon
bool IsValidConvexHull () const
    {
    if (m_hullPoints.size () < 3)
        return false;
    size_t n = m_hullPoints.size ();
    for (size_t i = 0; i < m_hullPoints.size (); i++)
        {
        size_t i1 = (i + 1) % n;
        size_t i2 = (i + 2) % n;
        if (m_hullPoints[i].CrossProductToTargets (m_hullPoints[i1], m_hullPoints[i2]) < 0.0)
            return false;
        }
    return true;
    }

//! @return true if the convex hull (to the left of edges) contains the test point.
bool ContainsPoint
(
DPoint2d const &point           //!< [in] point to test.
) const
    {
    DPoint2d xy0 = m_hullPoints.back ();
    //double tol = -1.0e-20;  // negative tol!!
    for (size_t i = 0; i < m_hullPoints.size (); i++)
        {
        DPoint2d xy1 = m_hullPoints[i];
        double c = xy0.CrossProductToTargets(xy1, point);
        if (c < 0.0)
            return false;
        xy0 = xy1;
        }
    return true;
    }

//! Offset the entire hull (in place) by distance.
void OffsetInPlace (double distance)
    {
    size_t n = m_hullPoints.size ();
    if (n >= 3)
        {
        DVec2d edgeA, perpA, edgeB, perpB;
        edgeA = DVec2d (m_hullPoints.back (), m_hullPoints.front ());
        edgeA.NormalizeInPlace ();
        perpA = edgeA.CWPerpendicularVector();
        DPoint2d hullPoint0 = m_hullPoints[0];
        for (size_t i = 0; i < m_hullPoints.size (); i++, perpA = perpB)
            {
            size_t j = i + 1;
            edgeB = DVec2d(m_hullPoints[i],
                    j < n ? m_hullPoints[j] : hullPoint0);
            edgeB.NormalizeInPlace();
            perpB = edgeB.CWPerpendicularVector();
            m_hullPoints[i] = m_hullPoints[i] + DVec2d::OffsetBisector (perpA, perpB, distance);
            }
        }
    }

// Return 2 distances bounding the intersection of the ray with a convex hull.
// ASSUME (for tolerancing) the ray has normalized direction vector.
DRange1d ClipRay
(
MRay2d const &ray             //!< [in] ray to clip.  Both negative and positive distances along the ray are possible.
)
    {
    double distanceA = -DBL_MAX;
    double distanceB = DBL_MAX;
    
    if (m_hullPoints.size () < 3)
        return MRange1d ();
    DPoint2d xy0 = m_hullPoints.back ();
    for (auto xy1 : m_hullPoints)
        {
        double distance, dhds;
        if (ray.IntersectUnboundedLine (xy0, xy1, distance, dhds))
            {
            if (dhds > 0.0)
                {
                if (distance < distanceB)
                    distanceB = distance;
                }
            else
                {
                if (distance > distanceA)
                    distanceA = distance;
                }
            if (distanceA > distanceB)
                return MRange1d ();
            }
        else
            {
            // ray is parallel to the edge.
            // Any single point out classifies it all . ..
            if (xy0.CrossProductToTargets (xy1, ray.m_origin) < 0.0)
                return MRange1d ();
            }
        xy0 = xy1;
        }
    MRange1d range;
    range.Extend (distanceA);
    range.Extend (distanceB);
    return range;
    }

//! Return the range of (fractional) ray postions for projections of all points from the arrays.
MRange1d RangeAlongRay
(
MRay2d const &ray
)
    {
    MRange1d range;
    for (auto xy1 : m_hullPoints)
        range.Extend (ray.ProjectionFraction (xy1));
    return range;
    }

//! Return the range of (fractional) ray postions for projections of all points from the arrays.
MRange1d RangePerpendicularToRay
(
MRay2d const &ray
)
    {
    MRange1d range;
    for (auto xy1 : m_hullPoints)
        range.Extend (ray.PerpendicularProjectionFraction (xy1));
    return range;
    }


static bool ComputeConvexHull(bvector<DPoint2d> const &xy, bvector<DPoint2d> &hull)
    {
    hull.clear();
    size_t n = xy.size();
    if (n < 3)
        return false;
    bvector<DPoint2d> xy1 = xy;
    std::sort(xy1.begin(), xy1.end(), DPoint2d__LexicalXYLessThan);
    hull.push_back(xy1[0]);    // This is sure to stay
    hull.push_back(xy1[1]);    // This one can be removed in the loop.
                               // first sweep creates upper hull . .. 

    for (size_t i = 2; i < n; i++)
        {
        DPoint2d candidate = xy1[i];
        size_t top = hull.size() - 1;
        while (top > 0 && hull[top - 1].CrossProductToTargets(hull[top], candidate) <= 0.0)
            {
            top--;
            hull.pop_back();
            }
        hull.push_back(candidate);
        }

    // second creates lower hull right to left
    size_t i0 = hull.size() - 1;
    // xy1.back () is already on stack.
    hull.push_back(xy1[n - 2]);
    for (size_t i = n - 2; i-- > 0;)
        {
        DPoint2d candidate = xy1[i];
        size_t top = hull.size() - 1;
        while (top > i0 && hull[top - 1].CrossProductToTargets(hull[top], candidate) <= 0.0)
            {
            top--;
            hull.pop_back();
            }
        if (i > 0)  // don't replicate start point !!!
            hull.push_back(candidate);
        }
    return true;
    }
};

#endif
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint3d, AngleXY)
    {
    DPoint3d point3D = DPoint3d::From(4, 5, 6);
    Check::Near(0.89605538457, point3D.AngleXY());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint3d, Swap)
    {
    DPoint3d point3D = DPoint3d::From(4, 5, 6);
    DPoint3d point3D_2 = DPoint3d::From(6, 6, 9);
    point3D.Swap(point3D_2);
    Check::ExactDouble(6, point3D.x);
    Check::ExactDouble(6, point3D.y);
    Check::ExactDouble(9, point3D.z);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint3d, SetToZero)
    {
    DPoint3d point;
    point = DPoint3d::From(2.3, 4.2, 4.4);
    point.Zero ();
    Check::ExactDouble(0, point.x);
    Check::ExactDouble(0, point.y);
    Check::ExactDouble(0, point.z);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint3d, MinAbs)
    {
    DPoint3d point;
    point = DPoint3d::From(-2.334, 1.234, 4.43);    
    Check::ExactDouble(1.234, point.MinAbs());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DPoint3d, MaxAbsIndex)
    {
    DPoint3d point;
    point = DPoint3d::From(-2.334, 1.234, 4.43);    
    Check::ExactDouble(2, point.MaxAbsIndex());
    point = DPoint3d::From(-2.334, -1.234, -4.43);    
    Check::ExactDouble(2, point.MaxAbsIndex());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPoint3d, FromRotationAngle)
    {
    DPoint3d pnt3d[] = { DPoint3d::FromProduct(DPoint3d::From(3, 3, 5),
                                           RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(60).Radians()),
                                           DVec3d::From(4, 4, 4)) };
    
    DPoint3d::AddToArray(pnt3d, 1, DPoint3d::From(2, 2, 3));

    DPoint3d expectedPnt = DPoint3d::From(9, 3.536, 13.464);
    Check::True(expectedPnt.AlmostEqualXY(pnt3d[0], 0.0005));
    
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPoint3d, Rotate360)
    {
    bvector<DPoint3d> pnt3d = { DPoint3d::From(2, 2, 0),
                                DPoint3d::From(0.2, 0.2, 0),
                                DPoint3d::From(0.11, 1, 0) };
    bvector<DPoint3d> pnt3dExp = pnt3d;
    pnt3d[0].RotateXY(Angle::FromDegrees(360).Radians());
    pnt3d[1].RotateXY(Angle::FromDegrees(360).Radians());
    pnt3d[2].RotateXY(Angle::FromDegrees(360).Radians());

    Check::True(DPoint3d::AlmostEqualXY(pnt3d, pnt3dExp));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPoint3d, ToleranceScale)
    {
    DPoint3d pnt1 = DPoint3d::From(4, 5, 2);
    DPoint3d pnt2 = DPoint3d::From(2, 2, 3);
    DPoint3d point3[2] = { DPoint3d::From(4, 5, 2), DPoint3d::From(2, 2, 3) };
    DPoint3d point, pointOrginal;

    double tol = 0.025;
    point.AddToArray(point3, 2, DPoint3d::From(tol, tol, tol));
    pointOrginal.DifferenceOf(pnt2, pnt1);
    point.DifferenceOf(point3[1], point3[0]);
    Check::Near(point, pointOrginal);

    bvector<DPoint3d> pntLeft = { pnt1, pnt2 };
    bvector<DPoint3d> pntRight = { point3[0], point3[1] };

    DPoint3d::AlmostEqualXY(pntLeft, pntRight, tol);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DPoint3d, OperatorOverload)
    {
    DPoint3d point = DPoint3d::From(0.1, 0.33, 0.11);
    DVec3d vec = DVec3d::From(3, 2.2, 6.001);
    DPoint3d pointCpy = point;
    point += vec;
    pointCpy.Add(vec);
    Check::True(point == pointCpy);
    point -= vec;
    pointCpy.Subtract(vec);
    Check::True(point == pointCpy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, CopyArrayOf3dPoints) 
    {
    bvector<DPoint3d> source = { DPoint3d::From(3,2,2),
                                 DPoint3d::From(3,0,1),
                                 DPoint3d::From(9,2,11) };
    bvector<DPoint3d> dest;
    DPoint3dOps::Copy(&dest, &source);
    Check::Near( dest[0], DPoint3d::From(3,2,2));
    Check::Near( dest[1], DPoint3d::From(3,0,1));
    Check::Near( dest[2], DPoint3d::From(9,2,11));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, LengthOfPolyline)
    {
    bvector<DPoint3d> source = { DPoint3d::From(0,0,0),
                                 DPoint3d::From(0,0,5),
                                 DPoint3d::From(0,6,5), };
    double length = PolylineOps::Length(source);
    printf("%f\n", length);
    Check::ExactDouble(length, 11);

    bvector<DPoint3d> sourceClosed = { DPoint3d::From(0,0,0),
                                       DPoint3d::From(4,0,0),
                                       DPoint3d::From(4,4,0),
                                       DPoint3d::From(0,4,0)}; //box
    
    length = PolylineOps::Length(sourceClosed, true);
    printf("%f\n", length);
    Check::ExactDouble(length, 16);
    bvector<DPoint3d> sourceClosed2 = { DPoint3d::From(0,0,0),
                                        DPoint3d::From(4,0,4),
                                        DPoint3d::From(4,4,4),
                                        DPoint3d::From(0,0,4)};
    length = PolylineOps::Length(sourceClosed2, false);
    printf("%0.18f    %0.18f\n", length, 4+2*sqrt(32));
    Check::Near(length, 4+2*sqrt(32));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, ConvexPolygon)
    {
    bvector<DPoint3d> pointsPolygon = { DPoint3d::From(5,1),
                                        DPoint3d::From(7,1),
                                        DPoint3d::From(8,3),
                                        DPoint3d::From(6.8,9),
                                        DPoint3d::From(3,3) };
    Check::True(PolygonOps::IsConvex(pointsPolygon));
    pointsPolygon[2] = DPoint3d::From(6.9, 3);
    Check::False(PolygonOps::IsConvex(pointsPolygon));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, NormalArea)
    {
    //planar polygon
    bvector<DPoint3d> pointsPolygon = { DPoint3d::From(2,1),
                                        DPoint3d::From(8,1),
                                        DPoint3d::From(8,8) };
    DVec3d areaNormal = PolygonOps::AreaNormal(pointsPolygon);
    Check::Parallel(areaNormal, DVec3d::From(0, 0, 1));
    Check::Near(21, areaNormal.Magnitude());

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d, CentroidOfPolygon)
    {
    bvector<DPoint2d> pointsPolygon = { DPoint2d::From(0,0),
                                        DPoint2d::From(0,4),
                                        DPoint2d::From(4,4),
                                        DPoint2d::From(4,0) };
    DPoint2d centroid;
    double area;
    PolygonOps::CentroidAndArea(pointsPolygon, centroid, area);
    Check::Near(centroid, DPoint2d::From(2, 2));
    Check::Near(area, PolygonOps::Area(pointsPolygon));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d,FromIntersectUnitPerpendicularsXY)
    {
    auto basePoint = DPoint3d::From (1,2,4);
    auto targetA   = DPoint3d::From (2,5,2);
    auto targetB   = DPoint3d::From (-2, 6, 2);
    bvector<double> fractions {0,1,0.2, 0.5};
    for (double fA : fractions)
        {
        Check::StartScope ("fA", fA);
        for (double fB : fractions)
            {
            Check::StartScope ("fB", fB);
            auto xyz = DPoint3d::FromIntersectPerpendicularsXY (basePoint, targetA, fA, targetB, fB);
            if (Check::True (xyz.IsValid (), "FromIntersectUnitPerpendicularsXY"))
                {
                auto xyzA = DPoint3d::FromInterpolate (basePoint, fA, targetA);
                auto xyzB = DPoint3d::FromInterpolate (basePoint, fB, targetB);
                Check::Near (1.0, 1.0 + xyzA.DotProductToPointsXY (basePoint, xyz), "right angle on line A as viewed");
                Check::Near (1.0, 1.0 + xyzB.DotProductToPointsXY (basePoint, xyz), "right angle on line B as viewed");
                Check::ExactDouble (basePoint.z, xyz.Value ().z, "Copy z from base point");
                }
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    auto targetA2 = DPoint3d::FromInterpolate (basePoint, 2.7, targetA);        // on line beyond targetA
    auto failureCase = DPoint3d::FromIntersectPerpendicularsXY (basePoint, targetA, 0.2, targetA2, 1.0);
    Check::False (failureCase.IsValid (), "Parallel lines do not intersect");
    Check::Near (basePoint, failureCase, "Parallel case returns base point");
    }
template <typename TScalar, typename TPoint>
void TestAlmostEqual (bvector<TPoint> &stroke0)
    {
    auto stroke1 = stroke0;
    double fluff = 0.0001;      // big numbers to deal with float !!!
    double tol = fluff * 0.1;
    Check::True (TPoint::AlmostEqual (stroke0, stroke1, tol));
    Check::True (TPoint::AlmostEqualXY (stroke0, stroke1, tol));
    stroke1.pop_back();
    Check::False (TPoint::AlmostEqual (stroke0, stroke1, tol));
    Check::False (TPoint::AlmostEqual (stroke1, stroke0, tol));

    Check::False (TPoint::AlmostEqualXY (stroke0, stroke1, tol));
    Check::False (TPoint::AlmostEqualXY (stroke1, stroke0, tol));


    stroke0.pop_back ();
    Check::True (TPoint::AlmostEqual (stroke0, stroke1, tol));
    Check::True (TPoint::AlmostEqualXY (stroke0, stroke1, tol));
    TScalar a = (TScalar)(fluff * 0.5);
    for (size_t i = 0; i < stroke0.size (); i++)
        {
        auto save = stroke0[i];

        stroke0[i].x += a;
        Check::False (TPoint::AlmostEqual (stroke0, stroke1, tol));
        Check::True (TPoint::AlmostEqual (stroke0, stroke1, fluff));

        Check::False (TPoint::AlmostEqualXY (stroke0, stroke1, tol));
        Check::True (TPoint::AlmostEqualXY (stroke0, stroke1, fluff));

        stroke0[i] = save;

        stroke0[i].y += a;
        Check::False (TPoint::AlmostEqual (stroke0, stroke1, tol));
        Check::True (TPoint::AlmostEqual (stroke0, stroke1, fluff));

        Check::False (TPoint::AlmostEqualXY (stroke0, stroke1, tol));
        Check::True (TPoint::AlmostEqualXY (stroke0, stroke1, fluff));
        stroke0[i] = save;

        stroke0[i].z += a;
        Check::False (TPoint::AlmostEqual (stroke0, stroke1, tol));
        Check::True (TPoint::AlmostEqual (stroke0, stroke1, fluff));

        Check::True (TPoint::AlmostEqualXY (stroke0, stroke1, tol));
        Check::True (TPoint::AlmostEqualXY (stroke0, stroke1, fluff));

        stroke0[i] = save;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DPoint3d,AlmostEqualBvector)
    {
    auto arc = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (1,2,3), 2.5);
    size_t numPoint = 5;
    DPoint3dDoubleArrays stroke0 (arc, numPoint);
    bvector<FPoint3d> stroke1;
    for (auto xyz : stroke0.m_xyz)
        stroke1.push_back (FPoint3d::From (xyz));


    TestAlmostEqual <double, DPoint3d> (stroke0.m_xyz);
    TestAlmostEqual <float, FPoint3d>(stroke1);
    }

