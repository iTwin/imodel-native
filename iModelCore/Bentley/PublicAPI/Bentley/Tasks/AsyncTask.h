/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/Bentley/Tasks/AsyncTask.h $
 |
 |  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/bset.h>
#include <functional>
#include <Bentley/BeDebugUtilities.h>
#include <Bentley/BeThread.h>

BEGIN_BENTLEY_TASKS_NAMESPACE

struct ITaskScheduler;

template <class T> struct PackagedAsyncTask;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct OnAsyncTaskCompletedListener
{
friend struct AsyncTask;
protected:
    virtual void _OnAsyncTaskCompleted (std::shared_ptr<struct AsyncTask> task) = 0;
};

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE AsyncTask : public std::enable_shared_from_this<AsyncTask>
    {
    public:
        friend struct TaskScheduler;
        friend struct AsyncTaskRunner;

        enum class Priority
            {
            Inherited = -1,
            Low = 1,
            Normal = 2,
            High = 3
            };

    private:
        static bool s_stackInfoEnabled;

    private:
        BeMutex m_mutex;
        BeConditionVariable m_completedCV;

        bset<std::weak_ptr<OnAsyncTaskCompletedListener>, std::owner_less<std::weak_ptr<OnAsyncTaskCompletedListener>>> m_onCompletedListeners;

        bool m_executed;
        BeAtomic<bool> m_completed;

        bvector <bpair<std::shared_ptr<AsyncTask>, std::shared_ptr<ITaskScheduler>>> m_thenTasks;
        bset<std::shared_ptr<AsyncTask>> m_subTasks;
        bset<std::shared_ptr<AsyncTask>> m_parentTasks;

        Priority m_priority;

        BeDebugUtilities::StackFrameInfo* m_stackInfo = nullptr;

    private:
        void SetPriority (Priority priority);

        void CheckCompletion (BeMutexHolder& lock);
        void OnCompleted (BeMutexHolder& lock);

        void AddSubTaskNoLock (std::shared_ptr<AsyncTask> task);
        void AddParentTaskNoLock (std::shared_ptr<AsyncTask> task);

        void ProcessTaskCompletion ();
        void CleanUpTask           (std::shared_ptr<AsyncTask> task, bset<std::shared_ptr<AsyncTask>>& tasksToCleanUp);  

        void NotifyOnCompletedListeners ();

    protected:
        BENTLEYDLL_EXPORT virtual void _OnExecute ();

        BENTLEYDLL_EXPORT void RegisterOnCompletedListener (std::shared_ptr<OnAsyncTaskCompletedListener> listener);
        BENTLEYDLL_EXPORT void UnregisterOnCompletedListener (std::shared_ptr<OnAsyncTaskCompletedListener> listener);

        BENTLEYDLL_EXPORT void AddThenTask (std::shared_ptr<AsyncTask> task, std::shared_ptr<ITaskScheduler> scheduler = nullptr);
        BENTLEYDLL_EXPORT void RemoveSubTask (std::shared_ptr<AsyncTask> task);
        BENTLEYDLL_EXPORT void AddParentTask (std::shared_ptr<AsyncTask> task);
        BENTLEYDLL_EXPORT void RemoveParentTask (std::shared_ptr<AsyncTask> task);
        BENTLEYDLL_EXPORT bset<std::shared_ptr<AsyncTask>> GetParentTasks ();

        BENTLEYDLL_EXPORT static void PushTaskToDefaultSheduler(std::shared_ptr<AsyncTask> task);

    public:
        BENTLEYDLL_EXPORT AsyncTask ();

        BENTLEYDLL_EXPORT virtual ~AsyncTask ();

        BENTLEYDLL_EXPORT void AddSubTask (std::shared_ptr<AsyncTask> task);

        //! Create task that will complete once all task in container are completed.
        //! @param tasks - contains std::shared_ptr<AsyncTask> or derived type tasks
        template<typename C>
        static std::shared_ptr<PackagedAsyncTask<void>> WhenAll(C tasks)
            {
            auto whenAll = std::make_shared<PackagedAsyncTask<void>>([]{});
            for (auto task : tasks)
                whenAll->AddSubTask(task);

            // To avoid race condition when adding task might end just before we add it
            // iterate trought all tasks and remove completed.
            for (auto task : tasks)
                if (task->IsCompleted())
                    whenAll->RemoveSubTask(task);

            PushTaskToDefaultSheduler(whenAll);
            return whenAll;
            }

        //! Check if task has completed its execution and has no more sub tasks left
        BENTLEYDLL_EXPORT bool IsCompleted () const;

        BENTLEYDLL_EXPORT Priority GetPriority () const;

        //! Block current thread and wait for task to be completed.
        //! WARNING - not recommended unless your design requires synchronizing async code execution to one thread.
        //! WARNING - might cause dead lock if used inproperly. Example - waiting in thread that also would be used to complete same task.
        //! WARNING - never use within Default sheduler threads - will produce assert. Redesign to async code or use custom thread for wait.
        BENTLEYDLL_EXPORT void Wait ();

        //! Waits for specified amount of time for task completion. See Wait()
        BENTLEYDLL_EXPORT void WaitFor (int milliseconds);

        BENTLEYDLL_EXPORT void Execute ();

        //! Enable adding caller stack information into each task for debugging.
        //! Useful when debugging deadlocks.
        //! This will only work on DEBUG builds.
        BENTLEYDLL_EXPORT static void SetStackInfoEnabled(bool enabled);

        //! Get caller stack frame info for debugging if available.
        BENTLEYDLL_EXPORT BeDebugUtilities::StackFrameInfo* GetStackInfo() const;

        //! Internal. Used to set caller stack information for task.
        BENTLEYDLL_EXPORT void SetStackInfo(size_t frameIndex);
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <class T, class P> struct PackagedThenAsyncTask;

template <class T>
struct PackagedAsyncTask : AsyncTask
    {
    private:
        std::function<T (void)> m_taskCallback;

    protected:
        T m_result;

    public:
        PackagedAsyncTask (const std::function<T (void)>& taskCallback) : AsyncTask (), m_taskCallback (taskCallback), m_result ()
            {
            }

        virtual void _OnExecute ()
            {
            m_result = m_taskCallback ();
            }

        //! Blocks until result is available. See Wait()
        T& GetResult ()
            {
            Wait (); return m_result;
            }

        template <class R>
        //! Execute new task with callback code in default thread after this task is finished.
        std::shared_ptr<PackagedThenAsyncTask<R, T>> Then (const std::function<R (T&)>& taskCallback)
            {
            auto task = std::make_shared<PackagedThenAsyncTask<R, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result));
            task->SetStackInfo(1);
            AddThenTask (task);
            return task;
            }

        template <class R>
        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedThenAsyncTask<R, T>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<R (T&)>& taskCallback)
            {
            auto task = std::make_shared<PackagedThenAsyncTask<R, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result));
            task->SetStackInfo(1);
            AddThenTask (task, scheduler);
            return task;
            }

        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedThenAsyncTask<void, T>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<void (T&)>& taskCallback)
            {
            auto task = std::make_shared<PackagedThenAsyncTask<void, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result));
            task->SetStackInfo(1);
            AddThenTask (task, scheduler);
            return task;
            }

        //! Execute new task with callback code in default thread after this task is finished.
        std::shared_ptr<PackagedThenAsyncTask<void, T>> Then (const std::function<void (T&)>& taskCallback)
            {
            auto task = std::make_shared<PackagedThenAsyncTask<void, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result));
            task->SetStackInfo(1);
            AddThenTask (task);
            return task;
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <>
struct PackagedAsyncTask<void> : AsyncTask
    {
    protected:
        std::function<void (void)> m_taskCallback;

    public:
        PackagedAsyncTask (const std::function<void (void)>& taskCallback) : AsyncTask (), m_taskCallback (taskCallback)
            {
            }

        virtual void _OnExecute ()
            {
            m_taskCallback ();
            }

        template <class R>
        //! Execute new task with callback code in default thread after this task is finished.
        std::shared_ptr<PackagedAsyncTask<R>> Then (const std::function<R (void)>& taskCallback)
            {
            auto task = std::make_shared<PackagedAsyncTask<R>> (taskCallback);
            task->SetStackInfo(1);
            AddThenTask (task);
            return task;
            }

        template <class R>
        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedAsyncTask<R>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<R (void)>& taskCallback)
            {
            auto task = std::make_shared<PackagedAsyncTask<R>> (taskCallback);
            task->SetStackInfo(1);
            AddThenTask (task, scheduler);
            return task;
            }

        //! Execute new task with callback code in default thread after this task is finished.
        std::shared_ptr<PackagedAsyncTask<void>> Then (const std::function<void (void)>& taskCallback)
            {
            auto task = std::make_shared<PackagedAsyncTask<void>> (taskCallback);
            task->SetStackInfo(1);
            AddThenTask (task);
            return task;
            }

        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedAsyncTask<void>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<void (void)>& taskCallback)
            {
            auto task = std::make_shared<PackagedAsyncTask<void>> (taskCallback);
            task->SetStackInfo(1);
            AddThenTask (task, scheduler);
            return task;
            }
    };

template <class T>
using AsyncTaskPtr = std::shared_ptr<PackagedAsyncTask<T>>;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <class T, class P>
struct PackagedThenAsyncTask : PackagedAsyncTask<T>
    {
    private:
        std::shared_ptr<P> m_parentResult;

        std::function<T (P&)> m_taskCallback;

    public:
        PackagedThenAsyncTask (const std::function<T (P&)>& taskCallback, std::shared_ptr<P> parentResult)
            : PackagedAsyncTask<T> (nullptr),
                m_taskCallback (taskCallback),
                m_parentResult (parentResult)
            {
                }

        virtual void _OnExecute ()
            {
            this->m_result = this->m_taskCallback (*m_parentResult.get ());
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <class P>
struct PackagedThenAsyncTask<void, P> : PackagedAsyncTask<void>
    {
    private:
        std::shared_ptr<P> m_parentResult;

        std::function<void (P&)> m_taskCallback;

    public:
        PackagedThenAsyncTask (const std::function<void (P&)>& taskCallback, std::shared_ptr<P> parentResult)
            : PackagedAsyncTask<void> (nullptr),
                m_taskCallback (taskCallback),
                m_parentResult (parentResult)
            {
            }

        virtual void _OnExecute ()
            {
            this->m_taskCallback (*m_parentResult.get ());
            }
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
//! Create task with return value that is flagged as completed without executing it.
AsyncTaskPtr<T> CreateCompletedAsyncTask (const T& result)
    {
    auto task = std::make_shared<PackagedAsyncTask<T>> ([=]
        {
        return result;
        });
    task->Execute ();
    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
//! Create task that is flagged as completed without executing it.
BENTLEYDLL_EXPORT AsyncTaskPtr<void> CreateCompletedAsyncTask ();

END_BENTLEY_TASKS_NAMESPACE
