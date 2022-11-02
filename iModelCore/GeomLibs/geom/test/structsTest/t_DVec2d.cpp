/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Initialization)
    {
    DVec2d vector0 = DVec2d::From (1.0, 2.0);
    DVec2d vector1;
    vector1.Init (1.0, 2.0);
    Check::Near (vector0.x, 1.0);
    Check::Near (vector0.y, 2.0);
    Check::Near (vector0, vector1);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST(DPoint2d, Initialization)
	//{
	//DPoint2d point0 = DPoint2d::From (1.0, 2.0);
	//DPoint2d point1;
//point1.InitFrom (

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, CrossProduct)
    {
    DVec2d vector0 = DVec2d::From (1.0, 2.0);
	DVec2d vector1 = DVec2d::From (3.0, 4.0);
	DPoint2d point0 = DPoint2d::From (1.0, 2.0);
	DPoint2d point1 = DPoint2d::From (3.0, 4.0);
	double vectorCrossProductCodedMethod = vector0.CrossProduct (vector1);
	double vectorCrossProductAlternateMethod = vector0.Magnitude () * vector1.Magnitude () * sin (vector0.AngleTo(vector1));
	double vectorCrossProductSquaredCodedMethod = vector0.CrossProductSquared (vector1);
	double vectorDotProductCodedMethod = vector0.DotProduct (vector1);
	double vectorDotProductAlternateMethod = vector0.Magnitude () * vector1.Magnitude () * cos (vector0.AngleTo(vector1));
	double pointDotProductCodedMethod = point0.DotProduct (point1);
	double pointDotProductAlternateMethod = point0.Magnitude () * point1.Magnitude () * cos (point0.AngleTo(point1));
	double vectorDotProductPassedWithValuesCodedMethod = vector0.DotProduct (3.0, 4.0);
	double vectorDotProductPassedWithValuesAlternateMethod = vector0.Magnitude () * vector1.Magnitude () * cos (vector0.AngleTo(vector1));
	
	Check::Near (vectorCrossProductCodedMethod, vectorCrossProductAlternateMethod);
	Check::Near (vectorCrossProductCodedMethod * vectorCrossProductCodedMethod, vectorCrossProductSquaredCodedMethod);
	Check::Near (vectorDotProductCodedMethod, vectorDotProductAlternateMethod);
	Check::Near (pointDotProductCodedMethod, pointDotProductAlternateMethod);
	Check::Near (vectorDotProductPassedWithValuesCodedMethod, vectorDotProductPassedWithValuesAlternateMethod);

	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, NormalizedDifference)
	{
	DVec2d vector0 = DVec2d::From (1.0, 2.0);
	DVec2d vector1 = DVec2d::From (3.0, 4.0);
	DVec2d vector2 = DVec2d::From (5.0, 6.0);
	DPoint2d point0 = DPoint2d::From (5.0, 6.0);
	DPoint2d point1 = DPoint2d::From (0.0, 0.0);
	vector0.Normalize ();
	double vector0NormalizedMagnitude = vector0.Magnitude ();
	DVec2d vector0PreNormalized = DVec2d::From (1.0, 2.0);
	double angleBetweenPreNormalizedVectorAndNormalizedVector = vector0PreNormalized.AngleTo (vector0);
	vector1.NormalizedDifference (point0, point1);
	//DVec2d vector1PreNormalized = DVec2d::From (3.0, 4.0);
	double vector1NormalizedMagnitude = vector1.Magnitude ();
	vector2.Normalize ();
	
	Check::Near (1.0, vector0NormalizedMagnitude);
	Check::Near (0.0, angleBetweenPreNormalizedVectorAndNormalizedVector);
	//Check::Near (1.0, DVec2d::From (5.0, 7.0).Normalize ());
	Check::Near (1.0, vector1NormalizedMagnitude);
	Check::Near (vector1, vector2);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, AngleTo)
	{
	DVec2d vector0 = DVec2d::From (1.0, 0.0);
	DVec2d vector1 = DVec2d::From (1.0, 1.0);
	DVec2d vector2 = DVec2d::From (0.0, 1.0);
	DVec2d vector3 = DVec2d::From (2.3, 4.5);
	DVec2d vector4 = DVec2d::From (2.3, 4.5);
	DVec2d vector5 = DVec2d::From (-1.0, 1.0);
	double angleFromVec0ToVec1 = vector0.AngleTo (vector1);
	double angleFromVec0ToVec0 = vector0.AngleTo (vector0);
	double angleFromVec0ToVec2 = vector0.AngleTo (vector2);
	double angleFromVec2ToVec0 = vector2.AngleTo (vector0);
	vector3.RotateCCW (0.67);
	double angleFromVec4ToVec3 = vector4.AngleTo (vector3);
	double smallAngleFromVec0ToVec5 = vector0.SmallerUnorientedAngleTo (vector5);
	double smallAngleFromVec4ToVec3 = vector4.SmallerUnorientedAngleTo (vector3);
	Check::Near (PI/4, angleFromVec0ToVec1);
	Check::Near (0.0, angleFromVec0ToVec0);
	Check::Near (PI/2, angleFromVec0ToVec2);
	Check::Near (-PI/2, angleFromVec2ToVec0);
	Check::Near (0.67, angleFromVec4ToVec3);
	Check::Near (PI/4, smallAngleFromVec0ToVec5);
	Check::Near (0.67, smallAngleFromVec4ToVec3);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, AngleInSector)
	{
	DVec2d vector0 = DVec2d::From (1.0, 0.0);
	DVec2d vector1 = DVec2d::From (0.0, 1.0);
	DVec2d vector2 = DVec2d::From (0.5, 0.5);
	DVec2d vector3 = DVec2d::From (-0.5, 0.5);
	DVec2d vector4 = DVec2d::From (-0.5, -0.5);
	DVec2d vector5 = DVec2d::From (0.5, -0.5);
	Check::True(vector2.IsVectorInSmallerSector (vector0, vector1));
	Check::True(!(vector3.IsVectorInSmallerSector (vector0, vector1)));
	Check::True(!(vector4.IsVectorInSmallerSector (vector0, vector1)));
	Check::True(!(vector5.IsVectorInSmallerSector (vector0, vector1)));
	

    // This loop tests:
    // vector.RotateCCW (baseVector, angle)
    // vector.RotateCCW ()
    // vector.AngleTo ()
    // vector.IsVectorInCCWSector ();
    // vector.IsVectorInSmallerSector ();
    DVec2d baseVector, endVector, testVector, checkVector;
    for (double baseAngle = 0.1; baseAngle < Angle::TwoPi (); baseAngle += 1.0)
		{
		baseVector = DVec2d::From (cos (baseAngle), sin (baseAngle));
        for (double sweepAngle = 0.42; sweepAngle < Angle::TwoPi (); sweepAngle += 1.0)
            {
            endVector.RotateCCW (baseVector, sweepAngle);
            // Check that inplace rotation gives same results...
            checkVector = baseVector;
            checkVector.RotateCCW (sweepAngle);
            Check::Near (endVector, checkVector);
            checkVector.RotateCCW (-sweepAngle);
            Check::Near (baseVector, checkVector);
            double sweepAngle1 = baseVector.AngleTo (endVector);
            Check::True (Angle::NearlyEqualAllowPeriodShift (sweepAngle, sweepAngle1));
            Check::Bool (sweepAngle < Angle::Pi (), Angle::NearlyEqual (sweepAngle, sweepAngle1));

            for (double testAngle = 0.12348; testAngle < Angle::TwoPi (); testAngle += 0.041)
                {
                testVector.RotateCCW (baseVector, testAngle);
                //bool inCCW = testAngle < sweepAngle;
                if (testAngle < sweepAngle)
                    Check::True (testVector.IsVectorInCCWSector (baseVector, endVector));
                else
                    Check::False (testVector.IsVectorInCCWSector (baseVector, endVector));
                    
                // CCWSector test flips with order....
                Check::True (testVector.IsVectorInCCWSector (baseVector, endVector) !=
                            testVector.IsVectorInCCWSector (endVector, baseVector)
                            );
                // SmallerSector is unaffected by order
                Check::Bool (testVector.IsVectorInSmallerSector (baseVector, endVector),
                            testVector.IsVectorInSmallerSector (endVector, baseVector));

                if (sweepAngle < Angle::Pi ())
                    {
                    Check::Bool (testAngle < sweepAngle, testVector.IsVectorInSmallerSector (baseVector, endVector));
                    }
                else
                    {
                    Check::Bool (testAngle > sweepAngle, testVector.IsVectorInSmallerSector (baseVector, endVector));
                    }
                }
            }
        }


	/*for (double angle1 = PI/3; angle1 < 2*PI; angle1 += PI/3)
		{
		DVec2d vector6 = DVec2d::From (1.0, 0.0);
		DVec2d vector7 = DVec2d::From (1.0, 0.0);
		vector6.RotateCCW (angle1);
		for (double angle2 = PI/6; angle2 < 2*PI; angle2 += PI/3)
			{
			DVec2d testVector = DVec2d::From (1.0, 0.0);
			testVector.RotateCCW (angle2);
			if (angle1 > angle2)
				{
				Check::True (testVector.IsVectorInSmallerSector (vector7, vector6));
				Check::True (!(testVector.IsVectorInCCWSector (vector7, vector6)));
				}
			else
				{
				Check::True (!(testVector.IsVectorInSmallerSector (vector7, vector6)));
				Check::True (testVector.IsVectorInCCWSector (vector7, vector6));
				}
			}
		}*/
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, RotateCCW)
	{
	DVec2d baseVector = DVec2d::From (1.0, 0.0);
	DVec2d rotVector = DVec2d::From (2.0, 3.0);
	double theta = 0.5;
	rotVector.RotateCCW (baseVector, theta);
	double angleFromBaseVecToRotVec = baseVector.AngleTo (rotVector);
	Check::Near (theta, angleFromBaseVecToRotVec);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, MagnitudeSquared)
	{
	DVec2d vector0 = DVec2d::From (3.0, 5.0);
	double magnitude = vector0.Magnitude ();
	double magnitudeSquared = vector0.MagnitudeSquared ();
	Check::Near (magnitude * magnitude, magnitudeSquared);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, UnitPerpendicular)
	{
	DVec2d vector0 = DVec2d::From (2.5, 7.9);
	DVec2d vector1 = DVec2d::From (3.4, 6.8);
	vector0.UnitPerpendicular (vector1);
	double angleFromVec1ToVec0 = vector1.AngleTo (vector0);
	double magnitudeVec0 = vector0.Magnitude ();
	Check::Near (PI/2, angleFromVec1ToVec0);
	Check::Near (1.0, magnitudeVec0);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Magnitude)
	{
	DVec2d vector0 = DVec2d::From (1.0, 0.0);
	DVec2d vector1 = DVec2d::From (2.5, 3.0);
	DVec2d vector2 = DVec2d::From (0.0, 0.0);
	double magnitudeVec0 = vector0.Magnitude ();
	double magnitudeVec1 = vector1.Magnitude ();
	double magnitudeVec2 = vector2.Magnitude ();
	Check::Near (1.0, magnitudeVec0);
	Check::Near (sqrt (15.25), magnitudeVec1);
	Check::Near (0.0, magnitudeVec2);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Scale)
	{
	DVec2d vector0 = DVec2d::From (1.0, 0.0);
	DVec2d vector1 = DVec2d::From (1.0, 1.0);
	DVec2d vector2 = DVec2d::From (5.0, 5.0);
	double scale = 5.0;
	vector0.Scale (vector1, scale);
	vector1.Scale (scale);
	Check::Near (vector0, vector2);
	Check::Near (vector1, vector2);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Negate)
	{
	DVec2d vector0 = DVec2d::From (1.0, 0.0);
	DVec2d vector1 = DVec2d::From (1.0, 1.0);
	DVec2d vector2 = DVec2d::From (-1.0, -1.0);
	vector0.Negate (vector1);
	vector1.Negate ();
	Check::Near (vector0, vector2);
	Check::Near (vector1, vector2);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Normalize)
	{
	DVec2d vector1 = DVec2d::From (13.8, 5.7);
	DVec2d vector2 = DVec2d::From (-1.0, -8.0);
	DVec2d vector3 = DVec2d::From (0.0, 0.0);
	DVec2d unit1, unit2, unit3;
	double mag1 = unit1.Normalize (vector1);
	double mag2 = unit2.Normalize (vector2);
	double mag3 = unit3.Normalize (vector3);
	Check::Near (mag1, vector1.Magnitude ());
	Check::Near (mag2, vector2.Magnitude ());
	Check::Near (1.0, unit1.Magnitude ());
	Check::Near (1.0, unit2.Magnitude ());
	Check::Near (1.0, unit3.Magnitude ());      // source was 00, expect 10 return.
	Check::True (unit1.IsParallelTo (vector1));
	Check::True (unit2.IsParallelTo (vector2));
	Check::Near (0.0, mag3);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, ScaleToLength)
	{
	DVec2d vector0 = DVec2d::From (0.0, 0.0);
	DVec2d vector1 = DVec2d::From (0.0, 0.0);
	DVec2d vector2 = DVec2d::From (9.6, 3.4);
	DVec2d vector3 = DVec2d::From (5.0, 0.0);
	DVec2d vector4 = DVec2d::From (3.2, 7.1);
	DVec2d vector5 = DVec2d::From (0.0, 0.0);
	double length = 5.0;
	double magnitudeVec1 = vector1.Magnitude ();
	double returnedLengthVec1 = vector0.ScaleToLength (vector1, length);
	double magnitudeVec0 = vector0.Magnitude ();
	double magnitudeVec4 = vector4.Magnitude ();
	double returnedLengthVec4 = vector2.ScaleToLength (vector4, length);
	double magnitudeVec2 = vector2.Magnitude ();
	double magnitudeVec5 = vector5.Magnitude ();
	double returnedLentghVec5 = vector5.ScaleToLength (length);
	double magnitudeVec5AfterScale = vector5.Magnitude ();
	Check::Near (magnitudeVec1, returnedLengthVec1);
	Check::Near (vector3, vector0);
	Check::Near (5.0, magnitudeVec0);
	Check::Near (magnitudeVec4, returnedLengthVec4);
	Check::Near (5.0, magnitudeVec2);
	Check::Near (magnitudeVec5, returnedLentghVec5);
	Check::Near (5.0, magnitudeVec5AfterScale);
	Check::Near (vector3, vector5);
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, IsParallelTo)
	{
	DVec2d vector0 = DVec2d::From (0.0, 1.0);
	DVec2d vector1 = DVec2d::From (0.0, -5.0);
	DVec2d vector2 = DVec2d::From (1.0, 0.0);
	DVec2d vector3 = DVec2d::From (8.0, 0.0);
	DVec2d vector4 = DVec2d::From (2.5, 2.5);
	DVec2d vector5 = DVec2d::From (7.5, 7.5);
	Check::True (vector0.IsParallelTo (vector1));
	Check::True (!(vector0.IsParallelTo (vector2)));
	Check::True (vector2.IsParallelTo (vector3));
	Check::True (!(vector2.IsParallelTo (vector1)));
	Check::True (vector4.IsParallelTo (vector5));
	Check::True (!(vector4.IsParallelTo (vector3)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, IsPositiveParallelTo)
    {
	DVec2d vector0 = DVec2d::From (0.2, 1.0);
	DVec2d vector1 = DVec2d::From (0.0, -5.0);

    DVec2d reverse1 = DVec2d::FromScale (vector1, -1.0);
    Check::True (vector1.IsParallelTo (vector1));
    Check::True (vector1.IsParallelTo (reverse1));
    Check::False (vector1.IsParallelTo (vector0));

    Check::True (vector1.IsPositiveParallelTo (vector1));
    Check::False (vector1.IsPositiveParallelTo (reverse1));
    Check::False (vector1.IsPositiveParallelTo (vector0));
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, IsPerpendicularTo)
    {
    DVec2d vector0 = DVec2d::From (1.0, 0.0);
    DVec2d vector1 = DVec2d::From (0.0, 1.0);
    DVec2d vector2 = DVec2d::From (3.4, 5.6);
    Check::True (vector0.IsPerpendicularTo (vector1));
    Check::True (!(vector0.IsPerpendicularTo (vector2)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, SafeDivide)
    {
    DVec2d vector0 = DVec2d::From (0.0, 0.0);
    DVec2d vector1 = DVec2d::From (1.0, 1.0);
    DVec2d vector2 = DVec2d::From (0.5, 0.5);
    double denominator0 = 0.0;
    double denominator1 = 2.0;
    double denominator2 = 0.000000000001;
    Check::True (!(vector0.SafeDivide (vector1, denominator0)));
    Check::Near (vector0, vector1);
    Check::True (vector0.SafeDivide (vector1, denominator1));
    Check::Near (vector2, vector0);
    Check::True (!(vector0.SafeDivide (vector1, denominator2)));
    Check::Near (vector0, vector1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, MaxAbs)
    {
    DVec2d vector0 = DVec2d::From (2.0, -7.8);
    DVec2d vector1 = DVec2d::From (2.0, 7.8);
    DVec2d vector2 = DVec2d::From (0.0, 0.0);
    double maxValVec0 = vector0.MaxAbs ();
    double maxValVec1 = vector1.MaxAbs ();
    double maxValVec2 = vector2.MaxAbs ();
    Check::Near (7.8, maxValVec0);
    Check::Near (7.8, maxValVec1);
    Check::Near (0.0, maxValVec2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Zero)
    {
    DVec2d vector0 = DVec2d::From (2.0, -7.8);
    DVec2d vector1 = DVec2d::From (20.0, 34.8);
    DVec2d vector2 = DVec2d::From (0.0, 0.0);
    DVec2d vector3 = DVec2d::From (1.0, 1.0);
    vector0.Zero ();
    vector1.Zero ();
    Check::Near (vector2, vector0);
    Check::Near (vector2, vector1);
    vector0.One ();
    vector1.One ();
    vector2.One ();
    Check::Near (vector3, vector0);
    Check::Near (vector3, vector1);
    Check::Near (vector3, vector2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Init)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (1.0, 2.0);
    DVec2d vector2 = DVec2d::From (4.0, 9.0);
    DVec2d vector3 = DVec2d::From (3.0, 5.6);
    DPoint2d point0 = DPoint2d::From (1.0, 2.0);
    double ax = 3.0;
    double ay = 5.6;
    vector0.Init (point0);
    vector2.Init (ax, ay);
    Check::Near (vector1, vector0);
    Check::Near (vector3, vector2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, XyOf)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (1.0, 2.0);
    DPoint4d point0 = DPoint4d::From (1.0, 2.0, 3.0, 4.0);
    vector0.XyOf (point0);
    Check::Near (vector1, vector0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, SetComponent)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (8.0, 5.0);
    DVec2d vector2 = DVec2d::From (8.0, 8.0);
    double component0 = 8.0;
    int index0 = 0;
    int index1 = 1;
    vector0.SetComponent (component0, index0);
    Check::Near (vector1, vector0);
    vector0.SetComponent (component0, index1);
    Check::Near (vector2, vector0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, GetComponent)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (4.0, 5.0);
    int index0 = 0;
    int index1 = 1;
    int index2 = 2;
    int index3 = 3;
    double xCoord = 7.8;
    double yCoord = 9.2;
    double returnedComponentX0 = vector0.GetComponent (index0);
    double returnedComponentY0 = vector0.GetComponent (index1);
    double returnedComponentX1 = vector0.GetComponent (index2);
    double returnedComponentY1 = vector0.GetComponent (index3);
    vector1.GetComponents (xCoord, yCoord);
    Check::Near (3.0, returnedComponentX0);
    Check::Near (5.0, returnedComponentY0);
    Check::Near (3.0, returnedComponentX1);
    Check::Near (5.0, returnedComponentY1);
    Check::Near (4.0, xCoord);
    Check::Near (5.0, yCoord);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, SumOf)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (4.0, 5.0);
    DVec2d vector2 = DVec2d::From (7.7, 5.3);
    DVec2d vector3 = DVec2d::From (11.7, 10.3);
    DVec2d vector4 = DVec2d::From (45.67, 36.89);
    DVec2d vector5 = DVec2d::From (53.37, 42.19);
    DVec2d vector6 = DVec2d::From (99.04, 79.08);
    DVec2d vector7 = DVec2d::From (266.85, 210.95);
    DVec2d vector8 = DVec2d::From (268.85, 218.95);
    DVec2d vector9 = DVec2d::From (10.3, 8.2);
    DVec2d vector10 = DVec2d::From (29.1, 5.2);
    DVec2d vector11 = DVec2d::From (175.72, 70.84);
    DVec2d vector12 = DVec2d::From (17.72, 7.84);
    DVec2d vector13 = DVec2d::From (301.532, 126.504);
    DVec2d origin0 = DVec2d::From (0.0, 0.0);
    DVec2d origin1 = DVec2d::From (2.0, 8.0);
    double scale = 5.0;
    double scale1 = 4.2;
    double scale2 = 7.1;
    vector0.SumOf (vector1, vector2);
    Check::Near (vector3, vector0);
    vector1.SumOf (vector2, vector4);
    Check::Near (vector5, vector1);
    vector4.Add (vector5);
    Check::Near (vector6, vector4);
    vector6.SumOf (origin0, vector5, scale);
    Check::Near (vector7, vector6);
    vector6.SumOf (origin1, vector5, scale);
    Check::Near (vector8, vector6);
    vector0.SumOf (origin1, vector9, scale, vector10, scale1);
    Check::Near (vector11, vector0);
    vector0.SumOf (origin1, vector9, scale, vector10, scale1, vector12, scale2);
    Check::Near (vector13, vector0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Subtract)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (4.0, 5.0);
    DVec2d vector2 = DVec2d::From (-1.0, 0.0);
    DVec2d vector3 = DVec2d::From (38.98, 53.23);
    DVec2d vector4 = DVec2d::From (49.01, 95.38);
    DVec2d vector5 = DVec2d::From (-10.03, -42.15);
    vector0.Subtract (vector1);
    vector3.Subtract (vector4);
    Check::Near (vector2, vector0);
    Check::Near (vector5, vector3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Interpolate)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (4.0, 5.0);
    DVec2d vector2 = DVec2d::From (-1.0, 0.0);
    DVec2d vector3 = DVec2d::From (2.5, 3.5);
    double fractionParameter = 0.3;
    vector0.Interpolate (vector1, fractionParameter, vector2);
    Check::Near (vector3, vector0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, DifferenceOf)
    {
    DVec2d vector0 = DVec2d::From (3.0, 5.0);
    DVec2d vector1 = DVec2d::From (4.0, 5.0);
    DVec2d vector2 = DVec2d::From (-1.0, 0.0);
    DVec2d vector3 = DVec2d::From (5.0, 5.0);
    DVec2d vector4 = DVec2d::From (5.9, 15.0);
    DVec2d vector5 = DVec2d::From (-2.0, 1.5);
    DPoint2d point0 = DPoint2d::From (1.0, 6.0);
    DPoint2d point1 = DPoint2d::From (3.0, 4.5);
    vector0.DifferenceOf (vector1, vector2);
    vector4.DifferenceOf (point0, point1);
    Check::Near (vector3, vector0);
    Check::Near (vector5, vector4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, Distance)
    {
    DVec2d vector0 = DVec2d::From (0.0, 0.0);
    DVec2d vector1 = DVec2d::From (4.0, 5.0);
    DVec2d vector2 = DVec2d::From (-1.0, -7.8);
    double distanceBetweenVec0Vec0 = vector0.Distance (vector0);
    double distanceBetweenVec1Vec2 = vector1.Distance (vector2);
    double distanceSquaredBetweenVec1Vec2 = vector1.DistanceSquared (vector2);
    Check::Near (0.0, distanceBetweenVec0Vec0);
    Check::Near (sqrt (188.84), distanceBetweenVec1Vec2);
    Check::Near (188.84, distanceSquaredBetweenVec1Vec2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d, IsEqual)
    {
    DVec2d vector0 = DVec2d::From (4.0, 5.0);
    DVec2d vector1 = DVec2d::From (4.0, 5.0);
    DVec2d vector2 = DVec2d::From (-1.0, -7.8);
    DVec2d vector3 = DVec2d::From (4.0001, 5.00001);
    DVec2d vector4 = DVec2d::From (4.01, 5.01);
    double tolerance0 = 0.0;
    double tolerance1 = 0.001;
    Check::True (vector0.IsEqual (vector1));
    Check::True (!(vector0.IsEqual (vector2)));
    Check::True (vector0.IsEqual (vector1, tolerance0));
    Check::True (vector0.IsEqual (vector3, tolerance1));
    Check::True (!(vector0.IsEqual (vector4, tolerance1)));
    }

#ifdef MixArray__
void mixArray (size_t n, bvector<size_t> &array1)
	{
	array1.resize(n);
	for (size_t i = 0; i < n; i++)
		{
		array1[i] = i;
		}
	for (size_t klow = 1, khigh = n - 1; klow < khigh; klow += 2, khigh--)
		{
		size_t tmpVal = array1[klow];
		array1[klow] = array1[khigh];
		array1[khigh] = tmpVal;
		}
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MixArray, mixArray)
	{
	bvector<size_t> array1;
	mixArray (20, array1);
	size_t array2[20] = {0, 19, 2, 18, 4, 17, 6, 16, 8, 15, 10, 14, 12, 13, 11, 9, 7, 5, 3, 1};
	for(size_t i = 0; i < 20; i++)
		{
		printf(" %d %d %d\n", i, array1[i], array2[i]);
		Check::Size (array1[i], array2[i]);
		}
	//size_t sizeArray = size(array1);
	//size_t array3[sizeArray] = {0};
	size_t array3[20];
	for(size_t l = 0; l < 20; l++)
		{
		array3[l] = 0;
		}
	for(size_t k = 0; k < 20; k++)
		{
		size_t x = array1[k];
		array3[k] = array3[k] += 1;
		printf(" %d %d %d\n", k, array1[k], array3[k]);
		}
	}

#endif


void CheckRootRoundTrip (double a)
    {
    double b = a * a;
    double a1 = sqrt (b);
    DVec2d vector = DVec2d::From (a, 0.0);
    a1 = vector.Magnitude ();
    static double s_relTol = 2.0e-16;   // really tight -- this is system sqrt !!!
    if (!Check::True (fabs (a1-a) < s_relTol * (1.0 + a), "System sqrt"))
        printf ("(a = %20.17le) (sqrt (a*a) = %22.17le) (diff %10.4le)\n", a, a1, a1-a);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(MagnitudeRoundoff, Test0)
    {
    CheckRootRoundTrip (1.0);
    CheckRootRoundTrip (0.0);
    CheckRootRoundTrip (1.0/3.0);
    CheckRootRoundTrip (sqrt (2.0));
    CheckRootRoundTrip (sqrt (15.0));
    CheckRootRoundTrip (pow (5.02342343, 0.2346237467));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec2d,MultipleRotate90)
    {
    DVec2d baseVector = DVec2d::From (3,-9);
    for (int rotateCount = 0; rotateCount < 9; rotateCount++)
        {
        DVec2d rotateA = DVec2d::FromRotate90CCW (baseVector, rotateCount);
        DVec2d rotateB = DVec2d::FromRotate90CCW (baseVector, -rotateCount);
        double thetaA = baseVector.AngleTo (rotateA);
        double thetaB = baseVector.AngleTo (rotateB);
        Check::True (Angle::NearlyEqualAllowPeriodShift (thetaA, rotateCount * Angle::PiOver2 ()));
        Check::True (Angle::NearlyEqualAllowPeriodShift (thetaB, -rotateCount * Angle::PiOver2 ()));
        DVec2d rotateA1 = baseVector;
        DVec2d rotateB1 = rotateB;
        for (int i = 0; i < rotateCount; i++)
            {
            rotateA1 = DVec2d::FromRotate90CCW (rotateA1);
            rotateB1 = DVec2d::FromRotate90CCW (rotateB1);
            }
        Check::Near (rotateA1, rotateA);
        Check::Near (rotateB1, baseVector);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, InitFromSum)
    {
    DVec2d vector0 = DVec2d::From(2, 3);
    DVec2d vector1 = DVec2d::From(4, 5);
    DVec2d vector2 = DVec2d::From(1, 3);
    DVec2d vector3 = DVec2d::From(5, 9);
    double scale1 = 2;
    double scale2 = 1.5;
    double scale3 = 0.25;
    DVec2d res = DVec2d::FromSumOf(vector0, vector1, scale1);
    Check::Near(10, res.GetComponent(0));
    Check::Near(13, res.GetComponent(1));
    res = DVec2d::FromSumOf(vector0, vector1, scale1, vector2, scale2);
    Check::Near(11.5, res.GetComponent(0));
    Check::Near(17.5, res.GetComponent(1));
    res = DVec2d::FromSumOf(vector0, vector1, scale1, vector2, scale2, vector3, scale3);
    Check::Near(12.75, res.GetComponent(0));
    Check::Near(19.75, res.GetComponent(1));
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, FromStartToEnd)
    {
    DPoint3d pointStart = DPoint3d::From(10, 11, 10);
    DPoint3d pointEnd = DPoint3d::From(13, 11.4, 7);
    DVec2d vector = DVec2d::FromStartEnd(pointStart, pointEnd);
    Check::Near(3, vector.GetComponent(0));
    Check::Near(0.4, vector.GetComponent(1));
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, DotProduct)
    {
    DVec2d vec0 = DVec2d::From(2, 3);
    DVec2d vec1 = DVec2d::From(5, 6);
    double res = vec0.DotProduct(vec1);
    double resExpected = vec0.Magnitude() * vec1.Magnitude() * cos(vec0.AngleTo(vec1));
    Check::Near(resExpected, res);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, PointVector)
    {
    DVec2d vec = DVec2d::From(5, 0);
    DPoint2d pnt = DPoint2d::From(0, 5);
    Check::ExactDouble(0, vec.DotProduct(pnt));
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, Interpolation) 
    {
    DVec2d vec0 = DVec2d::From(5, 5);
    DVec2d vec1 = DVec2d::From(7, 7);
    double fraction = 0.5;
    DVec2d interpVector = DVec2d::FromInterpolate(vec0, fraction, vec1);
    Check::Near(DVec2d::From(6, 6), interpVector);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, Resultant)
    {
    DVec3d vec0 = DVec3d::From(3, 7, 3);
    DVec3d vecR = DVec3d::From(8, 9, 9);
    DVec3d vec1 = DVec3d::From(vecR);
    vec1.Subtract(vec0);

    vec0.Negate();
    vec1.Negate();
    vec0.Add(vec1);
    vecR.Negate();
    Check::Near(vecR, vec0);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, ThreeDTo2D) 
    {
    DVec3d vec3d0 = DVec3d::From(5, 8, 2);
    DVec3d vec3d1 = DVec3d::From(9, 7, 2);
    DVec2d vec2d = DVec2d::FromStartEnd(vec3d0, vec3d1);

    vec3d1.Subtract(vec3d0);
    DVec2d vec2d2 = DVec2d::From(vec3d1.x, vec3d1.y);
    Check::Near(vec2d2, vec2d);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(DVec2d, RotationEqual)
    {
    DVec2d vec0 = DVec2d::From(3, 2);
    vec0.Rotate90CW();
    vec0.Rotate90CW();
    vec0.Rotate90CW();
    
    vec0.AlmostEqual(DVec2d::From(-3, 2));
    }