//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGFModelCompositionTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGFModelCompositionTester.h"

HGFModelCompositionTester::HGFModelCompositionTester()
    {

    Model1.AddAnisotropicScaling(2.0, 1.5, 0, 0);
    Model2.AddAnisotropicScaling(2.0, 0.5, 2.0, 0);
    Model4.AddAnisotropicScaling(5.0, 7.5, 0, 4.0);
    Model3.AddRotation(-PI/4, 0, 0);

    pResultModel1 = HFCPtr<HGF2DTransfoModel> (Model3.ComposeInverseWithDirectOf(Model1));
    pResultModel2 = HFCPtr<HGF2DTransfoModel> (Model3.ComposeInverseWithDirectOf(Model2));
    pResultModel3 = HFCPtr<HGF2DTransfoModel> (Model3.ComposeInverseWithDirectOf(Model4));

    Matrix1 = HFCMatrix<3,3>(pResultModel1->GetMatrix());
    Matrix2 = HFCMatrix<3,3>(pResultModel2->GetMatrix());
    Matrix3 = HFCMatrix<3,3>(pResultModel3->GetMatrix());

    }

//==================================================================================
// Problem that occured around end of november 1999
//
// Composing a Similitude with a Stretch should give the same
// result as composing the Similitude with an Affine that has
// the same scaling applied. -> It's not the case, position [0][1]
// differs...
//==================================================================================
TEST_F(HGFModelCompositionTester, TestGeneral)
    {

    ASSERT_DOUBLE_EQ(1.4142135623730951, Matrix1[0][0]);
    ASSERT_DOUBLE_EQ(1.4142135623730951, Matrix1[0][1]);
    ASSERT_NEAR(0.0, Matrix1[0][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0606601717798212, Matrix1[1][0]);
    ASSERT_DOUBLE_EQ(1.06066017177982120, Matrix1[1][1]);
    ASSERT_NEAR(0.0, Matrix1[1][2], MYEPSILON);
    ASSERT_NEAR(0.0, Matrix1[2][0], MYEPSILON);
    ASSERT_NEAR(0.0, Matrix1[2][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00000000000000000, Matrix1[2][2]);

    ASSERT_DOUBLE_EQ(1.414213562373095100, Matrix2[0][0]);
    ASSERT_DOUBLE_EQ(1.414213562373095100, Matrix2[0][1]);
    ASSERT_DOUBLE_EQ(-2.00000000000000000, Matrix2[0][2]);
    ASSERT_DOUBLE_EQ(-0.35355339059327373, Matrix2[1][0]);
    ASSERT_DOUBLE_EQ(0.353553390593273790, Matrix2[1][1]);
    ASSERT_NEAR(0.0, Matrix2[1][2], MYEPSILON);
    ASSERT_NEAR(0.0, Matrix2[2][0], MYEPSILON);
    ASSERT_NEAR(0.0, Matrix2[2][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.000000000000000000, Matrix2[2][2]);

    ASSERT_DOUBLE_EQ(3.5355339059327378, Matrix3[0][0]);
    ASSERT_DOUBLE_EQ(3.5355339059327378, Matrix3[0][1]);
    ASSERT_NEAR(0.0, Matrix3[0][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(-5.3033008588991066, Matrix3[1][0]);
    ASSERT_DOUBLE_EQ(5.30330085889910660, Matrix3[1][1]);
    ASSERT_DOUBLE_EQ(-26.000000000000000, Matrix3[1][2]);
    ASSERT_NEAR(0.0, Matrix3[2][0], MYEPSILON);
    ASSERT_NEAR(0.0, Matrix3[2][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00000000000000000, Matrix3[2][2]);

    }