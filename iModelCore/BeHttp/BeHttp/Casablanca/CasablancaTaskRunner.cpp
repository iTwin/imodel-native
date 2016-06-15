/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/Casablanca/CasablancaTaskRunner.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include "CasablancaTaskRunner.h"

#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include <BeHttp/HttpClient.h>

#include "../SimplePackagedAsyncTask.h"
#include "CasablancaHttpRequest.h"

#include <Bentley/BeTimeUtilities.h>
#include "../WebLogging.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CasablancaTaskRunner::CasablancaTaskRunner ()
    :   AsyncTaskRunner ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CasablancaTaskRunner::_RunAsyncTasksLoop()
    {
    while (true)
        {
        std::shared_ptr<AsyncTask> task = GetTaskScheduler()->WaitAndPop();

        auto httpTask = std::static_pointer_cast<SimplePackagedAsyncTask<std::shared_ptr<CasablancaHttpRequest>, HttpResponse>> (task);

        CasablancaHttpRequest::PerformPplxAsync(httpTask->GetData ()).then([=](HttpResponse& response)
            {
            SetCurrentRunningTask (task);
            httpTask->OnFinished (response);
            SetCurrentRunningTask (NULL);
            });

        if (IsStopping ())
            break;
        }
    }
