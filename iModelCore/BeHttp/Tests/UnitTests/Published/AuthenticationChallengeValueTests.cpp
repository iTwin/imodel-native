/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"
#include <BeHttp/HttpHeaders.h>

USING_NAMESPACE_BENTLEY_HTTP

struct AuthenticationChallengeValueTests : public ::testing::Test {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, Parse_NullValue_ErrorAndTypeAndRealmEmpty)
    {
    AuthenticationChallengeValue value;
    EXPECT_EQ(ERROR, AuthenticationChallengeValue::Parse(nullptr, value));
    EXPECT_EQ("", value.GetType());
    EXPECT_EQ("", value.GetRealm());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, Parse_NoValue_ErrorAndTypeAndRealmEmpty)
    {
    AuthenticationChallengeValue value;
    EXPECT_EQ(ERROR, AuthenticationChallengeValue::Parse("", value));
    EXPECT_EQ("", value.GetType());
    EXPECT_EQ("", value.GetRealm());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, Parse_InvalidValue_ErrorAndTypeAndRealmEmpty)
    {
    AuthenticationChallengeValue value;
    EXPECT_EQ(ERROR, AuthenticationChallengeValue::Parse("Some Invalid Value", value));
    EXPECT_EQ("", value.GetType());
    EXPECT_EQ("", value.GetRealm());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, Parse_RealmOnly_ErrorAndTypeAndRealmEmpty)
    {
    AuthenticationChallengeValue value;
    EXPECT_EQ(ERROR, AuthenticationChallengeValue::Parse("realm=\"Test realm here.\"", value));
    EXPECT_EQ("", value.GetType());
    EXPECT_EQ("", value.GetRealm());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, Parse_TypeOnly_TypeSetAndRealmEmpty)
    {
    AuthenticationChallengeValue value;
    EXPECT_EQ(SUCCESS, AuthenticationChallengeValue::Parse("Some-Type42", value));
    EXPECT_EQ("Some-Type42", value.GetType());
    EXPECT_EQ("", value.GetRealm());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, Parse_TypeAndRealm_TypeAndRealmSet)
    {
    AuthenticationChallengeValue value;
    EXPECT_EQ(SUCCESS, AuthenticationChallengeValue::Parse("SomeType realm=\"Test realm here.\"", value));
    EXPECT_EQ("SomeType", value.GetType());
    EXPECT_EQ("Test realm here.", value.GetRealm());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, ToString_Default_Empty)
    {
    AuthenticationChallengeValue value;
    EXPECT_EQ("", value.ToString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, ToString_TypeOnly_TypeReturned)
    {
    AuthenticationChallengeValue value("SomeType", "");
    EXPECT_EQ("SomeType", value.ToString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, ToString_RealmOnly_EmptyReturnedForError)
    {
    AuthenticationChallengeValue value("", "TestRealm");
    EXPECT_EQ("", value.ToString());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(AuthenticationChallengeValueTests, ToString_TypeAndRealm_FullFormatReturned)
    {
    AuthenticationChallengeValue value("SomeType", "Test realm here.");
    EXPECT_EQ("SomeType realm=\"Test realm here.\"", value.ToString());
    }
