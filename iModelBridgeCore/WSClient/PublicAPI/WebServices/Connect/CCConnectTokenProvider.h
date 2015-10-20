/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/CCConnectTokenProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Connect/IConnectAuthenticationPersistence.h>
#include <WebServices/Connect/IConnectTokenProvider.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Jahan.Zeb    02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct CCConnectTokenProvider : public IConnectTokenProvider
    {
    private:        
        mutable SamlTokenPtr m_token;
//        bool m_isValid = false;
//        wchar_t* m_relyingPartyAddress;

        SamlTokenPtr GetTokenFromConnectClient ();
//         wchar_t* m_buddiName;        

    public:
        std::shared_ptr<IConnectAuthenticationPersistence> m_persistence;

    public:
        // WSCLIENT_EXPORT CCConnectTokenProvider ();
        WSCLIENT_EXPORT CCConnectTokenProvider (std::shared_ptr<IConnectAuthenticationPersistence> customPersistence = nullptr);

        WSCLIENT_EXPORT SamlTokenPtr UpdateToken () override;
        WSCLIENT_EXPORT SamlTokenPtr GetToken () override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE