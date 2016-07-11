/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlHttpHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CurlHttpHandler.h"
#include <Bentley/BeTimeUtilities.h>
#include "../WebLogging.h"
#include "../SimplePackagedAsyncTask.h"
#include "CurlHttpRequest.h"
#include "NotificationPipe.h"
#include <BeHttp/HttpClient.h>
#include <Bentley/Tasks/AsyncTaskRunnerFactory.h>
#include "CurlTaskRunner.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

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
        std::shared_ptr<AsyncTaskRunnerFactory<CurlTaskRunner>> (new AsyncTaskRunnerFactory<CurlTaskRunner> ())
        );

    if (LOG.isSeverityEnabled(NativeLogging::LOG_DEBUG))
        {
        LOG.debugv("HttpBackend: %s", curl_version());
        }

#if defined (__APPLE__)
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
#if defined (__APPLE__) || defined (ANDROID)
    m_curlPool.Resize(0);
    curl_global_cleanup();
#endif
    
#if defined (__APPLE__)
    ApplicationEventsManager::GetInstance().RemoveApplicationEventsListener(*this);
#endif
    }

#if !defined (__APPLE__)
void ApplicationEventsManager::InitializeApplicationEventsListening(){}
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<Response> CurlHttpHandler::_PerformRequest(RequestCR request)
    {
    auto curlRequest = std::make_shared<CurlHttpRequest> (request, m_curlPool);
    auto task = std::make_shared<SimplePackagedAsyncTask<std::shared_ptr<CurlHttpRequest>, Response>> (curlRequest);
        
    m_webThreadPool->Push(task);

    NotificationPipe::GetDefault().Notify();
    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::AddApplicationEventsListener(IApplicationEventsListener& listener)
    {
    m_applicationEventsListeners.insert(&listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplicationEventsManager::RemoveApplicationEventsListener(IApplicationEventsListener& listener)
    {
    m_applicationEventsListeners.erase(&listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlHttpHandler::_OnApplicationResume()
    {
    CurlHttpRequest::ResetAllRequests();
    }
