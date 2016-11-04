/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/WorkerThreadPool.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <Bentley/Tasks/WorkerThreadPool.h>


USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThreadPool::WorkerThreadPool (int threadCount, Utf8CP name, std::shared_ptr<IAsyncTaskRunnerFactory> runnerFactory)
: AsyncTaskRunnerPool (threadCount, name, runnerFactory)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WorkerThreadPool::~WorkerThreadPool ()
    {
    }
