/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Configuration/BuddiClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BuddiClientTests.h"

#include <WebServices/Configuration/BuddiClient.h>
#include <BeHttp/ProxyHttpHandler.h>

TEST_F(BuddiClientTests, GetRegions_Default_ReturnsSomeRegions)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    BuddiClient client(proxy);

    auto result = client.GetRegions()->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    EXPECT_FALSE(result.GetValue().empty());
    }

TEST_F(BuddiClientTests, GetUrl_ExistingUrlName_ReturnsUrl)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    BuddiClient client(proxy);

    auto result = client.GetUrl("Login")->GetResult();
    EXPECT_TRUE(result.IsSuccess());
    }

TEST_F(BuddiClientTests, GetUrl_NotExistingUrlName_ReturnsUrlNotConfiguredError)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable();
    BuddiClient client(proxy);

    auto result = client.GetUrl("NotExistingUrlName")->GetResult();

    EXPECT_STREQ("", result.GetValue().c_str());
    EXPECT_EQ(BuddiError::UrlNotConfigured, result.GetError().GetStatus());
    }
