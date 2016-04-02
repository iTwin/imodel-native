#include <atomic>
#include <mutex>
#include <thread> 

using namespace std;

enum class ItemType
    {
    Points, 
    Indices,
    Texture
    };

class MemoryPoolItem : public RefCountedBase
    {
    private : 

        void*    m_item;
        __int64  m_size;
        __int64  m_nodeId;
        ItemType m_itemType;
        
    public :

        MemoryPoolItem(void* item, __int64 size, __int64 nodeId, ItemType& itemType)
            {
            m_item = item;
            m_size = size;
            m_nodeId = nodeId;
            m_itemType = itemType;
            }

        ~MemoryPoolItem()
            {
            delete item;
            }

        void* GetItem();

        __int64 GetSize()
            {
            return m_size;
            }        

        bool IsCorrect(__int64 nodeId, ItemType& itemType)
            {
            if (nodeId == m_nodeId && itemType == m_itemType)
                return true;

            return false;
            }
    };

typedef RefCounted<MemoryPoolItem> MemoryPoolItemPtr;

class MemoryPool
    {
    private : 

        __int64                    m_maxPoolSizeInBytes;
        bvector<MemoryPoolItemPtr> m_memPoolItems;
        bvector<clock_t>           m_lastAccessTime;
        /*
        atomic<bool>               m_isFull;
        atomic<bool>               m_lastAvailableInd;
        */

    public : 

        MemoryPool(__int64 maxPoolSizeInBytes)
            : 
            {
            m_maxPoolSizeInBytes = maxPoolSizeInBytes;
            m_memPoolItems.resize(500000);
            m_lastAccess.resize(500000);
            /*
            m_lastAvailableInd = 0;
            m_isFull = false;
            */
            }

        MemoryPoolItemPtr GetItem(__int64 id)
            {
            return m_memPoolItems[id];
            }

        __int64 AddItem(MemoryPoolItemPtr poolItem)
            {    
            __int64 itemInd = 0;

            for (; itemInd < m_memPoolItems.size(); itemInd++)
                {
                if (!m_memPoolItems[itemInd].IsValid())
                    {
                    break;
                    }
                }

            if (itemInd == m_memPoolItems.size())
                {
                assert(!"Need to increase pool items size");
                }
            else
                {
                m_memPoolItems[itemInd] = poolItem;
                }

            return itemInd;
            }


        /*
        static MemoryPool& GetMemoryPool()
            {
            static MemoryPool* s_manager = 0;

            if (s_manager == 0) 
                {
                s_manager = new MemoryPool();
                }

            return *s_manager;
            }
            */
    };


class QueryProcessor
    {
    private:
        
        int                           m_numWorkingThreads;
        std::thread*                  m_workingThreads;        
        atomic<bool>                  m_run;
        int                           m_threadId;
        MemoryPool&                   m_memPool;
                   
        void QueryThread(int threadId)
            {    
            m_threadId = threadId;

            do
                {

                } while (m_run);        
            }
         
    public:

        QueryProcessor(MemoryPool& memPool)
            : m_memPool (memPool)
            {       
            m_numWorkingThreads = std::thread::hardware_concurrency() - 1;            
            m_workingThreads = new std::thread[m_numWorkingThreads];                        
            m_run = false;        
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


static __int64 s_poolSize = 30000000;

int wmain(int argc, wchar_t* argv[])
    {
    argc = argc;
    argv = argv;

    MemoryPool memPool(s_poolSize); 
                            
    QueryProcessor queryProcessor;
    queryProcessor.Start();

    return 0;
    }