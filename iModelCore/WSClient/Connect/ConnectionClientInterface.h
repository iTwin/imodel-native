/*--------------------------------------------------------------------------------------+
|
|     $Source: Connect/ConnectionClientInterface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <WebServices/WebServices.h>
#include <WebServices/Connect/IConnectionClientInterface.h>

#include <BentleyDesktopClient/CCApi/CCPublic.h>
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

    public:
        ConnectionClientInterface();
        ~ConnectionClientInterface() override;

        virtual bool IsInstalled() override;
        virtual SamlTokenPtr GetSerializedDelegateSecurityToken(Utf8StringCR rpUri = nullptr) override;

        virtual bool IsRunning() override;
        virtual BentleyStatus StartClientApp() override;
        virtual bool IsLoggedIn() override;
        virtual Utf8String GetUserId() override;
        virtual void AddClientEventListener(event_callback callback) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
