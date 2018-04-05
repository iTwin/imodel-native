/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/Threading/LightThreadPool.h $
|    $RCSfile: LightThreadPool.h,v $
|   $Revision: 1.0 $
|       $Date: 2015/10/29 15:38:13 $
|     $Author: Elenie.Godzaridis $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/IScalableMesh.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


class LightThreadPool
    {
    public: 
    
        int                                        m_nbThreads;
        std::thread*                               m_threads;
        std::atomic<bool>*                         m_areThreadsBusy;
        std::recursive_mutex                       m_nodeMapLock;
        std::map<void*, std::atomic<unsigned int>> m_nodeMap;

        LightThreadPool();

        virtual ~LightThreadPool();
        
        static LightThreadPool* GetInstance();            

    private:

        static LightThreadPool* s_pool;

    };



bool TryReserveNodes(std::map<void*, std::atomic<unsigned int>>& map, void** reservedNodes, size_t nNodesToReserve, unsigned int id);
void SetThreadAvailableAsync(size_t threadId);
void RunOnNextAvailableThread(std::function<void(size_t threadId)> lambda);
void WaitForThreadStop(IScalableMeshProgress* p =nullptr);


struct RasterTexturingThreadPool;
typedef RefCountedPtr<RasterTexturingThreadPool> RasterTexturingThreadPoolPtr;

struct RasterTexturingWork;
typedef RefCountedPtr<RasterTexturingWork> RasterTexturingWorkPtr;


struct RasterTexturingWork : RefCountedBase
{
    friend struct RasterTexturingThreadPool;
    //size_t          GetMemorySize() { return _GetMemorySize(); }
protected:
    virtual void    _DoWork() = 0;
    //virtual size_t  _GetMemorySize() = 0;

public:

    void DoWork() { _DoWork(); };
};


struct RasterTexturingThreadPool : public RefCountedBase //: RefCounted<PointCloudWorkerThread::IStateListener>
{
    struct IActiveWait
    {
        virtual void Progress() = 0;
    };

private:

    //int                                 m_maxIdleThreads;
    int                                 m_numWorkingThreads;
    atomic<bool>                        m_run;
    atomic<bool>*                       m_threadRun;
    atomic<int>                         m_numberOfTask;
    //mutable BeConditionVariable         m_threadsCV;
    //mutable BeMutex                     m_workQueueCS;
    //mutable BeMutex                     m_memoryUsedCS;    
    std::future<void>*                    m_futures;

    //bmap<PointCloudWorkerThread*, bool> m_threads;
    //GroundDetectionWorkQueue            m_workQueue;
    bvector<RasterTexturingWorkPtr>       m_workQueue;
    std::mutex                            m_workQueueMutex;
    std::atomic<uint32_t>                 m_currentWorkInd;
    IActiveWait*                          m_activeWait;
    //size_t                              m_memoryUsed;


protected:

    RasterTexturingThreadPool(int numWorkingThreads);

    void WorkThread(/*DgnPlatformLib::Host* hostToAdopt, */int threadId);

    //GroundDetectionWorkQueue& GetQueue() { return m_workQueue; }
    /*
    virtual void _OnThreadBusy(PointCloudWorkerThread& thread) override;
    virtual void _OnThreadIdle(PointCloudWorkerThread& thread) override;
    virtual bool _AssignWork(PointCloudWorkerThread& thread);
    */

    //PointCloudWorkerThreadPtr CreateThread();
    //PointCloudWorkerThreadPtr GetIdleThread() const;
    //bool ShouldCreateNewThread() const;
    //bool IsTerminating() const {return m_isTerminating;}

    //bool   IsMemoryLimitReached(size_t memoryUsed) const;
    //void   SetMemoryUsed(size_t memoryUsed);
    //size_t GetMemoryUsed() const;                

public:

    static RasterTexturingThreadPoolPtr Create(int maxThreads) { return new RasterTexturingThreadPool(maxThreads); }
    virtual ~RasterTexturingThreadPool();
    //int  GetThreadsCount() const;

    void ClearQueueWork();

    void QueueWork(RasterTexturingWorkPtr& work);

    void Start(IActiveWait* activeWait = nullptr);

    void WaitAndStop();

    //void WaitUntilAllThreadsIdle() const;
    //bool WaitUntilWorkDone(uint32_t timeoutMillis) const;
    //void WaitUntilQueueIsNotFull() const;
    //void Terminate();
};

END_BENTLEY_SCALABLEMESH_NAMESPACE