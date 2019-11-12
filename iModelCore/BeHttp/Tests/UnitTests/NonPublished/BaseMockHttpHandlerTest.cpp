/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "BaseMockHttpHandlerTest.h"
#include <BeSQLite/L10N.h>

USING_NAMESPACE_BENTLEY_HTTP_UNIT_TESTS

BaseMockHttpHandlerTest::BaseMockHttpHandlerTest() :
m_handler(std::make_shared<MockHttpHandler>()),
m_client(nullptr, m_handler)
    {}

HttpClientCR BaseMockHttpHandlerTest::GetClient() const
    {
    return m_client;
    }

MockHttpHandler& BaseMockHttpHandlerTest::GetHandler() const
    {
    return *m_handler;
    }

std::shared_ptr<MockHttpHandler> BaseMockHttpHandlerTest::GetHandlerPtr() const
    {
    return m_handler;
    }