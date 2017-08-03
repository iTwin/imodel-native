/*--------------------------------------------------------------------------------------+
|
|     $Source: LicensingCrossPlatform/Licensing/Client.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
BeFileNameCR dbPath, 
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
ClientInfoPtr clientInfo,
const ConnectSignInManager::UserInfo& userInfo,
IHttpHandlerPtr httpHandler,
uint64_t heartbeatInterval
)
    {
    m_impl = std::make_unique<ClientImpl>(dbPath, authenticationProvider, clientInfo, userInfo, httpHandler, heartbeatInterval);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClientPtr Client::Create
(
BeFileNameCR dbPath, 
std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
ClientInfoPtr clientInfo,
const ConnectSignInManager::UserInfo& userInfo,
IHttpHandlerPtr httpHandler,
uint64_t heartbeatInterval
)
    {
    return std::shared_ptr<Client>(new Client(dbPath, authenticationProvider, clientInfo, userInfo, httpHandler, heartbeatInterval));
    }
   
/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Client::StartApplication()
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
