//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMNodeGroup.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



//#include <ImagePP/all/h/HCDCodecZlib.h>
#include <ImagePP/all/h/HFCPtr.h>
#include "ScalableMesh/Streaming/AzureStorage.h"
#include "Threading\LightThreadPool.h"
#include "SMSQLiteFile.h"
#include <curl/curl.h>
#include <condition_variable>
#ifdef VANCOUVER_API
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#endif

#ifndef NDEBUG
#define DEBUG_GROUPS
#endif

extern bool s_is_virtual_grouping;
extern bool s_stream_from_disk;

extern uint32_t s_max_number_nodes_in_group;
extern size_t s_max_group_size;
extern size_t s_max_group_depth;
extern size_t s_max_group_common_ancestor;

extern std::mutex fileMutex;

struct SMNodeHeader {
    uint64_t blockid;
    uint32_t offset;
    uint64_t size;
    };

struct SMGroupHeader : public bvector<SMNodeHeader>, public HFCShareableObject<SMGroupHeader> {
public:
    SMGroupHeader() : m_pGroupID(-1) {}
    SMGroupHeader(const size_t& pi_pGroupID) : m_pGroupID(pi_pGroupID) {}
    SMGroupHeader(const size_t& pi_pGroupID, const size_t& pi_pSize) : bvector<SMNodeHeader>(pi_pSize), m_pGroupID(pi_pGroupID) {}

    size_t GetID() { return m_pGroupID; }
    void   SetID(const size_t& pi_pGroupID) { m_pGroupID = pi_pGroupID; }

    void AddNode(const SMNodeHeader& pi_pNodeHeader) { this->push_back(pi_pNodeHeader); }

private:
    size_t m_pGroupID;
    };

struct SMGroupNodeIds : bvector<uint64_t> {
    uint64_t m_pSizeOfRawHeaders;
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
            group.m_pSizeOfRawHeaders += pi_pNodeHeaderSize;
            }

        void SaveToFile(const WString pi_pOutputDirPath)
            {
            assert(!m_pOldMasterHeader.empty()); // Old master header must be set!

            wstringstream ss;
            ss << WString(pi_pOutputDirPath + L"/MasterHeaderWithGroups.bin");
            auto group_header_filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_header_filename.c_str(), BeFileAccess::Write) ||
                BeFileStatus::Success == file.Create(group_header_filename.c_str()))
                {
                uint32_t NbChars = 0;

                // Save old Master Header part: size + data
                const uint32_t sizeOldMasterHeaderFile = (uint32_t)m_pOldMasterHeader.size();
                file.Write(&NbChars, &sizeOldMasterHeaderFile, sizeof(sizeOldMasterHeaderFile));
                assert(NbChars == sizeof(sizeOldMasterHeaderFile));

                file.Write(&NbChars, m_pOldMasterHeader.data(), sizeOldMasterHeaderFile);
                assert(NbChars == (uint32_t)m_pOldMasterHeader.size());

                file.Write(&NbChars, &s_is_virtual_grouping, sizeof(s_is_virtual_grouping));

                // Append group information
                for (auto& group : *this)
                    {
                    // Group id
                    auto const id = group.first;
                    file.Write(&NbChars, &id, sizeof(id));
                    assert(NbChars == sizeof(id));

                    auto& groupInfo = group.second;

                    // Group total size of headers
                    if (s_is_virtual_grouping)
                        {
                        auto const total_size = groupInfo.m_pSizeOfRawHeaders;
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
            m_pOldMasterHeader.resize(sizeof(pi_pOldMasterHeader));
            memcpy(m_pOldMasterHeader.data(), &pi_pOldMasterHeader, sizeof(pi_pOldMasterHeader));
            }
    private:
        bvector<uint8_t> m_pOldMasterHeader;
    };

static std::mutex globalGroupMtx;
class SMNodeGroup : public HFCShareableObject<SMNodeGroup>
    {
    template <typename Type, typename Queue = std::queue<Type>>
    class distributor : Queue, std::mutex, std::condition_variable {
        typename Queue::size_type capacity;
        unsigned int m_concurrency;
        bool done = false;
        std::vector<std::thread> threads;

    public:
        distributor(unsigned int concurrency = std::thread::hardware_concurrency()
                    //, unsigned int concurrency = 2
                    , typename Queue::size_type max_items_per_thread = 500)
            : capacity{ concurrency * max_items_per_thread },
              m_concurrency{ concurrency }
            {
            if (!concurrency)
                throw std::invalid_argument("Concurrency must be non-zero");
            if (!max_items_per_thread)
                throw std::invalid_argument("Max items per thread must be non-zero");
            }
        template<typename Function>
        distributor(Function function
                    , unsigned int concurrency = std::thread::hardware_concurrency()
                    //, unsigned int concurrency = 2
                    , typename Queue::size_type max_items_per_thread = 500
                    )
            : capacity{ concurrency * max_items_per_thread }
            {
            if (!concurrency)
                throw std::invalid_argument("Concurrency must be non-zero");
            if (!max_items_per_thread)
                throw std::invalid_argument("Max items per thread must be non-zero");

            for (unsigned int count{ 0 }; count < concurrency; count += 1)
                threads.emplace_back(static_cast<void (distributor::*)(Function)>
                                     (&distributor::consume), this, function);
            }

        distributor(distributor &&) = default;
        distributor &operator=(distributor &&) = delete;

        ~distributor()
            {
            Wait();
            }

        template <typename Function>
        void SetJob(Function function)
            {
            for (unsigned int count{ 0 }; count < m_concurrency; count += 1)
                threads.emplace_back(static_cast<void (distributor::*)(Function)>
                                     (&distributor::consume), this, function);
            }
        void operator()(Type &&value)
            {
            std::unique_lock<std::mutex> lock(*this);
            while (Queue::size() == capacity)
                {
#ifdef DEBUG_GROUPS
                //std::cout << "distributor queue is full, waiting for jobs to complete... " << this << std::endl;
#endif
                wait(lock);
                }
            Queue::push(std::forward<Type>(value));
            notify_one();
            }

        void Wait()
            {
                    {
                    std::lock_guard<std::mutex> guard(*this);
                    done = true;
                    notify_all();
                    }
                    for (auto &&thread : threads) if (thread.joinable()) thread.join();
            }

    private:
        template <typename Function>
        void consume(Function process)
            {
#ifdef DEBUG_GROUPS
            //std::cout << this << " is starting to process queue..." << Queue::size() << std::endl;
#endif
            std::unique_lock<std::mutex> lock(*this);
            while (true) {
                if (!Queue::empty()) {
                    Type item{ std::move(Queue::front()) };
                    Queue::pop();
                    notify_one();
                    lock.unlock();
                    process(item);
                    lock.lock();
                    }
                else if (done) {
#ifdef DEBUG_GROUPS
                    //std::cout << this << " done adding to queue, no more items can be added... " << this << std::endl;
#endif
                    break;
                    }
                else {
#ifdef DEBUG_GROUPS
                    //std::cout << "distributor queue is empty but not done yet... " << this << std::endl;
#endif
                    wait(lock);
                    }
                }
            }
        };
    public:

        SMNodeGroup(const size_t& pi_pID, const size_t& pi_pSize, const uint64_t& pi_pTotalSizeOfHeaders)
            : m_pGroupHeader(new SMGroupHeader(pi_pID, pi_pSize)),
            m_pRawHeaders(pi_pTotalSizeOfHeaders),
            m_stream_store(nullptr)
            {};

        SMNodeGroup(WString& pi_pDataSourceName, scalable_mesh::azure::Storage& pi_pStreamStore)
            {
            // reserve space for total number of nodes for this group
            m_pGroupHeader->reserve(s_max_number_nodes_in_group);
            m_pRawHeaders.reserve(3000 * s_max_number_nodes_in_group);
            m_pTotalSize = 2 * sizeof(size_t);
            m_pDataSourceName = pi_pDataSourceName + L"g_";
            m_stream_store = &pi_pStreamStore;
            }

        SMNodeGroup(const WString pi_pOutputDirPath, const size_t& pi_pGroupLevel, const size_t& pi_pGroupID)
            : m_pOutputDirPath(pi_pOutputDirPath), m_pLevel(pi_pGroupLevel), m_pGroupHeader(new SMGroupHeader(pi_pGroupID))
            {
            // reserve space for total number of nodes for this group
            m_pGroupHeader->reserve(s_max_number_nodes_in_group);
            m_pRawHeaders.reserve(3000 * s_max_number_nodes_in_group);

            m_pTotalSize = 2 * sizeof(size_t);
            }

        size_t GetLevel() { return m_pLevel; }

        void SetLevel(const size_t& pi_NewID) { m_pLevel = pi_NewID; }

        size_t GetID() { return m_pGroupHeader->GetID(); }

        void SetID(const size_t& pi_NewID) { m_pGroupHeader->SetID(pi_NewID); }

        void SetAncestor(const size_t& pi_pLevel) { m_pAncestor = pi_pLevel; }

        void SetDataSource(const WString& pi_pDataSourceName, scalable_mesh::azure::Storage& pi_pStreamStore)
            {
            m_pDataSourceName = pi_pDataSourceName;
            m_stream_store = &pi_pStreamStore;
            }

        size_t GetNumberNodes() { return m_pGroupHeader->size(); }

        size_t GetSizeOfHeaders() { return m_pRawHeaders.size(); }

        bvector<Byte>::pointer GetRawHeaders(const uint32_t& offset) { return m_pRawHeaders.data() + offset; }

        size_t GetTotalSize() { return m_pTotalSize; }

        void SetHeader(HFCPtr<SMGroupHeader> pi_pGroupHeader) { m_pGroupHeader = pi_pGroupHeader; }

        HFCPtr<SMGroupHeader> GetHeader() { return m_pGroupHeader; }

        void IncreaseDepth() { ++m_pDepth; }

        void DecreaseDepth()
            {
            assert(m_pDepth > 0);
            --m_pDepth;
            }

        WString GetFilePath() { return m_pOutputDirPath; }

        void Open(const size_t& pi_pGroupID) { SetID(pi_pGroupID); }

        void Close()
            {
            Save();
            Clear();
            }

        void Clear()
            {
            m_pGroupHeader->clear();
            m_pRawHeaders.clear();
            m_pTotalSize = 2 * sizeof(size_t);
            m_pAncestor = -1;
            }

        void AddNode(uint32_t pi_NodeID, const std::unique_ptr<Byte>& pi_Data, uint32_t pi_Size)
            {
            const auto oldSize = m_pRawHeaders.size();
            m_pGroupHeader->AddNode(SMNodeHeader{ pi_NodeID, (uint32_t)oldSize, pi_Size });
            m_pRawHeaders.resize(oldSize + pi_Size);
            memmove(&m_pRawHeaders[oldSize], pi_Data.get(), pi_Size);
            m_pTotalSize += pi_Size + sizeof(SMNodeHeader);
            }

        bool IsEmpty()
            {
            return m_pGroupHeader->empty();
            }

        bool IsFull()
            {
            //return GetNumberNodes() >= s_max_number_nodes_in_group;
            return GetTotalSize() >= s_max_group_size;
            }

        bool IsMaxDepthAchieved()
            {
            return m_pDepth >= s_max_group_depth;
            }

        bool IsCommonAncestorTooFar(const size_t& pi_pLevelRequested)
            {
            return (m_pAncestor == -1 ? false : pi_pLevelRequested >= s_max_group_common_ancestor + m_pAncestor);
            }

        bool IsLoaded() { return m_pIsLoaded; }

        void Save()
            {
            WString path(m_pOutputDirPath + L"\\g_");
            wstringstream ss;
            ss << path << this->GetID() << L".bin";
            auto group_filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_filename.c_str(), BeFileAccess::Write) ||
                BeFileStatus::Success == file.Create(group_filename.c_str()))
                {
                uint32_t NbChars = 0;
                auto id = this->GetID();
                file.Write(&NbChars, &id, sizeof(id));
                assert(NbChars == sizeof(id));

                if (s_is_virtual_grouping)
                    {
                    auto sizeHeaders = GetSizeOfHeaders();
                    file.Write(&NbChars, &sizeHeaders, sizeof(sizeHeaders));
                    assert(NbChars == sizeof(sizeHeaders));
                    }

                const auto numNodes = m_pGroupHeader->size();
                file.Write(&NbChars, &numNodes, sizeof(numNodes));
                assert(NbChars == sizeof(numNodes));

                file.Write(&NbChars, m_pGroupHeader->data(), (uint32_t)numNodes * sizeof(SMNodeHeader));
                assert(NbChars == numNodes * sizeof(SMNodeHeader));

                if (!s_is_virtual_grouping)
                    {
                    auto sizeHeaders = (uint32_t)GetSizeOfHeaders();
                    file.Write(&NbChars, GetRawHeaders(0), sizeHeaders * sizeof(uint8_t));
                    assert(NbChars == sizeHeaders * sizeof(uint8_t));
                    }
                }
            else
                {
                assert(!"Problem creating new group file");
                }

            file.Close();
            }

        StatusInt Load(const uint64_t& priorityNodeID);

        void LoadGroupParallel();

        bool ContainsNode(const uint64_t& pi_pNodeID)
            {
            assert(!m_pGroupHeader->empty());
            auto node = std::find_if(begin(*m_pGroupHeader), end(*m_pGroupHeader), [&](SMNodeHeader& nodeId)
                {
                return nodeId.blockid == pi_pNodeID;
                });
            return node != m_pGroupHeader->end();
            }

        SMNodeHeader& GetNodeHeader(const uint64_t& pi_pNodeHeaderID)
            {
            return *(std::find_if(begin(*m_pGroupHeader), end(*m_pGroupHeader), [&](SMNodeHeader& nodeId)
                {
                return pi_pNodeHeaderID == nodeId.blockid;
                }));
            }

    private:

        uint64_t GetSingleNodeFromStore(const uint64_t& pi_pNodeID, uint8_t* pi_pData)
            {
            //static set<uint64_t> nodeIds;
            //assert(nodeIds.insert(pi_pNodeID).second);
            //static int nbDownloadedNodeHeaders = 0;
            //++nbDownloadedNodeHeaders;
            //std::cout << "total node headers fetched: " << nbDownloadedNodeHeaders << std::endl;
            wstringstream ss;
            ss << m_pDataSourceName + L"n_" << pi_pNodeID << L".bin";
            auto filename = ss.str();
            if (s_stream_from_disk)
                {
                BeFile file;
                if (BeFileStatus::Success != OPEN_FILE(file, filename.c_str(), BeFileAccess::Read))
                    {
                    assert(false); // node header file must exist
                    }
                uint64_t fileSize;
                file.GetSize(fileSize);
                bvector<uint8_t> inBuffer(fileSize);
                uint32_t bytes_read = 0;
                file.Read(pi_pData, &bytes_read, fileSize);
                assert(bytes_read == fileSize);
                return fileSize;
                //PointBlock block;
                //block.SetDataSource(filename.c_str());
                //block.SetStore(*m_stream_store);
                //block.Load();
                }
            else
                {
                uint64_t dataSize = 0;
                m_stream_store->DownloadBlob(filename.c_str(), [&pi_pData, &dataSize](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                    {
                    if (buffer.empty())
                        {
                        dataSize = 0;
                        return;
                        }
                    dataSize = (uint64_t)buffer.size();

                    //pi_pData = new uint8_t[dataSize];
                    memmove(pi_pData, buffer.data(), dataSize);
                    });
                return dataSize;
                }
            }
        StatusInt LoadFromLocal(std::unique_ptr<uint8_t>& pi_pBuffer, uint32_t& pi_pBufferSize)
            {
            wstringstream ss;
            ss << m_pDataSourceName << this->GetID() << L".bin";
            auto group_filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_filename.c_str(), BeFileAccess::Read))
                {
                uint64_t fileSize;
                file.GetSize(fileSize);
                pi_pBuffer.reset(new uint8_t[fileSize]);
                file.Read(pi_pBuffer.get(), &pi_pBufferSize, (uint32_t)fileSize);
                assert(pi_pBufferSize == fileSize);
                }
            else
                {
                return ERROR_FILE_NOT_FOUND;
                }

            file.Close();
            return SUCCESS;
            }

        StatusInt LoadFromAzure(std::unique_ptr<uint8_t>& pi_pBuffer, uint32_t& pi_pBufferSize)
            {
            assert(m_stream_store != nullptr);

            wstringstream ss;
            ss << m_pDataSourceName << this->GetID() << L".bin";
            auto group_filename = ss.str();
            StatusInt status;
            m_stream_store->DownloadBlob(group_filename.c_str(), [&pi_pBuffer, &pi_pBufferSize, &status](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                {
                if (buffer.empty())
                    {
                    pi_pBufferSize = 0;
                    status = ERROR_EMPTY;
                    return;
                    }
                status = SUCCESS;
                pi_pBufferSize = (uint32_t)buffer.size();

                pi_pBuffer.reset(new uint8_t[pi_pBufferSize]);
                memcpy(pi_pBuffer.get(), buffer.data(), pi_pBufferSize);
                });
            return status;
            }

    private:
        bool   m_pIsLoaded = false;
        bool   m_pIsLoading = false;
        size_t m_pLevel = 0;
        size_t m_pTotalSize;
        size_t m_pNumLevels = 0;
        size_t m_pDepth = 0;
        size_t m_pAncestor = -1;
        bvector<uint8_t> m_pRawHeaders;
        WString m_pOutputDirPath;
        WString m_pDataSourceName;
        HFCPtr<SMGroupHeader> m_pGroupHeader;
        distributor<uint64_t> m_pNodeFetchDistributor;
        scalable_mesh::azure::Storage* m_stream_store;
        condition_variable m_pGroupCV;
        mutex m_pGroupMutex;
    };
