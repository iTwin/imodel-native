/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/SaasClientTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"

#include "Mocks/UlasProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class SaasClientTests : public ::testing::Test
    {
    private:
        std::shared_ptr<UlasProviderMock>        m_ulasProviderMock;

    public:
        SaasClientTests();
        static void SetUpTestCase();

        UlasProviderMock&                        GetUlasProviderMock() const;
        std::shared_ptr<UlasProviderMock>        GetUlasProviderMockPtr() const;

        void TearDown();
    };
