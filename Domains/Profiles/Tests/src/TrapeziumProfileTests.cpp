/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct TrapeziumProfileTestCase : ProfileValidationTestCase<TrapeziumProfile>
    {
public:
    typedef TrapeziumProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "Trapezium");

    TrapeziumProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "Trapezium", 10.0, 5.0, 10.0, 2.5);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "Trapezium");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "Trapezium", 1.0, 2.0, 3.0, 4.0);

    TrapeziumProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("Trapezium", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetTopWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetBottomWidth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetTopOffset());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), nullptr, INFINITY, INFINITY, INFINITY, INFINITY);

    TrapeziumProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Trapezium");
    profilePtr->SetTopWidth (1.0);
    profilePtr->SetBottomWidth (2.0);
    profilePtr->SetDepth (3.0);
    profilePtr->SetTopOffset (4.0);

    EXPECT_EQ ("Trapezium", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetTopWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetBottomWidth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetTopOffset());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 5.0, 10.0, 2.5);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 5.0, 10.0, 2.5);

    params.name = "Trapezium";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_InvalidTopWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "Trapezium", INFINITY, 5.0, 10.0, 2.5);

    TestParameterToBeFiniteAndPositive (params, params.topWidth, "TopWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_InvalidBottomWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "Trapezium", 10.0, INFINITY, 10.0, 2.5);

    TestParameterToBeFiniteAndPositive (params, params.bottomWidth, "BottomWidth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "Trapezium", 10.0, 5.0, INFINITY, 2.5);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TrapeziumProfileTestCase, Insert_InvalidTopOffset_FailedInsert)
    {
    CreateParams params (GetModel(), "Trapezium", 10.0, 5.0, 10.0, INFINITY);

    params.topOffset = std::numeric_limits<double>::signaling_NaN();
    EXPECT_FAIL_Insert (params);

    params.topOffset = 2.5;
    EXPECT_SUCCESS_Insert (params);
    }
