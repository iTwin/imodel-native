//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.h $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once




#include <ImagePP/all/h/HFCPtr.h>
#include <TilePublisher/TilePublisher.h>
#include "Threading/LightThreadPool.h"
#include "SMSQLiteFile.h"
#include "Stores/SMStoreUtils.h"
#include <condition_variable>
#include <json/json.h>
#ifndef LINUX_SCALABLEMESH_BUILD
#include <CloudDataSource/DataSourceManager.h>
#define SESSION_NAME DataSource::SessionName
#else
class DataSourceSessionName;
#define SESSION_NAME DataSourceSessionName
#endif
#include <queue>
#include <map>
#include <iomanip>
#include <unordered_map>
#include <wchar.h>

#ifdef VANCOUVER_API
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#endif

#define OPEN_OR_CREATE_FILE(beFile, pathStr, accessMode) BeFileStatus::Success == OPEN_FILE(beFile, pathStr, accessMode) || BeFileStatus::Success == beFile.Create(pathStr)

//#ifndef NDEBUG
//#define DEBUG_GROUPS
//#define DEBUG_AZURE
//extern std::mutex s_consoleMutex;
//#endif

#ifdef DEBUG_AZURE
#include <Bentley\BeConsole.h>
#endif

class DataSourceAccount;
class SMNodeGroup;
typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<SMNodeGroup> SMNodeGroupPtr;

extern bool s_stream_enable_caching;

BENTLEY_SM_EXPORT extern uint32_t s_max_number_nodes_in_group;
BENTLEY_SM_EXPORT extern size_t   s_max_group_size;
BENTLEY_SM_EXPORT extern uint32_t s_max_group_depth;
BENTLEY_SM_EXPORT extern uint32_t s_max_group_common_ancestor;

struct SMGroupGlobalParameters : public BENTLEY_NAMESPACE_NAME::RefCountedBase {

public:
    enum StrategyType
        {
        NONE,
        NORMAL,
        VIRTUAL,
        CESIUM,
        BIMCESIUM
        };

    typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<SMGroupGlobalParameters> Ptr;

public:
#ifndef LINUX_SCALABLEMESH_BUILD
    DataSourceAccount*                  GetDataSourceAccount();
    const DataSource::SessionName &     GetDataSourceSessionName();
#endif
    StrategyType                        GetStrategyType() { return m_strategyType; }
    uint32_t                            GetNextNodeID() { return m_nextNodeID++; }
    WString                             GetWellKnownText() { return m_wktStr; }
    void                                SetWellKnownText(const WString& wkt) { m_wktStr = wkt; }

    BENTLEY_SM_EXPORT static Ptr        Create(StrategyType strategy, DataSourceAccount* account, const SESSION_NAME &session);

private:

    //SMGroupGlobalParameters() = delete;
    SMGroupGlobalParameters(StrategyType strategy, DataSourceAccount* account, const SESSION_NAME &session);

    SMGroupGlobalParameters(const SMGroupGlobalParameters&) = delete;
    SMGroupGlobalParameters& operator=(const SMGroupGlobalParameters&) = delete;


#ifndef VANCOUVER_API
    virtual uint32_t _GetExcessiveRefCountThreshold() const override { return numeric_limits<uint32_t>::max(); }
#endif

private:

    StrategyType                m_strategyType = NORMAL;
    DataSourceAccount*          m_dataSourceAccount = nullptr;
#ifndef LINUX_SCALABLEMESH_BUILD
    DataSource::SessionName     m_dataSourceSessionName;
#endif
    std::atomic<uint32_t>       m_nextNodeID = {0};
    WString                     m_wktStr;
    };

struct SMGroupCache : public BENTLEY_NAMESPACE_NAME::RefCountedBase 
    {

public:
    typedef std::map<uint64_t, SMNodeGroupPtr> group_cache;
    typedef std::map<uint64_t, Json::Value*>   node_header_cache;
    typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<SMGroupCache> Ptr;

public:

    StatusInt               AddNodeToGroupCache(SMNodeGroupPtr group, const uint64_t & id, Json::Value * header);
    SMNodeGroupPtr          GetGroupForNodeIDFromCache(const uint64_t& nodeId);
    void                    RemoveNodeFromCache(const uint64_t& nodeId);
    Json::Value*            GetNodeFromCache(const uint64_t& nodeId);

    static SMGroupCache::Ptr Create(node_header_cache* nodeCache);

private:
    SMGroupCache() = delete;
    SMGroupCache(node_header_cache* nodeCache);


#ifndef VANCOUVER_API
    virtual uint32_t _GetExcessiveRefCountThreshold() const override;
#endif


private:

    std::mutex m_cacheMutex;
    std::shared_ptr<group_cache> m_downloadedGroupsPtr;
    node_header_cache *m_nodeHeadersPtr;
    DataSourceAccount *m_dataSourceAccount;

    };

struct SMNodeHeader {
    size_t blockid;
    size_t offset;
    size_t size;
    };

struct SMGroupHeader : public bvector<SMNodeHeader>, public HFCShareableObject<SMGroupHeader> {
public:
    SMGroupHeader() : m_groupID(-1) {}
    SMGroupHeader(const uint32_t& pi_pGroupID) : m_groupID(pi_pGroupID) {}
    SMGroupHeader(const uint32_t& pi_pGroupID, const size_t& pi_pSize) : bvector<SMNodeHeader>(pi_pSize), m_groupID(pi_pGroupID) {}

    uint32_t GetID() { return m_groupID; }
    void   SetID(const uint32_t& pi_pGroupID) { m_groupID = pi_pGroupID; }

    SMNodeHeader* AddNode(const SMNodeHeader& pi_pNodeHeader) { this->push_back(pi_pNodeHeader); return &this->back(); }

private:
    uint32_t m_groupID;
    };

struct SMGroupNodeIds : bvector<uint64_t> {
    uint64_t m_sizeOfRawHeaders;
    };

template <typename Type, typename Queue = std::queue<Type>>
class SMNodeDistributor : public Queue, std::mutex, std::condition_variable, public HFCShareableObject<SMNodeDistributor<Type, Queue> > {
    typename Queue::size_type capacity;
    unsigned int m_concurrency;
    bool m_done = false;
    std::vector<std::thread> m_threads;
    enum class ThreadState
        {
        IDLE,
        WORKING
        };
    std::map<std::thread::id, ThreadState> m_threadStates;
    std::function<void(Type)> m_workFunction;

public:
    typedef HFCPtr<SMNodeDistributor<Type, Queue>> Ptr;
    SMNodeDistributor(unsigned int concurrency = std::thread::hardware_concurrency()
                      , typename Queue::size_type max_items_per_thread = 5000)
        : capacity{ concurrency * max_items_per_thread },
        m_concurrency{ concurrency }
        {
        if (!concurrency)
            throw std::invalid_argument("Concurrency must be non-zero");
        if (!max_items_per_thread)
            throw std::invalid_argument("Max items per thread must be non-zero");
        }
    template<typename Function>
    SMNodeDistributor(Function function
                      , unsigned int concurrency /*= std::thread::hardware_concurrency()*/
                      //, unsigned int concurrency = 2
                      , typename Queue::size_type max_items_per_thread /*= 5000*/
    )
        : capacity{ concurrency * max_items_per_thread }
        {
        if (!concurrency)
            throw std::invalid_argument("Concurrency must be non-zero");
        if (!max_items_per_thread)
            throw std::invalid_argument("Max items per thread must be non-zero");

        for (unsigned int count{ 0 }; count < concurrency; count += 1)
            {
            m_threads.emplace_back(static_cast<void (SMNodeDistributor::*)(Function)>
                (&SMNodeDistributor::Consume), this, function);
            }
        }


template<typename Function, typename PredicateFunc>
SMNodeDistributor(Function function
	, PredicateFunc can_run_function
	, unsigned int concurrency /*= std::thread::hardware_concurrency()*/
	//, unsigned int concurrency = 2
	, typename Queue::size_type max_items_per_thread /*= 5000*/
)
	: capacity{ concurrency * max_items_per_thread }
{
	if (!concurrency)
		throw std::invalid_argument("Concurrency must be non-zero");
	if (!max_items_per_thread)
		throw std::invalid_argument("Max items per thread must be non-zero");

	for (unsigned int count{ 0 }; count < concurrency; count += 1)
	{
		m_threads.emplace_back(static_cast<void (SMNodeDistributor::*)(Function, PredicateFunc)>
			(&SMNodeDistributor::Consume), this, function, can_run_function);
	}
}

    //    SMNodeDistributor(SMNodeDistributor &&) = default;
    //    SMNodeDistributor &operator=(SMNodeDistributor &&) = delete;

    ~SMNodeDistributor()
        {
        this->CancelAll();
        //{
        //std::unique_lock<std::mutex> lock(*this);
        //static std::atomic<uint64_t> lastNumberOfItems = Queue::size();
        //while (!wait_for(lock, 1000ms, [this]
        //    {
        //    return Queue::empty();
        //    }))
        //    {
        //    std::lock_guard<mutex> clk(s_consoleMutex);
        //    std::cout << std::setw(100) << "\r  Speed : " << lastNumberOfItems - Queue::size() << " items/second     Remaining : " << Queue::size() << "                         ";
        //    lastNumberOfItems = Queue::size();
        //    }
        //}
        for (auto &&thread : m_threads) if (thread.joinable()) thread.join();
        }

    void InitCustomWork(std::function<void(Type)>&& function)
        {
        m_workFunction = std::move(function);
        m_threads.resize(m_concurrency);
        this->SpawnThreads();
        }

    void AddWorkItem(Type &&value, bool notify = true)
        {
        std::unique_lock<std::mutex> lock(*this);
        if (Queue::size() == capacity)
            {
            static std::atomic<uint64_t> lastNumberOfItems = {Queue::size()};
            while (!wait_for(lock, 1000ms, [this]
                {
                return Queue::size() < capacity / 2;
                }))
                {
               // std::lock_guard<mutex> clk(s_consoleMutex);
               // std::cout << "\r  Speed : " << lastNumberOfItems - Queue::size() << " items/second     Remaining : " << Queue::size() << "                         ";
                lastNumberOfItems = Queue::size();
                }
            }
        Queue::push(std::forward<Type>(value));
        if (notify) notify_one();
        }

    template <typename Function>
    void Wait(Function function)
        {
        std::unique_lock<std::mutex> lock(*this);
        wait(lock, function);
        }

    template <typename Function>
    void WaitUntilFinished(Function function)
        {
        std::unique_lock<std::mutex> lock(*this);
        bool areThreadsFinished = false;
        while (!wait_for(lock, 1000ms, function) || !areThreadsFinished)
            {
			if (function()) //allow a yield after progress finishes, to leave time for all threads to return
				wait_for(lock, 1000ms);
            for (auto state : m_threadStates)
                {
                if (!(areThreadsFinished = state.second == ThreadState::IDLE))
                    break;
                }
            if (Queue::empty() && areThreadsFinished)
                break;
            }
        }

    void WaitUntilFinished()
        {
        std::unique_lock<std::mutex> lock(*this);
        bool areThreadsFinished = false;
        while (true)
            {
            for (auto state : m_threadStates)
                {
                if (!(areThreadsFinished = state.second == ThreadState::IDLE))
                    break;
                }
            if (!Queue::empty() || !areThreadsFinished)
                {
                assert(lock.owns_lock());
                wait_for(lock, 1000ms);
                }
            else
                {
                break;
                }
            }
        }

    void CancelAll()
        {
        std::unique_lock<std::mutex> lock(*this);
        m_done = true;
        notify_all();
        }

    void Go()
        {
        notify_all();
        }

private:
    template <typename Function>
    void Consume(Function process)
        {
        std::unique_lock<std::mutex> lock(*this);
        while (true) {
            if (!Queue::empty()) {
                assert(lock.owns_lock());
                Type item{ std::move(Queue::front()) };
                Queue::pop();
                notify_one();
                m_threadStates[std::this_thread::get_id()] = ThreadState::WORKING;
                lock.unlock();
                process(item);
                lock.lock();
                }
            else if (m_done) {
//#ifdef DEBUG_GROUPS
//                    {
//                    std::lock_guard<mutex> clk(s_consoleMutex);
//                    std::cout << "[" << std::this_thread::get_id() << "] Finished work" << std::endl;
//                    }
//#endif
                    break;
                }
            else {
//#ifdef DEBUG_GROUPS
//                    {
//                    std::lock_guard<mutex> clk(s_consoleMutex);
//                    std::cout << "[" << std::this_thread::get_id() << "] Waiting for work" << std::endl;
//                    }
//#endif
                m_threadStates[std::this_thread::get_id()] = ThreadState::IDLE;
				notify_all();
                    wait(lock);
//#ifdef DEBUG_GROUPS
//                    {
//                    std::lock_guard<mutex> clk(s_consoleMutex);
//                    std::cout << "[" << std::this_thread::get_id() << "] Going to perform work; size of queue = " << Queue::size() << std::endl;
//                    }
//#endif
                }
            }
        }

	template <typename Function, typename PredicateFunc>
	void Consume(Function process, PredicateFunc is_process_ready)
	{
		std::unique_lock<std::mutex> lock(*this);
		while (true) {
			if (!Queue::empty()) {
				assert(lock.owns_lock());
				Type item{ std::move(Queue::front()) };
				Queue::pop();
				notify_one();
				m_threadStates[std::this_thread::get_id()] = ThreadState::WORKING;
				lock.unlock();
				if (is_process_ready(item))
				{
					process(item);
					lock.lock();
				}
				else
				{
					lock.lock();
					Queue::push(std::move(item));
				}
			}
			else if (m_done) {
				//#ifdef DEBUG_GROUPS
				//                    {
				//                    std::lock_guard<mutex> clk(s_consoleMutex);
				//                    std::cout << "[" << std::this_thread::get_id() << "] Finished work" << std::endl;
				//                    }
				//#endif
				break;
			}
			else {
				//#ifdef DEBUG_GROUPS
				//                    {
				//                    std::lock_guard<mutex> clk(s_consoleMutex);
				//                    std::cout << "[" << std::this_thread::get_id() << "] Waiting for work" << std::endl;
				//                    }
				//#endif
				m_threadStates[std::this_thread::get_id()] = ThreadState::IDLE;

				wait(lock);
				//#ifdef DEBUG_GROUPS
				//                    {
				//                    std::lock_guard<mutex> clk(s_consoleMutex);
				//                    std::cout << "[" << std::this_thread::get_id() << "] Going to perform work; size of queue = " << Queue::size() << std::endl;
				//                    }
				//#endif
			}
		}
	}

    void SpawnThreads()
        {
        assert(m_workFunction != nullptr);
        for (auto &thread : m_threads)
            {
            thread = std::thread(static_cast<void (SMNodeDistributor::*)(std::function<void(Type)>)>
                (&SMNodeDistributor::Consume), this, m_workFunction);
            }
        }
    };

#define FORWARD_DECL_GROUPING_STRATEGY(strategy) \
    template<class EXTENT> class strategy

FORWARD_DECL_GROUPING_STRATEGY(SMGroupingStrategy);
FORWARD_DECL_GROUPING_STRATEGY(SMBentleyGroupingStrategy);
FORWARD_DECL_GROUPING_STRATEGY(SMCesium3DTileStrategy);


#define CREATE_GROUPING_STRATEGY_FRIENDSHIP(strategy) \
    template<class EXTENT> friend class strategy;

#define ADD_GROUPING_STRATEGY_FRIENDSHIPS \
    CREATE_GROUPING_STRATEGY_FRIENDSHIP(SMBentleyGroupingStrategy) \
    CREATE_GROUPING_STRATEGY_FRIENDSHIP(SMCesium3DTileStrategy)

enum class UpAxis { X, Y, Z };

BENTLEY_SM_EXPORT extern SMGroupingStrategy<DRange3d>* s_groupingStrategy;

class SMNodeGroup : public BENTLEY_NAMESPACE_NAME::RefCountedBase
    {
    ADD_GROUPING_STRATEGY_FRIENDSHIPS

#ifndef VANCOUVER_API
    private:
        virtual uint32_t _GetExcessiveRefCountThreshold() const override { return numeric_limits<uint32_t>::max(); }
#endif

    public:
        typedef std::pair<uint64_t, SMNodeGroup*> DistributeData;

    private:
        bool   m_isLoaded = false;
        bool   m_isLoading = false;
        UpAxis   m_upAxis = UpAxis::Y;
        uint32_t m_level = 0;
        size_t m_totalSize;
        uint32_t m_nLevels = 0;
        uint32_t m_depth = 0;
        uint32_t m_ancestor = -1;
        uint64_t m_currentPosition = 0;
        uint64_t m_progress = 0;
        uint32_t m_maxGroupDepth = 0;

        SMGroupGlobalParameters::Ptr m_parametersPtr;
        SMGroupCache::Ptr            m_groupCachePtr;
        bool m_isRoot = false;
#ifndef LINUX_SCALABLEMESH_BUILD
        DataSourceURL m_url;
#endif
        bvector<uint8_t> m_rawHeaders;
        std::unordered_map<uint64_t, Json::Value*> m_tileTreeMap;
        SMNodeGroupPtr m_ParentGroup;
        Json::Value m_tilesetRootNode;
        WString m_outputDirPath;
        WString m_dataSourcePrefix;
        WString m_dataSourceExtension = L".bin";
        HFCPtr<SMGroupHeader> m_groupHeader;
        SMNodeDistributor<SMNodeGroup::DistributeData>::Ptr m_nodeDistributorPtr;
        condition_variable m_groupCV;
        mutex m_groupMutex;
        static mutex s_mutex;

    public:

        static SMNodeGroup* Create(SMGroupGlobalParameters::Ptr parameters, SMGroupCache::Ptr cache, const WString pi_pOutputDirPath, const uint32_t& pi_pGroupID, SMNodeGroupPtr parentGroup = nullptr)
            {
            return new SMNodeGroup(parameters, cache, pi_pOutputDirPath, pi_pGroupID, parentGroup);
            }

        //static SMNodeGroupPtr CreateCesium3DTilesGroup(SMGroupGlobalParameters::Ptr parameters, const uint32_t groupID, bool isRootGroup = false)
        //    {
        //    auto group = new SMNodeGroup(parameters, groupID);
        //    group->m_isRoot = true;
        //    return group;
        //    }
        //
        static SMNodeGroupPtr Create(SMGroupGlobalParameters::Ptr parameters, SMGroupCache::Ptr cache, const uint32_t groupID)
            {
            return new SMNodeGroup(parameters, cache, groupID);
            }

        void AppendHeader(const size_t& nodeID, const uint8_t* headerData, const size_t& size)
            {
            size_t oldSize, offset;
            oldSize = offset = m_rawHeaders.size();
            m_groupHeader->AddNode(SMNodeHeader{ nodeID, offset, size });

            m_rawHeaders.resize(oldSize + size);
            memmove(&m_rawHeaders[oldSize], headerData, size);
            m_totalSize += size + sizeof(SMNodeHeader);
            }

        void AppendHeader(const uint64_t& nodeID, Json::Value& jsonHeader)
            {
            assert(m_tileTreeMap.count(nodeID) == 0);
            m_tileTreeMap[nodeID] = &jsonHeader;
            }

        void DeclareRoot() { m_isRoot = true; }

        uint32_t GetLevel() { return m_level; }

        void SetLevel(const uint32_t& pi_NewID) { m_level = pi_NewID; }

        UpAxis GetGltfUpAxis() { return m_upAxis; }

        BENTLEY_SM_EXPORT void Append3DTile(const uint64_t& nodeID, const uint64_t& parentNodeID, const Json::Value& tile);

        BENTLEY_SM_EXPORT void AppendChildGroup(SMNodeGroupPtr childGroup);

        uint32_t GetID() { return m_groupHeader->GetID(); }

        void SetID(const uint32_t& pi_NewID) { m_groupHeader->SetID(pi_NewID); }

        void SetAncestor(const uint32_t& pi_pNewAncestor) { m_ancestor = pi_pNewAncestor; }

        void SetDataSourcePrefix(const WString& pi_pDataSourcePrefix) { m_dataSourcePrefix = pi_pDataSourcePrefix; }

        void SetDataSourceExtension(const WString& pi_pDataSourceExtension) { m_dataSourceExtension = pi_pDataSourceExtension; }

        size_t GetNumberNodes() { return m_groupHeader->size(); }

        size_t GetSizeOfHeaders() { return m_rawHeaders.size(); }

        bvector<Byte>::pointer GetRawHeaders(const size_t& offset) { return m_rawHeaders.data() + offset; }

#ifndef LINUX_SCALABLEMESH_BUILD
        DataSourceURL GetDataURLForNode(HPMBlockID blockID);

        DataSourceURL GetURL();

        void SetURL(DataSourceURL url);
#endif

        Json::Value GetJsonHeader(const uint64_t& id) 
            {
            assert(m_tileTreeMap.count(id) == 1);
            return *m_tileTreeMap[id];
            }

        Json::Value* GetSMMasterHeaderInfo();

        uint64_t GetRootTileID();

        Json::Value* DownloadNodeHeader(const uint64_t& id);

        unordered_map<uint64_t, Json::Value*>& GetJsonNodeHeaders()
            {
            return m_tileTreeMap;
            }

        size_t GetTotalSize() { return m_totalSize; }

        void SetHeader(HFCPtr<SMGroupHeader> pi_pGroupHeader) { m_groupHeader = pi_pGroupHeader; }

        HFCPtr<SMGroupHeader> GetHeader() { return m_groupHeader; }

        void SetHeaderDataAtCurrentPosition(const uint64_t& nodeID, const uint8_t* rawHeader, const uint64_t& headerSize);

        void IncreaseDepth() { ++m_depth; }

        void DecreaseDepth()
            {
            assert(m_depth > 0);
            --m_depth;
            }

        void SetMaxGroupDepth(const uint32_t& depth)
            {
            m_maxGroupDepth = depth;
            }

        WString GetFilePath() { return m_outputDirPath; }

        void Open(const uint32_t& pi_pGroupID)
            { 
            SetID(pi_pGroupID); 
            }

        template<class EXTENT> void Close()
            {
            GetStrategy<EXTENT>()->SaveNodeGroup(this);
            Clear();
            }

        void Clear()
            {
            m_groupHeader->clear();
            m_rawHeaders.clear();
            m_tileTreeMap.clear();
            m_totalSize = 2 * sizeof(size_t);
            m_ancestor = -1;
            }

        template<class EXTENT> size_t AddNode(SMIndexNodeHeader<EXTENT>& pi_NodeHeader)
            {
            return GetStrategy<EXTENT>()->AddNodeToGroup(pi_NodeHeader, this);
            }

        bool IsEmpty() { return m_groupHeader->empty() && m_tileTreeMap.empty(); }

        bool IsFull()
            {
            //return GetNumberNodes() >= s_max_number_nodes_in_group;
            return GetTotalSize() >= s_max_group_size;
            }

        bool IsMaxDepthAchieved()
            {
            return m_depth >= (m_maxGroupDepth ? m_maxGroupDepth : s_max_group_depth);
            }

        bool IsCommonAncestorTooFar(const uint32_t& pi_pLevelRequested)
            {
            return (m_ancestor == -1 ? false : pi_pLevelRequested >= s_max_group_common_ancestor + m_ancestor);
            }

        bool IsLoaded() { return m_isLoaded; }

        bool IsRoot() { return m_isRoot; }

#ifndef LINUX_SCALABLEMESH_BUILD
        DataSourceAccount   *           GetDataSourceAccount(void);
        const DataSource::SessionName & GetDataSourceSessionName(void);
#endif

        template<class EXTENT> SMGroupingStrategy<EXTENT>* GetStrategy()
            {
            if (!s_groupingStrategy)
                {
                switch (m_parametersPtr->GetStrategyType())
                    {
                    case SMGroupGlobalParameters::StrategyType::VIRTUAL:
                        {
                        s_groupingStrategy = new SMBentleyGroupingStrategy<EXTENT>(m_parametersPtr->GetStrategyType());
                        s_max_group_depth = 2;
                        break;
                        }
                    case SMGroupGlobalParameters::StrategyType::CESIUM:
                        {
                        s_groupingStrategy = new SMCesium3DTileStrategy<EXTENT>();
                        break;
                        }
                    default:
                        {
                        assert(false); // Unknown/invalid grouping strategy
                        }
                    }
                }
            assert(nullptr != s_groupingStrategy);

            return s_groupingStrategy;
            }

        void Save()
            {
            assert(!"Please use a strategy to save a group!");
            }

#ifndef LINUX_SCALABLEMESH_BUILD
        DataSource *InitializeDataSource();
#endif

        StatusInt Load();

        StatusInt Load(const uint64_t& priorityNodeID);

        void LoadGroupParallel();

        void SetDistributor(SMNodeDistributor<SMNodeGroup::DistributeData>& pi_pNodeDistributor)
            {
            m_nodeDistributorPtr = &pi_pNodeDistributor;
            }

        bool ContainsNode(const uint64_t& pi_pNodeID)
            {
            //if (!IsLoaded()) Load(pi_pNodeID);
            return (m_tileTreeMap.count(pi_pNodeID) == 1);
            //assert(!m_groupHeader->empty());
            //auto node = std::find_if(begin(*m_groupHeader), end(*m_groupHeader), [&](SMNodeHeader& nodeId)
            //    {
            //    return nodeId.blockid == pi_pNodeID;
            //    });
            //return node != m_groupHeader->end();
            }

        BENTLEY_SM_EXPORT void MergeChild(SMNodeGroupPtr child);

        SMNodeHeader* GetNodeHeader(const size_t& pi_pNodeHeaderID)
            {
            return (std::find_if(begin(*m_groupHeader), end(*m_groupHeader), [&](SMNodeHeader& nodeId)
                {
                return pi_pNodeHeaderID == nodeId.blockid;
                }));
            }

        static void SetWorkTo(SMNodeDistributor<DistributeData>& pi_pNodeDistributor)
            {
            // Initialize custom work specific to node header group parallel downloads
            pi_pNodeDistributor.InitCustomWork([](DistributeData data)
                {
#ifdef DEBUG_GROUPS
                        {
                   //     std::lock_guard<mutex> clk(s_consoleMutex);
                        std::cout << "Processing... " << data.first << std::endl;
                        }
#endif
                        uint64_t nodeID = data.first;
                        SMNodeGroup* group = data.second;
                        bvector<uint8_t> rawHeader;
                        auto headerSize = group->GetSingleNodeFromStore(nodeID, rawHeader);
                        assert(headerSize == rawHeader.size());
                        group->SetHeaderDataAtCurrentPosition(nodeID, rawHeader.data(), headerSize);
                        ++group->m_progress;
                        group->m_groupCV.notify_all();
                });

            }

        SMGroupGlobalParameters::Ptr GetParameters() { return m_parametersPtr; }
        SMGroupCache::Ptr GetCache() { return m_groupCachePtr; }

    private:

        // Constructor for reading a group
        SMNodeGroup(SMGroupGlobalParameters::Ptr parameters, SMGroupCache::Ptr cache, const uint32_t& pi_pID, size_t pi_pSize = 0, uint64_t pi_pTotalSizeOfHeaders = 0)
            : m_parametersPtr(parameters),
              m_groupCachePtr(cache),
              m_groupHeader(new SMGroupHeader(pi_pID, pi_pSize)),
              m_rawHeaders(pi_pTotalSizeOfHeaders)
            {
            };

        // Constructor for writing a group
        SMNodeGroup(SMGroupGlobalParameters::Ptr parameters, SMGroupCache::Ptr cache, const WString pi_pOutputDirPath, const uint32_t& pi_pGroupID, SMNodeGroupPtr parentGroup = nullptr)
            : m_parametersPtr(parameters),
            m_outputDirPath(pi_pOutputDirPath),
            m_groupHeader(new SMGroupHeader(pi_pGroupID)),
            m_ParentGroup(parentGroup)
            {
            // reserve space for total number of nodes for this group
            m_groupHeader->reserve(s_max_number_nodes_in_group);
            m_rawHeaders.reserve(3000 * s_max_number_nodes_in_group);

            // A group contains at least its ID and the number of nodes within it.
            m_totalSize = 2 * sizeof(size_t);

            m_isRoot = m_ParentGroup == nullptr;
            }

#ifndef LINUX_SCALABLEMESH_BUILD
        bool DownloadFromID(std::vector<DataSourceBuffer::BufferData>& dest)
            {
            wchar_t buffer[10000];
            swprintf(buffer, L"%s%lu%s", m_dataSourcePrefix.c_str(), this->GetID(), m_dataSourceExtension.c_str());

            return DownloadBlob(dest, DataSourceURL(buffer));
            }

        bool DownloadCesiumTileset(const DataSourceURL& url, Json::Value& tileset);

        bool DownloadBlob(std::vector<DataSourceBuffer::BufferData>& dest, const DataSourceURL& url);
#endif

        uint64_t GetSingleNodeFromStore(const uint64_t& pi_pNodeID, bvector<uint8_t>& pi_pData);

        StatusInt SaveNode(const uint64_t& id, Json::Value* header);

        StatusInt SaveTilesetToCache(Json::Value& tileset, const uint64_t& priorityNodeID, bool generateIDs = true);

        StatusInt SaveTileToCacheWithNewTileIDs(Json::Value& tile, uint64_t tileID, uint64_t parentID, bool isRootNode, uint64_t level);

        StatusInt SaveTileToCacheWithExistingTileIDs(Json::Value& tile);

        StatusInt SaveTileToCache(Json::Value& tile, uint64_t tileID);

        void WaitFor(SMNodeHeader& pi_pNode);
    };

class SMNodeGroupMasterHeader : public std::map<uint32_t, SMGroupNodeIds>, public HFCShareableObject<SMNodeGroupMasterHeader>
    {
    public:
        SMNodeGroupMasterHeader() {}

        SMNodeGroupMasterHeader(SMGroupGlobalParameters::Ptr parameters)
            : m_parametersPtr(parameters)
            {}

        void AddGroup(const uint32_t& pi_pGroupID, size_type pi_pCount = 10000)
            {
            auto& newGroup = this->operator[](pi_pGroupID);
            newGroup.reserve(s_max_number_nodes_in_group);
            }

        void RemoveGroup(const uint32_t& pi_pGroupID)
            {
            this->erase(pi_pGroupID);
            }

        void AddNodeToGroup(const uint32_t& pi_pGroupID, const uint64_t& pi_pNodeID, const uint64_t& pi_pNodeHeaderSize)
            {
            auto& group = this->operator[](pi_pGroupID);
            group.push_back(pi_pNodeID);
            group.m_sizeOfRawHeaders += pi_pNodeHeaderSize;
            }

        BENTLEY_SM_EXPORT void SaveToFile(const WString pi_pOutputDirPath) const;

        void SetOldMasterHeaderData(SQLiteIndexHeader pi_pOldMasterHeader)
            {
            // Save copy of master header for future reference
            m_masterHeader = pi_pOldMasterHeader;

            // Serialize master header
            m_oldMasterHeader.resize(sizeof(pi_pOldMasterHeader));
            memcpy(m_oldMasterHeader.data(), &pi_pOldMasterHeader, sizeof(pi_pOldMasterHeader));
            }

        bool IsBalanced() const { return m_masterHeader.m_balanced; }
        uint64_t GetSplitThreshold() const { return m_masterHeader.m_SplitTreshold; }
        uint64_t GetDepth() const { return m_masterHeader.m_depth; }
        SMTextureType IsTextured() const { return m_masterHeader.m_textured; }
        uint64_t GetTerrainDepth() const { return m_masterHeader.m_terrainDepth; }
        double GetResolution() const { return m_masterHeader.m_resolution; }
        bool IsTerrain() const { return m_masterHeader.m_isTerrain; }

    private:
        SMGroupGlobalParameters::Ptr m_parametersPtr;
        bvector<uint8_t> m_oldMasterHeader;
        SQLiteIndexHeader m_masterHeader;
    };


/**---------------------------------------------------------------------------------------------
    SMGroupingStrategy base class provides an interface for all node header grouping strategies
------------------------------------------------------------------------------------------------*/
template<class EXTENT>
class SMGroupingStrategy
    {
    public:
        void             Apply                      (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group);
        SMNodeGroupPtr   GetNextGroup               (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_CurrentGroup);
        void             ApplyPostProcess           (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_pGroup);
        void             ApplyPostChildNodeProcess  (SMIndexNodeHeader<EXTENT>& pi_parentHeader, SMIndexNodeHeader<EXTENT>& pi_childHeader, size_t childJSONIndex, SMNodeGroupPtr pi_pParentGroup, SMNodeGroupPtr& pi_pChildGroup);
        size_t           AddNodeToGroup             (SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group);
        void             AddGroup                   (SMNodeGroup* pi_pNodeGroup);
        uint64_t         GetClipID                  () { return m_clipID;}
        bool             IsClipBoundary             () { return m_isClipBoundary; }
        void             SetClipInfo                (const uint64_t& clipID, bool isClipBoundary);
        void             SetOldMasterHeader         (SMIndexMasterHeader<EXTENT>& oldMasterHeader);
        void             SaveAllOpenGroups          (bool saveRoot) const;
        void             SaveMasterHeader           (const WString pi_pOutputDirPath) const;
        void             SaveNodeGroup              (SMNodeGroupPtr pi_Group) const;
        void             SetSourceAndDestinationGCS(const GeoCoordinates::BaseGCSCPtr source, const GeoCoordinates::BaseGCSCPtr destination)
            {
            m_sourceGCS = source;
            m_destinationGCS = destination;
            }
        void             SetRootTransform(const Transform& transform) { m_rootTransform = transform; }
        void             SetTransform(const Transform& transform) { m_transform = transform; }
        void             Clear()
            {
            m_GroupID = 0;
            m_OpenGroups.clear();
            }

    protected:

        virtual void                _Apply                      (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group) = 0;
        virtual SMNodeGroupPtr      _GetNextGroup               (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_CurrentGroup) = 0;
        virtual void                _ApplyPostProcess           (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_pGroup) = 0;
        virtual void                _ApplyPostChildNodeProcess  (SMIndexNodeHeader<EXTENT>& pi_parentHeader, SMIndexNodeHeader<EXTENT>& pi_childHeader, size_t childJSONIndex, SMNodeGroupPtr pi_pParentGroup, SMNodeGroupPtr& pi_pChildGroup) = 0;
        virtual size_t              _AddNodeToGroup             (SMIndexNodeHeader<EXTENT> pi_NodeHeader, SMNodeGroupPtr pi_Group) = 0;
        virtual void                _AddGroup                   (SMNodeGroup* pi_pNodeGroup) = 0;
        virtual void                _SaveMasterHeader           (const WString pi_pOutputDirPath) const = 0;
        virtual void                _SaveNodeGroup              (SMNodeGroupPtr pi_Group) const = 0;

    protected:
        uint32_t m_GroupID = 0;
        std::map<uint32_t, SMNodeGroupPtr> m_OpenGroups;
        SMNodeGroupMasterHeader m_GroupMasterHeader;
        GeoCoordinates::BaseGCSCPtr m_sourceGCS;
        GeoCoordinates::BaseGCSCPtr m_destinationGCS;
        Transform m_rootTransform;
        Transform m_transform;
        bool m_isClipBoundary = false;
        uint64_t m_clipID = -1;
    };

template<class EXTENT> void SMGroupingStrategy<EXTENT>::SetClipInfo(const uint64_t& clipID, bool isClipBoundary)
    {
    m_clipID = clipID;
    m_isClipBoundary = isClipBoundary;
    }

template<class EXTENT> void SMGroupingStrategy<EXTENT>::SetOldMasterHeader(SMIndexMasterHeader<EXTENT>& oldMasterHeader)
    {
    m_GroupMasterHeader.SetOldMasterHeaderData(oldMasterHeader);
    }

/**----------------------------------------------------------------------------
This method saves all open groups in the Open Group map.

@param
-----------------------------------------------------------------------------*/
template<class EXTENT> void SMGroupingStrategy<EXTENT>::SaveAllOpenGroups(bool saveRoot) const
    {
    for (auto& openGroup : m_OpenGroups)
        {
        auto& group = openGroup.second;
        if (!saveRoot && group->IsRoot()) continue;
        if (!group->IsEmpty() && !group->IsFull())
            {
            group->Close<EXTENT>();
            }
        }
    }

template<class EXTENT> void SMGroupingStrategy<EXTENT>::SaveNodeGroup(SMNodeGroupPtr pi_Group) const
    {
    this->_SaveNodeGroup(pi_Group);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group)
    {
    this->_Apply(pi_NodeHeader, pi_Group);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::AddGroup(SMNodeGroup* pi_pNodeGroup)
    {
    this->_AddGroup(pi_pNodeGroup);
    }

template<class EXTENT>
size_t SMGroupingStrategy<EXTENT>::AddNodeToGroup(SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group)
    {
    return this->_AddNodeToGroup(pi_NodeHeader, pi_Group);
    }

template<class EXTENT>
SMNodeGroupPtr SMGroupingStrategy<EXTENT>::GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_CurrentGroup)
    {
    return this->_GetNextGroup(pi_NodeHeader, pi_CurrentGroup);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_pGroup)
    {
    this->_ApplyPostProcess(pi_NodeHeader, pi_pGroup);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::ApplyPostChildNodeProcess(SMIndexNodeHeader<EXTENT>& pi_parentHeader, SMIndexNodeHeader<EXTENT>& pi_childHeader, size_t childJSONIndex, SMNodeGroupPtr pi_pParentGroup, SMNodeGroupPtr& pi_pChildGroup)
    {
    this->_ApplyPostChildNodeProcess(pi_parentHeader, pi_childHeader, childJSONIndex, pi_pParentGroup, pi_pChildGroup);
    }


template<class EXTENT>
void SMGroupingStrategy<EXTENT>::SaveMasterHeader(const WString pi_pOutputDirPath) const
    {
    this->_SaveMasterHeader(pi_pOutputDirPath);
    }



/**---------------------------------------------------------------------------------------------
SMBentleyGroupingStrategy
------------------------------------------------------------------------------------------------*/
template<class EXTENT>
class SMBentleyGroupingStrategy : public SMGroupingStrategy<EXTENT>
    {
    public:

        SMBentleyGroupingStrategy(const SMGroupGlobalParameters::StrategyType& mode) : m_Mode(mode) {}

    protected:

        virtual void                _Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group);
        virtual void                _AddGroup(SMNodeGroup* pi_pNodeGroup);
        virtual size_t              _AddNodeToGroup(SMIndexNodeHeader<EXTENT> pi_NodeHeader, SMNodeGroupPtr pi_Group);
        virtual void                _ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_pGroup);
        virtual void                _ApplyPostChildNodeProcess(SMIndexNodeHeader<EXTENT>& pi_parentHeader, SMIndexNodeHeader<EXTENT>& pi_childHeader, size_t childJSONIndex, SMNodeGroupPtr pi_pParentGroup, SMNodeGroupPtr& pi_pChildGroup);
        virtual SMNodeGroupPtr    _GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_CurrentGroup);
        virtual void                _SaveMasterHeader(const WString pi_pOutputDirPath) const;
        virtual void                _SaveNodeGroup(SMNodeGroupPtr pi_Group) const;

    private:

        SMGroupGlobalParameters::StrategyType m_Mode;
    };

template<class EXTENT> 
void SMBentleyGroupingStrategy<EXTENT>::_Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group)
    {
    if (pi_Group->IsFull() || pi_Group->IsCommonAncestorTooFar((uint32_t)pi_NodeHeader.m_level))
        {
        pi_Group->Close<EXTENT>();
        pi_Group->Open(++m_GroupID);
        m_GroupMasterHeader.AddGroup(m_GroupID);
        }
    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_AddGroup(SMNodeGroup* pi_pNodeGroup)
    {
    m_OpenGroups[pi_pNodeGroup->GetLevel()] = pi_pNodeGroup;
    }

template<class EXTENT>
size_t SMBentleyGroupingStrategy<EXTENT>::_AddNodeToGroup(SMIndexNodeHeader<EXTENT> pi_NodeHeader, SMNodeGroupPtr pi_Group)
    {
    // Fetch node header data
    size_t headerSize = 0;
    std::unique_ptr<Byte> headerData = nullptr;
    SMStreamingStore<EXTENT>::SerializeHeaderToBinary(&pi_NodeHeader, headerData, headerSize);

    m_GroupMasterHeader.AddNodeToGroup(pi_Group->GetID(), (uint64_t)pi_NodeHeader.m_id.m_integerID, headerSize);

    pi_Group->AppendHeader((uint64_t)pi_NodeHeader.m_id.m_integerID, headerData.get(), headerSize);

    this->Apply(pi_NodeHeader, pi_Group);

    return headerSize;
    }

template<class EXTENT>
SMNodeGroupPtr SMBentleyGroupingStrategy<EXTENT>::_GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_CurrentGroup)
    {
    SMNodeGroupPtr nextGroup = pi_CurrentGroup->IsMaxDepthAchieved() ? nullptr : pi_CurrentGroup;
    if (!nextGroup.IsValid())
        {
        const uint32_t nextLevel = (uint32_t)pi_NodeHeader.m_level + 1;
        nextGroup = m_OpenGroups.count(nextLevel) > 0 ? m_OpenGroups[nextLevel] : nullptr;
        if (!nextGroup.IsValid())
            {
            nextGroup = new SMNodeGroup(pi_CurrentGroup->GetParameters(),
                                        pi_CurrentGroup->GetCache(),
                                        pi_CurrentGroup->GetFilePath(),
                                        ++m_GroupID);
            nextGroup->SetLevel(nextLevel);
            this->AddGroup(nextGroup.get());
            m_GroupMasterHeader.AddGroup(m_GroupID);
            }
        }
    assert((nextGroup == pi_CurrentGroup) || (nextGroup.IsValid()));
    return nextGroup;
    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_pGroup)
    {
    // Set eldest parent visited (reverse order of traversal) to maintain proximity of nodes in a group
    const uint32_t newAncestor = (uint32_t)pi_NodeHeader.m_level;
    for (auto rGroupIt = m_OpenGroups.rbegin(); rGroupIt != m_OpenGroups.rend(); ++rGroupIt)
        {
        auto& group = rGroupIt->second;
        auto& groupID = rGroupIt->first;
        if (newAncestor >= groupID) break;
        group->SetAncestor(newAncestor);
        }
    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_ApplyPostChildNodeProcess(SMIndexNodeHeader<EXTENT>& pi_parentHeader, SMIndexNodeHeader<EXTENT>& pi_childHeader, size_t childJSONIndex, SMNodeGroupPtr pi_pParentGroup, SMNodeGroupPtr& pi_pChildGroup)
    {

    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_SaveMasterHeader(const WString pi_pOutputDirPath) const
    {
    m_GroupMasterHeader.SaveToFile(pi_pOutputDirPath);
    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_SaveNodeGroup(SMNodeGroupPtr pi_Group) const
    {
    if (m_Mode == SMGroupGlobalParameters::StrategyType::VIRTUAL) return; // Don't need to save virtual groups, they will use normal headers to retrieve node header data
    if (pi_Group->IsEmpty()) return;

#ifndef LINUX_SCALABLEMESH_BUILD
    WString path(pi_Group->m_outputDirPath + L"\\g_");
    wchar_t buffer[10000];
    swprintf(buffer, L"%s%lu.bin", path.c_str(), pi_Group->GetID());
    std::wstring group_filename(buffer);
    BeFile file;
    if (OPEN_OR_CREATE_FILE(file, group_filename.c_str(), BeFileAccess::Write))
        {
        uint32_t NbChars = 0;
        uint32_t id = pi_Group->GetID();
        file.Write(&NbChars, &id, sizeof(id));
        assert(NbChars == sizeof(id));

        const size_t numNodes = pi_Group->m_groupHeader->size();
        file.Write(&NbChars, &numNodes, sizeof(numNodes));
        assert(NbChars == sizeof(numNodes));

        file.Write(&NbChars, pi_Group->m_groupHeader->data(), (uint32_t)numNodes * sizeof(SMNodeHeader));
        assert(NbChars == numNodes * sizeof(SMNodeHeader));

        size_t sizeHeaders = (uint32_t)pi_Group->GetSizeOfHeaders();
        file.Write(&NbChars, pi_Group->GetRawHeaders(0), (uint32_t)sizeHeaders);
        assert(NbChars == sizeHeaders);
        }
    else
        {
        assert(!"Problem creating new group file");
        }

    file.Close();
#endif
    }

/**---------------------------------------------------------------------------------------------
SMCesium3DTileStrategy
------------------------------------------------------------------------------------------------*/
template<class EXTENT>
class SMCesium3DTileStrategy : public SMGroupingStrategy<EXTENT>
    {
    protected:
        virtual void                _Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group);
        virtual void                _AddGroup(SMNodeGroup* pi_pNodeGroup);
        virtual size_t              _AddNodeToGroup(SMIndexNodeHeader<EXTENT> pi_NodeHeader, SMNodeGroupPtr pi_Group);
        virtual void                _ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_pGroup);
        virtual void                _ApplyPostChildNodeProcess(SMIndexNodeHeader<EXTENT>& pi_parentHeader, SMIndexNodeHeader<EXTENT>& pi_childHeader, size_t childJSONIndex, SMNodeGroupPtr pi_pParentGroup, SMNodeGroupPtr& pi_pChildGroup);
        virtual SMNodeGroupPtr      _GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_CurrentGroup);
        virtual void                _SaveMasterHeader(const WString pi_pOutputDirPath) const;
        virtual void                _SaveNodeGroup(SMNodeGroupPtr pi_Group) const;

    private:
        uint32_t m_MaxDepth = 5;
    };

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_Group)
    {
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_AddGroup(SMNodeGroup* pi_pNodeGroup)
    {
    m_OpenGroups[pi_pNodeGroup->GetID()] = pi_pNodeGroup;
    }

template<class EXTENT>
size_t SMCesium3DTileStrategy<EXTENT>::_AddNodeToGroup(SMIndexNodeHeader<EXTENT> pi_NodeHeader, SMNodeGroupPtr pi_Group)
    {
    Json::Value nodeTile;
    auto reprojectExtentHelper = [this](DRange3d& range, Json::Value& tile)
            {
            auto reprojectPointHelper = [this](DPoint3d& point)
                {
                GeoPoint inLatLong, outLatLong;
                if (m_sourceGCS->LatLongFromCartesian(inLatLong, point) != SUCCESS)
                    assert(false); // Error in reprojection
                if (m_sourceGCS->LatLongFromLatLong(outLatLong, inLatLong, *m_destinationGCS) != SUCCESS)
                    assert(false); // Error in reprojection
                if (m_destinationGCS->XYZFromLatLong(point, outLatLong) != SUCCESS)
                    assert(false); // Error in reprojection
                };
            //bvector<DPoint3d> corners(8);
            //range.get8Corners(corners.data());
            //for (auto& point : corners) reprojectPointHelper(point);
            //range = DRange3d::From(corners);
            DPoint3d    center = DPoint3d::FromInterpolate(range.low, .5, range.high);
            DPoint3d lx = DPoint3d::From(range.high.x, center.y, center.z);
            DPoint3d ly = DPoint3d::From(center.x, range.high.y, center.z);
            DPoint3d lz = DPoint3d::From(center.x, center.y, range.high.z);
            reprojectPointHelper(center);
            reprojectPointHelper(lx);
            reprojectPointHelper(ly);
            reprojectPointHelper(lz);
            DPoint3d halfAxesX, halfAxesY, halfAxesZ;
            halfAxesX.DifferenceOf(lx, center);
            halfAxesY.DifferenceOf(ly, center);
            halfAxesZ.DifferenceOf(lz, center);
            RotMatrix halfAxes = RotMatrix::FromRowValues(halfAxesX.x, halfAxesX.y, halfAxesX.z,
                                                          halfAxesY.x, halfAxesY.y, halfAxesY.z,
                                                          halfAxesZ.x, halfAxesZ.y, halfAxesZ.z);
            TilePublisher::WriteBoundingVolume(tile, center, halfAxes);
            //DPoint3d delta = DPoint3d::From(center.x - lx.x, center.y - ly.y, center.z - lz.z);
            //range.low = DPoint3d::From(lx.x, ly.y, lz.z);
            //range.high = DPoint3d::From(center.x + delta.x, center.y + delta.y, center.z + delta.z);
            };
    if (m_sourceGCS != nullptr && m_sourceGCS != m_destinationGCS)
        {
        reprojectExtentHelper(pi_NodeHeader.m_nodeExtent, nodeTile);
        }
    m_transform.Multiply(pi_NodeHeader.m_nodeExtent, pi_NodeHeader.m_nodeExtent);
    TilePublisher::WriteBoundingVolume(nodeTile, pi_NodeHeader.m_nodeExtent);
    if (pi_NodeHeader.m_nodeCount > 0 && pi_NodeHeader.m_contentExtentDefined && !pi_NodeHeader.m_contentExtent.IsNull())
        {
        if (m_sourceGCS != nullptr && m_sourceGCS != m_destinationGCS)
            {
            reprojectExtentHelper(pi_NodeHeader.m_contentExtent, nodeTile["content"]);
            }
        m_transform.Multiply(pi_NodeHeader.m_contentExtent, pi_NodeHeader.m_contentExtent);
        TilePublisher::WriteBoundingVolume(nodeTile, pi_NodeHeader.m_contentExtent);
        }

    SMStreamingStore<EXTENT>::SerializeHeaderToCesium3DTileJSON(&pi_NodeHeader, pi_NodeHeader.m_id, nodeTile);
    pi_Group->Append3DTile(pi_NodeHeader.m_id.m_integerID, pi_NodeHeader.m_parentNodeID.m_integerID, nodeTile);
    m_GroupMasterHeader.AddNodeToGroup(pi_Group->GetID(), (uint64_t)pi_NodeHeader.m_id.m_integerID, 0);
    return 0;
    }

template<class EXTENT>
SMNodeGroupPtr SMCesium3DTileStrategy<EXTENT>::_GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_CurrentGroup)
    {
    SMNodeGroupPtr nextGroup = pi_CurrentGroup->IsMaxDepthAchieved() ? nullptr : pi_CurrentGroup;
    if (!nextGroup.IsValid())
        {
        nextGroup = new SMNodeGroup(pi_CurrentGroup->GetParameters(),
                                    pi_CurrentGroup->GetCache(),
                                    pi_CurrentGroup->GetFilePath(),
                                    ++m_GroupID,
                                    pi_CurrentGroup);
        this->AddGroup(nextGroup.get());
        }
    assert((nextGroup == pi_CurrentGroup) || (nextGroup.IsValid()));
    return nextGroup;
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroupPtr pi_pGroup)
    {
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_ApplyPostChildNodeProcess(SMIndexNodeHeader<EXTENT>& pi_parentHeader, SMIndexNodeHeader<EXTENT>& pi_childHeader, size_t childJSONIndex, SMNodeGroupPtr pi_pParentGroup, SMNodeGroupPtr& pi_pChildGroup)
    {
    if (pi_pChildGroup != pi_pParentGroup)
        {
        if (pi_pChildGroup->m_tileTreeMap.size() < 10)
            {
            pi_pParentGroup->MergeChild(pi_pChildGroup);
            //for (auto& tile : pi_pChildGroup->m_tileTreeMap)
            //    {
            //    this->m_GroupMasterHeader.AddNodeToGroup(pi_pParentGroup->GetID(), tile.first, 0);
            //    }
            //this->m_GroupMasterHeader.RemoveGroup(pi_pChildGroup->GetID());
            pi_pChildGroup->m_tileTreeMap.clear();
            }
        else
            {
            SMNodeGroupPtr currentGroup = pi_pChildGroup;
            pi_pChildGroup = this->GetNextGroup(pi_parentHeader, pi_pParentGroup);
            assert(currentGroup != pi_pChildGroup);
            currentGroup->Close<EXTENT>();
            Json::Value& parentJSON = *pi_pParentGroup->m_tileTreeMap[pi_parentHeader.m_id.m_integerID];
            Json::Value& correspondingChildJSON = parentJSON["children"][(Json::Value::ArrayIndex)childJSONIndex];
            assert(correspondingChildJSON.isMember("boundingVolume"));
            }
        }
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_SaveMasterHeader(const WString pi_pOutputDirPath) const
    {
    // Save SM master header
    m_GroupMasterHeader.SaveToFile(pi_pOutputDirPath/*, SMNodeGroup::StrategyType::CESIUM*/);

    // NEEDS_WORK_SM_STREAMING: Save Cesium master header 
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_SaveNodeGroup(SMNodeGroupPtr pi_Group) const
    {
    Json::Value tileSet;
    tileSet["asset"]["version"] = "1.0";
    tileSet["asset"]["gltfUpAxis"] = "Z";
    tileSet["root"] = pi_Group->m_tilesetRootNode;
    tileSet["geometricError"] = tileSet["root"]["geometricError"].asFloat();
    if (pi_Group->IsRoot())
        {
        tileSet["root"]["transform"] = Json::Value(Json::arrayValue);
        auto& arrayValue = tileSet["root"]["transform"];

        auto matrix = DMatrix4d::From(m_rootTransform);
        for (size_t i = 0; i<4; i++)
            for (size_t j = 0; j<4; j++)
                arrayValue.append(matrix.coff[j][i]);

        // Save master header info in Cesium tileset
        auto& SMMasterHeader = tileSet["root"]["SMMasterHeader"];
        SMMasterHeader["Balanced"] = m_GroupMasterHeader.IsBalanced();
        SMMasterHeader["SplitTreshold"] = Json::Value(m_GroupMasterHeader.GetSplitThreshold());
        SMMasterHeader["Depth"] = Json::Value(m_GroupMasterHeader.GetDepth());
        SMMasterHeader["MeshDataDepth"] = Json::Value(m_GroupMasterHeader.GetTerrainDepth());
        SMMasterHeader["IsTerrain"] = m_GroupMasterHeader.IsTerrain();
        SMMasterHeader["DataResolution"] = m_GroupMasterHeader.GetResolution();
        SMMasterHeader["IsTextured"] = (uint32_t)m_GroupMasterHeader.IsTextured();
        auto wktString = pi_Group->GetParameters()->GetWellKnownText();
        if (!wktString.empty())
            SMMasterHeader["GCS"] = Utf8String(wktString.c_str());
#ifndef VANCOUVER_API
		SMMasterHeader["LastModifiedDateTime"] = DateTime::GetCurrentTimeUtc().ToString();
#else
        SMMasterHeader["LastModifiedDateTime"] = DateTime::GetCurrentTimeUtc().ToUtf8String();
#endif

        SMMasterHeader["tileToDb"] = Json::Value(Json::arrayValue);
        Transform tileToDb;
        tileToDb.InverseOf(m_transform);
        matrix = DMatrix4d::From(tileToDb);
        for (size_t i = 0; i<4; i++)
            for (size_t j = 0; j<4; j++)
                SMMasterHeader["tileToDb"].append(matrix.coff[j][i]);
        }

    //std::cout << "#nodes in group(" << pi_Group->m_groupHeader->GetID() << ") = " << pi_Group->m_tileTreeMap.size() << std::endl;

    auto utf8TileTree = Json::FastWriter().write(tileSet);
#ifndef LINUX_SCALABLEMESH_BUILD
    WString path(pi_Group->m_outputDirPath + L"\\n_");
    wchar_t buffer[10000];
    swprintf(buffer, L"%s%lu.json", path.c_str(), pi_Group->GetID());
    std::wstring group_filename(buffer);

    BeFile file;
    if (OPEN_OR_CREATE_FILE(file, group_filename.c_str(), BeFileAccess::Write))
        {
        file.Write(nullptr, utf8TileTree.c_str(), (uint32_t)utf8TileTree.size());
        }
#endif
    }
