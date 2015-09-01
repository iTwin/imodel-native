//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGF2DComplexTransfoModelTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DComplexTransfoModelTester.h"

HGF2DComplexTransfoModelTester::HGF2DComplexTransfoModelTester() 
    {

    //General
    Displacement = HGF2DDisplacement(10.0, 30.48);
    Rotation = PI/4;
    ScalingX = 1.00001;
    ScalingY = 2.00001;
    Scaling = 10.011;
    Anortho = -0.1;

    //Transformations
    Affine = HGF2DAffine(Displacement, Rotation, ScalingX, ScalingY, Anortho);
    Helmert = HGF2DHelmert(Displacement, Rotation);
    Projective = HGF2DProjective(Displacement, Rotation, ScalingX, ScalingY, Anortho);
    Similitude = HGF2DSimilitude(Displacement, Rotation, Scaling);
    Stretch = HGF2DStretch(Displacement, ScalingX, ScalingY);
    Translation = HGF2DTranslation(Displacement);

    //Coord Sys
    pWorld = new HGF2DCoordSys();
    pSys1 = new HGF2DCoordSys(Affine, pWorld);

    }

// THE PRESENT TEST CANNOT MAKE TESTS WITH NON-LINEAR TRANSFORMATION MODELS

//==================================================================================
// Construction tests
//==================================================================================
TEST_F (HGF2DComplexTransfoModelTester, ConstructorsTest)
    {

    //Default Constructor
    HGF2DComplexTransfoModel Transfo1;
    Transfo1.AddModel(Affine);
    ASSERT_EQ(1, Transfo1.GetNumberOfModels());

    //Copy Constructor
    HGF2DComplexTransfoModel Transfo2(Transfo1);
    ASSERT_EQ(1, Transfo2.GetNumberOfModels());

    //Operator = 
    HGF2DComplexTransfoModel Transfo3 = Transfo1;
    ASSERT_EQ(1, Transfo3.GetNumberOfModels());

    }

//==================================================================================
// AddModel          (const HGF2DTransfoModel& pi_rModelToAdd);
// GetNumberOfModels() const;
// GetModel(size_t modelNumber);
//==================================================================================
TEST_F (HGF2DComplexTransfoModelTester, ModelTest)
    {
    
    HGF2DComplexTransfoModel Transfo1;
    Transfo1.AddModel(Affine);
    ASSERT_EQ(1, Transfo1.GetNumberOfModels());
    
    Transfo1.AddModel(Helmert);
    ASSERT_EQ(2, Transfo1.GetNumberOfModels());

    Transfo1.AddModel(Projective);
    ASSERT_EQ(3, Transfo1.GetNumberOfModels());

    Transfo1.AddModel(Similitude);
    ASSERT_EQ(4, Transfo1.GetNumberOfModels());

    Transfo1.AddModel(Stretch);
    ASSERT_EQ(5, Transfo1.GetNumberOfModels());

    Transfo1.AddModel(Translation);
    ASSERT_EQ(6, Transfo1.GetNumberOfModels());

    HGF2DTransfoModel* TransfoModel1 = Transfo1.GetModel(0);
    ASSERT_EQ(HGF2DAffine::CLASS_ID, TransfoModel1->GetClassID());
    ASSERT_EQ(6, Transfo1.GetNumberOfModels());

    HGF2DTransfoModel* TransfoModel2 = Transfo1.GetModel(1);
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, TransfoModel2->GetClassID());
    ASSERT_EQ(6, Transfo1.GetNumberOfModels());

    HGF2DTransfoModel* TransfoModel3 = Transfo1.GetModel(2);
    ASSERT_EQ(HGF2DProjective::CLASS_ID, TransfoModel3->GetClassID());
    ASSERT_EQ(6, Transfo1.GetNumberOfModels());

    HGF2DTransfoModel* TransfoModel4 = Transfo1.GetModel(3);
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, TransfoModel4->GetClassID());
    ASSERT_EQ(6, Transfo1.GetNumberOfModels());

    HGF2DTransfoModel* TransfoModel5 = Transfo1.GetModel(4);
    ASSERT_EQ(HGF2DStretch::CLASS_ID, TransfoModel5->GetClassID());
    ASSERT_EQ(6, Transfo1.GetNumberOfModels());

    HGF2DTransfoModel* TransfoModel6 = Transfo1.GetModel(5);
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, TransfoModel6->GetClassID());
    ASSERT_EQ(6, Transfo1.GetNumberOfModels());
     
    }

//==================================================================================
// IsIdentity        () const;
// IsStretchable     (double pi_AngleTolerance = 0) const;
//==================================================================================
TEST_F (HGF2DComplexTransfoModelTester, IsTest)
    {

    HGF2DComplexTransfoModel Transfo1;

    ASSERT_TRUE(Transfo1.IsIdentity());
    ASSERT_TRUE(Transfo1.IsStretchable());

    Transfo1.AddModel(Stretch);
    ASSERT_FALSE(Transfo1.IsIdentity());
    ASSERT_TRUE(Transfo1.IsStretchable());

    Transfo1.AddModel(Translation);
    ASSERT_FALSE(Transfo1.IsIdentity());
    ASSERT_TRUE(Transfo1.IsStretchable());

    Transfo1.AddModel(Similitude);
    ASSERT_FALSE(Transfo1.IsIdentity());
    ASSERT_FALSE(Transfo1.IsStretchable());

    HGF2DComplexTransfoModel Transfo2;
    Transfo2.AddModel(Stretch);
    Transfo2.AddModel(Projective);
    Transfo2.AddModel(Translation);
    ASSERT_FALSE(Transfo2.IsIdentity());
    ASSERT_FALSE(Transfo2.IsStretchable());  

    HGF2DComplexTransfoModel Transfo3;
    Transfo3.AddModel(Helmert);
    Transfo3.AddModel(Projective);
    Transfo3.AddModel(Translation);
    ASSERT_FALSE(Transfo3.IsIdentity());
    ASSERT_FALSE(Transfo3.IsStretchable()); 

    HGF2DComplexTransfoModel Transfo4;
    Transfo4.AddModel(Affine);
    Transfo4.AddModel(Stretch);
    Transfo4.AddModel(Translation);
    ASSERT_FALSE(Transfo4.IsIdentity());
    ASSERT_FALSE(Transfo4.IsStretchable()); 

    }
    
//==================================================================================
// GetStretchParams  (double*  po_pScaleFactorX, double*  po_pScaleFactorY, 
//                    HGF2DDisplacement* po_pDisplacement) const; 
//==================================================================================
TEST_F (HGF2DComplexTransfoModelTester, GetStretchParamsTest)
    {

    double ScaleX;
    double ScaleY;
    HGF2DDisplacement Disp;
    
    HGF2DComplexTransfoModel Transfo1;

    Transfo1.GetStretchParams(&ScaleX, &ScaleY, &Disp);
    ASSERT_DOUBLE_EQ(1.0, ScaleX);
    ASSERT_DOUBLE_EQ(1.0, ScaleY);
    ASSERT_NEAR(0.0, Disp.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Disp.GetDeltaY(), MYEPSILON);

    Transfo1.AddModel(Affine);
    Transfo1.GetStretchParams(&ScaleX, &ScaleY, &Disp);
    ASSERT_DOUBLE_EQ(1.00001, ScaleX);
    ASSERT_DOUBLE_EQ(2.00001, ScaleY);
    ASSERT_DOUBLE_EQ(10.0000, Disp.GetDeltaX());
    ASSERT_DOUBLE_EQ(30.4800, Disp.GetDeltaY());

    Transfo1.AddModel(Projective);
    Transfo1.GetStretchParams(&ScaleX, &ScaleY, &Disp);
    ASSERT_DOUBLE_EQ(1.0000200001, ScaleX);
    ASSERT_DOUBLE_EQ(4.0000400001, ScaleY);
    ASSERT_DOUBLE_EQ(20.000000000, Disp.GetDeltaX());
    ASSERT_DOUBLE_EQ(60.960000000, Disp.GetDeltaY());

    Transfo1.AddModel(Helmert);
    Transfo1.GetStretchParams(&ScaleX, &ScaleY, &Disp);
    ASSERT_DOUBLE_EQ(1.0000200001, ScaleX);
    ASSERT_DOUBLE_EQ(4.0000400001, ScaleY);
    ASSERT_DOUBLE_EQ(30.000000000, Disp.GetDeltaX());
    ASSERT_DOUBLE_EQ(91.440000000, Disp.GetDeltaY());

    Transfo1.AddModel(Similitude);
    Transfo1.GetStretchParams(&ScaleX, &ScaleY, &Disp);
    ASSERT_DOUBLE_EQ(10.011200221001101, ScaleX);
    ASSERT_DOUBLE_EQ(40.044400441001102, ScaleY);
    ASSERT_DOUBLE_EQ(40.000000000000000, Disp.GetDeltaX());
    ASSERT_DOUBLE_EQ(121.92000000000000, Disp.GetDeltaY());

    }    

//==================================================================================
// CreateSimplifiedModel() const;
//==================================================================================
TEST_F (HGF2DComplexTransfoModelTester, CreateSimplifiedModelTest)
    {

    HGF2DComplexTransfoModel Transfo1;    
    HFCPtr<HGF2DTransfoModel> pResult1 = Transfo1.CreateSimplifiedModel();   
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult1->GetClassID());  

    HGF2DComplexTransfoModel Transfo2;
    Transfo2.AddModel(Translation);      
    HFCPtr<HGF2DTransfoModel> pResult2 = Transfo2.CreateSimplifiedModel();   
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pResult2->GetClassID()); 
    
    HGF2DComplexTransfoModel Transfo3;
    Transfo3.AddModel(Affine);      
    HFCPtr<HGF2DTransfoModel> pResult3 = Transfo3.CreateSimplifiedModel();   
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pResult3->GetClassID());

    HGF2DComplexTransfoModel Transfo4;
    Transfo4.AddModel(Helmert);      
    HFCPtr<HGF2DTransfoModel> pResult4 = Transfo4.CreateSimplifiedModel();   
    ASSERT_EQ(HGF2DHelmert::CLASS_ID, pResult4->GetClassID());
    
    HGF2DComplexTransfoModel Transfo5;
    Transfo5.AddModel(Projective);      
    HFCPtr<HGF2DTransfoModel> pResult5 = Transfo5.CreateSimplifiedModel();   
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pResult5->GetClassID());

    HGF2DComplexTransfoModel Transfo6;
    Transfo6.AddModel(Similitude);      
    HFCPtr<HGF2DTransfoModel> pResult6 = Transfo6.CreateSimplifiedModel();   
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult6->GetClassID());

    HGF2DComplexTransfoModel Transfo7;
    Transfo7.AddModel(Stretch);      
    HFCPtr<HGF2DTransfoModel> pResult7 = Transfo7.CreateSimplifiedModel();   
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pResult7->GetClassID());

    }   

//==================================================================================
// PreservesLinearity() const;
// PreservesParallelism() const;
// PreservesShape() const;
// PreservesDirection() const;
//==================================================================================
TEST_F (HGF2DComplexTransfoModelTester, PreserveTest)
    {

    HGF2DComplexTransfoModel Transfo1;

    ASSERT_TRUE(Transfo1.PreservesLinearity());
    ASSERT_TRUE(Transfo1.PreservesParallelism());
    ASSERT_TRUE(Transfo1.PreservesShape());
    ASSERT_TRUE(Transfo1.PreservesDirection());

    Transfo1.AddModel(Stretch);
    ASSERT_TRUE(Transfo1.PreservesLinearity());
    ASSERT_TRUE(Transfo1.PreservesParallelism());
    ASSERT_FALSE(Transfo1.PreservesShape());
    ASSERT_FALSE(Transfo1.PreservesDirection());

    Transfo1.AddModel(Translation);
    ASSERT_TRUE(Transfo1.PreservesLinearity());
    ASSERT_TRUE(Transfo1.PreservesParallelism());
    ASSERT_FALSE(Transfo1.PreservesShape());
    ASSERT_FALSE(Transfo1.PreservesDirection());

    Transfo1.AddModel(Similitude);
    ASSERT_TRUE(Transfo1.PreservesLinearity());
    ASSERT_TRUE(Transfo1.PreservesParallelism());
    ASSERT_FALSE(Transfo1.PreservesShape());
    ASSERT_FALSE(Transfo1.PreservesDirection());

    HGF2DComplexTransfoModel Transfo2;
    Transfo2.AddModel(Stretch);
    Transfo2.AddModel(Projective);
    Transfo2.AddModel(Translation);
    ASSERT_TRUE(Transfo2.PreservesLinearity());
    ASSERT_TRUE(Transfo2.PreservesParallelism());
    ASSERT_FALSE(Transfo2.PreservesShape());
    ASSERT_FALSE(Transfo2.PreservesDirection()); 

    HGF2DComplexTransfoModel Transfo3;
    Transfo3.AddModel(Helmert);
    Transfo3.AddModel(Projective);
    Transfo3.AddModel(Translation);
    ASSERT_TRUE(Transfo3.PreservesLinearity());
    ASSERT_TRUE(Transfo3.PreservesParallelism());
    ASSERT_FALSE(Transfo3.PreservesShape());
    ASSERT_FALSE(Transfo3.PreservesDirection());  

    HGF2DComplexTransfoModel Transfo4;
    Transfo4.AddModel(Affine);
    Transfo4.AddModel(Stretch);
    Transfo4.AddModel(Translation);
    ASSERT_TRUE(Transfo4.PreservesLinearity());
    ASSERT_TRUE(Transfo4.PreservesParallelism());
    ASSERT_FALSE(Transfo4.PreservesShape());
    ASSERT_FALSE(Transfo4.PreservesDirection()); 

    }
    
                     