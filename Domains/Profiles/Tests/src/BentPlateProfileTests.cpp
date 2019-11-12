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
struct BentPlateProfileTestCase : ProfileValidationTestCase<BentPlateProfile>
    {
public:
    typedef BentPlateProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "BP");

    BentPlateProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_ValidCreateParams_Success)
    {
    CreateParams requiredParams (GetModel(), "BP", 10.0, 1.0, Angle::FromDegrees (90), 5.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "BP", 10.0, 1.0, Angle::FromDegrees (90), 5.0, 0.5);
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_EmptyCreateParams_Error)
    {
    CreateParams params (GetModel(), "BP");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "BP", 1.0, 2.0, Angle::FromRadians (3.0), 4.0, 5.0);

    BentPlateProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("BP", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetBendAngle().Radians());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetBendOffset());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), "", 0.0, 0.0, Angle::FromRadians (0.0), 0.0, 0.0);

    BentPlateProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("BP");
    profilePtr->SetWidth (1.0);
    profilePtr->SetWallThickness (2.0);
    profilePtr->SetBendAngle (Angle::FromRadians (3.0));
    profilePtr->SetBendOffset (4.0);
    profilePtr->SetFilletRadius (5.0);

    EXPECT_EQ ("BP", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetWidth());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetWallThickness());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetBendAngle().Radians());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetBendOffset());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetFilletRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_InvalidProfileName_Error)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 1.0, Angle::FromDegrees (90), 5.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_ValidProfileName_Success)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 1.0, Angle::FromDegrees (90), 5.0);

    params.name = "BP";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_InvalidWidth_Error)
    {
    CreateParams params (GetModel(), "BP", INFINITY, 1.0, Angle::FromDegrees (90), 5.0);

    TestParameterToBeFiniteAndPositive (params, params.width, "Width", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_InvalidWallThickness_Error)
    {
    CreateParams params (GetModel(), "BP", 10.0, INFINITY, Angle::FromDegrees (90), 5.0);

    TestParameterToBeFiniteAndPositive (params, params.wallThickness, "WallThickness", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_WallThicknessAgainst90DegBendAngle_CorrectResult)
    {
    CreateParams params (GetModel(), "BP", 10.0, INFINITY, Angle::FromDegrees (90), 5.0);

    params.wallThickness = 10.0;
    EXPECT_FAIL_Insert (params);

    params.wallThickness = 10.0 - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params);

    params.bendOffset = 2.0;
    EXPECT_FAIL_Insert (params);

    params.wallThickness = 4.0;
    EXPECT_FAIL_Insert (params);

    params.wallThickness = 4.0 - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_InvalidBendAngle_Error)
    {
    CreateParams params (GetModel(), "BP", 10.0, 1.0, Angle::FromDegrees (INFINITY), 5.0);

    TestParameterToBeFiniteAndPositive (params, params.bendAngle, "BendAngle", false);

    params.bendAngle = Angle::FromRadians (0.0);
    EXPECT_FAIL_Insert (params) << "BendAngle must be greater than 0 degrees";

    params.bendAngle = Angle::FromRadians (PI);
    EXPECT_FAIL_Insert (params) << "BendAngle must be less than 180 degrees";

    params.bendAngle = Angle::FromRadians (PI / 2.0);
    EXPECT_SUCCESS_Insert (params) << "BendAngle must be in the range (0..180) degrees";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_InvalidBendOffset_Error)
    {
    CreateParams params (GetModel(), "BP", 10.0, 1.0, Angle::FromDegrees (90), INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.bendOffset, "BendOffset", false);

    params.bendOffset = 0.0;
    EXPECT_FAIL_Insert (params) << "BendOffset must be greater than zero";

    params.bendOffset = params.width;
    EXPECT_FAIL_Insert (params) << "BendOffset must be less than width";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (BentPlateProfileTestCase, Insert_FilletRadiusAgainst90DegBendAngle_CorrectResult)
    {
    CreateParams params (GetModel(), "BP", 10.0, 4.0, Angle::FromDegrees (90), 5.0);

    params.filletRadius = 3.0;
    EXPECT_SUCCESS_Insert (params);

    params.filletRadius = 3.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params);
    }
