/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/ClientTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "Utils/MockHttpHandler.h"

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class ClientTests : public ::testing::Test
    {
    private:
        std::shared_ptr<MockHttpHandler>    m_handler;
    public:
        ClientTests();
        static void SetUpTestCase();

        MockHttpHandler& GetHandler() const;
        std::shared_ptr<MockHttpHandler> GetHandlerPtr() const;

        void TearDown();
    };
