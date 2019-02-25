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
    MOCK_METHOD0(GetPolicy, folly::Future<std::shared_ptr<Policy>>());
};

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE
