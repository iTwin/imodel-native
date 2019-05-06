/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/WSJob.h>
#include "../../ClientConfiguration.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_HTTP

typedef AsyncResult<Http::Response, WSError>  HttpJobResult;
typedef std::shared_ptr<struct JobApi> JobApiPtr;
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                 julius.cepukenas    01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct JobApi
    {
    protected:
        const std::shared_ptr<const ClientConfiguration> m_configuration;

    private:
        IWSRepositoryClient::JobOptionsPtr m_options;

        AsyncTaskPtr<Http::Response> DeleteJob(Utf8StringCR jobUrl, ICancellationTokenPtr ct = nullptr) const;
        int64_t CalculateJobDurationMs(DateTime JobSheduledDate) const;

        AsyncTaskPtr<HttpJobResult> ExecuteViaJob
            (
            AsyncTaskPtr<Http::Response> requestTask,
            bool isJobApiEnabled,
            ICancellationTokenPtr ct
            ) const;

    public:
        WSCLIENT_EXPORT JobApi(std::shared_ptr<const ClientConfiguration> configuration) :
            m_configuration(configuration)
            {m_options = std::make_shared<IWSRepositoryClient::JobOptions>();}

        WSCLIENT_EXPORT static JobApiPtr Create(std::shared_ptr<const ClientConfiguration> configuration);

        WSCLIENT_EXPORT AsyncTaskPtr<HttpJobResult> WaitForJobToFinish
            (
            Http::Request request,
            uint32_t waitTime,
            WorkerThreadPtr thread,
            ICancellationTokenPtr ct,
            std::shared_ptr<HttpJobResult> finalResultOut
            ) const;

        WSCLIENT_EXPORT AsyncTaskPtr<HttpJobResult> WaitForJobToFinish
            (
            Http::Response& httpResponse,
            Utf8StringCR jobUrl,
            ICancellationTokenPtr ct
            ) const;

        WSCLIENT_EXPORT bool AddJobHeaderTo
            (
            HttpRequestHeadersR headers,
            WSInfoCR info,
            IWSRepositoryClient::JobOptionsPtr options
            ) const;

        WSCLIENT_EXPORT AsyncTaskPtr<HttpJobResult> ExecuteViaJob
            (
            Http::Request request,
            WSInfoCR info,
            IWSRepositoryClient::JobOptionsPtr options,
            ICancellationTokenPtr ct
            ) const;
        WSCLIENT_EXPORT AsyncTaskPtr<HttpJobResult> ExecuteViaJob
            (
            ChunkedUploadRequest request,
            WSInfoCR info,
            IWSRepositoryClient::JobOptionsPtr options,
            ICancellationTokenPtr ct
            ) const;

        WSCLIENT_EXPORT IWSRepositoryClient::JobOptionsPtr GetJobOptions() const {return m_options;}
        WSCLIENT_EXPORT void SetJobWaitOptions(IWSRepositoryClient::JobOptionsPtr options) {m_options = options;}
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
