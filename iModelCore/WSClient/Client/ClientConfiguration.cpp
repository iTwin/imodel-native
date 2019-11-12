/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ClientInternal.h"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ClientConfiguration::ClientConfiguration
(
Utf8StringCR serverUrl,
Utf8StringCR repositoryId,
IHttpHeaderProviderPtr defaultHeadersProvider,
IWSSchemaProviderPtr schemaProvider,
IHttpHandlerPtr customHandler
) :
m_serverUrl(serverUrl),
m_repositoryId(repositoryId),
m_headerProvider(std::make_shared<HeaderProvider>(defaultHeadersProvider)),
m_httpClient(std::make_shared<HttpClient>(m_headerProvider, customHandler)),
m_schemaProvider(schemaProvider),
m_httpHandler(customHandler),
m_activityIdGenerator(std::make_shared<ActivityIdGenerator>())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClientConfiguration::GetServerUrl() const
    {
    return m_serverUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClientConfiguration::GetRepositoryId() const
    {
    return m_repositoryId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpClientR ClientConfiguration::GetHttpClient() const
    {
    return *m_httpClient;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IHttpHandlerPtr ClientConfiguration::GetHttpHandler() const
    {
    return m_httpHandler;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName ClientConfiguration::GetDefaultSchemaPath(WSInfoCR info) const
    {
    if (nullptr == m_schemaProvider)
        {
        return BeFileName();
        }
    return m_schemaProvider->GetSchema(info);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Mantas.Smicius    10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IActivityIdGeneratorCR ClientConfiguration::GetActivityIdGenerator() const
    {
    return *m_activityIdGenerator;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
HttpRequestHeadersR ClientConfiguration::GetDefaultHeaders()
    {
    return m_headerProvider->GetDefaultHeaders();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClientConfiguration::HeaderProvider::FillHttpRequestHeaders(HttpRequestHeaders& headersOut) const
    {
    headersOut.GetMap().insert(m_defaultHeaders.GetMap().begin(), m_defaultHeaders.GetMap().end());
    m_customProvider->FillHttpRequestHeaders(headersOut);
    }