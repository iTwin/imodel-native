/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/Client.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Client.h>
#include "ClientImpl.h"

USING_NAMESPACE_BENTLEY_LICENSING

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Client::Client
(
const ConnectSignInManager::UserInfo& userInfo,
ClientInfoPtr clientInfo,
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
BeFileNameCR dbPath,
IHttpHandlerPtr httpHandler
)
    {
    m_impl = std::make_unique<ClientImpl>(userInfo, clientInfo, authenticationProvider, dbPath, httpHandler);
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
IHttpHandlerPtr httpHandler
)
    {
    return std::shared_ptr<Client>(new Client(userInfo, clientInfo, authenticationProvider, dbPath, httpHandler));
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
void Client::SetProjectId(Utf8String projectId)
    {
    m_impl->SetProjectId(projectId);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Client::SetFeatureString(Utf8String featureString)
    {
    m_impl->SetFeatureString(featureString);
    }
