/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ConnectTestsHelper.h"
#include <WebServices/Connect/Passport.h>
#include <Bentley/Base64Utilities.h>

#include <WebServices/Configuration/UrlProvider.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

struct PassportTests : BaseMockHttpHandlerTest
    {
private:
    std::shared_ptr<StubBuddiClient> m_client;
    RuntimeJsonLocalState m_localState;

public:
    virtual void SetUp() override
        {
        Passport::Initialize(GetHandlerPtr());
        m_client = std::make_shared<StubBuddiClient>();
        UrlProvider::Initialize(UrlProvider::Environment::Dev, UrlProvider::DefaultTimeout, &m_localState, m_client);
        };
    virtual void TearDown() override
        {
        Passport::Uninintialize();
        };
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PassportTests, User_Has_Passport)
    {
    Json::Value bodyJson = true;
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        EXPECT_TRUE (Utf8String::npos != request.GetUrl().find("82C9DF1F-12A0-45F0-A48E-CE9D928C2590"));
        return StubHttpResponse(HttpStatus::OK, bodyJson.toStyledString());
        });

    auto status = Passport::HasUserPassport("82C9DF1F-12A0-45F0-A48E-CE9D928C2590");
    EXPECT_EQ (SUCCESS, status);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier    04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (PassportTests, User_Doesnt_Have_Passport)
    {
    Json::Value bodyJson = false;
    GetHandler().ForFirstRequest([&] (Http::RequestCR request)
        {
        EXPECT_TRUE (Utf8String::npos != request.GetUrl().find("0F0E744F-D703-48AC-826E-C172852186EE"));
        return StubHttpResponse(HttpStatus::OK, bodyJson.toStyledString());
        });

    auto status = Passport::HasUserPassport("0F0E744F-D703-48AC-826E-C172852186EE");
    EXPECT_NE (SUCCESS, status);
    }
