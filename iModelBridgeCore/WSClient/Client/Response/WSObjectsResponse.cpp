/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/Response/WSObjectsResponse.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Client/Response/WSObjectsResponse.h>
#include <WebServices/Client/Response/WSObjectsReaderV2.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsResponse::WSObjectsResponse() :
m_httpBody(HttpStringBody::Create()),
m_isModified(true),
m_eTag(),
m_reader(WSObjectsReaderV2::Create())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WSObjectsResponse::WSObjectsResponse
(
std::shared_ptr<WSObjectsReader> reader,
HttpBodyPtr httpBody,
HttpStatus status,
Utf8String eTag,
Utf8String skipToken
) :
m_httpBody(httpBody),
m_isModified(HttpStatus::OK == status),
m_eTag(eTag),
m_skipToken(skipToken),
m_reader(reader)
    {
    BeAssert(m_httpBody != nullptr);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSObjectsResponse::GetETag() const
    {
    return m_eTag;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR WSObjectsResponse::GetSkipToken() const
    {
    return m_skipToken;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsResponse::IsFinal() const
    {
    return m_skipToken.empty();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool WSObjectsResponse::IsModified() const
    {
    return m_isModified;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueR WSObjectsResponse::GetJsonValuePrivate() const
    {
    if (!m_jsonValue)
        {
        m_jsonValue = std::make_shared<Json::Value>(m_httpBody->AsJson());
        }
    return *m_jsonValue;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<rapidjson::Document> WSObjectsResponse::GetRapidJsonDocumentPrivate() const
    {
    if (!m_rapidJsonDocument)
        {
        m_rapidJsonDocument = std::make_shared<rapidjson::Document>();
        m_httpBody->AsRapidJson(*m_rapidJsonDocument);
        }
    return m_rapidJsonDocument;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const WSObjectsReader::Instances& WSObjectsResponse::GetInstances() const
    {
    if (!m_readerInstances)
        {
        std::shared_ptr<rapidjson::Document> json = GetRapidJsonDocumentPrivate();
        m_readerInstances = std::make_shared<WSObjectsReader::Instances>(m_reader->ReadInstances(json));
        }
    return *m_readerInstances;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueCR WSObjectsResponse::GetJsonValue() const
    {
    return GetJsonValuePrivate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
JsonValueR WSObjectsResponse::GetJsonValue()
    {
    return GetJsonValuePrivate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
const rapidjson::Document& WSObjectsResponse::GetRapidJsonDocument() const
    {
    return *GetRapidJsonDocumentPrivate();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document& WSObjectsResponse::GetRapidJsonDocument()
    {
    return *GetRapidJsonDocumentPrivate();
    }
