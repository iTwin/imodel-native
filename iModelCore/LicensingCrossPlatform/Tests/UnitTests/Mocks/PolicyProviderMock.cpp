#include "PolicyProviderMock.h"

USING_NAMESPACE_BENTLEY_LICENSING
USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

folly::Future<std::shared_ptr<Policy>> PolicyProviderMock::GetPolicy() {
    m_getPolicyCalls++;
    return folly::makeFuture(m_mockedGetPolicy);
}

folly::Future<std::shared_ptr<Policy>> PolicyProviderMock::GetPolicyWithKey(Utf8StringCR accessKey) {
    m_getPolicyWithKeyCalls++;
    return folly::makeFuture(m_mockedGetPolicyWithKey);
}
