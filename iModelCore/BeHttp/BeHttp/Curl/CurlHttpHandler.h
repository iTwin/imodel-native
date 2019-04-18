/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/HttpRequest.h>
#include <BeHttp/IHttpHandler.h>
#include <Bentley/Tasks/WorkerThreadPool.h>

#include "CurlPool.h"
#include "CurlTaskRunner.h"
#include "NotificationPipe.h"
#include "../ApplicationEvents.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurlHttpHandler : IHttpHandler, IApplicationEventsListener
    {
    public:
        typedef std::function<void(Utf8CP name, std::function<void()> task, std::function<void()> onExpired)> StartBackgroundTask;

    private:
        static bool s_waitOnDestroy;

    protected:
        // NotificationPipe for wake up from idle when request is added. Will not work for cancellation of existing requests.
        NotificationPipe                    m_notifier;
        std::shared_ptr<CurlTaskRunner::Factory>    m_webRunnerFactory;
        std::shared_ptr<WorkerThreadPool>   m_webThreadPool;
        CurlPool                            m_curlPool;
        CurlPool                            m_emptyCurlPool;
        StartBackgroundTask                 m_startBackgroundTask;
        SimpleCancellationTokenPtr          m_ct;

    public:
        //! Prepare CURL for using, should be called once in process
        static void ProcessInitialize();
        //! Unload CURL, should be called when process is shutting down
        static void ProcessUninitialize();

        CurlHttpHandler();
        virtual ~CurlHttpHandler();

        virtual AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override;

        virtual void _OnApplicationSentToBackground() override;
        virtual void _OnApplicationSentToForeground() override;
        void InitStartBackgroundTask(StartBackgroundTask callback);

        void CancelAllRequests();

        static void SetWaitOnDestroy(bool wait) { s_waitOnDestroy = wait; }
    };

END_BENTLEY_HTTP_NAMESPACE
