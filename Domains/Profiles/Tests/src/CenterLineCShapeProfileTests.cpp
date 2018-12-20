/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/CenterLineCShapeProfileTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

    CreateParams fullParams (GetModel(), "CenterLineCShape", 10, 10, 1, 1, PI / 18);
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
    EXPECT_DOUBLE_EQ (3.0, profilePtr->GetGirth());
    EXPECT_DOUBLE_EQ (4.0, profilePtr->GetWallThickness());
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
