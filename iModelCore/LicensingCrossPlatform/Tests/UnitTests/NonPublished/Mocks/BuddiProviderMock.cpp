/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "BuddiProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

Utf8String BuddiProviderMock::EntitlementPolicyBaseUrl()
    {
    return m_mockedEntitlementPolicyBaseUrl;
    }

Utf8String BuddiProviderMock::UlasLocationBaseUrl()
    {
    return m_mockedUlasLocationBaseUrl;
    }

Utf8String BuddiProviderMock::UlasRealtimeLoggingBaseUrl()
    {
    return m_mockedUlasRealtimeLoggingBaseUrl;
    }

Utf8String BuddiProviderMock::UlasRealtimeFeatureUrl()
    {
    return m_mockedUlasRealtimeFeatureUrl;
    }

Utf8String BuddiProviderMock::UlasAccessKeyBaseUrl()
    {
    return m_mockedUlasAccessKeyBaseUrl;
    }

