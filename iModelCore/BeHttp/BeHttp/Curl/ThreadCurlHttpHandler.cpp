/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/ThreadCurlHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ThreadCurlHttpHandler.h"

#include "CurlHttpRequest.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadCurlHttpHandler::ThreadCurlHttpHandler()
    {
    auto parallelTransfers = 10;
    auto threadCount = parallelTransfers + 1; // +1 for tasks that are being canceled

    m_threadPool = WorkerThreadPool::Create(threadCount, "WebThreadPool");
    m_threadQueue.SetLimit(parallelTransfers);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadCurlHttpHandler::~ThreadCurlHttpHandler()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> ThreadCurlHttpHandler::_PerformRequest(RequestCR request)
    {
    if (request.GetMethod().EqualsI ("GET"))
        {
        return CurlHttpHandler::_PerformRequest(request);
        }

    return m_threadQueue.Push([=]
        {
        return m_threadPool->ExecuteAsync<Response> ([=]
            {
            CurlHttpRequest curlRequest(request, m_curlPool);

            ConnectionStatus connectionStatus;
            CURLcode curlStatus = CURLcode::CURL_LAST;
            do
                {
                curlRequest.PrepareRequest();
                CURL* curl = curlRequest.GetCurlHandle();
                curlStatus = curl_easy_perform(curl);
                connectionStatus = curlRequest.GetConnectionStatus(curlStatus);
                }
            while (curlRequest.ShouldRetry(connectionStatus));

            return curlRequest.ResolveResponse(connectionStatus);
            });
        }, request.GetCancellationToken());
    }

// Test for CURLM
//AsyncTaskPtr<Response> CurlHttpHandler::PerformRequest (HttpRequestCR request)
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
//
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
