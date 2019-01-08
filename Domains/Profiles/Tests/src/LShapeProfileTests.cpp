/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/LShapeProfileTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct LShapeProfileTestCase : ProfileValidationTestCase<LShapeProfile>
    {
public:
    typedef LShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "L");

    LShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "L", 10.0, 10.0, 1.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "L", 10.0, 10.0, 1.0, 0.5, 0.5, Angle::FromRadians (PI / 18.0));
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "L");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "L", 1.0, 2.0, 3.0, 4.0, 5.0, Angle::FromRadians (6.0));

    LShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("L", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetEdgeRadius());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetLegSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0, Angle::FromRadians (0.0));

    LShapeProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("L");
    profilePtr->SetWidth (1.0);
    profilePtr->SetDepth (2.0);
    profilePtr->SetThickness (3.0);
    profilePtr->SetFilletRadius (4.0);
    profilePtr->SetEdgeRadius (5.0);
    profilePtr->SetLegSlope (Angle::FromRadians (6.0));

    EXPECT_EQ ("L", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetEdgeRadius());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetLegSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, GetInnerFlangeFaceLength_WidthAndThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "L", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY));
    LShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetWidth (0.0);
    profilePtr->SetThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetInnerFlangeFaceLength());

    profilePtr->SetWidth (2.0);
    profilePtr->SetThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetInnerFlangeFaceLength());

    profilePtr->SetWidth (1.0);
    profilePtr->SetThickness (2.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetInnerFlangeFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, GetInnerWebFaceLength_DepthAndThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "L", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY));
    LShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (0.0);
    profilePtr->SetThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetInnerWebFaceLength());

    profilePtr->SetDepth (2.0);
    profilePtr->SetThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetInnerWebFaceLength());

    profilePtr->SetDepth (1.0);
    profilePtr->SetThickness (2.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetInnerWebFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, GetHorizontalLegSlopeHeight_WidthAndThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "L", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY));
    LShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetWidth (2.0);
    profilePtr->SetThickness (1.0);

    profilePtr->SetLegSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetHorizontalLegSlopeHeight());

    profilePtr->SetLegSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetHorizontalLegSlopeHeight());

    profilePtr->SetLegSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetHorizontalLegSlopeHeight());

    profilePtr->SetLegSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetHorizontalLegSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, GetVerticalLegSlopeHeight_DepthAndThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "L", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY));
    LShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (2.0);
    profilePtr->SetThickness (1.0);

    profilePtr->SetLegSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetVerticalLegSlopeHeight());

    profilePtr->SetLegSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetVerticalLegSlopeHeight());

    profilePtr->SetLegSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetVerticalLegSlopeHeight());

    profilePtr->SetLegSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetVerticalLegSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0);

    params.name = "L";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_InvalidWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "L", INFINITY, 10.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.width, "Width", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_ValidWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "L", INFINITY, 10.0, 1.0);

    params.width = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Width should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, INFINITY, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_InvalidThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.thickness, "Thickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_ThicknessEqualToWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, INFINITY);

    params.thickness = params.width;
    EXPECT_FAIL_Insert (params) << "Thickness should be less than the width.";

    params.thickness = BeNumerical::BeNextafter (params.width, 0.0);
    EXPECT_SUCCESS_Insert (params) << "Thickness should be less than the width.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_ThicknessEqualToDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, INFINITY);

    params.thickness = params.depth;
    EXPECT_FAIL_Insert (params) << "Thickness should be less than the depth.";

    params.thickness = BeNumerical::BeNextafter (params.depth, 0.0);
    EXPECT_SUCCESS_Insert (params) << "Thickness should be less than the depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_ValidThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, INFINITY);

    params.thickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlange_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "L_ZeroFlangeSlope_ShortFlange", 3.0, 100.0, 1.0, INFINITY);
    params.legSlope = Angle::FromRadians (0.0);

    LShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetInnerFlangeFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";

    params.filletRadius = BeNumerical::BeNextafter (1.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_FilletRadiusAgainstTheWeb_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "L_ZeroFlangeSlope_ShortWeb", 100.0, 3.0, 1.0, INFINITY);
    params.legSlope = Angle::FromRadians (0.0);

    LShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetInnerWebFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";

    params.filletRadius = BeNumerical::BeNextafter (1.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlangeWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "L_NonZeroFlangeSlope", 10.0, 20.0, 1.0, INFINITY);
    params.legSlope = Angle::FromRadians ((PI / 180.0) * 10.0);

    LShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForFlange = profilePtr->GetInnerFlangeFaceLength() / 2.0 - profilePtr->GetVerticalLegSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForFlange, 0.0) << "Web slope height cannot be greater than half of the inner flange face length";

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web slope height.";

    params.filletRadius = maximumFilletRadiusForFlange;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web slope height.";

    params.filletRadius = BeNumerical::BeNextafter (maximumFilletRadiusForFlange, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_FilletRadiusAgainstTheWebWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "L_NonZeroFlangeSlope", 20.0, 10.0, 1.0, INFINITY);
    params.legSlope = Angle::FromRadians ((PI / 180.0) * 10.0);

    LShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForWeb = profilePtr->GetInnerWebFaceLength() / 2.0 - profilePtr->GetHorizontalLegSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForWeb, 0.0) << "Flange slope height cannot be greater than half of the inner web face length";

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";

    params.filletRadius = maximumFilletRadiusForWeb;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";

    params.filletRadius = BeNumerical::BeNextafter (maximumFilletRadiusForWeb, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_InvalidEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, 1.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.edgeRadius, "EdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_EdgeRadiusAgainstThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, 1.0, 0.0, INFINITY);

    params.edgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of the thickness.";

    params.edgeRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to half of the thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_EdgeRadiusAgainstInnerFlangeFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, 1.0, 0.0, INFINITY);

    params.thickness = 4.0;
    params.edgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of the thickness.";

    params.thickness = 8.0;
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to half of inner flange face length.";

    params.edgeRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of inner flange face length.";

    params.edgeRadius = BeNumerical::BeNextafter (1.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to half of inner flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_InvalidLegSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "L", 10.0, 10.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.legSlope, "LegSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_LegSlopeOf30Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "L", 3.0, 3.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    double const angle = std::asin (1.0 / std::sqrt (5.0));
    params.legSlope = Angle::FromRadians (angle);
    LShapeProfilePtr profilePtr = CreateProfile (params);

    // Both leg face lengths is 2.0 so maximum alowed slope height should be half of that = 1.0
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetInnerFlangeFaceLength());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetInnerWebFaceLength());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetHorizontalLegSlopeHeight());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetVerticalLegSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Leg slope should be such, that the slope height should be less or equal to half of inner web and flange face lengths.";

    params.legSlope = Angle::FromRadians (BeNumerical::BeNextafter (angle, INFINITY));
    EXPECT_FAIL_Insert (params) << "Leg slope should be such, that the slope height should be less or equal to half of inner web and flange face lengths.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Insert_LegSlopeOf90Degrees_FailedInsert)
    {
    CreateParams params (GetModel(), "L", DBL_MAX, DBL_MAX, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.legSlope = Angle::FromRadians (0.0);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert.";

    params.legSlope = Angle::FromRadians (PI / 2.0);
    EXPECT_FAIL_Insert (params) << "Leg slope should be less than 90 degrees.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (LShapeProfileTestCase, Delete_ExistingDoubleLInstances_FailedDelete)
    {
    LShapeProfilePtr singleProfilePtr = InsertElement<LShapeProfile> (CreateParams (GetModel(), "L", 6.0, 10.0, 1.0));

    DoubleLShapeProfile::CreateParams doubleLParams (GetModel(), "DL", 1.0, singleProfilePtr->GetElementId());
    DoubleLShapeProfilePtr firstDoubleProfilePtr = InsertElement<DoubleLShapeProfile> (doubleLParams);
    DoubleLShapeProfilePtr secondDoubleProfilePtr = InsertElement<DoubleLShapeProfile> (doubleLParams);

    ASSERT_EQ (DgnDbStatus::DeletionProhibited, singleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::Success, firstDoubleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::DeletionProhibited, singleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::Success, secondDoubleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::Success, singleProfilePtr->Delete());
    }
