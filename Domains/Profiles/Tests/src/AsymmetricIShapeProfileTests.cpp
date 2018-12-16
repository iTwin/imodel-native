/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/AsymmetricIShapeProfileTests.cpp $
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
struct AsymmetricIShapeProfileTestCase : ProfileValidationTestCase<AsymmetricIShapeProfile>
    {
public:
    typedef AsymmetricIShapeProfile::CreateParams CreateParams;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (AsymmetricIShapeProfileTestCase, Create_MinimalCreateParams_ValidInstance)
    {
    CreateParams params (GetModel(), "AsymmetricI");

    AsymmetricIShapeProfilePtr profilePtr = CreateProfile (params);
    EXPECT_TRUE (profilePtr.IsValid());
    }
