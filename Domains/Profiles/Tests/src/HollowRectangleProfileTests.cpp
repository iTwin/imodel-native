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
struct HollowRectangleProfileTestCase : ProfileValidationTestCase<HollowRectangleProfile>
    {
public:
    typedef HollowRectangleProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "HollowRectangle");

    HollowRectangleProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_ValidCreateParams_SuccessfulInsert)
    {
    CreateParams requiredParams (GetModel(), "HollowRectangle", 10.0, 10.0, 1.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_EmptyCreateParams_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "HollowRectangle", 1.0, 2.0, 3.0, 4.0, 5.0);

    HollowRectangleProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("HollowRectangle", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetInnerFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetOuterFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), nullptr, INFINITY, INFINITY, INFINITY, INFINITY, INFINITY);

    HollowRectangleProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("HollowRectangle");
    profilePtr->SetWidth (1.0);
    profilePtr->SetDepth (2.0);
    profilePtr->SetWallThickness (3.0);
    profilePtr->SetInnerFilletRadius (4.0);
    profilePtr->SetOuterFilletRadius (5.0);

    EXPECT_EQ ("HollowRectangle", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetDepth());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetInnerFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetOuterFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InvalidProfileName_FailedInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_ValidProfileName_SuccessfulInsert)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 10.0, 1.0);

    params.name = "HollowRectangle";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InvalidWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", INFINITY, 10.0, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.width, "Width", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_ValidWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", INFINITY, 10.0, 1.0);

    params.width = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Width should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InvalidDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.depth, "Depth", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_ValidDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, INFINITY, 1.0);

    params.depth = 10.0;
    EXPECT_SUCCESS_Insert (params) << "Depth should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InvalidWallThickness_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, 10.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.wallThickness, "WallThickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_WallThicknessEqualsHalfWidth_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, 50.0, INFINITY);

    double const halfWidth = params.width / 2.0;

    params.wallThickness = halfWidth;
    EXPECT_FAIL_Insert (params) << "Wall thickness should be less than half of width.";

    params.wallThickness = halfWidth - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Wall thickness should be less than half of width.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_WallThicknessEqualsHalfDepth_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 50.0, 10.0, INFINITY);

    double const halfDepth = params.depth / 2.0;

    params.wallThickness = halfDepth;
    EXPECT_FAIL_Insert (params) << "Wall thickness should be less than half of depth.";

    params.wallThickness = halfDepth - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Wall thickness should be less than half of depth.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InvalidInnerFilletRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, 10.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.innerFilletRadius, "InnerFilletRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InnerFilletRadiusFitsWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, 50.0, 1.0, INFINITY);

    double const maxRadius = params.width / 2.0 - params.wallThickness;

    params.innerFilletRadius = maxRadius;
    EXPECT_SUCCESS_Insert (params) << "Inner fillet radius should be less than half width minus wall thickness";

    params.innerFilletRadius = maxRadius + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Inner fillet radius should be less than half width minus wall thickness";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InnerFilletRadiusFitsDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 50.0, 10.0, 1.0, INFINITY);

    double const maxRadius = params.depth / 2.0 - params.wallThickness;

    params.innerFilletRadius = maxRadius;
    EXPECT_SUCCESS_Insert (params) << "Inner fillet radius should be less than half depth minus wall thickness";

    params.innerFilletRadius = maxRadius + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Inner fillet radius should be less than half depth minus wall thickness";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_InvalidOuterFilletRadius_FailedInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, 10.0, 1.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.outerFilletRadius, "OuterFilletRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_OuterFilletRadiusIntersectsWithInnerCorner_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, 10.0, 1.0, INFINITY, INFINITY);

    params.innerFilletRadius = 0.5;
    double const maxOuterRadius = (2.0 + std::sqrt (2.0)) * params.wallThickness + params.innerFilletRadius;

    params.outerFilletRadius = maxOuterRadius;
    EXPECT_FAIL_Insert (params) << "Outer fillet radius should be such that the filleted outer corner doesn't intersect with the inner corner";

    params.outerFilletRadius = maxOuterRadius - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Outer fillet radius should be such that the filleted outer corner doesn't intersect with the inner corner";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_OuterFilletEqualsHalfWidth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 10.0, 50.0, 1.0, INFINITY, INFINITY);

    double const maxOuterRadius = params.width / 2.0;
    double const maxInnerRadius = maxOuterRadius - params.wallThickness;

    params.innerFilletRadius = maxInnerRadius;

    params.outerFilletRadius = maxOuterRadius;
    EXPECT_SUCCESS_Insert (params) << "Outer fillet radius should be less than half width";

    params.outerFilletRadius = maxOuterRadius + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Outer fillet radius should be less than half width";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (HollowRectangleProfileTestCase, Insert_OuterFilletRadiusEqualsHalfDepth_SuccessfulInsert)
    {
    CreateParams params (GetModel(), "HollowRectangle", 50.0, 10.0, 1.0, INFINITY, INFINITY);

    double const maxOuterRadius = params.depth / 2.0;
    double const maxInnerRadius = maxOuterRadius - params.wallThickness;

    params.innerFilletRadius = maxInnerRadius;

    params.outerFilletRadius = maxOuterRadius;
    EXPECT_SUCCESS_Insert (params) << "Outer fillet radius should be less than half depth";

    params.outerFilletRadius = maxOuterRadius + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Outer fillet radius should be less than half depth";
    }
