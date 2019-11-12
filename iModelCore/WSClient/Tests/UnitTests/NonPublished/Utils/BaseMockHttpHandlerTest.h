/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "WebServicesUnitTests.h"
#include "MockHttpHandler.h"
#include "WSClientBaseTest.h"

BEGIN_WSCLIENT_UNITTESTS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct BaseMockHttpHandlerTest : WSClientBaseTest
    {
    private:
        std::shared_ptr<MockHttpHandler>    m_handler;
        HttpClient                          m_client;

    public:
        BaseMockHttpHandlerTest();
        HttpClientCR GetClient() const;
        MockHttpHandler& GetHandler() const;
        std::shared_ptr<MockHttpHandler> GetHandlerPtr() const;

        void TearDown();
    };

END_WSCLIENT_UNITTESTS_NAMESPACE
