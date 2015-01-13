/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Response/WSCreateObjectResponse.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <WebServices/ObjectId.h>
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
        WS_EXPORT WSCreateObjectResponse ();
        WS_EXPORT WSCreateObjectResponse (JsonValueCR createdObject);

        WS_EXPORT JsonValueCR GetObject () const;
    };

typedef WSCreateObjectResponse& WSCreateObjectResponseR;
typedef const WSCreateObjectResponse& WSCreateObjectResponseCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
