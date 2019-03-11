/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Mocks/PolicyProviderMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../Licensing/Providers/IPolicyProvider.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct PolicyProviderMock : IPolicyProvider
{
public:
    folly::Future<std::shared_ptr<Policy>> GetPolicy() override;
    folly::Future<std::shared_ptr<Policy>> GetPolicyWithKey(Utf8StringCR accessKey) override;
    void MockGetPolicy(std::shared_ptr<Policy> mocked) { m_mockedGetPolicy = mocked; }
    void MockGetPolicyWithKey(std::shared_ptr<Policy> mocked) { m_mockedGetPolicyWithKey = mocked; };
    int GetPolicyCalls() const { return m_getPolicyCalls; };
    int GetPolicyWithKeyCalls() const { return m_getPolicyWithKeyCalls; };
private:
    int m_getPolicyCalls = 0;
    int m_getPolicyWithKeyCalls = 0;
    std::shared_ptr<Policy> m_mockedGetPolicy = nullptr;
    std::shared_ptr<Policy> m_mockedGetPolicyWithKey = nullptr;
};

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
