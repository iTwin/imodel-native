//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DStretchTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DStretchTester.h"

HGF2DStretchTester::HGF2DStretchTester() 
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

    //Stretch
    Stretch2.SetTranslation(HGF2DDisplacement(10.0, 10.0));
    Stretch2.SetXScaling(2.0);
    Stretch2.SetYScaling(2.0);

    }

//==================================================================================
// ConstructiontTest
//==================================================================================
TEST_F(HGF2DStretchTester, DefaultConstructiontTest)
    {

    HGF2DStretch DefaultStretch;

    // Check transformation properties
    ASSERT_TRUE(DefaultStretch.IsIdentity());
    ASSERT_TRUE(DefaultStretch.IsStretchable());
    ASSERT_TRUE(DefaultStretch.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    DefaultStretch.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DStretch::CLASS_ID, DefaultStretch.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = DefaultStretch.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = DefaultStretch.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_TRUE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(DefaultStretch.PreservesLinearity());
    ASSERT_TRUE(DefaultStretch.PreservesParallelism());
    ASSERT_TRUE(DefaultStretch.PreservesShape());
    ASSERT_TRUE(DefaultStretch.PreservesDirection());

    // Reversal test
    DefaultStretch.Reverse();

    // Check again transformation properties
    ASSERT_TRUE(DefaultStretch.IsIdentity());
    ASSERT_TRUE(DefaultStretch.IsStretchable());
    ASSERT_TRUE(DefaultStretch.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    DefaultStretch.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// HGF2DStretch(const HGF2DDisplacement& pi_rTranslation, double pi_ScalingX, double pi_ScalingY);
//==================================================================================
TEST_F (HGF2DStretchTester, SimpleTransformationConstructionTest)
    {

    HGF2DStretch     StretchTranslation(HGF2DDisplacement(10.0,10.0), 1.0, 1.0);
    HGF2DStretch     StretchScaling(HGF2DDisplacement(0.0,0.0), 10.0, 10.0);

    // Check transformation properties
    ASSERT_FALSE(StretchTranslation.IsIdentity());
    ASSERT_TRUE(StretchTranslation.IsStretchable());
    ASSERT_TRUE(StretchTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(StretchScaling.IsIdentity());
    ASSERT_TRUE(StretchScaling.IsStretchable());
    ASSERT_TRUE(StretchScaling.CanBeRepresentedByAMatrix());

    // Extract Translation stretch parameters
    StretchTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract Scaling stretch parameters
    StretchScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(10.0, ScalingX);
    ASSERT_DOUBLE_EQ(10.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DStretch::CLASS_ID, StretchTranslation.GetClassID());
    ASSERT_EQ(HGF2DStretch::CLASS_ID, StretchScaling.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = StretchTranslation.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix3 = StretchScaling.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix3[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[0][0]);
    ASSERT_NEAR(0.0, MyMatrix3[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = StretchTranslation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone3 = StretchScaling.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pModelClone->GetClassID());
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pModelClone3->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    ASSERT_FALSE(pModelClone3->IsIdentity());
    ASSERT_TRUE(pModelClone3->IsStretchable());
    ASSERT_TRUE(pModelClone3->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(StretchTranslation.PreservesLinearity());
    ASSERT_TRUE(StretchTranslation.PreservesParallelism());
    ASSERT_TRUE(StretchTranslation.PreservesShape());
    ASSERT_TRUE(StretchTranslation.PreservesDirection());

    ASSERT_TRUE(StretchScaling.PreservesLinearity());
    ASSERT_TRUE(StretchScaling.PreservesParallelism());
    ASSERT_TRUE(StretchScaling.PreservesShape());
    ASSERT_TRUE(StretchScaling.PreservesDirection());

    // Reversal test
    StretchTranslation.Reverse();
    StretchScaling.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(StretchTranslation.IsIdentity());
    ASSERT_TRUE(StretchTranslation.IsStretchable());
    ASSERT_TRUE(StretchTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(StretchScaling.IsIdentity());
    ASSERT_TRUE(StretchScaling.IsStretchable());
    ASSERT_TRUE(StretchScaling.CanBeRepresentedByAMatrix());

    // Extract Translation again stretch parameters
    StretchTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    // Extract Scaling again stretch parameters
    StretchScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.1, ScalingX);
    ASSERT_DOUBLE_EQ(0.1, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// operator=(const HGF2DAffine& pi_rObj);
// HGF2DStretch(const HGF2DAffine& pi_rObj);
//==================================================================================
TEST_F (HGF2DStretchTester, CopyTest)
    {

    HGF2DStretch StretchOperator = Stretch2;
    HGF2DStretch StretchCopy(Stretch2);

    // Check transformation properties
    ASSERT_FALSE(StretchOperator.IsIdentity());
    ASSERT_TRUE(StretchOperator.IsStretchable());
    ASSERT_TRUE(StretchOperator.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(StretchCopy.IsIdentity());
    ASSERT_TRUE(StretchCopy.IsStretchable());
    ASSERT_TRUE(StretchCopy.CanBeRepresentedByAMatrix());

    // Extract stretch parameters of AffineOperator
    StretchOperator.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract stretch parameters of AffineOperator
    StretchCopy.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DStretch::CLASS_ID, StretchOperator.GetClassID());
    ASSERT_EQ(HGF2DStretch::CLASS_ID, StretchCopy.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = StretchOperator.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(2.00, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = StretchCopy.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix2[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix2[1][2]);
    ASSERT_DOUBLE_EQ(2.00, MyMatrix2[0][0]);
    ASSERT_NEAR(0.0, MyMatrix2[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix2[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, MyMatrix2[1][1]);

    }

//==================================================================================
// CreateSimplifiedModel() const;
//==================================================================================
TEST_F(HGF2DStretchTester, CreateSimplifiedModelTest)
    {
    
    //Simplified to HGF2DIdentity
    HGF2DStretch     Stretch1(HGF2DDisplacement(0.0, 0.0), 1.0, 1.0);
                                     
    HFCPtr<HGF2DTransfoModel> newModel1 = Stretch1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, newModel1->GetClassID());

    //Simplified to HGF2DTranslation
    HGF2DStretch     Stretch2(HGF2DDisplacement(10.0, 0.0), 1.0, 1.0);
    
    HFCPtr<HGF2DTransfoModel> newModel2 = Stretch2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, newModel2->GetClassID());

    }

//==================================================================================
// CompositionIdentityTest
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionIdentityTest)
    {
      
    HFCPtr<HGF2DTransfoModel> pIdenCompStretch = MyIdentity.ComposeInverseWithDirectOf(DefaultStretch);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompStretch->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeTest
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionSimilitudeTest)
    {
     
    HFCPtr<HGF2DTransfoModel> pIdenCompStretch2 = MyIdentity.ComposeInverseWithInverseOf(DefaultStretch);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompStretch2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch2->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTest
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionSimilitudeWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompStretch5 = DefaultStretch.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompStretch5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch5->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch5->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeReversalWithIdentityTest
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionSimilitudeReversalWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompStretch6 = DefaultStretch.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch6->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompStretch6->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch6->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch6->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch6->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch6->PreservesDirection());

    }

//==================================================================================
// TranslationExtractionTest
//==================================================================================
TEST_F(HGF2DStretchTester, TranslationExtractionTest)
    {   
        
    // Translation extraction test
    HGF2DDisplacement MyTranslation = DefaultStretch.GetTranslation();

    ASSERT_NEAR(0.0, MyTranslation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, MyTranslation.GetDeltaY(), MYEPSILON);

    // Scale extraction test
    ASSERT_DOUBLE_EQ(1.0, DefaultStretch.GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, DefaultStretch.GetYScaling());

    // Create a clone of default similitude
    HFCPtr<HGF2DStretch> pDupStretch = (HGF2DStretch*)DefaultStretch.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupStretch->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupStretch->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, pDupStretch->GetYScaling());

    // Add translation
    pDupStretch->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(0.3408, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(1.0000, pDupStretch->GetYScaling());

    // Set scaling
    pDupStretch->SetXScaling(2.0);
    pDupStretch->SetYScaling(2.0);
    ASSERT_DOUBLE_EQ(0.3408, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetYScaling());

    // Add scaling
    pDupStretch->AddIsotropicScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(0.6816, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.0000, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(4.0000, pDupStretch->GetYScaling());

    // Addition of components with offset
    pDupStretch->AddIsotropicScaling(2.5, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(-13.295999999999999, pDupStretch->GetTranslation().GetDeltaX()); 
    ASSERT_DOUBLE_EQ(-5.0000000000000000, pDupStretch->GetTranslation().GetDeltaY()); 
    ASSERT_DOUBLE_EQ(10.0000000000000000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(10.0000000000000000, pDupStretch->GetYScaling());

    // Create a clone of default Stretch
    pDupStretch = (HGF2DStretch*)DefaultStretch.Clone();

    //Add scaling
    pDupStretch->AddAnisotropicScaling(2.0, 3.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_NEAR(0.0, pDupStretch->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupStretch->GetTranslation().GetDeltaY(), MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, pDupStretch->GetYScaling());

    // Addition of components with offset
    pDupStretch->AddAnisotropicScaling(2.0, 3.0, 10.0, 10.0);
   
    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(-10.0, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-20.0, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(9.000, pDupStretch->GetYScaling());

    }

//==================================================================================
// ConversionTest
//==================================================================================
TEST_F(HGF2DStretchTester, ConversionTest)
    {

    // Direct conversions
    double XCoord = 10.0;
    double YCoord = 10.0;

    DefaultStretch.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    double XConverted;
    double YConverted;

    DefaultStretch.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    double XArray[10];
    double YArray[10];
    DefaultStretch.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultStretch.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    // Inverse conversions
    DefaultStretch.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    DefaultStretch.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    DefaultStretch.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultStretch.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    }

TEST_F(HGF2DStretchTester, ConversionInOutArraysTest)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];

    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    DefaultStretch.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
    DefaultStretch.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
TEST_F(HGF2DStretchTester, ConstructiontTest3)
    {

    // Create duplicates for compare operations
    HGF2DStretch Stretch3(Stretch2);
    HGF2DStretch Stretch4(Stretch2);
    Stretch4.Reverse();

    // Check transformation properties
    ASSERT_FALSE(Stretch2.IsIdentity());
    ASSERT_TRUE(Stretch2.IsStretchable());
    ASSERT_TRUE(Stretch2.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    Stretch2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DStretch::CLASS_ID, Stretch2.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Stretch2.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]); 
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(2.00, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(2.0, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = Stretch2.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(Stretch2.PreservesLinearity());
    ASSERT_TRUE(Stretch2.PreservesParallelism());
    ASSERT_TRUE(Stretch2.PreservesShape());
    ASSERT_TRUE(Stretch2.PreservesDirection());

    // Reversal test
    Stretch2.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(Stretch2.IsIdentity());
    ASSERT_TRUE(Stretch2.IsStretchable());
    ASSERT_TRUE(Stretch2.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    Stretch2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.5, ScalingX);
    ASSERT_DOUBLE_EQ(0.5, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-5.0, Translation.GetDeltaX()); 
    ASSERT_DOUBLE_EQ(-5.0, Translation.GetDeltaY());

    Stretch2.Reverse();

    }

//==================================================================================
// CompositionIdentityTest3
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionIdentityTest3)
    {

    // Composition with similitude
    HFCPtr<HGF2DTransfoModel> pIdenCompStretch = MyIdentity.ComposeInverseWithDirectOf(Stretch2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompStretch->IsIdentity()); 

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeTest3
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionSimilitudeTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompStretch2 = MyIdentity.ComposeInverseWithInverseOf(Stretch2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompStretch2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch2->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTest3
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionSimilitudeWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompStretch5 = Stretch2.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompStretch5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch5->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch5->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeReversalWithIdentityTest3
//==================================================================================
TEST_F(HGF2DStretchTester, CompositionSimilitudeReversalWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompStretch6 = Stretch2.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pIdenCompStretch6->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompStretch6->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompStretch6->PreservesLinearity());
    ASSERT_TRUE(pIdenCompStretch6->PreservesParallelism());
    ASSERT_TRUE(pIdenCompStretch6->PreservesShape());
    ASSERT_TRUE(pIdenCompStretch6->PreservesDirection());

    }

//==================================================================================
// TranslationExtractionTest3
//==================================================================================
TEST_F(HGF2DStretchTester, TranslationExtractionTest3)
    {

    HGF2DDisplacement MyTranslation = Stretch2.GetTranslation();

    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaY());

    // Scale extraction test
    ASSERT_DOUBLE_EQ(2.0, Stretch2.GetXScaling());
    ASSERT_DOUBLE_EQ(2.0, Stretch2.GetYScaling());

    // Create a clone of default similitude
    HFCPtr<HGF2DStretch> pDupStretch = (HGF2DStretch*)Stretch2.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupStretch->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupStretch->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(2.0, pDupStretch->GetYScaling());

    // Add translation
    pDupStretch->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(0.3408, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetYScaling());

    // Set scaling
    pDupStretch->SetXScaling(2.0);
    pDupStretch->SetYScaling(2.0);
    ASSERT_DOUBLE_EQ(0.3408, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(2.0000, pDupStretch->GetYScaling());

    // Add scaling
    pDupStretch->AddIsotropicScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(0.6816, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(4.0000, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(4.0000, pDupStretch->GetYScaling());

    // Addition of components with offset
    pDupStretch->AddIsotropicScaling(2.5, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(-13.295999999999999, pDupStretch->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-5.0000000000000000, pDupStretch->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(10.0000000000000000, pDupStretch->GetXScaling());
    ASSERT_DOUBLE_EQ(10.0000000000000000, pDupStretch->GetYScaling());

    }

//==================================================================================
// ConversionTest3
//==================================================================================
TEST_F(HGF2DStretchTester, ConversionTest3)
    {

    // Direct conversions
    double XCoord = 10.0;
    double YCoord = 10.0;

    Stretch2.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(30.0, XCoord); 
    ASSERT_DOUBLE_EQ(30.0, YCoord);

    double XConverted;
    double YConverted;
    XCoord = 10.0;
    YCoord = 10.0;

    Stretch2.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(30.0, XConverted); 
    ASSERT_DOUBLE_EQ(30.0, YConverted);

    double XArray[10];
    double YArray[10];
    Stretch2.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_DOUBLE_EQ(10.0, XArray[0]);
    ASSERT_DOUBLE_EQ(12.0, XArray[1]);
    ASSERT_DOUBLE_EQ(14.0, XArray[2]);
    ASSERT_DOUBLE_EQ(16.0, XArray[3]);
    ASSERT_DOUBLE_EQ(18.0, XArray[4]);
    ASSERT_DOUBLE_EQ(20.0, XArray[5]);
    ASSERT_DOUBLE_EQ(22.0, XArray[6]);
    ASSERT_DOUBLE_EQ(24.0, XArray[7]);
    ASSERT_DOUBLE_EQ(26.0, XArray[8]);
    ASSERT_DOUBLE_EQ(28.0, XArray[9]);

    ASSERT_DOUBLE_EQ(30.0, YArray[0]);
    ASSERT_DOUBLE_EQ(30.0, YArray[1]);
    ASSERT_DOUBLE_EQ(30.0, YArray[2]);
    ASSERT_DOUBLE_EQ(30.0, YArray[3]);
    ASSERT_DOUBLE_EQ(30.0, YArray[4]);
    ASSERT_DOUBLE_EQ(30.0, YArray[5]);
    ASSERT_DOUBLE_EQ(30.0, YArray[6]);
    ASSERT_DOUBLE_EQ(30.0, YArray[7]);
    ASSERT_DOUBLE_EQ(30.0, YArray[8]);
    ASSERT_DOUBLE_EQ(30.0, YArray[9]);

    double XDist(10.0);
    double YDist(10.0);

    double XResultDist;
    double YResultDist;

    Stretch2.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(30.0, XResultDist);
    ASSERT_DOUBLE_EQ(30.0, YResultDist);

    // Inverse conversions
    XCoord = 10.0;
    YCoord = 10.0;
    Stretch2.ConvertInverse(&XCoord, &YCoord);
    ASSERT_NEAR(0.0, XCoord, MYEPSILON);
    ASSERT_NEAR(0.0, YCoord, MYEPSILON);

    XCoord = 10.0;
    YCoord = 10.0;

    Stretch2.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(0.0, XConverted, MYEPSILON);
    ASSERT_NEAR(0.0, YConverted, MYEPSILON);

    Stretch2.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_DOUBLE_EQ(-5.0, XArray[0]);
    ASSERT_DOUBLE_EQ(-4.5, XArray[1]);
    ASSERT_DOUBLE_EQ(-4.0, XArray[2]);
    ASSERT_DOUBLE_EQ(-3.5, XArray[3]);
    ASSERT_DOUBLE_EQ(-3.0, XArray[4]);
    ASSERT_DOUBLE_EQ(-2.5, XArray[5]);
    ASSERT_DOUBLE_EQ(-2.0, XArray[6]);
    ASSERT_DOUBLE_EQ(-1.5, XArray[7]);
    ASSERT_DOUBLE_EQ(-1.0, XArray[8]);
    ASSERT_DOUBLE_EQ(-0.5, XArray[9]);

    ASSERT_NEAR(0.0, YArray[0], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[1], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[2], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[3], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[4], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[5], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[6], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[7], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[8], MYEPSILON);
    ASSERT_NEAR(0.0, YArray[9], MYEPSILON);

    Stretch2.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(0.0, XResultDist, MYEPSILON);
    ASSERT_NEAR(0.0, YResultDist, MYEPSILON);

    }

TEST_F(HGF2DStretchTester, ConversionInOutArraysTest3)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];

    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    Stretch2.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_DOUBLE_EQ(10.0, XArrayInOut[0]);
    ASSERT_DOUBLE_EQ(12.0, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(14.0, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(16.0, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(18.0, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(20.0, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(22.0, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(24.0, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(26.0, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(28.0, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(30.0, YArrayInOut[9]);

    // Reset the in/out arrays before testing ConvertInverse
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }
    Stretch2.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_DOUBLE_EQ(-5.0, XArrayInOut[0]);
    ASSERT_DOUBLE_EQ(-4.5, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(-4.0, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(-3.5, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(-3.0, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(-2.5, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(-2.0, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(-1.5, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(-1.0, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(-0.5, XArrayInOut[9]);

    ASSERT_NEAR(0.0, YArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(0.0, YArrayInOut[9], MYEPSILON);
    }

//==================================================================================
// SpecialReversalTest
//==================================================================================
TEST_F(HGF2DStretchTester, SpecialReversalTest)
    {

    // Reversal of a similitude with rotation
    HGF2DStretch Stretch1;
    Stretch1.Reverse();

    ASSERT_DOUBLE_EQ(1.0, Stretch1.GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, Stretch1.GetYScaling());
    ASSERT_NEAR(0.0, Stretch1.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Stretch1.GetTranslation().GetDeltaY(), MYEPSILON);

    // Stretch with translation only
    HGF2DStretch Stretch2;
    Stretch2.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Stretch2.Reverse();

    ASSERT_DOUBLE_EQ(1.00, Stretch2.GetXScaling());
    ASSERT_DOUBLE_EQ(1.00, Stretch2.GetYScaling());
    ASSERT_DOUBLE_EQ(10.0, Stretch2.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Stretch2.GetTranslation().GetDeltaY());

    // Test with scaling only
    HGF2DStretch Stretch3;
    Stretch3.SetXScaling(0.25);
    Stretch3.SetYScaling(0.25);
    Stretch3.Reverse();

    ASSERT_DOUBLE_EQ(4.0, Stretch3.GetXScaling());
    ASSERT_DOUBLE_EQ(4.0, Stretch3.GetYScaling());
    ASSERT_NEAR(0.0, Stretch3.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Stretch3.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both scaling and translation
    HGF2DStretch Stretch5;
    Stretch5.SetXScaling(2.1);
    Stretch5.SetYScaling(2.1);
    Stretch5.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Stretch5.Reverse();

    ASSERT_DOUBLE_EQ(1/2.1, Stretch5.GetXScaling());
    ASSERT_DOUBLE_EQ(1/2.1, Stretch5.GetYScaling());
    ASSERT_DOUBLE_EQ(10.0 / 2.1, Stretch5.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0 / 2.1, Stretch5.GetTranslation().GetDeltaY());

    }

//==================================================================================
// SpecialCompositionTest
//==================================================================================
TEST_F(HGF2DStretchTester, SpecialCompositionTest)
    {

    HGF2DStretch Stretch1;
    Stretch1.SetXScaling(2.1);
    Stretch1.SetYScaling(2.1);
    Stretch1.SetTranslation(HGF2DDisplacement(-10.0, -10.0));

    HGF2DStretch Stretch2;
    Stretch2.SetXScaling(-632.1);
    Stretch2.SetYScaling(-632.1);
    Stretch2.SetTranslation(HGF2DDisplacement(1560.0, -1045.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1and2 = Stretch1.ComposeInverseWithDirectOf(Stretch2);
    HFCPtr<HGF2DStretch> pAffiRes = (HGF2DStretch*)pNewModel1and2->Clone();

    HFCMatrix<3, 3> MyMatrix1 = pAffiRes->GetMatrix();
    ASSERT_DOUBLE_EQ(7881.00, MyMatrix1[0][2]); 
    ASSERT_DOUBLE_EQ(5276.00, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(-1327.41, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(-1327.41, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel1andI2 = Stretch1.ComposeInverseWithInverseOf(Stretch2);
    HFCPtr<HGF2DStretch> pAffiRes2 = (HGF2DStretch*)pNewModel1andI2->Clone();

    MyMatrix1 = pAffiRes2->GetMatrix();
    ASSERT_DOUBLE_EQ(2.48378421135896190000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(-1.6373991457047936000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(-0.0033222591362126247, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(-0.0033222591362126247, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel2and1 = Stretch2.ComposeInverseWithDirectOf(Stretch1);
    HFCPtr<HGF2DStretch> pAffiRes3 = (HGF2DStretch*)pNewModel2and1->Clone();

    MyMatrix1 = pAffiRes3->GetMatrix();
    ASSERT_DOUBLE_EQ(3266.00, MyMatrix1[0][2]); 
    ASSERT_DOUBLE_EQ(-2204.50, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(-1327.41, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(-1327.41, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel2andI1 = Stretch2.ComposeInverseWithInverseOf(Stretch1);
    HFCPtr<HGF2DStretch> pAffiRes4 = (HGF2DStretch*)pNewModel2andI1->Clone();

    MyMatrix1 = pAffiRes4->GetMatrix();
    ASSERT_DOUBLE_EQ(747.619047619047590, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(-492.85714285714283, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(-301.00000000000000, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(-301.00000000000000, MyMatrix1[1][1]);

    // Composition with translation
    HGF2DTranslation TransModel;

    TransModel.SetTranslation(HGF2DDisplacement(10.0, 11.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1andTrans = Stretch1.ComposeInverseWithDirectOf(TransModel);
    HFCPtr<HGF2DStretch> pAffiRes5 = (HGF2DStretch*)pNewModel1andTrans->Clone();

    MyMatrix1 = pAffiRes5->GetMatrix();
    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(2.1, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(2.1, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel1andITrans = Stretch1.ComposeInverseWithInverseOf(TransModel);
    HFCPtr<HGF2DStretch> pAffiRes6 = (HGF2DStretch*)pNewModel1andITrans->Clone();

    MyMatrix1 = pAffiRes6->GetMatrix();
    ASSERT_DOUBLE_EQ(-20.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(-21.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(2.100, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(2.1, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModelTransand1 = TransModel.ComposeInverseWithDirectOf(Stretch1);
    HFCPtr<HGF2DStretch> pAffiRes7 = (HGF2DStretch*)pNewModelTransand1->Clone();

    MyMatrix1 = pAffiRes7->GetMatrix();
    ASSERT_DOUBLE_EQ(11.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(13.1, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(2.10, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(2.10, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModelTransandI1 = TransModel.ComposeInverseWithInverseOf(Stretch1);
    HFCPtr<HGF2DStretch> pAffiRes8 = (HGF2DStretch*)pNewModelTransandI1->Clone();

    MyMatrix1 = pAffiRes8->GetMatrix();
    ASSERT_DOUBLE_EQ(9.52380952380952370, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(0.47619047619047616, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.47619047619047616, MyMatrix1[1][1]);
   
    }
