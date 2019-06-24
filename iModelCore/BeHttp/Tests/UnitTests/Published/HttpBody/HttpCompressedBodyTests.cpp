
/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../WebTestsHelper.h"
#include "../FSTest.h"

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpCompressedBodyTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, SetPosition_NullPointerContent_Error)
    {
    auto body = HttpCompressedBody::Create(nullptr);
    body->Open();

    ASSERT_EQ(ERROR, body->SetPosition(2));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, SetPosition_WithoutOpen_Error)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));

    ASSERT_EQ(ERROR, body->SetPosition(2));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, SetPosition_PositionToContent_PositionSet)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    ASSERT_EQ(SUCCESS, body->SetPosition(2));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, SetPosition_AfterClose_PositionSet)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();
    body->Close();

    ASSERT_EQ(SUCCESS, body->SetPosition(2));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, SetPosition_PositionOutsideContent_Error)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    ASSERT_EQ(ERROR, body->SetPosition(body->GetLength() + 1));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Reset_NullPointerContent_SucceedsAndDoesNothing)
    {
    auto body = HttpCompressedBody::Create(nullptr);
    body->Open();

    ASSERT_EQ(SUCCESS, body->Reset());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Reset_NonEmptyContent_ContentNotChanged)
    {
    auto innerBody = HttpStringBody::Create("foo");
    auto body = HttpCompressedBody::Create(innerBody);
    body->Open();

    ASSERT_EQ(SUCCESS, body->Reset());
    ASSERT_STREQ("foo", innerBody->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Reset_PositionSet_ClearsPosition)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    ASSERT_EQ(SUCCESS, body->SetPosition(2));
    ASSERT_EQ(SUCCESS, body->Reset());

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Reset_PositionSetForContentBody_ContentBodyPositionNotChanged)
    {
    auto innerBody = HttpStringBody::Create("foo");
    auto body = HttpCompressedBody::Create(innerBody);
    body->Open();

    ASSERT_EQ(SUCCESS, body->SetPosition(2));
    ASSERT_EQ(SUCCESS, innerBody->SetPosition(2));
    ASSERT_EQ(SUCCESS, body->Reset());

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, innerBody->GetPosition(position));
    EXPECT_EQ(2, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Write_NullPointerContent_WritesNothing)
    {
    auto body = HttpCompressedBody::Create(nullptr);
    body->Open();

    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(0, body->Write("a", 1));
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Write_NonEmptyContent_WritesNothing)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    BeTest::SetFailOnAssert(false);
    ASSERT_EQ(0, body->Write("a", 1));
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_NullPointerContent_ReadsNothing)
    {
    auto body = HttpCompressedBody::Create(nullptr);
    body->Open();

    char buffer[32] = "";
    ASSERT_EQ(0, body->Read(buffer, 1));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_NonEmptyContentWithoutOpen_ReadsNothing)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));

    char buffer[32] = "";
    ASSERT_EQ(0, body->Read(buffer, 1));
    EXPECT_STREQ("", buffer);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_NonEmptyContent_Succeeds)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    char buffer[32] = "";
    ASSERT_EQ(body->GetLength(), body->Read(buffer, 32));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_NonEmptyContent_ContentNotChanged)
    {
    auto innerBody = HttpStringBody::Create("foo");
    auto body = HttpCompressedBody::Create(innerBody);
    body->Open();

    char buffer[32] = "";
    ASSERT_EQ(body->GetLength(), body->Read(buffer, 32));
    EXPECT_STREQ("foo", innerBody->AsString().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_NonEmptyContent_ContentBodyPositionNotChanged)
    {
    auto innerBody = HttpStringBody::Create("foo");
    auto body = HttpCompressedBody::Create(innerBody);
    body->Open();

    uint64_t originalPosition = 0;
    ASSERT_EQ(SUCCESS, innerBody->GetPosition(originalPosition));

    char buffer[32] = "";
    ASSERT_EQ(body->GetLength(), body->Read(buffer, 32));

    uint64_t position = 0;
    ASSERT_EQ(SUCCESS, innerBody->GetPosition(position));
    ASSERT_EQ(originalPosition, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_NonEmptyContentAfterClose_Succeeds)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();
    body->Close();

    char buffer[32] = "";
    ASSERT_EQ(body->GetLength(), body->Read(buffer, 32));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_AfterReset_ReadsNothing)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();
    
    ASSERT_EQ(SUCCESS, body->Reset());

    char buffer[32] = "";
    ASSERT_EQ(0, body->Read(buffer, 32));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Read_MultipleTimes_IncrementsPosition)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    char buffer[32] = "";
    uint64_t position = 0;

    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(0, position);

    ASSERT_EQ(2, body->Read(buffer, 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(2, position);

    std::fill_n(buffer, 32, (char)0);
    ASSERT_EQ(2, body->Read(buffer, 2));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(4, position);

    ASSERT_EQ(1, body->Read(buffer, 1));
    ASSERT_EQ(SUCCESS, body->GetPosition(position));
    EXPECT_EQ(5, position);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, GetLenght_NullPointerContent_ReturnsZero)
    {
    auto body = HttpCompressedBody::Create(nullptr);
    body->Open();

    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, GetLenght_EmptyContent_ReturnsLenght)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create());
    body->Open();

    EXPECT_LT(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, GetLenght_NonEmptyContentWithoutOpen_ReturnsZero)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));

    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, GetLenght_NonEmptySmallContent_ReturnsLenghtLargerThanContentLenght)
    {
    auto innerBody = HttpStringBody::Create("foo");
    auto body = HttpCompressedBody::Create(innerBody);
    body->Open();

    EXPECT_LT(innerBody->GetLength(), body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, GetLenght_NonEmptyLargeContent_ReturnsLenghtSmallerThanContentLenght)
    {
    auto innerBody = HttpStringBody::Create("abcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZabcdefghjklmnopQrstvwxyz1234567890!@#$%^&*()ABCDEFGHJKLMNOPQRSTVWXYZ");
    auto body = HttpCompressedBody::Create(innerBody);
    body->Open();

    EXPECT_GT(innerBody->GetLength(), body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, GetLenght_NonEmptyContentAfterClose_ReturnsLenght)
    {
    auto innerBody = HttpStringBody::Create("foo");
    auto body = HttpCompressedBody::Create(innerBody);
    body->Open();
    body->Close();

    EXPECT_LT(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, GetLenght_NonEmptyContentAfterReset_ReturnsZero)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    ASSERT_EQ(SUCCESS, body->Reset());

    EXPECT_EQ(0, body->GetLength());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, AsString_NullPointerContent_ReturnsEmptyString)
    {
    auto body = HttpCompressedBody::Create(nullptr);
    body->Open();

    BeTest::SetFailOnAssert(false);
    EXPECT_STREQ("", body->AsString().c_str());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, AsString_NonEmptyContentWithoutOpen_ReturnsEmptyString)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));

    BeTest::SetFailOnAssert(false);
    EXPECT_STREQ("", body->AsString().c_str());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, AsString_NonEmptyContent_ReturnsEmptyString)
    {
    auto body = HttpCompressedBody::Create(HttpStringBody::Create("foo"));
    body->Open();

    BeTest::SetFailOnAssert(false);
    EXPECT_STREQ("", body->AsString().c_str());
    BeTest::SetFailOnAssert(true);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Close_NullPointerContent_DoesNothing)
    {
    auto body = HttpCompressedBody::Create(nullptr);
    body->Open();
    body->Close();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Julius.Cepukenas                       11/16
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpCompressedBodyTests, Close_NonEmptyContent_DoesNothing)
    {
    auto innerBody = HttpStringBody::Create("foo");
    auto body = HttpCompressedBody::Create(innerBody);
    body -> Open();
    body -> Close();

    EXPECT_STREQ("foo", innerBody->AsString().c_str());
    }