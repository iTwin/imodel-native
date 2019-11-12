
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../WebTestsHelper.h"
#include "../FSTest.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpDummyBodyTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpDummyBodyTests, Write_BufferSize4_Returns4)
    {
    auto body = HttpDummyBody::Create();
    body->Open();

    EXPECT_EQ(4, body->Write("abc", 4));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpDummyBodyTests, Read_AfterWritingABuffer_Returns0)
    {
    auto body = HttpDummyBody::Create();
    body->Open();

    body->Write("abc", 4);
    EXPECT_EQ(0, body->Read((char *)"abc", 4));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       02/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpDummyBodyTests, GetLenght_AfterWritingABuffer_Returns0)
    {
    auto body = HttpDummyBody::Create();
    body->Open();

    body->Write("abc", 4);
    EXPECT_EQ(0, body->GetLength());
    }