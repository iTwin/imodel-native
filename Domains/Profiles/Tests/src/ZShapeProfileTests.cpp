/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ZShapeProfileTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES


/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct ZShapeProfileTestCase : ProfileValidationTestCase<ZShapeProfile>
    {
public:
    typedef ZShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "Z");

    ZShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "Z", 10, 10, 1, 1);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "Z", 10, 10, 1, 1, 1, 0.5, PI / 18);
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "Z");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "Z", 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);

    ZShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("Z", profilePtr->GetName());
    EXPECT_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_EQ (2.0, profilePtr->GetDepth());
    EXPECT_EQ (3.0, profilePtr->GetFlangeThickness());
    EXPECT_EQ (4.0, profilePtr->GetWebThickness());
    EXPECT_EQ (5.0, profilePtr->GetFilletRadius());
    EXPECT_EQ (6.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_EQ (7.0, profilePtr->GetFlangeSlope());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

    ZShapeProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Z");
    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetDepth (1.0);
    profilePtr->SetFlangeThickness (1.0);
    profilePtr->SetWebThickness (1.0);
    profilePtr->SetFilletRadius (1.0);
    profilePtr->SetFlangeEdgeRadius (1.0);
    profilePtr->SetFlangeSlope (1.0);

    EXPECT_EQ ("Z", profilePtr->GetName());
    EXPECT_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_EQ (1.0, profilePtr->GetDepth());
    EXPECT_EQ (1.0, profilePtr->GetFlangeThickness());
    EXPECT_EQ (1.0, profilePtr->GetWebThickness());
    EXPECT_EQ (1.0, profilePtr->GetFilletRadius());
    EXPECT_EQ (1.0, profilePtr->GetFlangeEdgeRadius());
    EXPECT_EQ (1.0, profilePtr->GetFlangeSlope());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, GetInnerFlangeFaceLength_FlangeWidthAndWebThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "Z", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY);
    ZShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (0.0);
    profilePtr->SetWebThickness (0.0);
    EXPECT_EQ (0.0, profilePtr->GetInnerFlangeFaceLength());

    profilePtr->SetFlangeWidth (2.0);
    profilePtr->SetWebThickness (1.0);
    EXPECT_EQ (1.0, profilePtr->GetInnerFlangeFaceLength());

    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetWebThickness (2.0);
    EXPECT_EQ (-1.0, profilePtr->GetInnerFlangeFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, GetInnerWebFaceLength_DepthAndFlangeThickness_CorrectValue)
    {
    CreateParams params (GetModel(), "Z", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY);
    ZShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetDepth (0.0);
    profilePtr->SetFlangeThickness (0.0);
    EXPECT_EQ (0.0, profilePtr->GetInnerWebFaceLength());

    profilePtr->SetDepth (2.0);
    profilePtr->SetFlangeThickness (1.0);
    EXPECT_EQ (1.0, profilePtr->GetInnerWebFaceLength());

    profilePtr->SetDepth (1.0);
    profilePtr->SetFlangeThickness (2.0);
    EXPECT_EQ (-1.0, profilePtr->GetInnerWebFaceLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, GetSlopeHeight_ProfileWithProperties_CorrectValue)
    {
    CreateParams params (GetModel(), "Z", INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY);
    ZShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetFlangeWidth (2.0);
    profilePtr->SetWebThickness (1.0);

    profilePtr->SetFlangeSlope (PI / 4.0);
    EXPECT_EQ (1.0, profilePtr->GetSlopeHeight());

    profilePtr->SetFlangeSlope (0.0);
    EXPECT_EQ (0.0, profilePtr->GetSlopeHeight());

    profilePtr->SetFlangeSlope (PI / 2.0);
    EXPECT_EQ (0.0, profilePtr->GetSlopeHeight());

    profilePtr->SetFlangeSlope (PI);
    EXPECT_EQ (0.0, profilePtr->GetSlopeHeight());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
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
TEST_F (ZShapeProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0, 1.0);

    params.name = "Z";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_InvalidFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "Z", INFINITY, 10.0, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeWidth, "FlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_ValidFlangeWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Z", INFINITY, 10.0, 1.0, 1.0);

    params.flangeWidth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Flange width should be of positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "Z", 10.0, INFINITY, 1.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Z", 10.0, INFINITY, 1.0, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_InvalidFlangeThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "Z", 10.0, 10.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeThickness, "FlangeThickness", false);

    params.flangeThickness = params.depth / 2.0;
    EXPECT_FAIL_Insert (params) << "Flange thickness should be less than half of the depth.";

    params.flangeThickness = nextafter<double, double> (params.depth / 2.0, 0.0);
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be less than half of the depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_ValidFlangeThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Z", 10.0, 10.0, INFINITY, 1.0);

    params.flangeThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Flange thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_InvalidWebThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "Z", 10.0, 10.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.webThickness, "WebThickness", false);

    params.webThickness = params.flangeWidth;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than flange width.";

    params.webThickness = params.flangeWidth + 1.0;
    EXPECT_FAIL_Insert (params) << "Web thickness should be less than flange width.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_ValidWebThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Z", 10.0, 10.0, 1.0, INFINITY);

    params.webThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Web thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_VariousFilletRadiusAndZeroFlangeSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "Z_ZeroFlangeSlope", 10.0, 10.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = 0.0;

    TestParameterToBeFiniteAndPositive (params, params.filletRadius, "FilletRadius", true);

    ZShapeProfilePtr profilePtr = CreateProfile (params);

    // Test against inner flange length
    params.depth = 100.0;
    params.flangeWidth = 10.0;
    params.filletRadius = 4.5;
    EXPECT_EQ (4.5, profilePtr->GetInnerFlangeFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";

    params.filletRadius = nextafter<double, double> (4.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";

    // Test against inner web face length
    params.flangeWidth = 100.0;
    params.depth = 10.0;
    params.filletRadius = 4.5;
    EXPECT_EQ (4.5, profilePtr->GetInnerWebFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";

    params.filletRadius = nextafter<double, double> (4.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlange_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "Z_ZeroFlangeSlope_ShortFlange", 3.0, 100.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = 0.0;

    ZShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_EQ (1.0, profilePtr->GetInnerFlangeFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";

    params.filletRadius = nextafter<double, double> (1.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the flange.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_FilletRadiusAgainstTheWeb_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "Z_ZeroFlangeSlope_ShortWeb", 100.0, 3.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = 0.0;

    ZShapeProfilePtr profilePtr = CreateProfile (params);

    params.filletRadius = 1.0;
    EXPECT_EQ (1.0, profilePtr->GetInnerWebFaceLength() / 2.0);
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";

    params.filletRadius = nextafter<double, double> (1.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (when flange slope is zero).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_FilletRadiusAgainstTheWebWithSlope_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "Z_NonZeroFlangeSlope", 20.0, 10.0, 1.0, 1.0, INFINITY);
    params.flangeSlope = (PI / 180.0) * 10.0;

    ZShapeProfilePtr profilePtr = CreateProfile (params);

    double const maximumFilletRadiusForWeb = profilePtr->GetInnerWebFaceLength() / 2.0 - profilePtr->GetSlopeHeight();
    EXPECT_GE (maximumFilletRadiusForWeb, 0.0) << "Flange slope height cannot be greater than half of the inner web face length";

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";

    params.filletRadius = maximumFilletRadiusForWeb;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";

    params.filletRadius = nextafter<double, double> (maximumFilletRadiusForWeb, INFINITY);
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web minus flange slope height.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_VariousFlangeEdgeRadius_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "Z", 10.0, 10.0, 1.0, 1.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.flangeEdgeRadius, "EdgeRadius", true);

    params.flangeEdgeRadius = 0.5;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of the flange thickness.";

    params.flangeEdgeRadius = nextafter<double, double> (0.5, INFINITY);
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to half of the flange thickness.";

    params.flangeThickness = 4.0;
    params.flangeEdgeRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of the flange thickness.";

    params.webThickness = 8.0;
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to inner flange face length.";

    params.flangeEdgeRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to half of inner flange face length.";

    params.flangeEdgeRadius = nextafter<double, double> (1.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to inner flange face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_FlangeSlopeOf45Degrees_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Z", 5.0, 9.0, 1.0, 1.0, 0.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.flangeSlope, "FlangeSlope", true);

    params.flangeSlope = PI / 4.0;
    ZShapeProfilePtr profilePtr = CreateProfile (params);

    // 45 degree angle means a slope height of 4, when the inner flange face length is 4
    // since inner web face length is 8, a slope of 45 degree should be the maximum allowed value
    EXPECT_EQ (4.0, profilePtr->GetInnerFlangeFaceLength());
    EXPECT_EQ (8.0, profilePtr->GetInnerWebFaceLength());
    EXPECT_EQ (4.0, profilePtr->GetSlopeHeight());
    EXPECT_SUCCESS_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";

    params.flangeSlope = nextafter<double, double> (PI / 4.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "Flange slope should be such, that the slope height should be less or equal to half of inner web face length.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (ZShapeProfileTestCase, Insert_FlangeSlopeOf90Degrees_FailedInsert)
    {
    CreateParams params (GetModel(), "Z", 1.0, DBL_MAX, 0.1, 0.1, 0.0, 0.0, INFINITY);

    params.flangeSlope = 0.0;
    EXPECT_SUCCESS_Insert (params) << "Profile should succeed to insert.";

    params.flangeSlope = PI / 2.0;
    EXPECT_FAIL_Insert (params) << "Flange slope should be less than PI.";

    params.flangeSlope = PI / 2.0 - PI / 10000;
    EXPECT_SUCCESS_Insert (params) << "Flange slope should be less than PI.";
    }
