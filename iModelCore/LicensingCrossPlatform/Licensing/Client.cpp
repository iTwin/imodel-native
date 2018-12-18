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
ClientInfoPtr clientInfo,
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
BeFileNameCR dbPath,
bool offlineMode,
Utf8String projectId,
Utf8String featureString,
IHttpHandlerPtr httpHandler
)
    {
	return std::shared_ptr<Client>(new Client(std::make_shared<ClientImpl>(userInfo, clientInfo, authenticationProvider, dbPath, offlineMode, projectId, featureString, httpHandler)));
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::CreateFree
(
	Utf8String accessToken,
	BeVersion clientVersion,
	BeFileNameCR dbPath,
	bool offlineMode,
	Utf8String projectId,
	Utf8String featureString,
	IHttpHandlerPtr httpHandler
)
	{
	return std::shared_ptr<Client>(new Client(std::make_shared<ClientFreeImpl>(accessToken, clientVersion, dbPath, offlineMode, projectId, featureString, httpHandler)));
	}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::CreateWithKey
(
	Utf8String accessKey,
	ClientInfoPtr clientInfo,
	BeFileNameCR dbPath,
	bool offlineMode,
	Utf8String projectId,
	Utf8String featureString,
	IHttpHandlerPtr httpHandler
)
{
	return std::shared_ptr<Client>(new Client(std::make_shared<ClientWithKeyImpl>(accessKey, clientInfo, dbPath, offlineMode, projectId, featureString, httpHandler)));
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
 BentleyStatus Client::MarkFeature(Utf8String featureId, FeatureUserDataMap* featureUserData)
     {
     return m_impl->MarkFeature(featureId, featureUserData);
     }
