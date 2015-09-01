//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DSimilitudeTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DSimilitudeTester.h"

HGF2DSimilitudeTester::HGF2DSimilitudeTester() 
    {

    //General
    Translation = HGF2DDisplacement(10.0, 30.48);
    Rotation = PI/4;
    ScalingX = 1.00001;
    ScalingY = 2.00001;
    Anortho = -0.000001;
    Affine1 = HGF2DAffine(Translation, Rotation, ScalingX, ScalingY, Anortho);

    pWorld = new HGF2DCoordSys();
    pSys1 = new HGF2DCoordSys(Affine1, pWorld);

    //Similitude
    Similitude2.SetTranslation(HGF2DDisplacement(10.0,10.0));
    Similitude2.SetScaling(2.0);
    Similitude2.SetRotation(PI/ 4);

    }

//==================================================================================
// ConstructiontTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, DefaultConstructiontTest)
    {

    HGF2DSimilitude DefaultSimilitude;

    // Check transformation properties
    ASSERT_TRUE(DefaultSimilitude.IsIdentity());
    ASSERT_TRUE(DefaultSimilitude.IsStretchable());
    ASSERT_TRUE(DefaultSimilitude.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    DefaultSimilitude.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, DefaultSimilitude.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = DefaultSimilitude.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = DefaultSimilitude.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_TRUE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(DefaultSimilitude.PreservesLinearity());
    ASSERT_TRUE(DefaultSimilitude.PreservesParallelism());
    ASSERT_TRUE(DefaultSimilitude.PreservesShape());
    ASSERT_TRUE(DefaultSimilitude.PreservesDirection());

    // Reversal test
    DefaultSimilitude.Reverse();

    // Check again transformation properties
    ASSERT_TRUE(DefaultSimilitude.IsIdentity());
    ASSERT_TRUE(DefaultSimilitude.IsStretchable());
    ASSERT_TRUE(DefaultSimilitude.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    DefaultSimilitude.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// HGF2DSimilitude(const HGF2DDisplacement& pi_rTranslation, const HGFAngle& pi_rRotation,
//             double pi_Scaling);
//==================================================================================
TEST_F (HGF2DSimilitudeTester, SimpleTransformationConstructionTest)
    {

    HGF2DSimilitude     SimilitudeTranslation(HGF2DDisplacement(10.0, 10.0), 0.0, 1.0);
    HGF2DSimilitude     SimilitudeRotation(HGF2DDisplacement(0.0, 0.0), PI/4, 1.0);
    HGF2DSimilitude     SimilitudeScaling(HGF2DDisplacement(0.0, 0.0), 0.0, 10.0);

    // Check transformation properties
    ASSERT_FALSE(SimilitudeTranslation.IsIdentity());
    ASSERT_TRUE(SimilitudeTranslation.IsStretchable());
    ASSERT_TRUE(SimilitudeTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(SimilitudeRotation.IsIdentity());
    ASSERT_FALSE(SimilitudeRotation.IsStretchable());
    ASSERT_TRUE(SimilitudeRotation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(SimilitudeScaling.IsIdentity());
    ASSERT_TRUE(SimilitudeScaling.IsStretchable());
    ASSERT_TRUE(SimilitudeScaling.CanBeRepresentedByAMatrix());

    // Extract Translation stretch parameters
    SimilitudeTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract Rotation stretch parameters
    SimilitudeRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Scaling stretch parameters
    SimilitudeScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(10.0, ScalingX);
    ASSERT_DOUBLE_EQ(10.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, SimilitudeTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/4, SimilitudeRotation.GetRotation());
    ASSERT_NEAR(0.0, SimilitudeScaling.GetRotation(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, SimilitudeTranslation.GetClassID());
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, SimilitudeRotation.GetClassID());
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, SimilitudeScaling.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = SimilitudeTranslation.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = SimilitudeRotation.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix2[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix2[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][1]);

    HFCMatrix<3, 3> MyMatrix3 = SimilitudeScaling.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix3[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[0][0]);
    ASSERT_NEAR(0.0, MyMatrix3[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = SimilitudeTranslation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone2 = SimilitudeRotation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone3 = SimilitudeScaling.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pModelClone->GetClassID());
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pModelClone2->GetClassID());
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pModelClone3->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    ASSERT_FALSE(pModelClone2->IsIdentity());
    ASSERT_FALSE(pModelClone2->IsStretchable());
    ASSERT_TRUE(pModelClone2->CanBeRepresentedByAMatrix());

    ASSERT_FALSE(pModelClone3->IsIdentity());
    ASSERT_TRUE(pModelClone3->IsStretchable());
    ASSERT_TRUE(pModelClone3->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(SimilitudeTranslation.PreservesLinearity());
    ASSERT_TRUE(SimilitudeTranslation.PreservesParallelism());
    ASSERT_TRUE(SimilitudeTranslation.PreservesShape());
    ASSERT_TRUE(SimilitudeTranslation.PreservesDirection());

    ASSERT_TRUE(SimilitudeRotation.PreservesLinearity());
    ASSERT_TRUE(SimilitudeRotation.PreservesParallelism());
    ASSERT_TRUE(SimilitudeRotation.PreservesShape());
    ASSERT_FALSE(SimilitudeRotation.PreservesDirection());

    ASSERT_TRUE(SimilitudeScaling.PreservesLinearity());
    ASSERT_TRUE(SimilitudeScaling.PreservesParallelism());
    ASSERT_TRUE(SimilitudeScaling.PreservesShape());
    ASSERT_TRUE(SimilitudeScaling.PreservesDirection());

    // Reversal test
    SimilitudeTranslation.Reverse();
    SimilitudeRotation.Reverse();
    SimilitudeScaling.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(SimilitudeTranslation.IsIdentity());
    ASSERT_TRUE(SimilitudeTranslation.IsStretchable());
    ASSERT_TRUE(SimilitudeTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(SimilitudeRotation.IsIdentity());
    ASSERT_FALSE(SimilitudeRotation.IsStretchable());
    ASSERT_TRUE(SimilitudeRotation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(SimilitudeScaling.IsIdentity());
    ASSERT_TRUE(SimilitudeScaling.IsStretchable());
    ASSERT_TRUE(SimilitudeScaling.CanBeRepresentedByAMatrix());

    // Extract Translation again stretch parameters
    SimilitudeTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    // Extract Rotation again stretch parameters
    SimilitudeRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Scaling again stretch parameters
    SimilitudeScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.1, ScalingX);
    ASSERT_DOUBLE_EQ(0.1, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, SimilitudeTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-PI/4, SimilitudeRotation.GetRotation());
    ASSERT_NEAR(0.0, SimilitudeScaling.GetRotation(), MYEPSILON);

    }

//==================================================================================
// operator=(const HGF2DAffine& pi_rObj);
// HGF2DSimilitude(const HGF2DAffine& pi_rObj);
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CopyTest)
    {

    HGF2DSimilitude SimilitudeOperator = Similitude2;
    HGF2DSimilitude SimilitudeCopy(Similitude2);

    // Check transformation properties
    ASSERT_FALSE(SimilitudeOperator.IsIdentity());
    ASSERT_FALSE(SimilitudeOperator.IsStretchable());
    ASSERT_TRUE(SimilitudeOperator.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(SimilitudeCopy.IsIdentity());
    ASSERT_FALSE(SimilitudeCopy.IsStretchable());
    ASSERT_TRUE(SimilitudeCopy.CanBeRepresentedByAMatrix());

    // Extract stretch parameters of AffineOperator
    SimilitudeOperator.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract stretch parameters of AffineOperator
    SimilitudeCopy.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, SimilitudeOperator.GetClassID());
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, SimilitudeCopy.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = SimilitudeOperator.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = SimilitudeCopy.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix2[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix2[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[1][1]);

    //Check Rotation
    ASSERT_DOUBLE_EQ(PI/ 4, SimilitudeOperator.GetRotation());
    ASSERT_DOUBLE_EQ(PI/ 4, SimilitudeCopy.GetRotation());

    }

//==================================================================================
// CreateSimplifiedModel() const;
//==================================================================================
TEST_F(HGF2DSimilitudeTester, CreateSimplifiedModelTest)
    {
    
    //Simplified to HGF2DIdentity
    HGF2DSimilitude     Similitude1(HGF2DDisplacement(0.0, 0.0), 0.0, 1.0);
                                     
    HFCPtr<HGF2DTransfoModel> newModel1 = Similitude1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, newModel1->GetClassID());

    //Simplified to HGF2DTranslation
    HGF2DSimilitude     Similitude2(HGF2DDisplacement(10.0, 0.0), 0.0, 1.0);
    
    HFCPtr<HGF2DTransfoModel> newModel2 = Similitude2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, newModel2->GetClassID());

    //Simplified to HGF2DStretch
    HGF2DSimilitude     Similitude3(HGF2DDisplacement(10.0, 0.0), 0.0, 2.0);
    
    HFCPtr<HGF2DTransfoModel> newModel3 = Similitude3.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DStretch::CLASS_ID, newModel3->GetClassID());

    }

//==================================================================================
// SetByMatrixParameters(double pi_A0, double pi_A1, double pi_A2, double pi_B0, double pi_B1, double pi_B2);
//==================================================================================
TEST_F (HGF2DSimilitudeTester, SetByMatrixParametersTest)
    {

    HGF2DSimilitude Similitude;
    Similitude.SetByMatrixParameters(1.0, 0.0, 10.0, 0.0, 1.0, 10.0);

    // Check transformation properties
    ASSERT_FALSE(Similitude.IsIdentity());
    ASSERT_FALSE(Similitude.IsStretchable());
    ASSERT_TRUE(Similitude.CanBeRepresentedByAMatrix());

    Similitude.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(1.0, Translation.GetDeltaX());
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Similitude.GetMatrix();

    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][2]);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[0][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][0]);
    ASSERT_NEAR(0.0, MyMatrix1[1][1], MYEPSILON);

    //Check Rotation
    ASSERT_DOUBLE_EQ(1.5707963267948966, Similitude.GetRotation());

    }

//==================================================================================
// CompositionIdentityTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(DefaultSimilitude);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionSimilitudeTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(DefaultSimilitude);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi2->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionSimilitudeWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = DefaultSimilitude.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi5->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi5->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeReversalWithIdentityTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionSimilitudeReversalWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = DefaultSimilitude.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi6->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi6->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi6->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi6->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi6->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi6->PreservesDirection());

    }

//==================================================================================
// TranslationExtractionTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, TranslationExtractionTest)
    {

    HGF2DDisplacement MyTranslation = DefaultSimilitude.GetTranslation();

    ASSERT_NEAR(0.0, MyTranslation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, MyTranslation.GetDeltaY(), MYEPSILON);

    // Scale extraction test
    ASSERT_DOUBLE_EQ(1.0, DefaultSimilitude.GetScaling());

    // Extraction of rotation
    ASSERT_NEAR(0.0, DefaultSimilitude.GetRotation(), MYEPSILON);

    // Create a clone of default similitude
    HFCPtr<HGF2DSimilitude> pDupSimi = (HGF2DSimilitude*)DefaultSimilitude.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(0.3408, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0000, pDupSimi->GetScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Set scaling
    pDupSimi->SetScaling(2.0);
    ASSERT_DOUBLE_EQ(0.3408, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Add scaling
    pDupSimi->AddScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(0.6816, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Set rotation
    pDupSimi->SetRotation(PI / 3.0);

    ASSERT_DOUBLE_EQ(0.6816, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetScaling());
    ASSERT_DOUBLE_EQ(PI / 3.0, pDupSimi->GetRotation());

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);

    ASSERT_NEAR(3.3103911068029408, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(2.3464631426894398, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetScaling(), 10E-12);
    ASSERT_NEAR((PI / 3.0) + (PI / 4.0), pDupSimi->GetRotation(), 10E-12);

    // Addition of components with offset
    pDupSimi->AddScaling(2.5,10.0, 10.0);

    ASSERT_NEAR(18.2759777670073510, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-34.133842143276397, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetScaling(), 10E-12);
    ASSERT_NEAR((PI / 3.0) + (PI / 4.0), pDupSimi->GetRotation(), 10E-12);

    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);

    ASSERT_NEAR(-1.2132034355964194, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-41.201474683058322, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetScaling(), 10E-12);
    ASSERT_NEAR((PI / 3.0) + (PI / 2.0), pDupSimi->GetRotation(), 10E-12);

    }

//==================================================================================
// ConversionTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, ConversionTest)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    DefaultSimilitude.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    double XConverted;
    double YConverted;

    DefaultSimilitude.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    double XArray[10];
    double YArray[10];
    DefaultSimilitude.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(0.0, XArray[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, XArray[1]);
    ASSERT_DOUBLE_EQ(2.0, XArray[2]);
    ASSERT_DOUBLE_EQ(3.0, XArray[3]);
    ASSERT_DOUBLE_EQ(4.0, XArray[4]);
    ASSERT_DOUBLE_EQ(5.0, XArray[5]);
    ASSERT_DOUBLE_EQ(6.0, XArray[6]);
    ASSERT_DOUBLE_EQ(7.0, XArray[7]);
    ASSERT_DOUBLE_EQ(8.0, XArray[8]);
    ASSERT_DOUBLE_EQ(9.0, XArray[9]);

    ASSERT_DOUBLE_EQ(10.0, YArray[0]);
    ASSERT_DOUBLE_EQ(10.0, YArray[1]);
    ASSERT_DOUBLE_EQ(10.0, YArray[2]);
    ASSERT_DOUBLE_EQ(10.0, YArray[3]);
    ASSERT_DOUBLE_EQ(10.0, YArray[4]);
    ASSERT_DOUBLE_EQ(10.0, YArray[5]);
    ASSERT_DOUBLE_EQ(10.0, YArray[6]);
    ASSERT_DOUBLE_EQ(10.0, YArray[7]);
    ASSERT_DOUBLE_EQ(10.0, YArray[8]);
    ASSERT_DOUBLE_EQ(10.0, YArray[9]);

    double XDist(10.0);
    double YDist(10.0);

    double XResultDist;
    double YResultDist;

    DefaultSimilitude.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    // Inverse conversions
    DefaultSimilitude.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    DefaultSimilitude.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    DefaultSimilitude.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(0.0, XArray[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, XArray[1]);
    ASSERT_DOUBLE_EQ(2.0, XArray[2]);
    ASSERT_DOUBLE_EQ(3.0, XArray[3]);
    ASSERT_DOUBLE_EQ(4.0, XArray[4]);
    ASSERT_DOUBLE_EQ(5.0, XArray[5]);
    ASSERT_DOUBLE_EQ(6.0, XArray[6]);
    ASSERT_DOUBLE_EQ(7.0, XArray[7]);
    ASSERT_DOUBLE_EQ(8.0, XArray[8]);
    ASSERT_DOUBLE_EQ(9.0, XArray[9]);

    ASSERT_DOUBLE_EQ(10.0, YArray[0]);
    ASSERT_DOUBLE_EQ(10.0, YArray[1]);
    ASSERT_DOUBLE_EQ(10.0, YArray[2]);
    ASSERT_DOUBLE_EQ(10.0, YArray[3]);
    ASSERT_DOUBLE_EQ(10.0, YArray[4]);
    ASSERT_DOUBLE_EQ(10.0, YArray[5]);
    ASSERT_DOUBLE_EQ(10.0, YArray[6]);
    ASSERT_DOUBLE_EQ(10.0, YArray[7]);
    ASSERT_DOUBLE_EQ(10.0, YArray[8]);
    ASSERT_DOUBLE_EQ(10.0, YArray[9]);

    DefaultSimilitude.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    }

TEST_F (HGF2DSimilitudeTester, ConversionInOutArraysTest)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];

    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    DefaultSimilitude.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(0.0, XArrayInOut[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(2.0, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(3.0, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(4.0, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(5.0, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(6.0, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(7.0, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(8.0, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(9.0, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[9]);

    // Reset the in/out arrays before testing ConvertInverse
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }
    DefaultSimilitude.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(0.0, XArrayInOut[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(2.0, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(3.0, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(4.0, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(5.0, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(6.0, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(7.0, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(8.0, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(9.0, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(10.0, YArrayInOut[9]);
    }

//==================================================================================
// ConstructiontTest3
//==================================================================================
TEST_F (HGF2DSimilitudeTester, ConstructiontTest3)
    {

    // Create duplicates for compare operations
    HGF2DSimilitude Similitude3(Similitude2);
    HGF2DSimilitude Similitude4(Similitude2);
    Similitude4.Reverse();

    // Check transformation properties
    ASSERT_FALSE(Similitude2.IsIdentity());
    ASSERT_FALSE(Similitude2.IsStretchable());
    ASSERT_TRUE(Similitude2.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    Similitude2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, Similitude2.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Similitude2.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = Similitude2.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_FALSE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(Similitude2.PreservesLinearity());
    ASSERT_TRUE(Similitude2.PreservesParallelism());
    ASSERT_TRUE(Similitude2.PreservesShape());
    ASSERT_FALSE(Similitude2.PreservesDirection());

    // Reversal test
    Similitude2.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(Similitude2.IsIdentity());
    ASSERT_FALSE(Similitude2.IsStretchable());
    ASSERT_TRUE(Similitude2.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    Similitude2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.5, ScalingX);
    ASSERT_DOUBLE_EQ(0.5, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-7.0710678118654755, Translation.GetDeltaX());
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    Similitude2.Reverse();

    }

//==================================================================================
// CompositionIdentityTest3
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(Similitude2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi->PreservesShape());
    ASSERT_FALSE(pIdenCompSimi->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeTest3
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionSimilitudeTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(Similitude2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi2->PreservesShape());
    ASSERT_FALSE(pIdenCompSimi2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTest3
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionSimilitudeWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = Similitude2.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi5->PreservesShape());
    ASSERT_FALSE(pIdenCompSimi5->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeReversalWithIdentityTest3
//==================================================================================
TEST_F (HGF2DSimilitudeTester, CompositionSimilitudeReversalWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = Similitude2.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pIdenCompSimi6->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi6->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi6->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi6->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi6->PreservesShape());
    ASSERT_FALSE(pIdenCompSimi6->PreservesDirection());

    }

//==================================================================================
// TranslationExtractionTest3
//==================================================================================
TEST_F (HGF2DSimilitudeTester, TranslationExtractionTest3)
    {

    HGF2DDisplacement MyTranslation = Similitude2.GetTranslation();

    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaY());

    // Scale extraction test
    ASSERT_DOUBLE_EQ(2.0, Similitude2.GetScaling());

    // Extraction of rotation
    ASSERT_DOUBLE_EQ(PI/4, Similitude2.GetRotation());

    // Create a clone of default similitude
    HFCPtr<HGF2DSimilitude> pDupSimi = (HGF2DSimilitude*)Similitude2.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetScaling());
    ASSERT_DOUBLE_EQ(PI/4, pDupSimi->GetRotation());

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(0.3408, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetScaling());
    ASSERT_DOUBLE_EQ(PI/4, pDupSimi->GetRotation());

    // Set scaling
    pDupSimi->SetScaling(2.0);
    ASSERT_DOUBLE_EQ(0.3408, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetScaling());
    ASSERT_DOUBLE_EQ(PI/4, pDupSimi->GetRotation());

    // Add scaling
    pDupSimi->AddScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(0.6816, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetScaling());
    ASSERT_DOUBLE_EQ(PI/4, pDupSimi->GetRotation());

    // Set rotation
    pDupSimi->SetRotation(PI / 3.0);

    ASSERT_DOUBLE_EQ(0.6816, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0000, pDupSimi->GetScaling());
    ASSERT_DOUBLE_EQ(PI/3, pDupSimi->GetRotation());

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);

    ASSERT_NEAR(3.3103911068029408, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(2.3464631426894398, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetScaling(), 10E-12);
    ASSERT_NEAR((PI/3) + (PI/4), pDupSimi->GetRotation(), 10E-12);

    // Addition of components with offset
    pDupSimi->AddScaling(2.5,10.0, 10.0);

    ASSERT_NEAR(18.2759777670073510, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-34.133842143276397, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetScaling(), 10E-12);
    ASSERT_NEAR((PI / 3.0) + (PI / 4.0), pDupSimi->GetRotation(), 10E-12);

    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);

    ASSERT_NEAR(-1.2132034355964194, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-41.201474683058322, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetScaling(), 10E-12);
    ASSERT_NEAR(PI/3 + PI/2, pDupSimi->GetRotation(), 10E-12);

    }

//==================================================================================
// ConversionTest3
//==================================================================================
TEST_F (HGF2DSimilitudeTester, ConversionTest3)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    Similitude2.ConvertDirect(&XCoord, &YCoord);
    ASSERT_NEAR(10.000000000000000, XCoord, MYEPSILON);
    ASSERT_NEAR(38.284271247461902, YCoord, MYEPSILON);

    double XConverted;
    double YConverted;
    XCoord = 10.0;
    YCoord = 10.0;

    Similitude2.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(10.000000000000000, XConverted, MYEPSILON);
    ASSERT_NEAR(38.284271247461902, YConverted, MYEPSILON);

    double XArray[10];
    double YArray[10];
    Similitude2.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(-4.1421356237309492, XArray[0], MYEPSILON);
    ASSERT_NEAR(-2.7279220613578543, XArray[1], MYEPSILON);
    ASSERT_NEAR(-1.3137084989847589, XArray[2], MYEPSILON);
    ASSERT_NEAR(0.10050506338833642, XArray[3], MYEPSILON);
    ASSERT_NEAR(1.51471862576143130, XArray[4], MYEPSILON);
    ASSERT_NEAR(2.92893218813452630, XArray[5], MYEPSILON);
    ASSERT_NEAR(4.34314575050762210, XArray[6], MYEPSILON);
    ASSERT_NEAR(5.75735931288071610, XArray[7], MYEPSILON);
    ASSERT_NEAR(7.17157287525381190, XArray[8], MYEPSILON);
    ASSERT_NEAR(8.58578643762690770, XArray[9], MYEPSILON); 

    ASSERT_NEAR(24.142135623730951, YArray[0], MYEPSILON);
    ASSERT_NEAR(25.556349186104047, YArray[1], MYEPSILON);
    ASSERT_NEAR(26.970562748477143, YArray[2], MYEPSILON);
    ASSERT_NEAR(28.384776310850235, YArray[3], MYEPSILON);
    ASSERT_NEAR(29.798989873223331, YArray[4], MYEPSILON);
    ASSERT_NEAR(31.213203435596427, YArray[5], MYEPSILON);
    ASSERT_NEAR(32.627416997969519, YArray[6], MYEPSILON);
    ASSERT_NEAR(34.041630560342611, YArray[7], MYEPSILON);
    ASSERT_NEAR(35.455844122715710, YArray[8], MYEPSILON);
    ASSERT_NEAR(36.870057685088810, YArray[9], MYEPSILON);

    double XDist(10.0);
    double YDist(10.0);

    double XResultDist;
    double YResultDist;

    Similitude2.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(10.000000000000000, XResultDist, MYEPSILON);
    ASSERT_NEAR(38.284271247461902, YResultDist, MYEPSILON);

    // Inverse conversions
    XCoord = 10.0;
    YCoord = 10.0;
    Similitude2.ConvertInverse(&XCoord, &YCoord);
    ASSERT_NEAR(0.0, XCoord, MYEPSILON);
    ASSERT_NEAR(0.0, YCoord, MYEPSILON);

    XCoord = 10.0;
    YCoord = 10.0;

    Similitude2.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(0.0, XConverted, MYEPSILON);
    ASSERT_NEAR(0.0, YConverted, MYEPSILON);

    Similitude2.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(-3.53553390593273820, XArray[0], MYEPSILON);
    ASSERT_NEAR(-3.18198051533946420, XArray[1], MYEPSILON);
    ASSERT_NEAR(-2.82842712474619070, XArray[2], MYEPSILON);
    ASSERT_NEAR(-2.47487373415291680, XArray[3], MYEPSILON);
    ASSERT_NEAR(-2.12132034355964280, XArray[4], MYEPSILON);
    ASSERT_NEAR(-1.76776695296636930, XArray[5], MYEPSILON);
    ASSERT_NEAR(-1.41421356237309540, XArray[6], MYEPSILON);
    ASSERT_NEAR(-1.06066017177982190, XArray[7], MYEPSILON);
    ASSERT_NEAR(-0.70710678118654791, XArray[8], MYEPSILON);
    ASSERT_NEAR(-0.35355339059327395, XArray[9], MYEPSILON);

    ASSERT_NEAR(3.53553390593273730, YArray[0], MYEPSILON);
    ASSERT_NEAR(3.18198051533946380, YArray[1], MYEPSILON);
    ASSERT_NEAR(2.82842712474618980, YArray[2], MYEPSILON);
    ASSERT_NEAR(2.47487373415291590, YArray[3], MYEPSILON);
    ASSERT_NEAR(2.12132034355964240, YArray[4], MYEPSILON);
    ASSERT_NEAR(1.76776695296636870, YArray[5], MYEPSILON);
    ASSERT_NEAR(1.41421356237309490, YArray[6], MYEPSILON);
    ASSERT_NEAR(1.06066017177982140, YArray[7], MYEPSILON);
    ASSERT_NEAR(0.70710678118654746, YArray[8], MYEPSILON);
    ASSERT_NEAR(0.35355339059327351, YArray[9], MYEPSILON);

    Similitude2.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(0.0, XResultDist, MYEPSILON);
    ASSERT_NEAR(0.0, YResultDist, MYEPSILON);

    }

TEST_F (HGF2DSimilitudeTester, ConversionInOutArraysTest3)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];

    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    Similitude2.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(-4.1421356237309492, XArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(-2.7279220613578543, XArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(-1.3137084989847589, XArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(0.10050506338833642, XArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(1.51471862576143130, XArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(2.92893218813452630, XArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(4.34314575050762210, XArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(5.75735931288071610, XArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(7.17157287525381190, XArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(8.58578643762690770, XArrayInOut[9], MYEPSILON); 

    ASSERT_NEAR(24.142135623730951, YArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(25.556349186104047, YArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(26.970562748477143, YArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(28.384776310850235, YArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(29.798989873223331, YArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(31.213203435596427, YArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(32.627416997969519, YArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(34.041630560342611, YArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(35.455844122715710, YArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(36.870057685088810, YArrayInOut[9], MYEPSILON);

    // Reset the in/out arrays before testing ConvertInverse
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }
    Similitude2.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(-3.53553390593273820, XArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(-3.18198051533946420, XArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(-2.82842712474619070, XArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(-2.47487373415291680, XArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(-2.12132034355964280, XArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(-1.76776695296636930, XArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(-1.41421356237309540, XArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(-1.06066017177982190, XArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(-0.70710678118654791, XArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(-0.35355339059327395, XArrayInOut[9], MYEPSILON);

    ASSERT_NEAR(3.53553390593273730, YArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(3.18198051533946380, YArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(2.82842712474618980, YArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(2.47487373415291590, YArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(2.12132034355964240, YArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(1.76776695296636870, YArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(1.41421356237309490, YArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(1.06066017177982140, YArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(0.70710678118654746, YArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(0.35355339059327351, YArrayInOut[9], MYEPSILON);
    }
//==================================================================================
// SpecialReversalTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, SpecialReversalTest)
    {

    HGF2DSimilitude Similitude1;
    Similitude1.SetRotation(PI / 4);
    Similitude1.Reverse();

    ASSERT_DOUBLE_EQ(1.0, Similitude1.GetScaling());
    ASSERT_DOUBLE_EQ(-PI / 4, Similitude1.GetRotation());
    ASSERT_NEAR(0.0, Similitude1.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Similitude1.GetTranslation().GetDeltaY(), MYEPSILON);

    // Similitude with translation only
    HGF2DSimilitude Similitude2;
    Similitude2.SetTranslation(HGF2DDisplacement(-10.0,-10.0));
    Similitude2.Reverse();

    ASSERT_DOUBLE_EQ(1.0, Similitude2.GetScaling());
    ASSERT_NEAR(0.0, Similitude2.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Similitude2.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Similitude2.GetTranslation().GetDeltaY());

    // Test with scaling only
    HGF2DSimilitude Similitude3;
    Similitude3.SetScaling(0.25);
    Similitude3.Reverse();

    ASSERT_DOUBLE_EQ(4.0, Similitude3.GetScaling());
    ASSERT_NEAR(0.0, Similitude3.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, Similitude3.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Similitude3.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both rotation and translation
    HGF2DSimilitude Similitude4;
    Similitude4.SetRotation(PI / 4);
    Similitude4.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Similitude4.Reverse();

    ASSERT_DOUBLE_EQ(1.000000000000000000, Similitude4.GetScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Similitude4.GetRotation());
    ASSERT_DOUBLE_EQ(14.14213562373095100, Similitude4.GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, Similitude4.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both scaling and translation
    HGF2DSimilitude Similitude5;
    Similitude5.SetScaling(2.1);
    Similitude5.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Similitude5.Reverse();

    ASSERT_DOUBLE_EQ(1/2.1, Similitude5.GetScaling());
    ASSERT_NEAR(0.0, Similitude5.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0 / 2.1, Similitude5.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0 / 2.1, Similitude5.GetTranslation().GetDeltaY());

    // Test with both scaling and rotation
    HGF2DSimilitude Similitude6;
    Similitude6.SetScaling(2.1);
    Similitude6.SetRotation(PI / 4);
    Similitude6.Reverse();

    ASSERT_DOUBLE_EQ(1/2.1, Similitude6.GetScaling());
    ASSERT_DOUBLE_EQ(-PI / 4, Similitude6.GetRotation());
    ASSERT_NEAR(0.0, Similitude6.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Similitude6.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both scaling and rotation and translation
    HGF2DSimilitude Similitude7;
    Similitude7.SetScaling(2.1);
    Similitude7.SetRotation(PI / 4);
    Similitude7.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Similitude7.Reverse();

    ASSERT_DOUBLE_EQ(1/2.1,  Similitude7.GetScaling());
    ASSERT_DOUBLE_EQ(-PI / 4, Similitude7.GetRotation());
    ASSERT_DOUBLE_EQ(6.7343502970147382, Similitude7.GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, Similitude7.GetTranslation().GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// SpecialCompositionTest
//==================================================================================
TEST_F (HGF2DSimilitudeTester, SpecialCompositionTest)
    {

    HGF2DSimilitude Similitude1;
    Similitude1.SetScaling(2.1);
    Similitude1.SetRotation(PI / 4);
    Similitude1.SetTranslation(HGF2DDisplacement(-10.0, -10.0));

    HGF2DSimilitude Similitude2;
    Similitude2.SetScaling(-632.1);
    Similitude2.SetRotation(PI / 3);
    Similitude2.SetTranslation(HGF2DDisplacement(1560.0, -1045.0));
 
    HFCPtr<HGF2DTransfoModel> pNewModel1and2 = Similitude1.ComposeInverseWithDirectOf(Similitude2);
    HFCPtr<HGF2DSimilitude> pSimiRes = (HGF2DSimilitude*)pNewModel1and2->Clone();

    HFCMatrix<3, 3> MyMatrix1 = pSimiRes->GetMatrix();
    ASSERT_NEAR(-753.64657732143587, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(7589.64657732143680, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(343.558988659536850, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(1282.17960107437220, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-1282.1796010743722, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(343.558988659536910, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel1andI2 = Similitude1.ComposeInverseWithInverseOf(Similitude2);
    HFCPtr<HGF2DSimilitude> pSimiRes2 = (HGF2DSimilitude*)pNewModel1andI2->Clone();

    MyMatrix1 = pSimiRes2->GetMatrix();
    ASSERT_NEAR(-0.17613715063580709000, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-2.96971979740795570000, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(-0.00320905590129258590, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.00085986393721767672, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.000859863937217676720, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(-0.00320905590129258590, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel2and1 = Similitude2.ComposeInverseWithDirectOf(Similitude1);
    HFCPtr<HGF2DSimilitude> pSimiRes3 = (HGF2DSimilitude*)pNewModel2and1->Clone();

    MyMatrix1 = pSimiRes3->GetMatrix();
    ASSERT_NEAR(3858.22764648100840, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(754.735983853251130, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(343.558988659536850, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(1282.17960107437240, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-1282.1796010743724, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(343.558988659536850, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel2andI1 = Similitude2.ComposeInverseWithInverseOf(Similitude1);

    HFCPtr<HGF2DSimilitude> pSimiRes4 = (HGF2DSimilitude*)pNewModel2andI1->Clone();

    MyMatrix1 = pSimiRes4->GetMatrix();
    ASSERT_NEAR(180.143870445144360, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-877.14912618616972, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(-290.74367371300951, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(77.9045325758587100, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-77.904532575858710, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(-290.74367371300951, MyMatrix1[1][1], 10E-12);

    // Composition with translation
    HGF2DTranslation TransModel;

    TransModel.SetTranslation(HGF2DDisplacement(10.0, 11.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1andTrans = Similitude1.ComposeInverseWithDirectOf(TransModel);
    HFCPtr<HGF2DSimilitude> pSimiRes5 = (HGF2DSimilitude*)pNewModel1andTrans->Clone();

    MyMatrix1 = pSimiRes5->GetMatrix();
    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(1.00000000000000000, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-1.4849242404917500, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel1andITrans = Similitude1.ComposeInverseWithInverseOf(TransModel);
    HFCPtr<HGF2DSimilitude> pSimiRes6 = (HGF2DSimilitude*)pNewModel1andITrans->Clone();

    MyMatrix1 = pSimiRes6->GetMatrix();
    ASSERT_NEAR(-20.000000000000000, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-21.000000000000000, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-1.4849242404917500, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModelTransand1 = TransModel.ComposeInverseWithDirectOf(Similitude1);
    HFCPtr<HGF2DSimilitude> pSimiRes7 = (HGF2DSimilitude*)pNewModelTransand1->Clone();

    MyMatrix1 = pSimiRes7->GetMatrix();
    ASSERT_NEAR(-11.484924240491749, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(21.1834090503267480, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-1.4849242404917500, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModelTransandI1 = TransModel.ComposeInverseWithInverseOf(Similitude1);
    HFCPtr<HGF2DSimilitude> pSimiRes8 = (HGF2DSimilitude*)pNewModelTransandI1->Clone();

    MyMatrix1 = pSimiRes8->GetMatrix();
    ASSERT_NEAR(13.80541810888021500, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(0.336717514850737700, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.336717514850736920, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(0.336717514850736870, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-0.33671751485073687, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.336717514850736920, MyMatrix1[1][1], 10E-12);
    
    }

