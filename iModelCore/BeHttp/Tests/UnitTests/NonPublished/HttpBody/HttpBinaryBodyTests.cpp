
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "../WebTestsHelper.h"
#include "../FSTest.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpBinaryBodyTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Open_EmptyVector_DoesNothing)
    {
    auto vector = std::make_shared<bvector<char>>();
    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    EXPECT_TRUE(vector->empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Open_VectorWithContent_DoesNothing)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    EXPECT_FALSE(vector->empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, AsString_VectorWithContent_EmptyString)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);

    BeTest::SetFailOnAssert(false);
    EXPECT_STREQ("", body->AsString().c_str());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
//Not sure about this
TEST_F(HttpBinaryBodyTests, SetPosition_EmptyBinaryVector_Error)
    {
    auto vector = std::make_shared<bvector<char>>();
    auto body = HttpBinaryBody::Create(vector);
    
    ASSERT_NE(SUCCESS, body->SetPosition(10));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, SetPosition_PositionToContent_PositionSet)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);

    ASSERT_EQ(SUCCESS, body->SetPosition(2));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, SetPosition_PositionOutsideContent_Error)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);

    ASSERT_NE(SUCCESS, body->SetPosition(10));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, SetPosition_PositionOutsideContent_PositionNotReset)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);

    ASSERT_EQ(SUCCESS, body->SetPosition(2));
    ASSERT_NE(SUCCESS, body->SetPosition(10));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Reset_EmptyVector_SucceedsAndDoesNothing)
    {
    auto vector = std::make_shared<bvector<char>>();
    auto body = HttpBinaryBody::Create(vector);
    ASSERT_EQ(SUCCESS, body->Reset());

    EXPECT_TRUE(vector->empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Reset_VectorWithContent_EmptiesVector)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    ASSERT_EQ(SUCCESS, body->Reset());

    EXPECT_TRUE(vector->empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Reset_PositionSet_ClearsPosition)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);

    ASSERT_EQ(SUCCESS, body->SetPosition(2));
    ASSERT_EQ(SUCCESS, body->Reset());

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Write_EmptyVector_WritesContent)
    {
    auto vector = std::make_shared<bvector<char>>();
    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    ASSERT_EQ(3, body->Write("abc", 3));

    Utf8String str(vector->begin(), vector->end());
    EXPECT_STREQ("abc", str.c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Write_VectorWithContent_OverridesContentAtPosition)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    ASSERT_EQ(1, body->Write("d", 1));

    Utf8String str(vector->begin(), vector->end());
    EXPECT_EQ("dbc", str);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Write_VectorWithContentWithoutOpen_OverridesContentAtPosition)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);

    ASSERT_EQ(1, body->Write("d", 1));

    Utf8String str(vector->begin(), vector->end());
    EXPECT_EQ("dbc", str);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Write_AfterResetCalled_Succeeds)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "a";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    ASSERT_EQ(SUCCESS, body->Reset());

    ASSERT_EQ(1, body->Write("b", 1));

    Utf8String str(vector->begin(), vector->end());
    EXPECT_EQ("b", str);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Write_MultipleTimes_IncrementsPosition)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);

    ASSERT_EQ(2, body->Write("de", 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);

    Utf8String str(vector->begin(), vector->end());
    EXPECT_EQ("dec", str);

    ASSERT_EQ(2, body->Write("fg", 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(4, position);

    str.clear();
    str.insert(str.begin(), vector->begin(), vector->end());
    EXPECT_EQ("defg", str);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Write_PositionSetToContent_OverridesCotentAtPosition)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abcd";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    ASSERT_EQ(SUCCESS, body->SetPosition(1));
    ASSERT_EQ(2, body->Write("fe", 2));

    Utf8String str(vector->begin(), vector->end());
    EXPECT_EQ("afed", str);

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Read_EmptyVector_ReadsNothing)
    {
    auto vector = std::make_shared<bvector<char>>();
    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    char buffer[32] = "";
    EXPECT_EQ(0, body->Read(buffer, 32));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Read_VectorWithContent_ReadsContent)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abcd";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    char buffer[32] = "";
    EXPECT_EQ(4, body->Read(buffer, 32));
    EXPECT_STREQ("abcd", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Read_VectorWithContentAfterReset_ReadsNothing)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abcd";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();
    ASSERT_EQ(SUCCESS, body->Reset());

    char buffer[32] = "";
    EXPECT_EQ(0, body->Read(buffer, 32));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Read_MultipleTimes_IncrementsPosition)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    char buffer[32] = "";
    uint64_t position = 0;

    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);

    ASSERT_EQ(2, body->Read(buffer, 2));
    EXPECT_STREQ("ab", buffer);
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);

    std::fill_n(buffer, 32, (char)0);
    ASSERT_EQ(1, body->Read(buffer, 2));
    EXPECT_STREQ("c", buffer);
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);

    ASSERT_EQ(0, body->Read(buffer, 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, Read_PositionSetToContent_ReadsContentAtPosition)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    ASSERT_EQ(SUCCESS, body->SetPosition(1));
    char buffer[32] = "";
    ASSERT_EQ(2, body->Read(buffer, 2));
    EXPECT_STREQ("bc", buffer);

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(3, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, GetLength_EmptyVector_ReturnsZero)
    {
    auto vector = std::make_shared<bvector<char>>();
    auto body = HttpBinaryBody::Create(vector);
    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, GetLength_VectorWithContentWithoutOpen_ReturnsVectorLenght)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    EXPECT_EQ(3, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, GetLength_VectorWithContentAfterOpen_ReturnsVectorLenght)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    body->Open();

    EXPECT_EQ(3, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, GetLength_VectorWithContentAfterReset_ReturnsZero)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    ASSERT_EQ(SUCCESS, body->Reset());
    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpBinaryBodyTests, GetLength_VectorWithContentAfterWrite_ReturnsNewLength)
    {
    auto vector = std::make_shared<bvector<char>>();

    const char * values = "abc";
    const char * end = values + strlen(values);
    vector->insert(vector->end(), values, end);

    auto body = HttpBinaryBody::Create(vector);
    ASSERT_EQ(3, body->GetLength());
    ASSERT_EQ(6, body->Write("NewNew", 6));
    EXPECT_EQ(6, body->GetLength());
    }