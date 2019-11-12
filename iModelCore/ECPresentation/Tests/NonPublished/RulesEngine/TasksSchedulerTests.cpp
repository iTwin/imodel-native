/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <folly/futures/DrivableExecutor.h>
#include "../../../Source/RulesDriven/RulesEngine/TaskScheduler.h"
#include "TestHelpers.h"

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct TestTask : ECPresentationTask
{
private:
    bool m_executed;
public:
    TestTask(BeMutex& mutex) 
        : ECPresentationTask(mutex, [&](IECPresentationTaskCR){m_executed = true;}), m_executed(false)
        {}
    bool IsExecuted() const {return m_executed;}
};
DEFINE_REF_COUNTED_PTR(TestTask);

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct ManualExecutor : folly::DrivableExecutor
{
private:
    std::queue<folly::Func> m_queue;
public:
    void add(folly::Func f) override {m_queue.push(std::move(f));}
    void drive() override
        {
        if (!m_queue.empty())
            {
            (m_queue.front())();
            m_queue.pop();
            }
        }
};

/*=================================================================================**//**
* Note: this test implementation doesn't prioritize tasks
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct TestTasksQueue : IECPresentationTasksQueue
{
private:
    mutable BeMutex m_mutex;
    std::deque<IECPresentationTaskPtr> m_queue;
protected:
    BeMutex& _GetMutex() const override { return m_mutex; }
    bool _HasTasks() const override { BeMutexHolder lock(m_mutex); return !m_queue.empty(); }
    void _Add(IECPresentationTask& task) override { BeMutexHolder lock(m_mutex); m_queue.push_back(&task); }
    IECPresentationTaskPtr _Pop(IECPresentationTask::Predicate const& pred) override
        {
        BeMutexHolder lock(m_mutex);
        for (auto iter = m_queue.begin(); iter != m_queue.end(); ++iter)
            {
            IECPresentationTaskPtr task = *iter;
            if (!pred || pred(*task))
                {
                m_queue.erase(iter);
                return task;
                }
            }
        return nullptr;
        }
    bvector<IECPresentationTaskPtr> _Get(IECPresentationTask::Predicate const& pred) const override
        {
        BeMutexHolder lock(m_mutex);
        bvector<IECPresentationTaskPtr> vec;
        for (IECPresentationTaskPtr task : m_queue)
            {
            if (!pred || pred(*task))
                vec.push_back(task);
            }
        return vec;
        }
    TasksCancelationResult _Cancel(IECPresentationTask::Predicate const& pred) override
        {
        BeMutexHolder lock(m_mutex);
        bset<IECPresentationTaskCPtr> matchingTasks;
        bset<IECPresentationTaskCPtr> canceledTasks;
        for (IECPresentationTaskPtr task : m_queue)
            {
            if (!pred || pred(*task))
                {
                if (nullptr != task->GetCancelationToken())
                    {
                    task->Cancel();
                    task->Complete();
                    canceledTasks.insert(task);
                    }
                matchingTasks.insert(task);
                }
            }
        m_queue.erase(std::remove_if(m_queue.begin(), m_queue.end(), [&canceledTasks](IECPresentationTaskPtr const& queuedTask)
            {
            return canceledTasks.end() != canceledTasks.find(queuedTask);
            }), m_queue.end());
        return TasksCancelationResult(matchingTasks);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksQueueTests : ::testing::Test
{
    ECPresentationTasksQueue m_queue;

    RefCountedPtr<ECPresentationTask> CreateTask() {return new ECPresentationTask(m_queue.GetMutex(), [](IECPresentationTaskCR){});}
    RefCountedPtr<ECPresentationTask> CreateCancelableTask() 
        {
        auto task = CreateTask();
        task->SetIsCancelable(true);
        return task;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, HasTasks)
    {
    EXPECT_FALSE(m_queue.HasTasks());

    m_queue.Add(*CreateTask());
    EXPECT_TRUE(m_queue.HasTasks());

    m_queue.Pop();
    EXPECT_FALSE(m_queue.HasTasks());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Get_ReturnsAllTasksInFIFOModeByPriority)
    {
    auto task1 = CreateTask();
    task1->SetPriority(1);
    m_queue.Add(*task1);
    
    auto task2 = CreateTask();
    task2->SetPriority(2);
    m_queue.Add(*task2);

    auto task3 = CreateTask();
    task3->SetPriority(2);
    m_queue.Add(*task3);

    bvector<IECPresentationTaskPtr> tasks = m_queue.Get();
    ASSERT_EQ(3, tasks.size());
    EXPECT_EQ(task2, tasks[0]);
    EXPECT_EQ(task3, tasks[1]);
    EXPECT_EQ(task1, tasks[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Get_ReturnsTasksThatMatchProvidedPredicate)
    {
    auto task1 = CreateTask();
    m_queue.Add(*task1);

    auto task2 = CreateTask();
    m_queue.Add(*task2);

    bvector<IECPresentationTaskPtr> tasks = m_queue.Get([&](IECPresentationTaskCR task)
        {
        return &task == task2.get();
        });
    ASSERT_EQ(1, tasks.size());
    EXPECT_EQ(task2, tasks[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Pop_ReturnsNullWhenQueueHasNoTasks)
    {
    EXPECT_TRUE(m_queue.Pop().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Pop_ReturnsTasksInFIFOModeByPriority)
    {
    auto task1 = CreateTask();
    task1->SetPriority(1);
    m_queue.Add(*task1);

    auto task2 = CreateTask();
    task2->SetPriority(2);
    m_queue.Add(*task2);

    auto task3 = CreateTask();
    task3->SetPriority(1);
    m_queue.Add(*task3);

    auto task4 = CreateTask();
    task4->SetPriority(2);
    m_queue.Add(*task4);

    EXPECT_EQ(task2, m_queue.Pop());
    EXPECT_EQ(task4, m_queue.Pop());
    EXPECT_EQ(task1, m_queue.Pop());
    EXPECT_EQ(task3, m_queue.Pop());
    EXPECT_TRUE(m_queue.Pop().IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Pop_FiltersTasks)
    {
    auto task1 = CreateTask();
    m_queue.Add(*task1);

    auto task2 = CreateTask();
    m_queue.Add(*task2);

    auto task3 = CreateTask();
    m_queue.Add(*task3);

    auto filter = [&](IECPresentationTaskCR task)
        {
        return &task != task2.get();
        };

    EXPECT_EQ(task1, m_queue.Pop(filter));
    EXPECT_EQ(task3, m_queue.Pop(filter));
    EXPECT_TRUE(m_queue.Pop(filter).IsNull());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Cancel_RemovesFromQueueAllTasksWhenNoPredicateProvided)
    {
    auto task1 = CreateCancelableTask();
    m_queue.Add(*task1);

    auto task2 = CreateCancelableTask();
    m_queue.Add(*task2);

    auto result = m_queue.Cancel();

    EXPECT_EQ(2, result.GetTasks().size());
    EXPECT_TRUE(result.GetCompletion().hasValue());

    EXPECT_TRUE(task1->GetCancelationToken()->IsCanceled());
    EXPECT_TRUE(task2->GetCancelationToken()->IsCanceled());

    EXPECT_FALSE(m_queue.HasTasks());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Cancel_RemovesFromQueueTasksThatMatchProvidedPredicate)
    {
    auto task1 = CreateCancelableTask();
    m_queue.Add(*task1);

    auto task2 = CreateCancelableTask();
    m_queue.Add(*task2);

    auto result = m_queue.Cancel([&](IECPresentationTaskCR task)
        {
        return &task == task2.get();
        });
    EXPECT_EQ(1, result.GetTasks().size());
    EXPECT_TRUE(result.GetCompletion().hasValue());
    
    EXPECT_FALSE(task1->GetCancelationToken()->IsCanceled());
    EXPECT_TRUE(task2->GetCancelationToken()->IsCanceled());

    EXPECT_TRUE(m_queue.HasTasks());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksQueueTests, Cancel_CancelsOnlyCancelableTasks)
    {
    auto task1 = CreateCancelableTask();
    m_queue.Add(*task1);

    auto task2 = CreateTask();
    m_queue.Add(*task2);

    auto result = m_queue.Cancel();
    EXPECT_EQ(2, result.GetTasks().size());
    EXPECT_FALSE(result.GetCompletion().poll().hasValue());

    EXPECT_TRUE(task1->GetCancelationToken()->IsCanceled());
    EXPECT_EQ(nullptr, task2->GetCancelationToken());

    EXPECT_TRUE(m_queue.HasTasks());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksSchedulerTests : ::testing::Test
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerTests, GetThreadsCount)
    {
    TThreadAllocationsMap allocations;

    ECPresentationTasksScheduler scheduler1(allocations, *new TestTasksQueue(), [](folly::Func){return folly::makeFuture();});
    EXPECT_EQ(0, scheduler1.GetThreadsCount());

    allocations.clear();
    allocations.Insert(1, 5);
    ECPresentationTasksScheduler scheduler2(allocations, *new TestTasksQueue(), [](folly::Func){return folly::makeFuture();});
    EXPECT_EQ(5, scheduler2.GetThreadsCount());

    allocations.clear();
    allocations.Insert(1, 5);
    allocations.Insert(2, 5);
    ECPresentationTasksScheduler scheduler3(allocations, *new TestTasksQueue(), [](folly::Func){return folly::makeFuture();});
    EXPECT_EQ(10, scheduler3.GetThreadsCount());
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2019
+===============+===============+===============+===============+===============+======*/
struct ECPresentationTasksSchedulerExecutionTests : ::testing::Test
    {
    TestTasksQueue* m_queue;
    ECPresentationTasksScheduler* m_scheduler;
    ManualExecutor m_executor;

    TThreadAllocationsMap CreateSimpleAllocationsMap(int threadsCount) const
        {
        TThreadAllocationsMap map;
        map.Insert(1000, threadsCount);
        return map;
        }
    void SetUp() override
        {
        // note: ECPresentationTasksScheduler taskes ownership of the queue - no need to delete it in the tests
        m_queue = new TestTasksQueue();
        m_scheduler = new ECPresentationTasksScheduler(CreateSimpleAllocationsMap(1), *m_queue, ThreadsHelper::WrapFollyExecutor(m_executor));
        }
    void TearDown() override
        {
        DELETE_AND_CLEAR(m_scheduler);
        m_queue = nullptr;
        }
    RefCountedPtr<TestTask> CreateTask() { return new TestTask(m_queue->GetMutex()); }
    RefCountedPtr<TestTask> CreateCancelableTask()
        {
        auto task = CreateTask();
        task->SetIsCancelable(true);
        return task;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Schedule_QueuesAndExecutesTasksOneAtATime)
    {
    auto task1 = CreateTask();
    m_scheduler->Schedule(*task1);
    // at this point a task1 should've been added to the queue, immediately popped from there,
    // added to the running tasks list and scheduled for execution via the executor
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));

    auto task2 = CreateTask();
    m_scheduler->Schedule(*task2);
    // at this point a task2 should've been added to the queue and waiting for task1 to complete
    EXPECT_TRUE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    
    // the first step executes task1
    m_executor.drive();
    EXPECT_TRUE(task1->IsExecuted());
    EXPECT_TRUE(task1->GetFuture().poll().hasValue());
    EXPECT_FALSE(task1->GetCompletion().poll().hasValue());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));

    // the second step completes task1, removes it from the running tasks list and puts task2 in there
    m_executor.drive();
    EXPECT_TRUE(task1->GetCompletion().poll().hasValue());
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // the third step executes task2
    m_executor.drive();
    EXPECT_TRUE(task2->IsExecuted());
    EXPECT_TRUE(task2->GetFuture().poll().hasValue());
    EXPECT_FALSE(task2->GetCompletion().poll().hasValue());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // the last step completes task2 and removes it from the running tasks list
    m_executor.drive();
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Schedule_QueuesAndExecutesTasksMultipleAtATime)
    {
    // allocate 2 threads to verify this
    m_scheduler->SetThreadAllocationsMap(CreateSimpleAllocationsMap(2));

    auto task1 = CreateTask();
    m_scheduler->Schedule(*task1);
    // at this point a task1 should've been added to the queue, immediately popped from there,
    // added to the running tasks list and scheduled for execution via the executor
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));

    auto task2 = CreateTask();
    m_scheduler->Schedule(*task2);
    // task2 should behave exactly as task1 since we can execute 2 tasks simultaneously
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // the first step executes task1
    m_executor.drive();
    EXPECT_TRUE(task1->IsExecuted());
    EXPECT_TRUE(task1->GetFuture().poll().hasValue());
    EXPECT_FALSE(task1->GetCompletion().poll().hasValue());
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // the second step executes task2
    m_executor.drive();
    EXPECT_TRUE(task2->IsExecuted());
    EXPECT_TRUE(task2->GetFuture().poll().hasValue());
    EXPECT_FALSE(task2->GetCompletion().poll().hasValue());
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // the third step completes task1 and removes it from the running tasks list
    m_executor.drive();
    EXPECT_TRUE(task1->GetCompletion().poll().hasValue());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // the last step completes task2 and removes it from the running tasks list
    m_executor.drive();
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Schedule_UsingTasksAllocationsMap_DoesntTakeHigherPrioritySlots)
    {
    // set up allocations map
    TThreadAllocationsMap allocations;
    allocations.Insert(1000, 1); // 1 allocation for tasks up to 1000 priority
    allocations.Insert(2000, 1); // 1 allocation for tasks up to 2000 priority
    m_scheduler->SetThreadAllocationsMap(allocations);

    auto task1 = CreateTask();
    task1->SetPriority(1000);
    m_scheduler->Schedule(*task1);
    // the task should get into the running tasks list and take one allocation from 'up to 1000 priority' tasks
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());

    auto task2 = CreateTask();
    task2->SetPriority(1000);
    m_scheduler->Schedule(*task2);
    // task2 should go to queue since there's no slot for it left in the allocations map
    EXPECT_TRUE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());

    auto task3 = CreateTask();
    task3->SetPriority(1500);
    m_scheduler->Schedule(*task3);
    // task3 gets into the running list - takes the 'up to 2000' priority slot
    EXPECT_TRUE(m_queue->HasTasks());
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());

    // advance executor 2 times to get task1 and task3 to execute
    m_executor.drive();
    m_executor.drive();
    EXPECT_TRUE(task1->IsExecuted());
    EXPECT_TRUE(task3->IsExecuted());
    // the tasks are still not completed..
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());

    // advance executor to complete task1
    m_executor.drive();
    // this completes task1, frees it's 'up to 1000' slot and starts another task that
    // fits into the slot - task2
    EXPECT_TRUE(task1->GetCompletion().poll().hasValue());
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // advance executor to complete task3
    m_executor.drive();
    EXPECT_TRUE(task3->GetCompletion().poll().hasValue());
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());

    // advance executor to complete task2
    m_executor.drive();
    m_executor.drive();
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Schedule_UsingTasksAllocationsMap_TakesLowerPrioritySlots)
    {
    // set up allocations map
    TThreadAllocationsMap allocations;
    allocations.Insert(1000, 1); // 1 allocation for tasks up to 1000 priority
    allocations.Insert(2000, 1); // 1 allocation for tasks up to 2000 priority
    m_scheduler->SetThreadAllocationsMap(allocations);

    auto task1 = CreateTask();
    task1->SetPriority(2000);
    m_scheduler->Schedule(*task1);
    // the task should get into the running tasks list and take one allocation from 'up to 2000 priority' tasks
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());

    auto task2 = CreateTask();
    task2->SetPriority(2000);
    m_scheduler->Schedule(*task2);
    // task2 should behave exactly as task1 - it takes the lower priority slot
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());

    // advance executor 4 times to complete both tasks
    for (int i = 0; i < 4; ++i)
        m_executor.drive();

    EXPECT_TRUE(task1->IsExecuted());
    EXPECT_TRUE(task2->IsExecuted());
    EXPECT_TRUE(task1->GetCompletion().poll().hasValue());
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Schedule_DoesntExecuteTasksWhenTheyAreBlockedByRunningTasks)
    {
    // need to allocate 2 threads to verify this
    m_scheduler->SetThreadAllocationsMap(CreateSimpleAllocationsMap(2));

    auto task1 = CreateTask();
    auto task2 = CreateTask();
    task1->SetBlockedTasksPredicate([&](IECPresentationTaskCR task)
        {
        return &task == task2.get();
        });
    m_scheduler->Schedule(*task1);
    // at this point a task1 should be in the running tasks list
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));

    m_scheduler->Schedule(*task2);
    // at this point task2 should be still in the queue, because it's being blocked by task1
    EXPECT_TRUE(m_queue->HasTasks());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task1));
    
    // let task1 complete
    m_executor.drive();
    m_executor.drive();
    EXPECT_TRUE(task1->GetCompletion().poll().hasValue());

    // at this point task2 should already be moved from the queue to running tasks list
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task2));

    // let task2 complete
    m_executor.drive();
    m_executor.drive();
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Cancel_CancelsQueuedCancelableTask)
    {
    auto task1 = CreateCancelableTask();
    m_scheduler->Schedule(*task1);
    // at this point a task1 shouldbe in the running tasks list and scheduled for execution via the executor

    auto task2 = CreateCancelableTask();
    m_scheduler->Schedule(*task2);
    // at this point a task2 should be in the queue
    EXPECT_TRUE(m_queue->HasTasks());
    
    // cancel the task that's in the queue
    TasksCancelationResult result = m_scheduler->Cancel([&](IECPresentationTaskCR task)
        {
        return &task == task2.get();
        });
    // only the queued task is canceled
    EXPECT_EQ(1, result.GetTasks().size());
    EXPECT_TRUE(result.GetTasks().end() != result.GetTasks().find(task2.get()));
    EXPECT_TRUE(task2->GetCancelationToken()->IsCanceled());

    // both the task and the cancelation completion future should be completed
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_TRUE(result.GetCompletion().poll().hasValue());

    // the queue should be empty
    EXPECT_FALSE(m_queue->HasTasks());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Cancel_DoesntCancelQueuedNotCancelableTask)
    {
    auto task1 = CreateTask();
    m_scheduler->Schedule(*task1);
    // at this point a task1 shouldbe in the running tasks list and scheduled for execution via the executor

    auto task2 = CreateTask();
    m_scheduler->Schedule(*task2);
    // at this point task 2 should be in the queue
    EXPECT_TRUE(m_queue->HasTasks());

    // attempt cancel the task that's in the queue
    TasksCancelationResult result = m_scheduler->Cancel([&](IECPresentationTaskCR task)
        {
        return &task == task2.get();
        });
    // we expect the task to _not_ be canceled, but be included in the result list
    EXPECT_EQ(1, result.GetTasks().size());
    EXPECT_TRUE(result.GetTasks().end() != result.GetTasks().find(task2.get()));
    EXPECT_TRUE(nullptr == task2->GetCancelationToken());

    // neither the task not the cancelation completion future should _not_ be completed
    EXPECT_FALSE(task2->GetCompletion().poll().hasValue());
    EXPECT_FALSE(result.GetCompletion().poll().hasValue());

    // the queue should _not_ be empty
    EXPECT_TRUE(m_queue->HasTasks());

    // let the task complete by driving the executor 2 times for task1 and 2 times for task2
    for (int i = 0; i < 4; ++i)
        m_executor.drive();

    // ensure both the task and the cancelation completion future are completed now
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_TRUE(result.GetCompletion().poll().hasValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Cancel_CancelsRunningCancelableTask)
    {
    auto task = CreateCancelableTask();
    m_scheduler->Schedule(*task);
    // at this point a task should be in the running tasks list and scheduled for execution via the executor

    // cancel the task
    TasksCancelationResult result = m_scheduler->Cancel();
    EXPECT_EQ(1, result.GetTasks().size());
    EXPECT_TRUE(result.GetTasks().end() != result.GetTasks().find(task.get()));
    EXPECT_TRUE(task->GetCancelationToken()->IsCanceled());

    // the task should _not_ be completed and should still be running - we're
    // letting it gracefully finish
    EXPECT_FALSE(task->GetCompletion().poll().hasValue());
    EXPECT_FALSE(result.GetCompletion().poll().hasValue());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());

    // advance the executor to get the task to execute (which it won't because
    // it's already canceled)
    m_executor.drive();
    EXPECT_FALSE(task->IsExecuted());

    // advance the executor again to get it completed
    m_executor.drive();
    EXPECT_FALSE(task->IsExecuted());
    EXPECT_TRUE(task->GetCompletion().poll().hasValue());
    EXPECT_TRUE(result.GetCompletion().poll().hasValue());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Cancel_DoesntCancelRunningNotCancelableTask)
    {
    auto task = CreateTask();
    m_scheduler->Schedule(*task);
    // at this point a task shouldbe in the running tasks list and scheduled for execution via the executor

    // attempt to cancel the task
    TasksCancelationResult result = m_scheduler->Cancel();

    // we expect the task to _not_ be canceled, but be included in the result list
    EXPECT_EQ(1, result.GetTasks().size());
    EXPECT_TRUE(result.GetTasks().end() != result.GetTasks().find(task.get()));
    EXPECT_TRUE(nullptr == task->GetCancelationToken());

    // the task should _not_ be completed and should still be running - we're
    // letting it gracefully finish
    EXPECT_FALSE(result.GetCompletion().poll().hasValue());
    EXPECT_EQ(1, m_scheduler->GetRunningTasks().size());

    // advance the executor to get the task to execute
    m_executor.drive();
    EXPECT_TRUE(task->IsExecuted());

    // advance the executor again to get it completed
    m_executor.drive();
    EXPECT_TRUE(task->GetCompletion().poll().hasValue());
    EXPECT_TRUE(result.GetCompletion().poll().hasValue());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Cancel_CancelsPendingCancelableTask)
    {
    // to get a task into pending state we need to allocate at least 2 threads
    m_scheduler->SetThreadAllocationsMap(CreateSimpleAllocationsMap(2));

    auto task1 = CreateTask();
    m_scheduler->Schedule(*task1);
    // at this point a task1 be in the running tasks list and scheduled for execution via the executor

    auto task2 = CreateCancelableTask();
    task2->SetBlockedTasksPredicate([&](IECPresentationTaskCR task)
        {
        return &task == task1.get();
        });
    m_scheduler->Schedule(*task2);
    // at this point task2 is added to the pending tasks list, because it wants to block 
    // task1 which is in already running tasks list

    // expect the queue to be empty
    EXPECT_FALSE(m_queue->HasTasks());

    // cancel task2
    TasksCancelationResult result = m_scheduler->Cancel([&](IECPresentationTaskCR task)
        {
        return &task == task2.get();
        });
    EXPECT_EQ(1, result.GetTasks().size());
    EXPECT_TRUE(result.GetTasks().end() != result.GetTasks().find(task2.get()));
    EXPECT_TRUE(task2->GetCancelationToken()->IsCanceled());

    // the task should be completed
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_TRUE(result.GetCompletion().poll().hasValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Cancel_DoesntCancelPendingNotCancelableTask)
    {
    // to get a task into pending state we need to allocate at least 2 threads
    m_scheduler->SetThreadAllocationsMap(CreateSimpleAllocationsMap(2));

    auto task1 = CreateTask();
    m_scheduler->Schedule(*task1);
    // at this point a task1 be in the running tasks list and scheduled for execution via the executor

    auto task2 = CreateTask();
    task2->SetBlockedTasksPredicate([&](IECPresentationTaskCR task)
        {
        return &task == task1.get();
        });
    m_scheduler->Schedule(*task2);
    // at this point task2 should be in the pending tasks list, because it wants to block 
    // task1 which is in already running tasks list

    // expect the queue to be empty
    EXPECT_FALSE(m_queue->HasTasks());

    // attempt to cancel task2
    TasksCancelationResult result = m_scheduler->Cancel([&](IECPresentationTaskCR task)
        {
        return &task == task2.get();
        });
    // verify task2 is in the cancelation list, but is not canceled
    EXPECT_EQ(1, result.GetTasks().size());
    EXPECT_TRUE(result.GetTasks().end() != result.GetTasks().find(task2.get()));
    EXPECT_TRUE(nullptr == task2->GetCancelationToken());

    // the task should _not_ be completed
    EXPECT_FALSE(task2->GetCompletion().poll().hasValue());
    EXPECT_FALSE(result.GetCompletion().poll().hasValue());

    // let the task complete by driving the executor 2 times for task1 and 2 times for task2
    for (int i = 0; i < 4; ++i)
        m_executor.drive();

    // ensure both the task and the cancelation completion future are completed now
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_TRUE(result.GetCompletion().poll().hasValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, Block_BlocksTasksFromExecution)
    {
    // block all tasks
    auto blocker = ECPresentationTasksBlocker::Create(*m_scheduler, [](IECPresentationTaskCR){return true;});

    auto task = CreateTask();
    m_scheduler->Schedule(*task);
    // at this point a task should be still in the queue because it's being blocked from execution
    EXPECT_TRUE(m_queue->HasTasks());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());

    // unblock
    blocker = nullptr;

    // at this point the task should be moved from the queue to running tasks list
    EXPECT_FALSE(m_queue->HasTasks());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().end() != m_scheduler->GetRunningTasks().find(task));

    // let the task complete
    m_executor.drive();
    m_executor.drive();
    EXPECT_TRUE(task->IsExecuted());
    EXPECT_TRUE(task->GetFuture().poll().hasValue());
    EXPECT_TRUE(task->GetCompletion().poll().hasValue());
    EXPECT_TRUE(m_scheduler->GetRunningTasks().empty());
    }

/*---------------------------------------------------------------------------------**//**
* @bsitest                                      Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ECPresentationTasksSchedulerExecutionTests, GetAllTasksCompletion)
    {
    // the test verifies that tasks of all 3 stages are included in the completion:
    // queued, pending and running. to get into a state where there are tasks with 
    // all 3 stages, we need to allocate at least 2 threads
    m_scheduler->SetThreadAllocationsMap(CreateSimpleAllocationsMap(2));

    auto task1 = CreateTask();
    m_scheduler->Schedule(*task1);
    // at this point a task1 be in the running tasks list and scheduled for execution via the executor
    EXPECT_FALSE(m_queue->HasTasks());

    auto task2 = CreateTask();
    task2->SetBlockedTasksPredicate([&](IECPresentationTaskCR task)
        {
        return &task == task1.get();
        });
    m_scheduler->Schedule(*task2);
    // at this point task2 is added to the pending tasks list, because it wants to block 
    // task1 which is in already running tasks list
    EXPECT_FALSE(m_queue->HasTasks());

    auto task3 = CreateTask();
    m_scheduler->Schedule(*task3);
    // task3 stays in queue because all allocations are reserved for task1 and task2
    EXPECT_TRUE(m_queue->HasTasks());

    auto completion = m_scheduler->GetAllTasksCompletion();

    // complete task1
    m_executor.drive();
    m_executor.drive();
    EXPECT_TRUE(task1->GetCompletion().poll().hasValue());
    EXPECT_FALSE(completion.poll().hasValue());

    // after task1 completes, task2 gets unblocked and there's also space for task3, so
    // both task2 and task2 are started
    EXPECT_EQ(2, m_scheduler->GetRunningTasks().size());

    // advance both tasks
    m_executor.drive();
    m_executor.drive();
    EXPECT_TRUE(task2->IsExecuted());
    EXPECT_TRUE(task3->IsExecuted());

    // complete task2
    m_executor.drive();
    EXPECT_TRUE(task2->GetCompletion().poll().hasValue());
    EXPECT_FALSE(completion.poll().hasValue());

    // complete task3
    m_executor.drive();
    EXPECT_TRUE(task3->GetCompletion().poll().hasValue());
    EXPECT_TRUE(completion.poll().hasValue());
    }
