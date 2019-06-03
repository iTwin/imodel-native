/*--------------------------------------------------------------------------------------+
 |
 |  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/Tasks/Tasks.h>
#include <Bentley/bset.h>
#include <functional>
#include <Bentley/BeDebugUtilities.h>
#include <Bentley/BeThread.h>
#include <set>

BEGIN_BENTLEY_TASKS_NAMESPACE

struct ITaskScheduler;

template <class T> struct PackagedAsyncTask;


//=======================================================================================
// @bsiclass                                              Benediktas.Lipnickas   10/2013
//=======================================================================================
struct OnAsyncTaskCompletedListener
{
friend struct AsyncTask;
protected:
    virtual void _OnAsyncTaskCompleted (std::shared_ptr<struct AsyncTask> task) = 0;
};

//=======================================================================================
//! Base class for all AsyncTask variations depending on return and receive values.
//! Shorthand type to use in most cases - AsyncTaskPtr<void>
// @bsiclass                                              Benediktas.Lipnickas   10/2013
//=======================================================================================
/**
@section SECTION_BentleyTasks_GettingStarted Getting started

Async programming is useful when you need to execute long running tasks and continue your main thread code running without blocking and waiting for
them to finish. Mostly used when network operations are involved.
Always use Then() to async tasks, this way any long running code will not block executing thread and it can continue work. Result handling code in
Then() will be executed asynchronously when work is done. Blocking Wait() and GetResult() can also be used, but
should be treated with care as they could stale your program or cause deadlocks.

@section SECTION_BentleyTasks_Samples Code samples

Then() hander execution is done in undefined thread, but you can your code execution thread by passing specific thread as first parameter:
\code{.cpp}
auto myThread = WorkerThread::Create("My Thread");
Foo()->Then(myThread, [=]
    {
    // Code executed in "My Thread" thread
    });
\endcode

Simple async function:
\code{.cpp}
AsyncTaskPtr<int> GetCount()
    {
    return WorkerThread::Create("My Thread")->ExecuteAsync<int>([=]
        {
        // ...
        return count;
        });
    }
\endcode

Define result type to be able to return both success and error data from async functions
\code{.cpp}
typedef AsyncResult<Utf8String, AsyncError> NameResult;
AsyncTaskPtr<NameResult> GetName()
    {
    return m_thread->ExecuteAsync<NameResult>([=]
        {
        // ...
        if (!success)
            return NameResult::Error("Could not connect to server to get name");
        return NameResulr::Success(name);
        });
    }
\endcode

Getting value from async call (using AsyncResult template):
\code{.cpp}
void Foo()
    {
    GetName()->Then([=] (NameResult result)
        {
        if (!result.IsSuccess())
            {
            // handle error
            return;
            }
        ShowNameInUI(result.GetValue());
        });
    }
\endcode

Handling chain of async calls and returning result value afer all are done:
\code{.cpp}

AsyncTaskPtr<Result> Foo()
    {
    return Boo()->Then([=]
        {
        // Task 1
        })
    ->Then([=]
        {
        // Task 2 - task 1 then-task
        })
     ->Then<Result>([=]
        {
        // Task 3 - task 2 then-task
        return Result::Success(42);
        });
    }
\endcode

Handling multiple async calls within async calls and still returning result value:
\code{.cpp}
AsyncTaskPtr<Result> Foo()
    {
    // Additional result variable is needed to carry result from sub-tasks.
    auto finalResult = std::make_shared<Result>();
    return Boo()->Then([=]
        {
        // Task 1
        Blah()->Then([=]
            {
            // Task 2 - task 1 sub-task
            finalResult->SetSuccess(42);
            });
        })
    ->Then<Result>([=]
        {
        // Task 3 - task 1 then-task
        // Will be executed after previous task and sub-tasks are done (Task 1 and Task 2)
        return *finalResult;
        });
 }
\endcode

Looping list of data and operating on it asynchronously pseudocode:
\code{.cpp}
void Sum(int* result, list<int>* list)
    {
    if (list->empty()) 
    return;
    return m_thread->Execute([=]
        {
        *result += list->at(0)
        })
    ->Then([=]
        {
        list->pop();
        Sum(result, list);
        });
 }
\endcode
*/
//=======================================================================================
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
        static BeMutex s_activeTasksMutex;
        static std::set<AsyncTask*> s_activeTasks;

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

        static Utf8String GetTaskDescription(AsyncTask& task, ITaskScheduler* scheduler, int padding);
        static bool IsTaskReferencedInSubTasks(AsyncTask& refTask, AsyncTask& inTask);
        static std::set<AsyncTask*> FilterTasks(std::set<AsyncTask*> tasks);

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
        //! Create async task. Usually derived class is used for this.
        //! @param stackDepth - used to control what stack frame info is captured for debugging when SetStackInfoEnabled(true) is called.
        //! 0 will capture function that calls AsyncTask constructor, 1 will capture funciton above that, and so on.
        BENTLEYDLL_EXPORT AsyncTask (size_t stackDepth = 0);

        BENTLEYDLL_EXPORT virtual ~AsyncTask ();

        BENTLEYDLL_EXPORT void AddSubTask (std::shared_ptr<AsyncTask> task);

        //! Create task that will complete once all task in container are completed.
        //! @param tasks - contains std::shared_ptr<AsyncTask> or derived type tasks
        template<typename C>
        static std::shared_ptr<PackagedAsyncTask<void>> WhenAll(C tasks)
            {
            auto whenAll = std::make_shared<PackagedAsyncTask<void>>([]{}, 1);
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

        //! Get caller stack frame info for debugging if available. Needs to be enabled with SetStackInfoEnabled().
        BENTLEYDLL_EXPORT BeDebugUtilities::StackFrameInfo* GetStackInfo() const;

        //! Internal. Used to set caller stack information for task.
        BENTLEYDLL_EXPORT void SetStackInfo(size_t frameIndex);

        //! Internal. Get debug summary for all currently active tasks. Needs to be enabled with SetStackInfoEnabled().
        BENTLEYDLL_EXPORT static Utf8String DebugActiveTasks();
    };
    
//=======================================================================================
// @bsiclass                                              Benediktas.Lipnickas   10/2013
//=======================================================================================
template <class T, class P> struct PackagedThenAsyncTask;

template <class T>
struct PackagedAsyncTask : AsyncTask
    {
    private:
        std::function<T (void)> m_taskCallback;

    protected:
        T m_result;

    public:
        PackagedAsyncTask (const std::function<T (void)>& taskCallback, size_t stackDepth = 0) : AsyncTask (++stackDepth), m_taskCallback (taskCallback), m_result ()
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
            auto task = std::make_shared<PackagedThenAsyncTask<R, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result), 1);
            AddThenTask (task);
            return task;
            }

        template <class R>
        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedThenAsyncTask<R, T>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<R (T&)>& taskCallback)
            {
            auto task = std::make_shared<PackagedThenAsyncTask<R, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result), 1);
            AddThenTask (task, scheduler);
            return task;
            }

        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedThenAsyncTask<void, T>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<void (T&)>& taskCallback)
            {
            auto task = std::make_shared<PackagedThenAsyncTask<void, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result), 1);
            AddThenTask (task, scheduler);
            return task;
            }

        //! Execute new task with callback code in default thread after this task is finished.
        std::shared_ptr<PackagedThenAsyncTask<void, T>> Then (const std::function<void (T&)>& taskCallback)
            {
            auto task = std::make_shared<PackagedThenAsyncTask<void, T>> (taskCallback, std::shared_ptr <T> (shared_from_this (), &m_result), 1);
            AddThenTask (task);
            return task;
            }
    };

//=======================================================================================
// @bsiclass                                              Benediktas.Lipnickas   10/2013
//=======================================================================================
template <>
struct PackagedAsyncTask<void> : AsyncTask
    {
    protected:
        std::function<void (void)> m_taskCallback;

    public:
        PackagedAsyncTask (const std::function<void (void)>& taskCallback, size_t stackDepth = 0) : AsyncTask (++stackDepth), m_taskCallback (taskCallback)
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
            auto task = std::make_shared<PackagedAsyncTask<R>> (taskCallback, 1);
            AddThenTask (task);
            return task;
            }

        template <class R>
        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedAsyncTask<R>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<R (void)>& taskCallback)
            {
            auto task = std::make_shared<PackagedAsyncTask<R>> (taskCallback, 1);
            AddThenTask (task, scheduler);
            return task;
            }

        //! Execute new task with callback code in default thread after this task is finished.
        std::shared_ptr<PackagedAsyncTask<void>> Then (const std::function<void (void)>& taskCallback)
            {
            auto task = std::make_shared<PackagedAsyncTask<void>> (taskCallback, 1);
            AddThenTask (task);
            return task;
            }

        //! Execute new task with callback code in sheduler/thread/pool after this task is finished.
        std::shared_ptr<PackagedAsyncTask<void>> Then (std::shared_ptr<ITaskScheduler> scheduler, const std::function<void (void)>& taskCallback)
            {
            auto task = std::make_shared<PackagedAsyncTask<void>> (taskCallback, 1);
            AddThenTask (task, scheduler);
            return task;
            }
    };

template <class T>
using AsyncTaskPtr = std::shared_ptr<PackagedAsyncTask<T>>;

//=======================================================================================
// @bsiclass                                              Benediktas.Lipnickas   10/2013
//=======================================================================================
template <class T, class P>
struct PackagedThenAsyncTask : PackagedAsyncTask<T>
    {
    private:
        std::shared_ptr<P> m_parentResult;

        std::function<T (P&)> m_taskCallback;

    public:
        PackagedThenAsyncTask (const std::function<T (P&)>& taskCallback, std::shared_ptr<P> parentResult, size_t stackDepth = 0)
            : PackagedAsyncTask<T> (nullptr, ++stackDepth),
                m_taskCallback (taskCallback),
                m_parentResult (parentResult)
            {
            }

        virtual void _OnExecute ()
            {
            this->m_result = this->m_taskCallback (*m_parentResult.get ());
            }
    };

//=======================================================================================
// @bsiclass                                              Benediktas.Lipnickas   10/2013
//=======================================================================================
template <class P>
struct PackagedThenAsyncTask<void, P> : PackagedAsyncTask<void>
    {
    private:
        std::shared_ptr<P> m_parentResult;

        std::function<void (P&)> m_taskCallback;

    public:
        PackagedThenAsyncTask (const std::function<void (P&)>& taskCallback, std::shared_ptr<P> parentResult, size_t stackDepth = 0)
            : PackagedAsyncTask<void> (nullptr, ++stackDepth),
                m_taskCallback (taskCallback),
                m_parentResult (parentResult)
            {
            }

        virtual void _OnExecute ()
            {
            this->m_taskCallback (*m_parentResult.get ());
            }
    };

//=======================================================================================
// @bsiclass
//=======================================================================================
template <typename T>
//! Create task with return value that is flagged as completed without executing it.
AsyncTaskPtr<T> CreateCompletedAsyncTask (const T& result)
    {
    auto task = std::make_shared<PackagedAsyncTask<T>> ([=]
        {
        return result;
        }, 1);
    task->Execute ();
    return task;
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
//! Create task that is flagged as completed without executing it.
BENTLEYDLL_EXPORT AsyncTaskPtr<void> CreateCompletedAsyncTask ();

END_BENTLEY_TASKS_NAMESPACE
