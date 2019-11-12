/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct CenterLineLShapeProfileTestCase : ProfileValidationTestCase<CenterLineLShapeProfile>
    {
public:
    typedef CenterLineLShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "CenterLineLShape");

    CenterLineLShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "CenterLineLShape", 10.0, 10.0, 1.0, 2.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "CenterLineLShape", 10.0, 10.0, 1.0, 1.20, 0.17);
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 1.0, 2.0, 3.0, 4.0, 5.0);
    CenterLineLShapeProfilePtr profilePtr = CreateProfile (params);

    EXPECT_EQ ("CenterLineLShape", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetGirth());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0);
    CenterLineLShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetName ("CenterLineLShape");
    profilePtr->SetWidth (1.0);
    profilePtr->SetDepth (1.0);
    profilePtr->SetGirth (1.0);
    profilePtr->SetWallThickness(1.0);
    profilePtr->SetFilletRadius (2.0);

    EXPECT_EQ ("CenterLineLShape", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetGirth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CenterLineLShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 1.0, 2.0, 3.0, 4.0, 5.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_InvalidWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", INFINITY, 4.0, 1.0, 1.0, 0.17);
    
    TestParameterToBeFiniteAndPositive (params, params.width, "Width", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_ValidWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", INFINITY, 4.0, 1.0, 1.5, 0.17);
    params.width = 4.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape Width should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 4.0, INFINITY, 1.0, 1.5, 0.17);
    params.depth = 4.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 4.0, INFINITY, 1.0, 1.0, 0.17);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_ValidWallThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 3.0, 4.0, INFINITY, 0.0, 0.0);
    params.wallThickness = 1.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape WallThickness should be positive value.";

    params.wallThickness = std::min (params.depth, params.width) / 2.0 - TESTS_EPSILON;

    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape WallThickness should be less than half of Width and half of Depth value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_InvalidWallThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 4.0, 5.0, INFINITY, 0.0, 0.0);
    TestParameterToBeFiniteAndPositive (params, params.wallThickness, "WallThickness", false);

    params.wallThickness = 3.0;

    EXPECT_FAIL_Insert (params) << "CenterLineLShape WallThickness should be less than half of Width and half of Depth value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_InvalidGirth_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 3.0, 4.0, 1.0, INFINITY, 0.0);
    TestParameterToBeFiniteAndPositive (params, params.girth, "Girth", true);

    params.girth = params.depth - params.wallThickness + TESTS_EPSILON;    
    EXPECT_FAIL_Insert (params) << "CenterLineLShape Girth should be less than Depth - WallThickness.";

    params.girth = params.wallThickness;
    EXPECT_FAIL_Insert (params) << "CenterLineLShape Girth should be greater than WallThickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_ValidGirth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineLShape", 4.0, 4.0, 1.0, INFINITY, 0.0);

    params.girth = 1.5;
    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape Girth should be positive.";

    params.girth = params.wallThickness + TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape Girth should be greater than WallThickness.";

    params.girth = params.depth - params.wallThickness - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape Girth should be less than Depth - WallThickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_ValidFilletRadius_SuccessfulInsert)
    {
    CreateParams params (GetModel (), "CenterLineLShape", 10.0, 10.0, 1.0, 0.0, INFINITY);

    params.filletRadius = 0.17;
    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape FilletRadius should be positive.";

    params.filletRadius = params.width / 2.0 - params.wallThickness;
    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape FilletRadius should be less or equal Width / 2.0 - WallThickness.";

    params.filletRadius = params.depth / 2.0 - params.wallThickness;
    EXPECT_SUCCESS_Insert (params) << "CenterLineLShape FilletRadius should be less or equal Depth / 2.0 - WallThickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineLShapeProfileTestCase, Insert_InvalidFilletRadius_FailedInsert)
    {
    CreateParams params (GetModel (), "CenterLineLShape", 10.0, 10.0, 1.0, 0.0, INFINITY);
    TestParameterToBeFiniteAndPositive (params, params.filletRadius, "FilletRadius", true);

    params.filletRadius = params.width / 2.0 - params.wallThickness + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "CenterLineLShape FilletRadius should be less or equal Width / 2.0 - WallThickness.";

    params.filletRadius = params.depth / 2.0 - params.wallThickness + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "CenterLineLShape FilletRadius should be less or equal Depth / 2.0 - WallThickness.";
    }