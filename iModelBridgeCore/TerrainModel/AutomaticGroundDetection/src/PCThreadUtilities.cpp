/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"
#include "PCThreadUtilities.h"
#include "Bentley/BeTimeUtilities.h"

//USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY

BEGIN_GROUND_DETECTION_NAMESPACE


/*======================================================================================+
|   Debug timer                                                 Grigas.Petraitis
+======================================================================================*/
//#define DEBUG_THREADPOOL 1
#if defined DEBUG_THREADPOOL
    struct DebugTimer
        {
        Utf8String  m_msg;
        uint64_t    m_timeBefore;
        DebugTimer(Utf8CP msg) : m_msg(msg), m_timeBefore(BeTimeUtilities::GetCurrentTimeAsUnixMillis()) {}
        ~DebugTimer() {BeDebugLog(Utf8PrintfString("%s took %llu ms", m_msg.c_str(),(BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_timeBefore)).c_str());}
        };
 #endif

/*======================================================================================+
|   ThreadSafeQueue
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::PushFront(T element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto item = Item::Create(element, NULL, m_first.get());
    if (m_first.IsNull())
        {
        BeAssert(m_last.IsNull());
        m_first = m_last = item;
        }
    else
        {
        m_first->prev = item;
        m_first = item;
        }
    m_count++;
    m_cv.notify_all();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::PushBack(T element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto item = Item::Create(element, m_last.get(), NULL);
    if (m_last.IsNull())
        {
        BeAssert(m_first.IsNull());
        m_first = m_last = item;
        }
    else
        {
        m_last->next = item;
        m_last = item;
        }
    m_count++;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopFront(T* element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_first.IsNull())
        return false;

    if (nullptr != element)
        *element = m_first->data;

    if (m_first.Equals(m_last))
        {
        m_first = m_last = NULL;
        }
    else
        {
        m_first->next->prev = NULL;
        m_first = m_first->next;
        }
    m_count--;
    m_cv.notify_all();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopBack(T* element)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_last.IsNull())
        return false;

    if (nullptr != element)
        *element = m_last->data;

    if (m_first.Equals(m_last))
        {
        m_first = m_last = NULL;
        }
    else
        {
        m_last->prev->next = NULL;
        m_last = m_last->prev;
        }
    m_count--;
    m_cv.notify_all();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::Pop(T* element) {return SchedulingMethod::FIFO == m_schedulingMethod ? PopFront(element) : PopBack(element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::Pop(T& element) {return Pop(&element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::Pop() {return Pop(nullptr);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopBack(T& element) {return PopBack(&element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopBack() {return PopBack(nullptr);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopFront(T& element) {return PopFront(&element);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::PopFront() {return PopFront(nullptr);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::Erase(Iterator const& iterator)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (!iterator.IsValid() || m_first.IsNull())
        return;

    Item* erase = iterator.m_curr;

    if (erase != m_first.get())
        erase->prev->next = erase->next;
    else
        m_first = erase->next;

    if (erase != m_last.get())
        erase->next->prev = erase->prev;
    else
        m_last = erase->prev;

    m_count--;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void ThreadSafeQueue<T>::Clear()
    {
    BeMutexHolder lock(m_cv.GetMutex());
    auto item = m_first;
    while (item.IsValid())
        {
        auto temp = item;
        item = item->next;
        temp->next = NULL;
        temp->prev = NULL;
        }
    m_first = NULL;
    m_last = NULL;
    m_count = 0;
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> bool ThreadSafeQueue<T>::IsEmpty() const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    return m_first.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> unsigned ThreadSafeQueue<T>::Size() const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    return m_count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> typename ThreadSafeQueue<T>::Iterator ThreadSafeQueue<T>::begin() const {return Iterator(m_first.get());}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                     Grigas.Petraitis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> typename ThreadSafeQueue<T>::Iterator ThreadSafeQueue<T>::end() const {return Iterator(nullptr);}

// explicitly implement for testing purposes, 
// note: must be done AFTER all template functions are defined
template struct ThreadSafeQueue<int>;



/*======================================================================================+
|   PointCloudThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudThread::PointCloudThread(/*DgnPlatformLib::Host* hostToAdopt,*/Utf8CP threadName)
    : m_threadId(-1), m_threadName(threadName)/*, m_hostToAdopt(hostToAdopt)*/
    {
    //BeAssert(NULL!=m_hostToAdopt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudThread::~PointCloudThread()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThread::Start()
    {
#if defined(BUILDTMFORDGNDB) && !defined(DGNDB06_API)
    BeThreadUtilities::StartNewThread(PlatformThreadRunner, this, 1024 * 1024);
#else
    BeThreadUtilities::StartNewThread(1024 * 1024, PlatformThreadRunner, this);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThread::ThreadRunner(void* arg)
    {
    auto& thread = *(PointCloudThread*) arg;
    thread.AddRef();
    thread.Run();
    thread.Release();
    }

/*---------------------------------------------------------------------------------**//**
* Runs on its own thread.
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThread::Run()
    {
    //According to Keith Bentley the AdoptHost function could be use the set the host created in the main thread
    //to the other working thread (see UstationLibHost.h for more information). 
    /*
    BeAssert(DgnPlatformLib::QueryHost() == NULL);
    DgnPlatformLib::AdoptHost(*m_hostToAdopt);
    BeAssert(DgnPlatformLib::QueryHost() != NULL);
    */
    m_threadId = BeThreadUtilities::GetCurrentThreadId();

    if (!m_threadName.empty())
        BeThreadUtilities::SetCurrentThreadName(m_threadName.c_str());


    _Run();

    /*
    BeAssert(DgnPlatformLib::QueryHost() != NULL);
    DgnPlatformLib::ForgetHost();
    BeAssert(DgnPlatformLib::QueryHost() == NULL);
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
intptr_t PointCloudThread::GetThreadId() const
    {
    return m_threadId;
    }

/*======================================================================================+
|   PointCloudThreadPool
+======================================================================================*/
#ifdef DEBUG_THREADPOOL
#define THREADPOOL_MSG(msg)     BeDebugLog(Utf8PrintfString("\t %llu\t ThreadPool\t %lld\t %s", BeTimeUtilities::GetCurrentTimeAsUnixMillis(), (Int64)BeThreadUtilities::GetCurrentThreadId(), msg).c_str());
#define THREADPOOL_TIMER(msg)   DebugTimer _debugtimer(msg);  
#else
#define THREADPOOL_MSG(msg) 
#define THREADPOOL_TIMER(msg)
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThreadPool::QueueWork(PointCloudWork& work)
    {
    WaitUntilQueueIsNotFull();

    THREADPOOL_TIMER("QueueWork")
    THREADPOOL_MSG("QueueWork")
    
    BeMutexHolder lock(m_workQueueCS);

    if (IsTerminating())
        {
        THREADPOOL_MSG("QueueWork: Ignore work item queued, terminating!")
        return;
        }
    auto thread = GetIdleThread();
    if (thread.IsNull())
        {
        THREADPOOL_MSG("QueueWork: No idle threads")
        if (!ShouldCreateNewThread())
            {
            m_workQueue.PushBack(&work);

            //Update memory used
            size_t memoryUsed = GetMemoryUsed() + work.GetMemorySize();
            SetMemoryUsed(memoryUsed);

            THREADPOOL_MSG("QueueWork: Added work item to queue")
            return;
            }
        thread = CreateThread();
        THREADPOOL_MSG("QueueWork: Created a new thread")
        }
    thread->DoWork(work);
    THREADPOOL_MSG("QueueWork: Sent work item to thread")
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudThreadPool::~PointCloudThreadPool()
    {
    THREADPOOL_MSG("~PointCloudThreadPool")
    Terminate();

    // clear the list of threads in a critical section
        {
        BeMutexHolder threadsLock(m_threadsCV.GetMutex());        
        THREADPOOL_MSG("Clear threads list")
        m_threads.clear();        
        m_threadsCV.notify_all();
        }
    
    BeMutexHolder workQueueLock(m_workQueueCS);        
    THREADPOOL_MSG("Clear work queue")
    m_workQueue.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThreadPool::Terminate()
    {
    THREADPOOL_MSG("Terminate")
    if (m_isTerminating)
        {
        THREADPOOL_MSG("Already terminating")
        return;
        }

    m_isTerminating = true;
    m_workQueue.GetConditionVariable().notify_all();

    // copy the threads list in a critical section
    bvector<PointCloudWorkerThreadPtr> threads;
        {
        BeMutexHolder threadsLock(m_threadsCV.GetMutex());
        for (auto pair : m_threads)
            threads.push_back(pair.first);
        }

    // terminate all threads
    THREADPOOL_MSG("Terminate all threads")
    for (auto thread : threads)
        {
        thread->Terminate();
        thread->Release();
        }
    BeMutexHolder workQueueLock(m_workQueueCS);
    THREADPOOL_MSG("Clear work queue")
    m_workQueue.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudWorkerThreadPtr PointCloudThreadPool::GetIdleThread() const
    {    
    BeMutexHolder lock(m_threadsCV.GetMutex());
    for (auto pair : m_threads)
        {
        if (!pair.second)
            return pair.first;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudWorkerThreadPtr PointCloudThreadPool::CreateThread()
    {
    auto thread = PointCloudWorkerThread::Create(/*DgnPlatformLib::QueryHost(),*/this, "BentleyThreadPoolWorker");
    thread->AddRef();
    thread->Start();    
    BeMutexHolder lock(m_threadsCV.GetMutex());
    m_threads[thread.get()] = false;
    m_threadsCV.notify_all();
    return thread;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int PointCloudThreadPool::GetThreadsCount() const
    {    
    BeMutexHolder lock(m_threadsCV.GetMutex());
    return(int) m_threads.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudThreadPool::ShouldCreateNewThread() const
    {
    // do not exceed the "max threads" parameter
    if (GetThreadsCount() >= m_maxThreads)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThreadPool::_OnThreadBusy(PointCloudWorkerThread& thread)
    {    
    BeMutexHolder lock(m_threadsCV.GetMutex());
    THREADPOOL_MSG("_OnThreadBusy: Marked the thread as busy")
    m_threads[&thread] = true;
    m_threadsCV.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThreadPool::_OnThreadIdle(PointCloudWorkerThread& thread)
    {
    THREADPOOL_MSG("_OnThreadIdle")

    if (!_AssignWork(thread))
        {
        THREADPOOL_MSG("_OnThreadIdle: No work items in queue")        
        BeMutexHolder lock(m_threadsCV.GetMutex());
        int idleThreadsCount = 0;
        for (auto pair : m_threads)
            {
            if (!pair.second)
                idleThreadsCount++;
            }
        
        THREADPOOL_MSG(Utf8PrintfString("_OnThreadIdle: Total idle threads: %d, allowed: %d", idleThreadsCount, m_maxIdleThreads).c_str())
        BeAssert(idleThreadsCount < m_maxIdleThreads || m_maxIdleThreads == 0);
        if (idleThreadsCount >= m_maxIdleThreads)
            {
            THREADPOOL_MSG("_OnThreadIdle removed the thread from list")
            m_threads.erase(&thread);
            if (!thread.TerminateRequested())
                {
                thread.Terminate();
                thread.Release();
                THREADPOOL_MSG("_OnThreadIdle: Terminated the thread")
                }
            }
        else
            {
            m_threads[&thread] = false;
            THREADPOOL_MSG("_OnThreadIdle: Marked the thread as idle")
            }

        m_threadsCV.notify_all();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudThreadPool::_AssignWork(PointCloudWorkerThread& thread)
    {
    if (m_isTerminating)
        return false;

    PointCloudWorkPtr work;
    if (true)
        {
        BeMutexHolder lock(m_workQueueCS);
        m_workQueue.Pop(work);
        }

    if (work.IsValid())
        {
        THREADPOOL_MSG("_OnThreadIdle: Popped work item from queue, send to idle thread")
        //Remove count of old work memory usage and add new one
        size_t memoryUsed = GetMemoryUsed() - thread.GetMemoryUsed();
        SetMemoryUsed(memoryUsed);
        //Set new work memory usage
        thread.SetMemoryUsed(work->GetMemorySize()); 
        thread.DoWork(*work);
        return true;
        }
    else
        {
        //Remove count of old work memory usage
        size_t memoryUsed = GetMemoryUsed() - thread.GetMemoryUsed();
        SetMemoryUsed(memoryUsed);
        //Set memory usage to zero since there is no work to do
        thread.SetMemoryUsed(0);
        }

    return false;
    }

//=======================================================================================
// @bsiclass                                    Marc.Bedard                     09/2015
//=======================================================================================
struct AllThreadsIdlePredicate : IConditionVariablePredicate
{
private:
    PointCloudThreadPool const& m_pool;
public:
    AllThreadsIdlePredicate(PointCloudThreadPool const& pool) : m_pool(pool) {}
    virtual bool _TestCondition(BeConditionVariable& cv) override 
        {
        BeMutexHolder lock(cv.GetMutex());
        for (auto pair : m_pool.m_threads)
            {
            bool isThreadBusy = pair.second;
            if (isThreadBusy && pair.first->GetThreadId() != BeThreadUtilities::GetCurrentThreadId())
                return false;
            }
        return true;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThreadPool::WaitUntilAllThreadsIdle() const
    {
    AllThreadsIdlePredicate predicate(*this);
    m_threadsCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

//=======================================================================================
// @bsiclass                                        Marc.Bedard            12/2015
//=======================================================================================
struct ThreadPoolWorkDonePredicate : IConditionVariablePredicate
    {
    private:
        PointCloudThreadPool const& m_pool;
    public:
        ThreadPoolWorkDonePredicate(PointCloudThreadPool const& pool) : m_pool(pool) {}
    virtual bool _TestCondition(BeConditionVariable& cv) override
        {
        BeMutexHolder lock(cv.GetMutex());

        //Work done if queue empty and all thread idle
        bool isQueueEmpty(m_pool.m_workQueue.IsEmpty());
        if (!isQueueEmpty)
            return false;

        for (auto pair : m_pool.m_threads)
            {
            bool isThreadBusy = pair.second;
            if (isThreadBusy && pair.first->GetThreadId() != BeThreadUtilities::GetCurrentThreadId())
                return false;
            }
        //All thread idle and queue empty -> work done!
        return true;
        }
    };

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudThreadPool::WaitUntilWorkDone(uint32_t timeoutMillis) const
    {
    bool isWorkDone(false);
    ThreadPoolWorkDonePredicate predicate(*this);
    isWorkDone = m_threadsCV.WaitOnCondition(&predicate, timeoutMillis);
    //If there is still work to do, wake threads - this prevent dead lock
    if (!isWorkDone)
        m_threadsCV.notify_all();
    return isWorkDone;
    }

 //=======================================================================================
// @bsiclass                                        Grigas.Petraitis            11/2014
//=======================================================================================
struct ThreadPoolQueueNotFullPredicate : IConditionVariablePredicate
    {
    private:
        PointCloudThreadPool const& m_pool;
    public:
        ThreadPoolQueueNotFullPredicate(PointCloudThreadPool const& pool) : m_pool(pool) {}
    virtual bool _TestCondition(BeConditionVariable& cv) override
        {
        BeMutexHolder lock(cv.GetMutex());
        //Lets thread pool terminate...
        if (m_pool.IsTerminating())
            return true;

        //Otherwise, compute memory used by queue, if larger than our limit, the queue is full...
        size_t memoryUsed(m_pool.GetMemoryUsed());
        return !m_pool.IsMemoryLimitReached(memoryUsed);
        }
    };

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThreadPool::WaitUntilQueueIsNotFull() const
    {
    ThreadPoolQueueNotFullPredicate predicate(*this);
    m_threadsCV.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudThreadPool::IsMemoryLimitReached(size_t memoryUsed) const
    {
    double percentMemorytoUse(0.75);
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    ::GlobalMemoryStatusEx(&statex);

    //Try to use only a certain % of physical memory
    bool limitReached = (memoryUsed > (statex.ullTotalPhys*percentMemorytoUse)) || ((statex.dwMemoryLoad > percentMemorytoUse) && memoryUsed!=0);

    return limitReached;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudThreadPool::SetMemoryUsed(size_t memoryUsed)
    {
    BeMutexHolder lock(m_memoryUsedCS);
    m_memoryUsed = memoryUsed;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PointCloudThreadPool::GetMemoryUsed() const
     {
     BeMutexHolder lock(m_memoryUsedCS);
     return m_memoryUsed;
     }

/*======================================================================================+
|   PointCloudWorkerThread
+======================================================================================*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudWorkerThread::PointCloudWorkerThread(/*DgnPlatformLib::Host* hostToAdopt,*/ IStateListener* stateListener, Utf8CP threadName)
    : PointCloudThread(/*hostToAdopt,*/ threadName), m_cv(), m_terminate(false), m_stateListener(stateListener), m_idleSince(BeTimeUtilities::GetCurrentTimeAsUnixMillis()), m_busySince(0), m_memoryUsed(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudWorkerThread::~PointCloudWorkerThread()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudWorkerThread::SetIsBusy(bool busy)
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (busy && 0 != m_busySince || !busy && 0 != m_idleSince)
        return;

    if (busy)
        {
        m_idleSince = 0;
        m_busySince = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        _OnBusy();
        }
    else
        {
        m_idleSince = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        m_busySince = 0;
        _OnIdle();
        }
    m_cv.notify_all();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudWorkerThread::_OnBusy()
    {
    if (m_stateListener.IsValid())
        m_stateListener->_OnThreadBusy(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudWorkerThread::_OnIdle()
    {
    if (m_stateListener.IsValid())
        m_stateListener->_OnThreadIdle(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudWorkerThread::IsBusy(uint64_t* busyTime) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_busySince != 0)
        {
        if (NULL != busyTime)
            *busyTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_busySince;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudWorkerThread::IsIdle(uint64_t* idleTime) const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    if (m_idleSince != 0)
        {
        if (NULL != idleTime)
            *idleTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - m_idleSince;
        return true;
        }
    return false;
    }

//=======================================================================================
// @bsiclass                                     Marc.Bedard                     09/2015
//=======================================================================================
struct IsIdlePredicate : IConditionVariablePredicate
{
private:
    PointCloudWorkerThread& m_thread;
public:
    IsIdlePredicate(PointCloudWorkerThread& thread) : m_thread(thread) {}
    virtual bool _TestCondition(BeConditionVariable &cv) override {return m_thread.IsIdle();}
};

//=======================================================================================
// @bsiclass                                      Marc.Bedard                     09/2015
//=======================================================================================
struct HasWorkOrTerminatesPredicate : IConditionVariablePredicate
{
private:
    PointCloudWorkerThread& m_thread;

public:
    HasWorkOrTerminatesPredicate(PointCloudWorkerThread& thread) : m_thread(thread) {}
    virtual bool _TestCondition(BeConditionVariable &cv) override {return m_thread.TerminateRequested() || m_thread.m_currentWork.IsValid();}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudWorkerThread::_DoWork(PointCloudWork& work)
    {
    // note: if m_threadId is -1 it means that the thread hasn't been started yet
    // but that also means that we're not on that thread and should wait for idle, etc.
    BeMutexHolder holder(m_cv.GetMutex(), BeMutexHolder::Lock::No);
    if (BeThreadUtilities::GetCurrentThreadId() != GetThreadId())
        {
        IsIdlePredicate predicate(*this);
        holder.lock();              
        m_cv.ProtectedWaitOnCondition(holder, &predicate, BeConditionVariable::Infinite);        
        }

    BeAssert(m_currentWork.IsNull());
    SetIsBusy(true);
    m_currentWork = &work;
    m_cv.notify_all();

    if (BeThreadUtilities::GetCurrentThreadId() != GetThreadId())
        holder.unlock();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudWorkerThread::_Run()
    {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    while (!m_terminate)
        {
        BeMutexHolder holder(m_cv.GetMutex(), BeMutexHolder::Lock::Yes);        
        HasWorkOrTerminatesPredicate predicate(*this);                                        
        m_cv.ProtectedWaitOnCondition(holder, &predicate, BeConditionVariable::Infinite);        

        if (!m_terminate)
            {            
            BeAssert(m_currentWork.IsValid());
            auto work = m_currentWork;
            m_currentWork = NULL;
            holder.unlock();
            work->_DoWork();
            }
        else
            holder.unlock();

        SetIsBusy(false);
        }
    SetIsBusy(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudWorkerThread::Terminate()
    {   
    BeMutexHolder lock(m_cv.GetMutex());
    /*
    if (m_terminate)
        return false;
        */

    m_terminate = true;
    m_cv.notify_all();
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudWorkerThread::TerminateRequested() const
    {
    BeMutexHolder lock(m_cv.GetMutex());
    return m_terminate;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionThreadPool::GroundDetectionThreadPool(int numWorkingThreads)
: m_numWorkingThreads(numWorkingThreads)//, m_isTerminating(false)
    {
    m_futures = new std::future<void>[m_numWorkingThreads];

    //m_workingThreads = new std::thread[m_numWorkingThreads];
        /*
        m_areWorkingThreadRunning = new std::atomic<bool>[m_maxThreads];

        for (size_t ind = 0; ind < m_numWorkingThreads; ind++)
            m_areWorkingThreadRunning[ind] = false;
            */
                
    m_currentWorkInd = 0;
    m_run = false;    
    }

GroundDetectionThreadPool::~GroundDetectionThreadPool()
    {
    WaitAndStop();
   
    delete[] m_futures;
    }

void GroundDetectionThreadPool::ClearQueueWork()
    {
    m_currentWorkInd = 0;
    m_workQueue.clear();
    }

void GroundDetectionThreadPool::QueueWork(GroundDetectionWorkPtr& work)
    {    
    assert(m_run == false);
    m_workQueue.push_back(work);
    }

void GroundDetectionThreadPool::Start(IActiveWait* activeWait)
    { 
    m_activeWait = activeWait;

    if (m_run == false)
        {                
        m_run = true;
        assert(m_currentWorkInd == 0);

        //Launch a group of threads
        for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
            {         
            m_futures[threadId] = std::async(&GroundDetectionThreadPool::WorkThread, this, /*DgnPlatformLib::QueryHost(), */threadId);            
            }
        }
    }

void GroundDetectionThreadPool::WaitAndStop()
    {         
    std::chrono::milliseconds span(500);

    for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
        {
        if (m_futures[threadId].valid())
            {
            if (m_activeWait == nullptr)
                { 
                m_futures[threadId].get();
                }
            else
                {
                while (m_futures[threadId].wait_for(span) == std::future_status::timeout)
                    {
                    m_activeWait->Progress();
                    }
                }
            }
/*
        if (m_workingThreads[threadId].joinable())
            m_workingThreads[threadId].join();                                
*/
        }
    
    m_run = false;
    }    
    
void GroundDetectionThreadPool::WorkThread(/*DgnPlatformLib::Host* hostToAdopt, */int threadId)
    {
        //DgnPlatformLib::AdoptHost(*hostToAdopt);

    GroundDetectionWorkPtr currentWork;

    do
        {  
        currentWork = nullptr;
        
        uint32_t currentInd = std::atomic_exchange<uint32_t> (&m_currentWorkInd, m_currentWorkInd + 1);
               
        if (currentInd < m_workQueue.size())
            {
            currentWork = m_workQueue[currentInd];
            currentWork->_DoWork();
            }                

        } while (m_run && (currentWork.IsValid()));
    }
    
END_GROUND_DETECTION_NAMESPACE
