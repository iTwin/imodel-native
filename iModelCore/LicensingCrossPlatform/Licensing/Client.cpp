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

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Client::Client
(
	Utf8String username,
	ClientInfoPtr clientInfo,
	BeFileNameCR dbPath,
	bool offlineMode,
	Utf8String projectId,
	Utf8String featureString,
	IHttpHandlerPtr httpHandler
)
{
	m_impl = std::make_unique<ClientFreeImpl>(username, clientInfo, dbPath, offlineMode, projectId, featureString, httpHandler);
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Client::Client
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
    m_impl = std::make_unique<ClientImpl>(userInfo, clientInfo, authenticationProvider, dbPath, offlineMode, projectId, featureString, httpHandler);
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
    return std::shared_ptr<Client>(new Client(userInfo, clientInfo, authenticationProvider, dbPath, offlineMode, projectId, featureString, httpHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::CreateFree
(
	Utf8String username,
	ClientInfoPtr clientInfo,
	BeFileNameCR dbPath,
	bool offlineMode,
	Utf8String projectId,
	Utf8String featureString,
	IHttpHandlerPtr customHttpHandler
)
	{
	return std::shared_ptr<Client>(new Client(username, clientInfo, dbPath, offlineMode, projectId, featureString, customHttpHandler));
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
