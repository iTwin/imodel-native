/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Utils/BaseMockHttpHandlerTest.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include "MockHttpHandler.h"
#include "WSClientBaseTest.h"

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class BaseMockHttpHandlerTest : public WSClientBaseTest
    {
    private:
        std::shared_ptr<MockHttpHandler>    m_handler;
        HttpClient                          m_client;

    public:
        BaseMockHttpHandlerTest();
        HttpClientCR GetClient() const;
        MockHttpHandler& GetHandler() const;
        std::shared_ptr<MockHttpHandler> GetHandlerPtr() const;
    };

END_WSCLIENT_UNITTESTS_NAMESPACE