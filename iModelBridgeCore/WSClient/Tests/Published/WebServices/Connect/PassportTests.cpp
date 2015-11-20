/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/WebServices/Connect/PassportTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PassportTests.h"
#include <WebServices/Connect/Passport.h>
#include <Bentley/Base64Utilities.h>

#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES
USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

void PassportTests::SetUp ()
    {
    Passport::Initialize (GetHandlerPtr ());
    m_client = std::make_shared<StubBuddiClient>();
    UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_client);
    }

void PassportTests::TearDown ()
    {
    Passport::Uninintialize ();
    }

TEST_F (PassportTests, User_Has_Passport)
    {
    Json::Value bodyJson = true;
    GetHandler ().ForFirstRequest ([&] (HttpRequestCR request)
        {
        EXPECT_TRUE (Utf8String::npos != request.GetUrl().find("82C9DF1F-12A0-45F0-A48E-CE9D928C2590"));
        return StubHttpResponse (HttpStatus::OK, bodyJson.toStyledString ());
        });

    auto status = Passport::HasUserPassport ("82C9DF1F-12A0-45F0-A48E-CE9D928C2590");
    EXPECT_EQ (SUCCESS, status);
    }

TEST_F (PassportTests, User_Doesnt_Have_Passport)
    {
    Json::Value bodyJson = false;
    GetHandler ().ForFirstRequest ([&] (HttpRequestCR request)
        {
        EXPECT_TRUE (Utf8String::npos != request.GetUrl().find("0F0E744F-D703-48AC-826E-C172852186EE"));
        return StubHttpResponse (HttpStatus::OK, bodyJson.toStyledString ());
        });

    auto status = Passport::HasUserPassport ("0F0E744F-D703-48AC-826E-C172852186EE");
    EXPECT_NE (SUCCESS, status);
    }