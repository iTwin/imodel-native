/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Client.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Client.h>
#include "ClientImpl.h"
#include "ClientFreeImpl.h"
#include "ClientWithKeyImpl.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Client::Client
(
	std::shared_ptr<struct ClientInterface> implementation
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
ApplicationInfoPtr applicationInfo,
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
BeFileNameCR dbPath,
bool offlineMode,
Utf8StringCR projectId,
Utf8StringCR featureString,
IHttpHandlerPtr httpHandler
)
    {
	return std::shared_ptr<Client>(new Client(std::make_shared<ClientImpl>(userInfo, applicationInfo, authenticationProvider, dbPath, offlineMode, projectId, featureString, httpHandler)));
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::CreateFree
(
	Utf8StringCR featureString,
	IHttpHandlerPtr httpHandler
)
	{
	return std::shared_ptr<Client>(new Client(std::make_shared<ClientFreeImpl>(featureString, httpHandler)));
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::CreateWithKey
(
	Utf8StringCR accessKey,
	ApplicationInfoPtr applicationInfo,
	BeFileNameCR dbPath,
	bool offlineMode,
	Utf8StringCR projectId,
	Utf8StringCR featureString,
	IHttpHandlerPtr httpHandler
)
{
	return std::shared_ptr<Client>(new Client(std::make_shared<ClientWithKeyImpl>(accessKey, applicationInfo, dbPath, offlineMode, projectId, featureString, httpHandler)));
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

 /*--------------------------------------------------------------------------------------+
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
 folly::Future<BentleyStatus> Client::TrackUsage(Utf8StringCR accessToken, BeVersionCR version, Utf8StringCR projectId)
	 {
	 return m_impl->TrackUsage(accessToken, version, projectId);
	 }
