                                                                            /*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/HollowCircleProfileTests.cpp $
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
struct HollowCircleProfileTestCase : ProfileValidationTestCase<HollowCircleProfile>
    {
public:
    typedef HollowCircleProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "HollowCircle");

    HollowCircleProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "HollowCircle", 10.0, 1.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowCircle");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "HollowCircle", 1.0, 2.0);

    HollowCircleProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("HollowCircle", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetRadius());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetWallThickness());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), nullptr, INFINITY, INFINITY);

    HollowCircleProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("HollowCircle");
    profilePtr->SetRadius (1.0);
    profilePtr->SetWallThickness (2.0);

    EXPECT_EQ ("HollowCircle", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetRadius());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetWallThickness());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 1.0);

    params.name = "HollowCircle";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_InvalidRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowCircle", INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.radius, "Radius", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_ValidRadius_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowCircle", INFINITY, 1.0);

    params.radius = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Radius should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_InvalidWallThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowCircle", 10.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.wallThickness, "WallThickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_WallThicknessEqualToRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowCircle", 10.0, INFINITY);

    params.wallThickness = params.radius;
    EXPECT_FAIL_Insert (params) << "Wall thickness should be less than radius.";

    params.wallThickness = params.radius - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Wall thickness should be less than radius.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowCircleProfileTestCase, Insert_ValidWallThickness_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowCircle", 10.0, INFINITY);

    params.wallThickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Radius should be positive value.";
    }
