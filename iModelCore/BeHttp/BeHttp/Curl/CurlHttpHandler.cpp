/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlHttpHandler.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CurlHttpHandler::CurlHttpHandler()
    {
    curl_global_init(CURL_GLOBAL_ALL);
    m_curlPool.Resize(10);

    m_webThreadPool = WorkerThreadPool::Create
        (
        1,
        "Curl Web",
        std::shared_ptr<AsyncTaskRunnerFactory<CurlTaskRunner>>(new AsyncTaskRunnerFactory<CurlTaskRunner>())
        );

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
    // XXX: First "#if defined" is ugly hack for Windows. We don't need to clean-up anything.
    // Here is what happens on Windows:
    // 1. CurlHttpHandler is created as global object (see FieldApps\MobileUtils\Native\Web\Http\DefaultHttpHandler.cpp)
    // 2. On application exit this global object is destroyed from DllMain and mutex usage is not allowed there. (See
    //  http://stackoverflow.com/questions/14711263/c11-stdmutex-in-visual-studio-2012-deadlock-when-locked-from-dllmain
    //  for details)
    //
    //  I have checked with WinDbg that all threads are already destroyed when this destructor is called. Therefore
    //  we will be good citizens on Androis and iOS only :-)
#if defined (BENTLEYCONFIG_OS_APPLE_IOS) || defined (BENTLEYCONFIG_OS_ANDROID)
    m_curlPool.Resize(0);
    curl_global_cleanup();
#endif

#if defined (BENTLEYCONFIG_OS_APPLE_IOS)
    ApplicationEventsManager::GetInstance().RemoveApplicationEventsListener(*this);
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> CurlHttpHandler::_PerformRequest(RequestCR request)
    {
    auto curlRequest = std::make_shared<CurlHttpRequest>(request, m_curlPool);
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
