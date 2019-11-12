/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz  04/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, DirectInitialization)
    {    
    DVec3d vector0;
    vector0.Init (1,2,3);
    Check::Near (vector0.x, 1.0);
    Check::Near (vector0.y, 2.0);
    Check::Near (vector0.z, 3.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, DotProduct)
    {
    DVec3d vector0;
    DVec3d vector1;
    vector0.Init (1,2,3);
    vector1.Init (5,7,11);
    Check::Near (vector0.DotProduct (vector1),
                vector0.DotProduct (vector1.x, vector1.y, vector1.z));
    Check::Near (vector0.DotProduct (vector1),
                   vector1.DotProduct (vector0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ParthaChail  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, FromArray)
    {
    DVec3d objInit;
    double arr[] = { 2.3, 3.4, 4.5 };

    objInit.InitFromArray(arr);

    DVec3d objFromArray = DVec3d::FromArray(arr);

    Check::Near (objInit.x, objFromArray.x);
    Check::Near (objInit.y, objFromArray.y);
    Check::Near (objInit.z, objFromArray.z);

    Check::Near (objInit.x, arr[0]);
    Check::Near (objInit.y, arr[1]);
    Check::Near (objInit.z, arr[2]);

    double ax, ay, az;
    objFromArray.GetComponents (ax, ay, az);
    Check::Near (ax, arr[0]);
    Check::Near (ay, arr[1]);
    Check::Near (az, arr[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ParthaChail  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, FromXYZ)
    {
    DVec3d objInit;
    double x = 2.3;
    double y = 3.4;
    double z = 4.5;
    objInit.Init (x, y, z);

    DVec3d objFromXYZ = DVec3d::From (x, y, z);

    Check::Near (objInit.x, objFromXYZ.x);
    Check::Near (objInit.y, objFromXYZ.y);
    Check::Near (objInit.z, objFromXYZ.z);

    Check::Near (objInit.x, x);
    Check::Near (objInit.y, y);
    Check::Near (objInit.z, z);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ParthaChail  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, FromPoint)
    {
    DVec3d objInit;

    double x = 2.3;
    double y = 3.4;
    double z = 4.5;

    DPoint3d point = DPoint3d::FromXYZ (x, y, z);

    objInit.Init (point);

    DVec3d objFromPoint = DVec3d::From (point);

    Check::Near (objInit.x, objFromPoint.x);
    Check::Near (objInit.y, objFromPoint.y);
    Check::Near (objInit.z, objFromPoint.z);

    Check::Near (objInit.x, point.x);
    Check::Near (objInit.y, point.y);
    Check::Near (objInit.z, point.z);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ParthaChail  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, FromXY)
    {
    DVec3d objInit;
    double x = 2.3;
    double y = 3.4;

    objInit.Init (x, y);

    DVec3d objFromXY = DVec3d::From (x, y);

    Check::Near (objInit.x, objFromXY.x);
    Check::Near (objInit.y, objFromXY.y);
    Check::Near (objInit.z, 0.0);
    Check::Near (objFromXY.z, 0.0);

    Check::Near (objInit.x, x);
    Check::Near (objInit.y, y);

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ParthaChail  10/09
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, FromXYAngleAndMagnitude)
    {
    DVec3d objInit;
    double theta = PI; 
    double magnitude = 12;

    objInit.InitFromXYAngleAndMagnitude (theta, magnitude);

    DVec3d objFromXYAngleAndMagnitude = DVec3d::FromXYAngleAndMagnitude (theta, magnitude);

    Check::Near (objInit.x, objFromXYAngleAndMagnitude.x);
    Check::Near (objInit.y, objFromXYAngleAndMagnitude.y);
    Check::Near (objInit.z, 0.0);
    Check::Near (objFromXYAngleAndMagnitude.z, 0.0);

    Check::Near (objInit.x, 12 * cos(theta) ); // Ax = magnitude * cos theta  = 12 * -1
    Check::Near (objInit.y, 12 * sin(theta) );   // Ay = magnitude sin 20 theta = 12 * 0

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, ConstructByMatrixAndTransformAccess)
    {
    RotMatrix matrix = RotMatrix::FromRowValues
                (
                1,2,3,
                5,7,11,
                13,17,19
                );
    DPoint3d translationAsPoint = DPoint3d::FromXYZ (23,27,29);
    Transform transform = Transform::From (matrix, translationAsPoint);
    DVec3d translationAsVector = DVec3d::FromTranslation (transform);
    Check::Near (translationAsPoint, translationAsVector);

    double mag[3] = {
                sqrt ((double) (1 * 1 + 5 * 5 + 13 * 13)),
                sqrt ((double) (2 * 2 + 7 * 7 + 17 * 17)),
                sqrt ((double) (3 * 3 + 11 * 11 + 19 * 19))
                    };

    DVec3d vectorQ = DVec3d::From (0.3, 1.2, 5.3);
    for (int j = -1; j < 3; j++)
        {
        for (int i = 0; i < 3; i++)
            {
            int k = 3 * j + i;
            Check::Int (i, Angle::Cyclic3dAxis (k));
            // Confirm row, column access matches cyclically within Transform ...
            Check::Near (DVec3d::FromMatrixColumn (transform, i), DVec3d::FromMatrixColumn (transform, k));
            Check::Near (DVec3d::FromMatrixRow (transform, i), DVec3d::FromMatrixRow (transform, k));
            // ... cyclically within RotMatrix ...
            Check::Near (DVec3d::FromColumn (matrix, i), DVec3d::FromColumn (matrix, k));
            Check::Near (DVec3d::FromRow (matrix, i), DVec3d::FromRow (matrix, k));
            // ... old style get
            DVec3d getRow, getCol;
            getCol.InitFromColumn (matrix, k);
            getRow.InitFromRow (matrix, k);
            Check::Near (DVec3d::FromColumn (matrix, i), getCol);
            Check::Near (DVec3d::FromRow    (matrix, i), getRow);

            Check::Near (getRow.DotProduct (vectorQ), vectorQ.DotProductRow (matrix, k));
            Check::Near (getCol.DotProduct (vectorQ), vectorQ.DotProductColumn (matrix, k));

            Check::Near (getRow.DotProduct (vectorQ), vectorQ.DotProductMatrixRow (transform, k));
            Check::Near (getCol.DotProduct (vectorQ), vectorQ.DotProductMatrixColumn (transform, k));

            // Between matrix and transform ...
            Check::Near (DVec3d::FromColumn (matrix, i), DVec3d::FromMatrixColumn (transform, k));
            Check::Near (DVec3d::FromRow (matrix, i), DVec3d::FromMatrixRow (transform, k));


            
            }
        }

    for (int iA = 0; iA < 3; iA++)
        {
        for (int iB = 0; iB < 3; iB++)
            {
            DVec3d vectorA = DVec3d::FromColumn (matrix, iA);
            DVec3d vectorB = DVec3d::FromColumn (matrix, iB);
            Check::Near (mag[iA], vectorA.Magnitude ());
            Check::Near (mag[iB], vectorB.Magnitude ());
            DVec3d vectorAB0 = DVec3d::FromCrossProduct (vectorA, vectorB);
            DVec3d vectorAB1 = DVec3d::FromColumnCrossProduct (matrix, iA, iB);
            DVec3d vectorAB2 = DVec3d::FromMatrixColumnCrossProduct (transform, iA, iB);
            Check::Near (vectorAB0, vectorAB1);
            Check::Near (vectorAB0, vectorAB2);
            // All components are integers.   Zeros should be zeros.
            if (iA == iB)
                {
                Check::Near (0.0, vectorAB0.MaxAbs ());
                }
            else
                {
                Check::Near (0.0, vectorAB0.DotProduct (vectorA));
                Check::Near (0.0, vectorAB0.DotProduct (vectorB));

                double a = mag[iA] * mag[iB] * sin (vectorA.AngleTo (vectorB));
                double b = vectorAB0.Magnitude ();
                Check::Near (a, b);
                }
            }
        }
    }

void TestProducts (DVec3dCR vector0, DVec3dCR vector1)
    {
    RotMatrix matrix;
    Transform transform;
    double length0 = vector0.Magnitude ();
    double length1 = vector1.Magnitude ();
    DVec3d cross = DVec3d::FromCrossProduct (vector0, vector1);
    // Anything that calculates an angle or parallelism uses cross.  zero dot is independent.
    Check::NearZero (cross.DotProduct (vector0));
    Check::NearZero (cross.DotProduct (vector1));
    double theta = vector0.AngleTo (vector1);
    Check::Near (length0 * length1 * sin (theta), cross.Magnitude ());
    Check::Near (length0 * length1 * cos (theta), vector0.DotProduct (vector1));

    Check::Near (cross, DVec3d::FromCrossProduct (vector0.x, vector0.y, vector0.z,
                                                  vector1.x, vector1.y, vector1.z));

    matrix.SetColumn (vector0, 0);
    matrix.SetColumn (vector1, 1);
    matrix.SetColumn (cross, 2);
    transform.InitFrom (matrix);
    DVec3d M01 = DVec3d::FromColumnCrossProduct (matrix, 0, 1);
    DVec3d T01 = DVec3d::FromMatrixColumnCrossProduct (transform, 0, 1);

    Check::Near (cross, M01);
    Check::Near (cross, T01);
    Check::Near (DVec3d::FromColumnCrossProduct (matrix, 0,2),
                DVec3d::FromMatrixColumnCrossProduct (transform, 0, 2));
    Check::Near (DVec3d::FromColumnCrossProduct (matrix, 1,2),
                DVec3d::FromMatrixColumnCrossProduct (transform, 1, 2));

    double dot = vector0.DotProduct (vector1);
    Check::Near (dot, vector0.DotProduct
                            (vector1.x, vector1.y, vector1.z));

    Check::Near (dot, vector1.DotProductColumn (matrix, 0));


    DPoint3d point0 = vector0;
    DPoint3d point1 = vector1;
    Check::Near (dot, vector0.DotProduct (point1));
    Check::Near (dot - vector0.z * vector1.z, vector0.DotProductXY (vector1));
    DVec3d W00, W01, W10;
    W00.CrossProduct (vector0, vector1);
    W01.CrossProduct (vector0, point1);
    W10.CrossProduct (point0, vector1);
    Check::Near (cross, W00);
    Check::Near (cross, W01);
    Check::Near (cross, W10);

    DPoint3d pointA, pointA0, pointA1;
    pointA.Init (2.3, 5.4, 0.99923);
    pointA0.SumOf (pointA, vector0);
    pointA1.SumOf (pointA, vector1);
    DVec3d crossToPoints;
    crossToPoints.CrossProductToPoints (pointA, pointA0, pointA1);
    Check::Near (cross, crossToPoints);
    Check::Near (cross.z, vector0.CrossProductXY (vector1));

    DVec3d cross1, crossAlpha, crossM;
    double alpha = 2.19090;
    cross1.NormalizedCrossProduct (vector0, vector1);
    crossAlpha.SizedCrossProduct (vector0, vector1, alpha);
    crossM.GeometricMeanCrossProduct (vector0, vector1);
    Check::True (cross1.IsParallelTo (cross));
    Check::True (crossAlpha.IsParallelTo (cross));
    Check::True (crossM.IsParallelTo (cross));
    Check::Near (1.0, cross1.Magnitude ());
    Check::Near (alpha, crossAlpha.Magnitude ());
    Check::Near (sqrt (length0) * sqrt (length1), crossM.Magnitude ());

        {
        DVec3d unit[3];
        DVec3d axes[3];
        vector0.GetNormalizedTriad (unit[0], unit[1], unit[2]);
        vector0.GetTriad (axes[0], axes[1], axes[2]);
        double a = vector0.Magnitude ();
        for (int i = 0; i < 3; i++)
            {
            int j = (i + 1) % 3;
            Check::Near (1.0, unit[i].Magnitude ());
            Check::Near (vector0.Magnitude (), axes[i].Magnitude ());
            Check::NearZero (unit[i].DotProduct (unit[j]));
            Check::NearZero (axes[i].DotProduct (axes[j]));
            Check::Near (1.0, unit[0].TripleProduct (unit[1], unit[2]));
            Check::Near (a * a * a, axes[0].TripleProduct (axes[1], axes[2]));
            }
        }
    }

void TestMeasures (DVec3dCR vector0, DVec3dCR vector1)
    {
    DVec3d vector01_constructor = DVec3d::FromStartEnd (vector0, vector1);
    DVec3d vector01_difference;
    DVec3d vector01_normalizedDifference;
    vector01_normalizedDifference.NormalizedDifference (vector1, vector0);
    vector01_difference.DVec3d::DifferenceOf (vector1, vector0);
    Check::Near (vector01_constructor.Magnitude (),
                    vector0.Distance (vector1));
    Check::Near (vector01_constructor.MagnitudeSquared (),
                    vector0.DistanceSquared (vector1));

    Check::True (vector01_difference.IsParallelTo (vector01_normalizedDifference));
    Check::Near (1.0, vector01_normalizedDifference.Magnitude ());

    Check::Near (vector01_constructor.MagnitudeXY (),
                    vector0.DistanceXY (vector1));
    Check::Near (vector01_constructor.MagnitudeSquaredXY (),
                    vector0.DistanceSquaredXY (vector1));
    double a = 2.0 * vector0.Magnitude ();
    for (int i = -6; i < 10; i++)
        {
        DVec3d workVector = vector0;
        double b = a + workVector.GetComponent (i);
        workVector.SetComponent (b, i);
        Check::Near (b, workVector.MaxAbs ());
        }

    double tol = 1.0e-8 * vector0.Magnitude ();
    double smaller = 0.01 * tol;
    double larger = 100.0 * tol;

    for (int i = -6; i < 10; i++)
        {
        DVec3d vector2 = vector1;
        Check::True (vector1.IsEqual (vector2));
        vector2.SetComponent (vector1.GetComponent (i) + smaller, i);
        Check::False (vector1.IsEqual (vector2));
        Check::True  (vector1.IsEqual (vector2, tol));
        vector2.SetComponent (vector1.GetComponent (i) + larger, i);
        Check::False (vector1.IsEqual (vector2));
        Check::False  (vector1.IsEqual (vector2, tol));
        }

    }

void TestOps (DVec3dCR vector0, DVec3dCR vector1)       // ASSUME independent vectors??
    {
    DVec3d unitA, unitB;
    unitA.Normalize (vector0);
    unitB = vector0;
    unitB.Normalize ();
    Check::Near (unitA, unitB);
    //double a = vector0.Magnitude ();
    double b = 2.901;
    DVec3d scaleA, scaleB;
    scaleB = vector0;
    scaleA.ScaleToLength (vector0, b);
    scaleB.ScaleToLength (b);
    Check::Near (scaleA, scaleB);
    Check::True (scaleA.IsParallelTo (vector0));
    Check::Near (b, scaleA.Magnitude ());

    double c = 4.201;
    DVec3d scaleC, scaleD, scaleE;
    scaleC = vector1;
    scaleC.Scale (c);
    scaleD.Scale (vector1, c);
    Check::Near (scaleC, scaleD);
    Check::True (scaleC.IsParallelTo (vector1));
    Check::Near (c * vector1.Magnitude (), scaleC.Magnitude ());

    Check::True (scaleE.SafeDivide (scaleC, c));
    Check::Near (vector1, scaleE);

    DVec3d negE, negF;
    negE = vector1;
    negE.Negate ();
    negF.Negate (vector1);
    Check::Near (negE, negF);
    Check::True (vector1.DotProduct (negE) < 0.0);
    Check::Near (vector1.DotProduct (negE), -vector1.MagnitudeSquared ());
    DVec3d ones;
    ones.One ();
    DVec3d zero;
    zero.Zero ();
    Check::Near (0.0, zero.MaxAbs ());
    Check::Near (1.0, ones.MaxAbs ());
    Check::Near (zero, DVec3d::From (0,0,0));
    Check::Near (ones, DVec3d::From (1,1,1));

    DVec3d reverse1 = DVec3d::FromScale (vector1, -1.0);
    Check::True (vector1.IsParallelTo (vector1));
    Check::True (vector1.IsParallelTo (reverse1));
    Check::False (vector1.IsParallelTo (vector0));

    Check::True (vector1.IsPositiveParallelTo (vector1));
    Check::False (vector1.IsPositiveParallelTo (reverse1));
    Check::False (vector1.IsPositiveParallelTo (vector0));

    }

void TestReadWrite (DVec3dCR vector0, DVec3dCR vector1, DVec3dCR vector2)
    {
    DPoint3d origin = DPoint3d::FromXYZ(5,2,-1);
    DVec3d originVector = DVec3d::From(origin);

    RotMatrix matrix = RotMatrix::FromColumnVectors (vector0, vector1, vector2);
    RotMatrix transpose = RotMatrix::FromRowVectors (vector0, vector1, vector2);

    Transform transform = Transform::From (matrix, origin);


    for (int i = -6; i < 10; i += 3)
        {
        Check::Near (DVec3d::FromColumn (matrix, i),    vector0);
        Check::Near (DVec3d::FromColumn (matrix, i+1),  vector1);
        Check::Near (DVec3d::FromColumn (matrix, i+2),  vector2);

        Check::Near (DVec3d::FromRow (transpose, i),    vector0);
        Check::Near (DVec3d::FromRow (transpose, i+1),  vector1);
        Check::Near (DVec3d::FromRow (transpose, i+2),  vector2);
        }

    double a0 = 2.9;
    double a1 = -1.2;
    double a2 = -3.4;


    DVec3d vector_0p1A, vector_0p1B, vector_0p1C;
    
    vector_0p1A.SumOf (vector0, vector1);
    vector_0p1B = vector0;
    vector_0p1B.Add (vector1);
    vector_0p1C.Multiply (matrix, 1.0, 1.0, 0.0);    
    Check::Near (vector_0p1A , vector_0p1B);
    Check::Near (vector_0p1A, vector_0p1C);

    DVec3d vector01A, vector01B, vector01C;
    vector01A = DVec3d::FromStartEnd (vector0, vector1);
    vector01B = vector1;
    vector01B.Subtract (vector0);
    vector01C.DifferenceOf (vector1, vector0);
    Check::Near (vector01A, vector01B);
    Check::Near (vector01A, vector01C);

    double f = 0.23491;
    DVec3d vectorF0, vectorF1;
    vectorF0.Interpolate (vector0, f, vector1);
    vectorF1.SumOf (vector0, vector01A, f);
    Check::Near (vectorF0, vectorF1);

    DVec3d vectorX;
    vectorX.Init (a0, a1, a2);
    DVec3d sum;
    DVec3d product;
    product.Multiply (matrix, vectorX);
    // Vs+Vs+Vs
    sum.SumOf (vector0, a0, vector1, a1, vector2, a2);
    Check::Near (product, sum);
    product.Multiply (matrix, a0, a1, 0.0);
    // Vs+Vs
    sum.SumOf (vector0, a0, vector1, a1);
    Check::Near (product, sum);

    // tedious point/vector flipflops...
    double b2 = -3.9;

    // V + Vs
    sum.SumOf (originVector, vector2, b2);
    DPoint3d pointA = DPoint3d::FromProduct (transform, 0.0, 0.0, b2);
    Check::Near (sum, DVec3d::From(pointA));

    sum.SumOf (originVector, vector1);
    Check::Near (sum, DPoint3d::FromProduct (transform, 0.0, 1.0, 0.0));

    sum.SumOf (originVector, vector0, a0, vector1, a1);
    Check::Near (sum, DPoint3d::FromProduct (transform, a0, a1, 0.0));

    sum.SumOf (originVector, vector0, a0, vector1, a1, vector2, a2);
    Check::Near (sum, DPoint3d::FromProduct (transform, a0, a1, a2));

    DVec3d wTA_0, wTA_1;
    DVec3d w = DVec3d::From (a0, a1, a2);
    wTA_0.MultiplyTranspose (matrix, a0, a1, a2);
    wTA_1.MultiplyTranspose (matrix, w);
    Check::Near (wTA_0, wTA_1);


    DVec3d v1, v2, v3;
    v1.Init (vector0.x, vector0.y, vector0.z);
    Check::Near (vector0, v1);
    v2.Init (vector0.x, vector0.y, 0.0);
    v3.Init (vector0.x, vector0.y);
    Check::Near (v2, v3);
    DPoint3d p4 = vector0;  // assign through inheritance
    DVec3d v4;
    v4.Init (p4);
    Check::Near (vector0, v4);

        {
        DVec3d vector0XY = DVec3d::From (vector0.x, vector0.y);
        double theta = atan2 (vector0.y, vector0.x);
        double thetaA = DVec3d::From (1,0,0).AngleToXY (vector0);
        Check::Near (theta, thetaA);
        double rho   = vector0.MagnitudeXY ();
        double rho2  = vector0.MagnitudeSquaredXY ();
        Check::Near (rho2, rho * rho, "MagnitudeSquaredXY");
        DVec3d vector0XY_A ;
        vector0XY_A.InitFromXYAngleAndMagnitude (theta, rho);
        DVec3d vector0XY_B = DVec3d::FromXYAngleAndMagnitude (theta, rho);
        Check::Near (vector0XY, vector0XY_A);
        Check::Near (vector0XY, vector0XY_B);

        double alpha = 0.3;
        DVec3d rotatedA, rotatedB;
        rotatedA.RotateXY (vector0, alpha);
        rotatedB = vector0;
        rotatedB.RotateXY (alpha);
        Check::Near (rotatedA, rotatedB);
        Check::Near (vector0.MagnitudeXY (), rotatedA.MagnitudeXY ());
        Check::Near (alpha, vector0.AngleToXY (rotatedA));
        Check::Near (vector0.z, rotatedA.z);
        
        DVec3d perpXY;
        perpXY.UnitPerpendicularXY (vector0);
        Check::Near (0.5 * Angle::Pi (), vector0.AngleToXY (perpXY));
        Check::Near (1.0, perpXY.MagnitudeXY());
        Check::Near (0.0, perpXY.z);
        
        }
    }

void FindTolerance (DVec3dCR vector0)
    {
    double eps = 1.0e-1;
    double a = vector0.Magnitude ();
    double approxTol = (1.0 + a) * DoubleOps::SmallCoordinateRelTol ();
    double rangeFactor = 10.0;
    static bool s_printToleranceDetails = false;
    for (size_t i = 0; i < 50; i++, eps *= 0.5)
        {
        DVec3d vector1 = vector0;
        double absDelta = fabs (eps * a);
        vector1.x += absDelta;
        if (vector0.AlmostEqual (vector1))
            {
            Check::True (absDelta < approxTol * rangeFactor && absDelta > approxTol / rangeFactor, "Tolerance Search");
            if (s_printToleranceDetails)
                printf (" (%g,%g,%g) Apparent AlmostEqual (reltol %g) (absDelta %g)\n",
                    vector0.x, vector0.y, vector0.z, eps, absDelta);
            return;
            }
        }
    Check::True (false, "UNABLE TO FIND TOLERANCE FOR ALMOSTEQUAL");
    }

// ASSUME vec0, vec1 not parallel!!
void TestAngles_go (DVec3d vec0, DVec3d vec1)
    {
    Check::False (vec0.IsParallelTo (vec1));
    Check::True (vec0.IsParallelTo (vec0));
  
    double positiveAngle = vec0.AngleTo (vec1);
    DVec3d perp1 = DVec3d::FromCrossProduct (vec0, vec1);
    DVec3d perp2 = DVec3d::FromCrossProduct (vec1, vec0);
    double smallerAngle = vec0.SmallerUnorientedAngleTo (vec1);
    double signedAngle1 = vec0.SignedAngleTo (vec1, perp1);
    double signedAngle2 = vec0.SignedAngleTo (vec1, perp2);

    Check::True (perp1.IsParallelTo (vec0));
    Check::True (perp2.IsParallelTo (vec1));

    Check::Near (signedAngle1, -signedAngle2);
    bool acuteAngle = positiveAngle < Angle::PiOver2 ();
    DVec3d sums[4];
    sums[0].SumOf (vec0, 1.0,  vec1, 1.0);
    sums[1].SumOf (vec0, -1.0, vec1, 1.0);
    sums[2].SumOf (vec0, -1.0, vec1, -1.0);
    sums[3].SumOf (vec0, 1.0, vec1, -1.0);
    Check::True (sums[0].IsVectorInSmallerSector  (vec0, vec1));
    Check::False (sums[1].IsVectorInSmallerSector (vec0, vec1));
    Check::False (sums[2].IsVectorInSmallerSector (vec0, vec1));
    Check::False (sums[3].IsVectorInSmallerSector (vec0, vec1));

    Check::True (sums[0].IsVectorInCCWSector (vec0, vec1, perp1));
    Check::False (sums[1].IsVectorInCCWSector (vec0, vec1, perp1));
    Check::False (sums[2].IsVectorInCCWSector (vec0, vec1, perp1));
    Check::False (sums[3].IsVectorInCCWSector (vec0, vec1, perp1));

    Check::False (sums[0].IsVectorInCCWSector (vec0, vec1, perp2));
    Check::True (sums[1].IsVectorInCCWSector (vec0, vec1, perp2));
    Check::True (sums[2].IsVectorInCCWSector (vec0, vec1, perp2));
    Check::True (sums[3].IsVectorInCCWSector (vec0, vec1, perp2));

    

    if (acuteAngle)
        {
        Check::Near (positiveAngle, smallerAngle);
        }
    else
        {
        Check::Near (smallerAngle, Angle::PiOver2 () - positiveAngle);
        }

    // Tilt from each vector in the z direction
    DVec3d tilt0, tilt1;
    tilt0.SumOf (vec0, 1.3, perp1, 0.3);
    tilt1.SumOf (vec1, 0.3, perp1, -0.2);
    Check::Near (signedAngle1, tilt0.PlanarAngleTo (tilt1, perp1));
    Check::False (tilt0.IsParallelTo (vec0));
    Check::False (tilt0.IsPerpendicularTo (vec0));
    Check::False (tilt0.IsPerpendicularTo (perp1));
    }

void TestAngles (DVec3d vec0, DVec3d vec1)
    {
    DVec3d neg0, neg1;
    neg0.Negate (vec0);
    neg1.Negate (vec1);

    TestAngles_go (vec0, vec1);
    TestAngles_go (neg0, vec1);
    TestAngles_go (vec0, neg1);
    TestAngles_go (neg0, neg1);
    }
void TestDifferenceOf (DPoint3d point0, DPoint3d point1)
    {
    DVec3d vecA = DVec3d::FromStartEnd (point0, point1);
    DVec3d vecB;
    vecB.DifferenceOf (point1, point0);
    Check::Near (vecA, vecB);
    DPoint3d point1A = point0;
            point1A.Add (vecA);
    DPoint3d point0A = point1;
            point0A.Subtract (vecA);
    Check::Near (point0, point0A);
    Check::Near (point1, point1A);
    }

void AllTests (DVec3dCR vector0, DVec3dCR vector1, DVec3dCR vector2)
    {
    TestProducts (vector0, vector1);
    TestMeasures (vector0, vector1);
    TestOps      (vector0, vector1);
    TestDifferenceOf (vector0, vector1);
    TestReadWrite (vector0, vector1, vector2);
    FindTolerance (vector0);
    FindTolerance (vector1);
    FindTolerance (vector2);
    for (int power = -10; power < 10; power += 3)
        {
        DVec3d vector;
        vector.Scale (vector2, pow (10.0, power));
        FindTolerance (vector);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec3d, All)
    {
    DVec3d vector0 = DVec3d::From (1,2,3);
    DVec3d vector1 = DVec3d::From (4,5,-6);
    DVec3d cross = DVec3d::FromCrossProduct (vector0, vector1);
    DVec3d vector2;
    vector2.SumOf (vector0, 1.2, vector1, -1.3, cross, 0.144);
    AllTests (vector0, vector1, vector2);
    }

void TestPerpendicularParts (double ux, double uy, double uz, double vx, double vy, double vz)
    {
    DVec3d U = DVec3d::From (ux,uy,uz);
    DVec3d V = DVec3d::From (vx,vy,vz);
    Check::StartScope ("PerpendicularParts");
    double f;
    DVec3d V0, V1;
    if (U.GetPerpendicularParts (V, f, V0, V1))
        {
        Check::Near (V.MagnitudeSquared (), V0.MagnitudeSquared () + V1.MagnitudeSquared (), "Pythagoras");
        Check::True (U.IsParallelTo (V0), "Parallel part direction");
        Check::True (U.IsPerpendicularTo (V1), "Parallel part direction");
        }
    Check::EndScope ();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DVec3d, PerpendicularParts)
    {
    TestPerpendicularParts (1,0,0,0,1,0);
    TestPerpendicularParts (1,1,1,0,1,0);
    TestPerpendicularParts (1,2,3,6,5,7);
    TestPerpendicularParts (1,2,3,1,2,3);
    }
    
// DSegment4d DSegment4d::From (double xA, double yA, double zA, double wA, double xB, double yB, double zB, double wB)
// LINE 145
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test1)
    {
    DSegment4d segmentA = DSegment4d::From(1, 2, 3, 4, 5, 6, 7, 8);
    DSegment4d segmentB = DSegment4d::From(DPoint4d::From (1, 2, 3, 4), DPoint4d::From (5, 6, 7, 8));
    Check::Near (segmentA.point[0], segmentB.point[0], "start points");
    }
    
// DSegment4d DSegment4d::From (double xA, double yA, double zA, double xB, double yB, double zB)
// LINE 153
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test2)
    {
    DSegment4d segmentC = DSegment4d::From(1.3, 2.6, 3.7, 5.1, 6.8, 7.5);
    Check::Near (segmentC.point[0].w, 1, "weight value");
    }
    
// DSegment4d DSegment4d::From (DSegment3dCR segment)
// LINE 160
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test3)
    {
    DSegment3d segmentD = DSegment3d::From (1.1, 2.2, 3.3, 2.3, 3.4, 4.5); 
    DSegment4d segmentE = DSegment4d::From (segmentD);
    for(int i=0;i<2;i++)
    {
    Check::Near (segmentE.point[i].x, segmentD.point[i].x, "x value");
    Check::Near (segmentE.point[i].y, segmentD.point[i].y, "y value");
    Check::Near (segmentE.point[i].z, segmentD.point[i].z, "z value");
    }
    }
    
// DSegment4d DSegment4d::FromFractionInterval (DSegment4d parent, double startFraction, double endFraction)
// LINE 172
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test4)
    {
    DSegment4d parent = DSegment4d::From(1, 2, 1, 2, 1, 2);
    DSegment4d segmentF;
    segmentF.FromFractionInterval (parent, 0.3, 0.7);
    Check::Near (segmentF.point[0].x, parent.point[0].x + 0.3 * (parent.point[1].x - parent.point[0].x), "x value");
    }
    
// DSegment4d DSegment4d::From (DPoint3dCR pointA, DPoint3dCR pointB)
// LINE 134
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test5)
    {
    DSegment3d segmentX = DSegment3d::From (0.5, 1, 4, 3, 2, 1.5); 
    DSegment3d segmentY = DSegment3d::From (5, 2, 2.3, 4.4, 1.7, 3.4); 
    DSegment4d segmentZ = DSegment4d::From (segmentX.point[0], segmentY.point[1]);
    Check::Near (segmentZ.point[0].x, segmentX.point[0].x, "x value");
    Check::Near (segmentZ.point[1].y, segmentY.point[1].y, "y value");
    }
    
// DSegment4d DSegment4d::From (DPoint4dCR pointA, DPoint4dCR pointB)
// LINE 123
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test6)
    {
    DSegment4d segmentP = DSegment4d::From (0.5, 1, 4, 2.4, 3, 2, 1.5, 2.2); 
    DSegment4d segmentQ = DSegment4d::From (5, 2, 2.3, 1.8, 4.4, 1.7, 3.4, 5.9); 
    DSegment4d segmentR = DSegment4d::From (segmentP.point[1], segmentQ.point[0]);
    Check::Near (segmentR.point[0].x, segmentP.point[1].x, "x value");
    Check::Near (segmentR.point[1].y, segmentQ.point[0].y, "y value");
    }
    
// bool DSegment4d::ProjectDPoint4dCartesianXYW (DPoint4dR closestPoint, double &closestParam, DPoint4dCR spacePoint) const
//LINE 89
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test7)
    {
        DSegment4d segmentCAR = DSegment4d::From (1.4, 2.2, 5.6, 7.7, 2.5, 3.3, 8.8, 9.4);
        double closestParam = 1;
        DPoint4d closestPoint = DPoint4d::From (2.7, 1.3, 8.4, 9.9);  
        DPoint4d spacePoint = DPoint4d::From (2.1, 3.1, 6.3, 8.9);
        segmentCAR.ProjectDPoint4dCartesianXYW(closestPoint, closestParam, spacePoint);
        double shortx = (spacePoint.x - closestPoint.x)*(spacePoint.x - closestPoint.x);
        double shorty = (spacePoint.y - closestPoint.y)*(spacePoint.y - closestPoint.y);
        double shortz = (spacePoint.z - closestPoint.z)*(spacePoint.z - closestPoint.z);
        double shortw = (spacePoint.w - closestPoint.w)*(spacePoint.w - closestPoint.w);
        double distshort = sqrt(shortx + shorty + shortz + shortw);
        DPoint4d closePoint = DPoint4d::From (closestPoint.x + 0.01, closestPoint.y + 0.01, closestPoint.z + 0.01,closestPoint.w + 0.01);
        double longx = (spacePoint.x - closePoint.x)*(spacePoint.x - closePoint.x);
        double longy = (spacePoint.y - closePoint.y)*(spacePoint.y - closePoint.y);
        double longz = (spacePoint.z - closePoint.z)*(spacePoint.z - closePoint.z);
        double longw = (spacePoint.w - closePoint.w)*(spacePoint.w - closePoint.w);
        double distlong = sqrt(longx + longy + longz + longw);
        Check::True ( distlong > distshort, "checking that closestPoint is the closest");
        
        
        /*DVec3d ortho, vec;
        
        double segfracx = segmentCAR.point[0].x + closestParam*(segmentCAR.point[1].x - segmentCAR.point[0].x);
        double segfracy = segmentCAR.point[0].y + closestParam*(segmentCAR.point[1].y - segmentCAR.point[0].y);
        double segfracz = segmentCAR.point[0].z + closestParam*(segmentCAR.point[1].z - segmentCAR.point[0].z);
        double segfracw = segmentCAR.point[0].w + closestParam*(segmentCAR.point[1].w - segmentCAR.point[0].w);
        DPoint4d Xs = DPoint4d::From (segfracx, segfracy, segfracz, segfracw);
        
        ortho.WeightedDifferenceOf (spacePoint, Xs);
        vec.WeightedDifferenceOf (segmentCAR.point[1], segmentCAR.point[0]);
        
        Check::Perpendicular (ortho, vec, "checking orthogonality");*/
    }

// void DSegment4d::Init (double x0, double y0, double z0, double x1, double y1, double z1)
// LINE 10
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test8)
    {
    DSegment4d segmentA1;
    segmentA1.Init(1.1, 2.2, 3.3, 4.4, 5.5, 6.6);
    Check::Near (segmentA1.point[0].x, 1.1, "x value");
    Check::Near (segmentA1.point[1].y, 5.5, "y value");
    }
  
// void DSegment4d::Init (double x0, double y0, double z0, double w0, double x1, double y1, double z1, double w1)  
// LINE 28 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test9)
{
    DSegment4d segmentB1;
    segmentB1.Init(1.1, 2.2, 3.3, 8.8, 4.4, 5.5, 6.6, 7.7);
    Check::Near (segmentB1.point[0].y, 2.2, "y value");
    Check::Near (segmentB1.point[1].w, 7.7, "w value");
    }
    
// void DSegment4d::Init (DSegment3dCR source)   
// LINE 48
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test10)
    {
    DSegment4d segmentC1;
    DSegment3d segmentD1 = DSegment3d::From (4, 3, 5, 6, 1, 8);
    segmentC1.Init(segmentD1);
    Check::Near (segmentC1.point[0].y, segmentD1.point[0].y, "y value");
    Check::Near (segmentC1.point[1].w, 1, "w value");
    }

// void DSegment4d::Init (DPoint3dCR point0, DPoint3dCR point1)   
// LINE 58
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test11)
    {
    DSegment4d segmentE1;
    DPoint3d pointA1 = DPoint3d::From (2.2, 4.4, 1.1);
    DPoint3d pointB1 = DPoint3d::From (3.3, 1.2, 0.7);
    segmentE1.Init(pointA1, pointB1);
    Check::Near (segmentE1.point[1].y, pointB1.y, "y value");
    Check::Near (segmentE1.point[0].w, 1, "w value");
    }

// void DSegment4d::Init (DPoint4dCR point0, DPoint4dCR point1)   
// LINE 68
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test12)
    {
    DSegment4d segmentZ1;
    DPoint4d pointX1 = DPoint4d::From (2.2, 4.4, 1.1, 6);
    DPoint4d pointY1 = DPoint4d::From (3.3, 1.2, 0.7, 3);
    segmentZ1.Init(pointX1, pointY1);
    Check::Near (segmentZ1.point[0].z, pointX1.z, "z value");
    Check::Near (segmentZ1.point[1].w, pointY1.w, "w value");
    }
    
// bool DSegment4d::FractionParameterToPoint (DPoint4dR outPoint, double fraction) const
// LINE 79
// ****What should I be testing here?****
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test13)
    {
    DSegment4d segmentR1 = DSegment4d::From (5, 6, 7, 8, 4, 3, 2, 1);
    DPoint4d pointQ1 = DPoint4d::From (5, 4, 3, 2);
    segmentR1.FractionParameterToPoint(pointQ1, 0.5);
    }
    
// bool DSegment4d::GetEndPoints (DPoint3dR point0, DPoint3dR point1)
// LINE 187
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test14)
    {
    DPoint3d pointAA = DPoint3d::From (1, 2, 3);
    DPoint3d pointAB = DPoint3d::From (7, 6, 5);
    DSegment4d segmentAA = DSegment4d::From (pointAA, pointAB);
    bool condition = segmentAA.GetEndPoints (pointAA, pointAB);
    Check::Bool (condition, true, "boolean test");
    //Check::Near (segmentAA.point[1].z, pointAB.z, "z value");
    }
    
// void DSegment4d::InitProduct (TransformCR transform, DSegment4dCR source)
// void DSegment4d::InitProduct (DMatrix4dCR mat, DSegment4dCR source)
// LINE 200 + LINE 205
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test15)
    {
    DSegment4d segmentBB;
    DSegment4d segmentCC;
    DSegment4d source = DSegment4d::From (2, 1, 3, 5, 4, 2, 8, 6);
    Transform testtran = Transform::FromRowValues (1, 2, 3, 1, 2, 4, 2, 1, 5, 3, 5, 1);
    DMatrix4d testmat = DMatrix4d::FromRowValues (1, 2, 3, 1, 2, 4, 2, 1, 5, 3, 5, 1, 0, 0, 0, 1);
    segmentBB.InitProduct (testtran, source);
    segmentCC.InitProduct (testmat, source);
    Check::Near (segmentBB.point[0].y, testtran.form3d[1][0]*source.point[0].x + testtran.form3d[1][1]*source.point[0].y + testtran.form3d[1][2]*source.point[0].z + testtran.form3d[1][3]*source.point[0].w, "y value");
    Check::Near (segmentBB.point[1].x, testtran.form3d[0][0]*source.point[1].x + testtran.form3d[0][1]*source.point[1].y + testtran.form3d[0][2]*source.point[1].z + testtran.form3d[0][3]*source.point[1].w, "x value");
    Check::Near (segmentBB.point[1].w, source.point[1].w, "w value");
    Check::Near (segmentBB.point[0].x, segmentCC.point[0].x, "x value");
    Check::Near (segmentBB.point[0].z, segmentCC.point[0].z, "z value");
    Check::Near (segmentBB.point[1].y, segmentCC.point[1].y, "y value");
    Check::Near (segmentBB.point[1].w, segmentCC.point[1].w, "w value");
    }
    

//void DSegment4d::FromFractinInterval (DSegment4d parent, double startFraction, double endFraction)
// LINE 219
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test17)
    {
    DSegment4d fract;   
    double testmag;
    double fractmag;
    double startFraction = 0.342;
    double endFraction = 0.784;
    DSegment4d parent = DSegment4d::From (3.7, 4.2, 2.2, 7.9, 1.1, 5.5, 8.3, 5.6);
    fract.FromFractionInterval (parent, startFraction, endFraction);
    double fdx = fract.point[1].x - fract.point[0].x;
    double fdy = fract.point[1].y - fract.point[0].y;
    double fdz = fract.point[1].z - fract.point[0].z;
    double fdw = fract.point[1].w - fract.point[0].w;
    double pdx = parent.point[1].x - parent.point[0].x;
    double pdy = parent.point[1].y - parent.point[0].y;
    double pdz = parent.point[1].z - parent.point[0].z;
    double pdw = parent.point[1].w - parent.point[0].w;
    fractmag = sqrt((fdx*fdx) + (fdy*fdy) + (fdz*fdz) + (fdw*fdw));
    testmag = sqrt((pdx*pdx) + (pdy*pdy) + (pdz*pdz) + (pdw*pdw)) * (endFraction - startFraction); 
    Check::Near (fractmag, testmag, "comparing magnitudes");
    }
    
//bool DSegment4d::FractionParameterToPoint(double fraction)
// LINE 231
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test18)
    {
    //(2.3, 4.2, 5.3, 1.7, 6.4, 8.9, 7, 3.4)
    DSegment4d segment = DSegment4d::From (1.1, 2.2, 5.5, 8.8, 5.6, 4.4, 6.7, 12.3);
    double fraction = 0.5;
    DPoint4d pnt = segment.FractionParameterToPoint(fraction);
    DPoint4d U, V;
    U.DifferenceOf (segment.point[1], segment.point[0]);
    V.DifferenceOf (pnt, segment.point[0]);
    double fract1 = U.DotProduct(V) / U.DotProduct(U);
    Check::Near (fraction, fabs( fract1), "comparing fractions");
    }
#ifdef abc
//bool DSegment4d::FractionParameterToPoint(DPoint3d pnt, double fraction)
// LINE 238
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test19)
    {
    DSegment4d parent = DSegment4d::From (2, 2, 2, 2, 3, 3, 3, 3);
    DPoint3d pnt = DPoint3d::From (0, 0, 0);
    double fraction = 0.5;
    parent.FractionParameterToPoint(pnt, fraction);
    double fractmag;
    double testmag;
    double fdx = pnt.x - parent.point[0].x;
    double fdy = pnt.y - parent.point[0].y;
    double fdz = pnt.z - parent.point[0].z;
    double fdw = 1 - parent.point[0].w;
    double pdx = parent.point[1].x - parent.point[0].x;
    double pdy = parent.point[1].y - parent.point[0].y;
    double pdz = parent.point[1].z - parent.point[0].z;
    double pdw = parent.point[1].w - parent.point[0].w;
    testmag = sqrt((pdx*pdx) + (pdy*pdy) + (pdz*pdz) + (pdw*pdw)) * fraction; 
    fractmag = sqrt((fdx*fdx) + (fdy*fdy) + (fdz*fdz) + (fdw*fdw));
    Check::Near (fractmag, testmag, "comparing magnitudes");
    }
#endif    
//double DSegment4d::PointToFractionParameter (double &param, DPoint3d pt) \
//LINE 245
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test20)
    {
    DSegment4d segment = DSegment4d::From (0, 0, 0, 1, 1, 0, 0, 1);
    DPoint3d pt = DPoint3d::From (0.3, 0, 0);
    double param = 0;
    segment.PointToFractionParameter (param, pt);
    Check::Near (param, 0.3, "checking fraction of segment");
    }
    
//bool DSegment4d::FractionParameterToTangent (DPoint3d &spacepoint, DVec3d tangent, double param)
//LINE 261
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test21)
    {
    DPoint3d pnt = DPoint3d::From (1.4, 3.2, 2.1);
    DSegment4d seg = DSegment4d::From (2.2, 3.4, 4.7, 6.8, 1.1, 5.7, 1.9, 9.3);
    DVec3d tan = DVec3d::From (2.1, 3.2, 5.6);
    double param = 0.37;
    seg.FractionParameterToTangent (pnt, tan, param);
    
    double mx = seg.point[1].x - seg.point[0].x;
    double my = seg.point[1].y - seg.point[0].y;
    double mz = seg.point[1].z - seg.point[0].z;
    double mmx = pnt.x - seg.point[0].x;
    double mmy = pnt.y - seg.point[0].y;
    double mmz = pnt.z - seg.point[0].z;
    double magsegfrac = param * sqrt( mx*mx + my*my + mz*mz);
    double magsegpnt = sqrt( mmx*mmx + mmy*mmy + mmz*mmz);
    Check::Near (pnt.x, seg.point[0].x + param*(seg.point[1].x - seg.point[0].x), "checking x value");
    Check::Near (pnt.y, seg.point[0].y + param*(seg.point[1].y - seg.point[0].y), "checking y value");
    Check::Near (pnt.z, seg.point[0].z + param*(seg.point[1].z - seg.point[0].z), "checking z value");
    Check::Near (magsegfrac, magsegpnt, "comparing magnitudes");
    }
//DPoint4d DSegment4d::FractionParameterToTangent  (DPoint4d spacepoint, DPoint4d &tangent, double param)
//LINE 272
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test22)
    {
    DPoint4d pt = DPoint4d::From (1, 2, 3, 4);
    DSegment4d seg = DSegment4d::From (2, 3, 4, 5, 9, 8, 7, 6);
    DPoint4d tan = DPoint4d::From (3, 6, 7, 5);
    double param = 0.5;
    DPoint4d pnt = seg.FractionParameterToTangent (pt, tan, param);
    
    double mx = seg.point[1].x - seg.point[0].x;
    double my = seg.point[1].y - seg.point[0].y;
    double mz = seg.point[1].z - seg.point[0].z;
    double mw = seg.point[1].w - seg.point[0].w;
    double mmx = pnt.x - seg.point[0].x;
    double mmy = pnt.y - seg.point[0].y;
    double mmz = pnt.z - seg.point[0].z;
    double mmw = pnt.w - seg.point[0].w;
    double magsegfrac = param * sqrt( mx*mx + my*my + mz*mz + mw*mw);
    double magsegpnt = sqrt( mmx*mmx + mmy*mmy + mmz*mmz + mmw*mmw);
    Check::Near (magsegfrac, magsegpnt, "comparing magnitudes");
    Check::Near (tan.x, seg.point[1].x - seg.point[0].x, "checking x value");
    Check::Near (tan.y, seg.point[1].y - seg.point[0].y, "checking y value");
    }
    
//bool DSegment4d::FractionToLength (double &arcLength, double fraction0, double fraction1)
//LINE 285
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test23)
    {
    double arcLength4d = 0.0;
    double arcLength4dtest = 0.0;
    double fraction0 = 0.378;
    double fraction1 = 0.898;
    double n = 2.5;
    DSegment4d seg4d = DSegment4d::From (1.1, 2.4, 5.5, 3.3, 4.4, 6.7, 12.3, 8.8);
    DSegment4d seg4dtest = DSegment4d::From (n*seg4d.point[0].x, n*seg4d.point[0].y, n*seg4d.point[0].z, n*seg4d.point[0].w, n*seg4d.point[1].x, n*seg4d.point[1].y, n*seg4d.point[1].z, n*seg4d.point[1].w);
    seg4d.FractionToLength (arcLength4d, fraction0, fraction1);
    seg4dtest.FractionToLength (arcLength4dtest, fraction0, fraction1);
    Check::Near (arcLength4dtest, arcLength4d, "comparing lengths");
    }

//bool DSegment4d::LengthToFraction (double &fraction1, double fraction0, double arcStep)
//LINE 293
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, Test24)
    {
    double arcStep = 0.7;
    double arcLength = 0.0;
    double fraction0 = 0.0;
    double fraction1 = 0.0;
    double frac = 0.0;
    DSegment4d seg = DSegment4d::From (0,0,0,1, 1,0,0,1);
    seg.LengthToFraction (fraction1, fraction0, arcStep);
    frac = fraction1;
    seg.FractionToLength (arcLength, fraction0, frac);
    Check::Near (arcLength, arcStep, "checking for same lengths");
    }
    
////bool DSegment4d::ClosestPointBoundedXY (DPoint3d closePoint, double closeParam, double distanceXY, DPoint3d spacePoint, DMatrix4dCP worldToLocal, bool extend0, bool extend1) 
////LINE 336
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
//TEST(DSegment4d, Test25)
//    {
//    DSegment4d zeroseg = DSegment4d::From (1.3, 4.3, 5.5, 7.6, zeroseg.point[0].x, zeroseg.point[0].y, zeroseg.point[0].z, zeroseg.point[0].w);
//    DPoint3d closePoint = DPoint3d::From (0, 0, 0);
//    double closeParam = 0.0;
//    double distanceXY = 0.0;
//    DPoint3d spacePoint = DPoint3d::From (2.2, 3.7, 8.8);
//    double ent[16];
//    for (int s = 0; s < 16; s++)
//        {
//        ent[s] = rand();
//        }
//    DMatrix4dCP worldToLocal = DMatrix4d::FromRowValues (ent[0], ent[1], ent[2], ent[3], ent[4], ent[5], ent[6], ent[7], ent[8], ent[9], ent[10], ent[11], ent[12], ent[13], ent[14], ent[15]);
//    bool extend0 = false;
//    bool extend1 = false;
//    zeroseg.ClosestPointBoundedXY (closePoint, closeParam, distanceXY, spacePoint, worldToLocal, extend0, extend1); 
//    }

////bool DSegment4d::ClosestPointBoundedXY (DPoint3d closePoint, double closeParam, double distanceXY, DPoint3d spacePoint, DMatrix4dCP worldToLocal)
////LINE 372
//TEST(DSegment4d, Test26)
//    {
//    
//    }



bool MaxCubicError (double x1, double f1, double x2, double f2, double &xm, double &fm, double &a, double &b)
    {
    double x4;
    bool x3c = false;
    double solution[2];
    
    double x12 = x1 * x1;
    double x13 = x12 * x1;
    double x22 = x2 * x2;
    double x23 = x22 * x2;
    double al1 = x13 - x12;
    double al2 = x23 - x22;
    double bet1 = x12 - x1;
    double bet2 = x22 - x2;
    
    b = f2 / (bet2 - bet1 * (al2 / al1));
    a = (f1 - bet1*b) / al1;
    
    double A = 3 * a;
    double B = 2*b - 2*a;
    double C = -b;
    
    double sqtrm = B*B - 4*A*C;
    
    if (sqtrm >= 0)
        {
        double st = sqrt(sqtrm);
        solution[0] = (-B + st) / (2 * A);
        solution[1] = (-B - st) / (2 * A);
        for (int i = 0; i < 2; i++)
            {
            if (solution[i] > 0 && solution[i] < 1)
                {
                xm = solution[i];
                fm = xm * (xm - 1) * (a * xm + b);
                x3c = true;
                }
            if (solution[i] < 0 && solution[i] > 1)
                {
                x4 = solution[i];
                }           
            }
            return x3c;     
        }        
    else 
        {
        return false;
        }
    
    }

//bool DSegment4d::MaxCubicError (double x1, double f1, double x2, double f2, double &xm, double &fm, double &a, double &b)
//LINE 404
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DSegment4d, Test27)
    {
    Check::StartScope ("MaxCubicError Tests");
    
    double a, b, xm, fm, prt;
    double ac[100];
    double bc[100];

    
    for (int i = 0; i < 20; i++)
        {
        ac[i] = -i - 1;
        bc[i] = -i - 1;
        prt = (i + 1) / 20;
        MaxCubicError (-bc[i] / ac[i], 0, 2, 4*ac[i] + 2*bc[i], xm, fm, a, b);
        Check::True (fabs(prt*(prt-1)*(a*prt + b)) <= fabs(fm), "checking if true local extreme value");
        Check::Near (a, ac[i]);
        Check::Near (b, bc[i]);
        }
    
    bool condition1 =  MaxCubicError (-1, -1, 3, -1, xm, fm, a, b);
    Check::True (condition1, "null");
  
    Check::EndScope ();
    }

bool MaxQuadraticError (double x1, double f1, double &fm)
    {
    double x12 = x1 * x1;
    
    if (x12 - x1 != 0 )
        {
        double al = f1 / (x12 - x1);
        double d2 = 0.5;
        double d4 = d2 * d2;
    
        fm = (d4 - d2)*al;
        return true;
        }
        
    else
        {
        return false;
        }
    }

//bool DSegment4d::MaxQuadraticError (double x1, double f1, double &fm)
//LINE 404
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DSegment4d, Test28)
    {
    Check::StartScope ("MaxQuadraticError Tests");
    
    double x1, fm, prt;
  
    for (int i = 0; i < 20; i++)
        {
        x1 = -i - 1;
        prt = (i + 1) / 20;
        MaxQuadraticError (x1, i, fm);
        Check::True (fabs(prt*(prt-1)*(i / (x1 * (x1 - 1)))) <= fabs(fm), "checking if true local extreme value");
        }

    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DSegment4d, EqualityCheck)
    {
    DSegment4d seg4d = DSegment4d::From(DPoint4d::From(3, 3, 3, 1), DPoint4d::From(10, 4, 6, 1));

    DPoint3d start, end, start2, end2;
    seg4d.GetStartPoint(start);
    seg4d.GetEndPoint(end);
    DSegment4d seg4d2 = DSegment4d::From(start, end);
    seg4d2.GetStartPoint(start2);
    seg4d2.GetEndPoint(end2);
    Check::Near(start, start2);
    Check::Near(end, end2);

    DRange3d range, range2;
    seg4d.GetRange(range);
    seg4d2.GetRange(range2);
    Check::True(range.IsEqual(range2));
    }

void TestVectorToVector (DVec3dCR U, DVec3dCR V)
    {
    double radians;
    Check::StartScope ("AngleAndAxisOfRotationFromVectorToVector");
    DVec3d axis;
    DVec3d zero;
    zero.Zero ();
    Check::False (zero.AngleAndAxisOfRotationFromVectorToVector (V, axis, radians));
    Check::False (U.AngleAndAxisOfRotationFromVectorToVector (zero, axis, radians));
    if (Check::True (U.AngleAndAxisOfRotationFromVectorToVector (V, axis, radians)))
        {
        DVec3d W = DVec3d::FromNormalizedCrossProduct (axis, U);
        DVec3d U1;
        U1.Normalize (U);
        DVec3d Q = DVec3d::FromSumOf (U1, cos(radians), W, sin (radians));
        Check::True (Q.IsParallelTo (V) && Q.DotProduct (V) > 0.0, "Rotate to Parallel");
        RotMatrix rotation;
        Check::True (rotation.InitRotationFromVectorToVector (U, V), "RotMatrix::InitRotationFromVectorToVector");
        DVec3d Q1;
        Q1.Multiply (rotation, U);
        Check::True (Q1.IsParallelTo (V) && Q1.DotProduct (V) > 0.0, "Rotate to Parallel");
        Check::Near (Q1.Magnitude (), U.Magnitude (), "rotation");
        Check::True (rotation.IsOrthogonal ());
        }
    Check::EndScope ();
        
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, RotationFromVectorToVector)
    {
    TestVectorToVector (DVec3d::From (1,0,0), DVec3d::From (0,1,0));
    TestVectorToVector (DVec3d::From (1,0,0), DVec3d::From (1,0,0));
    TestVectorToVector (DVec3d::From (1,0,0), DVec3d::From (-1,0,0));
    TestVectorToVector (DVec3d::From (1,0,0), DVec3d::From (4,5,6));
    TestVectorToVector (DVec3d::From (5,0,0), DVec3d::From (2,3,0));
    TestVectorToVector (DVec3d::From (0,3,0), DVec3d::From (4,5,6));
    TestVectorToVector (DVec3d::From (0,0,2), DVec3d::From (4,5,6));
    TestVectorToVector (DVec3d::From (0,0,-3), DVec3d::From (4,5,6));
    TestVectorToVector (DVec3d::From (1,2,-3), DVec3d::From (4,5,6));
    }

static void testProjectVectorToVector (DVec3dCR spaceVector, DVec3dCR targetVector)
    {
    double f;
    bool stat = spaceVector.ProjectToVector (targetVector, f);
    DVec3d projection = DVec3d::FromScale (targetVector, f);
    DVec3d perp = DVec3d::FromStartEnd (projection, spaceVector);
    if (stat)
        {
        Check::Perpendicular (perp, projection, "projection is perpendicular");
        }
    else
        {
        Check::Near (0.0, targetVector.Magnitude (), "confirm projection failure");
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, ProjectToVector)
    {
    testProjectVectorToVector (DVec3d::From (1,2,3), DVec3d::From (-2,8,4));
    }

static void testProjectVectorToPlane (DVec3dCR spaceVector, DVec3dCR vectorU, DVec3dCR vectorV)
    {
    DPoint2d uv;
    bool stat = spaceVector.ProjectToPlane (vectorU, vectorV, uv);
    DVec3d projection = DVec3d::FromSumOf (vectorU, uv.x, vectorV, uv.y);
    DVec3d perp = DVec3d::FromStartEnd (projection, spaceVector);
    if (stat)
        {
        Check::Perpendicular (perp, projection, "projection is perpendicular");
        }
    else
        {
        Check::Parallel (vectorU, vectorV, "confirm projection failure");
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DVec3d, ProjectToPlane)
    {
    testProjectVectorToPlane (DVec3d::From (3,2,4), DVec3d::From (1,0,0), DVec3d::From (0,1,0));
    testProjectVectorToPlane (DVec3d::From (3,2,4), DVec3d::From (0,0,1), DVec3d::From (0,1,0));
    testProjectVectorToPlane (DVec3d::From (3,2,4), DVec3d::From (0,1,1), DVec3d::From (1,1,0));
    testProjectVectorToPlane (DVec3d::From (3,2,4), DVec3d::From (1,2,1), DVec3d::From (-3,2,4));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DVec3d, CopyFrom4DPoint)
    {
    DVec3d vec3d;
    DPoint4d point4D = DPoint4d::From(3, 4, 5, 1);
    vec3d.XyzOf(point4D);
    Check::ExactDouble(3, vec3d.GetComponent(0));
    Check::ExactDouble(4, vec3d.GetComponent(1));
    Check::ExactDouble(5, vec3d.GetComponent(2));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DVec3d, FromSumOfVectors)
    {
    DVec3d vecRes;
    DVec3d vec0 = DVec3d::From(2, 3, 3);
    DVec3d vec1 = DVec3d::From(3, 7);
    DVec3d vec2 = DVec3d::From(3, 2, 7);
    DVec3d vec3 = DVec3d::From(1, 5, 1);
    double scale1 = 3.1;
    double scale2 = 1.6;
    double scale3 = 4.9;
    vecRes = DVec3d::FromSumOf(vec0, vec1, scale1);
    Check::Near(11.3, vecRes.GetComponent(0));
    Check::Near(24.7, vecRes.GetComponent(1));
    Check::Near(3, vecRes.GetComponent(2));
    vecRes = DVec3d::FromSumOf(vec0, vec1, scale1, vec2, scale2);
    Check::Near(16.1, vecRes.GetComponent(0));
    Check::Near(27.9, vecRes.GetComponent(1));
    Check::Near(14.2, vecRes.GetComponent(2));
    vecRes = DVec3d::FromSumOf(vec0, vec1, scale1, vec2, scale2, vec3, scale3);
    Check::Near(21, vecRes.GetComponent(0));
    Check::Near(52.4, vecRes.GetComponent(1));
    Check::Near(19.1, vecRes.GetComponent(2));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DVec3d, AngleBetweenVectors)
    {
    DVec3d vec1 = DVec3d::From(15, 0, 2);
    DVec3d vec2 = DVec3d::From(15, 15, 4);

    DPoint2d pnt1 = DPoint2d::From(vec1);
    DPoint2d pnt2 = DPoint2d::From(vec2);
    double dotP = pnt1.DotProduct(pnt2);

    double magV1 = pnt2.Magnitude();
    double magV2 = pnt1.Magnitude();

    double cosTheta = dotP / (magV1 * magV2);
    double angleExpected = Angle::Acos(cosTheta);

    double angleReceived = vec1.SmallerUnorientedAngleToXY(vec2);
    Check::Near(angleExpected, angleReceived, "angle Expected and angle Received unequal");

    vec1.Negate();
    angleReceived = vec1.SmallerUnorientedAngleToXY(vec2);
    Check::Near(angleExpected, angleReceived, "Smaller angle not returned");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DVec3d, SmallerUnorientedAngle)
    {
    DVec3d vector0 = DVec3d::From (2,1,-3);
    DVec3d vectorX, vectorY, vectorZ;
    vector0.GetNormalizedTriad (vectorX, vectorY, vectorZ);
    auto factors = bvector<DPoint2d> 
        {
        DPoint2d::From (1,1),
        DPoint2d::From (-1,1),
        DPoint2d::From (1,-1),
        DPoint2d::From (-1, -1),
        };
    for (auto degrees : {0, 1, 10,30,45,50, 80, 89})
        {
        auto theta = Angle::FromDegrees (degrees);
        auto c = theta.Cos ();
        auto s = theta.Sin ();
//        auto r0 = atan2 (s, c);
        auto r1 = theta.Radians ();
        for (auto scale : factors)
            {
            auto vectorXZ = DVec3d::FromSumOf (vectorZ, scale.x * c, vectorX, scale.y * s);
            auto r2 = vectorXZ.SmallerUnorientedAngleTo (vector0);
            //auto r3 = vector0.AngleTo (vectorXZ);
            //Check::Near (r3, r1);
            Check::Near (r1,r2, "SmallerUnorientedAngle");            
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DVec3d, CheckSector)
    {
    DVec3d vecBoundary1 = DVec3d::From(-15, 16, 3);
    DVec3d vecBoundary2 = DVec3d::From(15, 14, 0);

    DVec3d testVector = DVec3d::From(14, 0, 0);
    Check::True(testVector.IsVectorInCCWXYSector(vecBoundary1, vecBoundary2));

    DVec3d testVector2 = DVec3d::From(0, 15, 0);
    Check::False(testVector2.IsVectorInCCWXYSector(vecBoundary1, vecBoundary2));

    DVec3d resultant = DVec3d::FromSumOf(vecBoundary1, vecBoundary2, 2);
    Check::True(resultant.IsVectorInCCWXYSector(vecBoundary2, vecBoundary1));
    }

void test_DVec3dRotate90TowardsVector(DVec3dCR target, DVec3dCR vector) 
    {
    DVec3d result = vector;
    double theta = target.AngleTo(vector);
    for (int i = 0; i < 2; i++)
        {
        DVec3d normal0 = DVec3d::FromCrossProduct (target, result);
        result = DVec3d::FromRotate90Towards(result, target);
        Check::Near(result.Magnitude(), vector.Magnitude(), "rotate90TowardsTarget maintains magnitude");
        DVec3d normal1 = DVec3d::FromCrossProduct (target, result);

        normal1.Negate();
        Check::Parallel (normal0, normal1, "parallel negated normal");
        }
    Check::Near (theta, target.AngleTo (result), "maintain the original angle with target");
    Check::Near(result, vector);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DVec3d, RotateVectorTowardsTarget)
    {
    DVec3d vector = DVec3d::From(15, 8, 9);

    test_DVec3dRotate90TowardsVector(DVec3d::From(1, 6, 8), vector);
    test_DVec3dRotate90TowardsVector(DVec3d::From(1, 24, 8), vector);
    test_DVec3dRotate90TowardsVector(DVec3d::From(1, 0, 9.2), vector);
    test_DVec3dRotate90TowardsVector(DVec3d::From(11.3, 0.7, 12.4), vector);
    test_DVec3dRotate90TowardsVector(DVec3d::From(23.3, 18.2, 21.004), vector);

    //Check::Near(result, vector);
    /*DVec3d target = DVec3d::From(0, 8, 10);
    */
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DVec3d, RotateVector90AroundAxis)
    {
    DVec3d vector = DVec3d::From(15, 8, 9);
    DVec3d axis = DVec3d::From(0, 1, 0);
    double theta = axis.AngleTo(vector);
    DVec3d result = vector;

    for (int i = 0; i < 4; i++)
        {
        DVec3d normal0 = DVec3d::FromCrossProduct (axis, result);
        result = DVec3d::FromRotate90Around(result, axis);
        Check::Near(result.Magnitude(), vector.Magnitude(), "rotate90 maintains magnitude");
        Check::Near (theta, axis.AngleTo (result), "rotate90 maintains angle");
        DVec3d normal1 = DVec3d::FromCrossProduct (axis, result);
        Check::Perpendicular (normal0, normal1, "rotate90");
        }
    Check::Near(result, vector);
    }
void testRotateVectorAroundVectorByAngle (DVec3dCR vector, DVec3dCR axis, Angle angle)
    {
    ValidatedDVec3d result = DVec3d::FromRotateVectorAroundVector (vector, axis, angle);
    bool isParallel = vector.IsParallelTo (axis);
    if (Check::Bool (!isParallel, result.IsValid ()))
        {
        DVec3d vector1 = result.Value ();
        double radians1 = vector.PlanarAngleTo (vector1, axis);
        Check::True (Angle::NearlyEqualAllowPeriodShift (angle.Radians (), radians1), "rotation angle in plane perp to axis");
        Check::Near (axis.AngleTo (vector), axis.AngleTo (vector1), "angle from rotation axis");
        Check::Near (vector.Magnitude (), vector1.Magnitude (), "rotation does not change magnitude");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(DVec3d,RotateVectorAroundVectorByAngle)
    {
    testRotateVectorAroundVectorByAngle (DVec3d::From (1,0,0), DVec3d::From (0,0,1), Angle::FromDegrees (25.0));
    testRotateVectorAroundVectorByAngle (DVec3d::From (1,0,0), DVec3d::From (0,1,0), Angle::FromDegrees (-49.0));
    testRotateVectorAroundVectorByAngle (DVec3d::From (1,2,4), DVec3d::From (5,-2,1), Angle::FromDegrees (25.2));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DVec3d, OperatorOverload)
    {
    DVec3d vec0 = DVec3d::From(4, 2, 1);
    DVec3d vec1 = DVec3d::From(5, 2, 8);
    DVec3d sum;
    sum = vec0 + vec1;
    vec0 += vec1;
    Check::True(sum == vec0);

    DVec3d sub;
    sub = vec0 - vec1;
    sum -= vec1;
    Check::True(sum == sub);
    }
//END_BENTLEY_NAMESPACE
