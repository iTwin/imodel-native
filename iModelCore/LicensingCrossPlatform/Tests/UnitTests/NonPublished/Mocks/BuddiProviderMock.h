/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/NonPublished/Mocks/BuddiProviderMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../../Licensing/Providers/IBuddiProvider.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct BuddiProviderMock : IBuddiProvider
    {
private:
    Utf8String m_mockedEntitlementPolicyBaseUrl = "";
    Utf8String m_mockedUlasLocationBaseUrl = "";
    Utf8String m_mockedUlasRealtimeLoggingBaseUrl = "";
    Utf8String m_mockedUlasRealtimeFeatureUrl = "";
    Utf8String m_mockedUlasAccessKeyBaseUrl = "";

public:
    Utf8String EntitlementPolicyBaseUrl() override;
    Utf8String UlasLocationBaseUrl() override;
    Utf8String UlasRealtimeLoggingBaseUrl() override;
    Utf8String UlasRealtimeFeatureUrl() override;
    Utf8String UlasAccessKeyBaseUrl() override;

    void MockEntitlementPolicyBaseUrl(Utf8String mocked) { m_mockedEntitlementPolicyBaseUrl = mocked; };
    void MockUlasLocationBaseUrl(Utf8String mocked) { m_mockedUlasLocationBaseUrl = mocked; };
    void MockUlasRealtimeLoggingBaseUrl(Utf8String mocked) { m_mockedUlasRealtimeLoggingBaseUrl = mocked; };
    void MockUlasRealtimeFeatureUrl(Utf8String mocked) { m_mockedUlasRealtimeFeatureUrl = mocked; };
    void MockUlasAccessKeyBaseUrl(Utf8String mocked) { m_mockedUlasAccessKeyBaseUrl = mocked; };
    };

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
