#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

//#define TRACE_ON 1
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
    GRAPH_STORE,
    POOL_REPLACEITEM,
    WORKER_MESH_TASK,
    WORKER_FILTER_TASK,
    WORKER_STITCH_TASK,
    WORKER_STITCH_TASK_NEIGHBOR,
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
    bool started;
    Utf8String m_logDirectory; 
    bool m_outputObjLog; 

    CachedDataEventTracer();
    
    bool filter(TraceEvent& e);
    
public:

    BENTLEY_SM_EXPORT static CachedDataEventTracer* GetInstance();

    BENTLEY_SM_EXPORT void setLogDirectory(const Utf8String& logDir);

    BENTLEY_SM_EXPORT void setOutputObjLog(bool outputObjLog);
    
    BENTLEY_SM_EXPORT void start();
        
    BENTLEY_SM_EXPORT void logEvent(TraceEvent e);
        
    BENTLEY_SM_EXPORT void analyze(int processId = -1);    
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