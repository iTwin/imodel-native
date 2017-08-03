/*--------------------------------------------------------------------------------------+
 |
 |     $Source: LicensingCrossPlatform/PublicAPI/Licensing/Client.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

#define DEFAULT_HEARTBEAT_INTERVAL_MS 1*60*1000

BEGIN_BENTLEY_LICENSING_NAMESPACE
USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct Client> ClientPtr;
struct Client
{
private:
     std::unique_ptr<struct ClientImpl> m_impl;

private:
    Client
        (
        BeFileNameCR dbPath,
        std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
        ClientInfoPtr clientInfo,
        const ConnectSignInManager::UserInfo& userInfo,
        IHttpHandlerPtr httpHandler,
        uint64_t heartbeatInterval
        );

public:
    LICENSING_EXPORT static ClientPtr Create
        (
        BeFileNameCR dbPath,
        std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
        ClientInfoPtr clientInfo,
        const ConnectSignInManager::UserInfo& userInfo,
        IHttpHandlerPtr customHttpHandler = nullptr,
        uint64_t heartbeatIntervalMs = DEFAULT_HEARTBEAT_INTERVAL_MS
        );

    // TODO: Return more than BentleyStatus to indicate to the app if the user has rights to use this app or it's crippled etc...
    LICENSING_EXPORT BentleyStatus StartApplication(); 
    LICENSING_EXPORT BentleyStatus StopApplication();
};

END_BENTLEY_LICENSING_NAMESPACE
