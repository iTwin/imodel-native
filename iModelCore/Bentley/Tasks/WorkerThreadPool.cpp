/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
