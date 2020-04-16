/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include "Tracer.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

std::map<EventType, bool> __TRACEPOINT__typeToFilter = {
    {EventType::LOAD_MESH_CREATE_0,             false},
    {EventType::CACHED_MESH_ACQUIRE,            false},
    {EventType::LOAD_TEX_CREATE_0,              false},
    {EventType::CACHED_TEX_ACQUIRE,             false},
    {EventType::CACHED_MESH_RELEASE,            false},
    {EventType::CACHED_TEX_RELEASE,             false},
    {EventType::SWITCH_VIDEO_MESH,              false},
    {EventType::SWITCH_VIDEO_TEX,               false},
    {EventType::EVT_LOAD_NODE,                  false},
    {EventType::UNLOAD_NODE,                    false},
    {EventType::EVT_CREATE_DISPLAY_OVR_1,       false},
    {EventType::EVT_CREATE_DISPLAY_LOAD,        false},
    {EventType::EVT_CREATE_DISPLAY_OVR_PRELOAD, false},
    {EventType::POOL_ADDITEM,                   false},
    {EventType::POOL_REMOVEITEM,                false},
    {EventType::POOL_GETITEM,                   false},
    {EventType::POOL_REPLACEITEM,               false},
    {EventType::POOL_DELETEITEM,                false},
    {EventType::POOL_CHANGESIZEITEM,            false},
    {EventType::REFCT_ADDREF,                   false},
    {EventType::REFCT_DECREF,                   false},
    {EventType::BEFORE_SWITCH_VIDEO_TEX,        false},
    {EventType::GRAPH_STORE,                    false},
    {EventType::WORKER_MESH_TASK,               false},
    {EventType::WORKER_FILTER_TASK,             false},
    {EventType::WORKER_STITCH_TASK,             false},
    {EventType::WORKER_STITCH_TASK_NEIGHBOR,    false},
    {EventType::CLOUDDATASOURCE_LOAD,           false},
    {EventType::START_NEWQUERY,                 false},
    {EventType::QUERY_LOADNODELIST,             false},
    {EventType::QUERY_NUMBER_OF_OVERVIEWS,      false},
    {EventType::START_NEWQUERY_FOUNDNODES,      false},
    {EventType::START_NEWQUERY_SEARCHNODES,     false},
    {EventType::START_NEWQUERY_FIND,            false},
    {EventType::START_NEWQUERY_COLLECT,         false},
    {EventType::START_NEWQUERY_FINDLOADED,      false},
    {EventType::START_NEWQUERY_COLLECTCLIPS,    false},
    {EventType::START_NEWQUERY_CHECKCLIPS,      false},
    {EventType::CUSTOM_STRING,                  true}
};

std::map<EventType, std::string> __TRACEPOINT__typeDesc = {
    {EventType::LOAD_MESH_CREATE_0,             std::string("LOAD_(CREATE)_MESH")         },
    {EventType::CACHED_MESH_ACQUIRE,            std::string("ACQUIRE_CACHED_MESH ")      },
    {EventType::LOAD_TEX_CREATE_0,              std::string("LOAD_(CREATE)_TEX ")        },
    {EventType::CACHED_TEX_ACQUIRE,             std::string("ACQUIRE_CACHED_TEX ")       },
    {EventType::CACHED_MESH_RELEASE,            std::string("RELEASE_CACHED_MESH")        },
    {EventType::CACHED_TEX_RELEASE,             std::string("RELEASE_CACHED_TEX")         },
    {EventType::SWITCH_VIDEO_MESH,              std::string("SWITCH_TO_VRAM(MESH)")       },
    {EventType::SWITCH_VIDEO_TEX,               std::string("SWITCH_TO_VRAM(TEX)")       },
    {EventType::EVT_LOAD_NODE,                  std::string("LOAD_NODE")                  },
    {EventType::UNLOAD_NODE,                    std::string("UNLOAD_NODE")                },
    {EventType::EVT_CREATE_DISPLAY_OVR_1,       std::string("OVERVIEW_1")                 },
    {EventType::EVT_CREATE_DISPLAY_LOAD,        std::string("LOADNODEDISPLAYDATA")        },
    {EventType::EVT_CREATE_DISPLAY_OVR_PRELOAD, std::string("PRELOADOVERVIEW")            },
    {EventType::POOL_ADDITEM,                   std::string("ADDITEM")                    },
    {EventType::POOL_REMOVEITEM,                std::string("REMOVEITEM")                 },
    {EventType::POOL_GETITEM,                   std::string("GETITEM")                    },
    {EventType::POOL_DELETEITEM,                std::string("DELETEITEM")                 },
    {EventType::POOL_REPLACEITEM,               std::string("POOL_REPLACEITEM")           },
    {EventType::POOL_CHANGESIZEITEM,            std::string("POOL_CHANGESIZEITEM")        },
    {EventType::REFCT_ADDREF,                   std::string("INCREMENT")                  },
    {EventType::REFCT_DECREF,                   std::string("DECREMENT")                  },
    {EventType::BEFORE_SWITCH_VIDEO_TEX,        std::string("BEFORE_SWITCH_VIDEO_TEX")    },
    {EventType::GRAPH_STORE,                    std::string("GRAPH_STORE")                },
    {EventType::WORKER_MESH_TASK,               std::string("WORKER_MESH_TASK")           },
    {EventType::WORKER_FILTER_TASK,             std::string("WORKER_FILTER_TASK")         },
    {EventType::WORKER_STITCH_TASK,             std::string("WORKER_STITCH_TASK")         },
    {EventType::WORKER_STITCH_TASK_NEIGHBOR,    std::string("WORKER_STITCH_TASK_NEIGHBOR")},
    {EventType::CLOUDDATASOURCE_LOAD,           std::string("CLOUDDATASOURCE_LOAD")       },
    {EventType::START_NEWQUERY,                 std::string("START_NEWQUERY")             },
    {EventType::QUERY_LOADNODELIST,             std::string("QUERY_LOADNODELIST")         },
    {EventType::QUERY_NUMBER_OF_OVERVIEWS,      std::string("QUERY_NUMBEROFOVERVIEWS")    },
    {EventType::START_NEWQUERY_FOUNDNODES,      std::string("START_NEWQUERY_FOUNDNODES")  },
    {EventType::START_NEWQUERY_SEARCHNODES,     std::string("START_NEWQUERY_SEARCHNODES") },
    {EventType::START_NEWQUERY_FIND,            std::string("START_NEWQUERy_FIND")        },
    {EventType::START_NEWQUERY_COLLECT,         std::string("START_NEWQUERY_COLLECT")     },
    {EventType::START_NEWQUERY_FINDLOADED,      std::string("START_NEWQUERY_FINDLOADED")  },
    {EventType::START_NEWQUERY_COLLECTCLIPS,    std::string("START_NEWQUERY_COLLECTCLIPS")},
    {EventType::START_NEWQUERY_CHECKCLIPS,      std::string("START_NEWQUERY_CHECKCLIPS")  },
    {EventType::CUSTOM_STRING,                  std::string("CUSTOM_STRING")  }
    };

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

void CachedDataEventTracer::logEvent(const TraceEvent& e)
    {
    if (!started) return;
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
    traceFile.open(fileNameStr.str(), std::ios_base::trunc);
    bmap<uint64_t, bvector<std::string>> eventsByVal;
    for (TraceEvent* init = ring.data(); init != current; ++init)
        {
        std::ostringstream str;
        if(init->typeOfEvent == EventType::CUSTOM_STRING)
            {
            str << "[] >> " << __TRACEPOINT__typeDesc[init->typeOfEvent] << " : "
                << init->custom_string.c_str() 
                << " INSTANCE " << std::to_string(init->objVal) << std::endl;
            }
        else
            {
            str << std::to_string((unsigned long long) init->timestamp)
                << "[" << init->threadId << "] >> "
                << __TRACEPOINT__typeDesc[init->typeOfEvent] << " : " << std::to_string(init->nodeId)
                << " INSTANCE " << std::to_string(init->objVal) << " POOL ID " << std::to_string(init->poolId)
                << " MESH " << std::to_string(init->meshId) << " TEX " << std::to_string(init->texId)
                << " CT " << std::to_string(init->refCount)
                << std::endl;
            if(init->typeOfEvent == EventType::POOL_CHANGESIZEITEM)
                {
                str << " ITEM SIZE " << std::to_string(init->objectSize);
                }
            }
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

#if defined(_WIN32) && TRACE_ON
/*
How to use:
            DisplayStack("AddRef", m_nodeId, m_poolItemId, GetRefCount());
            All the parameters are only informative, see the end of the function.

            You need to add     DbgHelp.lib in ScalableMesh.mke around Urlmon.lib and Version.lib
*/


#include <DbgHelp.h>
void DisplayStack(char* text, uint64_t nodeID, uint64_t poolID, uint64_t refCount)
{
    DWORD  error;
    HANDLE hProcess;

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
    hProcess = GetCurrentProcess();

    if (!SymInitialize(hProcess, NULL, TRUE))
    {
        // SymInitialize failed
        error = GetLastError();
        printf("SymInitialize returned error : %d\n", error);
        return;
    }

    std::string outWalk;

    // Capture up to 25 stack frames from the current call stack.  We're going to
    // skip the first stack frame returned because that's the GetStackWalk function
    // itself, which we don't care about.
    PVOID addrs[25] = { 0 };
    USHORT frames = CaptureStackBackTrace(1, 25, addrs, NULL);

    for (USHORT i = 0; i < frames; i++)
    {
        // Allocate a buffer large enough to hold the symbol information on the stack and get 
        // a pointer to the buffer.  We also have to set the size of the symbol structure itself
        // and the number of bytes reserved for the name.
        ULONG64 buffer[(sizeof(SYMBOL_INFO) + 1024 + sizeof(ULONG64) - 1) / sizeof(ULONG64)] = { 0 };
        SYMBOL_INFO* info = (SYMBOL_INFO*)buffer;
        info->SizeOfStruct = sizeof(SYMBOL_INFO);
        info->MaxNameLen = 1024;

        // Attempt to get information about the symbol and add it to our output parameter.
        DWORD64 displacement = 0;
        if (::SymFromAddr(hProcess, (DWORD64)addrs[i], &displacement, info))
        {
            DWORD displacement2 = 0;
            IMAGEHLP_LINE64 line = { 0 };
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
            SymGetLineFromAddr(hProcess, (DWORD64)(addrs[i]), &displacement2, &line);

            outWalk.append(info->Name, info->NameLen);
            if (line.FileName != 0)
            {
                outWalk.append(" - ");
                outWalk.append(line.FileName);
                outWalk.append(" - ");
                char lineText[2014];
                sprintf(lineText, "%d", line.LineNumber);
                outWalk.append(lineText);
            }
            outWalk.append(" + ");
            //            outWalk.append("\n");
        }
    }
    outWalk.append("\n");
    ::SymCleanup(hProcess);

    std::ostringstream fileNameStr;
    fileNameStr << "C:\\traceLogs\\tracestack.log";
    std::ofstream traceFile;
    traceFile.open(fileNameStr.str(), std::ios_base::app);

    traceFile << clock() << "#" << text << "#" << nodeID << "#" << poolID << "#" << refCount << "#" << outWalk.c_str();

    traceFile.close();
}
#endif

END_BENTLEY_SCALABLEMESH_NAMESPACE
