/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/WorkerThread.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/AsyncTaskRunnerPool.h>

#define ASSERT_CURRENT_THREAD(expectedThread) BeAssert (BeThreadUtilities::GetCurrentThreadId () == expectedThread->GetThreadId () && "Code executed in wrong thread! Check your call stack.")

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct WorkerThread : public AsyncTaskRunnerPool
{
protected:
    BENTLEYDLL_EXPORT WorkerThread (Utf8CP name = nullptr);

public:
    static std::shared_ptr<struct WorkerThread> Create (Utf8CP name = nullptr)
        {
        return std::shared_ptr<WorkerThread> (new WorkerThread (name));
        }
    BENTLEYDLL_EXPORT virtual ~WorkerThread ();
    //! Return running thread id. Will return 0 if thread is not running.
    BENTLEYDLL_EXPORT intptr_t GetThreadId ();
};

typedef std::shared_ptr<WorkerThread> WorkerThreadPtr;

END_BENTLEY_TASKS_NAMESPACE
