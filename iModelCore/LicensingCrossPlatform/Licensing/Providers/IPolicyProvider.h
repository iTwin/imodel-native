/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/IPolicyProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

//#include <WebServices/Configuration/UrlProvider.h>

#include "../Policy.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IPolicyProvider> IPolicyProviderPtr;

struct IPolicyProvider
{
public:
    virtual folly::Future<std::shared_ptr<Policy>> GetPolicy() = 0;
    virtual folly::Future<std::shared_ptr<Policy>> GetPolicyWithKey(Utf8StringCR accessKey) = 0;
    virtual ~IPolicyProvider() {};
};

END_BENTLEY_LICENSING_NAMESPACE
