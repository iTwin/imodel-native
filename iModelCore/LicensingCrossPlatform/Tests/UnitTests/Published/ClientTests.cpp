/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Tests/UnitTests/Published/ClientTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ClientTests.h"

#include <Licensing/Client.h>

#include <BeHttp/HttpClient.h>

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS
USING_NAMESPACE_BENTLEY_HTTP

TEST_F (ClientTests, DummyTest)
    {
    BeFileName asssetsDir;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(asssetsDir);

    HttpClient::Initialize(asssetsDir);

	Client client;
    client.TestMethod();
    }
