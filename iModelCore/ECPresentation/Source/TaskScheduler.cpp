/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <numeric>
#include <folly/executors/InlineExecutor.h>
#include "RulesEngineTypes.h"
#include "TaskScheduler.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TaskDependencies::Has(ITaskDependency const& dep) const
    {
    return std::any_of(begin(), end(), [&dep](auto const& containedDep) { return containedDep->Matches(dep); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TaskDependencies::Has(std::function<bool(ITaskDependency const&)> pred) const
    {
    return std::any_of(begin(), end(), [&pred](auto const& containedDep) { return pred(*containedDep); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<bool(ITaskDependency const&)> StringBasedTaskDependency::CreatePredicate(Utf8CP dependencyType, std::function<bool(Utf8StringCR)> pred)
    {
    return [pred, dependencyType](ITaskDependency const& dep)
        {
        return dep.GetDependencyType() == dependencyType
            && pred(static_cast<StringBasedTaskDependency const&>(dep).GetValue());
        };
    }

Utf8CP TaskDependencyOnConnection::s_dependencyType = "Connection";
Utf8CP TaskDependencyOnSelection::s_dependencyType = "Selection";
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::function<bool(ITaskDependency const&)> TaskDependencyOnSelection::CreatePredicate(std::function<bool(SelectionInfoCR)> selectionInfoPredicate)
    {
    return [selectionInfoPredicate, type = s_dependencyType](ITaskDependency const& dep)
        {
        return dep.GetDependencyType() == type
            && selectionInfoPredicate(static_cast<TaskDependencyOnSelection const&>(dep).GetSelectionInfo());
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned ThreadsHelper::ComputeThreadsCount(TThreadAllocationsMap const& allocations)
    {
    return std::accumulate(allocations.begin(), allocations.end(), 0,
        [](unsigned sum, TThreadAllocationsMap::const_reference v){return sum + v.second;});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TThreadAllocationsMap::const_iterator ThreadsHelper::FindAllocationSlot(TThreadAllocationsMap const& freeSlots, int priority)
    {
    if (freeSlots.empty())
        return freeSlots.end();

    auto match = freeSlots.end();
    for (auto iter = freeSlots.begin(); iter != freeSlots.end(); ++iter)
        {
        if (iter->second > 0)
            match = iter;
        if (iter->first >= priority)
            break;
        }
    return match;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TThreadAllocationsMap ThreadsHelper::SubtractAllocations(TThreadAllocationsMap const& lhs, bvector<int> const& priorities)
    {
    TThreadAllocationsMap result = lhs;
    for (int priority : priorities)
        {
        auto slot = FindAllocationSlot(result, priority);
        if (slot == result.end() || slot->second == 0)
            {
            // note: if `lhs` doesn't contain a given priority, we do nothing
            continue;
            }
        --result[slot->first];
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<folly::Executor> ECPresentationTasksManager::CreateExecutor(TThreadAllocationsMap const& threadAllocations)
    {
    unsigned threadsCount = ThreadsHelper::ComputeThreadsCount(threadAllocations);
    if (threadsCount > 0)
        return std::make_unique<ECPresentationThreadPool>(threadsCount);
    return std::make_unique<folly::InlineExecutor>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationTasksScheduler* ECPresentationTasksManager::CreateScheduler(TThreadAllocationsMap const& threadAllocations, folly::Executor& executor)
    {
    return new ECPresentationTasksScheduler(threadAllocations, *new ECPresentationTasksQueue(), executor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationTasksManager::ECPresentationTasksManager(TThreadAllocationsMap threadAllocations)
    {
    m_tasksExecutor = CreateExecutor(threadAllocations);
    ECPresentationTasksScheduler* scheduler = CreateScheduler(threadAllocations, *m_tasksExecutor);
    m_scheduler = scheduler;
    m_threadsCount = scheduler->GetThreadsCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationTasksManager::~ECPresentationTasksManager()
    {
    m_scheduler->Cancel(nullptr).GetCompletion().wait();
    m_scheduler->GetAllTasksCompletion().wait();
    DELETE_AND_CLEAR(m_scheduler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::SetThreadAllocationsMap(TThreadAllocationsMap allocations)
    {
    m_threadAllocations = allocations;
    m_threadsCount = ThreadsHelper::ComputeThreadsCount(m_threadAllocations);
    CheckTasks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::_Schedule(IECPresentationTaskR task)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Tasks scheduler: schedule task `%s`", task.GetId().ToString().c_str()));
    m_queue->Add(task);
    CheckTasks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TThreadAllocationsMap ECPresentationTasksScheduler::CreateAvailableAllocationsMap() const
    {
    bvector<int> priorities;
    std::transform(m_runningTasks.begin(), m_runningTasks.end(), std::back_inserter(priorities), [](IECPresentationTaskPtr const& t){return t->GetPriority();});
    std::transform(m_pendingTasks.begin(), m_pendingTasks.end(), std::back_inserter(priorities), [](IECPresentationTaskPtr const& t){return t->GetPriority();});
    return ThreadsHelper::SubtractAllocations(m_threadAllocations, priorities);
    }

/*---------------------------------------------------------------------------------**//**
* Note: Must be called within a mutex
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationTask::Predicate ECPresentationTasksScheduler::CreateTasksFilter(TThreadAllocationsMap const& availableAllocationsMap) const
    {
    bvector<IECPresentationTask::Predicate> blockedTasksPredicates;
    for (IECPresentationTaskPtr const& task : m_pendingTasks)
        blockedTasksPredicates.push_back(task->GetOtherTasksBlockingPredicate());
    for (IECPresentationTaskPtr const& task : m_runningTasks)
        blockedTasksPredicates.push_back(task->GetOtherTasksBlockingPredicate());
    for (IECPresentationTasksBlocker const* blocker : m_blockers)
        blockedTasksPredicates.push_back([blocker](IECPresentationTaskCR task){return blocker->IsBlocked(task);});

    return [blockedTasksPredicates, &availableAllocationsMap](IECPresentationTaskCR task)
        {
        // make sure we still have idle threads to handle a task with this priority
        auto slot = ThreadsHelper::FindAllocationSlot(availableAllocationsMap, task.GetPriority());
        if (slot == availableAllocationsMap.end() || slot->second <= 0)
            return false;

        // make sure the task is not blocked by a running task
        for (IECPresentationTask::Predicate const& blockedTaskPredicate : blockedTasksPredicates)
            {
            if (blockedTaskPredicate && blockedTaskPredicate(task))
                return false;
            }

        // make sure this task is not blocked
        if (task.IsBlocked())
            return false;

        return true;
        };
    }

/*---------------------------------------------------------------------------------**//**
* Note: Must be called within a mutex
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPresentationTasksScheduler::CanExecute(IECPresentationTaskCR task) const
    {
    for (IECPresentationTasksBlocker const* blocker : m_blockers)
        {
        if (blocker->IsBlocked(task))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Task `%s` can't be executed - it's blocked by a blocker", task.GetId().ToString().c_str()));
            return false;
            }
        }

    // check if this task might block other task
    auto const& isBlockingOtherTasksPredicate = task.GetOtherTasksBlockingPredicate();
    if (isBlockingOtherTasksPredicate)
        {
        for (IECPresentationTaskPtr const& runningTask : m_runningTasks)
            {
            // we can't execute a task that wants to block an already running one
            if (isBlockingOtherTasksPredicate(*runningTask))
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Task `%s` can't be executed - it's blocking an already running task `%s`",
                    task.GetId().ToString().c_str(), runningTask->GetId().ToString().c_str()));
                return false;
                }
            }
        }

    // check if this task is blocked by something
    if (task.IsBlocked())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Task `%s` can't be executed - it's blocked by a predicate", task.GetId().ToString().c_str()));
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Note: Must be called within a mutex
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationTaskPtr ECPresentationTasksScheduler::PopTask(IECPresentationTask::Predicate const& filter, TThreadAllocationsMap& availableThreadAllocationsMap)
    {
    auto scope = Diagnostics::Scope::Create("Tasks scheduler: pop task");

    // first we try the pending tasks list
    for (auto it = m_pendingTasks.begin(); it != m_pendingTasks.end(); ++it)
        {
        auto pendingTask = *it;
        if (CanExecute(*pendingTask))
            {
            IECPresentationTaskPtr result = pendingTask;
            m_pendingTasks.erase(it);
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Returning pending task `%s`", result->GetId().ToString().c_str()));
            return result;
            }
        }
    // note: we don't need to update the allocations map if we return a task from
    // the pending tasks list - it's already been accounted for when it got into that list
    IECPresentationTaskPtr result = m_queue->Pop(filter);
    if (result.IsValid())
        {
        auto slot = ThreadsHelper::FindAllocationSlot(availableThreadAllocationsMap, result->GetPriority());
        if (slot != availableThreadAllocationsMap.end() && slot->second > 0)
            availableThreadAllocationsMap[slot->first]--;
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Returning queued task `%s`", result->GetId().ToString().c_str()));
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::CheckTasks()
    {
    auto scope = Diagnostics::Scope::Create("Tasks scheduler: check tasks");

    BeMutexHolder lock(GetMutex());
    if (m_runningTasks.size() >= m_threadsCount)
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Can't execute any tasks - all threads are busy. Running tasks: %" PRIu64, (uint64_t)m_runningTasks.size()));
        return;
        }

    auto tasksToRun = m_threadsCount - m_runningTasks.size();
    auto availableAllocationsMap = CreateAvailableAllocationsMap();
    auto filter = CreateTasksFilter(availableAllocationsMap);
    while (tasksToRun--)
        {
        IECPresentationTaskPtr task = PopTask(filter, availableAllocationsMap);
        if (task.IsNull())
            {
            // scheduler has no more tasks available to run
            break;
            }
        // have to make sure the task we want to execute doesn't want to block
        // the running tasks; if it does - put it into the pending tasks list
        if (!CanExecute(*task))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_DEBUG, Utf8PrintfString("Moved task `%s` to pending list", task->GetId().ToString().c_str()));
            m_pendingTasks.push_back(task);
            }
        else
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_DEBUG, Utf8PrintfString("Requesting task execute `%s`", task->GetId().ToString().c_str()));
            ExecuteTask(*task);
            }
        }

    // if no tasks are running at the moment but we have some tasks pending execute ping task that will complete after 0.5s.
    // after this task completes 'CheckTasks' will be called again and will check if some pending tasks might be executed now
    if (m_runningTasks.empty() && m_blockers.empty() && (!m_pendingTasks.empty() || m_queue->HasTasks()))
        {
        RefCountedPtr<ECPresentationTask> task = new ECPresentationTask(GetMutex(), [](auto const&)
            {
            BeThreadUtilities::BeSleep(500);
            });
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Requesting ping task execute `%s`", task->GetId().ToString().c_str()));
        ExecuteTask(*task);
        }
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct EnsureTaskCleanup
    {
    std::function<void()> m_cleanupFunc;
    EnsureTaskCleanup(std::function<void()> cleanupFunc) : m_cleanupFunc(cleanupFunc) {}
    ~EnsureTaskCleanup() {if (m_cleanupFunc) m_cleanupFunc();}
    void DoCleanup()
        {
        if (m_cleanupFunc) m_cleanupFunc();
        m_cleanupFunc = nullptr;
        }
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TaskExecutionScope
    {
    IECPresentationTaskR m_task;
    TaskExecutionScope(IECPresentationTaskR task) : m_task(task) {m_task.OnBeforeExecute();}
    ~TaskExecutionScope() {m_task.OnAfterExecute();}
    };

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct TempMutexUnlock
    {
    BeMutexHolderR m_lock;
    TempMutexUnlock(BeMutexHolderR lock) : m_lock(lock) {m_lock.unlock();}
    ~TempMutexUnlock() {m_lock.lock();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::ExecuteTask(IECPresentationTaskR task)
    {
    BeMutexHolder lock(GetMutex());
    ICancelationTokenCPtr cancelationToken = task.GetCancelationToken();
    m_runningTasks.insert(&task);
    folly::via(&m_executor).then([this, task = IECPresentationTaskPtr(&task), cancelationToken]() mutable
        {
        BeMutexHolder lock(GetMutex());
        TaskExecutionScope executionScope(*task);

        // cleanup in destructor of `EnsureTaskCleanup` just to make sure we do cleanup even if we throw
        EnsureTaskCleanup cleanup([this, task]()
            {
            auto completeScope = Diagnostics::Scope::Create(Utf8PrintfString("Task `%s` completing", task->GetId().ToString().c_str()));
            m_runningTasks.erase(task);
            task->Complete();
            CheckTasks();
            });

        std::function<void()> promiseResolver;
        {
        auto executeScope = Diagnostics::Scope::Create(Utf8PrintfString("Task `%s` executing", task->GetId().ToString().c_str()));
        ThrowIfCancelled(cancelationToken.get());
        TempMutexUnlock unlock(lock);
        promiseResolver = task->Execute();
        }

        // we want to cleanup before resolving the result promise
        cleanup.DoCleanup();

        if (promiseResolver)
            promiseResolver();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename TContainer>
static void CancelTasks(bset<IECPresentationTaskCPtr>& matchingTasks, TContainer& tasks, IECPresentationTask::Predicate const& pred, bool complete)
    {
    bset<IECPresentationTaskCPtr> tasksToRemove;
    for (IECPresentationTaskPtr const& task : tasks)
        {
        if (!pred || pred(*task))
            {
            // no cancelation token means the task is not cancelable
            if (task->GetCancelationToken() != nullptr)
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_DEBUG, Utf8PrintfString("Cancelling task `%s`", task->GetId().ToString().c_str()));
                task->Cancel();
                if (complete)
                    {
                    task->Complete();
                    tasksToRemove.insert(task);
                    }
                }
            else
                {
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Task `%s` is not cancellable", task->GetId().ToString().c_str()));
                }

            // we still want to include the task in the result list even
            // if it wasn't actually canceled - the callers want to get completions list
            // of all tasks matching predicate
            matchingTasks.insert(task);
            }
        }
    if (!tasksToRemove.empty())
        {
        tasks.erase(std::remove_if(tasks.begin(), tasks.end(), [&tasksToRemove](IECPresentationTaskPtr const& task)
            {
            return tasksToRemove.end() != tasksToRemove.find(task);
            }), tasks.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TasksCancelationResult ECPresentationTasksScheduler::_Cancel(IECPresentationTask::Predicate const& pred)
    {
    auto scope = Diagnostics::Scope::Create("Tasks scheduler: cancel tasks");
    BeMutexHolder lock(GetMutex());
    bset<IECPresentationTaskCPtr> matchingTasks;

    // cancel queued tasks
    {
    auto queuedTasksScope = Diagnostics::Scope::Create("Cancel queued tasks");
    TasksCancelationResult queueCancelationResult = m_queue->Cancel(pred);
    for (IECPresentationTaskCPtr const& task : queueCancelationResult.GetTasks())
        matchingTasks.insert(task);
    }

    // cancel pending tasks
    {
    auto pendingTasksScope = Diagnostics::Scope::Create("Cancel pending tasks");
    CancelTasks(matchingTasks, m_pendingTasks, pred, true);
    }

    // cancel running tasks
    {
    auto runningTasksScope = Diagnostics::Scope::Create("Cancel running tasks");
    CancelTasks(matchingTasks, m_runningTasks, pred, false);
    }

    return TasksCancelationResult(matchingTasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TasksCancelationResult ECPresentationTasksScheduler::_Restart(IECPresentationTask::Predicate const& pred)
    {
    auto scope = Diagnostics::Scope::Create("Tasks scheduler: restart tasks");
    BeMutexHolder lock(GetMutex());
    bset<IECPresentationTaskCPtr> matchingTasks;

    // restart running tasks
    bvector<IECPresentationTaskPtr> restartedTasks;
    for (IECPresentationTaskPtr const& task : m_runningTasks)
        {
        if (!pred || pred(*task))
            {
            DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_DEBUG, Utf8PrintfString("Restarting task `%s`", task->GetId().ToString().c_str()));
            task->Restart();
            restartedTasks.push_back(task);
            }
        }

    // put all restarted tasks to pending tasks list
    ContainerHelpers::Push(m_pendingTasks, restartedTasks);

    return TasksCancelationResult(ContainerHelpers::TransformContainer<bset<IECPresentationTaskCPtr>>(restartedTasks));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::_Block(IECPresentationTasksBlocker const* blocker)
    {
    auto scope = Diagnostics::Scope::Create("Tasks scheduler: add blocker");
    BeMutexHolder lock(GetMutex());
    m_blockers.push_back(blocker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::_Unblock(IECPresentationTasksBlocker const* blocker)
    {
    auto scope = Diagnostics::Scope::Create("Tasks scheduler: remove blocker");
    BeMutexHolder lock(GetMutex());
    m_blockers.erase(std::find(m_blockers.begin(), m_blockers.end(), blocker), m_blockers.end());
    CheckTasks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> ECPresentationTasksScheduler::_GetAllTasksCompletion(IECPresentationTask::Predicate const& predicate) const
    {
    BeMutexHolder lock(GetMutex());
    bvector<folly::Future<folly::Unit>> completions;
    for (IECPresentationTaskPtr const& task : m_runningTasks)
        {
        if (!predicate || predicate(*task))
            completions.push_back(task->GetCompletion());
        }
    for (IECPresentationTaskPtr const& task : m_queue->Get(predicate))
        completions.push_back(task->GetCompletion());
    return folly::collectAll(completions).then();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPresentationTasksQueue::_HasTasks() const
    {
    BeMutexHolder lock(m_mutex);
    return !m_prioritizedQueue.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksQueue::_Add(IECPresentationTask& task)
    {
    auto scope = Diagnostics::Scope::Create(Utf8PrintfString("Tasks queue: add task `%s`", task.GetId().ToString().c_str()));
    BeMutexHolder lock(m_mutex);
    int priority = task.GetPriority();
    auto iter = m_prioritizedQueue.find(priority);
    if (m_prioritizedQueue.end() == iter)
        iter = m_prioritizedQueue.Insert(priority, std::deque<IECPresentationTaskPtr>()).first;
    iter->second.push_back(&task);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationTaskPtr ECPresentationTasksQueue::_Pop(IECPresentationTask::Predicate const& filter)
    {
    auto scope = Diagnostics::Scope::Create("Tasks queue: pop task");
    BeMutexHolder lock(m_mutex);
    if (m_prioritizedQueue.empty())
        {
        DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, "Queue empty");
        return nullptr;
        }

    // note: m_prioritizedQueue is sorted from highest priority to lowest
    for (auto priorityBucketIter = m_prioritizedQueue.begin(); priorityBucketIter != m_prioritizedQueue.end(); ++priorityBucketIter)
        {
        auto& priorityBucket = *priorityBucketIter;
        for (auto taskIter = priorityBucket.second.begin(); taskIter != priorityBucket.second.end(); ++taskIter)
            {
            IECPresentationTaskPtr task = *taskIter;
            if (!filter || filter(*task))
                {
                priorityBucket.second.erase(taskIter);
                if (priorityBucket.second.empty())
                    m_prioritizedQueue.erase(priorityBucketIter);
                DIAGNOSTICS_DEV_LOG(DiagnosticsCategory::Tasks, LOG_TRACE, Utf8PrintfString("Returning `%s`", task->GetId().ToString().c_str()));
                return task;
                }
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IECPresentationTaskPtr> ECPresentationTasksQueue::_Get(IECPresentationTask::Predicate const& filter) const
    {
    auto scope = Diagnostics::Scope::Create("Tasks queue: get filtered tasks");
    BeMutexHolder lock(m_mutex);
    bvector<IECPresentationTaskPtr> tasks;
    for (auto priorityBucketIter = m_prioritizedQueue.begin(); priorityBucketIter != m_prioritizedQueue.end(); ++priorityBucketIter)
        {
        auto& priorityBucket = *priorityBucketIter;
        for (IECPresentationTaskPtr task : priorityBucket.second)
            {
            if (!filter || filter(*task))
                tasks.push_back(task);
            }
        }
    return tasks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TasksCancelationResult ECPresentationTasksQueue::_Cancel(IECPresentationTask::Predicate const& pred)
    {
    auto scope = Diagnostics::Scope::Create("Tasks queue: cancel");
    BeMutexHolder lock(m_mutex);
    bset<IECPresentationTaskCPtr> matchingTasks;
    bvector<int> emptyBuckets;
    for (auto& bucket : m_prioritizedQueue)
        {
        CancelTasks(matchingTasks, bucket.second, pred, true);
        if (bucket.second.empty())
            emptyBuckets.push_back(bucket.first);
        }
    for (int bucketId : emptyBuckets)
        m_prioritizedQueue.erase(bucketId);
    return TasksCancelationResult(matchingTasks);
    }
