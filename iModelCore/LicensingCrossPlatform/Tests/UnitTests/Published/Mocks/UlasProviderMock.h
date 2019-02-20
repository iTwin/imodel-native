/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Mocks/UlasProviderMock.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../../Licensing/Providers/IUlasProvider.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct UlasProviderMock : IUlasProvider
    {
public:
    MOCK_METHOD2(PostUsageLogs, BentleyStatus(UsageDb& usageDb, std::shared_ptr<Policy> policy));
    MOCK_METHOD2(SendUsageLogs, BentleyStatus(BeFileNameCR usageCSV, Utf8StringCR ultId));
    MOCK_METHOD2(PostFeatureLogs, BentleyStatus(UsageDb& usageDb, std::shared_ptr<Policy> policy));
    MOCK_METHOD2(SendFeatureLogs, BentleyStatus(BeFileNameCR featureCSV, Utf8StringCR ultId));
    };

END_BENTLEY_LICENSING_NAMESPACE
