/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/UlasProviderTests.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestsHelper.h"
#include "Utils/MockHttpHandler.h"
#include "Mocks/LicensingDbMock.h"
#include "Mocks/BuddiProviderMock.h"
#include "../../Licensing/Providers/UlasProvider.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

class UlasProviderTests : public ::testing::Test
{
private:
    std::shared_ptr<LicensingDbMock>                    m_licensingDbMock;
    std::shared_ptr<MockHttpHandler>                    m_handlerMock;
    std::shared_ptr<BuddiProviderMock>                  m_buddiMock;
    std::shared_ptr<UlasProvider>                       m_ulasProvider;
public:
    UlasProviderTests();
    static void SetUpTestCase();

    UlasProvider& GetUlasProvider() const;

    LicensingDbMock& GetLicensingDbMock() const;
    std::shared_ptr<LicensingDbMock> GetLicensingDbMockPtr() const;

    MockHttpHandler& GetMockHttp() const;
    std::shared_ptr<MockHttpHandler> GetHandlerPtr() const;

    BuddiProviderMock& GetMockBuddi() const;

    Utf8String MockUlasUrl();

    void TearDown();
};
