/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, InitIdentity)
    {
    RotMatrix matrix0, matrix2, matrix4, matrix6, matrix10, matrix12,  matrix16, matrix18, matrix20, matrix22, matrix24, matrix26, matrix28, matrix30;
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    RotMatrix matrix3 = RotMatrix::FromRowValues (2.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 4.0);
    RotMatrix matrix5 = RotMatrix::FromRowValues (5.0, 0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 0.0, 5.0);
    RotMatrix matrix7 = RotMatrix::FromRowValues (8.0, 10.0, 12.0, 16.0, 20.0, 24.0, 24.0, 30.0, 36.0);
    RotMatrix matrix9 = RotMatrix::FromRowValues (2.0, 3.0, 4.0, 2.0, 3.0, 4.0, 2.0, 3.0, 4.0);
    RotMatrix matrix11 = RotMatrix::FromRowValues (1.0, 4.0, 7.0, 2.0, 5.0, 8.0, 3.0, 6.0, 9.0);
    RotMatrix matrix15 = RotMatrix::FromRowValues (1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);
    RotMatrix matrix17 = RotMatrix::FromRowValues (2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    RotMatrix matrix19 = RotMatrix::FromRowValues (1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0);
    RotMatrix matrix21 = RotMatrix::FromRowValues (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    DVec3d vector0 = DVec3d::From (1.0, 2.0, 3.0);
    DVec3d vector1 = DVec3d::From (4.0, 5.0, 6.0);
    DVec3d vector2 = DVec3d::From (7.0, 8.0, 9.0);
    DVec3d vector3 = DVec3d::From (1.0, 0.0, 0.0);
    DVec3d vector4 = DVec3d::From (0.0, 1.0, 0.0);
    DVec3d vector5, vector6, vector7;
    vector5.Normalize (vector0);
    vector6.Normalize (vector1);
    vector7.NormalizedCrossProduct (vector5, vector6);
    vector6.NormalizedCrossProduct (vector7, vector5);
    RotMatrix matrix23 = RotMatrix::FromColumnVectors (vector5, vector6, vector7);
    DPoint3d point0 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point1 = DPoint3d::From (4.0, 5.0, 6.0);
    DPoint3d point2 = DPoint3d::From (0.0, 0.0, 0.0);
    double scale = 2.0;
    double xVector[3], yVector[3], zVector[3];
    xVector[0] = xVector[1] = xVector[2] = 2.0;
    yVector[0] = yVector[1] = yVector[2] = 3.0;
    zVector[0] = zVector[1] = zVector[2] = 4.0;
    double radian = PI/2;
    bool normalize = false;
    Transform transform = Transform::FromIdentity ();
    matrix0.InitIdentity ();
    matrix2.InitFromScaleFactors (2.0, 3.0, 4.0);
    matrix4.InitFromScale (5.0);
    matrix6.InitFromScaledOuterProduct (vector0, vector1, scale);
    matrix10.InitFromRowValues (2.0, 3.0, 4.0, 2.0, 3.0, 4.0, 2.0, 3.0, 4.0);
    matrix12.InitFromColumnVectors (vector0, vector1, vector2);
    matrix16.InitFromVectorAndRotationAngle (vector3, radian);
    matrix18.InitFromDirectionAndScale (vector3, scale);
    matrix20.InitFromAxisAndRotationAngle (0, radian);
    matrix22.InitFromPrincipleAxisRotations (matrix15, radian, 0.0, 0.0);
    Check::True (matrix24.InitFrom1Vector (vector3, 0, normalize));
    matrix26.InitFrom2Vectors (vector3, vector4);
    Check::True (matrix28.InitRotationFromOriginXY (point2, point0, point1));
    matrix30.InitFrom (transform);
    Check::Near (matrix1,  matrix0);
    Check::Near (matrix3,  matrix2);
    Check::Near (matrix5,  matrix4);
    Check::Near (matrix7,  matrix6);
    Check::Near (matrix9,  matrix10);
    Check::Near (matrix11, matrix12);
    Check::Near (matrix15, matrix16);
    Check::Near (matrix17, matrix18);
    Check::Near (matrix15, matrix20);
    Check::Near (matrix19, matrix22);
    Check::Near (matrix21, matrix24);
    Check::Near (matrix21, matrix26);
    Check::Near (matrix23, matrix28);
    Check::Near (matrix0,  matrix30);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, Multiply)
    {
    RotMatrix matrix0 = RotMatrix::FromRowValues (8.0, 10.0, 12.0, 16.0, 20.0, 24.0, 24.0, 30.0, 36.0);
    DPoint4d point0[3];
    DPoint4d point1[3];
    point1[0] = DPoint4d::From (1.0, 2.0, 3.0, 1.0);
    point1[1] = DPoint4d::From (4.0, 5.0, 6.0, 1.0);
    point1[2] = DPoint4d::From (7.0, 8.0, 9.0, 1.0);
    DPoint4d point2 = DPoint4d::From (64.0, 128.0, 192.0, 1.0);
    DPoint4d point3 = DPoint4d::From (154.0, 308.0, 462.0, 1.0);
    DPoint4d point4 = DPoint4d::From (244.0, 488.0, 732.0, 1.0);
    DPoint2d point5[3];
    DPoint2d point6[3];
    point6[0] = DPoint2d::From (1.0, 2.0);
    point6[1] = DPoint2d::From (4.0, 5.0);
    point6[2] = DPoint2d::From (7.0, 8.0);
    DPoint2d point7 = DPoint2d::From (28.0, 56.0);
    DPoint2d point8 = DPoint2d::From (82.0, 164.0);
    DPoint2d point9 = DPoint2d::From (136.0, 272.0);
    DRange3d range0;
    DRange3d range1 = DRange3d::FromMinMax (0.0, 1.0);
    DPoint3d point10[8];
    point10[0] = DPoint3d::From (0.0, 0.0, 0.0);
    point10[1] = DPoint3d::From (8.0, 16.0, 24.0);
    point10[2] = DPoint3d::From (10.0, 20.0, 30.0);
    point10[3] = DPoint3d::From (18.0, 36.0, 54.0);
    point10[4] = DPoint3d::From (12.0, 24.0, 36.0);
    point10[5] = DPoint3d::From (20.0, 40.0, 60.0);
    point10[6] = DPoint3d::From (22.0, 44.0, 66.0);
    point10[7] = DPoint3d::From (30.0, 60.0, 90.0);
    DRange3d range2;
    DPoint3d point11 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point12 = DPoint3d::From (64.0, 128.0, 192.0);
    DPoint3d point13, point15, point17, point19, point28[3];
    DPoint3d point14 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point16 = DPoint3d::From (112.0, 140.0, 168.0);
    DPoint3d point18 = DPoint3d::From (1.0, 2.0, 3.0);
    range2.InitFrom (point10, 8);
    DPoint3d point20[3];
    DPoint3d point21[3];
    point21[0] = DPoint3d::From (1.0, 2.0, 3.0);
    point21[1] = DPoint3d::From (4.0, 5.0, 6.0);
    point21[2] = DPoint3d::From (7.0, 8.0, 9.0);
    DPoint3d point22 = DPoint3d::From (64.0, 128.0, 192.0);
    DPoint3d point23 = DPoint3d::From (154.0, 308.0, 462.0);
    DPoint3d point24 = DPoint3d::From (244.0, 488.0, 732.0);
    DPoint3d point25 = DPoint3d::From (112.0, 140.0, 168.0);
    DPoint3d point26 = DPoint3d::From (256.0, 320.0, 384.0);
    DPoint3d point27 = DPoint3d::From (400.0, 500.0, 600.0);
    matrix0.Multiply (point0, point1, 3);
    matrix0.Multiply (point5, point6, 3);
    matrix0.Multiply (range0, range1);
    matrix0.Multiply (point11);
    matrix0.Multiply (point13, point14);
    matrix0.MultiplyTranspose (point15, point14);
    matrix0.MultiplyComponents (point17, 1.0, 2.0, 3.0);
    matrix0.MultiplyTranspose (point18);
    matrix0.MultiplyTransposeComponents (point19, 1.0, 2.0, 3.0);
    matrix0.Multiply (point20,  point21, 3);
    matrix0.MultiplyTranspose (point28, point21, 3);
    Check::Near (point2, point0[0]);
    Check::Near (point3, point0[1]);
    Check::Near (point4, point0[2]);
    Check::Near (point7, point5[0]);
    Check::Near (point8, point5[1]);
    Check::Near (point9, point5[2]);
    Check::Near (range2, range0);
    Check::Near (point12, point11);
    Check::Near (point12, point13);
    Check::Near (point16, point15);
    Check::Near (point12, point17);
    Check::Near (point16, point18);
    Check::Near (point16, point19);
    Check::Near (point22, point20[0]);
    Check::Near (point23, point20[1]);
    Check::Near (point24, point20[2]);
    Check::Near (point25, point28[0]);
    Check::Near (point26, point28[1]);
    Check::Near (point27, point28[2]);
    DRange3d rangeNew = DRange3d::NullRange();
    matrix0.Multiply (range0, rangeNew);
    Check::True(range0.IsNull());
    Check::Near(range0, range1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, InitProduct)
    {
    RotMatrix matrix0 = RotMatrix::FromRowValues (1.0, 2.0, 3.0, 2.0, 3.0, 1.0, 3.0, 1.0, 2.0);
    RotMatrix matrix1 = RotMatrix::FromRowValues (4.0, 5.0, 6.0, 5.0, 6.0, 4.0, 6.0, 4.0, 5.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (32.0, 29.0, 29.0, 29.0, 32.0, 29.0, 29.0, 29.0, 32.0);
    RotMatrix matrix3, matrix5, matrix6, matrix7;
    RotMatrix matrix4 = RotMatrix::FromIdentity ();
    Transform transform0 = Transform::FromIdentity ();
    matrix3.InitProduct (matrix0, matrix1);
    matrix5.InitProduct (matrix0, matrix1, matrix4);
    matrix6.InitProduct (matrix0, transform0);
    matrix7.InitProduct (transform0, matrix0);
    Check::Near (matrix2, matrix3);
    Check::Near (matrix2, matrix5);
    Check::Near (matrix0, matrix6);
    Check::Near (matrix0, matrix7);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, Zero)
    {
    RotMatrix matrix0;
    RotMatrix matrix1 = RotMatrix::FromRowValues (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    RotMatrix matrix2 = RotMatrix::FromIdentity ();
    RotMatrix matrix3 = RotMatrix::FromRowValues (9.0, 10.0, 12.0, 16.0, 21.0, 24.0, 24.0, 30.0, 37.0);
    DVec3d vector0 = DVec3d::From (1.0, 2.0, 3.0);
    DVec3d vector1 = DVec3d::From (4.0, 5.0, 6.0);
    double scale = 2.0;
    matrix0.Zero ();
    matrix2.AddScaledOuterProductInPlace (vector0, vector1, scale);
    Check::Near (matrix1, matrix0);
    Check::Near (matrix3, matrix2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, TransposeOf)
    {
    RotMatrix matrix0;
    RotMatrix matrix1 = RotMatrix::FromRowValues (9.0, 10.0, 12.0, 16.0, 21.0, 24.0, 24.0, 30.0, 37.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (9.0, 16.0, 24.0, 10.0, 21.0, 30.0, 12.0, 24.0, 37.0);
    matrix0.TransposeOf (matrix1);
    Check::Near (matrix2, matrix0);
    matrix1.Transpose ();
    Check::Near (matrix2, matrix1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, InverseOf)
    {
    RotMatrix matrix0, matrix3;
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (1.0, 1.0, 2.0, 1.0, 2.0, 4.0, 2.0, -1.0, 0.0);
    RotMatrix matrix4 = RotMatrix::FromRowValues (2.0, -1.0, 0.0, 4.0, -2.0, -1.0, -2.5, 1.5, 0.5);
    Check::True (!(matrix0.InverseOf (matrix1)));
    Check::True (matrix3.InverseOf (matrix2));
    Check::Near (matrix4, matrix3);
    Check::True (!(matrix1.Invert ()));
    Check::True (matrix2.Invert ());
    Check::Near (matrix4, matrix2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, ScaleRows)
    {
    RotMatrix matrix0, matrix4, matrix5;
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (2.0, 4.0, 6.0, 12.0, 15.0, 18.0, 28.0, 32.0, 36.0);
    RotMatrix matrix3 = RotMatrix::FromRowValues (2.0, 6.0, 12.0, 8.0, 15.0, 24.0, 14.0, 24.0, 36.0);
    RotMatrix matrix6 = RotMatrix::FromRowValues (4.0, 5.0, 6.0, 5.0, 6.0, 4.0, 6.0, 4.0, 5.0);
    RotMatrix matrix7;
    matrix7.InitProduct (matrix3, matrix6);
    double scale1 = 2.0;
    double scale2 = 3.0;
    double scale3 = 4.0;
    matrix0.ScaleRows (matrix1, scale1, scale2, scale3);
    matrix4.ScaleColumns (matrix1, scale1, scale2, scale3);
    matrix5.Scale (matrix1, scale1, scale2, scale3, matrix6);
    Check::Near (matrix2, matrix0);
    Check::Near (matrix3, matrix4);
    Check::Near (matrix7, matrix5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, GetColumns)
    {
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    DVec3d vec0, vec1, vec2, vec3, vec4, vec5;
    DVec3d vec3dX0 = DVec3d::From (1.0, 4.0, 7.0);
    DVec3d vec3dY0 = DVec3d::From (2.0, 5.0, 8.0);
    DVec3d vec3dZ0 = DVec3d::From (3.0, 6.0, 9.0);
    DVec3d vec3dX1 = DVec3d::From (1.0, 2.0, 3.0);
    DVec3d vec3dY1 = DVec3d::From (4.0, 5.0, 6.0);
    DVec3d vec3dZ1 = DVec3d::From (7.0, 8.0, 9.0);
    double x00, x01, x02, x10, x11, x12, x20, x21, x22;
    matrix1.GetColumns (vec0, vec1, vec2);
    matrix1.GetRows (vec3, vec4, vec5);
    matrix1.GetRowValues (x00, x01, x02, x10, x11, x12, x20, x21, x22);
    Check::Near (vec3dX0, vec0);
    Check::Near (vec3dY0, vec1);
    Check::Near (vec3dZ0, vec2);
    Check::Near (vec3dX1, vec3);
    Check::Near (vec3dY1, vec4);
    Check::Near (vec3dZ1, vec5);
    Check::Near (x00, 1.0);
    Check::Near (x01, 2.0);
    Check::Near (x02, 3.0);
    Check::Near (x10, 4.0);
    Check::Near (x11, 5.0);
    Check::Near (x12, 6.0);
    Check::Near (x20, 7.0);
    Check::Near (x21, 8.0);
    Check::Near (x22, 9.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, SetColumn)
    {
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    DVec3d vec0 = DVec3d::From (10, 11, 12);
    DVec3d vec1, vec3;
    DVec3d vec2 = DVec3d::From (10, 11, 12);
    matrix1.SetColumn (vec0, 0);
    matrix1.GetColumn (vec1, 0);
    double number = matrix1.GetComponentByRowAndColumn (0, 3);
    matrix2.SetRow (vec2, 0);
    matrix2.GetRow (vec3, 0);
    Check::Near (vec0, vec1);
    Check::Near (10.0, number);
    Check::Near (vec2, vec3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, NormalizeRowsOf)
    {
    RotMatrix matrix0, matrix3;
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (1.0 / (sqrt (14.0)), 2.0 / (sqrt (14.0)), 3.0 / (sqrt (14.0)), 4.0 / (sqrt (77.0)), 5.0 / (sqrt (77.0)), 6.0 / (sqrt (77.0)), 7.0 / (sqrt (194.0)), 8.0 / (sqrt (194.0)), 9.0 / (sqrt (194.0)));
    RotMatrix matrix4 = RotMatrix::FromRowValues (1.0 / (sqrt (66.0)), 2.0 / (sqrt (93.0)), 3.0 / (sqrt (126.0)), 4.0 / (sqrt (66.0)), 5.0 / (sqrt (93.0)), 6.0 / (sqrt (126.0)), 7.0 / (sqrt (66.0)), 8.0 / (sqrt (93.0)), 9.0 / (sqrt (126.0)));
    DVec3d scaleVector0, scaleVector2;
    DVec3d scaleVector1 = DVec3d::From (sqrt (14.0), sqrt (77.0), sqrt (194.0));
    DVec3d scaleVector3 = DVec3d::From (sqrt (66.0), sqrt (93.0), sqrt (126.0));
    matrix0.NormalizeRowsOf (matrix1, scaleVector0);
    matrix3.NormalizeColumnsOf (matrix1, scaleVector2);
    double d = matrix1.Determinant ();
    Check::Near (scaleVector1, scaleVector0);
    Check::Near (matrix2, matrix0);
    Check::Near (scaleVector3, scaleVector2);
    Check::Near (matrix4, matrix3);
    Check::Near (0.0, d);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, ConditionNumber)
    {
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 5.0, 4.0, 5.0, 7.0, 7.0, 8.0, 11.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    RotMatrix matrix3 = RotMatrix::FromIdentity ();
    double result1 = matrix1.ConditionNumber ();
    double result2 = matrix2.ConditionNumber ();
    Check::True (!(matrix1.IsIdentity ()));
    Check::True (matrix3.IsIdentity ());
    Check::Near (6.0 / ((sqrt (66.0)) * (sqrt (93.0)) * (sqrt (195.0))), result1);
    Check::Near (0.0, result2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, IsDiagonal)
    {
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 5.0, 4.0, 5.0, 7.0, 7.0, 8.0, -11.0);
    RotMatrix matrix2 = RotMatrix::FromIdentity ();
    double maxScale0 = 0.0, maxScale1 = 0.0, minValue = 0.0, maxValue = 0.0, minValue1 = 0.0, maxValue1 = 0.0, minValue2 = 0.0, maxValue2 = 0.0, minValue3 = 0.0, maxValue3 = 0.0;
    Check::True (!(matrix1.IsDiagonal ()));
    Check::True (matrix2.IsDiagonal ());
    Check::True (!(matrix1.IsUniformScale (maxScale0)), "confirm nonuniform scale");
    Check::True (matrix2.IsUniformScale (maxScale1), " confirm uniform scale");
    matrix1.DiagonalSignedRange (minValue, maxValue);
    matrix1.DiagonalAbsRange (minValue1, maxValue1);
    matrix1.OffDiagonalSignedRange (minValue2, maxValue2);
    matrix1.OffDiagonalAbsRange (minValue3, maxValue3);
    Check::Near (-11.0, maxScale0);
    Check::Near (1.0, maxScale1);
    Check::Near (-11.0, minValue);
    Check::Near (5.0, maxValue);
    Check::Near (1.0, minValue1);
    Check::Near (11.0, maxValue1);
    Check::Near (2.0, minValue2);
    Check::Near (8.0, maxValue2);
    Check::Near (2.0, minValue3);
    Check::Near (8.0, maxValue3);
    for (double s0 : bvector<double> {1,2,0.5, -2, -1, -0.5})
        {
        RotMatrix matrix4 = RotMatrix::FromIdentity ();
        matrix4.ScaleColumns (s0, s0, s0);
        double s1;
        Check::True (matrix4.IsUniformScale (s1), "mirror scale");
        Check::Near (s0, s1, "scale of principal mirror and scale");
        RotMatrix column2;
        double s2;
        Check::True (matrix4.IsRigidSignedScale (column2, s2), "rigid signed scale");
        Check::Near (s0, s2, "scale of RigidSignedScale");
        double s3;
        RotMatrix column3;
        // Confirm that IsRigidScale accept/reject matches sign
        Check::Bool (s0 > 0, matrix4.IsRigidScale (column3, s3), "Rigid Positive Scale");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, IsSignedPermutation)
    {
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 2.0, 5.0, 4.0, 5.0, 7.0, 7.0, 8.0, -11.0);
    RotMatrix matrix2 = RotMatrix::FromIdentity ();
    RotMatrix matrix3, matrix4, matrix6, matrix7;
    RotMatrix matrix5 = RotMatrix::FromRowValues (2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0);
    DVec3d axisScales0, axisScales1;
    DVec3d normal0 = DVec3d::From (0.0, 0.0, 1.0);
    double axisRatio0 = 0.0, axisRatio1 = 0.0, scale0 = 0.0, scale1 = 0.0;
    Check::True (!(matrix1.IsSignedPermutation ()));
    Check::True (matrix2.IsSignedPermutation ());
    Check::True (!(matrix1.IsRigid ()));
    Check::True (matrix2.IsRigid ());
    Check::True (!(matrix1.IsOrthogonal ()));
    Check::True (matrix2.IsOrthogonal ());
    Check::True (!(matrix1.IsOrthonormal (matrix3, axisScales0, axisRatio0)));
    Check::True (matrix2.IsOrthonormal (matrix4, axisScales1, axisRatio1));
    Check::True (!(matrix1.IsRigidScale (matrix6, scale0)));
    Check::True (matrix5.IsRigidScale (matrix7, scale1), "confirm rigid scale");
    Check::True (!(matrix1.IsPlanar (normal0)));
    Check::True (matrix2.IsPlanar (normal0));
    Check::Near (matrix2, matrix4);
    Check::Near (DVec3d::From (1.0, 1.0, 1.0), axisScales1);
    Check::Near (1.0, axisRatio1);
    Check::Near (matrix2, matrix7);
    Check::Near (2.0, scale1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, SquareAndNormalizeColumns)
    {
    RotMatrix matrix0, matrix3, matrix4, matrix5;
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 3.0, 3.0, 3.0);
    RotMatrix matrix2 = RotMatrix::FromIdentity ();
    Check::True (!(matrix0.SquareAndNormalizeColumns (matrix1, 0, 1)));
    Check::True (matrix3.SquareAndNormalizeColumns (matrix2, 0, 1));
    Check::True (!(matrix4.SquareAndNormalizeColumnsAnyOrder (matrix1)));
    Check::True (matrix5.SquareAndNormalizeColumnsAnyOrder (matrix2));
    }

// Check that B is an orthgonal XY squareup of A
void CheckSquareUp (RotMatrixCR A, int expectedOrientation)
    {
    RotMatrix B;
    B.SquareAndNormalizeColumns (A, 0, 1, expectedOrientation);
    Check::StartScope ("Squareup");
    double a = A.Determinant ();
    double b = B.Determinant ();
    double s = expectedOrientation;
    Check::True (B.IsOrthogonal (), "orthogonal");
    if (expectedOrientation == 0)
        s = a;
    Check::True (s * b > 0.0, "oriented");
    RotMatrix ATB;
    ATB.InitProductRotMatrixTransposeRotMatrix (A, B);
    double tol = Angle::SmallAngle () * A.MaxAbs ();
    Check::True (ATB.form3d[0][0] > 0.0, "old X positive dot new X");
    Check::True (fabs (ATB.form3d[0][1]) <= tol, "old X perp new Y");
    Check::True (fabs (ATB.form3d[0][2]) <= tol, "old X perp new Z");
    // [1,0] is arbitrary
    Check::True (ATB.form3d[1][1] > 0.0, "oldY in positive Y" );
    Check::True (fabs (ATB.form3d[1][2]) <= tol, "oldY perp new Z");
    // [2][0] and [2][1] are arbitrary
    // [2][2] only significant when preserving orientation.
    if (expectedOrientation == 0)
        Check::True (ATB.form3d[2][2] > 0.0);
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, SquareAndNormalizeColumnsA)
    {
    DVec3d U = DVec3d::From (1,2,3);
    DVec3d V = DVec3d::From (5,-2,3);
    DVec3d W = DVec3d::FromCrossProduct (U, V);
    RotMatrix A0= RotMatrix::FromColumnVectors (U, V, W);
    RotMatrix A1 = A0;
    A1.ScaleColumns (1,1,-1);  // This is left handed!!!

    CheckSquareUp (A0, 1);
    CheckSquareUp (A0, 0);
    CheckSquareUp (A0, -1);

    CheckSquareUp (A1, 1);
    CheckSquareUp (A1, 0);
    CheckSquareUp (A1, -1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, ShuffleColumnsOf)
    {
    RotMatrix matrix0, matrix6, matrix9;
    RotMatrix matrix1 = RotMatrix::FromRowValues (1.0, 5.0, 3.0, 8.0, 6.0, 9.0, 4.0, 1.0, 2.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (3.0, 5.0, 1.0, 9.0, 6.0, 8.0, 2.0, 1.0, 4.0);
    RotMatrix matrix3 = RotMatrix::FromRowValues (2.0, 5.0, 7.0, 3.0, 1.0, 9.0, 4.0, 6.0, 10.0);
    RotMatrix matrix4 = RotMatrix::FromRowValues (5.0, 2.0, 6.0, 9.0, 10.0, 3.0, 11.0, 12.0, 7.0);
    RotMatrix matrix5 = RotMatrix::FromRowValues (10.0, 17.0, 23.0, 23.0, 18.0, 30.0, 23.0, 25.0, 29.0);
    RotMatrix matrix7 = RotMatrix::FromRowValues (6.0, 7.0, 9.0, 17.0, 16.0, 12.0, 15.0, 13.0, 9.0);
    RotMatrix matrix8 = RotMatrix::FromRowValues (3.0, -3.0, -1.0, 6.0, 9.0, -6.0, 7.0, 6.0, -3.0);
    RotMatrix matrix10 = RotMatrix::FromRowValues (3.0, -3.0, -1.0, 6.0, 9.0, -6.0, 7.0, 6.0, -3.0);
    matrix0.ShuffleColumnsOf (matrix1, 2, 1, 0);
    matrix6.SumOf (matrix1, matrix3, 2.0, matrix4, 1.0);
    Check::Near (matrix2, matrix0);
    Check::Near (matrix5, matrix6);
    matrix1.Add (matrix4);
    Check::Near (matrix7, matrix1);
    matrix4.Subtract (matrix3);
    Check::Near (matrix8, matrix4);
    double sum0 = matrix8.SumSquares ();
    Check::Near (266.0, sum0);
    double maxAbs = matrix8.MaxAbs ();
    Check::Near (9.0, maxAbs);
    double maxDiff = matrix7.MaxDiff (matrix8);
    Check::Near (18.0, maxDiff);
    double sumOfDiagonalSquares = matrix8.SumDiagonalSquares ();
    Check::Near (99.0, sumOfDiagonalSquares);
    double sumOffDiagonalSquares = matrix8.SumOffDiagonalSquares ();
    Check::Near (167.0, sumOffDiagonalSquares);
    matrix9.Copy (matrix8);
    Check::Near (matrix8, matrix9);
    Check::True (!(matrix7.IsEqual (matrix8)));
    Check::True (matrix8.IsEqual (matrix10));
    Check::True (!(matrix7.IsEqual (matrix8, 1.0e-12)));
    Check::True (matrix8.IsEqual (matrix10, 1.0e-12));
    }

//TEST(RotMatrix, GivensRowOp)
//    {
//    RotMatrix matrix0;
//    RotMatrix matrix1 = RotMatrix::FromRowValues (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
//    matrix0.GivensRowOp (0.0, -1.0, 0, 1, matrix1);
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, Solve)
    {
    RotMatrix matrix1 = RotMatrix::FromRowValues (0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
    RotMatrix matrix2 = RotMatrix::FromRowValues (3.0, 5.0, 1.0, 9.0, 6.0, 8.0, 2.0, 1.0, 4.0);
    DPoint3d point0, point2, point4, point5;
    DVec3d point1 = DVec3d::From (1.0, 2.0, 3.0);
    DVec3d point3 = DVec3d::From (-16.0 / 11.0, 9.0 / 11.0, 14.0 / 11.0);
    DVec3d point6 = DVec3d::From (3.0 / 5.0, -2.0 / 5.0, 7.0 / 5.0);
    Check::True (!(matrix1.Solve (point0, point1)));
    Check::True (matrix2.Solve (point2, point1));
    Check::True (!(matrix1.SolveTranspose (point4, point1)));
    Check::True (matrix2.SolveTranspose (point5, point1));
    Check::Near (point1, point0);
    Check::Near (point3, point2);
    Check::Near (point1, point4);
    Check::Near (point6, point5);
    }


void TestMultiplication (RotMatrixCR A, double x, double y, double z)
    {
    RotMatrix AT, AT1;
    AT = A;
    AT.Transpose ();
    AT1.TransposeOf (A);
    Check::Near (AT, AT1);


    DPoint3d X = DPoint3d::From (x, y, z);
    //DPoint3d X0 = DPoint3d::From (x, y, 0.0);
    //DPoint2d Y = DPoint2d::From (x, y);
    DPoint3d AX, AX_components, ATTX, ATTX_components;

    double a = 2.4534;
    // Arrays with xyz then a*xyz.  All these arrays have same length.
    DPoint3d Xa [] = { {x, y, z}, {a*x, a*y, a*z}};
    DPoint3d X0a [] = { {x, y, 0.0}, {a*x, a*y, 0.0}};
    DPoint2d Ya [] = { {x, y}, {a*x, a*y} };

    int numXa = sizeof(Xa) / sizeof (X);
    DPoint2d AYa [10];
    DPoint3d AXa [10], AX0a [10], ATTXa[10];
    
    A.Multiply (AYa, Ya, numXa);
    A.Multiply (AXa, Xa, numXa);
    A.Multiply (AX0a, X0a, numXa);
    for (int i = 0; i < numXa; i++)
        Check::Near (AYa[i], AX0a[i].x, AX0a[i].y);

    DPoint3d ATX, ATX1;
    A.MultiplyTranspose (ATX, X);
    AT.Multiply (ATX1, X);
    Check::Near (ATX, ATX1);

    DPoint3d AX_inplace = X;  A.Multiply (AX_inplace);
    DPoint3d ATTX_inplace = X; AT.MultiplyTranspose (ATTX_inplace);
    A.Multiply (AX, X);
    AT.MultiplyTranspose (ATTX, X);
    Check::Near (AX, ATTX, "A*X == (AT)T*X");
    Check::Near (AX, ATTX_inplace, "A*X == (AT)T*X inplace");
    A.MultiplyComponents (AX_components, x, y, z);
    AT.MultiplyTransposeComponents (ATTX_components, x, y, z);
    AT.MultiplyTranspose (ATTXa, Xa, numXa);
    Check::Near (AX, AX_components);
    // Straight multiply array ...
    A.Multiply (AXa, Xa, numXa);
    DVec3d col0, col1, col2;
    A.GetColumns (col0, col1, col2);
    DPoint3d AX_lincomb;
    AX_lincomb.SumOf (col0, x, col1, y, col2, z);
    Check::Near (AX, AX_lincomb, "A*X == A[0]*x + A[1]*y + A[2]*z");

    Transform B = Transform::From (A);  // zero translation -- behaves just like RotMatrix
    DPoint3d BX, BX_components;
    B.Multiply (BX, X);
    Check::Near (AX, BX, "RotMatrix*X == Transform*X");
    B.Multiply (BX_components, x, y, z);
    Check::Near (BX, BX_components);

    DVec3d translation = DVec3d::From (3.1, 2.90, -0.42342);
    Transform C = Transform::From (A, translation);
    DPoint3d CX, CX_components;

    C.Multiply (CX, X);
    C.Multiply (CX_components, x, y, z);
    Check::Near (BX, BX_components);
    DVec3d BXtoCX = DVec3d::FromStartEnd (BX, CX);
    Check::Near (BXtoCX, translation);
    DPoint3d CX_inplace = X, CXw1_inplace = X;

    C.Multiply (CX_inplace);
    Check::Near (CX, CX_inplace);

    C.MultiplyWeighted (CXw1_inplace, 1.0);
    DPoint3d CXw1_inplace_array;
    double w = 1.0;
    C.MultiplyWeighted (&CXw1_inplace_array, &X, &w, 1);
    Check::Near (CX, CXw1_inplace_array);
    Check::Near (CXw1_inplace, CX);
    Transform CT = Transform::From (AT, translation);   // Transposing the rotmatrix part of transform is strange, but the method is there.
#ifdef MultiplyTranspose    
    DPoint3d CTX, CTX_components, CTX_inplace;
    CT.MultiplyTranspose (&CTX, &X, 1);
    CTX_inplace = X; CT.MultiplyTranspose (CTX_inplace);
    CT.MultiplyTranspose (CTX_components, x, y, z);
    Check::Near (CTX, CTX_components);
    Check::Near (CTX, CX);
    Check::Near (CTX, CTX_inplace);
#endif
    DPoint3d CX_MatrixOnly_components;
    C.MultiplyMatrixOnly (CX_MatrixOnly_components, x, y, z);
    Check::Near (BX, CX_MatrixOnly_components);

    DPoint3d CX_MatrixOnly, CX_MatrixOnly_inplace = X, CTX_MatrixOnly_components, CTX_TransposeMatrixOnly_components;
    C.MultiplyMatrixOnly (CX_MatrixOnly, X);
    C.MultiplyMatrixOnly (CX_MatrixOnly_inplace);
    CT.MultiplyMatrixOnly (CTX_MatrixOnly_components, x, y, z);
    DPoint3d CTX_TransposeMatrixOnly_inplace = X;
    CT.MultiplyTransposeMatrixOnly (CTX_TransposeMatrixOnly_inplace);
    Check::Near (CX_MatrixOnly, CTX_TransposeMatrixOnly_inplace);


    Check::Near (CTX_MatrixOnly_components, ATX);
    CT.MultiplyTransposeMatrixOnly (CTX_TransposeMatrixOnly_components, x, y, z);
    Check::Near (CX_MatrixOnly_components, CX_MatrixOnly);
    Check::Near (CX_MatrixOnly_components, CX_MatrixOnly_inplace);
    Check::Near (AX, CTX_TransposeMatrixOnly_components);

    DPoint3d CXa[4], CTXa_Transpose_inplace[4], CTXa_Transpose[4], CXa_inplace[4];
    for (int i = 0; i < numXa; i++)
        {
        CXa_inplace[i] = Xa[i];
        CTXa_Transpose_inplace[i] = Xa[i];
        }
    
    C.Multiply (CXa_inplace, numXa);
    C.Multiply (CXa, Xa, numXa);
//    CT.MultiplyTranspose (CTXa_Transpose_inplace, numXa);
    CT.Multiply (CTXa_Transpose, Xa, numXa);
    Check::Near (CXa, CXa_inplace, numXa);
//   Check::Near (CXa, CTXa_Transpose_inplace, numXa);


    // incomplete tests:
    // 1) all with DPoint2d or DPoint4d
    // 2) nontrivial weights.

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MatrixAndTransform, MultiplyA)
    {
    RotMatrix M0 = RotMatrix::FromRowValues (2,3,5, 7,11,13, 17,19, 23);
    TestMultiplication (M0, 29,31,37);

    TestMultiplication (M0, 1.1,0,0);
    TestMultiplication (M0, 0,1.2,0);
    TestMultiplication (M0, 0,0,1.3);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (MatrixAndTransform, ProductA)
    {
    RotMatrix matrixA = RotMatrix::FromRowValues
        (
        2.1, 0.3, 4.2,
        -0.312, 2.01, 0.987,
        0.23423, 1.231, -0.324
        );
    RotMatrix matrixB = RotMatrix::FromRowValues
        (
        4.1, 1.3, 2.2,
        -0.512, 3.01, 1.987,
        1.23423, 4.231, -2.324
        );

    RotMatrix matrixC = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), 0.23);
    RotMatrix matrixAB;
    matrixAB.InitProduct (matrixA, matrixB);
    DVec3d X = DVec3d::From (0.34534235, 0.1231, -0.876);
    DVec3d ABX, CX;
    matrixB.Multiply (ABX, X);
    matrixA.Multiply (ABX);
    matrixAB.Multiply (CX, X);
    Check::Near (ABX, CX);
    
    Check::Near (matrixA.Determinant () * matrixB.Determinant (), matrixAB.Determinant ());

    RotMatrix matrixABC, matrixAB_C;
    matrixABC.InitProduct (matrixA, matrixB, matrixC);
    matrixAB_C.InitProduct (matrixAB, matrixC);
    Check::Near (matrixABC, matrixAB_C);

    // Double letter names emphaszie connection to matrixB etc...
    Transform transform_AA = Transform::From (matrixA, DVec3d::From (1.2, 4.7, 2.9));
    Transform transform_BB = Transform::From (matrixB, DVec3d::From (3.2, 0.2, -1.0));
    Transform transform_A0 = Transform::From (matrixA);  // That has zero translation
    Transform transform_B0 = Transform::From (matrixB);
    RotMatrix matrix_A_BB, matrix_AA_B;
    matrix_A_BB.InitProduct (matrixA, transform_BB);
    matrix_AA_B.InitProduct (transform_AA, matrixB);
    Check::Near (matrixAB, matrix_A_BB);
    Check::Near (matrixAB, matrix_AA_B);
    
    Transform transform_AA_BB, transform_A0_BB, transform_AA_B0, transform_A_BB, transform_AA_B;
    transform_AA_BB.InitProduct (transform_AA, transform_BB);

    transform_AA_B0.InitProduct (transform_AA, transform_B0);
    transform_AA_B.InitProduct (transform_AA, matrixB);
    Check::Near (transform_AA_B, transform_AA_B0);

    transform_A0_BB.InitProduct (transform_A0, transform_BB);
    transform_A_BB.InitProduct (matrixA, transform_BB);

    Check::Near (transform_A0_BB, transform_A_BB);

    }

void CheckSVD (RotMatrixCR A)
    {
    RotMatrix U, V;
    DPoint3d D;
    Check::StartScope ("FactorRotateScaleRotate (SVD)", A);
    int rank = A.FactorRotateScaleRotate (U, D, V);
    double dSize = D.Magnitude ();
    for (int i = 0; i < 3; i++)
        {
        double di = D.GetComponent (i);
        Check::Bool (i < rank, di > dSize *Angle::SmallAngle (), "SVD Rank verification");
        }
    RotMatrix UD = U;
    UD.ScaleColumns (D.x, D.y, D.z);
    RotMatrix UDV;
    UDV.InitProduct (UD, V);
    Check::Near (A, UDV, "A = U*D*V");
    Check::EndScope ();
    }




void CheckOrthogonalizeColumns(RotMatrixCR A)
    {
    RotMatrix B, V;
    Check::StartScope ("OrthogonalFactors", A);
    A.FactorOrthogonalColumns (B, V);
    RotMatrix BTB, BV;
    BTB.InitProductRotMatrixTransposeRotMatrix (B, B);
    Check::True (BTB.IsDiagonal (), "B Columns Orthonormal");
    Check::True (V.IsOrthogonal (), "V Orthogonal");
    BV.InitProductRotMatrixRotMatrixTranspose (B, V);
    Check::Near (A, BV, "A=BV roundtrip");
    Check::EndScope ();
    }

void CheckOrthogonalFactors (RotMatrixCR A)
    {
    CheckOrthogonalizeColumns (A);
    CheckSVD (A);    
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,OrthogonalFactors)
    {
    CheckOrthogonalFactors (
        RotMatrix::FromRowValues (
            1,0,0,
            0,1,0,
            0,0,1
            ));
    CheckOrthogonalFactors (
        RotMatrix::FromRowValues (
            2,0,0,
            0,3,0,
            0,0,4
            ));
    CheckOrthogonalFactors (
        RotMatrix::FromRowValues (
            2,1,0,
            0,3,0,
            0,0,4
            ));
    
    CheckOrthogonalFactors (
        RotMatrix::FromRowValues (
            2,1,-1,
            5,3,-2,
            7,2,4
            ));
    }


bool VerifyCleanup (RotMatrixCR A, int axis,  bool expectCleanup)
    {
    RotMatrix B;
    double tolerance = 1.0e-6;
    bool actualCleanup = A.IsNearRigidScale (B, axis);
    Transform T0, T1;
    T0.InitFrom (A, DVec3d::From (1,2,3));
    T0.IsNearRigidScale (T1, axis);
    RotMatrix BfromTransform;
    BfromTransform.InitFrom (T1);
    if (!BfromTransform.IsEqual (B, 1.0e-15))
        return false;
    DVec3d W0, W1;
    T0.GetTranslation (W0);
    T1.GetTranslation (W1);
    if (!W0.IsEqual (W1))
        return false;
    if (actualCleanup)
        {
        RotMatrix B1, negB, AtB;
        double sB;
        AtB.InitProductRotMatrixTransposeRotMatrix (A, B);
        if (expectCleanup)
            {
            // B or -B should be a rigid scale ...
            negB.ScaleColumns (B, -1.0, -1.0, -1.0);
            double s = A.Determinant ();
            if (s > 0 && !B.IsRigidScale (B1, sB))
                return false;
            else if (s <= 0 && !negB.IsRigidScale (B1, sB))
                return false;
                
            double d0, d1;
            double e0, e1;
            AtB.DiagonalSignedRange (d0, d1);
            AtB.OffDiagonalSignedRange (e0, e1);
            if (Check::True (
                        fabs (e0) < tolerance
                    &&  fabs (e1) < tolerance
                    &&  fabs (d0 - d1) < tolerance * DoubleOps::MaxAbs (d0, d1),
                    "Matrix fixup conditions"))
                {
                return true;
                }
            }
        return false;
        }
    if (expectCleanup)
        return false;
    
    return A.IsEqual (B, 1.0e-16);
    }

bool VerifyShifts (RotMatrixCR A, int axis, double eps, bool expectCleanup)
    {
    Check::StartScope (expectCleanup ? "VerifySmallShifts" : "VerifyBigShifts");
    for (int mirrorAxis = -1; mirrorAxis < 3; mirrorAxis++)
        {
        for (int i = 0; i < 3; i++)
            {
            for (int j = 0; j < 3; j++)
                {
                RotMatrix B = A;
                B.form3d[i][j] += eps;
                if (mirrorAxis >= 0)
                    {
                    DVec3d U;
                    B.GetColumn (U, mirrorAxis);
                    U.Negate ();
                    B.SetColumn (U, mirrorAxis);
                    }
                if (!Check::True (VerifyCleanup (B, axis, expectCleanup), "Shifted matrix cleanup"))
                    return false;
                }
            }
         }
    Check::EndScope ();
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, InitWithRigidScaleCleanup)
    {
    Check::StartScope ("InitWithRigidScaleCleanup");
    double safeShift = 1.0e-8;
    double bigShift  = 1.0e-4;
    for (int axis = 0; axis < 5; axis++)
        {
        RotMatrix I = RotMatrix::FromIdentity ();
        Check::True (VerifyCleanup (I, axis, true), "Identity cleanup");
        Check::True (VerifyShifts (I, axis, safeShift, true), "Identity safe shifts");
        Check::True (VerifyShifts (I, axis, bigShift, false), "Identity big shifts");
        
        RotMatrix Q = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), 0.2);
        Check::True (VerifyCleanup (Q, axis, true), "rotation cleanup");
        Check::True (VerifyShifts (Q, axis, safeShift, true), "rotation safe shifts");
        Check::True (VerifyShifts (Q, axis, bigShift, false), "rotation big shifts");

        double a = 1.2;
        double b0 = 1.2 + safeShift;
        double b1 = 1.2 + bigShift;
        
        RotMatrix Qs = Q;
        Qs.ScaleColumns (a, a, a);
        Check::True (VerifyCleanup (Qs, axis, true), "scaled rotation cleanup");
        Check::True (VerifyShifts (Qs, axis, safeShift, true), "scaled rotation safe shifts");
        Check::True (VerifyShifts (Qs, axis, bigShift, false), "scaled rotation big shifts");

        Qs = Q;
        Qs.ScaleColumns (a, b0, b0);
        Check::True (VerifyCleanup (Qs, axis, true), "nonuniform scaled rotation cleanup");
        Check::True (VerifyShifts (Qs, axis, safeShift, true), "nonuniform scaled rotation safe shifts");
        Check::True (VerifyShifts (Qs, axis, bigShift, false), "nonuniform scaled rotation big shifts");

        Qs = Q;
        Qs.ScaleColumns (a, b1, b0);
        Check::True (VerifyCleanup (Qs, axis, false), "nonuniform scaled rotation cleanup");
        Check::True (VerifyShifts (Qs, axis, safeShift, false), "nonuniform scaled rotation safe shifts");
        Check::True (VerifyShifts (Qs, axis, bigShift, false), "nonuniform scaled rotation big shifts");
        }

    Check::EndScope ();
    }
    

RotMatrix MakePermutation (int ix, int iy, int iz, double x, double y, double z)
    {
    RotMatrix A;
    A.Zero ();
    A.form3d[0][ix] = x;
    A.form3d[1][iy] = y;
    A.form3d[2][iz] = z;
    return A;
    }

RotMatrix Shift (RotMatrixCR A, int i, int j, double delta)
    {
    RotMatrix B = A;
    B.form3d[i][j] += delta;
    return B;
    }

void VerifyPermutation (int ix, int iy, int iz, double x, double y, double z)
    {
    double tolerance = 1.0e-8;
    RotMatrix A = MakePermutation (ix, iy, iz, x, y, z);
    RotMatrix B, C, D;
    Check::True (A.IsSignedPermutation (), "Exact signed permutation");
    if (Check::True (A.IsNearSignedPermutation (B, tolerance), "Base signed permutation"))
        {
        Check::TrueZero (A.MaxDiff (B), "Exact signed permutation");
        for (int j = 0; j < 3; j++)
            {
            for (int k = 0; k < 3; k++)
                {
                B = Shift (A, j, k, 0.5 * tolerance);
                Check::True (B.IsNearSignedPermutation (C, tolerance), "epsilon signed");
                Check::TrueZero (A.MaxDiff (C), "Assign after match");

                B = Shift (A, j, k, 1.2 * tolerance);
                Check::False (B.IsNearSignedPermutation (D, tolerance), "epsilon signed");
                Check::TrueZero (B.MaxDiff (D), "Assign after failure");
                }
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (RotMatrix, IsNearSignedPermutation)
    {
    int ix, iy, iz;

    for (int i = 0; i < 6; i++)
        {
        if (i < 3)
            Angle::Cyclic3dAxes (ix, iy, iz, i);
        else
            Angle::Cyclic3dAxes (ix, iz, iy, i);
        VerifyPermutation (ix, iy, iz, 1,1,1);

        VerifyPermutation (ix, iy, iz, -1,1,1);
        VerifyPermutation (ix, iy, iz, 1,-1,1);
        VerifyPermutation (ix, iy, iz, 1,1,-1);

        VerifyPermutation (ix, iy, iz, -1,-1,1);
        VerifyPermutation (ix, iy, iz, 1,-1,-1);
        VerifyPermutation (ix, iy, iz, -1,1,-1);

        VerifyPermutation (ix, iy, iz, -1,-1,-1);
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix, EpsilonCase1)
    {
    int numFail = 0;
    for (double a = 10; a < 1e30 && numFail < 2; a*= 10.0)
        {
        RotMatrix A = RotMatrix::FromRowValues (1,0,0,   0,1,0, 0,0,a);
        RotMatrix B;
        if (B.InverseOf (A))
            {
            }
        else
            {
            printf (" Scale(1,1,a) considered singular for a = %le\n", a);
            numFail++;
            }
        }
    }
    
    
double det (double a, double b, double c, double d)
    {
    return a * d - b * c;
    }
void CheckEig2x2 (double a, double b, double c, double d)
    {
    double lambda[2];
    DVec2d eigenvector[2];
    int n = bsiSVD_realEigenvalues2x2 (a, b, c, d, lambda, eigenvector);
    for (int i = 0; i < n; i++)
        {

        Check::Near (0.0, det (a - lambda[i], b, c, d - lambda[i]), "eigenshift is singular");
        double wx = a * eigenvector[i].x + b * eigenvector[i].y;
        double wy = c * eigenvector[i].x + d * eigenvector[i].y;
        Check::Near (lambda[i] * eigenvector[i].x, wx, "Eigenvector scale");
        Check::Near (lambda[i] * eigenvector[i].y, wy, "Eigenvector scale");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Eigen, Solve2x2)
    {
    CheckEig2x2 (2, 0, 0, 3);
    CheckEig2x2 (4, 2, 2, 4);
    CheckEig2x2 (4, 1, 2, 4);
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,RepeatRotate)
    {
    int                                           count;
    RotMatrix            accumulatedRotation, incrementalRotation;
    accumulatedRotation.InitIdentity ();
    /* initialize matrix */
    //printf (" PI %.17g    cos(PI)=%.17g   sin(PI)=%.17g\n", PI, cos(PI), sin(PI));
    /* iteration rotate */
    for(count=0 ; count < 100 ; count++)
        {              
        /* matrix for multiply */
        incrementalRotation.InitIdentity ();
        incrementalRotation.InitFromPrincipleAxisRotations (incrementalRotation, PI, PI, PI);
        accumulatedRotation.InitProduct (accumulatedRotation, incrementalRotation);
    }
    double a, b;
    accumulatedRotation.OffDiagonalAbsRange (a, b);
    static double s_tol = 2.0e-14;
    if (!Check::True (b < s_tol))
        printf (" repeat rotation count %d accumulatedError %6.1le\n", count, b);


}
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,SkewFactors)
    {
    RotMatrix matrixA = RotMatrix::FromRowValues (1,2,3,-2,4,2,  -1,2,6);
    RotMatrix frame, skew;
    Check::True (matrixA.RotateAndSkewFactors (frame, skew, 0, 1), "Skew Factors");
    RotMatrix matrixB;
    matrixB.InitProduct (frame, skew);
    Check::True (frame.IsOrthogonal (), "Orthogonal frame");
    double s1 = skew.MaxAbs ();
    double s0 = skew.LowerTriangleMaxAbs ();
    Check::True (s0 < Angle::SmallAngle () * s1, "skew factor is upper triagnular");
    Check::Near (matrixA, matrixB, "factors");
    }



#ifdef Dgndb06GeomLibs
// Gyro vector convention:
// gyrospace X,Y,Z are (respectively) DOWN, RIGHT, and TOWARDS THE EYE.
// (gyrospace vectors are in the absolute system of the device.  But it is not important what that is -- just so they are to the same space and their row versus column usage is clarified by the gyroByRow parameter.
void ApplyGyroChangeToViewingVectors (
bool gyroByRow,         //!< [in] If TRUE, gyro X,Y,Z are ROWS of the gyro matrices.  If FALSE, they are COLUMNS
RotMatrixCR gyro0,      //!< [in] first gyro -- corresponds to forward0, up0.   Maps screen to gyrospace
RotMatrixCR gyro1,      //!< [in] second gyro.  Maps screen to gyrospace
DVec3dCR forward0,      //!< [in] model coordinates forward vector when gyro0 was recorded
DVec3dCR up0,           //!< [in] model coordinates up vector when gyro0 was recorded
DVec3dR forward1,       //!< [out] model coordinates up vector for gyro2
DVec3dR up1             //!< [out] model coordinates up vector for gyro1
)
    {
    if (gyroByRow)
        {
        ApplyGyroChangeToViewingVectors (false,
                RotMatrix::FromTransposeOf (gyro0),
                RotMatrix::FromTransposeOf (gyro1),
                forward0, up0,
                forward1, up1);
        }
    else
        {
        RotMatrix gyroToBSIColumnShuffler = RotMatrix::FromRowValues
                (
                0,-1,0,
                1,0,0,
                0,0,1
                );
        RotMatrix H0 = gyro0 * gyroToBSIColumnShuffler;
        RotMatrix H1 = gyro1 * gyroToBSIColumnShuffler;
        RotMatrix screenToScreenMotion = RotMatrix::FromTransposeOf (H1) * H0;
        DVec3d right0 = DVec3d::FromCrossProduct (up0, forward0);
        RotMatrix screenToModel = RotMatrix::FromColumnVectors (right0, up0, forward0);
        RotMatrix modelToScreen = RotMatrix::FromTransposeOf (screenToModel);
        RotMatrix modelToModel = screenToModel * screenToScreenMotion * modelToScreen;
        forward1 = modelToModel * forward0;
        up1 = modelToModel * up0;
        }
    }
#else
// Gyro vector convention:
// gyrospace X,Y,Z are (respectively) DOWN, RIGHT, and TOWARDS THE EYE.
// (gyrospace vectors are in the absolute system of the device.  But it is not important what that is -- just so they are to the same space and their row versus column usage is clarified by the gyroByRow parameter.
void ApplyGyroChangeToViewingVectors (
bool gyroByRow,         //!< [in] If TRUE, gyro X,Y,Z are ROWS of the gyro matrices.  If FALSE, they are COLUMNS
RotMatrixCR gyro0,      //!< [in] first gyro -- corresponds to forward0, up0.   Maps screen to gyrospace
RotMatrixCR gyro1,      //!< [in] second gyro.  Maps screen to gyrospace
DVec3dCR forward0,      //!< [in] model coordinates forward vector when gyro0 was recorded
DVec3dCR up0,           //!< [in] model coordinates up vector when gyro0 was recorded
DVec3dR forward1,       //!< [out] model coordinates up vector for gyro2
DVec3dR up1             //!< [out] model coordinates up vector for gyro1
)
    {
    RotMatrix gyro0T, gyro1T;
    if (gyroByRow)
        {
        gyro0T.TransposeOf (gyro0);
        gyro1T.TransposeOf (gyro1);
        ApplyGyroChangeToViewingVectors (false,
                gyro0T,
                gyro1T,
                forward0, up0,
                forward1, up1);
        }
    else
        {
        RotMatrix gyroToBSIColumnShuffler = RotMatrix::FromRowValues
                (
                0,-1,0,
                1,0,0,
                0,0,1
                );
        RotMatrix H0, H1;
        H0.InitProduct (gyro0, gyroToBSIColumnShuffler);
        H1.InitProduct (gyro1, gyroToBSIColumnShuffler);

        RotMatrix H1T;
        H1T.TransposeOf (H1);
        RotMatrix screenToScreenMotion;
        screenToScreenMotion.InitProduct (H1T, H0);
        DVec3d right0 = DVec3d::FromCrossProduct (up0, forward0);
        RotMatrix screenToModel = RotMatrix::FromColumnVectors (right0, up0, forward0);
        RotMatrix modelToScreen;
        modelToScreen.TransposeOf (screenToModel);
        RotMatrix modelToModel;
        modelToModel.InitProduct (screenToModel, screenToScreenMotion, modelToScreen);
        modelToModel.Multiply (forward1, forward0);
        modelToModel.Multiply (up1, up0);
        }
    }
#endif
ValidatedDVec3d ClampedUnit (DVec3dCR source, double tol)
    {
    for (int index = 0; index < 3; index++)
        {
        double a = source.GetComponent (index);
        double b = source.GetComponent (index + 1);
        double c = source.GetComponent (index + 2);
        double ea = fabs (a) - 1.0;
        if (fabs (ea) < tol && fabs (b * b) < tol && fabs (c * c) < tol)
            {
            DVec3d result = DVec3d::From (0,0,0);
            if (a > 0.0)
                result.SetComponent (1.0, index);
            else
                result.SetComponent (-1.0, index);
            return ValidatedDVec3d (result, true);
            }
        }
    return ValidatedDVec3d (source, false);
    }

RotMatrix ClampedUnits (RotMatrix A)
    {
    DVec3d U, V, W;
    A.GetColumns (U, V, W);
    double tol = 0.08;
    DVec3d U1 = ClampedUnit (U, tol);
    DVec3d V1 = ClampedUnit (V, tol);
    DVec3d W1 = ClampedUnit (W, tol);
    return RotMatrix::FromColumnVectors (U1, V1, W1);
    }

void VerifyNearRigid (RotMatrixCR matrix, char const *name)
    {
    Check::PrintIndent (0);
    Check::PrintIndent (0);
    Check::Print (matrix, name);
    Check::PrintIndent (2);
    double det = matrix.Determinant ();
    Check::Print (det, "det");
    Check::True (fabs (det - 1.0) < 0.03, "det of Near rigid matrix");
    Check::Print (ClampedUnits (matrix));
    auto QQT = ClampedUnits (matrix * RotMatrix::FromTransposeOf (matrix));
    Check::True (QQT.MaxDiff (RotMatrix::FromIdentity ()) < 0.05);



    DVec3d rotationAroundVector;
    double theta = matrix.GetRotationAngleAndVector (rotationAroundVector);
    Check::Print (rotationAroundVector, "rotate about");   Check::Print (theta, "radians"); Check::PrintIndent (0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,GyroData)
    {
    DVec3d f0_m = DVec3d::From (0,-1,0);
    DVec3d u0_m = DVec3d::From (0,0,1);




    static int s_transpose = 1;
    RotMatrix defaultGyro = RotMatrix::FromRowValues
            (
            0.040149,0.012912,-0.999110,
            0.012927,0.999826,0.013440,
            0.999110,-0.013455,0.039975
            );
    if (s_transpose)
        defaultGyro.Transpose ();

    VerifyNearRigid (defaultGyro, "Default Gyro");

    RotMatrix gyroToBSIColumnShuffler = RotMatrix::FromRowValues
                (
                0,-1,0,
                1,0,0,
                0,0,1
                );
    Check::Near (gyroToBSIColumnShuffler.Determinant (), 1.0, "right handed");
    Check::True (gyroToBSIColumnShuffler.IsOrthogonal (), "orthogonal");

    RotMatrix G0 = RotMatrix::FromRowValues
                (
                0.040149,0.012912,-0.999110,
                0.012927,0.999826,0.013440,
                0.999110,-0.013455,0.039975
                );
    //RotMatrix G1 = RotMatrix::FromRowValues (0.009096,-0.203890,-0.978952, 0.017604,0.978873,-0.203710, 0.999804,-0.015380,0.012493);
    RotMatrix G1 = RotMatrix::FromRowValues
                (
                0.973659,-0.222359,-0.050443,
                0.222252,0.974958,-0.007802,
                0.050915,-0.003614,0.998696
                );

    DVec3d f3_m, u3_m;
    ApplyGyroChangeToViewingVectors (true, G0, G1, f0_m, u0_m, f3_m, u3_m);

    if (s_transpose != 0)
        {
        G0.Transpose ();
        G1.Transpose ();
        }
    
    VerifyNearRigid (G0, "G0");
    VerifyNearRigid (G1, "G1");






    RotMatrix H0 = G0 * gyroToBSIColumnShuffler;
    RotMatrix H1 = G1 * gyroToBSIColumnShuffler;

    VerifyNearRigid (H0, "H0");
    VerifyNearRigid (H1, "H1");


    //RotMatrix H0T = RotMatrix::FromTransposeOf (H0);
    RotMatrix H1T = RotMatrix::FromTransposeOf (H1);


    DVec3d r0_m = DVec3d::FromCrossProduct (u0_m, f0_m);

    RotMatrix V0_screenToModel = RotMatrix::FromColumnVectors (r0_m, u0_m, f0_m);
    Check::True (V0_screenToModel.IsRigid ());
    RotMatrix V0_modelToScreen = RotMatrix::FromTransposeOf (V0_screenToModel);

    auto DeltaH  = H1T * H0;   // composite matrices 
    auto DeltaG = RotMatrix::FromTransposeOf (G1) * G0;

    VerifyNearRigid (DeltaG, "DeltaG");
    VerifyNearRigid (DeltaH, "DeltaH");

    auto screenToModel = V0_screenToModel * DeltaH;
    auto modelToModel = V0_screenToModel * DeltaH * V0_modelToScreen;

    Check::Print (modelToModel, "Model to model update");
    Check::Print (screenToModel, "screen to model update");
    auto f1_m = modelToModel * f0_m;
    auto u1_m = modelToModel * u0_m;
    Check::Print (f0_m, "f0_m"); Check::Print (u0_m, "u0_m");  Check::PrintIndent (0);
    Check::Print (f1_m, "f1_m"); Check::Print (u1_m, "u1_m");  Check::PrintIndent (0);
    double clampTol = 0.08;
    Check::Print (ClampedUnit (f1_m, clampTol), "fuzzy f1_m"); Check::Print (ClampedUnit(u1_m, clampTol), "fuzzy u1_m");  Check::PrintIndent (0);


    DVec3d f2_m, u2_m;
    ApplyGyroChangeToViewingVectors (false, G0, G1, f0_m, u0_m, f2_m, u2_m);
    Check::Near (f1_m, f2_m, "Verify packaged update");
    Check::Near (u1_m, u2_m, "Verify packaged update");

    Check::Near (f1_m, f3_m, "Verify packaged update");
    Check::Near (u1_m, u3_m, "Verify packaged update");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,FromCrossVector)
    {
    auto vectorA = DVec3d::From (3,5,7);
    auto vectorB = DVec3d::From (9.2, 0.241, -6.902);
    auto matrix = RotMatrix::FromCrossingVector (vectorA);
    // To confirm: matrix * B = A CROSS B
    auto matrixTimesB = matrix * vectorB;
    auto vectorCrossB = DVec3d::FromCrossProduct (vectorA, vectorB);
    Check::Near (matrixTimesB, vectorCrossB);
    // To confirm: transpose(B) * matrix = - transpose (A CROSS B)
    auto BTimesMatrix = vectorB * matrix;
    Check::Near (matrixTimesB, -1.0 * BTimesMatrix);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(RotMatrix, TriangularMatrix) 
    {
    RotMatrix triangularMatrix = RotMatrix::FromRowValues(-4, 2, 1, 8, 7, 2, -12, 4, -5);
    Check::ExactDouble(2, triangularMatrix.UpperTriangleMaxAbs());
    Check::ExactDouble(7, triangularMatrix.DiagonalMaxAbs());
    Check::ExactDouble(12, triangularMatrix.LowerTriangleMaxAbs());
    DRange1d upperRange = triangularMatrix.UpperTriangleAbsRange();
    DRange1d lowerRange = triangularMatrix.LowerTriangleAbsRange();
    DRange1d diagonalRange = triangularMatrix.DiagonalAbsRange();
    double low;
    double high;
    upperRange.GetLowHigh(low, high);
    Check::ExactDouble(1, low);
    Check::ExactDouble(2, high);
    lowerRange.GetLowHigh(low, high);
    Check::ExactDouble(4, low);
    Check::ExactDouble(12, high);
    diagonalRange.GetLowHigh(low, high);
    Check::ExactDouble(4, low);
    Check::ExactDouble(7, high);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(RotMatrix, Quaternion)
    {
    RotMatrix rotMat = RotMatrix::FromRowValues(1, 0, 0, 1, 1, 0, 2, 0, 1);
    double angle = rotMat.ColumnXAngleXY();
    Angle a = Angle::FromDegrees(45);
    Check::Near(a.Radians(), angle);
    rotMat = RotMatrix::FromRowValues(1, 0, 0, 3, 1, 0, 2, 0, 1);
    a = Angle::FromDegrees(71.5650511771);
    angle = rotMat.ColumnXAngleXY();
    Check::Near(a.Radians(), angle);
    }

void CheckMatrixRigid (RotMatrixCR matrix, bool isRigidScale, bool isRigidSignedScale, double s)
    {
    double s0, s1;
    RotMatrix matrix0, matrix1;
    if (Check::Bool (isRigidScale, matrix.IsRigidScale (matrix0, s0), "Rigid Scale extraction"))
        {
        if (isRigidScale)
            {
            RotMatrix matrix0A;
            matrix0A.ScaleColumns (matrix0, s0, s0, s0);
            Check::Near (matrix0A, matrix);
            Check::True (matrix0.IsRigid (), "Rigid factor");
            }
        }
    if (Check::Bool (isRigidSignedScale, matrix.IsRigidSignedScale (matrix1, s1), "Rigid Scale extraction"))
        {
        if (isRigidSignedScale)
            {
            RotMatrix matrix1A;
            matrix1A.ScaleColumns (matrix1, s1, s1, s1);
            Check::Near (matrix1A, matrix);
            Check::True (matrix1.IsRigid (), "Rigid factor");
            }
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,IsRigidSignedScale)
    {
    RotMatrix pureRotation = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), 0.23);
    CheckMatrixRigid (pureRotation, true, true, 1.0);

    for (double s : bvector<double>{1,2,0.5, -1, -2, -0.5})
        {
        RotMatrix matrix1, matrix2, matrix3, matrix4;


        matrix1.ScaleColumns (pureRotation, s, s, s);
        CheckMatrixRigid (matrix1, s > 0.0, true, s);

        matrix2.ScaleColumns (pureRotation, s, s, -s);
        CheckMatrixRigid (matrix2, s < 0.0, true, s);

        matrix3.ScaleColumns (pureRotation, s, -s, s);
        CheckMatrixRigid (matrix3, s < 0.0, true, s);

        matrix4.ScaleColumns (pureRotation, -s, s, s);
        CheckMatrixRigid (matrix4, s < 0.0, true, s);
        // Note: this sequence tests 0 and 1 sign flips.  2 and 3 gets caught when the bvector has the negative
        }
    
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(RotMatrix, FromQuaternion)
    {
    //point (1,-1, 2) under the rotation by an angle of 60 degree about an axis in the yz-plane that is
    //inclined at an angle of 60 to the positive y-axis

    // quaternion =  (1/4 ) * j + (sqrt(3)/4 ) * k + (sqrt(3)/2 ) http://www.imsc.res.in/~knr/131129workshop/writeup_knr.pdf
    RotMatrix rotQuaternion = RotMatrix::FromQuaternion(DPoint4d::From(0, 0.25, Angle::FromDegrees(60).Sin() / 2, Angle::FromDegrees(60).Sin()));
    DPoint3d subjectPoint = DPoint3d::From(1, -1, 2);
    DPoint3d subjectPointCpy = subjectPoint;
    rotQuaternion.Multiply(subjectPoint);

    printf("%f  %f  \n", (10 + 4 * sqrt(3)) / 8, subjectPoint.x);
    printf("%f  %f  \n", (1 + 2 * sqrt(3)) / 8, subjectPoint.y);
    printf("%f  %f  \n \n", (14 - 3 * sqrt(3)) / 8, subjectPoint.z);
    Check::Near((10 + 4 * sqrt(3)) / 8, subjectPoint.x);
    Check::Near((1 + 2 * sqrt(3)) / 8, subjectPoint.y);
    Check::Near((14 - 3 * sqrt(3)) / 8, subjectPoint.z);

    double quanternion[4];
    rotQuaternion.GetQuaternion(quanternion, false);
    
    RotMatrix rotQuaternion2 = RotMatrix::FromQuaternion(quanternion);
    rotQuaternion2.Multiply(subjectPointCpy);

    printf("%f  %f  \n", subjectPointCpy.x, subjectPoint.x);
    printf("%f  %f  \n", subjectPointCpy.y, subjectPoint.y);
    printf("%f  %f  \n \n", subjectPointCpy.z, subjectPoint.z);
    Check::Near(subjectPointCpy.x, subjectPoint.x);
    Check::Near(subjectPointCpy.y, subjectPoint.y);
    Check::Near(subjectPointCpy.z, subjectPoint.z);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(RotMatrix, RotTransformOnPointArray)
    {
    RotMatrix rotMat = RotMatrix::FromAxisAndRotationAngle(2, Angle::FromDegrees(178).Radians());
    DPoint3d outPoints[4];
    DPoint3d inPoints[4] = { DPoint3d::From(4,3,2),
                             DPoint3d::From(5,5,9),
                             DPoint3d::From(0.8,9,3.4),
                             DPoint3d::From(8,9,11) };

    rotMat.SolveArray(outPoints, inPoints, 4);
    bvector<DPoint3d> outPointsVec;
    bvector<DPoint3d> inPointsVec = { inPoints[0],
                                      inPoints[1],
                                      inPoints[2],
                                      inPoints[3] };
    rotMat.SolveArray(outPointsVec, inPointsVec);
    DPoint3d testOut;

    for (size_t i = 0; i < 4; i++) 
        {
        rotMat.Solve(testOut, inPoints[i]);
        Check::Near(testOut, outPoints[i]);
        Check::Near(outPoints[i], outPointsVec[i]);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(RotMatrix, Init)
    {
    RotMatrix rot;
    rot.InitFromRowValuesXY(2, 3, 7, 1);
    RotMatrix rotTest = RotMatrix::FromRowValues(2, 3, 0,
                                                 7, 1, 0,
                                                 0, 0, 1);

    Check::Near(rot, rotTest);
    double rowMajor[4] = { 2, 3, 7, 1 };
    rot.InitFromRowValuesXY(rowMajor);
    Check::Near(rot, rotTest);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(RotMatrix, ChangeRotation)
    {
    RotMatrix rot = RotMatrix::FromAxisAndRotationAngle(2, Angle::FromDegrees(90).Radians());
    double zRotVals[4];
    rot.GetRowValuesXY(zRotVals);
    Check::Near(zRotVals[0], Angle::FromDegrees(90).Cos());
    Check::Near(zRotVals[1], -Angle::FromDegrees(90).Sin());
    Check::Near(zRotVals[2], Angle::FromDegrees(90).Sin());
    Check::Near(zRotVals[3], Angle::FromDegrees(90).Cos());

    DPoint3d point = DPoint3d::From(4, 4, 2);
    rot.Multiply(point);

    RotMatrix rot2;
    rot2.Zero();
    rot2.SetColumn(0, Angle::FromDegrees(90).Cos(), Angle::FromDegrees(90).Sin(), 0);
    rot2.SetColumn(1, -Angle::FromDegrees(90).Sin(), Angle::FromDegrees(90).Cos(), 0);
    rot2.SetColumn(2, 0, 0, 1);

    Check::Near(rot, rot2);
    Check::Near(point, DPoint3d::From(-4, 4, 2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(RotMatrix,RotationFromStanadardView)
    {
    for (int viewIndex = 1; viewIndex < 9; viewIndex++)
        {
        Check::StartScope ("ViewIndex", (size_t)viewIndex);
        for (int axis = 0; axis < 3; axis++)
            {
            Check::StartScope ("Axis", (size_t)axis);
            for (double radians : bvector<double> {0.1, -0.2})
                {
                Check::StartScope ("Radians", radians);
                RotMatrix matrix;
                if (Check::True (bsiRotMatrix_initRotationFromStandardView (&matrix, axis, radians, viewIndex)))
                    {
                    double radians1;
                    int axis1;
                    int viewIndex1;
                    bool expectedResult = (viewIndex >= 7 && axis != 2) ? false : true;
                    bool actualResult = bsiRotMatrix_isRotationFromStandardView (&matrix, &axis1, &radians1, &viewIndex1, true, true, true);
                    if (expectedResult && Check::Bool (expectedResult, actualResult, "isRotationFrmoStandardView"))
                        {
                        RotMatrix matrix1;
                        bsiRotMatrix_initRotationFromStandardView (&matrix1, axis1, radians1, viewIndex1);
                        Check::Near (matrix, matrix1, "Standard view round trip");
                        }
                    }
                Check::EndScope ();
                }
            Check::EndScope ();
            }
        Check::EndScope ();
        }
    }

static double ModifyPitchAngleToPreventInversion (double radians) { return radians;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void NavigateMotion__GenerateRotationTransform_fromDgnPlatform(
TransformR transform,
double yawRate,
double pitchRate,
DPoint3dCR eyePoint,
RotMatrixCR viewRotation,
double deltaTime
)
    {
    double yawAngle = yawRate * deltaTime;
    double pitchAngle = ModifyPitchAngleToPreventInversion(pitchRate * deltaTime);

    RotMatrix   rotation, verticalRotation, horizontalRotation, invViewRotation;

    invViewRotation.InverseOf(viewRotation); // m_viewport->GetRotMatrix());
    verticalRotation.InitFromPrincipleAxisRotations(
        viewRotation, //m_viewport->GetRotMatrix(),
        pitchAngle, 0.0, 0.0);
    Check::Print (verticalRotation, "VInv * Pitch");
    verticalRotation.InitProduct(invViewRotation, verticalRotation);


    horizontalRotation.InitFromAxisAndRotationAngle(2, yawAngle);
    Check::Print (horizontalRotation, "yaw matrix");
    rotation.InitProduct(horizontalRotation, verticalRotation);
    Check::Print (rotation, "matrix product");

    transform.InitFromMatrixAndFixedPoint(rotation, eyePoint); /// m_viewport->GetCamera().GetEyePoint());
    Check::Print (transform, "transform");
    }

static void NavigateMotion__GenerateRotationTransform_forImodelJS(
TransformR transform,
double yawRate,
double pitchRate,
DPoint3dCR eyePoint,
RotMatrixCR viewRotation,
double deltaTime
)
    {
    double yawAngle = yawRate * deltaTime;
    double pitchAngle = ModifyPitchAngleToPreventInversion(pitchRate * deltaTime);

    RotMatrix invViewRotation;
    invViewRotation.InverseOf(viewRotation); // m_viewport->GetRotMatrix());
    Check::Print (viewRotation, "******************viewRotation");
    RotMatrix pitchRotation;
    pitchRotation.InitFromAxisAndRotationAngle (0, pitchAngle);
    Check::Print (pitchRotation, "Pitch");
    RotMatrix verticalRotationA, verticalRotationB;
    verticalRotationA.InitProduct(pitchRotation, viewRotation);
    verticalRotationB.InitProduct (invViewRotation, verticalRotationA);
    Check::Print (verticalRotationB, "VInv*Pitch");

    RotMatrix horizontalRotation, rotation;
    horizontalRotation.InitFromAxisAndRotationAngle (2, yawAngle);
    Check::Print (horizontalRotation, "yaw matrix");

    rotation.InitProduct (horizontalRotation, verticalRotationB);
    Check::Print (rotation, "matrix product");
    transform.InitFromMatrixAndFixedPoint(rotation, eyePoint); /// m_viewport->GetCamera().GetEyePoint());

    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
TEST(RotMatrix, NavigationMatrix)
    {
    int volume = Check::SetMaxVolume (1000);
    auto eyePoint = DPoint3d::From (10,15,23);
    auto viewRotation = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), 0.23);
    double yawRate = 0.5;
    double pitchRate = 0.25;
    double time = 0.1;
    Transform transformA, transformB;
    NavigateMotion__GenerateRotationTransform_fromDgnPlatform (transformA, yawRate, pitchRate, eyePoint, viewRotation, time);
    NavigateMotion__GenerateRotationTransform_forImodelJS(transformB, yawRate, pitchRate, eyePoint, viewRotation, time);
    Check::Near (transformA, transformB, "NavigateMotion__GenerateRotationTransform_forImodelJS"); 

    Check::Print (transformA, "NavigateMotion__GenerateRotationTransform_fromDgnPlatform");
    Check::Print (transformB, "NavigateMotion__GenerateRotationTransform_forImodelJS");

    Check::SetMaxVolume (volume);
    }