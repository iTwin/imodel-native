/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/TShapeProfileTests.cpp $
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
struct TShapeProfileTestCase : ProfileValidationTestCase<TShapeProfile>
    {
public:
    typedef TShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (TShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "T");

    TShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }
