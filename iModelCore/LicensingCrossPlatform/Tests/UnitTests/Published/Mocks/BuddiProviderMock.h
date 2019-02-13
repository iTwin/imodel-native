/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Mocks/BuddiProviderMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/Utils/IBuddiProvider.h>

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct BuddiProviderMock : IBuddiProvider
{
public:
    MOCK_METHOD0(UlasLocationBaseUrl, Utf8String());
    MOCK_METHOD0(EntitlementPolicyBaseUrl, Utf8String());
    MOCK_METHOD0(UlasRealtimeLoggingBaseUrl, Utf8String());
};

END_BENTLEY_LICENSING_NAMESPACE
