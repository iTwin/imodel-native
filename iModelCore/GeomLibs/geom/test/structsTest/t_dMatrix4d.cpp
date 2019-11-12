/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"


DMatrix4d SequentialIntegerMatrix (double a00)
    {
    return DMatrix4d::FromRowValues
        (
        a00, a00 + 1, a00 + 2, a00 + 3,
        a00 + 4, a00 + 5, a00 + 6, a00 + 7,
        a00 + 8, a00 + 9, a00 + 10, a00 + 11,
        a00 + 12, a00 + 13, a00 + 14, a00 + 15
        );
    }

DMatrix4d CopyAffine (DMatrix4d source)
    {
    DPoint4d row0, row1, row2, row3;
    source.GetRows (row0, row1, row2, row3);
    return DMatrix4d::FromRows (row0, row1, row2, DPoint4d::From (0,0,0,1));
    }

void MakeLatticePoints (bvector<DPoint3d> &xyz, double ax = 1.0, double ay = 1.0, double az = 1.0)
    {
    bvector<double> sign{-1,0,1};
    for (auto sx : sign)
        for (auto sy : sign)
            for (auto sz : sign)
                xyz.push_back (DPoint3d::From (sx * ax, sy * ay, sz * az));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    12/16
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, AccessRowColumnTranspose)
    {
    auto A = SequentialIntegerMatrix (1.0);
    // Transpose with and without aliasing.
    DMatrix4d AT;
    AT.TransposeOf (A);
    DPoint4d row0, row1, row2, row3;
    DPoint4d column0, column1, column2, column3;
    A.GetRows (row0, row1, row2, row3);
    AT.GetColumns (column0, column1, column2, column3);
    Check::Near (row0, column0, "Row vs Column of transpose");
    Check::Near (row1, column1, "Row vs Column of transpose");
    Check::Near (row2, column2, "Row vs Column of transpose");
    Check::Near (row3, column3, "Row vs Column of transpose");
    auto A1 = DMatrix4d::FromRows (row0, row1, row2, row3);
    auto AT1 = DMatrix4d::FromColumns (column0, column1, column2, column3);
    Check::ExactDouble (0.0, (A1-A).MaxAbs (), "Get/Set by row");
    Check::ExactDouble (0.0, (AT1-AT).MaxAbs (), "Get/Set by column");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMatrix4d, AccessXYZColumns)
    {
    auto A = SequentialIntegerMatrix (1.0);
    auto B = CopyAffine (A);
    auto B1 = CopyAffine (A);
    DVec3d b0, b1, b2;
    DPoint3d b3;
    B.GetColumnsXYZ (b0, b1, b2, b3);
    B1 = DMatrix4d::FromColumnVectors (b0, b1, b2, b3);
    Check::ExactDouble (0.0, (B1-B).MaxAbs (), "XYZ access");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMatrix4d, FromTranslation)
    {
    DMatrix4d A1 = DMatrix4d::FromTranslation (2,3,4);
    DMatrix4d A0 = DMatrix4d::FromRowValues
        (
        1,0,0,2,
        0,1,0,3,
        0,0,1,4,
        0,0,0,1
        );
    Check::ExactDouble (0.0, (A1-A0).MaxAbs (), "FromTranslation");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMatrix4d, ConstructionA)
    {
    DMatrix4d A1 = DMatrix4d::FromTranslation (2,3,4);
    DMatrix4d A0 = DMatrix4d::FromRowValues
        (
        1,0,0,2,
        0,1,0,3,
        0,0,1,4,
        0,0,0,1
        );
    Check::ExactDouble (0.0, (A1-A0).MaxAbs (), "FromTranslation");
    DMatrix4d B = DMatrix4d::FromPerspective (2,3,4);
    auto BT = DMatrix4d::FromTranspose (B);
    Check::ExactDouble (0.0, (A1-BT).MaxAbs (), "FromPerspective");

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMatrix4d, ConstructionB)
    {
    Transform transformA = YawPitchRollAngles::FromDegrees (10.0, 20.0, -2.5).ToTransform (DPoint3d::From (1,3,2));
    Transform transformC = YawPitchRollAngles::FromDegrees (-45, 2.0, 12.5).ToTransform (DPoint3d::From (1,5,8.8));
    RotMatrix matrixB = YawPitchRollAngles::FromDegrees (8.0, 2.4, 82.0).ToRotMatrix ();
    auto mA = DMatrix4d::From (transformA);
    auto mB = DMatrix4d::From (matrixB);
    mB.coff[3][3] = 0.0;
    auto mC = DMatrix4d::From (transformC);
    auto mABC0 = (mA * mB) * mC;
    auto mABC1 = mA * (mB * mC);
    auto mABC = DMatrix4d::From101WeightedProduct (transformA, matrixB, transformC);
    Check::Near (mABC0, mABC1, "Confirm Associative matrix4d multiplication");
    Check::Near (mABC0, mABC, "Confirm fast weighted A1*B0*C1 consruction");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMatrix4d, MutliplyAndRenormalize)
    {
    bvector<DPoint3d> allXYZ, allXYZ1, allXYZ2, allXYZ3;
    auto A = SequentialIntegerMatrix (1.0);
    auto B = CopyAffine (A);
    allXYZ1 = allXYZ;
    allXYZ2 = allXYZ;
    A.MultiplyAndRenormalize (allXYZ1);
    B.MultiplyAndRenormalize (allXYZ2);
    for (size_t i = 0; i < allXYZ.size (); i++)
        {
        auto xyz = allXYZ[i];
        auto xyzw = DPoint4d::From (xyz, 1.0);
        auto Ax = A * xyzw;
        DPoint3d Ax1;
        Ax.GetProjectedXYZ (Ax1);
        Check::Near (allXYZ1[i], Ax1, "full perspective renormalized A*x variant paths");
        auto Bx = B * xyzw;

        Check::Near (DPoint4d::From (allXYZ2[i], 1.0), Bx, "Affine A*x");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(DMatrix4d, Swaps)
    {

    auto A = SequentialIntegerMatrix (1.0);
    for (int i = -4; i < 9; i++)
        {
        for (int j = -4; j< 9; j++)
            {
            auto permuter = DMatrix4d::FromSwap (i,j);
            auto PA = permuter * A;
            auto AP = A * permuter;
            auto rowSwap = A;
            auto colSwap = A;
            rowSwap.SwapRows (i,j);
            colSwap.SwapColumns (i,j);
            Check::ExactDouble (0.0, (PA-rowSwap).MaxAbs (), "Row swaps");
            Check::ExactDouble (0.0, (AP-colSwap).MaxAbs (), "Column swaps");
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, AffineTransformation)
    {
    DMatrix4d mat0 = DMatrix4d::FromColumns(DPoint4d::From(0.4, 1, 0.1, 0), 
                                            DPoint4d::From(2, 1, 0.1, 0), 
                                            DPoint4d::From(1.5, 8, 2, 0), 
                                            DPoint4d::From(9, 9, 0.1, 1));
    Check::True(mat0.IsAffine());
    RotMatrix rotMat = RotMatrix::FromIdentity();
    DMatrix4d mot1 = DMatrix4d::From(rotMat);
    Check::True(mot1.IsAffine());
    mat0.CopyUpperTriangleToLower();
    Check::False(mat0.IsAffine());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, TransposedPoints)
    {
    DMatrix4d mat0 = DMatrix4d::FromColumns(DPoint4d::From(0.4, 1, 0.1, 0),
                                            DPoint4d::From(2, 1, 0.1, 0),
                                            DPoint4d::From(1.5, 8, 2, 0),
                                            DPoint4d::From(9, 9, 0.1, 1));
    DPoint4d pointsArr[] = { DPoint4d::From(1, 4, 1, 1),
                             DPoint4d::From(2, 3, 1, 1),
                             DPoint4d::From(2, 4, 5, 1),
                             DPoint4d::From(0, 1, 0, 1) };
    DPoint4d ptsTransposed[4];

    mat0.MultiplyTranspose(ptsTransposed, pointsArr, 4);

    mat0.TransposeOf(mat0);
    DMatrix4d matPoints = DMatrix4d::FromColumns(pointsArr[0], pointsArr[1], pointsArr[2], pointsArr[3]);
    mat0.InitProduct(mat0, matPoints);
    DPoint4d pointT;
    mat0.GetColumn(pointT, 0);
    Check::Near(pointT, ptsTransposed[0]);
    mat0.GetColumn(pointT, 1);
    Check::Near(pointT, ptsTransposed[1]);
    mat0.GetColumn(pointT, 2);
    Check::Near(pointT, ptsTransposed[2]);
    mat0.GetColumn(pointT, 3);
    Check::Near(pointT, ptsTransposed[3]);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, MultiplyAffine)
    {
    DMatrix4d mat0 = DMatrix4d::FromColumns(DPoint4d::From(0.4, 1, 0.1, 0),
                                            DPoint4d::From(2, 1, 0.1, 3),
                                            DPoint4d::From(1.5, 8, 2, 0),
                                            DPoint4d::From(9, 9, 0.1, 1));
    DPoint4d pointsArr[] = { DPoint4d::From(1, 4, 1, 3),
                             DPoint4d::From(2, 3, 1, 2),
                             DPoint4d::From(2, 4, 5, 1),
                             DPoint4d::From(0, 1, 0, 4) };
    DPoint4d ptsHomogeneous[4], ptsNonHom[4];
    mat0 = CopyAffine(mat0);
    mat0.MultiplyAffine(ptsHomogeneous, pointsArr, 4);
    mat0.Multiply(ptsNonHom, pointsArr, 4);
    
    Check::Near(ptsNonHom[0], ptsHomogeneous[0], "Homogeneous coordinates differ");
    Check::Near(ptsNonHom[1], ptsHomogeneous[1], "Homogeneous coordinates differ");
    Check::Near(ptsNonHom[2], ptsHomogeneous[2], "Homogeneous coordinates differ");
    Check::Near(ptsNonHom[3], ptsHomogeneous[3], "Homogeneous coordinates differ");
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, AdditionOverload)
    {
    DMatrix4d mat = DMatrix4d::FromRowValues(1, 0, 0, 0,
                                             0, 2.1, 0.1, 0.002,
                                             0, 0, 1, 0,
                                             8.00002, 0, 0.1, 12);
    DMatrix4d mat2 = DMatrix4d::FromRowValues(1, 0, 0, -0.11,
                                              -8, 22.1, 0, 9.01,
                                              7.3, 0, 1, 0,
                                              0, 30, 0, 9.1);
    DMatrix4d matCpy = mat;
    mat.Add(mat2);
    matCpy = matCpy + mat2;
    Check::Near(mat, matCpy);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, MultiplicationOverload)
    {
    DMatrix4d mat = DMatrix4d::FromScaleAndTranslation(DPoint3d::From(2, 3, 1), DPoint3d::From(4, 4, 4));
    DPoint4d point = DPoint4d::From(10, 10, 10, 1);
    DPoint4d res = mat * point;
    DPoint4d res2 =  mat.Multiply(DPoint3d::From(10, 10, 10), 1);
    Check::Near(res, res2);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, Subtract)
    {
    DMatrix4d mat = DMatrix4d::FromScaleAndTranslation(DPoint3d::From(2, 3, 1), DPoint3d::From(4, 4, 4));
    DMatrix4d mat2 = DMatrix4d::FromScaleAndTranslation(DPoint3d::From(2, 3, 1), DPoint3d::From(4, 4, 4));
    DMatrix4d matCpy = mat2;
    mat2.Subtract(mat);
    Check::True(mat2.MaxAbs() == 0);
    matCpy.Scale(2);
    matCpy.Subtract(mat);
    Check::Near(mat, matCpy);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Farhad.Kabir                    02/17
//---------------------------------------------------------------------------------------
TEST(DMatrix4d, TranslationChange)
    {
    DMatrix4d mat = DMatrix4d::FromTranslation(2.3, 3.4, 1.2);
    DPoint3d vec = DPoint3d::From(7, 3, 5);
    DPoint4d pnt = mat.Multiply(vec, 1);
    DPoint4d trans;
    trans.Zero();
    mat.GetColumn(trans, 3);

    Check::Near(pnt.x, vec.x + trans.x);
    Check::Near(pnt.y, vec.y + trans.y);
    Check::Near(pnt.z, vec.z + trans.z);
    //applying rotation
    mat.SetColumn(1, 0, Angle::FromDegrees(180).Cos(), Angle::FromDegrees(180).Sin(), 0);
    mat.SetColumn(2, DPoint4d::From(0, -Angle::FromDegrees(180).Sin(), Angle::FromDegrees(180).Cos(), 0));

    DPoint4d pnt2;
    pnt2 = mat.Multiply(DPoint3d::From(pnt.x, pnt.y, pnt.z), 1);

    mat.GetColumn(trans, 3);
    Check::Near(pnt2.x, pnt.x + trans.x);
    Check::Near(pnt2.y, -pnt.y + trans.y);
    Check::Near(pnt2.z, -pnt.z + trans.z);
    }