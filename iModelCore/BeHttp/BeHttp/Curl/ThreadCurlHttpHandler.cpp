/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "ThreadCurlHttpHandler.h"

#include "CurlHttpRequest.h"
#include "CurlTaskRunner.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

bool ThreadCurlHttpHandler::s_parallelizeAllRequests = false;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadCurlHttpHandler::ThreadCurlHttpHandler()
    {
    auto parallelTransfers = 10;
    auto threadCount = parallelTransfers + 1; // +1 for tasks that are being canceled

    m_threadPool = WorkerThreadPool::Create(threadCount, "BeHttp");
    m_threadQueue = LimitingTaskQueue<Response>(m_threadPool);
    m_threadQueue.SetLimit(parallelTransfers);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadCurlHttpHandler::SetParallelizeAllRequests(bool parallelize)
    {
    s_parallelizeAllRequests = parallelize;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadCurlHttpHandler::~ThreadCurlHttpHandler()
    {
    m_threadPool->OnEmpty()->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> ThreadCurlHttpHandler::_PerformRequest(RequestCR request)
    {
    if (s_parallelizeAllRequests || request.GetMethod().EqualsI("GET"))
        return CurlHttpHandler::_PerformRequest(request);

    return PerformThreadedRequest(request);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Travis.Cobbs    09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> ThreadCurlHttpHandler::PerformThreadedRequest(RequestCR request)
    {
    return m_threadQueue.Push([=]
        {
        return m_threadPool->ExecuteAsync<Response>([=]
            {
            CurlHttpRequest curlRequest(request, m_curlPool);

            bool retry = true;
            while (retry)
                {
                CurlTaskRunner::WaitWhileSuspended();
                if (!CurlTaskRunner::PrepareRequestIfNotSuspended(curlRequest))
                    continue;

                CURL* curl = curlRequest.GetCurlHandle();
                if (nullptr == curl)
                    break;
                CURLcode status = curl_easy_perform(curl);
                curlRequest.FinalizeRequest(status);
                retry = curlRequest.ShouldRetry();
                }

            return curlRequest.ResolveResponse();
            });
        }, request.GetCancellationToken());
    }

// Test for CURLM
//AsyncTaskPtr<Response> CurlHttpHandler::PerformRequest (RequestCR request)
//    {
//    return m_threadQueue.Push ([=]
//        {
//        return m_threadPool->ExecuteAsync<Response> ([=]
//            {
//            CurlHttpRequest request (request, m_curlPool);
//            request.PrepareRequest ();
//
//            CURLM* multi = curl_multi_init ();
//
//            CURLMcode culrmStatus;
//            CURLcode curlStatus = CURLcode::CURL_LAST;
//            do
//                {
//                request.PrepareRequest ();
//                CURL* curl = request.GetCurlHandle ();
//                  //if (nullptr == curl)
//                  // break;
//                culrmStatus = curl_multi_add_handle (multi, curl);
//                BeAssert (CURLM_OK == culrmStatus);
//
//                int running = 1;
//                while (running)
//                    {
//                    culrmStatus = curl_multi_perform (multi, &running);
//                    BeAssert (CURLM_OK == culrmStatus);
//                    }
//                int messages = 0;
//                CURLMsg* curlMsg = curl_multi_info_read (multi, &messages);
//                curlStatus = curlMsg->data.result;
//                BeAssert (curlMsg->easy_handle == curl);
//
//                culrmStatus = curl_multi_remove_handle (multi, curl);
//                BeAssert (CURLM_OK == culrmStatus);
//                }
//            while (request.ShouldRetry (request.GetConnectionStatus (curlStatus)));
//
//            curl_multi_cleanup (multi);
//
//            return request.ResolveResponse (request.GetConnectionStatus (curlStatus));
//            });
//        }, request.GetCancellationToken ());
//    }
