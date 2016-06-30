/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/WorkerThreadPool.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/AsyncTaskRunnerPool.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct WorkerThreadPool : public AsyncTaskRunnerPool
    {
    protected:
        BENTLEYDLL_EXPORT WorkerThreadPool (int threadCount = 1, Utf8CP name = nullptr, std::shared_ptr<IAsyncTaskRunnerFactory> runnerFactory = nullptr);

    public:
        static std::shared_ptr<struct WorkerThreadPool> Create (int threadCount = 1, Utf8CP name = nullptr, std::shared_ptr<IAsyncTaskRunnerFactory> runnerFactory = nullptr)
            {
            return std::shared_ptr<WorkerThreadPool> (new WorkerThreadPool (threadCount, name, runnerFactory));
            }

        BENTLEYDLL_EXPORT virtual ~WorkerThreadPool ();
    };

END_BENTLEY_TASKS_NAMESPACE
