/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Client.h>
#include "Providers/AuthHandlerProvider.h"
#include "Providers/BuddiProvider.h"
#include "Providers/PolicyProvider.h"
#include "Providers/UlasProvider.h"
#include "ClientImpl.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Client::Client
    (
    std::shared_ptr<struct ClientImpl> implementation
    )
    {
    m_impl = implementation;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::Create
    (
    const WebServices::ConnectSignInManager::UserInfo& userInfo,
    WebServices::ClientInfoPtr clientInfo,
    std::shared_ptr<WebServices::IConnectAuthenticationProvider> authenticationProvider,
    BeFileNameCR dbPath,
    bool offlineMode,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    Http::IHttpHandlerPtr httpHandler,
    AuthType authType
    )
    {
    ApplicationInfoPtr applicationInfo = std::make_shared<ApplicationInfo>(clientInfo->GetApplicationVersion(), clientInfo->GetDeviceId(), clientInfo->GetApplicationProductId());
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IAuthHandlerProviderPtr authHandlerProvider = std::make_shared<AuthHandlerProvider>(authenticationProvider, httpHandler);
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, httpHandler, authType, authHandlerProvider);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<Client>(new Client(std::make_shared<ClientImpl>(userInfo, applicationInfo, dbPath, offlineMode, policyProvider, ulasProvider, projectId, featureString, nullptr)));
    }

ClientPtr Client::Create
    (
    const WebServices::ConnectSignInManager::UserInfo& userInfo,
    ApplicationInfoPtr applicationInfo,
    std::shared_ptr<WebServices::IConnectAuthenticationProvider> authenticationProvider,
    BeFileNameCR dbPath,
    bool offlineMode,
    Utf8StringCR projectId,
    Utf8StringCR featureString,
    Http::IHttpHandlerPtr httpHandler,
    AuthType authType
    )
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IAuthHandlerProviderPtr authHandlerProvider = std::make_shared<AuthHandlerProvider>(authenticationProvider, httpHandler);
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, applicationInfo, httpHandler, authType, authHandlerProvider);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, httpHandler);
    return std::shared_ptr<Client>(new Client(std::make_shared<ClientImpl>(userInfo, applicationInfo, dbPath, offlineMode, policyProvider, ulasProvider, projectId, featureString, nullptr)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus Client::StartApplication()
    {
    return m_impl->StartApplication();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Client::StopApplication()
    {
    return m_impl->StopApplication();
    }

 /*--------------------------------------------------------------------------------------+
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
 BentleyStatus Client::MarkFeature(Utf8StringCR featureId, FeatureUserDataMapPtr featureUserData)
     {
     return m_impl->MarkFeature(featureId, featureUserData);
     }
