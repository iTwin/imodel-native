/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Licensing/Client.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include "LicenseStatus.h"

#include <WebServices/Connect/IConnectAuthenticationProvider.h>
#include <WebServices/Client/ClientInfo.h>
#include <WebServices/Connect/ConnectSignInManager.h> // Would be nice to remove this dependency

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

    Client
        (
		const ConnectSignInManager::UserInfo& userInfo,
        ClientInfoPtr clientInfo,
        std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
        BeFileNameCR dbPath,
        bool offlineMode,
        Utf8String projectId,
        Utf8String featureString,
        IHttpHandlerPtr httpHandler
        );

public:
    LICENSING_EXPORT static ClientPtr Create
        (
		const ConnectSignInManager::UserInfo& userInfo,
        ClientInfoPtr clientInfo,
        std::shared_ptr<IConnectAuthenticationProvider> authenticationProvider,
        BeFileNameCR dbPath,
        bool offlineMode,
        Utf8String projectId = "",
        Utf8String featureString = "",
        IHttpHandlerPtr customHttpHandler = nullptr
        );

    // TODO: Return more than BentleyStatus to indicate to the app if the user has rights to use this app or it's crippled etc...
    LICENSING_EXPORT LicenseStatus StartApplication(); 
    LICENSING_EXPORT BentleyStatus StopApplication();
    LICENSING_EXPORT BentleyStatus StartFeature();
    LICENSING_EXPORT BentleyStatus StopFeature();
};

END_BENTLEY_LICENSING_NAMESPACE
