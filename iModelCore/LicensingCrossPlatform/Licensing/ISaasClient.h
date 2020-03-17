/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Bentley/BeVersion.h>
#include <folly/BeFolly.h>
#include <Licensing/Utils/FeatureEvent.h>
#include <Licensing/UsageType.h>
#include <Licensing/AuthType.h>
#include <Licensing/EntitlementResult.h>

#include "TrackUsageStatus.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct ISaasClient
    {
public:
    virtual folly::Future<TrackUsageStatus> TrackUsage
		(
		Utf8StringCR accessToken,
		BeVersionCR version,
		Utf8StringCR projectId,
		AuthType authType,
		std::vector<int> productIds,
		Utf8StringCR deviceId,
		Utf8StringCR correlationId
		) = 0;
    
	virtual folly::Future<folly::Unit> PostUserUsage
		(
		Utf8StringCR accessToken,
		BeVersionCR version,
		Utf8StringCR projectId,
		AuthType authType,
		int productId,
		Utf8StringCR deviceId,
		UsageType usageType,
		Utf8StringCR correlationId,
		Utf8StringCR principalId
		) = 0;
    
	virtual folly::Future<folly::Unit> PostFeatureUsage
		(
		Utf8StringCR accessToken,
		FeatureEvent featureEvent,
		AuthType authType,
		int productId,
		Utf8StringCR deviceId,
		UsageType usageType,
		Utf8StringCR correlationId,
		Utf8StringCR principalId

		) = 0;
	
	virtual folly::Future<EntitlementResult> CheckEntitlement
		(
		Utf8StringCR accessToken,
		BeVersionCR version,
		Utf8StringCR projectId,
		AuthType authType,
		int productId,
		Utf8StringCR deviceId,
		Utf8StringCR correlationId
		) = 0;

    virtual ~ISaasClient() {};
    };

END_BENTLEY_LICENSING_NAMESPACE
