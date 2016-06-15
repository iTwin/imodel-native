/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientConfiguration.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/WSRepositoryClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClientConfiguration
    {
    private:
        const Utf8String m_serverUrl;
        const Utf8String m_repositoryId;
        const IWSSchemaProviderPtr m_schemaProvider;
        std::shared_ptr<HttpClient> m_httpClient;

    public:
        ClientConfiguration
            (
            Utf8StringCR serverUrl,
            Utf8StringCR repositoryId,
            IHttpHeaderProviderPtr defaultHeadersProvider,
            IWSSchemaProviderPtr schemaProvider,
            IHttpHandlerPtr customHandler
            );

        Utf8StringCR GetServerUrl() const;
        Utf8StringCR GetRepositoryId() const;
        HttpClientR GetHttpClient() const;
        BeFileName GetDefaultSchemaPath(WSInfoCR info) const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
