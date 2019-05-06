#include <ScalableMeshPCH.h>

#include "Tracer.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

std::string typeDesc[(int)EventType::TYPE_QTY] = { "LOAD (CREATE) MESH", " ACQUIRE CACHED MESH ", " LOAD (CREATE) TEX ", " ACQUIRE CACHED TEX " ,
"RELEASE CACHED MESH", "RELEASE CACHED TEX" ,"SWITCH TO VRAM(MESH)" ," SWITCH TO VRAM(TEX)", "LOAD NODE", "UNLOAD NODE", "OVERVIEW 1", "LOADNODEDISPLAYDATA", "PRELOADOVERVIEW",
"ADDITEM", "REMOVEITEM", "GETITEM", "INCREMENT", "DECREMENT", "BEFORE_SWITCH_VIDEO_TEX", "DELETEITEM","GRAPH_STORE", "POOL_REPLACEITEM", "WORKER_MESH_TASK", "WORKER_FILTER_TASK", "WORKER_STITCH_TASK", "WORKER_STITCH_TASK_NEIGHBOR",
"CLOUDDATASOURCE_LOAD", "START_NEWQUERY", "QUERY_LOADNODELIST", "QUERY_NUMBEROFOVERVIEWS",  "START_NEWQUERY_FOUNDNODES", "START_NEWQUERY_SEARCHNODES", "START_NEWQUERy_FIND","START_NEWQUERY_COLLECT", "START_NEWQUERY_FINDLOADED", "START_NEWQUERY_COLLECTCLIPS", "START_NEWQUERY_CHECKCLIPS"  };


bool typeToFilter[(int)EventType::TYPE_QTY] = { false, false, false, false,
false, false, false, false,
false, false, false, false,
false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, true, true, true, true, true, true, true, true, true, true };
   

CachedDataEventTracer* CachedDataEventTracer::s_instance;


CachedDataEventTracer::CachedDataEventTracer()
    {
    ring.resize(400000);
    current = ring.data();
    end = ring.data() + ring.size();
    started = false;
    valToFilter = -1;
    m_logDirectory = "c:\\";
    m_outputObjLog = true;
    }

bool CachedDataEventTracer::filter(TraceEvent& e)
    {
    if (!typeToFilter[(int)e.typeOfEvent]) return false;
    /* if (valToFilter == -1 && e.texId != -1)
    {
    valToFilter = e.texId;
    return true;
    }*/
    return true;// valToFilter == e.texId;
    }


CachedDataEventTracer* CachedDataEventTracer::GetInstance()
    {
    if (s_instance == nullptr)
        s_instance = new CachedDataEventTracer();
    return s_instance;
    }

void CachedDataEventTracer::setLogDirectory(const Utf8String& logDir)
    {
    m_logDirectory = logDir;
    }

void CachedDataEventTracer::setOutputObjLog(bool outputObjLog)
    {
    m_outputObjLog = outputObjLog;
    }

void CachedDataEventTracer::start()
    {
    started = true;
    }

void CachedDataEventTracer::logEvent(TraceEvent e)
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
 
void CachedDataEventTracer::analyze(int processId)
    {
    std::ostringstream fileNameStr;
    std::ostringstream fileNameObjStr;
    
    fileNameStr << m_logDirectory.c_str() << "trace";
    fileNameObjStr << m_logDirectory.c_str() << "traceByObj";

    
    if (processId != -1)
        {
        fileNameStr << std::to_string(processId);
        fileNameObjStr << std::to_string(processId);
        }
    
    fileNameStr << ".log";
    fileNameObjStr << ".log";

    std::ofstream traceFile;
    traceFile.open(fileNameStr.str(), std::ios_base::app);
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

    if (m_outputObjLog)
        {
        traceFile.open(fileNameObjStr.str(), std::ios_base::app);
        for (auto& obj : eventsByVal)
            {
            for (auto& str : obj.second)
                traceFile << str;

            traceFile << " ----------------- " << std::endl;
            }
        traceFile.close();
        }
    }


END_BENTLEY_SCALABLEMESH_NAMESPACE