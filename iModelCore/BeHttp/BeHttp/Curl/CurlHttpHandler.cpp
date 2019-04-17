/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "CurlHttpHandler.h"

#include <BeHttp/HttpClient.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/Tasks/AsyncTaskRunnerFactory.h>

#include "../SimplePackagedAsyncTask.h"
#include "../WebLogging.h"
#include "CurlHttpRequest.h"
#include "CurlTaskRunner.h"
#include "NotificationPipe.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

#define ENABLE_BACKGROUND_TRANSFER

bool CurlHttpHandler::s_waitOnDestroy = true;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpHandler::ProcessInitialize()
    {
    curl_global_init(CURL_GLOBAL_ALL);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpHandler::ProcessUninitialize()
    {
    // Windows specific.
    // This needs to be called when all other threads are destroyed - generally in process shutdown.
    // Otherwise DNS resolver thread may be still alive and try to run after CURL data is no longer here.
    curl_global_cleanup();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpHandler::CurlHttpHandler()
    {
    m_ct = SimpleCancellationToken::Create();
    CurlHttpRequest::SetGlobalCancellationToken(m_ct);

    m_curlPool.Resize(10);
    m_webRunnerFactory = std::make_shared<CurlTaskRunner::Factory>();
    m_webThreadPool = WorkerThreadPool::Create(1, "Curl Web", m_webRunnerFactory);

    InitStartBackgroundTask(nullptr);

#if defined (BENTLEYCONFIG_OS_APPLE_IOS)
    ApplicationEventsManager::GetInstance().AddApplicationEventsListener(*this);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpHandler::~CurlHttpHandler()
    {
    CancelAllRequests();

    m_webRunnerFactory->StopRunners();
    m_webThreadPool = nullptr;
    if (s_waitOnDestroy)
        m_webRunnerFactory->WaitUntilRunnersStopped();
    m_curlPool.Resize(0);

#if defined (BENTLEYCONFIG_OS_APPLE_IOS)
    ApplicationEventsManager::GetInstance().RemoveApplicationEventsListener(*this);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpHandler::CancelAllRequests()
    {
    m_ct->SetCanceled();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> CurlHttpHandler::_PerformRequest(RequestCR request)
    {
    // No need to reuse CURL handles when using curl_multi_*, just reuse CURLM handle
    auto curlRequest = std::make_shared<CurlHttpRequest>(request, m_emptyCurlPool);
    auto task = std::make_shared<SimplePackagedAsyncTask<std::shared_ptr<CurlHttpRequest>, Response>>(curlRequest);

    m_webThreadPool->Push(task);

    NotificationPipe::GetDefault().Notify();

    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpHandler::_OnApplicationSentToBackground()
    {
#ifdef ENABLE_BACKGROUND_TRANSFER
    CurlTaskRunner::Suspend();
    if (!CurlTaskRunner::AreRequestsRunning())
        return;

    auto onBackground = []
        {
        CurlTaskRunner::WaitWhileSuspendedAndRunning();
        BeThreadUtilities::BeSleep(1000); // Allow optimistic processing of http responses
        };
    auto onExpired = []
        {
        CurlHttpRequest::ResetAllActiveRequests();
        };

    m_startBackgroundTask("BeHttp.Suspending", onBackground, onExpired);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpHandler::_OnApplicationSentToForeground()
    {
#ifdef ENABLE_BACKGROUND_TRANSFER
    CurlTaskRunner::Activate();
#else
    CurlHttpRequest::ResetAllActiveRequests();
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpHandler::InitStartBackgroundTask(StartBackgroundTask callback)
    {
    if (callback)
        {
        m_startBackgroundTask = callback;
        return;
        }

    using namespace std::placeholders;
    m_startBackgroundTask = std::bind(&ApplicationEventsManager::StartBackgroundTask, _1, _2, _3);
    };
