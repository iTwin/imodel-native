/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/Passport.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <MobileDgn/Utils/Http/HttpClient.h>
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

public:
    WSCLIENT_EXPORT static StatusInt HasUserPassport(Utf8StringCR userGuid);
    WSCLIENT_EXPORT static Utf8String GetPassportUrl();
    WSCLIENT_EXPORT static void Initialize(std::shared_ptr<MobileDgn::Utils::IHttpHandler> customHttpHandler = nullptr);
    WSCLIENT_EXPORT static void Uninintialize();
};

END_BENTLEY_WEBSERVICES_NAMESPACE
