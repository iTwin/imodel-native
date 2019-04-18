/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>
#include <Licensing/AuthType.h>

#include "IBuddiProvider.h"
#include "IPolicyProvider.h"
//#include "../Policy.h"

#include <WebServices/Client/ClientInfo.h>
#include "IAuthHandlerProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE

struct PolicyProvider : IPolicyProvider
    {
protected:
	WebServices::ClientInfoPtr m_clientInfo;
    IBuddiProviderPtr m_buddiProvider;
    Http::IHttpHandlerPtr m_httpHandler;
    std::shared_ptr<IAuthHandlerProvider> m_authHandlerProvider;
	WebServices::IConnectAuthenticationProvider::HeaderPrefix m_headerPrefix;

public:
    LICENSING_EXPORT PolicyProvider
        (
        IBuddiProviderPtr buddiProvider,
		WebServices::ClientInfoPtr clientInfo,
        Http::IHttpHandlerPtr httpHandler,
        AuthType authType,
        std::shared_ptr<IAuthHandlerProvider> authHandlerProvider = nullptr // allow creation of non-authorized PolicyProvider
        );
    LICENSING_EXPORT folly::Future<std::shared_ptr<Policy>> GetPolicy();
    LICENSING_EXPORT folly::Future<std::shared_ptr<Policy>> GetPolicyWithKey(Utf8StringCR accessKey);
    LICENSING_EXPORT folly::Future<Utf8String> GetCertificate();
    LICENSING_EXPORT folly::Future<Utf8String> PerformGetPolicyRequest();
    LICENSING_EXPORT folly::Future<Utf8String> PerformGetPolicyWithKeyRequest(Utf8StringCR accessKey);
private:
	WebServices::IConnectAuthenticationProvider::HeaderPrefix GetHeaderPrefix(AuthType authType);
    };

END_BENTLEY_LICENSING_NAMESPACE
