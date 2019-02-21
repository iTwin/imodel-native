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
//#include "../Policy.h"

#include <WebServices/Client/ClientInfo.h>
#include "IAuthHandlerProvider.h"

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

struct PolicyProvider : IPolicyProvider
    {
protected:
    ClientInfoPtr m_clientInfo;
    IBuddiProviderPtr m_buddiProvider;
    std::shared_ptr<IAuthHandlerProvider> m_authHandlerProvider;

public:
    LICENSING_EXPORT PolicyProvider
        (
        IBuddiProviderPtr buddiProvider,
        ClientInfoPtr clientInfo,
        std::shared_ptr<IAuthHandlerProvider> authHandlerProvider
        );
    LICENSING_EXPORT folly::Future<std::shared_ptr<Policy>> GetPolicy();
    LICENSING_EXPORT folly::Future<Utf8String> GetCertificate();
    LICENSING_EXPORT folly::Future<Utf8String> PerformGetPolicyRequest();
    };

END_BENTLEY_LICENSING_NAMESPACE
