/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct RegularPolygonProfileTestCase : ProfileValidationTestCase<RegularPolygonProfile>
    {
public:
    typedef RegularPolygonProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "Polygon");

    RegularPolygonProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "Polygon", 5, 10.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "Polygon");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "Polygon", 1, 2.0);

    RegularPolygonProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("Polygon", profilePtr->GetName());
    EXPECT_EQ (1, profilePtr->GetSideCount());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetSideLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), nullptr, INFINITY, INFINITY);

    RegularPolygonProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("Polygon");
    profilePtr->SetSideCount (1);
    profilePtr->SetSideLength (2.0);

    EXPECT_EQ ("Polygon", profilePtr->GetName());
    EXPECT_EQ (1, profilePtr->GetSideCount());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetSideLength());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 5, 10.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 5, 10.0);

    params.name = "Polygon";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_InvalidSideCount_FailedInsert)
    {
    CreateParams params (GetModel(), "Polygon", INFINITY, 10.0);

    params.sideCount = 0;
    EXPECT_FAIL_Insert (params) << "SideCount should be greater or equal to 3";

    params.sideCount = 2;
    EXPECT_FAIL_Insert (params) << "SideCount should be greater or equal to 3";

    params.sideCount = 33;
    EXPECT_FAIL_Insert (params) << "SideCount should be less or equal to 32";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_ValidSideCount_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Polygon", INFINITY, 10.0);

    params.sideCount = 3;
    EXPECT_SUCCESS_Insert (params) << "SideCount should be greater or equal to 3";

    params.sideCount = 32;
    EXPECT_SUCCESS_Insert (params) << "SideCount should be less or equal to 32";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_InvalidSideLength_FailedInsert)
    {
    CreateParams params (GetModel(), "Polygon", 5, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.sideLength, "SideLength", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (RegularPolygonProfileTestCase, Insert_ValidSideLength_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "Polygon", 5, INFINITY);

    params.sideLength = 10.0;
    EXPECT_SUCCESS_Insert (params) << "SideLength should be greater than zero.";
    }
