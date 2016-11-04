/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/AsyncTaskRunner.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/Tasks/ITaskRunner.h>
#include <thread>
#include <Bentley/BeAtomic.h>
#include <Bentley/WString.h>
BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE AsyncTaskRunner : public ITaskRunner
    {
    private:
        BeAtomic<bool>                  m_isRunning;
        BeAtomic<bool>                  m_isStopping;

        std::weak_ptr<ITaskScheduler>   m_scheduler;
        std::shared_ptr<ITaskScheduler> m_schedulerToHoldWhileStopping;
        std::shared_ptr<AsyncTask>      m_currentRunningTask;

        Utf8String                      m_name;
        intptr_t                        m_id;

        std::shared_ptr<ITaskRunner>    m_thisPtr;
        
    protected:
        void _Run ();

        BENTLEYDLL_EXPORT virtual void _RunAsyncTasksLoop ();

        BENTLEYDLL_EXPORT void SetCurrentRunningTask (std::shared_ptr<AsyncTask> task) { m_currentRunningTask = task; }    

        BENTLEYDLL_EXPORT AsyncTaskRunner ();

    public:
        BENTLEYDLL_EXPORT static std::shared_ptr<AsyncTaskRunner> Create ()
            {
            return std::shared_ptr <AsyncTaskRunner> (new AsyncTaskRunner());
            }

        BENTLEYDLL_EXPORT virtual ~AsyncTaskRunner ();

        // ITaskRunner implementation
        BENTLEYDLL_EXPORT void Start (std::shared_ptr<ITaskScheduler> scheduler, Utf8CP name = nullptr) override;
        BENTLEYDLL_EXPORT void Stop () override;
        BENTLEYDLL_EXPORT void WakeUp() override;
        BENTLEYDLL_EXPORT bool IsRunning () const override;
        BENTLEYDLL_EXPORT bool IsStopping () const override;

        BENTLEYDLL_EXPORT intptr_t GetId () const override;

        BENTLEYDLL_EXPORT std::shared_ptr<AsyncTask>      GetCurrentRunningTask () override;
        BENTLEYDLL_EXPORT std::shared_ptr<ITaskScheduler> GetTaskScheduler () override;
    };

END_BENTLEY_TASKS_NAMESPACE
