/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <WebServicesTestsHelper.h>
#include <WebServices/Configuration/BuddiClient.h>
#include <BeHttp/ProxyHttpHandler.h>

struct BuddiClientTests : WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetRegions_Default_ReturnsSomeRegions)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    BuddiClient client(proxy);

    auto result = client.GetRegions()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetUrl_ExistingUrlName_ReturnsUrl)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    BuddiClient client(proxy);

    auto result = client.GetUrl("Login")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

/*--------------------------------------------------------------------------------------+
* @bsitest                                    Vincas.Razma                     12/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BuddiClientTests, GetUrl_NotExistingUrlName_ReturnsUrlNotConfiguredError)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    BuddiClient client(proxy);

    auto result = client.GetUrl("NotExistingUrlName")->GetResult();

    EXPECT_STREQ("", result.GetValue().c_str());
    EXPECT_EQ(BuddiError::UrlNotConfigured, result.GetError().GetStatus());
    }
