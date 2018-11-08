/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/ClientConfiguration.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/HttpClient.h>
#include <WebServices/Client/WebServicesClient.h>
#include <WebServices/Client/WSRepositoryClient.h>

#include "ActivityIdGenerator.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClientConfiguration 
    {
    private:
        struct HeaderProvider : public IHttpHeaderProvider
            {
            private:
                const IHttpHeaderProviderPtr m_customProvider;
                HttpRequestHeaders m_defaultHeaders;
            public:
                HeaderProvider(IHttpHeaderProviderPtr provider) : m_customProvider(provider) {};
                virtual void FillHttpRequestHeaders(HttpRequestHeaders& headersOut) const;
                HttpRequestHeadersR GetDefaultHeaders() { return m_defaultHeaders; };
            };

    private:
        const Utf8String m_serverUrl;
        const Utf8String m_repositoryId;
        BeVersion m_serviceVersion;

        const std::shared_ptr<HeaderProvider> m_headerProvider;
        const IWSSchemaProviderPtr m_schemaProvider;
        const std::shared_ptr<HttpClient> m_httpClient;
        const IHttpHandlerPtr m_httpHandler;
        const IActivityIdGeneratorPtr m_activityIdGenerator;
        Utf8String m_persistenceProviderId;
        size_t m_maxUrlLength = 2048;

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
        IHttpHandlerPtr GetHttpHandler() const;
        BeFileName GetDefaultSchemaPath(WSInfoCR info) const;
        IActivityIdGeneratorCR GetActivityIdGenerator() const;

        void SetServiceVersion(BeVersion version) { m_serviceVersion  = version; }
        BeVersionCR GetServiceVersion() const { return m_serviceVersion; }

        void SetMaxUrlLength(size_t length) {m_maxUrlLength = length;}
        size_t GetMaxUrlLength() const {return m_maxUrlLength;}

        void SetPersistenceProviderId(Utf8StringCR persistence) { m_persistenceProviderId = persistence; }
        Utf8StringCR GetPersistenceProviderId() const { return m_persistenceProviderId; }

        HttpRequestHeadersR GetDefaultHeaders();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
