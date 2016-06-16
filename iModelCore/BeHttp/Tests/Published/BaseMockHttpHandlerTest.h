/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/BaseMockHttpHandlerTest.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include "TestsHelper.h"
#include "WebTestsHelper.h"

BEGIN_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE

class BaseMockHttpHandlerTest
    {
private:
    std::shared_ptr<MockHttpHandler>    m_handler;
    HttpClient                          m_client;
    
public:
    BaseMockHttpHandlerTest ();
    HttpClientCR GetClient () const;
    MockHttpHandler& GetHandler () const;
    std::shared_ptr<MockHttpHandler> GetHandlerPtr () const;
    };

END_BENTLEY_HTTP_UNIT_TESTS_NAMESPACE