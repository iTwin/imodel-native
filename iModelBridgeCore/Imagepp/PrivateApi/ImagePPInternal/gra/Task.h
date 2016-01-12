//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PrivateApi/ImagePPInternal/gra/Task.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>

BEGIN_IMAGEPP_NAMESPACE 

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  1/2016
//----------------------------------------------------------------------------------------
struct WorkerPool
    {
public:
    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  1/2016
    //---------------------------------------------------------------------------------------
    struct Worker
        {
        public:
            Worker(WorkerPool &p) : m_pool(p) { }
            void operator()();
        private:
            WorkerPool &m_pool;
        };

    //----------------------------------------------------------------------------------------
    // @bsimethod                                                   Mathieu.Marchand  1/2016
    //---------------------------------------------------------------------------------------
    struct Task : RefCountedBase
        {
        public:
            Task(){m_isFinished=false;}
            virtual ~Task(){};
        
            void OnFinish()
                {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_isFinished = true;
                m_cv.notify_all();
                }

            void Wait()
                {
                std::unique_lock<std::mutex> lk(m_mutex);
                while (!m_isFinished)
                    m_cv.wait(lk);
                }

            virtual void _Run() = 0;

            bool IsCanceled() const {return GetRefCount() == 1;}    // if the pool is the only owner that means no one is interested on the result.
                   
        protected:
            std::mutex                  m_mutex;
            std::condition_variable     m_cv;
            bool                        m_isFinished;     
        };

    typedef RefCountedPtr<Task> TaskPtr;

    WorkerPool(size_t nbWorker);
     ~WorkerPool();
    
    //! Add a task to the end of the queue. Optionally(use with parsimony), add a task to the front of the queue. 
    //! Caller must retain a refCount of the task object otherwise it is going to be threated as canceled and won't be executed.
    void Enqueue(Task& task, bool atFront = false);
    
private:
    
    // need to keep track of threads so we can join them
    std::vector< std::thread > m_workers;

    // the task queue
    std::deque<TaskPtr> m_tasks;

    // synchronization
    std::mutex m_queue_mutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_stop;
};

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
inline void WorkerPool::Worker::operator()()
    {
    BeThreadUtilities::SetCurrentThreadName("WorkerPool worker");

    while (true)
        {
        TaskPtr taskP;

            {   // acquire lock
            std::unique_lock<std::mutex> lock(m_pool.m_queue_mutex);
            while(taskP.IsNull())
                {
                // look for a work item
                while (!m_pool.m_stop && m_pool.m_tasks.empty())
                    {
                    // if there are none wait for notification
                    m_pool.m_condition.wait(lock);
                    }

                if (m_pool.m_stop) // exit if the pool is stopped
                    return;

                // get the task from the queue
                taskP = m_pool.m_tasks.front();
                m_pool.m_tasks.pop_front();

                if(taskP->IsCanceled())
                    taskP = nullptr;
                }

            }   // release lock

        // execute the task
        taskP->_Run();

        // no matter what happen mark as finished and signal everyone.
        taskP->OnFinish();

        // Query executed, release it. 
        taskP = nullptr;    
        }
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
inline WorkerPool::WorkerPool(size_t threads)
    :m_stop(false)
{
    for(size_t i = 0;i<threads;++i)
        m_workers.push_back(std::thread(Worker(*this)));
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
inline WorkerPool::~WorkerPool()
{
    // stop all threads
    m_stop = true;
    m_condition.notify_all();
     
    // join them
    for(size_t i = 0;i< m_workers.size();++i)
        m_workers[i].join();
}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  9/2015
//----------------------------------------------------------------------------------------
inline void WorkerPool::Enqueue(WorkerPool::Task& task, bool atFront)
{
    { // acquire lock
        std::unique_lock<std::mutex> lock(m_queue_mutex);
         
        if(atFront)
            {
            // add the task and increment ref count.
            m_tasks.push_front(&task);
            }
        else
            {
            // add the task and increment ref count.
            m_tasks.push_back(&task);
            }

    } // release lock
     
    // wake up one thread
    m_condition.notify_one();
}


END_IMAGEPP_NAMESPACE
