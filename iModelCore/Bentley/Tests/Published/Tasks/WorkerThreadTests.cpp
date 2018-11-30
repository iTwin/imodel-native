/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/WorkerThreadTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WorkerThreadTests.h"

#include <Bentley/Tasks/WorkerThread.h>
#include <Bentley/Tasks/WorkerThreadPool.h>

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Destr_ThreadHasNoStrongRefsLeft_ThreadRunnerStops)
    {
    intptr_t threadId;

    std::weak_ptr <WorkerThread> tWeak;
    bool block = true;
    
        {
        auto thread = WorkerThread::Create ("TestThread");
        tWeak = thread;

        thread->ExecuteAsync ([&]
            {
            while (block)
                {
                }
            });

        threadId = thread->GetThreadId ();
        }

    ASSERT_FALSE (tWeak.expired());

    block = false;

    int timeoutMs = 30 * 1000;
    while (AsyncTasksManager::GetTaskRunner (threadId) != nullptr && timeoutMs > 0)
        {
        timeoutMs -= 100;
        BeThreadUtilities::BeSleep (100);
        }

    ASSERT_TRUE (AsyncTasksManager::GetTaskRunner (threadId) == nullptr);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, GetThreadId_EmptyThread_ReturnsZero)
    {
    auto thread = WorkerThread::Create ("TestThread");

    EXPECT_EQ (0, thread->GetThreadId ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, GetThreadId_StartedThread_ReturnsNonZero)
    {
    auto thread = WorkerThread::Create ("TestThread");

    thread->ExecuteAsync ([]
        {})->Wait ();

    EXPECT_NE (0, thread->GetThreadId ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, GetThreadId_CalledInTask_ReturnsThreadId)
    {
    auto thread = WorkerThread::Create ("TestThread");

    thread->ExecuteAsync ([=]
        {
        EXPECT_EQ (BeThreadUtilities::GetCurrentThreadId (), thread->GetThreadId ());
        })->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, OnEmpty_EmptyThread_DoesNotBlock)
    {
    auto thread = WorkerThread::Create ("TestThread");
    thread->OnEmpty ()->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, OnEmpty_OneTaskAdded_WaitsUntilEmpty)
    {
    auto thread = WorkerThread::Create ("TestThread");

    BeAtomic<int> i (0);
    BeAtomic<bool> block (true);

    thread->ExecuteAsync ([&]
        {
        while (block);
        i++;
        });

    block.store(false);
    thread->OnEmpty ()->Wait ();
    EXPECT_EQ (1, i);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Push_MultipleTasks_ExecutedInOriginalOrder)
    {
    auto thread = WorkerThread::Create ("TestThread");

    BeAtomic<int> i (0);

    auto t1 = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        EXPECT_EQ (1, ++i);
        });
    auto t2 = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        EXPECT_EQ (2, ++i);
        });
    auto t3 = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        EXPECT_EQ (3, ++i);
        });

    thread->Push (t1);
    thread->Push (t2);
    thread->Push (t3);

    thread->OnEmpty ()->Wait ();
    EXPECT_EQ (3, i);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Push_MultipleTasksWithPriority_HighesPriorityExecutesFirst)
    {
    auto thread = WorkerThread::Create ("TestThread");

    BeAtomic<int> i (0);

    auto t1 = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        EXPECT_EQ (3, ++i);
        });

    auto t2 = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        EXPECT_EQ (1, ++i);
        });

    auto t3 = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        EXPECT_EQ (2, ++i);
        });

    BeAtomic<bool> block (true);
    BeAtomic<bool> block2 (true);

    thread->ExecuteAsync ([&]
        {
        block2.store(false);
        // All 3 tasks will be pushed now
        while (block);
        });

    while (block2);

    thread->Push (t1, AsyncTask::Priority::Low);
    thread->Push (t2, AsyncTask::Priority::High);
    thread->Push (t3, AsyncTask::Priority::Normal);

    block.store(false);

    thread->OnEmpty ()->Wait ();
    EXPECT_EQ (3, i);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Push_NoParentTask_PrioritySetToNormal)
    {
    auto thread = WorkerThread::Create ("TestThread");
    auto task = std::make_shared<PackagedAsyncTask<void>> ([&]
        {});

    thread->Push (task);
    EXPECT_EQ (AsyncTask::Priority::Normal, task->GetPriority ());

    thread->OnEmpty ()->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Push_ParentTaskDefaultPriorityAndTaskWithDefaultPriority_PrioritySetNormal)
    {
    auto thread = WorkerThread::Create ("TestThread");
    auto task = std::make_shared<PackagedAsyncTask<void>> ([&]
        {});

    thread->ExecuteAsync ([&]
        {
        thread->Push (task);
        EXPECT_EQ (AsyncTask::Priority::Normal, task->GetPriority ());
        });

    thread->OnEmpty ()->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Push_ParentTaskWithHighPriorityAndTaskWithInheritedPriority_PrioritySetToParentTaskPriority)
    {
    auto thread = WorkerThread::Create ("TestThread");
    auto task = std::make_shared<PackagedAsyncTask<void>> ([&]
        {});

    thread->ExecuteAsync ([&]
        {
        thread->Push (task, AsyncTask::Priority::Inherited);
        EXPECT_EQ (AsyncTask::Priority::High, task->GetPriority ());
        }, AsyncTask::Priority::High);

    thread->OnEmpty ()->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Push_ParentTaskWithHighPriorityAndTaskWithLow_PriorityDidNotChange)
    {
    auto thread = WorkerThread::Create ("TestThread");
    auto task = std::make_shared<PackagedAsyncTask<void>> ([&]
        {});

    thread->ExecuteAsync ([&]
        {
        thread->Push (task, AsyncTask::Priority::Low);
        EXPECT_EQ (AsyncTask::Priority::Low, task->GetPriority ());
        }, AsyncTask::Priority::High);

    thread->OnEmpty ()->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, OnEmpty_NoExecutingTasks_ReturnsCompletedTask)
    {
    auto thread = WorkerThread::Create ("TestThread");

    EXPECT_TRUE (thread->OnEmpty ()->IsCompleted ());
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, OnEmpty_HasExecutingTasks_OnEmptyTaskIsCompletedAfterAllThreadTasksAreCompleted)
    {
    auto thread = WorkerThread::Create ("TestThread");

    BeAtomic<int> completedTasks (0);

    BeAtomic<bool> block (true);

    auto t1 = std::make_shared<PackagedAsyncTask<void>> ([&]
        {
        while (block);
        completedTasks++;
        });

    t1->Then ([&]
        {
        while (block);
        completedTasks++;
        });

    thread->Push (t1);

    thread->OnEmpty ()->Then ([&]
        {
        EXPECT_EQ (2, completedTasks);
        });

    block.store(false);

    thread->OnEmpty ()->Wait ();
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, ExecuteAsyncWithoutAttachingToCurrentTask_MultipleThenCallbackAdded_AllThenCallbacksExecuted)
    {
    auto thread = WorkerThread::Create ("TestThread");

    BeAtomic<bool> block (true);

    auto task = thread->ExecuteAsync ([&]
        {
        thread->ExecuteAsyncWithoutAttachingToCurrentTask ([&]
            {
            while (block);
            });
        });

    task->WaitFor (2000);

    EXPECT_TRUE (task->IsCompleted ());

    block.store(false);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, ExecuteAsync_ThenWithOtherThread_SecondTaskSheduledInOtherThread)
    {
    auto thread1 = WorkerThread::Create ("TestThread1");
    auto thread2 = WorkerThread::Create ("TestThread2");

    bool executed1 = false;
    bool executed2 = false;

    auto task1 = thread1->ExecuteAsync ([&]
        {
        EXPECT_EQ (BeThreadUtilities::GetCurrentThreadId (), thread1->GetThreadId ());
        executed1 = true;
        });
    auto task2 = task1->Then (thread2, [&]
        {
        EXPECT_EQ (BeThreadUtilities::GetCurrentThreadId (), thread2->GetThreadId ());
        executed2 = true;
        });

    task2->Wait ();

    EXPECT_TRUE (executed1);
    EXPECT_TRUE (executed2);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, ExecuteAsync_RecursivelyStartTasks_Succeeds)
    {
    auto thread = WorkerThread::Create ("TestThread");

    struct RecursiveTask : public PackagedAsyncTask<void>
        {
        std::shared_ptr<WorkerThread> m_thread;
        int m_depth;

        void _OnExecute ()
            {
            StartNewTask ();
            };

        void StartNewTask ()
            {
            if (m_depth == 0)
                {
                return;
                }
            m_depth--;
            m_thread->ExecuteAsync ([&]
                {
                StartNewTask ();
                });
            };

        RecursiveTask () : PackagedAsyncTask<void> (nullptr)
            {
            };
        };

    auto task = std::make_shared<RecursiveTask> ();
    task->m_thread = thread;
    task->m_depth = 2001; // Windows 7 64 bit debug build overflows stack at about 1200 tasks

    thread->Push (task);
    thread->OnEmpty()->Wait ();

    EXPECT_EQ (0, task->m_depth);
    }

//---------------------------------------------------------------------------------------
// @betest                                      Benediktas.Lipnickas
//---------------------------------------------------------------------------------------
TEST_F (WorkerThreadTests, Stop_ThreadStoppedInObjectDesctructorThatHadLifeProlongedByTaskInOtherThread_DoesNotDeadlock)
    {
    auto thread2 = WorkerThread::Create ("TestThread2");

    bool threadBlock = true;
    static bool threadStopped = false;

    struct ThreadContainer
        {
        std::shared_ptr<WorkerThread> thread;
        ThreadContainer () : thread (WorkerThread::Create ("TestThread1"))
            {
            }
        ~ThreadContainer ()
            {
            threadStopped = true;
            }
        };

    auto container = std::make_shared<ThreadContainer> ();
    container->thread->ExecuteAsync ([=, &threadBlock]
        {
        while (threadBlock);
        })
            ->Then (thread2, [=]
            {
            EXPECT_TRUE (container != nullptr); // prolong life of container in seperate thread
            })
                ->Then (thread2, [=]
                {
                // This is needed
                });

            container = nullptr;
            threadBlock = false;

            int timeoutMs = 30 * 1000;
            while (!threadStopped && timeoutMs > 0)
                {
                BeThreadUtilities::BeSleep (100);
                timeoutMs -= 100;
                }

            EXPECT_TRUE (threadStopped);
    }
