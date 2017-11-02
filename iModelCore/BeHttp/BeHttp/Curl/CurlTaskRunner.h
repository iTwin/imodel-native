/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/Curl/CurlTaskRunner.h $
 |
 |  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Tasks/AsyncTaskRunner.h>
#include "../SimplePackagedAsyncTask.h"
#include "CurlHttpRequest.h"
#include "CurlPool.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

USING_NAMESPACE_BENTLEY_TASKS

struct ITaskScheduler;
struct HttpRequestTask;
struct CurlTaskRunner;

typedef RefCountedPtr<CurlTaskRunner> CurlTaskRunnerPtr;
typedef std::shared_ptr<SimplePackagedAsyncTask<std::shared_ptr<CurlHttpRequest>, Response>> CurlHttpRequestTaskPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurlTaskRunner : Tasks::AsyncTaskRunner
    {
    private:
        static BeMutex s_suspendedMutex;
        static BeConditionVariable s_suspendedCondition;
        static volatile bool s_doSuspendNewRequests;
        static volatile bool s_isOnActiveRequestCountChangedInitialized;

    private:
        bmap<CURL*, CurlHttpRequestTaskPtr> m_curlToRequestMap;
        CURLM* m_multi;

    private:
        void WaitAndPopNewRequests();
        void AddTaskToCurlMultiMap(std::shared_ptr<AsyncTask> task);
        void ResolveFinishedCurl(CURLMsg* curlMsg);
        void WaitForData(long topTimeoutMs);

    protected:
        virtual void _RunAsyncTasksLoop() override;
        CurlTaskRunner () : AsyncTaskRunner(){}

    public:
        static std::shared_ptr<CurlTaskRunner> Create()
            {
            return std::shared_ptr <CurlTaskRunner>(new CurlTaskRunner());
            }

        //! Suspend request queue while leaving running requests to complete
        static bool IsSuspended();
        //! Suspend request queue while leaving running requests to complete
        static void Suspend();
        //! Activate request queue
        static void Activate();
        //! Wait if and while HTTP requests are suspended
        static void WaitWhileSuspended();
        //! Will sleep until there are running requests and HTTP traffic is suspended.
        static void WaitWhileSuspendedAndRunning();
        //! Check if there is active running requests
        static bool AreRequestsRunning();
        //! Prepare request while handling suspended state
        static bool PrepareRequestIfNotSuspended(CurlHttpRequest& request);
    };

END_BENTLEY_HTTP_NAMESPACE
