/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/ITaskRunner.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <memory>

BEGIN_BENTLEY_TASKS_NAMESPACE

struct AsyncTask;
struct ITaskScheduler;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct ITaskRunner : public std::enable_shared_from_this<ITaskRunner>
    {
    public:
        virtual void Start (std::shared_ptr<ITaskScheduler> scheduler, Utf8CP name = nullptr) = 0;
        virtual void Stop () = 0;
        virtual void WakeUp () = 0;
        virtual bool IsRunning () const = 0;
        virtual bool IsStopping () const = 0;

        virtual intptr_t GetId () const = 0;

        virtual std::shared_ptr<AsyncTask>      GetCurrentRunningTask () = 0;
        virtual std::shared_ptr<ITaskScheduler> GetTaskScheduler () = 0;
    };

END_BENTLEY_TASKS_NAMESPACE
