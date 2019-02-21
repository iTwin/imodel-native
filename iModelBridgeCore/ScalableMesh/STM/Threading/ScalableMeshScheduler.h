/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Threading/ScalableMeshScheduler.h $
|    $RCSfile: ScalableMeshScheduler.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/09/10 15:38:13 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

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
        SMTask(std::function<void()> taskCallback) :m_callable(taskCallback)
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


    virtual void OnTaskRequested(ScalableMeshThreadPool* requestor);

    public:
        static const uint8_t PRIORITY_LOW = 2;
        static const uint8_t PRIORITY_DEFAULT = 3;
        static const uint8_t PRIORITY_HIGH = 6;
        static const uint8_t PRIORITY_IMMEDIATE = 10;
        ScalableMeshScheduler();
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
    ScalableMeshThreadPool(uint8_t nThreads);

    void RegisterTaskScheduler(Scheduler* sched);
    bool AssignTask(SMTask& task);
    bool TryAssignTask(SMTask& task);
    
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE