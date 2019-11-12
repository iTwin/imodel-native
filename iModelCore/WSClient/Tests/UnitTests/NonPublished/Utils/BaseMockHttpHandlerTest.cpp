/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "BaseMockHttpHandlerTest.h"

USING_NAMESPACE_WSCLIENT_UNITTESTS

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

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseMockHttpHandlerTest::TearDown()
    {
    m_handler->ValidateAndClearExpectations();
    WSClientBaseTest::TearDown();
    }