/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/TaskScheduler.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <Bentley/Tasks/TaskScheduler.h>
#include <Bentley/Tasks/AsyncTask.h>

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TaskScheduler::TaskScheduler()
    {
    ResetOnEmptyTask ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
TaskScheduler::~TaskScheduler()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                        
+---------------+---------------+---------------+---------------+---------------+------*/
void TaskScheduler::ResetOnEmptyTask ()
    {
    m_onEmptyTask = std::make_shared<PackagedAsyncTask<void>> ([=] {});
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void TaskScheduler::Push (std::shared_ptr<AsyncTask> task, AsyncTask::Priority priority)
    {
    Push (task, AsyncTasksManager::GetCurrentThreadAsyncTask(), priority);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TaskScheduler::Push (std::shared_ptr<AsyncTask> task, std::shared_ptr<AsyncTask> parentTask, AsyncTask::Priority priority)
    {
    if (parentTask)
        parentTask->AddSubTask(task);

    if (priority == AsyncTask::Priority::Inherited)
        {
        if (parentTask)
            priority = parentTask->GetPriority();
        else
            priority = AsyncTask::Priority::Normal;
        }

    task->SetPriority(priority);
    m_taskQueue.Push(task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void TaskScheduler::RegisterTaskStart (std::shared_ptr<AsyncTask> task)
    {
    task->RegisterOnCompletedListener (shared_from_this());
    m_startedTasks.insert (task);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AsyncTask> TaskScheduler::WaitAndPop ()
    {
    BeMutexHolder lk (m_taskQueue.GetMutex());
    
    std::shared_ptr<AsyncTask> task = nullptr;
    m_taskQueue.ProtectedWaitAndPop(task, lk);
    
    RegisterTaskStart (task);
        
    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     04/2014
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AsyncTask> TaskScheduler::TryPop ()
    {
    BeMutexHolder lock (m_taskQueue.GetMutex ());
    
    std::shared_ptr<AsyncTask> task = nullptr;
    if (!m_taskQueue.ProtectedTryPop(task))
        {
        return nullptr;
        }
    
    RegisterTaskStart (task);
        
    return task;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void TaskScheduler::_OnAsyncTaskCompleted (std::shared_ptr<AsyncTask> task)
    {
    task->UnregisterOnCompletedListener(shared_from_this());
    
    BeMutexHolder lk (m_taskQueue.GetMutex ());
    m_startedTasks.erase (task);

    if (!ProtectedHasRunningTasks ())
        {
        m_onEmptyTask->Execute ();

        ResetOnEmptyTask ();

        lk.unlock ();

        _OnEmpty ();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                   
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskPtr<void> TaskScheduler::OnEmpty () const
    {
    BeMutexHolder lock (m_taskQueue.GetMutex ());
    if (!ProtectedHasRunningTasks ())
        {
        return CreateCompletedAsyncTask ();
        }

    return m_onEmptyTask;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool TaskScheduler::ProtectedHasRunningTasks () const
    {
    return !m_taskQueue.ProtectedEmpty () || !m_startedTasks.empty ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool TaskScheduler::HasRunningTasks() const
    {
    BeMutexHolder lock (m_taskQueue.GetMutex ());
    return ProtectedHasRunningTasks();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int TaskScheduler::GetQueueTaskCount() const
    {
    return m_taskQueue.Size();
    }
    
