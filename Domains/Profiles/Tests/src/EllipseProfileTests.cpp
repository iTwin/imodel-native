/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/EllipseProfileTests.cpp $
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
struct EllipseProfileTestCase : ProfileValidationTestCase<EllipseProfile>
    {
public:
    typedef EllipseProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "Ellipse");

    EllipseProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "Ellipse", 10.0, 10.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "Ellipse");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "Ellipse", 1.0, 2.0);

    EllipseProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("Ellipse", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetXRadius());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetYRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), nullptr, INFINITY, INFINITY);

    EllipseProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Ellipse");
    profilePtr->SetXRadius (1.0);
    profilePtr->SetYRadius (2.0);

    EXPECT_EQ ("Ellipse", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetXRadius());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetYRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0);

    params.name = "Ellipse";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_InvalidXRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "Ellipse", INFINITY, 10.0);

    TestParameterToBeFiniteAndPositive (params, params.xRadius, "XRadius", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_ValidXRadius_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Ellipse", INFINITY, 10.0);

    params.xRadius = 10.0;
    EXPECT_SUCCESS_Insert (params) << "XRadius should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_InvalidYRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "Ellipse", 10.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.yRadius, "YRadius", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (EllipseProfileTestCase, Insert_ValidYRadius_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Ellipse", 10.0, INFINITY);

    params.yRadius = 10.0;
    EXPECT_SUCCESS_Insert (params) << "YRadius should be positive value.";
    }
