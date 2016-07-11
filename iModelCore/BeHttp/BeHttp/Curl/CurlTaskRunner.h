/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/Curl/CurlTaskRunner.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Tasks/AsyncTaskRunner.h>
#include "../SimplePackagedAsyncTask.h"
#include "CurlHttpRequest.h"
#include "CurlPool.h"

BEGIN_BENTLEY_HTTP_NAMESPACE

struct ITaskScheduler;
struct HttpRequestTask;
struct CurlTaskRunner;

typedef RefCountedPtr<CurlTaskRunner> CurlTaskRunnerPtr;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurlTaskRunner : Tasks::AsyncTaskRunner
    {
private:
    bmap<CURL*, std::shared_ptr<SimplePackagedAsyncTask<std::shared_ptr<CurlHttpRequest>, Response>>> m_curlToRequestMap;
    CURLM* m_multi;
    
private:
    void WaitAndPopNewRequests ();
    void AddTaskToCurlMultiMap (std::shared_ptr<Tasks::AsyncTask> task);
    void ResolveFinishedCurl (CURLMsg* curlMsg);
    void WaitForData (long topTimeoutMs);
    
protected:
    virtual void _RunAsyncTasksLoop () override;

    BEHTTP_EXPORT CurlTaskRunner ();
    
public:
    BEHTTP_EXPORT static std::shared_ptr<CurlTaskRunner> Create ()
        {
        return std::shared_ptr <CurlTaskRunner> (new CurlTaskRunner ());
        }
    };

END_BENTLEY_HTTP_NAMESPACE
