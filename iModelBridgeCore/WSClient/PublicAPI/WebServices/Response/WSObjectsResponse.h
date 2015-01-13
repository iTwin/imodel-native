/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Response/WSObjectsResponse.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Common.h>
#include <MobileDgn/Utils/Http/HttpClient.h>
#include <WebServices/Response/WSObjectsReader.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct WSObjectsResponse
    {
    private:
        MobileDgn::Utils::HttpBodyPtr m_httpBody;
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
        WS_EXPORT WSObjectsResponse ();
        WS_EXPORT WSObjectsResponse (std::shared_ptr<WSObjectsReader> reader, MobileDgn::Utils::HttpBodyPtr httpBody, MobileDgn::Utils::HttpStatus status, Utf8String eTag);

        WS_EXPORT Utf8StringCR GetETag () const;
        WS_EXPORT bool IsModified () const;

        //! Lightweight wrapper to read object instances returned by server (WebApi version independant)
        WS_EXPORT const WSObjectsReader::Instances& GetInstances () const;

        //! Deprecated, use GetInstances. Get WebApi version specific server response 
        WS_EXPORT JsonValueCR GetJsonValue () const;
        WS_EXPORT JsonValueR GetJsonValue ();

        //! Deprecated, use GetInstances. Get WebApi version specific server response 
        WS_EXPORT const rapidjson::Document& GetRapidJsonDocument () const;
        WS_EXPORT rapidjson::Document& GetRapidJsonDocument ();
    };

typedef const WSObjectsResponse& WSObjectsResponseCR;
typedef WSObjectsResponse& WSObjectsResponseR;

END_BENTLEY_WEBSERVICES_NAMESPACE
