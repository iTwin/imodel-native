#include <atomic>
#include <mutex>
#include <thread> 

#include <Bentley\BentleyAllocator.h>
#include <Bentley\Bentley.r.h>
#include <Bentley\bmap.h>
#include <Bentley\bvector.h>
#include <Bentley\RefCounted.h>

#include <Geom\msgeomstructs.h>
#include <Geom\DPoint2d.h>
#include <Geom\DPoint3d.h>




using namespace std;

enum DataType
    {
    DATATYPE_POINT3D = 0,
    DATATYPE_INT32,
    DATATYPE_BYTE,
    DATATYPE_POINT2D, 
    DATATYPE_UNKNOWN, 
    };

class MemoryPoolItem : public RefCountedBase
    {
    private : 

        void*     m_item;
        uint64_t  m_size;
        uint64_t  m_nodeId;
        DataType  m_dataType;
        
    public :

        MemoryPoolItem(void* item, uint64_t size, uint64_t nodeId, DataType& dataType)
            {
            m_item = item;
            m_size = size;
            m_nodeId = nodeId;
            m_dataType = dataType;
            }

        virtual ~MemoryPoolItem()
            {
            delete [] m_item;
            }

        void* GetItem();

        uint64_t GetSize()
            {
            return m_size;
            }        

        bool IsCorrect(uint64_t nodeId, DataType& dataType)
            {
            if (nodeId == m_nodeId && dataType == m_dataType)
                return true;

            return false;
            }
    };

typedef RefCountedPtr<MemoryPoolItem> MemoryPoolItemPtr;

static clock_t s_timeDiff = CLOCKS_PER_SEC * 120;
static double s_maxMemBeforeFlushing = 1.2;

class MemoryPool
    {
    private : 

        uint64_t                    m_maxPoolSizeInBytes;
        atomic<uint64_t>            m_currentPoolSizeInBytes;
        bvector<MemoryPoolItemPtr>  m_memPoolItems;
        bvector<std::mutex*>        m_memPoolItemMutex;
        bvector<clock_t>            m_lastAccessTime;
        
        //std::mutex                 m_poolItemMutex;

        /*
        atomic<bool>               m_isFull;
        atomic<bool>               m_lastAvailableInd;
        */

    public : 

        MemoryPool(uint64_t maxPoolSizeInBytes)            
            : m_memPoolItemMutex(500)
            {
            m_currentPoolSizeInBytes = 0;
            m_maxPoolSizeInBytes = maxPoolSizeInBytes;            
            m_memPoolItems.resize(500);
            m_lastAccessTime.resize(500);
            m_memPoolItemMutex.resize(500);

            for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
                {
                m_memPoolItemMutex[itemId] = new mutex;                
                }
            }

        virtual ~MemoryPool()
            {
            for (size_t itemId = 0; itemId < m_memPoolItemMutex.size(); itemId++)
                {                
                delete m_memPoolItemMutex[itemId];
                }            
                        
            /*
            m_lastAvailableInd = 0;
            m_isFull = false;
            */
            }

        MemoryPoolItemPtr& GetItem(uint64_t id)
            {     
            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);
            m_lastAccessTime[id] = clock();
            return m_memPoolItems[id];
            }

        uint64_t AddItem(MemoryPoolItemPtr& poolItem)
            {    
            uint64_t itemInd = 0;            
            clock_t oldestTime = numeric_limits<clock_t>::max();
            uint64_t oldestInd = 0; 

            m_currentPoolSizeInBytes += poolItem->GetSize();
            
            bool needToFlush = false;

            if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
                {
                needToFlush = true;
                }

            clock_t currentTime = clock(); 

            for (; itemInd < (uint64_t)m_memPoolItems.size(); itemInd++)
                {
                    {   
                    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[itemInd]);

                    if (!m_memPoolItems[itemInd].IsValid())
                        {
                        break;
                        }
                    }

                if ((needToFlush && (currentTime - m_lastAccessTime[itemInd] > s_timeDiff)))                
                    {
                    break;
                    }                

                if (oldestTime > m_lastAccessTime[itemInd])
                    {
                    oldestTime = m_lastAccessTime[itemInd];
                    oldestInd = itemInd;
                    }
                }               

            if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes)
                {
                double flushTimeThreshold = (clock() + oldestTime) / 2.0;

                for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems.size() && m_currentPoolSizeInBytes > m_maxPoolSizeInBytes; itemIndToDelete++)
                    {     
                    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[itemIndToDelete]);

                    if (m_memPoolItems[itemIndToDelete].IsValid() && m_lastAccessTime[itemIndToDelete] < flushTimeThreshold)                    
                        {
                        m_currentPoolSizeInBytes -= m_memPoolItems[itemIndToDelete]->GetSize();                                        
                        m_memPoolItems[itemIndToDelete] = 0; 
                        itemInd = itemIndToDelete;
                        }                                        
                    }
                }
            
            if (m_currentPoolSizeInBytes > m_maxPoolSizeInBytes * s_maxMemBeforeFlushing)
                {
                for (size_t itemIndToDelete = 0; itemIndToDelete < m_memPoolItems.size() && m_currentPoolSizeInBytes > m_maxPoolSizeInBytes; itemIndToDelete++)
                    {  
                    std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[itemIndToDelete]);                    

                    if (m_memPoolItems[itemIndToDelete].IsValid())
                        {
                        m_currentPoolSizeInBytes -= m_memPoolItems[itemIndToDelete]->GetSize();                
                        m_memPoolItems[itemIndToDelete] = 0; 
                        itemInd = itemIndToDelete;
                        }

                    m_memPoolItemMutex[itemIndToDelete]->unlock();
                    }
                }            

            if (itemInd == m_memPoolItems.size())
                {
                if (m_currentPoolSizeInBytes < m_maxPoolSizeInBytes)
                    {                
                    m_memPoolItems.resize((size_t)(m_memPoolItems.size() * 1.5));
                    m_lastAccessTime.resize((size_t)(m_lastAccessTime.size() * 1.5));
                    }
                else
                    {
                    itemInd = oldestInd;                    
                    }                
                }
            
            m_memPoolItemMutex[itemInd]->lock();

            if (m_memPoolItems[itemInd].IsValid())            
                {                
                m_currentPoolSizeInBytes -= m_memPoolItems[itemInd]->GetSize();
                }

            m_memPoolItems[itemInd] = poolItem;
            m_lastAccessTime[itemInd] = clock();

            m_memPoolItemMutex[itemInd]->unlock();

#ifndef NDEBUG
            /*
            uint64_t totalSize = 0;

            for (auto poolItem : m_memPoolItems)
                {
                if (poolItem.IsValid())
                    {    
                    totalSize += poolItem->GetSize();
                    }
                }

            assert(totalSize == m_currentPoolSizeInBytes);*/

#endif

            return itemInd;
            }
    };

struct LoadingAction
    {
    uint64_t  m_nodeId;
    size_t   m_nbElements; 
    DataType m_dataType;    
    };

typedef bmap<uint64_t, uint64_t> DataIdToCacheIdMap;
std::mutex         s_dataIdToCacheIdMapMutex;
DataIdToCacheIdMap s_dataIdToCacheIdMap;

static double s_minProcessTime = 0.0001;
static double s_maxProcessTime = 0.0005;

size_t GetDataSizeInBytes(DataType dataType, size_t nbElems)
    {
    size_t dataSizeInBytes; 

    switch (dataType)
        {                                                    
        case DATATYPE_POINT3D : 
            dataSizeInBytes = nbElems * sizeof(DPoint3d);                            
            break;

        case DATATYPE_INT32 : 
            dataSizeInBytes = nbElems * sizeof(int32_t);                            
            break;

        case DATATYPE_BYTE : 
            dataSizeInBytes = nbElems;                                                        
            break;

        case DATATYPE_POINT2D : 
            dataSizeInBytes = nbElems * sizeof(DPoint2d);                                                        
            break;

        default:                             
            dataSizeInBytes = 0;
        }

    return dataSizeInBytes;
    }


class QueryProcessor
    {
    private:
        
        int                    m_numWorkingThreads;
        std::thread*           m_workingThreads;        
        atomic<bool>           m_run;
        int                    m_threadId;
        MemoryPool&            m_memPool;
        bvector<LoadingAction> m_loadingActions;
        atomic<size_t>         m_currentActionInd; 
                   
        void QueryThread(int threadId)
            {    
            m_threadId = threadId;

            do
                {                
                m_currentActionInd++;

                if (m_currentActionInd >= m_loadingActions.size())
                    break;
                
                uint64_t dataId = m_loadingActions[m_currentActionInd].m_nodeId | ((uint64_t)m_loadingActions[m_currentActionInd].m_dataType << 30);
                assert((m_loadingActions[m_currentActionInd].m_nodeId < dataId) || (m_loadingActions[m_currentActionInd].m_dataType == 0));
                
                MemoryPoolItemPtr memItemPtr;

                s_dataIdToCacheIdMapMutex.lock();
                auto mappedData(s_dataIdToCacheIdMap.find(dataId));                
                
                if (mappedData != s_dataIdToCacheIdMap.end())
                    {
                    memItemPtr = m_memPool.GetItem(mappedData->second);                    
                    }                                

                s_dataIdToCacheIdMapMutex.unlock();


                if (memItemPtr.IsValid() || !memItemPtr->IsCorrect(m_loadingActions[m_currentActionInd].m_nodeId, m_loadingActions[m_currentActionInd].m_dataType))                
                    {
                    s_dataIdToCacheIdMapMutex.lock();
                    s_dataIdToCacheIdMap.erase(dataId);
                    s_dataIdToCacheIdMapMutex.unlock();

                    void* item;
                    size_t dataSizeInBytes = GetDataSizeInBytes(m_loadingActions[m_currentActionInd].m_dataType, m_loadingActions[m_currentActionInd].m_nbElements);

                    if (dataSizeInBytes == 0)                    
                        continue;

                    item = new Byte[dataSizeInBytes];     
                    //Without memset Windows badly reports memory used??
                    memset(item, 0, dataSizeInBytes);
                    
                    MemoryPoolItemPtr memItemPtr(new MemoryPoolItem(item, dataSizeInBytes, m_loadingActions[m_currentActionInd].m_nodeId, m_loadingActions[m_currentActionInd].m_dataType));

                    uint64_t id = m_memPool.AddItem(memItemPtr);

                    s_dataIdToCacheIdMapMutex.lock();
                    s_dataIdToCacheIdMap.insert(bpair<uint64_t, uint64_t>(m_loadingActions[m_currentActionInd].m_nodeId, id));
                    s_dataIdToCacheIdMapMutex.unlock();
                    }


                //Emulate processus
                /*
                double processTime = (s_maxProcessTime - s_minProcessTime) * (rand() % 100) / 100 + s_minProcessTime;

                clock_t startProcess = clock(); 
                
                while (((clock() - startProcess) / CLOCKS_PER_SEC) < processTime)
                    {                    
                    }                
                    */

                } while (1);        
            }
         
    public:

        QueryProcessor(MemoryPool& memPool)
            : m_memPool (memPool)
            {       
            m_currentActionInd = std::numeric_limits<size_t>::max();
            //m_numWorkingThreads = std::thread::hardware_concurrency() - 1;            
            m_numWorkingThreads = 1;
            m_workingThreads = new std::thread[m_numWorkingThreads];                        
            m_run = false;   
            
            uint64_t totalMemoryRequiredForActions = 0;                        

            for (size_t ind = 0; ind < 5; ind++)
                {
                FILE* logFile = fopen ("D:\\MyDoc\\RM - SM - Sprint 12\\NewCaching\\MemoryAccessScenario\\display.log","r");

                while (1)
                    {
                    LoadingAction action;                                                                     
                    fscanf(logFile, "%lli %zi %i \r\n", &action.m_nodeId, &action.m_nbElements, &action.m_dataType);                        

                    if (feof(logFile) || ferror(logFile))
                        break;

                    m_loadingActions.push_back(action);

                    size_t memSize = GetDataSizeInBytes(action.m_dataType, action.m_nbElements);                                                                
                    totalMemoryRequiredForActions += memSize;
                    }     

                fclose(logFile);            
                }
            }

        virtual ~QueryProcessor()
            {
            for (size_t threadInd = 0; threadInd < m_numWorkingThreads; threadInd++)
                {
                if (m_workingThreads[threadInd].joinable())
                    m_workingThreads[threadInd].join();
                }

            delete[] m_workingThreads;            
            }
            
        void Start()
            { 
            if (m_run == false)
                {
                m_run = true;

                //Launch a group of threads
                for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                    {                                                                          
                    m_workingThreads[threadId] = std::thread(&QueryProcessor::QueryThread, this, threadId);
                    }                                        
                }
            }
        
        void Stop()
            {                        
            m_run = false;

            for (int threadId = 0; threadId < m_numWorkingThreads; ++threadId) 
                {
                if (m_workingThreads[threadId].joinable())
                    m_workingThreads[threadId].join();                    
                }         
            }       
    };


static uint64_t s_poolSize = 40000000;

int wmain(int argc, wchar_t* argv[])
    {
    argc = argc;
    argv = argv;

    MemoryPool memPool(s_poolSize); 
                            
    

    QueryProcessor queryProcessor(memPool);

    clock_t startTime = clock(); 
    queryProcessor.Start();
    queryProcessor.Stop();
    clock_t durationTime = (clock() - startTime) / CLOCKS_PER_SEC; 

    durationTime = durationTime;
    
    return 0;
    }