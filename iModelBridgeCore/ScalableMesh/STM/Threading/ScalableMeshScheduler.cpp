#include "ScalableMeshPCH.h"

#include "ScalableMeshScheduler.h"
#include <windows.h>
#ifndef NDEBUG
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
    {
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, char* threadName)
    {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
        {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {}
    }
#endif

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


ScalableMeshScheduler::ScalableMeshScheduler()
    {
    workers = new ScalableMeshThreadPool(DEFAULT_N_OF_THREADS);
    workers->RegisterTaskScheduler(this);
    for (size_t i = 0; i < MAX_N_OF_PRIORITIES; ++i) unassignedTasks[i] = std::queue<SMTask>();
    }

void ScalableMeshScheduler::OnTaskRequested(ScalableMeshThreadPool* requestor)
    {
    sched_lock.lock();
    for (uint8_t priority = MAX_N_OF_PRIORITIES-1; priority >= 0; priority--)
        {
        if (unassignedTasks[priority].size() > 0)
            {
            SMTask& t = unassignedTasks[priority].front();
            if (requestor->AssignTask(t))unassignedTasks[priority].pop();
            break;
            }
        }
    sched_lock.unlock();
    }

void ScalableMeshScheduler::ScheduleTask(SMTask& t, uint8_t priority)
    {
    sched_lock.lock();
    unassignedTasks[priority].push(t);
    for (uint8_t p = MAX_N_OF_PRIORITIES-1; p >= 0; p--)
        {
        if (unassignedTasks[p].size() > 0)
            {
            SMTask& t = unassignedTasks[p].front();
            if (workers->TryAssignTask(t))
                {
                unassignedTasks[p].pop();
                }
            break;
            }
        }
    sched_lock.unlock();
    }


ScalableMeshThreadPool::ScalableMeshThreadPool(uint8_t nThreads)
    {
    workerThreads.resize(nThreads);
    assignedTasks.resize(nThreads);
    for (uint8_t i = 0; i < nThreads; ++i)
        freeWorkers.push_back(i);
    hasWatcherThread = false;
    taskAvailable = false;
    }

void ScalableMeshThreadPool::_StartWatcherThread()
    {
    watcherThread = std::thread(std::bind(&ScalableMeshThreadPool::_Watch, this));
#ifndef NDEBUG
    DWORD ThreadId = ::GetThreadId(static_cast<HANDLE>(watcherThread.native_handle()));
    SetThreadName(ThreadId, "SM Watcher Thread");
#endif
    hasWatcherThread = true;
    }

void ScalableMeshThreadPool::_StartWork(uint8_t thread)
    {
    auto it = std::find(freeWorkers.begin(), freeWorkers.end(), thread);
    if (it != freeWorkers.end()) freeWorkers.erase(it);
    workerThreads[thread] = std::thread(std::bind(&SMTask::Execute, &assignedTasks[thread]));
#ifndef NDEBUG
    DWORD ThreadId = ::GetThreadId(static_cast<HANDLE>(workerThreads[thread].native_handle()));
    SetThreadName(ThreadId, "SM Worker Thread");
#endif
    }

void ScalableMeshThreadPool::_Watch()
    {
    if (taskAvailable)
        {
        if (freeWorkers.size() > 0)scheduler->OnTaskRequested(this);
        for (auto& task : assignedTasks)
            {
            if (task.executed)
                {
                freeWorkers.push_back(&task - &assignedTasks[0]);
                scheduler->OnTaskRequested(this);
                }
            }
        }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

void ScalableMeshThreadPool::RegisterTaskScheduler(Scheduler* sched)
    {
    scheduler = sched;
    }

bool ScalableMeshThreadPool::AssignTask(SMTask& task)
    {
    if (freeWorkers.size() == 0) return false;
    assignedTasks[freeWorkers.front()] = task;
    if(!hasWatcherThread) _StartWatcherThread();
    _StartWork(freeWorkers.front());
    return true;
    }

bool ScalableMeshThreadPool::TryAssignTask(SMTask& task)
    {
    if (!hasWatcherThread &&freeWorkers.size() > 0) return AssignTask(task);
    else taskAvailable = true;
    return false;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE