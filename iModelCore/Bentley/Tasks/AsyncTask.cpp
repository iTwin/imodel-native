/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/AsyncTasksManager.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include "ThreadingLogging.h"
#include <set>

USING_NAMESPACE_BENTLEY_TASKS

bool AsyncTask::s_stackInfoEnabled = false;

BeMutex AsyncTask::s_activeTasksMutex;
std::set<AsyncTask*> AsyncTask::s_activeTasks;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTask::AsyncTask (size_t stackDepth) 
    : m_executed (false), m_completed (false), m_priority (Priority::Normal)
    {
    if (s_stackInfoEnabled)
        {    
        BeMutexHolder lock(s_activeTasksMutex);
        s_activeTasks.insert(this);
        }

    // Skip std::make_shared that takes up 2 frames (Win x64, VS2017) and is usually used to construct AsyncTask
    stackDepth += 2;
    SetStackInfo(++stackDepth); 
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTask::~AsyncTask ()
    {
    if (s_stackInfoEnabled)
        {
        BeMutexHolder lock(s_activeTasksMutex);
        s_activeTasks.erase(this);
        }

    if (m_stackInfo)
        delete m_stackInfo;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AsyncTask::GetTaskDescription(AsyncTask& task, ITaskScheduler* scheduler, int padding)
    {
    Utf8String stackInfo;
    if (task.m_stackInfo)
        {
        stackInfo.Sprintf("%s %s:(%d)",
                          task.m_stackInfo->functionName.c_str(),
                          task.m_stackInfo->fileName.c_str(),
                          task.m_stackInfo->fileLine);
        }

    Utf8String paddingStr(padding * 4, ' ');
    Utf8String paddingStrDouble((padding + 1) * 4, ' ');

    Utf8String msg;

    msg += Utf8PrintfString("%s[%p] %s %s (%s_%s)%s (%s)\n",
                            paddingStr.c_str(), &task,
                            task.m_completed ? "Completed" : "NotCompleted",
                            task.m_executed ? "Executed" : "NotExecuted",
                            task.m_completed ? "C" : "NC",
                            task.m_executed ? "E" : "NE",
                            scheduler ? Utf8PrintfString(" Scheduler:[%p]", scheduler).c_str() : "",
                            stackInfo.c_str());

    if (!task.m_parentTasks.empty())
        {
        msg += paddingStrDouble + "Parents (tasks in which this was created):\n";
        for (auto otherTask : task.m_parentTasks)
            msg += Utf8PrintfString("%s[%p]\n", paddingStrDouble.c_str(), &*otherTask);
        }

    if (!task.m_subTasks.empty())
        {
        msg += paddingStrDouble + "SubTasks (tasks that block completion):\n";
        for (auto otherTask : task.m_subTasks)
            msg += GetTaskDescription(*otherTask, nullptr, padding + 1);
        }

    if (!task.m_thenTasks.empty())
        {
        msg += paddingStrDouble + "ThenTasks (chained tasks):\n";
        for (auto otherTask : task.m_thenTasks)
            msg += GetTaskDescription(*otherTask.first, &*otherTask.second, padding + 1);
        }

    return msg;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncTask::IsTaskReferencedInSubTasks(AsyncTask& refTask, AsyncTask& inTask)
    {
    for (auto task : inTask.m_subTasks)
        {
        if (&refTask == &*task)
            return true;
        if (IsTaskReferencedInSubTasks(refTask, *task))
            return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::set<AsyncTask*> AsyncTask::FilterTasks(std::set<AsyncTask*> tasks)
    {
    for (auto it = tasks.begin(); it != tasks.end();)
        {
        bool isReferenced = false;
        for (auto task : tasks)
            {
            if (&*task == &**it)
                continue;

            if (!IsTaskReferencedInSubTasks(**it, *task))
                continue;

            isReferenced = true;
            break;
            }

        if (isReferenced)
            it = tasks.erase(it);
        else
            it++;
        }
    return tasks;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AsyncTask::DebugActiveTasks()
    {
    BeMutexHolder lock(s_activeTasksMutex);
    Utf8String msg;

    if (!s_stackInfoEnabled)
        {
        msg = "Debugging is disabled.";
        return msg;
        }

    msg += "--------------------------- All tasks in hierarchy: ----------------------------\n";

    for (auto task : FilterTasks(s_activeTasks))
        msg += GetTaskDescription(*task, nullptr, 0);

    msg += "---------------------------------- All tasks: ----------------------------------\n";

    for (auto task : s_activeTasks)
        msg += Utf8PrintfString("[%p]\n", &*task);

    msg += "------------------------------------- End --------------------------------------\n";

    return msg;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::SetStackInfoEnabled(bool enabled)
    {
#ifdef DEBUG
    s_stackInfoEnabled = enabled;
    if (s_stackInfoEnabled)
        LOG.warning("AsyncTask stack info is enabled, this may cause runtime performance hit.");
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::SetStackInfo(size_t frameIndex)
    {
#ifdef DEBUG
    if (!s_stackInfoEnabled)
        return;

    if (m_stackInfo)
        return;

    frameIndex += 1; // Skip this function

    auto info = BeDebugUtilities::GetStackFrameInfoAt(frameIndex);
    m_stackInfo = new BeDebugUtilities::StackFrameInfo(std::move(info));
#endif
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeDebugUtilities::StackFrameInfo* AsyncTask::GetStackInfo() const
    {
    return m_stackInfo;
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
    BeMutexHolder lock (m_mutex);
    m_onCompletedListeners.insert (listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::UnregisterOnCompletedListener (std::shared_ptr<OnAsyncTaskCompletedListener> listener)
    {
    BeMutexHolder lock (m_mutex);
    m_onCompletedListeners.erase (listener);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::Execute ()
    {
    _OnExecute ();

    BeMutexHolder lock (m_mutex);

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
    BeMutexHolder lock (m_mutex);

    AddParentTaskNoLock(task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::AddParentTaskNoLock (std::shared_ptr<AsyncTask> task)
    {
    m_parentTasks.insert (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::RemoveParentTask (std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder lock (m_mutex);

    m_parentTasks.erase (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bset<std::shared_ptr<AsyncTask>> AsyncTask::GetParentTasks ()
    {
    BeMutexHolder lock (m_mutex);

    return m_parentTasks;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::AddSubTaskNoLock (std::shared_ptr<AsyncTask> task)
    {
    m_subTasks.insert (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                             Benediktas.Lipnickas   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::AddSubTask (std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder childLock(task->m_mutex);
    if (task->IsCompleted())
        return;

    BeMutexHolder lock(m_mutex);

    AddSubTaskNoLock (task);
    task->AddParentTaskNoLock(shared_from_this());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::RemoveSubTask (std::shared_ptr<AsyncTask> task)
    {
    BeMutexHolder lock (m_mutex);

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
            auto childTask = taskSchedulerPair.first;
            AddSubTaskNoLock(childTask);
            childTask->AddParentTask(shared_from_this());
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

    BeMutexHolder lock (m_mutex);

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
    BeMutexHolder lock (m_mutex);

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
    BeMutexHolder holder(m_mutex);
    IsAsyncTaskCompletedPredicate predicate(*this);
    m_completedCV.ProtectedWaitOnCondition(holder, &predicate, BeConditionVariable::Infinite);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTask::WaitFor (int milliseconds)
    {
    BeMutexHolder holder(m_mutex);
    IsAsyncTaskCompletedPredicate predicate(*this);
    m_completedCV.ProtectedWaitOnCondition(holder, &predicate, milliseconds);
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
    auto task = std::make_shared<PackagedAsyncTask<void>> ([=]{}, 1);
    task->Execute();
    return task;
    }

END_BENTLEY_TASKS_NAMESPACE
