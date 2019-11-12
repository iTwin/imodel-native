/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct TShapeProfileTestCase : ProfileValidationTestCase<TShapeProfile>
    {
public:
    typedef TShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "T");

    TShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "T", 10, 10, 1, 1);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "T", 10, 10, 1, 1, 1, 0.5, Angle::FromRadians (PI / 18), 0.5, Angle::FromRadians (PI / 32));
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "T");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "T", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, Angle::FromRadians (7.0), 8.0, Angle::FromRadians (9.0));

    TShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("T", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetFlangeThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (7.0, profilePtr->GetFlangeSlope().Radians());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetWebEdgeRadius());
    EXPECT_DOUBLE_EQ (9.0, profilePtr->GetWebSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, Angle::FromRadians (0.0));

    TShapeProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("T");
    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (3.0);
    profilePtr->SetWebThickness (4.0);
    profilePtr->SetFilletRadius (5.0);
    profilePtr->SetFlangeEdgeRadius (6.0);
    profilePtr->SetFlangeSlope (Angle::FromRadians (7.0));
    profilePtr->SetWebEdgeRadius (8.0);
    profilePtr->SetWebSlope (Angle::FromRadians (9.0));

    EXPECT_EQ ("T", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetFlangeThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebThickness());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (6.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_DOUBLE_EQ (7.0, profilePtr->GetFlangeSlope().Radians());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetWebEdgeRadius());
    EXPECT_DOUBLE_EQ (9.0, profilePtr->GetWebSlope().Radians());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, GetInnerFlangeFaceLength_FlangeWidthAndWebThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "T", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY), INFINITY, Angle::FromRadians (INFINITY));
    TShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (0.0);
    profilePtr->SetWebThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetFlangeInnerFaceLength());

    profilePtr->SetFlangeWidth (3.0);
    profilePtr->SetWebThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeInnerFaceLength());

    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetWebThickness (3.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetFlangeInnerFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, GetInnerWebFaceLength_DepthAndFlangeThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "T", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY), INFINITY, Angle::FromRadians (INFINITY));
    TShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (0.0);
    profilePtr->SetFlangeThickness (0.0);
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebFaceLength());

    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (1.0);
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebFaceLength());

    profilePtr->SetDepth (1.0);
    profilePtr->SetFlangeThickness (2.0);
    EXPECT_DOUBLE_EQ (-1.0, profilePtr->GetWebFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, GetFlangeSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params (GetModel(), "T", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY), INFINITY, Angle::FromRadians (INFINITY));
    TShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (3.0);
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
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, GetWebSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params (GetModel(), "T", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, Angle::FromRadians (INFINITY), INFINITY, Angle::FromRadians (INFINITY));
    TShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (1.0);

    profilePtr->SetWebSlope (Angle::FromRadians (PI / 4.0));
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (0.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (PI / 2.0));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebSlopeHeight());

    profilePtr->SetWebSlope (Angle::FromRadians (PI));
    EXPECT_DOUBLE_EQ (0.0, profilePtr->GetWebSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0, 1.0);

    params.name = "T";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "T", INFINITY, 10.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeWidth, "FlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_ValidFlangeWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "T", INFINITY, 10.0, 1.0, 1.0);

    params.flangeWidth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Flange width should be of positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, INFINITY, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, INFINITY, 1.0, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidFlangeThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeThickness, "FlangeThickness", false);

    params.flangeThickness = params.depth;
    EXPECT_FAIL_Insert (params) << "Flange thickness should be less than the depth.";

    params.flangeThickness = params.depth - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be less than half of the depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_ValidFlangeThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, INFINITY, 1.0);

    params.flangeThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidWebThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.webThickness, "WebThickness", false);

    params.webThickness = params.flangeWidth;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than flange width.";

    params.webThickness = params.flangeWidth + 1.0;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than flange width.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_ValidWebThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, INFINITY);

    params.webThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Web thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlange_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T_ZeroFlangeSlope_ShortFlange", 5.0, 100.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians (0.0);

    TShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeInnerFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FilletRadiusAgainstTheWeb_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T_ZeroFlangeSlope_ShortWeb", 100.0, 3.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians (0.0);

    TShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWebFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlangeWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T_NonZeroFlangeSlope", 10.0, 20.0, 1.0, 1.0, INFINITY);
    params.webSlope = Angle::FromRadians ((PI / 180.0) * 1.0);

    TShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForFlange = profilePtr->GetFlangeInnerFaceLength() / 2.0 - profilePtr->GetWebSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForFlange, 0.0) << "Web slope height cannot be greater than half of the inner flange face length";

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web slope height.";

    params.filletRadius = maximumFilletRadiusForFlange;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web slope height.";

    params.filletRadius = maximumFilletRadiusForFlange + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange minus web slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FilletRadiusAgainstTheWebWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T_NonZeroFlangeSlope", 20.0, 10.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = Angle::FromRadians ((PI / 180.0) * 10.0);

    TShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForWeb = profilePtr->GetWebFaceLength() / 2.0 - profilePtr->GetFlangeSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForWeb, 0.0) << "Flange slope height cannot be greater than half of the inner web face length";

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";

    params.filletRadius = maximumFilletRadiusForWeb;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";

    params.filletRadius = maximumFilletRadiusForWeb + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidFlangeEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, 1.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.flangeEdgeRadius, "FlangeEdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FlangeEdgeRadiusAgainstTheFlangeThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, 1.0, 0.0, INFINITY);

    params.flangeEdgeRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to the flange thickness.";

    params.flangeEdgeRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to the flange thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FlangeEdgeRadiusAgainstTheInnerFlangeFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, 1.0, 0.0, INFINITY);

    params.flangeThickness = 4.0;
    params.flangeEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to the flange thickness.";

    params.webThickness = 8.0;
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to inner flange face length.";

    params.flangeEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of inner flange face length.";

    params.flangeEdgeRadius = 0.5 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to inner flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidFlangeSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "T", 9.0, 10.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.flangeSlope, "FlangeSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FlangeSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "T", 9.0, 9.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (INFINITY));

    params.flangeSlope = Angle::FromRadians (PI / 4.0);
    TShapeProfilePtr profilePtr = CreateProfile (params);

    // 45 degree angle means a slope height of 4, when the inner flange face length is 4
    // since inner web face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFlangeInnerFaceLength());
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetWebFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFlangeSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";

    params.flangeSlope = Angle::FromRadians (PI / 4.0 + TESTS_EPSILON);
    EXPECT_FAIL_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_FlangeSlopeOf90Degrees_FailedInsert)
    {
    // Can't use DBL_MAX as it will assert in geometry generation
    double const depth = DBL_MAX / 4.0;
    CreateParams params (GetModel(), "T", 1.0, depth, 0.1, 0.1, 0.0, 0.0, Angle::FromRadians (INFINITY));

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
TEST_F (TShapeProfileTestCase, Insert_InvalidWebEdgeRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.webEdgeRadius, "WebEdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_WebEdgeRadiusAgainstWebThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);

    params.webEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Web edge radius should be less or equal to half of the web thickness.";

    params.webEdgeRadius = 0.5 + 0.0000001;
    EXPECT_FAIL_Insert (params) << "Web edge radius should be less or equal to half of the web thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_WebEdgeRadiusAgainstWebFace_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "T", 10.0, 10.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), INFINITY);

    params.webThickness = 4.0;
    params.webEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Web edge radius should be less or equal to half of the web thickness.";

    params.flangeThickness = 7.0;
    EXPECT_FAIL_Insert (params) << "Web edge radius should be less or equal to half of web face length.";

    params.webEdgeRadius = 1.5;
    EXPECT_SUCCESS_Insert (params) << "Web edge radius should be less or equal to half of web face length.";

    params.webEdgeRadius = 1.5 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Web edge radius should be less or equal to web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_InvalidWebSlope_FailedInsert)
    {
    CreateParams params (GetModel(), "T", 9.0, 10.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, Angle::FromRadians (INFINITY));

    TestParameterToBeFiniteAndPositive (params, params.webSlope, "WebSlope", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Insert_WebSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "T", 17.0, 5.0, 1.0, 1.0, 0.0, 0.0, Angle::FromRadians (0.0), 0.0, Angle::FromRadians (INFINITY));

    params.webSlope = Angle::FromRadians (PI / 4.0);
    TShapeProfilePtr profilePtr = CreateProfile (params);

    // 45 degree angle means a slope height of 4, when the inner web face length is 4
    // since inner flange face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_DOUBLE_EQ (8.0, profilePtr->GetFlangeInnerFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebFaceLength());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWebSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Web slope should be such, that the slope height should be less or equal to half of inner flange face length.";

    params.flangeSlope = Angle::FromRadians (PI / 4.0 + TESTS_EPSILON);
    EXPECT_FAIL_Insert (params) << "Web slope should be such, that the slope height should be less or equal to half of inner flange face length.";
    }
