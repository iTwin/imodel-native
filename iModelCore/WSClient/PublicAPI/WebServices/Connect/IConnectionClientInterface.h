/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/IConnectionClientInterface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/WebServices.h>
#include <WebServices/Connect/SamlToken.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Mark.Uvari    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/

typedef void(*event_callback)(int eventId, WCharCP data);

typedef std::shared_ptr<struct IConnectionClientInterface> IConnectionClientInterfacePtr;
struct IConnectionClientInterface 
    {
    public:
        enum EVENT_TYPE
            {
            LOGIN = 1,
            LOGOUT = 2,
            STARTUP = 3,
            SHUTDOWN = 4
            };

        virtual ~IConnectionClientInterface() {}

        virtual bool IsInstalled() { return false; }
        virtual SamlTokenPtr GetSerializedDelegateSecurityToken(Utf8StringCR rpUri = nullptr, Utf8StringP errorString = nullptr) { return nullptr; }
        virtual bool IsRunning() { return false; }
        virtual BentleyStatus StartClientApp() { return BentleyStatus::ERROR; }
        virtual bool IsLoggedIn() { return false; }
        virtual Utf8String GetUserId() { return ""; }
        virtual void AddClientEventListener(event_callback callback) {}
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
