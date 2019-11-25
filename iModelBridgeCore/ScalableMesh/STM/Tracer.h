/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>

//#define TRACE_ON 1

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

enum class EventType
{
    LOAD_MESH_CREATE_0,
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
    POOL_DELETEITEM,	
    POOL_REPLACEITEM,
    POOL_CHANGESIZEITEM,
    REFCT_ADDREF,
    REFCT_DECREF,
    BEFORE_SWITCH_VIDEO_TEX,
    GRAPH_STORE,
    WORKER_MESH_TASK,
    WORKER_FILTER_TASK,
    WORKER_STITCH_TASK,
    WORKER_STITCH_TASK_NEIGHBOR,
    CLOUDDATASOURCE_LOAD,
    START_NEWQUERY,
    QUERY_LOADNODELIST,
    QUERY_NUMBER_OF_OVERVIEWS,
    START_NEWQUERY_FOUNDNODES,
    START_NEWQUERY_SEARCHNODES,
    START_NEWQUERY_FIND,
    START_NEWQUERY_COLLECT,
    START_NEWQUERY_FINDLOADED,
    START_NEWQUERY_COLLECTCLIPS,
    START_NEWQUERY_CHECKCLIPS
};

struct TraceEvent
{
    std::thread::id threadId;
    EventType typeOfEvent;
    clock_t timestamp;
    uint64_t nodeId;
    uint64_t texId;
    uint64_t meshId;
    uint32_t refCount;
    uint64_t objVal;
    uint64_t poolId;
    int64_t objectSize;
    std::string stackTrace;
};


extern std::map<EventType, std::string> __TRACEPOINT__typeDesc;
extern std::map<EventType, bool> __TRACEPOINT__typeToFilter;

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
    
public:

    BENTLEY_SM_EXPORT static CachedDataEventTracer* GetInstance();

    BENTLEY_SM_EXPORT void setLogDirectory(const Utf8String& logDir);

    BENTLEY_SM_EXPORT void setOutputObjLog(bool outputObjLog);
    
    BENTLEY_SM_EXPORT void start();
        
    BENTLEY_SM_EXPORT void logEvent(TraceEvent e);
        
    BENTLEY_SM_EXPORT void analyze(int processId = -1);    
};

#if TRACE_ON
#define TRACEPOINT(type,id,meshid,texid,poolid,val,rc) \
{  \
if (__TRACEPOINT__typeToFilter[type]) { \
TraceEvent __TRACEPOINT__event; \
__TRACEPOINT__event.typeOfEvent = (type); \
__TRACEPOINT__event.nodeId = (id); \
__TRACEPOINT__event.meshId = (meshid); \
__TRACEPOINT__event.texId = (texid); \
__TRACEPOINT__event.poolId =(poolid); \
__TRACEPOINT__event.objVal = (uint64_t)(val); \
__TRACEPOINT__event.refCount = (rc); \
__TRACEPOINT__event.threadId = std::this_thread::get_id(); \
__TRACEPOINT__event.timestamp = clock(); \
CachedDataEventTracer::GetInstance()->logEvent(__TRACEPOINT__event);  } \
}

#define TRACEPOINTSize(type,id,meshid,texid,poolid,val,rc,objSize) \
{  \
if (__TRACEPOINT__typeToFilter[type]) { \
TraceEvent __TRACEPOINT__event; \
__TRACEPOINT__event.typeOfEvent = (type); \
__TRACEPOINT__event.nodeId = (id); \
__TRACEPOINT__event.meshId = (meshid); \
__TRACEPOINT__event.texId = (texid); \
__TRACEPOINT__event.poolId =(poolid); \
__TRACEPOINT__event.objVal = (uint64_t)(val); \
__TRACEPOINT__event.refCount = (rc); \
__TRACEPOINT__event.threadId = std::this_thread::get_id(); \
__TRACEPOINT__event.timestamp = clock(); \
__TRACEPOINT__event.objectSize = objSize; \
CachedDataEventTracer::GetInstance()->logEvent(__TRACEPOINT__event);  } \
}
#else
#define TRACEPOINT(type,id,meshid,texid,poolid,val,rc) \
{  \
}

#define TRACEPOINTSize(type,id,meshid,texid,poolid,val,rc,objSize) \
{  \
}
#endif

END_BENTLEY_SCALABLEMESH_NAMESPACE

USING_NAMESPACE_BENTLEY_SCALABLEMESH
