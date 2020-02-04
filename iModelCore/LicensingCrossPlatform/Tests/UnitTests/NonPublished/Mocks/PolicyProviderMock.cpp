/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PolicyProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

folly::Future<std::shared_ptr<Policy>> PolicyProviderMock::GetPolicy()
    {
    m_getPolicyCalls++;
    return folly::makeFuture(m_mockedGetPolicy);
    }

folly::Future<std::shared_ptr<Policy>> PolicyProviderMock::GetPolicy(Utf8StringCR projectId)
    {
    m_getPolicyCalls++;
    return folly::makeFuture(m_mockedGetPolicy);
    }

folly::Future<std::shared_ptr<Policy>> PolicyProviderMock::GetPolicyWithKey(Utf8StringCR accessKey, Utf8StringCR ultimateId)
    {
    m_getPolicyWithKeyCalls++;
    return folly::makeFuture(m_mockedGetPolicyWithKey);
    }
