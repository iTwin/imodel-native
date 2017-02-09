//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once




#include <ImagePP/all/h/HFCPtr.h>
#include "Threading\LightThreadPool.h"
#include "SMSQLiteFile.h"
#include "Stores/SMStoreUtils.h"
#include <condition_variable>
#include <CloudDataSource/DataSourceManager.h>
#include <queue>
#include <iomanip>

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
extern std::mutex s_consoleMutex;
//#endif

#ifdef DEBUG_AZURE
#include <Bentley\BeConsole.h>
#endif

class DataSourceAccount;

extern bool s_stream_enable_caching;

extern uint32_t s_max_number_nodes_in_group;
extern size_t   s_max_group_size;
extern uint32_t s_max_group_depth;
extern uint32_t s_max_group_common_ancestor;

//extern std::mutex fileMutex;

struct SMNodeHeader {
    uint64_t blockid;
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
class SMNodeDistributor : Queue, std::mutex, std::condition_variable, public HFCShareableObject<SMNodeDistributor<Type, Queue> > {
    typename Queue::size_type capacity;
    unsigned int m_concurrency;
    bool m_done = false;
    std::vector<std::thread> m_threads;
    std::function<void(Type)> m_workFunction;

public:
    typedef HFCPtr<SMNodeDistributor<Type, Queue>> Ptr;
    SMNodeDistributor(unsigned int concurrency = std::thread::hardware_concurrency()
                      //, unsigned int concurrency = 2
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
                      , unsigned int concurrency = std::thread::hardware_concurrency()
                      //, unsigned int concurrency = 2
                      , typename Queue::size_type max_items_per_thread = 5000
    )
        : capacity{ concurrency * max_items_per_thread }
        {
        if (!concurrency)
            throw std::invalid_argument("Concurrency must be non-zero");
        if (!max_items_per_thread)
            throw std::invalid_argument("Max items per thread must be non-zero");

        for (unsigned int count{ 0 }; count < concurrency; count += 1)
            m_threads.emplace_back(static_cast<void (SMNodeDistributor::*)(Function)>
            (&SMNodeDistributor::Consume), this, function);
        }

    //    SMNodeDistributor(SMNodeDistributor &&) = default;
    //    SMNodeDistributor &operator=(SMNodeDistributor &&) = delete;

    ~SMNodeDistributor()
        {
        this->CancelAll();
        {
        std::unique_lock<std::mutex> lock(*this);
        while (!wait_for(lock, 1000ms, [this]
            {
            return Queue::empty();
            }))
            {
                    {
                    std::lock_guard<mutex> clk(s_consoleMutex);
                    std::cout << std::setw(100) << "\r  Queue size (" << Queue::size() << ")                          ";
                    }

            }
            {
            std::lock_guard<mutex> clk(s_consoleMutex);
            std::cout << std::setw(100) << "\r  Queue size (" << Queue::size() << ")                          \n";
            }
        }
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
            while (!wait_for(lock, 1000ms, [this]
                {
                return Queue::size() < capacity / 2;
                }))
                {
//#ifdef DEBUG_GROUPS
                        {
                        std::lock_guard<mutex> clk(s_consoleMutex);
                        std::cout << "\r  Queue size (" << Queue::size() << ")                          ";
                        }
//#endif

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

class SMNodeGroup : public BENTLEY_NAMESPACE_NAME::RefCountedBase
    {
    ADD_GROUPING_STRATEGY_FRIENDSHIPS

    public:
        enum StrategyType
            {
            NONE,
            NORMAL,
            VIRTUAL,
            CESIUM
            };
        typedef std::pair<uint64_t, SMNodeGroup*> DistributeData;
        typedef BENTLEY_NAMESPACE_NAME::RefCountedPtr<SMNodeGroup> Ptr;

    private:
        bool   m_isLoaded = false;
        bool   m_isLoading = false;
        StrategyType   m_strategyType = NORMAL;
        uint32_t m_level = 0;
        size_t m_totalSize;
        uint32_t m_nLevels = 0;
        uint32_t m_depth = 0;
        uint32_t m_ancestor = -1;
        uint64_t m_currentPosition = 0;
        uint64_t m_progress = 0;
        uint32_t m_maxGroupDepth = 0;
        bvector<uint8_t> m_rawHeaders;
        unordered_map<uint64_t, Json::Value*> m_tileTreeMap;
        map<uint64_t, SMNodeGroup::Ptr> m_tileTreeChildrenGroups;
        SMNodeGroup::Ptr m_ParentGroup;
        Json::Value m_RootTileTreeNode;
        WString m_outputDirPath;
        WString m_dataSourcePrefix;
        WString m_dataSourceExtension = L".bin";
        HFCPtr<SMGroupHeader> m_groupHeader;
        SMNodeDistributor<SMNodeGroup::DistributeData>::Ptr m_nodeDistributorPtr;
        condition_variable m_groupCV;
        mutex m_groupMutex;
        DataSourceAccount *m_dataSourceAccount;

    public:
        // Constructor for reading a group
        SMNodeGroup(DataSourceAccount *dataSourceAccount, const uint32_t& pi_pID, StrategyType strategyType, size_t pi_pSize = 0, uint64_t pi_pTotalSizeOfHeaders = 0)
            : m_dataSourceAccount(dataSourceAccount),
              m_groupHeader(new SMGroupHeader(pi_pID, pi_pSize)),
              m_rawHeaders(pi_pTotalSizeOfHeaders),
              m_strategyType(strategyType)
            {
            };

        // Constructor for writing a group
        SMNodeGroup(DataSourceAccount *dataSourceAccount, const WString pi_pOutputDirPath, const uint32_t& pi_pGroupID, SMNodeGroup::Ptr parentGroup = nullptr, StrategyType strategyType = StrategyType::NONE)
            : m_dataSourceAccount(dataSourceAccount),
              m_outputDirPath(pi_pOutputDirPath),
              m_groupHeader(new SMGroupHeader(pi_pGroupID)),
              m_strategyType(strategyType),
              m_ParentGroup(parentGroup)
            {
            // reserve space for total number of nodes for this group
            m_groupHeader->reserve(s_max_number_nodes_in_group);
            m_rawHeaders.reserve(3000 * s_max_number_nodes_in_group);

            // A group contains at least its ID and the number of nodes within it.
            m_totalSize = 2 * sizeof(size_t);
            }

        static SMNodeGroup::Ptr CreateCesium3DTilesGroup(DataSourceAccount *dataSourceAccount, const uint32_t groupID)
            {
            return new SMNodeGroup(dataSourceAccount, groupID, StrategyType::CESIUM);
            }

        void AppendHeader(const uint64_t& nodeID, const uint8_t* headerData, const uint64_t& size)
            {
            size_t oldSize, offset;
            oldSize = offset = m_rawHeaders.size();
            m_groupHeader->AddNode(SMNodeHeader{ nodeID, (uint32_t)offset, size });

            m_rawHeaders.resize(oldSize + size);
            memmove(&m_rawHeaders[oldSize], headerData, size);
            m_totalSize += size + sizeof(SMNodeHeader);
            }

        void AppendHeader(const uint64_t& nodeID, Json::Value& jsonHeader)
            {
            assert(m_tileTreeMap.count(nodeID) == 0);
            m_tileTreeMap[nodeID] = &jsonHeader;
            }

        uint32_t GetLevel() { return m_level; }

        void SetLevel(const uint32_t& pi_NewID) { m_level = pi_NewID; }

        void Append3DTile(const uint64_t& nodeID, const uint64_t& parentNodeID, const Json::Value& tile);

        uint32_t GetID() { return m_groupHeader->GetID(); }

        void SetID(const uint32_t& pi_NewID) { m_groupHeader->SetID(pi_NewID); }

        void SetAncestor(const uint32_t& pi_pNewAncestor) { m_ancestor = pi_pNewAncestor; }

        void SetDataSourcePrefix(const WString& pi_pDataSourcePrefix) { m_dataSourcePrefix = pi_pDataSourcePrefix; }

        void SetDataSourceExtension(const WString& pi_pDataSourceExtension) { m_dataSourceExtension = pi_pDataSourceExtension; }

        size_t GetNumberNodes() { return m_groupHeader->size(); }

        size_t GetSizeOfHeaders() { return m_rawHeaders.size(); }

        bvector<Byte>::pointer GetRawHeaders(const size_t& offset) { return m_rawHeaders.data() + offset; }

        Json::Value GetJsonHeader(const uint64_t& id) 
            {
            assert(m_tileTreeMap.count(id) == 1);
            return *m_tileTreeMap[id];
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

        template<class EXTENT> uint32_t AddNode(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader)
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

        void SetDataSourceAccount(DataSourceAccount *dataSourceAccount)
            {
            m_dataSourceAccount = dataSourceAccount;
            }

        DataSourceAccount *GetDataSourceAccount(void)
            {
            return m_dataSourceAccount;
            }

        bool HasChildGroups()
            {
            return !m_tileTreeChildrenGroups.empty();
            }

        map<uint64_t, SMNodeGroup::Ptr>& GetChildGroups()
            {
            return m_tileTreeChildrenGroups;
            }

        void ClearChildGroups()
            {
            m_tileTreeChildrenGroups.clear();
            }

        template<class EXTENT> SMGroupingStrategy<EXTENT>* GetStrategy()
            {
            static SMGroupingStrategy<EXTENT>* s_groupingStrategy = nullptr;
            if (!s_groupingStrategy)
                {
                switch (m_strategyType)
                    {
                    case StrategyType::NORMAL:
                    case StrategyType::VIRTUAL:
                        {
                        s_groupingStrategy = new SMBentleyGroupingStrategy<EXTENT>(m_strategyType);
                        s_max_group_depth = 2;
                        break;
                        }
                    case StrategyType::CESIUM:
                        {
                        s_groupingStrategy = new SMCesium3DTileStrategy<EXTENT>();
                        break;
                        }
                    default:
                        {
                        assert(false); // Unknown/invalid grouping strategy
                        }
                    }
                // Add this group as the first group to the grouping strategy
                s_groupingStrategy->AddGroup(this);
                }
            assert(nullptr != s_groupingStrategy);

            return s_groupingStrategy;
            }

        void Save()
            {
            assert(!"Please use a strategy to save a group!");
            }

        DataSource *InitializeDataSource(std::unique_ptr<DataSource::Buffer[]> &dest, DataSourceBuffer::BufferSize destSize)
            {
            assert(this->GetDataSourceAccount() != nullptr);

            // Get the thread's DataSource or create a new one
            DataSource *dataSource = this->GetDataSourceAccount()->getOrCreateThreadDataSource();
            if (dataSource == nullptr)
                return nullptr;
            // Make sure caching is enabled for this DataSource
            dataSource->setCachingEnabled(s_stream_enable_caching);

            dest.reset(new unsigned char[destSize]);
            // Return the DataSource
            return dataSource;
            }

        StatusInt Load()
            {
            unique_lock<mutex> lk(m_groupMutex);
            if (m_isLoading)
                {
                m_groupCV.wait(lk, [this] {return !m_isLoading; });
                }
            else
                {
                m_isLoading = true;

                if (m_strategyType == VIRTUAL)
                    {
                    this->LoadGroupParallel();
                    }
                else
                    {
                    std::unique_ptr<DataSource::Buffer[]> dest;
                    DataSource*                           dataSource;
                    DataSource::DataSize                  readSize;
                    DataSourceBuffer::BufferSize          destSize = 5 * 1024 * 1024;

                    m_isLoading = true;

                    dataSource = this->InitializeDataSource(dest, destSize);
                    if (dataSource == nullptr)
                        {
                        m_isLoading = false;
                        m_groupCV.notify_all();
                        return ERROR;
                        }

                    this->LoadFromDataSource(dataSource, dest.get(), destSize, readSize);

                    if (readSize > 0)
                        {
                        uint32_t position = 0;
                        uint32_t id;
                        memcpy(&id, dest.get(), sizeof(uint32_t));
                        assert(m_groupHeader->GetID() == id);
                        position += sizeof(uint32_t);

                        uint32_t numNodes;
                        memcpy(&numNodes, dest.get() + position, sizeof(numNodes));
                        assert(m_groupHeader->size() == numNodes);
                        position += sizeof(numNodes);

                        memcpy(m_groupHeader->data(), dest.get() + position, numNodes * sizeof(SMNodeHeader));
                        position += numNodes * sizeof(SMNodeHeader);

                        const auto headerSectionSize = readSize - position;
                        m_rawHeaders.resize(headerSectionSize);
                        memcpy(m_rawHeaders.data(), dest.get() + position, headerSectionSize);
                        }
                    else
                        {
                        m_isLoading = false;
                        m_groupCV.notify_all();
                        return ERROR;
                        }
                    }

                m_isLoading = false;
                m_groupCV.notify_all();
                }

            m_isLoaded = true;
            return SUCCESS;
            }

        StatusInt Load(const uint64_t& priorityNodeID);

        void LoadGroupParallel();

        void SetDistributor(SMNodeDistributor<SMNodeGroup::DistributeData>& pi_pNodeDistributor)
            {
            m_nodeDistributorPtr = &pi_pNodeDistributor;
            }

        bool ContainsNode(const uint64_t& pi_pNodeID)
            {
            assert(!m_groupHeader->empty());
            auto node = std::find_if(begin(*m_groupHeader), end(*m_groupHeader), [&](SMNodeHeader& nodeId)
                {
                return nodeId.blockid == pi_pNodeID;
                });
            return node != m_groupHeader->end();
            }

        void MergeChild(SMNodeGroup::Ptr child);

        SMNodeHeader* GetNodeHeader(const uint64_t& pi_pNodeHeaderID)
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
                        std::lock_guard<mutex> clk(s_consoleMutex);
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

    private:

        void LoadFromDataSource(DataSource *dataSource, DataSource::Buffer *dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize)
            {
            if (dataSource == nullptr)
                return;

            wchar_t buffer[10000];
            swprintf(buffer, L"%s%lu%s", m_dataSourcePrefix.c_str(), this->GetID(), m_dataSourceExtension.c_str());

            DataSourceURL dataSourceURL(buffer);

            if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                {
                assert(!"Couldn't open data source.");
                return;
                }

            if (dataSource->read(dest, destSize, readSize, 0).isFailed())
                {
                assert(!"Couldn't read data source.");
                return;
                }

            if (dataSource->close().isFailed())
                {
                assert(!"Couldn't close data source.");
                return;
                }
            //this->GetDataSourceAccount()->destroyDataSource(dataSource);
            }

        uint64_t GetSingleNodeFromStore(const uint64_t& pi_pNodeID, bvector<uint8_t>& pi_pData)
            {

            std::unique_ptr<DataSource::Buffer[]>dest;
            DataSource*                          dataSource;
            DataSource::DataSize                 readSize;
            DataSourceBuffer::BufferSize         destSize = 5 * 1024 * 1024;

            DataSourceURL dataSourceURL(m_dataSourcePrefix.c_str());
            dataSourceURL += std::to_wstring(pi_pNodeID) + m_dataSourceExtension.c_str();

            dataSource = this->InitializeDataSource(dest, destSize);
            if (dataSource == nullptr)
                {
                assert(false);
                return 0;
                }

            if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                {
                assert(false);
                return 0;
                }

            if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
                {
                assert(false);
                return 0;
                }

            if (dataSource->close().isFailed())
                {
                assert(false);
                return 0;
                }

            //          this->GetDataSourceAccount()->destroyDataSource(dataSource);

            if (readSize > 0)
                {
                pi_pData.resize(readSize);
                memmove(pi_pData.data(), reinterpret_cast<char *>(dest.get()), readSize);
                }

            return readSize;
            }

        void WaitFor(SMNodeHeader& pi_pNode);
    };

class SMNodeGroupMasterHeader : public std::map<uint32_t, SMGroupNodeIds>, public HFCShareableObject<SMNodeGroupMasterHeader>
    {
    public:
        SMNodeGroupMasterHeader() {}

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

        void SaveToFile(const WString pi_pOutputDirPath, const SMNodeGroup::StrategyType& pi_pGroupMode) const
            {
            assert(!m_oldMasterHeader.empty()); // Old master header must be set!

                                                // NEEDS_WORK_SM_STREAMING : use new CloudDataSource
            wchar_t buffer[10000];
            switch (pi_pGroupMode)
                {
                case SMNodeGroup::StrategyType::NORMAL:
                    {
                    swprintf(buffer, L"%s/MasterHeaderWith%sGroups.bin", pi_pOutputDirPath.c_str(), L"");
                    break;
                    }
                case SMNodeGroup::StrategyType::VIRTUAL:
                    {
                    swprintf(buffer, L"%s/MasterHeaderWith%sGroups.bin", pi_pOutputDirPath.c_str(), L"Virtual");
                    break;
                    }
                case SMNodeGroup::StrategyType::CESIUM:
                    {
                    swprintf(buffer, L"%s/MasterHeaderWith%sGroups.bin", pi_pOutputDirPath.c_str(), L"Cesium");
                    break;
                    }
                default:
                    {
                    assert(!"Unknown grouping type");
                    return;
                    }
                }

            // Put group information in a single binary blob
            bvector<uint8_t> masterBlob(20 * 1000 * 1000);

            // Old Master Header part
            auto const oldMasterHeaderSize = m_oldMasterHeader.size();
            size_t totalSize = 0;

            memcpy(masterBlob.data(), &oldMasterHeaderSize, sizeof(oldMasterHeaderSize));
            totalSize += sizeof(uint32_t);

            memcpy(masterBlob.data() + totalSize, m_oldMasterHeader.data(), oldMasterHeaderSize);
            totalSize += m_oldMasterHeader.size();

            short mode = (short)pi_pGroupMode;
            memcpy(masterBlob.data() + totalSize, &mode, sizeof(mode));
            totalSize += sizeof(mode);

            // Append group information
            for (auto& group : *this)
                {
                auto const& groupInfo = group.second;
                auto const gid = group.first;
                auto const gNumNodes = groupInfo.size();

                uint64_t group_size = sizeof(gid) + sizeof(groupInfo.m_sizeOfRawHeaders) + sizeof(gNumNodes) + gNumNodes * sizeof(uint64_t);
                if (totalSize + group_size > masterBlob.size())
                    {
                    // increase the size of the blob
                    size_t oldSize = masterBlob.size();
                    size_t newSize = oldSize + 1000 * 1000;
                    masterBlob.resize(newSize);
                    }
                memcpy(masterBlob.data() + totalSize, &gid, sizeof(gid));
                totalSize += sizeof(gid);

                // Group total size of headers
                if (pi_pGroupMode == SMNodeGroup::VIRTUAL)
                    {
                    memcpy(masterBlob.data() + totalSize, &groupInfo.m_sizeOfRawHeaders, sizeof(groupInfo.m_sizeOfRawHeaders));
                    totalSize += sizeof(groupInfo.m_sizeOfRawHeaders);
                    }

                // Group number of nodes
                memcpy(masterBlob.data() + totalSize, &gNumNodes, sizeof(gNumNodes));
                totalSize += sizeof(gNumNodes);

                // Group node ids
                memcpy(masterBlob.data() + totalSize, groupInfo.data(), gNumNodes * sizeof(uint64_t));
                totalSize += gNumNodes * sizeof(uint64_t);
                }
            HCDPacket uncompressedPacket, compressedPacket;
            uncompressedPacket.SetBuffer(masterBlob.data(), totalSize);
            uncompressedPacket.SetDataSize(totalSize);
            WriteCompressedPacket(uncompressedPacket, compressedPacket);

            std::wstring group_header_filename(buffer);
            BeFile file;
            if (OPEN_OR_CREATE_FILE(file, group_header_filename.c_str(), BeFileAccess::Write))
                {
                uint32_t NbChars = 0;
                file.Write(&NbChars, &totalSize, (uint32_t)sizeof(totalSize));
                assert(NbChars == (uint32_t)sizeof(totalSize));

                file.Write(&NbChars, compressedPacket.GetBufferAddress(), (uint32_t)compressedPacket.GetDataSize());
                assert(NbChars == compressedPacket.GetDataSize());
                }
            else 
                {
                assert(!"Could not open or create file for writing the group master header");
                }
            file.Close();
            }

        void SetOldMasterHeaderData(SQLiteIndexHeader pi_pOldMasterHeader)
            {
            // Serialize master header
            m_oldMasterHeader.resize(sizeof(pi_pOldMasterHeader));
            memcpy(m_oldMasterHeader.data(), &pi_pOldMasterHeader, sizeof(pi_pOldMasterHeader));
            }
    private:
        bvector<uint8_t> m_oldMasterHeader;
    };


/**---------------------------------------------------------------------------------------------
    SMGroupingStrategy base class provides an interface for all node header grouping strategies
------------------------------------------------------------------------------------------------*/
template<class EXTENT>
class SMGroupingStrategy
    {
    public:
        void             Apply                      (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        SMNodeGroup::Ptr GetNextGroup               (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_CurrentGroup);
        void             ApplyPostProcess           (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pGroup);
        void             ApplyPostChildNodeProcess  (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pParentGroup, SMNodeGroup::Ptr& pi_pChildGroup);
        uint32_t         AddNodeToGroup             (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        void             AddGroup                   (SMNodeGroup* pi_pNodeGroup);
        void             SetOldMasterHeader         (SMIndexMasterHeader<EXTENT>& oldMasterHeader);
        void             SaveAllOpenGroups          () const;
        void             SaveMasterHeader           (const WString pi_pOutputDirPath) const;
        void             SaveNodeGroup              (SMNodeGroup::Ptr pi_Group) const;

    protected:

        virtual void                _Apply                      (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group) = 0;
        virtual SMNodeGroup::Ptr    _GetNextGroup               (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_CurrentGroup) = 0;
        virtual void                _ApplyPostProcess           (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pGroup) = 0;
        virtual void                _ApplyPostChildNodeProcess  (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pParentGroup, SMNodeGroup::Ptr& pi_pChildGroup) = 0;
        virtual uint32_t            _AddNodeToGroup             (const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group) = 0;
        virtual void                _AddGroup                   (SMNodeGroup* pi_pNodeGroup) = 0;
        virtual void                _SaveMasterHeader           (const WString pi_pOutputDirPath) const = 0;
        virtual void                _SaveNodeGroup              (SMNodeGroup::Ptr pi_Group) const = 0;

    protected:
        uint32_t m_GroupID = 0;
        std::map<uint32_t, SMNodeGroup::Ptr> m_OpenGroups;
        SMIndexMasterHeader<EXTENT> m_oldMasterHeader;
        SMNodeGroupMasterHeader m_GroupMasterHeader;
    };

template<class EXTENT> void SMGroupingStrategy<EXTENT>::SetOldMasterHeader(SMIndexMasterHeader<EXTENT>& oldMasterHeader)
    {
    m_oldMasterHeader = oldMasterHeader;
    m_GroupMasterHeader.SetOldMasterHeaderData(oldMasterHeader);
    }

/**----------------------------------------------------------------------------
This method saves all open groups in the Open Group map.

@param
-----------------------------------------------------------------------------*/
template<class EXTENT> void SMGroupingStrategy<EXTENT>::SaveAllOpenGroups() const
    {
    for (auto& openGroup : m_OpenGroups)
        {
        auto& group = openGroup.second;
        if (!group->IsEmpty() && !group->IsFull())
            {
            group->Close<EXTENT>();
            }
        }
    }

template<class EXTENT> void SMGroupingStrategy<EXTENT>::SaveNodeGroup(SMNodeGroup::Ptr pi_Group) const
    {
    this->_SaveNodeGroup(pi_Group);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    this->_Apply(pi_NodeHeader, pi_Group);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::AddGroup(SMNodeGroup* pi_pNodeGroup)
    {
    this->_AddGroup(pi_pNodeGroup);
    }

template<class EXTENT>
uint32_t SMGroupingStrategy<EXTENT>::AddNodeToGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    return this->_AddNodeToGroup(pi_NodeHeader, pi_Group);
    }

template<class EXTENT>
SMNodeGroup::Ptr SMGroupingStrategy<EXTENT>::GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_CurrentGroup)
    {
    return this->_GetNextGroup(pi_NodeHeader, pi_CurrentGroup);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pGroup)
    {
    this->_ApplyPostProcess(pi_NodeHeader, pi_pGroup);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::ApplyPostChildNodeProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pParentGroup, SMNodeGroup::Ptr& pi_pChildGroup)
    {
    this->_ApplyPostChildNodeProcess(pi_NodeHeader, pi_pParentGroup, pi_pChildGroup);
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

        SMBentleyGroupingStrategy(const SMNodeGroup::StrategyType& mode) : m_Mode(mode) {}

    protected:

        virtual void                _Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        virtual void                _AddGroup(SMNodeGroup* pi_pNodeGroup);
        virtual uint32_t            _AddNodeToGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        virtual void                _ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pGroup);
        virtual void                _ApplyPostChildNodeProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pParentGroup, SMNodeGroup::Ptr& pi_pChildGroup);
        virtual SMNodeGroup::Ptr    _GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_CurrentGroup);
        virtual void                _SaveMasterHeader(const WString pi_pOutputDirPath) const;
        virtual void                _SaveNodeGroup(SMNodeGroup::Ptr pi_Group) const;

    private:

        SMNodeGroup::StrategyType m_Mode;
    };

template<class EXTENT> 
void SMBentleyGroupingStrategy<EXTENT>::_Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
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
uint32_t SMBentleyGroupingStrategy<EXTENT>::_AddNodeToGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    // Fetch node header data
    uint32_t headerSize = 0;
    std::unique_ptr<Byte> headerData = nullptr;
    SMStreamingStore<EXTENT>::SerializeHeaderToBinary(&pi_NodeHeader, headerData, headerSize);

    m_GroupMasterHeader.AddNodeToGroup(pi_Group->GetID(), (uint64_t)pi_NodeHeader.m_id.m_integerID, headerSize);

    pi_Group->AppendHeader((uint64_t)pi_NodeHeader.m_id.m_integerID, headerData.get(), headerSize);

    this->Apply(pi_NodeHeader, pi_Group);

    return headerSize;
    }

template<class EXTENT>
SMNodeGroup::Ptr SMBentleyGroupingStrategy<EXTENT>::_GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_CurrentGroup)
    {
    SMNodeGroup::Ptr nextGroup = pi_CurrentGroup->IsMaxDepthAchieved() ? nullptr : pi_CurrentGroup;
    if (!nextGroup.IsValid())
        {
        const uint32_t nextLevel = (uint32_t)pi_NodeHeader.m_level + 1;
        nextGroup = m_OpenGroups.count(nextLevel) > 0 ? m_OpenGroups[nextLevel] : nullptr;
        if (!nextGroup.IsValid())
            {
            nextGroup = new SMNodeGroup(pi_CurrentGroup->GetDataSourceAccount(),
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
void SMBentleyGroupingStrategy<EXTENT>::_ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pGroup)
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
void SMBentleyGroupingStrategy<EXTENT>::_ApplyPostChildNodeProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pParentGroup, SMNodeGroup::Ptr& pi_pChildGroup)
    {

    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_SaveMasterHeader(const WString pi_pOutputDirPath) const
    {
    m_GroupMasterHeader.SaveToFile(pi_pOutputDirPath, m_Mode);
    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_SaveNodeGroup(SMNodeGroup::Ptr pi_Group) const
    {
    if (m_Mode == SMNodeGroup::StrategyType::VIRTUAL) return; // Don't need to save virtual groups, they will use normal headers to retrieve node header data
    if (pi_Group->IsEmpty()) return;

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
    }

/**---------------------------------------------------------------------------------------------
SMCesium3DTileStrategy
------------------------------------------------------------------------------------------------*/
template<class EXTENT>
class SMCesium3DTileStrategy : public SMGroupingStrategy<EXTENT>
    {
    protected:
        virtual void                _Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        virtual void                _AddGroup(SMNodeGroup* pi_pNodeGroup);
        virtual uint32_t            _AddNodeToGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        virtual void                _ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pGroup);
        virtual void                _ApplyPostChildNodeProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pParentGroup, SMNodeGroup::Ptr& pi_pChildGroup);
        virtual SMNodeGroup::Ptr    _GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_CurrentGroup);
        virtual void                _SaveMasterHeader(const WString pi_pOutputDirPath) const;
        virtual void                _SaveNodeGroup(SMNodeGroup::Ptr pi_Group) const;

    private:
        uint32_t m_MaxDepth = 5;
    };

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_AddGroup(SMNodeGroup* pi_pNodeGroup)
    {
    m_OpenGroups[pi_pNodeGroup->GetID()] = pi_pNodeGroup;
    }

template<class EXTENT>
uint32_t SMCesium3DTileStrategy<EXTENT>::_AddNodeToGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    Json::Value nodeTile;
    SMStreamingStore<EXTENT>::SerializeHeaderToCesium3DTileJSON(&pi_NodeHeader, pi_NodeHeader.m_id, nodeTile);
    pi_Group->Append3DTile(pi_NodeHeader.m_id.m_integerID, pi_NodeHeader.m_parentNodeID.m_integerID, nodeTile);
    m_GroupMasterHeader.AddNodeToGroup(pi_Group->GetID(), (uint64_t)pi_NodeHeader.m_id.m_integerID, 0);
    return 0;
    }

template<class EXTENT>
SMNodeGroup::Ptr SMCesium3DTileStrategy<EXTENT>::_GetNextGroup(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_CurrentGroup)
    {
    SMNodeGroup::Ptr nextGroup = pi_CurrentGroup->IsMaxDepthAchieved() ? nullptr : pi_CurrentGroup;
    if (!nextGroup.IsValid())
        {
        nextGroup = new SMNodeGroup(pi_CurrentGroup->GetDataSourceAccount(),
                                    pi_CurrentGroup->GetFilePath(),
                                    ++m_GroupID,
                                    pi_CurrentGroup,
                                    SMNodeGroup::StrategyType::CESIUM);
        this->AddGroup(nextGroup.get());
        }
    assert((nextGroup == pi_CurrentGroup) || (nextGroup.IsValid()));
    return nextGroup;
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_ApplyPostProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pGroup)
    {
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_ApplyPostChildNodeProcess(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_pParentGroup, SMNodeGroup::Ptr& pi_pChildGroup)
    {
    if (pi_pChildGroup != pi_pParentGroup)
        {
        if (pi_pChildGroup->m_tileTreeMap.size() < 10)
            {
            pi_pParentGroup->MergeChild(pi_pChildGroup);
            for (auto& tile : pi_pChildGroup->m_tileTreeMap)
                {
                this->m_GroupMasterHeader.AddNodeToGroup(pi_pParentGroup->GetID(), tile.first, 0);
                }
            this->m_GroupMasterHeader.RemoveGroup(pi_pChildGroup->GetID());
            pi_pChildGroup->m_tileTreeMap.clear();
            }
        else
            {
            SMNodeGroup::Ptr currentGroup = pi_pChildGroup;
            pi_pChildGroup = this->GetNextGroup(pi_NodeHeader, pi_pParentGroup);
            assert(currentGroup != pi_pChildGroup);
            currentGroup->Close<EXTENT>();
            }
        }
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_SaveMasterHeader(const WString pi_pOutputDirPath) const
    {
    // Save SM master header
    m_GroupMasterHeader.SaveToFile(pi_pOutputDirPath, SMNodeGroup::StrategyType::CESIUM);

    // NEEDS_WORK_SM_STREAMING: Save Cesium master header 
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_SaveNodeGroup(SMNodeGroup::Ptr pi_Group) const
    {
    Json::Value tileSet;
    tileSet["asset"]["version"] = "0.0";
    tileSet["root"] = pi_Group->m_RootTileTreeNode;

    //std::cout << "#nodes in group(" << pi_Group->m_groupHeader->GetID() << ") = " << pi_Group->m_tileTreeMap.size() << std::endl;

    auto utf8TileTree = Json::FastWriter().write(tileSet);

    WString path(pi_Group->m_outputDirPath + L"\\n_");
    wchar_t buffer[10000];
    swprintf(buffer, L"%s%lu.json", path.c_str(), pi_Group->GetID());
    std::wstring group_filename(buffer);

    BeFile file;
    if (OPEN_OR_CREATE_FILE(file, group_filename.c_str(), BeFileAccess::Write))
        {
        file.Write(nullptr, utf8TileTree.c_str(), (uint32_t)utf8TileTree.size());
        }
    }
