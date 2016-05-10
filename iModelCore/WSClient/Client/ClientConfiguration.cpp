/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientConfiguration.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
m_schemaProvider(schemaProvider),
m_httpClient(std::make_shared<HttpClient>(defaultHeadersProvider, customHandler)),
m_httpHandler(customHandler)
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
