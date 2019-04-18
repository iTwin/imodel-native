/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    LICENSING_EXPORT Utf8String UlasLocationBaseUrl();
    LICENSING_EXPORT Utf8String EntitlementPolicyBaseUrl();
    LICENSING_EXPORT Utf8String UlasRealtimeLoggingBaseUrl();
    LICENSING_EXPORT Utf8String UlasRealtimeFeatureUrl();
    LICENSING_EXPORT Utf8String UlasAccessKeyBaseUrl();
};

END_BENTLEY_LICENSING_NAMESPACE
