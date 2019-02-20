/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Client.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Client.h>
#include "Providers/BuddiProvider.h"
#include "Providers/PolicyProvider.h"
#include "Providers/UlasProvider.h"
#include "ClientImpl.h"
#include "ClientWithKeyImpl.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Client::Client
(
    std::shared_ptr<struct IClient> implementation
)
{
	m_impl = implementation;
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::Create
(
const ConnectSignInManager::UserInfo& userInfo,
ClientInfoPtr clientInfo,
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
BeFileNameCR dbPath,
bool offlineMode,
Utf8StringCR projectId,
Utf8StringCR featureString,
IHttpHandlerPtr httpHandler
)
    {
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IPolicyProviderPtr policyProvider = std::make_shared<PolicyProvider>(buddiProvider, clientInfo, authenticationProvider, httpHandler);
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, clientInfo, dbPath, httpHandler);
    return std::shared_ptr<Client>(new Client(std::make_shared<ClientImpl>(userInfo, clientInfo, authenticationProvider, dbPath, offlineMode, buddiProvider, policyProvider, ulasProvider, projectId, featureString, httpHandler)));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::CreateWithKey
(
	Utf8StringCR accessKey,
	ClientInfoPtr clientInfo,
	BeFileNameCR dbPath,
	bool offlineMode,
	Utf8StringCR projectId,
	Utf8StringCR featureString,
	IHttpHandlerPtr httpHandler
)
{
    IBuddiProviderPtr buddiProvider = std::make_shared<BuddiProvider>();
    IUlasProviderPtr ulasProvider = std::make_shared<UlasProvider>(buddiProvider, clientInfo, dbPath, httpHandler);
    return std::shared_ptr<Client>(new Client(std::make_shared<ClientWithKeyImpl>(accessKey, clientInfo, dbPath, offlineMode, buddiProvider, ulasProvider, projectId, featureString, httpHandler)));
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
 BentleyStatus Client::MarkFeature(Utf8StringCR featureId, FeatureUserDataMap* featureUserData)
     {
     return m_impl->MarkFeature(featureId, featureUserData);
     }
