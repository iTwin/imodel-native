//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DAffineTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DAffineTester.h"

HGF2DAffineTester::HGF2DAffineTester() 
    {

    //General
    Translation = HGF2DDisplacement(10.0, 30.48);
    Rotation = PI/4;
    ScalingX = 1.00001;
    ScalingY = 2.00001;
    Anortho = -0.000001;
    Affine1 = HGF2DAffine(Translation, Rotation, ScalingX, ScalingY, Anortho);

    // COORDINATE SYSTEMS
    pWorld = new HGF2DCoordSys();
    pSys1 = new HGF2DCoordSys(Affine1, pWorld);

    //Affine
    Affine2.SetTranslation(HGF2DDisplacement(10.0, 10.0));
    Affine2.SetXScaling(2.0);
    Affine2.SetYScaling(2.0);
    Affine2.SetRotation(PI/4);

    }

//==================================================================================
// DefaultConstructiontTest
//==================================================================================
TEST_F (HGF2DAffineTester, DefaultConstructiontTest)
    {

    // Check transformation properties
    ASSERT_TRUE(DefaultAffine.IsIdentity());
    ASSERT_TRUE(DefaultAffine.IsStretchable());
    ASSERT_TRUE(DefaultAffine.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    DefaultAffine.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DAffine::CLASS_ID, DefaultAffine.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = DefaultAffine.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = DefaultAffine.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_TRUE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(DefaultAffine.PreservesLinearity());
    ASSERT_TRUE(DefaultAffine.PreservesParallelism());
    ASSERT_TRUE(DefaultAffine.PreservesShape());
    ASSERT_TRUE(DefaultAffine.PreservesDirection());

    // Reversal test
    DefaultAffine.Reverse();

    // Check again transformation properties
    ASSERT_TRUE(DefaultAffine.IsIdentity());
    ASSERT_TRUE(DefaultAffine.IsStretchable());
    ASSERT_TRUE(DefaultAffine.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    DefaultAffine.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// HGF2DAffine(const HGF2DDisplacement& pi_rTranslation, const HGFAngle& pi_rRotation,
//             double pi_ScalingX, double pi_ScalingY, const HGFAngle& pi_rAnorthogonality);
//==================================================================================
TEST_F (HGF2DAffineTester, SimpleTransformationConstructionTest)
    {

    HGF2DAffine     AffineTranslation(HGF2DDisplacement(10.0, 10.0), 0.0, 1.0, 1.0, 0.0);
    HGF2DAffine     AffineRotation(HGF2DDisplacement(0.0, 0.0), PI/4, 1.0, 1.0, 0.0);
    HGF2DAffine     AffineScaling(HGF2DDisplacement(0.0, 0.0), 0.0, 10.0, 10.0, 0.0);
    HGF2DAffine     AffineAnothorgonality(HGF2DDisplacement(0.0, 0.0), 0.0, 1.0, 1.0, (PI/4));

    // Check transformation properties
    ASSERT_FALSE(AffineTranslation.IsIdentity());
    ASSERT_TRUE(AffineTranslation.IsStretchable());
    ASSERT_TRUE(AffineTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(AffineRotation.IsIdentity());
    ASSERT_FALSE(AffineRotation.IsStretchable());
    ASSERT_TRUE(AffineRotation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(AffineScaling.IsIdentity());
    ASSERT_TRUE(AffineScaling.IsStretchable());
    ASSERT_TRUE(AffineScaling.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(AffineAnothorgonality.IsIdentity());
    ASSERT_FALSE(AffineAnothorgonality.IsStretchable());
    ASSERT_TRUE(AffineAnothorgonality.CanBeRepresentedByAMatrix());

    // Extract Translation stretch parameters
    AffineTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract Rotation stretch parameters
    AffineRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Scaling stretch parameters
    AffineScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(10.0, ScalingX);
    ASSERT_DOUBLE_EQ(10.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Anothorgality stretch parameters
    AffineAnothorgonality.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, AffineTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/4, AffineRotation.GetRotation());
    ASSERT_NEAR(0.0, AffineScaling.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, AffineAnothorgonality.GetRotation(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DAffine::CLASS_ID, AffineTranslation.GetClassID());
    ASSERT_EQ(HGF2DAffine::CLASS_ID, AffineRotation.GetClassID());
    ASSERT_EQ(HGF2DAffine::CLASS_ID, AffineScaling.GetClassID());
    ASSERT_EQ(HGF2DAffine::CLASS_ID, AffineAnothorgonality.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = AffineTranslation.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = AffineRotation.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix2[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix2[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][1]);

    HFCMatrix<3, 3> MyMatrix3 = AffineScaling.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix3[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[0][0]);
    ASSERT_NEAR(0.0, MyMatrix3[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[1][1]);

    HFCMatrix<3, 3> MyMatrix4 = AffineAnothorgonality.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix4[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix4[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.000000000000000000, MyMatrix4[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix4[0][1]);
    ASSERT_NEAR(0.0, MyMatrix4[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.70710678118654757, MyMatrix4[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = AffineTranslation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone2 = AffineRotation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone3 = AffineScaling.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone4 = AffineAnothorgonality.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pModelClone->GetClassID());
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pModelClone2->GetClassID());
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pModelClone3->GetClassID());
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pModelClone4->GetClassID());

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

    ASSERT_FALSE(pModelClone4->IsIdentity());
    ASSERT_FALSE(pModelClone4->IsStretchable());
    ASSERT_TRUE(pModelClone4->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(AffineTranslation.PreservesLinearity());
    ASSERT_TRUE(AffineTranslation.PreservesParallelism());
    ASSERT_TRUE(AffineTranslation.PreservesShape());
    ASSERT_TRUE(AffineTranslation.PreservesDirection());

    ASSERT_TRUE(AffineRotation.PreservesLinearity());
    ASSERT_TRUE(AffineRotation.PreservesParallelism());
    ASSERT_TRUE(AffineRotation.PreservesShape());
    ASSERT_FALSE(AffineRotation.PreservesDirection());

    ASSERT_TRUE(AffineScaling.PreservesLinearity());
    ASSERT_TRUE(AffineScaling.PreservesParallelism());
    ASSERT_TRUE(AffineScaling.PreservesShape());
    ASSERT_TRUE(AffineScaling.PreservesDirection());

    ASSERT_TRUE(AffineAnothorgonality.PreservesLinearity());
    ASSERT_TRUE(AffineAnothorgonality.PreservesParallelism());
    ASSERT_FALSE(AffineAnothorgonality.PreservesShape());
    ASSERT_FALSE(AffineAnothorgonality.PreservesDirection());

    // Reversal test
    AffineTranslation.Reverse();
    AffineRotation.Reverse();
    AffineScaling.Reverse();
    AffineAnothorgonality.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(AffineTranslation.IsIdentity());
    ASSERT_TRUE(AffineTranslation.IsStretchable());
    ASSERT_TRUE(AffineTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(AffineRotation.IsIdentity());
    ASSERT_FALSE(AffineRotation.IsStretchable());
    ASSERT_TRUE(AffineRotation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(AffineScaling.IsIdentity());
    ASSERT_TRUE(AffineScaling.IsStretchable());
    ASSERT_TRUE(AffineScaling.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(AffineAnothorgonality.IsIdentity());
    ASSERT_FALSE(AffineAnothorgonality.IsStretchable());
    ASSERT_TRUE(AffineAnothorgonality.CanBeRepresentedByAMatrix());

    // Extract Translation again stretch parameters
    AffineTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    // Extract Rotation again stretch parameters
    AffineRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Scaling again stretch parameters
    AffineScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.1, ScalingX);
    ASSERT_DOUBLE_EQ(0.1, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Anothorgonality again stretch parameters
    AffineScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_NEAR(0.1, ScalingX, MYEPSILON);
    ASSERT_NEAR(0.1, ScalingY, MYEPSILON);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, AffineTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-PI/4, AffineRotation.GetRotation());
    ASSERT_NEAR(0.0, AffineScaling.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, AffineAnothorgonality.GetRotation(), MYEPSILON); 

    }

//==================================================================================
// operator=(const HGF2DAffine& pi_rObj);
// HGF2DAffine(const HGF2DAffine& pi_rObj);
//==================================================================================
TEST_F (HGF2DAffineTester, CopyTest)
    {

    HGF2DAffine AffineOperator = Affine2;
    HGF2DAffine AffineCopy(Affine2);

    // Check transformation properties
    ASSERT_FALSE(AffineOperator.IsIdentity());
    ASSERT_FALSE(AffineOperator.IsStretchable());
    ASSERT_TRUE(AffineOperator.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(AffineCopy.IsIdentity());
    ASSERT_FALSE(AffineCopy.IsStretchable());
    ASSERT_TRUE(AffineCopy.CanBeRepresentedByAMatrix());

    // Extract stretch parameters of AffineOperator
    AffineOperator.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract stretch parameters of AffineOperator
    AffineCopy.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DAffine::CLASS_ID, AffineOperator.GetClassID());
    ASSERT_EQ(HGF2DAffine::CLASS_ID, AffineCopy.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = AffineOperator.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = AffineCopy.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix2[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix2[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[1][1]);

    //Check Rotation
    ASSERT_DOUBLE_EQ(PI/ 4, AffineOperator.GetRotation());
    ASSERT_DOUBLE_EQ(PI/ 4, AffineCopy.GetRotation());

    }

//==================================================================================
// CreateSimplifiedModel() const;
//==================================================================================
TEST_F(HGF2DAffineTester, CreateSimplifiedModelTest)
    {
    
    //Simplified to HGF2DIdentity
    HGF2DAffine     Affine1(HGF2DDisplacement(0.0, 0.0), 0.0, 1.0, 1.0, 0.0);
                                     
    HFCPtr<HGF2DTransfoModel> newModel1 = Affine1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, newModel1->GetClassID());

    //Simplified to HGF2DTranslation
    HGF2DAffine     Affine2(HGF2DDisplacement(10.0, 0.0), 0.0, 1.0, 1.0, 0.0);
    
    HFCPtr<HGF2DTransfoModel> newModel2 = Affine2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, newModel2->GetClassID());

    //Simplified to HGF2DStretch
    HGF2DAffine     Affine3(HGF2DDisplacement(10.0, 0.0), 0.0, 2.0, 3.0, 0.0);
    
    HFCPtr<HGF2DTransfoModel> newModel3 = Affine3.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DStretch::CLASS_ID, newModel3->GetClassID());

    //Simplified to HGF2DSimilitude
    HGF2DAffine     Affine4(HGF2DDisplacement(0.0, 0.0), PI, 1.0, 1.0, 0.0);
    
    HFCPtr<HGF2DTransfoModel> newModel4 = Affine4.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, newModel4->GetClassID());

    }

//==================================================================================
// SetByMatrixParameters(double pi_A0, double pi_A1, double pi_A2, double pi_B0, double pi_B1, double pi_B2);
//==================================================================================
TEST_F (HGF2DAffineTester, SetByMatrixParametersTest)
    {

    HGF2DAffine Affine1;
    Affine1.SetByMatrixParameters(1.0, 0.0, 10.0, 0.0, 1.0, 10.0);

    // Check transformation properties
    ASSERT_FALSE(Affine1.IsIdentity());
    ASSERT_FALSE(Affine1.IsStretchable());
    ASSERT_TRUE(Affine1.CanBeRepresentedByAMatrix());

    Affine1.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.00000000000000000, ScalingX);
    ASSERT_DOUBLE_EQ(-14.142135623730947, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(1.0, Translation.GetDeltaX());
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Affine1.GetMatrix();

    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[0][2]);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[0][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][1]);

    //Check Rotation
    ASSERT_DOUBLE_EQ(1.5707963267948966, Affine1.GetRotation());

    }
    
//==================================================================================
// CompositionIdentityTest
//==================================================================================
TEST_F (HGF2DAffineTester, CompositionIdentityTest)
    {

    // Composition with similitude
    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(DefaultAffine);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi->GetClassID());

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
TEST_F (HGF2DAffineTester, CompositionSimilitudeTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(DefaultAffine);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi2->GetClassID());

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
TEST_F (HGF2DAffineTester, CompositionSimilitudeWithIdentityTest)
    {
  
    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = DefaultAffine.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi5->GetClassID());

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
TEST_F (HGF2DAffineTester, CompositionSimilitudeReversalWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = DefaultAffine.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi6->GetClassID());

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
TEST_F (HGF2DAffineTester, TranslationExtractionTest)
    {
      
    HGF2DDisplacement MyTranslation = DefaultAffine.GetTranslation();

    ASSERT_NEAR(0.0, MyTranslation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, MyTranslation.GetDeltaY(), MYEPSILON);

    // Scale extraction test
    ASSERT_DOUBLE_EQ(1.0, DefaultAffine.GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, DefaultAffine.GetYScaling());

    // Extraction of rotation
    ASSERT_NEAR(0.0, DefaultAffine.GetRotation(), MYEPSILON);

    // Create a clone of default similitude
    HFCPtr<HGF2DAffine> pDupSimi = (HGF2DAffine*)DefaultAffine.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(1.0, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Set scaling
    pDupSimi->SetXScaling(2.0);
    pDupSimi->SetYScaling(2.0);
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Add scaling
    pDupSimi->AddIsotropicScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(4.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Set rotation
    pDupSimi->SetRotation(PI / 3.0);

    ASSERT_NEAR(2.0000000000000000, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(1.0471975511965976, pDupSimi->GetRotation(), 10E-12);

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);

    ASSERT_NEAR(4.2426406871192848, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(1.4142135623730954, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(1.8325957145940459, pDupSimi->GetRotation(), 10E-12);

    // Addition of components with offset
    pDupSimi->AddIsotropicScaling(2.5, 10.0, 10.0 );

    ASSERT_NEAR(20.6066017177982130, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-36.464466094067262, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(1.83259571459404590, pDupSimi->GetRotation(), 10E-12);

    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);

    ASSERT_NEAR(-1.2132034355964212, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-44.497474683058329, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(2.61799387799149400, pDupSimi->GetRotation(), 10E-12);

    // Set Anorthogonality
    pDupSimi->SetAnorthogonality(PI / 4);

    // Check result of setting
    ASSERT_NEAR(-1.2132034355964212, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-44.497474683058329, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(2.61799387799149400, pDupSimi->GetRotation(), 10E-12);
    ASSERT_NEAR(0.78539816339744828, pDupSimi->GetAnorthogonality(), 10E-12);

    // Create a clone of default projective
    pDupSimi = (HGF2DAffine*)DefaultAffine.Clone();

    //Add Scaling
    pDupSimi->AddAnisotropicScaling(2.0, 3.0);

    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddAnisotropicScaling(5.0, 6.0, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(-40.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-50.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(10.00, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(18.00, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddHorizontalFlip();

    ASSERT_DOUBLE_EQ(-40.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-50.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(-10.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(18.00, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddVerticalFlip();

    ASSERT_DOUBLE_EQ(-40.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-50.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(-10.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(-18.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddHorizontalFlip(5.0);

    ASSERT_DOUBLE_EQ(-30.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-50.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(10.00, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(-18.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddVerticalFlip(5.0);

    ASSERT_DOUBLE_EQ(-30.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-40.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(10.00, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(18.00, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    }

//==================================================================================
// ConversionTest
//==================================================================================
TEST_F (HGF2DAffineTester, ConversionTest)
    {

    // Direct conversions
    double XCoord = 10.0;
    double YCoord = 10.0;

    DefaultAffine.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    double XConverted;
    double YConverted;

    DefaultAffine.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    double XArray[10];
    double YArray[10];
    DefaultAffine.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultAffine.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    // Inverse conversions
    DefaultAffine.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    DefaultAffine.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    DefaultAffine.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultAffine.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    }

//==================================================================================
// ConstructiontTest3
//==================================================================================
TEST_F (HGF2DAffineTester, ConstructiontTest3)
    {

    // Create duplicates for compare operations
    HGF2DAffine Affine3(Affine2);
    HGF2DAffine Affine4(Affine2);
    Affine4.Reverse();

    // Check transformation properties
    ASSERT_FALSE(Affine2.IsIdentity());
    ASSERT_FALSE(Affine2.IsStretchable());
    ASSERT_TRUE(Affine2.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    Affine2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DAffine::CLASS_ID, Affine2.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Affine2.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = Affine2.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_FALSE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(Affine2.PreservesLinearity());
    ASSERT_TRUE(Affine2.PreservesParallelism());
    ASSERT_TRUE(Affine2.PreservesShape());
    ASSERT_FALSE(Affine2.PreservesDirection());

    // Reversal test
    Affine2.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(Affine2.IsIdentity());
    ASSERT_FALSE(Affine2.IsStretchable());
    ASSERT_TRUE(Affine2.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    Affine2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.5, ScalingX);
    ASSERT_DOUBLE_EQ(0.5, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-7.0710678118654755, Translation.GetDeltaX());
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Reversal test
    Affine2.Reverse();
    
    }

//==================================================================================
// CompositionIdentityTest3
//==================================================================================
TEST_F (HGF2DAffineTester, CompositionIdentityTest3)
    {

    // Composition with similitude
    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(Affine2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi->GetClassID());

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
TEST_F (HGF2DAffineTester, CompositionSimilitudeTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(Affine2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi2->PreservesParallelism());
    #ifdef WIP_IPPTEST_BUG_23
    ASSERT_TRUE(pIdenCompSimi2->PreservesShape());
    #endif
    ASSERT_FALSE(pIdenCompSimi2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTest3
//==================================================================================
TEST_F (HGF2DAffineTester, CompositionSimilitudeWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = Affine2.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi5->GetClassID());

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
TEST_F (HGF2DAffineTester, CompositionSimilitudeReversalWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = Affine2.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pIdenCompSimi6->GetClassID());

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
TEST_F (HGF2DAffineTester, TranslationExtractionTest3)
    {

    HGF2DDisplacement MyTranslation = Affine2.GetTranslation();

    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaY());

    // Scale extraction test
    ASSERT_DOUBLE_EQ(2.0, Affine2.GetXScaling());
    ASSERT_DOUBLE_EQ(2.0, Affine2.GetYScaling());

    // Extraction of rotation
    ASSERT_DOUBLE_EQ(PI/4, Affine2.GetRotation());

    // Create a clone of default similitude
    HFCPtr<HGF2DAffine> pDupSimi = (HGF2DAffine*)Affine2.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(1.0, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(1.00000000000000000, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(0.78539816339744828, pDupSimi->GetRotation(), 10E-12);

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_NEAR(1.00000000000000000, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(0.78539816339744828, pDupSimi->GetRotation(), 10E-12);

    // Set scaling
    pDupSimi->SetXScaling(2.0);
    pDupSimi->SetYScaling(2.0);
    ASSERT_NEAR(1.00000000000000000, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(0.78539816339744828, pDupSimi->GetRotation(), 10E-12);

    // Add scaling
    pDupSimi->AddIsotropicScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_NEAR(2.00000000000000000, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(4.00000000000000000, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(4.00000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(4.00000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(0.78539816339744828, pDupSimi->GetRotation(), 10E-12);

    // Set rotation
    pDupSimi->SetRotation(PI / 3.0);

    ASSERT_NEAR(2.0000000000000000, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(1.0471975511965976, pDupSimi->GetRotation(), 10E-12);

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);
    ASSERT_NEAR(4.2426406871192848, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(1.4142135623730954, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(4.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(1.8325957145940459, pDupSimi->GetRotation(), 10E-12);

    // Addition of components with offset
    pDupSimi->AddIsotropicScaling(2.5, 10.0, 10.0);
    ASSERT_NEAR(20.6066017177982130, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-36.464466094067262, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(1.83259571459404590, pDupSimi->GetRotation(), 10E-12);

    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);
    ASSERT_NEAR(-1.2132034355964212, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-44.497474683058329, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(2.61799387799149400, pDupSimi->GetRotation(), 10E-12);

    // Set Anorthogonality
    pDupSimi->SetAnorthogonality(PI / 4);

    // Check result of setting
    ASSERT_NEAR(-1.2132034355964212, pDupSimi->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-44.497474683058329, pDupSimi->GetTranslation().GetDeltaY(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetXScaling(), 10E-12);
    ASSERT_NEAR(10.0000000000000000, pDupSimi->GetYScaling(), 10E-12);
    ASSERT_NEAR(2.61799387799149400, pDupSimi->GetRotation(), 10E-12);
    ASSERT_NEAR(0.78539816339744828, pDupSimi->GetAnorthogonality(), 10E-12);

    }

//==================================================================================
// ConversionTest3
//==================================================================================
TEST_F (HGF2DAffineTester, ConversionTest3)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    Affine2.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.000000000000000, XCoord);
    ASSERT_DOUBLE_EQ(38.284271247461902, YCoord);

    double XConverted;
    double YConverted;
    XCoord = 10.0;
    YCoord = 10.0;

    Affine2.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.000000000000000, XConverted);
    ASSERT_DOUBLE_EQ(38.284271247461902, YConverted);

    double XArray[10];
    double YArray[10];
    Affine2.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray); 
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

    Affine2.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.000000000000000, XResultDist);
    ASSERT_DOUBLE_EQ(38.284271247461902, YResultDist);

    // Inverse conversions
    XCoord = 10.0;
    YCoord = 10.0;
    Affine2.ConvertInverse(&XCoord, &YCoord);
    ASSERT_NEAR(0.0, XCoord, MYEPSILON);
    ASSERT_NEAR(0.0, YCoord, MYEPSILON);

    XCoord = 10.0;
    YCoord = 10.0;

    Affine2.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(0.0, XConverted, MYEPSILON);
    ASSERT_NEAR(0.0, YConverted, MYEPSILON);

    Affine2.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    Affine2.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(0.0, XResultDist, MYEPSILON);
    ASSERT_NEAR(0.0, YResultDist, MYEPSILON);

    }

//==================================================================================
// ReversalSimilitudeWithRotationTest
//==================================================================================
TEST_F (HGF2DAffineTester, ReversalSimilitudeWithRotationTest)
    {

    HGF2DAffine Affine1;
    Affine1.SetRotation(PI / 4);
    Affine1.Reverse();

    ASSERT_DOUBLE_EQ(1.000000000000000000, Affine1.GetXScaling());
    ASSERT_DOUBLE_EQ(1.000000000000000000, Affine1.GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Affine1.GetRotation());
    ASSERT_NEAR(0.0, Affine1.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Affine1.GetTranslation().GetDeltaY(), MYEPSILON);

    // Affine with translation only
    HGF2DAffine Affine2;
    Affine2.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Affine2.Reverse();

    ASSERT_DOUBLE_EQ(1.00, Affine2.GetXScaling());
    ASSERT_DOUBLE_EQ(1.00, Affine2.GetYScaling());
    ASSERT_NEAR(0.0, Affine2.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Affine2.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Affine2.GetTranslation().GetDeltaY());

    // Test with scaling only
    HGF2DAffine Affine3;
    Affine3.SetXScaling(0.25);
    Affine3.SetYScaling(0.25);
    Affine3.Reverse();

    ASSERT_DOUBLE_EQ(4.0, Affine3.GetXScaling());
    ASSERT_DOUBLE_EQ(4.0, Affine3.GetYScaling());
    ASSERT_NEAR(0.0, Affine3.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, Affine3.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Affine3.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both rotation and translation
    HGF2DAffine Affine4;
    Affine4.SetRotation(PI / 4);
    Affine4.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Affine4.Reverse();

    ASSERT_DOUBLE_EQ(1.000000000000000000, Affine4.GetXScaling());
    ASSERT_DOUBLE_EQ(1.000000000000000000, Affine4.GetYScaling());
    ASSERT_DOUBLE_EQ(14.14213562373095100, Affine4.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Affine4.GetRotation());
    ASSERT_NEAR(0.0, Affine4.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both scaling and translation
    HGF2DAffine Affine5;
    Affine5.SetXScaling(2.1);
    Affine5.SetYScaling(2.1);
    Affine5.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Affine5.Reverse();

    ASSERT_DOUBLE_EQ(0.47619047619047622, Affine5.GetXScaling());
    ASSERT_DOUBLE_EQ(0.47619047619047622, Affine5.GetYScaling());
    ASSERT_NEAR(0.0, Affine5.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(4.7619047619047619, Affine5.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.7619047619047619, Affine5.GetTranslation().GetDeltaY());

    // Test with both scaling and rotation
    HGF2DAffine Affine6;
    Affine6.SetXScaling(2.1);
    Affine6.SetYScaling(2.1);
    Affine6.SetRotation(PI / 4);
    Affine6.Reverse();

    ASSERT_DOUBLE_EQ(0.476190476190476160, Affine6.GetXScaling());
    ASSERT_DOUBLE_EQ(0.476190476190476160, Affine6.GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Affine6.GetRotation());
    ASSERT_NEAR(0.0, Affine6.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Affine6.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both scaling and rotation and translation
    HGF2DAffine Affine7;
    Affine7.SetXScaling(2.1);
    Affine7.SetYScaling(2.1);
    Affine7.SetRotation(PI / 4);
    Affine7.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Affine7.Reverse();

    ASSERT_DOUBLE_EQ(0.476190476190476160, Affine7.GetXScaling());
    ASSERT_DOUBLE_EQ(0.476190476190476160, Affine7.GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Affine7.GetRotation());
    ASSERT_DOUBLE_EQ(6.734350297014738200, Affine7.GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, Affine7.GetTranslation().GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// SpecialCompositionTest
//==================================================================================
TEST_F (HGF2DAffineTester, SpecialCompositionTest)
    {
 
    HGF2DAffine Affine1;
    Affine1.SetXScaling(2.1);
    Affine1.SetYScaling(2.1);
    Affine1.SetRotation(PI / 4);
    Affine1.SetTranslation(HGF2DDisplacement(-10.0,-10.0));

    HGF2DAffine Affine2;
    Affine2.SetXScaling(-632.1);
    Affine2.SetYScaling(-632.1);
    Affine2.SetRotation(PI / 3);
    Affine2.SetTranslation(HGF2DDisplacement(1560.0,-1045.0));
    HFCPtr<HGF2DTransfoModel> pNewModel1and2 = Affine1.ComposeInverseWithDirectOf(Affine2);

    HFCPtr<HGF2DAffine> pAffiRes = (HGF2DAffine*)pNewModel1and2->Clone();

    ASSERT_NEAR(1327.41000000000000, pAffiRes->GetXScaling(), MYEPSILON);
    ASSERT_NEAR(1327.41000000000000, pAffiRes->GetYScaling(), MYEPSILON);
    ASSERT_NEAR(-753.64657732143496, pAffiRes->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(7589.64657732143680, pAffiRes->GetTranslation().GetDeltaY(), MYEPSILON);
    ASSERT_NEAR(-1.3089969389957474, pAffiRes->GetRotation(), MYEPSILON);

    HFCMatrix<3, 3> MyMatrix1 = pAffiRes->GetMatrix();
    ASSERT_NEAR(-753.64657732143496, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(7589.64657732143680, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(343.558988659536850, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(1282.17960107437240, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(-1282.1796010743724, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(343.558988659536850, MyMatrix1[1][1], MYEPSILON);

    HFCPtr<HGF2DTransfoModel> pNewModel1andI2 = Affine1.ComposeInverseWithInverseOf(Affine2);
    HFCPtr<HGF2DAffine> pAffiRes2 = (HGF2DAffine*)pNewModel1andI2->Clone();

    ASSERT_DOUBLE_EQ(-0.17613715063580726, pAffiRes2->GetTranslation().GetDeltaX()); 
    ASSERT_DOUBLE_EQ(-2.96971979740795570, pAffiRes2->GetTranslation().GetDeltaY());

    MyMatrix1 = pAffiRes2->GetMatrix();
    ASSERT_NEAR(-0.17613715063580726000, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(-2.96971979740795570000, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(-0.00320905590129258630, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(-0.00085986393721767661, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.000859863937217676610, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(-0.00320905590129258630, MyMatrix1[1][1], MYEPSILON);

    HFCPtr<HGF2DTransfoModel> pNewModel2and1 = Affine2.ComposeInverseWithDirectOf(Affine1);
    HFCPtr<HGF2DAffine> pAffiRes3 = (HGF2DAffine*)pNewModel2and1->Clone();

    ASSERT_DOUBLE_EQ(3858.2276464810084, pAffiRes3->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(754.73598385325113, pAffiRes3->GetTranslation().GetDeltaY());

    MyMatrix1 = pAffiRes3->GetMatrix();
    ASSERT_NEAR(3858.22764648100840, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(754.735983853251130, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(343.558988659536850, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(1282.17960107437240, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(-1282.1796010743724, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(343.558988659536850, MyMatrix1[1][1], MYEPSILON);

    HFCPtr<HGF2DTransfoModel> pNewModel2andI1 = Affine2.ComposeInverseWithInverseOf(Affine1);
    HFCPtr<HGF2DAffine> pAffiRes4 = (HGF2DAffine*)pNewModel2andI1->Clone();

    ASSERT_NEAR(-301.00000000000000, pAffiRes4->GetXScaling(), MYEPSILON);
    ASSERT_NEAR(-301.00000000000000, pAffiRes4->GetYScaling(), MYEPSILON);
    ASSERT_NEAR(0.26179938779914935, pAffiRes4->GetRotation(), MYEPSILON);
    ASSERT_NEAR(180.143870445144360, pAffiRes4->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(-877.14912618616961, pAffiRes4->GetTranslation().GetDeltaY(), MYEPSILON);

    MyMatrix1 = pAffiRes4->GetMatrix();
    ASSERT_NEAR(180.143870445144360, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(-877.14912618616972, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(-290.74367371300951, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(77.9045325758587100, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(-77.904532575858710, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(-290.74367371300951, MyMatrix1[1][1], MYEPSILON);

    // Composition with translation
    HGF2DTranslation TransModel;

    TransModel.SetTranslation(HGF2DDisplacement(10.0,11.0));
    HFCPtr<HGF2DTransfoModel> pNewModel1andTrans = Affine1.ComposeInverseWithDirectOf(TransModel);

    HFCPtr<HGF2DAffine> pAffiRes5 = (HGF2DAffine*)pNewModel1andTrans->Clone();

    ASSERT_DOUBLE_EQ(2.1, pAffiRes5->GetXScaling());
    ASSERT_DOUBLE_EQ(2.1, pAffiRes5->GetYScaling());
    ASSERT_NEAR(0.0, pAffiRes5->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pAffiRes5->GetTranslation().GetDeltaY());

    MyMatrix1 = pAffiRes5->GetMatrix();
    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(1.00000000000000000, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(-1.4849242404917500, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][1], MYEPSILON);

    HFCPtr<HGF2DTransfoModel> pNewModel1andITrans = Affine1.ComposeInverseWithInverseOf(TransModel);
    HFCPtr<HGF2DAffine> pAffiRes6 = (HGF2DAffine*)pNewModel1andITrans->Clone();

    ASSERT_NEAR(2.10000000000000000, pAffiRes6->GetXScaling(), MYEPSILON);
    ASSERT_NEAR(2.10000000000000000, pAffiRes6->GetYScaling(), MYEPSILON);
    ASSERT_NEAR(0.78539816339744828, pAffiRes6->GetRotation(), MYEPSILON);
    ASSERT_NEAR(-20.000000000000000, pAffiRes6->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(-21.000000000000000, pAffiRes6->GetTranslation().GetDeltaY(), MYEPSILON);

    MyMatrix1 = pAffiRes6->GetMatrix();
    ASSERT_NEAR(-20.000000000000000, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(-21.000000000000000, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(-1.4849242404917500, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][1], MYEPSILON);

    HFCPtr<HGF2DTransfoModel> pNewModelTransand1 = TransModel.ComposeInverseWithDirectOf(Affine1);
    HFCPtr<HGF2DAffine> pAffiRes7 = (HGF2DAffine*)pNewModelTransand1->Clone();

    ASSERT_NEAR(2.10000000000000000, pAffiRes7->GetXScaling(), MYEPSILON);
    ASSERT_NEAR(2.10000000000000000, pAffiRes7->GetYScaling(), MYEPSILON);
    ASSERT_NEAR(0.78539816339744828, pAffiRes7->GetRotation(), MYEPSILON);
    ASSERT_NEAR(-11.484924240491749, pAffiRes7->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(21.1834090503267480, pAffiRes7->GetTranslation().GetDeltaY(), MYEPSILON);

    MyMatrix1 = pAffiRes7->GetMatrix();
    ASSERT_NEAR(-11.484924240491749, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(21.1834090503267480, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(-1.4849242404917500, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(1.48492424049175000, MyMatrix1[1][1], MYEPSILON);

    HFCPtr<HGF2DTransfoModel> pNewModelTransandI1 = TransModel.ComposeInverseWithInverseOf(Affine1);
    HFCPtr<HGF2DAffine> pAffiRes8 = (HGF2DAffine*)pNewModelTransandI1->Clone();

    ASSERT_NEAR(0.476190476190476160, pAffiRes8->GetXScaling(), MYEPSILON);
    ASSERT_NEAR(0.476190476190476160, pAffiRes8->GetYScaling(), MYEPSILON);
    ASSERT_NEAR(-0.78539816339744828, pAffiRes8->GetRotation(), MYEPSILON);
    ASSERT_NEAR(13.80541810888021500, pAffiRes8->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.336717514850738590, pAffiRes8->GetTranslation().GetDeltaY(), MYEPSILON);

    MyMatrix1 = pAffiRes8->GetMatrix();
    ASSERT_NEAR(13.80541810888021500, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(0.336717514850738590, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(0.336717514850736920, MyMatrix1[0][0], MYEPSILON);
    ASSERT_NEAR(0.336717514850736870, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(-0.33671751485073687, MyMatrix1[1][0], MYEPSILON);
    ASSERT_NEAR(0.336717514850736920, MyMatrix1[1][1], MYEPSILON);

    }

//==================================================================================
// Test which failed Sep 8 1998
//==================================================================================
TEST_F (HGF2DAffineTester, TestWhoFailed)
    {

    double TheAngle(PI / 2.0);
    double TransX(36121.585200100);
    double TransY(5237.3471000000);
    HGF2DDisplacement MyDisp(TransX, TransY);
    double Anortho(0.0);

    HGF2DAffine TheAffine(MyDisp, TheAngle, 0.61298245614035, -1.6313680595306, Anortho);
    TheAffine.Reverse();
    HGF2DAffine Affi2(TheAffine);
    TheAffine.Reverse();
    HFCPtr<HGF2DTransfoModel> pTheResult = TheAffine.ComposeInverseWithDirectOf(Affi2);

    ASSERT_DOUBLE_EQ(0.6129824561403500, TheAffine.GetXScaling());
    ASSERT_DOUBLE_EQ(-1.631368059530600, TheAffine.GetYScaling());
    ASSERT_DOUBLE_EQ(1.5707963267948966, TheAffine.GetRotation());

    }

//==================================================================================
// ComposeInverseWithDirectOfWithPointer
//==================================================================================
TEST_F (HGF2DAffineTester, ComposeInverseWithDirectOfWithPointer)
    {

    HGF2DAffine MyAffine;

    HGF2DAffine MyOtherAffine(HGF2DDisplacement(0.0, 0.0), PI/4, 1.0, 1.0, 0.0);

    HFCPtr<HGF2DTransfoModel> pM1 = MyOtherAffine.ComposeInverseWithDirectOf(MyAffine);
    HFCPtr<HGF2DTransfoModel> pM2 = MyOtherAffine.ComposeInverseWithDirectOf(*pM1);
    HFCPtr<HGF2DTransfoModel> pM3 = MyOtherAffine.ComposeInverseWithDirectOf(*pM2);
    HFCPtr<HGF2DTransfoModel> pM4 = MyOtherAffine.ComposeInverseWithDirectOf(*pM3);
    HFCPtr<HGF2DTransfoModel> pM5 = MyOtherAffine.ComposeInverseWithDirectOf(*pM4);
    HFCPtr<HGF2DTransfoModel> pM6 = MyOtherAffine.ComposeInverseWithDirectOf(*pM5);

    }

TEST_F (HGF2DAffineTester, ConvertUsingInOutArraysDefaultAffine)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for (uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    DefaultAffine.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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

    DefaultAffine.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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

TEST_F (HGF2DAffineTester, ConvertUsingInOutArraysAffine2)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for (uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    Affine2.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut); 
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

    // Reset values in the in/out array
    for (uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }
    Affine2.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
