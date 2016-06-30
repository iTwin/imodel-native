//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/SMStreamingTileStore.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#pragma once



#include "SMPointTileStore.h"
#include "SMNodeGroup.h"
#include <ImagePP/all/h/HCDCodecZlib.h>
#include "ScalableMesh/Streaming/AzureStorage.h"
#include "Threading\LightThreadPool.h"
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
extern bool s_stream_from_disk;
extern bool s_stream_from_file_server;
extern bool s_stream_from_grouped_store;
extern bool s_is_virtual_grouping;

extern std::mutex fileMutex;

// Helper point block data structure
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
    void Load()
        {
        if (!this->IsLoaded())
            {
            if (this->IsLoading())
                {
                this->LockAndWait();
                }
            else
                {
                wchar_t buffer[10000];
                swprintf(buffer, L"%sp_%llu.bin", m_pDataSource.c_str(), m_pID);
                WString filename(buffer);
                if (s_stream_from_disk)
                    {
                    this->LoadFromLocal(filename);
                    }
                else if (s_stream_from_file_server)
                    {
                    this->LoadFromFileSystem(filename);
                    }
                else {
                    this->LoadFromAzure(filename);
                    }
                m_pIsLoaded = true;
                }
            }
        assert(this->IsLoaded());
        }
    void UnLoad()
        {
        m_pIsLoaded = false;
        m_pIsLoading = false;
        this->clear();
        }
    void SetLoaded()
        {
        m_pIsLoaded = true;
        m_pIsLoading = false;
        m_pPointBlockCV.notify_all();
        }
    void SetID(const uint64_t& pi_ID)
        {
        m_pID = pi_ID;
        }
    uint64_t GetID() { return m_pID; }
    void SetDataSource(const WString& pi_DataSource)
        {
        m_pDataSource = pi_DataSource;
        }
    void SetStore(const scalable_mesh::azure::Storage& pi_Store)
        {
        m_stream_store = &pi_Store;
        }
    void DecompressPoints(uint8_t* pi_CompressedData, uint32_t pi_CompressedDataSize, uint32_t pi_UncompressedDataSize)
        {
        HPRECONDITION(pi_CompressedDataSize <= (numeric_limits<uint32_t>::max) ());

        this->resize(pi_UncompressedDataSize);

        // initialize codec
        HFCPtr<HCDCodec> pCodec = new HCDCodecZlib(pi_CompressedDataSize);
        const size_t unCompressedDataSize = pCodec->DecompressSubset(pi_CompressedData,
                                                                     pi_CompressedDataSize,
                                                                     this->data(),
                                                                     pi_UncompressedDataSize);
        assert(unCompressedDataSize != 0 && pi_UncompressedDataSize == unCompressedDataSize);
        }

private:
    struct MemoryStruct {
        bvector<Byte>* memory;
        size_t         size;
        };
    static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
        {
        size_t realsize = size * nmemb;
        struct MemoryStruct *mem = (struct MemoryStruct *)userp;

        assert(mem->memory->capacity() >= mem->memory->size() + realsize);

        //    mem->memory->assign((Byte*)contents, (Byte*)contents + realsize);
        mem->memory->insert(mem->memory->end(), (Byte*)contents, (Byte*)contents + realsize);

        return realsize;
        }
    StatusInt LoadFromLocal(const WString& m_pFilename)
        {
        uint32_t uncompressedSize = 0;
        BeFile file;
        if (BeFileStatus::Success == OPEN_FILE_SHARE(file, m_pFilename.c_str(), BeFileAccess::Read))
            {

            size_t fileSize = 0;
            file.GetSize(fileSize);

            // Read uncompressed size
            uint32_t bytesRead = 0;
            auto read_result = file.Read(&uncompressedSize, &bytesRead, sizeof(uint32_t));
            assert(BeFileStatus::Success == read_result);
            assert(bytesRead == sizeof(uint32_t));

            // Read compressed points
            auto compressedSize = fileSize - sizeof(uint32_t);
            bvector<uint8_t> ptData(compressedSize);
            read_result = file.Read(&ptData[0], &bytesRead, (uint32_t)compressedSize);
            assert(bytesRead == compressedSize);
            assert(BeFileStatus::Success == read_result);
            file.Close();

            this->DecompressPoints(&ptData[0], (uint32_t)compressedSize, uncompressedSize);
            }
        else
            {
            //assert(!"Problem opening block of points for reading");
            file.Close();
            return ERROR_FILE_NOT_FOUND;
            }
        return SUCCESS;
        }
    StatusInt LoadFromAzure(const WString& m_pFilename)
        {
        bool blobDownloaded = false;
        m_stream_store->DownloadBlob(m_pFilename.c_str(), [this, &blobDownloaded](scalable_mesh::azure::Storage::point_buffer_type& buffer)
            {
            assert(!buffer.empty());

            uint32_t uncompressedSize = reinterpret_cast<uint32_t&>(buffer[0]);
            uint32_t sizeData = (uint32_t)buffer.size() - sizeof(uint32_t);
            this->DecompressPoints(buffer.data() + sizeof(uint32_t), sizeData, uncompressedSize);

            blobDownloaded = true;
            });
        return blobDownloaded ? SUCCESS : ERROR;
        }
    void LoadFromFileSystem(const WString& m_pFilename)
        {
        CURL *curl_handle;
        bool retCode = true;
        struct MemoryStruct chunk;
        const int maxCountData = 100000;
        Utf8String blockUrl = "http://realitydatastreaming.azurewebsites.net/Mesh/c1sub_scalablemesh/";
        Utf8String name;
        BeStringUtilities::WCharToUtf8(name, m_pFilename.c_str());
        blockUrl += name;

        chunk.memory = this;
        chunk.memory->reserve(maxCountData);
        chunk.size = 0;
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();

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
        CURLcode res = curl_easy_perform(curl_handle);
        /* check for errors */
        if (CURLE_OK != res)
            {
            retCode = false;
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            assert(false);
            }

        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();

        assert(!chunk.memory->empty() && chunk.memory->size() <= 1000000);

        uint32_t uncompressedSize = reinterpret_cast<uint32_t&>((*chunk.memory)[0]);
        uint32_t sizeData = (uint32_t)chunk.memory->size() - sizeof(uint32_t);

        this->DecompressPoints(&(*chunk.memory)[0] + sizeof(uint32_t), sizeData, uncompressedSize);
        }

private:
    bool m_pIsLoading = false;
    bool m_pIsLoaded = false;
    uint64_t m_pID = -1;
    WString m_pDataSource;
    const scalable_mesh::azure::Storage* m_stream_store;
    condition_variable m_pPointBlockCV;
    mutex m_pPointBlockMutex;
    };


template <typename POINT, typename EXTENT> class SMStreamingPointTaggedTileStore : public SMPointTileStore<POINT, EXTENT>// , public HFCShareableObject<SMPointTileStore<POINT, EXTENT> >
    {

    private:
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
                group->Load(blockID.m_integerID);
                }
            return group;
            }

        void ReadNodeHeaderFromBinary(SMPointNodeHeader<EXTENT>* header, uint8_t* headerData, uint64_t& maxCountData) const
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
            memcpy(&header->m_isTextured, headerData + dataIndex, sizeof(header->m_isTextured));
            dataIndex += sizeof(header->m_isTextured);
            memcpy(&header->m_nbFaceIndexes, headerData + dataIndex, sizeof(header->m_nbFaceIndexes));
            dataIndex += sizeof(header->m_nbFaceIndexes);
            uint32_t graphID;
            memcpy(&graphID, headerData + dataIndex, sizeof(graphID));
            header->m_graphID = graphID != IDTMFile::GetNullNodeID() ? HPMBlockID(graphID) : IDTMFile::GetNullNodeID();
            dataIndex += sizeof(graphID);

            memcpy(&header->m_nodeExtent, headerData + dataIndex, 6 * sizeof(double));
            dataIndex += 6 * sizeof(double);

            memcpy(&header->m_contentExtentDefined, headerData + dataIndex, sizeof(header->m_contentExtentDefined));
            dataIndex += sizeof(header->m_contentExtentDefined);
            if (header->m_contentExtentDefined)
                {
                memcpy(&header->m_contentExtent, headerData + dataIndex, 6 * sizeof(double));
                dataIndex += 6 * sizeof(double);
                }

            /* Indices */
            uint32_t idx;
            memcpy(&idx, headerData + dataIndex, sizeof(idx));
            dataIndex += sizeof(idx);
            header->m_ptsIndiceID.resize(1);
            header->m_ptsIndiceID[0] = idx;

            if (header->m_isTextured)
                {
                header->m_textureID.resize(1);
                header->m_textureID[0] = IDTMFile::GetNullNodeID();
                header->m_ptsIndiceID.resize(2);
                header->m_ptsIndiceID[1] = (int)idx;
                header->m_ptsIndiceID[0] = SQLiteNodeHeader::NO_NODEID;
                header->m_nbTextures = 1;
                header->m_uvsIndicesID.resize(1);
                header->m_uvsIndicesID[0] = idx;
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

        void GetNodeHeaderBinary(const HPMBlockID& blockID, std::unique_ptr<uint8_t>& po_pBinaryData, uint64_t& po_pDataSize)
            {
            wchar_t buffer[10000];
            swprintf(buffer, L"%sn_%llu.bin", m_pathToHeaders.c_str(), blockID.m_integerID);
            std::wstring filename(buffer);
            if (s_stream_from_disk)
                {
                BeFile file;
                if (BeFileStatus::Success != OPEN_FILE(file, filename.c_str(), BeFileAccess::Read))
                    {
                    assert(false); // node header file must exist
                    return;
                    }

                file.GetSize(po_pDataSize);
                po_pBinaryData.reset(new uint8_t[po_pDataSize]);

                uint32_t bytes_read = 0;
                file.Read(po_pBinaryData.get(), &bytes_read, po_pDataSize);
                assert(po_pDataSize == bytes_read);
                }
            else if (s_stream_from_file_server)
                {
                // NEEDS_WORK_SM_STREAMING: deactivate streaming from file server for now.
                assert(!"Streaming from file server is deactivated for the moment...");
                //ss << L".bin";
                //auto blob_name = ss.str();
                //bvector<Byte> buffer;
                //DownloadBlockFromFileServer(blob_name.c_str(), &buffer, 100000);
                //assert(!buffer.empty() && buffer.size() <= 100000);
                //Json::Reader reader;
                //reader.parse(reinterpret_cast<char*>(&buffer.front()), reinterpret_cast<char*>(&buffer.back()), result);
                }
            else {
                auto blob_name = filename.c_str();
                m_stream_store.DownloadBlob(blob_name, [&po_pBinaryData, &po_pDataSize](scalable_mesh::azure::Storage::point_buffer_type& buffer)
                    {
                    assert(!buffer.empty());
                    po_pDataSize = buffer.size();
                    po_pBinaryData.reset(new uint8_t[po_pDataSize]);
                    memmove(po_pBinaryData.get(), buffer.data(), po_pDataSize);
                    //Json::Reader reader;
                    //reader.parse(reinterpret_cast<char*>(&buffer.front()), reinterpret_cast<char*>(&buffer.back()), result);
                    });
                }
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

        Json::Value GetNodeHeaderJSON(HPMBlockID blockID)
            {
            uint64_t headerSize = 0;
            std::unique_ptr<Byte> headerData = nullptr;
            this->GetNodeHeaderBinary(blockID, headerData, headerSize);
            Json::Value result;
            Json::Reader().parse(reinterpret_cast<char*>(headerData.get()), reinterpret_cast<char*>(headerData.get() + headerSize), result);
            return result;
            }

        PointBlock& GetBlock(HPMBlockID blockID) const
            {
            // std::map [] operator is not thread safe while inserting new elements
            m_pointCacheLock.lock();
            PointBlock& block = m_pointCache[blockID.m_integerID];
            m_pointCacheLock.unlock();
            assert((block.GetID() != uint64_t(-1) ? block.GetID() == blockID.m_integerID : true));
            if (!block.IsLoaded())
                {
                block.SetID(blockID.m_integerID);
                block.SetDataSource(m_pathToPoints);
                block.SetStore(m_stream_store);
                block.Load();
                }
            assert(block.GetID() == blockID.m_integerID);
            assert(block.IsLoaded() && !block.empty());
            return block;
            }

    public:
        enum SMStreamingDataType
            {
            POINTS,
            INDICES,
            UVS,
            UVINDICES
            };

        // Constructor / Destructor

        SMStreamingPointTaggedTileStore(const WString& path, SMStreamingDataType type, bool compress = true, bool areNodeHeadersGrouped = false, WString headers_path = L"")
            :m_rootDirectory(path),
            m_pathToPoints(path),
            m_pathToHeaders(headers_path),
            m_use_node_header_grouping(areNodeHeadersGrouped),
            //m_storage_connection_string(L"DefaultEndpointsProtocol=https;AccountName=pcdsustest;BlobEndpoint=https://scalablemesh.azureedge.net;AccountKey=3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ=="),
            m_storage_connection_string(L"DefaultEndpointsProtocol=https;AccountName=pcdsustest;AccountKey=3EQ8Yb3SfocqbYpeIUxvwu/aEdiza+MFUDgQcIkrxkp435c7BxV8k2gd+F+iK/8V2iho80kFakRpZBRwFJh8wQ=="),
            m_stream_store(m_storage_connection_string.c_str(), L"scalablemeshtest")
            {
            bool haveHeaders = false;
            switch (type)
                {
                case SMStreamingDataType::POINTS:
                    m_pathToPoints += L"points/";
                    haveHeaders = true; // only points can carry node header information
                    break;
                case SMStreamingDataType::INDICES:
                    m_pathToPoints += L"indices/";
                    break;
                case SMStreamingDataType::UVS:
                    m_pathToPoints += L"uvs/";
                    break;
                case SMStreamingDataType::UVINDICES:
                    m_pathToPoints += L"uvindices/";
                    break;
                default:
                    assert(!"Unkown data type for streaming");
                }
            if (haveHeaders && m_pathToHeaders.empty())
                {
                // Set default path to headers relative to root directory
                m_pathToHeaders = m_rootDirectory + L"headers/";
                }

            // NEEDS_WORK_SM_STREAMING : create only directory structure if and only if in creation mode
            if (s_stream_from_disk)
                {
                // Create base directory structure to store information if not already done
                // NEEDS_WORK_SM_STREAMING : directory/file functions are Windows only
                if (0 == CreateDirectoryW(m_rootDirectory.c_str(), NULL))
                    {
                    assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }

                if (0 == CreateDirectoryW(m_pathToPoints.c_str(), NULL))
                    {
                    assert(ERROR_PATH_NOT_FOUND != GetLastError());
                    }

                if (haveHeaders)
                    {
                    if (0 == CreateDirectoryW(m_pathToHeaders.c_str(), NULL))
                        {
                        assert(ERROR_PATH_NOT_FOUND != GetLastError());
                        }
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
            assert(!"TODO!");
            return false;
            }


        // New function
        virtual std::string GetSpatialReferenceSystem()
            {
            // NEEDS_WORK_SM_STREAMING : Add check to spatial reference system
            assert(!"TODO!");
            return string("");
            }

        // ITileStore interface
        virtual void Close()
            {
            }

        virtual bool StoreMasterHeader(SMPointIndexHeader<EXTENT>* indexHeader, size_t headerSize)
            {
            if (indexHeader != NULL && indexHeader->m_rootNodeBlockID.IsValid())
                {
                Json::Value masterHeader;
                masterHeader["balanced"] = indexHeader->m_balanced;
                masterHeader["depth"] = (uint32_t)indexHeader->m_depth;
                masterHeader["rootNodeBlockID"] = ConvertBlockID(indexHeader->m_rootNodeBlockID);
                masterHeader["splitThreshold"] = indexHeader->m_SplitTreshold;
                masterHeader["singleFile"] = false;
                masterHeader["isTerrain"] = true;

                // Write to file
                auto filename = (m_rootDirectory + L"MasterHeader.sscm").c_str();
                BeFile file;
                uint64_t buffer_size;
                auto jsonWriter = [&file, &indexHeader, &buffer_size](BeFile& file, Json::Value& object) {

                    Json::StyledWriter writer;
                    auto buffer = writer.write(object);
                    buffer_size = buffer.size();
                    file.Write(NULL, buffer.c_str(), buffer_size);
                    };
                if (BeFileStatus::Success == OPEN_FILE(file, filename, BeFileAccess::Write))//file.Open(filename, BeFileAccess::Write, BeFileSharing::None))
                    {
                    jsonWriter(file, masterHeader);
                    }
                else if (BeFileStatus::Success == file.Create(filename))
                    {
                    jsonWriter(file, masterHeader);
                    }
                else
                    {
                    assert(!"Problem creating master header file");
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
                    wchar_t buffer[10000];
                    swprintf(buffer, L"%sMasterHeaderWithGroups.bin", m_rootDirectory.c_str());
                    std::wstring filename(buffer);
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
                        indexHeader->m_isTerrain = oldMasterHeader.m_isTerrain;
                        indexHeader->m_singleFile = oldMasterHeader.m_singleFile;
                        assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

                        auto rootNodeBlockID = oldMasterHeader.m_rootNodeBlockID;
                        indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();

                        memcpy(&s_is_virtual_grouping, masterHeaderBuffer + position, sizeof(s_is_virtual_grouping));
                        position += sizeof(s_is_virtual_grouping);

                        // Parse rest of file -- group information
                        while (position < headerSize)
                            {
                            size_t group_id;
                            memcpy(&group_id, masterHeaderBuffer + position, sizeof(group_id));
                            position += sizeof(group_id);

                            uint64_t group_totalSizeOfHeaders(0);
                            if (s_is_virtual_grouping)
                                {
                                memcpy(&group_totalSizeOfHeaders, masterHeaderBuffer + position, sizeof(group_totalSizeOfHeaders));
                                position += sizeof(group_totalSizeOfHeaders);
                                }

                            size_t group_numNodes;
                            memcpy(&group_numNodes, masterHeaderBuffer + position, sizeof(size_t));
                            position += sizeof(size_t);
                            //assert(group_size <= s_max_number_nodes_in_group);

                            auto group = HFCPtr<SMNodeGroup>(new SMNodeGroup(group_id, group_numNodes, group_totalSizeOfHeaders));
                            // NEEDS_WORK_SM : group datasource doesn't need to depend on type of grouping
                            group->SetDataSource(s_is_virtual_grouping ? m_pathToHeaders : m_pathToHeaders + L"g_", m_stream_store);
                            m_nodeHeaderGroups.push_back(group);

                            vector<uint64_t> nodeIds(group_numNodes);
                            memcpy(nodeIds.data(), masterHeaderBuffer + position, group_numNodes*sizeof(uint64_t));
                            position += group_numNodes*sizeof(uint64_t);

                            group->GetHeader()->resize(group_numNodes);
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
                    auto filename = (m_rootDirectory + L"MasterHeader.sscm").c_str();
                    if (BeFileStatus::Success != OPEN_FILE(file, filename, BeFileAccess::Read))//file.Open(filename, BeFileAccess::Read, BeFileSharing::None))
                        {
                        //assert(!"Local master header could not be found"); // possible during SM generation
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
                    indexHeader->m_isTerrain = masterHeader["isTerrain"].asBool();
                    indexHeader->m_singleFile = masterHeader["singleFile"].asBool();
                    assert(indexHeader->m_singleFile == false); // cloud is always multifile. So if we use streamingTileStore without multiFile, there are problem

                    auto rootNodeBlockID = masterHeader["rootNodeBlockID"].asUInt();
                    indexHeader->m_rootNodeBlockID = rootNodeBlockID != IDTMFile::GetNullNodeID() ? HPMBlockID(rootNodeBlockID) : HPMBlockID();
                    file.Close();
                    }
                else {
                    auto blob_name = m_rootDirectory + L"MasterHeader.sscm";
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
                        indexHeader->m_isTerrain = masterHeader["isTerrain"].asBool();

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

            if (NULL != DataTypeArray && countData > 0)
                {
                wchar_t buffer[10000];
                swprintf(buffer, L"%sp_%llu.bin", m_pathToPoints.c_str(), blockID.m_integerID);
                std::wstring filename(buffer);
                BeFile file;
                auto fileOpened = OPEN_FILE(file, filename.c_str(), BeFileAccess::Write);
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
                if (m_pointCache.count(blockID.m_integerID) > 0)
                    {
                    // must update data count
                    auto& points = this->m_pointCache[blockID.m_integerID];
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

        void SerializeHeaderToBinary(const SMPointNodeHeader<EXTENT>* pi_pHeader, std::unique_ptr<Byte>& po_pBinaryData, uint32_t& po_pDataSize) const
            {
            assert(po_pBinaryData == nullptr && po_pDataSize == 0);

            po_pBinaryData.reset(new Byte[3000]);

            const auto filtered = pi_pHeader->m_filtered;
            memcpy(po_pBinaryData.get() + po_pDataSize, &filtered, sizeof(filtered));
            po_pDataSize += sizeof(filtered);
            const auto parentBlockID = pi_pHeader->m_parentNodeID.IsValid() ? ConvertBlockID(pi_pHeader->m_parentNodeID) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &parentBlockID, sizeof(parentBlockID));
            po_pDataSize += sizeof(parentBlockID);
            const auto subNodeNoSplitID = pi_pHeader->m_SubNodeNoSplitID.IsValid() ? ConvertBlockID(pi_pHeader->m_SubNodeNoSplitID) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &subNodeNoSplitID, sizeof(subNodeNoSplitID));
            po_pDataSize += sizeof(subNodeNoSplitID);
            const auto level = pi_pHeader->m_level;
            memcpy(po_pBinaryData.get() + po_pDataSize, &level, sizeof(level));
            po_pDataSize += sizeof(level);
            const auto isBranched = pi_pHeader->m_IsBranched;
            memcpy(po_pBinaryData.get() + po_pDataSize, &isBranched, sizeof(isBranched));
            po_pDataSize += sizeof(isBranched);
            const auto isLeaf = pi_pHeader->m_IsLeaf;
            memcpy(po_pBinaryData.get() + po_pDataSize, &isLeaf, sizeof(isLeaf));
            po_pDataSize += sizeof(isLeaf);
            const auto splitThreshold = pi_pHeader->m_SplitTreshold;
            memcpy(po_pBinaryData.get() + po_pDataSize, &splitThreshold, sizeof(splitThreshold));
            po_pDataSize += sizeof(splitThreshold);
            const auto totalCount = pi_pHeader->m_totalCount;
            memcpy(po_pBinaryData.get() + po_pDataSize, &totalCount, sizeof(totalCount));
            po_pDataSize += sizeof(totalCount);
            const auto nodeCount = pi_pHeader->m_nodeCount;
            memcpy(po_pBinaryData.get() + po_pDataSize, &nodeCount, sizeof(nodeCount));
            po_pDataSize += sizeof(nodeCount);
            const auto arePoints3d = pi_pHeader->m_arePoints3d;
            memcpy(po_pBinaryData.get() + po_pDataSize, &arePoints3d, sizeof(arePoints3d));
            po_pDataSize += sizeof(arePoints3d);
            const auto isTextured = pi_pHeader->m_isTextured;
            memcpy(po_pBinaryData.get() + po_pDataSize, &isTextured, sizeof(isTextured));
            po_pDataSize += sizeof(isTextured);
            const auto nbFaceIndexes = pi_pHeader->m_nbFaceIndexes;
            memcpy(po_pBinaryData.get() + po_pDataSize, &nbFaceIndexes, sizeof(nbFaceIndexes));
            po_pDataSize += sizeof(nbFaceIndexes);
            const auto graphID = pi_pHeader->m_graphID.IsValid() ? ConvertBlockID(pi_pHeader->m_graphID) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &graphID, sizeof(graphID));
            po_pDataSize += sizeof(graphID);

            memcpy(po_pBinaryData.get() + po_pDataSize, &pi_pHeader->m_nodeExtent, 6 * sizeof(double));
            po_pDataSize += 6 * sizeof(double);

            const auto contentExtentDefined = pi_pHeader->m_contentExtentDefined;
            memcpy(po_pBinaryData.get() + po_pDataSize, &contentExtentDefined, sizeof(contentExtentDefined));
            po_pDataSize += sizeof(contentExtentDefined);
            if (contentExtentDefined)
                {
                memcpy(po_pBinaryData.get() + po_pDataSize, &pi_pHeader->m_contentExtent, 6 * sizeof(double));
                po_pDataSize += 6 * sizeof(double);
                }

            /* Indice IDs */
            const auto idx = pi_pHeader->m_ptsIndiceID[0].IsValid() ? ConvertBlockID(pi_pHeader->m_ptsIndiceID[0]) : IDTMFile::GetNullNodeID();
            memcpy(po_pBinaryData.get() + po_pDataSize, &idx, sizeof(idx));
            po_pDataSize += sizeof(idx);


            /* Mesh components and clips */
            const auto numberOfMeshComponents = pi_pHeader->m_numberOfMeshComponents;
            memcpy(po_pBinaryData.get() + po_pDataSize, &numberOfMeshComponents, sizeof(numberOfMeshComponents));
            po_pDataSize += sizeof(numberOfMeshComponents);
            for (size_t componentIdx = 0; componentIdx < pi_pHeader->m_numberOfMeshComponents; componentIdx++)
                {
                const auto component = pi_pHeader->m_meshComponents[componentIdx];
                memcpy(po_pBinaryData.get() + po_pDataSize, &component, sizeof(component));
                po_pDataSize += sizeof(component);
                }

            const auto nbClipSetsIDs = (uint32_t)pi_pHeader->m_clipSetsID.size();
            memcpy(po_pBinaryData.get() + po_pDataSize, &nbClipSetsIDs, sizeof(nbClipSetsIDs));
            po_pDataSize += sizeof(nbClipSetsIDs);
            for (size_t i = 0; i < nbClipSetsIDs; ++i)
                {
                const auto clip = ConvertNeighborID(pi_pHeader->m_clipSetsID[i]);
                memcpy(po_pBinaryData.get() + po_pDataSize, &clip, sizeof(clip));
                po_pDataSize += sizeof(clip);
                }

            /* Children and Neighbors */
            const auto nbChildren = isLeaf || (!isBranched  && !pi_pHeader->m_SubNodeNoSplitID.IsValid()) ? 0 : (!isBranched ? 1 : pi_pHeader->m_numberOfSubNodesOnSplit);
            memcpy(po_pBinaryData.get() + po_pDataSize, &nbChildren, sizeof(nbChildren));
            po_pDataSize += sizeof(nbChildren);
            for (size_t childInd = 0; childInd < nbChildren; childInd++)
                {
                const auto id = ConvertChildID(pi_pHeader->m_apSubNodeID[childInd]);
                memcpy(po_pBinaryData.get() + po_pDataSize, &id, sizeof(id));
                po_pDataSize += sizeof(id);
                }

            for (size_t neighborPosInd = 0; neighborPosInd < MAX_NEIGHBORNODES_COUNT; neighborPosInd++)
                {
                const auto numNeighbors = pi_pHeader->m_apNeighborNodeID[neighborPosInd].size();
                memcpy(po_pBinaryData.get() + po_pDataSize, &numNeighbors, sizeof(numNeighbors));
                po_pDataSize += sizeof(numNeighbors);
                for (size_t neighborInd = 0; neighborInd < numNeighbors; neighborInd++)
                    {
                    const auto nodeId = ConvertNeighborID(pi_pHeader->m_apNeighborNodeID[neighborPosInd][neighborInd]);
                    memcpy(po_pBinaryData.get() + po_pDataSize, &nodeId, sizeof(nodeId));
                    po_pDataSize += sizeof(nodeId);
                    }
                }
            }

        void SerializeHeaderToJSON(const SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID, Json::Value& block)
            {
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
            }

        virtual size_t StoreHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            uint32_t headerSize = 0;
            std::unique_ptr<Byte> headerData = nullptr;
            SerializeHeaderToBinary(header, headerData, headerSize);
            //SerializeHeaderToJSON(header, blockID, block);

            wchar_t buffer[10000];
            swprintf(buffer, L"%sn_%llu.bin", m_pathToHeaders.c_str(), blockID.m_integerID);
            std::wstring filename(buffer);
            BeFile file;
            if (BeFileStatus::Success == OPEN_FILE(file, filename.c_str(), BeFileAccess::Write) || BeFileStatus::Success == file.Create(filename.c_str()))
                {
                //    Json::StyledWriter writer;
                //    auto buffer = writer.write(object);
                //    buffer_size = buffer.size();
                //    file.Write(NULL, buffer.c_str(), buffer_size);
                file.Write(NULL, headerData.get(), headerSize);
                }
            else
                {
                assert(!"Problem opening/creating header file");
                }
            file.Close();

            return 1;
            }

        virtual size_t LoadHeader(SMPointNodeHeader<EXTENT>* header, HPMBlockID blockID)
            {
            if (s_stream_from_grouped_store)
                {
                auto group = this->GetGroup(blockID);
                auto node_header = group->GetNodeHeader(blockID.m_integerID);
                ReadNodeHeaderFromBinary(header, group->GetRawHeaders(node_header.offset), node_header.size);
                //group->removeNodeData(blockID.m_integerID);
                }
            else {
                //auto nodeHeader = this->GetNodeHeaderJSON(blockID);
                //ReadNodeHeaderFromJSON(header, nodeHeader);
                uint64_t headerSize = 0;
                std::unique_ptr<Byte> headerData = nullptr;
                this->GetNodeHeaderBinary(blockID, headerData, headerSize);
                ReadNodeHeaderFromBinary(header, headerData.get(), headerSize);
                }
            return 1;
            }

        virtual size_t LoadBlock(POINT* DataTypeArray, size_t maxCountData, HPMBlockID blockID)
            {
            auto& block = this->GetBlock(blockID);
            auto blockSize = block.size();
            assert(block.size() <= maxCountData * sizeof(POINT));
            memmove(DataTypeArray, block.data(), block.size());
            block.UnLoad();

            return blockSize;
            }

        virtual bool DestroyBlock(HPMBlockID blockID)
            {
            return true;
            }

    private:
        WString m_rootDirectory;
        WString m_pathToPoints;
        WString m_pathToHeaders;
        bool m_use_node_header_grouping;
        // NEEDS_WORK_SM_STREAMING: should only have one stream store for all data types
        WString m_storage_connection_string;
        scalable_mesh::azure::Storage m_stream_store;
        bvector<HFCPtr<SMNodeGroup>> m_nodeHeaderGroups;
        // Use cache to avoid refetching data after a call to GetBlockDataCount(); cache is cleared when data has been received and returned by the store
        mutable std::map<IDTMFile::NodeID, PointBlock> m_pointCache;
        mutable std::mutex m_pointCacheLock;
    };
