/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/CenterLineCShapeProfileTests.cpp $
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
struct CenterLineCShapeProfileTestCase : ProfileValidationTestCase<CenterLineCShapeProfile>
    {
public:
    typedef CenterLineCShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "CenterLineCShape");

    CenterLineCShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "CenterLineCShape", 10, 10, 1, 1);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "CenterLineCShape", 10, 10, 1, 1.20, 0.17);
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 1.0, 2.0, 3.0, 4.0, 5.0);
    CenterLineCShapeProfilePtr profilePtr = CreateProfile (params);

    EXPECT_EQ ("CenterLineCShape", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetGirth());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0);
    CenterLineCShapeProfilePtr profilePtr = CreateProfile (params);

    profilePtr->SetName ("CenterLineCShape");
    profilePtr->SetFlangeWidth (1.0);
    profilePtr->SetDepth (1.0);
    profilePtr->SetGirth (1.0);
    profilePtr->SetWallThickness(1.0);
    profilePtr->SetFilletRadius (2.0);

    EXPECT_EQ ("CenterLineCShape", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetFlangeWidth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetGirth());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(CenterLineCShapeProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 1.0, 2.0, 3.0, 4.0, 5.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_InvalidFlangeWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", INFINITY, 4.0, 1.0, 1.0, 0.17);
    
    TestParameterToBeFiniteAndPositive (params, params.flangeWidth, "FlangeWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_ValidFlangeWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", INFINITY, 4.0, 1.0, 1.5, 0.17);
    params.flangeWidth = 4.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape FlangeWidth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 4.0, INFINITY, 1.0, 1.5, 0.17);
    params.depth = 4.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 4.0, INFINITY, 1.0, 1.0, 0.17);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_ValidWallThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 3.0, 4.0, INFINITY, 0.0, 0.0);
    params.wallThickness = 1.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape WallThickness should be positive value.";

    params.wallThickness = std::min (params.depth, params.flangeWidth) / 2.0 - TESTS_EPSILON;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape WallThickness should be less than half of FlangeWidth and half of Depth value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_InvalidWallThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 4.0, 5.0, INFINITY, 0.0, 0.0);
    TestParameterToBeFiniteAndPositive (params, params.wallThickness, "WallThickness", false);

    params.wallThickness = 2.0;

    EXPECT_FAIL_Insert (params) << "CenterLineCShape WallThickness should be less than half of FlangeWidth and half of Depth value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_InvalidGirth_FailedInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 3.0, 4.0, 1.0, INFINITY, 0.0);
    
    TestParameterToBeFiniteAndPositive (params, params.girth, "Girth", true);

    params.girth = params.depth / 2.0;

    EXPECT_FAIL_Insert (params) << "CenterLineCShape Girth should be less of half Depth.";

    params.girth = params.wallThickness + params.filletRadius - TESTS_EPSILON;

    EXPECT_FAIL_Insert (params) << "CenterLineCShape Girth should be greater or equal Wallthickness + FilletRadius.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_ValidGirth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "CenterLineCShape", 4.0, 4.0, 1.0, INFINITY, 0.0);

    params.girth = 1.5;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape Girth should be positive.";

    params.girth = params.depth / 2.0 - TESTS_EPSILON;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape Girth should be less of half Depth.";

    params.girth = params.wallThickness + params.filletRadius;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape Girth should be greater or equal WallThickness + FilletRadius.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_ValidFilletRadius_SuccessfulInsert)
    {
    CreateParams params (GetModel (), "CenterLineCShape", 10.0, 10.0, 1.0, 0.0, INFINITY);

    params.filletRadius = 0.17;
    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape FilletRadius should be positive.";

    params.filletRadius = params.flangeWidth / 2.0 - params.wallThickness;
    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape FilletRadius should be less or equal FlangeWidth / 2.0 - WallThickness.";

    params.filletRadius = params.depth / 2.0 - params.wallThickness;
    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape FilletRadius should be less or equal Depth / 2.0 - WallThickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_InvalidFilletRadius_FailedInsert)
    {
    CreateParams params (GetModel (), "CenterLineCShape", 10.0, 10.0, 1.0, 0.0, INFINITY);
    EXPECT_FAIL_Insert (params) << "CenterLineCShape FilletRadius should be positive or zero.";
    TestParameterToBeFiniteAndPositive (params, params.filletRadius, "FilletRadius", true);

    params.filletRadius = -0.17;
    TestParameterToBeFiniteAndPositive (params, params.filletRadius, "FilletRadius", true);

    params.filletRadius = params.flangeWidth / 2.0 - params.wallThickness + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "CenterLineCShape FilletRadius should be less or equal FlangeWidth / 2.0 - WallThickness.";

    params.filletRadius = params.depth / 2.0 - params.wallThickness + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "CenterLineCShape FilletRadius should be less or equal Depth / 2.0 - WallThickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CenterLineCShapeProfileTestCase, Insert_FilletRadiusAgainstTheGirth_CorrectInsertResult)
    {
    CreateParams params (GetModel (), "CenterLineCShape", 10.0, 10.0, 1.0, 0.0, 0.17);

    params.girth = 4.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape Girth should be greater or equal WallThickness + FilletRadius.";

    params.filletRadius = 3.0;

    EXPECT_SUCCESS_Insert (params) << "CenterLineCShape FilletRadius should be small enough to fit into inner space";
    }