/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Client/WSClientTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSClientTests.h"

#include <Bentley/BeDebugLog.h>
#include <BeHttp/ProxyHttpHandler.h>
#include <WebServices/Client/WSClient.h>

TEST_F (WSClientTests, GetServerInfo_TMAPrereleaseServer_UsesCorrectVersion)
    {
    auto proxy = ProxyHttpHandler::GetFiddlerProxyIfReachable ();

    Utf8String serverUrl = "http://10.228.96.112/ws47/";

    auto client = WSClient::Create (serverUrl, StubValidClientInfo (), proxy);

    auto result = client->GetServerInfo (nullptr)->GetResult ();
    ASSERT_TRUE (result.IsSuccess ());
    EXPECT_EQ (WSInfo::Type::BentleyWSG, result.GetValue ().GetType ());
    EXPECT_EQ (BeVersion(2, 0), result.GetValue ().GetVersion ());
    EXPECT_EQ (BeVersion (2, 0), result.GetValue ().GetWebApiVersion ());
    }
