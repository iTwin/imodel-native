/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/Curl/CurlTaskRunner.cpp $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

BeMutex CurlTaskRunner::s_suspendedMutex;
BeConditionVariable CurlTaskRunner::s_suspendedCondition;
volatile bool CurlTaskRunner::s_isOnActiveRequestCountChangedInitialized = false;
volatile bool CurlTaskRunner::s_doSuspendNewRequests = false;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CurlTaskRunner::CurlTaskRunner() : AsyncTaskRunner()
    {
    m_curlRunning.store(false);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::Suspend()
    {
    BeMutexHolder lock(s_suspendedMutex);
    if (!s_isOnActiveRequestCountChangedInitialized)
        {
        s_isOnActiveRequestCountChangedInitialized = true;
        CurlHttpRequest::SetOnActiveRequestCountChanged([]
            {
            s_suspendedCondition.notify_all();
            });
        };

    s_doSuspendNewRequests = true;

    s_suspendedCondition.notify_all();
    NotificationPipe::GetDefault().Notify();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::Activate()
    {
    BeMutexHolder lock(s_suspendedMutex);

    s_doSuspendNewRequests = false;

    s_suspendedCondition.notify_all();
    NotificationPipe::GetDefault().Notify();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurlTaskRunner::IsSuspended()
    {
    BeMutexHolder lock(s_suspendedMutex);
    return s_doSuspendNewRequests;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::WaitWhileSuspended()
    {
    struct Predicate : IConditionVariablePredicate
        {
        virtual bool _TestCondition(BeConditionVariable &cv) override
            {
            return !s_doSuspendNewRequests;
            }
        };

    Predicate predicate;
    s_suspendedCondition.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::WaitWhileSuspendedAndRunning()
    {
    struct Predicate : IConditionVariablePredicate
        {
        virtual bool _TestCondition(BeConditionVariable &cv) override
            {
            LOG.errorv("CURL WaitWhileSuspendedAndRunning _TestCondition suspended: %i, requests: %d", s_doSuspendNewRequests, CurlHttpRequest::GetActiveRequestCount());

            if (!s_doSuspendNewRequests)
                return true;

            // Check global request count due to ThreadCurlHttpHandler issuing requests as well
            if (0 == CurlHttpRequest::GetActiveRequestCount())
                return true;

            return false;
            }
        };

    Predicate predicate;
    s_suspendedCondition.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurlTaskRunner::AreRequestsRunning()
    {
    return 0 != CurlHttpRequest::GetActiveRequestCount();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Vincas.Razma            10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurlTaskRunner::PrepareRequestIfNotSuspended(CurlHttpRequest& curlRequest)
    {
    BeMutexHolder lock(s_suspendedMutex);
    if (s_doSuspendNewRequests)
        return false;
    curlRequest.PrepareRequest();
    return true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::_RunAsyncTasksLoop()
    {
    m_curlRunning.store(true);
    m_multi = curl_multi_init();

    if(0 != HttpClient::GetOptions().GetMaxConnectionsPerHost())
        curl_multi_setopt(m_multi, CURLMOPT_MAX_HOST_CONNECTIONS, HttpClient::GetOptions().GetMaxConnectionsPerHost());

    if (0 != HttpClient::GetOptions().GetMaxTotalConnections())
        curl_multi_setopt(m_multi, CURLMOPT_MAX_TOTAL_CONNECTIONS, HttpClient::GetOptions().GetMaxTotalConnections());

    int runningRequests = 0;
    CURLMcode status;

    NotificationPipe::GetDefault().Open();

    do
        {
#ifdef LOG_WEB_TIMES
        uint64_t begin = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
#endif
        /*
        LOG.errorv("CurlTaskRunner::_RunAsyncTasksLoop() suspended: %s, running: %d, queue: %d",
        s_doSuspendNewRequests ? "yes" : "no",
        CurlHttpRequest::GetActiveRequestCount(),
        GetTaskScheduler()->GetQueueTaskCount());
        */

        if (s_doSuspendNewRequests)
            {
            if (m_curlToRequestMap.empty())
                {
                s_suspendedCondition.notify_all();
                WaitWhileSuspended();
                }
            }
        else
            {
            WaitAndPopNewRequests();
            }

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

    m_curlRunning.store(false);
    m_curlRunningCondition.notify_all();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::WaitAndPopNewRequests()
    {
    NotificationPipe::GetDefault().ClearNotifications();

    std::shared_ptr<AsyncTask> task;

    while ((task = GetTaskScheduler()->TryPop()) != nullptr)
        AddTaskToCurlMultiMap(task);

    if (!m_curlToRequestMap.empty())
        return;

    // No running requests, wait for more
    task = GetTaskScheduler()->WaitAndPop();

    if (IsStopping())
        return;

    AddTaskToCurlMultiMap(task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::AddTaskToCurlMultiMap(std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder lock(s_suspendedMutex);
    if (s_doSuspendNewRequests)
        {
        GetTaskScheduler()->Push(task);
        return;
        }

    HttpClient::BeginNetworkActivity();

    auto requestTask = std::static_pointer_cast<SimplePackagedAsyncTask<std::shared_ptr<CurlHttpRequest>, Response>> (task);
    std::shared_ptr<CurlHttpRequest> request = requestTask->GetData();

    request->PrepareRequest();
    CURL* curl = request->GetCurlHandle();
    if (nullptr == curl)
        {
        ResolveRequestTask(requestTask);
        return;
        }

    auto status = curl_multi_add_handle(m_multi, curl);
    BeAssert(CURLM_OK == status);

    m_curlToRequestMap[curl] = requestTask;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::ResolveFinishedCurl (CURLMsg* curlMsg)
    {
    CURL* finishedCurl = curlMsg->easy_handle;
    CURLcode code = curlMsg->data.result;

    auto status = curl_multi_remove_handle (m_multi, finishedCurl);
    BeAssert (CURLM_OK == status);

    auto requestTask = m_curlToRequestMap[finishedCurl];
    requestTask->GetData()->FinalizeRequest(code);

    m_curlToRequestMap.erase (finishedCurl);

    ResolveRequestTask(requestTask);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::ResolveRequestTask(CurlHttpRequestTaskPtr requestTask)
    {
    HttpClient::EndNetworkActivity();

    if (requestTask->GetData()->ShouldRetry())
        {
        GetTaskScheduler()->Push(requestTask);
        return;
        }

    Response response = requestTask->GetData()->ResolveResponse();

    SetCurrentRunningTask(requestTask);
    requestTask->OnFinished(response);
    SetCurrentRunningTask(nullptr);
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

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::WaitUntilStopped()
    {
    struct Predicate : IConditionVariablePredicate
        {
        BeAtomic<bool>* curlRunning = nullptr;
        virtual bool _TestCondition(BeConditionVariable &cv) override
            {
            return !curlRunning->load();
            }
        };

    Predicate predicate;
    predicate.curlRunning = &this->m_curlRunning;
    this->m_curlRunningCondition.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ITaskRunner> CurlTaskRunner::Factory::CreateRunner()
    {
    auto runner = CurlTaskRunner::Create();
    m_runners.insert(runner);
    return runner;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::Factory::StopRunners()
    {
    for (auto& runner : m_runners)
        runner->Stop();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlTaskRunner::Factory::WaitUntilRunnersStopped()
    {
    for (auto& runner : m_runners)
        runner->WaitUntilStopped();
    }