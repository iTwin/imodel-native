/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/BuddiProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "IBuddiProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct BuddiProvider : IBuddiProvider
{
public:
    Utf8String UlasLocationBaseUrl();
    Utf8String EntitlementPolicyBaseUrl();
    Utf8String UlasRealtimeLoggingBaseUrl();
};

END_BENTLEY_LICENSING_NAMESPACE
