/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/CenterLineLShapeProfileTests.cpp $
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
    CreateParams requiredParams (GetModel(), "CenterLineLShape", 10, 10, 1, 0.0);
    EXPECT_SUCCESS_Insert (requiredParams) << "Profile should succeed to insert with valid required create parameters.";

    CreateParams fullParams (GetModel(), "CenterLineLShape", 10, 10, 1, 1.20, 0.17);
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
