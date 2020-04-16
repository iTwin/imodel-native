/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMesh.h>
#include <queue>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE
class ScalableMeshThreadPool;

class SMTask
    {
    std::function<void()> m_callable;
    public:
        std::atomic<bool> executed;
        SMTask()
            {
            executed = false;
            }
        explicit SMTask(std::function<void()> taskCallback) :m_callable(taskCallback)
            {
            executed = false;
            };

        SMTask(std::function<bool()> taskCallback, bool value) :m_callable([taskCallback] () { taskCallback(); })
            {
            executed = false;
            };

        SMTask(std::function<int()> taskCallback, int value) :m_callable([taskCallback] () { taskCallback(); })
            {
            executed = false;
            };

        SMTask(const SMTask& task)
            {
            m_callable = task.m_callable;
            executed = false;
            }

        SMTask& operator=(const SMTask& task)
            {
            m_callable = task.m_callable;
            executed = false;
            return *this;
            }

        void Execute()
            {
            m_callable();
            };
    };

class Scheduler
    {
    public:
        virtual void OnTaskRequested(ScalableMeshThreadPool* requestor) = 0;
    };


class ScalableMeshScheduler : public Scheduler
    {
    static const uint8_t MAX_N_OF_PRIORITIES = 10;
    static const uint8_t DEFAULT_N_OF_THREADS = 5;
    friend class ScalableMeshThreadPool;
    ScalableMeshThreadPool* workers;
    std::queue<SMTask> unassignedTasks[MAX_N_OF_PRIORITIES];
    std::mutex sched_lock;


    virtual void OnTaskRequested(ScalableMeshThreadPool* requestor) override;

    public:
        static const uint8_t PRIORITY_LOW = 2;
        static const uint8_t PRIORITY_DEFAULT = 3;
        static const uint8_t PRIORITY_HIGH = 6;
        static const uint8_t PRIORITY_IMMEDIATE = 10;
        ScalableMeshScheduler();
        ScalableMeshScheduler(const ScalableMeshScheduler&) = delete;
        ScalableMeshScheduler& operator=(const ScalableMeshScheduler&) = delete;
        ScalableMeshScheduler(ScalableMeshScheduler&&) = delete;
        ScalableMeshScheduler& operator=(ScalableMeshScheduler&&) = delete;
        void ScheduleTask(SMTask& t, uint8_t priority=PRIORITY_DEFAULT);
    };

class ScalableMeshThreadPool
    {
    private:
    Scheduler* scheduler;
    std::vector<std::thread> workerThreads;
    std::vector<SMTask> assignedTasks;
    std::vector<uint8_t> freeWorkers;
    std::thread watcherThread;

    std::atomic<bool> taskAvailable;
    std::atomic<bool> hasWatcherThread;

    void _StartWork(uint8_t thread);
    void _StartWatcherThread();
    void _Watch();
    public:
    explicit ScalableMeshThreadPool(uint8_t nThreads);

    void RegisterTaskScheduler(Scheduler* sched);
    bool AssignTask(const SMTask& task);
    bool TryAssignTask(const SMTask& task);
    
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE