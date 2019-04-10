/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/CenterLineZShapeProfileTests.cpp $
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
struct CenterLineZShapeProfileTestCase : ProfileValidationTestCase<CenterLineZShapeProfile>
    {
public:
    typedef CenterLineZShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "Z");

    CenterLineZShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_ValidCreateParams_Success)
    {
    CreateParams requiredParams (GetModel(), "Z", 10, 10, 1);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "Z", 10, 10, 1, 0.5, 2);
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_EmptyCreateParams_Error)
    {
    CreateParams params (GetModel(), "Z");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "Z", 1.0, 2.0, 3.0, 4.0, 5.0);

    CenterLineZShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("Z", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetGirth());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0);

    CenterLineZShapeProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Z");
    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetDepth (2.0);
    profilePtr->SetWallThickness (3.0);
    profilePtr->SetFilletRadius (4.0);
    profilePtr->SetGirth (5.0);

    EXPECT_EQ ("Z", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetGirth());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_InvalidProfileName_Error)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_ValidProfileName_Success)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0);

    params.name = "Z";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_InvalidFlangeWidth_Error)
    {
    CreateParams params (GetModel(), "Z", INFINITY, 10.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.flangeWidth, "FlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_ValidFlangeWidth_Success)
    {
    CreateParams params (GetModel(), "Z", INFINITY, 10.0, 1.0);

    params.flangeWidth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Flange width should be of positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_InvalidDepth_Error)
    {
    CreateParams params (GetModel(), "Z", 10.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_ValidDepth_Success)
    {
    CreateParams params (GetModel(), "Z", 10.0, INFINITY, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_InvalidWallThickness_Error)
    {
    CreateParams params (GetModel(), "Z", 10.0, 10.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.wallThickness, "WallThickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_WallThicknessLessThanHalfDepth_Error)
    {
    CreateParams params (GetModel(), "Z", 100.0, 10.0, INFINITY);

    params.wallThickness = params.depth / 2.0;
    EXPECT_FAIL_Insert (params) << "Wall thickness should be less than half of the depth.";

    params.wallThickness = params.depth / 2.0 - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Wall thickness should be less than half of the depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_WallThicknessLessThanFlangeWidth_Error)
    {
    CreateParams params (GetModel(), "Z", 10.0, 100.0, INFINITY);

    params.girth = 0.0;
    params.wallThickness = params.flangeWidth;
    EXPECT_FAIL_Insert (params) << "Wall thickness should be less than flange width when there is no girth.";

    params.wallThickness = params.flangeWidth - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Wall thickness should be less than flange width when there is no girth.";

    params.girth = 5.0;
    EXPECT_FAIL_Insert (params) << "Wall thickness should be less than half flange width when girth is present.";

    params.wallThickness = params.flangeWidth / 2.0;
    EXPECT_FAIL_Insert (params) << "Wall thickness should be less than half flange width when girth is present.";

    params.wallThickness = params.flangeWidth / 2.0 - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Wall thickness should be less than half flange width when girth is present.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_InvalidFilletRadius_Error)
    {
    CreateParams params (GetModel(), "Z", 10.0, 10.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.filletRadius, "FilletRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlangeNoGirth_CorrectResult)
    {
    CreateParams params (GetModel(), "Z", INFINITY, 100.0, 1.0, INFINITY, INFINITY);
    CenterLineZShapeProfilePtr profilePtr = CreateProfile (params);

    params.girth = 0.0;
    params.flangeWidth = 3.0;
    params.filletRadius = 2.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to the inner face of the flange when girth is 0.";

    params.filletRadius = 2.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to the inner face of the flange when girth is 0.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_FilletRadiusAgainstTheFlangeWithGirth_CorrectResult)
    {
    CreateParams params (GetModel(), "Z", INFINITY, 100.0, 1.0, INFINITY, INFINITY);
    CenterLineZShapeProfilePtr profilePtr = CreateProfile (params);

    params.girth = 5.0;
    params.flangeWidth = 4.0;
    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to the half of the inner face of the flange (width - 2 * thickness) when girth is present.";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to the half of the inner face of the flange (width - 2 * thickness) when girth is present.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_FilletRadiusAgainstTheWeb_CorrectResult)
    {
    CreateParams params (GetModel(), "Z", 100.0, INFINITY, 1.0, INFINITY);
    CenterLineZShapeProfilePtr profilePtr = CreateProfile (params);

    params.depth = 4.0;
    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (web - 2 * thicknes).";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to half of the inner face of the web (web - 2 * thicknes).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineZShapeProfileTestCase, Insert_FilletRadiusAgainstTheGirth_CorrectResult)
    {
    CreateParams params (GetModel(), "Z", 100.0, 100.0, 1.0, INFINITY, INFINITY);
    CenterLineZShapeProfilePtr profilePtr = CreateProfile (params);

    params.girth = 2.0;
    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Fillet radius should be less or equal to inner girth face (girth - thickness).";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Fillet radius should be less or equal to inner girth face (girth - thickness).";
    }
