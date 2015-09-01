//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/HGFModelSimplificationTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "../imagepptestpch.h"
#include "HGFModelSimplificationTester.h"

HGFModelSimplificationTester::HGFModelSimplificationTester() 
    {

    //Similitude
    Similitude6 = HGF2DSimilitude(HGF2DDisplacement(2.0, 0.0), (56 * PI/180), 3.0);

    //Stretch
    Stretch2 = HGF2DStretch(HGF2DDisplacement(0.0, 0.3), 1.0, 1.0);

    // Projective with perspective
    Projective10;
    TheMatrix[0][0] = 1.0;
    TheMatrix[0][1] = 0.0;
    TheMatrix[0][2] = 0.0;
    TheMatrix[1][0] = 0.0;
    TheMatrix[1][1] = 1.0;
    TheMatrix[1][2] = 0.0;
    TheMatrix[2][0] = 3.0;
    TheMatrix[2][1] = 4.0;
    TheMatrix[2][2] = 1.0;
    Projective10.SetByMatrix(TheMatrix);

    }

//==================================================================================
// TranslationCreateSimplifiedModel
//==================================================================================
TEST_F(HGFModelSimplificationTester, TranslationCreateSimplifiedModel)
    {

    // empty translation
    HGF2DTranslation  Translation1(HGF2DDisplacement(0.0, 0.0));

    // Translation
    HGF2DTranslation  Translation2(HGF2DDisplacement(1.2, 0.0));

    // This one should give an identity
    pResult = Translation1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult->GetClassID());

    // This one should not be simplified
    pResult = Translation2.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    }

//==================================================================================
// SimilitudeCreateSimplifiedModel
//==================================================================================
TEST_F(HGFModelSimplificationTester, SimilitudeCreateSimplifiedModel)
    {

    // Similitude with only a translation
    HGF2DSimilitude   Similitude2(HGF2DDisplacement(0.0, 0.3), 0.0, 1.0);

    // Similitude with only a rotation
    HGF2DSimilitude   Similitude3(HGF2DDisplacement(0.0, 0.0), (5 * PI/180), 1.0);

    // Similitude with only a scaling
    HGF2DSimilitude   Similitude4(HGF2DDisplacement(0.0, 0.0), 0.0, 3.0);

    // Similitude with scaling and translation
    HGF2DSimilitude   Similitude5(HGF2DDisplacement(2.0, 0.0), 0.0, 3.0);

    // This one should give an identity
    pResult = Similitude1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult->GetClassID());

    // This one should give a translation
    pResult = Similitude2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pResult->GetClassID());

    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.3, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaY());

    // This one should not simplify
    pResult = Similitude3.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    // This one should give a stretch
    pResult = Similitude4.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetYScaling());

    // This one should give a stretch
    pResult = Similitude5.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetYScaling());

    // This one should not simplify
    pResult = Similitude6.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    }

//==================================================================================
// StretchCreateSimplifiedModel
//==================================================================================
TEST_F(HGFModelSimplificationTester, StretchCreateSimplifiedModel)
    {

    // Empty stretch
    HGF2DStretch      Stretch1;

    // Stretch with only a scaling
    HGF2DStretch      Stretch3(HGF2DDisplacement(0.0, 0.0), 1.0, 2.3);

    // Stretch with everything
    HGF2DStretch      Stretch4(HGF2DDisplacement(4.0, 34251.324), 1.0, 2.3);

    // This one should give an identity
    pResult = Stretch1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult->GetClassID());

    // This one should give a translation
    pResult = Stretch2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pResult->GetClassID());

    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.3, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaY());

    // This one should not simplify
    pResult = Stretch3.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    // This one should not simplify
    pResult = Stretch4.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    }

//==================================================================================
// AffineCreateSimplifiedModel
//==================================================================================
TEST_F(HGFModelSimplificationTester, AffineCreateSimplifiedModel)
    {

    // Empty affine
    HGF2DAffine       Affine1;

    // Affine with translation
    HGF2DAffine       Affine2(HGF2DDisplacement(0.0, 0.3), 0.0, 1.0, 1.0, 0.0);

    // Affine with rotation
    HGF2DAffine       Affine3(HGF2DDisplacement(0.0, 0.0), (10 * PI/180), 1.0, 1.0, 0.0);

    // Affine with anorthogonality
    HGF2DAffine       Affine4(HGF2DDisplacement(0.0, 0.0), 0.0, 1.0, 1.0, (10 * PI/180));

    // Affine with scaling
    HGF2DAffine       Affine5(HGF2DDisplacement(0.0, 0.0), 0.0, 2.0, 3.0, 0.0);

    // Affine with scaling amd rotation
    HGF2DAffine       Affine6(HGF2DDisplacement(0.0, 0.0), (15 * PI/180), 2.0, 2.0, 0.0);

    // Affine with anisotropic scaling amd rotation
    HGF2DAffine       Affine7(HGF2DDisplacement(0.0, 0.0), (15 * PI/180), 2.0, 3.0, 0.0);

    // Affine with scaling, rotation and translation
    HGF2DAffine       Affine8(HGF2DDisplacement(5.0, 0.0), (7 * PI/180), 2.0, 2.0, 0.0);

    // Affine with anisotropic scaling, rotation and translation
    HGF2DAffine       Affine9(HGF2DDisplacement(5.0, 0.0), (24 * PI/180), 2.0, 4.0, 0.0);

    // Full Affine
    HGF2DAffine       Affine10(HGF2DDisplacement(5.0, 0.0), (15 * PI/180) , 2.0, 4.0, (42 * PI/180));

    // This one should give an identity
    pResult = Affine1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult->GetClassID());

    // This one should give a translation
    pResult = Affine2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pResult->GetClassID());

    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.3, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaY()); 

    // This one should give a similitude
    pResult = Affine3.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((10 * PI/180), ((HFCPtr<HGF2DSimilitude>&)pResult)->GetRotation());

    // This one should not simplify
    pResult = Affine4.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    // This one should give a stretch
    pResult = Affine5.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ(2.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetYScaling());   

    // This one should give a similitude
    pResult = Affine6.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((15 * PI/180), ((HFCPtr<HGF2DSimilitude>&)pResult)->GetRotation());
    ASSERT_DOUBLE_EQ(2.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetScaling());

    // This one should not simplify
    pResult = Affine7.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    // This one should give a similitude
    pResult = Affine8.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((7 * PI/180), ((HFCPtr<HGF2DSimilitude>&)pResult)->GetRotation());
    ASSERT_DOUBLE_EQ(2.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetScaling());
    ASSERT_DOUBLE_EQ(5.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetTranslation().GetDeltaY(), MYEPSILON);

    // This one should not simplify
    pResult = Affine9.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    // This one should not simplify
    pResult = Affine10.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    }

//==================================================================================
// ProjectiveCreateSimplifiedModel
//==================================================================================
TEST_F(HGFModelSimplificationTester, ProjectiveCreateSimplifiedModel)
    {

    // Empty projective
    HGF2DProjective   Projective1;

    // Projective with translation
    HGF2DProjective   Projective2(HGF2DDisplacement(0.0, 0.3), 0.0, 1.0, 1.0, 0.0);

    // Projective with rotation
    HGF2DProjective   Projective3(HGF2DDisplacement(0.0, 0.0), (10 * PI/180), 1.0, 1.0, 0.0);

    // Projective with anorthogonality
    HGF2DProjective   Projective4(HGF2DDisplacement(0.0, 0.0), 0.0, 1.0, 1.0, (10 * PI/180));

    // Projective with scaling
    HGF2DProjective   Projective5(HGF2DDisplacement(0.0, 0.0), 0.0, 3.0, 3.0, 0.0);

    // Projective with scaling and rotation
    HGF2DProjective   Projective6(HGF2DDisplacement(0.0, 0.0), (10 * PI/180), 3.0, 3.0, 0.0);

    // Projective with anisotropic scaling and rotation
    HGF2DProjective   Projective7(HGF2DDisplacement(0.0, 0.0), (10 * PI/180), 2.0, 3.0, 0.0);

    // Projective with scaling, rotation and translation
    HGF2DProjective   Projective8(HGF2DDisplacement(5.0, 0.0), (10 * PI/180), 3.0, 3.0, 0.0);

    // Projective with anisotropic scaling, rotation and translation
    HGF2DProjective   Projective9(HGF2DDisplacement(5.0, 10.0), (10 * PI/180), 2.0, 3.0, 0.0);

    // This one should give an identity
    pResult = Projective1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult->GetClassID());

    // This one should give a translation
    pResult = Projective2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DTranslation::CLASS_ID, pResult->GetClassID());

    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_DOUBLE_EQ(0.3, ((HFCPtr<HGF2DTranslation>&)pResult)->GetTranslation().GetDeltaY());

    // This one should give a similitude
    pResult = Projective3.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((10 * PI/180), ((HFCPtr<HGF2DSimilitude>&)pResult)->GetRotation());

    // This one should give an affine
    pResult = Projective4.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((10 * PI/180), ((HFCPtr<HGF2DAffine>&)pResult)->GetAnorthogonality());

    // This one should give a stretch
    pResult = Projective5.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DStretch::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DStretch>&)pResult)->GetYScaling());

    // This one should give a similitude
    pResult = Projective6.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((10 * PI/180), ((HFCPtr<HGF2DSimilitude>&)pResult)->GetRotation());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetScaling());

    // This one should give an affine
    pResult = Projective7.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((10 * PI/180), ((HFCPtr<HGF2DAffine>&)pResult)->GetRotation());
    ASSERT_DOUBLE_EQ(2.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetYScaling());
    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetTranslation().GetDeltaX(), MYEPSILON);
    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetTranslation().GetDeltaX(), MYEPSILON);

    // This one should give a similitude
    pResult = Projective8.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((10 * PI/180), ((HFCPtr<HGF2DSimilitude>&)pResult)->GetRotation());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetScaling());
    ASSERT_DOUBLE_EQ(5.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetTranslation().GetDeltaX());
    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DSimilitude>&)pResult)->GetTranslation().GetDeltaY(), MYEPSILON);

    // This one should give an affine
    pResult = Projective9.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DAffine::CLASS_ID, pResult->GetClassID());

    ASSERT_DOUBLE_EQ((10 * PI/180), ((HFCPtr<HGF2DAffine>&)pResult)->GetRotation());
    ASSERT_DOUBLE_EQ(2.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetXScaling());
    ASSERT_DOUBLE_EQ(3.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetYScaling());
    ASSERT_DOUBLE_EQ(5.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetTranslation().GetDeltaX());
    ASSERT_DOUBLE_EQ(10.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetTranslation().GetDeltaY());
    ASSERT_NEAR(0.0, ((HFCPtr<HGF2DAffine>&)pResult)->GetAnorthogonality(), MYEPSILON);

    // This one should not simplify
    pResult = Projective10.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    }

//==================================================================================
// ComplexTransfoModelCreateSimplifiedModel
//==================================================================================
TEST_F(HGFModelSimplificationTester, ComplexTransfoModelCreateSimplifiedModel)
    { 

    // Empty complex
    HGF2DComplexTransfoModel Complex1;

    // Complex with one model (not simplifiable)
    HGF2DComplexTransfoModel Complex2;
    Complex2.AddModel(Similitude6);

    // Complex with one model (simplifiable)
    HGF2DComplexTransfoModel Complex3;
    Complex3.AddModel(Similitude1);

    // Complex with two models (not simplifiable)
    HGF2DComplexTransfoModel Complex4;
    Complex4.AddModel(Similitude6);
    Complex4.AddModel(Projective10);

    // Complex with two models (simplifiable first)
    HGF2DComplexTransfoModel Complex5;
    Complex5.AddModel(Similitude1);
    Complex5.AddModel(Projective10);

    // Complex with two models (simplifiable second)
    HGF2DComplexTransfoModel Complex6;
    Complex6.AddModel(Projective10);
    Complex6.AddModel(Similitude1);

    // Complex with three models (simplifiable second)
    HGF2DComplexTransfoModel Complex7;
    Complex7.AddModel(Projective10);
    Complex7.AddModel(Similitude1);
    Complex7.AddModel(Stretch2);

    // Complex with three models (simplifiable third)
    HGF2DComplexTransfoModel Complex8;
    Complex8.AddModel(Projective10);
    Complex8.AddModel(Similitude6);
    Complex8.AddModel(Similitude1);

        // This one should give an identity
    pResult = Complex1.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult->GetClassID());

    // Should give the internal similitude back
    pResult = Complex2.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DSimilitude::CLASS_ID, pResult->GetClassID());

    // Should give an identity because internal similitude is simplifiable
    pResult = Complex3.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DIdentity::CLASS_ID, pResult->GetClassID());

    // not simplifiable
    pResult = Complex4.CreateSimplifiedModel();
    ASSERT_TRUE(pResult == 0);

    // Should give a projective (first model simplifies to identity)
    pResult = Complex5.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pResult->GetClassID());

    // Should give a projective (second model simplifies to identity)
    pResult = Complex6.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pResult->GetClassID());

    // Should give a projective because second simplifies to identity and
    // other (stretch) will be handled by projective
    pResult = Complex7.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pResult->GetClassID());

    // Should give a projective because third simplifies to identity and
    // other (similitude) will be handled by projective
    pResult = Complex8.CreateSimplifiedModel();
    ASSERT_EQ(HGF2DProjective::CLASS_ID, pResult->GetClassID());

    }

   







