/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/Curl/CurlTaskRunner.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "CurlTaskRunner.h"
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include <BeHttp/HttpClient.h>
#include "../SimplePackagedAsyncTask.h"
#include "CurlHttpRequest.h"
#include "NotificationPipe.h"
#include <Bentley/BeTimeUtilities.h>
#include "../WebLogging.h"

//#define LOG_WEB_TIMES

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CurlTaskRunner::CurlTaskRunner()
    : AsyncTaskRunner()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::_RunAsyncTasksLoop()
    {
    m_multi = curl_multi_init();

    int runningRequests = 0;
    CURLMcode status;

    NotificationPipe::GetDefault().Open();

    do
        {
#ifdef LOG_WEB_TIMES
        uint64_t begin = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
#endif

        // TODO: limit running request count?
        WaitAndPopNewRequests();

#ifdef LOG_WEB_TIMES
        uint64_t afterQueue = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
#endif

        status = curl_multi_perform(m_multi, &runningRequests);
        BeAssert(CURLM_OK == status);

#ifdef LOG_WEB_TIMES
        uint64_t afterMulti = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
#endif

        int messageCount = 0;
        do
            {
            CURLMsg* curlMsg = curl_multi_info_read(m_multi, &messageCount);

            if (nullptr == curlMsg)
                {
                continue;
                }
            if (curlMsg->msg != CURLMSG_DONE) // TODO: is CURLMSG_DONE check needed?
                {
                LOG.errorv("Unexpected CURLMsg message: %d", curlMsg->msg);
                BeAssert(false);
                continue;
                }

            ResolveFinishedCurl(curlMsg);
            }
        while (messageCount > 0);

#ifdef LOG_WEB_TIMES
        uint64_t afterHandling = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
#endif

        if (runningRequests > 0)
            {
            WaitForData(1000);
            }

#ifdef LOG_WEB_TIMES
        uint64_t end = BeTimeUtilities::GetCurrentTimeAsUnixMillis();

        LOG.tracev("WebLoop: queue:%4llums curlm:%4llums msgs:%4llums idle:%4llums runningRequests:%4d curlToRequestMap:%4d",
            afterQueue - begin,
            afterMulti - afterQueue,
            afterHandling - afterMulti,
            end - afterHandling,
            runningRequests,
            m_curlToRequestMap.size());
#endif
        }
    while (CURLM_OK == status && !IsStopping());

    NotificationPipe::GetDefault().Close();

    // Cleanup
    status = curl_multi_cleanup(m_multi);
    BeAssert(CURLM_OK == status);

    LOG.tracev("WebLoop: Ended");
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::WaitAndPopNewRequests()
    {
    NotificationPipe::GetDefault().ClearNotifications();

    std::shared_ptr<AsyncTask> task;

    while ((task = GetTaskScheduler()->TryPop()) != nullptr)
        {
        AddTaskToCurlMultiMap(task);
        }

    if (m_curlToRequestMap.empty())
        {
        task = GetTaskScheduler()->WaitAndPop();

        if (IsStopping())
            {
            return;
            }

        AddTaskToCurlMultiMap(task);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::AddTaskToCurlMultiMap(std::shared_ptr<AsyncTask> task)
    {
    HttpClient::BeginNetworkActivity();

    auto httpTask = std::static_pointer_cast<SimplePackagedAsyncTask<std::shared_ptr<CurlHttpRequest>, Response>> (task);

    std::shared_ptr<CurlHttpRequest> request = httpTask->GetData();
    request->PrepareRequest();

    auto status = curl_multi_add_handle(m_multi, request->GetCurlHandle());
    BeAssert(CURLM_OK == status);

    m_curlToRequestMap[request->GetCurlHandle()] = httpTask;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::ResolveFinishedCurl(CURLMsg* curlMsg)
    {
    HttpClient::EndNetworkActivity();

    CURL* finishedCurl = curlMsg->easy_handle;
    CURLcode code = curlMsg->data.result;

    auto status = curl_multi_remove_handle(m_multi, finishedCurl);
    BeAssert(CURLM_OK == status);

    auto finishedRequestTask = m_curlToRequestMap[finishedCurl];
    ConnectionStatus finishedStatus = finishedRequestTask->GetData()->GetConnectionStatus(code);

    m_curlToRequestMap.erase(finishedCurl);

    if (finishedRequestTask->GetData()->ShouldRetry(finishedStatus))
        {
        AddTaskToCurlMultiMap(finishedRequestTask);
        }
    else
        {
        Response response = finishedRequestTask->GetData()->ResolveResponse(finishedStatus);
        SetCurrentRunningTask(finishedRequestTask);

        finishedRequestTask->OnFinished(response);
        SetCurrentRunningTask(nullptr);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
::timeval MsToTimeval(long ms)
    {
    ::timeval tv;
    if (ms > 0)
        {
        tv.tv_sec = ms / 1000;
        tv.tv_usec = (ms % 1000) * 1000;
        }
    else
        {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        }
    return tv;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::WaitForData(long topTimeoutMs)
    {
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd = -1;

    FD_ZERO (&fdread);
    FD_ZERO (&fdwrite);
    FD_ZERO (&fdexcep);

    long curl_timeo = -1;

    CURLMcode status;

    status = curl_multi_timeout(m_multi, &curl_timeo);
    BeAssert(CURLM_OK == status);

    // no timeout available (-1) - no requests running, set top value
    // timeout from requests - limit to top value
    if (curl_timeo < 0 || curl_timeo > topTimeoutMs)
        {
        curl_timeo = topTimeoutMs;
        }
    curl_timeo = topTimeoutMs;

    ::timeval timeout = MsToTimeval(curl_timeo);

    // Get file keys from the transfers 
    status = curl_multi_fdset(m_multi, &fdread, &fdwrite, &fdexcep, &maxfd);
    BeAssert(CURLM_OK == status);

    NotificationPipe::GetDefault().AddListenFdToFdSet(fdread, maxfd);

    int rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
    if (rc == -1)
        {
        LOG.errorv("CurlTaskRunner::WaitForData: %s", GetLastNativeSocketErrorForLog().c_str());
        BeAssert(false);
        }
    }

