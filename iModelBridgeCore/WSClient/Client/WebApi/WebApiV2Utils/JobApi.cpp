/*--------------------------------------------------------------------------------------+
|
|     $Source: Client/WebApi/WebApiV2Utils/JobApi.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include "JobApi.h"

#define HEADER_MasAsyncJob              "Mas-Async-Job"
#define HEADER_OperationLocation        "Operation-Location"
#define VALUE_Allow                     "Allow"

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
JobApiPtr JobApi::Create(std::shared_ptr<const ClientConfiguration> configuration)
    {
    return std::make_shared<JobApi>(configuration);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<HttpJobResult> JobApi::ExecuteViaJob(Http::Request request, WSInfoCR info, IWSRepositoryClient::JobOptionsPtr options, ICancellationTokenPtr ct) const
    {
    bool isJobApiEnabled = AddJobHeaderTo(request.GetHeaders(), info, options);
    return ExecuteViaJob(request.PerformAsync(), isJobApiEnabled, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<HttpJobResult> JobApi::ExecuteViaJob(ChunkedUploadRequest request, WSInfoCR info, IWSRepositoryClient::JobOptionsPtr options, ICancellationTokenPtr ct) const
    {
    bool isJobApiEnabled = AddJobHeaderTo(request.GetLastRequestHeaders(), info, options);
    return ExecuteViaJob(request.PerformAsync(), isJobApiEnabled, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<HttpJobResult> JobApi::ExecuteViaJob
    (
    AsyncTaskPtr<Http::Response> requestTask,
    bool isJobApiEnabled,
    ICancellationTokenPtr ct
    ) const
    {
    auto finalResult = std::make_shared<HttpJobResult>();
    return requestTask->Then([=] (Http::Response& httpResponse)
        {
        if (!isJobApiEnabled)
            {
            *finalResult = HttpJobResult::Success(httpResponse);
            return;
            }

        Utf8String operationLocation = httpResponse.GetHeaders().GetValue(HEADER_OperationLocation);
        if (HttpStatus::Accepted != httpResponse.GetHttpStatus() || operationLocation.empty())
            {
            *finalResult = HttpJobResult::Success(httpResponse);
            return;
            }

        WaitForJobToFinish(httpResponse, operationLocation, ct)
            ->Then([=] (HttpJobResult& response)
            {
            *finalResult = response;
            });

        })->Then<HttpJobResult>([=] ()
            {
            return *finalResult;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<HttpJobResult> JobApi::WaitForJobToFinish
(
Http::Response& httpResponse,
Utf8StringCR jobUrl,
ICancellationTokenPtr ct
) const
    {
    LOG.debug("Async- Job request has started");
    auto finalResult = std::make_shared<HttpJobResult>();

    auto jobThread = WorkerThread::Create("JobThread");
    Http::Request jobRequest = m_configuration->GetHttpClient().CreateRequest(jobUrl, "GET");

    return WaitForJobToFinish(jobRequest, m_options->GetInitialWaitIntervalMs(), jobThread, ct, finalResult)
        ->Then<HttpJobResult>(jobThread, [=] (HttpJobResult& result)
        {
        DeleteJob(jobUrl, ct);

        if (finalResult->IsSuccess())
            {
            WSJob wsJob(result.GetValue(), httpResponse.GetEffectiveUrl());
            if (!wsJob.IsValid() && !wsJob.HasResponseContent())
                {
                LOG.warning("Bad response format of the successful job");
                return HttpJobResult::Error(WSError::Id::ServerError);
                }

            return HttpJobResult::Success(wsJob.GetResponse());
            }

        return *finalResult;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<HttpJobResult> JobApi::WaitForJobToFinish
(
Http::Request jobRequest,
uint32_t waitTime,
WorkerThreadPtr thread,
ICancellationTokenPtr ct,
std::shared_ptr<HttpJobResult> finalResultOut
) const
    {
    LOG.debug("Investigating Async Job status");

    waitTime = (uint32_t) (waitTime * m_options->GetInitialWaitIntervalMs());
    if (waitTime > m_options->GetMaxWaitIntervalTimeMs())
        waitTime = m_options->GetMaxWaitIntervalTimeMs();

    jobRequest.SetCancellationToken(ct);
    return jobRequest.PerformAsync()->Then(thread, [=] (Http::Response& jobResponse)
        {
        if (HttpStatus::OK != jobResponse.GetHttpStatus())
            {
            LOG.debug("Asynchronous request: Unexpected error happend while investigating request");
            finalResultOut->SetError(WSError::Id::ServerError);
            return;
            }

        WSJob wsJob(jobResponse);

        auto jobStatus = wsJob.GetStatus();
        if (jobStatus == WSJob::Status::Failed)
            {
            LOG.debug("Asynchronous request: Request failed");
            finalResultOut->SetError(WSError::Id::ServerError);
            return;
            }

        if (jobStatus == WSJob::Status::Succeeded)
            {
            LOG.debug("Asynchronous request: Request succeeded");
            finalResultOut->SetSuccess(jobResponse);
            return;
            }

        DateTime date = wsJob.GetScheduleTime();
        int64_t jobDurationTimeMs = CalculateJobDurationMs(date);
        if (m_options->GetMaxTotalWaitTimeMs() <= jobDurationTimeMs)
            {
            LOG.debugv("Asynchronous request: Error - job took longer than configured maximum time of %lums", m_options->GetMaxTotalWaitTimeMs());
            finalResultOut->SetError(WSError::Id::ServerError);
            return;
            }

        if (WSJob::Status::NotStarted != jobStatus && WSJob::Status::Running != jobStatus)
            {
            LOG.debug("Asynchronous request: Unexpected error happend while performing request");
            finalResultOut->SetError(WSError::Id::ServerError);
            return;
            }

        LOG.debugv("Asynchronous request: performing wait %lums", waitTime);
        BeThreadUtilities::BeSleep(waitTime);

        WaitForJobToFinish(jobRequest, waitTime, thread, ct, finalResultOut);
        })->Then<HttpJobResult>([=]
            {
            return *finalResultOut;
            });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
bool JobApi::AddJobHeaderTo(HttpRequestHeadersR headers, WSInfoCR info, IWSRepositoryClient::JobOptionsPtr options) const
    {
    if (!options || !options->IsJobsApiEnabled() || info.GetVersion() < BeVersion(2,6,6,0))
        return false;

    headers.AddValue(HEADER_MasAsyncJob, VALUE_Allow);
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
AsyncTaskPtr<Http::Response> JobApi::DeleteJob(Utf8StringCR jobUrl, ICancellationTokenPtr ct) const
    {
    Http::Request request = m_configuration->GetHttpClient().CreateRequest(jobUrl, "DELETE");
    request.SetCancellationToken(ct);
    return request.PerformAsync();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                julius.cepukenas   12/2017
+--------------------------------------------------------------------------------------*/
int64_t JobApi::CalculateJobDurationMs(DateTime JobSheduledDate) const
    {
    int64_t currentTimeMs = 0;
    DateTime::GetCurrentTime().ToUnixMilliseconds(currentTimeMs);
    int64_t jobScheduledTimeMs = 0;
    JobSheduledDate.ToUnixMilliseconds(jobScheduledTimeMs);
    return currentTimeMs - jobScheduledTimeMs;
    }
