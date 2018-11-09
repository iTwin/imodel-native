/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Tasks/AsyncTaskRunner.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include <Bentley/Tasks/AsyncTaskRunner.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <Bentley/Tasks/TaskScheduler.h>
#include "ThreadingLogging.h"

USING_NAMESPACE_BENTLEY_TASKS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskRunner::AsyncTaskRunner () 
    : m_isRunning (false), m_isStopping (false)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AsyncTaskRunner::~AsyncTaskRunner ()
    {

    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                 
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunner::Start (std::shared_ptr<ITaskScheduler> scheduler, Utf8CP name)
    {
    m_scheduler = scheduler;
    m_name = name;

    std::thread t (&AsyncTaskRunner::_Run, this);
    t.detach ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunner::Stop ()
    {
    m_isStopping.store(true);

    m_thisPtr = shared_from_this ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                      
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunner::WakeUp()
    {
    if (m_schedulerToHoldWhileStopping = m_scheduler.lock())
        {
        m_schedulerToHoldWhileStopping->Push(std::make_shared<AsyncTask>()); // Push dummy task to wake up thread runner.
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncTaskRunner::IsRunning () const
    {
    return m_isRunning;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool AsyncTaskRunner::IsStopping () const
    {
    return m_isStopping;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunner::_Run ()
    {
    m_isRunning.store(true);

    BeThreadUtilities::SetCurrentThreadName (m_name.c_str ());

    LOG.infov ("Thread started: %s", m_name.c_str ());
    AsyncTasksManager::RegisterAsyncTaskRunner (shared_from_this());
    m_id = BeThreadUtilities::GetCurrentThreadId ();

    _RunAsyncTasksLoop ();

    LOG.infov("Thread exiting: %s", m_name.c_str());
    m_isRunning.store(false);
    m_schedulerToHoldWhileStopping = nullptr;

    AsyncTasksManager::UnregisterCurrentThreadAsyncTaskRunner ();
    m_thisPtr = nullptr;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void AsyncTaskRunner::_RunAsyncTasksLoop ()
    {
    while (true)
        {
        std::shared_ptr<AsyncTask> task = GetTaskScheduler ()->WaitAndPop ();

        m_currentRunningTask = task;
        task->Execute ();
        m_currentRunningTask = nullptr;

        if (m_isStopping)
            {
            break;
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<ITaskScheduler> AsyncTaskRunner::GetTaskScheduler ()
    {
    return m_scheduler.lock();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
std::shared_ptr<AsyncTask> AsyncTaskRunner::GetCurrentRunningTask ()
    {
    return m_currentRunningTask;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                            Benediktas.Lipnickas     10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t AsyncTaskRunner::GetId () const
    {
    return m_id;
    }
