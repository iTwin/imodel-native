/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Licensing/Client.h>
#include <Licensing/ClientConfig.h>
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

ClientPtr Client::Create
(
    ClientConfig config
)
    {
    //TODO check required params
    return Client::Create(config.GetUserInfo(), config.GetAppInfo(), config.GetAuthProvider(), config.GetDBPath(), config.GetOfflineMode(), config.GetProjectId(), config.GetFeatureString(), config.GetCustomHttpHandler(), config.GetAuthType());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LicenseStatus Client::StartApplication()
    {
    return m_impl->StartApplication();
    }

LicenseStatus Client::StartApplicationForProject(Utf8StringCR projectId)
    {
    return m_impl->StartApplicationForProject(projectId);
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
BentleyStatus Client::MarkFeature(Utf8StringCR featureId, FeatureUserDataMapPtr featureUserData, Utf8StringCR projectId)
    {   
    return m_impl->MarkFeature(featureId, featureUserData, projectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t Client::GetTrialDaysRemaining(Utf8StringCR projectId)
    {    
    return m_impl->GetTrialDaysRemaining(projectId);
    }

int64_t Client::ImportCheckout(BeFileNameCR filepath)
    {
    return m_impl->ImportCheckout(filepath);
    }

BentleyStatus Client::DeleteLocalCheckout(Utf8StringCR productId)
    {
    return m_impl->DeleteLocalCheckout(productId);
    }
