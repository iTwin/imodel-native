//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DHelmertTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DHelmertTester.h"

HGF2DHelmertTester::HGF2DHelmertTester() 
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

    //Helmert
    Herlmert2.SetTranslation(HGF2DDisplacement(10.0,10.0));
    Herlmert2.SetRotation(PI/ 4);

    }

//==================================================================================
// ConstructiontTest
//==================================================================================
TEST_F (HGF2DHelmertTester, DefaultConstructiontTest)
    {

    HGF2DHelmert DefaultHelmert;

    // Check transformation properties
    ASSERT_TRUE(DefaultHelmert.IsIdentity());
    ASSERT_TRUE(DefaultHelmert.IsStretchable());
    ASSERT_TRUE(DefaultHelmert.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    DefaultHelmert.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, DefaultHelmert.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = DefaultHelmert.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = DefaultHelmert.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_TRUE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(DefaultHelmert.PreservesLinearity());
    ASSERT_TRUE(DefaultHelmert.PreservesParallelism());
    ASSERT_TRUE(DefaultHelmert.PreservesShape());
    ASSERT_TRUE(DefaultHelmert.PreservesDirection());

    // Reversal test
    DefaultHelmert.Reverse();

    // Check again transformation properties
    ASSERT_TRUE(DefaultHelmert.IsIdentity());
    ASSERT_TRUE(DefaultHelmert.IsStretchable());
    ASSERT_TRUE(DefaultHelmert.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    DefaultHelmert.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// HGF2DHelmert(const HGF2DDisplacement& pi_rTranslation, const HGFAngle& pi_rRotation,
//             double pi_Scaling);
//==================================================================================
TEST_F (HGF2DHelmertTester, SimpleTransformationConstructionTest)
    {

    HGF2DHelmert     HelmertTranslation(HGF2DDisplacement(10.0, 10.0), 0.0);
    HGF2DHelmert     HelmertRotation(HGF2DDisplacement(0.0, 0.0), PI/4);

    // Check transformation properties
    ASSERT_FALSE(HelmertTranslation.IsIdentity());
    ASSERT_TRUE(HelmertTranslation.IsStretchable());
    ASSERT_TRUE(HelmertTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(HelmertRotation.IsIdentity());
    ASSERT_FALSE(HelmertRotation.IsStretchable());
    ASSERT_TRUE(HelmertRotation.CanBeRepresentedByAMatrix());

    // Extract Translation stretch parameters
    HelmertTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract Rotation stretch parameters
    HelmertRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, HelmertTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/4, HelmertRotation.GetRotation());

    // Check type of model
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, HelmertTranslation.GetClassID());
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, HelmertRotation.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = HelmertTranslation.GetMatrix();
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = HelmertRotation.GetMatrix();
    ASSERT_NEAR(0.0, MyMatrix2[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix2[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = HelmertTranslation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone2 = HelmertRotation.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pModelClone->GetClassID());
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pModelClone2->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    ASSERT_FALSE(pModelClone2->IsIdentity());
    ASSERT_FALSE(pModelClone2->IsStretchable());
    ASSERT_TRUE(pModelClone2->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(HelmertTranslation.PreservesLinearity());
    ASSERT_TRUE(HelmertTranslation.PreservesParallelism());
    ASSERT_TRUE(HelmertTranslation.PreservesShape());
    ASSERT_TRUE(HelmertTranslation.PreservesDirection());

    ASSERT_TRUE(HelmertRotation.PreservesLinearity());
    ASSERT_TRUE(HelmertRotation.PreservesParallelism());
    ASSERT_TRUE(HelmertRotation.PreservesShape());
    ASSERT_FALSE(HelmertRotation.PreservesDirection());

    // Reversal test
    HelmertTranslation.Reverse();
    HelmertRotation.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(HelmertTranslation.IsIdentity());
    ASSERT_TRUE(HelmertTranslation.IsStretchable());
    ASSERT_TRUE(HelmertTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(HelmertRotation.IsIdentity());
    ASSERT_FALSE(HelmertRotation.IsStretchable());
    ASSERT_TRUE(HelmertRotation.CanBeRepresentedByAMatrix());

    // Extract Translation again stretch parameters
    HelmertTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    // Extract Rotation again stretch parameters
    HelmertRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, HelmertTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-0.78539816339744828, HelmertRotation.GetRotation());

    }

//==================================================================================
// operator=(const HGF2DHelmert& pi_rObj);
// HGF2DHelmert(const HGF2DHelmert& pi_rObj);
//==================================================================================
TEST_F (HGF2DHelmertTester, CopyTest)
    {

    HGF2DHelmert HelmertOperator = Herlmert2;
    HGF2DHelmert HelmertCopy(Herlmert2);

    // Check transformation properties
    ASSERT_FALSE(HelmertOperator.IsIdentity());
    ASSERT_FALSE(HelmertOperator.IsStretchable());
    ASSERT_TRUE(HelmertOperator.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(HelmertCopy.IsIdentity());
    ASSERT_FALSE(HelmertCopy.IsStretchable());
    ASSERT_TRUE(HelmertCopy.CanBeRepresentedByAMatrix());

    // Extract stretch parameters of AffineOperator
    HelmertOperator.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract stretch parameters of AffineOperator
    HelmertCopy.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, HelmertOperator.GetClassID());
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, HelmertCopy.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = HelmertOperator.GetMatrix();
    ASSERT_DOUBLE_EQ(10.00000000000000000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.00000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = HelmertCopy.GetMatrix();

    ASSERT_DOUBLE_EQ(10.00000000000000000, MyMatrix2[0][2]);
    ASSERT_DOUBLE_EQ(10.00000000000000000, MyMatrix2[1][2]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][1]);

    //Check Rotation
    ASSERT_DOUBLE_EQ(PI/ 4, HelmertOperator.GetRotation());
    ASSERT_DOUBLE_EQ(PI/ 4, HelmertCopy.GetRotation());

    }

//==================================================================================
// CreateSimplifiedModel() const;
//==================================================================================
TEST_F(HGF2DHelmertTester, CreateSimplifiedModelTest)
    {
    
    //Simplified to HGF2DIdentity
    HGF2DHelmert     Helmert1(HGF2DDisplacement(0.0, 0.0), 0.0);
                                     
    HFCPtr<HGF2DTransfoModel> newModel1 = Helmert1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, newModel1->GetClassID());

    //Simplified to HGF2DTranslation
    HGF2DHelmert     Herlmert2(HGF2DDisplacement(10.0, 0.0), 0.0);
    
    HFCPtr<HGF2DTransfoModel> newModel2 = Herlmert2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, newModel2->GetClassID());

    }

//==================================================================================
// SetByMatrixParameters(double pi_A0, double pi_A1, double pi_A2, double pi_B0, double pi_B1, double pi_B2);
//==================================================================================
TEST_F (HGF2DHelmertTester, SetByMatrixParametersTest)
    {

    HGF2DHelmert Helmert;
    Helmert.SetByMatrixParameters(1.0, 0.0, 10.0, 0.0, 1.0, 10.0);

    // Check transformation properties
    ASSERT_FALSE(Helmert.IsIdentity());
    ASSERT_FALSE(Helmert.IsStretchable());
    ASSERT_TRUE(Helmert.CanBeRepresentedByAMatrix());

    Helmert.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(1.0, Translation.GetDeltaX());
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Helmert.GetMatrix();

    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[0][2]);
    ASSERT_NEAR(0.0, MyMatrix1[1][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[0][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(-1.0, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[1][0]);
    ASSERT_NEAR(0.0, MyMatrix1[1][1], MYEPSILON);

    //Check Rotation
    ASSERT_DOUBLE_EQ(1.5707963267948966, Helmert.GetRotation());

    }

//==================================================================================
// CompositionIdentityTest
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(DefaultHelmert);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi->PreservesDirection());

    }

//==================================================================================
// CompositionHelmertTest
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionHelmertTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(DefaultHelmert);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi2->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi2->PreservesDirection());

    }

//==================================================================================
// CompositionHelmertWithIdentityTest
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionHelmertWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = DefaultHelmert.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_TRUE(pIdenCompSimi5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi5->PreservesShape());
    ASSERT_TRUE(pIdenCompSimi5->PreservesDirection());

    }

//==================================================================================
// CompositionHelmertReversalWithIdentityTest
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionHelmertReversalWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = DefaultHelmert.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi6->GetClassID());

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
TEST_F (HGF2DHelmertTester, TranslationExtractionTest)
    {

    HGF2DDisplacement MyTranslation = DefaultHelmert.GetTranslation();

    ASSERT_NEAR(0.0, MyTranslation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, MyTranslation.GetDeltaY(), MYEPSILON);

    // Extraction of rotation
    ASSERT_NEAR(0.0, DefaultHelmert.GetRotation(), MYEPSILON);

    // Create a clone of default helmert
    HFCPtr<HGF2DHelmert> pDupSimi = (HGF2DHelmert*)DefaultHelmert.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(0.3408, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);

    // Set rotation
    pDupSimi->SetRotation(PI / 3.0);

    ASSERT_DOUBLE_EQ(0.3408000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000000000000000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0471975511965976, pDupSimi->GetRotation());

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);

    ASSERT_DOUBLE_EQ(1.6551955534014704, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(1.1732315713447199, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.8325957145940459, pDupSimi->GetRotation());

    // Addition of components with offset
    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(12.0000000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-4.4829356237309499, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.61799387799149400, pDupSimi->GetRotation());

    }

//==================================================================================
// ConversionTest
//==================================================================================
TEST_F (HGF2DHelmertTester, ConversionTest)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    DefaultHelmert.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    double XConverted;
    double YConverted;

    DefaultHelmert.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    double XArray[10];
    double YArray[10];
    DefaultHelmert.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultHelmert.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    // Inverse conversions
    DefaultHelmert.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    DefaultHelmert.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    DefaultHelmert.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultHelmert.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    }

//==================================================================================
// ConstructiontTest3
//==================================================================================
TEST_F (HGF2DHelmertTester, ConstructiontTest3)
    {

    // Create duplicates for compare operations
    HGF2DHelmert Helmert3(Herlmert2);
    HGF2DHelmert Helmert4(Herlmert2);
    Helmert4.Reverse();

    // Check transformation properties
    ASSERT_FALSE(Herlmert2.IsIdentity());
    ASSERT_FALSE(Herlmert2.IsStretchable());
    ASSERT_TRUE(Herlmert2.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    Herlmert2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, Herlmert2.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = Herlmert2.GetMatrix();

    ASSERT_DOUBLE_EQ(10.00000000000000000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.00000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = Herlmert2.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_FALSE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(Herlmert2.PreservesLinearity());
    ASSERT_TRUE(Herlmert2.PreservesParallelism());
    ASSERT_TRUE(Herlmert2.PreservesShape());
    ASSERT_FALSE(Herlmert2.PreservesDirection());

    // Reversal test
    Herlmert2.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(Herlmert2.IsIdentity());
    ASSERT_FALSE(Herlmert2.IsStretchable());
    ASSERT_TRUE(Herlmert2.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    Herlmert2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-14.142135623730951, Translation.GetDeltaX());
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    Herlmert2.Reverse();

    }

//==================================================================================
// CompositionIdentityTest3
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(Herlmert2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi->PreservesShape());
    ASSERT_FALSE(pIdenCompSimi->PreservesDirection());

    }

//==================================================================================
// CompositionHelmertTest3
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionHelmertTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(Herlmert2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi2->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi2->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi2->PreservesShape());
    ASSERT_FALSE(pIdenCompSimi2->PreservesDirection());

    }

//==================================================================================
// CompositionHelmertWithIdentityTest3
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionHelmertWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = Herlmert2.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi5->GetClassID());

    // The result should have the properties of an identity
    ASSERT_FALSE(pIdenCompSimi5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompSimi5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompSimi5->PreservesParallelism());
    ASSERT_TRUE(pIdenCompSimi5->PreservesShape());
    ASSERT_FALSE(pIdenCompSimi5->PreservesDirection());

    }

//==================================================================================
// CompositionHelmertReversalWithIdentityTest3
//==================================================================================
TEST_F (HGF2DHelmertTester, CompositionHelmertReversalWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = Herlmert2.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a similitude
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pIdenCompSimi6->GetClassID());

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
TEST_F (HGF2DHelmertTester, TranslationExtractionTest3)
    {

    HGF2DDisplacement MyTranslation = Herlmert2.GetTranslation();

    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaY());

    // Extraction of rotation
    ASSERT_DOUBLE_EQ(PI/4, Herlmert2.GetRotation());

    // Create a clone of default similitude
    HFCPtr<HGF2DHelmert> pDupSimi = (HGF2DHelmert*)Herlmert2.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(0.3408, 1.0);

    // Set translation
    pDupSimi->SetTranslation(Trans1);

    // Check result of setting
    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00000000000000000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(0.78539816339744828, pDupSimi->GetRotation());

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(0.34080000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.00000000000000000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(0.78539816339744828, pDupSimi->GetRotation());

    // Set rotation
    pDupSimi->SetRotation(PI / 3.0);

    ASSERT_DOUBLE_EQ(0.3408000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0000000000000000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0471975511965976, pDupSimi->GetRotation());

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);

    ASSERT_DOUBLE_EQ(1.6551955534014704, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(1.1732315713447199, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.8325957145940459, pDupSimi->GetRotation());

    // Addition of components with offset
    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(12.0000000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-4.4829356237309499, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.61799387799149400, pDupSimi->GetRotation());

    }

//==================================================================================
// ConversionTest3
//==================================================================================
TEST_F (HGF2DHelmertTester, ConversionTest3)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    Herlmert2.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.000000000000000, XCoord); 
    ASSERT_DOUBLE_EQ(24.142135623730951, YCoord);

    double XConverted;
    double YConverted;
    XCoord = 10.0;
    YCoord = 10.0;

    Herlmert2.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.000000000000000, XConverted); 
    ASSERT_DOUBLE_EQ(24.142135623730951, YConverted);

    double XArray[10];
    double YArray[10];
    Herlmert2.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(2.9289321881345254, XArray[0], MYEPSILON);
    ASSERT_NEAR(3.6360389693210728, XArray[1], MYEPSILON);
    ASSERT_NEAR(4.3431457505076203, XArray[2], MYEPSILON);
    ASSERT_NEAR(5.0502525316941682, XArray[3], MYEPSILON);
    ASSERT_NEAR(5.7573593128807161, XArray[4], MYEPSILON);
    ASSERT_NEAR(6.4644660940672631, XArray[5], MYEPSILON);
    ASSERT_NEAR(7.1715728752538110, XArray[6], MYEPSILON);
    ASSERT_NEAR(7.8786796564403581, XArray[7], MYEPSILON);
    ASSERT_NEAR(8.5857864376269060, XArray[8], MYEPSILON);
    ASSERT_NEAR(9.2928932188134539, XArray[9], MYEPSILON);

    ASSERT_NEAR(17.071067811865476, YArray[0], MYEPSILON);
    ASSERT_NEAR(17.778174593052022, YArray[1], MYEPSILON);
    ASSERT_NEAR(18.485281374238571, YArray[2], MYEPSILON);
    ASSERT_NEAR(19.192388155425117, YArray[3], MYEPSILON);
    ASSERT_NEAR(19.899494936611667, YArray[4], MYEPSILON);
    ASSERT_NEAR(20.606601717798213, YArray[5], MYEPSILON);
    ASSERT_NEAR(21.313708498984759, YArray[6], MYEPSILON);
    ASSERT_NEAR(22.020815280171306, YArray[7], MYEPSILON);
    ASSERT_NEAR(22.727922061357855, YArray[8], MYEPSILON);
    ASSERT_NEAR(23.435028842544405, YArray[9], MYEPSILON);

    double XDist(10.0);
    double YDist(10.0);

    double XResultDist;
    double YResultDist;

    Herlmert2.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.000000000000000, XResultDist); 
    ASSERT_DOUBLE_EQ(24.142135623730951, YResultDist);

    // Inverse conversions
    XCoord = 10.0;
    YCoord = 10.0;
    Herlmert2.ConvertInverse(&XCoord, &YCoord);
    ASSERT_NEAR(0.0, XCoord, MYEPSILON);
    ASSERT_NEAR(0.0, YCoord, MYEPSILON);

    XCoord = 10.0;
    YCoord = 10.0;

    Herlmert2.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(0.0, XConverted, MYEPSILON);
    ASSERT_NEAR(0.0, YConverted, MYEPSILON);

    Herlmert2.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(-7.07106781186547640, XArray[0], MYEPSILON);
    ASSERT_NEAR(-6.36396103067892850, XArray[1], MYEPSILON);
    ASSERT_NEAR(-5.65685424949238150, XArray[2], MYEPSILON);
    ASSERT_NEAR(-4.94974746830583360, XArray[3], MYEPSILON);
    ASSERT_NEAR(-4.24264068711928570, XArray[4], MYEPSILON);
    ASSERT_NEAR(-3.53553390593273860, XArray[5], MYEPSILON);
    ASSERT_NEAR(-2.82842712474619070, XArray[6], MYEPSILON);
    ASSERT_NEAR(-2.12132034355964370, XArray[7], MYEPSILON);
    ASSERT_NEAR(-1.41421356237309580, XArray[8], MYEPSILON);
    ASSERT_NEAR(-0.70710678118654791, XArray[9], MYEPSILON);

    ASSERT_NEAR(7.07106781186547460, YArray[0], MYEPSILON);
    ASSERT_NEAR(6.36396103067892760, YArray[1], MYEPSILON);
    ASSERT_NEAR(5.65685424949237970, YArray[2], MYEPSILON);
    ASSERT_NEAR(4.94974746830583180, YArray[3], MYEPSILON);
    ASSERT_NEAR(4.24264068711928480, YArray[4], MYEPSILON);
    ASSERT_NEAR(3.53553390593273730, YArray[5], MYEPSILON);
    ASSERT_NEAR(2.82842712474618980, YArray[6], MYEPSILON);
    ASSERT_NEAR(2.12132034355964280, YArray[7], MYEPSILON);
    ASSERT_NEAR(1.41421356237309490, YArray[8], MYEPSILON);
    ASSERT_NEAR(0.70710678118654702, YArray[9], MYEPSILON);

    Herlmert2.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(0.0, XResultDist, MYEPSILON);
    ASSERT_NEAR(0.0, YResultDist, MYEPSILON);

    }

//==================================================================================
// SpecialReversalTest
//==================================================================================
TEST_F (HGF2DHelmertTester, SpecialReversalTest)
    {

    HGF2DHelmert Helmert1;
    Helmert1.SetRotation(PI / 4);
    Helmert1.Reverse();

    ASSERT_DOUBLE_EQ(-0.78539816339744828, Helmert1.GetRotation());
    ASSERT_NEAR(0.0, Helmert1.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Helmert1.GetTranslation().GetDeltaY(), MYEPSILON);

    // Helmert with translation only
    HGF2DHelmert Herlmert2;
    Herlmert2.SetTranslation(HGF2DDisplacement(-10.0,-10.0));
    Herlmert2.Reverse();

    ASSERT_NEAR(0.0, Herlmert2.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Herlmert2.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Herlmert2.GetTranslation().GetDeltaY());

    // Test with both rotation and translation
    HGF2DHelmert Helmert4;
    Helmert4.SetRotation(PI / 4);
    Helmert4.SetTranslation(HGF2DDisplacement(-10.0, -10.0));
    Helmert4.Reverse();

    ASSERT_DOUBLE_EQ(-0.78539816339744828, Helmert4.GetRotation());
    ASSERT_DOUBLE_EQ(14.14213562373095100, Helmert4.GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, Helmert4.GetTranslation().GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// SpecialCompositionTest
//==================================================================================
TEST_F (HGF2DHelmertTester, SpecialCompositionTest)
    {

    HGF2DHelmert Helmert1;
    Helmert1.SetRotation(PI / 4);
    Helmert1.SetTranslation(HGF2DDisplacement(-10.0, -10.0));

    HGF2DHelmert Herlmert2;
    Herlmert2.SetRotation(PI / 3);
    Herlmert2.SetTranslation(HGF2DDisplacement(1560.0, -1045.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1and2 = Helmert1.ComposeInverseWithDirectOf(Herlmert2);
    HFCPtr<HGF2DHelmert> pSimiRes = (HGF2DHelmert*)pNewModel1and2->Clone();

    ASSERT_DOUBLE_EQ(1.83259571459404590, pSimiRes->GetRotation());
    ASSERT_DOUBLE_EQ(1563.66025403784440, pSimiRes->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-1058.6602540378444, pSimiRes->GetTranslation().GetDeltaY());

    HFCMatrix<3, 3> MyMatrix1 = pSimiRes->GetMatrix();
    ASSERT_NEAR(1563.660254037844400, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-1058.66025403784440, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(-0.25881904510252063, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.96592582628906831, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.965925826289068310, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(-0.25881904510252063, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel1andI2 = Helmert1.ComposeInverseWithInverseOf(Herlmert2);
    HFCPtr<HGF2DHelmert> pSimiRes2 = (HGF2DHelmert*)pNewModel1andI2->Clone();

    ASSERT_NEAR(-0.26179938779914935, pSimiRes2->GetRotation(), 10E-12);
    ASSERT_NEAR(111.3362929168936700, pSimiRes2->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(1877.159883941568800, pSimiRes2->GetTranslation().GetDeltaY(), 10E-12);

    MyMatrix1 = pSimiRes2->GetMatrix();
    ASSERT_NEAR(111.3362929168936700, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(1877.159883941568800, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.965925826289068310, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(0.258819045102520680, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-0.25881904510252068, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.965925826289068310, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel2and1 = Herlmert2.ComposeInverseWithDirectOf(Helmert1);
    HFCPtr<HGF2DHelmert> pSimiRes3 = (HGF2DHelmert*)pNewModel2and1->Clone();

    ASSERT_NEAR(1.8325957145940459, pSimiRes3->GetRotation(), 10E-12);
    ASSERT_NEAR(1832.0131649909563, pSimiRes3->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(354.15999231107173, pSimiRes3->GetTranslation().GetDeltaY(), 10E-12);

    MyMatrix1 = pSimiRes3->GetMatrix();
    ASSERT_NEAR(1832.013164990956300, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(354.1599923110717300, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(-0.25881904510252063, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.96592582628906831, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.965925826289068310, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(-0.25881904510252063, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel2andI1 = Herlmert2.ComposeInverseWithInverseOf(Helmert1);
    HFCPtr<HGF2DHelmert> pSimiRes4 = (HGF2DHelmert*)pNewModel2andI1->Clone();

    ASSERT_NEAR(0.26179938779914935, pSimiRes4->GetRotation(), 10E-12);
    ASSERT_NEAR(378.302127934803020, pSimiRes4->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-1842.0131649909563, pSimiRes4->GetTranslation().GetDeltaY(), 10E-12);

    MyMatrix1 = pSimiRes4->GetMatrix();
    ASSERT_NEAR(378.3021279348030200, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-1842.01316499095630, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.965925826289068310, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.25881904510252068, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.258819045102520680, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.965925826289068310, MyMatrix1[1][1], 10E-12);

    // Composition with translation
    HGF2DTranslation TransModel;

    TransModel.SetTranslation(HGF2DDisplacement(10.0, 11.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1andTrans = Helmert1.ComposeInverseWithDirectOf(TransModel);
    HFCPtr<HGF2DHelmert> pSimiRes5 = (HGF2DHelmert*)pNewModel1andTrans->Clone();

    ASSERT_NEAR(0.78539816339744828, pSimiRes5->GetRotation(), 10E-12);
    ASSERT_NEAR(0.0, pSimiRes5->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(1.00000000000000000, pSimiRes5->GetTranslation().GetDeltaY(), 10E-12);

    MyMatrix1 = pSimiRes5->GetMatrix();
    ASSERT_NEAR(0.0, MyMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(1.000000000000000000, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.70710678118654757, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel1andITrans = Helmert1.ComposeInverseWithInverseOf(TransModel);
    HFCPtr<HGF2DHelmert> pSimiRes6 = (HGF2DHelmert*)pNewModel1andITrans->Clone();

    ASSERT_NEAR(PI / 4, pSimiRes6->GetRotation(), 10E-12);
    ASSERT_NEAR(-20.0, pSimiRes6->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(-21.0, pSimiRes6->GetTranslation().GetDeltaY(), 10E-12);

    MyMatrix1 = pSimiRes6->GetMatrix();
    ASSERT_NEAR(-20.0000000000000000, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-21.0000000000000000, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.70710678118654757, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModelTransand1 = TransModel.ComposeInverseWithDirectOf(Helmert1);
    HFCPtr<HGF2DHelmert> pSimiRes7 = (HGF2DHelmert*)pNewModelTransand1->Clone();

    ASSERT_NEAR(0.78539816339744828, pSimiRes7->GetRotation(), 10E-12);
    ASSERT_NEAR(-10.707106781186546, pSimiRes7->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(4.84924240491749710, pSimiRes7->GetTranslation().GetDeltaY(), 10E-12);

    MyMatrix1 = pSimiRes7->GetMatrix();
    ASSERT_NEAR(-10.7071067811865460, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(4.849242404917497100, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.70710678118654757, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModelTransandI1 = TransModel.ComposeInverseWithInverseOf(Helmert1);
    HFCPtr<HGF2DHelmert> pSimiRes8 = (HGF2DHelmert*)pNewModelTransandI1->Clone();

    ASSERT_NEAR(-PI / 4.0, pSimiRes8->GetRotation(), 10E-12);
    ASSERT_NEAR(28.9913780286484480, pSimiRes8->GetTranslation().GetDeltaX(), 10E-12);
    ASSERT_NEAR(0.70710678118654968, pSimiRes8->GetTranslation().GetDeltaY(), 10E-12);

    MyMatrix1 = pSimiRes8->GetMatrix();
    ASSERT_NEAR(28.99137802864844800, MyMatrix1[0][2], 10E-12);
    ASSERT_NEAR(0.707106781186549680, MyMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[0][0], 10E-12);
    ASSERT_NEAR(0.707106781186547460, MyMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-0.70710678118654746, MyMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.707106781186547570, MyMatrix1[1][1], 10E-12);
    
    }

//==================================================================================
// ConversionTest using in/out arrays of locations
//==================================================================================
TEST_F (HGF2DHelmertTester, ConversionTestInOutArraysDefaultHelmert)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for (uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    DefaultHelmert.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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

    //Reset in/out array
    for (uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }
    DefaultHelmert.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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

TEST_F (HGF2DHelmertTester, ConversionTestInOutArraysHelmert2)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for (uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    Herlmert2.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(2.9289321881345254, XArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(3.6360389693210728, XArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(4.3431457505076203, XArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(5.0502525316941682, XArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(5.7573593128807161, XArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(6.4644660940672631, XArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(7.1715728752538110, XArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(7.8786796564403581, XArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(8.5857864376269060, XArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(9.2928932188134539, XArrayInOut[9], MYEPSILON);

    ASSERT_NEAR(17.071067811865476, YArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(17.778174593052022, YArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(18.485281374238571, YArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(19.192388155425117, YArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(19.899494936611667, YArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(20.606601717798213, YArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(21.313708498984759, YArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(22.020815280171306, YArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(22.727922061357855, YArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(23.435028842544405, YArrayInOut[9], MYEPSILON);

    //Reset in/out array
    for (uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }
    Herlmert2.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(-7.07106781186547640, XArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(-6.36396103067892850, XArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(-5.65685424949238150, XArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(-4.94974746830583360, XArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(-4.24264068711928570, XArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(-3.53553390593273860, XArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(-2.82842712474619070, XArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(-2.12132034355964370, XArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(-1.41421356237309580, XArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(-0.70710678118654791, XArrayInOut[9], MYEPSILON);

    ASSERT_NEAR(7.07106781186547460, YArrayInOut[0], MYEPSILON);
    ASSERT_NEAR(6.36396103067892760, YArrayInOut[1], MYEPSILON);
    ASSERT_NEAR(5.65685424949237970, YArrayInOut[2], MYEPSILON);
    ASSERT_NEAR(4.94974746830583180, YArrayInOut[3], MYEPSILON);
    ASSERT_NEAR(4.24264068711928480, YArrayInOut[4], MYEPSILON);
    ASSERT_NEAR(3.53553390593273730, YArrayInOut[5], MYEPSILON);
    ASSERT_NEAR(2.82842712474618980, YArrayInOut[6], MYEPSILON);
    ASSERT_NEAR(2.12132034355964280, YArrayInOut[7], MYEPSILON);
    ASSERT_NEAR(1.41421356237309490, YArrayInOut[8], MYEPSILON);
    ASSERT_NEAR(0.70710678118654702, YArrayInOut[9], MYEPSILON);
    }
