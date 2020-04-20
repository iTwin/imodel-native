/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <numeric>
#include <folly/executors/InlineExecutor.h>
#include "TaskScheduler.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool TaskDependencies::Has(ITaskDependency const& dep) const
    {
    return std::any_of(begin(), end(), [&dep](auto const& containedDep) { return containedDep->Matches(dep); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
+---------------+---------------+---------------+---------------+---------------+------*/
bool TaskDependencies::Has(std::function<bool(ITaskDependency const&)> pred) const
    {
    return std::any_of(begin(), end(), [&pred](auto const& containedDep) { return pred(*containedDep); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                04/2020
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
* @bsimethod                                    Grigas.Petraitis                04/2020
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
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned ThreadsHelper::ComputeThreadsCount(TThreadAllocationsMap const& allocations)
    {
    return std::accumulate(allocations.begin(), allocations.end(), 0,
        [](unsigned sum, TThreadAllocationsMap::const_reference v){return sum + v.second;});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TExecutorFunc ThreadsHelper::WrapFollyExecutor(folly::Executor& executor)
    {
    return [&](folly::Func func) -> folly::Future<folly::Unit>
        {
        return folly::via(&executor, std::move(func));
        };
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
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
* @bsimethod                                    Grigas.Petraitis                08/2019
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
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Executor* ECPresentationTasksManager::CreateExecutor(TThreadAllocationsMap const& threadAllocations)
    {
    unsigned threadsCount = ThreadsHelper::ComputeThreadsCount(threadAllocations);
    if (threadsCount > 0)
        return new ECPresentationThreadPool(threadsCount);
    return new folly::InlineExecutor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationTasksScheduler* ECPresentationTasksManager::CreateScheduler(TThreadAllocationsMap const& threadAllocations, folly::Executor& executor)
    {
    return new ECPresentationTasksScheduler(threadAllocations, *new ECPresentationTasksQueue(), ThreadsHelper::WrapFollyExecutor(executor));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationTasksManager::ECPresentationTasksManager(TThreadAllocationsMap threadAllocations)
    {
    m_tasksExecutor = CreateExecutor(threadAllocations);
    ECPresentationTasksScheduler* scheduler = CreateScheduler(threadAllocations, *m_tasksExecutor);
    m_scheduler = scheduler;
    m_threadsCount = scheduler->GetThreadsCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationTasksManager::~ECPresentationTasksManager()
    {
    m_scheduler->Cancel(nullptr).GetCompletion().wait();
    m_scheduler->GetAllTasksCompletion().wait();
    DELETE_AND_CLEAR(m_scheduler);
    DELETE_AND_CLEAR(m_tasksExecutor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::SetThreadAllocationsMap(TThreadAllocationsMap allocations)
    {
    m_threadAllocations = allocations;
    m_threadsCount = ThreadsHelper::ComputeThreadsCount(m_threadAllocations);
    CheckTasks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::_Schedule(IECPresentationTaskR task)
    {
    m_queue->Add(task);
    CheckTasks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TThreadAllocationsMap ECPresentationTasksScheduler::CreateAvailableAllocationsMap() const
    {
    bvector<int> priorities;
    std::transform(m_runningTasks.begin(), m_runningTasks.end(), std::back_inserter(priorities), [](IECPresentationTaskPtr const& t){return t->GetPriority();});
    std::transform(m_pendingTasks.begin(), m_pendingTasks.end(), std::back_inserter(priorities), [](IECPresentationTaskPtr const& t) {return t->GetPriority(); });
    return ThreadsHelper::SubtractAllocations(m_threadAllocations, priorities);
    }

/*---------------------------------------------------------------------------------**//**
* Note: Must be called within a mutex
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationTask::Predicate ECPresentationTasksScheduler::CreateTasksFilter(TThreadAllocationsMap const& availableAllocationsMap) const
    {
    bvector<IECPresentationTask::Predicate> blockedTasksPredicates;
    for (IECPresentationTaskPtr const& task : m_pendingTasks)
        blockedTasksPredicates.push_back(task->GetBlockedTasksPredicate());
    for (IECPresentationTaskPtr const& task : m_runningTasks)
        blockedTasksPredicates.push_back(task->GetBlockedTasksPredicate());
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

        return true;
        };
    }

/*---------------------------------------------------------------------------------**//**
* Note: Must be called within a mutex
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPresentationTasksScheduler::CanExecute(IECPresentationTaskCR task) const
    {
    auto const& isTaskBlockedPredicate = task.GetBlockedTasksPredicate();
    if (!isTaskBlockedPredicate)
        return true;

    for (IECPresentationTaskPtr const& runningTask : m_runningTasks)
        {
        // we can't execute a task that wants to block an already running one
        if (isTaskBlockedPredicate(*runningTask))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Note: Must be called within a mutex
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationTaskPtr ECPresentationTasksScheduler::PopTask(IECPresentationTask::Predicate const& filter, TThreadAllocationsMap& availableThreadAllocationsMap)
    {
    // first we try the pending tasks list
    for (auto it=m_pendingTasks.begin(); it!=m_pendingTasks.end(); ++it)
        {
        auto pendingTask = *it;
        if (CanExecute(*pendingTask))
            {
            IECPresentationTaskPtr result = pendingTask;
            m_pendingTasks.erase(it);
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
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::CheckTasks()
    {
    BeMutexHolder lock(GetMutex());
    if (m_runningTasks.size() >= m_threadsCount)
        return;

    auto tasksToRun = m_threadsCount - m_runningTasks.size();
    auto availableAllocationsMap = CreateAvailableAllocationsMap();
    auto filter = CreateTasksFilter(availableAllocationsMap);
    while (tasksToRun--)
        {
        IECPresentationTaskPtr task = PopTask(filter, availableAllocationsMap);
        if (task.IsNull())
            {
            // scheduler has no more tasks available to run
            return;
            }
        // have to make sure the task we want to execute doesn't want to block
        // the running tasks; if it does - put it into the pending tasks list
        if (!CanExecute(*task))
            m_pendingTasks.push_back(task);
        else
            ExecuteTask(*task);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::ExecuteTask(IECPresentationTaskR task)
    {
    BeMutexHolder lock(GetMutex());
    m_runningTasks.insert(&task);
    m_executor([task = IECPresentationTaskPtr(&task)]
        {
        if (task->GetCancelationToken() && task->GetCancelationToken()->IsCanceled())
            return;
        task->Execute();
        })
    .ensure([&, task = IECPresentationTaskPtr(&task)]()
        {
        BeMutexHolder lock(GetMutex());
        m_runningTasks.erase(task);
        CheckTasks();
        task->Complete();
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
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
                task->Cancel();
                if (complete)
                    {
                    task->Complete();
                    tasksToRemove.insert(task);
                    }
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
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TasksCancelationResult ECPresentationTasksScheduler::_Cancel(IECPresentationTask::Predicate const& pred)
    {
    BeMutexHolder lock(GetMutex());
    bset<IECPresentationTaskCPtr> matchingTasks;

    // cancel queued tasks
    TasksCancelationResult queueCancelationResult = m_queue->Cancel(pred);
    for (IECPresentationTaskCPtr const& task : queueCancelationResult.GetTasks())
        matchingTasks.insert(task);

    // cancel pending tasks
    CancelTasks(matchingTasks, m_pendingTasks, pred, true);

    // cancel running tasks
    CancelTasks(matchingTasks, m_runningTasks, pred, false);

    return TasksCancelationResult(matchingTasks);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::_Block(IECPresentationTasksBlocker const* blocker)
    {
    BeMutexHolder lock(GetMutex());
    m_blockers.push_back(blocker);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                09/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksScheduler::_Unblock(IECPresentationTasksBlocker const* blocker)
    {
    BeMutexHolder lock(GetMutex());
    m_blockers.erase(std::find(m_blockers.begin(), m_blockers.end(), blocker), m_blockers.end());
    CheckTasks();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
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
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPresentationTasksQueue::_HasTasks() const
    {
    BeMutexHolder lock(m_mutex);
    return !m_prioritizedQueue.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void ECPresentationTasksQueue::_Add(IECPresentationTask& task)
    {
    BeMutexHolder lock(m_mutex);
    int priority = task.GetPriority();
    auto iter = m_prioritizedQueue.find(priority);
    if (m_prioritizedQueue.end() == iter)
        iter = m_prioritizedQueue.Insert(priority, std::deque<IECPresentationTaskPtr>()).first;
    iter->second.push_back(&task);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationTaskPtr ECPresentationTasksQueue::_Pop(IECPresentationTask::Predicate const& filter)
    {
    BeMutexHolder lock(m_mutex);
    if (m_prioritizedQueue.empty())
        return nullptr;

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
                return task;
                }
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<IECPresentationTaskPtr> ECPresentationTasksQueue::_Get(IECPresentationTask::Predicate const& filter) const
    {
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
* @bsimethod                                    Grigas.Petraitis                08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TasksCancelationResult ECPresentationTasksQueue::_Cancel(IECPresentationTask::Predicate const& pred)
    {
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
