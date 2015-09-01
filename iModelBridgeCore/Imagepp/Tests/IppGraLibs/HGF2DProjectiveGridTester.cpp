//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DProjectiveGridTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DNonLinearTestIdentity.h"
#include "HGF2DProjectiveGridTester.h"

HGF2DProjectiveGridTester::HGF2DProjectiveGridTester() 
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

    }

//==================================================================================
// ConstructiontTest
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, ConstructiontTest)
    {

    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);

    // Check transformation properties
    ASSERT_FALSE(FirstAltaPhotoModel.IsIdentity());
    ASSERT_FALSE(FirstAltaPhotoModel.IsStretchable());
    ASSERT_FALSE(FirstAltaPhotoModel.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    FirstAltaPhotoModel.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DProjectiveGrid::CLASS_ID, FirstAltaPhotoModel.GetClassID());

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = FirstAltaPhotoModel.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DProjectiveGrid::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_FALSE(pModelClone->IsStretchable());
    ASSERT_FALSE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_FALSE(FirstAltaPhotoModel.PreservesLinearity());
    ASSERT_FALSE(FirstAltaPhotoModel.PreservesParallelism());
    ASSERT_FALSE(FirstAltaPhotoModel.PreservesShape());
    ASSERT_FALSE(FirstAltaPhotoModel.PreservesDirection());

    // Reversal test
    FirstAltaPhotoModel.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(FirstAltaPhotoModel.IsIdentity());
    ASSERT_FALSE(FirstAltaPhotoModel.IsStretchable());
    ASSERT_FALSE(FirstAltaPhotoModel.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    FirstAltaPhotoModel.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    FirstAltaPhotoModel.Reverse();

    }

//==================================================================================
// CompositionIdentityTest
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionIdentityTest)
    {

    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);
    HFCPtr<HGF2DTransfoModel> pIdenCompAPM = MyIdentity.ComposeInverseWithDirectOf(FirstAltaPhotoModel);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DProjectiveGrid::CLASS_ID, pIdenCompAPM->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM->IsIdentity());

    // Check geometric properties
    ASSERT_FALSE(pIdenCompAPM->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM->PreservesDirection());
    }

//==================================================================================
// CompositionSimilitudeTest
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionSimilitudeTest)
    {

    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);
    HFCPtr<HGF2DTransfoModel> pIdenCompAPM2 = MyIdentity.ComposeInverseWithInverseOf(FirstAltaPhotoModel);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DProjectiveGrid::CLASS_ID, pIdenCompAPM2->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM2->IsIdentity());

    // Check geometric properties
    ASSERT_FALSE(pIdenCompAPM2->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM2->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM2->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM2->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeWithIdentityTest
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionSimilitudeWithIdentityTest)
    {
             
    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);
    HFCPtr<HGF2DTransfoModel> pIdenCompAPM5 = FirstAltaPhotoModel.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DProjectiveGrid::CLASS_ID, pIdenCompAPM5->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM5->IsIdentity());

    // Check geometric properties
    ASSERT_FALSE(pIdenCompAPM5->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM5->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM5->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM5->PreservesDirection());

    }

//==================================================================================
// CompositionSimilitudeReversalWithIdentityTest
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionSimilitudeReversalWithIdentityTest)
    {

    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);
    HFCPtr<HGF2DTransfoModel> pIdenCompAPM6 = FirstAltaPhotoModel.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a alta photo model
    ASSERT_EQ(HGF2DProjectiveGrid::CLASS_ID, pIdenCompAPM6->GetClassID());

    // The result should have the properties of an alta photo model
    ASSERT_FALSE(pIdenCompAPM6->IsIdentity());

    // Check geometric properties
    ASSERT_FALSE(pIdenCompAPM6->PreservesLinearity());
    ASSERT_FALSE(pIdenCompAPM6->PreservesParallelism());
    ASSERT_FALSE(pIdenCompAPM6->PreservesShape());
    ASSERT_FALSE(pIdenCompAPM6->PreservesDirection());

    }
        
//==================================================================================
// StudyPrecisionOverTest
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, StudyPrecisionOverTest)
    {
    
    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);

    ASSERT_DOUBLE_EQ(60.0, FirstAltaPhotoModel.GetDirectStep());
    ASSERT_DOUBLE_EQ(59.9, FirstAltaPhotoModel.GetInverseStep());

    // Precision calculations
    double MeanError;
    double MaxError;
    FirstAltaPhotoModel.StudyPrecisionOver(HGF2DLiteExtent(0.0, 0.0, 10.0, 10.0), 60.0, &MeanError, &MaxError);

    }

//==================================================================================
// ConversionTest
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, ConversionTest)
    {

    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);

    // Direct conversions
    double XCoord = 10.0;
    double YCoord = 10.0;

    FirstAltaPhotoModel.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    XCoord = 10.0;
    YCoord = 10.0;

    double XConverted;
    double YConverted;

    FirstAltaPhotoModel.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    double XArray[10];
    double YArray[10];

    FirstAltaPhotoModel.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    FirstAltaPhotoModel.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    // Inverse conversions
    FirstAltaPhotoModel.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    XCoord = 10.0;
    YCoord = 10.0;

    FirstAltaPhotoModel.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    XCoord = 10.0;
    YCoord = 10.0;

    FirstAltaPhotoModel.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    FirstAltaPhotoModel.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    }

TEST_F (HGF2DProjectiveGridTester, ConversionInOutArraysTest)
    {
    HGF2DProjectiveGrid FirstAltaPhotoModel(NonLinearTestIdentity, 60.0, 59.9);
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];

    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    FirstAltaPhotoModel.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
    FirstAltaPhotoModel.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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


#ifdef WIP_IPPTEST

//==================================================================================
// ConstructiontTestNoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, ConstructiontTestNoLinear)
    {

    }

//==================================================================================
// CompositionTest1NoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionTest1NoLinear)
    {

    }

//==================================================================================
// CompositionTest2NoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionTest2NoLinear)
    {

    }

 //==================================================================================
// CompositionTest3NoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionTest3NoLinear)
    {

    }

//==================================================================================
// CompositionTest4NoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionTest4NoLinear)
    {
              
    }

//==================================================================================
// StudyPrecisionOverTestNoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, StudyPrecisionOverTestNoLinear)
    {

    }

//==================================================================================
// ConversionTestNoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, ConversionTestNoLinear)
    {

    }

//==================================================================================
// CompositionTestNoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, CompositionTestNoLinear)
    {

    }

//==================================================================================
// ComposeInverseWithDirectOfTestNoLinear
//==================================================================================
TEST_F (HGF2DProjectiveGridTester, ComposeInverseWithDirectOfTestNoLinear)
    {

    }

#endif
