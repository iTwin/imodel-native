#include "ScalableMeshPCH.h" 
#include "../ImagePPHeaders.h"
#include "LightThreadPool.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


std::recursive_mutex s_nodeMapLock;
std::map<void*, std::atomic<unsigned int>> s_nodeMap;

bool TryReserveNodes(std::map<void*, std::atomic<unsigned int>>& map, void** reservedNodes, size_t nNodesToReserve, unsigned int id)
    {
    bool isReserved = true;
    for (size_t i = 0; i < nNodesToReserve && isReserved; ++i)
        {
        unsigned int val = (unsigned int)-1;
        if (!map[reservedNodes[i]].compare_exchange_weak(val,id)) isReserved = false;
        }
    if (!isReserved) 
        for (size_t i = 0; i < nNodesToReserve; ++i)
            {
            unsigned int val = (unsigned int)-1;
            map[reservedNodes[i]].compare_exchange_strong(id, val);
            }
    return isReserved;
    }

std::thread s_threads[8];
std::atomic<bool> s_areThreadsBusy[8];
void SetThreadAvailableAsync(size_t threadId)
    {
    std::atomic<bool>* areThreadsBusy = s_areThreadsBusy;
    std::thread* threadP = s_threads;
    std::thread t = std::thread([areThreadsBusy, threadId, threadP] ()
        {
        if(threadP[threadId].joinable()) threadP[threadId].join();
        bool expected = true;
        areThreadsBusy[threadId].compare_exchange_strong(expected, false);
        assert(expected);
        });
    t.detach();
    }


void RunOnNextAvailableThread(std::function<void(size_t threadId)> lambda)
    {
    bool wait = true;
    while (wait)
        {
        for (size_t t = 0; t < 8; ++t)
            {
            bool expected = false;
            if (s_areThreadsBusy[t].compare_exchange_weak(expected, true))
                {
                if (s_threads[t].joinable()) continue;
                wait = false;
                s_threads[t] = std::thread(std::bind(lambda, t));
                break;
                }
            }
        }
    }


void WaitForThreadStop()
    {
    bool notAllThreadsStopped = true;
    while (notAllThreadsStopped)
        {
        volatile int n = 0;
        std::thread* arrayT = s_threads;
        volatile uint64_t ptr = (uint64_t)arrayT;
        ptr = ptr;
        for (size_t t = 0; t < 8; ++t)
            {
            if (!s_areThreadsBusy[t] || !s_threads[t].joinable()) ++n;
            }
        if (n == 8) notAllThreadsStopped = false;
        else         std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    for (size_t t = 0; t < 8; ++t)
        {
        s_areThreadsBusy[t] = false;

        if (s_threads[t].joinable())
            s_threads[t].join();

        s_threads[t] = std::thread();
        }
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE