/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <folly/BeFolly.h>
#include <folly/Singleton.h>
#include <folly/SingletonThreadLocal.h>
#include <folly/io/async/Request.h>

using namespace BeFolly;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::Worker::Work()
    {
    // This keeps this object alive until we return from this method. Otherwise this object may be deleted when its pool
    // is deleted and this method may access members of a deleted object.
    // On some systems (Windows, optimized) the worker threads are stopped before the static pool object is destroyed.
    // In that case we'll release this reference when this thread is terminated, because C++ guarantees destructors
    // are called when the callstack is unwound. This object will then be deleted in the pool destructor
    // since this refcount will be dropped, even though this thread is gone.
    // On other systems (Linux) the threads are not stopped until after static destructors are called. In that
    // case this object will get freed when the thread reacts to the "stop" message, which may happen after the
    // pool is deleted.
    RefCountedPtr<Worker> me(this);
    // This releases the ref count that was increased in Worker::Start method - the worker is kept alive by the above RefCountedPtr
    Release();
    
    SetName();

    for (;;)
        {
        folly::Function<void()> task;
            {
            BeMutexHolder lock(m_cv->GetMutex());
            while (!m_stopped && !m_pool.HasWork())
                m_cv->InfiniteWait(lock);

            if (m_stopped)
                return; // if this flag is on, the pool object has been freed and we can't access it

            task = std::move(m_pool.m_tasks.front());
            m_pool.m_tasks.pop();
            }

        task();
        m_cv->notify_one();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::Worker::SetName()
    {
    BeMutexHolder lock(m_cv->GetMutex());
    if (!m_stopped)
        {
        // Can't use m_pool is m_stopped = true - the pool may already be destroyed
        BeThreadUtilities::SetCurrentThreadName(Utf8PrintfString("%s worker %d", m_pool.GetName(), m_id).c_str());
        }
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
    // Increase ref count to prevent the object from being destroyed in case the threadpool
    // is destroyed (and releases its workers) before the Worker::Work is called - we want
    // the object to stay valid until we get there.
    AddRef();
    BeThreadUtilities::StartNewThread(Main, this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool::ThreadPool(int nThreads, Utf8CP name) : m_name(name)
    {
    // use a shared_ptr so each thread can keep it alive until they all exit
    m_cv = std::make_shared<BeConditionVariable>();

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
        BeMutexHolder lock(m_cv->GetMutex());
        m_tasks.emplace(std::move(func));
        }

    m_cv->notify_one();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ThreadPool::WaitForIdle()
    {
    BeMutexHolder holder(m_cv->GetMutex());
    while (!m_tasks.empty())
        m_cv->InfiniteWait(holder);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool::~ThreadPool()
    {
    // Note:
    // It might seem tempting to wait for the threads to terminate here. That doesn't work
    // for static pools that are only destroyed on program exit. In that case, on Windows, all of the other threads
    // are killed by the system before the destructor is called, leading to chaos. On Linux the threads are still
    // alive, so we signal them to stop. If you want to use a non-static thread pool, call `WaitForIdle` if you want to clear the threads
    // and wait for them to finish before you delete your pool. For static thread pools, there's no point in waiting anyway, since the
    // program is ending.

    if (true)
        {
        BeMutexHolder holder(m_cv->GetMutex());
        for (auto& worker : m_workers)
            worker->m_stopped = true;
        }
    m_cv->notify_all(); // condition variable will be valid until last thread exits
    }

BEGIN_UNNAMED_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct IoThreadPoolImp : ThreadPool
    {
    IoThreadPoolImp() : ThreadPool(std::max<uint32_t>(10, BeThreadUtilities::GetHardwareConcurrency()*2), "IO"){}
    };

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct CpuThreadPoolImp : ThreadPool
    {
    CpuThreadPoolImp() : ThreadPool(BeThreadUtilities::GetHardwareConcurrency(), "CPU"){}
    };

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool& ThreadPool::GetIoPool()
    {
    static folly::Singleton<IoThreadPoolImp> s_pool;
    return *s_pool.try_get_fast();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ThreadPool& ThreadPool::GetCpuPool()
    {
    static folly::Singleton<CpuThreadPoolImp> s_pool;
    return *s_pool.try_get_fast();
    }

namespace folly
    {
template<>
    LeakySingleton<ThreadLocal<SingletonThreadLocal<std::shared_ptr<RequestContext>,detail::DefaultTag>::Wrapper,void,void>,detail::DefaultTag>::Entry &
    LeakySingleton<ThreadLocal<SingletonThreadLocal<std::shared_ptr<RequestContext>,detail::DefaultTag>::Wrapper,void,void>,detail::DefaultTag>::entryInstance()
        {
        static auto entry = detail::createGlobal<Entry, void>();
        return *entry;
        }
    }
