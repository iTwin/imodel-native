/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/IUlasProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "../UsageDb.h"
#include "../Policy.h"

#include <memory>

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IUlasProvider> IUlasProviderPtr;

struct IUlasProvider
    {
public:
    virtual BentleyStatus PostUsageLogs(UsageDb& usageDb, std::shared_ptr<Policy> policy) = 0;
    virtual BentleyStatus PostFeatureLogs(UsageDb& usageDb, std::shared_ptr<Policy> policy) = 0;
    virtual ~IUlasProvider() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
