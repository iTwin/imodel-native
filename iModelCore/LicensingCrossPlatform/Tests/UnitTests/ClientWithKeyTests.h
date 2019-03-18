/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/ClientWithKeyTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"

#include "Mocks/LicensingDbMock.h"
#include "Mocks/PolicyProviderMock.h"
#include "Mocks/UlasProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class ClientWithKeyTests : public ::testing::Test
    {
    private:
        std::shared_ptr<LicensingDbMock>         m_licensingDbMock;
        std::shared_ptr<PolicyProviderMock>      m_policyProviderMock;
        std::shared_ptr<UlasProviderMock>        m_ulasProviderMock;

    public:
        ClientWithKeyTests();
        static void SetUpTestCase();

        PolicyProviderMock&                      GetPolicyProviderMock() const;
        std::shared_ptr<PolicyProviderMock>      GetPolicyProviderMockPtr() const;
        UlasProviderMock&                        GetUlasProviderMock() const;
        std::shared_ptr<UlasProviderMock>        GetUlasProviderMockPtr() const;
        LicensingDbMock&                         GetLicensingDbMock() const;
        std::shared_ptr<LicensingDbMock>         GetLicensingDbMockPtr() const;

        void TearDown();
    };
