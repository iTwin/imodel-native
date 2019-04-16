/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include <WebServices/Configuration/UrlProvider.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IBuddiProvider> IBuddiProviderPtr;

struct IBuddiProvider
    {
public:
    virtual Utf8String UlasLocationBaseUrl() = 0;
    virtual Utf8String EntitlementPolicyBaseUrl() = 0;
    virtual Utf8String UlasRealtimeLoggingBaseUrl() = 0;
    virtual Utf8String UlasRealtimeFeatureUrl() = 0;
    virtual Utf8String UlasAccessKeyBaseUrl() = 0;
    virtual ~IBuddiProvider() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
