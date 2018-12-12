#include "ScalableMeshPCH.h" 
#include "../ImagePPHeaders.h"
#include <ScalableMesh/IScalableMeshProgress.h>
#include "LightThreadPool.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


LightThreadPool::LightThreadPool()
    {
    m_nbThreads = std::max((int)1, (int)std::thread::hardware_concurrency() - 2);
    m_threads = new std::thread[m_nbThreads];
    m_areThreadsBusy = new std::atomic<bool>[m_nbThreads];                    

    for (size_t threadInd = 0; threadInd < m_nbThreads; threadInd++)        
        { 
        m_areThreadsBusy[threadInd] = false;        
        }
    }

LightThreadPool::~LightThreadPool()
    {
    delete [] m_threads;
    delete [] m_areThreadsBusy;
    }


LightThreadPool* LightThreadPool::GetInstance()
    {
    if (s_pool == nullptr)
        { 
        s_pool = new LightThreadPool;
        }

    return s_pool;
    }


LightThreadPool* LightThreadPool::s_pool = nullptr;
    
/*
extern int LightThreadPool::GetInstance()->m_nbThreads = std::max((int)1, (int)std::thread::hardware_concurrency() - 2);
extern std::thread LightThreadPool::GetInstance()->m_threads[LightThreadPool::GetInstance()->m_nbThreads];
extern std::atomic<bool> LightThreadPool::GetInstance()->m_areThreadsBusy[LightThreadPool::GetInstance()->m_nbThreads];
extern std::recursive_mutex s_nodeMapLock;
extern std::map<void*, std::atomic<unsigned int>> s_nodeMap;
*/


bool TryReserveNodes(std::map<void*, std::shared_ptr<std::atomic<unsigned int>>>& map, void** reservedNodes, size_t nNodesToReserve, unsigned int id)
    {
    bool isReserved = true;
    for (size_t i = 0; i < nNodesToReserve && isReserved; ++i)
        {
        unsigned int val = (unsigned int)-1;
        if (map.count(reservedNodes[i]) == 0 || !map[reservedNodes[i]]->compare_exchange_weak(val,id)) isReserved = false;
        }
    if (!isReserved) 
        for (size_t i = 0; i < nNodesToReserve; ++i)
            {
            unsigned int val = (unsigned int)-1;
            if(map.count(reservedNodes[i]) == 0)
            {
                map[reservedNodes[i]] = std::make_shared<std::atomic<unsigned int>>();
                *map[reservedNodes[i]] = (unsigned int)-1;
            }
            map[reservedNodes[i]]->compare_exchange_strong(id, val);
            }
    return isReserved;
    }


void SetThreadAvailableAsync(size_t threadId)
    {
    std::atomic<bool>* areThreadsBusy = LightThreadPool::GetInstance()->m_areThreadsBusy;
    /*std::thread* threadP = LightThreadPool::GetInstance()->m_threads;
    std::thread t = std::thread([areThreadsBusy, threadId, threadP] ()
        {
        if(threadP[threadId].joinable()) threadP[threadId].join();*/
        bool expected = true;
        areThreadsBusy[threadId].compare_exchange_strong(expected, false);
        assert(expected);
    /*    });
    t.detach();*/
    }


void RunOnNextAvailableThread(std::function<void(size_t threadId)> lambda)
    {
    bool wait = true;
    while (wait)
        {
//#ifndef NDEBUG
//        static std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
//       std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
//        if (std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() > 500)
//            {
//            int nBusyThreads = 0;
//            for (size_t t = 0; t < LightThreadPool::GetInstance()->m_nbThreads; ++t)
//                if (LightThreadPool::GetInstance()->m_areThreadsBusy[t]) ++nBusyThreads;
//            std::cout << nBusyThreads << std::endl;
//            start_time = std::chrono::steady_clock::now();
//            }
//
//#endif
        for (size_t t = 0; t < LightThreadPool::GetInstance()->m_nbThreads; ++t)
            {
            bool expected = false;
            if (LightThreadPool::GetInstance()->m_areThreadsBusy[t].compare_exchange_weak(expected, true))
                {
                if (LightThreadPool::GetInstance()->m_threads[t].joinable()) LightThreadPool::GetInstance()->m_threads[t].join();
                wait = false;
                LightThreadPool::GetInstance()->m_threads[t] = std::thread(std::bind(lambda, t));
                break;
                }
            }
        }
    }


void WaitForThreadStop(IScalableMeshProgress* p)
    {
    bool notAllThreadsStopped = true;
    while (notAllThreadsStopped)
        {
          int n = 0;
//        std::thread* arrayT = LightThreadPool::GetInstance()->m_threads;
//        volatile uint64_t ptr = (uint64_t)arrayT;
//        ptr = ptr;
		if (p != nullptr) p->UpdateListeners();
        for (size_t t = 0; t < LightThreadPool::GetInstance()->m_nbThreads; ++t)
            {
            if (!LightThreadPool::GetInstance()->m_areThreadsBusy[t] || !LightThreadPool::GetInstance()->m_threads[t].joinable()) ++n;
            }
        if (n == LightThreadPool::GetInstance()->m_nbThreads) notAllThreadsStopped = false;
        else         std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    for (size_t t = 0; t < LightThreadPool::GetInstance()->m_nbThreads; ++t)
        {
        LightThreadPool::GetInstance()->m_areThreadsBusy[t] = false;

        if (LightThreadPool::GetInstance()->m_threads[t].joinable())
            LightThreadPool::GetInstance()->m_threads[t].join();

        LightThreadPool::GetInstance()->m_threads[t] = std::thread();
        }
    }

WorkerThreadPool::WorkerThreadPool(int numWorkingThreads)
    : m_numWorkingThreads(numWorkingThreads)
    {
    m_futures = new std::future<void>[m_numWorkingThreads];
    
    m_currentWorkInd = 0;
    m_run = false;
    m_threadRun = new atomic<bool>[m_numWorkingThreads];

    for (int i = 0; i < m_numWorkingThreads; i++)
        m_threadRun[i] = false;

    m_numberOfTask = 0;
    }

WorkerThreadPool::~WorkerThreadPool()
    {
    WaitAndStop();

    delete[] m_futures;
    delete[] m_threadRun;
    }

void WorkerThreadPool::ClearQueueWork()
    {
    m_workQueueMutex.lock();
    m_currentWorkInd = 0;
    m_workQueue.clear();
    m_workQueueMutex.unlock();
    }

void WorkerThreadPool::QueueWork(WorkItemPtr& work)
    {    
    m_workQueueMutex.lock();
    m_workQueue.push_back(work);
    m_workQueueMutex.unlock();
    m_numberOfTask++;
    }

void WorkerThreadPool::Start(IActiveWait* activeWait)
    {
    m_activeWait = activeWait;

    if (m_run == false)
        {
        m_run = true;
        assert(m_currentWorkInd == 0);

        //Launch a group of threads
        for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId)
            {
            m_futures[threadId] = std::async(&WorkerThreadPool::WorkThread, this, threadId);
            }
        }
    }

void WorkerThreadPool::WaitAndStop()
    {
    std::chrono::milliseconds span(500);

    for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId)
        {
        if (m_futures[threadId].valid())
            {
            if (m_activeWait == nullptr)
                {
                m_futures[threadId].get();
                }
            else
                {
                while (m_futures[threadId].wait_for(span) == std::future_status::timeout)
                    {
                    m_activeWait->Progress();
                    }
                }
            }        
        }

    m_run = false;
    }

void WorkerThreadPool::WorkThread(int threadId)
    {    
    WorkItemPtr currentWork;
    int i;

    do
        {
        currentWork = nullptr;        

        m_workQueueMutex.lock();
        if (m_workQueue.size() > 0)
            { 
            currentWork = m_workQueue.back();
            m_workQueue.pop_back();
            m_threadRun[threadId] = true;
            m_numberOfTask--;
            }

        m_workQueueMutex.unlock();
        
        if (currentWork.IsValid())
            {                      
            currentWork->_DoWork();                            
            m_threadRun[threadId] = false;
            }

        i = 0;

        for (; i < m_numWorkingThreads; i++)
            if (m_threadRun[i] == true) break;
    
        } while (m_run && (i < m_numWorkingThreads || m_numberOfTask > 0));
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE