/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/BaseMockHttpHandlerTest.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BaseMockHttpHandlerTest.h"

USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BaseMockHttpHandlerTest::BaseMockHttpHandlerTest() :
    m_handler(std::make_shared<MockHttpHandler>()),
    m_client(nullptr, m_handler)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HttpClientCR BaseMockHttpHandlerTest::GetClient() const
    {
    return m_client;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MockHttpHandler& BaseMockHttpHandlerTest::GetHandler() const
    {
    return *m_handler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<MockHttpHandler> BaseMockHttpHandlerTest::GetHandlerPtr() const
    {
    return m_handler;
    }
