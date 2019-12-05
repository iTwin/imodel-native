/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <memory>
#include <vector>
#include <folly/BeFolly.h>
#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include "WebEntitlementResult.h"
#include "../Policy.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef std::shared_ptr<struct IEntitlementProvider> IEntitlementProviderPtr;

struct IEntitlementProvider
{
public:
	virtual folly::Future<WebEntitlementResult> FetchWebEntitlementV4(const std::vector<int>& productIds, BeVersionCR version, Utf8StringCR deviceId, Utf8StringCR projectId, Utf8StringCR accessToken) = 0;
	virtual ~IEntitlementProvider() {};
};

END_BENTLEY_LICENSING_NAMESPACE