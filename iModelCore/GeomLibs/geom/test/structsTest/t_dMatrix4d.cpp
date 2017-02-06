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