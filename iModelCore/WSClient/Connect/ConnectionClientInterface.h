/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/WebServices.h>
#include <WebServices/Connect/IConnectionClientInterface.h>

#include <CCApi/CCPublic.h>
#include <Bentley/Base64Utilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mark.Uvari    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ConnectionClientInterface : public IConnectionClientInterface
    {
    private:
        CCAPIHANDLE m_ccApi;
        static bset<event_callback> s_listeners;
        static BeMutex s_mutex;

        CCDATABUFHANDLE GetUserInformation();
        static void EventListener(int eventId, WCharCP data);
        static void CCApiStatusToString(int status, Utf8StringP errorStringOut);

    public:
        ConnectionClientInterface();
        ~ConnectionClientInterface() override;

        virtual bool IsInstalled() override;
        virtual SamlTokenPtr GetSerializedDelegateSecurityToken(Utf8StringCR rpUri = nullptr, Utf8StringP errorStringOut = nullptr) override;

        virtual bool IsRunning() override;
        virtual BentleyStatus StartClientApp() override;
        virtual bool IsLoggedIn() override;
        virtual Utf8String GetUserId() override;
        virtual void AddClientEventListener(event_callback callback) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
