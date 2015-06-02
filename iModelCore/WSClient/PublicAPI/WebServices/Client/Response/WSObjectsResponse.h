/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Client/Response/WSObjectsResponse.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Client/WebServicesClient.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <WebServices/Client/Response/WSObjectsReader.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsResponse
    {
    private:
        HttpBodyPtr m_httpBody;
        bool        m_isModified;
        Utf8String  m_eTag;

        mutable std::shared_ptr<Json::Value>         m_jsonValue;
        mutable std::shared_ptr<rapidjson::Document> m_rapidJsonDocument;

        std::shared_ptr<WSObjectsReader> m_reader;
        mutable std::shared_ptr<WSObjectsReader::Instances> m_readerInstances;

    private:
        JsonValueR GetJsonValuePrivate () const;
        std::shared_ptr<rapidjson::Document> GetRapidJsonDocumentPrivate () const;

    public:
        WSCLIENT_EXPORT WSObjectsResponse ();
        WSCLIENT_EXPORT WSObjectsResponse (std::shared_ptr<WSObjectsReader> reader, HttpBodyPtr httpBody, HttpStatus status, Utf8String eTag);

        WSCLIENT_EXPORT Utf8StringCR GetETag () const;
        WSCLIENT_EXPORT bool IsModified () const;

        //! Lightweight wrapper to read object instances returned by server (WebApi version independant)
        WSCLIENT_EXPORT const WSObjectsReader::Instances& GetInstances () const;

        //! Deprecated, use GetInstances. Get WebApi version specific server response 
        WSCLIENT_EXPORT JsonValueCR GetJsonValue () const;
        WSCLIENT_EXPORT JsonValueR GetJsonValue ();

        //! Deprecated, use GetInstances. Get WebApi version specific server response 
        WSCLIENT_EXPORT const rapidjson::Document& GetRapidJsonDocument () const;
        WSCLIENT_EXPORT rapidjson::Document& GetRapidJsonDocument ();
    };

typedef const WSObjectsResponse& WSObjectsResponseCR;
typedef WSObjectsResponse& WSObjectsResponseR;

END_BENTLEY_WEBSERVICES_NAMESPACE
