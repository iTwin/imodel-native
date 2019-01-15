/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/CShapeProfileTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct CShapeProfileTestCase : ProfileValidationTestCase<CShapeProfile>
    {
public:
    typedef CShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "C");

    CShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "C", 10, 10, 1, 1);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "C", 10, 10, 1, 1, 1, 0.5, Angle::FromRadians (PI / 18));
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "C");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "C", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, Angle::FromRadians (7.0));
    CShapeProfilePtr profilePtr = CreateProfile (params);

    EXPECT_EQ ("C", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetFlangeThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (7.0, profilePtr->GetFlangeSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Angle::FromRadians (0.0));
    CShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetName ("C");
    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetDepth (1.0);
    profilePtr->SetFlangeThickness (1.0);
    profilePtr->SetWebThickness (1.0);
    profilePtr->SetFilletRadius (1.0);
    profilePtr->SetFlangeEdgeRadius (1.0);
    profilePtr->SetFlangeSlope (Angle::FromRadians (1.0));

    EXPECT_EQ ("C", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeThickness());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, GetInnerFlangeFaceLength_FlangeWidthAndWebThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "C", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY));
    CShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (0.0);
    profilePtr->SetWebThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetInnerFlangeFaceLength());

    profilePtr->SetFlangeWidth (2.0);
    profilePtr->SetWebThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetInnerFlangeFaceLength());

    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetWebThickness (2.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetInnerFlangeFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, GetInnerWebFaceLength_DepthAndFlangeThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "C", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY));
    CShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (0.0);
    profilePtr->SetFlangeThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetInnerWebFaceLength());

    profilePtr->SetDepth (3.0);
    profilePtr->SetFlangeThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetInnerWebFaceLength());

    profilePtr->SetDepth (1.0);
    profilePtr->SetFlangeThickness (3.0);
    EXPECT_DOUBLE_EQ (-5.0, profilePtr->GetInnerWebFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, GetFlangeSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params (GetModel(), "C", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY));
    CShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (2.0);
    profilePtr->SetWebThickness (1.0);

    profilePtr->SetFlangeSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetFlangeSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetFlangeSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeSlopeHeight());

    profilePtr->SetFlangeSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0, 1.0);

    params.name = "C";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_InvalidFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "C", INFINITY, 10.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeWidth, "FlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_ValidFlangeWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "C", INFINITY, 10.0, 1.0, 1.0);

    params.flangeWidth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Flange width should be of positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, INFINITY, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, INFINITY, 1.0, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_InvalidFlangeThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeThickness, "FlangeThickness", false);

    params.flangeThickness = params.depth / 2.0;
    EXPECT_FAIL_Insert (params) << "Flange thickness should be less than half of the depth.";

    params.flangeThickness = BeNumerical::BeNextafter (params.depth / 2.0, 0.0);
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be less than half of the depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_ValidFlangeThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, INFINITY, 1.0);

    params.flangeThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_InvalidWebThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.webThickness, "WebThickness", false);

    params.webThickness = params.flangeWidth;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than flange width.";

    params.webThickness = params.flangeWidth + 1.0;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than flange width.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_ValidWebThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, 1.0, INFINITY);

    params.webThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Web thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_InvalidFilletRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, 1.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.filletRadius, "FilletRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlange_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "C_ZeroFlangeSlope_ShortFlange", 2.0, 100.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians (0.0);

    CShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 0.5;
    EXPECT_DOUBLE_EQ (0.5, profilePtr->GetInnerFlangeFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";

    params.filletRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_FilletRadiusAgainstTheWeb_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "C_ZeroFlangeSlope_ShortWeb", 100.0, 3.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians (0.0);

    CShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 0.5;
    EXPECT_DOUBLE_EQ (0.5, profilePtr->GetInnerWebFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";

    params.filletRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_FilletRadiusAgainstTheWebWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "C_NonZeroFlangeSlope", 10.0, 10.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians ((PI / 180.0) * 10.0);

    CShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForWeb = profilePtr->GetInnerWebFaceLength() / 2.0 - profilePtr->GetFlangeSlopeHeight();
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
TEST_F (CShapeProfileTestCase, Insert_InvalidFlangeEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, 1.0, 1.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.flangeEdgeRadius, "FlangeEdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_FlangeEdgeRadiusAgainstTheFlangeThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, 1.0, 1.0, 0.0, INFINITY);

    params.flangeEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of the flange thickness.";

    params.flangeEdgeRadius = BeNumerical::BeNextafter (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to half of the flange thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_FlangeEdgeRadiusAgainstTheInnerFlangeFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "C", 10.0, 10.0, 1.0, 1.0, 0.0, INFINITY);

    params.flangeThickness = 4.0;
    params.flangeEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of the flange thickness.";

    params.webThickness = 8.0;
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to inner flange face length.";

    params.flangeEdgeRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of inner flange face length.";

    params.flangeEdgeRadius = BeNumerical::BeNextafter (1.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to inner flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_InvalidFlangeSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "C", 5.0, 10.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.flangeSlope, "FlangeSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_FlangeSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "C", 5.0, 10.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.flangeSlope = Angle::FromRadians (PI / 4.0);
    CShapeProfilePtr profilePtr = CreateProfile (params);

    // 45 degree angle means a slope height of 4, when the inner flange face length is 4
    // since inner web face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetInnerFlangeFaceLength());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetInnerWebFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFlangeSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";

    params.flangeSlope = Angle::FromRadians (BeNumerical::BeNextafter (PI / 4.0, INFINITY));
    EXPECT_FAIL_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Insert_FlangeSlopeOf90Degrees_FailedInsert)
    {
    // Can't use DBL_MAX as it will assert in geometry generation
    double const depth = DBL_MAX / 4.0;
    CreateParams params (GetModel(), "C", 1.0, depth, 0.1, 0.1, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.flangeSlope = Angle::FromRadians (0.0);
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert.";

    params.flangeSlope = Angle::FromRadians (PI / 2.0);
    EXPECT_FAIL_Insert (params) << "Flange slope should be less than 90 degrees.";

    params.flangeSlope = Angle::FromRadians (PI / 2.0 - PI / 10000);
    EXPECT_SUCCESS_Insert (params) << "Flange slope should be less than 90 degrees.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Delete_ExistingDoubleCInstances_FailedDelete)
    {
    CShapeProfilePtr singleProfilePtr = InsertElement<CShapeProfile> (CreateParams (GetModel(), "L", 6.0, 10.0, 1.0, 1.0));

    DoubleCShapeProfile::CreateParams doubleLParams (GetModel(), "DC", 1.0, singleProfilePtr->GetElementId());
    DoubleCShapeProfilePtr firstDoubleProfilePtr = InsertElement<DoubleCShapeProfile> (doubleLParams);
    DoubleCShapeProfilePtr secondDoubleProfilePtr = InsertElement<DoubleCShapeProfile> (doubleLParams);

    ASSERT_EQ (DgnDbStatus::ForeignKeyConstraint, singleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::Success, firstDoubleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::ForeignKeyConstraint, singleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::Success, secondDoubleProfilePtr->Delete());
    ASSERT_EQ (DgnDbStatus::Success, singleProfilePtr->Delete());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CShapeProfileTestCase, Update_ExistingDoubleCInstances_UpdatedDoubleCShape)
    {
    CShapeProfilePtr singleProfilePtr = InsertElement<CShapeProfile> (CreateParams (GetModel(), "C", 6.0, 10.0, 1.0, 1.0));

    DoubleCShapeProfile::CreateParams doubleLParams (GetModel(), "DC", 1.0, singleProfilePtr->GetElementId());
    DoubleCShapeProfileCPtr doubleProfilePtr = InsertElement<DoubleCShapeProfile> (doubleLParams);

    DRange3d range;
    ASSERT_TRUE (doubleProfilePtr->GetShape()->TryGetRange (range));
    EXPECT_EQ (6.0 * 2.0 + 1.0, range.XLength()) << "DoubleCShapeProfile width should be SingleProfile.Width * 2 + Spacing";

    singleProfilePtr->SetFlangeWidth (10.0);

    DgnDbStatus status;
    singleProfilePtr->Update (&status);
    ASSERT_EQ (DgnDbStatus::Success, status);

    DoubleCShapeProfileCPtr updateDoubleProfilePtr = DoubleCShapeProfile::Get (GetDb(), doubleProfilePtr->GetElementId());
    ASSERT_TRUE (updateDoubleProfilePtr->GetShape()->TryGetRange (range));
    EXPECT_EQ (10.0 * 2.0 + 1.0, range.XLength()) << "DoubleCShapeProfile width should be SingleProfile.Width * 2 + Spacing";
    }
