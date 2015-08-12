/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/Response/WSCreateObjectResponse.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/ObjectId.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSCreateObjectResponse
    {
    private:
        Json::Value m_createdObject;

    public:
        WSCLIENT_EXPORT WSCreateObjectResponse();
        WSCLIENT_EXPORT WSCreateObjectResponse(JsonValueCR createdObject);

        WSCLIENT_EXPORT JsonValueCR GetObject() const;
    };

typedef WSCreateObjectResponse& WSCreateObjectResponseR;
typedef const WSCreateObjectResponse& WSCreateObjectResponseCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
