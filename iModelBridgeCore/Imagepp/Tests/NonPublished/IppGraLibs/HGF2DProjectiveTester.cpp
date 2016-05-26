//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/NonPublished/IppGraLibs/HGF2DProjectiveTester.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGF2DProjectiveTester.h"

HGF2DProjectiveTester::HGF2DProjectiveTester() 
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

    //Projective
    Projective2.AddIsotropicScaling(2.0);
    Projective2.AddRotation(PI/ 4);
    Projective2.AddTranslation(HGF2DDisplacement(10.0, 10.0));

    }
    
//==================================================================================
// DefaultConstructiontTest
//==================================================================================
TEST_F(HGF2DProjectiveTester, DefaultConstructiontTest)
    {

    // Check transformation properties
    ASSERT_TRUE(DefaultProjective.IsIdentity());
    ASSERT_TRUE(DefaultProjective.IsStretchable());
    ASSERT_TRUE(DefaultProjective.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    DefaultProjective.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract stretch parameters
    DefaultProjective.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 1000.0, 1000.0);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DProjective::CLASS_ID, DefaultProjective.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> TheMatrix1 = DefaultProjective.GetMatrix();

    ASSERT_NEAR(0.0, TheMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, TheMatrix1[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, TheMatrix1[0][0]);
    ASSERT_NEAR(0.0, TheMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, TheMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.0, TheMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = DefaultProjective.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_TRUE(pModelClone->IsIdentity());
    ASSERT_TRUE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(DefaultProjective.PreservesLinearity());
    ASSERT_TRUE(DefaultProjective.PreservesParallelism());
    ASSERT_TRUE(DefaultProjective.PreservesShape());
    ASSERT_TRUE(DefaultProjective.PreservesDirection());

    // Reversal test
    DefaultProjective.Reverse();

    // Check again transformation properties
    ASSERT_TRUE(DefaultProjective.IsIdentity());
    ASSERT_TRUE(DefaultProjective.IsStretchable());
    ASSERT_TRUE(DefaultProjective.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    DefaultProjective.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract again stretch parameters
    DefaultProjective.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 1000.0, 1000.0);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// HGF2DProjective(const HGF2DDisplacement& pi_rTranslation, const HGFAngle& pi_rRotation,
//             double pi_ScalingX, double pi_ScalingY, const HGFAngle& pi_rAnorthogonality);
//==================================================================================
TEST_F (HGF2DProjectiveTester, SimpleTransformationConstructionTest)
    {

    HGF2DProjective     ProjectiveTranslation(HGF2DDisplacement(10.0, 10.0), 0.0, 1.0, 1.0, 0.0);
    HGF2DProjective     ProjectiveRotation(HGF2DDisplacement(0.0, 0.0), PI/4, 1.0, 1.0, 0.0);
    HGF2DProjective     ProjectiveScaling(HGF2DDisplacement(0.0, 0.0), 0.0, 10.0, 10.0, 0.0);
    HGF2DProjective     ProjectiveAnothorgonality(HGF2DDisplacement(0.0, 0.0), 0.0, 1.0, 1.0, (PI/4));

    // Check transformation properties
    ASSERT_FALSE(ProjectiveTranslation.IsIdentity());
    ASSERT_TRUE(ProjectiveTranslation.IsStretchable());
    ASSERT_TRUE(ProjectiveTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveRotation.IsIdentity());
    ASSERT_FALSE(ProjectiveRotation.IsStretchable());
    ASSERT_TRUE(ProjectiveRotation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveScaling.IsIdentity());
    ASSERT_TRUE(ProjectiveScaling.IsStretchable());
    ASSERT_TRUE(ProjectiveScaling.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveAnothorgonality.IsIdentity());
    ASSERT_FALSE(ProjectiveAnothorgonality.IsStretchable());
    ASSERT_TRUE(ProjectiveAnothorgonality.CanBeRepresentedByAMatrix());

    // Extract Translation stretch parameters
    ProjectiveTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract Rotation stretch parameters
    ProjectiveRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Scaling stretch parameters
    ProjectiveScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(10.0, ScalingX);
    ASSERT_DOUBLE_EQ(10.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Anothorgality stretch parameters
    ProjectiveAnothorgonality.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, ProjectiveTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(PI/4, ProjectiveRotation.GetRotation());
    ASSERT_NEAR(0.0, ProjectiveScaling.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, ProjectiveAnothorgonality.GetRotation(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveTranslation.GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveRotation.GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveScaling.GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveAnothorgonality.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = ProjectiveTranslation.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[0][0]);
    ASSERT_NEAR(0.0, MyMatrix1[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix1[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = ProjectiveRotation.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix2[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix2[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(0.707106781186547570, MyMatrix2[1][1]);

    HFCMatrix<3, 3> MyMatrix3 = ProjectiveScaling.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix3[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[0][0]);
    ASSERT_NEAR(0.0, MyMatrix3[0][1], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[1][1]);

    HFCMatrix<3, 3> MyMatrix4 = ProjectiveAnothorgonality.GetMatrix();

    ASSERT_NEAR(0.0, MyMatrix4[0][2], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix4[1][2], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.000000000000000000, MyMatrix4[0][0]);
    ASSERT_DOUBLE_EQ(-0.70710678118654757, MyMatrix4[0][1]);
    ASSERT_NEAR(0.0, MyMatrix4[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(0.70710678118654757, MyMatrix4[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = ProjectiveTranslation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone2 = ProjectiveRotation.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone3 = ProjectiveScaling.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone4 = ProjectiveAnothorgonality.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone->GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone2->GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone3->GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone4->GetClassID());

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
    ASSERT_TRUE(ProjectiveTranslation.PreservesLinearity());
    ASSERT_TRUE(ProjectiveTranslation.PreservesParallelism());
    ASSERT_TRUE(ProjectiveTranslation.PreservesShape());
    ASSERT_TRUE(ProjectiveTranslation.PreservesDirection());

    ASSERT_TRUE(ProjectiveRotation.PreservesLinearity());
    ASSERT_TRUE(ProjectiveRotation.PreservesParallelism());
    ASSERT_TRUE(ProjectiveRotation.PreservesShape());
    ASSERT_FALSE(ProjectiveRotation.PreservesDirection());

    ASSERT_TRUE(ProjectiveScaling.PreservesLinearity());
    ASSERT_TRUE(ProjectiveScaling.PreservesParallelism());
    ASSERT_TRUE(ProjectiveScaling.PreservesShape());
    ASSERT_TRUE(ProjectiveScaling.PreservesDirection());

    ASSERT_TRUE(ProjectiveAnothorgonality.PreservesLinearity());
    ASSERT_TRUE(ProjectiveAnothorgonality.PreservesParallelism());
    ASSERT_FALSE(ProjectiveAnothorgonality.PreservesShape());
    ASSERT_FALSE(ProjectiveAnothorgonality.PreservesDirection());

    // Reversal test
    ProjectiveTranslation.Reverse();
    ProjectiveRotation.Reverse();
    ProjectiveScaling.Reverse();
    ProjectiveAnothorgonality.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(ProjectiveTranslation.IsIdentity());
    ASSERT_TRUE(ProjectiveTranslation.IsStretchable());
    ASSERT_TRUE(ProjectiveTranslation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveRotation.IsIdentity());
    ASSERT_FALSE(ProjectiveRotation.IsStretchable());
    ASSERT_TRUE(ProjectiveRotation.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveScaling.IsIdentity());
    ASSERT_TRUE(ProjectiveScaling.IsStretchable());
    ASSERT_TRUE(ProjectiveScaling.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveAnothorgonality.IsIdentity());
    ASSERT_FALSE(ProjectiveAnothorgonality.IsStretchable());
    ASSERT_TRUE(ProjectiveAnothorgonality.CanBeRepresentedByAMatrix());

    // Extract Translation again stretch parameters
    ProjectiveTranslation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    // Extract Rotation again stretch parameters
    ProjectiveRotation.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Scaling again stretch parameters
    ProjectiveScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.1, ScalingX);
    ASSERT_DOUBLE_EQ(0.1, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    // Extract Anothorgonality again stretch parameters
    ProjectiveScaling.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.1, ScalingX);
    ASSERT_DOUBLE_EQ(0.1, ScalingY);

    // Check again value of displacement
    ASSERT_NEAR(0.0, Translation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    //Check value of rotation
    ASSERT_NEAR(0.0, ProjectiveTranslation.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(-PI/4, ProjectiveRotation.GetRotation());
    ASSERT_NEAR(0.0, ProjectiveScaling.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, ProjectiveAnothorgonality.GetRotation(), MYEPSILON); 

    }

//==================================================================================
// HGF2DProjective(const HFCMatrix<3, 3>& pi_rMatrix);
// HGF2DProjective(double const           pi_pMatrix[3][3])
// SetByMatrix(const HFCMatrix<3, 3>& pi_rMatrix);
//==================================================================================
TEST_F (HGF2DProjectiveTester, SpecialConstructorTest)
    {

    HFCMatrix<3, 3> MyMatrix1; 

    MyMatrix1[0][0] = 1.0;
    MyMatrix1[0][1] = 0.0;
    MyMatrix1[0][2] = 10.0;
    MyMatrix1[1][0] = 0.0;
    MyMatrix1[1][1] = 1.0;
    MyMatrix1[1][2] = 10.0;
    MyMatrix1[2][0] = 0.0;
    MyMatrix1[2][1] = 0.0;
    MyMatrix1[2][2] = 1.0;

    double MyMatrix2[3][3] = {1.0, 0.0, 10.0, 0.0, 1.0, 10.0, 0.0, 0.0, 1.0};
    
    HGF2DProjective     ProjectiveTranslation1(MyMatrix1);
    HGF2DProjective     ProjectiveTranslation2(MyMatrix2);
    HGF2DProjective     ProjectiveTranslation3(MyMatrix1);

    // Check transformation properties
    ASSERT_FALSE(ProjectiveTranslation1.IsIdentity());
    ASSERT_TRUE(ProjectiveTranslation1.IsStretchable());
    ASSERT_TRUE(ProjectiveTranslation1.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveTranslation2.IsIdentity());
    ASSERT_TRUE(ProjectiveTranslation2.IsStretchable());
    ASSERT_TRUE(ProjectiveTranslation2.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveTranslation3.IsIdentity());
    ASSERT_TRUE(ProjectiveTranslation3.IsStretchable());
    ASSERT_TRUE(ProjectiveTranslation3.CanBeRepresentedByAMatrix());

    // Extract ProjectiveTranslation1 stretch parameters
    ProjectiveTranslation1.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract ProjectiveTranslation2 stretch parameters
    ProjectiveTranslation2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract ProjectiveTranslation3 stretch parameters
    ProjectiveTranslation3.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    //Check value of rotation
    ASSERT_NEAR(0.0, ProjectiveTranslation1.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, ProjectiveTranslation2.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, ProjectiveTranslation3.GetRotation(), MYEPSILON);

    // Check type of model
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveTranslation1.GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveTranslation2.GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveTranslation3.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix3 = ProjectiveTranslation1.GetMatrix();
    ASSERT_DOUBLE_EQ(1.00, MyMatrix3[0][0]);
    ASSERT_NEAR(0.0, MyMatrix3[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[0][2]);
    ASSERT_NEAR(0.0, MyMatrix3[0][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix3[1][1]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix3[1][2]);
    ASSERT_NEAR(0.0, MyMatrix3[2][0], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix3[2][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix3[2][2]);

    HFCMatrix<3, 3> MyMatrix4 = ProjectiveTranslation2.GetMatrix();
    ASSERT_DOUBLE_EQ(1.00, MyMatrix4[0][0]);
    ASSERT_NEAR(0.0, MyMatrix4[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix4[0][2]);
    ASSERT_NEAR(0.0, MyMatrix4[0][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix4[1][1]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix4[1][2]);
    ASSERT_NEAR(0.0, MyMatrix4[2][0], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix4[2][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix4[2][2]);

    HFCMatrix<3, 3> MyMatrix5 = ProjectiveTranslation3.GetMatrix();
    ASSERT_DOUBLE_EQ(1.00, MyMatrix5[0][0]);
    ASSERT_NEAR(0.0, MyMatrix5[1][0], MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix5[0][2]);
    ASSERT_NEAR(0.0, MyMatrix5[0][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix5[1][1]);
    ASSERT_DOUBLE_EQ(10.0, MyMatrix5[1][2]);
    ASSERT_NEAR(0.0, MyMatrix5[2][0], MYEPSILON);
    ASSERT_NEAR(0.0, MyMatrix5[2][1], MYEPSILON);
    ASSERT_DOUBLE_EQ(1.00, MyMatrix5[2][2]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone1 = ProjectiveTranslation1.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone2 = ProjectiveTranslation2.Clone();
    HFCPtr<HGF2DTransfoModel> pModelClone3 = ProjectiveTranslation2.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone1->GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone2->GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone3->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone1->IsIdentity());
    ASSERT_TRUE(pModelClone1->IsStretchable());
    ASSERT_TRUE(pModelClone1->CanBeRepresentedByAMatrix());

    ASSERT_FALSE(pModelClone2->IsIdentity());
    ASSERT_TRUE(pModelClone2->IsStretchable());
    ASSERT_TRUE(pModelClone2->CanBeRepresentedByAMatrix());

    ASSERT_FALSE(pModelClone3->IsIdentity());
    ASSERT_TRUE(pModelClone3->IsStretchable());
    ASSERT_TRUE(pModelClone3->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(ProjectiveTranslation1.PreservesLinearity());
    ASSERT_TRUE(ProjectiveTranslation1.PreservesParallelism());
    ASSERT_TRUE(ProjectiveTranslation1.PreservesShape());
    ASSERT_TRUE(ProjectiveTranslation1.PreservesDirection());

    ASSERT_TRUE(ProjectiveTranslation2.PreservesLinearity());
    ASSERT_TRUE(ProjectiveTranslation2.PreservesParallelism());
    ASSERT_TRUE(ProjectiveTranslation2.PreservesShape());
    ASSERT_TRUE(ProjectiveTranslation2.PreservesDirection());

    ASSERT_TRUE(ProjectiveTranslation3.PreservesLinearity());
    ASSERT_TRUE(ProjectiveTranslation3.PreservesParallelism());
    ASSERT_TRUE(ProjectiveTranslation3.PreservesShape());
    ASSERT_TRUE(ProjectiveTranslation3.PreservesDirection());

    // Reversal test
    ProjectiveTranslation1.Reverse();
    ProjectiveTranslation2.Reverse();
    ProjectiveTranslation3.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(ProjectiveTranslation1.IsIdentity()); 
    ASSERT_FALSE(ProjectiveTranslation2.IsIdentity()); 
    ASSERT_FALSE(ProjectiveTranslation3.IsIdentity()); 

    // Extract ProjectiveTranslation1 again stretch parameters
    ProjectiveTranslation1.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    // Extract ProjectiveTranslation2 again stretch parameters
    ProjectiveTranslation2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    // Extract ProjectiveTranslation3 again stretch parameters
    ProjectiveTranslation3.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.0, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(-10.0, Translation.GetDeltaY());

    //Check value of rotation
    ASSERT_NEAR(0.0, ProjectiveTranslation1.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, ProjectiveTranslation2.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, ProjectiveTranslation3.GetRotation(), MYEPSILON);

    }

//==================================================================================
// operator=(const HGF2DAffine& pi_rObj);
// HGF2DProjective(const HGF2DAffine& pi_rObj);
//==================================================================================
TEST_F (HGF2DProjectiveTester, CopyTest)
    {

    HGF2DProjective ProjectiveOperator = Projective2;
    HGF2DProjective ProjectiveCopy(Projective2);

    // Check transformation properties
    ASSERT_FALSE(ProjectiveOperator.IsIdentity());
    ASSERT_FALSE(ProjectiveOperator.IsStretchable());
    ASSERT_TRUE(ProjectiveOperator.CanBeRepresentedByAMatrix());

    ASSERT_FALSE(ProjectiveCopy.IsIdentity());
    ASSERT_FALSE(ProjectiveCopy.IsStretchable());
    ASSERT_TRUE(ProjectiveCopy.CanBeRepresentedByAMatrix());

    // Extract stretch parameters of ProjectiveOperator
    ProjectiveOperator.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract stretch parameters of ProjectiveOperator
    ProjectiveCopy.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveOperator.GetClassID());
    ASSERT_EQ(HGF2DProjective::CLASS_ID, ProjectiveCopy.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> MyMatrix1 = ProjectiveOperator.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix1[1][1]);

    HFCMatrix<3, 3> MyMatrix2 = ProjectiveCopy.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix2[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, MyMatrix2[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, MyMatrix2[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, MyMatrix2[1][1]);

    //Check Rotation
    ASSERT_DOUBLE_EQ(PI/ 4, ProjectiveOperator.GetRotation());
    ASSERT_DOUBLE_EQ(PI/ 4, ProjectiveCopy.GetRotation());

    }

//==================================================================================
// CreateSimplifiedModel() const;
//==================================================================================
TEST_F(HGF2DProjectiveTester, CreateSimplifiedModelTest)
    {
    
    //Simplified to HGF2DIdentity
    HGF2DProjective     Projective1(HGF2DDisplacement(0.0, 0.0), 0.0, 1.0, 1.0, 0.0);
                                     
    HFCPtr<HGF2DTransfoModel> newModel1 = Projective1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, newModel1->GetClassID());

    //Simplified to HGF2DTranslation
    HGF2DProjective     Projective2(HGF2DDisplacement(10.0, 0.0), 0.0, 1.0, 1.0, 0.0);
    
    HFCPtr<HGF2DTransfoModel> newModel2 = Projective2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, newModel2->GetClassID());

    //Simplified to HGF2DStretch
    HGF2DProjective     Projective3(HGF2DDisplacement(10.0, 0.0), 0.0, 2.0, 3.0, 0.0);
    
    HFCPtr<HGF2DTransfoModel> newModel3 = Projective3.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DStretch::CLASS_ID, newModel3->GetClassID());

    //Simplified to HGF2DSimilitude
    HGF2DProjective     Projective4(HGF2DDisplacement(0.0, 0.0), 180.0, 1.0, 1.0, 0.0);
    
    HFCPtr<HGF2DTransfoModel> newModel4 = Projective4.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, newModel4->GetClassID());

    //Simplified to HGF2DAffine
    HGF2DProjective     Projective5(HGF2DDisplacement(0.0, 0.0), 40.0, 1.0, 1.0, 20.0);
    
    HFCPtr<HGF2DTransfoModel> newModel5 = Projective5.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DAffine::CLASS_ID, newModel5->GetClassID());

    }

//==================================================================================
// CompositionIdentityTest
//==================================================================================
TEST_F(HGF2DProjectiveTester, CompositionIdentityTest)
    {

    // Composition with similitude
    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(DefaultProjective);

    // The result should be a projective
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi->GetClassID());

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
TEST_F(HGF2DProjectiveTester, CompositionSimilitudeTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(DefaultProjective);

    // The result should be a projective
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi2->GetClassID());

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
TEST_F(HGF2DProjectiveTester, CompositionSimilitudeWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = DefaultProjective.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a projective
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi5->GetClassID());

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
TEST_F(HGF2DProjectiveTester, CompositionSimilitudeReversalWithIdentityTest)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = DefaultProjective.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a projective
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi6->GetClassID());

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
TEST_F(HGF2DProjectiveTester, TranslationExtractionTest)
    {

    HGF2DDisplacement MyTranslation = DefaultProjective.GetTranslation();

    ASSERT_NEAR(0.0, MyTranslation.GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, MyTranslation.GetDeltaY(), MYEPSILON);

    // Scale extraction test
    ASSERT_DOUBLE_EQ(1.0, DefaultProjective.GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, DefaultProjective.GetYScaling());

    // Extraction of rotation
    ASSERT_NEAR(0.0, DefaultProjective.GetRotation(), MYEPSILON);

    // Create a clone of default projective
    HFCPtr<HGF2DProjective> pDupSimi = (HGF2DProjective*)DefaultProjective.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(1.0, 1.0);

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(1.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    // Add scaling
    pDupSimi->AddIsotropicScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(2.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);

    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(2.8284271247461898, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.0000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(2.0000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(PI / 4.0, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    // Addition of components with offset
    pDupSimi->AddIsotropicScaling(2.5, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(-15.000000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-7.9289321881345254, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(5.00000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(5.00000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(0.78539816339744828, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(4.99999999999999910, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-20.355339059327378, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(5.00000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(5.00000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(1.57079632679489630, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    // Create a clone of default projective
    pDupSimi = (HGF2DProjective*)DefaultProjective.Clone();

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

    ASSERT_DOUBLE_EQ(40.00, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(-50.0, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(-10.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(18.00, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddVerticalFlip();

    ASSERT_DOUBLE_EQ(40.00, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(50.00, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(-10.0, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(-18.0, pDupSimi->GetYScaling());
    ASSERT_NEAR(0.0, pDupSimi->GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddHorizontalFlip(5.0);

    ASSERT_DOUBLE_EQ(-30.0, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(50.00, pDupSimi->GetTranslation().GetDeltaY());
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
TEST_F(HGF2DProjectiveTester, ConversionTest)
    {

    // Direct conversions
    double XCoord = 10.0;
    double YCoord = 10.0;

    DefaultProjective.ConvertDirect(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    double XConverted;
    double YConverted;

    DefaultProjective.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    double XArray[10];
    double YArray[10];
    DefaultProjective.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultProjective.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    // Inverse conversions
    DefaultProjective.ConvertInverse(&XCoord, &YCoord);
    ASSERT_DOUBLE_EQ(10.0, XCoord);
    ASSERT_DOUBLE_EQ(10.0, YCoord);

    DefaultProjective.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_DOUBLE_EQ(10.0, XConverted);
    ASSERT_DOUBLE_EQ(10.0, YConverted);

    DefaultProjective.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    DefaultProjective.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_DOUBLE_EQ(10.0, XResultDist);
    ASSERT_DOUBLE_EQ(10.0, YResultDist);

    }

TEST_F(HGF2DProjectiveTester, ConversionInOutArraysTest)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    DefaultProjective.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
    DefaultProjective.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
TEST_F(HGF2DProjectiveTester, ConstructiontTest3)
    {

    // Create duplicates for compare operations
    HGF2DProjective Projective3(Projective2);
    HGF2DProjective Projective4(Projective2);
    Projective4.Reverse();

    // Check transformation properties
    ASSERT_FALSE(Projective2.IsIdentity());
    ASSERT_FALSE(Projective2.IsStretchable());
    ASSERT_TRUE(Projective2.CanBeRepresentedByAMatrix());

    // Extract stretch parameters
    Projective2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Extract stretch parameters
    Projective2.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 1000.0, 1000.0);
    ASSERT_DOUBLE_EQ(2.0, ScalingX);
    ASSERT_DOUBLE_EQ(2.0, ScalingY);

    // Check value of displacement
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Translation.GetDeltaY());

    // Check type of model
    ASSERT_EQ(HGF2DProjective::CLASS_ID, Projective2.GetClassID());

    // Extract matrix parameters
    HFCMatrix<3, 3> TheMatrix1 = Projective2.GetMatrix();

    ASSERT_DOUBLE_EQ(10.0000000000000000, TheMatrix1[0][2]);
    ASSERT_DOUBLE_EQ(10.0000000000000000, TheMatrix1[1][2]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, TheMatrix1[0][0]);
    ASSERT_DOUBLE_EQ(-1.4142135623730951, TheMatrix1[0][1]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, TheMatrix1[1][0]);
    ASSERT_DOUBLE_EQ(1.41421356237309510, TheMatrix1[1][1]);

    // Check clonage
    HFCPtr<HGF2DTransfoModel> pModelClone = Projective2.Clone();

    // Check nature of clone
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pModelClone->GetClassID());

    // Check transformation properties of clone
    ASSERT_FALSE(pModelClone->IsIdentity());
    ASSERT_FALSE(pModelClone->IsStretchable());
    ASSERT_TRUE(pModelClone->CanBeRepresentedByAMatrix());

    // Check geometric properties
    ASSERT_TRUE(Projective2.PreservesLinearity());
    ASSERT_TRUE(Projective2.PreservesParallelism());
    ASSERT_TRUE(Projective2.PreservesShape());
    ASSERT_FALSE(Projective2.PreservesDirection());

    // Reversal test
    Projective2.Reverse();

    // Check again transformation properties
    ASSERT_FALSE(Projective2.IsIdentity());
    ASSERT_FALSE(Projective2.IsStretchable());
    ASSERT_TRUE(Projective2.CanBeRepresentedByAMatrix());

    // Extract again stretch parameters
    Projective2.GetStretchParams(&ScalingX, &ScalingY, &Translation);
    ASSERT_DOUBLE_EQ(0.5, ScalingX);
    ASSERT_DOUBLE_EQ(0.5, ScalingY);

    // Check again value of displacement
    ASSERT_DOUBLE_EQ(-7.0710678118654755, Translation.GetDeltaX());
    ASSERT_NEAR(0.0, Translation.GetDeltaY(), MYEPSILON);

    Projective2.Reverse();

    }

//==================================================================================
// CompositionIdentityTest3
//==================================================================================
TEST_F(HGF2DProjectiveTester, CompositionIdentityTest3)
    {

    // Composition with similitude
    HFCPtr<HGF2DTransfoModel> pIdenCompSimi = MyIdentity.ComposeInverseWithDirectOf(Projective2);

    // The result should be a projective
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi->GetClassID());

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
TEST_F(HGF2DProjectiveTester, CompositionSimilitudeTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi2 = MyIdentity.ComposeInverseWithInverseOf(Projective2);

    // The result should be a similitude
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi2->GetClassID());

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
TEST_F(HGF2DProjectiveTester, CompositionSimilitudeWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi5 = Projective2.ComposeInverseWithDirectOf(MyIdentity);

    // The result should be a projective
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi5->GetClassID());

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
TEST_F(HGF2DProjectiveTester, CompositionSimilitudeReversalWithIdentityTest3)
    {

    HFCPtr<HGF2DTransfoModel> pIdenCompSimi6 = Projective2.ComposeInverseWithInverseOf(MyIdentity);

    // The result should be a projective
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pIdenCompSimi6->GetClassID());

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
TEST_F(HGF2DProjectiveTester, TranslationExtractionTest3)
    {

    HGF2DDisplacement MyTranslation = Projective2.GetTranslation();

    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, MyTranslation.GetDeltaY());

    // Scale extraction test
    ASSERT_DOUBLE_EQ(2.0, Projective2.GetXScaling());
    ASSERT_DOUBLE_EQ(2.0, Projective2.GetYScaling());

    // Extraction of rotation
    ASSERT_DOUBLE_EQ(PI/4, Projective2.GetRotation());

    // Create a clone of default projective
    HFCPtr<HGF2DProjective> pDupSimi = (HGF2DProjective*)Projective2.Clone();

    // Create translation tools
    HGF2DDisplacement Trans1(0.0, 1.0);
    HGF2DDisplacement Trans2(1.0, 1.0);

    // Add translation
    pDupSimi->AddTranslation(Trans2);

    // Check result of setting
    ASSERT_DOUBLE_EQ(11.0000000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(11.0000000000000000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(2.00000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(2.00000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(0.78539816339744828, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    // Add scaling
    pDupSimi->AddIsotropicScaling(2.0);

    // Notice that addition of scaling modifies translation component
    ASSERT_DOUBLE_EQ(22.0000000000000000, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(22.0000000000000000, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.00000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(4.00000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(0.78539816339744828, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    // Add rotation
    pDupSimi->AddRotation(PI / 4.0);

    ASSERT_NEAR(0.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(31.112698372208090, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(4.0000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(4.0000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(1.5707963267948963, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    // Addition of components with offset
    pDupSimi->AddIsotropicScaling(2.5, 10.0, 10.0);

    ASSERT_NEAR(-15.0, pDupSimi->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(62.781745930520231, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(10.000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(10.000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(1.5707963267948963, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    pDupSimi->AddRotation(PI / 4, 10.0, 10.0);

    ASSERT_DOUBLE_EQ(-44.9999999999999930, pDupSimi->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(29.64466094067263700, pDupSimi->GetTranslation().GetDeltaY());
    ASSERT_DOUBLE_EQ(-10.0000000000000000, pDupSimi->GetXScaling());
    ASSERT_DOUBLE_EQ(-10.0000000000000000, pDupSimi->GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744861, pDupSimi->GetRotation());
    ASSERT_NEAR(0.0, pDupSimi->GetAnorthogonality(), MYEPSILON);

    }

//==================================================================================
// ConversionTestTest3
//==================================================================================
TEST_F(HGF2DProjectiveTester, ConversionTestTest3)
    {

    double XCoord = 10.0;
    double YCoord = 10.0;

    Projective2.ConvertDirect(&XCoord, &YCoord);
    ASSERT_NEAR(10.000000000000000, XCoord, MYEPSILON);
    ASSERT_NEAR(38.284271247461902, YCoord, MYEPSILON);

    double XConverted;
    double YConverted;
    XCoord = 10.0;
    YCoord = 10.0;

    Projective2.ConvertDirect(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(10.000000000000000, XConverted, MYEPSILON);
    ASSERT_NEAR(38.284271247461902, YConverted, MYEPSILON);

    double XArray[10];
    double YArray[10];
    Projective2.ConvertDirect(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    Projective2.ConvertDirect(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(10.000000000000000, XResultDist, MYEPSILON);
    ASSERT_NEAR(38.284271247461902, YResultDist, MYEPSILON);

    // Inverse conversions
    XCoord = 10.0;
    YCoord = 10.0;
    Projective2.ConvertInverse(&XCoord, &YCoord);
    ASSERT_NEAR(0.0, XCoord, MYEPSILON);
    ASSERT_NEAR(0.0, YCoord, MYEPSILON);

    XCoord = 10.0;
    YCoord = 10.0;

    Projective2.ConvertInverse(XCoord, YCoord, &XConverted, &YConverted);
    ASSERT_NEAR(0.0, XConverted, MYEPSILON);
    ASSERT_NEAR(0.0, YConverted, MYEPSILON);

    Projective2.ConvertInverse(YCoord, 0.0, 10, 1.0, XArray, YArray);
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

    Projective2.ConvertInverse(XDist, YDist, &XResultDist, &YResultDist);
    ASSERT_NEAR(0.0, XResultDist, MYEPSILON);
    ASSERT_NEAR(0.0, YResultDist, MYEPSILON);

    }

TEST_F(HGF2DProjectiveTester, ConversionInOutArraysTest3)
    {
    const int NUMBER_OF_POINTS = 10;
    double XArrayInOut[NUMBER_OF_POINTS];
    double YArrayInOut[NUMBER_OF_POINTS];
    for(uint32_t i = 0; i < NUMBER_OF_POINTS; i++)
        {
        XArrayInOut[i] = i;
        YArrayInOut[i] = 10.0;
        }

    Projective2.ConvertDirect(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
    Projective2.ConvertInverse(NUMBER_OF_POINTS, XArrayInOut, YArrayInOut);
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
TEST_F(HGF2DProjectiveTester, SpecialReversalTest)
    {

    // Reversal of a similitude with rotation
    HGF2DProjective Projective1;
    Projective1.AddRotation(PI / 4);

    Projective1.Reverse();

    ASSERT_DOUBLE_EQ(1.000000000000000000, Projective1.GetXScaling());
    ASSERT_DOUBLE_EQ(1.000000000000000000, Projective1.GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Projective1.GetRotation());
    ASSERT_NEAR(0.0, Projective1.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Projective1.GetTranslation().GetDeltaY(), MYEPSILON);

    // Projective with translation only
    HGF2DProjective Projective2;
    Projective2.AddTranslation(HGF2DDisplacement(-10.0,-10.0));

    Projective2.Reverse();

    ASSERT_DOUBLE_EQ(1.00, Projective2.GetXScaling());
    ASSERT_DOUBLE_EQ(1.00, Projective2.GetYScaling());
    ASSERT_NEAR(0.0, Projective2.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(10.0, Projective2.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, Projective2.GetTranslation().GetDeltaY());

    // Test with scaling only
    HGF2DProjective Projective3;
    Projective3.AddIsotropicScaling(0.25);

    Projective3.Reverse();

    ASSERT_DOUBLE_EQ(4.0, Projective3.GetXScaling());
    ASSERT_DOUBLE_EQ(4.0, Projective3.GetYScaling());
    ASSERT_NEAR(0.0, Projective3.GetRotation(), MYEPSILON);
    ASSERT_NEAR(0.0, Projective3.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Projective3.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both rotation and translation
    HGF2DProjective Projective4;
    Projective4.AddRotation(PI / 4);
    Projective4.AddTranslation(HGF2DDisplacement(-10.0, -10.0));

    Projective4.Reverse();

    ASSERT_DOUBLE_EQ(1.000000000000000000, Projective4.GetXScaling());
    ASSERT_DOUBLE_EQ(1.000000000000000000, Projective4.GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Projective4.GetRotation());
    ASSERT_DOUBLE_EQ(14.14213562373095100, Projective4.GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, Projective4.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both scaling and translation
    HGF2DProjective Projective5;
    Projective5.AddIsotropicScaling(0.25);
    Projective5.AddTranslation(HGF2DDisplacement(-10.0, -10.0));
    Projective5.Reverse();

    ASSERT_DOUBLE_EQ(4.00, Projective5.GetXScaling());
    ASSERT_DOUBLE_EQ(4.00, Projective5.GetYScaling());
    ASSERT_NEAR(0.0, Projective5.GetRotation(), MYEPSILON);
    ASSERT_DOUBLE_EQ(40.0, Projective5.GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(40.0, Projective5.GetTranslation().GetDeltaY());

    // Test with both scaling and rotation
    HGF2DProjective Projective6;
    Projective6.AddIsotropicScaling(0.25);
    Projective6.AddRotation(PI / 4);

    Projective6.Reverse();

    ASSERT_DOUBLE_EQ(4.000000000000000000, Projective6.GetXScaling());
    ASSERT_DOUBLE_EQ(4.000000000000000000, Projective6.GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Projective6.GetRotation());
    ASSERT_NEAR(0.0, Projective6.GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, Projective6.GetTranslation().GetDeltaY(), MYEPSILON);

    // Test with both scaling and rotation and translation
    HGF2DProjective Projective7;
    Projective7.AddIsotropicScaling(0.25);
    Projective7.AddRotation(PI / 4);
    Projective7.AddTranslation(HGF2DDisplacement(-10.0, -10.0));

    Projective7.Reverse();

    ASSERT_DOUBLE_EQ(4.000000000000000000, Projective7.GetXScaling());
    ASSERT_DOUBLE_EQ(4.000000000000000000, Projective7.GetYScaling());
    ASSERT_DOUBLE_EQ(-0.78539816339744828, Projective7.GetRotation());
    ASSERT_DOUBLE_EQ(56.56854249492380400, Projective7.GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, Projective7.GetTranslation().GetDeltaY(), MYEPSILON);

    }

//==================================================================================
// SpecialCompositionTest
//==================================================================================
TEST_F(HGF2DProjectiveTester, SpecialCompositionTest)
    {

    HGF2DProjective Projective1;
    Projective1.AddIsotropicScaling(2.1);
    Projective1.AddRotation(PI / 4);
    Projective1.AddTranslation(HGF2DDisplacement(-10.0, -10.0));

    HGF2DProjective Projective2;
    Projective2.AddIsotropicScaling(-632.1);
    Projective2.AddRotation(PI / 3);
    Projective2.AddTranslation(HGF2DDisplacement(1560.0, -1045.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1and2 = Projective1.ComposeInverseWithDirectOf(Projective2);
    HFCPtr<HGF2DProjective> pProjRes = (HGF2DProjective*)pNewModel1and2->Clone();

    HFCMatrix<3, 3> TheMatrix1 = pProjRes->GetMatrix();
    ASSERT_NEAR(-753.64657732143496, TheMatrix1[0][2], 10E-12);
    ASSERT_NEAR(7589.64657732143680, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(343.558988659536850, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(1282.17960107437240, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-1282.1796010743724, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(343.558988659536850, TheMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel1andI2 = Projective1.ComposeInverseWithInverseOf(Projective2);
    HFCPtr<HGF2DProjective> pProjRes2 = (HGF2DProjective*)pNewModel1andI2->Clone();

    TheMatrix1 = pProjRes2->GetMatrix();
    ASSERT_NEAR(-0.17613715063580726000, TheMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-2.96971979740795570000, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(-0.00320905590129258630, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-0.00085986393721767661, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(0.000859863937217676610, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(-0.00320905590129258630, TheMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel2and1 = Projective2.ComposeInverseWithDirectOf(Projective1);
    HFCPtr<HGF2DProjective> pProjRes3 = (HGF2DProjective*)pNewModel2and1->Clone();

    TheMatrix1 = pProjRes3->GetMatrix();
    ASSERT_NEAR(3858.22764648100840, TheMatrix1[0][2], 10E-12);
    ASSERT_NEAR(754.735983853251130, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(343.558988659536850, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(1282.17960107437240, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-1282.1796010743724, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(343.558988659536850, TheMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel2andI1 = Projective2.ComposeInverseWithInverseOf(Projective1);
    HFCPtr<HGF2DProjective> pProjRes4 = (HGF2DProjective*)pNewModel2andI1->Clone();

    TheMatrix1 = pProjRes4->GetMatrix();
    ASSERT_NEAR(180.143870445144360, TheMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-877.14912618616972, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(-290.74367371300951, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(77.9045325758587100, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-77.904532575858710, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(-290.74367371300951, TheMatrix1[1][1], 10E-12);

    // Composition with translation
    HGF2DTranslation TransModel;
    TransModel.SetTranslation(HGF2DDisplacement(10.0, 11.0));

    HFCPtr<HGF2DTransfoModel> pNewModel1andTrans = Projective1.ComposeInverseWithDirectOf(TransModel);
    HFCPtr<HGF2DProjective> pProjRes5 = (HGF2DProjective*)pNewModel1andTrans->Clone();

    TheMatrix1 = pProjRes5->GetMatrix();
    ASSERT_NEAR(0.0, TheMatrix1[0][2], MYEPSILON);
    ASSERT_NEAR(1.00000000000000000, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-1.4849242404917500, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModel1andITrans = Projective1.ComposeInverseWithInverseOf(TransModel);
    HFCPtr<HGF2DProjective> pProjRes6 = (HGF2DProjective*)pNewModel1andITrans->Clone();

    TheMatrix1 = pProjRes6->GetMatrix();
    ASSERT_NEAR(-20.000000000000000, TheMatrix1[0][2], 10E-12);
    ASSERT_NEAR(-21.000000000000000, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-1.4849242404917500, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModelTransand1 = TransModel.ComposeInverseWithDirectOf(Projective1);
    HFCPtr<HGF2DProjective> pProjRes7 = (HGF2DProjective*)pNewModelTransand1->Clone();

    TheMatrix1 = pProjRes7->GetMatrix();
    ASSERT_NEAR(-11.484924240491749, TheMatrix1[0][2], 10E-12);
    ASSERT_NEAR(21.1834090503267480, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(-1.4849242404917500, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(1.48492424049175000, TheMatrix1[1][1], 10E-12);

    HFCPtr<HGF2DTransfoModel> pNewModelTransandI1 = TransModel.ComposeInverseWithInverseOf(Projective1);
    HFCPtr<HGF2DProjective> pProjRes8 = (HGF2DProjective*)pNewModelTransandI1->Clone();

    TheMatrix1 = pProjRes8->GetMatrix();
    ASSERT_NEAR(13.80541810888021500, TheMatrix1[0][2], 10E-12);
    ASSERT_NEAR(0.336717514850737700, TheMatrix1[1][2], 10E-12);
    ASSERT_NEAR(0.336717514850736920, TheMatrix1[0][0], 10E-12);
    ASSERT_NEAR(0.336717514850736870, TheMatrix1[0][1], 10E-12);
    ASSERT_NEAR(-0.33671751485073687, TheMatrix1[1][0], 10E-12);
    ASSERT_NEAR(0.336717514850736920, TheMatrix1[1][1], 10E-12);

    }

//==================================================================================
// IsStretchableTest
//==================================================================================
TEST_F(HGF2DProjectiveTester, IsStretchableTest)
    {

    HFCMatrix<3, 3> TheMatrix1;

    TheMatrix1[0][0] = -1.0;
    TheMatrix1[0][1] = 0.0;
    TheMatrix1[0][2] = 448.0;
    TheMatrix1[1][0] = 0.0;
    TheMatrix1[1][1] = 1.0;
    TheMatrix1[1][2] = 0.0;
    TheMatrix1[2][2] = 1.0;

    HGF2DProjective TheProj(TheMatrix1);

    ASSERT_TRUE(TheProj.IsStretchable());

    TheProj.GetStretchParams(&ScalingX, &ScalingY, &Translation);

    ASSERT_DOUBLE_EQ(-1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.00, ScalingY);

    TheProj.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 1000.0, 1000.0);

    ASSERT_DOUBLE_EQ(-1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.00, ScalingY);

    }

//==================================================================================
// IsStretchableTest2
//==================================================================================
TEST_F(HGF2DProjectiveTester, IsStretchableTest2)
    {

    HFCMatrix<3, 3> TheMatrix1;

    TheMatrix1[0][0] = -1;
    TheMatrix1[0][1] = 0;
    TheMatrix1[0][2] = 448.0;
    TheMatrix1[1][0] = 0;
    TheMatrix1[1][1] = 1;
    TheMatrix1[1][2] = 0.0000001;
    TheMatrix1[2][0] = 0.00000013;
    TheMatrix1[2][1] = 0.0;
    TheMatrix1[2][2] = 1.0;

    HGF2DProjective TheProj(TheMatrix1);

    ASSERT_FALSE(TheProj.IsStretchable());

    TheProj.GetStretchParams(&ScalingX, &ScalingY, &Translation);

    ASSERT_DOUBLE_EQ(-1.0, ScalingX);
    ASSERT_DOUBLE_EQ(1.00, ScalingY);

    TheProj.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 0.0, 0.0);

    ASSERT_NEAR(-1.0000582364000365, ScalingX, MYEPSILON);
    ASSERT_NEAR(1.00000000000000000, ScalingY, MYEPSILON);

    TheProj.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 1000.0, 1000.0);

    ASSERT_NEAR(-0.99979827999851612, ScalingX, MYEPSILON);
    ASSERT_NEAR(0.999870013629333810, ScalingY, MYEPSILON);

    }

//==================================================================================
// IsStretchableTest3
//==================================================================================
TEST_F(HGF2DProjectiveTester, IsStretchableTest3)
    {

    HFCMatrix<3, 3> TheMatrix1;

    TheMatrix1[0][0] = -0.7071;
    TheMatrix1[0][1] = 0.7071;
    TheMatrix1[0][2] = 448.0;
    TheMatrix1[1][0] = 0.7071;
    TheMatrix1[1][1] = 0.7071;
    TheMatrix1[1][2] = 0.0000001;
    TheMatrix1[2][0] = 0.00000013;
    TheMatrix1[2][1] = 0.0;
    TheMatrix1[2][2] = 1.0;

    HGF2DProjective TheProj(TheMatrix1);

    ASSERT_FALSE(TheProj.IsStretchable());

    TheProj.GetStretchParams(&ScalingX, &ScalingY, &Translation);

    ASSERT_DOUBLE_EQ(-0.99999040995401534, ScalingX);
    ASSERT_DOUBLE_EQ(0.999990409954015340, ScalingY);

    TheProj.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 0.0, 0.0, 0.01);

    ASSERT_DOUBLE_EQ(-1.0000315927040342, ScalingX);
    ASSERT_DOUBLE_EQ(0.99999040995375210, ScalingY);

    TheProj.GetStretchParamsAt(&ScalingX, &ScalingY, &Translation, 1000.0, 1000.0);

    ASSERT_DOUBLE_EQ(-0.99977163243554656, ScalingX);
    ASSERT_DOUBLE_EQ(0.999860415180864770, ScalingY);

    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  5/2016
//----------------------------------------------------------------------------------------
TEST_F (HGF2DProjectiveTester, ProjectiveReverse)
    {
    double matrix[3][3];
    matrix[0][0] = 5.5147648962581428;
    matrix[0][1] = 0.0042851133851941970;
    matrix[0][2] = -7523.2436222951546;
    matrix[1][0] = -0.0050683470067062007;
    matrix[1][1] = 5.4906630316330816;
    matrix[1][2] = -41556.362377641810;
    matrix[2][0] = -1.4525233209147976e-010;
    matrix[2][1] = -6.5392539244920719e-008;
    matrix[2][2] = 1.0000000000000000;

    HFCPtr<HGF2DProjective> pProjec = new HGF2DProjective(matrix);
    HFCPtr<HGF2DProjective> pProjecInverse = static_cast<HGF2DProjective*>(pProjec->Clone());
    pProjecInverse->Reverse();

    // Compute coordinates of four corners
    HGF2DPosition BottomLeft(0, 0);
    HGF2DPosition TopLeft(0, 256);
    HGF2DPosition TopRight(512, 256);
    HGF2DPosition BottomRight(512, 0);

    pProjec->ConvertPosInverse(&BottomLeft);
    pProjec->ConvertPosInverse(&TopLeft);
    pProjec->ConvertPosInverse(&TopRight);
    pProjec->ConvertPosInverse(&BottomRight);

    HGF2DPosition BottomLeft2(0, 0);
    HGF2DPosition TopLeft2(0, 256);
    HGF2DPosition TopRight2(512, 256);
    HGF2DPosition BottomRight2(512, 0);

    pProjecInverse->ConvertPosDirect(&BottomLeft2);
    pProjecInverse->ConvertPosDirect(&TopLeft2);
    pProjecInverse->ConvertPosDirect(&TopRight2);
    pProjecInverse->ConvertPosDirect(&BottomRight2);

    ASSERT_DOUBLE_EQ(BottomLeft.GetX(), BottomLeft2.GetX());
    ASSERT_DOUBLE_EQ(BottomLeft.GetY(), BottomLeft2.GetY());
    ASSERT_DOUBLE_EQ(TopLeft.GetX(), TopLeft2.GetX());
    ASSERT_DOUBLE_EQ(TopLeft.GetY(), TopLeft2.GetY());
    ASSERT_DOUBLE_EQ(TopRight.GetX(), TopRight2.GetX());
    ASSERT_DOUBLE_EQ(TopRight.GetY(), TopRight2.GetY());
    ASSERT_DOUBLE_EQ(BottomRight.GetX(), BottomRight2.GetX());
    ASSERT_DOUBLE_EQ(BottomRight.GetY(), BottomRight2.GetY());
    }