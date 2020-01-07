/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "EntitlementProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

folly::Future<WebEntitlementResult> EntitlementProviderMock::FetchWebEntitlementV4(const std::vector<int>& productIds, BeVersionCR version, Utf8StringCR deviceId, Utf8StringCR projectId, Utf8StringCR accessToken, AuthType authType)
    {
    m_fetchWebEntitlementV4Calls++;
    WebEntitlementResult webEntitlement {m_productId, m_status, m_principalId};
    return folly::makeFuture(webEntitlement);
    }