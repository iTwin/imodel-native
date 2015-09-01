//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DLinearModelAdapterTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DNonLinearTestIdentity.h"
#include "HGF2DLinearModelAdapterTester.h"

#define MYEPSILON   (HNumeric<double>::GLOBAL_EPSILON())

HGF2DLinearModelAdapterTester::HGF2DLinearModelAdapterTester()
    {

    //General
    Translation = HGF2DDisplacement(10.0, 30.48);
    Rotation = PI/4;
    ScalingX = 1.0000;
    ScalingY = 2.0000;
    Anortho = -0.000001;
    Affine1 = HGF2DAffine(Translation, Rotation, ScalingX, ScalingY, Anortho);
    MyArea = HGF2DLiteExtent(0.0, 0.0, 100000, 100000);

    pWorld = new HGF2DCoordSys();
    pSys1 = new HGF2DCoordSys(Affine1, pWorld);

    //HGF2DLinearModelAdapter
    FirstAltaPhotoModel = HGF2DLinearModelAdapter(NonLinearTestIdentity, MyArea, 1000.0);
    FirstAltaPhotoModel2 = HGF2DLinearModelAdapter(NonLinearTestIdentity, MyArea, 1000.0, true);
    AltaPhotoModel1 = HGF2DLinearModelAdapter(NonLinearTestIdentity, MyArea, 1000.0);
    AltaPhotoModel2 = HGF2DLinearModelAdapter(NonLinearTestIdentity, MyArea, 1000.0, true);

    }

//==================================================================================
// ConstructiontTestAdaptedAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConstructiontTestAdaptedAsProjective)
    {

    // Check transformation properties
    ASSERT_FALSE(FirstAltaPhotoModel.IsIdentity());
    ASSERT_FALSE(FirstAltaPhotoModel.IsStretchable());
    ASSERT_TRUE(FirstAltaPhotoModel.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    FirstAltaPhotoModel.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.99999999999999933, ScalingX);
    ASSERT_DOUBLE_EQ(0.99999999999999867, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, FirstAltaPhotoModel.GetClassID());

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = FirstAltaPhotoModel.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_FALSE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(FirstAltaPhotoModel.PreservesLinearity());
    ASSERT_FALSE(FirstAltaPhotoModel.PreservesParallelism());
    ASSERT_FALSE(FirstAltaPhotoModel.PreservesShape());
    ASSERT_FALSE(FirstAltaPhotoModel.PreservesDirection());

    // Reversal test
    FirstAltaPhotoModel.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(FirstAltaPhotoModel.IsIdentity());
    ASSERT_FALSE(FirstAltaPhotoModel.IsStretchable());
    ASSERT_TRUE(FirstAltaPhotoModel.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    FirstAltaPhotoModel.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0000000000000013, ScalingX);
    ASSERT_DOUBLE_EQ(1.0000000000000013, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    FirstAltaPhotoModel.Reverse();

    }

//==================================================================================
// CompositionIdentityTestAdaptedAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionIdentityTestAdaptedAsProjective)
    {
      
    // Composition with model
    HFCPtr<HGF2DTransfoModel> pIdenCompAPM = MyIdentity.ComposeInverseWithDirectOf(FirstAltaPhotoModel);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompAPM->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeTestAdaptedAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionSimilitudeTestAdaptedAsProjective)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompAPM2 = MyIdentity.ComposeInverseWithInverseOf(FirstAltaPhotoModel);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM2->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM2->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompAPM2->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM2->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM2->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTestAdaptedAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionSimilitudeWithIdentityTestAdaptedAsProjective)
    {
    
    HFCPtr<HGF2DTransfoModel> pIdenCompAPM5 = FirstAltaPhotoModel.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM5->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM5->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompAPM5->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM5->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM5->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM5->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeReversalWithIdentityTestAdaptedAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionSimilitudeReversalWithIdentityTestAdaptedAsProjective)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompAPM6 = FirstAltaPhotoModel.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM6->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM6->IsIdentity());

    // Check geometric properties
    ASSERT_TRUE(pIdenCompAPM6->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM6->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM6->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM6->PreservesDirection());

    }

//==================================================================================
// StudyPrecisionOverTestAdaptedAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, StudyPrecisionOverTestAdaptedAsProjective)
    {

    ASSERT_DOUBLE_EQ(1000.0, FirstAltaPhotoModel.GetStep());

    // Precision calculations
    double MeanError;
    double MaxError;
    FirstAltaPhotoModel.StudyPrecisionOver(HGF2DLiteExtent(0.0, 0.0, 10.0, 10.0), 1.0, &MeanError, &MaxError);

    }

//==================================================================================
// ConversionTestAdaptedAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConversionTestAdaptedAsProjective)
    {

    // Direct conversions
    double XCoord = 10.0;
    double YCoord = 10.0;

    FirstAltaPhotoModel.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.000000000036366, XCoord);
    ASSERT_DOUBLE_EQ(10.000000000029090, YCoord);

    XCoord = 10.0;
    YCoord = 10.0;

    double XConverted;
    double YConverted;

    FirstAltaPhotoModel.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.000000000036366, XConverted);
    ASSERT_DOUBLE_EQ(10.000000000029090, YConverted);

    double XArray[10];
    double YArray[10];

    FirstAltaPhotoModel.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(0.0, XArray[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0000000000363729, XArray[1]);
    ASSERT_DOUBLE_EQ(2.0000000000363722, XArray[2]);
    ASSERT_DOUBLE_EQ(3.0000000000363718, XArray[3]);
    ASSERT_DOUBLE_EQ(4.0000000000363709, XArray[4]);
    ASSERT_DOUBLE_EQ(5.0000000000363700, XArray[5]);
    ASSERT_DOUBLE_EQ(6.0000000000363700, XArray[6]);
    ASSERT_DOUBLE_EQ(7.0000000000363691, XArray[7]);
    ASSERT_DOUBLE_EQ(8.0000000000363674, XArray[8]);
    ASSERT_DOUBLE_EQ(9.0000000000363674, XArray[9]);

    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[0]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[1]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[2]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[3]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[4]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[5]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[6]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[7]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[8]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[9]);

    double XDist(10.0);
    double YDist(10.0);

    double XResultDist;
    double YResultDist;

    FirstAltaPhotoModel.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.000000000036366, XResultDist);
    ASSERT_DOUBLE_EQ(10.000000000029090, YResultDist);

    // Inverse conversions
    FirstAltaPhotoModel.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(9.9999999999636344, XCoord);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YCoord);

    XCoord = 10.0;
    YCoord = 10.0;

    FirstAltaPhotoModel.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(9.9999999999636344, XConverted);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YConverted);

    XCoord = 10.0;
    YCoord = 10.0;

    FirstAltaPhotoModel.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(0.0, XArray[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.9999999999636271, XArray[1]);
    ASSERT_DOUBLE_EQ(1.9999999999636278, XArray[2]);
    ASSERT_DOUBLE_EQ(2.9999999999636282, XArray[3]);
    ASSERT_DOUBLE_EQ(3.9999999999636291, XArray[4]);
    ASSERT_DOUBLE_EQ(4.9999999999636300, XArray[5]);
    ASSERT_DOUBLE_EQ(5.9999999999636300, XArray[6]);
    ASSERT_DOUBLE_EQ(6.9999999999636309, XArray[7]);
    ASSERT_DOUBLE_EQ(7.9999999999636318, XArray[8]);
    ASSERT_DOUBLE_EQ(8.9999999999636326, XArray[9]);

    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[0]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[1]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[2]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[3]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[4]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[5]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[6]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[7]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[8]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[9]);

    FirstAltaPhotoModel.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(9.9999999999636344, XResultDist);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YResultDist);
    }


TEST_F(HGF2DLinearModelAdapterTester, ConversionInOutArraysTestAdaptedAsProjective)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10;
        }
    FirstAltaPhotoModel.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(0.0, XArrayInOut[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0000000000363729, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(2.0000000000363722, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(3.0000000000363718, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(4.0000000000363709, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(5.0000000000363700, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(6.0000000000363700, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(7.0000000000363691, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(8.0000000000363674, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(9.0000000000363674, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[9]);

    // Reset in/out arrays before testing ConvertInverse
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10;
        }
    FirstAltaPhotoModel.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(0.0, XArrayInOut[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.9999999999636271, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(1.9999999999636278, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(2.9999999999636282, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(3.9999999999636291, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(4.9999999999636300, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(5.9999999999636300, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(6.9999999999636309, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(7.9999999999636318, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(8.9999999999636326, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[9]);
    }
#ifdef WIP_IPPTEST //To complete once the new class is written

//==================================================================================
// ConstructiontTestNoLinearAsProjective
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConstructiontTestNoLinearAsProjective)
    {

    }

//==================================================================================
// CompositionTest1
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest1NoLinearAsProjective)
    {

    }

//==================================================================================
// CompositionTest2
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest2NoLinearAsProjective)
    {

    }

//==================================================================================
// CompositionTest3
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest3NoLinearAsProjective)
    {

    }

//==================================================================================
// CompositionTest4
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest4NoLinearAsProjective)
    {

    }

//==================================================================================
// StudyPrecisionOverTest
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, StudyPrecisionOverTestNoLinearAsProjective)
    {

    }

//==================================================================================
// ConversionTest
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConversionTestNoLinearAsProjective)
    {

    }

//==================================================================================
// SpecialCompositionTest
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, SpecialCompositionTestNoLinearAsProjective)
    {


    }

//==================================================================================
// ComposeInverseWithDirectOfTest
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ComposeInverseWithDirectOfTestNoLinearAsProjective)
    {


    }

#endif

//==================================================================================
// ConstructiontTestAdaptedAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConstructiontTestAdaptedAsSimpleAffine)
    {

    //Check transformation properties
    ASSERT_FALSE(FirstAltaPhotoModel2.IsIdentity());
    ASSERT_FALSE(FirstAltaPhotoModel2.IsStretchable());
    ASSERT_TRUE(FirstAltaPhotoModel2.CanBeRepresentedByAMatrix());

    //Extract stretch parameters
    FirstAltaPhotoModel2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    //Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check type of model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, FirstAltaPhotoModel2.GetClassID());

    //Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = FirstAltaPhotoModel2.Clone();

    //Check nature of clone
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pModelClone->GetClassID());

    //Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_FALSE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    //Check geometric properties
    ASSERT_TRUE(FirstAltaPhotoModel2.PreservesLinearity());
    ASSERT_TRUE(FirstAltaPhotoModel2.PreservesParallelism());
    ASSERT_FALSE(FirstAltaPhotoModel2.PreservesShape());
    ASSERT_FALSE(FirstAltaPhotoModel2.PreservesDirection());

    //Reversal test
    FirstAltaPhotoModel2.Reverse();

    //Check again transformation properties
    ASSERT_FALSE(FirstAltaPhotoModel2.IsIdentity());
    ASSERT_FALSE(FirstAltaPhotoModel2.IsStretchable());
    ASSERT_TRUE(FirstAltaPhotoModel2.CanBeRepresentedByAMatrix());

    //Extract again stretch parameters
    FirstAltaPhotoModel2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    //Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    FirstAltaPhotoModel2.Reverse();

    }

//==================================================================================
// ConstructiontTestAdaptedAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionIdentityTestAdaptedAsSimpleAffine)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompAPM = MyIdentity.ComposeInverseWithDirectOf(FirstAltaPhotoModel2);

    //The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM->GetClassID());

    //The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM->IsIdentity());

    //Check geometric properties
    ASSERT_TRUE(pIdenCompAPM->PreservesLinearity());
    ASSERT_TRUE(pIdenCompAPM->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeTestAdaptedAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionSimilitudeTestAdaptedAsSimpleAffine)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompAPM2 = MyIdentity.ComposeInverseWithInverseOf(FirstAltaPhotoModel2);

    //The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM2->GetClassID());

    //The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM2->IsIdentity());

    //Check geometric properties
    ASSERT_TRUE(pIdenCompAPM2->PreservesLinearity());
    ASSERT_TRUE(pIdenCompAPM2->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM2->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTestAdaptedAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionSimilitudeWithIdentityTestAdaptedAsSimpleAffine)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompAPM5 = FirstAltaPhotoModel2.ComposeInverseWithDirectOf(MyIdentity);

    //The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM5->GetClassID());

    //The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM5->IsIdentity());

    //Check geometric properties
    ASSERT_TRUE(pIdenCompAPM5->PreservesLinearity());
    ASSERT_TRUE(pIdenCompAPM5->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM5->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM5->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeReversalWithIdentityTestAdaptedAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionSimilitudeReversalWithIdentityTestAdaptedAsSimpleAffine)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompAPM6 = FirstAltaPhotoModel2.ComposeInverseWithInverseOf(MyIdentity);

    //The result should be a alta photo model
    ASSERT_EQ(HGF2DLinearModelAdapter::CLASS_ID, pIdenCompAPM6->GetClassID());

    //The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM6->IsIdentity());

    //Check geometric properties
    ASSERT_TRUE(pIdenCompAPM6->PreservesLinearity());
    ASSERT_TRUE(pIdenCompAPM6->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM6->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM6->PreservesDirection());

    }

//==================================================================================
// StudyPrecisionOverTestAdaptedAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, StudyPrecisionOverTestAdaptedAsSimpleAffine)
    {

    ASSERT_DOUBLE_EQ(1000.0, FirstAltaPhotoModel.GetStep());

    //Precision calculations
    double MeanError;
    double MaxError;
    FirstAltaPhotoModel.StudyPrecisionOver(HGF2DLiteExtent(0.0, 0.0, 10.0, 10.0), 1.0, &MeanError, &MaxError);

    }

//==================================================================================
// ConversionTestAdaptedAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConversionTestAdaptedAsSimpleAffine)
    {

    //Direct conversions
    double XCoord = 10.0;
    double YCoord = 10.0;

    FirstAltaPhotoModel.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.000000000036366, XCoord);
    ASSERT_DOUBLE_EQ(10.000000000029090, YCoord);

    XCoord = 10.0;
    YCoord = 10.0;

    double XConverted;
    double YConverted;

    FirstAltaPhotoModel.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.000000000036366, XConverted);
    ASSERT_DOUBLE_EQ(10.000000000029090, YConverted);

    double XArray[10];
    double YArray[10];

    FirstAltaPhotoModel.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(0.0, XArray[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0000000000363729, XArray[1]);
    ASSERT_DOUBLE_EQ(2.0000000000363722, XArray[2]);
    ASSERT_DOUBLE_EQ(3.0000000000363718, XArray[3]);
    ASSERT_DOUBLE_EQ(4.0000000000363709, XArray[4]);
    ASSERT_DOUBLE_EQ(5.0000000000363700, XArray[5]);
    ASSERT_DOUBLE_EQ(6.0000000000363700, XArray[6]);
    ASSERT_DOUBLE_EQ(7.0000000000363691, XArray[7]);
    ASSERT_DOUBLE_EQ(8.0000000000363674, XArray[8]);
    ASSERT_DOUBLE_EQ(9.0000000000363674, XArray[9]);

    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[0]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[1]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[2]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[3]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[4]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[5]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[6]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[7]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[8]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArray[9]);

    double XDist(10.0);
    double YDist(10.0);

    double XResultDist;
    double YResultDist;

    FirstAltaPhotoModel.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.000000000036366, XResultDist);
    ASSERT_DOUBLE_EQ(10.000000000029090, YResultDist);

    //Inverse conversions
    FirstAltaPhotoModel.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(9.9999999999636344, XCoord);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YCoord);

    XCoord = 10.0;
    YCoord = 10.0;

    FirstAltaPhotoModel.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(9.9999999999636344, XConverted);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YConverted);

    XCoord = 10.0;
    YCoord = 10.0;

    FirstAltaPhotoModel.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
    ASSERT_NEAR(0.0, XArray[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.9999999999636271, XArray[1]);
    ASSERT_DOUBLE_EQ(1.9999999999636278, XArray[2]);
    ASSERT_DOUBLE_EQ(2.9999999999636282, XArray[3]);
    ASSERT_DOUBLE_EQ(3.9999999999636291, XArray[4]);
    ASSERT_DOUBLE_EQ(4.9999999999636300, XArray[5]);
    ASSERT_DOUBLE_EQ(5.9999999999636300, XArray[6]);
    ASSERT_DOUBLE_EQ(6.9999999999636309, XArray[7]);
    ASSERT_DOUBLE_EQ(7.9999999999636318, XArray[8]);
    ASSERT_DOUBLE_EQ(8.9999999999636326, XArray[9]);

    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[0]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[1]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[2]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[3]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[4]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[5]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[6]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[7]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[8]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArray[9]);

    FirstAltaPhotoModel.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(9.9999999999636344, XResultDist);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YResultDist);

    }

TEST_F(HGF2DLinearModelAdapterTester, ConversionInOutArraysTestAdaptedAsSimpleAffine)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10;
        }
    FirstAltaPhotoModel.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(0.0, XArrayInOut[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0000000000363729, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(2.0000000000363722, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(3.0000000000363718, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(4.0000000000363709, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(5.0000000000363700, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(6.0000000000363700, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(7.0000000000363691, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(8.0000000000363674, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(9.0000000000363674, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(10.00000000002909, YArrayInOut[9]);

    // Reset the in/out arrays before testing ConvertInverse
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10;
        }
    FirstAltaPhotoModel.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
    ASSERT_NEAR(0.0, XArrayInOut[0], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.9999999999636271, XArrayInOut[1]);
    ASSERT_DOUBLE_EQ(1.9999999999636278, XArrayInOut[2]);
    ASSERT_DOUBLE_EQ(2.9999999999636282, XArrayInOut[3]);
    ASSERT_DOUBLE_EQ(3.9999999999636291, XArrayInOut[4]);
    ASSERT_DOUBLE_EQ(4.9999999999636300, XArrayInOut[5]);
    ASSERT_DOUBLE_EQ(5.9999999999636300, XArrayInOut[6]);
    ASSERT_DOUBLE_EQ(6.9999999999636309, XArrayInOut[7]);
    ASSERT_DOUBLE_EQ(7.9999999999636318, XArrayInOut[8]);
    ASSERT_DOUBLE_EQ(8.9999999999636326, XArrayInOut[9]);

    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[0]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[1]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[2]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[3]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[4]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[5]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[6]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[7]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[8]);
    ASSERT_DOUBLE_EQ(9.9999999999709104, YArrayInOut[9]);
    }

#ifdef WIP_IPPTEST //To complete once the new class is done

//==================================================================================
// ConstructiontTestNoLinearAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConstructiontTestNoLinearAsSimpleAffine)
    {

    }

//==================================================================================
// CompositionTest1AsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest1AsSimpleAffine)
    {

    }

//==================================================================================
// CompositionTest2AsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest2AsSimpleAffine)
    {

    }

//==================================================================================
// CompositionTest3AsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest3AsSimpleAffine)
    {

    }

//==================================================================================
// CompositionTest4AsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, CompositionTest4AsSimpleAffine)
    {

    }

//==================================================================================
// StudyPrecisionOverTest
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, StudyPrecisionOverTestAsSimpleAffine)
    {
  
    }

//==================================================================================
// ConversionTestNoLinearAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConversionTestNoLinearAsSimpleAffine)
    {

    }

//==================================================================================
// SpecialCompositionNoLinearAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, SpecialCompositionNoLinearAsSimpleAffine)
    {

    }

//==================================================================================
// ConstructiontTestNoLinearAsSimpleAffine
//==================================================================================
TEST_F(HGF2DLinearModelAdapterTester, ConstructiontTestNoLinearAsSimpleAffine)
    {

    }

#endif

    
