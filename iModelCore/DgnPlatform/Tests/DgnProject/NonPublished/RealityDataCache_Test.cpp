/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/NonPublished/RealityDataCache_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Bentley/BeDebugLog.h>
#include <DgnPlatform/DgnCore/RealityDataCache.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataQueue, Push_Pop)
    {
    static const int itemsCount = 10;
    RealityDataQueue<int> queue;
    
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
TEST (RealityDataQueue, Clear)
    {
    RealityDataQueue<int> queue;
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
    UInt32                   m_sleepTime;
    static int               s_nCalls;
    static bvector<intptr_t> s_threadIds;

    TestWork (BeConditionVariable& cv, UInt32 sleepTime) 
        : m_cv (cv), m_sleepTime (sleepTime)
        { }

    virtual void _DoWork () override 
        {
        BeThreadUtilities::BeSleep (m_sleepTime);

        BeCriticalSectionHolder lock (m_cv.GetCriticalSection ());
        s_nCalls++;
        s_threadIds.push_back (BeThreadUtilities::GetCurrentThreadId ());
        m_cv.Wake (true);
        }

    static RefCountedPtr<TestWork> Create (BeConditionVariable& cv, UInt32 sleepTime = 0) { return new TestWork (cv, sleepTime); }
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
    WorkItemsCountPredicate predicate (1);
    thread->DoWork (*TestWork::Create (cv));
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 500));     // no timeout
    ASSERT_EQ (1, TestWork::s_nCalls);                      // work item executed
    ASSERT_EQ (1, TestWork::s_threadIds.size ());           // work item executed on one thread
    ASSERT_NE (currentThreadId, TestWork::s_threadIds[0]);  // work item executed on a separate thread
    thread->Terminate ();
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
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 500)); // no timeout
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

    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 500)); // no timeout
    ASSERT_EQ (workItemsCount, TestWork::s_nCalls);     // all work items executed
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    10/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, Queueing)
    {
    static int workItemsCount = 5;
    static int workItemSleepTimeout = 50;

    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (1, 1);
    WorkItemsCountPredicate predicate (workItemsCount);

    auto before = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
    for (int i = 0; i < workItemsCount; i++)
        {
        pool->QueueWork (*TestWork::Create (cv, workItemSleepTimeout));
        BeThreadUtilities::BeSleep (10); // give the other thread time to step in
        }

    // the time taken to queue all items should not be more than time taken
    // to execute all but one work items taking more time means that the requesting
    // (this) thread is hung while the thread poll thread executes the work item
    auto after = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
    auto diff  = after - before;
    ASSERT_LE (diff, workItemSleepTimeout * (workItemsCount - 1));

    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 500)); // no timeout
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
    static int workItemSleepTimeout = 50;
    static int maxThreads = 10;
    static int maxIdleThreads = 10;

    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (10, 10);
    WorkItemsCountPredicate predicate (workItemsCount);

    for (int i = 0; i < workItemsCount; i++)
        pool->QueueWork (*TestWork::Create (cv, workItemSleepTimeout));
    
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 500));     // no timeout
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
    static int workItemSleepTimeout = 50;
    static int maxThreads = 10;
    static int maxIdleThreads = 0;

    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (maxThreads, maxIdleThreads);
    WorkItemsCountPredicate predicate (workItemsCount);

    for (int i = 0; i < workItemsCount; i++)
        pool->QueueWork (*TestWork::Create (cv, workItemSleepTimeout));
    
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 500));                 // no timeout
    ASSERT_EQ (workItemsCount, TestWork::s_nCalls);                     // all work items executed
    ASSERT_EQ (workItemsCount, TestWork::s_threadIds.size ());          // 10 thread IDs in the list
    ASSERT_EQ (maxThreads, CountUniqueItems (TestWork::s_threadIds));   // all thread IDs are different

    pool->WaitUntilAllThreadsIdle ();
    ASSERT_EQ (maxIdleThreads, pool->GetThreadsCount ());               // only maxIdleThreads are left active
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Grigas.Petraitis    11/2014
//---------------------------------------------------------------------------------------
TEST (RealityDataThreadPool, Parallel)
    {
    static int workItemsCount = 10;
    static int workItemSleepTimeout = 50;
    static int maxThreads = 5;
    static int maxIdleThreads = 0;

    BeConditionVariable cv;
    auto pool = RealityDataThreadPool::Create (maxThreads, maxIdleThreads);
    WorkItemsCountPredicate predicate (workItemsCount);

    auto before = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
    for (int i = 0; i < workItemsCount; i++)
        pool->QueueWork (*TestWork::Create (cv, workItemSleepTimeout));
    
    ASSERT_TRUE (cv.WaitOnCondition (&predicate, 500));                 // no timeout
    ASSERT_EQ (workItemsCount, TestWork::s_nCalls);                     // all work items executed

    auto after = BeTimeUtilities::GetCurrentTimeAsUnixMillis ();
    auto diff  = (int) (after - before);
    auto expected = workItemSleepTimeout * workItemsCount / maxThreads;
    BeDebugLog (Utf8PrintfString ("RealityDataThreadPool.Parallel took %d ms (expected around %d)", diff, expected).c_str ());
    ASSERT_LT (abs (diff - expected), 40);
    }
