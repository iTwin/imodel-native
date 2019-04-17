/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "WebTestsHelper.h"
#include <BeHttp/HttpError.h>
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

struct HttpErrorTests : public ::testing::Test
    {
    static void SetUpTestCase()
        {
        BeFileName temp;
        BeTest::GetHost().GetTempDir(temp);
        ASSERT_EQ(DbResult::BE_SQLITE_OK, BeSQLiteLib::Initialize(temp));

        BeFileName path;
        BeTest::GetHost().GetDgnPlatformAssetsDirectory(path);
        path.AppendToPath(L"sqlang/BeHttp_en.sqlang.db3");

        ASSERT_EQ(SUCCESS, L10N::Initialize(BeSQLite::L10N::SqlangFiles(path)));
        }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, Ctor_ResponseResourceNotFound_MessageAndDefaultDescription)
    {
    Response response = StubHttpResponse(HttpStatus::NotFound);
    HttpError error(response);
    EXPECT_STREQ("Resource not found", error.GetDisplayMessage().c_str());
    EXPECT_STREQ("Http error: 404", error.GetDisplayDescription().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, Ctor_ResponseWithProxyAuthenticationRequiredAndNoHeaders_MessageAndNoDescription)
    {
    Response response = StubHttpResponse(HttpStatus::ProxyAuthenticationRequired);
    HttpError error(response);
    EXPECT_STREQ("Proxy authentication required", error.GetDisplayMessage().c_str());
    EXPECT_STREQ("", error.GetDisplayDescription().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, Ctor_ResponseWithProxyAuthenticationRequiredProxyAuthenticateHeaderWithoutRealm_MessageAndNoDescription)
    {
    Response response = StubHttpResponse(HttpStatus::ProxyAuthenticationRequired);
    response.GetContent()->GetHeaders().SetProxyAuthenticate("Foo");
    HttpError error(response);
    EXPECT_STREQ("Proxy authentication required", error.GetDisplayMessage().c_str());
    EXPECT_STREQ("", error.GetDisplayDescription().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, Ctor_ResponseWithProxyAuthenticationRequiredProxyAuthenticateHeaderWithRealm_MessageWithRealmAndNoDescription)
    {
    Response response = StubHttpResponse(HttpStatus::ProxyAuthenticationRequired);
    response.GetContent()->GetHeaders().SetProxyAuthenticate("Foo realm=\"Test server realm\"");
    HttpError error(response);
    EXPECT_STREQ("Proxy authentication required for server from 'Test server realm'", error.GetDisplayMessage().c_str());
    EXPECT_STREQ("", error.GetDisplayDescription().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, IsValid_DefaultCtor_False)
    {
    EXPECT_FALSE(HttpError().IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, IsValid_FromNotConnectedResponse_True)
    {
    for (int i = (int) ConnectionStatus::None; i <= (int) ConnectionStatus::UnknownError; i++)
        {
        BeTest::SetFailOnAssert(false);
        HttpError error(StubHttpResponse((ConnectionStatus) i));
        BeTest::SetFailOnAssert(true);
        EXPECT_TRUE(error.IsValid());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, IsValid_FromConnectedResponse_True)
    {
    for (int i = (int) HttpStatus::None; i <= (int) HttpStatus::NoneLast; i++)
        {
        BeTest::SetFailOnAssert(false);
        HttpError error(StubHttpResponse((HttpStatus) i));
        BeTest::SetFailOnAssert(true);
        EXPECT_TRUE(error.IsValid());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, IsValid_FromNotConnectedStatus_True)
    {
    for (int i = (int) ConnectionStatus::None; i <= (int) ConnectionStatus::UnknownError; i++)
        {
        BeTest::SetFailOnAssert(false);
        HttpError error((ConnectionStatus) i, HttpStatus::None);
        BeTest::SetFailOnAssert(true);
        EXPECT_TRUE(error.IsValid());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                Vincas.Razma                        10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(HttpErrorTests, IsValid_FromConnectedStatus_True)
    {
    for (int i = (int) HttpStatus::None; i <= (int) HttpStatus::NoneLast; i++)
        {
        BeTest::SetFailOnAssert(false);
        HttpError error(ConnectionStatus::OK, (HttpStatus) i);
        BeTest::SetFailOnAssert(true);
        EXPECT_TRUE(error.IsValid());
        }
    }
