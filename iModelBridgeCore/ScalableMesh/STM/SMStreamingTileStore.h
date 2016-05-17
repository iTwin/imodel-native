//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMStreamingTileStore.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



#include "SMPointTileStore.h"
#include <ImagePP/all/h/HCDCodecZlib.h>
#include "ScalableMesh/Streaming/AzureStorage.h"
#include <curl/curl.h>
#include <condition_variable>
#ifdef VANCOUVER_API
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::None)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode, BeFileSharing::Read)
#else
#define OPEN_FILE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#define OPEN_FILE_SHARE(beFile, pathStr, accessMode) beFile.Open(pathStr, accessMode)
#endif

//static bool s_useStreamingStore = true;
extern bool s_save_grouped_store;
extern bool s_stream_from_disk;
extern bool s_stream_from_file_server;
extern bool s_stream_from_grouped_store;
extern uint32_t s_max_number_nodes_in_group;
extern size_t s_max_group_size;
extern size_t s_max_group_depth;
extern size_t s_max_group_common_ancestor;

extern std::mutex fileMutex;

struct SMNodeHeader {
    uint64_t blockid;
    uint32_t offset;
    uint32_t size;
    };

struct SMGroupHeader : public vector<SMNodeHeader>, public HFCShareableObject<SMGroupHeader> {
public:
    SMGroupHeader() : m_pGroupID(-1) {}
    SMGroupHeader(const size_t& pi_pGroupID) : m_pGroupID(pi_pGroupID) {}
    SMGroupHeader(const size_t& pi_pGroupID, const size_t& pi_pSize) : vector<SMNodeHeader>(pi_pSize), m_pGroupID(pi_pGroupID){}

    size_t GetID() { return m_pGroupID; }
    void   SetID(const size_t& pi_pGroupID) { m_pGroupID = pi_pGroupID; }

    void AddNode(const SMNodeHeader& pi_pNodeHeader) { this->push_back(pi_pNodeHeader); }

private:
    size_t m_pGroupID;
    };

class SMNodeGroupMasterHeader : public std::map<size_t, vector<uint64_t>>, public HFCShareableObject<SMNodeGroupMasterHeader>
    {
    public:
        SMNodeGroupMasterHeader() {}

        void AddGroup(const size_t& pi_pGroupID, size_type pi_pCount = 10000)
            {
            auto& newGroup = this->operator[](pi_pGroupID);
            newGroup.reserve(s_max_number_nodes_in_group);
            }

        void AddNodeToGroup(const size_t& pi_pGroupID, const uint64_t& pi_pNodeID)
            {
            this->operator[](pi_pGroupID).push_back(pi_pNodeID);
            }

        void SaveToFile(const WString pi_pOutputDirPath)
            {
            assert(!m_pOldMasterHeader.empty()); // Old master header must be set!

            // First, we need to copy the old Master Header file -- then append group information --
            //auto oldMasterHeaderFilename = m_pOldMasterHeaderPath;
            //oldMasterHeaderFilename.append(L"\\MasterHeader.sscm");
            //BeFile oldMasterHeaderFile;
            //if (BeFileStatus::Success != OPEN_FILE(oldMasterHeaderFile, oldMasterHeaderFilename.c_str(), BeFileAccess::Read)) //oldMasterHeaderFile.Open(oldMasterHeaderFilename.c_str(), BeFileAccess::Read, BeFileSharing::None))
            //    assert(!"Problem reading the old Master Header File... does it exist?");
            //uint64_t fileSize;
            //oldMasterHeaderFile.GetSize(fileSize);
            //bvector<Byte> oldMasterHeaderBuffer(fileSize);
            //uint32_t bytes_read;
            //oldMasterHeaderFile.Read(oldMasterHeaderBuffer.data(), &bytes_read, fileSize);
            //assert(bytes_read == fileSize);

            wstringstream ss;
            ss << WString(pi_pOutputDirPath + L"\\MasterHeader.bin");
            auto group_header_filename = ss.str();
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, group_header_filename.c_str(), BeFileAccess::Write) ||//  file.Open(group_header_filename.c_str(), BeFileAccess::Write, BeFileSharing::None) ||
                BeFileStatus::Success == file.Create(group_header_filename.c_str()))
                {
                uint32_t NbChars = 0;

                // Save old Master Header file into new Master Header file
                // NEEDS_WORK_SM : The old Master Header is saved in JSON format. Should be binary.
                const uint32_t sizeOldMasterHeaderFile = (uint32_t)m_pOldMasterHeader.size();
                file.Write(&NbChars, &sizeOldMasterHeaderFile, sizeof(sizeOldMasterHeaderFile));
                assert(NbChars == sizeof(sizeOldMasterHeaderFile));

                file.Write(&NbChars, m_pOldMasterHeader.data(), sizeOldMasterHeaderFile);
                assert(NbChars == (uint32_t)m_pOldMasterHeader.size());

                // Append group information
                for (auto& group : *this)
                    {
                    // Group id
                    auto const id = group.first;
                    file.Write(&NbChars, &id, sizeof(id));
                    assert(NbChars == sizeof(id));

                    auto& groupInfo = group.second;

                    // Group size
                    auto const size = groupInfo.size();
                    file.Write(&NbChars, &size, sizeof(size));
                    assert(NbChars == sizeof(size));

                    // Group node ids
                    file.Write(&NbChars, groupInfo.data(), (uint32_t)size * sizeof(uint64_t));
                    assert(NbChars == (uint32_t)size * sizeof(uint64_t));
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

class SMNodeGroup : public HFCShareableObject<SMNodeGroup>
    {
    public:

        SMNodeGroup(const size_t& pi_pID, const size_t& pi_pSize) 
            : m_pGroupHeader(new SMGroupHeader(pi_pID, pi_pSize)),
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
        
        size_t GetSizeOfHeaders() { return m_pRawHeaders.size();  }
        
        bvector<Byte>::pointer GetRawHeaders() { return m_pRawHeaders.data(); }
        
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
            if (BeFileStatus::Success == OPEN_FILE(file, group_filename.c_str(), BeFileAccess::Write) ||//file.Open(group_filename.c_str(), BeFileAccess::Write, BeFileSharing::None) ||
                BeFileStatus::Success == file.Create(group_filename.c_str()))
                {
                uint32_t NbChars = 0;
                auto id = this->GetID();
                file.Write(&NbChars, &id, sizeof(id));
                HASSERT(NbChars == sizeof(id));

                const auto numNodes = m_pGroupHeader->size();
                file.Write(&NbChars, &numNodes, sizeof(numNodes));
                HASSERT(NbChars == sizeof(numNodes));

                file.Write(&NbChars, m_pGroupHeader->data(), (uint32_t)numNodes * sizeof(SMNodeHeader));
                HASSERT(NbChars == numNodes * sizeof(SMNodeHeader));

                auto sizeHeaders = (uint32_t)GetSizeOfHeaders();
                file.Write(&NbChars, GetRawHeaders(), sizeHeaders * sizeof(uint8_t));
                HASSERT(NbChars == sizeHeaders * sizeof(uint8_t));
                }
            else
                {
                HASSERT(!"Problem creating new group file");
                }

            file.Close();
            }

        void Load()
            {
            unique_lock<mutex> lk(m_pGroupMutex);
            if (m_pIsLoading)
                {
                m_pGroupCV.wait(lk, [this] {return !m_pIsLoading; });
                }
            else {
                std::unique_ptr<uint8_t> inBuffer = nullptr;
                uint32_t bytes_read = 0;
                m_pIsLoading = true;
                if (s_stream_from_disk)
                    {
                    this->LoadFromLocal(inBuffer, bytes_read);
                    }
                else
                    {
                    this->LoadFromAzure(inBuffer, bytes_read);
                    }
                uint32_t position = 0;
                size_t id;
                memcpy(&id, inBuffer.get(), sizeof(size_t));
                assert(m_pGroupHeader->GetID() == id);
                position += sizeof(size_t);

                size_t numNodes;
                memcpy(&numNodes, inBuffer.get() + position, sizeof(numNodes));
                assert(m_pGroupHeader->size() == numNodes);
                position += sizeof(numNodes);

                memcpy(m_pGroupHeader->data(), inBuffer.get() + position, numNodes * sizeof(SMNodeHeader));
                position += (uint32_t)numNodes * sizeof(SMNodeHeader);

                const auto headerSectionSize = bytes_read - position;
                m_pRawHeaders.resize(headerSectionSize);
                memcpy(m_pRawHeaders.data(), inBuffer.get() + position, headerSectionSize);

                m_pIsLoading = false;
                m_pGroupCV.notify_all();
                }
            m_pIsLoaded = true;
            }

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

        void LoadFromLocal(std::unique_ptr<uint8_t>& pi_pBuffer, uint32_t& pi_pBufferSize)
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
                HASSERT(!"Problem reading gouped header file");
                }

            file.Close();
            }

        void LoadFromAzure(std::unique_ptr<uint8_t>& pi_pBuffer, uint32_t& pi_pBufferSize)
            {
            assert(m_stream_store != nullptr);

            wstringstream ss;
            ss << m_pDataSourceName << this->GetID() << L".bin";
            auto group_filename = ss.str();
            m_stream_store->DownloadBlob(group_filename.c_str(), [&pi_pBuffer, &pi_pBufferSize](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                {
                if (buffer.empty())
                    {
                    pi_pBufferSize = 0;
                    return;
                    }
                pi_pBufferSize = (uint32_t)buffer.size();

                pi_pBuffer.reset(new uint8_t[pi_pBufferSize]);
                memcpy(pi_pBuffer.get(), buffer.data(), pi_pBufferSize);
                });
            }

    private:
        bool   m_pIsLoaded = false;
        bool   m_pIsLoading = false;
        size_t m_pLevel = 0;
        size_t m_pTotalSize;
        size_t m_pNumLevels = 0;
        size_t m_pDepth = 0;
        size_t m_pAncestor = -1;
        bvector<Byte> m_pRawHeaders;
        WString m_pOutputDirPath;
        WString m_pDataSourceName;
        HFCPtr<SMGroupHeader> m_pGroupHeader;
        scalable_mesh::azure::Storage* m_stream_store;
        condition_variable m_pGroupCV;
        mutex m_pGroupMutex;
    };



template <typename POINT, typename EXTENT> class SMStreamingPointTaggedTileStore : public SMPointTileStore<POINT, EXTENT>// , public HFCShareableObject<SMPointTileStore<POINT, EXTENT> >
    {

    private:
        struct MemoryStruct {
            bvector<Byte>* memory;
            size_t         size;
            };

        struct PointBlock : public bvector<uint8_t> {
        public:
            bool IsLoading() { return m_pIsLoading; }
            bool IsLoaded() { return m_pIsLoaded; }
            void LockAndWait()
                {
                unique_lock<mutex> lock(m_pPointBlockMutex);
                m_pPointBlockCV.wait(lock, [this]() { return m_pIsLoaded; });
                }
            void SetLoading() { m_pIsLoading = true; }
            void SetLoaded() 
                { 
                m_pIsLoaded = true;
                m_pIsLoading = false;
                m_pPointBlockCV.notify_all();
                }
        private:
            bool m_pIsLoading = false;
            bool m_pIsLoaded = false;
            condition_variable m_pPointBlockCV;
            mutex m_pPointBlockMutex;
            };

        static IDTMFile::NodeID ConvertBlockID(const HPMBlockID& blockID)
            {
            return static_cast<IDTMFile::NodeID>(blockID.m_integerID);
            }

        static IDTMFile::NodeID ConvertChildID(const HPMBlockID& childID)
            {
            return static_cast<IDTMFile::NodeID>(childID.m_integerID);
            }

        static IDTMFile::NodeID ConvertNeighborID(const HPMBlockID& neighborID)
            {
            return static_cast<IDTMFile::NodeID>(neighborID.m_integerID);
            }

        static bool IsValidID(const HPMBlockID& blockID)
            {
            return blockID.IsValid() && blockID.m_integerID != IDTMFile::GetNullNodeID();
            }

        bool WriteCompressedPacket(const HCDPacket& pi_uncompressedPacket,
                                   HCDPacket& pi_compressedPacket) const
            {
            HPRECONDITION(pi_uncompressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

            // initialize codec
            HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_uncompressedPacket.GetDataSize());
            pi_compressedPacket.SetBufferOwnership(true);
            size_t compressedBufferSize = pCodec->GetSubsetMaxCompressedSize();
            pi_compressedPacket.SetBuffer(new Byte[compressedBufferSize], compressedBufferSize * sizeof(Byte));
            const size_t compressedDataSize = pCodec->CompressSubset(pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetDataSize() * sizeof(Byte), pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetBufferSize() * sizeof(Byte));
            pi_compressedPacket.SetDataSize(compressedDataSize);

            return true;
            }

        bool LoadCompressedPacket(const HCDPacket& pi_compressedPacket,
                                  HCDPacket& pi_uncompressedPacket) const
            {
            HPRECONDITION(pi_compressedPacket.GetDataSize() <= (numeric_limits<uint32_t>::max) ());

            // initialize codec
            HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_compressedPacket.GetDataSize());
            pi_uncompressedPacket.SetBufferOwnership(true);
            pi_uncompressedPacket.SetBuffer(new Byte[pi_uncompressedPacket.GetDataSize()], pi_uncompressedPacket.GetDataSize() * sizeof(Byte));
            const size_t compressedDataSize = pCodec->DecompressSubset(pi_compressedPacket.GetBufferAddress(), pi_compressedPacket.GetDataSize() * sizeof(Byte), pi_uncompressedPacket.GetBufferAddress(), pi_uncompressedPacket.GetBufferSize() * sizeof(Byte));
            pi_uncompressedPacket.SetDataSize(compressedDataSize);

            return true;
            }
        
        static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
            {
            size_t realsize = size * nmemb;
            struct MemoryStruct *mem = (struct MemoryStruct *)userp;

            assert(mem->memory->capacity() >= mem->memory->size() + realsize);

            //    mem->memory->assign((Byte*)contents, (Byte*)contents + realsize);
            mem->memory->insert(mem->memory->end(), (Byte*)contents, (Byte*)contents + realsize);

            return realsize;
            }

        void ReadNodeHeaderFromBinary(SMPointNodeHeader<EXTENT>* header, Byte* headerData, size_t maxCountData) const
            {
            size_t dataIndex = 0;

            memcpy(&header->m_filtered, headerData + dataIndex, sizeof(header->m_filtered));
            dataIndex += sizeof(header->m_filtered);
            uint32_t parentNodeID;
            memcpy(&parentNodeID, headerData + dataIndex, sizeof(parentNodeID));
            header->m_parentNodeID = parentNodeID != IDTMFile::GetNullNodeID() ? HPMBlockID(parentNodeID) : IDTMFile::GetNullNodeID();
            dataIndex += sizeof(parentNodeID);
            uint32_t subNodeNoSplitID;
            memcpy(&subNodeNoSplitID, headerData + dataIndex, sizeof(subNodeNoSplitID));
            header->m_SubNodeNoSplitID = subNodeNoSplitID != IDTMFile::GetNullNodeID() ? HPMBlockID(subNodeNoSplitID) : IDTMFile::GetNullNodeID();
            dataIndex += sizeof(subNodeNoSplitID);
            memcpy(&header->m_level, headerData + dataIndex, sizeof(header->m_level));
            dataIndex += sizeof(header->m_level);
            memcpy(&header->m_IsBranched, headerData + dataIndex, sizeof(header->m_IsBranched));
            dataIndex += sizeof(header->m_IsBranched);
            memcpy(&header->m_IsLeaf, headerData + dataIndex, sizeof(header->m_IsLeaf));
            dataIndex += sizeof(header->m_IsLeaf);
            memcpy(&header->m_SplitTreshold, headerData + dataIndex, sizeof(header->m_SplitTreshold));
            dataIndex += sizeof(header->m_SplitTreshold);
            memcpy(&header->m_totalCount, headerData + dataIndex, sizeof(header->m_totalCount));
            dataIndex += sizeof(header->m_totalCount);
            memcpy(&header->m_nodeCount, headerData + dataIndex, sizeof(header->m_nodeCount));
            dataIndex += sizeof(header->m_nodeCount);
            memcpy(&header->m_arePoints3d, headerData + dataIndex, sizeof(header->m_arePoints3d));
            dataIndex += sizeof(header->m_arePoints3d);
            memcpy(&header->m_nbFaceIndexes, headerData + dataIndex, sizeof(header->m_nbFaceIndexes));
            dataIndex += sizeof(header->m_nbFaceIndexes);
            uint32_t graphID;
            memcpy(&graphID, headerData + dataIndex, sizeof(graphID));
            header->m_graphID = graphID != IDTMFile::GetNullNodeID() ? HPMBlockID(graphID) : IDTMFile::GetNullNodeID();
            dataIndex += sizeof(graphID);

            double xMin;
            memcpy(&xMin, headerData + dataIndex, sizeof(double));
            ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, xMin);
            dataIndex += sizeof(double);
            double yMin;
            memcpy(&yMin, headerData + dataIndex, sizeof(double));
            ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, yMin);
            dataIndex += sizeof(double);
            double zMin;
            memcpy(&zMin, headerData + dataIndex, sizeof(double));
            ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, zMin);
            dataIndex += sizeof(double);
            double xMax;
            memcpy(&xMax, headerData + dataIndex, sizeof(double));
            ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, xMax);
            dataIndex += sizeof(double);
            double yMax;
            memcpy(&yMax, headerData + dataIndex, sizeof(double));
            ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, yMax);
            dataIndex += sizeof(double);
            double zMax;
            memcpy(&zMax, headerData + dataIndex, sizeof(double));
            ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, zMax);
            dataIndex += sizeof(double);

            memcpy(&header->m_contentExtentDefined, headerData + dataIndex, sizeof(header->m_contentExtentDefined));
            dataIndex += sizeof(header->m_contentExtentDefined);
            if (header->m_contentExtentDefined)
                {
                double xMin;
                memcpy(&xMin, headerData + dataIndex, sizeof(double));
                ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, xMin);
                dataIndex += sizeof(double);
                double yMin;
                memcpy(&yMin, headerData + dataIndex, sizeof(double));
                ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, yMin);
                dataIndex += sizeof(double);
                double zMin;
                memcpy(&zMin, headerData + dataIndex, sizeof(double));
                ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, zMin);
                dataIndex += sizeof(double);
                double xMax;
                memcpy(&xMax, headerData + dataIndex, sizeof(double));
                ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, xMax);
                dataIndex += sizeof(double);
                double yMax;
                memcpy(&yMax, headerData + dataIndex, sizeof(double));
                ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, yMax);
                dataIndex += sizeof(double);
                double zMax;
                memcpy(&zMax, headerData + dataIndex, sizeof(double));
                ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, zMax);
                dataIndex += sizeof(double);
                }

            /* Indices */
            uint32_t nbIndiceIds;
            memcpy(&nbIndiceIds, headerData + dataIndex, sizeof(nbIndiceIds));
            dataIndex += sizeof(nbIndiceIds);
            header->m_ptsIndiceID.clear();
            header->m_ptsIndiceID.reserve(nbIndiceIds);
            for (size_t i = 0; i < nbIndiceIds; i++)
                {
                uint32_t indiceId;
                memcpy(&indiceId, headerData + dataIndex, sizeof(indiceId));
                dataIndex += sizeof(indiceId);
                header->m_ptsIndiceID.push_back(indiceId != IDTMFile::GetNullNodeID() ? HPMBlockID(indiceId) : IDTMFile::GetNullNodeID());
                }
            
            /* UVs */
            uint32_t uvID;
            memcpy(&uvID, headerData + dataIndex, sizeof(uvID));
            dataIndex += sizeof(uvID);
            header->m_uvID = uvID != IDTMFile::GetNullNodeID() ? HPMBlockID(uvID) : 0;

            uint32_t nbUVIDs;
            memcpy(&nbUVIDs, headerData + dataIndex, sizeof(nbUVIDs));
            dataIndex += sizeof(nbUVIDs);
            header->m_uvsIndicesID.clear();
            header->m_uvsIndicesID.reserve(nbUVIDs);
            for (size_t i = 0; i < nbUVIDs; i++)
                {
                uint32_t uvIndice;
                memcpy(&uvIndice, headerData + dataIndex, sizeof(uvIndice));
                dataIndex += sizeof(uvIndice);
                header->m_uvsIndicesID.push_back(uvIndice != IDTMFile::GetNullNodeID() ? HPMBlockID(uvIndice) : IDTMFile::GetNullNodeID());
                }
            
            /* Textures */
            uint32_t nbTextureIDs;
            memcpy(&nbTextureIDs, headerData + dataIndex, sizeof(nbTextureIDs));
            dataIndex += sizeof(nbTextureIDs);
            header->m_textureID.clear();
            header->m_textureID.reserve(nbTextureIDs);
            for (size_t i = 0; i < nbTextureIDs; i++)
                {
                uint32_t textureID;
                memcpy(&textureID, headerData + dataIndex, sizeof(textureID));
                dataIndex += sizeof(textureID);
                header->m_textureID.push_back(textureID != IDTMFile::GetNullNodeID() ? HPMBlockID(textureID) : IDTMFile::GetNullNodeID());
                }
            
            /* Mesh components */
            size_t numberOfMeshComponents;
            memcpy(&numberOfMeshComponents, headerData + dataIndex, sizeof(numberOfMeshComponents));
            dataIndex += sizeof(numberOfMeshComponents);
            header->m_numberOfMeshComponents = numberOfMeshComponents;
            header->m_meshComponents = new int[numberOfMeshComponents];
            for (size_t componentIdx = 0; componentIdx < header->m_numberOfMeshComponents; componentIdx++)
                {
                int component;
                memcpy(&component, headerData + dataIndex, sizeof(component));
                dataIndex += sizeof(component);
                header->m_meshComponents[componentIdx] = component;
                }
            
            /* Clips */
            uint32_t nbClipSetsIDs;
            memcpy(&nbClipSetsIDs, headerData + dataIndex, sizeof(nbClipSetsIDs));
            dataIndex += sizeof(nbClipSetsIDs);
            header->m_clipSetsID.clear();
            header->m_clipSetsID.reserve(nbClipSetsIDs);
            for (size_t i = 0; i < nbClipSetsIDs; ++i)
                {
                uint32_t clip;
                memcpy(&clip, headerData + dataIndex, sizeof(clip));
                dataIndex += sizeof(clip);
                header->m_clipSetsID.push_back(clip != IDTMFile::GetNullNodeID() ? HPMBlockID(clip) : IDTMFile::GetNullNodeID());
                }
            
            /* Children */
            size_t nbChildren;
            memcpy(&nbChildren, headerData + dataIndex, sizeof(nbChildren));
            dataIndex += sizeof(nbChildren);
            header->m_apSubNodeID.clear();
            header->m_apSubNodeID.reserve(nbChildren);
            header->m_numberOfSubNodesOnSplit = nbChildren;
            for (size_t childInd = 0; childInd < nbChildren; childInd++)
                {
                uint32_t childID;
                memcpy(&childID, headerData + dataIndex, sizeof(childID));
                dataIndex += sizeof(childID);
                header->m_apSubNodeID.push_back(childID != IDTMFile::GetNullNodeID() ? HPMBlockID(childID) : IDTMFile::GetNullNodeID());
                }
            
            /* Neighbors */
            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {
                size_t numNeighbors;
                memcpy(&numNeighbors, headerData + dataIndex, sizeof(numNeighbors));
                dataIndex += sizeof(numNeighbors);
                header->m_apNeighborNodeID[neighborPosInd].clear();
                header->m_apNeighborNodeID[neighborPosInd].reserve(numNeighbors);
                for (size_t neighborInd = 0; neighborInd < numNeighbors; neighborInd++)
                    {
                    uint32_t neighborId;
                    memcpy(&neighborId, headerData + dataIndex, sizeof(neighborId));
                    dataIndex += sizeof(neighborId);
                    header->m_apNeighborNodeID[neighborPosInd].push_back(neighborId != IDTMFile::GetNullNodeID() ? HPMBlockID(neighborId) : IDTMFile::GetNullNodeID());
                    }
                }
            assert(dataIndex == maxCountData);
            }

        void ReadNodeHeaderFromJSON(SMPointNodeHeader<EXTENT>* header, const Json::Value& nodeHeader) const
            {
            auto& nodeExtent = nodeHeader["nodeExtent"];
            assert(nodeExtent.isObject());
            ExtentOp<EXTENT>::SetXMin(header->m_nodeExtent, nodeExtent["xMin"].asDouble());
            ExtentOp<EXTENT>::SetYMin(header->m_nodeExtent, nodeExtent["yMin"].asDouble());
            ExtentOp<EXTENT>::SetZMin(header->m_nodeExtent, nodeExtent["zMin"].asDouble());
            ExtentOp<EXTENT>::SetXMax(header->m_nodeExtent, nodeExtent["xMax"].asDouble());
            ExtentOp<EXTENT>::SetYMax(header->m_nodeExtent, nodeExtent["yMax"].asDouble());
            ExtentOp<EXTENT>::SetZMax(header->m_nodeExtent, nodeExtent["zMax"].asDouble());

            header->m_contentExtentDefined = nodeHeader["contentExtent"].asBool();
            if (header->m_contentExtentDefined)
                {
                auto& contentExtent = nodeHeader["contentExtent"];
                assert(contentExtent.isObject());
                ExtentOp<EXTENT>::SetXMin(header->m_contentExtent, contentExtent["xMin"].asDouble());
                ExtentOp<EXTENT>::SetYMin(header->m_contentExtent, contentExtent["yMin"].asDouble());
                ExtentOp<EXTENT>::SetZMin(header->m_contentExtent, contentExtent["zMin"].asDouble());
                ExtentOp<EXTENT>::SetXMax(header->m_contentExtent, contentExtent["xMax"].asDouble());
                ExtentOp<EXTENT>::SetYMax(header->m_contentExtent, contentExtent["yMax"].asDouble());
                ExtentOp<EXTENT>::SetZMax(header->m_contentExtent, contentExtent["zMax"].asDouble());
                }

            header->m_level = nodeHeader["resolution"].asUInt();
            header->m_filtered = nodeHeader["filtered"].asBool();
            uint32_t parentNodeID = nodeHeader["parentID"].asUInt();
            header->m_parentNodeID = parentNodeID != IDTMFile::GetNullNodeID() ? HPMBlockID(parentNodeID) : IDTMFile::GetNullNodeID();
            header->m_numberOfSubNodesOnSplit = nodeHeader["nbChildren"].asUInt();
            header->m_apSubNodeID.resize(header->m_numberOfSubNodesOnSplit);
            //header->m_IsLeaf = nodeHeader["isLeaf"].asBool();
            auto& children = nodeHeader["children"];
            assert(children.isArray());
            if (/*!header->m_IsLeaf &&*/ children.size() > 0)
                {
                for (auto& child : children)
                    {
                    auto childInd = child["index"].asUInt();
                    auto nodeId = child["id"].asUInt();
                    if (nodeId != IDTMFile::GetNullNodeID()) header->m_apSubNodeID[childInd] = HPMBlockID(nodeId);
                    }
                }
            header->m_IsLeaf = header->m_apSubNodeID.size() == 0 || (!header->m_apSubNodeID[0].IsValid());
            header->m_IsBranched = !header->m_IsLeaf && (header->m_apSubNodeID.size() > 1 && header->m_apSubNodeID[1].IsValid());
            if (!header->m_IsLeaf && !header->m_IsBranched) header->m_SubNodeNoSplitID = header->m_apSubNodeID[0];

            auto& neighbors = nodeHeader["neighbors"];
            assert(neighbors.isArray());
            if (neighbors.size() > 0)
                {
                for (auto& neighbor : neighbors)
                    {
                    assert(neighbor.isObject());
                    auto nodePos = neighbor["nodePos"].asUInt();
                    auto nodeId = neighbor["nodeId"].asUInt();
                    header->m_apNeighborNodeID[nodePos].push_back(nodeId);
                    }
                }

            //header->m_IsBranched = nodeHeader["isBranched"].asBool();
            header->m_balanced = nodeHeader["isBalanced"].asBool();
            header->m_SplitTreshold = nodeHeader["splitThreshold"].asUInt();
            header->m_totalCountDefined = true;
            header->m_totalCount = nodeHeader["totalCount"].asUInt();
            header->m_nodeCount = nodeHeader["nodeCount"].asUInt();
            header->m_arePoints3d = nodeHeader["arePoints3d"].asBool();
            header->m_nbFaceIndexes = nodeHeader["nbFaceIndexes"].asUInt();
            header->m_graphID = nodeHeader["graphID"].asUInt();
            header->m_uvID = nodeHeader["uvID"].asUInt();
            header->m_isTextured = nodeHeader["areTextured"].asBool();

            if (header->m_isTextured)
                {
                header->m_textureID.resize(1);
                header->m_textureID[0] = HPMBlockID(header->m_uvID);

                auto& indices = nodeHeader["indiceID"];
                assert(indices.isArray());
                header->m_ptsIndiceID.resize(indices.size());
                if (indices.size() > 0)
                    {
                    for (size_t indiceID = 0; indiceID < (size_t)indices.size(); indiceID++)
                        {
                        auto id = indices[(Json::ArrayIndex)indiceID].asUInt();
                        header->m_ptsIndiceID[indiceID] = id != IDTMFile::GetNullNodeID() ? HPMBlockID(id) : IDTMFile::GetNullNodeID();
                        }
                    }

                auto& uvIndiceIDs = nodeHeader["uvIndiceIDs"];
                assert(uvIndiceIDs.isArray());
                header->m_uvsIndicesID.resize(uvIndiceIDs.size());
                if (uvIndiceIDs.size() > 0)
                    {
                    for (size_t indiceID = 0; indiceID < (size_t)uvIndiceIDs.size(); indiceID++)
                        {
                        auto uvIndiceID = uvIndiceIDs[(Json::ArrayIndex)indiceID].asUInt();
                        header->m_uvsIndicesID[indiceID] = uvIndiceID != IDTMFile::GetNullNodeID() ? HPMBlockID(uvIndiceID) : IDTMFile::GetNullNodeID();
                        }
                    }
                else
                    {
                    header->m_uvsIndicesID.resize(1);
                    header->m_uvsIndicesID[0] = HPMBlockID();
                    }
                }


            //if (ConvertBlockID(header->m_uvID) == IDTMFile::GetNullNodeID()) header->m_uvID = HPMBlockID();
            //if (ConvertBlockID(header->m_graphID) == IDTMFile::GetNullNodeID()) header->m_graphID = HPMBlockID();
            header->m_numberOfMeshComponents = (size_t)nodeHeader["numberOfMeshComponents"].asUInt();
            auto& meshComponents = nodeHeader["meshComponents"];
            assert(meshComponents.isArray());
            header->m_meshComponents = new int[header->m_numberOfMeshComponents];
            if (header->m_numberOfMeshComponents > 0)
                {
                for (size_t i = 0; i < (size_t)header->m_numberOfMeshComponents; i++)
                    {
                    header->m_meshComponents[i] = meshComponents[(Json::ArrayIndex)i].asInt();
                    }
                }

            auto& clipSets = nodeHeader["clipSetsID"];
            assert(clipSets.isArray());
            header->m_clipSetsID.resize(clipSets.size());
            if (header->m_clipSetsID.size() > 0)
                {
                for (size_t i = 0; i < header->m_clipSetsID.size(); ++i) header->m_clipSetsID[i] = clipSets[(Json::ArrayIndex)i].asInt();
                }

            }

        bool DownloadBlockFromFileServer(const std::wstring& block_name, bvector<Byte>* pointData, size_t maxCountData) const
            {
            CURL *curl_handle;
            CURLcode res;
            bool retCode = true;

            struct MemoryStruct chunk;

            pointData->reserve(maxCountData);
            chunk.memory = pointData;
            chunk.size = 0;

            curl_global_init(CURL_GLOBAL_ALL);

            curl_handle = curl_easy_init();

            Utf8String blockUrl = "http://realitydatastreaming.azurewebsites.net/Mesh/c1sub_scalablemesh/";
            Utf8String name;
            BeStringUtilities::WCharToUtf8(name, block_name.c_str());
            blockUrl += name;

            
            // NEEDS_WORK_SM_STREAMING: Remove this when streaming works reasonably well
            //std::lock_guard<std::mutex> lock(fileMutex);
            //BeFile file;
            //if (BeFileStatus::Success == OPEN_FILE(file, L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\FitView.node", BeFileAccess::ReadWrite) ||
            //    BeFileStatus::Success == file.Create(L"C:\\Users\\Richard.Bois\\Documents\\ScalableMeshWorkDir\\FitView.node"))
            //    {
            //    file.SetPointer(0, BeFileSeekOrigin::End);
            //    auto node_location = L"http://realitydatastreaming.azurewebsites.net/Mesh/c1sub_scalablemesh/" + block_name + L"\n";
            //    Utf8String utf8_node_location;
            //    BeStringUtilities::WCharToUtf8(utf8_node_location, node_location.c_str());
            //    file.Write(NULL, utf8_node_location.c_str(), (uint32_t)utf8_node_location.length());
            //    }
            //else
            //    {
            //    assert(!"Problem creating nodes file");
            //    }
            //
            //file.Close();
            
            curl_easy_setopt(curl_handle, CURLOPT_URL, blockUrl.c_str());

            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

            //    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

            /* get it! */
            res = curl_easy_perform(curl_handle);

            /* check for errors */
            if (res != CURLE_OK)
                {
                retCode = false;
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
                }

            /* cleanup curl stuff */
            curl_easy_cleanup(curl_handle);

            /* we're done with libcurl, so clean it up */
            curl_global_cleanup();
            return retCode;
            }

        HFCPtr<SMNodeGroup> FindGroup(HPMBlockID blockID)
            {
            auto nodeIDToFind = this->ConvertBlockID(blockID);
            for (auto& group : m_nodeHeaderGroups)
                {
                if (group->ContainsNode(nodeIDToFind))
                    {
                    return group;
                    }
                }

            return nullptr;
            }

        HFCPtr<SMNodeGroup> GetGroup(HPMBlockID blockID)
            {
            auto group = this->FindGroup(blockID);
            assert(group != nullptr);
            if (!group->IsLoaded())
                {
                group->Load();
                }
            return group;
            }

        Json::Value GetNodeHeader(HPMBlockID blockID)
            {
			//NEEDS_WORK_SM_STREAMING : are we loading node headers multiple times?
            std::lock_guard<std::mutex> lock(headerLock);
            Json::Value result;
            WString pathToNodes(m_path + L"nodes\\");
            wstringstream ss;
            ss << pathToNodes << L"n_" << ConvertBlockID(blockID) << L".bin";

            auto filename = ss.str();

            if (s_stream_from_disk)
                {
                BeFile file;
                if (BeFileStatus::Success != OPEN_FILE(file, filename.c_str(), BeFileAccess::Read))
                    {
                    assert(false); // node header file must exist
                    return 1;
                    }
                char inBuffer[100000];
                uint32_t bytes_read = 0;
                file.Read(inBuffer, &bytes_read, 100000);

                Json::Reader reader;
                reader.parse(&inBuffer[0], &inBuffer[bytes_read], result);
                }
            else if (s_stream_from_file_server)
                {
                ss << L".bin";
                auto blob_name = ss.str();
                bvector<Byte> buffer;
                DownloadBlockFromFileServer(blob_name.c_str(), &buffer, 100000);
                assert(!buffer.empty() && buffer.size() <= 100000);
                Json::Reader reader;
                reader.parse(reinterpret_cast<char*>(&buffer.front()), reinterpret_cast<char*>(&buffer.back()), result);
                }
            else {
                auto blob_name = filename.c_str();
                m_stream_store.DownloadBlob(blob_name, [&result](scalable_mesh::azure::Storage::point_buffer_type& buffer)
                    {
                    assert(!buffer.empty());
                    Json::Reader reader;
                    reader.parse(reinterpret_cast<char*>(&buffer.front()), reinterpret_cast<char*>(&buffer.back()), result);
                    });
                }
            return result;
            }

        PointBlock& GetBlock(HPMBlockID blockID) const
            {
            auto blockIDConvert = ConvertBlockID(blockID);
            PointBlock* block = nullptr;
            if (m_countInfo.count(blockIDConvert) > 0)
                {
                assert(m_countInfo.count(blockIDConvert) == 1);

                // Block already created. Check if data available.
                block = &m_countInfo[blockIDConvert];
                if (block->IsLoading())
                    {
                    // Data not available yet
                    block->LockAndWait();
                    }
                }
            else
                {
                block = &this->GetNodeData(blockID);
                }
            assert(block != nullptr && block->IsLoaded());
            return *block;
            }

        PointBlock& GetNodeData(HPMBlockID blockID) const
            {
            assert(this->m_countInfo.count(ConvertBlockID(blockID)) == 0);
            auto blockIDConvert = ConvertBlockID(blockID);
            auto& points = this->m_countInfo[blockIDConvert];
            points.SetLoading();

            WString pathToPoints((s_stream_from_disk ? m_path + L"points\\" : m_path));
            wstringstream ss;
            ss << pathToPoints << L"p_" << blockIDConvert << L".bin";
            auto filename = ss.str();

            if (s_stream_from_disk)
                {
                uint32_t uncompressedSize = 0;
                BeFile file;
                if (BeFileStatus::Success == OPEN_FILE_SHARE(file, filename.c_str(), BeFileAccess::Read))
                    {

                    size_t fileSize = 0;
                    file.GetSize(fileSize);

                    // Read uncompressed size
                    uint32_t bytesRead = 0;
                    auto read_result = file.Read(&uncompressedSize, &bytesRead, sizeof(uint32_t));
                    HASSERT(BeFileStatus::Success == read_result);
                    HASSERT(bytesRead == sizeof(uint32_t));

                    // Read compressed points
                    auto compressedSize = fileSize - sizeof(uint32_t);
                    bvector<uint8_t> ptData(compressedSize);
                    read_result = file.Read(&ptData[0], &bytesRead, (uint32_t)compressedSize);
                    HASSERT(bytesRead == compressedSize);
                    HASSERT(BeFileStatus::Success == read_result);
                    file.Close();

                    HCDPacket uncompressedPacket, compressedPacket;
                    compressedPacket.SetBuffer(&ptData[0], ptData.size());
                    compressedPacket.SetDataSize(ptData.size());
                    uncompressedPacket.SetDataSize(uncompressedSize);
                    LoadCompressedPacket(compressedPacket, uncompressedPacket);
                    assert(uncompressedSize == uncompressedPacket.GetDataSize());
                    points.resize(uncompressedSize);
                    memcpy(points.data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());
                    }
                else
                    {
                    HASSERT(!"Problem opening block of points for reading");
                    file.Close();
                    }
                }
            else if (s_stream_from_file_server)
                {
                bvector<uint8_t> buffer;
                DownloadBlockFromFileServer(filename, &buffer, 1000000);
                assert(!buffer.empty() && buffer.size() <= 1000000);

                uint32_t UncompressedSize = reinterpret_cast<uint32_t&>(buffer[0]);
                uint32_t sizeData = (uint32_t)buffer.size();

                HCDPacket uncompressedPacket, compressedPacket;
                compressedPacket.SetBuffer(&buffer[0] + sizeof(uint32_t), sizeData - sizeof(uint32_t));
                compressedPacket.SetDataSize(sizeData - sizeof(uint32_t));
                uncompressedPacket.SetDataSize(UncompressedSize);
                LoadCompressedPacket(compressedPacket, uncompressedPacket);
                assert(UncompressedSize == uncompressedPacket.GetDataSize());
                points.resize(UncompressedSize);
                memcpy(points.data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());
                }
            else {
                // stream from azure
                bool blobDownloaded = false;
                m_stream_store.DownloadBlob(filename.c_str(), [this, &points, &blobDownloaded](scalable_mesh::azure::Storage::point_buffer_type& buffer)
                    {
                    assert(!buffer.empty());
                    //assert(buffer.size() == sizeDataLocal);
                    //assert(0 == memcmp(&buffer[0], dataArrayTmp, sizeDataLocal));
                    uint32_t UncompressedSize = reinterpret_cast<uint32_t&>(buffer[0]);
                    uint32_t sizeData = (uint32_t)buffer.size() - sizeof(uint32_t);

                    HCDPacket uncompressedPacket, compressedPacket;
                    compressedPacket.SetBuffer(&buffer[0] + sizeof(uint32_t), sizeData);
                    compressedPacket.SetDataSize(sizeData);
                    uncompressedPacket.SetDataSize(UncompressedSize);
                    LoadCompressedPacket(compressedPacket, uncompressedPacket);
                    assert(UncompressedSize == uncompressedPacket.GetDataSize());
                    points.resize(UncompressedSize);
                    memcpy(points.data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());
                    blobDownloaded = true;
                    });
                assert(blobDownloaded);
                }
            points.SetLoaded();

            return points;
            }

    public:
        // Constructor / Destroyer

        SMStreamingPointTaggedTileStore(WString& path, WString grouped_headers_path = L"", bool areNodeHeadersGrouped = false, bool compress = true)
            :m_path(path),
            m_path_to_grouped_headers(grouped_headers_path),
            m_node_id(0),
            m_use_node_header_grouping(areNodeHeadersGrouped),
            m_pCodec(new HCDCodecZlib()),
            m_storage_connection_string(L"DefaultEndpointsProtocol=https;AccountName=pcdsustest;AccountKey=3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ=="),
            m_stream_store(m_storage_connection_string.c_str(), L"scalablemeshtest")
            {
            if (s_stream_from_disk)
                {
                // Create base directory structure to store information if not already done
                // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only
                if (0 == CreateDirectoryW(m_path.c_str(), NULL))
                    {
                    assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }

                WString pathToNodes(m_path + L"nodes/");
                if (0 == CreateDirectoryW(pathToNodes.c_str(), NULL))
                    {
                    assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }
                WString pathToPoints(m_path + L"points/");
                if (0 == CreateDirectoryW(pathToPoints.c_str(), NULL))
                    {
                    assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }
                }
            else
                {
                // stream from azure
                }
            }

        virtual ~SMStreamingPointTaggedTileStore()
            {
            }

        virtual bool HasSpatialReferenceSystem()
        {
        // NEEDS_WORK_SM_STREAMING : Add check to spatial reference system
        HASSERT(!"TODO!");
        return false;
        }


        // New function
        virtual std::string GetSpatialReferenceSystem()
        {
        // NEEDS_WORK_SM_STREAMING : Add check to spatial reference system
        HASSERT(!"TODO!");
        return string("");
        }

        // ITileStore interface
        virtual void Close()
        {
        }

        virtual bool StoreMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
            {
            //SMPointTaggedTileStore::StoreMasterHeader(indexHeader, headerSize);
            if (indexHeader != NULL && indexHeader->m_rootNodeBlockID.IsValid())
                {
                m_masterHeader["balanced"] = indexHeader->m_balanced;
                m_masterHeader["depth"] = (uint32_t)indexHeader->m_depth;
                m_masterHeader["rootNodeBlockID"] = ConvertBlockID(indexHeader->m_rootNodeBlockID);
                m_masterHeader["splitThreshold"] = indexHeader->m_SplitTreshold;
                //HASSERT(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem
                //m_masterHeader["singleFile"] = indexHeader->m_singleFile;
                m_masterHeader["singleFile"] = false;

                // Write to file
                auto filename = (m_path + L"MasterHeader.sscm").c_str();
                BeFile file;
                uint64_t buffer_size;
                auto jsonWriter = [&file, &indexHeader, &buffer_size](BeFile& file, Json::Value& object) {

                    Json::StyledWriter writer;
                    auto buffer = writer.write(object);
                    buffer_size = buffer.size();
                    file.Write(NULL, buffer.c_str(), buffer_size);
                    };
                if (BeFileStatus::Success == OPEN_FILE(file, filename, BeFileAccess::Write) )//file.Open(filename, BeFileAccess::Write, BeFileSharing::None))
                    {
                    jsonWriter(file, m_masterHeader);
                    }
                else if (BeFileStatus::Success == file.Create(filename))
                    {
                    jsonWriter(file, m_masterHeader);
                    }
                else
                    {
                    HASSERT(!"Problem creating master header file");
                    }
                file.Close();
                }

            return true;
            }

        virtual size_t LoadMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
            {

            if (indexHeader != NULL)
                {

                if (m_use_node_header_grouping || s_stream_from_grouped_store)
                    {
                    wstringstream ss;
                    ss << m_path_to_grouped_headers << L"MasterHeader.bin";
                    auto filename = ss.str();
                    if (m_nodeHeaderGroups.empty())
                        {
                        char* masterHeaderBuffer = nullptr;
                        if (s_stream_from_disk)
                            {
                            BeFile file;
                            if (BeFileStatus::Success != OPEN_FILE(file, filename.c_str(), BeFileAccess::Read))
                                {
                                return 0;
                                }
                            file.GetSize(headerSize);
                            //bvector<Byte> masterHeaderBuffer(fileSize);
                            masterHeaderBuffer = new char[headerSize];
                            uint32_t bytes_read;
                            file.Read(masterHeaderBuffer, &bytes_read, (uint32_t)headerSize);
                            assert(bytes_read == headerSize);

                            file.Close();
                            }
                        else 
                            {
                            m_stream_store.DownloadBlob(filename.c_str(), [indexHeader, &headerSize, &masterHeaderBuffer](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                                {
                                if (buffer.empty())
                                    {
                                    headerSize = 0;
                                    return;
                                    }
                                headerSize = (uint32_t)buffer.size();

                                masterHeaderBuffer = new char[headerSize];
                                memcpy(masterHeaderBuffer, buffer.data(), headerSize);
                                });
                            }

                        uint64_t position = 0;

                        uint32_t sizeOfOldMasterHeaderPart;
                        memcpy(&sizeOfOldMasterHeaderPart, masterHeaderBuffer + position, sizeof(sizeOfOldMasterHeaderPart));
                        position += sizeof(sizeOfOldMasterHeaderPart);
                        assert(sizeOfOldMasterHeaderPart == sizeof(SQLiteIndexHeader));

                        SQLiteIndexHeader oldMasterHeader;
                        memcpy(&oldMasterHeader, masterHeaderBuffer + position, sizeof(SQLiteIndexHeader));
                        position += sizeof(SQLiteIndexHeader);
                        indexHeader->m_SplitTreshold = oldMasterHeader.m_SplitTreshold;
                        indexHeader->m_balanced = oldMasterHeader.m_balanced;
                        indexHeader->m_depth = oldMasterHeader.m_depth;
                        indexHeader->m_singleFile = oldMasterHeader.m_singleFile;
                        HASSERT(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem
                        
                        auto rootNodeBlockID = oldMasterHeader.m_rootNodeBlockID;
                        indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();


                        //{ // NEEDS_WORK_SM : The old Master Header is saved in JSON format. Should be binary.
                        //Json::Reader reader;
                        //Json::Value masterHeader;
                        //reader.parse(masterHeaderBuffer + position, masterHeaderBuffer + sizeOfOldMasterHeaderPart, masterHeader);
                        //position += sizeOfOldMasterHeaderPart;
                        //
                        //indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
                        //indexHeader->m_balanced = masterHeader["balanced"].asBool();
                        //indexHeader->m_depth = masterHeader["depth"].asUInt();
                        //indexHeader->m_singleFile = masterHeader["singleFile"].asBool();
                        //HASSERT(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem
                        //
                        //auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
                        //indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();
                        //}

                        // Parse rest of file -- group information
                        while (position < headerSize)
                            {
                            size_t group_id;
                            memcpy(&group_id, masterHeaderBuffer + position, sizeof(group_id));
                            position += sizeof(group_id);

                            size_t group_size;
                            memcpy(&group_size, masterHeaderBuffer + position, sizeof(size_t));
                            position += sizeof(size_t);
                            //assert(group_size <= s_max_number_nodes_in_group);

                            auto group = HFCPtr<SMNodeGroup>(new SMNodeGroup(group_id, group_size));
                            group->SetDataSource(m_path_to_grouped_headers + L"g_", m_stream_store);
                            m_nodeHeaderGroups.push_back(group);

                            vector<uint64_t> nodeIds(group_size);
                            memcpy(nodeIds.data(), masterHeaderBuffer + position, group_size*sizeof(uint64_t));
                            position += group_size*sizeof(uint64_t);

                            group->GetHeader()->resize(group_size);
                            transform(begin(nodeIds), end(nodeIds), begin(*group->GetHeader()), [](const uint64_t& nodeId) 
                                {
                                return SMNodeHeader{ nodeId, 0, 0 };
                                });
                            }

                        delete[] masterHeaderBuffer;
                        }
                    }

                else if (s_stream_from_disk)
                    {
                    // For this particular implementation the header size is unused ... The indexHeader is unique and of known size
                    BeFile file;
                    auto filename = (m_path + L"MasterHeader.sscm").c_str();
                    if (BeFileStatus::Success != OPEN_FILE(file, filename, BeFileAccess::Read))//file.Open(filename, BeFileAccess::Read, BeFileSharing::None))
                        {
                        assert(!"Local master header could not be found");
                        return 0;
                        }
                    char inBuffer[100000];
                    uint32_t bytes_read = 0;
                    file.Read(inBuffer, &bytes_read, (uint32_t)headerSize);

                    Json::Reader reader;
                    Json::Value masterHeader;
                    reader.parse(&inBuffer[0], &inBuffer[bytes_read], masterHeader);

                    indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
                    indexHeader->m_balanced = masterHeader["balanced"].asBool();
                    indexHeader->m_depth = masterHeader["depth"].asUInt();
                indexHeader->m_singleFile = masterHeader["singleFile"].asBool();
                HASSERT(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

                    auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
                    indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();
                    file.Close();

                    // Save in local stm file
                    //if (m_DTMFile != NULL)
                    //	return SMPointTaggedTileStore::StoreMasterHeader(indexHeader, headerSize);
                    }
                else {
                    auto blob_name = m_path + L"MasterHeader.sscm";
                    m_stream_store.DownloadBlob(blob_name.c_str(), [indexHeader, &headerSize](const scalable_mesh::azure::Storage::point_buffer_type& buffer)
                        {
                        if (buffer.empty()) 
                            {
                            headerSize = 0;
                            return;
                            }
                        headerSize = (uint32_t)buffer.size();
                        Json::Reader reader;
                        Json::Value masterHeader;
                        reader.parse(reinterpret_cast<const char*>(&buffer.front()), reinterpret_cast<const char*>(&buffer.back()), masterHeader);

                        indexHeader->m_SplitTreshold = masterHeader["splitThreshold"].asUInt();
                        indexHeader->m_balanced = masterHeader["balanced"].asBool();
                        indexHeader->m_depth = masterHeader["depth"].asUInt();

                        auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
                        indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

                        });

                    }
                return headerSize;
                }

            return 0;
            }

        virtual HPMBlockID StoreNewBlock(POINT* DataTypeArray, size_t countData)
            {
            assert(false); // Should not pass here
            return HPMBlockID(-1);
            }

        virtual HPMBlockID StoreBlock(POINT* DataTypeArray, size_t countData, HPMBlockID blockID)
            {
            HPRECONDITION(blockID.IsValid());

            auto blockIDConvert = ConvertBlockID(blockID);
            if (NULL != DataTypeArray && countData > 0)
                {
                WString pathToPoints(m_path + L"points/");
                wstringstream ss;
                ss << pathToPoints << L"p_" << blockIDConvert << L".bin";
                auto filename = ss.str();
                BeFile file;
                auto fileOpened = OPEN_FILE(file, filename.c_str(), BeFileAccess::Write);//file.Open(filename.c_str(), BeFileAccess::Write, BeFileSharing::None);
                if (BeFileStatus::Success != fileOpened)
                    {
                    auto fileCreated = file.Create(filename.c_str());
                    assert(BeFileStatus::Success == fileCreated);
                    fileOpened = fileCreated;
                    }
                assert(BeFileStatus::Success == fileOpened);

                HCDPacket uncompressedPacket, compressedPacket;
                size_t bufferSize = countData * sizeof(POINT);
                Byte* dataArrayTmp = new Byte[bufferSize];
                memcpy(dataArrayTmp, DataTypeArray, bufferSize);
                uncompressedPacket.SetBuffer(dataArrayTmp, bufferSize);
                uncompressedPacket.SetDataSize(bufferSize);
                WriteCompressedPacket(uncompressedPacket, compressedPacket);

                Byte* data = new Byte[compressedPacket.GetDataSize() + sizeof(uint32_t)];
                auto UncompressedSize = (uint32_t)uncompressedPacket.GetDataSize();
                reinterpret_cast<uint32_t&>(*data) = UncompressedSize;
                if (m_countInfo.count(blockIDConvert) > 0)
                    {
                    // must update data count
                    auto& points = this->m_countInfo[blockIDConvert];
                    points.resize(UncompressedSize);
                    memcpy(points.data(), uncompressedPacket.GetBufferAddress(), uncompressedPacket.GetDataSize());
                    }

                memcpy(data + sizeof(uint32_t), compressedPacket.GetBufferAddress(), compressedPacket.GetDataSize());
                file.Write(NULL, data, (uint32_t)compressedPacket.GetDataSize() + sizeof(uint32_t));
                file.Close();
                delete[] data;
                delete[] dataArrayTmp;
                }
            return blockID;
            }

     virtual size_t GetBlockDataCount(HPMBlockID blockID) const
            {
            if (IsValidID(blockID))
                return this->GetBlock(blockID).size() / sizeof(POINT);
            return 0;
         }


        virtual size_t StoreHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            Json::Value block;

            block["id"] = ConvertBlockID(blockID);
            block["resolution"] = (IDTMFile::NodeID)header->m_level;
            block["filtered"] = header->m_filtered;
            block["parentID"] = header->m_parentNodeID.IsValid() ? ConvertBlockID(header->m_parentNodeID) : IDTMFile::GetNullNodeID();
            block["isLeaf"] = header->m_IsLeaf;
            block["isBranched"] = header->m_IsBranched;
            block["splitThreshold"] = header->m_SplitTreshold;

            size_t nbChildren = header->m_IsLeaf || (!header->m_IsBranched  && !header->m_SubNodeNoSplitID.IsValid()) ? 0 : (!header->m_IsBranched ? 1 : header->m_numberOfSubNodesOnSplit);

            block["nbChildren"] = nbChildren;

            auto& children = block["children"];

            if (nbChildren > 1)
                {
                for (size_t childInd = 0; childInd < nbChildren; childInd++)
                    {
                    Json::Value& child = childInd >= children.size() ? children.append(Json::Value()) : children[(int)childInd];
                    child["index"] = (uint8_t)childInd;
                    child["id"] = header->m_apSubNodeID[childInd].IsValid() ? ConvertChildID(header->m_apSubNodeID[childInd]) : IDTMFile::GetNullNodeID();
                    }
                }
            else if (nbChildren == 1)
                {
                Json::Value& child = children.empty() ? children.append(Json::Value()) : children[0];
                child["index"] = 0;
                child["id"] = header->m_SubNodeNoSplitID.IsValid() ? ConvertChildID(header->m_SubNodeNoSplitID) : ConvertChildID(header->m_apSubNodeID[0]);
                }

            auto& neighbors = block["neighbors"];
            int neighborInfoInd = 0;
            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {
                for (size_t neighborInd = 0; neighborInd < header->m_apNeighborNodeID[neighborPosInd].size(); neighborInd++)
                    {
                    Json::Value& neighbor = (uint32_t)neighborInfoInd >= neighbors.size() ? neighbors.append(Json::Value()) : neighbors[(uint32_t)neighborInfoInd];
                    neighbor["nodePos"] = (uint8_t)neighborPosInd;
                    neighbor["nodeId"] = ConvertNeighborID(header->m_apNeighborNodeID[neighborPosInd][neighborInd]);
                    neighborInfoInd++;
                    }
                }

            auto& extent = block["nodeExtent"];

            extent["xMin"] = ExtentOp<EXTENT>::GetXMin(header->m_nodeExtent);
            extent["yMin"] = ExtentOp<EXTENT>::GetYMin(header->m_nodeExtent);
            extent["zMin"] = ExtentOp<EXTENT>::GetZMin(header->m_nodeExtent);
            extent["xMax"] = ExtentOp<EXTENT>::GetXMax(header->m_nodeExtent);
            extent["yMax"] = ExtentOp<EXTENT>::GetYMax(header->m_nodeExtent);
            extent["zMax"] = ExtentOp<EXTENT>::GetZMax(header->m_nodeExtent);

            if (header->m_contentExtentDefined)
                {
                block["contentExtentDefined"] = true;
                auto& contentExtent = block["contentExtent"];
                contentExtent["xMin"] = ExtentOp<EXTENT>::GetXMin(header->m_contentExtent);
                contentExtent["yMin"] = ExtentOp<EXTENT>::GetYMin(header->m_contentExtent);
                contentExtent["zMin"] = ExtentOp<EXTENT>::GetZMin(header->m_contentExtent);
                contentExtent["xMax"] = ExtentOp<EXTENT>::GetXMax(header->m_contentExtent);
                contentExtent["yMax"] = ExtentOp<EXTENT>::GetYMax(header->m_contentExtent);
                contentExtent["zMax"] = ExtentOp<EXTENT>::GetZMax(header->m_contentExtent);
                }
            else
                {
                block["contentExtentDefined"] = false;
                }


            block["totalCount"] = header->m_totalCount;
            block["nodeCount"] = header->m_nodeCount;
            block["arePoints3d"] = header->m_arePoints3d;

            /*

            //why was this commented?
            // assert(header->m_3dPointsDescBins.size() <= USHORT_MAX);
            // m_indexHandler->SetNb3dPointsBins(ConvertBlockID(blockID), header->m_3dPointsDescBins.size());

            */

            block["nbFaceIndexes"] = header->m_nbFaceIndexes;
            block["graphID"] = header->m_graphID.IsValid() ? ConvertBlockID(header->m_graphID) : IDTMFile::GetNullNodeID();
            block["nbIndiceID"] = (int)header->m_ptsIndiceID.size();

            auto& indiceID = block["indiceID"];
            for (size_t i = 0; i < header->m_ptsIndiceID.size(); i++)
                {
                Json::Value& indice = (uint32_t)i >= indiceID.size() ? indiceID.append(Json::Value()) : indiceID[(uint32_t)i];
                indice = header->m_ptsIndiceID[i].IsValid() ? ConvertBlockID(header->m_ptsIndiceID[i]) : IDTMFile::GetNullNodeID();
                }

            if (header->m_isTextured /*&& !header->m_textureID.empty() && IsValidID(header->m_textureID[0])*/)
                {
                block["areTextured"] = true;
                /*block["nbTextureIDs"] = (int)header->m_textureID.size();
                auto& textureIDs = block["textureIDs"];
                for (size_t i = 0; i < header->m_textureID.size(); i++)
                    {
                    auto convertedID = ConvertBlockID(header->m_textureID[i]);
                    if (convertedID != IDTMFile::GetNullNodeID())
                        {
                        Json::Value& textureID = (uint32_t)i >= textureIDs.size() ? textureIDs.append(Json::Value()) : textureIDs[(uint32_t)i];
                        textureID = header->m_textureID[i].IsValid() ? convertedID : IDTMFile::GetNullNodeID();
                        }
                    }*/
                block["uvID"] = header->m_uvID.IsValid() ? ConvertBlockID(header->m_uvID) : IDTMFile::GetNullNodeID();

                block["nbUVIDs"] = (int)header->m_uvsIndicesID.size();
                auto& uvIndiceIDs = block["uvIndiceIDs"];
                for (size_t i = 0; i < header->m_uvsIndicesID.size(); i++)
                    {
                    Json::Value& uvIndice = (uint32_t)i >= uvIndiceIDs.size() ? uvIndiceIDs.append(Json::Value()) : uvIndiceIDs[(uint32_t)i];
                    uvIndice = header->m_uvsIndicesID[i].IsValid() ? ConvertBlockID(header->m_uvsIndicesID[i]) : IDTMFile::GetNullNodeID();
                    }
                }
            else {
                block["areTextured"] = false;
                }

            block["numberOfMeshComponents"] = header->m_numberOfMeshComponents;
            auto& meshComponents = block["meshComponents"];
            for (size_t componentIdx = 0; componentIdx < header->m_numberOfMeshComponents; componentIdx++)
                {
                auto& component = (uint32_t)componentIdx >= meshComponents.size() ? meshComponents.append(Json::Value()) : meshComponents[(uint32_t)componentIdx];
                component = header->m_meshComponents[componentIdx];
                }

            if (header->m_clipSetsID.size() > 0)
                {
                auto& clipSetsID = block["clipSetsID"];
                for (size_t i = 0; i < header->m_clipSetsID.size(); ++i)
                    {
                    auto& clip = (uint32_t)i >= clipSetsID.size() ? clipSetsID.append(Json::Value()) : clipSetsID[(uint32_t)i];
                    clip = ConvertNeighborID(header->m_clipSetsID[i]);
                    }
                }
            //  else
            block["nbClipSets"] = (uint32_t)header->m_clipSetsID.size();

            // Save node to disk
            WString pathToNodes(m_path + L"nodes/");

            wstringstream ss;
            ss << pathToNodes << L"n_" << ConvertBlockID(blockID) << L".bin";

            auto filename = ss.str();
            BeFile file;
            uint64_t buffer_size;
            auto jsonWriter = [&buffer_size](BeFile& file, Json::Value& object) {

                Json::StyledWriter writer;
                auto buffer = writer.write(object);
                buffer_size = buffer.size();
                file.Write(NULL, buffer.c_str(), buffer_size);
                };
            if (BeFileStatus::Success == OPEN_FILE(file, filename.c_str(), BeFileAccess::Write))//file.Open(filename.c_str(), BeFileAccess::Write, BeFileSharing::None))
                {
                jsonWriter(file, block);
                }
            else if (BeFileStatus::Success == file.Create(filename.c_str()))
                {
                jsonWriter(file, block);
                }
            else
                {
                HASSERT(!"Problem creating master header file");
                }
            file.Close();

            return 1;
            }

        virtual size_t LoadHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            if (s_stream_from_grouped_store)
                {
                auto group = this->GetGroup(blockID);
                auto node_header = group->GetNodeHeader(ConvertBlockID(blockID));
                ReadNodeHeaderFromBinary(header, group->GetRawHeaders() + node_header.offset, node_header.size);
                header->m_isTextured = !header->m_textureID.empty();
                }
            else {
                auto nodeHeader = this->GetNodeHeader(blockID);
                ReadNodeHeaderFromJSON(header, nodeHeader);
                }
            return 1;
            }


        virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            auto& block = this->GetBlock(blockID);
            assert(block.size() <= maxCountData * sizeof(POINT));
            memcpy(DataTypeArray, block.data(), block.size());
               
            return block.size();
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            //if (m_DTMFile != NULL) return SMPointTaggedTileStore::DestroyBlock(blockID);

            return true;
            }
        /*
        static const IDTMFile::FeatureType MASS_POINT_FEATURE_TYPE = 0;
        */

    protected:
        /*const IDTMFile::File::Ptr& GetFileP() const
        {
        return m_DTMFile;
        }
        */

    private:

        WString m_path;
        Json::Value m_masterHeader;
        Json::Value m_header;
        uint32_t m_node_id;
        bool m_stream_from_disk;
        bool m_use_node_header_grouping;
        HFCPtr<HCDCodec>        m_pCodec;
        // NEEDS_WORK_SM_STREAMING: should only have one stream store for all data types
        WString m_storage_connection_string;
        scalable_mesh::azure::Storage m_stream_store;
        WString m_path_to_grouped_headers;
        bvector<HFCPtr<SMNodeGroup>> m_nodeHeaderGroups;
        mutable std::map<IDTMFile::NodeID, PointBlock> m_countInfo;
        std::condition_variable m_groupCV;
        std::mutex headerLock;

    };
