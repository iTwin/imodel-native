/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Providers/PolicyProvider.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Licensing/Licensing.h>
#include <Licensing/LicenseStatus.h>

#include "IBuddiProvider.h"
#include "IPolicyProvider.h"
#include "../Policy.h"

#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/IConnectAuthenticationProvider.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

struct PolicyProvider : IPolicyProvider
    {
protected:
    ClientInfoPtr m_clientInfo;
    IBuddiProviderPtr m_buddiProvider;
    std::shared_ptr<IConnectAuthenticationProvider> m_authProvider;
    IHttpHandlerPtr m_httpHandler;

    folly::Future<Utf8String> GetCertificate();
    folly::Future<Utf8String> PerformGetPolicyRequest();
public:
    PolicyProvider
        (
        IBuddiProviderPtr buddiProvider,
        ClientInfoPtr clientInfo,
        std::shared_ptr<IConnectAuthenticationProvider> authProvider,
        IHttpHandlerPtr httpHandler
        );
    folly::Future<std::shared_ptr<Policy>> GetPolicy();
    };

END_BENTLEY_LICENSING_NAMESPACE
