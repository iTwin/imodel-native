/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
| Notice: This file is an adaptation of <DgnPlatform\RealityDataCache.h> which I was not sure we can use directly.
|         see original file as a reference...
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/BeThread.h>
#include <Bentley/DateTime.h>

#include "PCThreadUtilities.h"
#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>

//__PUBLISH_SECTION_END__
GROUND_DETECTION_TYPEDEF(PointCloudThread)
GROUND_DETECTION_TYPEDEF(PointCloudThreadPool)
GROUND_DETECTION_TYPEDEF(PointCloudWork)
GROUND_DETECTION_TYPEDEF(PointCloudWorkerThread)

GROUND_DETECTION_TYPEDEF(GroundDetectionWork)
GROUND_DETECTION_TYPEDEF(GroundDetectionThreadPool)



//__PUBLISH_SECTION_START__

BEGIN_GROUND_DETECTION_NAMESPACE

//__PUBLISH_SECTION_END__

struct HasWorkOrTerminatesPredicate;
struct IsIdlePredicate;
struct AllThreadsIdlePredicate;
struct ThreadPoolWorkDonePredicate;
//__PUBLISH_SECTION_START__

//__PUBLISH_SECTION_END__
/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
enum class SchedulingMethod
    {
    FIFO,   // queue
    LIFO    // stack
    };

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
template<typename T> struct ThreadSafeQueue
{
    struct Item;
    typedef RefCountedPtr<Item> ItemPtr;
    struct Item : RefCountedBase
        {
        ItemPtr prev;
        ItemPtr next;
        T       data;
        Item(T d, Item* p, Item* n) : prev(p), next(n), data(d) {}
        static ItemPtr Create(T d, Item* p, Item* n) {return new Item(d, p, n);}
        };

    struct Iterator
    {
    friend struct ThreadSafeQueue<T>;
    private:
        Item* m_curr;
        Iterator(Item* first) : m_curr(first) {}
    public:
        bool IsValid() const {return nullptr != m_curr;}
        Iterator& operator++() {m_curr = m_curr->next.get(); return *this;}
        T const& operator*() {return m_curr->data;}
        bool operator!=(Iterator const& other) const {return m_curr != other.m_curr;}
        bool operator==(Iterator const& other) const {return m_curr == other.m_curr;}
    };

private:
    mutable BeConditionVariable m_cv;
    ItemPtr m_first;
    ItemPtr m_last;
    unsigned m_count;
    SchedulingMethod m_schedulingMethod;

private:
    bool PopBack(T* element);
    bool PopFront(T* element);
    bool Pop(T* element);
    
public:
    ThreadSafeQueue(SchedulingMethod defaultSchedulingMethod = SchedulingMethod::FIFO) : m_count(0), m_schedulingMethod(defaultSchedulingMethod) {}
    void PushBack(T element);
    void PushFront(T element);
    bool Pop(T& element);
    bool PopBack(T& element);
    bool PopFront(T& element);
    bool Pop();
    bool PopBack();
    bool PopFront();
    bool IsEmpty() const;
    unsigned Size() const;
    BeConditionVariable& GetConditionVariable() const {return m_cv;}
    Iterator begin() const;
    Iterator end() const;
    void Erase(Iterator const& iterator);
    void Clear();
};

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct PointCloudThread : RefCountedBase
{
private:
    intptr_t    m_threadId;
    Utf8String  m_threadName;
    //DgnPlatformLib::Host* m_hostToAdopt;
    
private:
    static void ThreadRunner(void* arg);
#if defined(__unix__)
    static void* PlatformThreadRunner(void* arg) {ThreadRunner(arg); return NULL;}
#else
    static unsigned __stdcall PlatformThreadRunner(void* arg) {ThreadRunner(arg); return 0;}
#endif
    
protected:
    PointCloudThread(/*Bentley::DgnPlatform::DgnPlatformLib::Host* hostToAdopt,*/Utf8CP threadName = NULL);
    ~PointCloudThread();
    virtual void _Run() = 0;

public:
    intptr_t GetThreadId() const;
    void     Run();   //!< Call this to invoke the _Run method in the current thread
    void     Start(); //!< Call this to start the thread
};

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct PointCloudWork : IRefCounted
{
friend struct PointCloudWorkerThread;
size_t          GetMemorySize() { return _GetMemorySize(); }
protected:
    virtual void    _DoWork() = 0;
    virtual size_t  _GetMemorySize() = 0;

public:

    void DoWork() { _DoWork(); }
};

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct GroundDetectionWork : RefCountedBase
{
friend struct GroundDetectionThreadPool;
//size_t          GetMemorySize() { return _GetMemorySize(); }
protected:
    virtual void    _DoWork() = 0;
    //virtual size_t  _GetMemorySize() = 0;

public: 

    void DoWork() { _DoWork(); };
};


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct PointCloudWorkerThread : PointCloudThread
{
    friend struct PointCloudThreadPool;
    friend struct HasWorkOrTerminatesPredicate;
    friend struct IsIdlePredicate;
        
    struct IStateListener : IRefCounted
    {
    friend struct PointCloudWorkerThread;
    protected:
        virtual void _OnThreadBusy(PointCloudWorkerThread& thread) {};
        virtual void _OnThreadIdle(PointCloudWorkerThread& thread) {};
    };
    typedef RefCountedPtr<IStateListener> IStateListenerPtr;

private:
    mutable BeConditionVariable m_cv;
    bool                        m_terminate;
    PointCloudWorkPtr           m_currentWork;
    uint64_t                    m_idleSince;
    uint64_t                    m_busySince;
    IStateListenerPtr           m_stateListener;
    size_t                      m_memoryUsed;

    void SetIsBusy(bool);

protected:
    PointCloudWorkerThread(/*Bentley::DgnPlatform::DgnPlatformLib::Host* hostToAdopt,*/IStateListener* stateListener, Utf8CP threadName);
    ~PointCloudWorkerThread();
    virtual void _OnBusy();
    virtual void _OnIdle();
    virtual void _DoWork(PointCloudWork& work);
    virtual void _Run() override;

protected:
    bool TerminateRequested() const;

public:
            PointCloudWork& DoWork(PointCloudWork& work) { _DoWork(work); return work; }
    bool IsBusy(uint64_t* busyTime = NULL) const;
    bool IsIdle(uint64_t* idleTime = NULL) const;
    void   SetMemoryUsed(size_t memoryUsed) { m_memoryUsed = memoryUsed; }
    size_t GetMemoryUsed() const { return m_memoryUsed; }
    void Terminate();
    //! Create a new PointCloudWorkerThread thread.
    //! note This function does not start the new thread. You must call the Start method on the returned thread object in order to start it.
    static PointCloudWorkerThreadPtr Create(/*Bentley::DgnPlatform::DgnPlatformLib::Host* hostToAdopt,*/ IStateListener* stateListener = NULL, Utf8CP threadName = NULL) { return new PointCloudWorkerThread(/*hostToAdopt, */stateListener, threadName); }
};

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct PointCloudWorkQueue : ThreadSafeQueue<PointCloudWorkPtr>
    {
    PointCloudWorkQueue(SchedulingMethod defaultSchedulingMethod = SchedulingMethod::FIFO) :ThreadSafeQueue<PointCloudWorkPtr>(defaultSchedulingMethod) {}
    ~PointCloudWorkQueue() {}
    };


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PointCloudThreadPool : RefCounted<PointCloudWorkerThread::IStateListener>
{       
    friend struct AllThreadsIdlePredicate;
    friend struct ThreadPoolWorkDonePredicate;
    friend struct ThreadPoolQueueNotFullPredicate;

private:
    int                                 m_maxIdleThreads;
    int                                 m_maxThreads;
    bool                                m_isTerminating;
    mutable BeConditionVariable         m_threadsCV;
    mutable BeMutex                     m_workQueueCS;
    mutable BeMutex                     m_memoryUsedCS;
    bmap<PointCloudWorkerThread*, bool> m_threads;
    PointCloudWorkQueue                 m_workQueue;
    size_t                              m_memoryUsed;
    
protected:
    PointCloudThreadPool(int maxThreads, int maxIdleThreads, SchedulingMethod schedulingMethod)
        : m_workQueue(schedulingMethod), m_maxThreads(maxThreads), m_maxIdleThreads(maxIdleThreads), m_isTerminating(false), m_memoryUsed(0)
        {
        }
    PointCloudWorkQueue& GetQueue() { return m_workQueue; }
    virtual void _OnThreadBusy(PointCloudWorkerThread& thread) override;
    virtual void _OnThreadIdle(PointCloudWorkerThread& thread) override;
    virtual bool _AssignWork(PointCloudWorkerThread& thread);
    PointCloudWorkerThreadPtr CreateThread();
    PointCloudWorkerThreadPtr GetIdleThread() const;
    bool ShouldCreateNewThread() const;
    bool IsTerminating() const {return m_isTerminating;}

    bool   IsMemoryLimitReached(size_t memoryUsed) const;
    void   SetMemoryUsed(size_t memoryUsed);
    size_t GetMemoryUsed() const;                

public:
    static PointCloudThreadPoolPtr Create(int maxThreads, int maxIdleThreads, SchedulingMethod schedulingMethod) { return new PointCloudThreadPool(maxThreads, maxIdleThreads, schedulingMethod); }
    virtual ~PointCloudThreadPool();
    int  GetThreadsCount() const;
    void QueueWork(PointCloudWork& work);
    void WaitUntilAllThreadsIdle() const;
    bool WaitUntilWorkDone(uint32_t timeoutMillis) const;
    void WaitUntilQueueIsNotFull() const;
    void Terminate();
};

/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     09/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE GroundDetectionThreadPool : public RefCountedBase //: RefCounted<PointCloudWorkerThread::IStateListener>
    {           
    struct IActiveWait  
        {
        virtual void Progress() = 0;         
        };

    /*
    friend struct AllThreadsIdlePredicate;
    friend struct ThreadPoolWorkDonePredicate;
    friend struct ThreadPoolQueueNotFullPredicate;
    */

private:

    //int                                 m_maxIdleThreads;
    int                                 m_numWorkingThreads;    
    atomic<bool>                        m_run;
    //mutable BeConditionVariable         m_threadsCV;
    //mutable BeMutex                     m_workQueueCS;
    //mutable BeMutex                     m_memoryUsedCS;    
    std::future<void>*                    m_futures;

    //bmap<PointCloudWorkerThread*, bool> m_threads;
    //GroundDetectionWorkQueue            m_workQueue;
    bvector<GroundDetectionWorkPtr>       m_workQueue;    
    std::mutex                            m_workQueueMutex;
    std::atomic<uint32_t>                 m_currentWorkInd; 
    IActiveWait*                          m_activeWait; 
    //size_t                              m_memoryUsed;

    
protected:

    GroundDetectionThreadPool(int numWorkingThreads);

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

    static GroundDetectionThreadPoolPtr Create(int maxThreads) { return new GroundDetectionThreadPool(maxThreads); }
    virtual ~GroundDetectionThreadPool();
    //int  GetThreadsCount() const;

    void ClearQueueWork();

    void QueueWork(GroundDetectionWorkPtr& work);    
   
    void Start(IActiveWait* activeWait = nullptr);

    void WaitAndStop();

    //void WaitUntilAllThreadsIdle() const;
    //bool WaitUntilWorkDone(uint32_t timeoutMillis) const;
    //void WaitUntilQueueIsNotFull() const;
    //void Terminate();
    };

//__PUBLISH_SECTION_START__

END_GROUND_DETECTION_NAMESPACE
