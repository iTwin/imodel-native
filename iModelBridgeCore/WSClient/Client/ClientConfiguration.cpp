/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientConfiguration.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
HttpRequestHeadersCR defaultHeaders,
BeFileNameCP defaultSchemaPath,
IHttpHandlerPtr customHandler
) :
m_serverUrl (serverUrl),
m_repositoryId (repositoryId),
m_defaultSchemaPath (nullptr == defaultSchemaPath ? L"" : *defaultSchemaPath),
m_httpClient (std::make_shared<HttpClient> (customHandler))
    {
    BeAssert (nullptr == defaultSchemaPath || !defaultSchemaPath->empty ());
    m_httpClient->DefaultRequestHeaders () = defaultHeaders;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClientConfiguration::GetServerUrl () const
    {
    return m_serverUrl;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ClientConfiguration::GetRepositoryId () const
    {
    return m_repositoryId;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HttpClientR ClientConfiguration::GetHttpClient () const
    {
    return *m_httpClient;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileNameCR ClientConfiguration::GetDefaultSchemaPath () const
    {
    return m_defaultSchemaPath;
    }