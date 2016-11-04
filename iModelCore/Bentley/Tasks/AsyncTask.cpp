/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/AsyncTask.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/TaskScheduler.h>

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTask::AsyncTask () 
    : m_executed (false), m_completed (false), m_priority (Priority::Normal)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTask::~AsyncTask ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTask::Priority AsyncTask::GetPriority () const
    {
    return m_priority;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     03/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::SetPriority (Priority priority)
    {
    m_priority = priority;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::RegisterOnCompletedListener (std::shared_ptr<OnAsyncTaskCompletedListener> listener)
    {
    BeMutexHolder lock (m_completedCV.GetMutex());
    m_onCompletedListeners.insert (listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::UnregisterOnCompletedListener (std::shared_ptr<OnAsyncTaskCompletedListener> listener)
    {
    BeMutexHolder lock (m_completedCV.GetMutex());
    m_onCompletedListeners.erase (listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::Execute ()
    {
    _OnExecute ();

    BeMutexHolder lock (m_completedCV.GetMutex());

    m_executed = true;

    CheckCompletion (lock);

    if (IsCompleted ())
        {
        ProcessTaskCompletion ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::_OnExecute ()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::CleanUpTask (std::shared_ptr<AsyncTask> task, bset<std::shared_ptr<AsyncTask>>& tasksToCleanUp)
    {
    if (!task->IsCompleted ())
        {
        return;
        }

    for (auto parentTask : task->GetParentTasks ())
        {
        tasksToCleanUp.insert (parentTask);

        parentTask->RemoveSubTask (task);
        }

    task->NotifyOnCompletedListeners ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                           
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::ProcessTaskCompletion ()
    {
    bset<std::shared_ptr<AsyncTask>> tasksToCleanUp;

    tasksToCleanUp.insert (shared_from_this ());

    while (!tasksToCleanUp.empty ())
        {
        auto taskToCleanUp = *tasksToCleanUp.begin ();
        tasksToCleanUp.erase (taskToCleanUp);

        CleanUpTask (taskToCleanUp, tasksToCleanUp);
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::AddParentTask (std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder lock (m_completedCV.GetMutex());

    m_parentTasks.insert (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::RemoveParentTask (std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder lock (m_completedCV.GetMutex());

    m_parentTasks.erase (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bset<std::shared_ptr<AsyncTask>> AsyncTask::GetParentTasks ()
    {
    BeMutexHolder lock (m_completedCV.GetMutex());

    return m_parentTasks;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::AddSubTaskNoLock (std::shared_ptr<AsyncTask> task)
    {
    task->AddParentTask (shared_from_this ());

    m_subTasks.insert (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::AddSubTask (std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder lock (m_completedCV.GetMutex());

    AddSubTaskNoLock (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::RemoveSubTask (std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder lock (m_completedCV.GetMutex());

    task->RemoveParentTask (shared_from_this ());

    m_subTasks.erase (task);

    CheckCompletion (lock);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::CheckCompletion (BeMutexHolder& lock)
    {
    if (!m_executed)
        {
        return;
        }

    if (!m_subTasks.empty ())
        {
        return;
        }

    OnCompleted (lock);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::OnCompleted (BeMutexHolder& lock)
    {
    if (!m_thenTasks.empty ())
        {
        auto thenTasksCopy (m_thenTasks);
        m_thenTasks.clear ();

        for (auto taskSchedulerPair : thenTasksCopy)
            {
            AddSubTaskNoLock (taskSchedulerPair.first);
            }

        lock.unlock ();

        for (auto taskSchedulerPair : thenTasksCopy)
            {
            taskSchedulerPair.second->Push (taskSchedulerPair.first, nullptr);
            }
        }
    else
        {
        m_completed.store(true);

        lock.unlock ();

        m_completedCV.notify_all ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::NotifyOnCompletedListeners ()
    {
    if (!IsCompleted ())
        {
        return;
        }

    BeMutexHolder lock (m_completedCV.GetMutex());

    bvector<std::shared_ptr<OnAsyncTaskCompletedListener>> aliveListeners;

    // To avoid calling listeners while holding lock, make a copy of listeners.
    // While doing that remove dead weak pointers.
    m_onCompletedListeners.erase (std::remove_if (m_onCompletedListeners.begin (), m_onCompletedListeners.end (),
        [&] (const std::weak_ptr<OnAsyncTaskCompletedListener>& item)
        {
        std::shared_ptr<OnAsyncTaskCompletedListener> ptr = item.lock ();
        if (ptr)
            {
            aliveListeners.push_back (ptr);
            return false;
            }
        else
            {
            return true;
            }
        }
    ), m_onCompletedListeners.end ());

    lock.unlock ();

    for (auto listener : aliveListeners)
        {
        listener->_OnAsyncTaskCompleted (shared_from_this ());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncTask::IsCompleted () const
    {
    return m_completed;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::AddThenTask (std::shared_ptr<AsyncTask> task, std::shared_ptr<ITaskScheduler> scheduler)
    {
    BeMutexHolder lock (m_completedCV.GetMutex());

    if (scheduler == nullptr)
        {
        scheduler = AsyncTasksManager::GetDefaultScheduler ();
        }

    if (m_completed)
        {
        lock.unlock ();
        scheduler->Push (task);
        }
    else
        {
        m_thenTasks.push_back (make_bpair(task, scheduler));
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                              Pranciskus.Ambrazas    03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
struct IsAsyncTaskCompletedPredicate : IConditionVariablePredicate
    {
private:
    AsyncTask const& m_task;
public:
    IsAsyncTaskCompletedPredicate(AsyncTask const& task)
        : m_task(task) {}
    virtual bool  _TestCondition(BeConditionVariable &cv) override { return m_task.IsCompleted(); }
    };

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::Wait ()
    {
    IsAsyncTaskCompletedPredicate predicate(*this);
    m_completedCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::WaitFor (int milliseconds) 
    {
    IsAsyncTaskCompletedPredicate predicate(*this);
    m_completedCV.WaitOnCondition(&predicate, milliseconds);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::PushTaskToDefaultSheduler(std::shared_ptr<AsyncTask> task)
    {
    AsyncTasksManager::GetDefaultScheduler()->Push(task);
    }

BEGIN_BENTLEY_TASKS_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> CreateCompletedAsyncTask ()
    {
    auto task = std::make_shared<PackagedAsyncTask<void>> ([=]{});
    task->Execute ();
    return task;
    }

END_BENTLEY_TASKS_NAMESPACE
