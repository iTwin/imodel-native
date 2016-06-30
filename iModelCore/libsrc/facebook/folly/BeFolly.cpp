/*--------------------------------------------------------------------------------------+
|
|     $Source: folly/BeFolly.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <folly/BeFolly.h>
#include <folly/Singleton.h>

using namespace BeFolly;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IOThreadPool::Worker::Work()
    {
    for(;;)
        {
        std::function<void()> task;
            {
            BeMutexHolder lock(m_pool.m_cv.GetMutex());
            while (!m_pool.HasWork() && !m_pool.IsStopped())
                m_pool.m_cv.InfiniteWait(lock);

            if (m_pool.IsStopped())
                return;

            task = std::move(m_pool.m_tasks.front());
            m_pool.m_tasks.pop();
            }

        task();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL IOThreadPool::Worker::Main(void* arg)
    {
    BeThreadUtilities::SetCurrentThreadName("IOPoolWorker");

    ((Worker*)arg)->Work();

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IOThreadPool::Worker::Start()
    {
    BeThreadUtilities::StartNewThread(50*1024, Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
IOThreadPool::IOThreadPool(int nThreads)
    {
    for (int i=0; i<nThreads; ++i)
        m_workers.emplace_back(new Worker(*this));

    for (auto& worker : m_workers)
        worker->Start();
    }                                                 \

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IOThreadPool::add(folly::Func func)
    {
    if (true)
        {
        BeMutexHolder lock(m_cv.GetMutex());
        if (IsStopped())
            {
            BeAssert(false);
            return;
            }
        m_tasks.emplace(func);
        }

    m_cv.notify_one();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void IOThreadPool::WaitForIdle()
    {
    BeMutexHolder holder(m_cv.GetMutex());
    while (!m_tasks.empty())
        m_cv.InfiniteWait(holder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
IOThreadPool::~IOThreadPool()
    {
    m_stop = true;
    m_cv.notify_all();

    WaitForIdle();
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct IOThreadPoolImp : IOThreadPool
{
    IOThreadPoolImp() : IOThreadPool(BeThreadUtilities::GetHardwareConcurrency()){}
    ~IOThreadPoolImp() {}
};
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
IOThreadPool& IOThreadPool::GetPool()
    {
    static folly::Singleton<IOThreadPoolImp> s_pool;
    return *s_pool.get();
    }
