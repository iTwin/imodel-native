/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ConnectTokenProvider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectTokenProvider : public IConnectTokenProvider
    {
    public:
        std::shared_ptr<IConnectAuthenticationPersistence> m_persistence;
        bool m_isTokenBasedAuthentication;

    private:
        bool ShouldRenewToken(DateTime tokenSetTime, int64_t renewTokenAfter);

    public:
        WSCLIENT_EXPORT ConnectTokenProvider
            (
            std::shared_ptr<IConnectAuthenticationPersistence> customPersistence = nullptr,
            bool isTokenBasedAuthentication = false
            );

        WSCLIENT_EXPORT SamlTokenPtr UpdateToken() override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken() override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
