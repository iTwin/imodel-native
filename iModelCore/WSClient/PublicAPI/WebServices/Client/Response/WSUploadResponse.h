/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/Response/WSUploadResponse.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/Response/WSResponse.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSUploadResponse : public WSResponse
    {
    private:
        HttpBodyPtr m_responseBody;

    public:
        WSUploadResponse() {};
        WSUploadResponse(HttpBodyPtr responseJsonBody, Utf8String uploadedFileETag = nullptr) :
            m_responseBody(responseJsonBody),
            WSResponse(true, uploadedFileETag)
            {};

        //! Get server upload response
        HttpBodyPtr GetBody() const
            {
            return m_responseBody;
            };

        //! Get server upload response JSON
        BentleyStatus GetJson(JsonValueR jsonOut) const
            {
            if (m_responseBody.IsNull())
                return ERROR;
            if (!Json::Reader::Parse(m_responseBody->AsString(), jsonOut))
                return ERROR;
            return SUCCESS;
            };

        //! Get server upload response JSON
        BentleyStatus GetJson(rapidjson::Document& jsonOut) const
            {
            if (m_responseBody.IsNull())
                return ERROR;
            if (jsonOut.Parse<0>(m_responseBody->AsString().c_str()).HasParseError())
                return ERROR;
            return SUCCESS;
            };

        // ! Get new ETag for uploaded file if any. Could return empty if no file or backend does not support renewing ETag
        Utf8String GetFileETag() const { return GetETag(); };
    };

typedef WSUploadResponse& WSUploadResponseR;
typedef const WSUploadResponse& WSUploadResponseCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
