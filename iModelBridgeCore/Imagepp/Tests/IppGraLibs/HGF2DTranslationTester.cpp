//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DTranslationTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DTranslationTester.h"

HGF2DTranslationTester::HGF2DTranslationTester() 
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

    //Translation
    Translation2.SetTranslation(HGF2DDisplacement(10.0,10.0));
    DefaultTranslation.SetTranslation(HGF2DDisplacement(0.0, 0.0));

    }

//==================================================================================
// ConstructiontTest
//==================================================================================
TEST_F (HGF2DTranslationTester, DefaultConstructiontTest)
    {

    // Check transformation properties
    ASSERT_TRUE(DefaultTranslation.IsIdentity());
    ASSERT_TRUE(DefaultTranslation.IsStretchable());
    ASSERT_TRUE(DefaultTranslation.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    DefaultTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, DefaultTranslation.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = DefaultTranslation.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = DefaultTranslation.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_TRUE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(DefaultTranslation.PreservesLinearity());
    ASSERT_TRUE(DefaultTranslation.PreservesParallelism());
    ASSERT_TRUE(DefaultTranslation.PreservesShape());
    ASSERT_TRUE(DefaultTranslation.PreservesDirection());

    // Reversal test
    DefaultTranslation.Reverse();

    // Check again transformation properties
    ASSERT_TRUE(DefaultTranslation.IsIdentity());
    ASSERT_TRUE(DefaultTranslation.IsStretchable());
    ASSERT_TRUE(DefaultTranslation.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    DefaultTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// operator=(const HGF2DTranslation& pi_rObj);
// HGF2DTranslation(const HGF2DTranslation& pi_rObj);
//==================================================================================
TEST_F (HGF2DTranslationTester, CopyTest)
    {

    HGF2DTranslation TranslationOperator = Translation2;
    HGF2DTranslation TranslationCopy(Translation2);

    // Check transformation properties
    ASSERT_FALSE(TranslationOperator.IsIdentity());
    ASSERT_TRUE(TranslationOperator.IsStretchable());
    ASSERT_TRUE(TranslationOperator.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(TranslationCopy.IsIdentity());
    ASSERT_TRUE(TranslationCopy.IsStretchable());
    ASSERT_TRUE(TranslationCopy.CanBeRepresentedByAMatrix());

    // Extract stretch parameters of AffineOperator
    TranslationOperator.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract stretch parameters of AffineOperator
    TranslationCopy.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, TranslationOperator.GetClassID());
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, TranslationCopy.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = TranslationOperator.GetMatrix();
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = TranslationCopy.GetMatrix();
    ASSERT_DOUBLE_EQ(10.0, MyMatrix2[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix2[1][2]);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix2[0][0]);
    ASSERT_NEAR(0.0, MyMatrix2[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix2[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix2[1][1]);

    }

//==================================================================================
// CreateSimplifiedModel() const;
//==================================================================================
TEST_F(HGF2DTranslationTester, CreateSimplifiedModelTest)
    {
    
    //Simplified to HGF2DIdentity
    HGF2DTranslation     Translation1(HGF2DDisplacement(0.0, 0.0));
                                     
    HFCPtr<HGF2DTransfoModel> newModel1 = Translation1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, newModel1->GetClassID());

    }

//==================================================================================
// CompositionIdentityTest
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(DefaultTranslation);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi->PreservesDirection());

    }

//==================================================================================
// CompositionTranslationTest
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionTranslationTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(DefaultTranslation);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi2->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi2->PreservesDirection());

    }

//==================================================================================
// CompositionTranslationWithIdentityTest
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionTranslationWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = DefaultTranslation.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi5->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi5->PreservesDirection());

    }

//==================================================================================
// CompositionTranslationReversalWithIdentityTest
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionTranslationReversalWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = DefaultTranslation.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi6->GetClassID());

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
TEST_F (HGF2DTranslationTester, TranslationExtractionTest)
    {

    HGF2DDisplacement MyTranslation = DefaultTranslation.GetTranslation();

    ASSERT_NEAR(0.0, MyTranslation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, MyTranslation.GetDeltaY(), MYEPSILON);

    // Create a clone of default helmert
    HFCPtr<HGF2DTranslation> pDupSimi = (HGF2DTranslation*)DefaultTranslation.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaY());

    }

//==================================================================================
// ConversionTest
//==================================================================================
TEST_F (HGF2DTranslationTester, ConversionTest)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    DefaultTranslation.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    double XConverted;
    double YConverted;

    DefaultTranslation.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    double XArray[10];
    double YArray[10];
    DefaultTranslation.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultTranslation.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    // Inverse conversions
    DefaultTranslation.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    DefaultTranslation.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    DefaultTranslation.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultTranslation.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    }

TEST_F (HGF2DTranslationTester, ConversionInOutArraysTest)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];

    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    DefaultTranslation.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
    DefaultTranslation.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
TEST_F (HGF2DTranslationTester, ConstructiontTest3)
    {

    // Create duplicates for compare operations
    HGF2DTranslation Translation3(Translation2);
    HGF2DTranslation Translation4(Translation2);
    Translation4.Reverse();

    // Check transformation properties
    ASSERT_FALSE(Translation2.IsIdentity());
    ASSERT_TRUE(Translation2.IsStretchable());
    ASSERT_TRUE(Translation2.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    Translation2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, Translation2.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Translation2.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = Translation2.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(Translation2.PreservesLinearity());
    ASSERT_TRUE(Translation2.PreservesParallelism());
    ASSERT_TRUE(Translation2.PreservesShape());
    ASSERT_TRUE(Translation2.PreservesDirection());

    // Reversal test
    Translation2.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(Translation2.IsIdentity());
    ASSERT_TRUE(Translation2.IsStretchable());
    ASSERT_TRUE(Translation2.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    Translation2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    Translation2.Reverse();

    }

//==================================================================================
// CompositionIdentityTest3
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(Translation2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi->PreservesDirection());

    }

//==================================================================================
// CompositionTranslationTest3
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionTranslationTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(Translation2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi2->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi2->PreservesDirection());

    }

//==================================================================================
// CompositionTranslationWithIdentityTest3
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionTranslationWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = Translation2.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi5->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi5->PreservesDirection());

    }

//==================================================================================
// CompositionTranslationReversalWithIdentityTest3
//==================================================================================
TEST_F (HGF2DTranslationTester, CompositionTranslationReversalWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = Translation2.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pIdenCompSimi6->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi6->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi6->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi6->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi6->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi6->PreservesDirection());

    }

//==================================================================================
// TranslationExtractionTest3
//==================================================================================
TEST_F (HGF2DTranslationTester, TranslationExtractionTest3)
    {

    HGF2DDisplacement MyTranslation = Translation2.GetTranslation();

    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaY());

    // Create a clone of default similitude
    HFCPtr<HGF2DTranslation> pDupSimi = (HGF2DTranslation*)Translation2.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaY());

    }

//==================================================================================
// ConversionTest3
//==================================================================================
TEST_F (HGF2DTranslationTester, ConversionTest3)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    Translation2.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(20.0, XCoord);
    ASSERT_DOUBLE_EQ(20.0, YCoord);

    double XConverted;
    double YConverted;
    XCoord = 10.0;
    YCoord = 10.0;

    Translation2.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(20.0, XConverted);
    ASSERT_DOUBLE_EQ(20.0, YConverted);

    double XArray[10];
    double YArray[10];
    Translation2.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_DOUBLE_EQ(10.0, XArray[0]);
    ASSERT_DOUBLE_EQ(11.0, XArray[1]);
    ASSERT_DOUBLE_EQ(12.0, XArray[2]);
    ASSERT_DOUBLE_EQ(13.0, XArray[3]);
    ASSERT_DOUBLE_EQ(14.0, XArray[4]);
    ASSERT_DOUBLE_EQ(15.0, XArray[5]);
    ASSERT_DOUBLE_EQ(16.0, XArray[6]);
    ASSERT_DOUBLE_EQ(17.0, XArray[7]);
    ASSERT_DOUBLE_EQ(18.0, XArray[8]);
    ASSERT_DOUBLE_EQ(19.0, XArray[9]);

    ASSERT_DOUBLE_EQ(20.0, YArray[0]);
    ASSERT_DOUBLE_EQ(20.0, YArray[1]);
    ASSERT_DOUBLE_EQ(20.0, YArray[2]);
    ASSERT_DOUBLE_EQ(20.0, YArray[3]);
    ASSERT_DOUBLE_EQ(20.0, YArray[4]);
    ASSERT_DOUBLE_EQ(20.0, YArray[5]);
    ASSERT_DOUBLE_EQ(20.0, YArray[6]);
    ASSERT_DOUBLE_EQ(20.0, YArray[7]);
    ASSERT_DOUBLE_EQ(20.0, YArray[8]);
    ASSERT_DOUBLE_EQ(20.0, YArray[9]);

    double XDist(10.0);
    double YDist(10.0);

    double XResultDist;
    double YResultDist;

    Translation2.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(20.0, XResultDist);
    ASSERT_DOUBLE_EQ(20.0, YResultDist);

    // Inverse conversions
    XCoord = 10.0;
    YCoord = 10.0;
    Translation2.ConvertInverse(&XCoord, &YCoord);
    ASSERT_NEAR(0.0, XCoord, MYEPSILON);
    ASSERT_NEAR(0.0, YCoord, MYEPSILON);

    XCoord = 10.0;
    YCoord = 10.0;

    Translation2.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(0.0, XConverted, MYEPSILON);
    ASSERT_NEAR(0.0, YConverted, MYEPSILON);

    Translation2.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_DOUBLE_EQ(-10.0, XArray[0]);
    ASSERT_DOUBLE_EQ(-9.0, XArray[1]);
    ASSERT_DOUBLE_EQ(-8.0, XArray[2]);
    ASSERT_DOUBLE_EQ(-7.0, XArray[3]);
    ASSERT_DOUBLE_EQ(-6.0, XArray[4]);
    ASSERT_DOUBLE_EQ(-5.0, XArray[5]);
    ASSERT_DOUBLE_EQ(-4.0, XArray[6]);
    ASSERT_DOUBLE_EQ(-3.0, XArray[7]);
    ASSERT_DOUBLE_EQ(-2.0, XArray[8]);
    ASSERT_DOUBLE_EQ(-1.0, XArray[9]);

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

    Translation2.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(0.0, XResultDist, MYEPSILON);
    ASSERT_NEAR(0.0, YResultDist, MYEPSILON);

    }

TEST_F (HGF2DTranslationTester, ConversionInOutArraysTest3)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];

    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    Translation2.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_DOUBLE_EQ(10.0, XArrayInOut[0]);
    ASSERT_DOUBLE_EQ(11.0, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(12.0, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(13.0, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(14.0, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(15.0, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(16.0, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(17.0, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(18.0, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(19.0, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(20.0, YArrayInOut[9]);

    // Reset the in/out arrays before testing ConvertInverse
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }
    Translation2.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_DOUBLE_EQ(-10.0, XArrayInOut[0]);
    ASSERT_DOUBLE_EQ(-9.0, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(-8.0, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(-7.0, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(-6.0, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(-5.0, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(-4.0, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(-3.0, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(-2.0, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(-1.0, XArrayInOut[9]);

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
TEST_F (HGF2DTranslationTester, SpecialReversalTest)
    {

    HGF2DTranslation Translation1;
    Translation1.Reverse();

    ASSERT_NEAR(0.0, Translation1.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation1.GetTranslation().GetDeltaY(), MYEPSILON);

    // Translation with translation only
    HGF2DTranslation Translation2(HGF2DDisplacement(-10.0,-10.0));
    Translation2.Reverse();

    ASSERT_DOUBLE_EQ(10.0, Translation2.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation2.GetTranslation().GetDeltaY());

    }

//==================================================================================
// SpecialCompositionTest
//==================================================================================
TEST_F (HGF2DTranslationTester, SpecialCompositionTest)
    {

    HGF2DTranslation Translation1(HGF2DDisplacement(-10.0, -10.0));
    HGF2DTranslation Translation2(HGF2DDisplacement(1560.0, -1045.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1and2 = Translation1.ComposeInverseWithDirectOf(Translation2);
    HFCPtr<HGF2DTranslation> pSimiRes = (HGF2DTranslation*)pNewModel1and2->Clone();

    HFCMatrix<3, 3> MyMatrix1 = pSimiRes->GetMatrix();
    ASSERT_DOUBLE_EQ(1550.00, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(-1055.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel1andI2 = Translation1.ComposeInverseWithInverseOf(Translation2);
    HFCPtr<HGF2DTranslation> pSimiRes2 = (HGF2DTranslation*)pNewModel1andI2->Clone();

    MyMatrix1 = pSimiRes2->GetMatrix();
    ASSERT_DOUBLE_EQ(-1570.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(1035.00, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel2and1 = Translation2.ComposeInverseWithDirectOf(Translation1);
    HFCPtr<HGF2DTranslation> pSimiRes3 = (HGF2DTranslation*)pNewModel2and1->Clone();

    MyMatrix1 = pSimiRes3->GetMatrix();
    ASSERT_DOUBLE_EQ(1550.00, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(-1055.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel2andI1 = Translation2.ComposeInverseWithInverseOf(Translation1);
    HFCPtr<HGF2DTranslation> pSimiRes4 = (HGF2DTranslation*)pNewModel2andI1->Clone();

    MyMatrix1 = pSimiRes4->GetMatrix();
    ASSERT_DOUBLE_EQ(1570.00, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(-1035.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00000, MyMatrix1[1][1]);

    // Composition with translation
    HGF2DTranslation TransModel;
    TransModel.SetTranslation(HGF2DDisplacement(10.0, 11.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1andTrans = Translation1.ComposeInverseWithDirectOf(TransModel);
    HFCPtr<HGF2DTranslation> pSimiRes5 = (HGF2DTranslation*)pNewModel1andTrans->Clone();

    MyMatrix1 = pSimiRes5->GetMatrix();
    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModel1andITrans = Translation1.ComposeInverseWithInverseOf(TransModel);
    HFCPtr<HGF2DTranslation> pSimiRes6 = (HGF2DTranslation*)pNewModel1andITrans->Clone();

    MyMatrix1 = pSimiRes6->GetMatrix();
    ASSERT_DOUBLE_EQ(-20.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(-21.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.000, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    HFCPtr<HGF2DTransfoModel> pNewModelTransand1 = TransModel.ComposeInverseWithDirectOf(Translation1);
    
    }

