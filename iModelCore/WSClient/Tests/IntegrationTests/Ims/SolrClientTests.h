/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/IntegrationTests/Ims/SolrClientTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServicesTestsHelper.h>
#include <BeHttp/ProxyHttpHandler.h>

struct SolrClientTests : public WSClientBaseTest
    {
    StubLocalState m_localState;
    std::shared_ptr<ProxyHttpHandler> m_proxy;
    Utf8String m_serverUrl;
    Credentials m_credentials;
    void SetUp();
    };
