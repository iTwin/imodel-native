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

#include <ImagePP\h\HmrMacro.h>
#include <ImagePP\all\h\HFCPtr.h>
#include <ImagePP\all\h\HPMDataStore.h>



USING_NAMESPACE_IMAGEPP
using namespace std;

class PerformanceLogger
    {
    private :

        FILE* m_perfLogFile;

    public : 

        PerformanceLogger()
            {
#ifdef NDEBUG
            m_perfLogFile = fopen("D:\\MyDoc\\RM - SM - Sprint 12\\NewCaching\\PerformanceLogRelease.log", "a");
#else
            m_perfLogFile = fopen("D:\\MyDoc\\RM - SM - Sprint 12\\NewCaching\\PerformanceLog.log", "a");
#endif
            fprintf(m_perfLogFile, "\r\n-----------------------------------------------------------------------------\r\n");
            char date[9];
            char timeStr [9];
                
            _strdate(date);
            _strtime(timeStr);
            fprintf(m_perfLogFile, "Start date : %s %s\r\n", date, timeStr);            
            }

        ~PerformanceLogger()
            {
            fprintf(m_perfLogFile, "\r\n-----------------------------------------------------------------------------\r\n");
            fclose(m_perfLogFile);
            }

        void LogLoadActionStat(size_t nbLoadActions, uint64_t totalNbMem, int numWorkingThreads)
            {
            fprintf(m_perfLogFile, "nbLoadAction : %zi, totalNbMem : %llu nbThreads : %i \r\n", nbLoadActions, totalNbMem, numWorkingThreads);
            }

        void LogPerformanceStat(clock_t durationTime)
            {
            fprintf(m_perfLogFile, "durationTime : %li\r\n", durationTime);
            }

    };

PerformanceLogger s_performanceLogger;



enum class DataTypeDesc
    {
    Point3d = 0,
    Int32,
    Byte,
    Point2d, 
    DiffSet, 
    Graph,
    Texture,
    Unknown, 
    };

DataTypeDesc GetDataType(const type_info& typeInfo)
    {
    if (typeid(DPoint3d) == typeInfo)
        return DataTypeDesc::Point3d;
    else if (typeid(int32_t) == typeInfo)
        return DataTypeDesc::Int32;
    else if (typeid(Byte) == typeInfo)
        return DataTypeDesc::Byte;
    else if (typeid(DPoint2d) == typeInfo)
        return DataTypeDesc::Point2d;
    else    
        return DataTypeDesc::Unknown;

    /*
    switch (typeInfo)
        {
        case typeid(DPoint3d):
            return DataTypeDesc::Point3d;
            break; 
        case typeid(int32_t):
            return DataTypeDesc::Int32;
            break; 
        case typeid(Byte):
            return DataTypeDesc::Byte;
            break; 
        case typeid(DPoint2d):
            return DataTypeDesc::Point2d;
            break; 
        default : 
            return DataTypeDesc::Unknown;
            break; 
        }  
        */
    }

/*
enum class ContainerType
    {
    PoolVector = 0
    }
    */

template <typename DataType> class HPMTypedPooledVector;
template <typename DataType> class HPMTypedPoolItem;


class MemoryPoolItem : public RefCountedBase
    {
    protected : 

        Byte*         m_data;
        uint64_t      m_size;
        uint64_t      m_nodeId;
        DataTypeDesc  m_dataType;
        bool          m_dirty;
        //ContainerType m_containerType;

        //type_info m_dataType;
        
    public :

        MemoryPoolItem()
            {
            m_dataType = DataTypeDesc::Unknown;
            m_dirty = false;
            m_nodeId = numeric_limits<uint64_t>::max();
            }

        MemoryPoolItem(Byte* data, uint64_t size, uint64_t nodeId, DataTypeDesc& dataType)
            {
            m_data = data;
            m_size = size;
            m_nodeId = nodeId;
            m_dataType = dataType;
            m_data = data;
            m_dirty = true;
            }

        template<typename T>
        RefCountedPtr<HPMTypedPooledVector<T>> GetAsPoolVector()
            {
            if (GetDataType(typeid(T)) != m_dataType)
                return 0;

            return dynamic_cast<HPMTypedPooledVector<T>*>(this);
            }

        template<typename T>
        RefCountedPtr<HPMTypedPoolItem<T>> GetAsTypedPoolItem()
            {/*
            if (GetDataType(typeid(T)) != m_dataType)
               return 0;
               */

            return dynamic_cast<HPMTypedPoolItem<T>*>(this);
            }
        
        virtual ~MemoryPoolItem()
            {
            delete [] m_data;
            }

        void* GetItem();

        uint64_t GetSize()
            {
            return m_size;
            }        

        bool IsCorrect(uint64_t nodeId, DataTypeDesc& dataType)
            {
            if (nodeId == m_nodeId && dataType == m_dataType)
                return true;

            return false;
            }
    };


/*
class CustomTypedPoolItemCreator
    {
    public : 
                
        virtual Byte* AllocateData() = 0;

        virtual uint64_t GetSize() = 0;
    };

class TextureTypedPoolItemCreator : public CustomTypedPoolItemCreator
    {
    TextureTypedPoolItemCreator(size_t sizeX, size_t sizeY)
        {
        }

    virtual ~TextureTypedPoolItemCreator()
        {
        }

    }
    */

template <typename DataType> class HPMTypedPoolItem : public MemoryPoolItem
    {
    protected : 

    public : 
        
        HPMTypedPoolItem(size_t size, uint64_t nodeId, DataTypeDesc dataType)
            {            
            m_size = size;
            m_nodeId = nodeId;
            m_data = (Byte*)new Byte[m_size];
            memset(m_data, 1, m_size);                               
            m_dataType = dataType;
            }

        virtual ~HPMTypedPoolItem()
            {
            }
    };

template <typename DataType> class HPMTypedPooledVector : public MemoryPoolItem
    {
    protected:
        
        size_t    m_nbItems;

    public:
        typedef DataType value_type;
        
        HPMTypedPooledVector()
            {
            }

        HPMTypedPooledVector(size_t nbItems, uint64_t nodeId)                        
            {
            m_nbItems = nbItems;
            m_size = nbItems * sizeof(DataType);
            m_nodeId = nodeId;
            m_data = (Byte*)new DataType[nbItems];
            memset(m_data, 1, m_size);
                                   
            m_dataType = GetDataType(typeid(DataType));
            }

        ~HPMTypedPooledVector()
            {
            }    
    };


template <typename DataType> class HPMStoredTypedPooledVector : public HPMTypedPooledVector<DataType>
    {
    private: 

        IHPMDataStore<DataType>* m_store;
        bool                     m_isDirty;

    public:        
                
        HPMStoredTypedPooledVector(size_t nbItems, uint64_t nodeId, IHPMDataStore<DataType>* store)
            : HPMStoredTypedPooledVector(nbItems, nodeId)
            {            
            m_store = store;            
            m_store->LoadBlock (m_data, m_nbItems, m_nodeId);
            }

        ~HPMStoredTypedPooledVector()
            {
            if (m_dirty)
                {
                }
            }    
    };

typedef RefCountedPtr<MemoryPoolItem> MemoryPoolItemPtr;

//First impl - dead lock
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


        
        bool GetItem(MemoryPoolItemPtr& memItemPtr, uint64_t id)
            {     
            std::lock_guard<std::mutex> lock(*m_memPoolItemMutex[id]);
            m_lastAccessTime[id] = clock();
            memItemPtr = m_memPoolItems[id];
            return memItemPtr.IsValid();
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
    uint64_t     m_nodeId;
    size_t       m_nbElements; 
    DataTypeDesc m_dataType;    
    };

atomic<uint64_t> s_nbTotalDataLoaded; 

typedef bmap<uint64_t, uint64_t> DataIdToCacheIdMap;
std::mutex         s_dataIdToCacheIdMapMutex;
DataIdToCacheIdMap s_dataIdToCacheIdMap;


static double s_minProcessTime = 0.001;
static double s_maxProcessTime = 0.005;

size_t GetDataSizeInBytes(DataTypeDesc dataType, size_t nbElems)
    {
    size_t dataSizeInBytes; 

    switch (dataType)
        {                                                    
        case DataTypeDesc::Point3d : 
            dataSizeInBytes = nbElems * sizeof(DPoint3d);                            
            break;

        case DataTypeDesc::Int32 : 
            dataSizeInBytes = nbElems * sizeof(int32_t);                            
            break;

        case DataTypeDesc::Byte : 
            dataSizeInBytes = nbElems;                                                        
            break;

        case DataTypeDesc::Point2d : 
            dataSizeInBytes = nbElems * sizeof(DPoint2d);                                                        
            break;

        default:                             
            dataSizeInBytes = 0;
        }

    return dataSizeInBytes;
    }

static bool s_createPoolVector = true;

class QueryProcessor
    {
    private:
        
        int                    m_numWorkingThreads;
        std::thread*           m_workingThreads;        
        atomic<bool>           m_run;
        int                    m_threadId;
        MemoryPool&            m_memPool;
        bvector<LoadingAction> m_loadingActions;
        mutex                  m_loadActionsMutex;
        size_t                 m_currentActionInd; 
                   
        void QueryThread(int threadId)
            {    
            m_threadId = threadId;

            do
                {       
                //atomic<size_t> currentActionInd;
                size_t currentActionInd;

                m_loadActionsMutex.lock();                                
                m_currentActionInd++;
                currentActionInd = m_currentActionInd;
                m_loadActionsMutex.unlock();
                
                if (currentActionInd >= m_loadingActions.size())
                    break;
                
                uint64_t dataId = m_loadingActions[currentActionInd].m_nodeId | ((uint64_t)m_loadingActions[currentActionInd].m_dataType << 30);
                assert((m_loadingActions[currentActionInd].m_nodeId < dataId) || ((int)m_loadingActions[currentActionInd].m_dataType == 0));

                s_nbTotalDataLoaded += GetDataSizeInBytes(m_loadingActions[currentActionInd].m_dataType, m_loadingActions[currentActionInd].m_nbElements); 

                                                
                uint64_t cacheId = numeric_limits<uint64_t>::max();

                s_dataIdToCacheIdMapMutex.lock();
                auto mappedData(s_dataIdToCacheIdMap.find(dataId));                
                
                if (mappedData != s_dataIdToCacheIdMap.end())
                    {
                    cacheId = mappedData->second;
                    }

                s_dataIdToCacheIdMapMutex.unlock();

                MemoryPoolItemPtr memItemPtr;

                if (cacheId != numeric_limits<uint64_t>::max())
                    {
                    m_memPool.GetItem(memItemPtr, cacheId);                    
                    }                                
                                
                if (!memItemPtr.IsValid() || !memItemPtr->IsCorrect(m_loadingActions[currentActionInd].m_nodeId, m_loadingActions[currentActionInd].m_dataType))                
                    {
                    s_dataIdToCacheIdMapMutex.lock();
                    s_dataIdToCacheIdMap.erase(dataId);
                    s_dataIdToCacheIdMapMutex.unlock();

                    if (!s_createPoolVector)
                        {
                        Byte* item;
                        size_t dataSizeInBytes = GetDataSizeInBytes(m_loadingActions[currentActionInd].m_dataType, m_loadingActions[currentActionInd].m_nbElements);

                        if (dataSizeInBytes == 0)                    
                            continue;

                        item = new Byte[dataSizeInBytes];     
                        //Without memset Windows badly reports memory used??
                        memset(item, 0, dataSizeInBytes);

                        memItemPtr = new MemoryPoolItem(item, dataSizeInBytes, m_loadingActions[currentActionInd].m_nodeId, m_loadingActions[currentActionInd].m_dataType);
                        }
                    else
                        {                  
                        if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Point3d)
                            {                                                             
                            memItemPtr = new HPMTypedPoolItem<DPoint3d>(m_loadingActions[currentActionInd].m_nbElements * sizeof(DPoint3d), m_loadingActions[currentActionInd].m_nodeId, DataTypeDesc::Point3d);
                            //memItemPtr = new HPMTypedPooledVector<DPoint3d>(m_loadingActions[currentActionInd].m_nbElements, m_loadingActions[currentActionInd].m_nodeId);
                            }
                        else
                        if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Int32)
                            {
                            memItemPtr = new HPMTypedPooledVector<int32_t>(m_loadingActions[currentActionInd].m_nbElements, m_loadingActions[currentActionInd].m_nodeId);
                            }
                        else
                        if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Byte)
                            {
                            memItemPtr = new HPMTypedPooledVector<Byte>(m_loadingActions[currentActionInd].m_nbElements, m_loadingActions[currentActionInd].m_nodeId);
                            }
                        else
                        if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Point2d)
                            {
                            memItemPtr = new HPMTypedPooledVector<DPoint2d>(m_loadingActions[currentActionInd].m_nbElements, m_loadingActions[currentActionInd].m_nodeId);
                            }
                        else
                            {
                            assert(!"Unknown type");
                            continue;
                            }                                                                        
                        }
                                                                               
                    uint64_t id = m_memPool.AddItem(memItemPtr);

                    s_dataIdToCacheIdMapMutex.lock();
                    s_dataIdToCacheIdMap.insert(bpair<uint64_t, uint64_t>(m_loadingActions[currentActionInd].m_nodeId, id));
                    s_dataIdToCacheIdMapMutex.unlock();
                    }
                else
                    {
                    if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Point3d)
                        {                                     
                        RefCountedPtr<HPMTypedPoolItem<DPoint3d>> poolVector(memItemPtr->GetAsTypedPoolItem<DPoint3d>());
                        poolVector = poolVector;                        

            /*
                        RefCountedPtr<HPMTypedPooledVector<DPoint3d>> poolVector(memItemPtr->GetAsPoolVector<DPoint3d>());
                        poolVector = poolVector;                        
                        */
                        }
                    else
                    if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Int32)
                        {                        
                        RefCountedPtr<HPMTypedPooledVector<int32_t>> poolVector(memItemPtr->GetAsPoolVector<int32_t>());
                        poolVector = poolVector;                        
                        }
                    else
                    if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Byte)
                        {
                        RefCountedPtr<HPMTypedPooledVector<Byte>> poolVector(memItemPtr->GetAsPoolVector<Byte>());
                        poolVector = poolVector;                                                
                        }
                    else
                    if (m_loadingActions[currentActionInd].m_dataType == DataTypeDesc::Point2d)
                        {
                        RefCountedPtr<HPMTypedPooledVector<DPoint2d>> poolVector(memItemPtr->GetAsPoolVector<DPoint2d>());
                        poolVector = poolVector;                                                                        
                        }
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
            m_numWorkingThreads = std::thread::hardware_concurrency() - 1;            
            //m_numWorkingThreads = 1;
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

                    /*
                    if (m_loadingActions.size() > 300)
                        break;
                        */

                    size_t memSize = GetDataSizeInBytes(action.m_dataType, action.m_nbElements);                                                                
                    totalMemoryRequiredForActions += memSize;
                    }     

                fclose(logFile);            
                }

            s_performanceLogger.LogLoadActionStat(m_loadingActions.size(), totalMemoryRequiredForActions, m_numWorkingThreads);
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
    
    s_nbTotalDataLoaded = 0; 

    MemoryPool memPool(s_poolSize); 
                            
    

    QueryProcessor queryProcessor(memPool);

    clock_t startTime = clock(); 
    queryProcessor.Start();
    queryProcessor.Stop();
    clock_t durationTime = (clock() - startTime) / CLOCKS_PER_SEC; 

    durationTime = durationTime;

    s_performanceLogger.LogPerformanceStat(durationTime);
    
    return 0;
    }