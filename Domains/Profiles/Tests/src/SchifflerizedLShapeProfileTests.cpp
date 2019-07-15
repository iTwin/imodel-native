/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ProfileValidationTestCase.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_PROFILES

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                                      02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct SchifflerizedLShapeProfileTestCase : ProfileValidationTestCase<SchifflerizedLShapeProfile>
    {
public:
    typedef SchifflerizedLShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "L");

    SchifflerizedLShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_ValidCreateParams_Success)
    {
    CreateParams requiredParams (GetModel(), "L", 10.0, 1.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "L", 10.0, 1.0, 1.0, 0.5, 0.5);
    EXPECT_SUCCESS_Insert (fullParams) << "Profile should succeed to insert with valid full create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_EmptyCreateParams_Error)
    {
    CreateParams params (GetModel(), "L");
    EXPECT_FAIL_Insert (params) << "Profile should fail with empty create parameters.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, GetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams params (GetModel(), "L", 1.0, 2.0, 3.0, 4.0, 5.0);

    SchifflerizedLShapeProfilePtr profilePtr = CreateProfile (params);
    ASSERT_TRUE (profilePtr.IsValid());

    EXPECT_EQ ("L", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetLegLength());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetThickness());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetLegBendOffset());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetEdgeRadius());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, SetProperties_ProfileInstance_ValidProperties)
    {
    CreateParams createParams (GetModel(), "", 0.0, 0.0, 0.0, 0.0, 0.0);

    SchifflerizedLShapeProfilePtr profilePtr = CreateProfile (createParams);
    ASSERT_TRUE (profilePtr.IsValid());

    profilePtr->SetName ("L");
    profilePtr->SetLegLength (1.0);
    profilePtr->SetThickness (2.0);
    profilePtr->SetLegBendOffset (3.0);
    profilePtr->SetFilletRadius (4.0);
    profilePtr->SetEdgeRadius (5.0);

    EXPECT_EQ ("L", profilePtr->GetName());
    EXPECT_DOUBLE_EQ (1.0, profilePtr->GetLegLength());
    EXPECT_DOUBLE_EQ (2.0, profilePtr->GetThickness());
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetLegBendOffset());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetFilletRadius());
    EXPECT_DOUBLE_EQ (5.0, profilePtr->GetEdgeRadius());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_InvalidProfileName_Error)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 1.0);

    params.name = nullptr;
    EXPECT_FAIL_Insert (params) << "Profile name cannot be null.";

    params.name = "";
    EXPECT_FAIL_Insert (params) << "Profile name cannot be empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_ValidProfileName_Success)
    {
    CreateParams params (GetModel(), nullptr, 10.0, 1.0);

    params.name = "L";
    EXPECT_SUCCESS_Insert (params) << "Profile name should be non empty.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_InvalidLegLength_Error)
    {
    CreateParams params (GetModel(), "L", INFINITY, 1.0);

    TestParameterToBeFiniteAndPositive (params, params.legLength, "LegLength", false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_ValidLegLength_Success)
    {
    CreateParams params (GetModel(), "L", INFINITY, 1.0);

    params.legLength = 10.0;
    EXPECT_SUCCESS_Insert (params) << "LegLength should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_InvalidThickness_Error)
    {
    CreateParams params (GetModel(), "L", 10.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.thickness, "Thickness", false);

    params.thickness = params.legLength / std::sqrt (3.0);
    EXPECT_FAIL_Insert (params) << "Thickness should be less than legLength divided by sqrt (3.0).";

    params.thickness -= TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "Thickness should be less than legLength divided by sqrt (3.0).";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_ValidThickness_Success)
    {
    CreateParams params (GetModel(), "L", 10.0, INFINITY);

    params.thickness = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Thickness should be positive value.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_InvalidLegBendOffset_Fail)
    {
    CreateParams params (GetModel(), "L", 10.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.legBendOffset, "LegBendOffset", true);

    params.legBendOffset = 9.0;
    EXPECT_FAIL_Insert (params) << "LegBendOffset should be less than legLength minus thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_ValidLegBendOffset_Fail)
    {
    CreateParams params (GetModel(), "L", 10.0, 1.0, INFINITY);

    params.legBendOffset = 9.0 - TESTS_EPSILON;
    EXPECT_SUCCESS_Insert (params) << "LegBendOffset should be less than legLength minus thickness.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_FilletRadiusGreaterThanLegBendOffset_Error)
    {
    CreateParams params (GetModel(), "L", 10.0, 1.0, 1.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.filletRadius, "FilletRadius", true);

    params.filletRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "FilletRadius should be less or equal to legBendOffset.";

    params.filletRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "FilletRadius should be less or equal to legBendOffset.";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_InvalidEdgeRadius_Error)
    {
    CreateParams params (GetModel(), "L", 10.0, 1.0, 0.0, 0.0, INFINITY);

    TestParameterToBeFiniteAndPositive (params, params.edgeRadius, "EdgeRadius", true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (SchifflerizedLShapeProfileTestCase, Insert_EdgeRadiusAgainstThickness_CorrectInsertResult)
    {
    CreateParams params (GetModel(), "L", 10.0, 1.0, 1.0, 0.0, INFINITY);

    params.edgeRadius = 1.0;
    EXPECT_SUCCESS_Insert (params) << "Edge radius should be less or equal to the thickness.";

    params.edgeRadius = 1.0 + TESTS_EPSILON;
    EXPECT_FAIL_Insert (params) << "Edge radius should be less or equal to half of the thickness.";
    }
