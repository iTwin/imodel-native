/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlHttpHandler.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeHttp/HttpRequest.h>
#include <BeHttp/IHttpHandler.h>
#include <Bentley/Tasks/WorkerThreadPool.h>

#include "CurlPool.h"
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

    protected:
        // NotificationPipe for wake up from idle when request is added. Will not work for cancellation of existing requests.
        NotificationPipe                    m_notifier;
        std::shared_ptr<WorkerThreadPool>   m_webThreadPool;
        CurlPool                            m_curlPool;
        StartBackgroundTask                 m_startBackgroundTask;

    public:
        CurlHttpHandler();
        virtual ~CurlHttpHandler();

        virtual AsyncTaskPtr<Response> _PerformRequest(RequestCR request) override;

        virtual void _OnApplicationSentToBackground() override;
        virtual void _OnApplicationSentToForeground() override;
        void InitStartBackgroundTask(StartBackgroundTask callback);
    };

END_BENTLEY_HTTP_NAMESPACE
