/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/Passport.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <BeHttp/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct Passport
    {
    enum PassportStatus
        {
        HAS_PASSPORT = 0,
        NO_PASSPORT,
        PASSPORT_ERROR
        };

    private:
        Passport() {}
        static Utf8String GetServiceUrl();

    public:
        WSCLIENT_EXPORT static StatusInt HasUserPassport(Utf8StringCR userGuid);
        WSCLIENT_EXPORT static void Initialize(std::shared_ptr<IHttpHandler> customHttpHandler = nullptr);
        WSCLIENT_EXPORT static void Uninintialize();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
