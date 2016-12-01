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
void ThreadPool::Worker::Work()
    {
    SetName();

    for (;;)
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
        m_pool.m_cv.notify_one();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::Worker::SetName()
    {
    BeThreadUtilities::SetCurrentThreadName(Utf8PrintfString("%s worker %d", m_pool.GetName(), m_id).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
THREAD_MAIN_IMPL ThreadPool::Worker::Main(void* arg)
    {
    ((Worker*)arg)->Work();
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::Worker::Start()
    {
    BeThreadUtilities::StartNewThread(50*1024, Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool::ThreadPool(int nThreads, Utf8CP name) : m_name(name)
    {
    for (int i=0; i<nThreads; ++i)
        m_workers.emplace_back(new Worker(*this, i+1));

    for (auto& worker : m_workers)
        worker->Start();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::add(folly::Func func)
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
void ThreadPool::WaitForIdle()
    {
    BeMutexHolder holder(m_cv.GetMutex());
    while (!m_tasks.empty())
        m_cv.InfiniteWait(holder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool::~ThreadPool()
    {
    if (true)
        {
        BeMutexHolder holder(m_cv.GetMutex());
        m_stop = true;
        }
    m_cv.notify_all();

    WaitForIdle();
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct IoThreadPoolImp : ThreadPool
{
    IoThreadPoolImp() : ThreadPool(BeThreadUtilities::GetHardwareConcurrency()*2, "IO"){}
    ~IoThreadPoolImp() {}
};
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct CpuThreadPoolImp : ThreadPool
{
    CpuThreadPoolImp() : ThreadPool(BeThreadUtilities::GetHardwareConcurrency(), "CPU"){}
    ~CpuThreadPoolImp() {}
};
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool& ThreadPool::GetIoPool()
    {
    static folly::Singleton<IoThreadPoolImp> s_pool;
    return *s_pool.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool& ThreadPool::GetCpuPool()
    {
    static folly::Singleton<CpuThreadPoolImp> s_pool;
    return *s_pool.get();
    }
