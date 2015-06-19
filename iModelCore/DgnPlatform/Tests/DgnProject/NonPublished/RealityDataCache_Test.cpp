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
#include <DgnPlatform/DgnCore/RealityDataCache.h>

#include <functional>
#include <memory>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (ThreadSafeQueue, Push_Pop)
    {
    static const int itemsCount = 10;
    ThreadSafeQueue<int> queue;
    
    for (int i = 0; i < itemsCount; i++)
        queue.Push (i);

    for (int i = 0; i < itemsCount; i++)
        {
        int result;
        ASSERT_TRUE (queue.Pop (result));
        ASSERT_EQ (result, i);
        }

    for (int i = 0; i < itemsCount; i++)
        {
        int result;
        queue.Push (i);
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
    queue.Push (1);
    queue.Push (2);
    queue.Clear ();

    int num;
    ASSERT_FALSE (queue.Pop (num));
    }

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct TestWork : RefCounted<RealityDataWork>
    {
    BeConditionVariable&     m_cv;
    std::function<void (void)> m_work;
    static int               s_nCalls;
    static bvector<intptr_t> s_threadIds;
    
    TestWork (BeConditionVariable& cv, std::function<void (void)> const& work)
        : m_cv (cv), m_work (work)
        { }

    virtual void _DoWork () override 
        {
        if (NULL != m_work)
            m_work();

        BeMutexHolder lock (m_cv.GetMutex());
        s_nCalls++;
        s_threadIds.push_back (BeThreadUtilities::GetCurrentThreadId ());
        m_cv.notify_all();
        }

    static RefCountedPtr<TestWork> Create (BeConditionVariable& cv, std::function<void (void)> const& work = nullptr) { return new TestWork (cv, work); }
    };
int TestWork::s_nCalls = 0;
bvector<intptr_t> TestWork::s_threadIds;

//=======================================================================================
// @bsiclass                                        Grigas.Petraitis            10/2014
//=======================================================================================
struct WorkItemsCountPredicate : IConditionVariablePredicate
    {
    int m_expectedWorkItemsCount;
    WorkItemsCountPredicate (int expectedWorkItemsCount) : m_expectedWorkItemsCount (expectedWorkItemsCount) 
        {
        TestWork::s_nCalls = 0;
        TestWork::s_threadIds.clear ();
        }
    virtual bool _TestCondition (BeConditionVariable &cv) override 
        {
        return m_expectedWorkItemsCount == TestWork::s_nCalls;
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataWorkerThread, DoWork)
    {
    auto currentThreadId = BeThreadUtilities::GetCurrentThreadId ();
    BeConditionVariable cv;
    auto thread = RealityDataWorkerThread::Create ();
    thread->Start ();
    WorkItemsCountPredicate predicate (1);
    thread->DoWork (*TestWork::Create (cv));
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));    // no timeout
    ASSERT_EQ (1, TestWork::s_nCalls);                      // work item executed
    ASSERT_EQ (1, TestWork::s_threadIds.size ());           // work item executed on one thread
    ASSERT_NE (currentThreadId, TestWork::s_threadIds[0]);  // work item executed on a separate thread
    thread->Terminate ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Mantas.Ragauskas    01/2015
//---------------------------------------------------------------------------------------
TEST (RealityDataWorkerThread, IsIdle)
    {
    uint64_t idleTime;
    BeConditionVariable cv;
    auto   thread      = RealityDataWorkerThread::Create ();
    thread->Start ();
    WorkItemsCountPredicate predicate (1);
   
    thread->DoWork (*TestWork::Create (cv));
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));    // no timeout
    ASSERT_EQ (1, TestWork::s_nCalls);                      // work item executed
    
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
    uint64_t workingTime;
    auto   thread = RealityDataWorkerThread::Create ();
    thread->Start ();
    BeConditionVariable cv;
    WorkItemsCountPredicate predicate (1);

    BeAtomic<bool> stop (false);
    thread->DoWork (*TestWork::Create (cv, [&stop] ()
        {
        while (!stop);
        }));
    ASSERT_TRUE (thread->IsBusy (&workingTime));        // Thread is busy with task
    stop = true;
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ   (1, TestWork::s_nCalls);                // work item executed
    thread->Terminate ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Mantas.Ragauskas    01/2015
//---------------------------------------------------------------------------------------
TEST (RealityDataWorkerThread, Terminate)
    {
    auto thread = RealityDataWorkerThread::Create ();
    thread->Start ();
    BeConditionVariable cv;
    WorkItemsCountPredicate predicate (1);

    TestWork::s_nCalls = 0;
    thread->Terminate ();
    thread->DoWork (*TestWork::Create (cv));

    ASSERT_FALSE (cv.WaitOnCondition (&predicate, 100));
    ASSERT_NE    (1, TestWork::s_nCalls);       // work item not executed after termination
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, QueueWork_1_Item)
    {
    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (1, 1);
    WorkItemsCountPredicate predicate (1);
    pool->QueueWork (*TestWork::Create (cv));
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ (1, TestWork::s_nCalls);                  // work item executed
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, QueueWork_Many_Items)
    {
    static int workItemsCount = 1000;

    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (1, 1);
    WorkItemsCountPredicate predicate (workItemsCount);
    for (int i = 0; i < workItemsCount; i++)
        pool->QueueWork (*TestWork::Create (cv));

    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ (workItemsCount, TestWork::s_nCalls);     // all work items executed
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, Queueing)
    {
    static int workItemsCount = 5;

    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (1, 1);
    WorkItemsCountPredicate predicate (workItemsCount);
    
    BeAtomic<bool> block(true);
    pool->QueueWork (*TestWork::Create (cv, [&block] ()
        {
        while (block)
            ;
        }));

    // queueing additional work should not wait for the first work to be finished
    for (int i = 0; i < workItemsCount - 1; i++)
        {
        pool->QueueWork (*TestWork::Create (cv));
        }

    block = false;

    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));// no timeout
    ASSERT_EQ (workItemsCount, TestWork::s_nCalls);     // all work items executed
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

    BeAtomic<bool> block(true);
    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (10, 10);
    WorkItemsCountPredicate predicate (workItemsCount);

    for (int i = 0; i < workItemsCount; i++)
        {
        pool->QueueWork (*TestWork::Create (cv, [&block]()
            {
            while(block);
            }));
        }
    
    block = false;
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));    // no timeout
    ASSERT_EQ (workItemsCount, TestWork::s_nCalls);         // all work items executed
    ASSERT_EQ (maxIdleThreads, pool->GetThreadsCount ());   // total threads count after the threads finished their work
    
    // make sure each work item executed on a separate thread
    ASSERT_EQ (workItemsCount, TestWork::s_threadIds.size ());
    ASSERT_EQ (maxThreads, CountUniqueItems (TestWork::s_threadIds));
    AssertNoItemsMatch (TestWork::s_threadIds, BeThreadUtilities::GetCurrentThreadId ());
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
    auto pool = RealityDataThreadPool::Create (maxThreads, maxIdleThreads);
    WorkItemsCountPredicate predicate (workItemsCount);

    BeAtomic<bool> block(true);
    for (int i = 0; i < workItemsCount; i++)
        pool->QueueWork (*TestWork::Create (cv, [&block](){while (block);}));
    
    block = false;
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 5000));                // no timeout
    ASSERT_EQ (workItemsCount, TestWork::s_nCalls);                     // all work items executed
    ASSERT_EQ (workItemsCount, TestWork::s_threadIds.size ());          // 10 thread IDs in the list
    ASSERT_EQ (maxThreads, CountUniqueItems (TestWork::s_threadIds));   // all thread IDs are different

    pool->WaitUntilAllThreadsIdle ();
    ASSERT_EQ (maxIdleThreads, pool->GetThreadsCount ());               // only maxIdleThreads are left active
    }
