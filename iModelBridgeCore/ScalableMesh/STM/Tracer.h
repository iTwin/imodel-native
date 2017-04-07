#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

enum EventType
{
    LOAD_MESH_CREATE_0 = 0,
    CACHED_MESH_ACQUIRE,
    LOAD_TEX_CREATE_0,
    CACHED_TEX_ACQUIRE,
    CACHED_MESH_RELEASE,
    CACHED_TEX_RELEASE,
    SWITCH_VIDEO_MESH,
    SWITCH_VIDEO_TEX,
    EVT_LOAD_NODE,
    UNLOAD_NODE,
    EVT_CREATE_DISPLAY_OVR_1,
    EVT_CREATE_DISPLAY_LOAD,
    EVT_CREATE_DISPLAY_OVR_PRELOAD,
    POOL_ADDITEM,
    POOL_REMOVEITEM,
    POOL_GETITEM,
    REFCT_ADDREF,
    REFCT_DECREF,
    BEFORE_SWITCH_VIDEO_TEX,
    POOL_DELETEITEM,
    TYPE_QTY
};

struct TraceEvent
{
    uint64_t threadId;
    int typeOfEvent;
    clock_t timestamp;
    uint64_t nodeId;
    uint64_t texId;
    uint64_t meshId;
    uint32_t refCount;
    uint64_t objVal;
    uint64_t poolId;
};


extern std::string typeDesc[(int)EventType::TYPE_QTY];

struct CachedDataEventTracer
{
private:
    static CachedDataEventTracer* s_instance;
    bvector<TraceEvent> ring;
    std::atomic<TraceEvent*> current;
    TraceEvent* end;
    int64_t valToFilter;
    bool typeToFilter[(int)EventType::TYPE_QTY] = { true, true, true, true,
        true, true, true, true,
        true, true, true, true,
        true, true, true, true, true, true, true, true };
    bool started;

    CachedDataEventTracer()
    {
        ring.resize(400000);
        current = ring.data();
        end = ring.data() + ring.size();
        started = false;
        valToFilter = -1;
    }

    bool filter(TraceEvent& e)
    {
        if (!typeToFilter[(int)e.typeOfEvent]) return false;
       /* if (valToFilter == -1 && e.texId != -1)
        {
            valToFilter = e.texId;
            return true;
        }*/
        return true;// valToFilter == e.texId;
    }

public:

    static CachedDataEventTracer* GetInstance()
    {
        if (s_instance == nullptr)
            s_instance = new CachedDataEventTracer();
        return s_instance;
    }

    void start()
    {
        started = true;
    }

    void logEvent(TraceEvent e)
    {
        if (!started) return;
        if (!filter(e)) return;
        TraceEvent* val = end;
        volatile bool test = false;
        if (test) analyze();
        TraceEvent* newPos = current++;
        current.compare_exchange_strong(val, ring.data());
        if (newPos >= end)newPos = ring.data();
        *newPos = e;
    }

    void analyze()
    {
        std::ofstream traceFile;
        traceFile.open("c:\\trace.log", std::ios_base::app);
        bmap<uint64_t, bvector<std::string>> eventsByVal;
        for (TraceEvent* init = ring.data(); init != current; ++init)
        {
            std::ostringstream str;
            str << std::to_string((unsigned long long) init->timestamp) << "[" << std::to_string(init->threadId) << "] >> " << typeDesc[(int)init->typeOfEvent] << " : " << std::to_string(init->nodeId)
                << " INSTANCE " << std::to_string(init->objVal) << " POOL ID " << std::to_string(init->poolId) << " MESH " << std::to_string(init->meshId) << " TEX " << std::to_string(init->texId) << " CT " << std::to_string(init->refCount) << std::endl;
            traceFile << str.str();
            eventsByVal[init->objVal].push_back(str.str());
        }
        traceFile.close();
        traceFile.open("c:\\traceByObj.log", std::ios_base::app);
        for (auto& obj : eventsByVal)
        {
            for (auto& str : obj.second)
                traceFile << str;

            traceFile << " ----------------- " << std::endl;
        }
        traceFile.close();
    }



};



#define THREAD_ID() ((uint64_t)std::hash<std::thread::id>()(std::this_thread::get_id()))


#if TRACE_ON
#define TRACEPOINT(threadt,type,id,meshid,texid,poolid,val, rc) \
{  \
TraceEvent e;\
e.typeOfEvent = (type); \
e.refCount = (rc); \
e.threadId = (uint64_t)(threadt);\
e.nodeId = (id); \
e.texId = (texid); \
e.meshId = (meshid); \
e.poolId =(poolid); \
e.objVal = (uint64_t)(val); \
e.timestamp = clock(); \
CachedDataEventTracer::GetInstance()->logEvent(e); \
}
#else
#define TRACEPOINT(thread,type,id,meshid,texid,poolid,val, rc) \
{  \
}
#endif

END_BENTLEY_SCALABLEMESH_NAMESPACE

USING_NAMESPACE_BENTLEY_SCALABLEMESH