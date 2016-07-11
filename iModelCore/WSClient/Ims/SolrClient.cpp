/*--------------------------------------------------------------------------------------+
|
|     $Source: Ims/SolrClient.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ClientInternal.h"
#include <WebServices/IMS/SolrClient.h>

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ISolrClient::~ISolrClient()
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
SolrClient::SolrClient(Utf8String serverUrl, Utf8String collectionPath, IHttpHeaderProviderPtr defaultHeadersProvider, IHttpHandlerPtr customHandler)
    : m_serverUrl(serverUrl),
    m_httpClient(std::make_shared<HttpClient>(defaultHeadersProvider, customHandler))
    {
    while (collectionPath.StartsWith(R"(\)") || collectionPath.StartsWith("/")
           || collectionPath.EndsWith(R"(\)") || collectionPath.EndsWith("/"))
           collectionPath.Trim(R"(\/)");
    m_collectionPath = collectionPath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<SolrClient> SolrClient::Create
(
Utf8StringCR serverUrl,
Utf8String collectionPath,
ClientInfoPtr clientInfo,
IHttpHandlerPtr customHandler
)
    {
    BeAssert(nullptr != clientInfo);
    return std::shared_ptr<SolrClient>(new SolrClient(serverUrl, collectionPath, clientInfo, customHandler));
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrClient::GetBaseUrl() const
    {
    return m_serverUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR SolrClient::GetCollectionPath() const
    {
    return m_collectionPath;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    David.Jones     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<SolrGetResult> SolrClient::SendGetRequest(SolrQueryCR query) const
    {
    Utf8String url(m_serverUrl + (m_serverUrl.EndsWith("/") ? "" : "/") + m_collectionPath + "/select" + query.ToString());
    BeAssert(url.size() < 2000 && "<Warning> Url length might be problematic as it is longer than most default settings");

    Http::Request request = m_httpClient->CreateGetJsonRequest(url);

    return request.PerformAsync()->Then<SolrGetResult>([this] (Http::Response& httpResponse)
        {
        HttpStatus status = httpResponse.GetHttpStatus();
        if (HttpStatus::OK == status ||
            HttpStatus::NotModified == status)
            {
            return SolrGetResult::Success(httpResponse.GetBody().AsJson());
            }
        return SolrGetResult::Error(httpResponse);
        });
    }