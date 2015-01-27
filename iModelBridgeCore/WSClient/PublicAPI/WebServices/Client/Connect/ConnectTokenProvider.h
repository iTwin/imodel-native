/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/Connect/ConnectTokenProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Client/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectTokenProvider : public IConnectTokenProvider
    {
    public:
        std::shared_ptr<IConnectAuthenticationPersistence> m_persistence;

    public:
        WSCLIENT_EXPORT ConnectTokenProvider (std::shared_ptr<IConnectAuthenticationPersistence> customPersistence = nullptr);

        WSCLIENT_EXPORT SamlTokenPtr UpdateToken () override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken () override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
