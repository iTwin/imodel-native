#include "testHarness.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Transform, InitIdentity)
    {
    Transform transform0, transform2, transform4, transform5, transform6, transform7, transform8, transform9, transform11, transform12, transform14, transform16, transform17, transform18, transform19, transform21, transform23, transform25, transform26, transform27, transform28, transform30, transform31;
    Transform transform1 = Transform::FromRowValues (1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    Transform transform3 = Transform::FromRowValues (1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 2.0, 0.0, 0.0, 1.0, 3.0);
    Transform transform10 = Transform::FromRowValues (1.0, 5.0, 3.0, -19.0, 8.0, 6.0, 9.0, -45.0, 4.0, 1.0, 2.0, -9.0);
    Transform transform13 = Transform::FromRowValues (0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 5.0, 2.0, 0.0, 0.0, 7.0, 3.0);
    Transform transform15 = Transform::FromRowValues (1.0, 1.0, 1.0, 0.0, 2.0, 0.0, 5.0, 0.0, 3.0, 0.0, 7.0, 0.0);
    Transform transform20 = Transform::FromRowValues (2.0, 0.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    Transform transform22 = Transform::FromRowValues (0.0, -3.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    Transform transform24 = Transform::FromRowValues (1.0, 3.0, 0.0, 0.0, 2.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    Transform transform29 = Transform::FromRowValues (1.0 / (sqrt (5.0)), -2.0 / (sqrt (5.0)), 0.0, 0.0, 2.0 / (sqrt (5.0)), 1.0 / (sqrt (5.0)), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    Transform transform32 = Transform::FromRowValues (0.0, 0.0, 1.0, 0.0, -2.0, -4.0, 0.0, 0.0, 1.0, 3.0, 0.0, 0.0);
    RotMatrix matrix0 = RotMatrix::FromIdentity ();
    RotMatrix matrix1;
    RotMatrix matrix2 = RotMatrix::FromRowValues (1.0, 5.0, 3.0, 8.0, 6.0, 9.0, 4.0, 1.0, 2.0);
    RotMatrix matrix3 = RotMatrix::FromRowValues (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    DVec3d vec0 = DVec3d::From (1.0, 0.0, 0.0);
    DVec3d vec1 = DVec3d::From (0.0, 0.0, 0.0);
    DVec3d vec2 = DVec3d::From (1.0, 5.0, 7.0);
    DPoint3d point0 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point1 = DPoint3d::From (0.0, 0.0, 0.0);
    DPoint3d point2 = DPoint3d::From (1.0, 0.0, 0.0);
    DPoint3d point3 = DPoint3d::From (1.0, 5.0, 7.0);
    DPoint3d point4 = DPoint3d::From (0.0, 1.0, 0.0);
    DPoint2d point5 = DPoint2d::From (0.0, 0.0);
    DPoint2d point6 = DPoint2d::From (1.0, 2.0);
    DPoint2d point7 = DPoint2d::From (3.0, 4.0);
    DVec2d vec3 = DVec2d::From (1.0, 2.0);
    DVec2d vec4 = DVec2d::From (3.0, 4.0);
    DMatrix4d matrix4 = DMatrix4d::FromRowValues (1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    double radians = PI/2;
    transform0.InitIdentity ();
    transform2.InitFrom (matrix0);
    transform4.InitFrom (matrix0, point0);
    transform5.InitFrom (point0);
    transform6.InitFrom (1.0, 2.0, 3.0);
    transform7.InitFromRowValues (1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 2.0, 0.0, 0.0, 1.0, 3.0);
    transform8.InitFromLineAndRotationAngle (point1, point2, radians);
    matrix1.InitFromVectorAndRotationAngle (vec0, radians);
    transform9.InitFrom (matrix1, point1);
    transform11.InitFromMatrixAndFixedPoint (matrix2, point0);
    transform12.InitFromOriginAndVectors (point0, vec1, vec0, vec2);
    transform14.InitFrom4Points (point1, point0, point2, point3);
    transform16.InitFromPlaneOf3Points (point1, point2, point4);
    transform17.InitFromPlaneNormalToLine (point1, point2, 0, true);
    transform18.InitFrom (matrix3, point1);
    transform19.InitFromOriginAndLengths (point5, 2.0, 3.0);
    transform21.InitFromOriginAngleAndLengths (point5, PI/2, 2.0, 3.0);
    transform23.InitFromOriginAndVectors (point5, vec3, vec4);
    transform25.InitFrom3Points (point5, point6, point7);
    transform26.InitNormalizedFrameFromOriginXPointYPoint (point1, point2, point4);
    Check::True (transform27.InitFrom (matrix4));
    transform28.InitFrom2Points (point5, point6, 0, true);
    Check::True (transform30.InitUniformScaleApproximation (transform3, 0, 1));
    transform31.InitFromPrincipleAxisRotations (transform24, PI/2, PI/2, PI/2);
    Check::Near (transform1, transform0);
    Check::Near (transform1, transform2);
    Check::Near (transform3, transform4);
    Check::Near (transform3, transform5);
    Check::Near (transform3, transform6);
    Check::Near (transform3, transform7);
    Check::Near (transform9, transform8);
    Check::Near (transform10, transform11);
    Check::Near (transform13, transform12);
    Check::Near (transform15, transform14);
    Check::Near (transform1, transform16);
    Check::Near (transform18, transform17);
    Check::Near (transform20, transform19);
    Check::Near (transform22, transform21);
    Check::Near (transform24, transform23);
    Check::Near (transform24, transform25);
    Check::Near (transform1, transform26);
    Check::Near (transform1, transform27);
    Check::Near (transform29, transform28);
    Check::Near (transform3, transform30);
    Check::Near (transform32, transform31);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Transform, Multiply)
    {
    Transform transform0 = Transform::FromRowValues (1.0, 5.0, 3.0, -19.0, 8.0, 6.0, 9.0, -45.0, 4.0, 1.0, 2.0, -9.0);
    DPoint3d point0 = DPoint3d::From (1.0, 5.0, 7.0);
    DPoint3d point1 = DPoint3d::From (1.0, 5.0, 7.0);
    DPoint3d point2, point11, point13, point15, point17, point20;
    //DPoint3d point12;
    DPoint3d point3 = DPoint3d::From (28.0, 56.0, 14.0);
    DPoint3d point4 = DPoint3d::From (1.0, 5.0, 7.0);
    DPoint3d point5 = DPoint3d::From (9.0, 11.0, 5.0);
    DPoint3d point6[2];
    DPoint3d point7[2];
    point6[0] = point1;
    point6[1] = point5;
    DPoint3d point8 = DPoint3d::From (41.0, 93.0, 39.0);
    //DPoint3d point9 = DPoint3d::From (1.0, 2.0, 3.0);
    //DPoint3d point10 = DPoint3d::From (10.0, -25.0, 18.0);
    DPoint3d point14 = DPoint3d::From (47.0, 101.0, 23.0);
    DPoint3d point16 = DPoint3d::From (1.0, 5.0, 7.0);
    DPoint3d point18 = DPoint3d::From (29.0, 20.0, 27.0);
    DPoint3d point19 = DPoint3d::From (1.0, 2.0, 3.0);
    DPoint3d point21[2];
    point21[0] = DPoint3d::From (1.0, 1.0, 1.0);
    point21[1] = DPoint3d::From (2.0, 2.0, 2.0);
    DPoint3d point22 = DPoint3d::From (-10.0, -22.0, -2.0);
    DPoint3d point23 = DPoint3d::From (-1.0, 1.0, 5.0);
    //DPoint3d point24[2];
    //point24[0] = DPoint3d::From (1.0, 2.0, 3.0);
    //point24[1] = DPoint3d::From (1.0, 1.0, 1.0);
    //DPoint3d point25 = DPoint3d::From (-6.0, -33.0, 5.0);
    double weight[2];
    weight[0] = weight[1] = 2.0;
    transform0.Multiply (point0);
    transform0.Multiply (point2, point1);
    transform0.MultiplyWeighted (point4, 2.0);
    transform0.MultiplyWeighted (point7, point6, weight, 2);
    //transform0.MultiplyTranspose (point9);
    transform0.Multiply (point11, 1.0, 5.0, 7.0);
    //transform0.MultiplyTranspose (point12, 1.0, 2.0, 3.0);
    transform0.MultiplyMatrixOnly (point13, 1.0, 5.0, 7.0);
    transform0.MultiplyMatrixOnly (point15, point16);
    transform0.MultiplyMatrixOnly (point16);
    transform0.MultiplyTransposeMatrixOnly (point17, 1.0, 2.0, 3.0);
    transform0.MultiplyTransposeMatrixOnly (point20, point19);
    transform0.MultiplyTransposeMatrixOnly (point19);
    transform0.Multiply (point21, 2);
    //transform0.MultiplyTranspose (point24, 2);
    Check::Near (point3, point0);
    Check::Near (point3, point2);
    Check::Near (point5, point4);
    Check::Near (point5, point7[0]);
    Check::Near (point8, point7[1]);
    //Check::Near (point10, point9);
    Check::Near (point3, point11);
    //Check::Near (point10, point12);
    Check::Near (point14, point13);
    Check::Near (point14, point15);
    Check::Near (point14, point16);
    Check::Near (point18, point17);
    Check::Near (point18, point20);
    Check::Near (point18, point19);
    Check::Near (point22, point21[0]);
    Check::Near (point23, point21[1]);
    //Check::Near (point10, point24[0]);
    //Check::Near (point25, point24[1]);
    }

void CheckTransformMultiply (TransformCR A, double bx, double by, double bz)
    {
    DPoint3d B3 = DPoint3d::From (bx, by, bz);
    DPoint3d ABxyz, AB3;
    A.Multiply (AB3, B3);
    A.Multiply (ABxyz, bx, by, bz);
    Check::Near (AB3, ABxyz);
    DPoint4d C = DPoint4d::From (AB3, 1.0);
    DPoint4d AB4;
    DPoint4d B4 = DPoint4d::From (bx, by, bz, 1.0);
    A.Multiply (AB4, B4);
    Check::Near (AB4, C);

    DVec3d colX, colY, colZ;
    DPoint3d origin;
    A.GetOriginAndVectors (origin, colX, colY, colZ);

    DPoint2d B2 = DPoint2d::From (bx, by);

    DPoint3d AB2xy, AB2;
    A.Multiply (AB2, B2);
    A.Multiply (AB2xy, bx, by, 0.0);
    Check::Near (AB2, AB2xy);

    DPoint2d C2;
    A.Multiply (C2, B2);
    
    Check::Near (C2, DPoint2d::From (AB2xy.x, AB2xy.y));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Transform, MultiplyA)
    {
    Transform A = Transform::FromRowValues (
            8.1,2,3,4,
            3,5,1.2, 2.1,
            0.3,.4,7,3.5);
    CheckTransformMultiply (A, 1,2,3);
    CheckTransformMultiply (A, 3,5,3);
    }


void TestMixedProduct (TransformCR TA, TransformCR TB)
    {
    RotMatrix MA, MB;
    Transform TMA, TMB;
    MA.InitFrom (TA);
    MB.InitFrom (TB);
    TMA.InitFrom (MA);
    TMB.InitFrom (MB);
    Transform TA_MB1, MA_TB1;
    Transform TA_MB2, MA_TB2;
    TA_MB1.InitProduct (TA, MB);
    TA_MB2.InitProduct (TA, TMB);

    Check::Near (TA_MB1, TA_MB2, "Transform*RotMatrix");

    MA_TB1.InitProduct (MA, TB);
    MA_TB2.InitProduct (TMA, TB);

    Check::Near (MA_TB1, MA_TB2, "Rotmatrix*Transfrom");


    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, MixedProducts)
    {
    Transform TA = Transform::FromRowValues (
                2,3,4,5,
                6,7,8,9,
                10,11,12,13);
    Transform TB = Transform::FromRowValues (
                0.3, 0.23, 0.67,    9.3,
                -0.3, 0.2, 0.98,    10.983798712,
                0.4, 0.5, 0.29,     -2.01);
            
    TestMixedProduct (TA, TB);
                
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, Init1)
    {
    Transform transform0 = Transform::FromRowValues (1,2,3,4,5,6,7,8,9,10,11,12);
    DPoint3d origin;
    DVec3d vectorX, vectorY, vectorZ;
    transform0.GetOriginAndVectors (origin, vectorX, vectorY, vectorZ);
    DPoint3d point0, point1, point2, point3;
    transform0.Get4Points (point0, point1, point2, point3);
    DPoint3d points[4];
    transform0.Get4Points (points);
    Check::Near (points[0], point0, "Get 4 Points");
    Check::Near (points[1], point1, "Get 4 Points");
    Check::Near (points[2], point2, "Get 4 Points");
    Check::Near (points[3], point3, "Get 4 Points");
    Transform transform1 = Transform::From4Points (point0, point1, point2, point3);
    Check::Near (transform0, transform1, "Get/Set 4Points");
    Check::Near (point1, DPoint3d::FromSumOf (origin, vectorX, 1.0), "point1");
    Check::Near (point2, DPoint3d::FromSumOf (origin, vectorY, 1.0), "point2");
    Check::Near (point3, DPoint3d::FromSumOf (origin, vectorZ, 1.0), "point3");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, Init2)
    {
    DPoint3d point0 = DPoint3d::From (1,2,3);
    DPoint3d point1 = DPoint3d::From (2.5, 1.1, 4.5);
    DPoint3d point2 = DPoint3d::From (-2.3, 0.01232, 0.2342);
    DVec3d vector01 = DVec3d::FromStartEnd (point0, point1);
    DVec3d vector02 = DVec3d::FromStartEnd (point0, point2);
    Transform transform0 = Transform::FromPlaneOf3Points (point0, point1, point2);
    DPoint3d originA;
    DVec3d vectorA0, vectorA1, vectorA2;
    transform0.GetOriginAndVectors (originA, vectorA0, vectorA1, vectorA2);
    //Check::Perpendicular (vectorA0, vectorA1, "XperpY");  // no, FromPlaneOf3Points is skewed
    Check::Perpendicular (vectorA1, vectorA2, "YperpZ");
    Check::Perpendicular (vectorA2, vectorA0, "ZperpX");
    Check::Perpendicular (vectorA2, vector01, "Zperp(01)");
    Check::Perpendicular (vectorA2, vector02, "Zperp(02)");
    Check::Parallel (vector01, vectorA0, "Xpar(01)");
    
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, MaxDiff)
    {
    Transform transform0 = Transform::FromRowValues (1,2,3,4,5,6,7,8,9,10,11,12);
    double a = 1.2349;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 4; j++)
            {
            Transform transform1 = transform0;
            transform0.form3d[i][j] += a;
            Check::Near (a, transform0.MaxDiff (transform1), "maxDiff");
            }
    }

void CheckXY (DPoint3dCR xyz, DPoint2dCR xy, char const*name)
    {
    Check::StartScope (name);
    Check::Near (xyz.x, xy.x, "x");
    Check::Near (xyz.y, xy.y, "y");
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, GetOriginAndVectors)
    {
    Transform transform0 = Transform::FromRowValues (100,2,3,  4,15,6, 7,8,19,10,11,12);
    DPoint3d origin3d;
    DVec3d vectorU3d, vectorV3d, vectorW3d;
    DVec3d vectorU3d1, vectorV3d1, vectorW3d1;
    DPoint2d origin2d;
    DVec2d vectorU2d, vectorV2d;
    transform0.GetOriginAndVectors (origin3d, vectorU3d, vectorV3d, vectorW3d);
    transform0.GetOriginAndVectors (origin2d, vectorU2d, vectorV2d);
    transform0.GetMatrixColumn (vectorU3d1, 0);
    transform0.GetMatrixColumn (vectorV3d1, 1);
    transform0.GetMatrixColumn (vectorW3d1, 2);
    Check::Near (vectorU3d, vectorU3d1, "U");
    Check::Near (vectorV3d, vectorV3d1, "V");
    Check::Near (vectorW3d, vectorW3d1, "W");
    CheckXY (vectorU3d, vectorU2d, "U");
    CheckXY (vectorV3d, vectorV2d, "V");
    CheckXY (origin3d, origin2d, "origin");

    RotMatrix matrix0;
    transform0.GetMatrix (matrix0);
    DVec3d vectorU3d2, vectorV3d2, vectorW3d2;
    matrix0.GetColumns (vectorU3d2, vectorV3d2, vectorW3d2);
    Check::Near (vectorU3d, vectorU3d2, "U");
    Check::Near (vectorV3d, vectorV3d2, "V");
    Check::Near (vectorW3d, vectorW3d2, "W");

    DPoint3d pointA = DPoint3d::From (0.3, 0.5, 0.12);
    DPoint3d pointB, pointC;
    transform0.OffsetPointByColumn (pointB, pointA, 0);
    pointC.SumOf (pointA, vectorU3d);
    Check::Near (pointB, pointC, "Add column 0 to point");

    transform0.OffsetPointByColumn (pointB, pointA, 1);
    pointC.SumOf (pointA, vectorV3d);
    Check::Near (pointB, pointC, "Add column to point");

    transform0.OffsetPointByColumn (pointB, pointA, 2);
    pointC.SumOf (pointA, vectorW3d);
    Check::Near (pointB, pointC, "Add column to point");

    Check::Near (
            transform0.Determinant (),
            matrix0.Determinant (), "Determinant");
    Check::Near (
            transform0.Determinant (),
            vectorW3d.TripleProduct (vectorU3d, vectorV3d), "Determinant");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, Init3)
    {
    Transform transform0 = Transform::FromRowValues (100,2,3,  4,15,6, 7,8,19,10,11,12);
    Transform transform2;
    transform2.Copy(transform0);
    Check::Near (transform0, transform2, "Copy");
    }

void testTransformImplicitPlane (TransformCR transform)
    {
    double a0 = 0.34, b0 = 1.2, c0 = 3.9, d0 = 4.1;
    double a1, b1, c1, d1;
    transform.TransformImplicitPlane (a1, b1, c1, d1, a0, b0, c0, d0);
    DVec3d vec0 = DVec3d::From (2,5,7);
    DVec3d vec1;
    for (int i = 0; i < 4; i++)
        {
        transform.Multiply (vec1, vec0);
        Check::Near (
            vec0.DotProduct (a0, b0, c0) - d0,
            vec1.DotProduct (a1, b1, c1) - d1,
            "Transformed plane");
        vec0.x += 3.0;
        vec0.y *= 1.0;
        vec0.z *= -0.3;
        }    
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, TransformPlane)
    {
    Transform transform;
    transform.InitIdentity ();
    testTransformImplicitPlane (transform);
    testTransformImplicitPlane (Transform::FromRowValues (100,2,3,  4,15,6, 7,8,19,10,11,12));
    }

void TestMixedTransforms (TransformCR transform, double ax, double ay, double az)
    {
    Check::StartScope ("MixedDimension transform * point");
    DVec3d U3, V3, W3;
    DVec2d U2, V2, W2;
    DPoint3d A3;
    DPoint2d A2;
    transform.GetOriginAndVectors (A3, U3, V3, W3);
    U2.Init (U3.x, U3.y);
    V2.Init (V3.x, V3.y);
    W2.Init (W3.x, W3.y);
    A2.Init (A3.x, A3.y);

    DPoint3d X3;
    DPoint2d X2;
    X3.Init (ax, ay, az);
    X2.Init (ax, ay);
    DPoint3d Y23;
    DPoint2d Y22, Y32;
    transform.Multiply (Y32, X3);
    transform.Multiply (Y23, X2);
    transform.Multiply (Y22, X2);

    DPoint3d Z23;
    DPoint2d Z32, Z22;
    Z23 = DPoint3d::FromSumOf (A3, U3, ax, V3, ay);
    Check::Near (Y23, Z23, "A*(x,y) ---> (xyz)");
    Z32 = DPoint2d::FromSumOf (A2, U2, ax, V2, ay, W2, az);
    Check::Near (Y32, Z32, "A*(x,y,z) ---> (xy)");
    Z22 = DPoint2d::FromSumOf (A2, U2, ax, V2, ay);
    Check::Near (Y22, Z22, "A*(x,y) ---> (xy)");
    Check::EndScope ();

#define NN 10
    DPoint3d X3n[NN], Y23n[NN];
    DPoint2d X2n[NN], Y22n[NN], Y32n[NN];
    int n = NN;
    for (int i = 0; i < n; i++)
        {
        X3n[i].Init (ax * i, ay * 2 * i, az * 3 * i);
        X2n[i].Init (ax * i, ay * 2 * i);
        }
    transform.Multiply (Y22n, X2n, NN);
    transform.Multiply (Y32n, X3n, NN);
    transform.Multiply (Y23n, X2n, NN);

    for (int i = 0; i < n; i++)
        {
        Z23.SumOf (A3, U3, X2n[i].x, V3, X2n[i].y);
        Check::Near (Z23, Y23n[i], "A*(x,y) ==> (x,y,z) in array");
        Z32.SumOf (A2, U2, X3n[i].x, V2, X3n[i].y, W2, X3n[i].z);
        Check::Near (Z23, Y23n[i], "A*(x,y) ==> (x,y,z) in array");
        Z22.SumOf (A2, U2, X3n[i].x, V2, X3n[i].y);
        Check::Near (Z22, Y22n[i], "A*(x,y) ==> (x,y) in array");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform,MixedTransforms)
    {
    Transform transform0 = Transform::FromRowValues (100,2,3,  4,15,6, 7,8,19,10,11,12);
    TestMixedTransforms (transform0, 0.23, 0.4, -0.2);
    TestMixedTransforms (transform0, 1, 0, 0);
    TestMixedTransforms (transform0, 0, 1, 0);
    TestMixedTransforms (transform0, 0, 0, 1);
    }


DPoint4d SumOf (DPoint4dCR A0, double a0, DPoint4dCR A1, double a1, DPoint4dCR A2, double a2, DPoint4dCR A3, double a3)
    {
    DPoint4d result;
    result.SumOf (A0, a0, A1, a1);
    result.SumOf (result, A2, a2, A3, a3);
    return result;
    }

void TestXYZW (TransformCR transform, double ax, double ay, double az, double aw)
    {
    Check::StartScope ("MixedDimension transform * point");
    DVec3d U3, V3, W3;
    DPoint3d A3;
    transform.GetOriginAndVectors (A3, U3, V3, W3);
    DPoint4d A4 = DPoint4d::From (A3, 1.0);
    DPoint4d U4 = DPoint4d::From (U3, 0.0);
    DPoint4d V4 = DPoint4d::From (V3, 0.0);
    DPoint4d W4 = DPoint4d::From (W3, 0.0);
    DPoint4d X4, Y4, Z4;
    X4.SetComponents (ax, ay, az, aw);
    transform.Multiply (Y4, X4);
    Z4 = SumOf (U4, ax, V4, ay, W4, az, A4, aw);
    Check::Near (Y4, Z4, "A*(x,y,z,w)");
#define NN 10
    DPoint4d X4n[NN], Y4n[NN];
    int n = NN;
    for (int i = 0; i < n; i++)
        {
        X4n[i].SetComponents (ax * i, ay * 2 * i, az * 3 * i, aw * 4 * i);
        }
    transform.Multiply (Y4n, X4n, NN);
    for (int i = 0; i < n; i++)
        {
        Z4 = SumOf (U4, X4n[i].x, V4, X4n[i].y, W4, X4n[i].z, A4, X4n[i].w);
        Check::Near (Z4, Y4n[i], "A*(x,y,z,w) in array");
        }
    Check::EndScope ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform,XYZWTransforms)
    {
    Transform transform0 = Transform::FromRowValues (100,2,3,  4,15,6, 7,8,19, 10,11,12);
    TestXYZW (transform0, 0.23, 0.4, -0.2, 0.3);
    }

void TestQuickTranslate (TransformCR A, double ax, double ay, double az, double s)
    {
    Transform  B = Transform::FromRowValues (
                    1,0,0,  s * ax,
                    0,1,0,  s * ay,
                    0,0,1,  s * az
                    );
    DVec3d b = DVec3d::From (ax, ay, az);
    Transform AB, BA;
    AB.InitProduct (A, B);
    BA.InitProduct (B, A);
    Transform AB1, BA1;
    AB1.MultiplyTransformTranslation (A, b, s);
    BA1.MultiplyTranslationTransform (b, s, A);
    Check::Near (AB, AB1, "Fast Transform*(scale, translate)");
    Check::Near (BA, BA1, "Fast (scale, translate)*Transform");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform,QuickTranslate)
    {
    Transform A = Transform::FromRowValues (100,2,3,  4,15,6, 7,8,19,10,11,12);
    TestQuickTranslate (A, 0,0,0,4.52);
    TestQuickTranslate (A, 7,2,3, 1);
    TestQuickTranslate (A, 7,2,3, 5);
    }

static DPoint3d s_points[] =
    {
    {0,0,0},
    {1,0,0},
    {0,1,0},
    {0,0,1},
    {1,1,0},
    {1,1,1},
    {1,2,3},
    {-2,3,9},
    }; 
void TestScaleAroundPlane (DPoint3dCR origin, DVec3dCR normal)
    {
    Transform mirror, onPlane;
    mirror.InitFromMirrorPlane (origin, normal);
    onPlane.InitFromProjectionToPlane (origin, normal);
    Transform projection[4];
    double scaleFactor[4] = {0,-1,2,3};

    for (int j = 0; j < 4; j++)
        projection[j].InitFromScalePerpendicularToPlane (origin, normal, scaleFactor[j]);


    int n = _countof(s_points);
    for (int i = 0; i < n; i++)
        {
        DPoint3d pointA = s_points[i];
        DPoint3d pointM, image;
        DPoint3d pointOn;
        mirror.Multiply (pointM, pointA);
        Check::Near (pointA.DotDifference(origin, normal), -pointM.DotDifference(origin, normal), "Mirror");

        onPlane.Multiply (pointOn, pointA);
        Check::Near (0.0, pointOn.DotDifference(origin, normal), "Project");

        for (int j = 0; j < 4; j++)
            {
            projection[j].Multiply (image, pointA);
            double dA = pointA.DotDifference(origin, normal);
            double d  = image.DotDifference(origin, normal);
            Check::Near (scaleFactor[j] * dA, d, "Scale");
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, Mirror)
    {
    TestScaleAroundPlane (DPoint3d::From (0,0,0), DVec3d::From (1,0,0));
    TestScaleAroundPlane (DPoint3d::From (0.3,0.2,0.1), DVec3d::From (1.3,0.4,0.2));
    
    }
    

void testSolve (TransformCR forward)
    {
    Transform inverse;
    if (Check::True (inverse.InverseOf (forward), "inverse"))
        {
        DPoint3d X, Y0, Y1;
        X = DPoint3d::From (0.5, 1.5, 2.333);
        Check::True (forward.Solve (Y0, X), "Solve");
        inverse.Multiply (Y1, X);
        Check::Near (Y0, Y1, "Inverse*X = Solve (forward, X)");
        Transform AB = Transform::FromProduct (forward, inverse);
        Check::True (AB.IsIdentity (), "Ainv*A=I");
        }
    }    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, Solve)
    {
    testSolve (Transform::FromRowValues
            (
            1,0,0,1,
            0,2,1,2,
            0,0,3,2
            ));
    double a = 0.001;
    testSolve (Transform::FromRowValues
            (
            a,0,0, 1,
            0,a,0, 2,
            0,0,a, 3
            ));
    }

//! ASSUME map.M0 is world to view
//!        map.M1 is view back to world.
//!     Hence map.M1 column Z is eyepoint in world.
//! @param [in] map viewing transformation.  Eyepoint is at 0010 after transformation.
//! @param [in] planeOigin plane origin BEFORE viewing transform
//! @param [in] planeNormal plane normal BEFORE viewing transform
//! @parma [in] testPoint point in world.
//! @return true if testPoint and eyepoint are on same side of the plane.
bool IsPointInFrontOfPlaneAsViewed (DMap4dCR map, DPoint3dCR planeOrigin, DVec3dCR planeNormal, DPoint3dCR testPoint)
    {
    DPoint4d homogeneousPlane;
    
    if (homogeneousPlane.PlaneFromOriginAndNormal (planeOrigin, planeNormal))
        {
        static double s_relTol = 1.0e-5;    // fluffy viewing tolerance?
        DPoint4d worldEyePoint;
        map.M1.GetColumn (worldEyePoint, 2);
        double a0 = homogeneousPlane.DotProduct (worldEyePoint);
        double a1 = homogeneousPlane.DotProduct (testPoint, 1.0);
        double tol = s_relTol * (1.0 + fabs (a0) + fabs (a1) + fabs (homogeneousPlane.w));
        return a0 * a1 >0 || fabs (a1) < tol;
        }
    return false;    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (DMatrix4d, IsPointInFrontAsViewed)
    {
    DPoint3d planeOrigin = DPoint3d::From (1,2,3);
    DVec3d planeNormal = DVec3d::From (1,1,5);
    DVec3d planeX, planeY, planeZ;
    planeNormal.GetNormalizedTriad (planeX, planeY, planeZ);
    DPoint3d rayOrigin = DPoint3d::FromSumOf (planeOrigin, planeX, 2, planeY, 3);  // somewhere else on the plane.
    DMap4d map;
    map.InitIdentity ();
    // NOTE: Eyepoint is at +Zinfinity. 
    DVec3d rayDirection = DVec3d::FromSumOf (planeX, 0.3, planeY, 0.1, planeZ, 3);    // clearly out of plane, but not perpendicular
    for (double f = -2.0; f < 3.0; f+= 0.993)   // stay away from 0.0
        {
        DPoint3d testPoint = DPoint3d::FromSumOf (rayOrigin, rayDirection, f);
        double testPointAltitude = testPoint.DotDifference (planeOrigin, planeNormal);
        Check::Bool (testPointAltitude * planeNormal.z > 0.0 , IsPointInFrontOfPlaneAsViewed (map, planeOrigin, planeNormal, testPoint), "eye plane test");
        }
        
    Check::Bool (true, IsPointInFrontOfPlaneAsViewed (map, planeOrigin, planeNormal, planeOrigin), "eye plane test -- plane origin itself");
    }


//! Decompose a transform into a product
//!  transform = Translation * Rotation * Scale
//! @param [in] transform result transform
//! @param [out] translation xyz translation
//! @param [out] rotation pure rotation.
//! @param [out] principal axis scaling
bool FactorTranslateRotateScale (TransformCR transform, DPoint3dR translation, RotMatrixR rotation, DVec3dR scales)
    {
    DVec3d xVec, yVec, zVec;
    transform.GetOriginAndVectors (translation, xVec, yVec, zVec);
    scales.x = xVec.Normalize ();
    scales.y = yVec.Normalize ();
    scales.z = zVec.Normalize ();
    rotation.InitFromColumnVectors (xVec, yVec, zVec);
    return rotation.IsRigid ();
    }

//! Determine a transofrm
//!    transform = translation * rotationAroundOrigin * primaryAxisScaleFactors
//! @param [in] parentFrame a coordinate frame
//! @param [in] childFrame  another coordinate frame, in the same coordinates as the parentFrame
//! @param [out] childFromParent child coordinate frame as viewed in the parent system.
//! @param [out] origin coordinates of the child frame origin as viewed from the parent frame origin.
//! @param [out] rotation rotation of the child frame axes relative to the parent frame
//! @param [out] scales x,y,z scale factors.
//!
//! @return false if either (a) the parent frame is singular or (b) the transition from parent to child has an additional rotation
//!         "before the " principal axis scaling.
bool FactorIncrementalTranslateRotateScale (TransformCR parentFrame, TransformCR childFrame, TransformR childFromParent,
DPoint3dR translation,
RotMatrixR rotation,
DVec3dR scales
)
    {
    Transform parentInverse;
    if (parentInverse.InverseOf (parentFrame))
        {
        childFromParent = Transform::FromProduct (parentInverse, childFrame);
        return FactorTranslateRotateScale (childFromParent, translation, rotation, scales);
        }
    return false;
    }

//! Assemble a transofrm
//!    transform = translation * rotationAroundOrigin * primaryAxisScaleFactors
Transform AssembleTranslateRotateScale (DPoint3dCR origin, RotMatrixCR rotation, DVec3dCR scales)
    {
    RotMatrix matrix = rotation;
    matrix.ScaleColumns (rotation, scales.x, scales.y, scales.z);
    return Transform::From (matrix, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Transform, FactorTranslateRotateScale)
    {
    DPoint3d origin01 = DPoint3d::From (2,4,7);
    DVec3d vector01 = DVec3d::From (1,2,3);
    double theta01 = 0.2;
    RotMatrix rotation01 = RotMatrix::FromVectorAndRotationAngle (vector01, theta01);
    DVec3d scale01 = DVec3d::From (1,1,1);
    Transform frame1 = AssembleTranslateRotateScale (origin01, rotation01, scale01);
    
    DPoint3d origin12 = DPoint3d::From (5,1,3);
    DVec3d vector12 = DVec3d::From (2,7,3);;
    DVec3d scale12 = DVec3d::From (1.1,3,2.3);
    double theta12 = 0.5;
    RotMatrix rotation12 = RotMatrix::FromVectorAndRotationAngle (vector12, theta12);
    Transform transform12 = AssembleTranslateRotateScale (origin12, rotation12, scale12);
    Transform frame2 = Transform::FromProduct (frame1, transform12);

    DPoint3d origin12A;
    RotMatrix rotation12A;
    DVec3d scale12A;
    Transform transform12A;
    if (Check::True (FactorIncrementalTranslateRotateScale (frame1, frame2,
                transform12A, origin12A, rotation12A, scale12A),
                "FactorIncremental"))
        {
        Check::Near (origin12, origin12A, "origin");
        Check::Near (rotation12, rotation12A, "rotation");
        Check::Near (scale12, scale12A, "scale");
        }
                    
    }
// Return a transform which (conceptually) applies roll around X, pitch around Y, yaw around Z, all with given point fixed.
//
Transform Transform_FromXRollYPitchZYawAroundPoint (DPoint3dCR fixedPoint, double rollRadians, double pitchRadians, double yawRadians)
    {
    RotMatrix roll = RotMatrix::FromAxisAndRotationAngle (0, rollRadians);
    RotMatrix pitch = RotMatrix::FromAxisAndRotationAngle (1, pitchRadians);
    RotMatrix yaw = RotMatrix::FromAxisAndRotationAngle (2, yawRadians);
    RotMatrix product;
    product.InitProduct (yaw, pitch, roll);
    return Transform::FromMatrixAndFixedPoint (product, fixedPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Transform, BuildRollPitchYaw)
    {
    DPoint3d center = DPoint3d::From (1,2,3);
    double roll = 0.1;
    double pitch = 0.2;
    double yaw = 0.3;
    Transform transformRoll = Transform_FromXRollYPitchZYawAroundPoint (center, roll, 0, 0);
    Transform transformPitch = Transform_FromXRollYPitchZYawAroundPoint (center, 0, pitch, 0);
    Transform transformYaw = Transform_FromXRollYPitchZYawAroundPoint (center, 0, 0, yaw);
    Transform transformA = Transform::FromProduct (transformYaw, transformPitch, transformRoll);
    Transform transformB = Transform_FromXRollYPitchZYawAroundPoint (center, roll, pitch, yaw);
    Check::Near (transformA, transformB, "rollPitchYaw composition");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ConeTransforms,ValidateAxes)
    {
#ifdef abc
    Transform t =  Transform::From(
        0,    -0.000866, -0.0005,   0,
        0,     0.0005,   -0.000866, 0,
        0.001, 0,         0,        0);
#endif
    RotMatrix A =  RotMatrix::FromRowValues (
        0,    -0.000866, -0.0005,
        0,     0.0005,   -0.000866,
        0.001, 0,         0);

    RotMatrix ATA;
    ATA.InitProductRotMatrixTransposeRotMatrix (A, A);
    Check::Print (A, "A");
    Check::Print (ATA, "ATA");
    RotMatrix B, C;
    DPoint3d Q;
    A.FactorRotateScaleRotate (B,Q,C);
    Check::Print (B, "B");
    Check::Print (Q, "Q");
    Check::Print (C, "C");
    double degrees = Angle::RadiansToDegrees (atan2 (A.form3d[1][1], A.form3d[0][1]));
    Check::Print (degrees, "degrees");
    Angle q = Angle::FromDegrees (60.0);
    Check::Print (q.Cos(), "cos (60)");
    Check::Print (q.Sin (), "sin (60)");
    }
struct DMap3d;
typedef DMap3d const &DMap3dCR;
typedef ValidatedValue<DMap3d> ValidatedDMap3d;
//! A DMap4d is a combination of two 3d-to-3d transformations for 2-way linear mappings.
//! The 2 transformations are defined in a different way than the usual 3 by 4 matrix.
//! <ul>
//! <li> The stored data is two points (DPoint3d) origin0 and origin1, and two matrices (RotMatrix) matrix0 and Matrix1
//! <li>The algebraic form for the two transformations to a point xyz are:
//!   <ul>
//!   <li>Transform0:     result = origin1 + matrix0 * (xyz - origin0)
//!   <li>Transform1:     result = origin0 + matrix1 * (xyz - origin1)
//!   </ul>
//! <li> Simple manipulation shows that if matrix0 and matrix1 are inverses these two transforms are inverses.
//!    <ul>
//!    <li>The standard for of Transform0 has
//!       <ul>
//!       <li>"matrix part" matrix0
//!       <li>"translation part" origin1 - matrix0 * origin0
//!       </ul>
//!    <li>The standard for of Transform0 has
//!       <ul>
//!       <li>"matrix part" matrix1
//!       <li>"translation part" origin0 - matrix0 * origin1
//!       </ul>
//!     </ul>
//! <li>It is very rare for a caller to compute and supply both matrices or points.
//! <li>Rather, the caller will supply just one matrix and one point to a "From" method that computes the other appropriately.
//!    <ul>
//!    <li>When the matrix describes a coordinate frame (with local xyz axes as columns):
//!       <ul>
//!       <li>call DMap3d::FromOriginAndAxes(origin, axes)
//!       <li>Transform0 is the "local to world" transformation
//!       <li>Transform1 the "world To Local" transformation
//!       </ul>
//!    <li>When the matrix describes an action such as rotation, scaling, mirroring
//!       <ul>
//!       <li>call DMap3d::FromFixedPointAndMatrix(fixedPoint, matrix)
//!       <li>Transform0 applies that matrix to vectors "from the fixed point"
//!       <li>Transform1 reverses the action.
//!       </ul>
//! </ul>
//!
//!
struct DMap3d
{
private:
    DMap3d ();
    DPoint3d  m_origin [2];
    RotMatrix m_matrix [2];

public:
DMap3d (DPoint3dCR origin0, RotMatrixCR matrix0, DPoint3dCR origin1, RotMatrixCR matrix1);

DMap3d (DPoint3dCR origin);

// Create a map in which "Transform0" is local to world with given origin and axes, and "Transform1" is world to local.
static ValidatedDMap3d FromOriginAndAxes (DPoint3dCR origin, RotMatrixCR axes);

// Create a map in which Transform0 applies the operation matrix to (X-origin), and Transform1 is the inverse.
static ValidatedDMap3d FromFixedPointAndMatrix (DPoint3dCR origin, RotMatrixCR operation);

DPoint3d MultiplyTransform0 (DPoint3dCR xyz0) const;
DPoint3d MultiplyTransform1 (DPoint3dCR xyz1) const;

DPoint3d MultiplyTransform0 (double x, double y, double z) const;
DPoint3d MultiplyTransform1 (double x, double y, double z) const;

DVec3d MultiplyMatrix0 (DVec3dCR vector) const;
DVec3d MultiplyMatrix1 (DVec3dCR vector) const;

void MultiplyTransform0 (bvector<DPoint3d> &data) const;

void MultiplyTransform1 (bvector<DPoint3d> &data) const;
//! Return the common form of Transform0
Transform GetTransform0 () const;
//! Return the common form of Transform1
Transform GetTransform1 () const;

DMap3d Multiply (DMap3dCR mapB) const;
//! Return the DMap3d with matrix and origins swapped.
DMap3d Inverse () const;
};

DMap3d DMap3d::Inverse () const
    {
    return DMap3d (m_origin[1], m_matrix[1], m_origin[0], m_matrix[0]);
    }

DMap3d DMap3d::Multiply (DMap3dCR other) const
    {
    RotMatrix matrix0, matrix1;
    matrix0.InitProduct (m_matrix[0], other.m_matrix[0]);
    matrix1.InitProduct (other.m_matrix[1], m_matrix[1]);
    DPoint3d origin0 = other.m_origin[0];
    DPoint3d origin1 = MultiplyTransform0 (other.m_origin[1]);
    return DMap3d (origin0, matrix0, origin1, matrix1);
    }

Transform DMap3d::GetTransform0 () const {return Transform::From (m_matrix[0], MultiplyTransform0 (0,0,0));}
Transform DMap3d::GetTransform1 () const {return Transform::From (m_matrix[1], MultiplyTransform1 (0,0,0));}

DMap3d::DMap3d (DPoint3dCR origin)
    {
    m_origin[1] = origin;
    m_origin[0].Zero ();
    m_matrix[0] = RotMatrix::FromIdentity ();
    m_matrix[1] = RotMatrix::FromIdentity ();
    }

DMap3d::DMap3d (DPoint3dCR origin0, RotMatrixCR matrix0, DPoint3dCR origin1, RotMatrixCR matrix1)
    {
    m_origin[0] = origin0;
    m_origin[1] = origin1;
    m_matrix[0] = matrix0;
    m_matrix[1] = matrix1;
    }

ValidatedDMap3d DMap3d::FromOriginAndAxes (DPoint3dCR origin, RotMatrixCR axes)
    {
    RotMatrix inverse;
    DPoint3d zero;
    zero.Zero ();
    if (inverse.InverseOf (axes))
        {
        return ValidatedDMap3d (DMap3d (zero, axes, origin, inverse));
        }
    return ValidatedDMap3d (DMap3d (origin));
    }

ValidatedDMap3d DMap3d::FromFixedPointAndMatrix (DPoint3dCR origin, RotMatrixCR operation)
    {
    RotMatrix inverse;
    DPoint3d zero;
    zero.Zero ();
    if (inverse.InverseOf (operation))
        {
        return ValidatedDMap3d (DMap3d (origin, operation, origin, inverse));
        }
    return ValidatedDMap3d (DMap3d (origin));
    }

DPoint3d DoMultiply (DPoint3dCR pointA, RotMatrixCR matrix, DPoint3dCR pointX, DPoint3dCR pointB)
    {
    double u = pointX.x - pointB.x;
    double v = pointX.y - pointB.y;
    double w = pointX.z - pointB.z;
    return DPoint3d::From (
        pointA.x + matrix.form3d[0][0] * u + matrix.form3d[0][1] * v + matrix.form3d[0][2] * w,
        pointA.y + matrix.form3d[1][0] * u + matrix.form3d[1][1] * v + matrix.form3d[1][2] * w,
        pointA.z + matrix.form3d[2][0] * u + matrix.form3d[2][1] * v + matrix.form3d[2][2] * w
        );
    }

DPoint3d DoMultiply (DPoint3dCR pointA, RotMatrixCR matrix, double x, double y, double z, DPoint3dCR pointB)
    {
    double u = x - pointB.x;
    double v = y - pointB.y;
    double w = z - pointB.z;
    return DPoint3d::From (
        pointA.x + matrix.form3d[0][0] * u + matrix.form3d[0][1] * v + matrix.form3d[0][2] * w,
        pointA.y + matrix.form3d[1][0] * u + matrix.form3d[1][1] * v + matrix.form3d[1][2] * w,
        pointA.z + matrix.form3d[2][0] * u + matrix.form3d[2][1] * v + matrix.form3d[2][2] * w
        );
    }



void DoMultiply (DPoint3dR result, DPoint3dCR pointA, RotMatrixCR matrix, DPoint3dCR pointX, DPoint3dCR pointB)
    {
    double u = pointX.x - pointB.x;
    double v = pointX.y - pointB.y;
    double w = pointX.z - pointB.z;
    result.x = pointA.x + matrix.form3d[0][0] * u + matrix.form3d[0][1] * v + matrix.form3d[0][2] * w;
    result.y = pointA.y + matrix.form3d[1][0] * u + matrix.form3d[1][1] * v + matrix.form3d[1][2] * w;
    result.z = pointA.z + matrix.form3d[2][0] * u + matrix.form3d[2][1] * v + matrix.form3d[2][2] * w;
    }

DPoint3d DMap3d::MultiplyTransform0 (DPoint3dCR xyz0) const
    {
    return DoMultiply (m_origin[1], m_matrix[0], xyz0, m_origin[0]);
    }

DPoint3d DMap3d::MultiplyTransform1 (DPoint3dCR xyz1) const
    {
    return DoMultiply (m_origin[0], m_matrix[1], xyz1, m_origin[1]);
    }

DVec3d DMap3d::MultiplyMatrix0 (DVec3dCR vector) const
    {
    DVec3d result;
    m_matrix[0].Multiply (result, vector);
    return result;
    }

DVec3d DMap3d::MultiplyMatrix1 (DVec3dCR vector) const
    {
    DVec3d result;
    m_matrix[1].Multiply (result, vector);
    return result;
    }


DPoint3d DMap3d::MultiplyTransform0 (double x, double y, double z) const
    {
    return DoMultiply (m_origin[1], m_matrix[0], x, y, z, m_origin[0]);
    }

DPoint3d DMap3d::MultiplyTransform1 (double x, double y, double z) const
    {
    return DoMultiply (m_origin[0], m_matrix[1], x, y, z, m_origin[1]);
    }



void DMap3d::MultiplyTransform0 (bvector<DPoint3d> &data) const
    {
    for (auto &xyz : data)
        DoMultiply (xyz, m_origin[1], m_matrix[0], xyz, m_origin[0]);
    }

void DMap3d::MultiplyTransform1 (bvector<DPoint3d> &data) const
    {
    for (auto &xyz : data)
        DoMultiply (xyz, m_origin[0], m_matrix[1], xyz, m_origin[1]);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMap3d,LocalToWorld)
    {
    bvector <DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (1,2,3));
    points.push_back (DPoint3d::From (-1,2,-3));
    points.push_back (DPoint3d::From (1,-2.12321214234,3));


    for (auto theta : bvector<double> {0.0, 0.2, -0.2532423479879})
        {
        RotMatrix axes = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), theta);
        DPoint3d origin = DPoint3d::From (1,2,3);
        auto vmap = DMap3d::FromOriginAndAxes (origin, axes);
        auto map = vmap.Value ();
        auto localToWorld = Transform::From (axes, origin);
        Transform worldToLocal;
        worldToLocal.InverseOf (localToWorld);
        Check::Near (localToWorld, map.GetTransform0 (), "DMap3d::GetTransform0");
        Check::Near (worldToLocal, map.GetTransform1 (), "DMap3d::GetTransform1");
        for (auto xyzA : points)
            {
            DPoint3d xyzB0, xyzB1, xyzC0, xyzC1;
            localToWorld.Multiply (xyzB0, xyzA);
            xyzB1 = map.MultiplyTransform0 (xyzA);
            Check::Near (xyzB0, xyzB1);
            worldToLocal.Multiply (xyzC0, xyzB0);
            xyzC1 = map.MultiplyTransform1 (xyzB0);
            Check::Near (xyzC0, xyzC1);
            Check::Near (xyzC0, xyzA);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMap3d,MatrixAndFixedPoint)
    {
    bvector <DPoint3d> points;
    points.push_back (DPoint3d::From (0,0,0));
    points.push_back (DPoint3d::From (1,2,3));
    points.push_back (DPoint3d::From (-1,2,-3));
    points.push_back (DPoint3d::From (1,-2.12321214234,3));


    for (auto theta : bvector<double> {0.0, 0.2, -0.2532423479879})
        {
        RotMatrix axes = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), theta);
        DPoint3d origin = DPoint3d::From (1,2,3);
        auto vmap = DMap3d::FromFixedPointAndMatrix (origin, axes);
        auto map = vmap.Value ();
        auto localToWorld = Transform::FromMatrixAndFixedPoint (axes, origin);
        Transform worldToLocal;
        worldToLocal.InverseOf (localToWorld);

        Check::Near (localToWorld, map.GetTransform0 (), "DMap3d::GetTransform0");
        Check::Near (worldToLocal, map.GetTransform1 (), "DMap3d::GetTransform1");
        for (auto xyzA : points)
            {
            DPoint3d xyzB0, xyzB1, xyzC0, xyzC1;
            localToWorld.Multiply (xyzB0, xyzA);
            xyzB1 = map.MultiplyTransform0 (xyzA);
            Check::Near (xyzB0, xyzB1);
            worldToLocal.Multiply (xyzC0, xyzB0);
            xyzC1 = map.MultiplyTransform1 (xyzB0);
            Check::Near (xyzC0, xyzC1);
            Check::Near (xyzC0, xyzA);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMap3d,Compose)
    {
    for (auto theta : bvector<double> {0.0, 0.2, -0.2532423479879})
        {
        RotMatrix axesA = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (1,2,3), theta);
        RotMatrix axesB = RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.3, 2.9, 4.0), 2.167 * theta);
        auto originA = DPoint3d::From (2,4,1);
        auto originB = DPoint3d::From (-2,3,8);
        auto mapA = DMap3d::FromOriginAndAxes (originA, axesA);
        auto mapB = DMap3d::FromOriginAndAxes (originB, axesB);
        auto localToWorldA = mapA.Value().GetTransform0 ();
        auto worldToLocalA = mapA.Value().GetTransform1 ();

        auto localToWorldB = mapB.Value().GetTransform0 ();
        auto worldToLocalB = mapB.Value().GetTransform1 ();
        Transform localToWorldD = Transform::FromProduct (localToWorldA, localToWorldB);
        Transform worldToLocalD= Transform::FromProduct (worldToLocalB, worldToLocalA);
        auto mapC = mapA.Value ().Multiply (mapB.Value ());
        auto localToWorldC = mapC.GetTransform0 ();
        auto worldToLocalC = mapC.GetTransform1 ();
        Check::Near (localToWorldC, localToWorldD, "DMap3d Composition 0");
        Check::Near (worldToLocalC, worldToLocalD, "DMap3d Composition 1");

        }

    }

void BuildInwardSpiral (double df, double fMax, bvector<DPoint3d> &xyz)
    {
    // Get points on a spiral sweeping from (1,0) in towards the origin.   f is a fractional part of a circle.
    for (double f = 0.0; f < fMax; f += df)
        {
        double radians = f * Angle::TwoPi ();
        double radius = 1.0 / (1.0 + f);
        xyz.push_back (DPoint3d::From (radius * cos(radians), radius * sin(radians), 0.0));
        }

    }
void DMap3dRoundoffTests (int selector, DVec3dCR vector)
    {
    bvector<DPoint3d> points;
    bvector<DPoint3d> origins;
    BuildInwardSpiral (0.0832423, 2.1, points);
    BuildInwardSpiral (0.1287321, 2.5, origins);
    bvector<double> dataScales {1.0, 0.289882374, 8.3243897987};
    Check::PrintIndent (1);
    if (selector == 1)
        {
        Check::Print (vector, "Rotate around");
        Check::Print (dataScales, "angles");
        }
    else if (selector == 2)
        {
        Check::Print (vector, "Directional Scale");
        Check::Print (dataScales, "scales factors");
        }
    else
        Check::Print (dataScales, "uniform scale factors");

    for (DPoint3d origin0 : origins)
        {
        Check::PrintIndent (0);
        Check::Print (origin0, "PrimaryOrigin");
        for (double originScale = 1.0; originScale < 1.0e12; originScale *= 100.0)
            {
            DPoint3d origin = origin0;
            origin.Scale (originScale);
            Check::PrintIndent (2);
            Check::Print (origin, "origin");
            UsageSums eAffine, eMap;
            for (double dataScale: dataScales)
                {
                RotMatrix matrix;
                if (selector == 1)
                    {
                    matrix = RotMatrix::FromVectorAndRotationAngle (vector, dataScale);
                    }
                else if (selector == 2)
                    {
                    matrix = RotMatrix::FromDirectionAndScale (vector, dataScale);
                    }
                else
                    {
                    matrix = RotMatrix::FromScale (dataScale);
                    }
                    DMap3d map = DMap3d::FromFixedPointAndMatrix (origin, matrix).Value ();
                Transform transform0 = map.GetTransform0 ();
                Transform transform1 = map.GetTransform1 ();
                for (DPoint3d xyz : points)
                    {
                    DPoint3d xyzA = origin + DVec3d::From(xyz);   // Place the point "out there"
                    DPoint3d xyzAffine1 = transform0 * xyzA; // Traditional Transform
                    DPoint3d xyzAffine2 = transform1 * xyzAffine1;  // and inverse
                    DPoint3d xyzMap1    = map.MultiplyTransform0 (xyzA);
                    DPoint3d xyzMap2    = map.MultiplyTransform1 (xyzMap1);
                    double dAffine = xyzAffine2.Distance (xyzA);
                    double dMap    = xyzMap2.Distance (xyzA);
                    eAffine.Accumulate (dAffine);
                    eMap.Accumulate (dMap);
#ifdef PrintAll
                    Check::PrintIndent (6);
                    Check::Print(xyz, "primary point");
                    Check::Print (dAffine, "affine e");
                    Check::Print (dMap, "map e");
#endif
                    }
                }
            Check::Print (eAffine.Sum (), "Affine Sum");
            Check::Print (eMap.Sum (), "Map Sum");
            Check::Print (DoubleOps::ValidatedDivide (eAffine.Sum (), eMap.Sum ()), "A/M");
            }
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMap3d,RoundoffTest)
    {
    DVec3d vector = DVec3d::From (1,2,5);
    DMap3dRoundoffTests (0, vector);
    DMap3dRoundoffTests (1, vector);
    DMap3dRoundoffTests (2, vector);
    }

void TestSweepToPlane (DVec3dCR sweepVector, DPlane3dCR plane)
    {
    auto transform = Transform::FromSweepToPlane (sweepVector, plane);
    Check::True (transform.IsValid (), "Validate Transform");
    bvector<DPoint3d> points
        {
        DPoint3d::From (0,0,0),
        DPoint3d::From (1,2,5),
        DPoint3d::From (-2,3,4)
        };
    for (auto xyz : points)
        {
        DPoint3d xyz1 = transform *xyz;
        Check::Near (0.0, plane.Evaluate (xyz1));
        Check::Parallel (sweepVector, xyz1 - xyz);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Transform,ProjectToPlane)
    {
    TestSweepToPlane (
        DVec3d::From (0,0,1),
        DPlane3d::FromOriginAndNormal (0,0,0,   0,0,1)
        );
    TestSweepToPlane (
        DVec3d::From (1,1,1),
        DPlane3d::FromOriginAndNormal (0,0,0,   0,0,1)
        );
    TestSweepToPlane (
        DVec3d::From (0,0,1),
        DPlane3d::FromOriginAndNormal (0,0,0,   1,2,3)
        );
    TestSweepToPlane (
        DVec3d::From (0,0,1),
        DPlane3d::FromOriginAndNormal (2,4,-2,   0,0,1)
        );
    TestSweepToPlane (
        DVec3d::From (0,0,1),
        DPlane3d::FromOriginAndNormal (2,4,-2,   1,2,3)
        );
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Transform,ColumnAccess)
    {
    Transform transform0 = Transform::FromRowValues
            (
            1,2,3,5,
            6,7,8,9,
            10,11,12,13
            );
    auto columnX = transform0.ColumnX ();
    auto columnY = transform0.ColumnY ();
    auto columnZ = transform0.ColumnZ ();
    auto translation = transform0.Translation ();
    auto transform1 = Transform::FromOriginAndVectors (translation, columnX, columnY, columnZ);
    Check::Near (transform0, transform1, "Round trip column accesors");
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(Transform, CheckTrans)
    {
    Transform tr = Transform::FromRowValues (0.0, 10.0, 0.0, 32.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 3.0);
    Transform tr2;
    tr2.Copy(tr);
    DPoint2d translation = DPoint2d::From(0, 0);
    tr2.SetTranslation(translation);
    tr.ZeroTranslation();
    Check::True(tr2.IsEqual(tr));
    DPoint2d pt2;
    tr2.GetTranslation(pt2);
    Check::ExactDouble(0, pt2.x);
    Check::ExactDouble(0, pt2.y);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(Transform, SetGetRowColumn) 
    {
    Transform tr;
    tr.InitIdentity();
    tr.SetMatrixRow(DVec3d::From(2, 4, 1), 2);
    tr.SetMatrixRow(DVec3d::From(2, 9, 0), 1);
    tr.SetMatrixRow(DVec3d::From(4, 1, 1), 0);
    DVec3d points;
    tr.GetMatrixRow(points, 2);
    Check::ExactDouble(points.GetComponent(0), 2);
    Check::ExactDouble(points.GetComponent(1), 4);
    Check::ExactDouble(points.GetComponent(2), 1);
    tr.GetMatrixRow(points, 1);
    Check::ExactDouble(points.x, 2);
    Check::ExactDouble(points.y, 9);
    Check::ExactDouble(points.z, 0);
    tr.GetMatrixRow(points, 0);
    Check::ExactDouble(points.GetComponent(0), 4);
    Check::ExactDouble(points.GetComponent(1), 1);
    Check::ExactDouble(points.GetComponent(2), 1);
    tr.SetMatrixColumn(DVec3d::From(4, 4, 4), 2);
    tr.GetMatrixColumn(points, 2);
    Check::ExactDouble(points.GetComponent(0), 4);
    Check::ExactDouble(points.GetComponent(1), 4);
    Check::ExactDouble(points.GetComponent(2), 4);
    tr.SetMatrixByRowAndColumn(2, 2, 22);
    Check::ExactDouble(22, tr.GetFromMatrixByRowAndColumn(2, 2));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(Transform, SetFixedPoint)
    {
    Transform tr = Transform::FromRowValues(1, 8, 10, 1, 10, 12, 9, 5, 0, 0, 1, 7);
    DPoint2d pt2D = DPoint2d::From(2, 3);
    DPoint3d trans;
    tr.GetTranslation(trans);
    Transform tr2;
    tr2.Copy(tr);
    Check::True(tr2.IsEqual(tr));
    tr.SetFixedPoint(pt2D);
    trans = tr.Translation();
    Check::ExactDouble(-24, trans.GetComponent(0));
    Check::ExactDouble(-53, trans.GetComponent(1));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(Transform, OriginCheck)
    {
    Transform tr = Transform::FromRowValues(1, 8, 10, 1, 10, 12, 9, 5, 0, 0, 1, 7);
    DPoint3d pt3D = tr.Origin();
    Check::ExactDouble(1, pt3D.GetComponent(0));
    Check::ExactDouble(5, pt3D.GetComponent(1));
    Check::ExactDouble(7, pt3D.GetComponent(2));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(Transform, MatrixColumnMagnitude)
    {
    Transform tr;
    tr.InitFromScaleFactors(5, 8, 3);
    Check::ExactDouble(5.0, tr.MatrixColumnMagnitude(0));
    double mag = tr.ColumnXMagnitude();
    Check::ExactDouble(5.0, mag);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    01/17
//---------------------------------------------------------------------------------------
TEST(Transform, GetRotationPart)
    {
    Transform tr = Transform::FromOriginAndVectors(DPoint2d::From(2, 3), DVec2d::From(1, 2), DVec2d::From(3, 2));
    Transform tr2 = Transform::FromRowValues(1, 3, 0, 2,
                                             2, 2, 0, 3,
                                             0, 0, 1, 0);
    Check::Near(tr, tr2);
    DPoint2d pt2d;
    tr.GetTranslation(pt2d);
    RotMatrix rt1 = tr.Matrix();
    RotMatrix rt2;
    tr.GetMatrix(rt2);
    Check::Near(rt1, rt2);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, RigidTransform)
    {
    Transform tr = Transform::From(RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(60).Radians()));
    Check::True(tr.IsRigid());
    Transform tr2 = Transform::FromRowValues(1, 0, 0, 1,
                                   0, 0.5, -0.866, 2,
                                   0, 0.866, 0.5, 1);
    Check::False(tr2.IsRigid());
    DPoint3d translate;
    
    Transform tr3 = Transform::FromRowValues(1, 0, 0, 3,
                                             0, 1, 0, 3,
                                             0, 0, 1, 3);
    
    Check::True(tr3.IsTranslate(translate));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, IsUniformScale) 
    {
    Transform tr = Transform::From(RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(60).Radians()));
    DPoint3d tre;
    double scale;
    Check::False(tr.IsUniformScale(tre, scale));
    Check::Near(scale, 1);
    Transform tr3 = Transform::FromRowValues(3, 0, 0, 0,
                                             0, 2, 0, 0,
                                             0, 0, 1, 0);
    Check::False(tr3.IsUniformScale(tre, scale));
    Check::Near(scale, 1);
    tr3 = Transform::FromRowValues(3, 0, 0, 0,
                                   0, 3, 0, 0,
                                   0, 0, 3, 0);
    Check::True(tr3.IsUniformScale(tre, scale));
    Check::Near(scale, 3);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, RotationAroundLine)
    {
    Transform tr = Transform::From(RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(60).Radians()));
    DPoint3d point;
    DVec3d line;
    double angle;
    Check::True(tr.IsRotateAroundLine(point, line, angle));
    Check::Near(angle, Angle::FromDegrees(60).Radians());
    tr = Transform::FromRowValues(3, 0, 0, 0,
                                  0, 3, 0, 0,
                                  0, 0, 3, 0);
    Check::False(tr.IsRotateAroundLine(point, line, angle));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, MultiplyTranspose)
    {
    Transform tr = Transform::From(RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(180).Radians()));
    DPoint3d point = DPoint3d::From(4, 7, 3);
    tr.MultiplyTranspose(point);
    Check::Near(point, DPoint3d::From(4, -7, -3));

    tr = Transform::FromRowValues(1, 0, 0, 3,
                                  0, Angle::FromDegrees(180).Cos(), -Angle::FromDegrees(180).Sin(), 2,
                                  0, Angle::FromDegrees(180).Sin(), Angle::FromDegrees(180).Cos(), 4);
    point = DPoint3d::FromZero();
    tr.MultiplyTranspose(point, 4, 7, 3);
    Check::Near(point, DPoint3d::From(7, -5, 1));

    //multiply with array of points
    DPoint3d inPoints[] = { DPoint3d::From(4, 5, 3),
                            DPoint3d::From(0.04, 1.05, 0.309),
                            DPoint3d::From(4, -5, 3.3),
                            DPoint3d::From(-9.4, 5.1, 3) };
    DPoint3d outPoints[4], outPointsCopy[4];
    outPointsCopy[0] = inPoints[0];
    outPointsCopy[1] = inPoints[1];
    outPointsCopy[2] = inPoints[2];
    outPointsCopy[3] = inPoints[3];
    tr.MultiplyTranspose(outPoints, inPoints, 4);
    tr.MultiplyTranspose(inPoints, 4);
    tr.MultiplyTranspose(outPointsCopy, 4);
    Check::Near(inPoints[0], outPoints[0]);
    Check::Near(outPointsCopy[0], outPoints[0]);
    Check::Near(inPoints[1], outPoints[1]);
    Check::Near(outPointsCopy[1], outPoints[1]);
    Check::Near(inPoints[2], outPoints[2]);
    Check::Near(outPointsCopy[2], outPoints[2]);
    Check::Near(inPoints[3], outPoints[3]);
    Check::Near(outPointsCopy[3], outPoints[3]);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, Transform2Dto3D) 
    {
    Transform tr = Transform::FromRowValues(1, 0, 0, 3,
                                  0, Angle::FromDegrees(180).Cos(), -Angle::FromDegrees(180).Sin(), 2,
                                  0, Angle::FromDegrees(180).Sin(), Angle::FromDegrees(180).Cos(), 4);
    
    bvector<DPoint3d> outPoints;
    bvector<DPoint2d> inPoints = { DPoint2d::From(4, 5),
                                   DPoint2d::From(0.04, 1.05),
                                   DPoint2d::From(4, -5),
                                   DPoint2d::From(-9.4, 5.1) };
    tr.Multiply(outPoints, inPoints);
    
    Check::Near(outPoints[0].x, 7); Check::Near(outPoints[0].y, -3); Check::Near(outPoints[0].z, 4);
    Check::Near(outPoints[1].x, 3.04); Check::Near(outPoints[1].y, 0.95); Check::Near(outPoints[1].z, 4);
    Check::Near(outPoints[2].x, 7); Check::Near(outPoints[2].y, 7); Check::Near(outPoints[2].z, 4);
    Check::Near(outPoints[3].x, -6.4); Check::Near(outPoints[3].y, -3.1); Check::Near(outPoints[3].z, 4);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------

void uniformScaleAndRotateAboutLine(Transform tr, bool flag)
    {
    DPoint3d      fixedPoint, fixedPointExpected;
    DVec3d       directionVector;
    double          radians, scal;

    if (flag == true)
        {
        double radiansExpected;
        Check::True(tr.IsUniformScaleAndRotateAroundLine(fixedPoint, directionVector, radians, scal));
        Check::True(tr.IsRotateAroundLine(fixedPointExpected, directionVector, radiansExpected));
        Check::Near(radians, radiansExpected);
        Check::Near(fixedPoint, fixedPointExpected);
        Check::Near(scal, 1);
        }
    else
        {
        double scale;
        Check::True(tr.IsUniformScaleAndRotateAroundLine(fixedPoint, directionVector, radians, scal));
        Check::True(tr.IsUniformScale(fixedPointExpected, scale));
        Check::Near(radians, 0);
        Check::Near(scal, scale);
        Check::Near(fixedPoint, fixedPointExpected);
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, UnfiformScaleAndRotate)
    {
    uniformScaleAndRotateAboutLine(Transform::FromLineAndRotationAngle(DPoint3d::From(7, 3, 7),
                                                                       DPoint3d::From(11, 11, 11),
                                                                       Angle::FromDegrees(45).Radians()),
                                                                       true);
    uniformScaleAndRotateAboutLine(Transform::FromLineAndRotationAngle(DPoint3d::From(7.1, 3.03, 7),
                                                                       DPoint3d::From(0.1, 1.1, 0.1),
                                                                       Angle::FromDegrees(180).Radians()),
                                                                       true);
    uniformScaleAndRotateAboutLine(Transform::FromLineAndRotationAngle(DPoint3d::From(1, 3.2, 1.7),
                                                                       DPoint3d::From(11, 11, 11),
                                                                       Angle::FromDegrees(330).Radians()),
                                                                       true);

    uniformScaleAndRotateAboutLine(Transform::FromScaleFactors(2.3, 2.3, 2.3),
                                                                       false);
    uniformScaleAndRotateAboutLine(Transform::FromScaleFactors(12, 12, 12),
                                                                       false);
    uniformScaleAndRotateAboutLine(Transform::FromScaleFactors(0.3, 0.3, 0.3),
                                                                       false);
   
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, FixedPoint) 
    {
    Transform tr = Transform::FromRowValues(1, 0, 0, 2,
                                            0, 1, 0, 3,
                                            0, 0, 1, 2);  // pure translation
    DPoint3d fixedPoint;
    Check::False(tr.GetAnyFixedPoint(fixedPoint));

    tr = Transform::FromRowValues(1, 4, 0, 2,
                                  0, 1, 3, 3,
                                  0, 0, 1, 2);    //impure translation
    
    Check::True(tr.GetAnyFixedPoint(fixedPoint));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, TransformSegment)
    {
    Transform trans = Transform::From(DPoint3d::From(2, 2, 2));
    DSegment3d seg3d = DSegment3d::FromOriginAndDirection(DPoint3d::From(2, 2, 2), DPoint3d::From(10, 10, 10));
    trans.Multiply(seg3d);
    DPoint3d segmentStart = DPoint3d::From(2, 2, 2), segmentEnd = DPoint3d::From(12, 12, 12);
    trans.Multiply(segmentStart);
    trans.Multiply(segmentEnd);
    Check::Near(segmentStart, seg3d.point[0]);
    Check::Near(segmentEnd, seg3d.point[1]);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, TransformRay)
    {
    Transform trans = Transform::From(DPoint3d::From(2, 2, 2));
    DRay3d ray3d = DRay3d::FromOriginAndTarget(DPoint3d::From(2, 2, 2), DPoint3d::From(12, 12, 12));
    trans.Multiply(ray3d);
    DPoint3d rayStart = DPoint3d::From(2, 2, 2);
    trans.Multiply(rayStart);
    Check::Near(rayStart, ray3d.origin);
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, UnaffectedPoint)
    {
    //identity
    Transform tr = Transform::FromIdentity();
    
    DPoint3d pnt;
    tr.Multiply(pnt);

    DPoint3d fixedPoint;
    Check::False(tr.GetAnyFixedPoint(fixedPoint));

    tr.SetTranslation(DPoint3d::From(4, 4.4, 3));
    Check::False(tr.GetAnyFixedPoint(fixedPoint));
    tr = Transform::From(RotMatrix::FromAxisAndRotationAngle(2, Angle::FromDegrees(60).Radians()));
    Check::True(tr.GetAnyFixedPoint(fixedPoint));
   
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, PlanarTransform)
    {
    //if transformatin has 2d effect
    bvector<Transform> tr = {
                            //planar
                            Transform::FromRowValues(1,0,0, 0 ,0,1,0,2,0,0,1,1),
                            Transform::FromRowValues(1,0,0,3,0,1,0, 0 ,0,0,1,6),
                            Transform::FromRowValues(1,0,0,3,0,1,0,2.2,0,0,1, 0 ),
                            Transform::From(RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(60).Radians())),
                            Transform::From(RotMatrix::FromAxisAndRotationAngle(1, Angle::FromDegrees(270).Radians())),
                            Transform::From(RotMatrix::FromAxisAndRotationAngle(2, Angle::FromDegrees(90).Radians())),
                            Transform::FromScaleFactors(10, 2.3, 1),
                            Transform::FromScaleFactors(3, 1, 3),
                            Transform::FromScaleFactors(1, 2.3, 5.3),
                            //non planar
                            Transform::FromRowValues(1,0,0,3,0,1,0,2,0,0,1,3),
                            Transform::FromRowValues(1,0,0,3,0,1,0,2,0,0,1,0),
                            Transform::FromRowValues(1,0,0,0,0,1,0,2,0,0,1,3),
                            Transform::From(RotMatrix::FromAxisAndRotationAngle(2, Angle::FromDegrees(60).Radians())),
                            Transform::From(RotMatrix::FromAxisAndRotationAngle(0, Angle::FromDegrees(60).Radians())),
                            Transform::From(RotMatrix::FromAxisAndRotationAngle(1, Angle::FromDegrees(60).Radians())),
                            Transform::FromScaleFactors(10, 2.3, 1),
                            Transform::FromScaleFactors(1, 2.3, 5.3),
                            Transform::FromScaleFactors(1.11, 1, 5.3)
                            };
    bvector <DVec3d> normals = {
                               DVec3d::From(1,0,0),
                               DVec3d::From(0,1,0),
                               DVec3d::From(0,0,1),
                               
                               DVec3d::From(1,0,0),
                               DVec3d::From(0,1,0),
                               DVec3d::From(0,0,1),
                               
                               DVec3d::From(0,0,1),
                               DVec3d::From(0,1,0),
                               DVec3d::From(1,0,0),

                               DVec3d::From(1,0,0),
                               DVec3d::From(0,1,0),
                               DVec3d::From(0,0,1),

                               DVec3d::From(1,0,0),
                               DVec3d::From(0,1,0),
                               DVec3d::From(0,0,1),

                               DVec3d::From(1,0,0),
                               DVec3d::From(0,1,0),
                               DVec3d::From(0,0,1)
                               };
    for (size_t i = 0; i < tr.size(); i++)
        {
        if (i < 9)
            {
            Check::True(tr[i].IsPlanar(normals[i]));
            }
        else
            {
            Check::False(tr[i].IsPlanar(normals[i]));
            }
        }

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, TransformOnPointArray) 
    {
    bvector<DPoint3d> outPoints;
    bvector<DPoint3d> inPoints = { DPoint3d::From(4,4,4),
                                  DPoint3d::From(7,8,2),
                                  DPoint3d::From(-3,2,-3),
                                  DPoint3d::From(0.4,0.2,0.2) };
    Transform tr = Transform::FromRowValues(1, 0, 0, 4,
                                            0, Angle::FromDegrees(60).Cos(), -Angle::FromDegrees(60).Sin(), 6,
                                            0, Angle::FromDegrees(60).Sin(), Angle::FromDegrees(60).Cos(), 5);
    tr.SolveArray(outPoints, inPoints);

    DPoint3d testOut;
    for (size_t i = 0; i < inPoints.size(); i++) 
        {
        tr.Solve(testOut, inPoints[i]);
        Check::Near(testOut, outPoints[i]);
        }
    
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(Transform, WeightedTransform)
    {
    Transform tr = Transform::FromScaleFactors(0.7, 3, 5);
    bvector<DPoint3d> outPoints;
    bvector<DPoint3d> inPoints = { DPoint3d::From(4,5,2),
                                   DPoint3d::From(8,7,8),
                                   DPoint3d::From(3,7,9),
                                   DPoint3d::From(0.87, 8.2, 90.1) };
    bvector<double> weights = { 6, 9, 1.8, 4 };
    tr.MultiplyWeighted(outPoints, inPoints, &weights);
    DPoint3d testOut;
    for (size_t i = 0; i < inPoints.size(); i++) 
        {
        testOut = inPoints[i];
        tr.MultiplyWeighted(testOut, weights[i]);

        Check::Near(testOut, outPoints[i]);
        }
    }

#define NPC_000 0
#define NPC_100 1
#define NPC_010 2
#define NPC_110 3

#define NPC_001 4
#define NPC_101 5
#define NPC_011 6
#define NPC_111 7
enum class Coords
{
WORLD = 0,
NPC = 1,
VIEW = 2
};
struct ViewPortEmulator
{

// These vars and methods emulate viewing services in the viewport ...
DMap4d m_worldToView;
DMap4d m_viewToNPC;

DPoint3d WorldToView (DPoint3dCR worldPoint)
    {
    DPoint3d viewPoint;
    m_worldToView.M0.MultiplyAndRenormalize (&viewPoint, &worldPoint, 1);
    return viewPoint;
    }
DPoint3d ViewToWorld (DPoint3dCR viewPoint)
    {
    DPoint3d worldPoint;
    m_worldToView.M1.MultiplyAndRenormalize (&worldPoint, &viewPoint, 1);
    return worldPoint;
    }


ViewPortEmulator (DMap4dCR worldToView, DMap4dCR viewToNPC) : m_worldToView (worldToView), m_viewToNPC (viewToNPC)
    {
    }

DRay3d ViewPointToWorldRay (DPoint3d viewPoint)
    {
    // create two points A and B on the back and front planes.
    DPoint4d npcA = m_viewToNPC.M0.Multiply (viewPoint, 1.0);
    npcA.z = 0.0;        // mvoe it to back plane
    DPoint4d npcB = npcA;
    npcB.z = 1.0;       // move it to front plane
    DPoint3d worldA, worldB;
    DPoint4d viewA, viewB;
    m_viewToNPC.M1.Multiply (&viewA, &npcA, 1);
    m_viewToNPC.M1.Multiply (&viewB, &npcB, 1);
    m_worldToView.M1.MultiplyAndNormalize (&worldA, &viewA, 1);
    m_worldToView.M1.MultiplyAndNormalize (&worldB, &viewB, 1);
    return DRay3d::FromOriginAndVector (worldB, worldA - worldB);
    }
DPoint3d ToView3d (Coords coords, DPoint3dCR point)
    {
    DPoint3d viewPoint;
    if (coords == Coords::NPC)
        m_viewToNPC.M1.MultiplyAndRenormalize (viewPoint, point);
    else if (coords == Coords::WORLD)
        m_worldToView.M0.MultiplyAndRenormalize (viewPoint, point);
    else
        viewPoint = point;
    return viewPoint;
    }
DSegment3d ToView3d (Coords coords, DSegment3dCR segment)
    {
    DPoint3d point0, point1;
    m_worldToView.M0.MultiplyAndRenormalize (point0, segment.point[0]);
    m_worldToView.M0.MultiplyAndRenormalize (point1, segment.point[1]);
    return DSegment3d::From (point0, point1);
    }

// "DisplayWorld" ... means 
//   map to view coordinates
//   Check::SaveTransformed
void DisplaySegment (Coords coords, DPoint3dCR pointA, DPoint3dCR pointB)
    {
    Check::SaveTransformed (DSegment3d::From (ToView3d (coords, pointA), ToView3d (coords, pointB)));
    }

// "DisplayWorld" ... means 
//   map to view coordinates
//   Check::SaveTransformed
void DisplayPlane (Coords coords, DPoint3dDVec3dDVec3dCR plane, int numX, int numY)
    {
    double xMax = numX + 1.0;     // extend a little on first line
    for (int iY = 0; iY <= numY; iY++)
        {
        DisplaySegment (coords,
            plane.Evaluate (0.0, (double)iY),
            plane.Evaluate (xMax, (double)iY));
        xMax = numX;
        }
    double yMax = numY;
    for (int iX = 0; iX <= numX; iX++)
        {
        DisplaySegment (coords,
            plane.Evaluate ((double)iX, 0.0),
            plane.Evaluate ((double)iX, yMax));
        }
    }


void DisplaySegment (Coords coords, DSegment3dCR segment)
    {
    Check::SaveTransformed (DSegment3d::From (ToView3d (coords, segment.point[0]), ToView3d (coords, segment.point[1])));
    }
void DisplayRange (Coords coords, DRange3dCR range, bool displayWithXYZPatterns)
    {
    bvector<DSegment3d> segments;
    StrokeRange (segments, range, displayWithXYZPatterns);
    for (auto &s : segments)
        DisplaySegment (coords, s);
    }
void DisplayCuboid (Coords coords, DPoint3d corners[8], bool displayWithXYZPatterns)
    {
    bvector<DSegment3d> segments;
    StrokeFrustum (segments, corners, displayWithXYZPatterns);
    for (auto &s : segments)
        DisplaySegment (coords, s);
    }
};
struct DragState
{
DPoint3d m_initialViewPoint;
};

struct ViewClipCuboidTool
{
ViewPortEmulator &m_viewport;

DragState m_dragState;


DPoint3d m_cuboid[8]; // 8 points of view cube.  ASSUMED TO BE PARALLEL SIDES
ViewClipCuboidTool (ViewPortEmulator &viewport) : m_viewport (viewport) {
    m_dragState.m_initialViewPoint = DPoint3d::From (0,0,0);
    }

void SetRangeCuboid (DRange3dCR range)
    {
    range.Get8Corners (m_cuboid);
    }
void TransformCuboid (TransformCR transform)
    {
    transform.Multiply (m_cuboid, m_cuboid, 8);
    }
DPoint3d CuboidFractionsToWorld (DPoint3dCR uvw)
    {
    return m_cuboid[NPC_000]
        + uvw.x * (m_cuboid[NPC_100] - m_cuboid[NPC_000])
        + uvw.y * (m_cuboid[NPC_010] - m_cuboid[NPC_000])
        + uvw.z * (m_cuboid[NPC_001] - m_cuboid[NPC_000]);
    }


// Given 3 points as fractions in the cuboid.
// Return a plane with origin at first point, U and V vectors towards targets.
DPoint3dDVec3dDVec3d CuboidFractionsToPlaneByVectors
(
DPoint3d uvwOrigin,
DPoint3d uvwXTarget,
DPoint3d uvwYTarget
)
    {
    return DPoint3dDVec3dDVec3d::FromOriginAndTargets (
        CuboidFractionsToWorld (uvwOrigin),
        CuboidFractionsToWorld (uvwXTarget),
        CuboidFractionsToWorld (uvwYTarget)
        );
    }
// Return the (single point) intersection of a ray with a plane.
// Return false if ray and plane are parallel.
ValidatedDPoint3d IntersectRayWithPlane
(
DRay3dCR ray,
DPoint3dDVec3dDVec3dCR plane
)
    {
    // Point on ray is at parameter s is:    X(s) = ray.origin + s * ray.direction
    // Point on plane at parameters u,v is:  Y(u,v) = plane.origin + u * plane.vectorU + v * plane.vectorV
    // At intersection, these two are the same. So this is 3 equations in 3 unknowns (u,v,-s):
    //    [plane.vectorU plane.vectorV ray.direction] * column([u v -s]) = ray.origin - plane.origin
    //
    RotMatrix matrix = RotMatrix::FromColumnVectors (plane.vectorU, plane.vectorV, ray.direction);
    DVec3d rhs = ray.origin - plane.origin;
    DVec3d uvw;
    if (matrix.Solve (uvw, rhs))
        return ValidatedDPoint3d (ray.FractionParameterToPoint (-uvw.z), true);
    return ValidatedDPoint3d (ray.origin, false);
    }
// Given two rays and a plane
// (Plane defined by origin and vectors, as a DPoint3dDVec3dDVec3d)
// Find the intersections of the two rays with the plane.
// Return the line segment joining the two points.
// (The vector along this segemnt is the vector to transform geometry)
// If either ray is parallel to the plane, return an invalid segment betweeen the ray origins.
ValidatedDSegment3d MoveOnPlane
(
DRay3dCR rayA,                      // first ray (e.g. a ray from a mouse point into a view)
DRay3dCR rayB,                      // second ray (e.g. a ray from a mouse point into a view)
DPoint3dDVec3dDVec3dCR plane       // plane described by origin and 2 vectors
)
    {
    auto pointA = IntersectRayWithPlane (rayA, plane);
    auto pointB = IntersectRayWithPlane (rayB, plane);
    if (pointA.IsValid () && pointB.IsValid ())
        {
        return ValidatedDSegment3d (DSegment3d::From (pointA.Value (), pointB.Value ()));
        }
    return ValidatedDSegment3d (DSegment3d::From (rayA.origin, rayB.origin), false);
    }

#ifdef CompileMoveOnCuboidMidZPlane

// This is the arg list (targetPointView) of the original MoveOnPlane.
// The new name emphasizes that it chooses a particular plane (middle z of cuboid).
void MoveOnCuboidMidZPlane (DPoint3dCR targetPointView)
    {
    DPoint3d startingPointView = m_dragState.m_initialViewPoint;
    // Construct a world space plane through the mid-Z of the cuboid ....
    auto plane = CuboidFractionsToPlaneByVectors (
        DPoint3d::From (0.5, 0.5, 0.5),
        DPoint3d::From (1.0, 0.5, 0.5),
        DPoint3d::From (0.5, 0.0, 0.5));

    // construct world space rays from prior and current view points into the view . . 
    DRay3d startRay = ViewPointToWorldRay (startingPointView);
    DRay3d endRay   = ViewPointToWorldRay (targetPointView);
    // Find intersections of each ray with the plane . ..
    auto shiftSegment = MoveOnPlane (rayA, rayB, plane);
    if (shiftSegment.IsValid ())
        {
        // shift be the vector along this segment . ..
        shiftTransform = Transform::From (shiftSegment.Value ().VectorStartToEnd ());
        trans.Multiply(m_cuboid, m_dragState.m_startingCuboid, _countof(m_cuboid));

        FinishVolumeManipulation();
        }
    }
//
// Using the GetViewport () coordinate transformations,
// 1) Convert given view point to npc
// 2) move the npc z coordinates to 0 and 1 to get points on front and back
// 3) convert the npc front and back points to world
// 4) return a ray from front world point towards back world point.
//
DRay3d ViewPointToWorldRay (DPoint3d viewPoint)
    {
    // create two points A and B on the back and front planes.
    DPoint4d npcA;
    GetViewport()->ViewToNpc (&npcA, &viewPoint, 1);
    npcA.z = 0.0;        // move pointA to the back plane
    DPoint4d npcB = npcA;
    npcB.z = 1.0;       // move pointB to front plane
    DPoint3d worldA, worldB;
    GetViewport()->NpcToWorld (&worldA, &npcA, 1);
    GetViewport()->NpcToWorld (&worldB, &npcB, 1);
    return DRay3d::FromOriginAndVector (worldB, worldA - worldB);
    }

#endif
};
struct TransformAndString
{
Transform m_transform;
char const *m_string;
TransformAndString (Transform transform, char const *string) : m_transform (transform), m_string (string) {}
};

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 08/18
//---------------------------------------------------------------------------------------
TEST(View,MouseMove)
    {
    DMap4d worldToViewMap;
    double radiansW = Angle::DegreesToRadians (30);
    auto worldToView = Transform::From(RotMatrix::FromAxisAndRotationAngle (2, radiansW), DPoint3d::From (10,20,0));
    worldToViewMap.InitFromTransform (worldToView, false);

    DMap4d viewToNpcMap;
    viewToNpcMap.InitFromTransform (
        Transform::FromRowValues (
            100, 0, 0, 0,
            0, 100, 0, 0,
            0, 0, 100, -50), true);
    auto viewBox = DRange3d::From (0,0,-50, 100, 100, 50);

    ViewPortEmulator viewPort (worldToViewMap, viewToNpcMap);
    DVec3d vectorQ = DVec3d::From (0.1, 0.38, 0.8);
    vectorQ.Normalize ();
    double radiansQ = Angle::DegreesToRadians (80);
    RotMatrix matrixQ = RotMatrix::FromVectorAndRotationAngle (vectorQ, radiansQ);
    Transform transformQ = Transform::FromMatrixAndFixedPoint (matrixQ, DPoint3d::From (0,0,0));
    DVec3d vectorQ1;
    double radiansQ1 = matrixQ.GetRotationAngleAndVector (vectorQ1);
    Check::Near (radiansQ, radiansQ1, "verify degreeQ rotation");
    Check::Parallel (vectorQ, vectorQ1, "verify axis of rotation");
    for (auto placement : bvector<TransformAndString> {
            TransformAndString(Transform::FromIdentity (), "Identity"),
            TransformAndString(Transform::FromMatrixAndFixedPoint (
                RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0,0,1), Angle::DegreesToRadians (30)),
                    DPoint3d::From (0,0,0)), "Z rotation"),
            TransformAndString (transformQ, "General Rotation")
            })
        {
        SaveAndRestoreCheckTransform shifter (200,0,0);
        auto transform = placement.m_transform;
        Check::KeyinText (Check::TransformPoint (DPoint3d::From (0,0,0)), placement.m_string);
        Check::True (RotMatrix::From (transform).IsOrthogonal ());
        ViewClipCuboidTool tool (viewPort);
        tool.SetRangeCuboid (DRange3d::From (0,0,0, 12,16,20));
        tool.TransformCuboid (transform);
        DPoint3d viewPointA = DPoint3d::From (20,40,0);   // pixels at mouse start
        DPoint3d viewPointB = DPoint3d::From (80,40,0);   // pixels at mouse end

        viewPort.DisplaySegment (Coords::VIEW, viewPointA, viewPointB);
        viewPort.DisplayCuboid (Coords::WORLD, tool.m_cuboid, true);
        viewPort.DisplayRange (Coords::VIEW, viewBox, false);


        DRay3d worldRayA = viewPort.ViewPointToWorldRay (viewPointA);
        DRay3d worldRayB = viewPort.ViewPointToWorldRay (viewPointB);
        auto worldPlane = tool.CuboidFractionsToPlaneByVectors (
                    DPoint3d::From (0.5,0.5, 0.5),  // CuboidFractions center
                DPoint3d::From (1.0,0.5, 0.5),  // CuboidFractions right center
                DPoint3d::From (0.5,1.0, 0.5) 
                );
        viewPort.DisplayPlane (Coords::WORLD, worldPlane, 3, 2);
        auto shiftSegment = tool.MoveOnPlane (worldRayA, worldRayB, worldPlane);
        if (Check::True (shiftSegment.IsValid ()))
            {
            auto segment = shiftSegment.Value ();
            auto shiftTransform = Transform::From (DVec3d::FromStartEnd (segment.point[0], segment.point[1]));
            viewPort.DisplaySegment (Coords::WORLD, segment);
            tool.TransformCuboid(shiftTransform);
            viewPort.DisplayCuboid (Coords::WORLD, tool.m_cuboid, true);
            }
        }
    Check::ClearGeometry ("View.MouseMove");
    Check::KeyinImport ("View.MouseMove");
    Check::ClearKeyins ("View.MouseMove");
    }