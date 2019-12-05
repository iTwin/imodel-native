/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include "../../../../Licensing/Providers/IEntitlementProvider.h"
#include "../../../../Licensing/Providers/WebEntitlementResult.h"

#include "../TestsHelper.h"

BEGIN_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE

struct EntitlementProviderMock : IEntitlementProvider
{
private:
	int m_productId = -1;
	LicenseStatus m_status = LicenseStatus::AccessDenied;
	Utf8String m_principalId;

	int m_fetchWebEntitlementV4Calls = 0;
public:
	virtual folly::Future<WebEntitlementResult> FetchWebEntitlementV4(const std::vector<int>& productIds, BeVersionCR version, Utf8StringCR deviceId, Utf8StringCR projectId, Utf8StringCR accessToken) override;

	void MockV4Result(WebEntitlementResult mocked)
		{
		m_productId = mocked.ProductId;
		m_status = mocked.Status;
		m_principalId = mocked.PrincipalId;
		}

	int FetchWebEntitlementV4Calls() const { return m_fetchWebEntitlementV4Calls; }
};

END_BENTLEY_LICENSING_UNIT_TESTS_NAMESPACE