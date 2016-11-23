//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

#ifdef VANCOUVER_API
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#endif

#ifndef NDEBUG
#define DEBUG_GROUPS
#define DEBUG_AZURE
extern std::mutex s_consoleMutex;
#endif

#ifdef DEBUG_AZURE
#include <Bentley\BeConsole.h>
#endif

class DataSourceAccount;

extern bool s_stream_enable_caching;

extern uint32_t s_max_number_nodes_in_group;
extern size_t s_max_group_size;
extern size_t s_max_group_depth;
extern size_t s_max_group_common_ancestor;

//extern std::mutex fileMutex;

struct SMNodeHeader {
    uint64_t blockid;
    uint32_t offset;
    uint64_t size;
    };

struct SMGroupHeader : public bvector<SMNodeHeader>, public HFCShareableObject<SMGroupHeader> {
public:
    SMGroupHeader() : m_groupID(-1) {}
    SMGroupHeader(const size_t& pi_pGroupID) : m_groupID(pi_pGroupID) {}
    SMGroupHeader(const size_t& pi_pGroupID, const size_t& pi_pSize) : bvector<SMNodeHeader>(pi_pSize), m_groupID(pi_pGroupID) {}

    size_t GetID() { return m_groupID; }
    void   SetID(const size_t& pi_pGroupID) { m_groupID = pi_pGroupID; }

    void AddNode(const SMNodeHeader& pi_pNodeHeader) { this->push_back(pi_pNodeHeader); }

private:
    size_t m_groupID;
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
        while (Queue::size() == capacity)
            {
#ifdef DEBUG_GROUPS
                    {
                    std::lock_guard<mutex> clk(s_consoleMutex);
                    std::cout << "[" << std::this_thread::get_id() << "] Queue is full, waiting for jobs to complete" << std::endl;
                    }
#endif
                    wait(lock, [this]
                        {
                        return Queue::size() < capacity / 2;
                        });
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
#ifdef DEBUG_GROUPS
                    {
                    std::lock_guard<mutex> clk(s_consoleMutex);
                    std::cout << "[" << std::this_thread::get_id() << "] Finished work" << std::endl;
                    }
#endif
                    break;
                }
            else {
#ifdef DEBUG_GROUPS
                    {
                    std::lock_guard<mutex> clk(s_consoleMutex);
                    std::cout << "[" << std::this_thread::get_id() << "] Waiting for work" << std::endl;
                    }
#endif
                    wait(lock);
#ifdef DEBUG_GROUPS
                    {
                    std::lock_guard<mutex> clk(s_consoleMutex);
                    std::cout << "[" << std::this_thread::get_id() << "] Going to perform work; size of queue = " << Queue::size() << std::endl;
                    }
#endif
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

template<class EXTENT>
class SMGroupingStrategy;


class SMNodeGroup : public HFCShareableObject<SMNodeGroup>
    {

    public:
        enum StrategyType
            {
            NONE,
            NORMAL,
            VIRTUAL,
            CESIUM
            };
        typedef std::pair<uint64_t, SMNodeGroup*> DistributeData;
        typedef HFCPtr<SMNodeGroup> Ptr;

        template<class EXTENT>
        static SMGroupingStrategy<EXTENT>* s_groupingStrategy;

    private:
        bool   m_isLoaded = false;
        bool   m_isLoading = false;
        StrategyType   m_strategyType = NORMAL;
        size_t m_level = 0;
        size_t m_totalSize;
        size_t m_nLevels = 0;
        size_t m_depth = 0;
        size_t m_ancestor = -1;
        uint64_t m_currentPosition = 0;
        uint64_t m_progress = 0;
        bvector<uint8_t> m_rawHeaders;
        WString m_outputDirPath;
        WString m_dataSourceName;
        HFCPtr<SMGroupHeader> m_groupHeader;
        SMNodeDistributor<SMNodeGroup::DistributeData>::Ptr m_nodeDistributorPtr;
        condition_variable m_groupCV;
        mutex m_groupMutex;
        DataSourceAccount *m_dataSourceAccount;

    public:
        // Constructor for reading a group
        SMNodeGroup(DataSourceAccount *dataSourceAccount, const size_t& pi_pID, StrategyType strategyType, const size_t& pi_pSize, const uint64_t& pi_pTotalSizeOfHeaders)
            : m_dataSourceAccount(dataSourceAccount),
              m_groupHeader(new SMGroupHeader(pi_pID, pi_pSize)),
              m_rawHeaders(pi_pTotalSizeOfHeaders),
              m_strategyType(strategyType)
            {
            };

        // Constructor for writing a group
        SMNodeGroup(DataSourceAccount *dataSourceAccount, const WString pi_pOutputDirPath, const size_t& pi_pGroupLevel, const size_t& pi_pGroupID, StrategyType strategyType = StrategyType::NONE)
            : m_dataSourceAccount(dataSourceAccount),
              m_outputDirPath(pi_pOutputDirPath),
              m_level(pi_pGroupLevel),
              m_groupHeader(new SMGroupHeader(pi_pGroupID)),
              m_strategyType(strategyType)
            {
            // reserve space for total number of nodes for this group
            m_groupHeader->reserve(s_max_number_nodes_in_group);
            m_rawHeaders.reserve(3000 * s_max_number_nodes_in_group);

            // A group contains at least its ID and the number of nodes within it.
            m_totalSize = 2 * sizeof(size_t);
            }

        size_t GetLevel() { return m_level; }

        void SetLevel(const size_t& pi_NewID) { m_level = pi_NewID; }

        size_t GetID() { return m_groupHeader->GetID(); }

        void SetID(const size_t& pi_NewID) { m_groupHeader->SetID(pi_NewID); }

        void SetAncestor(const size_t& pi_pLevel) { m_ancestor = pi_pLevel; }

        void SetDataSource(const WString& pi_pDataSourceName) { m_dataSourceName = pi_pDataSourceName; }

        size_t GetNumberNodes() { return m_groupHeader->size(); }

        size_t GetSizeOfHeaders() { return m_rawHeaders.size(); }

        bvector<Byte>::pointer GetRawHeaders(const uint32_t& offset) { return m_rawHeaders.data() + offset; }

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

        WString GetFilePath() { return m_outputDirPath; }

        void Open(const size_t& pi_pGroupID) { SetID(pi_pGroupID); }

        void Close()
            {
            Save();
            Clear();
            }

        void Clear()
            {
            m_groupHeader->clear();
            m_rawHeaders.clear();
            m_totalSize = 2 * sizeof(size_t);
            m_ancestor = -1;
            }

        template<class EXTENT> uint32_t AddNode(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader)
            {
            // Fetch node header data
            uint32_t headerSize = 0;
            std::unique_ptr<Byte> headerData = nullptr;
            SMStreamingStore<EXTENT>::SerializeHeaderToBinary(&pi_NodeHeader, headerData, headerSize);

            const auto oldSize = m_rawHeaders.size();
            m_groupHeader->AddNode(SMNodeHeader{ (uint64_t)pi_NodeHeader.m_id.m_integerID, (uint32_t)oldSize, headerSize });
            m_rawHeaders.resize(oldSize + headerSize);
            memmove(&m_rawHeaders[oldSize], headerData.get(), headerSize);
            m_totalSize += headerSize + sizeof(SMNodeHeader);

            s_groupingStrategy<EXTENT>->Apply(pi_NodeHeader, this);

            // is cleanup necessary?
            //delete[] headerData.release();
            return headerSize;
            }

        bool IsEmpty() { return m_groupHeader->empty(); }

        bool IsFull()
            {
            //return GetNumberNodes() >= s_max_number_nodes_in_group;
            return GetTotalSize() >= s_max_group_size;
            }

        bool IsMaxDepthAchieved()
            {
            return m_depth >= s_max_group_depth;
            }

        bool IsCommonAncestorTooFar(const size_t& pi_pLevelRequested)
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
        template<class EXTENT> SMGroupingStrategy<EXTENT>* GetStrategy()
            {
            if (!s_groupingStrategy<EXTENT>)
                {
                switch (m_strategyType)
                    {
                    case StrategyType::NORMAL:
                        {
                        s_groupingStrategy<EXTENT> = new SMBentleyGroupingStrategy<EXTENT>();
                        break;
                        }
                    case StrategyType::VIRTUAL:
                        {
                        s_groupingStrategy<EXTENT> = new SMBentleyGroupingStrategy<EXTENT>();
                        break;
                        }
                    case StrategyType::CESIUM:
                        {
                        s_groupingStrategy<EXTENT> = new SMCesium3DTileStrategy<EXTENT>();
                        break;
                        }
                    default:
                        {
                        assert(false); // Unknown/invalid grouping strategy
                        }
                    }
                }
            assert(nullptr != s_groupingStrategy<EXTENT>);

            return s_groupingStrategy<EXTENT>;
            }

        void Save()
            {
            if (m_strategyType == VIRTUAL) return; // Don't need to save virtual groups, they will use normal headers to retrieve node header data

            WString path(m_outputDirPath + L"\\g_");
            wchar_t buffer[10000];
            swprintf(buffer, L"%s%llu.bin", path.c_str(), this->GetID());
            std::wstring group_filename(buffer);
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_filename.c_str(), BeFileAccess::Write) ||
                BeFileStatus::Success == file.Create(group_filename.c_str()))
                {
                uint32_t NbChars = 0;
                auto id = this->GetID();
                file.Write(&NbChars, &id, sizeof(id));
                assert(NbChars == sizeof(id));

                const auto numNodes = m_groupHeader->size();
                file.Write(&NbChars, &numNodes, sizeof(numNodes));
                assert(NbChars == sizeof(numNodes));

                file.Write(&NbChars, m_groupHeader->data(), (uint32_t)numNodes * sizeof(SMNodeHeader));
                assert(NbChars == numNodes * sizeof(SMNodeHeader));

                auto sizeHeaders = (uint32_t)GetSizeOfHeaders();
                file.Write(&NbChars, GetRawHeaders(0), sizeHeaders * sizeof(uint8_t));
                assert(NbChars == sizeHeaders * sizeof(uint8_t));
                }
            else
                {
                assert(!"Problem creating new group file");
                }

            file.Close();
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
                        size_t id;
                        memcpy(&id, dest.get(), sizeof(size_t));
                        assert(m_groupHeader->GetID() == id);
                        position += sizeof(size_t);

                        size_t numNodes;
                        memcpy(&numNodes, dest.get() + position, sizeof(numNodes));
                        assert(m_groupHeader->size() == numNodes);
                        position += sizeof(numNodes);

                        memcpy(m_groupHeader->data(), dest.get() + position, numNodes * sizeof(SMNodeHeader));
                        position += (uint32_t)numNodes * sizeof(SMNodeHeader);

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

        SMNodeHeader& GetNodeHeader(const uint64_t& pi_pNodeHeaderID)
            {
            return *(std::find_if(begin(*m_groupHeader), end(*m_groupHeader), [&](SMNodeHeader& nodeId)
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
            swprintf(buffer, L"%s%llu.bin", m_dataSourceName.c_str(), this->GetID());

            DataSourceURL dataSourceURL(buffer);

            if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                return;

            if (dataSource->read(dest, destSize, readSize, 0).isFailed())
                return;

            if (dataSource->close().isFailed())
                return;
            }

        uint64_t GetSingleNodeFromStore(const uint64_t& pi_pNodeID, bvector<uint8_t>& pi_pData)
            {

            std::unique_ptr<DataSource::Buffer[]>dest;
            DataSource*                          dataSource;
            DataSource::DataSize                 readSize;
            DataSourceBuffer::BufferSize         destSize = 5 * 1024 * 1024;

            DataSourceURL dataSourceURL(m_dataSourceName.c_str());
            dataSourceURL.append(L"n_" + std::to_wstring(pi_pNodeID) + L".bin");

            dataSource = this->InitializeDataSource(dest, destSize);
            if (dataSource == nullptr)
                return 0;

            if (dataSource->open(dataSourceURL, DataSourceMode_Read).isFailed())
                return 0;

            if (dataSource->read(dest.get(), destSize, readSize, 0).isFailed())
                return 0;

            if (dataSource->close().isFailed())
                return 0;

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

class SMNodeGroupMasterHeader : public std::map<size_t, SMGroupNodeIds>, public HFCShareableObject<SMNodeGroupMasterHeader>
    {
    public:
        SMNodeGroupMasterHeader() {}

        void AddGroup(const size_t& pi_pGroupID, size_type pi_pCount = 10000)
            {
            auto& newGroup = this->operator[](pi_pGroupID);
            newGroup.reserve(s_max_number_nodes_in_group);
            }

        void AddNodeToGroup(const size_t& pi_pGroupID, const uint64_t& pi_pNodeID, const uint64_t& pi_pNodeHeaderSize)
            {
            auto& group = this->operator[](pi_pGroupID);
            group.push_back(pi_pNodeID);
            group.m_sizeOfRawHeaders += pi_pNodeHeaderSize;
            }

        void SaveToFile(const WString pi_pOutputDirPath, const short& pi_pGroupMode)
            {
            assert(!m_oldMasterHeader.empty()); // Old master header must be set!

                                                // NEEDS_WORK_SM_STREAMING : use new CloudDataSource
            wchar_t buffer[10000];
            swprintf(buffer, L"%s/MasterHeaderWith%sGroups.bin", pi_pOutputDirPath.c_str(), (pi_pGroupMode == SMNodeGroup::VIRTUAL ? L"Virtual" : L""));
            std::wstring group_header_filename(buffer);
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_header_filename.c_str(), BeFileAccess::Write) ||
                BeFileStatus::Success == file.Create(group_header_filename.c_str()))
                {
                uint32_t NbChars = 0;

                // Save old Master Header part: size + data
                const uint32_t sizeOldMasterHeaderFile = (uint32_t)m_oldMasterHeader.size();
                file.Write(&NbChars, &sizeOldMasterHeaderFile, sizeof(sizeOldMasterHeaderFile));
                assert(NbChars == sizeof(sizeOldMasterHeaderFile));

                file.Write(&NbChars, m_oldMasterHeader.data(), sizeOldMasterHeaderFile);
                assert(NbChars == (uint32_t)m_oldMasterHeader.size());

                file.Write(&NbChars, &pi_pGroupMode, sizeof(pi_pGroupMode));

                // Append group information
                for (auto& group : *this)
                    {
                    // Group id
                    auto const id = group.first;
                    file.Write(&NbChars, &id, sizeof(id));
                    assert(NbChars == sizeof(id));

                    auto& groupInfo = group.second;

                    // Group total size of headers
                    if (pi_pGroupMode == SMNodeGroup::VIRTUAL)
                        {
                        auto const total_size = groupInfo.m_sizeOfRawHeaders;
                        file.Write(&NbChars, &total_size, sizeof(total_size));
                        assert(NbChars == sizeof(total_size));
                        }

                    // Group number of nodes
                    auto const numNodes = groupInfo.size();
                    file.Write(&NbChars, &numNodes, sizeof(numNodes));
                    assert(NbChars == sizeof(numNodes));

                    // Group node ids
                    file.Write(&NbChars, groupInfo.data(), (uint32_t)numNodes * sizeof(uint64_t));
                    assert(NbChars == (uint32_t)numNodes * sizeof(uint64_t));
                    }
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

template<class EXTENT>
SMGroupingStrategy<EXTENT>* SMNodeGroup::s_groupingStrategy = nullptr;

template<class EXTENT>
class SMGroupingStrategy
    {
    public:
        void Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        SMNodeGroup::Ptr GetNextGroup(uint32_t currentLevel, SMNodeGroup::Ptr pi_CurrentGroup);
        void ApplyPostProcess(uint32_t level, SMNodeGroup::Ptr pi_pGroup);
        void AddOpenGroup(const size_t& pi_pGroupKey, SMNodeGroup* pi_pNodeGroup);
        void SaveAllOpenGroups() const;

    protected:

        virtual void         _Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group) = 0;
        virtual SMNodeGroup::Ptr _GetNextGroup(uint32_t currentLevel, SMNodeGroup::Ptr pi_CurrentGroup) = 0;
        virtual void         _ApplyPostProcess(uint32_t level, SMNodeGroup::Ptr pi_pGroup) = 0;

    protected:
        uint32_t m_GroupID = 0;
        std::map<size_t, SMNodeGroup::Ptr> m_OpenGroups;
    };

/**----------------------------------------------------------------------------
This method adds a group in the Open Group map. Will overwrite an existing value.

@param
-----------------------------------------------------------------------------*/
template<class EXTENT> void SMGroupingStrategy<EXTENT>::AddOpenGroup(const size_t& pi_pGroupKey, SMNodeGroup* pi_pNodeGroup)
    {
    m_OpenGroups[pi_pGroupKey] = pi_pNodeGroup;
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
            group->Save();
            }
        }
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    this->_Apply(pi_NodeHeader, pi_Group);
    }

template<class EXTENT>
SMNodeGroup::Ptr SMGroupingStrategy<EXTENT>::GetNextGroup(uint32_t currentLevel, SMNodeGroup::Ptr pi_CurrentGroup)
    {
    return this->_GetNextGroup(currentLevel, pi_CurrentGroup);
    }

template<class EXTENT>
void SMGroupingStrategy<EXTENT>::ApplyPostProcess(uint32_t level, SMNodeGroup::Ptr pi_pGroup)
    {
    this->_ApplyPostProcess(level, pi_pGroup);
    }

template<class EXTENT>
class SMBentleyGroupingStrategy : public SMGroupingStrategy<EXTENT>
    {
    protected:
        virtual void         _Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        virtual SMNodeGroup::Ptr _GetNextGroup(uint32_t currentLevel, SMNodeGroup::Ptr pi_CurrentGroup);
        virtual void         _ApplyPostProcess(uint32_t level, SMNodeGroup::Ptr pi_pGroup);
    };

template<class EXTENT> 
void SMBentleyGroupingStrategy<EXTENT>::_Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    //auto groupID = pi_Group->GetID();
    //pi_pGroupsHeader->AddNodeToGroup(groupID, ConvertBlockID(GetBlockID()), headerSize);

    if (pi_Group->IsFull() || pi_Group->IsCommonAncestorTooFar(pi_NodeHeader.m_level))
        {
        pi_Group->Close();
        pi_Group->Open(++m_GroupID);
        //pi_pGroupsHeader->AddGroup(s_GroupID);
        }
    }

template<class EXTENT>
SMNodeGroup::Ptr SMBentleyGroupingStrategy<EXTENT>::_GetNextGroup(uint32_t currentLevel, SMNodeGroup::Ptr pi_CurrentGroup)
    {
    pi_CurrentGroup->IncreaseDepth();
    SMNodeGroup::Ptr nextGroup = pi_CurrentGroup->IsMaxDepthAchieved() ? nullptr : pi_CurrentGroup;
    if (!nextGroup)
        {
        const uint32_t nextLevel = currentLevel + 1;
        nextGroup = m_OpenGroups.count(nextLevel) > 0 ? m_OpenGroups[nextLevel] : nullptr;
        if (!nextGroup)
            {
            nextGroup = new SMNodeGroup(pi_CurrentGroup->GetDataSourceAccount(),
                                        pi_CurrentGroup->GetFilePath(),
                                        nextLevel,
                                        ++m_GroupID);
            this->AddOpenGroup(nextLevel, nextGroup);
            //pi_pGroupsHeader->AddGroup(s_GroupID);
            }
        }
    assert((nextGroup == pi_CurrentGroup) || (nextGroup != nullptr));
    return nextGroup.GetPtr();
    }

template<class EXTENT>
void SMBentleyGroupingStrategy<EXTENT>::_ApplyPostProcess(uint32_t level, SMNodeGroup::Ptr pi_pGroup)
    {
    // Set eldest parent visited (reverse order of traversal) to maintain proximity of nodes in a group
    const size_t newAncestor = level;
    for (auto rGroupIt = m_OpenGroups.rbegin(); rGroupIt != m_OpenGroups.rend(); ++rGroupIt)
        {
        auto& group = rGroupIt->second;
        auto& groupID = rGroupIt->first;
        if (newAncestor >= groupID) break;
        group->SetAncestor(newAncestor);
        }

    pi_pGroup->DecreaseDepth();
    }

template<class EXTENT>
class SMCesium3DTileStrategy : public SMGroupingStrategy<EXTENT>
    {
    protected:
        virtual void _Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group);
        virtual SMNodeGroup::Ptr _GetNextGroup(uint32_t currentLevel, SMNodeGroup::Ptr pi_CurrentGroup);
        virtual void         _ApplyPostProcess(uint32_t level, SMNodeGroup::Ptr pi_pGroup);
    };

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_Apply(const SMIndexNodeHeader<EXTENT>& pi_NodeHeader, SMNodeGroup::Ptr pi_Group)
    {
    }

template<class EXTENT>
SMNodeGroup::Ptr SMCesium3DTileStrategy<EXTENT>::_GetNextGroup(uint32_t currentLevel, SMNodeGroup::Ptr pi_CurrentGroup)
    {
    return pi_CurrentGroup.GetPtr();
    }

template<class EXTENT>
void SMCesium3DTileStrategy<EXTENT>::_ApplyPostProcess(uint32_t level, SMNodeGroup::Ptr pi_pGroup)
    {
    }

