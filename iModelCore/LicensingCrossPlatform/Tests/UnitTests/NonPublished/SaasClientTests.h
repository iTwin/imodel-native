/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"

#include "Mocks/UlasProviderMock.h"
#include "Mocks/EntitlementProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class SaasClientTests : public ::testing::Test
    {
    private:
        std::shared_ptr<UlasProviderMock>          m_ulasProviderMock;
        std::shared_ptr<EntitlementProviderMock>   m_entitlementProviderMock;

    public:
        SaasClientTests();
        static void SetUpTestCase();

        UlasProviderMock&                        GetUlasProviderMock() const;
        std::shared_ptr<UlasProviderMock>        GetUlasProviderMockPtr() const;
        EntitlementProviderMock&                 GetEntitlementProviderMock() const;
        std::shared_ptr<EntitlementProviderMock> GetEntitlementProviderMockPtr() const;

        void TearDown();
    };
