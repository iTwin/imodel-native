/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/CircleProfileTests.cpp $
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
struct CircleProfileTestCase : ProfileValidationTestCase<CircleProfile>
    {
public:
    typedef CircleProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "Circle");

    CircleProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "Circle", 10.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "Circle");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "Circle", 1.0);

    CircleProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("Circle", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), nullptr, INFINITY);

    CircleProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Circle");
    profilePtr->SetRadius (1.0);

    EXPECT_EQ ("Circle", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0);

    params.name = "Circle";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, Insert_InvalidRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "Circle", INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.radius, "Radius", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (CircleProfileTestCase, Insert_ValidWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Circle", INFINITY);

    params.radius = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Radius should be positive value.";
    }
