/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "IEntitlementProvider.h"
#include "IBuddiProvider.h"
#include "IUlasProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct EntitlementProvider : IEntitlementProvider
    {
    protected:
        IBuddiProviderPtr m_buddiProvider;
        Http::IHttpHandlerPtr m_httpHandler;

    public:
        LICENSING_EXPORT EntitlementProvider
        (
            IBuddiProviderPtr     buddiProvider,
            Http::IHttpHandlerPtr httpHandler
        );

        LICENSING_EXPORT folly::Future<WebEntitlementResult> FetchWebEntitlementV4(const std::vector<int>& productIds, BeVersionCR version, Utf8StringCR deviceId, Utf8StringCR projectId, Utf8StringCR accessToken, AuthType authType);
    };

END_BENTLEY_LICENSING_NAMESPACE