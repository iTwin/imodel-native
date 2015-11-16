/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/NonPublished/RealityDataCache_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeDebugLog.h>
#include <DgnPlatform/RealityDataCache.h>

#include <functional>
#include <memory>

USING_NAMESPACE_BENTLEY_DGN

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (ThreadSafeQueue, Push_Pop)
    {
    static const int itemsCount = 10;
    ThreadSafeQueue<int> queue;
    
    for (int i = 0; i < itemsCount; i++)
        queue.PushBack (i);

    for (int i = 0; i < itemsCount; i++)
        {
        int result;
        ASSERT_TRUE (queue.Pop (result));
        ASSERT_EQ (result, i);
        }

    for (int i = 0; i < itemsCount; i++)
        {
        int result;
        queue.PushBack (i);
        ASSERT_TRUE (queue.Pop (result));
        ASSERT_EQ (result, i);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (ThreadSafeQueue, Clear)
    {
    ThreadSafeQueue<int> queue;
    queue.PushBack (1);
    queue.PushBack (2);
    queue.Clear ();

    int num;
    ASSERT_FALSE (queue.Pop (num));
    }

typedef RefCountedPtr<struct TestWork> TestWorkPtr;
//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct TestWork : RefCounted<RealityDataWork>
    {
    BeConditionVariable&     m_cv;
    std::function<void (void)> m_work;
    int                     m_nCalls;
    bvector<intptr_t>       m_threadIds;
    
    TestWork (BeConditionVariable& cv, std::function<void (void)> const& work)
        : m_cv(cv), m_work(work), m_nCalls(0)
        { }

    virtual void _DoWork () override 
        {
        BeMutexHolder lock (m_cv.GetMutex());
        if (NULL != m_work)
            m_work();
        m_nCalls++;
        m_threadIds.push_back (BeThreadUtilities::GetCurrentThreadId ());
        m_cv.notify_all();
        }

    static TestWorkPtr Create (BeConditionVariable& cv, std::function<void (void)> const& work = nullptr) { return new TestWork (cv, work); }
    };

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct WorkItemsCountPredicate : IConditionVariablePredicate
    {
    int m_expectedWorkItemsCount;
    TestWork const* m_work;
    unsigned const* m_itemsCount;
    WorkItemsCountPredicate (TestWork const& work, int expectedWorkItemsCount) : m_work(&work), m_itemsCount(nullptr), m_expectedWorkItemsCount(expectedWorkItemsCount) {}
    WorkItemsCountPredicate (unsigned const& itemsCount, int expectedWorkItemsCount) : m_work(nullptr), m_itemsCount(&itemsCount), m_expectedWorkItemsCount(expectedWorkItemsCount) {}
    virtual bool _TestCondition (BeConditionVariable &cv) override 
        {
        if (nullptr != m_work)
            return m_expectedWorkItemsCount == m_work->m_nCalls;
        if (nullptr != m_itemsCount)
            return m_expectedWorkItemsCount == *m_itemsCount;
        return false;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataWorkerThread, DoWork)
    {
    auto currentThreadId = BeThreadUtilities::GetCurrentThreadId ();
    BeConditionVariable cv;
    TestWorkPtr work = TestWork::Create(cv);
    WorkItemsCountPredicate predicate (*work, 1);

    auto thread = RealityDataWorkerThread::Create ();
    thread->Start ();
    thread->DoWork (*work);

    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));    // no timeout
    ASSERT_EQ (1, work->m_nCalls);                      // work item executed
    ASSERT_EQ (1, work->m_threadIds.size());           // work item executed on one thread
    ASSERT_NE (currentThreadId, work->m_threadIds[0]);  // work item executed on a separate thread
    thread->Terminate ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Mantas.Ragauskas    01/2015
//---------------------------------------------------------------------------------------
TEST (RealityDataWorkerThread, IsIdle)
    {
    uint64_t idleTime;
    BeConditionVariable cv;
    TestWorkPtr work = TestWork::Create(cv);
    WorkItemsCountPredicate predicate (*work, 1);

    auto   thread = RealityDataWorkerThread::Create ();
    thread->Start ();
    thread->DoWork (*work);

    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));    // no timeout
    ASSERT_EQ (1, work->m_nCalls);                      // work item executed
    
    // Work is done, thread is created and idling
    while (!thread->IsIdle (&idleTime))
        ;

    ASSERT_TRUE (idleTime >= 0);
    thread->Terminate ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Mantas.Ragauskas    01/2015
//---------------------------------------------------------------------------------------
TEST (RealityDataWorkerThread, IsBusy)
    {
    BeAtomic<bool> stop(false);
    uint64_t workingTime;
    BeConditionVariable cv;
    TestWorkPtr work = TestWork::Create(cv, [&stop] ()
        {
        while (!stop);
        });
    WorkItemsCountPredicate predicate (*work, 1);
    
    auto   thread = RealityDataWorkerThread::Create ();
    thread->Start ();
    thread->DoWork (*work);
    ASSERT_TRUE (thread->IsBusy (&workingTime));        // Thread is busy with task
    stop = true;
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ   (1, work->m_nCalls);                // work item executed
    thread->Terminate ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Mantas.Ragauskas    01/2015
//---------------------------------------------------------------------------------------
TEST (RealityDataWorkerThread, Terminate)
    {
    BeConditionVariable cv;
    TestWorkPtr work = TestWork::Create(cv);
    WorkItemsCountPredicate predicate (*work, 1);

    auto thread = RealityDataWorkerThread::Create ();
    thread->Start ();
    thread->Terminate ();
    thread->DoWork (*work);

    ASSERT_FALSE (cv.WaitOnCondition (&predicate, 100));
    ASSERT_NE    (1, work->m_nCalls);       // work item not executed after termination
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, QueueWork_1_Item)
    {
    BeConditionVariable cv;
    TestWorkPtr work = TestWork::Create(cv);
    WorkItemsCountPredicate predicate (*work, 1);

    auto pool = RealityDataThreadPool::Create (1, 1, SchedulingMethod::FIFO);
    pool->QueueWork (*work);

    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ (1, work->m_nCalls);                  // work item executed
    pool->Terminate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, QueueWork_Many_Items)
    {
    static int workItemsCount = 1000;
    unsigned nCalls = 0;

    BeConditionVariable cv;

    auto pool = RealityDataThreadPool::Create (1, 1, SchedulingMethod::FIFO);
    for (int i = 0; i < workItemsCount; i++)
        {
        TestWorkPtr work = TestWork::Create(cv, [&nCalls](){nCalls++;});
        pool->QueueWork(*work);
        }

    WorkItemsCountPredicate predicate (nCalls, workItemsCount);
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ (workItemsCount, nCalls);     // all work items executed
    pool->Terminate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, Queueing)
    {
    static int workItemsCount = 5;
    BeAtomic<bool> block(true);
    unsigned nCalls = 0;
    BeConditionVariable cv;    

    auto pool = RealityDataThreadPool::Create (1, 1, SchedulingMethod::FIFO);
    pool->QueueWork(*TestWork::Create(cv, [&block, &nCalls] ()
        {
        while (block);
        nCalls++;
        }));

    // queueing additional work should not wait for the first work to be finished
    for (int i = 0; i < workItemsCount - 1; i++)
        {
        pool->QueueWork(*TestWork::Create(cv, [&block, &nCalls] ()
            {
            while (block);
            nCalls++;
            }));
        }

    block = false;

    WorkItemsCountPredicate predicate(nCalls, workItemsCount);
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ (workItemsCount, nCalls);     // all work items executed
    pool->Terminate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    11/2014
//---------------------------------------------------------------------------------------
template<typename T> static size_t CountUniqueItems (bvector<T> const& list)
    {
    bvector<T> uniqueItems;
    for (auto item : list)
        {
        bool unique = true;
        for (auto uniqueItem : uniqueItems)
            {
            if (uniqueItem == item)
                {
                unique = false;
                break;
                }
            }
        if (unique)
            uniqueItems.push_back (item);
        }
    return uniqueItems.size ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    11/2014
//---------------------------------------------------------------------------------------
template<typename T> static void AssertNoItemsMatch (bvector<T> const& list, T const& item)
    {
    for (auto listItem : list)
        ASSERT_NE (item, listItem);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    11/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, SpawnsThreads)
    {
    static int workItemsCount = 15;
    static int maxThreads = 10;
    static int maxIdleThreads = 10;
    
    BeConditionVariable cv;
    BeAtomic<bool> block(true);
    unsigned nCalls = 0;
    bvector<intptr_t> threadIds;

    auto pool = RealityDataThreadPool::Create (10, 10, SchedulingMethod::FIFO);
    for (int i = 0; i < workItemsCount; i++)
        {
        pool->QueueWork(*TestWork::Create (cv, [&block, &nCalls, &threadIds]()
            {
            while(block);
            nCalls++;
            threadIds.push_back (BeThreadUtilities::GetCurrentThreadId ());
            }));
        }
    
    block = false;

    WorkItemsCountPredicate predicate (nCalls, workItemsCount);
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));    // no timeout
    ASSERT_EQ (workItemsCount, nCalls);         // all work items executed
    ASSERT_EQ (maxIdleThreads, pool->GetThreadsCount ());   // total threads count after the threads finished their work
    
    // make sure each work item executed on a separate thread
    ASSERT_EQ (workItemsCount, threadIds.size());
    ASSERT_EQ (maxThreads, CountUniqueItems (threadIds));
    AssertNoItemsMatch (threadIds, BeThreadUtilities::GetCurrentThreadId ());
    pool->Terminate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    11/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, TerminatesSpawnedThreads)
    {
    static int workItemsCount = 15;
    static int maxThreads = 10;
    static int maxIdleThreads = 0;
    
    BeConditionVariable cv;
    BeAtomic<bool> block(true);
    unsigned nCalls = 0;
    bvector<intptr_t> threadIds;

    auto pool = RealityDataThreadPool::Create (maxThreads, maxIdleThreads, SchedulingMethod::FIFO);
    for (int i = 0; i < workItemsCount; i++)
        {
        pool->QueueWork(*TestWork::Create(cv, [&block, &nCalls, &threadIds]()
            {
            while (block);
            nCalls++;
            threadIds.push_back (BeThreadUtilities::GetCurrentThreadId ());
            }));
        }
    
    block = false;

    WorkItemsCountPredicate predicate (nCalls, workItemsCount);
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));                // no timeout
    ASSERT_EQ (workItemsCount, nCalls);                     // all work items executed
    ASSERT_EQ (workItemsCount, threadIds.size());          // 10 thread IDs in the list
    ASSERT_EQ (maxThreads, CountUniqueItems (threadIds));   // all thread IDs are different

    pool->WaitUntilAllThreadsIdle ();
    ASSERT_EQ (maxIdleThreads, pool->GetThreadsCount ());               // only maxIdleThreads are left active
    pool->Terminate();
    }